// Copyright Epic Games, Inc. All Rights Reserved.

#include "Airtime/MGAirtimeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGAirtimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default scoring config
	ScoringConfig.PointsPerSecondAirtime = 100.0f;
	ScoringConfig.PointsPerMeterHeight = 50.0f;
	ScoringConfig.PointsPerMeterDistance = 25.0f;
	ScoringConfig.PerfectLandingMultiplier = 2.0f;
	ScoringConfig.GreatLandingMultiplier = 1.5f;
	ScoringConfig.GoodLandingMultiplier = 1.0f;
	ScoringConfig.RoughLandingMultiplier = 0.5f;
	ScoringConfig.BadLandingMultiplier = 0.25f;
	ScoringConfig.CrashLandingMultiplier = 0.0f;
	ScoringConfig.TrickChainMultiplierPerTrick = 0.25f;
	ScoringConfig.MaxTrickChainMultiplier = 3.0f;
	ScoringConfig.NearMissWhileAirborneBonus = 1.5f;
	ScoringConfig.SpeedBonusThreshold = 100.0f;
	ScoringConfig.SpeedBonusMultiplier = 1.25f;

	// Initialize landing config
	LandingConfig.PerfectAngleTolerance = 5.0f;
	LandingConfig.GreatAngleTolerance = 15.0f;
	LandingConfig.GoodAngleTolerance = 30.0f;
	LandingConfig.RoughAngleTolerance = 45.0f;
	LandingConfig.CrashAngleThreshold = 60.0f;
	LandingConfig.MinGroundCheckDistance = 50.0f;
	LandingConfig.LandingImpactThreshold = 500.0f;
	LandingConfig.PerfectSpeedRetention = 0.95f;
	LandingConfig.CrashSpeedLoss = 0.5f;

	// Register default tricks
	{
		FMGTrickDefinition BarrelRoll;
		BarrelRoll.Type = EMGAirtimeTrick::Barrel;
		BarrelRoll.DisplayName = FText::FromString(TEXT("Barrel Roll"));
		BarrelRoll.BasePoints = 200;
		BarrelRoll.MinAirtimeRequired = 1.5f;
		BarrelRoll.RotationRequired = 360.0f;
		BarrelRoll.RotationAxis = FRotator(1.0f, 0.0f, 0.0f);
		BarrelRoll.ExecutionTime = 0.8f;
		RegisterTrick(BarrelRoll);
	}
	{
		FMGTrickDefinition Flip;
		Flip.Type = EMGAirtimeTrick::Flip;
		Flip.DisplayName = FText::FromString(TEXT("Flip"));
		Flip.BasePoints = 250;
		Flip.MinAirtimeRequired = 2.0f;
		Flip.RotationRequired = 360.0f;
		Flip.RotationAxis = FRotator(0.0f, 1.0f, 0.0f);
		Flip.ExecutionTime = 1.0f;
		RegisterTrick(Flip);
	}
	{
		FMGTrickDefinition Spin;
		Spin.Type = EMGAirtimeTrick::Spin;
		Spin.DisplayName = FText::FromString(TEXT("360 Spin"));
		Spin.BasePoints = 150;
		Spin.MinAirtimeRequired = 1.0f;
		Spin.RotationRequired = 360.0f;
		Spin.RotationAxis = FRotator(0.0f, 0.0f, 1.0f);
		Spin.ExecutionTime = 0.6f;
		RegisterTrick(Spin);
	}
	{
		FMGTrickDefinition Corkscrew;
		Corkscrew.Type = EMGAirtimeTrick::Corkscrew;
		Corkscrew.DisplayName = FText::FromString(TEXT("Corkscrew"));
		Corkscrew.BasePoints = 400;
		Corkscrew.MinAirtimeRequired = 2.5f;
		Corkscrew.RotationRequired = 720.0f;
		Corkscrew.RotationAxis = FRotator(1.0f, 0.0f, 1.0f);
		Corkscrew.ExecutionTime = 1.2f;
		RegisterTrick(Corkscrew);
	}
	{
		FMGTrickDefinition FlatSpin;
		FlatSpin.Type = EMGAirtimeTrick::FlatSpin;
		FlatSpin.DisplayName = FText::FromString(TEXT("Flat Spin"));
		FlatSpin.BasePoints = 300;
		FlatSpin.MinAirtimeRequired = 1.8f;
		FlatSpin.RotationRequired = 540.0f;
		FlatSpin.RotationAxis = FRotator(0.0f, 0.0f, 1.0f);
		FlatSpin.ExecutionTime = 0.9f;
		RegisterTrick(FlatSpin);
	}
	{
		FMGTrickDefinition Invert;
		Invert.Type = EMGAirtimeTrick::Invert;
		Invert.DisplayName = FText::FromString(TEXT("Invert"));
		Invert.BasePoints = 175;
		Invert.MinAirtimeRequired = 1.2f;
		Invert.RotationRequired = 180.0f;
		Invert.RotationAxis = FRotator(0.0f, 1.0f, 0.0f);
		Invert.ExecutionTime = 0.5f;
		RegisterTrick(Invert);
	}
	{
		FMGTrickDefinition NoseGrab;
		NoseGrab.Type = EMGAirtimeTrick::NoseGrab;
		NoseGrab.DisplayName = FText::FromString(TEXT("Nose Grab"));
		NoseGrab.BasePoints = 100;
		NoseGrab.MinAirtimeRequired = 0.8f;
		NoseGrab.RotationRequired = 0.0f;
		NoseGrab.RotationAxis = FRotator::ZeroRotator;
		NoseGrab.ExecutionTime = 0.4f;
		RegisterTrick(NoseGrab);
	}
	{
		FMGTrickDefinition TailGrab;
		TailGrab.Type = EMGAirtimeTrick::TailGrab;
		TailGrab.DisplayName = FText::FromString(TEXT("Tail Grab"));
		TailGrab.BasePoints = 100;
		TailGrab.MinAirtimeRequired = 0.8f;
		TailGrab.RotationRequired = 0.0f;
		TailGrab.RotationAxis = FRotator::ZeroRotator;
		TailGrab.ExecutionTime = 0.4f;
		RegisterTrick(TailGrab);
	}

	// Start airtime tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AirtimeTickTimer, [this]()
		{
			TickAirtime(0.033f);
		}, 0.033f, true);
	}
}

void UMGAirtimeSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AirtimeTickTimer);
	}

	SaveAirtimeData();

	Ramps.Empty();
	ActiveJumps.Empty();
	TrickDefinitions.Empty();
	RampRecords.Empty();
	PlayerStats.Empty();
	DiscoveredRamps.Empty();

	Super::Deinitialize();
}

// Ramp Registration
void UMGAirtimeSubsystem::RegisterRamp(const FMGRampDefinition& Ramp)
{
	if (Ramp.RampId.IsEmpty())
	{
		return;
	}

	Ramps.Add(Ramp.RampId, Ramp);

	// Initialize record if not exists
	if (!RampRecords.Contains(Ramp.RampId))
	{
		FMGRampRecord NewRecord;
		NewRecord.RampId = Ramp.RampId;
		RampRecords.Add(Ramp.RampId, NewRecord);
	}
}

void UMGAirtimeSubsystem::UnregisterRamp(const FString& RampId)
{
	Ramps.Remove(RampId);
}

FMGRampDefinition UMGAirtimeSubsystem::GetRamp(const FString& RampId) const
{
	if (const FMGRampDefinition* Found = Ramps.Find(RampId))
	{
		return *Found;
	}
	return FMGRampDefinition();
}

TArray<FMGRampDefinition> UMGAirtimeSubsystem::GetAllRamps() const
{
	TArray<FMGRampDefinition> Result;
	Ramps.GenerateValueArray(Result);
	return Result;
}

