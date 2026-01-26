// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGCameraVFXComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UMGCameraVFXComponent::UMGCameraVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UMGCameraVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheCameraReferences();
}

void UMGCameraVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Ensure slow motion is reset
	if (bInSlowMotion)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}

	Super::EndPlay(EndPlayReason);
}

void UMGCameraVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update all camera effects
	UpdateFOV(DeltaTime);
	UpdateFlash(DeltaTime);
	UpdateJudder(DeltaTime);
	UpdateSlowMotion(DeltaTime);
	UpdateContinuousShake(DeltaTime);
	UpdateCustomShake(DeltaTime);

	// Apply post-process
	ApplyPostProcess();
}

// ==========================================
// CAMERA SHAKE
// ==========================================

void UMGCameraVFXComponent::TriggerShake(EMGCameraShakeType ShakeType, float Scale)
{
	float Intensity, Duration, Frequency;
	GetShakeParameters(ShakeType, Intensity, Duration, Frequency);

	Intensity *= Scale * ShakeIntensityMultiplier;

	TriggerCustomShake(Intensity, Duration, Frequency);
}

void UMGCameraVFXComponent::TriggerCustomShake(float Intensity, float Duration, float Frequency)
{
	CustomShakeIntensity = Intensity;
	CustomShakeDuration = Duration;
	CustomShakeTimer = 0.0f;
	CustomShakeFrequency = Frequency;
}

void UMGCameraVFXComponent::StartContinuousShake(EMGCameraShakeType ShakeType, float Scale)
{
	bContinuousShakeActive = true;
	ContinuousShakeType = ShakeType;
	ContinuousShakeScale = Scale;
}

void UMGCameraVFXComponent::StopContinuousShake()
{
	bContinuousShakeActive = false;
}

void UMGCameraVFXComponent::SetShakeIntensityMultiplier(float Multiplier)
{
	ShakeIntensityMultiplier = FMath::Max(Multiplier, 0.0f);
}

// ==========================================
// SPEED EFFECTS
// ==========================================

void UMGCameraVFXComponent::UpdateSpeedEffects(float SpeedKPH)
{
	if (!bSpeedEffectsEnabled)
	{
		TargetSpeedIntensity = 0.0f;
		return;
	}

	// Calculate intensity based on speed
	if (SpeedKPH <= SpeedEffectConfig.StartThreshold)
	{
		TargetSpeedIntensity = 0.0f;
	}
	else if (SpeedKPH >= SpeedEffectConfig.MaxThreshold)
	{
		TargetSpeedIntensity = 1.0f;
	}
	else
	{
		float Range = SpeedEffectConfig.MaxThreshold - SpeedEffectConfig.StartThreshold;
		TargetSpeedIntensity = (SpeedKPH - SpeedEffectConfig.StartThreshold) / Range;
	}

	// Smooth interpolation
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	CurrentSpeedIntensity = FMath::FInterpTo(CurrentSpeedIntensity, TargetSpeedIntensity, DeltaTime, 5.0f);

	// Apply FOV increase
	SpeedFOVOffset = CurrentSpeedIntensity * SpeedEffectConfig.MaxFOVIncrease;

	// Start speed vibration at high speeds
	if (CurrentSpeedIntensity > 0.8f && !bContinuousShakeActive)
	{
		StartContinuousShake(EMGCameraShakeType::SpeedVibration, CurrentSpeedIntensity * 0.3f);
	}
	else if (CurrentSpeedIntensity <= 0.7f && bContinuousShakeActive && ContinuousShakeType == EMGCameraShakeType::SpeedVibration)
	{
		StopContinuousShake();
	}
}

void UMGCameraVFXComponent::SetSpeedEffectConfig(const FMGSpeedEffectConfig& Config)
{
	SpeedEffectConfig = Config;
}

void UMGCameraVFXComponent::SetSpeedEffectsEnabled(bool bEnabled)
{
	bSpeedEffectsEnabled = bEnabled;

	if (!bEnabled)
	{
		CurrentSpeedIntensity = 0.0f;
		TargetSpeedIntensity = 0.0f;
		SpeedFOVOffset = 0.0f;
	}
}

