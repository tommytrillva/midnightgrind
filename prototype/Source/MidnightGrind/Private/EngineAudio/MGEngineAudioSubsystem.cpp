// Copyright Midnight Grind. All Rights Reserved.

#include "EngineAudio/MGEngineAudioSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGEngineAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultProfiles();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			EngineTickHandle,
			this,
			&UMGEngineAudioSubsystem::OnEngineTick,
			0.016f,
			true
		);
	}
}

void UMGEngineAudioSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EngineTickHandle);
	}
	Super::Deinitialize();
}

bool UMGEngineAudioSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// Vehicle Registration
void UMGEngineAudioSubsystem::RegisterVehicle(FName VehicleID, const FMGEngineAudioProfile& Profile, bool bIsPlayer)
{
	FMGVehicleAudioInstance Instance;
	Instance.VehicleID = VehicleID;
	Instance.Profile = Profile;
	Instance.bIsPlayerVehicle = bIsPlayer;
	Instance.State.State = EMGEngineState::Off;

	ActiveVehicles.Add(VehicleID, Instance);

	if (bIsPlayer)
	{
		PlayerVehicleID = VehicleID;
	}
}

void UMGEngineAudioSubsystem::UnregisterVehicle(FName VehicleID)
{
	ActiveVehicles.Remove(VehicleID);

	if (PlayerVehicleID == VehicleID)
	{
		PlayerVehicleID = NAME_None;
	}
}

void UMGEngineAudioSubsystem::SetPlayerVehicle(FName VehicleID)
{
	if (FMGVehicleAudioInstance* OldPlayer = ActiveVehicles.Find(PlayerVehicleID))
	{
		OldPlayer->bIsPlayerVehicle = false;
	}

	PlayerVehicleID = VehicleID;

	if (FMGVehicleAudioInstance* NewPlayer = ActiveVehicles.Find(VehicleID))
	{
		NewPlayer->bIsPlayerVehicle = true;
	}
}

// Engine State Updates
void UMGEngineAudioSubsystem::UpdateEngineState(FName VehicleID, const FMGEngineAudioState& State)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (!Instance)
	{
		return;
	}

	EMGEngineState OldState = Instance->State.State;
	Instance->State = State;

	if (OldState != State.State)
	{
		OnEngineStateChanged.Broadcast(VehicleID, State.State);
	}
}

void UMGEngineAudioSubsystem::SetRPM(FName VehicleID, float RPM)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance)
	{
		Instance->State.TargetRPM = FMath::Max(0.0f, RPM);
	}
}

void UMGEngineAudioSubsystem::SetThrottle(FName VehicleID, float ThrottleInput)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance)
	{
		Instance->State.ThrottleInput = FMath::Clamp(ThrottleInput, 0.0f, 1.0f);
	}
}

void UMGEngineAudioSubsystem::SetGear(FName VehicleID, int32 Gear)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance && Instance->State.CurrentGear != Gear)
	{
		int32 OldGear = Instance->State.CurrentGear;
		Instance->State.CurrentGear = Gear;
		OnGearChanged.Broadcast(VehicleID, Gear);

		if (Instance->State.State != EMGEngineState::Off)
		{
			TriggerGearShift(VehicleID, OldGear, Gear);
		}
	}
}

void UMGEngineAudioSubsystem::SetTurboBoost(FName VehicleID, float Boost)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance)
	{
		float OldBoost = Instance->State.TurboBoost;
		Instance->State.TurboBoost = FMath::Clamp(Boost, 0.0f, 2.0f);

		// Trigger blowoff on throttle lift with boost
		if (OldBoost > 0.5f && Boost < 0.2f && Instance->State.ThrottleInput < 0.1f)
		{
			TriggerTurboBlowoff(VehicleID);
		}
	}
}

void UMGEngineAudioSubsystem::SetVehicleLocation(FName VehicleID, FVector Location)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance)
	{
		Instance->Location = Location;
		Instance->DistanceToListener = FVector::Dist(Location, ListenerLocation);
	}
}

