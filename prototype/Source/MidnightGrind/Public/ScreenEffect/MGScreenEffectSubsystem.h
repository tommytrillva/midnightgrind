// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGScreenEffectSubsystem.h
 * @brief Screen-Space Visual Effects System for Racing Feedback
 *
 * =============================================================================
 * Screen-Space Visual Effects System
 * =============================================================================
 *
 * @section overview OVERVIEW
 * --------
 * This file defines the screen effect system that creates dynamic visual feedback
 * overlaid on the game view. These effects communicate game state to the player
 * through visual language - speed lines show velocity, red vignettes indicate
 * damage, chromatic aberration emphasizes impacts, and Y2K effects add style.
 *
 * @section screen_effects WHAT ARE SCREEN EFFECTS?
 * ------------------------
 * Screen effects are 2D visual modifications applied to the entire rendered frame
 * (post-process effects). Unlike 3D world effects (particles, lights), these exist
 * in "screen space" and are rendered after the 3D scene is complete.
 *
 * Common examples in games:
 *   - Vignette: Darkening around screen edges
 *   - Motion blur: Streaking when moving fast
 *   - Chromatic aberration: RGB color separation (lens distortion look)
 *   - Screen shake: Camera displacement for impact
 *
 * @section beginners KEY CONCEPTS FOR BEGINNERS
 * --------------------------
 *
 * 1. GAME INSTANCE SUBSYSTEM
 *    This class inherits from UGameInstanceSubsystem:
 *    - Persists for the entire game session
 *    - One instance shared across all levels
 *    - Access via: GetGameInstance()->GetSubsystem<UMGScreenEffectSubsystem>()
 *
 * 2. EFFECT CATEGORIES (EMGScreenEffectCategory)
 *    Effects are organized by their purpose:
 *    - Speed: Velocity-based effects (speed lines, FOV changes)
 *    - Damage: Health/impact feedback (red vignette, desaturation)
 *    - Boost: Nitro/turbo activation visuals (blur trails, glow)
 *    - Impact: Collision feedback (screen shake, flash)
 *    - Y2K: Stylized aesthetic effects (glitch, scanlines, neon)
 *
 * 3. BLEND MODES (EMGEffectBlendMode)
 *    How effects combine with the game image:
 *    - Additive: Brightens (effect + scene) - good for glows
 *    - Multiply: Darkens (effect * scene) - good for vignettes
 *    - Screen: Brightens without over-saturation
 *    - Overlay: Contrast enhancement
 *    - AlphaBlend: Standard transparency blending
 *
 * 4. INTENSITY CURVES (EMGIntensityCurve)
 *    How effect intensity changes over time:
 *    - Linear: Constant rate change
 *    - EaseIn: Starts slow, accelerates
 *    - EaseOut: Starts fast, decelerates (most natural for impacts)
 *    - Bounce/Elastic: Overshoots and settles (cartoon-style)
 *    - Pulse/Flicker: Repeating patterns (damage warnings)
 *
 * 5. ACTIVE EFFECTS (FMGActiveEffect)
 *    Each playing effect tracks its state:
 *    - EffectId: Unique identifier (GUID) for stopping/modifying
 *    - Intensity: Current strength (0.0 - 1.0)
 *    - Duration: Total play time
 *    - ElapsedTime: Time since started
 *    - Priority: Higher priority effects render on top
 *    - bLooping: Whether effect repeats
 *
 * 6. EFFECT PARAMETERS
 *    Each effect category has a parameter struct for customization:
 *    - FMGSpeedEffectParams: Line count, blur amount, FOV increase
 *    - FMGDamageEffectParams: Vignette style, pulse rate, desaturation
 *    - FMGBoostEffectParams: Trail length, bloom intensity, colors
 *    - FMGImpactEffectParams: Shake intensity, flash duration
 *    - FMGY2KEffectParams: Glitch intensity, scanlines, RGB split
 *
 * @section usage COMMON USAGE PATTERNS
 * ---------------------
 *
 * Speed-based effects (called from vehicle each frame):
 *   ScreenEffectSubsystem->UpdateSpeedEffect(CurrentSpeedKPH);
 *
 * Damage feedback (called when hit):
 *   ScreenEffectSubsystem->TriggerDamageFlash(1.0f);
 *   ScreenEffectSubsystem->UpdateDamageEffect(HealthPercent);
 *
 * Boost activation:
 *   ScreenEffectSubsystem->StartBoostEffect(1.0f);
 *   // When boost ends:
 *   ScreenEffectSubsystem->StopBoostEffect(0.3f); // 0.3s fade out
 *
 * Impact from collision:
 *   ScreenEffectSubsystem->TriggerImpact(EMGImpactEffectType::HeavyCrash, Force);
 *
 * Screen transitions (level changes):
 *   ScreenEffectSubsystem->FadeOut(0.5f);
 *   // Load new level, then:
 *   ScreenEffectSubsystem->FadeIn(0.5f);
 *
 * Y2K stylized effects (race finish celebration):
 *   FGuid EffectId = ScreenEffectSubsystem->StartY2KEffect(EMGY2KEffectType::NeonPulse, 3.0f);
 *
 * @section presets PRESETS
 * -------
 * Presets (FMGEffectPreset) store complete effect configurations.
 * Use ApplyPreset("Cinematic") to quickly switch between visual styles.
 * SavePreset() captures current settings for later use.
 *
 * @section output OUTPUT VALUES
 * -------------
 * The subsystem calculates final values each frame that can be read by materials:
 *   - GetRadialBlurAmount(): For post-process material parameters
 *   - GetChromaticAberrationAmount(): RGB separation strength
 *   - GetVignetteIntensity(): Edge darkening amount
 *   - GetGlitchIntensity(): Y2K glitch effect strength
 *
 * These output values are typically bound to Material Parameter Collections
 * that drive the actual post-process materials in the world.
 *
 * @section performance PERFORMANCE NOTES
 * -----------------
 * - Effects are composited, so many active effects have minimal extra cost
 * - GlobalEffectScale can reduce all effects for performance/accessibility
 * - Categories can be individually disabled in settings
 * - Screen shake uses a separate update path from visual effects
 *
 * @see EMGScreenEffectCategory For all effect categories
 * @see FMGEffectPreset For preset configuration
 * @see FMGActiveEffect For active effect instance tracking
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGScreenEffectSubsystem.generated.h"

