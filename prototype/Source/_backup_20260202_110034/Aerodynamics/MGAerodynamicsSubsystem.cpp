// Copyright Epic Games, Inc. All Rights Reserved.

#include "Aerodynamics/MGAerodynamicsSubsystem.h"
#include "TimerManager.h"

void UMGAerodynamicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SessionCounter = 0;

	// Set up default slipstream config
	SlipstreamConfig.MinDistance = 200.0f;
	SlipstreamConfig.MaxDistance = 2000.0f;
	SlipstreamConfig.OptimalDistance = 500.0f;
	SlipstreamConfig.ConeAngleDegrees = 15.0f;
	SlipstreamConfig.MaxDragReduction = 0.40f;
	SlipstreamConfig.MaxSpeedBonus = 0.10f;
	SlipstreamConfig.ChargeRate = 0.2f;
	SlipstreamConfig.DischargeRate = 0.5f;
	SlipstreamConfig.SlingshotBoost = 0.15f;
	SlipstreamConfig.SlingshotDuration = 2.0f;
	SlipstreamConfig.MinSpeedMPH = 80.0f;
	SlipstreamConfig.SlipstreamPoints = 50;

	// Set up default global config
	GlobalConfig.AirDensityBase = 1.225f;
	GlobalConfig.AltitudeEffect = 0.0001f;
	GlobalConfig.TemperatureEffect = 0.004f;
	GlobalConfig.HumidityEffect = 0.001f;
	GlobalConfig.DownforceGripMultiplierMax = 1.5f;
	GlobalConfig.DragTopSpeedPenaltyMax = 0.15f;
	GlobalConfig.bSimulateDetailedAero = true;
	GlobalConfig.bEnableWindEffects = true;
	GlobalConfig.bEnableSlipstream = true;

	// Register default aero profiles
	FMGAeroProfileDefinition StandardProfile;
	StandardProfile.ProfileId = TEXT("Standard");
	StandardProfile.DisplayName = FText::FromString(TEXT("Standard"));
	StandardProfile.Type = EMGAeroProfile::Standard;
	StandardProfile.DragCoefficient = 0.30f;
	StandardProfile.LiftCoefficient = 0.10f;
	StandardProfile.DownforceCoefficient = 0.50f;
	StandardProfile.FrontalArea = 2.0f;
	StandardProfile.DownforceFrontBias = 0.45f;
	StandardProfile.TopSpeedEffect = 1.0f;
	StandardProfile.CorneringGripEffect = 1.0f;
	StandardProfile.BrakingStabilityEffect = 1.0f;
	StandardProfile.SlipstreamEffectiveness = 1.0f;
	RegisterAeroProfile(StandardProfile);

	FMGAeroProfileDefinition LowDragProfile;
	LowDragProfile.ProfileId = TEXT("LowDrag");
	LowDragProfile.DisplayName = FText::FromString(TEXT("Low Drag"));
	LowDragProfile.Type = EMGAeroProfile::LowDrag;
	LowDragProfile.DragCoefficient = 0.22f;
	LowDragProfile.LiftCoefficient = 0.15f;
	LowDragProfile.DownforceCoefficient = 0.25f;
	LowDragProfile.FrontalArea = 1.8f;
	LowDragProfile.DownforceFrontBias = 0.50f;
	LowDragProfile.TopSpeedEffect = 1.10f;
	LowDragProfile.CorneringGripEffect = 0.90f;
	LowDragProfile.BrakingStabilityEffect = 0.95f;
	LowDragProfile.SlipstreamEffectiveness = 1.20f;
	RegisterAeroProfile(LowDragProfile);

	FMGAeroProfileDefinition HighDownforceProfile;
	HighDownforceProfile.ProfileId = TEXT("HighDownforce");
	HighDownforceProfile.DisplayName = FText::FromString(TEXT("High Downforce"));
	HighDownforceProfile.Type = EMGAeroProfile::HighDownforce;
	HighDownforceProfile.DragCoefficient = 0.45f;
	HighDownforceProfile.LiftCoefficient = -0.05f;
	HighDownforceProfile.DownforceCoefficient = 1.50f;
	HighDownforceProfile.FrontalArea = 2.2f;
	HighDownforceProfile.DownforceFrontBias = 0.40f;
	HighDownforceProfile.TopSpeedEffect = 0.90f;
	HighDownforceProfile.CorneringGripEffect = 1.30f;
	HighDownforceProfile.BrakingStabilityEffect = 1.20f;
	HighDownforceProfile.SlipstreamEffectiveness = 0.80f;
	RegisterAeroProfile(HighDownforceProfile);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGAerodynamicsSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			AeroTickTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->TickAerodynamics(0.033f);
				}
			},
			0.033f,
			true
		);
	}

	LoadAeroData();
}

void UMGAerodynamicsSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AeroTickTimer);
	}

	SaveAeroData();
	Super::Deinitialize();
}

// ============================================================================
// Vehicle Registration
// ============================================================================

void UMGAerodynamicsSubsystem::RegisterVehicle(const FString& VehicleId, const FString& ProfileId)
{
	if (VehicleId.IsEmpty())
	{
		return;
	}

	FMGVehicleAeroState State;
	State.VehicleId = VehicleId;
	State.AirDensity = GlobalConfig.AirDensityBase;

	const FMGAeroProfileDefinition* Profile = AeroProfiles.Find(ProfileId);
	if (Profile)
	{
		State.EffectiveDragCoefficient = Profile->DragCoefficient;
		State.EffectiveDownforceCoefficient = Profile->DownforceCoefficient;
	}

	VehicleStates.Add(VehicleId, State);
	VehicleProfiles.Add(VehicleId, ProfileId);
}

void UMGAerodynamicsSubsystem::UnregisterVehicle(const FString& VehicleId)
{
	VehicleStates.Remove(VehicleId);
	VehicleProfiles.Remove(VehicleId);
	ActiveSlipstreams.Remove(VehicleId);
	VehicleSpoilers.Remove(VehicleId);
}

FMGVehicleAeroState UMGAerodynamicsSubsystem::GetVehicleAeroState(const FString& VehicleId) const
{
	if (const FMGVehicleAeroState* Found = VehicleStates.Find(VehicleId))
	{
		return *Found;
	}
	return FMGVehicleAeroState();
}

void UMGAerodynamicsSubsystem::SetVehicleProfile(const FString& VehicleId, const FString& ProfileId)
{
	FString OldProfileId;
	if (const FString* Old = VehicleProfiles.Find(VehicleId))
	{
		OldProfileId = *Old;
	}

	VehicleProfiles.Add(VehicleId, ProfileId);

	// Update state
	FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = AeroProfiles.Find(ProfileId);

	if (State && Profile)
	{
		State->EffectiveDragCoefficient = Profile->DragCoefficient;
		State->EffectiveDownforceCoefficient = Profile->DownforceCoefficient;
	}

	OnAeroProfileChanged.Broadcast(VehicleId, OldProfileId, ProfileId);
}

// ============================================================================
// Aerodynamic Profiles
// ============================================================================

void UMGAerodynamicsSubsystem::RegisterAeroProfile(const FMGAeroProfileDefinition& Profile)
{
	if (Profile.ProfileId.IsEmpty())
	{
		return;
	}
	AeroProfiles.Add(Profile.ProfileId, Profile);
}

FMGAeroProfileDefinition UMGAerodynamicsSubsystem::GetAeroProfile(const FString& ProfileId) const
{
	if (const FMGAeroProfileDefinition* Found = AeroProfiles.Find(ProfileId))
	{
		return *Found;
	}
	return FMGAeroProfileDefinition();
}

TArray<FMGAeroProfileDefinition> UMGAerodynamicsSubsystem::GetAllProfiles() const
{
	TArray<FMGAeroProfileDefinition> Result;
	for (const auto& Pair : AeroProfiles)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

// ============================================================================
// Force Calculations
// ============================================================================

float UMGAerodynamicsSubsystem::CalculateDragForce(const FString& VehicleId, float SpeedMS) const
{
	const FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return 0.0f;
	}

	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	float Cd = State->EffectiveDragCoefficient;
	float A = Profile ? Profile->FrontalArea : 2.0f;
	float Rho = State->AirDensity;

	// Apply slipstream drag reduction
	float DragReduction = State->SlipstreamBonus;

	// Drag = 0.5 * Rho * V^2 * Cd * A
	float Drag = 0.5f * Rho * SpeedMS * SpeedMS * Cd * A;
	Drag *= (1.0f - DragReduction);

	return Drag;
}

float UMGAerodynamicsSubsystem::CalculateLiftForce(const FString& VehicleId, float SpeedMS) const
{
	const FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return 0.0f;
	}

	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	float Cl = Profile ? Profile->LiftCoefficient : 0.1f;
	float A = Profile ? Profile->FrontalArea : 2.0f;
	float Rho = State->AirDensity;

	// Lift = 0.5 * Rho * V^2 * Cl * A
	float Lift = 0.5f * Rho * SpeedMS * SpeedMS * Cl * A;

	return Lift;
}

