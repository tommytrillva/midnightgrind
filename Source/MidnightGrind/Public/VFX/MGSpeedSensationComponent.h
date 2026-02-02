// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGSpeedSensationComponent.h
 * @brief Unified Speed Sensation System - Makes speed feel visceral and intense
 *
 * =============================================================================
 * SPEED SENSATION SYSTEM - "Make Speed Feel FAST"
 * =============================================================================
 *
 * @section Overview
 * The MGSpeedSensationComponent is the master orchestrator for all visual and
 * audio feedback that communicates SPEED to the player. This system makes the
 * difference between "moving fast" and "FEELING fast."
 *
 * It dynamically scales multiple effects based on vehicle velocity, creating
 * an adrenaline-pumping sensation that matches the illegal street racing vibe
 * of Midnight Grind while maintaining visual clarity for gameplay.
 *
 * @section WhatItDoes What This Component Does
 *
 * When you're driving at 250 KPH down a neon-lit Tokyo street:
 * - Camera FOV widens (tunnel vision effect)
 * - Screen gets subtle radial blur emanating from the vanishing point
 * - Speed lines/motion trails streak past your periphery
 * - Chromatic aberration creates RGB fringing at screen edges
 * - Motion blur intensifies on fast-moving objects
 * - Subtle screen shake adds road vibration feel
 * - Post-process vignette focuses attention on the road ahead
 * - Particle effects trail behind the vehicle
 * - Audio pitch shifts (doppler) as you pass objects
 * - HUD elements respond to G-forces
 *
 * All of these scale smoothly with velocity, creating a cohesive sensation.
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **1. Velocity-Based Scaling**
 * Most effects use a 0-1 intensity curve based on speed:
 * - Below MinSpeedThreshold (e.g., 80 KPH): No effects
 * - At MaxSpeedThreshold (e.g., 300 KPH): Full intensity
 * - In between: Smooth interpolation using curves
 *
 * **2. Contextual Modifiers**
 * Base intensity is modified by context:
 * - Proximity to objects: More intense near walls/traffic
 * - Tunnel/enclosed space: Amplified effects
 * - Time of day: Stronger effects at night (neon trails)
 * - Weather: Rain adds extra motion blur
 * - Boost/NOS: Temporary intensity spike
 *
 * **3. Gameplay Clarity Balance**
 * While effects should feel intense, players must still:
 * - See the road ahead clearly
 * - Identify upcoming corners
 * - Spot traffic and obstacles
 * - Read UI/HUD elements
 *
 * The system includes "clarity zones" where effects are reduced.
 *
 * **4. Effect Categories**
 * 
 * **Camera Effects** (via MGCameraVFXComponent):
 * - FOV: Base 90° → 105° at max speed (configurable)
 * - Subtle vibration shake (high-speed road rumble)
 * - Smooth interpolation to avoid jarring transitions
 *
 * **Post-Process Effects** (via MGPostProcessSubsystem):
 * - Motion Blur: Directional blur based on camera velocity
 * - Radial Blur: Emanates from screen center/vanishing point
 * - Chromatic Aberration: RGB channel separation at edges
 * - Vignette: Darkens periphery to focus attention
 * - Bloom: Slight increase to enhance neon lights
 *
 * **Screen Effects** (via MGScreenEffectSubsystem):
 * - Speed Lines: Animated streaks in chosen style (Radial/Anime/Neon)
 * - Tunnel Effect: Darkened edges with speed streaks
 * - Digital overlays: Cyberpunk-style HUD distortion
 *
 * **Particle Effects** (via MGVehicleVFXComponent):
 * - Speed trails: Ribbon particles flowing from vehicle
 * - Air distortion: Heat-haze-like warping behind car
 * - Tire vortex: Spiral effects on wheels at high speed
 *
 * **Audio Effects** (integrates with MGEngineAudioComponent):
 * - Wind intensity scales with speed
 * - Doppler shift on passed objects
 * - Engine audio compression/reverb in tunnels
 * - Tire squeal pitch modulation
 *
 * @section EffectProfiles Effect Profiles
 *
 * Preset configurations for different visual styles:
 *
 * **Modern** (Default):
 * - Subtle FOV increase (90° → 100°)
 * - Light motion blur
 * - Minimal chromatic aberration
 * - Clean, readable aesthetic
 *
 * **Arcade**:
 * - Aggressive FOV change (90° → 110°)
 * - Heavy radial blur
 * - Anime-style speed lines
 * - Colorful particle trails
 *
 * **Simulation**:
 * - Minimal FOV change (90° → 95°)
 * - Realistic motion blur only
 * - No speed lines
 * - Natural camera shake
 *
 * **Y2K/Cyberpunk**:
 * - Moderate FOV (90° → 105°)
 * - Heavy chromatic aberration
 * - Neon-colored speed lines
 * - Digital glitch effects at extreme speed
 * - Scanline overlays
 *
 * **Cinematic**:
 * - Dramatic FOV (90° → 108°)
 * - Film-like motion blur
 * - Heavy vignette
 * - Lens flare/bloom enhancement
 *
 * @section Integration How It Works (Technical)
 *
 * Each frame (in TickComponent):
 * 1. Calculate current velocity magnitude
 * 2. Determine speed intensity (0-1 normalized)
 * 3. Apply contextual modifiers (proximity, tunnel, boost)
 * 4. Calculate per-effect intensities using curves
 * 5. Update camera effects via CameraVFX component
 * 6. Update post-process via PostProcessSubsystem
 * 7. Update screen effects via ScreenEffectSubsystem
 * 8. Spawn/update particle trails via VFX subsystem
 * 9. Notify audio system of speed intensity
 *
 * @section Usage Usage Examples
 *
 * **Basic Setup (Automatic):**
 * @code
 * // In your vehicle Blueprint or C++ class
 * // The component auto-detects and connects to other systems
 * 
 * UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Speed FX")
 * UMGSpeedSensationComponent* SpeedSensation;
 *
 * // Constructor
 * SpeedSensation = CreateDefaultSubobject<UMGSpeedSensationComponent>(TEXT("SpeedSensation"));
 * 
 * // That's it! The component will automatically:
 * // - Find the camera component
 * // - Connect to the post-process subsystem
 * // - Integrate with screen effects
 * // - Update all effects based on vehicle speed
 * @endcode
 *
 * **Choosing a Profile:**
 * @code
 * // In BeginPlay or from settings menu
 * SpeedSensation->SetEffectProfile(EMGSpeedSensationProfile::Y2KCyberpunk);
 * 
 * // Or customize individual categories
 * SpeedSensation->SetCategoryIntensity(EMGSpeedEffectCategory::CameraFOV, 0.7f);
 * SpeedSensation->SetCategoryIntensity(EMGSpeedEffectCategory::ChromaticAberration, 1.2f);
 * @endcode
 *
 * **Temporary Intensity Boost (e.g., NOS activation):**
 * @code
 * void AMyVehicle::ActivateNOS()
 * {
 *     // Temporarily amplify all speed effects
 *     SpeedSensation->BoostIntensity(1.5f, 2.0f);  // 1.5x multiplier for 2 seconds
 *     
 *     // Or boost specific effects only
 *     FMGSpeedBoostParams BoostParams;
 *     BoostParams.FOVMultiplier = 1.3f;
 *     BoostParams.MotionBlurMultiplier = 2.0f;
 *     BoostParams.ParticleTrailMultiplier = 3.0f;
 *     BoostParams.Duration = 2.0f;
 *     SpeedSensation->ApplySpeedBoost(BoostParams);
 * }
 * @endcode
 *
 * **Proximity Intensity (near-miss effect):**
 * @code
 * // When player narrowly avoids a collision
 * void AMyVehicle::OnNearMiss(float Distance)
 * {
 *     // Calculate intensity based on how close it was
 *     float Intensity = 1.0f - (Distance / NearMissThreshold);
 *     
 *     // Trigger brief intensity spike
 *     SpeedSensation->TriggerProximityPulse(Intensity, 0.5f);
 * }
 * @endcode
 *
 * **Tunnel/Environment Detection:**
 * @code
 * // When entering a tunnel (detected via trigger volume)
 * void AMyVehicle::OnEnterTunnel()
 * {
 *     SpeedSensation->SetEnvironmentMultiplier(1.5f);  // Amplify effects in tunnel
 * }
 *
 * void AMyVehicle::OnExitTunnel()
 * {
 *     SpeedSensation->SetEnvironmentMultiplier(1.0f);  // Back to normal
 * }
 * @endcode
 *
 * **Accessibility/Performance:**
 * @code
 * // Reduce all effects for motion-sensitive players
 * SpeedSensation->SetGlobalIntensityScale(0.5f);
 * 
 * // Disable specific categories
 * SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::ScreenShake, false);
 * SpeedSensation->SetCategoryEnabled(EMGSpeedEffectCategory::ChromaticAberration, false);
 * 
 * // Or use a low-intensity profile
 * SpeedSensation->SetEffectProfile(EMGSpeedSensationProfile::Simulation);
 * @endcode
 *
 * @section Performance Performance Considerations
 *
 * - Effects are evaluated every frame but most are cheap screen-space operations
 * - Particle systems use pooling via VFXSubsystem
 * - Camera shake uses simple offset math
 * - Post-process is GPU-bound; profile for target platform
 * - Lower-end hardware can use Simulation profile or reduced global scale
 *
 * @section BestPractices Best Practices
 *
 * 1. **Start Subtle**: Default intensities should be noticeable but not overwhelming
 * 2. **Player Control**: Expose intensity sliders in settings
 * 3. **Context Aware**: Amplify in tunnels, reduce in technical sections
 * 4. **Boost = Reward**: Use intensity spikes as positive feedback
 * 5. **Test with Controllers**: Wheel players can handle more; controller players may need less
 *
 * @see UMGCameraVFXComponent For camera-specific effects
 * @see UMGPostProcessSubsystem For post-process pipeline
 * @see UMGScreenEffectSubsystem For screen-space overlays
 * @see UMGVehicleVFXComponent For particle trails
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "MGSpeedSensationComponent.generated.h"