// Forward declarations
class UMGScreenEffectSubsystem;

/**
 * Screen effect category
 */
UENUM(BlueprintType)
enum class EMGScreenEffectCategory : uint8
{
    Speed           UMETA(DisplayName = "Speed"),
    Damage          UMETA(DisplayName = "Damage"),
    Boost           UMETA(DisplayName = "Boost"),
    Impact          UMETA(DisplayName = "Impact"),
    Environment     UMETA(DisplayName = "Environment"),
    Transition      UMETA(DisplayName = "Transition"),
    Feedback        UMETA(DisplayName = "Feedback"),
    Y2K             UMETA(DisplayName = "Y2K Aesthetic")
};

/**
 * Screen effect blend mode
 */
UENUM(BlueprintType)
enum class EMGEffectBlendMode : uint8
{
    Additive        UMETA(DisplayName = "Additive"),
    Multiply        UMETA(DisplayName = "Multiply"),
    Screen          UMETA(DisplayName = "Screen"),
    Overlay         UMETA(DisplayName = "Overlay"),
    Replace         UMETA(DisplayName = "Replace"),
    AlphaBlend      UMETA(DisplayName = "Alpha Blend")
};

/**
 * Effect intensity curve type
 */
UENUM(BlueprintType)
enum class EMGIntensityCurve : uint8
{
    Linear          UMETA(DisplayName = "Linear"),
    EaseIn          UMETA(DisplayName = "Ease In"),
    EaseOut         UMETA(DisplayName = "Ease Out"),
    EaseInOut       UMETA(DisplayName = "Ease In Out"),
    Bounce          UMETA(DisplayName = "Bounce"),
    Elastic         UMETA(DisplayName = "Elastic"),
    Pulse           UMETA(DisplayName = "Pulse"),
    Flicker         UMETA(DisplayName = "Flicker")
};