// ==========================================
// DRIFT EFFECTS
// ==========================================

void UMGCameraVFXComponent::UpdateDriftEffects(float DriftAngle, float DriftIntensity)
{
	if (!bDriftEffectsEnabled)
	{
		TargetDriftRoll = 0.0f;
		TargetDriftOffset = FVector::ZeroVector;
		return;
	}

	// Calculate roll based on drift angle
	// Positive drift angle = turning right = camera rolls left
	float NormalizedAngle = FMath::Clamp(DriftAngle / 45.0f, -1.0f, 1.0f);
	TargetDriftRoll = -NormalizedAngle * DriftCameraConfig.MaxRoll * DriftIntensity;

	// Calculate offset (camera shifts in direction of drift)
	TargetDriftOffset = DriftCameraConfig.DriftOffset * NormalizedAngle * DriftIntensity;

	// Smooth interpolation
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	CurrentDriftRoll = FMath::FInterpTo(CurrentDriftRoll, TargetDriftRoll, DeltaTime, DriftCameraConfig.RollInterpSpeed);
	CurrentDriftOffset = FMath::VInterpTo(CurrentDriftOffset, TargetDriftOffset, DeltaTime, DriftCameraConfig.OffsetInterpSpeed);

	// Apply to camera
	if (CachedCameraComponent)
	{
		// Note: Actual application would depend on camera rig setup
		// This provides the calculated values that a camera system can use
	}
}

void UMGCameraVFXComponent::SetDriftCameraConfig(const FMGDriftCameraConfig& Config)
{
	DriftCameraConfig = Config;
}

void UMGCameraVFXComponent::SetDriftEffectsEnabled(bool bEnabled)
{
	bDriftEffectsEnabled = bEnabled;

	if (!bEnabled)
	{
		CurrentDriftRoll = 0.0f;
		TargetDriftRoll = 0.0f;
		CurrentDriftOffset = FVector::ZeroVector;
		TargetDriftOffset = FVector::ZeroVector;
	}
}

// ==========================================
// IMPACT EFFECTS
// ==========================================

void UMGCameraVFXComponent::TriggerImpactFlash(const FMGImpactFlashConfig& Config)
{
	CurrentFlashColor = Config.Color;
	FlashDuration = Config.Duration;
	FlashTimer = 0.0f;
	FlashIntensity = Config.Intensity;
	bFlashChromaticAberration = Config.bAddChromaticAberration;

	// Also trigger shake
	TriggerShake(EMGCameraShakeType::Medium, Config.Intensity);
}

void UMGCameraVFXComponent::TriggerImpactFlashPreset(float ImpactForce)
{
	FMGImpactFlashConfig Config;

	// Scale based on impact force
	float NormalizedForce = FMath::Clamp(ImpactForce / 20000.0f, 0.1f, 1.0f);

	Config.Color = FLinearColor::White;
	Config.Duration = FMath::Lerp(0.05f, 0.2f, NormalizedForce);
	Config.Intensity = FMath::Lerp(0.2f, 0.8f, NormalizedForce);
	Config.bAddChromaticAberration = NormalizedForce > 0.5f;

	TriggerImpactFlash(Config);
}

void UMGCameraVFXComponent::TriggerJudder(float Intensity, float Duration)
{
	bJudderActive = true;
	JudderIntensity = Intensity;
	JudderDuration = Duration;
	JudderTimer = 0.0f;
}

// ==========================================
// POST PROCESS
// ==========================================

void UMGCameraVFXComponent::SetChromaticAberration(float Intensity)
{
	OverrideChromaticAberration = Intensity;
}

void UMGCameraVFXComponent::SetVignette(float Intensity)
{
	OverrideVignette = Intensity;
}

void UMGCameraVFXComponent::SetSaturation(float Saturation)
{
	OverrideSaturation = Saturation;
}

void UMGCameraVFXComponent::SetColorTint(FLinearColor Tint)
{
	OverrideColorTint = Tint;
}

