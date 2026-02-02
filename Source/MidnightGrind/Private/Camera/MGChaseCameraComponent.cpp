// Copyright Midnight Grind. All Rights Reserved.

#include "Camera/MGChaseCameraComponent.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

UMGChaseCameraComponent::UMGChaseCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	CurrentDistance = CameraConfig.BaseDistance;
	CurrentHeight = CameraConfig.BaseHeight;
	CurrentFOV = BaseFOV;
}

void UMGChaseCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeReferences();
	ApplyStylePresets();
}

void UMGChaseCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bCameraEnabled || !CachedVehiclePawn || !CachedSpringArm || !CachedCamera)
	{
		return;
	}

	// Update camera properties
	UpdateSpeedBasedDistance(DeltaTime);
	UpdateHeightAdjustment(DeltaTime);
	UpdateSpeedBasedFOV(DeltaTime);
	UpdateSteeringAnticipation(DeltaTime);
	UpdateTerrainAdaptation(DeltaTime);

	// Apply final transform
	ApplyCameraTransform();
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGChaseCameraComponent::SetCameraStyle(EMGChaseCameraStyle Style)
{
	CameraStyle = Style;
	ApplyStylePresets();
}

void UMGChaseCameraComponent::SetChaseCameraConfig(const FMGChaseCameraConfig& Config)
{
	CameraConfig = Config;
}

void UMGChaseCameraComponent::SetCameraEnabled(bool bEnabled)
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

void UMGChaseCameraComponent::InitializeReferences()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	CachedVehiclePawn = Cast<AMGVehiclePawn>(Owner);
	if (!CachedVehiclePawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGChaseCameraComponent: Owner is not a MGVehiclePawn"));
		return;
	}

	CachedSpringArm = CachedVehiclePawn->SpringArm;
	CachedCamera = CachedVehiclePawn->Camera;

	if (!CachedSpringArm)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGChaseCameraComponent: No SpringArm found"));
	}

	if (!CachedCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGChaseCameraComponent: No Camera found"));
	}
}

void UMGChaseCameraComponent::UpdateSpeedBasedDistance(float DeltaTime)
{
	float SpeedKPH = GetVehicleSpeedKPH();
	float SpeedAlpha = FMath::Clamp(SpeedKPH / CameraConfig.MaxSpeedKPH, 0.0f, 1.0f);

	// Calculate target distance with speed bonus
	float TargetDistance = CameraConfig.BaseDistance + (SpeedAlpha * CameraConfig.SpeedDistanceBonus);

	// Smooth interpolation
	CurrentDistance = FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaTime, CameraConfig.PositionLagSpeed * 0.5f);
}

void UMGChaseCameraComponent::UpdateHeightAdjustment(float DeltaTime)
{
	float SpeedKPH = GetVehicleSpeedKPH();
	float SpeedAlpha = FMath::Clamp(SpeedKPH / CameraConfig.MaxSpeedKPH, 0.0f, 1.0f);

	// Calculate target height with speed bonus
	float TargetHeight = CameraConfig.BaseHeight + (SpeedAlpha * CameraConfig.SpeedHeightBonus);

	// Add terrain adjustment if enabled
	if (CameraConfig.bAdaptToTerrain)
	{
		TargetHeight += CurrentTerrainAdjust;
	}

	// Smooth interpolation
	CurrentHeight = FMath::FInterpTo(CurrentHeight, TargetHeight, DeltaTime, CameraConfig.PositionLagSpeed * 0.5f);
}

void UMGChaseCameraComponent::UpdateSpeedBasedFOV(float DeltaTime)
{
	float SpeedKPH = GetVehicleSpeedKPH();
	float SpeedAlpha = FMath::Clamp(SpeedKPH / CameraConfig.MaxSpeedKPH, 0.0f, 1.0f);

	// Calculate target FOV with speed bonus
	float TargetFOV = BaseFOV + (SpeedAlpha * CameraConfig.SpeedFOVBonus);

	// Smooth interpolation
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 8.0f);
}

void UMGChaseCameraComponent::UpdateSteeringAnticipation(float DeltaTime)
{
	float SteeringInput = GetSteeringInput();

	// Calculate target lateral offset based on steering
	FVector RightVector = CachedVehiclePawn ? CachedVehiclePawn->GetActorRightVector() : FVector::RightVector;
	FVector TargetOffset = RightVector * (SteeringInput * CameraConfig.SteerAnticipationOffset);

	// Smooth interpolation
	CurrentSteerOffset = FMath::VInterpTo(CurrentSteerOffset, TargetOffset, DeltaTime, CameraConfig.RotationLagSpeed);
}