float UMGAerodynamicsSubsystem::CalculateDownforce(const FString& VehicleId, float SpeedMS) const
{
	const FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return 0.0f;
	}

	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	float Cdf = State->EffectiveDownforceCoefficient;
	float A = Profile ? Profile->FrontalArea : 2.0f;
	float Rho = State->AirDensity;

	// Apply spoiler effect
	const FMGSpoilerConfig* Spoiler = VehicleSpoilers.Find(VehicleId);
	if (Spoiler && Spoiler->bIsActive)
	{
		float SpeedMPH = SpeedMS / 0.44704f;
		if (SpeedMPH >= Spoiler->MinSpeedForEffect)
		{
			Cdf *= Spoiler->DownforceMultiplier;
		}
	}

	// Downforce = 0.5 * Rho * V^2 * Cdf * A
	float Downforce = 0.5f * Rho * SpeedMS * SpeedMS * Cdf * A;

	return Downforce;
}

FVector UMGAerodynamicsSubsystem::CalculateTotalAeroForce(const FString& VehicleId, FVector Velocity) const
{
	float SpeedMS = Velocity.Size();
	FVector Direction = Velocity.GetSafeNormal();

	float Drag = CalculateDragForce(VehicleId, SpeedMS);
	float Lift = CalculateLiftForce(VehicleId, SpeedMS);
	float Downforce = CalculateDownforce(VehicleId, SpeedMS);

	FVector DragForce = -Direction * Drag;
	FVector LiftForce = FVector::UpVector * (Lift - Downforce);

	// Add wind effects
	FVector WindForce = CalculateWindForce(VehicleId, Velocity);

	return DragForce + LiftForce + WindForce;
}

// ============================================================================
// Update Vehicle State
// ============================================================================

void UMGAerodynamicsSubsystem::UpdateVehicleAero(const FString& VehicleId, FVector Position, FVector Velocity, float DeltaTime)
{
	FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return;
	}

	float SpeedMS = Velocity.Size();
	float SpeedMPH = SpeedMS / 0.44704f;

	State->CurrentSpeed = SpeedMPH;
	State->AirDensity = GetAirDensityAtLocation(Position);

	// Calculate forces
	State->DragForce = CalculateDragForce(VehicleId, SpeedMS);
	State->LiftForce = CalculateLiftForce(VehicleId, SpeedMS);
	State->DownforceTotal = CalculateDownforce(VehicleId, SpeedMS);

	// Calculate front/rear downforce split
	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	if (Profile)
	{
		State->DownforceFront = State->DownforceTotal * Profile->DownforceFrontBias;
		State->DownforceRear = State->DownforceTotal * (1.0f - Profile->DownforceFrontBias);
	}

	// Calculate grip multiplier from downforce
	float DownforceKg = State->DownforceTotal / 9.81f;
	float GripBonus = FMath::Clamp(DownforceKg / 500.0f, 0.0f, GlobalConfig.DownforceGripMultiplierMax - 1.0f);
	State->GripMultiplier = 1.0f + GripBonus;

	// Calculate top speed multiplier from drag
	float DragPenalty = FMath::Clamp(State->DragForce / 5000.0f, 0.0f, GlobalConfig.DragTopSpeedPenaltyMax);
	State->TopSpeedMultiplier = 1.0f - DragPenalty;

	// Apply slipstream bonus if active
	if (State->SlipstreamState != EMGSlipstreamState::None)
	{
		State->TopSpeedMultiplier += State->SlipstreamBonus;
	}

	// Update wind effects
	if (GlobalConfig.bEnableWindEffects)
	{
		State->WindForce = GetWindAtLocation(Position);

		// Determine wind effect type
		if (State->WindForce.Size() > 1.0f)
		{
			float WindDot = FVector::DotProduct(State->WindForce.GetSafeNormal(), Velocity.GetSafeNormal());

			if (WindDot > 0.7f)
			{
				State->CurrentWindEffect = EMGWindEffect::Tailwind;
			}
			else if (WindDot < -0.7f)
			{
				State->CurrentWindEffect = EMGWindEffect::Headwind;
			}
			else
			{
				State->CurrentWindEffect = EMGWindEffect::Crosswind;
			}
		}
		else
		{
			State->CurrentWindEffect = EMGWindEffect::None;
		}
	}

	// Broadcast downforce changes if significant
	static TMap<FString, float> LastDownforce;
	float* LastDF = LastDownforce.Find(VehicleId);
	if (!LastDF || FMath::Abs(*LastDF - State->DownforceTotal) > 100.0f)
	{
		float OldDF = LastDF ? *LastDF : 0.0f;
		LastDownforce.Add(VehicleId, State->DownforceTotal);
		OnDownforceChanged.Broadcast(VehicleId, OldDF, State->DownforceTotal);
	}
}