// Forward declarations
class UMGCameraVFXComponent;
class UMGPostProcessSubsystem;
class UMGScreenEffectSubsystem;
class UMGVehicleVFXComponent;
class UMGEngineAudioComponent;
class UCurveFloat;

/**
 * Speed sensation profile presets
 */
UENUM(BlueprintType)
enum class EMGSpeedSensationProfile : uint8
{
	Modern         UMETA(DisplayName = "Modern (Balanced)"),
	Arcade         UMETA(DisplayName = "Arcade (Intense)"),
	Simulation     UMETA(DisplayName = "Simulation (Subtle)"),
	Y2KCyberpunk   UMETA(DisplayName = "Y2K Cyberpunk"),
	Cinematic      UMETA(DisplayName = "Cinematic"),
	Custom         UMETA(DisplayName = "Custom")
};

/**
 * Speed effect categories
 */
UENUM(BlueprintType)
enum class EMGSpeedEffectCategory : uint8
{
	CameraFOV              UMETA(DisplayName = "Camera FOV"),
	ScreenShake            UMETA(DisplayName = "Screen Shake"),
	MotionBlur             UMETA(DisplayName = "Motion Blur"),
	RadialBlur             UMETA(DisplayName = "Radial Blur"),
	SpeedLines             UMETA(DisplayName = "Speed Lines"),
	ChromaticAberration    UMETA(DisplayName = "Chromatic Aberration"),
	Vignette               UMETA(DisplayName = "Vignette"),
	ParticleTrails         UMETA(DisplayName = "Particle Trails"),
	AudioDoppler           UMETA(DisplayName = "Audio Doppler"),
	HUDDistortion          UMETA(DisplayName = "HUD Distortion")
};

