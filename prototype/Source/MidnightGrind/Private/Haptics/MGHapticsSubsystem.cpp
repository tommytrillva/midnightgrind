// Copyright Midnight Grind. All Rights Reserved.

#include "Haptics/MGHapticsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGHapticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default configuration
	Config.bEnabled = true;
	Config.GlobalIntensity = 1.0f;
	Config.EngineVibrationIntensity = 1.0f;
	Config.CollisionIntensity = 1.0f;
	Config.SurfaceIntensity = 1.0f;
	Config.bAdaptiveTriggers = true;
	Config.TriggerIntensity = 1.0f;
	Config.bBrakeTriggerFeedback = true;
	Config.bThrottleTriggerFeedback = true;
	Config.MaxConcurrentHaptics = 4;
	Config.bReduceOnLowBattery = true;
	Config.LowBatteryThreshold = 0.2f;

	InitializeDefaultPatterns();

	// Initialize surface feedback map
	FMGSurfaceFeedback AsphaltFeedback;
	AsphaltFeedback.SurfaceType = EMGSurfaceType::Asphalt;
	AsphaltFeedback.BaseFrequency = 0.0f;
	AsphaltFeedback.BaseAmplitude = 0.0f;
	AsphaltFeedback.SpeedMultiplier = 0.0f;
	AsphaltFeedback.TriggerResistance = 0.0f;
	SurfaceFeedbackMap.Add(EMGSurfaceType::Asphalt, AsphaltFeedback);

	FMGSurfaceFeedback GravelFeedback;
	GravelFeedback.SurfaceType = EMGSurfaceType::Gravel;
	GravelFeedback.BaseFrequency = 30.0f;
	GravelFeedback.BaseAmplitude = 0.3f;
	GravelFeedback.SpeedMultiplier = 0.5f;
	GravelFeedback.TriggerResistance = 0.2f;
	SurfaceFeedbackMap.Add(EMGSurfaceType::Gravel, GravelFeedback);

	FMGSurfaceFeedback DirtFeedback;
	DirtFeedback.SurfaceType = EMGSurfaceType::Dirt;
	DirtFeedback.BaseFrequency = 20.0f;
	DirtFeedback.BaseAmplitude = 0.2f;
	DirtFeedback.SpeedMultiplier = 0.4f;
	DirtFeedback.TriggerResistance = 0.15f;
	SurfaceFeedbackMap.Add(EMGSurfaceType::Dirt, DirtFeedback);

	FMGSurfaceFeedback RumblestripFeedback;
	RumblestripFeedback.SurfaceType = EMGSurfaceType::Rumblestrip;
	RumblestripFeedback.BaseFrequency = 60.0f;
	RumblestripFeedback.BaseAmplitude = 0.7f;
	RumblestripFeedback.SpeedMultiplier = 0.8f;
	RumblestripFeedback.TriggerResistance = 0.0f;
	SurfaceFeedbackMap.Add(EMGSurfaceType::Rumblestrip, RumblestripFeedback);

	FMGSurfaceFeedback WetFeedback;
	WetFeedback.SurfaceType = EMGSurfaceType::Wet;
	WetFeedback.BaseFrequency = 10.0f;
	WetFeedback.BaseAmplitude = 0.1f;
	WetFeedback.SpeedMultiplier = 0.3f;
	WetFeedback.TriggerResistance = 0.0f;
	WetFeedback.bAffectsSteering = true;
	SurfaceFeedbackMap.Add(EMGSurfaceType::Wet, WetFeedback);

	// Start haptics tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HapticsTickHandle,
			this,
			&UMGHapticsSubsystem::OnHapticsTick,
			0.016f, // ~60 Hz
			true
		);
	}
}

void UMGHapticsSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HapticsTickHandle);
	}

	StopAllHaptics();
	ClearTriggerFeedback();

	Super::Deinitialize();
}