TArray<FMGRampDefinition> UMGAirtimeSubsystem::GetRampsInArea(FVector Center, float Radius) const
{
	TArray<FMGRampDefinition> Result;
	float RadiusSq = Radius * Radius;

	for (const auto& Pair : Ramps)
	{
		if (FVector::DistSquared(Pair.Value.Location, Center) <= RadiusSq)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGRampDefinition> UMGAirtimeSubsystem::GetRampsForTrack(const FString& TrackId) const
{
	TArray<FMGRampDefinition> Result;

	for (const auto& Pair : Ramps)
	{
		if (Pair.Value.TrackId == TrackId)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

// Jump Detection
bool UMGAirtimeSubsystem::CheckRampLaunch(const FString& PlayerId, FVector Location, FVector Velocity)
{
	if (IsAirborne(PlayerId))
	{
		return false;
	}

	float Speed = Velocity.Size();

	for (const auto& Pair : Ramps)
	{
		const FMGRampDefinition& Ramp = Pair.Value;

		if (Speed < Ramp.MinLaunchSpeed)
		{
			continue;
		}

		if (IsInRampTrigger(Location, Ramp))
		{
			// Apply launch angle to velocity
			FVector LaunchDir = Ramp.Rotation.Vector();
			FVector LaunchVelocity = Velocity;

			float LaunchAngleRad = FMath::DegreesToRadians(Ramp.LaunchAngle);
			float HorizontalSpeed = FMath::Cos(LaunchAngleRad) * Speed;
			float VerticalSpeed = FMath::Sin(LaunchAngleRad) * Speed;

			// Apply speed boost
			float BoostMultiplier = 1.0f + (Ramp.SpeedBoostPercent / 100.0f);
			HorizontalSpeed *= BoostMultiplier;
			VerticalSpeed *= BoostMultiplier;

			LaunchVelocity = LaunchDir * HorizontalSpeed;
			LaunchVelocity.Z = VerticalSpeed;

			StartJump(PlayerId, Ramp.RampId, Location, LaunchVelocity);
			return true;
		}
	}

	return false;
}

void UMGAirtimeSubsystem::StartJump(const FString& PlayerId, const FString& RampId, FVector LaunchPosition, FVector LaunchVelocity)
{
	FMGActiveJump NewJump;
	NewJump.JumpId = GenerateJumpId();
	NewJump.PlayerId = PlayerId;
	NewJump.bIsAirborne = true;
	NewJump.LaunchTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	NewJump.LaunchPosition = LaunchPosition;
	NewJump.LaunchVelocity = LaunchVelocity;
	NewJump.LaunchSpeed = LaunchVelocity.Size();
	NewJump.LaunchAngle = FMath::RadiansToDegrees(FMath::Atan2(LaunchVelocity.Z, FMath::Sqrt(LaunchVelocity.X * LaunchVelocity.X + LaunchVelocity.Y * LaunchVelocity.Y)));
	NewJump.CurrentMultiplier = 1.0f;

	if (const FMGRampDefinition* Ramp = Ramps.Find(RampId))
	{
		NewJump.Type = Ramp->Type;
		NewJump.RampId = RampId;

		// Check for secret ramp discovery
		if (Ramp->bIsSecret && !DiscoveredRamps.Contains(RampId))
		{
			DiscoverSecretRamp(PlayerId, RampId);
		}
	}
	else
	{
		NewJump.Type = EMGJumpType::Terrain;
	}

	ActiveJumps.Add(PlayerId, NewJump);

	OnJumpStarted.Broadcast(PlayerId, NewJump.Type, NewJump.LaunchSpeed);
}

void UMGAirtimeSubsystem::UpdateJump(const FString& PlayerId, FVector Position, FVector Velocity, FRotator Rotation, bool bIsGrounded, float DeltaTime)
{
	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump || !Jump->bIsAirborne)
	{
		return;
	}

	if (bIsGrounded)
	{
		// Calculate landing angle
		FVector VelocityDir = Velocity.GetSafeNormal();
		float LandingAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Abs(VelocityDir.Z)));

		EndJump(PlayerId, Position, Velocity, LandingAngle);
		return;
	}

	// Update jump metrics
	UpdateJumpMetrics(*Jump, Position, DeltaTime);

	// Update rotation tracking
	FRotator DeltaRotation = Rotation - Jump->CurrentRotation;
	Jump->CurrentRotation = Rotation;
	Jump->TotalRotation += DeltaRotation;

	// Detect tricks from rotation
	DetectTricks(PlayerId, DeltaRotation, DeltaTime);

	// Check for max height
	float Height = Position.Z - Jump->LaunchPosition.Z;
	if (Height > Jump->MaxHeight)
	{
		Jump->MaxHeight = Height;
		OnMaxHeightReached.Broadcast(PlayerId, Jump->MaxHeight);
	}

	OnAirtimeUpdate.Broadcast(PlayerId, Jump->AirtimeDuration, Height);
}

FMGJumpResult UMGAirtimeSubsystem::EndJump(const FString& PlayerId, FVector LandingPosition, FVector LandingVelocity, float LandingAngle)
{
	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump)
	{
		return FMGJumpResult();
	}

	// Calculate landing quality based on angle
	EMGLandingQuality Quality;
	if (LandingAngle <= LandingConfig.PerfectAngleTolerance)
	{
		Quality = EMGLandingQuality::Perfect;
	}
	else if (LandingAngle <= LandingConfig.GreatAngleTolerance)
	{
		Quality = EMGLandingQuality::Great;
	}
	else if (LandingAngle <= LandingConfig.GoodAngleTolerance)
	{
		Quality = EMGLandingQuality::Good;
	}
	else if (LandingAngle <= LandingConfig.RoughAngleTolerance)
	{
		Quality = EMGLandingQuality::Rough;
	}
	else if (LandingAngle <= LandingConfig.CrashAngleThreshold)
	{
		Quality = EMGLandingQuality::Bad;
	}
	else
	{
		Quality = EMGLandingQuality::Crash;
	}

	// Finalize horizontal distance
	Jump->HorizontalDistance = FVector::Dist2D(LandingPosition, Jump->LaunchPosition);

	FMGJumpResult Result = FinalizeJump(PlayerId, Quality);
	Result.LandingSpeed = LandingVelocity.Size();

	// Update records and stats
	UpdateRecords(PlayerId, Result);
	UpdatePlayerStats(PlayerId, Result);

	// Broadcast landing event
	int32 LandingBonus = GetLandingBonus(Quality);
	OnLanding.Broadcast(PlayerId, Quality, LandingBonus);
	OnJumpRating.Broadcast(PlayerId, Result.Rating, Result.RampId);
	OnJumpEnded.Broadcast(PlayerId, Result);

	// Remove active jump
	ActiveJumps.Remove(PlayerId);

	return Result;
}

bool UMGAirtimeSubsystem::IsAirborne(const FString& PlayerId) const
{
	if (const FMGActiveJump* Jump = ActiveJumps.Find(PlayerId))
	{
		return Jump->bIsAirborne;
	}
	return false;
}

FMGActiveJump UMGAirtimeSubsystem::GetActiveJump(const FString& PlayerId) const
{
	if (const FMGActiveJump* Jump = ActiveJumps.Find(PlayerId))
	{
		return *Jump;
	}
	return FMGActiveJump();
}

// Tricks
void UMGAirtimeSubsystem::RegisterTrick(const FMGTrickDefinition& Trick)
{
	TrickDefinitions.Add(Trick.Type, Trick);
}

