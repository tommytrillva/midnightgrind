// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGAIRacerController.cpp
 * @brief Implementation of AI racing intelligence
 *
 * This implementation follows the GDD design pillar "Unified Challenge":
 * - AI uses the same physics as players
 * - No rubber-banding speed boosts
 * - Difficulty comes from decision quality, not physics cheats
 * - Skill-based catch-up through risk-taking and optimization
 */

#include "AI/MGAIRacerController.h"
#include "AI/MG_AI_DriverProfile.h"
#include "Track/MGTrackSubsystem.h"
#include "GameModes/MGRaceGameMode.h"
#include "Weather/MGWeatherSubsystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleDamageSystem.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ==========================================
// CONSTANTS
// ==========================================

namespace AIConstants
{
	/** Conversion from meters to UE units (cm) */
	constexpr float MetersToUnits = 100.0f;

	/** Minimum speed to consider for calculations (cm/s) */
	constexpr float MinCalculationSpeed = 100.0f;

	/** Default braking deceleration (m/s^2) */
	constexpr float DefaultBrakingDecel = 12.0f;

	/** Slipstream speed bonus percentage */
	constexpr float MaxSlipstreamBonus = 0.05f; // 5% max speed increase

	/** Time gap considered "close" for racing decisions (seconds) */
	constexpr float CloseGapThreshold = 1.5f;

	/** Large gap threshold for mode changes (seconds) */
	constexpr float LargeGapThreshold = 5.0f;
}

// ==========================================
// CONSTRUCTOR & LIFECYCLE
// ==========================================

AMGAIRacerController::AMGAIRacerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

void AMGAIRacerController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		TrackSubsystem = World->GetSubsystem<UMGTrackSubsystem>();
		RaceGameMode = Cast<AMGRaceGameMode>(UGameplayStatics::GetGameMode(World));
	}
}

void AMGAIRacerController::Tick(float MGDeltaTime)
{
	Super::Tick(DeltaTime);

	// Skip update if not in valid racing state
	if (!VehiclePawn || CurrentState == EMGAIDrivingState::Waiting || CurrentState == EMGAIDrivingState::Finished)
	{
		return;
	}

	// Core update sequence
	UpdatePerception();
	UpdateRacingLineProgress();
	UpdateTactics(DeltaTime);
	UpdateMoodAndLearning(DeltaTime); // NEW: Adaptive AI - mood and learning systems
	UpdateStateMachine(DeltaTime);
	CalculateSteering(DeltaTime);
	ApplySteering();

	TimeInState += DeltaTime;
}

void AMGAIRacerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	VehiclePawn = InPawn;
}

void AMGAIRacerController::OnUnPossess()
{
	VehiclePawn = nullptr;
	Super::OnUnPossess();
}

// ==========================================
// CONFIGURATION
// ==========================================

void AMGAIRacerController::SetDriverProfile(UMGAIDriverProfile* Profile)
{
	DriverProfile = Profile;

	// Apply profile-specific configuration
	if (DriverProfile)
	{
		OvertakeThreshold = DriverProfile->Aggression.OvertakeAggression;
		MinFollowingGap = FMath::Lerp(1.5f, 0.5f, DriverProfile->Aggression.ProximityTolerance);
	}
}

void AMGAIRacerController::SetDifficultyMultiplier(float Multiplier)
{
	DifficultyMultiplier = FMath::Clamp(Multiplier, 0.5f, 1.5f);
}

void AMGAIRacerController::SetSkillBasedCatchUpEnabled(bool bEnabled)
{
	bSkillBasedCatchUpEnabled = bEnabled;
}

void AMGAIRacerController::SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine)
{
	RacingLinePoints = RacingLine;
	CurrentRacingLineIndex = 0;

	// Calculate total racing line length
	TotalRacingLineLength = 0.0f;
	for (int32 i = 0; i < RacingLinePoints.Num(); ++i)
	{
		int32 NextIndex = (i + 1) % RacingLinePoints.Num();
		TotalRacingLineLength += FVector::Dist(
			RacingLinePoints[i].Position,
			RacingLinePoints[NextIndex].Position
		);
	}
}

void AMGAIRacerController::SetOvertakeAggression(float Aggression)
{
	OvertakeThreshold = FMath::Clamp(Aggression, 0.0f, 1.0f);
}

// ==========================================
// STATE QUERIES
// ==========================================

float AMGAIRacerController::GetCurrentSpeed() const
{
	if (VehiclePawn)
	{
		return VehiclePawn->GetVelocity().Size();
	}
	return 0.0f;
}

float AMGAIRacerController::GetDistanceToRacingLine() const
{
	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return 0.0f;
	}

	FVector CurrentPos = VehiclePawn->GetActorLocation();
	int32 ClosestIndex = FindClosestRacingLinePoint(CurrentPos);

	if (ClosestIndex >= 0 && ClosestIndex < RacingLinePoints.Num())
	{
		return FVector::Dist(CurrentPos, RacingLinePoints[ClosestIndex].Position);
	}

	return 0.0f;
}

// ==========================================
// RACE CONTROL
// ==========================================

void AMGAIRacerController::StartRacing()
{
	SetState(EMGAIDrivingState::Racing);
	TacticalData = FMGAITacticalData();
}

void AMGAIRacerController::StopRacing()
{
	SetState(EMGAIDrivingState::Finished);
	CurrentSteering = FMGAISteeringOutput();
	CurrentSteering.Brake = 1.0f;
}

void AMGAIRacerController::ForceState(EMGAIDrivingState NewState)
{
	SetState(NewState);
}

void AMGAIRacerController::NotifyCollision(AActor* OtherActor, const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	// Enter recovery if collision is significant
	if (CurrentState == EMGAIDrivingState::Racing ||
		CurrentState == EMGAIDrivingState::Overtaking ||
		CurrentState == EMGAIDrivingState::Defending)
	{
		// Calculate impact severity based on velocity
		if (VehiclePawn)
		{
			FVector Velocity = VehiclePawn->GetVelocity();
			float ImpactSeverity = FMath::Abs(FVector::DotProduct(Velocity, ImpactNormal));
			float NormalizedSeverity = FMath::Clamp(ImpactSeverity / 2000.0f, 0.0f, 1.0f);

			// Record contact in driver profile for aggression escalation
			if (DriverProfile && OtherActor)
			{
				APawn* OtherPawn = Cast<APawn>(OtherActor);
				bool bWasPlayer = OtherPawn && Cast<APlayerController>(OtherPawn->GetController()) != nullptr;

				// Determine if contact seemed intentional
				bool bSeemedIntentional = false;
				if (OtherPawn)
				{
					FVector TheirVelocity = OtherPawn->GetVelocity();
					FVector ToUs = VehiclePawn->GetActorLocation() - OtherPawn->GetActorLocation();
					float DotTowardUs = FVector::DotProduct(TheirVelocity.GetSafeNormal(), ToUs.GetSafeNormal());
					bSeemedIntentional = DotTowardUs > 0.5f && TheirVelocity.Size() > 500.0f;
				}

				DriverProfile->RecordContact(OtherActor, NormalizedSeverity, bWasPlayer, bSeemedIntentional);
				HandleContactResponse(DriverProfile->GetContactResponse(NormalizedSeverity), OtherActor, NormalizedSeverity);
			}

			// Only enter recovery for significant impacts
			if (ImpactSeverity > 500.0f)
			{
				// Check state BEFORE calling SetState, as SetState modifies CurrentState
				bool bWasOvertaking = (CurrentState == EMGAIDrivingState::Overtaking);

				SetState(EMGAIDrivingState::Recovering);
				RecoveryTimer = FMath::Lerp(1.0f, 3.0f, NormalizedSeverity);

				if (bWasOvertaking)
				{
					OvertakeTimer = 0.0f;
				}
			}
		}
	}
}

void AMGAIRacerController::NotifyOffTrack()
{
	if (CurrentState != EMGAIDrivingState::Recovering)
	{
		SetState(EMGAIDrivingState::Recovering);

		// Recovery time based on skill
		float BaseRecovery = 2.0f;
		if (DriverProfile)
		{
			BaseRecovery *= (2.0f - DriverProfile->Skill.RecoverySkill);
		}
		RecoveryTimer = BaseRecovery;
	}
}

void AMGAIRacerController::UpdateRacePosition(int32 Position, int32 TotalRacers, float InGapToLeader, float InGapToAhead)
{
	CurrentRacePosition = Position;
	TotalRacersInRace = TotalRacers;
	GapToLeader = InGapToLeader;
	GapToVehicleAhead = InGapToAhead;
}

// ==========================================
// CORE UPDATE METHODS
// ==========================================

