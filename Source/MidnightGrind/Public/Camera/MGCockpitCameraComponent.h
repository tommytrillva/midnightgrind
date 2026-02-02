// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGCockpitCameraComponent.h
 * @brief Cockpit/interior camera for immersive first-person racing
 *
 * Features:
 * - Head bob and shake based on vehicle movement
 * - G-force simulation (camera shifts during acceleration/braking)
 * - Steering wheel animation sync
 * - Look-to-apex during corners
 * - Realistic cockpit motion for immersion
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGCockpitCameraComponent.generated.h"

class UCameraComponent;
class AMGVehiclePawn;

/**
 * Head movement style
 */
UENUM(BlueprintType)
enum class EMGHeadMovementStyle : uint8
{
	/** Minimal movement - stable view */
	Stable,
	/** Realistic movement with physics */
	Realistic,
	/** Exaggerated movement for arcade feel */
	Arcade,
	/** Custom configured movement */
	Custom
};

/**
 * G-force simulation configuration
 */
USTRUCT(BlueprintType)
struct FMGGForceConfig
{
	GENERATED_BODY()

	/** Enable G-force camera effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Forward/back shift during acceleration/braking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalShiftAmount = 5.0f;

	/** Lateral shift during cornering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralShiftAmount = 8.0f;

	/** Vertical shift during bumps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalShiftAmount = 3.0f;

	/** G-force response speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ResponseSpeed = 3.0f;

	/** Maximum G-force effect magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxGForce = 3.0f;
};

/**
 * Head bob configuration
 */
USTRUCT(BlueprintType)
struct FMGHeadBobConfig
{
	GENERATED_BODY()

	/** Enable head bob */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Vertical bob amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalBobAmount = 1.5f;

	/** Horizontal bob amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalBobAmount = 1.0f;

	/** Bob frequency based on speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BobFrequency = 2.0f;

	/** Speed threshold to start bobbing (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedKPH = 20.0f;
};

/**
 * Look-to-apex configuration
 */
USTRUCT(BlueprintType)
struct FMGLookToApexConfig
{
	GENERATED_BODY()

	/** Enable look-to-apex during corners */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Maximum look angle (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLookAngle = 15.0f;

	/** Look interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookSpeed = 3.0f;

	/** Angular velocity threshold to trigger look */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngularVelocityThreshold = 30.0f;
};

/**
 * Cockpit shake configuration
 */
USTRUCT(BlueprintType)
struct FMGCockpitShakeConfig
{
	GENERATED_BODY()

	/** Enable cockpit shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Shake from engine vibration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineShakeAmount = 0.3f;

	/** Shake from road surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoadShakeAmount = 0.5f;

	/** Shake frequency */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShakeFrequency = 25.0f;

	/** RPM influence on shake (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPMInfluence = 0.6f;
};

/**
 * MGCockpitCameraComponent
 * 
 * Immersive cockpit camera with:
 * - G-force simulation for realistic head movement
 * - Speed-based head bob
 * - Look-to-apex during cornering
 * - Engine and road surface shake
 * - Realistic first-person racing experience
 */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGCockpitCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGCockpitCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set head movement style */
	UFUNCTION(BlueprintCallable, Category = "Camera|Cockpit")
	void SetHeadMovementStyle(EMGHeadMovementStyle Style);

	/** Get current head movement style */
	UFUNCTION(BlueprintPure, Category = "Camera|Cockpit")
	EMGHeadMovementStyle GetHeadMovementStyle() const { return HeadMovementStyle; }

	/** Configure G-force effects */
	UFUNCTION(BlueprintCallable, Category = "Camera|GForce")
	void SetGForceConfig(const FMGGForceConfig& Config);

	/** Configure head bob */
	UFUNCTION(BlueprintCallable, Category = "Camera|HeadBob")
	void SetHeadBobConfig(const FMGHeadBobConfig& Config);