FMGTrickDefinition UMGAirtimeSubsystem::GetTrickDefinition(EMGAirtimeTrick Type) const
{
	if (const FMGTrickDefinition* Found = TrickDefinitions.Find(Type))
	{
		return *Found;
	}
	return FMGTrickDefinition();
}

bool UMGAirtimeSubsystem::PerformTrick(const FString& PlayerId, EMGAirtimeTrick Trick)
{
	if (!CanPerformTrick(PlayerId, Trick))
	{
		return false;
	}

	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump)
	{
		return false;
	}

	const FMGTrickDefinition* TrickDef = TrickDefinitions.Find(Trick);
	if (!TrickDef)
	{
		return false;
	}

	// Add trick to active tricks
	Jump->ActiveTricks.Add(Trick);
	Jump->TricksCompleted++;

	// Calculate trick score with chain bonus
	int32 TrickScore = CalculateTrickScore(Trick, Jump->TricksCompleted);
	Jump->CurrentScore += TrickScore;

	// Update chain multiplier
	float ChainBonus = FMath::Min(1.0f + (Jump->TricksCompleted - 1) * ScoringConfig.TrickChainMultiplierPerTrick,
		ScoringConfig.MaxTrickChainMultiplier);
	Jump->CurrentMultiplier = ChainBonus;

	OnTrickCompleted.Broadcast(PlayerId, Trick, TrickScore);

	if (Jump->TricksCompleted > 1)
	{
		OnTrickChain.Broadcast(PlayerId, Jump->TricksCompleted, ChainBonus);
	}

	return true;
}

void UMGAirtimeSubsystem::DetectTricks(const FString& PlayerId, FRotator DeltaRotation, float DeltaTime)
{
	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump || !Jump->bIsAirborne)
	{
		return;
	}

	// Accumulate total rotation
	float TotalPitch = FMath::Abs(Jump->TotalRotation.Pitch);
	float TotalYaw = FMath::Abs(Jump->TotalRotation.Yaw);
	float TotalRoll = FMath::Abs(Jump->TotalRotation.Roll);

	// Check for barrel roll (roll axis)
	if (TotalRoll >= 360.0f && CanPerformTrick(PlayerId, EMGAirtimeTrick::Barrel))
	{
		PerformTrick(PlayerId, EMGAirtimeTrick::Barrel);
		Jump->TotalRotation.Roll = FMath::Fmod(Jump->TotalRotation.Roll, 360.0f);
	}

	// Check for flip (pitch axis)
	if (TotalPitch >= 360.0f && CanPerformTrick(PlayerId, EMGAirtimeTrick::Flip))
	{
		PerformTrick(PlayerId, EMGAirtimeTrick::Flip);
		Jump->TotalRotation.Pitch = FMath::Fmod(Jump->TotalRotation.Pitch, 360.0f);
	}

	// Check for spin (yaw axis)
	if (TotalYaw >= 360.0f && CanPerformTrick(PlayerId, EMGAirtimeTrick::Spin))
	{
		PerformTrick(PlayerId, EMGAirtimeTrick::Spin);
		Jump->TotalRotation.Yaw = FMath::Fmod(Jump->TotalRotation.Yaw, 360.0f);
	}

	// Check for flat spin (540+ yaw)
	if (TotalYaw >= 540.0f && CanPerformTrick(PlayerId, EMGAirtimeTrick::FlatSpin))
	{
		PerformTrick(PlayerId, EMGAirtimeTrick::FlatSpin);
		Jump->TotalRotation.Yaw = FMath::Fmod(Jump->TotalRotation.Yaw, 540.0f);
	}

	// Check for invert (180 pitch)
	if (TotalPitch >= 180.0f && TotalPitch < 360.0f && !Jump->ActiveTricks.Contains(EMGAirtimeTrick::Invert))
	{
		if (CanPerformTrick(PlayerId, EMGAirtimeTrick::Invert))
		{
			PerformTrick(PlayerId, EMGAirtimeTrick::Invert);
		}
	}

	// Check for corkscrew (combined roll and yaw)
	if (TotalRoll >= 360.0f && TotalYaw >= 360.0f && CanPerformTrick(PlayerId, EMGAirtimeTrick::Corkscrew))
	{
		PerformTrick(PlayerId, EMGAirtimeTrick::Corkscrew);
	}
}

bool UMGAirtimeSubsystem::CanPerformTrick(const FString& PlayerId, EMGAirtimeTrick Trick) const
{
	const FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump || !Jump->bIsAirborne)
	{
		return false;
	}

	// Check if ramp allows tricks
	if (const FMGRampDefinition* Ramp = Ramps.Find(Jump->RampId))
	{
		if (!Ramp->bAllowTricks)
		{
			return false;
		}
	}

	const FMGTrickDefinition* TrickDef = TrickDefinitions.Find(Trick);
	if (!TrickDef)
	{
		return false;
	}

	// Check minimum airtime
	if (Jump->AirtimeDuration < TrickDef->MinAirtimeRequired)
	{
		return false;
	}

	return true;
}