float UMGAerodynamicsSubsystem::GetEffectiveTopSpeed(const FString& VehicleId, float BaseTopSpeed) const
{
	const FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return BaseTopSpeed;
	}

	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	float Multiplier = State->TopSpeedMultiplier;
	if (Profile)
	{
		Multiplier *= Profile->TopSpeedEffect;
	}

	return BaseTopSpeed * Multiplier;
}

float UMGAerodynamicsSubsystem::GetEffectiveGrip(const FString& VehicleId, float BaseGrip) const
{
	const FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return BaseGrip;
	}

	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	float Multiplier = State->GripMultiplier;
	if (Profile)
	{
		Multiplier *= Profile->CorneringGripEffect;
	}

	return BaseGrip * Multiplier;
}

// ============================================================================
// Slipstream
// ============================================================================

void UMGAerodynamicsSubsystem::CheckSlipstream(const FString& FollowerId, const FString& LeaderId, FVector FollowerPos, FVector LeaderPos, FVector FollowerVelocity, FVector LeaderVelocity)
{
	if (!GlobalConfig.bEnableSlipstream)
	{
		return;
	}

	float FollowerSpeedMPH = FollowerVelocity.Size() / 0.44704f;

	// Check minimum speed
	if (FollowerSpeedMPH < SlipstreamConfig.MinSpeedMPH)
	{
		if (IsInSlipstream(FollowerId))
		{
			ExitSlipstream(FollowerId);
		}
		return;
	}

	float Distance = FVector::Dist(FollowerPos, LeaderPos);
	FVector LeaderForward = LeaderVelocity.GetSafeNormal();

	bool bInCone = IsInSlipstreamCone(FollowerPos, LeaderPos, LeaderForward, SlipstreamConfig.ConeAngleDegrees, SlipstreamConfig.MaxDistance);

	if (bInCone && Distance >= SlipstreamConfig.MinDistance && Distance <= SlipstreamConfig.MaxDistance)
	{
		// Enter or update slipstream
		FMGSlipstreamSession* Session = ActiveSlipstreams.Find(FollowerId);

		if (!Session)
		{
			// New session
			FMGSlipstreamSession NewSession;
			NewSession.SessionId = GenerateSessionId();
			NewSession.FollowerVehicleId = FollowerId;
			NewSession.LeaderVehicleId = LeaderId;
			NewSession.State = EMGSlipstreamState::Entering;
			NewSession.Distance = Distance;
			NewSession.StartTime = FDateTime::Now();

			ActiveSlipstreams.Add(FollowerId, NewSession);
			Session = ActiveSlipstreams.Find(FollowerId);

			OnSlipstreamEntered.Broadcast(FollowerId, LeaderId, Distance);
		}

		// Update session
		UpdateSlipstreamState(*Session, Distance, 0.033f);

		// Update vehicle state
		FMGVehicleAeroState* State = VehicleStates.Find(FollowerId);
		if (State)
		{
			State->SlipstreamState = Session->State;
			State->SlipstreamBonus = Session->SpeedBonus;
			State->SlipstreamCharge = Session->ChargeLevel;
		}
	}
	else
	{
		// Exit slipstream if we were in it
		if (IsInSlipstream(FollowerId))
		{
			ExitSlipstream(FollowerId);
		}
	}
}

bool UMGAerodynamicsSubsystem::IsInSlipstream(const FString& VehicleId) const
{
	const FMGSlipstreamSession* Session = ActiveSlipstreams.Find(VehicleId);
	return Session && Session->State != EMGSlipstreamState::None;
}

FMGSlipstreamSession UMGAerodynamicsSubsystem::GetSlipstreamSession(const FString& VehicleId) const
{
	if (const FMGSlipstreamSession* Found = ActiveSlipstreams.Find(VehicleId))
	{
		return *Found;
	}
	return FMGSlipstreamSession();
}

float UMGAerodynamicsSubsystem::GetSlipstreamCharge(const FString& VehicleId) const
{
	if (const FMGSlipstreamSession* Session = ActiveSlipstreams.Find(VehicleId))
	{
		return Session->ChargeLevel;
	}
	return 0.0f;
}

bool UMGAerodynamicsSubsystem::IsSlingshotReady(const FString& VehicleId) const
{
	if (const FMGSlipstreamSession* Session = ActiveSlipstreams.Find(VehicleId))
	{
		return Session->SlingshotReady >= 1.0f;
	}
	return false;
}

