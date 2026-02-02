// Copyright Midnight Grind. All Rights Reserved.

#include "Camera/MGDynamicCameraComponent.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UMGDynamicCameraComponent::UMGDynamicCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Set reasonable defaults
	CurrentDistance = BaseCameraDistance;
	CurrentHeight = BaseCameraHeight;
}

void UMGDynamicCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeReferences();
	ApplyBehaviorModePresets();
}

void UMGDynamicCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedVehiclePawn || !CachedSpringArm || !CachedCamera)
	{
		return;
	}

	// Update camera subsystems in order
	UpdateLookAheadTarget(DeltaTime);
	UpdateSpeedAdaptivePosition(DeltaTime);
	UpdateTurnLean(DeltaTime);
	UpdateCollisionAvoidance(DeltaTime);
	ApplyCameraSmoothing(DeltaTime);
	ApplyRetroAesthetic(DeltaTime);

	// Update velocity tracking for next frame
	FVector CurrentVelocity = GetVehicleVelocity();
	CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / FMath::Max(DeltaTime, 0.001f);
	PreviousVelocity = CurrentVelocity;
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGDynamicCameraComponent::SetBehaviorMode(EMGCameraBehaviorMode Mode)
{
	BehaviorMode = Mode;
	ApplyBehaviorModePresets();
}

void UMGDynamicCameraComponent::SetLookAheadConfig(const FMGCameraLookAheadConfig& Config)
{
	LookAheadConfig = Config;
}

void UMGDynamicCameraComponent::SetSmoothingConfig(const FMGCameraSmoothingConfig& Config)
{
	SmoothingConfig = Config;
}

void UMGDynamicCameraComponent::SetCollisionConfig(const FMGCameraCollisionConfig& Config)
{
	CollisionConfig = Config;
}

void UMGDynamicCameraComponent::SetTurnLeanConfig(const FMGCameraTurnLeanConfig& Config)
{
	TurnLeanConfig = Config;
}

void UMGDynamicCameraComponent::SetSpeedAdaptiveConfig(const FMGCameraSpeedAdaptiveConfig& Config)
{
	SpeedAdaptiveConfig = Config;
}

void UMGDynamicCameraComponent::SetRetroAestheticConfig(const FMGRetroAestheticConfig& Config)
{
	RetroAestheticConfig = Config;
}

void UMGDynamicCameraComponent::ResetToDefaults()
{
	CurrentDistance = BaseCameraDistance;
	CurrentHeight = BaseCameraHeight;
	CurrentLookAheadTarget = FVector::ZeroVector;
	CurrentLeanAngle = 0.0f;
	TargetLeanAngle = 0.0f;
	CurrentLateralOffset = FVector::ZeroVector;
	TargetLateralOffset = FVector::ZeroVector;
	CurrentCollisionPush = 0.0f;
	TargetCollisionPush = 0.0f;
	bIsAvoidingCollision = false;
}

// ==========================================
// CAMERA CONTROL
// ==========================================

void UMGDynamicCameraComponent::SetBaseCameraDistance(float Distance)
{
	BaseCameraDistance = FMath::Max(Distance, 100.0f);
}

void UMGDynamicCameraComponent::SetBaseCameraHeight(float Height)
{
	BaseCameraHeight = FMath::Max(Height, 50.0f);
}

void UMGDynamicCameraComponent::SetLookAheadEnabled(bool bEnabled)
{
	LookAheadConfig.bEnabled = bEnabled;
	if (!bEnabled)
	{
		CurrentLookAheadTarget = FVector::ZeroVector;
		PreviousLookAheadTarget = FVector::ZeroVector;
	}
}

void UMGDynamicCameraComponent::SetCollisionAvoidanceEnabled(bool bEnabled)
{
	CollisionConfig.bEnabled = bEnabled;
	if (!bEnabled)
	{
		CurrentCollisionPush = 0.0f;
		TargetCollisionPush = 0.0f;
		bIsAvoidingCollision = false;
	}
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void UMGDynamicCameraComponent::InitializeReferences()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Cache vehicle pawn
	CachedVehiclePawn = Cast<AMGVehiclePawn>(Owner);
	if (!CachedVehiclePawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynamicCameraComponent: Owner is not a MGVehiclePawn"));
		return;
	}

	// Cache spring arm
	CachedSpringArm = CachedVehiclePawn->SpringArm;
	if (!CachedSpringArm)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynamicCameraComponent: No SpringArm found on vehicle"));
	}

	// Cache camera
	CachedCamera = CachedVehiclePawn->Camera;
	if (!CachedCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynamicCameraComponent: No Camera found on vehicle"));
	}

	// Initialize velocity tracking
	PreviousVelocity = GetVehicleVelocity();
	CurrentAcceleration = FVector::ZeroVector;
}