/**
 * Speed intensity curve type
 */
UENUM(BlueprintType)
enum class EMGSpeedCurveType : uint8
{
	Linear         UMETA(DisplayName = "Linear"),
	EaseIn         UMETA(DisplayName = "Ease In (Slow Start)"),
	EaseOut        UMETA(DisplayName = "Ease Out (Fast Start)"),
	EaseInOut      UMETA(DisplayName = "Ease In-Out (S-Curve)"),
	Exponential    UMETA(DisplayName = "Exponential"),
	Custom         UMETA(DisplayName = "Custom Curve")
};

/**
 * Camera FOV settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedFOVSettings
{
	GENERATED_BODY()

	/** Base FOV at low speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "60.0", ClampMax = "120.0"))
	float BaseFOV = 90.0f;

	/** Maximum FOV increase at top speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "0.0", ClampMax = "40.0"))
	float MaxFOVIncrease = 15.0f;

	/** How quickly FOV changes (higher = more responsive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float FOVInterpSpeed = 3.0f;

	/** Curve defining FOV response to speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
	EMGSpeedCurveType FOVCurve = EMGSpeedCurveType::EaseOut;

	/** Custom curve asset (if FOVCurve == Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV", meta = (EditCondition = "FOVCurve == EMGSpeedCurveType::Custom"))
	UCurveFloat* CustomFOVCurve = nullptr;
};

/**
 * Screen shake settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedShakeSettings
{
	GENERATED_BODY()

	/** Enable high-speed vibration shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	bool bEnableSpeedShake = true;

	/** Speed threshold to start shake (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0"))
	float ShakeStartSpeed = 150.0f;

	/** Maximum shake intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxShakeIntensity = 0.15f;

	/** Shake frequency (Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float ShakeFrequency = 25.0f;

	/** Shake is directional based on velocity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	bool bDirectionalShake = true;
};

/**
 * Motion blur settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedMotionBlurSettings
{
	GENERATED_BODY()

	/** Enable velocity-scaled motion blur */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionBlur")
	bool bEnableMotionBlur = true;

	/** Base motion blur amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionBlur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseBlurAmount = 0.3f;

	/** Additional blur at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionBlur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxBlurIncrease = 0.4f;

	/** Enable radial blur emanating from vanishing point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionBlur")
	bool bEnableRadialBlur = true;

	/** Maximum radial blur strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionBlur", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxRadialBlurStrength = 0.25f;

	/** Radial blur center offset (0.5, 0.5 = screen center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MotionBlur")
	FVector2D RadialBlurCenter = FVector2D(0.5f, 0.55f);
};

/**
 * Speed lines settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedLinesSettings
{
	GENERATED_BODY()

	/** Enable speed lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines")
	bool bEnableSpeedLines = true;

	/** Speed line style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines")
	FName SpeedLineStyle = "Radial";

	/** Line density (number of lines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines", meta = (ClampMin = "0", ClampMax = "200"))
	int32 LineDensity = 32;

	/** Line opacity at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxLineOpacity = 0.5f;

	/** Line color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines")
	FLinearColor LineColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.6f);

	/** Speed lines only in peripheral vision (clearer center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines")
	bool bPeripheralOnly = true;

	/** Inner radius where lines fade out (0-1, 0 = center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpeedLines", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClearCenterRadius = 0.3f;
};

/**
 * Chromatic aberration settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedChromaticSettings
{
	GENERATED_BODY()

	/** Enable chromatic aberration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chromatic")
	bool bEnableChromatic = true;

	/** Base chromatic intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chromatic", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float BaseIntensity = 0.0f;

	/** Additional intensity at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chromatic", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float MaxIntensityIncrease = 0.8f;

	/** Chromatic starts at screen edges and moves inward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chromatic")
	bool bRadialDistribution = true;

	/** Edge-only: keep center clear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chromatic", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CenterClearRadius = 0.4f;
};

/**
 * Vignette settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedVignetteSettings
{
	GENERATED_BODY()

	/** Enable speed-based vignette */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
	bool bEnableVignette = true;

	/** Base vignette intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseIntensity = 0.1f;

	/** Additional intensity at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxIntensityIncrease = 0.3f;

	/** Vignette color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette")
	FLinearColor VignetteColor = FLinearColor::Black;

	/** Vignette size (higher = smaller dark area) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vignette", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VignetteSize = 0.5f;
};

/**
 * Particle trail settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedParticleSettings
{
	GENERATED_BODY()

	/** Enable particle speed trails */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	bool bEnableParticleTrails = true;

	/** Trail particle system to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	TSoftObjectPtr<class UNiagaraSystem> TrailSystem;

	/** Spawn rate multiplier at max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float MaxSpawnRateMultiplier = 3.0f;

	/** Trail color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	FLinearColor TrailColor = FLinearColor(1.0f, 0.8f, 0.4f, 0.7f);

	/** Trail lifetime multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float TrailLifetimeMultiplier = 1.0f;
};

/**
 * Audio doppler settings
 */