void UMGCameraVFXComponent::ResetPostProcessOverrides()
{
	OverrideChromaticAberration = -1.0f;
	OverrideVignette = -1.0f;
	OverrideSaturation = -1.0f;
	OverrideColorTint = FLinearColor(-1.0f, -1.0f, -1.0f, -1.0f);
}

// ==========================================
// SLOW MOTION
// ==========================================

void UMGCameraVFXComponent::StartSlowMotion(float TimeDilation, float TransitionTime)
{
	bInSlowMotion = true;
	TargetTimeDilation = FMath::Clamp(TimeDilation, 0.01f, 1.0f);
	SlowMotionTransitionTime = TransitionTime;

	if (TransitionTime <= 0.0f)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TargetTimeDilation);
	}
}

void UMGCameraVFXComponent::EndSlowMotion(float TransitionTime)
{
	bInSlowMotion = false;
	TargetTimeDilation = 1.0f;
	SlowMotionTransitionTime = TransitionTime;

	if (TransitionTime <= 0.0f)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}
}

// ==========================================
// FOV
// ==========================================

void UMGCameraVFXComponent::SetBaseFOV(float FOV)
{
	BaseFOV = FMath::Clamp(FOV, 60.0f, 120.0f);
}

float UMGCameraVFXComponent::GetCurrentFOV() const
{
	return BaseFOV + SpeedFOVOffset + TempFOVOffset;
}

void UMGCameraVFXComponent::AddFOVOffset(float Offset, float Duration)
{
	TempFOVOffset = Offset;
	TempFOVOffsetDuration = Duration;
	TempFOVOffsetTimer = 0.0f;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGCameraVFXComponent::CacheCameraReferences()
{
	// Find camera component on owner
	AActor* Owner = GetOwner();
	if (Owner)
	{
		CachedCameraComponent = Owner->FindComponentByClass<UCameraComponent>();
	}

	// Find player camera manager
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			CachedCameraManager = PC->PlayerCameraManager;
		}
	}
}

void UMGCameraVFXComponent::UpdateFOV(float DeltaTime)
{
	// Handle temporary FOV offset decay
	if (TempFOVOffsetDuration > 0.0f)
	{
		TempFOVOffsetTimer += DeltaTime;
		if (TempFOVOffsetTimer >= TempFOVOffsetDuration)
		{
			TempFOVOffset = 0.0f;
			TempFOVOffsetDuration = 0.0f;
		}
		else
		{
			// Smooth decay
			float Progress = TempFOVOffsetTimer / TempFOVOffsetDuration;
			TempFOVOffset *= (1.0f - Progress * DeltaTime * 3.0f);
		}
	}

	// Apply FOV to camera
	float TargetFOV = GetCurrentFOV();

	if (CachedCameraComponent)
	{
		float CurrentFOV = CachedCameraComponent->FieldOfView;
		float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 8.0f);
		CachedCameraComponent->SetFieldOfView(NewFOV);
	}
}

void UMGCameraVFXComponent::UpdateFlash(float DeltaTime)
{
	if (FlashDuration <= 0.0f)
	{
		return;
	}

	FlashTimer += DeltaTime;
	if (FlashTimer >= FlashDuration)
	{
		FlashDuration = 0.0f;
		CurrentFlashColor = FLinearColor::Transparent;

		if (bFlashChromaticAberration)
		{
			// Reset chromatic aberration if it was added by flash
			if (OverrideChromaticAberration > 0.0f)
			{
				OverrideChromaticAberration = -1.0f;
			}
		}
	}
	else
	{
		float Progress = FlashTimer / FlashDuration;
		float CurrentFlashIntensity = FlashIntensity * (1.0f - Progress);

		// Apply flash as screen overlay (would need HUD integration)

		// Apply chromatic aberration during flash
		if (bFlashChromaticAberration)
		{
			OverrideChromaticAberration = CurrentFlashIntensity;
		}
	}
}

