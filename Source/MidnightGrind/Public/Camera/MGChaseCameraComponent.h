// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGChaseCameraComponent.h
 * @brief Specialized chase camera for racing with advanced following behavior
 *
 * Enhanced chase camera with:
 * - Predictive positioning based on steering input
 * - Speed-based dynamic distance and FOV
 * - Smooth corner cutting behavior
 * - Drift-aware positioning
 * - Terrain-adaptive height adjustment
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGChaseCameraComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AMGVehiclePawn;

/**
 * Chase camera style
 */
UENUM(BlueprintType)
enum class EMGChaseCameraStyle : uint8
{
	/** Standard chase - smooth following */
	Standard,
	/** Tight chase - close to vehicle, responsive */
	Tight,
	/** Cinematic chase - wider angles, more dramatic */
	Cinematic,
	/** Action chase - dynamic repositioning for excitement */
	Action
};

/**
 * Chase camera configuration
 */
USTRUCT(BlueprintType)
struct FMGChaseCameraConfig
{
	GENERATED_BODY()

	/** Base distance behind vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDistance = 600.0f;

	/** Base height above vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseHeight = 200.0f;

	/** Camera pitch angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchAngle = -10.0f;

	/** Lateral offset for steering anticipation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteerAnticipationOffset = 100.0f;

	/** Speed at which to reach max distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedKPH = 250.0f;

	/** Additional distance at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedDistanceBonus = 150.0f;

	/** Additional height at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedHeightBonus = 75.0f;

	/** FOV increase at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedFOVBonus = 15.0f;

	/** Position lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositionLagSpeed = 8.0f;

	/** Rotation lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationLagSpeed = 10.0f;

	/** Enable terrain height adaptation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAdaptToTerrain = true;

	/** Terrain adaptation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TerrainAdaptSpeed = 5.0f;
};

/**
 * MGChaseCameraComponent
 * 
 * Specialized chase camera optimized for racing with predictive
 * positioning, speed-based adjustments, and drift awareness
 */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGChaseCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGChaseCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set chase camera style */
	UFUNCTION(BlueprintCallable, Category = "Camera|Chase")
	void SetCameraStyle(EMGChaseCameraStyle Style);

	/** Get current camera style */
	UFUNCTION(BlueprintPure, Category = "Camera|Chase")
	EMGChaseCameraStyle GetCameraStyle() const { return CameraStyle; }

	/** Configure chase camera parameters */
	UFUNCTION(BlueprintCallable, Category = "Camera|Chase")
	void SetChaseCameraConfig(const FMGChaseCameraConfig& Config);

	/** Get current chase camera config */
	UFUNCTION(BlueprintPure, Category = "Camera|Chase")
	FMGChaseCameraConfig GetChaseCameraConfig() const { return CameraConfig; }

	/** Enable/disable this camera */
	UFUNCTION(BlueprintCallable, Category = "Camera|Chase")
	void SetCameraEnabled(bool bEnabled);

	/** Is this camera currently active? */
	UFUNCTION(BlueprintPure, Category = "Camera|Chase")
	bool IsCameraEnabled() const { return bCameraEnabled; }

	// ==========================================
	// CAMERA STATE
	// ==========================================

	/** Get current effective distance */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	float GetCurrentDistance() const { return CurrentDistance; }

	/** Get current effective height */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	float GetCurrentHeight() const { return CurrentHeight; }

	/** Get current effective FOV */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	float GetCurrentFOV() const { return CurrentFOV; }

protected:
	// ==========================================
	// CONFIGURATION PROPERTIES
	// ==========================================

	/** Camera style */
	UPROPERTY(EditAnywhere, Category = "Camera|Style")
	EMGChaseCameraStyle CameraStyle = EMGChaseCameraStyle::Standard;

	/** Chase camera configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|Config")
	FMGChaseCameraConfig CameraConfig;

	/** Base FOV */
	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	float BaseFOV = 90.0f;

	// ==========================================
	// RUNTIME STATE
	// ==========================================

	/** Is camera enabled? */
	bool bCameraEnabled = true;

	/** Current effective distance */
	float CurrentDistance = 600.0f;

	/** Current effective height */
	float CurrentHeight = 200.0f;

	/** Current effective FOV */
	float CurrentFOV = 90.0f;

	/** Current steering anticipation offset */
	FVector CurrentSteerOffset = FVector::ZeroVector;

	/** Current terrain height adjustment */
	float CurrentTerrainAdjust = 0.0f;

	/** Cached spring arm */
	UPROPERTY()
	TObjectPtr<USpringArmComponent> CachedSpringArm;

	/** Cached camera */
	UPROPERTY()
	TObjectPtr<UCameraComponent> CachedCamera;

	/** Cached vehicle pawn */
	UPROPERTY()
	TObjectPtr<AMGVehiclePawn> CachedVehiclePawn;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Initialize component references */
	void InitializeReferences();

	/** Update camera distance based on speed */
	void UpdateSpeedBasedDistance(float DeltaTime);

	/** Update camera height based on speed and terrain */
	void UpdateHeightAdjustment(float DeltaTime);

	/** Update FOV based on speed */
	void UpdateSpeedBasedFOV(float DeltaTime);

	/** Update steering anticipation offset */
	void UpdateSteeringAnticipation(float DeltaTime);

	/** Update terrain adaptation */
	void UpdateTerrainAdaptation(float DeltaTime);

	/** Apply camera configuration to components */
	void ApplyCameraTransform();

	/** Get vehicle speed in KPH */
	float GetVehicleSpeedKPH() const;

	/** Get steering input (-1 to 1) */
	float GetSteeringInput() const;

	/** Check terrain height below camera */
	float GetTerrainHeight(const FVector& Location) const;

	/** Apply style presets */
	void ApplyStylePresets();
};