USTRUCT(BlueprintType)
struct FMGSpeedAudioSettings
{
	GENERATED_BODY()

	/** Enable audio doppler effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bEnableAudioEffects = true;

	/** Wind intensity scales with speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bScaleWindIntensity = true;

	/** Maximum wind volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MaxWindMultiplier = 1.5f;

	/** Enable doppler shift on passed objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bEnableDopplerShift = true;

	/** Doppler intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DopplerIntensity = 0.5f;

	/** Engine audio compression at high speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bCompressEngineAudio = true;
};

/**
 * Speed boost parameters (temporary intensity multiplier)
 */
USTRUCT(BlueprintType)
struct FMGSpeedBoostParams
{
	GENERATED_BODY()

	/** Duration of boost effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
	float Duration = 2.0f;

	/** Global intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float GlobalMultiplier = 1.5f;

	/** FOV multiplier (applied on top of speed-based FOV) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float FOVMultiplier = 1.3f;

	/** Motion blur multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float MotionBlurMultiplier = 2.0f;

	/** Speed lines multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float SpeedLinesMultiplier = 2.5f;

	/** Particle trail multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float ParticleTrailMultiplier = 4.0f;

	/** Chromatic aberration multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float ChromaticMultiplier = 1.5f;

	/** Fade out time at end of boost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost", meta = (ClampMin = "0.0", ClampMax = "5.0"))
	float FadeOutTime = 0.5f;
};

/**
 * Complete speed sensation configuration profile
 */
USTRUCT(BlueprintType)
struct FMGSpeedSensationConfig
{
	GENERATED_BODY()