TArray<EMGAirtimeTrick> UMGAirtimeSubsystem::GetAvailableTricks(const FString& PlayerId) const
{
	TArray<EMGAirtimeTrick> Result;

	const FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump || !Jump->bIsAirborne)
	{
		return Result;
	}

	for (const auto& Pair : TrickDefinitions)
	{
		if (CanPerformTrick(PlayerId, Pair.Key))
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

// Landing
EMGLandingQuality UMGAirtimeSubsystem::CalculateLandingQuality(FVector Velocity, FVector SurfaceNormal, FRotator VehicleRotation) const
{
	// Calculate angle between vehicle up and surface normal
	FVector VehicleUp = VehicleRotation.Quaternion().GetUpVector();
	float DotProduct = FVector::DotProduct(VehicleUp, SurfaceNormal);
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

	// Adjust for velocity direction
	FVector VelocityDir = Velocity.GetSafeNormal();
	float VelocityAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Abs(FVector::DotProduct(VelocityDir, SurfaceNormal))));

	float CombinedAngle = (AngleDegrees + VelocityAngle) * 0.5f;

	if (CombinedAngle <= LandingConfig.PerfectAngleTolerance)
	{
		return EMGLandingQuality::Perfect;
	}
	else if (CombinedAngle <= LandingConfig.GreatAngleTolerance)
	{
		return EMGLandingQuality::Great;
	}
	else if (CombinedAngle <= LandingConfig.GoodAngleTolerance)
	{
		return EMGLandingQuality::Good;
	}
	else if (CombinedAngle <= LandingConfig.RoughAngleTolerance)
	{
		return EMGLandingQuality::Rough;
	}
	else if (CombinedAngle <= LandingConfig.CrashAngleThreshold)
	{
		return EMGLandingQuality::Bad;
	}

	return EMGLandingQuality::Crash;
}

int32 UMGAirtimeSubsystem::GetLandingBonus(EMGLandingQuality Quality) const
{
	switch (Quality)
	{
		case EMGLandingQuality::Perfect:
			return 500;
		case EMGLandingQuality::Great:
			return 250;
		case EMGLandingQuality::Good:
			return 100;
		case EMGLandingQuality::Rough:
			return 25;
		case EMGLandingQuality::Bad:
			return 0;
		case EMGLandingQuality::Crash:
			return -100;
		default:
			return 0;
	}
}

float UMGAirtimeSubsystem::GetLandingSpeedRetention(EMGLandingQuality Quality) const
{
	switch (Quality)
	{
		case EMGLandingQuality::Perfect:
			return LandingConfig.PerfectSpeedRetention;
		case EMGLandingQuality::Great:
			return 0.9f;
		case EMGLandingQuality::Good:
			return 0.8f;
		case EMGLandingQuality::Rough:
			return 0.65f;
		case EMGLandingQuality::Bad:
			return 0.5f;
		case EMGLandingQuality::Crash:
			return LandingConfig.CrashSpeedLoss;
		default:
			return 0.8f;
	}
}

// Scoring
int32 UMGAirtimeSubsystem::CalculateJumpScore(const FMGActiveJump& Jump, EMGLandingQuality Landing) const
{
	float BaseScore = 0.0f;

	// Score from airtime
	BaseScore += Jump.AirtimeDuration * ScoringConfig.PointsPerSecondAirtime;

	// Score from height
	BaseScore += (Jump.MaxHeight / 100.0f) * ScoringConfig.PointsPerMeterHeight;

	// Score from distance
	BaseScore += (Jump.HorizontalDistance / 100.0f) * ScoringConfig.PointsPerMeterDistance;

	// Add trick score
	BaseScore += Jump.CurrentScore;

	// Apply landing multiplier
	float LandingMultiplier = 1.0f;
	switch (Landing)
	{
		case EMGLandingQuality::Perfect:
			LandingMultiplier = ScoringConfig.PerfectLandingMultiplier;
			break;
		case EMGLandingQuality::Great:
			LandingMultiplier = ScoringConfig.GreatLandingMultiplier;
			break;
		case EMGLandingQuality::Good:
			LandingMultiplier = ScoringConfig.GoodLandingMultiplier;
			break;
		case EMGLandingQuality::Rough:
			LandingMultiplier = ScoringConfig.RoughLandingMultiplier;
			break;
		case EMGLandingQuality::Bad:
			LandingMultiplier = ScoringConfig.BadLandingMultiplier;
			break;
		case EMGLandingQuality::Crash:
			LandingMultiplier = ScoringConfig.CrashLandingMultiplier;
			break;
	}

	// Apply near miss bonus
	if (Jump.bNearMissWhileAirborne)
	{
		LandingMultiplier *= ScoringConfig.NearMissWhileAirborneBonus;
	}

	// Apply ramp multiplier
	float RampMultiplier = 1.0f;
	if (const FMGRampDefinition* Ramp = Ramps.Find(Jump.RampId))
	{
		RampMultiplier = Ramp->PointMultiplier;
	}

	return FMath::RoundToInt(BaseScore * LandingMultiplier * RampMultiplier * Jump.CurrentMultiplier);
}