void UMGDynamicCameraComponent::UpdateLookAheadTarget(float DeltaTime)
{
	if (!LookAheadConfig.bEnabled || !CachedVehiclePawn)
	{
		CurrentLookAheadTarget = FVector::ZeroVector;
		return;
	}

	float SpeedKPH = GetVehicleSpeedKPH();
	
	// Only apply look-ahead above minimum speed
	if (SpeedKPH < LookAheadConfig.MinSpeedKPH)
	{
		FVector ZeroTarget = FVector::ZeroVector;
		CurrentLookAheadTarget = FMath::VInterpTo(CurrentLookAheadTarget, ZeroTarget, DeltaTime, LookAheadConfig.InterpSpeed);
		return;
	}

	// Calculate look-ahead distance based on speed
	float SpeedAlpha = FMath::Clamp((SpeedKPH - LookAheadConfig.MinSpeedKPH) / 150.0f, 0.0f, 1.0f);
	float LookAheadDistance = SpeedAlpha * LookAheadConfig.MaxDistance * LookAheadConfig.DistanceMultiplier;

	// Get vehicle forward direction and velocity direction
	FVector VehicleForward = CachedVehiclePawn->GetActorForwardVector();
	FVector Velocity = GetVehicleVelocity();
	FVector VelocityDir = Velocity.GetSafeNormal();

	// Blend between vehicle forward and velocity direction
	FVector LookDirection = FMath::Lerp(VehicleForward, VelocityDir, 0.7f).GetSafeNormal();

	// Calculate target position
	FVector VehicleLocation = CachedVehiclePawn->GetActorLocation();
	FVector TargetPosition = VehicleLocation + (LookDirection * LookAheadDistance);
	TargetPosition.Z += LookAheadConfig.VerticalOffset;

	// Smooth interpolation to target
	CurrentLookAheadTarget = FMath::VInterpTo(CurrentLookAheadTarget, TargetPosition, DeltaTime, LookAheadConfig.InterpSpeed);
}

void UMGDynamicCameraComponent::UpdateSpeedAdaptivePosition(float DeltaTime)
{
	if (!SpeedAdaptiveConfig.bEnabled || !CachedSpringArm)
	{
		return;
	}

	float SpeedAlpha = GetSpeedAlpha();

	// Calculate target distance and height
	float TargetDistance = BaseCameraDistance + (SpeedAlpha * SpeedAdaptiveConfig.MaxSpeedDistanceIncrease);
	float TargetHeight = BaseCameraHeight + (SpeedAlpha * SpeedAdaptiveConfig.MaxSpeedHeightIncrease);

	// Smooth interpolation
	CurrentDistance = FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaTime, SpeedAdaptiveConfig.PositionInterpSpeed);
	CurrentHeight = FMath::FInterpTo(CurrentHeight, TargetHeight, DeltaTime, SpeedAdaptiveConfig.PositionInterpSpeed);

	// Apply to spring arm
	CachedSpringArm->TargetArmLength = CurrentDistance;
	CachedSpringArm->SocketOffset = FVector(0.0f, 0.0f, CurrentHeight);
}

