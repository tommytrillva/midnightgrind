// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCameraVFXComponent.h
 * @brief Camera visual effects component for racing immersion
 *
 * @section Overview
 * UMGCameraVFXComponent manages all camera-based visual effects that enhance the
 * racing experience. This includes dynamic field of view changes, post-processing
 * effects, camera shake, and screen effects that respond to gameplay events.
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Field of View (FOV)**
 * FOV determines how wide the camera's view is, measured in degrees. A higher FOV
 * creates a "fish-eye" effect that conveys speed. This component dynamically
 * increases FOV as the vehicle goes faster, making high-speed driving feel more
 * intense. The base FOV (typically 90 degrees) expands up to MaxFOVIncrease at
 * maximum speed.
 *
 * **Post-Processing Effects**
 * Post-processing modifies the rendered image after the 3D scene is drawn:
 * - Motion Blur: Blurs fast-moving objects to simulate real camera behavior
 * - Chromatic Aberration: Separates color channels at screen edges (RGB fringing)
 * - Vignette: Darkens the screen corners to focus attention on the center
 * - Radial Blur: Creates "speed lines" emanating from a central point
 *
 * **Camera Shake**
 * Controlled camera movement that simulates impacts, terrain rumble, or high-speed
 * vibration. Different presets (Light, Medium, Heavy) suit different events like
 * collisions, near misses, or NOS activation. Continuous shake can simulate
 * constant high-speed vibration.
 *
 * **Drift Camera Effects**
 * When the player drifts, the camera can roll (tilt) and offset to emphasize the
 * sideways motion. This makes drifting feel more dynamic and cinematic.
 *
 * @section Architecture
 * The component attaches to any actor (typically the player's vehicle or camera
 * actor) and works by:
 * 1. Receiving updates about vehicle state (speed, drift angle, impacts)
 * 2. Interpolating effect intensities smoothly to avoid jarring transitions
 * 3. Applying effects to the camera and post-process settings each frame
 *
 * This component communicates with the vehicle physics system and can be controlled
 * by Blueprint or C++ game logic.
 *
 * @section UsageExamples Usage Examples
 *
 * **Basic Setup in C++:**
 * @code
 * // In your vehicle class header
 * UPROPERTY(VisibleAnywhere)
 * UMGCameraVFXComponent* CameraVFX;
 *
 * // In constructor
 * CameraVFX = CreateDefaultSubobject<UMGCameraVFXComponent>(TEXT("CameraVFX"));
 *
 * // In Tick, update speed effects based on vehicle speed
 * float SpeedKPH = GetVehicleSpeedKPH();
 * CameraVFX->UpdateSpeedEffects(SpeedKPH);
 * @endcode
 *
 * **Triggering Camera Shake on Collision:**
 * @code
 * void AMyVehicle::OnCollision(float ImpactForce)
 * {
 *     // Scale shake intensity by impact force
 *     if (ImpactForce > 5000.0f)
 *         CameraVFX->TriggerShake(EMGCameraShakeType::Heavy);
 *     else if (ImpactForce > 2000.0f)
 *         CameraVFX->TriggerShake(EMGCameraShakeType::Medium);
 *     else
 *         CameraVFX->TriggerShake(EMGCameraShakeType::Light);
 *
 *     // Also trigger impact flash
 *     CameraVFX->TriggerImpactFlashPreset(ImpactForce);
 * }
 * @endcode
 *
 * **Drift Camera Effects:**
 * @code
 * // Called from vehicle physics update
 * void AMyVehicle::UpdateDriftVisuals(float DeltaTime)
 * {
 *     float DriftAngle = CalculateDriftAngle(); // Degrees
 *     float DriftIntensity = FMath::Clamp(FMath::Abs(DriftAngle) / 45.0f, 0.0f, 1.0f);
 *
 *     CameraVFX->UpdateDriftEffects(DriftAngle, DriftIntensity);
 * }
 * @endcode
 *
 * **Slow Motion for Dramatic Moments:**
 * @code
 * // Near miss reward
 * void AMyVehicle::OnNearMiss()
 * {
 *     CameraVFX->StartSlowMotion(0.3f, 0.1f);  // 30% time scale, 0.1s transition
 *     CameraVFX->TriggerShake(EMGCameraShakeType::NearMiss);
 *
 *     // Schedule return to normal speed
 *     GetWorld()->GetTimerManager().SetTimer(SlowMoTimer, [this]()
 *     {
 *         CameraVFX->EndSlowMotion(0.2f);
 *     }, 0.5f, false);
 * }
 * @endcode
 *
 * **Blueprint Usage:**
 * In Blueprint, drag off the CameraVFX component reference and call:
 * - "Update Speed Effects" node with your vehicle's current speed
 * - "Trigger Shake" node with the appropriate shake type enum
 * - "Set Speed Effect Config" to customize thresholds and intensities
 *
 * @see UMGVFXSubsystem For global VFX management
 * @see UMGVehicleVFXComponent For vehicle-specific particle effects
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "MGCameraVFXComponent.generated.h"

