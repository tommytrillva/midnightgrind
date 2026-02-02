// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MG_VHCL_ArcadeEnhancements.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "Vehicle/MGVehiclePawn.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UMGVehicleArcadeEnhancements::UMGVehicleArcadeEnhancements()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // Run after physics update
}

void UMGVehicleArcadeEnhancements::BeginPlay()
{
	Super::BeginPlay();

	// Find the vehicle movement component
	AActor* Owner = GetOwner();
	if (Owner)
	{
		MovementComponent = Owner->FindComponentByClass<UMGVehicleMovementComponent>();
		
		if (!MovementComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("MG_VHCL_ArcadeEnhancements: No movement component found on %s"), *Owner->GetName());
		}

		// Bind to collision events
		if (AMGVehiclePawn* VehiclePawn = Cast<AMGVehiclePawn>(Owner))
		{
			// Subscribe to vehicle collision delegate if available
			// VehiclePawn->OnVehicleCollision.AddDynamic(this, &UMGVehicleArcadeEnhancements::OnVehicleCollision);
		}
	}
}

void UMGVehicleArcadeEnhancements::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableArcadeMode || !MovementComponent)
	{
		return;
	}

	// Apply arcade assists in order
	if (Config.bDriftAssist)
	{
		ApplyDriftAssist(DeltaTime);
	}

	if (Config.bCollisionBounce)
	{
		ApplyCollisionRecovery(DeltaTime);
	}

	if (Config.bOversteerCorrect)
	{
		ApplyOversteerCorrection(DeltaTime);
	}

	if (Config.bMaintainSpeedInDrift)
	{
		MaintainDriftSpeed(DeltaTime);
	}

	if (Config.bDisableSimSystems)
	{
		DisableSimulationSystems();
	}

	// Debug visualization
	if (bShowDebugInfo)
	{
		DrawDebugInfo();
	}
}

void UMGVehicleArcadeEnhancements::ApplyDriftAssist(float DeltaTime)
{
	if (!MovementComponent) return;

	// Only assist when player is trying to drift
	float CurrentSpeed = GetCurrentSpeedMPH();
	if (CurrentSpeed < Config.MinDriftSpeedMPH)
	{
		bDriftAssistActive = false;
		CurrentDriftCorrection = 0.0f;
		return;
	}

	float SteeringInput = GetSteeringInput();
	bool bHandbrake = IsHandbrakeEngaged();

	// Check if drift conditions are met
	if (FMath::Abs(SteeringInput) > 0.3f && bHandbrake)
	{
		float CurrentAngle = GetCurrentDriftAngle();
		float AngleDelta = CurrentAngle - Config.IdealDriftAngle;

		// Check if we're outside tolerance range
		if (FMath::Abs(AngleDelta) > Config.DriftAngleTolerance)
		{
			bDriftAssistActive = true;

			// Calculate correction strength
			float CorrectionSign = (AngleDelta > 0) ? -1.0f : 1.0f;
			float CorrectionMagnitude = FMath::Abs(AngleDelta) / Config.IdealDriftAngle;
			CorrectionMagnitude = FMath::Clamp(CorrectionMagnitude, 0.0f, 1.0f);

			// Apply scaled correction
			float CorrectionAmount = CorrectionSign * CorrectionMagnitude * Config.DriftAssistStrength;
			CurrentDriftCorrection = CorrectionAmount;

			// Apply steering correction
			ApplySteeringCorrection(CorrectionAmount * 0.5f);

			// Broadcast event
			OnDriftAssistEngaged.Broadcast(CorrectionAmount);
		}
		else
		{
			// Within tolerance, no assist needed
			bDriftAssistActive = false;
			CurrentDriftCorrection = 0.0f;
		}
	}
	else
	{
		bDriftAssistActive = false;
		CurrentDriftCorrection = 0.0f;
	}
}

void UMGVehicleArcadeEnhancements::ApplyCollisionRecovery(float DeltaTime)
{
	if (!MovementComponent) return;

	TimeSinceCollision += DeltaTime;

	// Check if we're in recovery window
	if (TimeSinceCollision < Config.RecoveryDuration)
	{
		bCollisionRecoveryActive = true;

		// Calculate recovery strength (fades over time)
		float RecoveryStrength = 1.0f - (TimeSinceCollision / Config.RecoveryDuration);

		AActor* Owner = GetOwner();
		if (!Owner) return;

		// Apply upward force to prevent getting stuck
		FVector UpVector = Owner->GetActorUpVector();
		FVector RecoveryForce = UpVector * 5000.0f * RecoveryStrength;

		// Add bounce away from collision normal
		if (!LastCollisionNormal.IsNearlyZero())
		{
			FVector BounceForce = LastCollisionNormal * 3000.0f * RecoveryStrength;
			BounceForce *= Config.BounceMult;
			RecoveryForce += BounceForce;

			// Broadcast bounce event
			OnCollisionBounce.Broadcast(BounceForce, BounceForce.Size());
		}

		// Apply the combined impulse
		ApplyBounceImpulse(RecoveryForce, RecoveryForce.Size());
	}
	else
	{
		bCollisionRecoveryActive = false;
	}
}