	/** Profile name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FName ProfileName = "Modern";

	/** Speed threshold to start effects (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float MinSpeedThreshold = 80.0f;

	/** Speed for maximum effects (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float MaxSpeedThreshold = 300.0f;

	/** Global intensity scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float GlobalIntensityScale = 1.0f;

	/** Camera FOV settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedFOVSettings FOVSettings;

	/** Screen shake settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedShakeSettings ShakeSettings;

	/** Motion blur settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedMotionBlurSettings MotionBlurSettings;

	/** Speed lines settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedLinesSettings SpeedLinesSettings;

	/** Chromatic aberration settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedChromaticSettings ChromaticSettings;

	/** Vignette settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedVignetteSettings VignetteSettings;

	/** Particle trail settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedParticleSettings ParticleSettings;

	/** Audio effects settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	FMGSpeedAudioSettings AudioSettings;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSpeedIntensityChanged, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSpeedThresholdCrossed, bool, bEnteredHighSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSpeedBoostApplied, float, Multiplier);

/**
 * MGSpeedSensationComponent
 * 
 * Master orchestrator for all speed-related visual and audio effects.
 * Automatically integrates with camera, post-process, screen effects, and audio systems.
 */
UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGSpeedSensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGSpeedSensationComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// PROFILE MANAGEMENT
	// ==========================================

	/** Apply a preset effect profile */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Profile")
	void SetEffectProfile(EMGSpeedSensationProfile Profile);

	/** Get current profile */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Profile")
	EMGSpeedSensationProfile GetCurrentProfile() const { return CurrentProfile; }

