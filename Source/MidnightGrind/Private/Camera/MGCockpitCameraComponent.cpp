// Copyright Midnight Grind. All Rights Reserved.

#include "Camera/MGCockpitCameraComponent.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UMGCockpitCameraComponent::UMGCockpitCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UMGCockpitCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeReferences();
	ApplyStylePresets();

	// Initialize velocity tracking
	if (CachedVehiclePawn)
	{
		PreviousVelocity = GetVehicleVelocity();
	}
}

void UMGCockpitCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bCameraEnabled || !CachedVehiclePawn || !CachedCamera)
	{
		return;
	}

	// Update camera subsystems
	CalculateGForces(DeltaTime);
	UpdateGForce(DeltaTime);
	UpdateHeadBob(DeltaTime);
	UpdateLookToApex(DeltaTime);
	UpdateCockpitShake(DeltaTime);

	// Apply final transform
	ApplyCameraTransform();

	// Update velocity tracking for next frame
	PreviousVelocity = GetVehicleVelocity();
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGCockpitCameraComponent::SetHeadMovementStyle(EMGHeadMovementStyle Style)
{
	HeadMovementStyle = Style;
	ApplyStylePresets();
}

void UMGCockpitCameraComponent::SetGForceConfig(const FMGGForceConfig& Config)
{
	GForceConfig = Config;
}

void UMGCockpitCameraComponent::SetHeadBobConfig(const FMGHeadBobConfig& Config)
{
	HeadBobConfig = Config;
}

void UMGCockpitCameraComponent::SetLookToApexConfig(const FMGLookToApexConfig& Config)
{
	LookToApexConfig = Config;
}

void UMGCockpitCameraComponent::SetCockpitShakeConfig(const FMGCockpitShakeConfig& Config)
{
	ShakeConfig = Config;
}

void UMGCockpitCameraComponent::SetCameraEnabled(bool bEnabled)
{
	bCameraEnabled = bEnabled;

	if (CachedCamera)
	{
		CachedCamera->SetActive(bEnabled);
	}
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void UMGCockpitCameraComponent::InitializeReferences()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	CachedVehiclePawn = Cast<AMGVehiclePawn>(Owner);
	if (!CachedVehiclePawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGCockpitCameraComponent: Owner is not a MGVehiclePawn"));
		return;
	}

	CachedCamera = CachedVehiclePawn->InteriorCamera;
	if (!CachedCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGCockpitCameraComponent: No InteriorCamera found"));
	}
}

void UMGCockpitCameraComponent::CalculateGForces(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	FVector CurrentVelocity = GetVehicleVelocity();
	FVector Acceleration = (CurrentVelocity - PreviousVelocity) / DeltaTime;

	if (!CachedVehiclePawn)
	{
		return;
	}

	// Transform acceleration to vehicle local space
	FVector LocalAcceleration = CachedVehiclePawn->GetActorRotation().UnrotateVector(Acceleration);

	// Convert to G-forces (1G = 980 cm/s²)
	const float GravityConstant = 980.0f;
	CurrentLongitudinalG = LocalAcceleration.X / GravityConstant; // Forward/back
	CurrentLateralG = LocalAcceleration.Y / GravityConstant;      // Left/right

	// Clamp to max G-force
	CurrentLongitudinalG = FMath::Clamp(CurrentLongitudinalG, -GForceConfig.MaxGForce, GForceConfig.MaxGForce);
	CurrentLateralG = FMath::Clamp(CurrentLateralG, -GForceConfig.MaxGForce, GForceConfig.MaxGForce);
}