void UMGVehicleArcadeEnhancements::ApplyOversteerCorrection(float DeltaTime)
{
	if (!MovementComponent) return;

	// Only correct if not intentionally drifting
	if (IsHandbrakeEngaged()) return;

	float CurrentAngle = GetCurrentDriftAngle();
	
	// Check if we're oversteering unintentionally
	if (FMath::Abs(CurrentAngle) > Config.OversteerThreshold)
	{
		// Apply counter-steering to reduce oversteer
		float CorrectionAmount = -FMath::Sign(CurrentAngle) * Config.OversteerCorrectStrength;
		ApplySteeringCorrection(CorrectionAmount * 0.3f);
	}
}

void UMGVehicleArcadeEnhancements::MaintainDriftSpeed(float DeltaTime)
{
	if (!MovementComponent) return;

	// Only maintain speed when actively drifting
	if (!IsHandbrakeEngaged()) return;

	float CurrentSpeed = GetCurrentSpeedMPH();

	// If slowing down too much during drift, add throttle
	if (CurrentSpeed < Config.MinDriftMaintenanceSpeed)
	{
		float SpeedDeficit = 1.0f - (CurrentSpeed / Config.MinDriftMaintenanceSpeed);
		float BoostAmount = SpeedDeficit * Config.SpeedBoostAmount;
		ApplyThrottleBoost(BoostAmount);
	}
}

void UMGVehicleArcadeEnhancements::DisableSimulationSystems()
{
	if (!MovementComponent) return;

	// Disable wear and temperature simulation
	// This prevents punishment for aggressive driving in arcade mode

	// Access clutch wear state
	FMGClutchWearState& ClutchState = MovementComponent->GetClutchWearState();
	ClutchState.WearLevel = 0.0f;
	ClutchState.ClutchTemperature = 50.0f; // Cool
	ClutchState.bIsOverheating = false;
	ClutchState.bIsBurntOut = false;

	// Reset brake temperature
	MovementComponent->SetBrakeTemperature(50.0f);

	// Set tire temps to optimal (if direct access is available)
	// MovementComponent->SetTireTemperatureOptimal();
}

void UMGVehicleArcadeEnhancements::OnVehicleCollision(AActor* OtherActor, FVector HitLocation, FVector HitNormal)
{
	// Record collision for bounce recovery
	LastCollisionNormal = HitNormal;
	LastCollisionTime = GetWorld()->GetTimeSeconds();
	TimeSinceCollision = 0.0f;
}

// ==========================================
// HELPER FUNCTIONS
// ==========================================

float UMGVehicleArcadeEnhancements::GetCurrentDriftAngle() const
{
	if (!MovementComponent) return 0.0f;

	FMGDriftState DriftState = MovementComponent->GetDriftState();
	return DriftState.DriftAngle;
}

float UMGVehicleArcadeEnhancements::GetCurrentSpeedMPH() const
{
	if (!MovementComponent) return 0.0f;
	return MovementComponent->GetSpeedMPH();
}

bool UMGVehicleArcadeEnhancements::IsHandbrakeEngaged() const
{
	if (!MovementComponent) return false;
	return MovementComponent->IsHandbrakeEngaged();
}

float UMGVehicleArcadeEnhancements::GetSteeringInput() const
{
	if (!MovementComponent) return 0.0f;
	// Access steering input from movement component
	// return MovementComponent->GetSteeringInput();
	return 0.0f; // Placeholder - implement based on actual API
}

void UMGVehicleArcadeEnhancements::ApplySteeringCorrection(float CorrectionAmount)
{
	if (!MovementComponent) return;

	// Get current steering and blend in correction
	float CurrentSteering = GetSteeringInput();
	float CorrectedSteering = FMath::Clamp(CurrentSteering + CorrectionAmount, -1.0f, 1.0f);
	
	MovementComponent->SetSteeringInput(CorrectedSteering);
}

void UMGVehicleArcadeEnhancements::ApplyThrottleBoost(float BoostAmount)
{
	if (!MovementComponent) return;

	// Add throttle without overriding player input
	float CurrentThrottle = 0.0f; // Get from movement component
	float BoostedThrottle = FMath::Clamp(CurrentThrottle + BoostAmount, 0.0f, 1.0f);
	
	MovementComponent->SetThrottleInput(BoostedThrottle);
}

void UMGVehicleArcadeEnhancements::ApplyBounceImpulse(const FVector& Direction, float Force)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (RootPrimitive && RootPrimitive->IsSimulatingPhysics())
	{
		RootPrimitive->AddImpulse(Direction, NAME_None, true);
	}
}

void UMGVehicleArcadeEnhancements::DrawDebugInfo()
{
	if (!MovementComponent || !GetWorld()) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Location = Owner->GetActorLocation() + FVector(0, 0, 200);
	
	// Draw drift assist status
	FString DriftInfo = FString::Printf(TEXT("Drift Assist: %s\nCorrection: %.2f\nAngle: %.1f°"),
		bDriftAssistActive ? TEXT("ACTIVE") : TEXT("Inactive"),
		CurrentDriftCorrection,
		GetCurrentDriftAngle()
	);
	
	DrawDebugString(GetWorld(), Location, DriftInfo, nullptr, FColor::Yellow, 0.0f, true);

	// Draw collision recovery status
	if (bCollisionRecoveryActive)
	{
		FVector RecoveryLocation = Location + FVector(0, 0, 100);
		DrawDebugString(GetWorld(), RecoveryLocation, TEXT("COLLISION RECOVERY"), nullptr, FColor::Red, 0.0f, true);
	}
}