	/** Set custom configuration */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Profile")
	void SetCustomConfiguration(const FMGSpeedSensationConfig& Config);

	/** Get current configuration */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Profile")
	FMGSpeedSensationConfig GetConfiguration() const { return Config; }

	// ==========================================
	// INTENSITY CONTROL
	// ==========================================

	/** Set global intensity scale (0 = off, 1 = normal, 2 = double) */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Intensity")
	void SetGlobalIntensityScale(float Scale);

	/** Get current global intensity scale */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Intensity")
	float GetGlobalIntensityScale() const { return Config.GlobalIntensityScale; }

	/** Set intensity for a specific effect category */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Intensity")
	void SetCategoryIntensity(EMGSpeedEffectCategory Category, float Intensity);

	/** Get intensity for a category */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Intensity")
	float GetCategoryIntensity(EMGSpeedEffectCategory Category) const;

	/** Enable/disable a specific category */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Intensity")
	void SetCategoryEnabled(EMGSpeedEffectCategory Category, bool bEnabled);

	/** Is category enabled */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Intensity")
	bool IsCategoryEnabled(EMGSpeedEffectCategory Category) const;

	/** Get current normalized speed intensity (0-1) */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Intensity")
	float GetCurrentSpeedIntensity() const { return CurrentSpeedIntensity; }

	// ==========================================
	// CONTEXTUAL MODIFIERS
	// ==========================================

	/** Set environment intensity multiplier (e.g., 1.5 in tunnels) */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Context")
	void SetEnvironmentMultiplier(float Multiplier);

	/** Trigger proximity intensity pulse (near-miss effect) */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Context")
	void TriggerProximityPulse(float Intensity, float Duration);

	/** Apply temporary speed boost effect (e.g., NOS) */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Context")
	void ApplySpeedBoost(const FMGSpeedBoostParams& BoostParams);

	/** Simple boost with just global multiplier */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Context")
	void BoostIntensity(float Multiplier, float Duration);

	/** Stop any active boost */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Context")
	void StopBoost();

	/** Is boost currently active */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Context")
	bool IsBoostActive() const { return bBoostActive; }

	// ==========================================
	// MANUAL CONTROL (for cutscenes, etc.)
	// ==========================================

	/** Manually set speed (overrides vehicle velocity) */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Manual")
	void SetManualSpeed(float SpeedKPH);

	/** Clear manual speed override */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Manual")
	void ClearManualSpeed();

	/** Is using manual speed */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Manual")
	bool IsUsingManualSpeed() const { return bUseManualSpeed; }

	/** Pause all effects */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Manual")
	void PauseEffects();

	/** Resume effects */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Manual")
	void ResumeEffects();

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get current vehicle speed in KPH */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Utility")
	float GetCurrentSpeedKPH() const;

	/** Reset all effects to defaults */
	UFUNCTION(BlueprintCallable, Category = "SpeedSensation|Utility")
	void ResetToDefaults();

	/** Get available preset profile names */
	UFUNCTION(BlueprintPure, Category = "SpeedSensation|Utility")
	TArray<FString> GetAvailableProfiles() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when speed intensity changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "SpeedSensation|Events")
	FMGOnSpeedIntensityChanged OnSpeedIntensityChanged;

	/** Fired when crossing speed threshold (entering/exiting high speed) */
	UPROPERTY(BlueprintAssignable, Category = "SpeedSensation|Events")
	FMGOnSpeedThresholdCrossed OnSpeedThresholdCrossed;

	/** Fired when speed boost is applied */
	UPROPERTY(BlueprintAssignable, Category = "SpeedSensation|Events")
	FMGOnSpeedBoostApplied OnSpeedBoostApplied;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Current effect profile */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	EMGSpeedSensationProfile CurrentProfile = EMGSpeedSensationProfile::Modern;

	/** Effect configuration */
	UPROPERTY(EditDefaultsOnly, Category = "Configuration")
	FMGSpeedSensationConfig Config;

	/** Per-category intensity multipliers */
	UPROPERTY()
	TMap<EMGSpeedEffectCategory, float> CategoryIntensities;