bool UMGHapticsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FGuid UMGHapticsSubsystem::PlayHaptic(EMGHapticType Type, float Intensity, int32 Priority)
{
	if (!Config.bEnabled)
	{
		return FGuid();
	}

	FMGHapticPattern Pattern = GetDefaultPattern(Type);
	return PlayHapticPattern(Pattern, Intensity, Priority);
}

FGuid UMGHapticsSubsystem::PlayHapticPattern(const FMGHapticPattern& Pattern, float IntensityMultiplier, int32 Priority)
{
	if (!Config.bEnabled)
	{
		return FGuid();
	}

	// Check if we need to make room
	int32 Slot = GetNextPrioritySlot(Priority);
	if (Slot == INDEX_NONE && ActiveHaptics.Num() >= Config.MaxConcurrentHaptics)
	{
		return FGuid();
	}

	FMGActiveHaptic NewHaptic;
	NewHaptic.HapticID = FGuid::NewGuid();
	NewHaptic.Pattern = Pattern;
	NewHaptic.IntensityMultiplier = IntensityMultiplier;
	NewHaptic.Priority = Priority;
	NewHaptic.CurrentStep = 0;

	// Calculate total duration
	float TotalDuration = 0.0f;
	for (float Duration : Pattern.Durations)
	{
		TotalDuration += Duration;
	}
	NewHaptic.TimeRemaining = Pattern.bLooping ? FLT_MAX : TotalDuration;

	if (Pattern.Durations.Num() > 0)
	{
		NewHaptic.StepTimeRemaining = Pattern.Durations[0];
	}

	// Insert or replace based on priority
	if (Slot != INDEX_NONE)
	{
		ActiveHaptics[Slot] = NewHaptic;
	}
	else
	{
		ActiveHaptics.Add(NewHaptic);
	}

	OnHapticStarted.Broadcast(NewHaptic.HapticID);

	return NewHaptic.HapticID;
}

FGuid UMGHapticsSubsystem::PlayHapticPulse(float Intensity, float Duration, EMGHapticChannel Channel)
{
	FMGHapticPattern Pattern;
	Pattern.PatternID = FName("Pulse");
	Pattern.Amplitudes.Add(Intensity);
	Pattern.Frequencies.Add(100.0f);
	Pattern.Durations.Add(Duration);
	Pattern.Channel = Channel;
	Pattern.bLooping = false;

	return PlayHapticPattern(Pattern, 1.0f, 0);
}

void UMGHapticsSubsystem::StopHaptic(FGuid HapticID)
{
	for (int32 i = ActiveHaptics.Num() - 1; i >= 0; i--)
	{
		if (ActiveHaptics[i].HapticID == HapticID)
		{
			ActiveHaptics.RemoveAt(i);
			OnHapticStopped.Broadcast(HapticID);
			return;
		}
	}
}

void UMGHapticsSubsystem::StopAllHaptics()
{
	for (const FMGActiveHaptic& Haptic : ActiveHaptics)
	{
		OnHapticStopped.Broadcast(Haptic.HapticID);
	}
	ActiveHaptics.Empty();
}

void UMGHapticsSubsystem::PauseHaptic(FGuid HapticID)
{
	for (FMGActiveHaptic& Haptic : ActiveHaptics)
	{
		if (Haptic.HapticID == HapticID)
		{
			Haptic.bPaused = true;
			return;
		}
	}
}

void UMGHapticsSubsystem::ResumeHaptic(FGuid HapticID)
{
	for (FMGActiveHaptic& Haptic : ActiveHaptics)
	{
		if (Haptic.HapticID == HapticID)
		{
			Haptic.bPaused = false;
			return;
		}
	}
}

void UMGHapticsSubsystem::UpdateEngineRPM(float RPM, float MaxRPM)
{
	if (MaxRPM > 0.0f)
	{
		CurrentEngineRPMPercent = FMath::Clamp(RPM / MaxRPM, 0.0f, 1.0f);
	}
	UpdateContinuousFeedback();
}

void UMGHapticsSubsystem::OnGearShift(bool bUpshift)
{
	float Intensity = bUpshift ? 0.5f : 0.7f;
	PlayHaptic(EMGHapticType::GearShift, Intensity, 5);
}

