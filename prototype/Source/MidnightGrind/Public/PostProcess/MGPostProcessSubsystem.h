// Copyright Midnight Grind. All Rights Reserved.

// MGPostProcessSubsystem.h
// Midnight Grind - Y2K Visual Effects and Post-Processing System
// Provides retro PS1/PS2 visual effects with modern post-processing capabilities

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPostProcessSubsystem.generated.h"

// Visual style presets
UENUM(BlueprintType)
enum class EMGVisualStyle : uint8
{
    Modern                  UMETA(DisplayName = "Modern"),
    PS2Authentic            UMETA(DisplayName = "PS2 Authentic"),
    PS1Retro                UMETA(DisplayName = "PS1 Retro"),
    Y2KNeon                 UMETA(DisplayName = "Y2K Neon"),
    CRTArcade               UMETA(DisplayName = "CRT Arcade"),
    VHSNostalgia            UMETA(DisplayName = "VHS Nostalgia"),
    Cyberpunk               UMETA(DisplayName = "Cyberpunk"),
    NightCity               UMETA(DisplayName = "Night City"),
    SunsetDrive             UMETA(DisplayName = "Sunset Drive"),
    Custom                  UMETA(DisplayName = "Custom")
};

// Color grading modes
UENUM(BlueprintType)
enum class EMGColorGradingMode : uint8
{
    Neutral                 UMETA(DisplayName = "Neutral"),
    Warm                    UMETA(DisplayName = "Warm"),
    Cool                    UMETA(DisplayName = "Cool"),
    Neon                    UMETA(DisplayName = "Neon"),
    Desaturated             UMETA(DisplayName = "Desaturated"),
    HighContrast            UMETA(DisplayName = "High Contrast"),
    Vintage                 UMETA(DisplayName = "Vintage"),
    Cinema                  UMETA(DisplayName = "Cinema"),
    Custom                  UMETA(DisplayName = "Custom")
};

// Screen effect types
UENUM(BlueprintType)
enum class EMGScreenEffect : uint8
{
    None                    UMETA(DisplayName = "None"),
    Bloom                   UMETA(DisplayName = "Bloom"),
    MotionBlur              UMETA(DisplayName = "Motion Blur"),
    RadialBlur              UMETA(DisplayName = "Radial Blur"),
    SpeedLines              UMETA(DisplayName = "Speed Lines"),
    ChromaticAberration     UMETA(DisplayName = "Chromatic Aberration"),
    FilmGrain               UMETA(DisplayName = "Film Grain"),
    Vignette                UMETA(DisplayName = "Vignette"),
    Scanlines               UMETA(DisplayName = "Scanlines"),
    CRTCurvature            UMETA(DisplayName = "CRT Curvature"),
    VHSDistortion           UMETA(DisplayName = "VHS Distortion"),
    GlitchEffect            UMETA(DisplayName = "Glitch"),
    NitroBlur               UMETA(DisplayName = "Nitro Blur"),
    DriftSmoke              UMETA(DisplayName = "Drift Smoke"),
    ImpactFlash             UMETA(DisplayName = "Impact Flash")
};

// Pixelation modes for retro look
UENUM(BlueprintType)
enum class EMGPixelationMode : uint8
{
    None                    UMETA(DisplayName = "None"),
    Subtle                  UMETA(DisplayName = "Subtle (720p)"),
    PS2                     UMETA(DisplayName = "PS2 (480p)"),
    PS1                     UMETA(DisplayName = "PS1 (240p)"),
    Extreme                 UMETA(DisplayName = "Extreme (160p)"),
    Custom                  UMETA(DisplayName = "Custom")
};

// Weather/time effects
UENUM(BlueprintType)
enum class EMGTimeOfDay : uint8
{
    Dawn                    UMETA(DisplayName = "Dawn"),
    Morning                 UMETA(DisplayName = "Morning"),
    Noon                    UMETA(DisplayName = "Noon"),
    Afternoon               UMETA(DisplayName = "Afternoon"),
    Sunset                  UMETA(DisplayName = "Sunset"),
    Dusk                    UMETA(DisplayName = "Dusk"),
    Night                   UMETA(DisplayName = "Night"),
    Midnight                UMETA(DisplayName = "Midnight"),
    Custom                  UMETA(DisplayName = "Custom")
};