EMGJumpRating UMGAirtimeSubsystem::CalculateRating(const FString& RampId, float Distance) const
{
	const FMGRampDefinition* Ramp = Ramps.Find(RampId);
	if (!Ramp)
	{
		// Default ratings for unknown ramps
		float DistanceMeters = Distance / 100.0f;
		if (DistanceMeters >= 200.0f) return EMGJumpRating::Legend;
		if (DistanceMeters >= 150.0f) return EMGJumpRating::Diamond;
		if (DistanceMeters >= 100.0f) return EMGJumpRating::Platinum;
		if (DistanceMeters >= 75.0f) return EMGJumpRating::Gold;
		if (DistanceMeters >= 50.0f) return EMGJumpRating::Silver;
		if (DistanceMeters >= 30.0f) return EMGJumpRating::Bronze;
		return EMGJumpRating::None;
	}

	float DistanceMeters = Distance / 100.0f;

	if (DistanceMeters >= Ramp->LegendDistanceMeters)
	{
		return EMGJumpRating::Legend;
	}
	else if (DistanceMeters >= Ramp->DiamondDistanceMeters)
	{
		return EMGJumpRating::Diamond;
	}
	else if (DistanceMeters >= Ramp->PlatinumDistanceMeters)
	{
		return EMGJumpRating::Platinum;
	}
	else if (DistanceMeters >= Ramp->GoldDistanceMeters)
	{
		return EMGJumpRating::Gold;
	}
	else if (DistanceMeters >= Ramp->SilverDistanceMeters)
	{
		return EMGJumpRating::Silver;
	}
	else if (DistanceMeters >= Ramp->BronzeDistanceMeters)
	{
		return EMGJumpRating::Bronze;
	}

	return EMGJumpRating::None;
}

int32 UMGAirtimeSubsystem::CalculateTrickScore(EMGAirtimeTrick Trick, int32 ChainCount) const
{
	const FMGTrickDefinition* TrickDef = TrickDefinitions.Find(Trick);
	if (!TrickDef)
	{
		return 0;
	}

	float ChainMultiplier = 1.0f;
	if (ChainCount > 1 && TrickDef->bCanChain)
	{
		ChainMultiplier = 1.0f + (ChainCount - 1) * (TrickDef->ChainMultiplier - 1.0f);
		ChainMultiplier = FMath::Min(ChainMultiplier, ScoringConfig.MaxTrickChainMultiplier);
	}

	return FMath::RoundToInt(TrickDef->BasePoints * ChainMultiplier);
}

// Records
FMGRampRecord UMGAirtimeSubsystem::GetRampRecord(const FString& RampId) const
{
	if (const FMGRampRecord* Found = RampRecords.Find(RampId))
	{
		return *Found;
	}
	return FMGRampRecord();
}

float UMGAirtimeSubsystem::GetPersonalBestDistance(const FString& RampId) const
{
	if (const FMGRampRecord* Found = RampRecords.Find(RampId))
	{
		return Found->PersonalBestDistance;
	}
	return 0.0f;
}

float UMGAirtimeSubsystem::GetWorldRecord(const FString& RampId) const
{
	if (const FMGRampRecord* Found = RampRecords.Find(RampId))
	{
		return Found->WorldRecordDistance;
	}
	return 0.0f;
}

void UMGAirtimeSubsystem::SetWorldRecord(const FString& RampId, float Distance, const FString& PlayerName)
{
	FMGRampRecord* Record = RampRecords.Find(RampId);
	if (!Record)
	{
		FMGRampRecord NewRecord;
		NewRecord.RampId = RampId;
		NewRecord.WorldRecordDistance = Distance;
		NewRecord.WorldRecordHolder = PlayerName;
		RampRecords.Add(RampId, NewRecord);
	}
	else
	{
		Record->WorldRecordDistance = Distance;
		Record->WorldRecordHolder = PlayerName;
	}
}

// Bonuses
void UMGAirtimeSubsystem::RegisterNearMissWhileAirborne(const FString& PlayerId)
{
	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump || !Jump->bIsAirborne)
	{
		return;
	}

	Jump->bNearMissWhileAirborne = true;
	Jump->NearMissCount++;
	Jump->CurrentMultiplier *= ScoringConfig.NearMissWhileAirborneBonus;

	OnNearMissWhileAirborne.Broadcast(PlayerId, ScoringConfig.NearMissWhileAirborneBonus);
}

void UMGAirtimeSubsystem::ApplySpeedBonus(const FString& PlayerId, float SpeedMPH)
{
	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump || !Jump->bIsAirborne)
	{
		return;
	}

	if (SpeedMPH >= ScoringConfig.SpeedBonusThreshold)
	{
		Jump->CurrentMultiplier *= ScoringConfig.SpeedBonusMultiplier;
	}
}

// Stats
FMGAirtimePlayerStats UMGAirtimeSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGAirtimePlayerStats* Found = PlayerStats.Find(PlayerId))
	{
		return *Found;
	}
	return FMGAirtimePlayerStats();
}

void UMGAirtimeSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGAirtimePlayerStats NewStats;
	NewStats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, NewStats);
}