float UMGAerodynamicsSubsystem::ActivateSlingshot(const FString& VehicleId)
{
	FMGSlipstreamSession* Session = ActiveSlipstreams.Find(VehicleId);
	if (!Session || Session->SlingshotReady < 1.0f)
	{
		return 0.0f;
	}

	float Boost = SlipstreamConfig.SlingshotBoost;

	Session->State = EMGSlipstreamState::Slingshot;
	Session->SlingshotReady = 0.0f;
	Session->ChargeLevel = 0.0f;

	// Award points
	AwardSlipstreamPoints(VehicleId, Session->Duration);

	// Update stats
	UpdatePlayerStats(VehicleId, *Session);
	FMGAeroPlayerStats& Stats = PlayerStats.FindOrAdd(VehicleId);
	Stats.SlingshotsUsed++;

	OnSlingshotUsed.Broadcast(VehicleId, Boost * 100.0f);

	// End slipstream after slingshot
	ExitSlipstream(VehicleId);

	return Boost;
}

void UMGAerodynamicsSubsystem::ExitSlipstream(const FString& VehicleId)
{
	FMGSlipstreamSession* Session = ActiveSlipstreams.Find(VehicleId);
	if (!Session)
	{
		return;
	}

	// Award points for time spent
	int32 Points = FMath::RoundToInt(Session->Duration * SlipstreamConfig.SlipstreamPoints);

	// Update stats
	UpdatePlayerStats(VehicleId, *Session);

	// Update vehicle state
	FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (State)
	{
		State->SlipstreamState = EMGSlipstreamState::None;
		State->SlipstreamBonus = 0.0f;
		State->SlipstreamCharge = 0.0f;
	}

	OnSlipstreamExited.Broadcast(VehicleId, Session->Duration, Points);

	ActiveSlipstreams.Remove(VehicleId);
}

// ============================================================================
// Wind Effects
// ============================================================================

void UMGAerodynamicsSubsystem::RegisterWindZone(const FMGWindZone& Zone)
{
	if (Zone.ZoneId.IsEmpty())
	{
		return;
	}
	WindZones.Add(Zone.ZoneId, Zone);
}

void UMGAerodynamicsSubsystem::UnregisterWindZone(const FString& ZoneId)
{
	WindZones.Remove(ZoneId);
}

FMGWindZone UMGAerodynamicsSubsystem::GetWindZone(const FString& ZoneId) const
{
	if (const FMGWindZone* Found = WindZones.Find(ZoneId))
	{
		return *Found;
	}
	return FMGWindZone();
}

TArray<FMGWindZone> UMGAerodynamicsSubsystem::GetAllWindZones() const
{
	TArray<FMGWindZone> Result;
	for (const auto& Pair : WindZones)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

void UMGAerodynamicsSubsystem::SetGlobalWind(FVector Direction, float Speed)
{
	GlobalWindDirection = Direction.GetSafeNormal();
	GlobalWindSpeed = Speed;
}

FVector UMGAerodynamicsSubsystem::GetWindAtLocation(FVector Location) const
{
	FVector TotalWind = GlobalWindDirection * GlobalWindSpeed;

	// Add wind zone effects
	for (const auto& Pair : WindZones)
	{
		const FMGWindZone& Zone = Pair.Value;

		float Distance = FVector::Dist(Location, Zone.Center);
		if (Distance <= Zone.Radius)
		{
			float Falloff = 1.0f - (Distance / Zone.Radius);
			TotalWind += Zone.WindDirection * Zone.WindSpeed * Falloff;
		}
	}

	return TotalWind;
}

FVector UMGAerodynamicsSubsystem::CalculateWindForce(const FString& VehicleId, FVector VehicleVelocity) const
{
	const FMGVehicleAeroState* State = VehicleStates.Find(VehicleId);
	if (!State)
	{
		return FVector::ZeroVector;
	}

	FVector WindVelocity = State->WindForce;
	FVector RelativeWind = WindVelocity - VehicleVelocity;

	float WindSpeed = RelativeWind.Size();
	if (WindSpeed < 0.1f)
	{
		return FVector::ZeroVector;
	}

	// Simplified wind force calculation
	const FString* ProfileId = VehicleProfiles.Find(VehicleId);
	const FMGAeroProfileDefinition* Profile = ProfileId ? AeroProfiles.Find(*ProfileId) : nullptr;

	float A = Profile ? Profile->FrontalArea : 2.0f;
	float Cd = State->EffectiveDragCoefficient;

	float Force = 0.5f * State->AirDensity * WindSpeed * WindSpeed * Cd * A * 0.1f;

	return RelativeWind.GetSafeNormal() * Force;
}

// ============================================================================
// Spoilers
// ============================================================================

void UMGAerodynamicsSubsystem::SetVehicleSpoiler(const FString& VehicleId, const FMGSpoilerConfig& Spoiler)
{
	VehicleSpoilers.Add(VehicleId, Spoiler);
}

FMGSpoilerConfig UMGAerodynamicsSubsystem::GetVehicleSpoiler(const FString& VehicleId) const
{
	if (const FMGSpoilerConfig* Found = VehicleSpoilers.Find(VehicleId))
	{
		return *Found;
	}
	return FMGSpoilerConfig();
}

void UMGAerodynamicsSubsystem::SetSpoilerAngle(const FString& VehicleId, float AngleDegrees)
{
	FMGSpoilerConfig* Spoiler = VehicleSpoilers.Find(VehicleId);
	if (Spoiler)
	{
		Spoiler->AngleDegrees = AngleDegrees;
	}
}

void UMGAerodynamicsSubsystem::SetSpoilerActive(const FString& VehicleId, bool bActive)
{
	FMGSpoilerConfig* Spoiler = VehicleSpoilers.Find(VehicleId);
	if (Spoiler)
	{
		Spoiler->bIsActive = bActive;
	}
}

// ============================================================================
// Stats
// ============================================================================

FMGAeroPlayerStats UMGAerodynamicsSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGAeroPlayerStats* Found = PlayerStats.Find(PlayerId))
	{
		return *Found;
	}
	return FMGAeroPlayerStats();
}

void UMGAerodynamicsSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGAeroPlayerStats Stats;
	Stats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, Stats);
}

// ============================================================================
// Configuration
// ============================================================================

void UMGAerodynamicsSubsystem::SetSlipstreamConfig(const FMGSlipstreamConfig& Config)
{
	SlipstreamConfig = Config;
}

FMGSlipstreamConfig UMGAerodynamicsSubsystem::GetSlipstreamConfig() const
{
	return SlipstreamConfig;
}

void UMGAerodynamicsSubsystem::SetGlobalAeroConfig(const FMGGlobalAeroConfig& Config)
{
	GlobalConfig = Config;
}

FMGGlobalAeroConfig UMGAerodynamicsSubsystem::GetGlobalAeroConfig() const
{
	return GlobalConfig;
}

// ============================================================================
// Update
// ============================================================================

void UMGAerodynamicsSubsystem::UpdateAerodynamics(float DeltaTime)
{
	UpdateSlipstreams(DeltaTime);
	UpdateWindEffects(DeltaTime);
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGAerodynamicsSubsystem::SaveAeroData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Aerodynamics");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("aero_stats.dat");

	FBufferArchive SaveArchive;

	// Version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save player stats
	int32 NumPlayers = PlayerStats.Num();
	SaveArchive << NumPlayers;

	for (const auto& Pair : PlayerStats)
	{
		FString PlayerId = Pair.Key;
		SaveArchive << PlayerId;

		const FMGAeroPlayerStats& Stats = Pair.Value;
		float TotalSlipstreamTime = Stats.TotalSlipstreamTime;
		float LongestSlipstreamSession = Stats.LongestSlipstreamSession;
		int32 SlingshotsUsed = Stats.SlingshotsUsed;
		int32 OvertakesFromSlipstream = Stats.OvertakesFromSlipstream;
		int32 SlipstreamPointsEarned = Stats.SlipstreamPointsEarned;
		float TopSpeedInSlipstream = Stats.TopSpeedInSlipstream;
		float DistanceDraftedMiles = Stats.DistanceDraftedMiles;
		int32 PerfectSlipstreams = Stats.PerfectSlipstreams;

		SaveArchive << TotalSlipstreamTime;
		SaveArchive << LongestSlipstreamSession;
		SaveArchive << SlingshotsUsed;
		SaveArchive << OvertakesFromSlipstream;
		SaveArchive << SlipstreamPointsEarned;
		SaveArchive << TopSpeedInSlipstream;
		SaveArchive << DistanceDraftedMiles;
		SaveArchive << PerfectSlipstreams;
	}

	// Save custom aero profiles
	int32 NumProfiles = AeroProfiles.Num();
	SaveArchive << NumProfiles;

	for (const auto& Pair : AeroProfiles)
	{
		FString ProfileId = Pair.Key;
		SaveArchive << ProfileId;

		const FMGAeroProfileDefinition& Profile = Pair.Value;
		FString DisplayNameStr = Profile.DisplayName.ToString();
		int32 TypeInt = static_cast<int32>(Profile.Type);
		float DragCoeff = Profile.DragCoefficient;
		float LiftCoeff = Profile.LiftCoefficient;
		float DownforceCoeff = Profile.DownforceCoefficient;
		float FrontalArea = Profile.FrontalArea;
		float DownforceFrontBias = Profile.DownforceFrontBias;

		SaveArchive << DisplayNameStr;
		SaveArchive << TypeInt;
		SaveArchive << DragCoeff;
		SaveArchive << LiftCoeff;
		SaveArchive << DownforceCoeff;
		SaveArchive << FrontalArea;
		SaveArchive << DownforceFrontBias;
	}

	// Write to file
	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	UE_LOG(LogTemp, Log, TEXT("MGAerodynamicsSubsystem: Saved aero data for %d players, %d profiles"), NumPlayers, NumProfiles);
}