/**
 * Speed line style
 */
UENUM(BlueprintType)
enum class EMGSpeedLineStyle : uint8
{
    Radial          UMETA(DisplayName = "Radial"),
    Horizontal      UMETA(DisplayName = "Horizontal"),
    Tunnel          UMETA(DisplayName = "Tunnel"),
    Anime           UMETA(DisplayName = "Anime Style"),
    Digital         UMETA(DisplayName = "Digital"),
    Neon            UMETA(DisplayName = "Neon Trails")
};

/**
 * Damage vignette style
 */
UENUM(BlueprintType)
enum class EMGDamageVignetteStyle : uint8
{
    Classic         UMETA(DisplayName = "Classic Red"),
    Cracked         UMETA(DisplayName = "Cracked Glass"),
    Glitch          UMETA(DisplayName = "Digital Glitch"),
    Blood           UMETA(DisplayName = "Blood Splatter"),
    Sparks          UMETA(DisplayName = "Electric Sparks"),
    Smoke           UMETA(DisplayName = "Smoke Effect")
};

/**
 * Boost effect style
 */
UENUM(BlueprintType)
enum class EMGBoostEffectStyle : uint8
{
    BlurTrails      UMETA(DisplayName = "Blur Trails"),
    NeonGlow        UMETA(DisplayName = "Neon Glow"),
    FireTrails      UMETA(DisplayName = "Fire Trails"),
    Electric        UMETA(DisplayName = "Electric"),
    Warp            UMETA(DisplayName = "Space Warp"),
    Chromatic       UMETA(DisplayName = "Chromatic Burst")
};

/**
 * Impact effect type
 */
UENUM(BlueprintType)
enum class EMGImpactEffectType : uint8
{
    LightBump       UMETA(DisplayName = "Light Bump"),
    MediumCollision UMETA(DisplayName = "Medium Collision"),
    HeavyCrash      UMETA(DisplayName = "Heavy Crash"),
    WallScrape      UMETA(DisplayName = "Wall Scrape"),
    VehicleContact  UMETA(DisplayName = "Vehicle Contact"),
    Landing         UMETA(DisplayName = "Jump Landing")
};

/**
 * Screen shake type
 */
UENUM(BlueprintType)
enum class EMGScreenShakeType : uint8
{
    None            UMETA(DisplayName = "None"),
    Subtle          UMETA(DisplayName = "Subtle"),
    Normal          UMETA(DisplayName = "Normal"),
    Intense         UMETA(DisplayName = "Intense"),
    Directional     UMETA(DisplayName = "Directional"),
    Rotational      UMETA(DisplayName = "Rotational")
};

/**
 * Y2K effect type
 */
UENUM(BlueprintType)
enum class EMGY2KEffectType : uint8
{
    PixelBurst      UMETA(DisplayName = "Pixel Burst"),
    VHSGlitch       UMETA(DisplayName = "VHS Glitch"),
    RGBSplit        UMETA(DisplayName = "RGB Split"),
    ScanlineFlash   UMETA(DisplayName = "Scanline Flash"),
    MatrixRain      UMETA(DisplayName = "Matrix Rain"),
    HologramFlicker UMETA(DisplayName = "Hologram Flicker"),
    NeonPulse       UMETA(DisplayName = "Neon Pulse"),
    RetroTransition UMETA(DisplayName = "Retro Transition")
};

/**
 * Active screen effect instance
 */
USTRUCT(BlueprintType)
struct FMGActiveEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FGuid EffectId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EMGScreenEffectCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EMGIntensityCurve IntensityCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EMGEffectBlendMode BlendMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bPaused;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FLinearColor TintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FVector2D ScreenPosition;

    FMGActiveEffect()
        : Intensity(1.0f)
        , Duration(1.0f)
        , ElapsedTime(0.0f)
        , FadeInTime(0.1f)
        , FadeOutTime(0.2f)
        , IntensityCurve(EMGIntensityCurve::EaseOut)
        , BlendMode(EMGEffectBlendMode::Additive)
        , Priority(0)
        , bLooping(false)
        , bPaused(false)
        , TintColor(FLinearColor::White)
        , ScreenPosition(0.5f, 0.5f)
    {}
};