void UMGDynamicCameraComponent::UpdateTurnLean(float DeltaTime)
{
	if (!TurnLeanConfig.bEnabled)
	{
		TargetLeanAngle = 0.0f;
		TargetLateralOffset = FVector::ZeroVector;
		CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, 0.0f, DeltaTime, TurnLeanConfig.LeanInterpSpeed);
		CurrentLateralOffset = FMath::VInterpTo(CurrentLateralOffset, FVector::ZeroVector, DeltaTime, TurnLeanConfig.LeanInterpSpeed);
		return;
	}

	float TurnRate = CalculateTurnRate();
	
	// Check if turn rate exceeds threshold
	if (FMath::Abs(TurnRate) < TurnLeanConfig.AngularVelocityThreshold)
	{
		TargetLeanAngle = 0.0f;
		TargetLateralOffset = FVector::ZeroVector;
	}
	else
	{
		// Calculate lean based on turn rate
		float NormalizedTurnRate = FMath::Clamp(TurnRate / 180.0f, -1.0f, 1.0f);
		
		// Apply drift multiplier if drifting
		float DriftMultiplier = IsVehicleDrifting() ? TurnLeanConfig.DriftLeanMultiplier : 1.0f;
		
		TargetLeanAngle = -NormalizedTurnRate * TurnLeanConfig.MaxLeanAngle * DriftMultiplier;
		
		// Calculate lateral offset (camera shifts opposite to turn direction)
		FVector RightVector = CachedVehiclePawn ? CachedVehiclePawn->GetActorRightVector() : FVector::RightVector;
		TargetLateralOffset = RightVector * (-NormalizedTurnRate * TurnLeanConfig.LateralOffset);
	}

	// Smooth interpolation
	CurrentLeanAngle = FMath::FInterpTo(CurrentLeanAngle, TargetLeanAngle, DeltaTime, TurnLeanConfig.LeanInterpSpeed);
	CurrentLateralOffset = FMath::VInterpTo(CurrentLateralOffset, TargetLateralOffset, DeltaTime, TurnLeanConfig.LeanInterpSpeed);

	// Apply lean to camera roll
	if (CachedCamera)
	{
		FRotator CameraRotation = CachedCamera->GetRelativeRotation();
		CameraRotation.Roll = CurrentLeanAngle;
		CachedCamera->SetRelativeRotation(CameraRotation);
	}

	// Apply lateral offset to spring arm socket offset
	if (CachedSpringArm)
	{
		FVector CurrentSocketOffset = CachedSpringArm->SocketOffset;
		CurrentSocketOffset.Y = CurrentLateralOffset.Y; // Assuming Y is lateral
		CachedSpringArm->SocketOffset = CurrentSocketOffset;
	}
}

void UMGDynamicCameraComponent::UpdateCollisionAvoidance(float DeltaTime)
{
	if (!CollisionConfig.bEnabled || !CachedSpringArm || !CachedVehiclePawn)
	{
		bIsAvoidingCollision = false;
		TargetCollisionPush = 0.0f;
		CurrentCollisionPush = FMath::FInterpTo(CurrentCollisionPush, 0.0f, DeltaTime, CollisionConfig.RecoverySpeed);
		return;
	}

	// Calculate desired camera position
	FVector VehicleLocation = CachedVehiclePawn->GetActorLocation();
	FVector CameraOffset = -CachedVehiclePawn->GetActorForwardVector() * CurrentDistance;
	CameraOffset.Z += CurrentHeight;
	FVector DesiredCameraPosition = VehicleLocation + CameraOffset;

	// Trace from vehicle to desired camera position
	FHitResult HitResult;
	bool bHit = PerformCollisionTrace(VehicleLocation, DesiredCameraPosition, HitResult);

	if (bHit)
	{
		bIsAvoidingCollision = true;

		// Calculate push distance based on hit location
		float HitDistance = (HitResult.Location - VehicleLocation).Size();
		float DesiredDistance = (DesiredCameraPosition - VehicleLocation).Size();
		float PushNeeded = FMath::Max(0.0f, DesiredDistance - HitDistance);
		PushNeeded = FMath::Min(PushNeeded, CollisionConfig.MaxPushDistance);

		// Apply response type
		switch (CollisionConfig.ResponseType)
		{
		case EMGCameraCollisionResponse::PushForward:
			TargetCollisionPush = PushNeeded;
			break;

		case EMGCameraCollisionResponse::MoveUp:
			if (CachedSpringArm)
			{
				FVector SocketOffset = CachedSpringArm->SocketOffset;
				SocketOffset.Z += PushNeeded * 0.5f;
				CachedSpringArm->SocketOffset = SocketOffset;
			}
			TargetCollisionPush = 0.0f;
			break;

		case EMGCameraCollisionResponse::Adaptive:
			// Blend between push forward and move up based on hit angle
			FVector HitNormal = HitResult.Normal;
			float UpwardBias = FMath::Abs(FVector::DotProduct(HitNormal, FVector::UpVector));
			
			TargetCollisionPush = PushNeeded * (1.0f - UpwardBias);
			
			if (CachedSpringArm && UpwardBias > 0.3f)
			{
				FVector SocketOffset = CachedSpringArm->SocketOffset;
				SocketOffset.Z += PushNeeded * UpwardBias * 0.5f;
				CachedSpringArm->SocketOffset = SocketOffset;
			}
			break;

		default:
			TargetCollisionPush = 0.0f;
			break;
		}
	}
	else
	{
		// No collision - recover to normal position
		bIsAvoidingCollision = false;
		TargetCollisionPush = 0.0f;
	}

	// Smooth collision push interpolation
	CurrentCollisionPush = FMath::FInterpTo(CurrentCollisionPush, TargetCollisionPush, DeltaTime, 
		bIsAvoidingCollision ? 15.0f : CollisionConfig.RecoverySpeed);

	// Apply collision push to spring arm
	if (CachedSpringArm && CurrentCollisionPush > 0.0f)
	{
		CachedSpringArm->TargetArmLength = CurrentDistance - CurrentCollisionPush;
	}
}