// Engine Actions
void UMGEngineAudioSubsystem::StartEngine(FName VehicleID)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (!Instance)
	{
		return;
	}

	if (Instance->State.State != EMGEngineState::Off)
	{
		return;
	}

	Instance->State.State = EMGEngineState::Starting;
	OnEngineStateChanged.Broadcast(VehicleID, EMGEngineState::Starting);

	// Would play startup sound
	// After startup completes, transition to idle
	if (UWorld* World = GetWorld())
	{
		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(
			TempHandle,
			[this, VehicleID]()
			{
				FMGVehicleAudioInstance* Inst = ActiveVehicles.Find(VehicleID);
				if (Inst && Inst->State.State == EMGEngineState::Starting)
				{
					Inst->State.State = EMGEngineState::Idle;
					Inst->State.CurrentRPM = Inst->Profile.IdleRPM;
					Inst->State.TargetRPM = Inst->Profile.IdleRPM;
					OnEngineStateChanged.Broadcast(VehicleID, EMGEngineState::Idle);
				}
			},
			1.5f,
			false
		);
	}
}

void UMGEngineAudioSubsystem::StopEngine(FName VehicleID)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (!Instance)
	{
		return;
	}

	Instance->State.State = EMGEngineState::Off;
	Instance->State.CurrentRPM = 0.0f;
	Instance->State.TargetRPM = 0.0f;

	OnEngineStateChanged.Broadcast(VehicleID, EMGEngineState::Off);
}

void UMGEngineAudioSubsystem::Rev(FName VehicleID, float Intensity)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (!Instance || Instance->State.State == EMGEngineState::Off)
	{
		return;
	}

	float TargetRPM = FMath::Lerp(
		Instance->Profile.IdleRPM,
		Instance->Profile.RedlineRPM * 0.9f,
		FMath::Clamp(Intensity, 0.0f, 1.0f)
	);

	Instance->State.TargetRPM = TargetRPM;
	Instance->State.State = EMGEngineState::Revving;
}

void UMGEngineAudioSubsystem::TriggerBackfire(FName VehicleID)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (!Instance)
	{
		return;
	}

	Instance->State.bIsBackfiring = true;
	OnBackfire.Broadcast(VehicleID);

	// Would play backfire sound

	if (UWorld* World = GetWorld())
	{
		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(
			TempHandle,
			[this, VehicleID]()
			{
				FMGVehicleAudioInstance* Inst = ActiveVehicles.Find(VehicleID);
				if (Inst)
				{
					Inst->State.bIsBackfiring = false;
				}
			},
			0.2f,
			false
		);
	}
}

void UMGEngineAudioSubsystem::TriggerTurboBlowoff(FName VehicleID)
{
	OnTurboBlowoff.Broadcast(VehicleID);
	// Would play blowoff sound
}

void UMGEngineAudioSubsystem::TriggerGearShift(FName VehicleID, int32 FromGear, int32 ToGear)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (!Instance)
	{
		return;
	}

	Instance->State.bIsShifting = true;

	// Simulate RPM drop on upshift
	if (ToGear > FromGear && ToGear > 0)
	{
		Instance->State.TargetRPM = FMath::Max(
			Instance->Profile.IdleRPM,
			Instance->State.CurrentRPM - 2000.0f
		);
	}

	// Would play gear change sound

	if (UWorld* World = GetWorld())
	{
		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(
			TempHandle,
			[this, VehicleID]()
			{
				FMGVehicleAudioInstance* Inst = ActiveVehicles.Find(VehicleID);
				if (Inst)
				{
					Inst->State.bIsShifting = false;
				}
			},
			0.15f,
			false
		);
	}
}

// Queries
FMGEngineAudioState UMGEngineAudioSubsystem::GetEngineState(FName VehicleID) const
{
	const FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	return Instance ? Instance->State : FMGEngineAudioState();
}

FMGEngineAudioProfile UMGEngineAudioSubsystem::GetEngineProfile(FName VehicleID) const
{
	const FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	return Instance ? Instance->Profile : FMGEngineAudioProfile();
}

TArray<FName> UMGEngineAudioSubsystem::GetActiveVehicles() const
{
	TArray<FName> Result;
	ActiveVehicles.GetKeys(Result);
	return Result;
}

float UMGEngineAudioSubsystem::GetCurrentRPM(FName VehicleID) const
{
	const FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	return Instance ? Instance->State.CurrentRPM : 0.0f;
}