void AMGAIRacerController::UpdatePerception()
{
	PerceivedVehicles.Empty();

	if (!VehiclePawn)
	{
		return;
	}

	FVector MyLocation = VehiclePawn->GetActorLocation();
	FVector MyVelocity = VehiclePawn->GetVelocity();
	FVector MyForward = VehiclePawn->GetActorForwardVector();
	FVector MyRight = VehiclePawn->GetActorRightVector();
	float MySpeed = MyVelocity.Size();

	// Find all vehicles in perception radius
	TArray<AActor*> NearbyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

	for (AActor* Actor : NearbyActors)
	{
		if (Actor == VehiclePawn)
		{
			continue;
		}

		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn)
		{
			continue;
		}

		FVector OtherLocation = OtherPawn->GetActorLocation();
		float Distance = FVector::Dist(MyLocation, OtherLocation);

		// Skip if outside perception radius
		if (Distance > PerceptionRadius * AIConstants::MetersToUnits)
		{
			continue;
		}

		FMGAIVehiclePerception Perception;
		Perception.Vehicle = OtherPawn;
		Perception.RelativePosition = OtherLocation - MyLocation;
		Perception.RelativeVelocity = OtherPawn->GetVelocity() - MyVelocity;
		Perception.Distance = Distance;

		// Calculate angle to other vehicle
		FVector ToOther = Perception.RelativePosition.GetSafeNormal();
		float DotForward = FVector::DotProduct(MyForward, ToOther);
		float DotRight = FVector::DotProduct(MyRight, ToOther);
		Perception.Angle = FMath::RadiansToDegrees(FMath::Atan2(DotRight, DotForward));

		// Determine relative position
		Perception.bIsAhead = DotForward > 0.0f;
		Perception.bIsOnLeft = DotRight < 0.0f;

		// Calculate speed difference
		float OtherSpeed = OtherPawn->GetVelocity().Size();
		Perception.SpeedDifference = MySpeed - OtherSpeed;

		// Calculate time to collision
		float ClosingSpeed = -FVector::DotProduct(Perception.RelativeVelocity, ToOther);
		if (ClosingSpeed > AIConstants::MinCalculationSpeed)
		{
			Perception.TimeToCollision = Distance / ClosingSpeed;
		}
		else
		{
			Perception.TimeToCollision = FLT_MAX;
		}

		// Check if player
		APlayerController* PC = Cast<APlayerController>(OtherPawn->GetController());
		Perception.bIsPlayer = PC != nullptr;

		// Check if in slipstream range
		Perception.bInSlipstreamRange = IsInSlipstream(OtherPawn);

		// Estimate skill based on observed driving
		// (In a full implementation, this would track their line accuracy, braking points, etc.)
		Perception.EstimatedSkill = 0.5f;

		PerceivedVehicles.Add(Perception);
	}

	// Sort by distance for efficient processing
	PerceivedVehicles.Sort([](const FMGAIVehiclePerception& A, const FMGAIVehiclePerception& B)
	{
		return A.Distance < B.Distance;
	});
}

void AMGAIRacerController::UpdateRacingLineProgress()
{
	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return;
	}

	FVector CurrentPos = VehiclePawn->GetActorLocation();
	int32 ClosestIndex = FindClosestRacingLinePoint(CurrentPos);

	// Only allow forward progress (prevents going backwards on track)
	if (ClosestIndex > CurrentRacingLineIndex)
	{
		CurrentRacingLineIndex = ClosestIndex;
	}
	// Handle lap wrap-around
	else if (CurrentRacingLineIndex > RacingLinePoints.Num() - 10 && ClosestIndex < 10)
	{
		CurrentRacingLineIndex = ClosestIndex;
	}

	// Calculate normalized progress
	if (TotalRacingLineLength > 0.0f)
	{
		float DistanceProgress = 0.0f;
		for (int32 i = 0; i < CurrentRacingLineIndex; ++i)
		{
			int32 NextIndex = (i + 1) % RacingLinePoints.Num();
			DistanceProgress += FVector::Dist(
				RacingLinePoints[i].Position,
				RacingLinePoints[NextIndex].Position
			);
		}
		RacingLineProgress = DistanceProgress / TotalRacingLineLength;
	}
}

void AMGAIRacerController::UpdateTactics(float MGDeltaTime)
{
	// Update time following
	FMGAIVehiclePerception VehicleAhead = GetVehicleAhead();
	if (VehicleAhead.Vehicle && VehicleAhead.Distance < 20.0f * AIConstants::MetersToUnits)
	{
		TacticalData.TimeFollowing += DeltaTime;
		TacticalData.TacticalTarget = VehicleAhead.Vehicle;
	}
	else
	{
		TacticalData.TimeFollowing = 0.0f;
		TacticalData.TacticalTarget = nullptr;
	}

	// Update catch-up mode based on position and skill-based system
	if (bSkillBasedCatchUpEnabled)
	{
		TacticalData.CatchUpMode = DetermineCatchUpMode();
	}
	else
	{
		TacticalData.CatchUpMode = EMGAICatchUpBehavior::None;
	}

	// Update risk level based on situation
	TacticalData.CurrentRiskLevel = GetSituationalRiskLevel();

	// Find distance to next overtaking zone
	TacticalData.DistanceToOvertakeZone = FLT_MAX;
	for (int32 i = CurrentRacingLineIndex; i < FMath::Min(CurrentRacingLineIndex + 50, RacingLinePoints.Num()); ++i)
	{
		int32 Index = i % RacingLinePoints.Num();
		if (RacingLinePoints[Index].bIsOvertakingZone)
		{
			TacticalData.DistanceToOvertakeZone = RacingLinePoints[Index].DistanceAlongTrack -
				RacingLinePoints[CurrentRacingLineIndex].DistanceAlongTrack;
			break;
		}
	}

	// Update slipstream status
	TacticalData.bInSlipstream = false;
	TacticalData.SlipstreamBonus = 0.0f;
	for (const FMGAIVehiclePerception& Perceived : PerceivedVehicles)
	{
		if (Perceived.bInSlipstreamRange && Perceived.bIsAhead)
		{
			TacticalData.bInSlipstream = true;
			TacticalData.SlipstreamBonus = CalculateSlipstreamBonus(
				VehiclePawn->GetActorLocation(),
				VehiclePawn->GetVelocity()
			);
			break;
		}
	}

	// Simulate tire wear (affects grip/confidence at high difficulty)
	if (DifficultyMultiplier > 1.0f)
	{
		float WearRate = 0.001f * DifficultyMultiplier;
		if (CurrentState == EMGAIDrivingState::PushingHard)
		{
			WearRate *= 2.0f;
		}
		TacticalData.SimulatedTireWear = FMath::Min(1.0f, TacticalData.SimulatedTireWear + WearRate * DeltaTime);
	}

	// Update aggression state based on racing situation
	if (DriverProfile)
	{
		bool bUnderPressure = false;
		bool bApplyingPressure = false;

		// Check if we're under pressure (someone close behind)
		FMGAIVehiclePerception Behind = GetVehicleBehind();
		if (Behind.Vehicle && Behind.Distance < 15.0f * AIConstants::MetersToUnits)
		{
			bUnderPressure = true;
		}

		// Check if we're applying pressure (close to someone ahead)
		FMGAIVehiclePerception Ahead = GetVehicleAhead();
		if (Ahead.Vehicle && Ahead.Distance < 15.0f * AIConstants::MetersToUnits)
		{
			bApplyingPressure = true;
		}

		DriverProfile->UpdateAggressionState(DeltaTime, CurrentRacePosition, bUnderPressure, bApplyingPressure);
	}
}

void AMGAIRacerController::UpdateStateMachine(float MGDeltaTime)
{
	switch (CurrentState)
	{
		case EMGAIDrivingState::Racing:
			HandleRacingState(DeltaTime);
			break;

		case EMGAIDrivingState::Overtaking:
			HandleOvertakingState(DeltaTime);
			break;

		case EMGAIDrivingState::Defending:
			HandleDefendingState(DeltaTime);
			break;

		case EMGAIDrivingState::Recovering:
			HandleRecoveringState(DeltaTime);
			break;

		case EMGAIDrivingState::PushingHard:
			HandlePushingHardState(DeltaTime);
			break;

		case EMGAIDrivingState::ManagingLead:
			HandleManagingLeadState(DeltaTime);
			break;

		case EMGAIDrivingState::Drafting:
			HandleDraftingState(DeltaTime);
			break;

		default:
			break;
	}
}

void AMGAIRacerController::CalculateSteering(float MGDeltaTime)
{
	switch (CurrentState)
	{
		case EMGAIDrivingState::Racing:
		case EMGAIDrivingState::PushingHard:
		case EMGAIDrivingState::ManagingLead:
			CurrentSteering = CalculateRacingLineSteering();
			break;

		case EMGAIDrivingState::Overtaking:
			CurrentSteering = CalculateOvertakeSteering();
			break;

		case EMGAIDrivingState::Defending:
			CurrentSteering = CalculateDefenseSteering();
			break;

		case EMGAIDrivingState::Recovering:
			CurrentSteering = CalculateRecoverySteering();
			break;

		case EMGAIDrivingState::Drafting:
			CurrentSteering = CalculateDraftingSteering();
			break;

		default:
			CurrentSteering = FMGAISteeringOutput();
			break;
	}

	// Calculate and apply target speed
	CurrentTargetSpeed = CalculateTargetSpeed();

	// Apply skill-based adjustments (NOT rubber banding - just risk level changes)
	if (bSkillBasedCatchUpEnabled)
	{
		CurrentTargetSpeed *= (1.0f + CalculateSkillBasedAdjustment());
	}

	// Apply driver profile modifiers
	if (DriverProfile)
	{
		ApplyProfileModifiers(CurrentSteering);
		ApplyAggressionModifiers(CurrentSteering);
	}

	// Apply slipstream bonus (this is physics-valid - real drafting effect)
	if (TacticalData.bInSlipstream)
	{
		// Slipstream reduces air resistance, allowing higher speed at same throttle
		// This is NOT cheating - it's real physics
		CurrentTargetSpeed *= (1.0f + TacticalData.SlipstreamBonus);
	}
}