// Bloom settings
USTRUCT(BlueprintType)
struct FMGBloomSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Threshold = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Tint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0", ClampMax = "64.0"))
    float SizeScale = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bConvolutionBloom = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnamorphicRatio = 0.0f;
};

// Motion blur settings
USTRUCT(BlueprintType)
struct FMGMotionBlurSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Amount = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Max = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPerObjectMotionBlur = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSpeedBasedIntensity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedThreshold = 100.0f;
};

// Chromatic aberration settings
USTRUCT(BlueprintType)
struct FMGChromaticAberrationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartOffset = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRadialDistribution = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D RedOffset = FVector2D(1.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D GreenOffset = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D BlueOffset = FVector2D(-1.0f, 0.0f);
};

// Film grain settings
USTRUCT(BlueprintType)
struct FMGFilmGrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Response = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAnimated = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnimationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor GrainTint = FLinearColor::White;
};

// Vignette settings
USTRUCT(BlueprintType)
struct FMGVignetteSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor Color = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Size = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float Feather = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRounded = true;
};

// CRT/Scanline settings
USTRUCT(BlueprintType)
struct FMGScanlineSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100", ClampMax = "2000"))
    int32 LineCount = 480;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Thickness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bScrolling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ScrollSpeed = 0.0f;
};

// CRT curvature settings
USTRUCT(BlueprintType)
struct FMGCRTSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curvature = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float CornerRadius = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowBezel = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BezelColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PhosphorBleed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGScanlineSettings Scanlines;
};

// VHS distortion settings
USTRUCT(BlueprintType)
struct FMGVHSSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ColorBleed = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StaticNoise = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Jitter = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTrackingLines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TrackingLineFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDateTimeStamp = false;
};

// Glitch effect settings
USTRUCT(BlueprintType)
struct FMGGlitchSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float Frequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBlockGlitch = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bColorShift = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bScanlineOffset = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDigitalNoise = true;
};

// Color grading settings
USTRUCT(BlueprintType)
struct FMGColorGradingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGColorGradingMode Mode = EMGColorGradingMode::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float Temperature = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float Tint = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Contrast = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "4.0"))
    float Gamma = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Gain = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float Offset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ShadowTint = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor MidtoneTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HighlightTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> LUTTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LUTIntensity = 0.0f;
};

// Speed effect settings for racing
USTRUCT(BlueprintType)
struct FMGSpeedEffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRadialBlur = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RadialBlurIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSpeedLines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SpeedLineIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFOVIncrease = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "30.0"))
    float MaxFOVIncrease = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bChromaticAberrationBoost = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedThreshold = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeedForEffect = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EffectRampUpTime = 0.5f;
};

// Nitro visual effect settings
USTRUCT(BlueprintType)
struct FMGNitroEffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScreenTintIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ScreenTintColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float BloomBoost = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRadialBlurPulse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bChromaticPulse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float PulseFrequency = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHeatDistortion = true;
};

// Pixelation settings
USTRUCT(BlueprintType)
struct FMGPixelationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPixelationMode Mode = EMGPixelationMode::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "32", ClampMax = "1920"))
    int32 CustomWidth = 480;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "24", ClampMax = "1080"))
    int32 CustomHeight = 270;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDithering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bColorReduction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2", ClampMax = "256"))
    int32 ColorPalette = 256;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAffineTextureMapping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bVertexSnapping = false;
};