bool UMGEngineAudioSubsystem::IsEngineRunning(FName VehicleID) const
{
	const FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	return Instance && Instance->State.State != EMGEngineState::Off;
}

// Profile Management
void UMGEngineAudioSubsystem::RegisterProfile(const FMGEngineAudioProfile& Profile)
{
	RegisteredProfiles.Add(Profile.ProfileID, Profile);
}

FMGEngineAudioProfile UMGEngineAudioSubsystem::GetProfile(FName ProfileID) const
{
	const FMGEngineAudioProfile* Profile = RegisteredProfiles.Find(ProfileID);
	return Profile ? *Profile : FMGEngineAudioProfile();
}

void UMGEngineAudioSubsystem::ApplyExhaustUpgrade(FName VehicleID, EMGExhaustType ExhaustType)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance)
	{
		Instance->Profile.ExhaustType = ExhaustType;

		// Adjust audio characteristics based on exhaust
		switch (ExhaustType)
		{
		case EMGExhaustType::Sport:
			Instance->Profile.ExhaustPop *= 1.3f;
			Instance->Profile.BassPunch *= 1.2f;
			break;
		case EMGExhaustType::Performance:
			Instance->Profile.ExhaustPop *= 1.5f;
			Instance->Profile.BassPunch *= 1.4f;
			Instance->Profile.BackfireChance *= 1.2f;
			break;
		case EMGExhaustType::Racing:
			Instance->Profile.ExhaustPop *= 2.0f;
			Instance->Profile.BassPunch *= 1.6f;
			Instance->Profile.BackfireChance *= 1.5f;
			break;
		case EMGExhaustType::Straight:
			Instance->Profile.ExhaustPop *= 2.5f;
			Instance->Profile.BassPunch *= 2.0f;
			Instance->Profile.BackfireChance *= 2.0f;
			break;
		default:
			break;
		}
	}
}

void UMGEngineAudioSubsystem::ApplyTurboUpgrade(FName VehicleID, bool bTurbo, float BoostPressure)
{
	FMGVehicleAudioInstance* Instance = ActiveVehicles.Find(VehicleID);
	if (Instance)
	{
		if (bTurbo)
		{
			Instance->Profile.EngineType = EMGEngineType::Turbocharged;
			Instance->Profile.TurboLag = FMath::Lerp(0.5f, 0.2f, BoostPressure);
		}
	}
}

// Listener
void UMGEngineAudioSubsystem::SetListenerLocation(FVector Location)
{
	ListenerLocation = Location;
}

void UMGEngineAudioSubsystem::SetInteriorMode(bool bInterior)
{
	bInteriorMode = bInterior;
}

// Settings
void UMGEngineAudioSubsystem::SetMaxAudibleVehicles(int32 MaxVehicles)
{
	MaxAudibleVehicles = FMath::Max(1, MaxVehicles);
}

void UMGEngineAudioSubsystem::SetEngineVolume(float Volume)
{
	EngineVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}

// Internal
void UMGEngineAudioSubsystem::OnEngineTick()
{
	float DeltaTime = 0.016f;

	CalculateAudibility();

	for (auto& Pair : ActiveVehicles)
	{
		FMGVehicleAudioInstance& Instance = Pair.Value;

		if (Instance.State.State != EMGEngineState::Off)
		{
			ProcessRPMInterpolation(Instance, DeltaTime);
			ProcessBackfireChance(Instance);
			UpdateVehicleAudio(Instance);
		}
	}
}

void UMGEngineAudioSubsystem::UpdateVehicleAudio(FMGVehicleAudioInstance& Instance)
{
	if (Instance.Audibility <= 0.0f && !Instance.bIsPlayerVehicle)
	{
		return;
	}

	float RPM = Instance.State.CurrentRPM;
	float Throttle = Instance.State.ThrottleInput;

	// Select appropriate layers based on throttle
	const TArray<FMGEngineSoundLayer>& Layers = Throttle > 0.1f ?
		Instance.Profile.OnThrottleLayers :
		Instance.Profile.OffThrottleLayers;

	for (const FMGEngineSoundLayer& Layer : Layers)
	{
		if (RPM >= Layer.MinRPM && RPM <= Layer.MaxRPM)
		{
			float Volume = CalculateLayerVolume(RPM, Layer);
			PlayEngineLayer(Instance, Layer, Volume * Instance.Audibility);
		}
	}

	// Check for rev limiter
	if (RPM >= Instance.Profile.RevLimiterRPM && !Instance.State.bIsRevLimited)
	{
		Instance.State.bIsRevLimited = true;
		OnRevLimiterHit.Broadcast(Instance.VehicleID);
	}
	else if (RPM < Instance.Profile.RedlineRPM)
	{
		Instance.State.bIsRevLimited = false;
	}
}