	/** Per-category enabled flags */
	UPROPERTY()
	TMap<EMGSpeedEffectCategory, bool> CategoryEnabled;

	// ==========================================
	// STATE
	// ==========================================

	/** Current normalized speed intensity (0-1) */
	UPROPERTY()
	float CurrentSpeedIntensity = 0.0f;

	/** Target intensity (for smooth interpolation) */
	UPROPERTY()
	float TargetSpeedIntensity = 0.0f;

	/** Environment intensity multiplier */
	UPROPERTY()
	float EnvironmentMultiplier = 1.0f;

	/** Proximity pulse intensity */
	UPROPERTY()
	float ProximityPulseIntensity = 0.0f;
	
	/** Proximity pulse timer */
	UPROPERTY()
	float ProximityPulseDuration = 0.0f;
	
	/** Proximity pulse elapsed */
	UPROPERTY()
	float ProximityPulseElapsed = 0.0f;

	/** Boost active flag */
	UPROPERTY()
	bool bBoostActive = false;

	/** Current boost params */
	UPROPERTY()
	FMGSpeedBoostParams CurrentBoostParams;

	/** Boost elapsed time */
	UPROPERTY()
	float BoostElapsedTime = 0.0f;

	/** Manual speed override */
	UPROPERTY()
	bool bUseManualSpeed = false;

	/** Manual speed value */
	UPROPERTY()
	float ManualSpeedKPH = 0.0f;

	/** Effects paused */
	UPROPERTY()
	bool bEffectsPaused = false;

	/** Was in high speed last frame */
	UPROPERTY()
	bool bWasInHighSpeed = false;

	/** Last notified intensity (for change detection) */
	UPROPERTY()
	float LastNotifiedIntensity = 0.0f;

	// ==========================================
	// CACHED REFERENCES
	// ==========================================

	UPROPERTY()
	UMGCameraVFXComponent* CachedCameraVFX = nullptr;

	UPROPERTY()
	UMGPostProcessSubsystem* CachedPostProcessSubsystem = nullptr;

	UPROPERTY()
	UMGScreenEffectSubsystem* CachedScreenEffectSubsystem = nullptr;

	UPROPERTY()
	UMGVehicleVFXComponent* CachedVehicleVFX = nullptr;

	UPROPERTY()
	UMGEngineAudioComponent* CachedEngineAudio = nullptr;

	UPROPERTY()
	class APawn* CachedOwnerPawn = nullptr;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Initialize component references */
	void InitializeReferences();

	/** Create default configuration for a profile */
	FMGSpeedSensationConfig CreateConfigForProfile(EMGSpeedSensationProfile Profile) const;

	/** Calculate speed intensity from velocity */
	float CalculateSpeedIntensity(float SpeedKPH) const;

	/** Apply curve to intensity value */
	float ApplyCurve(float Value, EMGSpeedCurveType CurveType, UCurveFloat* CustomCurve) const;

	/** Update all effect systems */
	void UpdateEffects(float DeltaTime);

	/** Update camera effects */
	void UpdateCameraEffects(float IntensityOverride = -1.0f);

	/** Update post-process effects */
	void UpdatePostProcessEffects(float IntensityOverride = -1.0f);

	/** Update screen effects */
	void UpdateScreenEffects(float IntensityOverride = -1.0f);

	/** Update particle effects */
	void UpdateParticleEffects(float IntensityOverride = -1.0f);

	/** Update audio effects */
	void UpdateAudioEffects(float IntensityOverride = -1.0f);

	/** Update boost state */
	void UpdateBoost(float DeltaTime);

	/** Update proximity pulse */
	void UpdateProximityPulse(float DeltaTime);

	/** Calculate final intensity with all modifiers */
	float CalculateFinalIntensity(EMGSpeedEffectCategory Category, float BaseIntensity) const;

	/** Check for threshold crossing */
	void CheckThresholdCrossing();

	/** Notify intensity change */
	void NotifyIntensityChange(float NewIntensity);
};