// Complete post-process profile
USTRUCT(BlueprintType)
struct FMGPostProcessProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGVisualStyle VisualStyle = EMGVisualStyle::Modern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGBloomSettings Bloom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGMotionBlurSettings MotionBlur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGChromaticAberrationSettings ChromaticAberration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGFilmGrainSettings FilmGrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGVignetteSettings Vignette;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGCRTSettings CRT;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGVHSSettings VHS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGGlitchSettings Glitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGColorGradingSettings ColorGrading;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGSpeedEffectSettings SpeedEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGNitroEffectSettings NitroEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGPixelationSettings Pixelation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ExposureCompensation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoExposure = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnVisualStyleChanged, EMGVisualStyle, NewStyle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnProfileChanged, const FMGPostProcessProfile&, NewProfile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnEffectTriggered, EMGScreenEffect, Effect, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSpeedEffectIntensityChanged, float, Intensity);

/**
 * UMGPostProcessSubsystem
 * Y2K and retro visual effects system for Midnight Grind
 * Provides authentic PS1/PS2 aesthetics with modern post-processing
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPostProcessSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Visual style
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Style")
    void SetVisualStyle(EMGVisualStyle Style);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Style")
    EMGVisualStyle GetCurrentVisualStyle() const { return CurrentStyle; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Style")
    void ApplyProfile(const FMGPostProcessProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Style")
    void ApplyProfileByName(const FName& ProfileName);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Style")
    FMGPostProcessProfile GetCurrentProfile() const { return CurrentProfile; }

    UFUNCTION(BlueprintPure, Category = "PostProcess|Style")
    TArray<FName> GetAvailableProfileNames() const;

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Style")
    void SaveProfileAs(const FName& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Style")
    void DeleteProfile(const FName& ProfileName);

    // Individual effect control
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Bloom")
    void SetBloomSettings(const FMGBloomSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Bloom")
    void SetBloomIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Bloom")
    FMGBloomSettings GetBloomSettings() const { return CurrentProfile.Bloom; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|MotionBlur")
    void SetMotionBlurSettings(const FMGMotionBlurSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|MotionBlur")
    void SetMotionBlurAmount(float Amount);

    UFUNCTION(BlueprintPure, Category = "PostProcess|MotionBlur")
    FMGMotionBlurSettings GetMotionBlurSettings() const { return CurrentProfile.MotionBlur; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ChromaticAberration")
    void SetChromaticAberrationSettings(const FMGChromaticAberrationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ChromaticAberration")
    void SetChromaticAberrationIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category = "PostProcess|ChromaticAberration")
    FMGChromaticAberrationSettings GetChromaticAberrationSettings() const { return CurrentProfile.ChromaticAberration; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|FilmGrain")
    void SetFilmGrainSettings(const FMGFilmGrainSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|FilmGrain")
    void SetFilmGrainIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category = "PostProcess|FilmGrain")
    FMGFilmGrainSettings GetFilmGrainSettings() const { return CurrentProfile.FilmGrain; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Vignette")
    void SetVignetteSettings(const FMGVignetteSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Vignette")
    void SetVignetteIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Vignette")
    FMGVignetteSettings GetVignetteSettings() const { return CurrentProfile.Vignette; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ColorGrading")
    void SetColorGradingSettings(const FMGColorGradingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|ColorGrading")
    void SetColorGradingMode(EMGColorGradingMode Mode);

    UFUNCTION(BlueprintPure, Category = "PostProcess|ColorGrading")
    FMGColorGradingSettings GetColorGradingSettings() const { return CurrentProfile.ColorGrading; }

    // Retro effects
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void SetCRTSettings(const FMGCRTSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void EnableCRTEffect(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Retro")
    FMGCRTSettings GetCRTSettings() const { return CurrentProfile.CRT; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void SetVHSSettings(const FMGVHSSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void EnableVHSEffect(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Retro")
    FMGVHSSettings GetVHSSettings() const { return CurrentProfile.VHS; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void SetGlitchSettings(const FMGGlitchSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void TriggerGlitch(float Duration = 0.5f, float Intensity = 1.0f);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Retro")
    FMGGlitchSettings GetGlitchSettings() const { return CurrentProfile.Glitch; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void SetPixelationSettings(const FMGPixelationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Retro")
    void SetPixelationMode(EMGPixelationMode Mode);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Retro")
    FMGPixelationSettings GetPixelationSettings() const { return CurrentProfile.Pixelation; }

    // Racing-specific effects
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void SetSpeedEffectSettings(const FMGSpeedEffectSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void UpdateSpeedEffect(float CurrentSpeed);

    UFUNCTION(BlueprintPure, Category = "PostProcess|Racing")
    FMGSpeedEffectSettings GetSpeedEffectSettings() const { return CurrentProfile.SpeedEffects; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void SetNitroEffectSettings(const FMGNitroEffectSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void ActivateNitroEffect(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void PulseNitroEffect();

    UFUNCTION(BlueprintPure, Category = "PostProcess|Racing")
    FMGNitroEffectSettings GetNitroEffectSettings() const { return CurrentProfile.NitroEffects; }

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void TriggerDriftEffect(float DriftAngle);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void TriggerCollisionEffect(float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Racing")
    void TriggerBoostEffect(float Duration);

    // Temporary effects
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Temporary")
    void FlashScreen(FLinearColor Color, float Duration = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Temporary")
    void ShakeScreen(float Intensity, float Duration = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Temporary")
    void PulseVignette(float Intensity, float Duration = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Temporary")
    void RadialBlurPulse(float Intensity, float Duration = 0.2f);

    // Time of day
    UFUNCTION(BlueprintCallable, Category = "PostProcess|TimeOfDay")
    void SetTimeOfDay(EMGTimeOfDay Time);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|TimeOfDay")
    void SetTimeOfDayBlend(EMGTimeOfDay FromTime, EMGTimeOfDay ToTime, float Alpha);

    UFUNCTION(BlueprintPure, Category = "PostProcess|TimeOfDay")
    EMGTimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    // Utility
    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    void UpdateEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    void ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "PostProcess|Utility")
    void DisableAllEffects();

    UFUNCTION(BlueprintPure, Category = "PostProcess|Utility")
    float GetCurrentSpeedEffectIntensity() const { return CurrentSpeedEffectIntensity; }

    UFUNCTION(BlueprintPure, Category = "PostProcess|Utility")
    bool IsNitroEffectActive() const { return bNitroEffectActive; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "PostProcess|Events")
    FMGOnVisualStyleChanged OnVisualStyleChanged;

    UPROPERTY(BlueprintAssignable, Category = "PostProcess|Events")
    FMGOnProfileChanged OnProfileChanged;

    UPROPERTY(BlueprintAssignable, Category = "PostProcess|Events")
    FMGOnEffectTriggered OnEffectTriggered;

    UPROPERTY(BlueprintAssignable, Category = "PostProcess|Events")
    FMGOnSpeedEffectIntensityChanged OnSpeedEffectIntensityChanged;

protected:
    UPROPERTY()
    FMGPostProcessProfile CurrentProfile;

    UPROPERTY()
    TMap<FName, FMGPostProcessProfile> SavedProfiles;

    UPROPERTY()
    EMGVisualStyle CurrentStyle = EMGVisualStyle::Modern;

    UPROPERTY()
    EMGTimeOfDay CurrentTimeOfDay = EMGTimeOfDay::Noon;

    UPROPERTY()
    float CurrentSpeedEffectIntensity = 0.0f;

    UPROPERTY()
    bool bNitroEffectActive = false;

    UPROPERTY()
    float GlitchTimeRemaining = 0.0f;

    UPROPERTY()
    float FlashTimeRemaining = 0.0f;

    UPROPERTY()
    FLinearColor FlashColor = FLinearColor::White;

    FTimerHandle EffectUpdateHandle;
    FTimerHandle TemporaryEffectHandle;

    void InitializeDefaultProfiles();
    FMGPostProcessProfile CreateProfileForStyle(EMGVisualStyle Style);
    FMGColorGradingSettings GetColorGradingForTimeOfDay(EMGTimeOfDay Time);
    void ApplyEffectsToPostProcessVolume();
    void UpdateTemporaryEffects(float DeltaTime);
    void ProcessSpeedEffects(float Speed, float DeltaTime);
};