void UMGDynamicCameraComponent::ApplyCameraSmoothing(float DeltaTime)
{
	if (!CachedSpringArm)
	{
		return;
	}

	// Calculate smoothing factor based on acceleration if enabled
	float PositionLag = SmoothingConfig.PositionLagSpeed;
	float RotationLag = SmoothingConfig.RotationLagSpeed;

	if (SmoothingConfig.bUseAccelerationCurve)
	{
		// Reduce lag during high acceleration for more responsive feel
		float AccelMagnitude = CurrentAcceleration.Size();
		float AccelAlpha = FMath::Clamp(AccelMagnitude / 1000.0f, 0.0f, 1.0f);
		float AccelInfluence = AccelAlpha * SmoothingConfig.AccelerationInfluence;

		PositionLag *= (1.0f + AccelInfluence);
		RotationLag *= (1.0f + AccelInfluence * 0.5f);
	}

	// Apply to spring arm
	CachedSpringArm->CameraLagSpeed = PositionLag;
	CachedSpringArm->CameraRotationLagSpeed = RotationLag;
	CachedSpringArm->bEnableCameraLag = true;
	CachedSpringArm->bEnableCameraRotationLag = true;

	// Apply look-ahead by adjusting spring arm target offset
	if (LookAheadConfig.bEnabled && !CurrentLookAheadTarget.IsNearlyZero())
	{
		FVector VehicleLocation = CachedVehiclePawn ? CachedVehiclePawn->GetActorLocation() : FVector::ZeroVector;
		FVector LookOffset = CurrentLookAheadTarget - VehicleLocation;
		
		// Convert to local space
		FRotator VehicleRotation = CachedVehiclePawn ? CachedVehiclePawn->GetActorRotation() : FRotator::ZeroRotator;
		FVector LocalOffset = VehicleRotation.UnrotateVector(LookOffset);
		
		// Apply subtle offset to spring arm target (don't follow fully, just bias toward it)
		CachedSpringArm->TargetOffset = LocalOffset * 0.2f;
	}
	else
	{
		CachedSpringArm->TargetOffset = FVector::ZeroVector;
	}
}

void UMGDynamicCameraComponent::ApplyRetroAesthetic(float DeltaTime)
{
	if (!RetroAestheticConfig.bEnabled || !CachedCamera)
	{
		return;
	}

	// Retro aesthetic effects are primarily post-process
	// This would integrate with MGCameraVFXComponent for actual rendering

	// Example: Apply subtle camera jitter for PS1 vertex wobble feel
	if (RetroAestheticConfig.VertexJitterIntensity > 0.0f)
	{
		UWorld* World = GetWorld();
		float Time = World ? World->GetTimeSeconds() : 0.0f;
		
		FVector Jitter;
		Jitter.X = FMath::PerlinNoise1D(Time * 20.0f) * RetroAestheticConfig.VertexJitterIntensity;
		Jitter.Y = FMath::PerlinNoise1D(Time * 20.0f + 100.0f) * RetroAestheticConfig.VertexJitterIntensity;
		Jitter.Z = FMath::PerlinNoise1D(Time * 20.0f + 200.0f) * RetroAestheticConfig.VertexJitterIntensity;

		// Apply as subtle offset (very small, just for aesthetic)
		FVector RelativeLocation = CachedCamera->GetRelativeLocation();
		CachedCamera->SetRelativeLocation(RelativeLocation + Jitter);
	}

	// Additional retro effects would be applied via post-process materials
	// and the MGCameraVFXComponent (chromatic aberration, dithering, etc.)
}

// ==========================================
// HELPER METHODS
// ==========================================

float UMGDynamicCameraComponent::CalculateTurnRate() const
{
	FVector AngularVelocity = GetVehicleAngularVelocity();
	return FMath::RadiansToDegrees(AngularVelocity.Z); // Yaw angular velocity
}