void UMGEngineAudioSubsystem::CalculateAudibility()
{
	TArray<FMGVehicleAudioInstance*> SortedVehicles;

	for (auto& Pair : ActiveVehicles)
	{
		Pair.Value.DistanceToListener = FVector::Dist(Pair.Value.Location, ListenerLocation);
		SortedVehicles.Add(&Pair.Value);
	}

	// Sort by distance
	SortedVehicles.Sort([](const FMGVehicleAudioInstance& A, const FMGVehicleAudioInstance& B)
	{
		if (A.bIsPlayerVehicle) return true;
		if (B.bIsPlayerVehicle) return false;
		return A.DistanceToListener < B.DistanceToListener;
	});

	// Set audibility
	for (int32 i = 0; i < SortedVehicles.Num(); i++)
	{
		FMGVehicleAudioInstance* Instance = SortedVehicles[i];

		if (Instance->bIsPlayerVehicle)
		{
			Instance->Audibility = 1.0f;
		}
		else if (i < MaxAudibleVehicles)
		{
			float DistanceFactor = 1.0f - FMath::Clamp(Instance->DistanceToListener / MaxAudibleDistance, 0.0f, 1.0f);
			Instance->Audibility = DistanceFactor;
		}
		else
		{
			Instance->Audibility = 0.0f;
		}
	}
}

void UMGEngineAudioSubsystem::ProcessRPMInterpolation(FMGVehicleAudioInstance& Instance, float DeltaTime)
{
	float& CurrentRPM = Instance.State.CurrentRPM;
	float TargetRPM = Instance.State.TargetRPM;

	// Apply rev limiter
	if (TargetRPM >= Instance.Profile.RevLimiterRPM)
	{
		TargetRPM = Instance.Profile.RedlineRPM - 200.0f;
	}

	// Ensure minimum idle
	if (Instance.State.State == EMGEngineState::Idle && TargetRPM < Instance.Profile.IdleRPM)
	{
		TargetRPM = Instance.Profile.IdleRPM;
	}

	// Interpolate RPM
	float InterpSpeed = RPMInterpolationSpeed;
	if (Instance.State.bIsShifting)
	{
		InterpSpeed *= 3.0f; // Faster RPM drop during shifts
	}

	CurrentRPM = FMath::FInterpTo(CurrentRPM, TargetRPM, DeltaTime, InterpSpeed);

	// Update engine state based on RPM and throttle
	if (Instance.State.State != EMGEngineState::Off && Instance.State.State != EMGEngineState::Starting)
	{
		if (Instance.State.ThrottleInput > 0.1f)
		{
			Instance.State.State = CurrentRPM > Instance.Profile.RedlineRPM * 0.9f ?
				EMGEngineState::Redline : EMGEngineState::OnThrottle;
		}
		else if (CurrentRPM <= Instance.Profile.IdleRPM * 1.1f)
		{
			Instance.State.State = EMGEngineState::Idle;
		}
		else
		{
			Instance.State.State = EMGEngineState::OffThrottle;
		}
	}
}

void UMGEngineAudioSubsystem::ProcessBackfireChance(FMGVehicleAudioInstance& Instance)
{
	// Backfire on throttle lift at high RPM
	if (Instance.State.State == EMGEngineState::OffThrottle &&
		Instance.State.CurrentRPM > Instance.Profile.RedlineRPM * 0.7f)
	{
		float Chance = Instance.Profile.BackfireChance * 0.01f; // Per tick
		if (FMath::FRand() < Chance)
		{
			TriggerBackfire(Instance.VehicleID);
		}
	}
}