void AMGAIRacerController::ApplySteering()
{
	if (!VehiclePawn)
	{
		return;
	}

	// The actual application to the vehicle is handled by the vehicle reading our outputs
	// This maintains the Unified Challenge principle - same physics for all
}

// ==========================================
// STATE HANDLERS
// ==========================================

void AMGAIRacerController::HandleWaitingState(float MGDeltaTime)
{
	// Wait for race start signal
	CurrentSteering = FMGAISteeringOutput();
}

void AMGAIRacerController::HandleRacingState(float MGDeltaTime)
{
	// Check for state transitions

	// Should we start drafting?
	if (ShouldStartDrafting())
	{
		SetState(EMGAIDrivingState::Drafting);
		return;
	}

	// Should we attempt an overtake?
	if (ShouldAttemptOvertake())
	{
		FMGAIVehiclePerception Ahead = GetVehicleAhead();
		if (Ahead.Vehicle)
		{
			// Choose which side to overtake on
			bool bLeftClear = IsOvertakePathClear(true);
			bool bRightClear = IsOvertakePathClear(false);

			if (bLeftClear || bRightClear)
			{
				bOvertakeOnLeft = bLeftClear && (!bRightClear || FMath::RandBool());
				TacticalData.OvertakeStrategy = ChooseOvertakeStrategy(Ahead);
				TacticalData.TacticalTarget = Ahead.Vehicle;
				SetState(EMGAIDrivingState::Overtaking);
				OvertakeTimer = 0.0f;
				return;
			}
		}
	}

	// Should we defend position?
	if (ShouldDefendPosition())
	{
		FMGAIVehiclePerception Behind = GetVehicleBehind();
		if (Behind.Vehicle)
		{
			TacticalData.DefenseStrategy = ChooseDefenseStrategy(Behind);
			TacticalData.TacticalTarget = Behind.Vehicle;
			SetState(EMGAIDrivingState::Defending);
			return;
		}
	}

	// Check for skill-based mode changes (not rubber banding!)
	if (bSkillBasedCatchUpEnabled)
	{
		EMGAICatchUpBehavior NewMode = DetermineCatchUpMode();
		if (NewMode == EMGAICatchUpBehavior::MaxEffort || NewMode == EMGAICatchUpBehavior::RiskTaking)
		{
			SetState(EMGAIDrivingState::PushingHard);
			return;
		}
		else if (NewMode == EMGAICatchUpBehavior::Conservation && CurrentRacePosition == 1)
		{
			SetState(EMGAIDrivingState::ManagingLead);
			return;
		}
	}
}