// Discovery
void UMGAirtimeSubsystem::DiscoverSecretRamp(const FString& PlayerId, const FString& RampId)
{
	if (DiscoveredRamps.Contains(RampId))
	{
		return;
	}

	DiscoveredRamps.Add(RampId);

	// Update player stats
	FMGAirtimePlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGAirtimePlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		NewStats.SecretRampsFound = 1;
		PlayerStats.Add(PlayerId, NewStats);
	}
	else
	{
		Stats->SecretRampsFound++;
	}

	OnSecretRampFound.Broadcast(PlayerId, RampId);
}

bool UMGAirtimeSubsystem::IsRampDiscovered(const FString& RampId) const
{
	return DiscoveredRamps.Contains(RampId);
}

TArray<FString> UMGAirtimeSubsystem::GetDiscoveredRamps() const
{
	return DiscoveredRamps;
}

// Configuration
void UMGAirtimeSubsystem::SetScoringConfig(const FMGAirtimeScoringConfig& Config)
{
	ScoringConfig = Config;
}

FMGAirtimeScoringConfig UMGAirtimeSubsystem::GetScoringConfig() const
{
	return ScoringConfig;
}

void UMGAirtimeSubsystem::SetLandingConfig(const FMGLandingConfig& Config)
{
	LandingConfig = Config;
}

FMGLandingConfig UMGAirtimeSubsystem::GetLandingConfig() const
{
	return LandingConfig;
}

// Update
void UMGAirtimeSubsystem::UpdateAirtimeSystem(float DeltaTime)
{
	TickAirtime(DeltaTime);
}

// Protected
void UMGAirtimeSubsystem::TickAirtime(float DeltaTime)
{
	UpdateActiveJumps(DeltaTime);
}

void UMGAirtimeSubsystem::UpdateActiveJumps(float DeltaTime)
{
	for (auto& Pair : ActiveJumps)
	{
		if (Pair.Value.bIsAirborne)
		{
			Pair.Value.AirtimeDuration += DeltaTime;
		}
	}
}

void UMGAirtimeSubsystem::UpdateJumpMetrics(FMGActiveJump& Jump, FVector Position, float DeltaTime)
{
	// Update current height
	Jump.CurrentHeight = Position.Z - Jump.LaunchPosition.Z;

	// Update max height
	if (Jump.CurrentHeight > Jump.MaxHeight)
	{
		Jump.MaxHeight = Jump.CurrentHeight;
	}

	// Update horizontal distance
	Jump.HorizontalDistance = FVector::Dist2D(Position, Jump.LaunchPosition);
}

FMGJumpResult UMGAirtimeSubsystem::FinalizeJump(const FString& PlayerId, EMGLandingQuality Landing)
{
	FMGActiveJump* Jump = ActiveJumps.Find(PlayerId);
	if (!Jump)
	{
		return FMGJumpResult();
	}

	FMGJumpResult Result;
	Result.ResultId = GenerateResultId();
	Result.PlayerId = PlayerId;
	Result.RampId = Jump->RampId;
	Result.Type = Jump->Type;
	Result.AirtimeDuration = Jump->AirtimeDuration;
	Result.MaxHeight = Jump->MaxHeight;
	Result.HorizontalDistance = Jump->HorizontalDistance;
	Result.LaunchSpeed = Jump->LaunchSpeed;
	Result.LandingQuality = Landing;
	Result.TricksPerformed = Jump->ActiveTricks;
	Result.TrickCount = Jump->TricksCompleted;
	Result.TotalRotation = FMath::Abs(Jump->TotalRotation.Pitch) + FMath::Abs(Jump->TotalRotation.Yaw) + FMath::Abs(Jump->TotalRotation.Roll);
	Result.Timestamp = FDateTime::Now();

	// Calculate scores
	int32 BaseAirtimeScore = FMath::RoundToInt(Jump->AirtimeDuration * ScoringConfig.PointsPerSecondAirtime);
	int32 HeightScore = FMath::RoundToInt((Jump->MaxHeight / 100.0f) * ScoringConfig.PointsPerMeterHeight);
	int32 DistanceScore = FMath::RoundToInt((Jump->HorizontalDistance / 100.0f) * ScoringConfig.PointsPerMeterDistance);

	Result.BaseScore = BaseAirtimeScore + HeightScore + DistanceScore;
	Result.TrickScore = Jump->CurrentScore;
	Result.LandingBonus = GetLandingBonus(Landing);

	Result.TotalScore = CalculateJumpScore(*Jump, Landing);
	Result.Rating = CalculateRating(Jump->RampId, Jump->HorizontalDistance);

	// Check for personal best
	FMGRampRecord* Record = RampRecords.Find(Jump->RampId);
	if (Record && Jump->HorizontalDistance > Record->PersonalBestDistance)
	{
		Result.bIsPersonalBest = true;
	}

	// Check for world record
	if (Record && Jump->HorizontalDistance > Record->WorldRecordDistance)
	{
		Result.bIsWorldRecord = true;
	}

	return Result;
}