void UMGHapticsSubsystem::OnRedline(bool bInRedline)
{
	// Start or stop redline haptic
	static FGuid RedlineHapticID;

	if (bInRedline && !RedlineHapticID.IsValid())
	{
		FMGHapticPattern Pattern;
		Pattern.PatternID = FName("Redline");
		Pattern.HapticType = EMGHapticType::RedlineWarning;
		Pattern.Amplitudes.Add(0.3f);
		Pattern.Amplitudes.Add(0.6f);
		Pattern.Frequencies.Add(80.0f);
		Pattern.Frequencies.Add(120.0f);
		Pattern.Durations.Add(0.1f);
		Pattern.Durations.Add(0.1f);
		Pattern.bLooping = true;
		Pattern.LoopInterval = 0.0f;

		RedlineHapticID = PlayHapticPattern(Pattern, 1.0f, 2);
	}
	else if (!bInRedline && RedlineHapticID.IsValid())
	{
		StopHaptic(RedlineHapticID);
		RedlineHapticID.Invalidate();
	}
}

void UMGHapticsSubsystem::UpdateSpeed(float SpeedKPH)
{
	CurrentSpeed = SpeedKPH;
	UpdateContinuousFeedback();
}

void UMGHapticsSubsystem::OnCollision(float ImpactForce, FVector ImpactDirection)
{
	float Intensity = FMath::Clamp(ImpactForce / 1000.0f, 0.1f, 1.0f);
	Intensity *= Config.CollisionIntensity;

	// Determine channel based on impact direction
	EMGHapticChannel Channel = EMGHapticChannel::Both;
	if (FMath::Abs(ImpactDirection.Y) > 0.5f)
	{
		Channel = ImpactDirection.Y > 0 ? EMGHapticChannel::RightOnly : EMGHapticChannel::LeftOnly;
	}

	FMGHapticPattern Pattern;
	Pattern.PatternID = FName("Collision");
	Pattern.HapticType = EMGHapticType::Collision;
	Pattern.Amplitudes.Add(Intensity);
	Pattern.Amplitudes.Add(Intensity * 0.5f);
	Pattern.Amplitudes.Add(Intensity * 0.2f);
	Pattern.Frequencies.Add(150.0f);
	Pattern.Frequencies.Add(100.0f);
	Pattern.Frequencies.Add(50.0f);
	Pattern.Durations.Add(0.1f);
	Pattern.Durations.Add(0.15f);
	Pattern.Durations.Add(0.1f);
	Pattern.Channel = Channel;

	PlayHapticPattern(Pattern, 1.0f, 10);
}

void UMGHapticsSubsystem::OnLanding(float ImpactForce)
{
	float Intensity = FMath::Clamp(ImpactForce / 500.0f, 0.2f, 1.0f);
	PlayHaptic(EMGHapticType::LandingImpact, Intensity, 8);
}

void UMGHapticsSubsystem::OnDriftStart()
{
	bIsDrifting = true;
	PlayHaptic(EMGHapticType::Drift, 0.3f, 3);
}

void UMGHapticsSubsystem::OnDriftEnd()
{
	bIsDrifting = false;
}

void UMGHapticsSubsystem::UpdateDriftAngle(float Angle)
{
	CurrentDriftAngle = Angle;
}

void UMGHapticsSubsystem::OnBoostActivate()
{
	bIsBoosting = true;
	PlayHaptic(EMGHapticType::NitroActivate, 0.8f, 7);
}

void UMGHapticsSubsystem::OnBoostDeactivate()
{
	bIsBoosting = false;
	CurrentBoostIntensity = 0.0f;
}