void AMGAIRacerController::HandleOvertakingState(float MGDeltaTime)
{
	OvertakeTimer += DeltaTime;

	// Check if overtake is complete (we passed them)
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (!Ahead.Vehicle || Ahead.Vehicle != TacticalData.TacticalTarget)
	{
		// We've passed the target or they're no longer ahead
		OnOvertakeComplete.Broadcast(TacticalData.TacticalTarget, TacticalData.OvertakeStrategy);
		TacticalData.TacticalTarget = nullptr;
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Check timeout based on profile patience
	float MaxTime = MaxOvertakeTime;
	if (DriverProfile)
	{
		MaxTime = DriverProfile->Aggression.OvertakePatience * 2.0f;
	}

	if (OvertakeTimer > MaxTime)
	{
		// Abort overtake attempt
		TacticalData.TacticalTarget = nullptr;
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Check if path is no longer clear (someone moved into our line)
	if (!IsOvertakePathClear(bOvertakeOnLeft))
	{
		// Try the other side
		bool bOtherSideClear = IsOvertakePathClear(!bOvertakeOnLeft);
		if (bOtherSideClear)
		{
			bOvertakeOnLeft = !bOvertakeOnLeft;
		}
		else
		{
			// Both sides blocked, abort
			TacticalData.TacticalTarget = nullptr;
			SetState(EMGAIDrivingState::Racing);
		}
	}
}

void AMGAIRacerController::HandleDefendingState(float MGDeltaTime)
{
	// Check if we still need to defend
	FMGAIVehiclePerception Behind = GetVehicleBehind();

	// Stop defending if attacker is no longer close
	if (!Behind.Vehicle || Behind.Distance > 25.0f * AIConstants::MetersToUnits)
	{
		TacticalData.TacticalTarget = nullptr;
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Stop defending after a reasonable time (prevents excessive blocking)
	float MaxDefendTime = 10.0f;
	if (DriverProfile)
	{
		MaxDefendTime = 5.0f + DriverProfile->Aggression.DefenseAggression * 10.0f;
	}

	if (TimeInState > MaxDefendTime)
	{
		TacticalData.TacticalTarget = nullptr;
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Check if we got overtaken
	if (Behind.Vehicle == TacticalData.TacticalTarget)
	{
		// Check their position relative to us
		if (Behind.bIsAhead)
		{
			// They passed us
			OnWasOvertaken.Broadcast(Behind.Vehicle);
			TacticalData.TacticalTarget = nullptr;
			SetState(EMGAIDrivingState::Racing);
		}
	}
}

void AMGAIRacerController::HandleRecoveringState(float MGDeltaTime)
{
	RecoveryTimer -= DeltaTime;

	if (RecoveryTimer <= 0.0f)
	{
		// Recovery complete
		SetState(EMGAIDrivingState::Racing);
	}
}

void AMGAIRacerController::HandlePushingHardState(float MGDeltaTime)
{
	// In this state, we're taking more risks to catch up
	// This is skill-based - we brake later and get on throttle earlier
	// NOT rubber banding - no speed advantage, just optimized driving

	// Check if we should drop back to normal racing
	EMGAICatchUpBehavior CurrentMode = DetermineCatchUpMode();
	if (CurrentMode != EMGAICatchUpBehavior::MaxEffort && CurrentMode != EMGAICatchUpBehavior::RiskTaking)
	{
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Check for overtake opportunity while pushing hard
	if (ShouldAttemptOvertake())
	{
		FMGAIVehiclePerception Ahead = GetVehicleAhead();
		if (Ahead.Vehicle && (IsOvertakePathClear(true) || IsOvertakePathClear(false)))
		{
			bOvertakeOnLeft = IsOvertakePathClear(true);
			TacticalData.OvertakeStrategy = ChooseOvertakeStrategy(Ahead);
			TacticalData.TacticalTarget = Ahead.Vehicle;
			SetState(EMGAIDrivingState::Overtaking);
			OvertakeTimer = 0.0f;
		}
	}
}

void AMGAIRacerController::HandleManagingLeadState(float MGDeltaTime)
{
	// Conservative driving when in the lead
	// Not slowing down artificially - just not taking unnecessary risks

	EMGAICatchUpBehavior CurrentMode = DetermineCatchUpMode();

	// If gap to second place shrinks, go back to normal racing
	if (CurrentMode != EMGAICatchUpBehavior::Conservation || GapToVehicleAhead < AIConstants::CloseGapThreshold)
	{
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// If someone is very close behind, might need to defend
	if (ShouldDefendPosition())
	{
		FMGAIVehiclePerception Behind = GetVehicleBehind();
		if (Behind.Vehicle)
		{
			TacticalData.DefenseStrategy = ChooseDefenseStrategy(Behind);
			TacticalData.TacticalTarget = Behind.Vehicle;
			SetState(EMGAIDrivingState::Defending);
		}
	}
}

void AMGAIRacerController::HandleDraftingState(float MGDeltaTime)
{
	// Stay in draft of lead vehicle until ready to pass

	// Check if still in valid drafting position
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (!Ahead.Vehicle || !Ahead.bInSlipstreamRange)
	{
		SetState(EMGAIDrivingState::Racing);
		return;
	}

	// Check if we should slingshot out
	bool bNearOvertakeZone = TacticalData.DistanceToOvertakeZone < 50.0f * AIConstants::MetersToUnits;
	bool bGoodSpeedAdvantage = Ahead.SpeedDifference > 100.0f; // We're faster

	if (bNearOvertakeZone && bGoodSpeedAdvantage)
	{
		// Execute slipstream pass
		TacticalData.OvertakeStrategy = EMGOvertakeStrategy::SlipstreamPass;
		TacticalData.TacticalTarget = Ahead.Vehicle;
		bOvertakeOnLeft = IsOvertakePathClear(true);
		SetState(EMGAIDrivingState::Overtaking);
		OvertakeTimer = 0.0f;
	}
}

// ==========================================
// STEERING CALCULATIONS
// ==========================================

FMGAISteeringOutput AMGAIRacerController::CalculateRacingLineSteering()
{
	FMGAISteeringOutput Output;

	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return Output;
	}

	// Get target point on racing line
	FMGAIRacingLinePoint TargetPoint = GetRacingLinePointAhead(SteeringLookAhead);
	Output.TargetPoint = TargetPoint.Position;

	// Add collision avoidance offset
	FVector AvoidanceOffset = CalculateAvoidanceOffset();
	Output.TargetPoint += AvoidanceOffset;

	// Calculate steering using PID controller
	FVector ToTarget = Output.TargetPoint - VehiclePawn->GetActorLocation();
	ToTarget.Z = 0;
	ToTarget.Normalize();

	FVector Forward = VehiclePawn->GetActorForwardVector();
	Forward.Z = 0;
	Forward.Normalize();

	float DotRight = FVector::DotProduct(VehiclePawn->GetActorRightVector(), ToTarget);
	float DotForward = FMath::Max(FVector::DotProduct(Forward, ToTarget), 0.1f);

	// PID steering calculation
	float SteeringError = FMath::Atan2(DotRight, DotForward);
	UWorld* World = GetWorld();
	float MGDeltaTime = World ? World->GetDeltaSeconds() : 0.016f;

	SteeringErrorIntegral += SteeringError * DeltaTime;
	SteeringErrorIntegral = FMath::Clamp(SteeringErrorIntegral, -1.0f, 1.0f);

	float SteeringDerivative = (SteeringError - LastSteeringError) / FMath::Max(DeltaTime, 0.001f);
	LastSteeringError = SteeringError;

	Output.Steering = SteeringPGain * SteeringError +
					  SteeringIGain * SteeringErrorIntegral +
					  SteeringDGain * SteeringDerivative;
	Output.Steering = FMath::Clamp(Output.Steering, -1.0f, 1.0f);

	// Add personality-based noise
	Output.Steering = AddSteeringNoise(Output.Steering);

	// Calculate throttle and brake
	float CurrentSpeed = GetCurrentSpeed();
	float SpeedDiff = CurrentTargetSpeed - CurrentSpeed;

	// Throttle calculation
	if (SpeedDiff > 50.0f)
	{
		Output.Throttle = 1.0f;
		Output.Brake = 0.0f;
	}
	else if (SpeedDiff < -100.0f)
	{
		Output.Throttle = 0.0f;
		Output.Brake = FMath::Clamp(-SpeedDiff / 500.0f, 0.0f, 1.0f);
	}
	else
	{
		Output.Throttle = FMath::Clamp(SpeedDiff / 200.0f + 0.5f, 0.0f, 1.0f);
		Output.Brake = 0.0f;
	}

	// Apply braking for upcoming corners
	FMGAIRacingLinePoint AheadPoint = GetRacingLinePointAhead(SpeedLookAhead);
	if (AheadPoint.bIsBrakingZone)
	{
		float RequiredSpeed = AheadPoint.TargetSpeed * AIConstants::MetersToUnits;
		if (CurrentSpeed > RequiredSpeed)
		{
			float BrakingDistance = CalculateBrakingDistance(CurrentSpeed, RequiredSpeed);
			float DistanceToPoint = FVector::Dist(VehiclePawn->GetActorLocation(), AheadPoint.Position);

			if (DistanceToPoint <= BrakingDistance * 1.2f) // 20% safety margin
			{
				Output.Throttle = 0.0f;
				float BrakeIntensity = FMath::Clamp(BrakingDistance / DistanceToPoint, 0.3f, 1.0f);
				Output.Brake = BrakeIntensity;
			}
		}
	}

	// NOS usage decision
	if (DriverProfile && AheadPoint.bIsAccelerationZone && !AheadPoint.bIsBrakingZone)
	{
		float NOSChance = DriverProfile->Speed.NOSUsageFrequency * 0.01f;
		if (FMath::FRand() < NOSChance)
		{
			Output.bNOS = true;
		}
	}

	// Set gear and confidence
	Output.DesiredGear = TargetPoint.OptimalGear;
	Output.Confidence = 1.0f - TacticalData.SimulatedTireWear * 0.3f;

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateOvertakeSteering()
{
	// Start with racing line steering as base
	FMGAISteeringOutput Output = CalculateRacingLineSteering();

	if (!VehiclePawn)
	{
		return Output;
	}

	// Calculate lateral offset for overtake
	float OvertakeOffset = 0.0f;
	FMGAIRacingLinePoint CurrentPoint = GetRacingLinePointAhead(0.0f);

	switch (TacticalData.OvertakeStrategy)
	{
		case EMGOvertakeStrategy::Patient:
			// Small offset, wait for opportunity
			OvertakeOffset = (bOvertakeOnLeft ? -1.0f : 1.0f) * CurrentPoint.TrackWidth * 0.3f;
			break;

		case EMGOvertakeStrategy::LateBraking:
			// Dive inside at braking zone
			OvertakeOffset = (bOvertakeOnLeft ? -1.0f : 1.0f) * CurrentPoint.TrackWidth * 0.4f;
			// Later braking point (more risk)
			if (DriverProfile)
			{
				float RiskFactor = DriverProfile->Aggression.RiskTaking;
				// This makes the AI brake later - NOT faster, just later
				// Same physics, different decision
			}
			break;

		case EMGOvertakeStrategy::BetterExit:
			// Focus on corner exit, smaller offset during corner
			OvertakeOffset = (bOvertakeOnLeft ? -1.0f : 1.0f) * CurrentPoint.TrackWidth * 0.25f;
			// More aggressive throttle on exit
			if (CurrentPoint.bIsAccelerationZone)
			{
				Output.Throttle = FMath::Min(Output.Throttle + 0.15f, 1.0f);
			}
			break;

		case EMGOvertakeStrategy::AroundOutside:
			// Take outside line
			OvertakeOffset = (bOvertakeOnLeft ? -1.0f : 1.0f) * CurrentPoint.TrackWidth * 0.45f;
			break;

		case EMGOvertakeStrategy::SlipstreamPass:
			// Slight offset to break out of slipstream
			OvertakeOffset = (bOvertakeOnLeft ? -1.0f : 1.0f) * CurrentPoint.TrackWidth * 0.35f;
			// Full throttle with slipstream advantage
			Output.Throttle = 1.0f;
			break;

		case EMGOvertakeStrategy::Pressure:
			// Stay close, look for mistake
			OvertakeOffset = (bOvertakeOnLeft ? -1.0f : 1.0f) * CurrentPoint.TrackWidth * 0.2f;
			break;
	}

	// Apply offset to target point
	FVector RightVector = VehiclePawn->GetActorRightVector();
	Output.TargetPoint += RightVector * OvertakeOffset * AIConstants::MetersToUnits;

	// More aggressive throttle during overtake
	Output.Throttle = FMath::Min(Output.Throttle + 0.1f, 1.0f);

	// Consider NOS usage based on profile aggression
	if (DriverProfile && DriverProfile->Aggression.OvertakeAggression > 0.7f)
	{
		if (FMath::FRand() < 0.1f) // 10% chance per frame to consider NOS
		{
			Output.bNOS = true;
		}
	}

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateDefenseSteering()
{
	FMGAISteeringOutput Output = CalculateRacingLineSteering();

	if (!VehiclePawn)
	{
		return Output;
	}

	FMGAIVehiclePerception Behind = GetVehicleBehind();
	if (!Behind.Vehicle)
	{
		return Output;
	}

	FMGAIRacingLinePoint CurrentPoint = GetRacingLinePointAhead(0.0f);
	float DefenseOffset = 0.0f;

	switch (TacticalData.DefenseStrategy)
	{
		case EMGDefenseStrategy::CoverLine:
			// Stay on racing line - force them around
			// No offset needed
			break;

		case EMGDefenseStrategy::CoverInside:
			// Move to cover inside line
			DefenseOffset = Behind.bIsOnLeft ? -CurrentPoint.TrackWidth * 0.3f : CurrentPoint.TrackWidth * 0.3f;
			break;

		case EMGDefenseStrategy::PaceDefense:
			// Maintain pace, don't let them close
			Output.Throttle = FMath::Min(Output.Throttle + 0.1f, 1.0f);
			break;

		case EMGDefenseStrategy::DefensiveLine:
			// Take defensive line through corner
			if (CurrentPoint.bIsApex || CurrentPoint.bIsBrakingZone)
			{
				DefenseOffset = Behind.bIsOnLeft ? -CurrentPoint.TrackWidth * 0.2f : CurrentPoint.TrackWidth * 0.2f;
			}
			break;
	}

	// Apply defense offset
	FVector RightVector = VehiclePawn->GetActorRightVector();
	Output.TargetPoint += RightVector * DefenseOffset * AIConstants::MetersToUnits;

	// One move rule - can only make one defensive move per straight
	// (Fair racing - no weaving)

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateRecoverySteering()
{
	FMGAISteeringOutput Output;

	if (RacingLinePoints.Num() == 0 || !VehiclePawn)
	{
		return Output;
	}

	// Find closest point on racing line and steer toward it
	int32 ClosestIndex = FindClosestRacingLinePoint(VehiclePawn->GetActorLocation());
	if (ClosestIndex >= 0)
	{
		Output.TargetPoint = RacingLinePoints[ClosestIndex].Position;
	}

	// Calculate steering toward recovery point
	FVector ToTarget = Output.TargetPoint - VehiclePawn->GetActorLocation();
	ToTarget.Z = 0;
	ToTarget.Normalize();

	FVector Forward = VehiclePawn->GetActorForwardVector();
	Forward.Z = 0;
	Forward.Normalize();

	float DotRight = FVector::DotProduct(VehiclePawn->GetActorRightVector(), ToTarget);
	Output.Steering = FMath::Clamp(DotRight * 2.0f, -1.0f, 1.0f);

	// Conservative throttle during recovery
	Output.Throttle = 0.4f;
	Output.Brake = 0.0f;

	// Reduce confidence during recovery
	Output.Confidence = 0.5f;

	return Output;
}

FMGAISteeringOutput AMGAIRacerController::CalculateDraftingSteering()
{
	FMGAISteeringOutput Output = CalculateRacingLineSteering();

	// Adjust target to stay behind lead vehicle in their slipstream
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (Ahead.Vehicle && Ahead.bInSlipstreamRange)
	{
		// Aim for position directly behind lead vehicle
		APawn* LeadPawn = Cast<APawn>(Ahead.Vehicle);
		if (LeadPawn)
		{
			FVector LeadPosition = LeadPawn->GetActorLocation();
			FVector LeadBackward = -LeadPawn->GetActorForwardVector();

			// Target position behind lead vehicle
			Output.TargetPoint = LeadPosition + LeadBackward * 10.0f * AIConstants::MetersToUnits;
		}

		// Full throttle while drafting
		Output.Throttle = 1.0f;
		Output.Brake = 0.0f;
	}

	return Output;
}

FVector AMGAIRacerController::CalculateAvoidanceOffset()
{
	FVector Offset = FVector::ZeroVector;

	if (!VehiclePawn)
	{
		return Offset;
	}

	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		// Only avoid vehicles ahead and close
		if (!Perception.bIsAhead || Perception.Distance > 15.0f * AIConstants::MetersToUnits)
		{
			continue;
		}

		// Skip if this is our overtake/draft target
		if (Perception.Vehicle == TacticalData.TacticalTarget &&
			(CurrentState == EMGAIDrivingState::Overtaking || CurrentState == EMGAIDrivingState::Drafting))
		{
			continue;
		}

		// Calculate avoidance strength based on distance
		float MaxAvoidanceRange = 15.0f * AIConstants::MetersToUnits;
		float AvoidanceStrength = 1.0f - (Perception.Distance / MaxAvoidanceRange);
		AvoidanceStrength = FMath::Square(AvoidanceStrength);

		// Avoid to opposite side
		float AvoidDirection = Perception.bIsOnLeft ? 1.0f : -1.0f;
		Offset += VehiclePawn->GetActorRightVector() * AvoidDirection * AvoidanceStrength * 300.0f;
	}

	return Offset;
}

float AMGAIRacerController::CalculateTargetSpeed()
{
	if (RacingLinePoints.Num() == 0)
	{
		return 50.0f * AIConstants::MetersToUnits; // Default 50 m/s
	}

	// Get speeds from racing line
	FMGAIRacingLinePoint CurrentPoint = GetRacingLinePointAhead(0.0f);
	FMGAIRacingLinePoint AheadPoint = GetRacingLinePointAhead(SpeedLookAhead);

	// Use minimum of current and ahead target speeds
	float BaseSpeed = FMath::Min(CurrentPoint.TargetSpeed, AheadPoint.TargetSpeed);

	// Apply difficulty multiplier (affects skill, not physics)
	// Lower difficulty = more conservative speed choices
	BaseSpeed *= (0.8f + 0.2f * DifficultyMultiplier);

	// Apply profile modifiers
	if (DriverProfile)
	{
		BaseSpeed *= DriverProfile->Speed.BaseSpeedPercent;

		if (CurrentPoint.bIsApex)
		{
			BaseSpeed *= DriverProfile->Speed.CornerSpeedMultiplier;
		}
		else if (!CurrentPoint.bIsBrakingZone && !CurrentPoint.bIsApex)
		{
			BaseSpeed *= DriverProfile->Speed.StraightSpeedMultiplier;
		}
	}

	// Apply tire wear effect (realistic, not cheating)
	if (TacticalData.SimulatedTireWear > 0.3f)
	{
		float WearPenalty = (TacticalData.SimulatedTireWear - 0.3f) * 0.1f;
		BaseSpeed *= (1.0f - WearPenalty);
	}

	// Apply grip level from track surface
	BaseSpeed *= CurrentPoint.GripLevel;

	// Apply weather conditions using unified weather API
	// This combines road grip, aquaplaning, temperature, and precipitation effects
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			// Get unified grip multiplier (includes road condition, aquaplaning, weather intensity)
			const FVector VehicleLocation = VehiclePawn ? VehiclePawn->GetActorLocation() : FVector::ZeroVector;
			const float CurrentSpeedKPH = CurrentSpeed / AIConstants::MetersToUnits * 3.6f;
			const float UnifiedGrip = WeatherSubsystem->GetUnifiedGripMultiplier(VehicleLocation, CurrentSpeedKPH);
			BaseSpeed *= UnifiedGrip;

			// AI perception-based caution (visibility, fog, night combined)
			// Lower perception = more caution needed
			const float AIPerception = WeatherSubsystem->GetUnifiedAIPerceptionMultiplier();
			if (AIPerception < 0.8f)
			{
				// Reduce speed based on perception loss
				// Skill level affects how much caution is taken
				const float PerceptionLoss = 1.0f - AIPerception;
				const float VisibilityCaution = DriverProfile ?
					FMath::Lerp(PerceptionLoss * 0.3f, PerceptionLoss * 0.1f, DriverProfile->Skill.SkillLevel) :
					PerceptionLoss * 0.2f;
				BaseSpeed *= (1.0f - VisibilityCaution);
			}
		}
	}

	return BaseSpeed * AIConstants::MetersToUnits;
}

float AMGAIRacerController::CalculateSkillBasedAdjustment()
{
	// This is NOT rubber banding - no speed boosts
	// Instead, it affects risk tolerance and decision-making
	// Returns a multiplier for how aggressively we pursue our target speed

	if (!bSkillBasedCatchUpEnabled)
	{
		return 0.0f;
	}

	float Adjustment = 0.0f;

	switch (TacticalData.CatchUpMode)
	{
		case EMGAICatchUpBehavior::None:
			Adjustment = 0.0f;
			break;

		case EMGAICatchUpBehavior::RiskTaking:
			// Brake slightly later, accelerate slightly earlier
			// This is skill-based - better execution of same physics
			Adjustment = 0.02f; // 2% more aggressive target
			break;

		case EMGAICatchUpBehavior::DraftingFocus:
			// Only get bonus when actually drafting
			if (TacticalData.bInSlipstream)
			{
				Adjustment = TacticalData.SlipstreamBonus;
			}
			break;

		case EMGAICatchUpBehavior::MaxEffort:
			// Push to the limit of skill
			// Still same physics - just optimal execution
			Adjustment = 0.03f; // 3% more aggressive
			break;

		case EMGAICatchUpBehavior::Conservation:
			// Drive more conservatively when leading
			Adjustment = -0.02f; // 2% slower targets (wider safety margins)
			break;
	}

	// Scale by difficulty - harder difficulty = more skillful AI
	Adjustment *= DifficultyMultiplier;

	return Adjustment;
}

// ==========================================
// TACTICAL DECISIONS
// ==========================================

bool AMGAIRacerController::ShouldAttemptOvertake() const
{
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (!Ahead.Vehicle)
	{
		return false;
	}

	// Too far away to consider
	if (Ahead.Distance > 20.0f * AIConstants::MetersToUnits)
	{
		return false;
	}

	// Check effective aggression (includes mood modifiers)
	float OvertakeChance = OvertakeThreshold;
	if (DriverProfile)
	{
		// Use effective aggression which factors in mood and rivalry
		float EffectiveAggression = DriverProfile->GetEffectiveAggression();
		OvertakeChance = DriverProfile->Aggression.OvertakeAggression * EffectiveAggression;
	}

	// Modify by difficulty
	OvertakeChance *= DifficultyMultiplier;

	// More likely if we've been following for a while
	if (TacticalData.TimeFollowing > 3.0f)
	{
		OvertakeChance += 0.2f;
	}

	// More likely near overtaking zones
	if (TacticalData.DistanceToOvertakeZone < 100.0f * AIConstants::MetersToUnits)
	{
		OvertakeChance += 0.15f;
	}

	// More likely if we're faster
	if (Ahead.SpeedDifference > 0.0f)
	{
		OvertakeChance += 0.1f;
	}

	// Less likely against player (to avoid feeling unfair)
	if (Ahead.bIsPlayer)
	{
		OvertakeChance *= 0.8f;
	}

	// Aggression escalation increases overtake likelihood
	if (DriverProfile)
	{
		float EscalatedAggression = DriverProfile->GetEscalatedAggression();
		OvertakeChance *= (1.0f + EscalatedAggression * 0.3f);

		// Grudge against target increases overtake urgency
		if (DriverProfile->HasGrudgeAgainst(Ahead.Vehicle))
		{
			float GrudgeIntensity = DriverProfile->GetGrudgeIntensity(Ahead.Vehicle);
			OvertakeChance += GrudgeIntensity * 0.3f;
		}

		// Battle mode increases aggressiveness
		if (DriverProfile->bInBattleMode && DriverProfile->BattleOpponent.Get() == Ahead.Vehicle)
		{
			OvertakeChance += 0.25f;
		}
	}

	// Weather caution - less likely to attempt risky overtakes in poor conditions
	// Uses unified weather difficulty rating for consistent behavior
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			// Use weather difficulty rating (1-5) to scale overtake willingness
			const int32 Difficulty = WeatherSubsystem->GetWeatherDifficultyRating();

			// Difficulty 1 = clear (no penalty)
			// Difficulty 2-3 = moderate (30-50% reduction)
			// Difficulty 4-5 = severe (60-80% reduction)
			if (Difficulty >= 2)
			{
				const float DifficultyPenalty = FMath::Lerp(0.7f, 0.2f, (Difficulty - 2) / 3.0f);
				OvertakeChance *= DifficultyPenalty;
			}

			// Additional penalty for hazardous conditions (aquaplaning, severe weather)
			if (WeatherSubsystem->AreConditionsHazardous())
			{
				OvertakeChance *= 0.5f; // 50% additional reduction in hazardous conditions
			}
		}
	}

	// Random check (scaled for per-frame calling)
	return FMath::FRand() < OvertakeChance * 0.05f;
}

bool AMGAIRacerController::ShouldDefendPosition() const
{
	FMGAIVehiclePerception Behind = GetVehicleBehind();
	if (!Behind.Vehicle)
	{
		return false;
	}

	// Only defend if they're close
	if (Behind.Distance > 15.0f * AIConstants::MetersToUnits)
	{
		return false;
	}

	float DefendChance = 0.5f;
	if (DriverProfile)
	{
		// Use effective aggression which factors in mood (frustrated/vengeful AI defends harder)
		float EffectiveAggression = DriverProfile->GetEffectiveAggression();
		DefendChance = DriverProfile->Aggression.DefenseAggression * EffectiveAggression;
	}

	// More likely against player (adds challenge without cheating physics)
	if (Behind.bIsPlayer && DriverProfile && DriverProfile->Aggression.bTargetsPlayer)
	{
		DefendChance += 0.2f;
	}

	// Random check
	return FMath::FRand() < DefendChance * 0.03f;
}

EMGOvertakeStrategy AMGAIRacerController::ChooseOvertakeStrategy(const FMGAIVehiclePerception& Target) const
{
	// Choose strategy based on situation and personality

	FMGAIRacingLinePoint CurrentPoint = GetRacingLinePointAhead(0.0f);

	// If near braking zone and aggressive, try late braking
	if (CurrentPoint.bIsBrakingZone && DriverProfile && DriverProfile->Aggression.RiskTaking > 0.6f)
	{
		return EMGOvertakeStrategy::LateBraking;
	}

	// If in draft range, use slipstream
	if (Target.bInSlipstreamRange && TacticalData.DistanceToOvertakeZone < 100.0f * AIConstants::MetersToUnits)
	{
		return EMGOvertakeStrategy::SlipstreamPass;
	}

	// If we have speed advantage, use better exit
	if (Target.SpeedDifference > 50.0f)
	{
		return EMGOvertakeStrategy::BetterExit;
	}

	// Default based on personality
	if (DriverProfile)
	{
		if (DriverProfile->Personality == EMGDriverPersonality::Aggressive)
		{
			return EMGOvertakeStrategy::Pressure;
		}
		else if (DriverProfile->Personality == EMGDriverPersonality::Calculated)
		{
			return EMGOvertakeStrategy::BetterExit;
		}
	}

	return EMGOvertakeStrategy::Patient;
}

EMGDefenseStrategy AMGAIRacerController::ChooseDefenseStrategy(const FMGAIVehiclePerception& Attacker) const
{
	FMGAIRacingLinePoint CurrentPoint = GetRacingLinePointAhead(0.0f);

	// In corner, use defensive line
	if (CurrentPoint.bIsApex || CurrentPoint.bIsBrakingZone)
	{
		return EMGDefenseStrategy::DefensiveLine;
	}

	// Based on personality
	if (DriverProfile)
	{
		if (DriverProfile->Personality == EMGDriverPersonality::Aggressive)
		{
			return EMGDefenseStrategy::CoverInside;
		}
		else if (DriverProfile->Personality == EMGDriverPersonality::Calculated)
		{
			return EMGDefenseStrategy::PaceDefense;
		}
	}

	return EMGDefenseStrategy::CoverLine;
}

EMGAICatchUpBehavior AMGAIRacerController::DetermineCatchUpMode() const
{
	if (!bSkillBasedCatchUpEnabled)
	{
		return EMGAICatchUpBehavior::None;
	}

	// If leading with comfortable gap, conserve
	if (CurrentRacePosition == 1 && GapToVehicleAhead > AIConstants::LargeGapThreshold)
	{
		return EMGAICatchUpBehavior::Conservation;
	}

	// If far behind, push harder
	if (GapToLeader > AIConstants::LargeGapThreshold)
	{
		if (DriverProfile && DriverProfile->Aggression.RiskTaking > 0.5f)
		{
			return EMGAICatchUpBehavior::MaxEffort;
		}
		return EMGAICatchUpBehavior::RiskTaking;
	}

	// If in pack but can draft, focus on that
	if (TacticalData.bInSlipstream)
	{
		return EMGAICatchUpBehavior::DraftingFocus;
	}

	return EMGAICatchUpBehavior::None;
}

bool AMGAIRacerController::ShouldStartDrafting() const
{
	FMGAIVehiclePerception Ahead = GetVehicleAhead();
	if (!Ahead.Vehicle || !Ahead.bInSlipstreamRange)
	{
		return false;
	}

	// More likely if similar speeds (can hold the draft)
	if (FMath::Abs(Ahead.SpeedDifference) < 100.0f)
	{
		return FMath::FRand() < 0.3f;
	}

	return false;
}

// ==========================================
// UTILITY METHODS
// ==========================================

FMGAIRacingLinePoint AMGAIRacerController::GetRacingLinePointAhead(float Distance) const
{
	if (RacingLinePoints.Num() == 0)
	{
		return FMGAIRacingLinePoint();
	}

	float DistanceAccum = 0.0f;
	int32 Index = CurrentRacingLineIndex;

	while (DistanceAccum < Distance && Index < RacingLinePoints.Num() - 1)
	{
		int32 NextIndex = (Index + 1) % RacingLinePoints.Num();
		float SegmentDist = FVector::Dist(RacingLinePoints[Index].Position, RacingLinePoints[NextIndex].Position);
		DistanceAccum += SegmentDist;
		Index = NextIndex;
	}

	// Wrap for circuits
	Index = Index % RacingLinePoints.Num();
	return RacingLinePoints[Index];
}

int32 AMGAIRacerController::FindClosestRacingLinePoint(const FVector& Position) const
{
	if (RacingLinePoints.Num() == 0)
	{
		return -1;
	}

	int32 ClosestIndex = 0;
	float ClosestDistSq = FLT_MAX;

	// Search around current index for efficiency
	int32 SearchStart = FMath::Max(0, CurrentRacingLineIndex - 10);
	int32 SearchEnd = FMath::Min(RacingLinePoints.Num() - 1, CurrentRacingLineIndex + 20);

	for (int32 i = SearchStart; i <= SearchEnd; ++i)
	{
		float DistSq = FVector::DistSquared(Position, RacingLinePoints[i].Position);
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}

APawn* AMGAIRacerController::GetPlayerVehicle() const
{
	if (UWorld* World = GetWorld())
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			return PC->GetPawn();
		}
	}
	return nullptr;
}

FMGAIVehiclePerception AMGAIRacerController::GetVehicleAhead() const
{
	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		if (Perception.bIsAhead && FMath::Abs(Perception.Angle) < 60.0f)
		{
			return Perception;
		}
	}
	return FMGAIVehiclePerception();
}

FMGAIVehiclePerception AMGAIRacerController::GetVehicleBehind() const
{
	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		if (!Perception.bIsAhead && FMath::Abs(Perception.Angle) > 120.0f)
		{
			return Perception;
		}
	}
	return FMGAIVehiclePerception();
}