void UMGAirtimeSubsystem::UpdateRecords(const FString& PlayerId, const FMGJumpResult& Result)
{
	if (Result.RampId.IsEmpty())
	{
		return;
	}

	FMGRampRecord* Record = RampRecords.Find(Result.RampId);
	if (!Record)
	{
		FMGRampRecord NewRecord;
		NewRecord.RampId = Result.RampId;
		NewRecord.PersonalBestDistance = Result.HorizontalDistance;
		NewRecord.PersonalBestScore = Result.TotalScore;
		NewRecord.BestRating = Result.Rating;
		NewRecord.TotalAttempts = 1;
		NewRecord.SuccessfulLandings = (Result.LandingQuality != EMGLandingQuality::Crash) ? 1 : 0;
		NewRecord.PersonalBestDate = FDateTime::Now();
		RampRecords.Add(Result.RampId, NewRecord);

		if (Result.bIsPersonalBest)
		{
			OnNewPersonalBest.Broadcast(PlayerId, Result.RampId, Result.HorizontalDistance);
		}
		return;
	}

	Record->TotalAttempts++;
	if (Result.LandingQuality != EMGLandingQuality::Crash)
	{
		Record->SuccessfulLandings++;
	}

	if (Result.HorizontalDistance > Record->PersonalBestDistance)
	{
		Record->PersonalBestDistance = Result.HorizontalDistance;
		Record->PersonalBestScore = Result.TotalScore;
		Record->BestRating = Result.Rating;
		Record->PersonalBestDate = FDateTime::Now();
		OnNewPersonalBest.Broadcast(PlayerId, Result.RampId, Result.HorizontalDistance);
	}
}

void UMGAirtimeSubsystem::UpdatePlayerStats(const FString& PlayerId, const FMGJumpResult& Result)
{
	FMGAirtimePlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGAirtimePlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	Stats->TotalJumps++;
	Stats->TotalAirtime += Result.AirtimeDuration;
	Stats->TotalTricks += Result.TrickCount;
	Stats->TotalPoints += Result.TotalScore;

	if (Result.AirtimeDuration > Stats->LongestAirtime)
	{
		Stats->LongestAirtime = Result.AirtimeDuration;
	}

	if (Result.MaxHeight > Stats->HighestJump)
	{
		Stats->HighestJump = Result.MaxHeight;
	}

	if (Result.HorizontalDistance > Stats->LongestDistance)
	{
		Stats->LongestDistance = Result.HorizontalDistance;
	}

	if (Result.TotalScore > Stats->HighestSingleJumpScore)
	{
		Stats->HighestSingleJumpScore = Result.TotalScore;
	}

	if (Result.LandingQuality == EMGLandingQuality::Perfect)
	{
		Stats->PerfectLandings++;
	}
	else if (Result.LandingQuality == EMGLandingQuality::Crash)
	{
		Stats->CrashLandings++;
	}

	// Update trick counts
	for (EMGAirtimeTrick Trick : Result.TricksPerformed)
	{
		int32& TrickCount = Stats->TrickCounts.FindOrAdd(Trick);
		TrickCount++;
	}

	// Update rating counts
	int32& RatingCount = Stats->RatingCounts.FindOrAdd(Result.Rating);
	RatingCount++;

	// Update ramp best distances
	if (!Result.RampId.IsEmpty())
	{
		float* BestDistance = Stats->RampBestDistances.Find(Result.RampId);
		if (!BestDistance || Result.HorizontalDistance > *BestDistance)
		{
			Stats->RampBestDistances.Add(Result.RampId, Result.HorizontalDistance);
		}
	}
}

bool UMGAirtimeSubsystem::IsInRampTrigger(FVector Position, const FMGRampDefinition& Ramp) const
{
	// Transform position to ramp local space
	FVector LocalPos = Ramp.Rotation.UnrotateVector(Position - Ramp.Location);

	// Check if within trigger bounds
	bool bInWidth = FMath::Abs(LocalPos.Y) <= Ramp.TriggerWidth * 0.5f;
	bool bInLength = LocalPos.X >= 0.0f && LocalPos.X <= Ramp.TriggerLength;
	bool bInHeight = LocalPos.Z >= -100.0f && LocalPos.Z <= 200.0f;

	return bInWidth && bInLength && bInHeight;
}

FString UMGAirtimeSubsystem::GenerateJumpId() const
{
	return FString::Printf(TEXT("JUMP_%d_%lld"), ++const_cast<UMGAirtimeSubsystem*>(this)->JumpCounter, FDateTime::Now().GetTicks());
}

FString UMGAirtimeSubsystem::GenerateResultId() const
{
	return FString::Printf(TEXT("RESULT_%d_%lld"), ++const_cast<UMGAirtimeSubsystem*>(this)->ResultCounter, FDateTime::Now().GetTicks());
}

// Save/Load
void UMGAirtimeSubsystem::SaveAirtimeData()
{
	// Save implementation would persist records, stats, and discovered ramps
	// Using platform-specific save system or cloud saves
}

void UMGAirtimeSubsystem::LoadAirtimeData()
{
	// Load implementation would restore records, stats, and discovered ramps
	// From platform-specific save system or cloud saves
}