/**
 * Speed effect parameters
 */
USTRUCT(BlueprintType)
struct FMGSpeedEffectParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    EMGSpeedLineStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float MinSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float MaxSpeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float LineCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float LineLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float LineOpacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float RadialBlurAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float MotionBlurScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float VignetteIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    float FOVIncrease;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    FLinearColor LineColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    bool bEnableSpeedLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    bool bEnableRadialBlur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    bool bEnableMotionBlur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
    bool bEnableFOVChange;

    FMGSpeedEffectParams()
        : Style(EMGSpeedLineStyle::Radial)
        , MinSpeedThreshold(100.0f)
        , MaxSpeedThreshold(300.0f)
        , LineCount(32.0f)
        , LineLength(0.5f)
        , LineOpacity(0.4f)
        , RadialBlurAmount(0.02f)
        , MotionBlurScale(1.0f)
        , VignetteIntensity(0.3f)
        , FOVIncrease(5.0f)
        , LineColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.5f))
        , bEnableSpeedLines(true)
        , bEnableRadialBlur(true)
        , bEnableMotionBlur(true)
        , bEnableFOVChange(true)
    {}
};

/**
 * Damage effect parameters
 */
USTRUCT(BlueprintType)
struct FMGDamageEffectParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EMGDamageVignetteStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float VignetteRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float VignetteSoftness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DesaturationAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float RedTintIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float PulseRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float HealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float ChromaticAberration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    FLinearColor DamageColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bEnablePulse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bEnableDesaturation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    bool bEnableChromatic;

    FMGDamageEffectParams()
        : Style(EMGDamageVignetteStyle::Classic)
        , VignetteRadius(0.7f)
        , VignetteSoftness(0.3f)
        , DesaturationAmount(0.3f)
        , RedTintIntensity(0.4f)
        , PulseRate(2.0f)
        , HealthThreshold(0.3f)
        , ChromaticAberration(0.01f)
        , DamageColor(FLinearColor(1.0f, 0.0f, 0.0f, 0.5f))
        , bEnablePulse(true)
        , bEnableDesaturation(true)
        , bEnableChromatic(true)
    {}
};

/**
 * Boost effect parameters
 */
USTRUCT(BlueprintType)
struct FMGBoostEffectParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    EMGBoostEffectStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float IntensityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float RadialBlurCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float ChromaticStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float BloomIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float ExposureBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float TrailLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    float TrailOpacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    FLinearColor BoostColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    FLinearColor TrailColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    bool bEnableTrails;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    bool bEnableBloom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boost")
    bool bEnableChromatic;

    FMGBoostEffectParams()
        : Style(EMGBoostEffectStyle::NeonGlow)
        , IntensityScale(1.0f)
        , RadialBlurCenter(0.5f)
        , ChromaticStrength(0.02f)
        , BloomIntensity(1.5f)
        , ExposureBoost(0.2f)
        , TrailLength(0.3f)
        , TrailOpacity(0.6f)
        , BoostColor(FLinearColor(0.0f, 0.8f, 1.0f, 1.0f))
        , TrailColor(FLinearColor(1.0f, 0.5f, 0.0f, 0.8f))
        , bEnableTrails(true)
        , bEnableBloom(true)
        , bEnableChromatic(true)
    {}
};

/**
 * Impact effect parameters
 */