class UCameraComponent;
class UPostProcessComponent;
class APlayerCameraManager;

/**
 * Camera shake preset
 */
UENUM(BlueprintType)
enum class EMGCameraShakeType : uint8
{
	None,
	/** Light shake for small impacts */
	Light,
	/** Medium shake for collisions */
	Medium,
	/** Heavy shake for major impacts */
	Heavy,
	/** Rumble shake for terrain */
	Rumble,
	/** NOS activation shake */
	NOS,
	/** Near miss shake */
	NearMiss,
	/** Finish line celebration */
	Victory,
	/** Constant high-speed vibration */
	SpeedVibration
};

/**
 * Speed effect configuration
 */
USTRUCT(BlueprintType)
struct FMGSpeedEffectConfig
{
	GENERATED_BODY()

	/** Speed threshold to start effects (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartThreshold = 100.0f;

	/** Speed for maximum effect intensity (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxThreshold = 250.0f;

	/** FOV increase at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFOVIncrease = 15.0f;

	/** Motion blur strength at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMotionBlur = 0.5f;

	/** Radial blur intensity at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRadialBlur = 0.3f;

	/** Chromatic aberration at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChromaticAberration = 0.5f;

	/** Vignette intensity at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxVignette = 0.4f;
};

/**
 * Drift camera effect configuration
 */
USTRUCT(BlueprintType)
struct FMGDriftCameraConfig
{
	GENERATED_BODY()

	/** Camera roll during drift (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRoll = 5.0f;

	/** Camera offset during drift */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DriftOffset = FVector(0.0f, 50.0f, 0.0f);

	/** Roll interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RollInterpSpeed = 3.0f;

	/** Offset interpolation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OffsetInterpSpeed = 4.0f;
};

/**
 * Impact flash configuration
 */
USTRUCT(BlueprintType)
struct FMGImpactFlashConfig
{
	GENERATED_BODY()

	/** Flash color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	/** Flash duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.1f;

	/** Flash intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 0.5f;

	/** Add chromatic aberration during flash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAddChromaticAberration = true;
};

/**
 * Camera VFX Component
 * Handles camera-specific visual effects for racing
 *
 * Features:
 * - Dynamic FOV based on speed
 * - Speed-based post-processing (blur, aberration)
 * - Multiple camera shake presets
 * - Drift camera roll and offset
 * - Impact flash and judder effects
 * - Replay slow-motion support
 */
UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGCameraVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGCameraVFXComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CAMERA SHAKE
	// ==========================================

	/** Trigger a preset camera shake */
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void TriggerShake(EMGCameraShakeType ShakeType, float Scale = 1.0f);

	/** Trigger custom shake */
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void TriggerCustomShake(float Intensity, float Duration, float Frequency = 20.0f);

	/** Start continuous shake (e.g., speed vibration) */
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void StartContinuousShake(EMGCameraShakeType ShakeType, float Scale = 1.0f);

	/** Stop continuous shake */
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void StopContinuousShake();

	/** Set shake intensity multiplier */
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void SetShakeIntensityMultiplier(float Multiplier);

	// ==========================================
	// SPEED EFFECTS
	// ==========================================