bool AMGAIRacerController::IsOvertakePathClear(bool bOnLeft) const
{
	for (const FMGAIVehiclePerception& Perception : PerceivedVehicles)
	{
		if (Perception.Distance > 25.0f * AIConstants::MetersToUnits)
		{
			continue;
		}

		// Check if vehicle is in our overtaking lane
		if ((bOnLeft && Perception.bIsOnLeft) || (!bOnLeft && !Perception.bIsOnLeft))
		{
			if (FMath::Abs(Perception.Angle) < 90.0f) // In front quadrant
			{
				return false;
			}
		}
	}
	return true;
}

void AMGAIRacerController::ApplyProfileModifiers(FMGAISteeringOutput& Output)
{
	if (!DriverProfile)
	{
		return;
	}

	// Apply skill-based inaccuracy (makes lower skill AI less precise)
	float LineInaccuracy = 1.0f - DriverProfile->Skill.LineAccuracy;
	LineInaccuracy *= (1.0f / DifficultyMultiplier); // Harder difficulty = less mistakes

	Output.Steering += FMath::FRandRange(-LineInaccuracy, LineInaccuracy) * 0.1f;
	Output.Steering = FMath::Clamp(Output.Steering, -1.0f, 1.0f);

	// Braking accuracy
	float BrakeInaccuracy = 1.0f - DriverProfile->Skill.BrakingAccuracy;
	Output.Brake *= 1.0f + FMath::FRandRange(-BrakeInaccuracy, BrakeInaccuracy) * 0.15f;
	Output.Brake = FMath::Clamp(Output.Brake, 0.0f, 1.0f);

	// Random mistakes based on consistency
	float MistakeChance = DriverProfile->Skill.MistakeFrequency * 0.005f;
	MistakeChance *= (1.0f / DifficultyMultiplier);

	if (FMath::FRand() < MistakeChance)
	{
		// Small mistake - lift throttle or minor steering error
		if (FMath::RandBool())
		{
			Output.Throttle *= FMath::FRandRange(0.3f, 0.7f);
		}
		else
		{
			Output.Steering += FMath::FRandRange(-0.2f, 0.2f);
			Output.Steering = FMath::Clamp(Output.Steering, -1.0f, 1.0f);
		}
		Output.Confidence *= 0.7f;
	}
}