void UMGCockpitCameraComponent::UpdateGForce(float DeltaTime)
{
	if (!GForceConfig.bEnabled)
	{
		TargetGForceOffset = FVector::ZeroVector;
		CurrentGForceOffset = FMath::VInterpTo(CurrentGForceOffset, FVector::ZeroVector, DeltaTime, GForceConfig.ResponseSpeed);
		return;
	}

	// Calculate offset based on G-forces
	// Longitudinal G (acceleration/braking) - head moves opposite to acceleration
	float LongitudinalOffset = -CurrentLongitudinalG * GForceConfig.LongitudinalShiftAmount;

	// Lateral G (cornering) - head moves opposite to lateral force
	float LateralOffset = -CurrentLateralG * GForceConfig.LateralShiftAmount;

	// Vertical component from combined G-forces (head compressed under high G)
	float TotalG = FMath::Sqrt(CurrentLongitudinalG * CurrentLongitudinalG + CurrentLateralG * CurrentLateralG);
	float VerticalOffset = -TotalG * GForceConfig.VerticalShiftAmount * 0.3f;

	TargetGForceOffset = FVector(LongitudinalOffset, LateralOffset, VerticalOffset);

	// Smooth interpolation
	CurrentGForceOffset = FMath::VInterpTo(CurrentGForceOffset, TargetGForceOffset, DeltaTime, GForceConfig.ResponseSpeed);
}

void UMGCockpitCameraComponent::UpdateHeadBob(float DeltaTime)
{
	if (!HeadBobConfig.bEnabled)
	{
		CurrentHeadBobOffset = FVector::ZeroVector;
		HeadBobPhase = 0.0f;
		return;
	}

	float SpeedKPH = GetVehicleSpeedKPH();

	// Only bob above minimum speed
	if (SpeedKPH < HeadBobConfig.MinSpeedKPH)
	{
		CurrentHeadBobOffset = FVector::ZeroVector;
		HeadBobPhase = 0.0f;
		return;
	}

	// Update bob phase based on speed
	float SpeedAlpha = FMath::Clamp((SpeedKPH - HeadBobConfig.MinSpeedKPH) / 100.0f, 0.0f, 1.0f);
	HeadBobPhase += DeltaTime * HeadBobConfig.BobFrequency * (1.0f + SpeedAlpha);

	// Calculate bob offsets using sine waves
	float VerticalBob = FMath::Sin(HeadBobPhase) * HeadBobConfig.VerticalBobAmount;
	float HorizontalBob = FMath::Sin(HeadBobPhase * 0.5f) * HeadBobConfig.HorizontalBobAmount;

	CurrentHeadBobOffset = FVector(0.0f, HorizontalBob, VerticalBob);
}

void UMGCockpitCameraComponent::UpdateLookToApex(float DeltaTime)
{
	if (!LookToApexConfig.bEnabled)
	{
		TargetLookAngle = 0.0f;
		CurrentLookAngle = FMath::FInterpTo(CurrentLookAngle, 0.0f, DeltaTime, LookToApexConfig.LookSpeed);
		return;
	}

	// Get angular velocity to detect cornering
	FVector AngularVelocity = GetVehicleAngularVelocity();
	float YawRate = AngularVelocity.Z; // Degrees per second

	// Check if turning hard enough
	if (FMath::Abs(YawRate) < LookToApexConfig.AngularVelocityThreshold)
	{
		TargetLookAngle = 0.0f;
	}
	else
	{
		// Calculate look angle based on turn rate
		float NormalizedYawRate = FMath::Clamp(YawRate / 90.0f, -1.0f, 1.0f);
		TargetLookAngle = NormalizedYawRate * LookToApexConfig.MaxLookAngle;
	}

	// Smooth interpolation
	CurrentLookAngle = FMath::FInterpTo(CurrentLookAngle, TargetLookAngle, DeltaTime, LookToApexConfig.LookSpeed);
}

void UMGCockpitCameraComponent::UpdateCockpitShake(float DeltaTime)
{
	if (!ShakeConfig.bEnabled)
	{
		CurrentShakeOffset = FVector::ZeroVector;
		return;
	}

	UWorld* World = GetWorld();
	float Time = World ? World->GetTimeSeconds() : 0.0f;

	// Engine shake based on RPM
	float RPMPercent = GetVehicleRPMPercent();
	float EngineShake = RPMPercent * ShakeConfig.EngineShakeAmount * ShakeConfig.RPMInfluence;

	// Road shake based on speed
	float SpeedKPH = GetVehicleSpeedKPH();
	float SpeedAlpha = FMath::Clamp(SpeedKPH / 150.0f, 0.0f, 1.0f);
	float RoadShake = SpeedAlpha * ShakeConfig.RoadShakeAmount;

	// Combine shakes with different frequencies
	float TotalShake = EngineShake + RoadShake;

	FVector ShakeOffset;
	ShakeOffset.X = FMath::PerlinNoise1D(Time * ShakeConfig.ShakeFrequency) * TotalShake * 0.5f;
	ShakeOffset.Y = FMath::PerlinNoise1D(Time * ShakeConfig.ShakeFrequency + 100.0f) * TotalShake;
	ShakeOffset.Z = FMath::PerlinNoise1D(Time * ShakeConfig.ShakeFrequency + 200.0f) * TotalShake * 0.7f;

	CurrentShakeOffset = ShakeOffset;
}