void UMGCameraVFXComponent::UpdateJudder(float DeltaTime)
{
	if (!bJudderActive)
	{
		return;
	}

	JudderTimer += DeltaTime;
	if (JudderTimer >= JudderDuration)
	{
		bJudderActive = false;
	}
	else
	{
		// Apply judder as camera offset/rotation
		float Progress = JudderTimer / JudderDuration;

		// Rapid random offset that decays
		FVector JudderOffset;
		JudderOffset.X = FMath::RandRange(-1.0f, 1.0f) * JudderIntensity * (1.0f - Progress) * 10.0f;
		JudderOffset.Y = FMath::RandRange(-1.0f, 1.0f) * JudderIntensity * (1.0f - Progress) * 10.0f;
		JudderOffset.Z = FMath::RandRange(-1.0f, 1.0f) * JudderIntensity * (1.0f - Progress) * 5.0f;

		FRotator JudderRotation;
		JudderRotation.Pitch = FMath::RandRange(-1.0f, 1.0f) * JudderIntensity * (1.0f - Progress) * 2.0f;
		JudderRotation.Yaw = FMath::RandRange(-1.0f, 1.0f) * JudderIntensity * (1.0f - Progress) * 2.0f;
		JudderRotation.Roll = FMath::RandRange(-1.0f, 1.0f) * JudderIntensity * (1.0f - Progress) * 1.0f;

		ApplyShakeToCamera(JudderOffset, JudderRotation);
	}
}

void UMGCameraVFXComponent::UpdateSlowMotion(float DeltaTime)
{
	if (SlowMotionTransitionTime <= 0.0f)
	{
		return;
	}

	float CurrentDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
	float NewDilation = FMath::FInterpTo(CurrentDilation, TargetTimeDilation, DeltaTime / CurrentDilation, 1.0f / SlowMotionTransitionTime);

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), NewDilation);

	if (FMath::IsNearlyEqual(NewDilation, TargetTimeDilation, 0.01f))
	{
		SlowMotionTransitionTime = 0.0f;
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TargetTimeDilation);
	}
}

void UMGCameraVFXComponent::UpdateContinuousShake(float DeltaTime)
{
	if (!bContinuousShakeActive)
	{
		return;
	}

	float Intensity, Duration, Frequency;
	GetShakeParameters(ContinuousShakeType, Intensity, Duration, Frequency);

	Intensity *= ContinuousShakeScale * ShakeIntensityMultiplier;

	// Generate continuous shake
	float Time = GetWorld()->GetTimeSeconds() * Frequency;

	FVector ShakeOff;
	ShakeOff.X = FMath::PerlinNoise1D(Time) * Intensity * 5.0f;
	ShakeOff.Y = FMath::PerlinNoise1D(Time + 100.0f) * Intensity * 5.0f;
	ShakeOff.Z = FMath::PerlinNoise1D(Time + 200.0f) * Intensity * 2.0f;

	FRotator ShakeRot;
	ShakeRot.Pitch = FMath::PerlinNoise1D(Time + 300.0f) * Intensity * 0.5f;
	ShakeRot.Yaw = FMath::PerlinNoise1D(Time + 400.0f) * Intensity * 0.5f;
	ShakeRot.Roll = FMath::PerlinNoise1D(Time + 500.0f) * Intensity * 0.3f;

	ApplyShakeToCamera(ShakeOff, ShakeRot);
}

void UMGCameraVFXComponent::UpdateCustomShake(float DeltaTime)
{
	if (CustomShakeDuration <= 0.0f)
	{
		return;
	}

	CustomShakeTimer += DeltaTime;
	if (CustomShakeTimer >= CustomShakeDuration)
	{
		CustomShakeDuration = 0.0f;
		ShakeOffset = FVector::ZeroVector;
	}
	else
	{
		float Progress = CustomShakeTimer / CustomShakeDuration;
		float CurrentIntensity = CustomShakeIntensity * (1.0f - Progress);

		float Time = GetWorld()->GetTimeSeconds() * CustomShakeFrequency;

		ShakeOffset.X = FMath::Sin(Time * 1.1f) * CurrentIntensity * 10.0f;
		ShakeOffset.Y = FMath::Sin(Time * 0.9f + 1.0f) * CurrentIntensity * 10.0f;
		ShakeOffset.Z = FMath::Sin(Time * 1.3f + 2.0f) * CurrentIntensity * 5.0f;

		FRotator ShakeRot;
		ShakeRot.Pitch = FMath::Sin(Time * 0.8f + 3.0f) * CurrentIntensity * 1.0f;
		ShakeRot.Yaw = FMath::Sin(Time * 1.2f + 4.0f) * CurrentIntensity * 1.0f;
		ShakeRot.Roll = FMath::Sin(Time * 0.7f + 5.0f) * CurrentIntensity * 0.5f;

		ApplyShakeToCamera(ShakeOffset, ShakeRot);
	}
}