void UMGHapticsSubsystem::UpdateBoostIntensity(float Intensity)
{
	CurrentBoostIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGHapticsSubsystem::SetCurrentSurface(EMGSurfaceType Surface)
{
	if (CurrentSurface != Surface)
	{
		CurrentSurface = Surface;
		OnSurfaceChanged.Broadcast(Surface);

		// Trigger surface change feedback
		PlayHaptic(EMGHapticType::SurfaceChange, 0.3f, 1);
	}
}

void UMGHapticsSubsystem::UpdateSurfaceFeedback(float SpeedMultiplier)
{
	UpdateContinuousFeedback();
}

void UMGHapticsSubsystem::SetTriggerFeedback(const FMGTriggerFeedback& Feedback)
{
	if (!Config.bAdaptiveTriggers)
	{
		return;
	}

	CurrentTriggerFeedback = Feedback;

	// Apply intensity modifiers
	CurrentTriggerFeedback.LeftStrength *= Config.TriggerIntensity;
	CurrentTriggerFeedback.RightStrength *= Config.TriggerIntensity;

	OnTriggerFeedbackChanged.Broadcast(CurrentTriggerFeedback);

	// This would call platform-specific trigger APIs
}

void UMGHapticsSubsystem::SetBrakeTrigger(float Resistance, float StartPosition)
{
	if (!Config.bBrakeTriggerFeedback)
	{
		return;
	}

	FMGTriggerFeedback Feedback = CurrentTriggerFeedback;
	Feedback.LeftEffect = EMGTriggerEffect::Resistance;
	Feedback.LeftResistance = FMath::Clamp(Resistance, 0.0f, 1.0f);
	Feedback.LeftStartPosition = FMath::Clamp(StartPosition, 0.0f, 1.0f);
	SetTriggerFeedback(Feedback);
}

void UMGHapticsSubsystem::SetThrottleTrigger(float Resistance, float Frequency)
{
	if (!Config.bThrottleTriggerFeedback)
	{
		return;
	}

	FMGTriggerFeedback Feedback = CurrentTriggerFeedback;
	if (Frequency > 0.0f)
	{
		Feedback.RightEffect = EMGTriggerEffect::Vibration;
		Feedback.RightFrequency = Frequency;
	}
	else
	{
		Feedback.RightEffect = EMGTriggerEffect::Resistance;
	}
	Feedback.RightResistance = FMath::Clamp(Resistance, 0.0f, 1.0f);
	SetTriggerFeedback(Feedback);
}

void UMGHapticsSubsystem::ClearTriggerFeedback()
{
	CurrentTriggerFeedback = FMGTriggerFeedback();
	OnTriggerFeedbackChanged.Broadcast(CurrentTriggerFeedback);
}

void UMGHapticsSubsystem::OnCheckpointPassed()
{
	PlayHaptic(EMGHapticType::CheckpointPass, 0.5f, 6);
}

void UMGHapticsSubsystem::OnLapCompleted()
{
	PlayHaptic(EMGHapticType::LapComplete, 0.7f, 6);
}

void UMGHapticsSubsystem::OnRaceFinished(int32 Position)
{
	float Intensity = Position <= 3 ? 1.0f : 0.5f;
	PlayHaptic(EMGHapticType::RaceFinish, Intensity, 9);
}

void UMGHapticsSubsystem::OnCountdownTick(int32 TickNumber)
{
	PlayHaptic(EMGHapticType::CountdownTick, 0.4f, 8);
}

void UMGHapticsSubsystem::OnCountdownGo()
{
	PlayHaptic(EMGHapticType::CountdownGo, 0.9f, 9);
}

void UMGHapticsSubsystem::RegisterPattern(const FMGHapticPattern& Pattern)
{
	RegisteredPatterns.Add(Pattern.PatternID, Pattern);
}

FMGHapticPattern UMGHapticsSubsystem::GetPattern(FName PatternID) const
{
	const FMGHapticPattern* Pattern = RegisteredPatterns.Find(PatternID);
	return Pattern ? *Pattern : FMGHapticPattern();
}

TArray<FMGHapticPattern> UMGHapticsSubsystem::GetAllPatterns() const
{
	TArray<FMGHapticPattern> Result;
	for (const auto& Pair : RegisteredPatterns)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

void UMGHapticsSubsystem::SetConfig(const FMGHapticsConfig& NewConfig)
{
	Config = NewConfig;

	if (!Config.bEnabled)
	{
		StopAllHaptics();
		ClearTriggerFeedback();
	}
}

void UMGHapticsSubsystem::SetEnabled(bool bEnabled)
{
	Config.bEnabled = bEnabled;
	if (!bEnabled)
	{
		StopAllHaptics();
		ClearTriggerFeedback();
	}
}

void UMGHapticsSubsystem::SetGlobalIntensity(float Intensity)
{
	Config.GlobalIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
}

TArray<FMGActiveHaptic> UMGHapticsSubsystem::GetActiveHaptics() const
{
	return ActiveHaptics;
}

bool UMGHapticsSubsystem::IsHapticActive(FGuid HapticID) const
{
	for (const FMGActiveHaptic& Haptic : ActiveHaptics)
	{
		if (Haptic.HapticID == HapticID)
		{
			return true;
		}
	}
	return false;
}

bool UMGHapticsSubsystem::HasAdaptiveTriggerSupport() const
{
	// This would check platform capabilities
	return true;
}

float UMGHapticsSubsystem::GetControllerBatteryLevel() const
{
	return ControllerBatteryLevel;
}

void UMGHapticsSubsystem::OnHapticsTick()
{
	if (!Config.bEnabled)
	{
		return;
	}

	float DeltaTime = 0.016f;
	UpdateActiveHaptics(DeltaTime);
	ApplyHapticOutput();
}

void UMGHapticsSubsystem::UpdateActiveHaptics(float DeltaTime)
{
	for (int32 i = ActiveHaptics.Num() - 1; i >= 0; i--)
	{
		FMGActiveHaptic& Haptic = ActiveHaptics[i];

		if (Haptic.bPaused)
		{
			continue;
		}

		Haptic.StepTimeRemaining -= DeltaTime;
		Haptic.TimeRemaining -= DeltaTime;

		// Move to next step if current is done
		if (Haptic.StepTimeRemaining <= 0.0f)
		{
			Haptic.CurrentStep++;

			if (Haptic.CurrentStep >= Haptic.Pattern.Durations.Num())
			{
				if (Haptic.Pattern.bLooping)
				{
					Haptic.CurrentStep = 0;
					if (Haptic.Pattern.Durations.Num() > 0)
					{
						Haptic.StepTimeRemaining = Haptic.Pattern.Durations[0];
					}
				}
				else
				{
					// Haptic complete
					FGuid CompletedID = Haptic.HapticID;
					ActiveHaptics.RemoveAt(i);
					OnHapticStopped.Broadcast(CompletedID);
					continue;
				}
			}
			else
			{
				Haptic.StepTimeRemaining = Haptic.Pattern.Durations[Haptic.CurrentStep];
			}
		}

		// Check overall duration
		if (!Haptic.Pattern.bLooping && Haptic.TimeRemaining <= 0.0f)
		{
			FGuid CompletedID = Haptic.HapticID;
			ActiveHaptics.RemoveAt(i);
			OnHapticStopped.Broadcast(CompletedID);
		}
	}
}

void UMGHapticsSubsystem::ApplyHapticOutput()
{
	float LeftIntensity = 0.0f;
	float RightIntensity = 0.0f;

	// Combine all active haptics
	for (const FMGActiveHaptic& Haptic : ActiveHaptics)
	{
		if (Haptic.bPaused)
		{
			continue;
		}

		int32 Step = FMath::Clamp(Haptic.CurrentStep, 0, Haptic.Pattern.Amplitudes.Num() - 1);
		if (Step >= 0 && Step < Haptic.Pattern.Amplitudes.Num())
		{
			float Amplitude = Haptic.Pattern.Amplitudes[Step] * Haptic.IntensityMultiplier;

			switch (Haptic.Pattern.Channel)
			{
			case EMGHapticChannel::Both:
				LeftIntensity = FMath::Max(LeftIntensity, Amplitude);
				RightIntensity = FMath::Max(RightIntensity, Amplitude);
				break;
			case EMGHapticChannel::LeftOnly:
				LeftIntensity = FMath::Max(LeftIntensity, Amplitude);
				break;
			case EMGHapticChannel::RightOnly:
				RightIntensity = FMath::Max(RightIntensity, Amplitude);
				break;
			}
		}
	}

	ApplyIntensityModifiers(LeftIntensity, RightIntensity);

	// This would call platform-specific haptic APIs
}

void UMGHapticsSubsystem::InitializeDefaultPatterns()
{
	// Engine Idle
	FMGHapticPattern EngineIdle;
	EngineIdle.PatternID = FName("EngineIdle");
	EngineIdle.HapticType = EMGHapticType::EngineIdle;
	EngineIdle.Amplitudes.Add(0.1f);
	EngineIdle.Frequencies.Add(40.0f);
	EngineIdle.Durations.Add(1.0f);
	EngineIdle.bLooping = true;
	RegisteredPatterns.Add(EngineIdle.PatternID, EngineIdle);

	// Collision
	FMGHapticPattern Collision;
	Collision.PatternID = FName("Collision");
	Collision.HapticType = EMGHapticType::Collision;
	Collision.Amplitudes.Add(1.0f);
	Collision.Amplitudes.Add(0.5f);
	Collision.Amplitudes.Add(0.2f);
	Collision.Frequencies.Add(150.0f);
	Collision.Frequencies.Add(100.0f);
	Collision.Frequencies.Add(50.0f);
	Collision.Durations.Add(0.1f);
	Collision.Durations.Add(0.15f);
	Collision.Durations.Add(0.1f);
	RegisteredPatterns.Add(Collision.PatternID, Collision);

	// Gear Shift
	FMGHapticPattern GearShift;
	GearShift.PatternID = FName("GearShift");
	GearShift.HapticType = EMGHapticType::GearShift;
	GearShift.Amplitudes.Add(0.6f);
	GearShift.Amplitudes.Add(0.2f);
	GearShift.Frequencies.Add(100.0f);
	GearShift.Frequencies.Add(50.0f);
	GearShift.Durations.Add(0.05f);
	GearShift.Durations.Add(0.05f);
	RegisteredPatterns.Add(GearShift.PatternID, GearShift);

	// Boost Activate
	FMGHapticPattern BoostActivate;
	BoostActivate.PatternID = FName("BoostActivate");
	BoostActivate.HapticType = EMGHapticType::NitroActivate;
	BoostActivate.Amplitudes.Add(0.8f);
	BoostActivate.Amplitudes.Add(1.0f);
	BoostActivate.Amplitudes.Add(0.6f);
	BoostActivate.Frequencies.Add(100.0f);
	BoostActivate.Frequencies.Add(150.0f);
	BoostActivate.Frequencies.Add(80.0f);
	BoostActivate.Durations.Add(0.1f);
	BoostActivate.Durations.Add(0.15f);
	BoostActivate.Durations.Add(0.1f);
	RegisteredPatterns.Add(BoostActivate.PatternID, BoostActivate);

	// Checkpoint Pass
	FMGHapticPattern Checkpoint;
	Checkpoint.PatternID = FName("Checkpoint");
	Checkpoint.HapticType = EMGHapticType::CheckpointPass;
	Checkpoint.Amplitudes.Add(0.4f);
	Checkpoint.Amplitudes.Add(0.6f);
	Checkpoint.Frequencies.Add(80.0f);
	Checkpoint.Frequencies.Add(120.0f);
	Checkpoint.Durations.Add(0.1f);
	Checkpoint.Durations.Add(0.1f);
	RegisteredPatterns.Add(Checkpoint.PatternID, Checkpoint);

	// Countdown Go
	FMGHapticPattern CountdownGo;
	CountdownGo.PatternID = FName("CountdownGo");
	CountdownGo.HapticType = EMGHapticType::CountdownGo;
	CountdownGo.Amplitudes.Add(0.8f);
	CountdownGo.Amplitudes.Add(1.0f);
	CountdownGo.Amplitudes.Add(0.5f);
	CountdownGo.Frequencies.Add(100.0f);
	CountdownGo.Frequencies.Add(150.0f);
	CountdownGo.Frequencies.Add(100.0f);
	CountdownGo.Durations.Add(0.1f);
	CountdownGo.Durations.Add(0.2f);
	CountdownGo.Durations.Add(0.1f);
	RegisteredPatterns.Add(CountdownGo.PatternID, CountdownGo);
}

FMGHapticPattern UMGHapticsSubsystem::GetDefaultPattern(EMGHapticType Type) const
{
	FName PatternID;
	switch (Type)
	{
	case EMGHapticType::EngineIdle:
		PatternID = FName("EngineIdle");
		break;
	case EMGHapticType::Collision:
		PatternID = FName("Collision");
		break;
	case EMGHapticType::GearShift:
		PatternID = FName("GearShift");
		break;
	case EMGHapticType::NitroActivate:
		PatternID = FName("BoostActivate");
		break;
	case EMGHapticType::CheckpointPass:
		PatternID = FName("Checkpoint");
		break;
	case EMGHapticType::CountdownGo:
		PatternID = FName("CountdownGo");
		break;
	default:
		break;
	}

	const FMGHapticPattern* Pattern = RegisteredPatterns.Find(PatternID);
	return Pattern ? *Pattern : FMGHapticPattern();
}

void UMGHapticsSubsystem::ApplyIntensityModifiers(float& LeftIntensity, float& RightIntensity)
{
	// Apply global intensity
	LeftIntensity *= Config.GlobalIntensity;
	RightIntensity *= Config.GlobalIntensity;

	// Reduce on low battery if enabled
	if (Config.bReduceOnLowBattery && ControllerBatteryLevel < Config.LowBatteryThreshold)
	{
		float BatteryMultiplier = ControllerBatteryLevel / Config.LowBatteryThreshold;
		LeftIntensity *= BatteryMultiplier;
		RightIntensity *= BatteryMultiplier;
	}

	// Clamp final values
	LeftIntensity = FMath::Clamp(LeftIntensity, 0.0f, 1.0f);
	RightIntensity = FMath::Clamp(RightIntensity, 0.0f, 1.0f);
}

void UMGHapticsSubsystem::UpdateContinuousFeedback()
{
	// Update trigger feedback based on current state
	if (Config.bAdaptiveTriggers)
	{
		FMGTriggerFeedback Feedback;

		// Brake trigger - resistance based on speed
		if (Config.bBrakeTriggerFeedback)
		{
			float BrakeResistance = FMath::Clamp(CurrentSpeed / 300.0f, 0.1f, 0.8f);
			Feedback.LeftEffect = EMGTriggerEffect::Resistance;
			Feedback.LeftResistance = BrakeResistance;
			Feedback.LeftStartPosition = 0.2f;
		}

		// Throttle trigger - vibration based on RPM
		if (Config.bThrottleTriggerFeedback && CurrentEngineRPMPercent > 0.8f)
		{
			Feedback.RightEffect = EMGTriggerEffect::Vibration;
			Feedback.RightFrequency = 20.0f + (CurrentEngineRPMPercent * 40.0f);
			Feedback.RightStrength = (CurrentEngineRPMPercent - 0.8f) * 5.0f;
		}

		SetTriggerFeedback(Feedback);
	}
}

int32 UMGHapticsSubsystem::GetNextPrioritySlot(int32 Priority)
{
	// Find lowest priority haptic that is lower than requested
	int32 LowestPriorityIndex = INDEX_NONE;
	int32 LowestPriority = Priority;

	for (int32 i = 0; i < ActiveHaptics.Num(); i++)
	{
		if (ActiveHaptics[i].Priority < LowestPriority)
		{
			LowestPriority = ActiveHaptics[i].Priority;
			LowestPriorityIndex = i;
		}
	}

	return LowestPriorityIndex;
}