void AMGAIRacerController::SetState(EMGAIDrivingState NewState)
{
	if (CurrentState != NewState)
	{
		EMGAIDrivingState OldState = CurrentState;
		CurrentState = NewState;
		TimeInState = 0.0f;

		// Reset state-specific data
		if (NewState != EMGAIDrivingState::Overtaking)
		{
			OvertakeTimer = 0.0f;
		}

		OnDrivingStateChanged.Broadcast(OldState, NewState);
	}
}

float AMGAIRacerController::AddSteeringNoise(float BaseValue)
{
	if (!DriverProfile)
	{
		return BaseValue;
	}

	float NoiseAmount = 0.02f; // Base noise

	// Personality affects noise
	switch (DriverProfile->Personality)
	{
		case EMGDriverPersonality::Unpredictable:
			NoiseAmount = 0.08f;
			break;
		case EMGDriverPersonality::Rookie:
			NoiseAmount = 0.05f;
			break;
		case EMGDriverPersonality::Calculated:
			NoiseAmount = 0.01f;
			break;
		default:
			break;
	}

	// Consistency modifies noise
	NoiseAmount *= (1.0f - DriverProfile->Skill.Consistency);

	// Difficulty reduces noise
	NoiseAmount *= (1.0f / DifficultyMultiplier);

	return BaseValue + FMath::FRandRange(-NoiseAmount, NoiseAmount);
}