void UMGCameraVFXComponent::ApplyShakeToCamera(const FVector& Offset, const FRotator& RotationOffset)
{
	if (CachedCameraManager)
	{
		// Use camera manager's built-in shake offset
		// Note: This is a simplified approach - full implementation would use camera shake classes
	}

	if (CachedCameraComponent)
	{
		// Direct camera manipulation
		// Note: Should be accumulated with other effects, not overwritten
	}
}

void UMGCameraVFXComponent::GetShakeParameters(EMGCameraShakeType Type, float& OutIntensity, float& OutDuration, float& OutFrequency) const
{
	switch (Type)
	{
	case EMGCameraShakeType::Light:
		OutIntensity = LightShakeIntensity;
		OutDuration = 0.2f;
		OutFrequency = 25.0f;
		break;

	case EMGCameraShakeType::Medium:
		OutIntensity = MediumShakeIntensity;
		OutDuration = 0.35f;
		OutFrequency = 20.0f;
		break;

	case EMGCameraShakeType::Heavy:
		OutIntensity = HeavyShakeIntensity;
		OutDuration = 0.5f;
		OutFrequency = 15.0f;
		break;

	case EMGCameraShakeType::Rumble:
		OutIntensity = 0.2f;
		OutDuration = 0.0f; // Continuous
		OutFrequency = 30.0f;
		break;

	case EMGCameraShakeType::NOS:
		OutIntensity = 0.4f;
		OutDuration = 0.3f;
		OutFrequency = 35.0f;
		break;

	case EMGCameraShakeType::NearMiss:
		OutIntensity = 0.25f;
		OutDuration = 0.15f;
		OutFrequency = 40.0f;
		break;

	case EMGCameraShakeType::Victory:
		OutIntensity = 0.3f;
		OutDuration = 1.0f;
		OutFrequency = 10.0f;
		break;

	case EMGCameraShakeType::SpeedVibration:
		OutIntensity = 0.15f;
		OutDuration = 0.0f; // Continuous
		OutFrequency = 50.0f;
		break;

	default:
		OutIntensity = 0.0f;
		OutDuration = 0.0f;
		OutFrequency = 20.0f;
		break;
	}
}

void UMGCameraVFXComponent::ApplyPostProcess()
{
	// Calculate combined post-process values
	float FinalChromaticAberration = 0.0f;
	float FinalVignette = 0.0f;
	float FinalMotionBlur = 0.0f;

	// Speed-based effects
	if (bSpeedEffectsEnabled && CurrentSpeedIntensity > 0.0f)
	{
		FinalChromaticAberration += CurrentSpeedIntensity * SpeedEffectConfig.MaxChromaticAberration;
		FinalVignette += CurrentSpeedIntensity * SpeedEffectConfig.MaxVignette;
		FinalMotionBlur += CurrentSpeedIntensity * SpeedEffectConfig.MaxMotionBlur;
	}

	// Override values
	if (OverrideChromaticAberration >= 0.0f)
	{
		FinalChromaticAberration = FMath::Max(FinalChromaticAberration, OverrideChromaticAberration);
	}

	if (OverrideVignette >= 0.0f)
	{
		FinalVignette = FMath::Max(FinalVignette, OverrideVignette);
	}

	// Apply to post-process volume or camera
	// Note: Full implementation would modify actual post-process settings
	// This depends on how the project's post-processing is set up
}