USTRUCT(BlueprintType)
struct FMGImpactEffectParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ShakeDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ShakeFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ChromaticPunch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float FlashIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float FlashDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float DistortionAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    float ZoomPunch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FVector2D ImpactDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    FLinearColor FlashColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    EMGScreenShakeType ShakeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    bool bEnableShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    bool bEnableFlash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
    bool bEnableDistortion;

    FMGImpactEffectParams()
        : ShakeIntensity(1.0f)
        , ShakeDuration(0.3f)
        , ShakeFrequency(20.0f)
        , ChromaticPunch(0.03f)
        , FlashIntensity(0.5f)
        , FlashDuration(0.1f)
        , DistortionAmount(0.02f)
        , ZoomPunch(0.02f)
        , ImpactDirection(0.0f, 0.0f)
        , FlashColor(FLinearColor::White)
        , ShakeType(EMGScreenShakeType::Normal)
        , bEnableShake(true)
        , bEnableFlash(true)
        , bEnableDistortion(true)
    {}
};

/**
 * Y2K effect parameters
 */
USTRUCT(BlueprintType)
struct FMGY2KEffectParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    EMGY2KEffectType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float GlitchIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float GlitchFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float RGBSplitAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float ScanlineIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float ScanlineCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float PixelSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float NoiseAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float HologramFlicker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    FLinearColor NeonColor1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    FLinearColor NeonColor2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    float ColorCycleSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    bool bEnableGlitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    bool bEnableScanlines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Y2K")
    bool bEnableNoise;

    FMGY2KEffectParams()
        : Type(EMGY2KEffectType::NeonPulse)
        , GlitchIntensity(0.3f)
        , GlitchFrequency(5.0f)
        , RGBSplitAmount(0.005f)
        , ScanlineIntensity(0.2f)
        , ScanlineCount(240.0f)
        , PixelSize(4.0f)
        , NoiseAmount(0.05f)
        , HologramFlicker(0.1f)
        , NeonColor1(FLinearColor(1.0f, 0.0f, 0.8f, 1.0f))
        , NeonColor2(FLinearColor(0.0f, 1.0f, 0.8f, 1.0f))
        , ColorCycleSpeed(1.0f)
        , bEnableGlitch(true)
        , bEnableScanlines(true)
        , bEnableNoise(true)
    {}
};

/**
 * Transition effect parameters
 */
USTRUCT(BlueprintType)
struct FMGTransitionParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FLinearColor FadeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EMGIntensityCurve Curve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bUseWipeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FVector2D WipeDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bUsePixelation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float MaxPixelSize;

    FMGTransitionParams()
        : Duration(0.5f)
        , FadeColor(FLinearColor::Black)
        , Curve(EMGIntensityCurve::EaseInOut)
        , bUseWipeEffect(false)
        , WipeDirection(1.0f, 0.0f)
        , bUsePixelation(false)
        , MaxPixelSize(32.0f)
    {}
};

/**
 * Screen effect preset
 */
USTRUCT(BlueprintType)
struct FMGEffectPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FMGSpeedEffectParams SpeedParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FMGDamageEffectParams DamageParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FMGBoostEffectParams BoostParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FMGImpactEffectParams ImpactParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FMGY2KEffectParams Y2KParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    bool bEnableSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    bool bEnableDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    bool bEnableBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    bool bEnableImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    bool bEnableY2K;

    FMGEffectPreset()
        : PresetName(TEXT("Default"))
        , bEnableSpeed(true)
        , bEnableDamage(true)
        , bEnableBoost(true)
        , bEnableImpact(true)
        , bEnableY2K(true)
    {}
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEffectStarted, const FGuid&, EffectId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnEffectEnded, const FGuid&, EffectId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnEffectIntensityChanged, const FGuid&, EffectId, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTransitionStarted, bool, bFadeIn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTransitionComplete, bool, bFadeIn);