float UMGDynamicCameraComponent::GetVehicleSpeedKPH() const
{
	if (!CachedVehiclePawn)
	{
		return 0.0f;
	}

	UMGVehicleMovementComponent* Movement = CachedVehiclePawn->GetMGVehicleMovement();
	if (!Movement)
	{
		return 0.0f;
	}

	// Convert cm/s to KPH
	return Movement->GetForwardSpeed() * 0.036f;
}

FVector UMGDynamicCameraComponent::GetVehicleVelocity() const
{
	if (!CachedVehiclePawn)
	{
		return FVector::ZeroVector;
	}

	UMGVehicleMovementComponent* Movement = CachedVehiclePawn->GetMGVehicleMovement();
	if (!Movement)
	{
		return FVector::ZeroVector;
	}

	return Movement->GetVelocity();
}

FVector UMGDynamicCameraComponent::GetVehicleAngularVelocity() const
{
	if (!CachedVehiclePawn)
	{
		return FVector::ZeroVector;
	}

	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(CachedVehiclePawn->GetRootComponent());
	if (!RootPrimitive)
	{
		return FVector::ZeroVector;
	}

	return RootPrimitive->GetPhysicsAngularVelocityInDegrees();
}

bool UMGDynamicCameraComponent::IsVehicleDrifting() const
{
	if (!CachedVehiclePawn)
	{
		return false;
	}

	// Access runtime state to check drift status
	return CachedVehiclePawn->GetRuntimeState().bIsDrifting;
}

bool UMGDynamicCameraComponent::PerformCollisionTrace(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
	if (!GetWorld())
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	return GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		CollisionConfig.TraceChannel,
		QueryParams
	);
}

void UMGDynamicCameraComponent::ApplyBehaviorModePresets()
{
	// Apply preset configurations based on behavior mode
	switch (BehaviorMode)
	{
	case EMGCameraBehaviorMode::Classic:
		LookAheadConfig.DistanceMultiplier = 1.0f;
		SmoothingConfig.PositionLagSpeed = 8.0f;
		SmoothingConfig.RotationLagSpeed = 10.0f;
		TurnLeanConfig.MaxLeanAngle = 6.0f;
		break;

	case EMGCameraBehaviorMode::Aggressive:
		LookAheadConfig.DistanceMultiplier = 2.0f;
		LookAheadConfig.InterpSpeed = 5.0f;
		SmoothingConfig.PositionLagSpeed = 12.0f;
		SmoothingConfig.RotationLagSpeed = 15.0f;
		TurnLeanConfig.MaxLeanAngle = 10.0f;
		TurnLeanConfig.LeanInterpSpeed = 6.0f;
		break;

	case EMGCameraBehaviorMode::Cinematic:
		LookAheadConfig.DistanceMultiplier = 1.5f;
		SmoothingConfig.PositionLagSpeed = 4.0f;
		SmoothingConfig.RotationLagSpeed = 5.0f;
		TurnLeanConfig.MaxLeanAngle = 12.0f;
		SpeedAdaptiveConfig.MaxSpeedDistanceIncrease = 200.0f;
		break;

	case EMGCameraBehaviorMode::Drift:
		LookAheadConfig.DistanceMultiplier = 1.2f;
		SmoothingConfig.PositionLagSpeed = 6.0f;
		TurnLeanConfig.MaxLeanAngle = 15.0f;
		TurnLeanConfig.DriftLeanMultiplier = 2.0f;
		TurnLeanConfig.LateralOffset = 120.0f;
		break;

	case EMGCameraBehaviorMode::Arcade:
		LookAheadConfig.bEnabled = false;
		SmoothingConfig.PositionLagSpeed = 15.0f;
		SmoothingConfig.RotationLagSpeed = 20.0f;
		SmoothingConfig.bUseAccelerationCurve = false;
		TurnLeanConfig.MaxLeanAngle = 4.0f;
		SpeedAdaptiveConfig.bEnabled = false;
		break;

	case EMGCameraBehaviorMode::Custom:
		// No preset changes - use configured values
		break;
	}
}

float UMGDynamicCameraComponent::GetSpeedAlpha() const
{
	if (!SpeedAdaptiveConfig.bEnabled)
	{
		return 0.0f;
	}

	float SpeedKPH = GetVehicleSpeedKPH();
	return FMath::Clamp(SpeedKPH / SpeedAdaptiveConfig.MaxSpeedThresholdKPH, 0.0f, 1.0f);
}