void UMGAerodynamicsSubsystem::LoadAeroData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Aerodynamics") / TEXT("aero_stats.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("MGAerodynamicsSubsystem: No saved aero data found"));
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGAerodynamicsSubsystem: Unknown save version %d"), Version);
		return;
	}

	// Load player stats
	int32 NumPlayers;
	LoadArchive << NumPlayers;

	for (int32 i = 0; i < NumPlayers; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGAeroPlayerStats Stats;
		Stats.PlayerId = PlayerId;

		LoadArchive << Stats.TotalSlipstreamTime;
		LoadArchive << Stats.LongestSlipstreamSession;
		LoadArchive << Stats.SlingshotsUsed;
		LoadArchive << Stats.OvertakesFromSlipstream;
		LoadArchive << Stats.SlipstreamPointsEarned;
		LoadArchive << Stats.TopSpeedInSlipstream;
		LoadArchive << Stats.DistanceDraftedMiles;
		LoadArchive << Stats.PerfectSlipstreams;

		PlayerStats.Add(PlayerId, Stats);
	}

	// Load custom aero profiles
	int32 NumProfiles;
	LoadArchive << NumProfiles;

	for (int32 i = 0; i < NumProfiles; ++i)
	{
		FString ProfileId;
		LoadArchive << ProfileId;

		FMGAeroProfileDefinition Profile;
		Profile.ProfileId = ProfileId;

		FString DisplayNameStr;
		int32 TypeInt;

		LoadArchive << DisplayNameStr;
		LoadArchive << TypeInt;
		LoadArchive << Profile.DragCoefficient;
		LoadArchive << Profile.LiftCoefficient;
		LoadArchive << Profile.DownforceCoefficient;
		LoadArchive << Profile.FrontalArea;
		LoadArchive << Profile.DownforceFrontBias;

		Profile.DisplayName = FText::FromString(DisplayNameStr);
		Profile.Type = static_cast<EMGAeroProfile>(TypeInt);

		AeroProfiles.Add(ProfileId, Profile);
	}

	UE_LOG(LogTemp, Log, TEXT("MGAerodynamicsSubsystem: Loaded aero data for %d players, %d profiles"), NumPlayers, NumProfiles);
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGAerodynamicsSubsystem::TickAerodynamics(float DeltaTime)
{
	UpdateAerodynamics(DeltaTime);
}

void UMGAerodynamicsSubsystem::UpdateSlipstreams(float DeltaTime)
{
	for (auto& Pair : ActiveSlipstreams)
	{
		FMGSlipstreamSession& Session = Pair.Value;
		Session.Duration += DeltaTime;

		// Charge slingshot
		if (Session.State == EMGSlipstreamState::Active || Session.State == EMGSlipstreamState::Optimal)
		{
			Session.ChargeLevel += SlipstreamConfig.ChargeRate * DeltaTime;
			Session.SlingshotReady = FMath::Clamp(Session.ChargeLevel, 0.0f, 1.0f);

			if (Session.SlingshotReady >= 1.0f)
			{
				OnSlingshotReady.Broadcast(Pair.Key, SlipstreamConfig.SlingshotBoost, SlipstreamConfig.SlingshotDuration);
			}
		}
	}
}

void UMGAerodynamicsSubsystem::UpdateWindEffects(float DeltaTime)
{
	// Update turbulence and gusts in wind zones
	for (auto& Pair : WindZones)
	{
		FMGWindZone& Zone = Pair.Value;

		if (Zone.GustFrequency > 0.0f)
		{
			// Simulate gusts (simplified)
			float GustWave = FMath::Sin(FDateTime::Now().GetSecond() * Zone.GustFrequency);
			Zone.WindSpeed *= (1.0f + GustWave * Zone.GustIntensity);
		}
	}
}