void UMGCockpitCameraComponent::ApplyCameraTransform()
{
	if (!CachedCamera || !CachedVehiclePawn)
	{
		return;
	}

	// Combine all offsets
	FVector TotalOffset = CockpitPosition;
	TotalOffset += CurrentGForceOffset;
	TotalOffset += CurrentHeadBobOffset;
	TotalOffset += CurrentShakeOffset;

	// Apply position
	CachedCamera->SetRelativeLocation(TotalOffset);

	// Apply look-to-apex rotation
	FRotator CameraRotation = FRotator::ZeroRotator;
	CameraRotation.Yaw = CurrentLookAngle;
	CachedCamera->SetRelativeRotation(CameraRotation);
}

// ==========================================
// HELPER METHODS
// ==========================================

float UMGCockpitCameraComponent::GetVehicleSpeedKPH() const
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

	return Movement->GetForwardSpeed() * 0.036f; // cm/s to KPH
}

float UMGCockpitCameraComponent::GetVehicleRPMPercent() const
{
	if (!CachedVehiclePawn)
	{
		return 0.0f;
	}

	return CachedVehiclePawn->GetRuntimeState().RPMPercent;
}

FVector UMGCockpitCameraComponent::GetVehicleVelocity() const
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

FVector UMGCockpitCameraComponent::GetVehicleAngularVelocity() const
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

void UMGCockpitCameraComponent::ApplyStylePresets()
{
	switch (HeadMovementStyle)
	{
	case EMGHeadMovementStyle::Stable:
		GForceConfig.bEnabled = false;
		HeadBobConfig.bEnabled = false;
		LookToApexConfig.bEnabled = false;
		ShakeConfig.EngineShakeAmount = 0.1f;
		ShakeConfig.RoadShakeAmount = 0.2f;
		break;

	case EMGHeadMovementStyle::Realistic:
		GForceConfig.bEnabled = true;
		GForceConfig.LongitudinalShiftAmount = 5.0f;
		GForceConfig.LateralShiftAmount = 8.0f;
		GForceConfig.VerticalShiftAmount = 3.0f;
		GForceConfig.ResponseSpeed = 3.0f;

		HeadBobConfig.bEnabled = true;
		HeadBobConfig.VerticalBobAmount = 1.5f;
		HeadBobConfig.HorizontalBobAmount = 1.0f;

		LookToApexConfig.bEnabled = true;
		LookToApexConfig.MaxLookAngle = 12.0f;

		ShakeConfig.EngineShakeAmount = 0.3f;
		ShakeConfig.RoadShakeAmount = 0.5f;
		break;

	case EMGHeadMovementStyle::Arcade:
		GForceConfig.bEnabled = true;
		GForceConfig.LongitudinalShiftAmount = 8.0f;
		GForceConfig.LateralShiftAmount = 12.0f;
		GForceConfig.VerticalShiftAmount = 5.0f;
		GForceConfig.ResponseSpeed = 5.0f;

		HeadBobConfig.bEnabled = true;
		HeadBobConfig.VerticalBobAmount = 2.5f;
		HeadBobConfig.HorizontalBobAmount = 2.0f;
		HeadBobConfig.BobFrequency = 3.0f;

		LookToApexConfig.bEnabled = true;
		LookToApexConfig.MaxLookAngle = 20.0f;
		LookToApexConfig.LookSpeed = 5.0f;

		ShakeConfig.EngineShakeAmount = 0.5f;
		ShakeConfig.RoadShakeAmount = 0.8f;
		break;

	case EMGHeadMovementStyle::Custom:
		// No preset changes - use configured values
		break;
	}
}