float AMGAIRacerController::CalculateSlipstreamBonus(const FVector& Position, const FVector& Velocity) const
{
	float MaxBonus = AIConstants::MaxSlipstreamBonus;
	float CurrentBonus = 0.0f;

	for (const FMGAIVehiclePerception& Perceived : PerceivedVehicles)
	{
		if (Perceived.bIsAhead && Perceived.bInSlipstreamRange)
		{
			// Bonus scales with proximity and alignment
			float DistanceFactor = 1.0f - (Perceived.Distance / (SlipstreamRange * AIConstants::MetersToUnits));
			float AngleFactor = 1.0f - (FMath::Abs(Perceived.Angle) / SlipstreamAngle);

			CurrentBonus = FMath::Max(CurrentBonus, MaxBonus * DistanceFactor * AngleFactor);
		}
	}

	return CurrentBonus;
}

bool AMGAIRacerController::IsInSlipstream(AActor* LeadVehicle) const
{
	if (!LeadVehicle || !VehiclePawn)
	{
		return false;
	}

	APawn* LeadPawn = Cast<APawn>(LeadVehicle);
	if (!LeadPawn)
	{
		return false;
	}

	FVector MyLocation = VehiclePawn->GetActorLocation();
	FVector LeadLocation = LeadPawn->GetActorLocation();
	FVector LeadForward = LeadPawn->GetActorForwardVector();
	FVector LeadBackward = -LeadForward;

	// Check if we're behind the lead vehicle
	FVector ToUs = MyLocation - LeadLocation;
	float DotBack = FVector::DotProduct(ToUs.GetSafeNormal(), LeadBackward);

	// Must be behind (DotBack > 0)
	if (DotBack <= 0.0f)
	{
		return false;
	}

	// Check distance
	float Distance = FVector::Dist(MyLocation, LeadLocation);
	if (Distance > SlipstreamRange * AIConstants::MetersToUnits)
	{
		return false;
	}

	// Check angle (must be relatively in line)
	float Angle = FMath::RadiansToDegrees(FMath::Acos(DotBack));
	return Angle < SlipstreamAngle;
}

float AMGAIRacerController::GetSituationalRiskLevel() const
{
	float BaseRisk = 0.5f;

	if (DriverProfile)
	{
		// Use effective aggression which includes mood modifiers
		// Desperate/Vengeful AI takes MORE risks, Intimidated takes LESS
		float EffectiveAggression = DriverProfile->GetEffectiveAggression();
		BaseRisk = DriverProfile->Aggression.RiskTaking * EffectiveAggression;
	}

	// Increase risk when behind
	if (GapToLeader > AIConstants::CloseGapThreshold)
	{
		BaseRisk += 0.1f;
	}

	// Decrease risk when leading
	if (CurrentRacePosition == 1)
	{
		BaseRisk -= 0.1f;
	}

	// Increase risk late in race (would need lap info)
	// BaseRisk += LateRaceFactor;

	return FMath::Clamp(BaseRisk, 0.0f, 1.0f);
}

float AMGAIRacerController::CalculateBrakingDistance(float CurrentSpeed, float TargetSpeed) const
{
	// v^2 = v0^2 + 2*a*d
	// d = (v^2 - v0^2) / (2*a)

	float SpeedDiff = CurrentSpeed - TargetSpeed;
	if (SpeedDiff <= 0.0f)
	{
		return 0.0f;
	}

	float Deceleration = AIConstants::DefaultBrakingDecel * AIConstants::MetersToUnits;

	// Apply effective skill factor - better skill = later braking (shorter distance)
	// Mood affects skill: InTheZone = better braking, Desperate = worse
	if (DriverProfile)
	{
		float EffectiveSkill = DriverProfile->GetEffectiveSkill();
		Deceleration *= (0.8f + 0.4f * DriverProfile->Skill.BrakingAccuracy * EffectiveSkill);
	}

	// Apply weather conditions to braking using unified weather API
	// Combines road grip, aquaplaning, temperature effects
	if (UWorld* World = GetWorld())
	{
		if (UMGWeatherSubsystem* WeatherSubsystem = World->GetSubsystem<UMGWeatherSubsystem>())
		{
			// Get unified grip multiplier (includes all weather effects)
			const FVector VehicleLocation = VehiclePawn ? VehiclePawn->GetActorLocation() : FVector::ZeroVector;
			const float CurrentSpeedKPH = CurrentSpeed / AIConstants::MetersToUnits * 3.6f;
			const float UnifiedGrip = WeatherSubsystem->GetUnifiedGripMultiplier(VehicleLocation, CurrentSpeedKPH);

			// Grip affects braking effectiveness
			// Lower grip = reduced deceleration capability
			if (UnifiedGrip > 0.01f)
			{
				Deceleration *= UnifiedGrip;
			}

			// Add safety margin in hazardous conditions
			// Skilled drivers need smaller margins due to better vehicle control
			if (WeatherSubsystem->AreConditionsHazardous())
			{
				const float SafetyMargin = DriverProfile ?
					FMath::Lerp(1.3f, 1.1f, DriverProfile->GetEffectiveSkill()) : 1.2f;
				Deceleration /= SafetyMargin;
			}
		}
	}

	float Distance = (CurrentSpeed * CurrentSpeed - TargetSpeed * TargetSpeed) / (2.0f * Deceleration);

	return Distance;
}

// ==========================================
// ADAPTIVE BEHAVIOR & LEARNING
// ==========================================