	/** Update speed-based effects */
	UFUNCTION(BlueprintCallable, Category = "Camera|Speed")
	void UpdateSpeedEffects(float SpeedKPH);

	/** Set speed effect configuration */
	UFUNCTION(BlueprintCallable, Category = "Camera|Speed")
	void SetSpeedEffectConfig(const FMGSpeedEffectConfig& Config);

	/** Enable/disable speed effects */
	UFUNCTION(BlueprintCallable, Category = "Camera|Speed")
	void SetSpeedEffectsEnabled(bool bEnabled);

	/** Get current speed effect intensity (0-1) */
	UFUNCTION(BlueprintPure, Category = "Camera|Speed")
	float GetSpeedEffectIntensity() const { return CurrentSpeedIntensity; }

	// ==========================================
	// DRIFT EFFECTS
	// ==========================================

	/** Update drift camera effects */
	UFUNCTION(BlueprintCallable, Category = "Camera|Drift")
	void UpdateDriftEffects(float DriftAngle, float DriftIntensity);

	/** Set drift camera configuration */
	UFUNCTION(BlueprintCallable, Category = "Camera|Drift")
	void SetDriftCameraConfig(const FMGDriftCameraConfig& Config);

	/** Enable/disable drift camera effects */
	UFUNCTION(BlueprintCallable, Category = "Camera|Drift")
	void SetDriftEffectsEnabled(bool bEnabled);

	// ==========================================
	// IMPACT EFFECTS
	// ==========================================

	/** Trigger impact flash */
	UFUNCTION(BlueprintCallable, Category = "Camera|Impact")
	void TriggerImpactFlash(const FMGImpactFlashConfig& Config);

	/** Trigger impact flash with preset */
	UFUNCTION(BlueprintCallable, Category = "Camera|Impact")
	void TriggerImpactFlashPreset(float ImpactForce);

	/** Trigger camera judder (brief freeze + snap) */
	UFUNCTION(BlueprintCallable, Category = "Camera|Impact")
	void TriggerJudder(float Intensity, float Duration = 0.05f);

	// ==========================================
	// POST PROCESS
	// ==========================================

	/** Set chromatic aberration override */
	UFUNCTION(BlueprintCallable, Category = "Camera|PostProcess")
	void SetChromaticAberration(float Intensity);

	/** Set vignette override */
	UFUNCTION(BlueprintCallable, Category = "Camera|PostProcess")
	void SetVignette(float Intensity);

	/** Set saturation */
	UFUNCTION(BlueprintCallable, Category = "Camera|PostProcess")
	void SetSaturation(float Saturation);

	/** Set color tint */
	UFUNCTION(BlueprintCallable, Category = "Camera|PostProcess")
	void SetColorTint(FLinearColor Tint);

	/** Reset all post-process overrides */
	UFUNCTION(BlueprintCallable, Category = "Camera|PostProcess")
	void ResetPostProcessOverrides();

	// ==========================================
	// SLOW MOTION
	// ==========================================

	/** Start slow motion effect */
	UFUNCTION(BlueprintCallable, Category = "Camera|SlowMo")
	void StartSlowMotion(float TimeDilation = 0.3f, float TransitionTime = 0.2f);

	/** End slow motion effect */
	UFUNCTION(BlueprintCallable, Category = "Camera|SlowMo")
	void EndSlowMotion(float TransitionTime = 0.2f);

	/** Is in slow motion */
	UFUNCTION(BlueprintPure, Category = "Camera|SlowMo")
	bool IsInSlowMotion() const { return bInSlowMotion; }

	// ==========================================
	// FOV
	// ==========================================

	/** Set base FOV */
	UFUNCTION(BlueprintCallable, Category = "Camera|FOV")
	void SetBaseFOV(float FOV);

	/** Get current FOV (base + speed effects) */
	UFUNCTION(BlueprintPure, Category = "Camera|FOV")
	float GetCurrentFOV() const;

	/** Add temporary FOV offset (e.g., NOS boost) */
	UFUNCTION(BlueprintCallable, Category = "Camera|FOV")
	void AddFOVOffset(float Offset, float Duration);

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Speed effect configuration */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Speed")
	FMGSpeedEffectConfig SpeedEffectConfig;

