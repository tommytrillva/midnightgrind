// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGDynamicCameraComponent.h
 * @brief Advanced dynamic camera system for Midnight Grind racing
 *
 * This component provides sophisticated camera behavior including:
 * - Look-ahead prediction based on vehicle velocity
 * - Smooth camera interpolation with acceleration curves
 * - Collision avoidance with environment
 * - Dynamic FOV adjustments for speed sensation
 * - Turn-based camera lean and offset
 * - Speed-adaptive camera positioning
 * - Retro Y2K/PS1-PS2 aesthetic effects
 */

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MGDynamicCameraComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AMGVehiclePawn;

/**
 * Camera behavior mode
 */
UENUM(BlueprintType)
enum class EMGCameraBehaviorMode : uint8
{
	/** Classic chase camera with smooth following */
	Classic,
	/** Aggressive camera that anticipates turns */
	Aggressive,
	/** Cinematic camera with dramatic angles */
	Cinematic,
	/** Drift-focused camera with exaggerated lean */
	Drift,
	/** Fixed distance with minimal lag (arcade style) */
	Arcade,
	/** Custom behavior configured via parameters */
	Custom
};

/**
 * Camera collision response
 */
UENUM(BlueprintType)
enum class EMGCameraCollisionResponse : uint8
{
	/** No collision avoidance */
	None,
	/** Push camera forward when obstructed */
	PushForward,
	/** Move camera upward when obstructed */
	MoveUp,
	/** Blend between forward and up */
	Adaptive
};

/**
 * Look-ahead configuration
 */
USTRUCT(BlueprintType)
struct FMGCameraLookAheadConfig
{
	GENERATED_BODY()

	/** Enable look-ahead prediction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Look-ahead distance multiplier based on speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceMultiplier = 1.5f;

	/** Vertical offset for look-ahead target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VerticalOffset = 50.0f;

	/** Interpolation speed for look-ahead transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed = 3.0f;

	/** Maximum look-ahead distance in Unreal units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 1000.0f;

	/** Minimum speed (KPH) to enable look-ahead */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedKPH = 60.0f;
};

/**
 * Camera smoothing configuration
 */
USTRUCT(BlueprintType)
struct FMGCameraSmoothingConfig
{
	GENERATED_BODY()

	/** Position lag speed (lower = more lag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositionLagSpeed = 8.0f;

	/** Rotation lag speed (lower = more lag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationLagSpeed = 10.0f;

	/** Use acceleration-based smoothing curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseAccelerationCurve = true;

	/** Acceleration influence on smoothing (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationInfluence = 0.5f;

	/** Maximum camera velocity for smoothing limits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxVelocity = 3000.0f;
};

/**
 * Camera collision avoidance configuration
 */
USTRUCT(BlueprintType)
struct FMGCameraCollisionConfig
{
	GENERATED_BODY()

	/** Enable collision avoidance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Collision response strategy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCameraCollisionResponse ResponseType = EMGCameraCollisionResponse::Adaptive;

	/** Collision probe radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProbeRadius = 30.0f;

	/** Recovery speed when obstruction clears */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecoverySpeed = 5.0f;

	/** Maximum push distance when avoiding collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPushDistance = 300.0f;

	/** Collision trace channel to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Camera;
};

/**
 * Turn-based camera lean configuration
 */
USTRUCT(BlueprintType)
struct FMGCameraTurnLeanConfig
{
	GENERATED_BODY()

	/** Enable camera lean during turns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Maximum lean angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLeanAngle = 8.0f;

	/** Lateral offset during turns */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralOffset = 80.0f;

	/** Lean interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeanInterpSpeed = 4.0f;

	/** Angular velocity threshold to trigger lean */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngularVelocityThreshold = 30.0f;

	/** Exaggerate lean during drifts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftLeanMultiplier = 1.5f;
};

/**
 * Speed-adaptive positioning configuration
 */
USTRUCT(BlueprintType)
struct FMGCameraSpeedAdaptiveConfig
{
	GENERATED_BODY()

	/** Enable speed-adaptive positioning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** Additional distance at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedDistanceIncrease = 150.0f;

	/** Additional height at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedHeightIncrease = 50.0f;

	/** Speed threshold for max distance (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedThresholdKPH = 250.0f;

	/** Interpolation speed for position changes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PositionInterpSpeed = 2.0f;
};

/**
 * Retro aesthetic camera effects
 */
USTRUCT(BlueprintType)
struct FMGRetroAestheticConfig
{
	GENERATED_BODY()