void UMGAerodynamicsSubsystem::CalculateAeroForces(const FString& VehicleId, float DeltaTime)
{
	// This is called from UpdateVehicleAero
}

float UMGAerodynamicsSubsystem::GetAirDensityAtLocation(FVector Location) const
{
	float Altitude = Location.Z / 100.0f; // Convert cm to m
	float DensityReduction = Altitude * GlobalConfig.AltitudeEffect;
	return GlobalConfig.AirDensityBase * (1.0f - DensityReduction);
}

bool UMGAerodynamicsSubsystem::IsInSlipstreamCone(FVector FollowerPos, FVector LeaderPos, FVector LeaderForward, float ConeAngle, float MaxDist) const
{
	FVector ToFollower = FollowerPos - LeaderPos;
	float Distance = ToFollower.Size();

	if (Distance > MaxDist || Distance < 1.0f)
	{
		return false;
	}

	FVector ToFollowerNorm = ToFollower.GetSafeNormal();

	// Follower should be behind the leader
	float Dot = FVector::DotProduct(-LeaderForward, ToFollowerNorm);

	float ConeAngleRad = FMath::DegreesToRadians(ConeAngle);
	float ConeThreshold = FMath::Cos(ConeAngleRad);

	return Dot >= ConeThreshold;
}

void UMGAerodynamicsSubsystem::UpdateSlipstreamState(FMGSlipstreamSession& Session, float Distance, float DeltaTime)
{
	EMGSlipstreamState OldState = Session.State;
	Session.Distance = Distance;

	// Calculate drag reduction based on distance
	float DistanceFactor = 1.0f - ((Distance - SlipstreamConfig.MinDistance) / (SlipstreamConfig.MaxDistance - SlipstreamConfig.MinDistance));
	DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

	Session.DragReduction = SlipstreamConfig.MaxDragReduction * DistanceFactor;
	Session.SpeedBonus = SlipstreamConfig.MaxSpeedBonus * DistanceFactor;

	// Determine state
	bool bOptimal = FMath::Abs(Distance - SlipstreamConfig.OptimalDistance) < 200.0f;

	if (bOptimal)
	{
		Session.State = EMGSlipstreamState::Optimal;
		Session.bIsOptimal = true;
	}
	else if (Session.Duration < 0.5f)
	{
		Session.State = EMGSlipstreamState::Entering;
	}
	else
	{
		Session.State = EMGSlipstreamState::Active;
		Session.bIsOptimal = false;
	}

	if (OldState != Session.State)
	{
		OnSlipstreamStateChanged.Broadcast(Session.FollowerVehicleId, OldState, Session.State);

		if (Session.State == EMGSlipstreamState::Optimal)
		{
			OnOptimalSlipstream.Broadcast(Session.FollowerVehicleId, 1.5f);
		}
	}
}

void UMGAerodynamicsSubsystem::AwardSlipstreamPoints(const FString& VehicleId, float Duration)
{
	int32 Points = FMath::RoundToInt(Duration * SlipstreamConfig.SlipstreamPoints);

	FMGAeroPlayerStats& Stats = PlayerStats.FindOrAdd(VehicleId);
	Stats.SlipstreamPointsEarned += Points;
}

void UMGAerodynamicsSubsystem::UpdatePlayerStats(const FString& PlayerId, const FMGSlipstreamSession& Session)
{
	FMGAeroPlayerStats& Stats = PlayerStats.FindOrAdd(PlayerId);
	Stats.PlayerId = PlayerId;
	Stats.TotalSlipstreamTime += Session.Duration;

	if (Session.Duration > Stats.LongestSlipstreamSession)
	{
		Stats.LongestSlipstreamSession = Session.Duration;
	}

	if (Session.bIsOptimal)
	{
		Stats.PerfectSlipstreams++;
	}

	// Convert duration to miles (rough estimate)
	float SpeedMPH = 100.0f; // Assume average speed
	float Miles = (SpeedMPH * Session.Duration) / 3600.0f;
	Stats.DistanceDraftedMiles += Miles;
}

FString UMGAerodynamicsSubsystem::GenerateSessionId() const
{
	return FString::Printf(TEXT("SLIP_%d_%lld"), ++const_cast<UMGAerodynamicsSubsystem*>(this)->SessionCounter,
	                       FDateTime::Now().GetTicks());
}