void UMGEngineAudioSubsystem::PlayEngineLayer(FMGVehicleAudioInstance& Instance, const FMGEngineSoundLayer& Layer, float Volume)
{
	// Would play the sound layer at calculated pitch and volume
	float Pitch = CalculatePitchFromRPM(Instance.State.CurrentRPM, Layer);
	float FinalVolume = Volume * Layer.VolumeMultiplier * EngineVolume;

	if (bInteriorMode && Instance.bIsPlayerVehicle)
	{
		// Apply interior filtering
		FinalVolume *= 0.7f;
	}
}

float UMGEngineAudioSubsystem::CalculatePitchFromRPM(float RPM, const FMGEngineSoundLayer& Layer) const
{
	float NormalizedRPM = (RPM - Layer.MinRPM) / (Layer.MaxRPM - Layer.MinRPM);
	float Pitch = FMath::Lerp(Layer.MinPitch, Layer.MaxPitch, NormalizedRPM);
	return FMath::Clamp(Pitch * Layer.PitchMultiplier, 0.1f, 4.0f);
}

float UMGEngineAudioSubsystem::CalculateLayerVolume(float RPM, const FMGEngineSoundLayer& Layer) const
{
	float LayerCenter = (Layer.MinRPM + Layer.MaxRPM) * 0.5f;
	float LayerWidth = (Layer.MaxRPM - Layer.MinRPM) * 0.5f;

	// Calculate fade based on distance from layer boundaries
	float FadeIn = FMath::Clamp((RPM - Layer.MinRPM) / Layer.CrossfadeWidth, 0.0f, 1.0f);
	float FadeOut = FMath::Clamp((Layer.MaxRPM - RPM) / Layer.CrossfadeWidth, 0.0f, 1.0f);

	return FMath::Min(FadeIn, FadeOut);
}

void UMGEngineAudioSubsystem::InitializeDefaultProfiles()
{
	// V8 Muscle Car
	{
		FMGEngineAudioProfile Profile;
		Profile.ProfileID = FName("V8_Muscle");
		Profile.EngineType = EMGEngineType::V8;
		Profile.ExhaustType = EMGExhaustType::Stock;
		Profile.IdleRPM = 700.0f;
		Profile.RedlineRPM = 6500.0f;
		Profile.RevLimiterRPM = 6700.0f;
		Profile.BackfireChance = 0.2f;
		Profile.Rumble = 0.8f;
		Profile.BassPunch = 0.9f;
		RegisterProfile(Profile);
	}

	// Inline 4 Turbo
	{
		FMGEngineAudioProfile Profile;
		Profile.ProfileID = FName("I4_Turbo");
		Profile.EngineType = EMGEngineType::Turbocharged;
		Profile.ExhaustType = EMGExhaustType::Sport;
		Profile.IdleRPM = 900.0f;
		Profile.RedlineRPM = 8000.0f;
		Profile.RevLimiterRPM = 8200.0f;
		Profile.TurboLag = 0.3f;
		Profile.BackfireChance = 0.4f;
		Profile.ExhaustPop = 0.7f;
		RegisterProfile(Profile);
	}

	// V12 Exotic
	{
		FMGEngineAudioProfile Profile;
		Profile.ProfileID = FName("V12_Exotic");
		Profile.EngineType = EMGEngineType::V12;
		Profile.ExhaustType = EMGExhaustType::Performance;
		Profile.IdleRPM = 1000.0f;
		Profile.RedlineRPM = 9000.0f;
		Profile.RevLimiterRPM = 9200.0f;
		Profile.BackfireChance = 0.3f;
		Profile.Rumble = 0.4f;
		Profile.BassPunch = 0.5f;
		RegisterProfile(Profile);
	}

	// Electric
	{
		FMGEngineAudioProfile Profile;
		Profile.ProfileID = FName("Electric");
		Profile.EngineType = EMGEngineType::Electric;
		Profile.IdleRPM = 0.0f;
		Profile.RedlineRPM = 20000.0f;
		Profile.RevLimiterRPM = 21000.0f;
		Profile.BackfireChance = 0.0f;
		Profile.Rumble = 0.0f;
		Profile.BassPunch = 0.0f;
		RegisterProfile(Profile);
	}
}