	/** Drift camera configuration */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Drift")
	FMGDriftCameraConfig DriftCameraConfig;

	/** Base FOV */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float BaseFOV = 90.0f;

	/** Shake intensity multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Shake")
	float ShakeIntensityMultiplier = 1.0f;

	/** Light shake parameters */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Shake")
	float LightShakeIntensity = 0.3f;

	/** Medium shake parameters */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Shake")
	float MediumShakeIntensity = 0.6f;

	/** Heavy shake parameters */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Shake")
	float HeavyShakeIntensity = 1.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current speed effect intensity (0-1) */
	float CurrentSpeedIntensity = 0.0f;

	/** Target speed effect intensity */
	float TargetSpeedIntensity = 0.0f;

	/** Current FOV offset from speed */
	float SpeedFOVOffset = 0.0f;

	/** Temporary FOV offset */
	float TempFOVOffset = 0.0f;
	float TempFOVOffsetDuration = 0.0f;
	float TempFOVOffsetTimer = 0.0f;

	/** Current drift roll */
	float CurrentDriftRoll = 0.0f;

	/** Target drift roll */
	float TargetDriftRoll = 0.0f;

	/** Current drift offset */
	FVector CurrentDriftOffset = FVector::ZeroVector;

	/** Target drift offset */
	FVector TargetDriftOffset = FVector::ZeroVector;

	/** Effects enabled flags */
	bool bSpeedEffectsEnabled = true;
	bool bDriftEffectsEnabled = true;

	/** Continuous shake active */
	bool bContinuousShakeActive = false;
	EMGCameraShakeType ContinuousShakeType = EMGCameraShakeType::None;
	float ContinuousShakeScale = 1.0f;

	/** Flash state */
	FLinearColor CurrentFlashColor = FLinearColor::Transparent;
	float FlashTimer = 0.0f;
	float FlashDuration = 0.0f;
	float FlashIntensity = 0.0f;
	bool bFlashChromaticAberration = false;

	/** Judder state */
	bool bJudderActive = false;
	float JudderTimer = 0.0f;
	float JudderDuration = 0.0f;
	float JudderIntensity = 0.0f;

	/** Slow motion state */
	bool bInSlowMotion = false;
	float TargetTimeDilation = 1.0f;
	float SlowMotionTransitionTime = 0.0f;

	/** Post-process overrides */
	float OverrideChromaticAberration = -1.0f; // -1 = no override
	float OverrideVignette = -1.0f;
	float OverrideSaturation = -1.0f;
	FLinearColor OverrideColorTint = FLinearColor(-1.0f, -1.0f, -1.0f, -1.0f);

	/** Shake accumulator for custom shake */
	FVector ShakeOffset = FVector::ZeroVector;
	float CustomShakeIntensity = 0.0f;
	float CustomShakeDuration = 0.0f;
	float CustomShakeTimer = 0.0f;
	float CustomShakeFrequency = 20.0f;

	// ==========================================
	// CACHED REFERENCES
	// ==========================================

	UPROPERTY()
	UCameraComponent* CachedCameraComponent = nullptr;

	UPROPERTY()
	APlayerCameraManager* CachedCameraManager = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Find and cache camera references */
	void CacheCameraReferences();

	/** Update FOV */
	void UpdateFOV(float DeltaTime);

	/** Update flash effect */
	void UpdateFlash(float DeltaTime);

	/** Update judder effect */
	void UpdateJudder(float DeltaTime);

	/** Update slow motion */
	void UpdateSlowMotion(float DeltaTime);

	/** Update continuous shake */
	void UpdateContinuousShake(float DeltaTime);

	/** Update custom shake */
	void UpdateCustomShake(float DeltaTime);

	/** Apply shake to camera */
	void ApplyShakeToCamera(const FVector& Offset, const FRotator& RotationOffset);

	/** Get shake parameters for type */
	void GetShakeParameters(EMGCameraShakeType Type, float& OutIntensity, float& OutDuration, float& OutFrequency) const;

	/** Apply post-process settings */
	void ApplyPostProcess();
};