void AMGAIRacerController::UpdateMoodAndLearning(float MGDeltaTime)
{
	if (!DriverProfile)
	{
		return;
	}

	// Track position changes for mood updates
	float PositionDelta = static_cast<float>(LastKnownPosition - CurrentRacePosition);

	// Track damage from vehicle damage system
	float DamageReceived = 0.0f;
	if (AMGVehiclePawn* MGVehicle = Cast<AMGVehiclePawn>(VehiclePawn))
	{
		if (UMGVehicleDamageSystem* DamageSystem = MGVehicle->VehicleDamageSystem)
		{
			// Get current damage percentage (0-100) and convert to 0-1
			float CurrentDamage = DamageSystem->GetOverallDamagePercent() / 100.0f;

			// Calculate damage received this frame (positive if damage increased)
			DamageReceived = FMath::Max(0.0f, CurrentDamage - LastKnownDamage);
			LastKnownDamage = CurrentDamage;
		}
	}

	// Track if we were overtaken this frame
	bool bWasOvertakenThisFrame = (CurrentState == EMGAIDrivingState::Defending && TimeInState < 0.5f);

	// Update mood based on race events
	DriverProfile->UpdateMood(PositionDelta, DamageReceived, bWasOvertakenThisFrame);

	// Remember position for next update
	LastKnownPosition = CurrentRacePosition;

	// Learn from player behavior (if player is nearby)
	APawn* PlayerVehicle = GetPlayerVehicle();
	if (PlayerVehicle)
	{
		FMGAIVehiclePerception PlayerPerception;
		bool bFoundPlayer = false;

		// Find player in perceived vehicles
		for (const FMGAIVehiclePerception& Perceived : PerceivedVehicles)
		{
			if (Perceived.bIsPlayer)
			{
				PlayerPerception = Perceived;
				bFoundPlayer = true;
				break;
			}
		}

		// If player is close enough to observe, learn their behavior
		if (bFoundPlayer && PlayerPerception.Distance < 30.0f * AIConstants::MetersToUnits)
		{
			// Observe aggression from their proximity and overtaking attempts
			float ObservedAggression = 0.5f; // Baseline
			if (PlayerPerception.Distance < 10.0f * AIConstants::MetersToUnits)
			{
				ObservedAggression = 0.8f; // Very close = aggressive
			}
			if (PlayerPerception.SpeedDifference > 100.0f)
			{
				ObservedAggression += 0.2f; // Fast closing = aggressive
			}

			// Observe braking from player vehicle input
			float ObservedBraking = 0.5f;
			if (AMGVehiclePawn* PlayerMGVehicle = Cast<AMGVehiclePawn>(PlayerVehicle))
			{
				if (UMGVehicleMovementComponent* Movement = PlayerMGVehicle->GetMGVehicleMovement())
				{
					// GetBrakeInput returns 0-1, use directly as braking intensity
					ObservedBraking = Movement->GetBrakeInput();
				}
			}

			// Observe overtake side preference
			float OvertakeSide = PlayerPerception.bIsOnLeft ? -1.0f : 1.0f;

			// Feed observations to learning system (throttled to once per second)
			static float LearningTimer = 0.0f;
			LearningTimer += DeltaTime;
			if (LearningTimer > 1.0f)
			{
				DriverProfile->LearnPlayerBehavior(ObservedAggression, ObservedBraking, OvertakeSide);
				LearningTimer = 0.0f;
			}
		}
	}
}

// ==========================================
// AGGRESSION RESPONSE SYSTEM
// ==========================================

void AMGAIRacerController::HandleContactResponse(EMGContactResponse Response, AActor* Offender, float Severity)
{
	if (!DriverProfile || !Offender)
	{
		return;
	}

	switch (Response)
	{
		case EMGContactResponse::Ignore:
			// Do nothing - focus on racing
			break;

		case EMGContactResponse::BackOff:
			// Reduce aggression temporarily, increase following gap
			MinFollowingGap = FMath::Min(MinFollowingGap + 0.5f, 3.0f);
			// If we were overtaking the offender, abort
			if (CurrentState == EMGAIDrivingState::Overtaking && TacticalData.TacticalTarget == Offender)
			{
				SetState(EMGAIDrivingState::Racing);
				TacticalData.TacticalTarget = nullptr;
			}
			break;

		case EMGContactResponse::Retaliate:
			// Enter battle mode with offender
			DriverProfile->EnterBattleMode(Offender);
			// If they're ahead, start aggressive pursuit
			for (const FMGAIVehiclePerception& Perceived : PerceivedVehicles)
			{
				if (Perceived.Vehicle == Offender && Perceived.bIsAhead)
				{
					TacticalData.TacticalTarget = Offender;
					TacticalData.OvertakeStrategy = EMGOvertakeStrategy::Pressure;
					break;
				}
			}
			break;

		case EMGContactResponse::Protect:
			// Become more defensive, avoid contact
			MinFollowingGap = FMath::Min(MinFollowingGap + 1.0f, 3.0f);
			if (TacticalData.CatchUpMode == EMGAICatchUpBehavior::MaxEffort)
			{
				TacticalData.CatchUpMode = EMGAICatchUpBehavior::RiskTaking;
			}
			else if (TacticalData.CatchUpMode == EMGAICatchUpBehavior::RiskTaking)
			{
				TacticalData.CatchUpMode = EMGAICatchUpBehavior::None;
			}
			break;

		case EMGContactResponse::Mirror:
			{
				APawn* OffenderPawn = Cast<APawn>(Offender);
				if (OffenderPawn)
				{
					float TheirSpeed = OffenderPawn->GetVelocity().Size();
					if (TheirSpeed > GetCurrentSpeed() * 1.1f)
					{
						DriverProfile->EnterBattleMode(Offender);
					}
				}
			}
			break;

		case EMGContactResponse::Report:
			// Record the incident for future reference
			break;
	}
}

void AMGAIRacerController::ApplyAggressionModifiers(FMGAISteeringOutput& Output)
{
	if (!DriverProfile)
	{
		return;
	}

	float EscalatedAggression = DriverProfile->GetEscalatedAggression();
	Output.Throttle = FMath::Min(Output.Throttle * (1.0f + EscalatedAggression * 0.1f), 1.0f);

	if (Output.Brake > 0.0f && EscalatedAggression > 0.7f)
	{
		Output.Brake *= (1.0f - (EscalatedAggression - 0.7f) * 0.3f);
	}

	FMGPersonalityBehaviors Behaviors = DriverProfile->GetEffectivePersonalityBehaviors();

	if (Behaviors.BrakePointBias > 0.0f && Output.Brake > 0.0f)
	{
		Output.Brake *= (1.0f - Behaviors.BrakePointBias * 0.2f);
	}
	else if (Behaviors.BrakePointBias < 0.0f && Output.Brake > 0.0f)
	{
		Output.Brake = FMath::Min(Output.Brake * (1.0f - Behaviors.BrakePointBias * 0.2f), 1.0f);
	}

	for (const FMGAIVehiclePerception& Perceived : PerceivedVehicles)
	{
		if (Perceived.Distance < 5.0f * AIConstants::MetersToUnits)
		{
			Output.Confidence *= Behaviors.SideBySideWillingness;
			break;
		}
	}

	if (DriverProfile->CurrentAggressionStage == EMGAggressionStage::Rage)
	{
		if (FMath::FRand() < 0.05f)
		{
			Output.Steering += FMath::FRandRange(-0.15f, 0.15f);
			Output.Steering = FMath::Clamp(Output.Steering, -1.0f, 1.0f);
		}
		if (FMath::FRand() < 0.03f)
		{
			Output.Brake *= FMath::FRandRange(0.7f, 1.3f);
			Output.Brake = FMath::Clamp(Output.Brake, 0.0f, 1.0f);
		}
	}
}

bool AMGAIRacerController::ShouldAttemptDirtyMove(AActor* Target) const
{
	if (!DriverProfile || !Target)
	{
		return false;
	}

	bool bIsDefending = (CurrentState == EMGAIDrivingState::Defending);
	if (!DriverProfile->WillUseDirtyTactics(CurrentRacePosition, bIsDefending))
	{
		return false;
	}

	if (DriverProfile->HasGrudgeAgainst(Target))
	{
		float GrudgeIntensity = DriverProfile->GetGrudgeIntensity(Target);
		return FMath::FRand() < GrudgeIntensity * 0.5f;
	}

	APawn* TargetPawn = Cast<APawn>(Target);
	if (TargetPawn)
	{
		bool bIsPlayer = Cast<APlayerController>(TargetPawn->GetController()) != nullptr;
		if (bIsPlayer && DriverProfile->Aggression.bTargetsPlayer)
		{
			return FMath::FRand() < DriverProfile->GetEscalatedAggression() * 0.3f;
		}
	}

	return FMath::FRand() < DriverProfile->GetEscalatedAggression() * 0.1f;
}

float AMGAIRacerController::GetPersonalityBrakeAdjustment() const
{
	if (!DriverProfile)
	{
		return 0.0f;
	}

	FMGPersonalityBehaviors Behaviors = DriverProfile->GetEffectivePersonalityBehaviors();
	float Adjustment = Behaviors.BrakePointBias;

	switch (DriverProfile->CurrentAggressionStage)
	{
		case EMGAggressionStage::Elevated:
			Adjustment += 0.1f;
			break;
		case EMGAggressionStage::High:
			Adjustment += 0.2f;
			break;
		case EMGAggressionStage::Maximum:
			Adjustment += 0.3f;
			break;
		case EMGAggressionStage::Rage:
			Adjustment += 0.4f;
			break;
		default:
			break;
	}

	if (DriverProfile->CurrentMood == EMGAIMood::Desperate)
	{
		Adjustment += 0.2f;
	}
	else if (DriverProfile->CurrentMood == EMGAIMood::Intimidated)
	{
		Adjustment -= 0.15f;
	}
	else if (DriverProfile->CurrentMood == EMGAIMood::InTheZone)
	{
		Adjustment += 0.1f;
	}

	return FMath::Clamp(Adjustment, -0.5f, 0.5f);
}