	/** Enable retro visual effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/** PS1/PS2 style vertex jitter intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VertexJitterIntensity = 0.05f;

	/** Color palette reduction (0-1, 0=full color) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColorReduction = 0.2f;

	/** Texture dithering intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DitheringIntensity = 0.15f;

	/** Screen curvature (CRT effect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScreenCurvature = 0.3f;

	/** Scanline intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScanlineIntensity = 0.1f;

	/** Chromatic aberration for Y2K aesthetic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChromaticAberration = 0.4f;
};

/**
 * MGDynamicCameraComponent
 * 
 * Advanced racing camera system with:
 * - Predictive look-ahead based on velocity
 * - Smooth interpolation with acceleration curves
 * - Environmental collision avoidance
 * - Dynamic positioning adapting to speed
 * - Turn-based camera lean and offset
 * - Retro Y2K/PS1-PS2 aesthetic effects
 */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGDynamicCameraComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMGDynamicCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set camera behavior mode */
	UFUNCTION(BlueprintCallable, Category = "Camera|Behavior")
	void SetBehaviorMode(EMGCameraBehaviorMode Mode);

	/** Get current behavior mode */
	UFUNCTION(BlueprintPure, Category = "Camera|Behavior")
	EMGCameraBehaviorMode GetBehaviorMode() const { return BehaviorMode; }

	/** Configure look-ahead behavior */
	UFUNCTION(BlueprintCallable, Category = "Camera|LookAhead")
	void SetLookAheadConfig(const FMGCameraLookAheadConfig& Config);

	/** Configure smoothing behavior */
	UFUNCTION(BlueprintCallable, Category = "Camera|Smoothing")
	void SetSmoothingConfig(const FMGCameraSmoothingConfig& Config);

	/** Configure collision avoidance */
	UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
	void SetCollisionConfig(const FMGCameraCollisionConfig& Config);

	/** Configure turn lean behavior */
	UFUNCTION(BlueprintCallable, Category = "Camera|TurnLean")
	void SetTurnLeanConfig(const FMGCameraTurnLeanConfig& Config);

	/** Configure speed-adaptive positioning */
	UFUNCTION(BlueprintCallable, Category = "Camera|SpeedAdaptive")
	void SetSpeedAdaptiveConfig(const FMGCameraSpeedAdaptiveConfig& Config);

	/** Configure retro aesthetic effects */
	UFUNCTION(BlueprintCallable, Category = "Camera|Aesthetic")
	void SetRetroAestheticConfig(const FMGRetroAestheticConfig& Config);

	/** Reset to default camera configuration */
	UFUNCTION(BlueprintCallable, Category = "Camera|Configuration")
	void ResetToDefaults();

	// ==========================================
	// CAMERA CONTROL
	// ==========================================

	/** Set base camera distance from vehicle */
	UFUNCTION(BlueprintCallable, Category = "Camera|Position")
	void SetBaseCameraDistance(float Distance);

	/** Set base camera height above vehicle */
	UFUNCTION(BlueprintCallable, Category = "Camera|Position")
	void SetBaseCameraHeight(float Height);

	/** Get current camera distance (including speed adjustments) */
	UFUNCTION(BlueprintPure, Category = "Camera|Position")
	float GetCurrentCameraDistance() const { return CurrentDistance; }

	/** Get current camera height (including speed adjustments) */
	UFUNCTION(BlueprintPure, Category = "Camera|Position")
	float GetCurrentCameraHeight() const { return CurrentHeight; }

	/** Enable/disable look-ahead prediction */
	UFUNCTION(BlueprintCallable, Category = "Camera|LookAhead")
	void SetLookAheadEnabled(bool bEnabled);

	/** Enable/disable collision avoidance */
	UFUNCTION(BlueprintCallable, Category = "Camera|Collision")
	void SetCollisionAvoidanceEnabled(bool bEnabled);

	// ==========================================
	// CAMERA STATE
	// ==========================================

	/** Get look-ahead target position in world space */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	FVector GetLookAheadTarget() const { return CurrentLookAheadTarget; }

	/** Get current camera lean angle */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	float GetCurrentLeanAngle() const { return CurrentLeanAngle; }

	/** Is camera currently avoiding collision? */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	bool IsAvoidingCollision() const { return bIsAvoidingCollision; }