	/** Configure look-to-apex */
	UFUNCTION(BlueprintCallable, Category = "Camera|LookToApex")
	void SetLookToApexConfig(const FMGLookToApexConfig& Config);

	/** Configure cockpit shake */
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void SetCockpitShakeConfig(const FMGCockpitShakeConfig& Config);

	/** Enable/disable this camera */
	UFUNCTION(BlueprintCallable, Category = "Camera|Cockpit")
	void SetCameraEnabled(bool bEnabled);

	/** Is this camera currently active? */
	UFUNCTION(BlueprintPure, Category = "Camera|Cockpit")
	bool IsCameraEnabled() const { return bCameraEnabled; }

	// ==========================================
	// CAMERA STATE
	// ==========================================

	/** Get current G-force offset */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	FVector GetGForceOffset() const { return CurrentGForceOffset; }

	/** Get current head bob offset */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	FVector GetHeadBobOffset() const { return CurrentHeadBobOffset; }

	/** Get current look angle */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	float GetLookAngle() const { return CurrentLookAngle; }

protected:
	// ==========================================
	// CONFIGURATION PROPERTIES
	// ==========================================

	/** Head movement style */
	UPROPERTY(EditAnywhere, Category = "Camera|Style")
	EMGHeadMovementStyle HeadMovementStyle = EMGHeadMovementStyle::Realistic;

	/** G-force configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|GForce")
	FMGGForceConfig GForceConfig;

	/** Head bob configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|HeadBob")
	FMGHeadBobConfig HeadBobConfig;

	/** Look-to-apex configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|LookToApex")
	FMGLookToApexConfig LookToApexConfig;

	/** Cockpit shake configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|Shake")
	FMGCockpitShakeConfig ShakeConfig;

	/** Cockpit camera position offset from vehicle origin */
	UPROPERTY(EditAnywhere, Category = "Camera|Position")
	FVector CockpitPosition = FVector(150.0f, 0.0f, 120.0f);

	// ==========================================
	// RUNTIME STATE
	// ==========================================

	/** Is camera enabled? */
	bool bCameraEnabled = true;

	/** Current G-force offset */
	FVector CurrentGForceOffset = FVector::ZeroVector;

	/** Target G-force offset */
	FVector TargetGForceOffset = FVector::ZeroVector;

	/** Current head bob offset */
	FVector CurrentHeadBobOffset = FVector::ZeroVector;

	/** Head bob phase (for sine wave) */
	float HeadBobPhase = 0.0f;

	/** Current look angle from look-to-apex */
	float CurrentLookAngle = 0.0f;

	/** Target look angle */
	float TargetLookAngle = 0.0f;

	/** Current shake offset */
	FVector CurrentShakeOffset = FVector::ZeroVector;

	/** Previous vehicle velocity for acceleration calc */
	FVector PreviousVelocity = FVector::ZeroVector;

	/** Current lateral G-force */
	float CurrentLateralG = 0.0f;

	/** Current longitudinal G-force */
	float CurrentLongitudinalG = 0.0f;

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

	/** Update G-force simulation */
	void UpdateGForce(float DeltaTime);

	/** Update head bob */
	void UpdateHeadBob(float DeltaTime);

	/** Update look-to-apex */
	void UpdateLookToApex(float DeltaTime);

	/** Update cockpit shake */
	void UpdateCockpitShake(float DeltaTime);

	/** Apply all effects to camera */
	void ApplyCameraTransform();

	/** Get vehicle speed in KPH */
	float GetVehicleSpeedKPH() const;

	/** Get vehicle RPM percentage (0-1) */
	float GetVehicleRPMPercent() const;

	/** Get vehicle velocity */
	FVector GetVehicleVelocity() const;

	/** Get vehicle angular velocity */
	FVector GetVehicleAngularVelocity() const;

	/** Calculate G-forces */
	void CalculateGForces(float DeltaTime);

	/** Apply style presets */
	void ApplyStylePresets();
};