void UMGChaseCameraComponent::UpdateTerrainAdaptation(float DeltaTime)
{
	if (!CameraConfig.bAdaptToTerrain || !CachedVehiclePawn)
	{
		CurrentTerrainAdjust = 0.0f;
		return;
	}

	// Get desired camera position
	FVector VehicleLocation = CachedVehiclePawn->GetActorLocation();
	FVector CameraOffset = -CachedVehiclePawn->GetActorForwardVector() * CurrentDistance;
	FVector DesiredCameraLocation = VehicleLocation + CameraOffset;
	DesiredCameraLocation.Z += CurrentHeight;

	// Check terrain height at camera position
	float TerrainHeight = GetTerrainHeight(DesiredCameraLocation);
	
	// Calculate adjustment needed to stay above terrain
	float MinimumClearance = 150.0f; // Unreal units
	float RequiredHeight = TerrainHeight + MinimumClearance;
	float Adjustment = FMath::Max(0.0f, RequiredHeight - DesiredCameraLocation.Z);

	// Smooth interpolation
	float TargetAdjust = Adjustment;
	CurrentTerrainAdjust = FMath::FInterpTo(CurrentTerrainAdjust, TargetAdjust, DeltaTime, CameraConfig.TerrainAdaptSpeed);
}

void UMGChaseCameraComponent::ApplyCameraTransform()
{
	if (!CachedSpringArm || !CachedCamera)
	{
		return;
	}

	// Apply distance to spring arm
	CachedSpringArm->TargetArmLength = CurrentDistance;

	// Apply height and steering offset to socket offset
	FVector SocketOffset = FVector::ZeroVector;
	SocketOffset.Z = CurrentHeight;
	SocketOffset += CurrentSteerOffset;
	CachedSpringArm->SocketOffset = SocketOffset;

	// Apply pitch angle
	FRotator SpringArmRotation = CachedSpringArm->GetRelativeRotation();
	SpringArmRotation.Pitch = CameraConfig.PitchAngle;
	CachedSpringArm->SetRelativeRotation(SpringArmRotation);

	// Apply FOV
	CachedCamera->SetFieldOfView(CurrentFOV);

	// Apply lag settings
	CachedSpringArm->bEnableCameraLag = true;
	CachedSpringArm->bEnableCameraRotationLag = true;
	CachedSpringArm->CameraLagSpeed = CameraConfig.PositionLagSpeed;
	CachedSpringArm->CameraRotationLagSpeed = CameraConfig.RotationLagSpeed;
}

// ==========================================
// HELPER METHODS
// ==========================================

float UMGChaseCameraComponent::GetVehicleSpeedKPH() const
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

float UMGChaseCameraComponent::GetSteeringInput() const
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

	return Movement->GetSteeringInput();
}

float UMGChaseCameraComponent::GetTerrainHeight(const FVector& Location) const
{
	if (!GetWorld())
	{
		return Location.Z;
	}

	FVector TraceStart = Location + FVector(0.0f, 0.0f, 1000.0f);
	FVector TraceEnd = Location - FVector(0.0f, 0.0f, 3000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return HitResult.Location.Z;
	}

	return Location.Z;
}

void UMGChaseCameraComponent::ApplyStylePresets()
{
	switch (CameraStyle)
	{
	case EMGChaseCameraStyle::Standard:
		CameraConfig.BaseDistance = 600.0f;
		CameraConfig.BaseHeight = 200.0f;
		CameraConfig.PitchAngle = -10.0f;
		CameraConfig.PositionLagSpeed = 8.0f;
		CameraConfig.RotationLagSpeed = 10.0f;
		CameraConfig.SteerAnticipationOffset = 100.0f;
		CameraConfig.SpeedDistanceBonus = 150.0f;
		CameraConfig.SpeedHeightBonus = 75.0f;
		CameraConfig.SpeedFOVBonus = 15.0f;
		break;

	case EMGChaseCameraStyle::Tight:
		CameraConfig.BaseDistance = 400.0f;
		CameraConfig.BaseHeight = 150.0f;
		CameraConfig.PitchAngle = -8.0f;
		CameraConfig.PositionLagSpeed = 12.0f;
		CameraConfig.RotationLagSpeed = 15.0f;
		CameraConfig.SteerAnticipationOffset = 80.0f;
		CameraConfig.SpeedDistanceBonus = 100.0f;
		CameraConfig.SpeedHeightBonus = 50.0f;
		CameraConfig.SpeedFOVBonus = 12.0f;
		break;

	case EMGChaseCameraStyle::Cinematic:
		CameraConfig.BaseDistance = 800.0f;
		CameraConfig.BaseHeight = 250.0f;
		CameraConfig.PitchAngle = -12.0f;
		CameraConfig.PositionLagSpeed = 4.0f;
		CameraConfig.RotationLagSpeed = 5.0f;
		CameraConfig.SteerAnticipationOffset = 150.0f;
		CameraConfig.SpeedDistanceBonus = 200.0f;
		CameraConfig.SpeedHeightBonus = 100.0f;
		CameraConfig.SpeedFOVBonus = 20.0f;
		break;

	case EMGChaseCameraStyle::Action:
		CameraConfig.BaseDistance = 500.0f;
		CameraConfig.BaseHeight = 180.0f;
		CameraConfig.PitchAngle = -15.0f;
		CameraConfig.PositionLagSpeed = 10.0f;
		CameraConfig.RotationLagSpeed = 12.0f;
		CameraConfig.SteerAnticipationOffset = 120.0f;
		CameraConfig.SpeedDistanceBonus = 180.0f;
		CameraConfig.SpeedHeightBonus = 80.0f;
		CameraConfig.SpeedFOVBonus = 18.0f;
		break;
	}
}