	/** Get collision push distance */
	UFUNCTION(BlueprintPure, Category = "Camera|State")
	float GetCollisionPushDistance() const { return CurrentCollisionPush; }

protected:
	// ==========================================
	// CONFIGURATION PROPERTIES
	// ==========================================

	/** Camera behavior mode */
	UPROPERTY(EditAnywhere, Category = "Camera|Behavior")
	EMGCameraBehaviorMode BehaviorMode = EMGCameraBehaviorMode::Classic;

	/** Look-ahead configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|LookAhead")
	FMGCameraLookAheadConfig LookAheadConfig;

	/** Smoothing configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|Smoothing")
	FMGCameraSmoothingConfig SmoothingConfig;

	/** Collision avoidance configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|Collision")
	FMGCameraCollisionConfig CollisionConfig;

	/** Turn lean configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|TurnLean")
	FMGCameraTurnLeanConfig TurnLeanConfig;

	/** Speed-adaptive configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|SpeedAdaptive")
	FMGCameraSpeedAdaptiveConfig SpeedAdaptiveConfig;

	/** Retro aesthetic configuration */
	UPROPERTY(EditAnywhere, Category = "Camera|Aesthetic")
	FMGRetroAestheticConfig RetroAestheticConfig;

	/** Base camera distance from vehicle */
	UPROPERTY(EditAnywhere, Category = "Camera|Position")
	float BaseCameraDistance = 600.0f;

	/** Base camera height above vehicle */
	UPROPERTY(EditAnywhere, Category = "Camera|Position")
	float BaseCameraHeight = 200.0f;

	/** Pitch angle offset (degrees) */
	UPROPERTY(EditAnywhere, Category = "Camera|Position")
	float BasePitchOffset = -10.0f;

	// ==========================================
	// RUNTIME STATE
	// ==========================================

	/** Current camera distance (base + speed adjustments) */
	float CurrentDistance = 600.0f;

	/** Current camera height (base + speed adjustments) */
	float CurrentHeight = 200.0f;

	/** Current look-ahead target position */
	FVector CurrentLookAheadTarget = FVector::ZeroVector;

	/** Previous look-ahead target for smoothing */
	FVector PreviousLookAheadTarget = FVector::ZeroVector;

	/** Current camera lean angle (degrees) */
	float CurrentLeanAngle = 0.0f;

	/** Target lean angle for interpolation */
	float TargetLeanAngle = 0.0f;

	/** Current lateral offset from turns */
	FVector CurrentLateralOffset = FVector::ZeroVector;

	/** Target lateral offset for interpolation */
	FVector TargetLateralOffset = FVector::ZeroVector;

	/** Current collision push distance */
	float CurrentCollisionPush = 0.0f;

	/** Target collision push for interpolation */
	float TargetCollisionPush = 0.0f;

	/** Is currently avoiding collision */
	bool bIsAvoidingCollision = false;

	/** Previous vehicle velocity for acceleration calculation */
	FVector PreviousVelocity = FVector::ZeroVector;

	/** Current vehicle acceleration */
	FVector CurrentAcceleration = FVector::ZeroVector;

	/** Cached spring arm component */
	UPROPERTY()
	TObjectPtr<USpringArmComponent> CachedSpringArm;

	/** Cached camera component */
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

	/** Update look-ahead target position */
	void UpdateLookAheadTarget(float DeltaTime);

	/** Update camera position based on speed */
	void UpdateSpeedAdaptivePosition(float DeltaTime);

	/** Update camera lean based on turn rate */
	void UpdateTurnLean(float DeltaTime);

	/** Update collision avoidance */
	void UpdateCollisionAvoidance(float DeltaTime);

	/** Apply smoothing to camera movement */
	void ApplyCameraSmoothing(float DeltaTime);

	/** Apply retro aesthetic effects */
	void ApplyRetroAesthetic(float DeltaTime);

	/** Calculate turn rate from angular velocity */
	float CalculateTurnRate() const;

	/** Get vehicle speed in KPH */
	float GetVehicleSpeedKPH() const;

	/** Get vehicle velocity */
	FVector GetVehicleVelocity() const;

	/** Get vehicle angular velocity */
	FVector GetVehicleAngularVelocity() const;

	/** Check if vehicle is drifting */
	bool IsVehicleDrifting() const;

	/** Perform collision trace for avoidance */
	bool PerformCollisionTrace(const FVector& Start, const FVector& End, FHitResult& OutHit);

	/** Apply behavior mode presets */
	void ApplyBehaviorModePresets();

	/** Get speed alpha (0-1) based on current speed */
	float GetSpeedAlpha() const;
};