/**
 * Screen Effect Subsystem
 *
 * Manages screen-space visual effects including speed lines, damage vignettes,
 * boost effects, impact feedback, and Y2K aesthetic effects. Provides layered
 * effect composition with priority and blending support.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGScreenEffectSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGScreenEffectSubsystem();

    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    // Update (call per frame)
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect")
    void UpdateEffects(float DeltaTime);

    // Speed effects
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Speed")
    void UpdateSpeedEffect(float CurrentSpeed);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Speed")
    void SetSpeedEffectParams(const FMGSpeedEffectParams& Params);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Speed")
    FMGSpeedEffectParams GetSpeedEffectParams() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Speed")
    float GetCurrentSpeedEffectIntensity() const;

    // Damage effects
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Damage")
    void UpdateDamageEffect(float HealthPercent);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Damage")
    void SetDamageEffectParams(const FMGDamageEffectParams& Params);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Damage")
    FMGDamageEffectParams GetDamageEffectParams() const;

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Damage")
    void TriggerDamageFlash(float Intensity = 1.0f);

    // Boost effects
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Boost")
    void StartBoostEffect(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Boost")
    void StopBoostEffect(float FadeOutTime = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Boost")
    void UpdateBoostEffect(float BoostAmount);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Boost")
    void SetBoostEffectParams(const FMGBoostEffectParams& Params);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Boost")
    FMGBoostEffectParams GetBoostEffectParams() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Boost")
    bool IsBoostEffectActive() const;

    // Impact effects
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Impact")
    void TriggerImpact(EMGImpactEffectType Type, float Intensity = 1.0f, FVector2D Direction = FVector2D::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Impact")
    void SetImpactEffectParams(EMGImpactEffectType Type, const FMGImpactEffectParams& Params);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Impact")
    FMGImpactEffectParams GetImpactEffectParams(EMGImpactEffectType Type) const;

    // Screen shake
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Shake")
    void StartScreenShake(float Intensity, float Duration, EMGScreenShakeType Type = EMGScreenShakeType::Normal);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Shake")
    void StopScreenShake();

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Shake")
    FVector2D GetCurrentShakeOffset() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Shake")
    bool IsScreenShaking() const;

    // Y2K effects
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Y2K")
    FGuid StartY2KEffect(EMGY2KEffectType Type, float Duration = 1.0f, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Y2K")
    void StopY2KEffect(const FGuid& EffectId);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Y2K")
    void StopAllY2KEffects();

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Y2K")
    void SetY2KEffectParams(const FMGY2KEffectParams& Params);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Y2K")
    FMGY2KEffectParams GetY2KEffectParams() const;

    // Transitions
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Transition")
    void FadeToColor(const FLinearColor& Color, float Duration = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Transition")
    void FadeFromColor(const FLinearColor& Color, float Duration = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Transition")
    void FadeOut(float Duration = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Transition")
    void FadeIn(float Duration = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Transition")
    void SetTransitionParams(const FMGTransitionParams& Params);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Transition")
    bool IsTransitioning() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Transition")
    float GetTransitionProgress() const;

    // Generic effects
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Generic")
    FGuid PlayEffect(const FString& EffectName, float Duration = 1.0f, float Intensity = 1.0f,
                     EMGScreenEffectCategory Category = EMGScreenEffectCategory::Feedback);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Generic")
    void StopEffect(const FGuid& EffectId, float FadeOutTime = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Generic")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Generic")
    void PauseEffect(const FGuid& EffectId);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Generic")
    void ResumeEffect(const FGuid& EffectId);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Generic")
    void SetEffectIntensity(const FGuid& EffectId, float Intensity);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Generic")
    bool IsEffectActive(const FGuid& EffectId) const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Generic")
    TArray<FMGActiveEffect> GetActiveEffects() const;

    // Presets
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Presets")
    void ApplyPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Presets")
    void SavePreset(const FString& PresetName);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Presets")
    TArray<FString> GetAvailablePresets() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Presets")
    FMGEffectPreset GetCurrentPreset() const;

    // Global settings
    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Settings")
    void SetGlobalEffectScale(float Scale);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Settings")
    float GetGlobalEffectScale() const;

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Settings")
    void SetEffectsEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Settings")
    bool AreEffectsEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "ScreenEffect|Settings")
    void SetCategoryEnabled(EMGScreenEffectCategory Category, bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Settings")
    bool IsCategoryEnabled(EMGScreenEffectCategory Category) const;

    // Post-process values (for material parameter binding)
    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetRadialBlurAmount() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetChromaticAberrationAmount() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetVignetteIntensity() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    FLinearColor GetVignetteColor() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetDesaturationAmount() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetBloomMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    FLinearColor GetScreenTint() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetGlitchIntensity() const;

    UFUNCTION(BlueprintPure, Category = "ScreenEffect|Output")
    float GetScanlineIntensity() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "ScreenEffect|Events")
    FMGOnEffectStarted OnEffectStarted;

    UPROPERTY(BlueprintAssignable, Category = "ScreenEffect|Events")
    FMGOnEffectEnded OnEffectEnded;

    UPROPERTY(BlueprintAssignable, Category = "ScreenEffect|Events")
    FMGOnEffectIntensityChanged OnEffectIntensityChanged;

    UPROPERTY(BlueprintAssignable, Category = "ScreenEffect|Events")
    FMGOnTransitionStarted OnTransitionStarted;

    UPROPERTY(BlueprintAssignable, Category = "ScreenEffect|Events")
    FMGOnTransitionComplete OnTransitionComplete;

protected:
    // Effect parameters
    UPROPERTY()
    FMGSpeedEffectParams SpeedParams;

    UPROPERTY()
    FMGDamageEffectParams DamageParams;

    UPROPERTY()
    FMGBoostEffectParams BoostParams;

    UPROPERTY()
    FMGY2KEffectParams Y2KParams;

    UPROPERTY()
    FMGTransitionParams TransitionParams;

    UPROPERTY()
    TMap<EMGImpactEffectType, FMGImpactEffectParams> ImpactParamsMap;

    // Active effects
    UPROPERTY()
    TMap<FGuid, FMGActiveEffect> ActiveEffects;

    // Current state
    UPROPERTY()
    float CurrentSpeedEffectIntensity;

    UPROPERTY()
    float CurrentDamageEffectIntensity;

    UPROPERTY()
    float CurrentBoostEffectIntensity;

    UPROPERTY()
    bool bBoostActive;

    // Screen shake
    UPROPERTY()
    bool bShakeActive;

    UPROPERTY()
    float ShakeIntensity;

    UPROPERTY()
    float ShakeDuration;

    UPROPERTY()
    float ShakeElapsedTime;

    UPROPERTY()
    float ShakeFrequency;

    UPROPERTY()
    EMGScreenShakeType ShakeType;

    UPROPERTY()
    FVector2D CurrentShakeOffset;

    // Transition
    UPROPERTY()
    bool bTransitioning;

    UPROPERTY()
    bool bFadingIn;

    UPROPERTY()
    float TransitionProgress;

    UPROPERTY()
    float TransitionDuration;

    UPROPERTY()
    FLinearColor TransitionColor;

    // Output values
    UPROPERTY()
    float OutputRadialBlur;

    UPROPERTY()
    float OutputChromatic;

    UPROPERTY()
    float OutputVignette;

    UPROPERTY()
    FLinearColor OutputVignetteColor;

    UPROPERTY()
    float OutputDesaturation;

    UPROPERTY()
    float OutputBloom;

    UPROPERTY()
    FLinearColor OutputTint;

    UPROPERTY()
    float OutputGlitch;

    UPROPERTY()
    float OutputScanlines;

    // Settings
    UPROPERTY()
    float GlobalEffectScale;

    UPROPERTY()
    bool bEffectsEnabled;

    UPROPERTY()
    TMap<EMGScreenEffectCategory, bool> CategoryEnabled;

    // Presets
    UPROPERTY()
    TMap<FString, FMGEffectPreset> EffectPresets;

    UPROPERTY()
    FString CurrentPresetName;

    // Helper functions
    void InitializeDefaultParams();
    void InitializePresets();
    void UpdateActiveEffects(float DeltaTime);
    void UpdateShake(float DeltaTime);
    void UpdateTransition(float DeltaTime);
    void CalculateOutputValues();
    float ApplyIntensityCurve(float T, EMGIntensityCurve Curve) const;
    void CleanupExpiredEffects();
};
