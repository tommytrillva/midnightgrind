// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGPostProcessSubsystem.generated.h"

class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * Visual preset
 */
UENUM(BlueprintType)
enum class EMGVisualPreset : uint8
{
	/** Modern look */
	Modern,
	/** PS2 era aesthetic */
	PS2,
	/** PS1 era aesthetic */
	PS1,
	/** VHS/CRT filter */
	Retro,
	/** Clean arcade look */
	Arcade,
	/** Stylized noir */
	Noir,
	/** Custom user settings */
	Custom
};

/**
 * Screen effect type
 */
UENUM(BlueprintType)
enum class EMGScreenEffect : uint8
{
	/** Radial speed lines */
	SpeedLines,
	/** Screen shake */
	ScreenShake,
	/** Motion blur */
	MotionBlur,
	/** Chromatic aberration */
	ChromaticAberration,
	/** Vignette */
	Vignette,
	/** Film grain */
	FilmGrain,
	/** Bloom */
	Bloom,
	/** Color grading */
	ColorGrade
};

/**
 * PS1/PS2 aesthetic settings
 */
USTRUCT(BlueprintType)
struct FMGRetroSettings
{
	GENERATED_BODY()

	/** Enable vertex jitter (PS1 wobble) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableVertexJitter = false;

	/** Vertex jitter intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VertexJitterIntensity = 0.5f;

	/** Enable texture affine mapping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAffineMapping = false;

	/** Enable resolution downscale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableResolutionDownscale = false;

	/** Resolution scale (1.0 = native) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.25", ClampMax = "1.0"))
	float ResolutionScale = 1.0f;

	/** Enable color banding (reduce color depth) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableColorBanding = false;

	/** Color depth (bits per channel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "2", ClampMax = "8"))
	int32 ColorDepth = 5;

	/** Enable dithering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableDithering = false;

	/** Dither intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DitherIntensity = 0.5f;

	/** Enable scanlines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableScanlines = false;

	/** Scanline intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScanlineIntensity = 0.3f;

	/** Enable CRT curvature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableCRTCurvature = false;

	/** CRT curvature amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CRTCurvature = 0.1f;
};

/**
 * Post-process settings
 */
USTRUCT(BlueprintType)
struct FMGPostProcessSettings
{
	GENERATED_BODY()

	/** Bloom intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BloomIntensity = 0.5f;

	/** Bloom threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BloomThreshold = 1.0f;

	/** Chromatic aberration intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChromaticAberration = 0.0f;

	/** Vignette intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VignetteIntensity = 0.3f;

	/** Film grain intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FilmGrainIntensity = 0.0f;

	/** Motion blur intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MotionBlurIntensity = 0.5f;

	/** Motion blur max velocity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MotionBlurMaxVelocity = 1000.0f;

	/** Exposure compensation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExposureCompensation = 0.0f;

	/** Contrast */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float Contrast = 1.0f;

	/** Saturation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Saturation = 1.0f;

	/** Color temperature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1500", ClampMax = "15000"))
	float ColorTemperature = 6500.0f;

	/** Tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorTint = FLinearColor::White;

	/** Retro settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRetroSettings RetroSettings;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisualPresetChanged, EMGVisualPreset, NewPreset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostProcessSettingsChanged, const FMGPostProcessSettings&, Settings);

/**
 * Post-Process Subsystem
 * Manages visual effects and post-processing
 *
 * Features:
 * - Visual presets (PS1/PS2 aesthetics)
 * - Dynamic post-processing
 * - Screen effects
 * - Color grading
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPostProcessSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnVisualPresetChanged OnVisualPresetChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPostProcessSettingsChanged OnPostProcessSettingsChanged;

	// ==========================================
	// PRESETS
	// ==========================================

	/** Set visual preset */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void SetVisualPreset(EMGVisualPreset Preset);

	/** Get current preset */
	UFUNCTION(BlueprintPure, Category = "Visual")
	EMGVisualPreset GetCurrentPreset() const { return CurrentPreset; }

	/** Get preset settings */
	UFUNCTION(BlueprintPure, Category = "Visual")
	FMGPostProcessSettings GetPresetSettings(EMGVisualPreset Preset) const;

	// ==========================================
	// SETTINGS
	// ==========================================

	/** Get current settings */
	UFUNCTION(BlueprintPure, Category = "Visual")
	FMGPostProcessSettings GetCurrentSettings() const { return CurrentSettings; }

	/** Apply settings */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ApplySettings(const FMGPostProcessSettings& Settings);

	/** Reset to preset defaults */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ResetToPresetDefaults();

	// ==========================================
	// INDIVIDUAL EFFECTS
	// ==========================================

	/** Set bloom intensity */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetBloomIntensity(float Intensity);

	/** Set motion blur intensity */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetMotionBlurIntensity(float Intensity);

	/** Set chromatic aberration */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetChromaticAberration(float Intensity);

	/** Set vignette intensity */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetVignetteIntensity(float Intensity);

	/** Set film grain */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetFilmGrainIntensity(float Intensity);

	/** Set saturation */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetSaturation(float Saturation);

	/** Set contrast */
	UFUNCTION(BlueprintCallable, Category = "Visual|Effects")
	void SetContrast(float Contrast);

	// ==========================================
	// SPEED EFFECTS
	// ==========================================

	/** Update speed-based effects */
	UFUNCTION(BlueprintCallable, Category = "Visual|Speed")
	void UpdateSpeedEffects(float SpeedKPH, float MaxSpeedKPH);

	/** Set speed lines intensity */
	UFUNCTION(BlueprintCallable, Category = "Visual|Speed")
	void SetSpeedLinesIntensity(float Intensity);

	/** Set radial blur for speed */
	UFUNCTION(BlueprintCallable, Category = "Visual|Speed")
	void SetSpeedRadialBlur(float Intensity);

	// ==========================================
	// RETRO EFFECTS
	// ==========================================

	/** Enable/disable retro effects */
	UFUNCTION(BlueprintCallable, Category = "Visual|Retro")
	void SetRetroEffectsEnabled(bool bEnabled);

	/** Set retro settings */
	UFUNCTION(BlueprintCallable, Category = "Visual|Retro")
	void SetRetroSettings(const FMGRetroSettings& Settings);

	/** Set resolution scale */
	UFUNCTION(BlueprintCallable, Category = "Visual|Retro")
	void SetResolutionScale(float Scale);

	/** Set color depth */
	UFUNCTION(BlueprintCallable, Category = "Visual|Retro")
	void SetColorDepth(int32 Depth);

	// ==========================================
	// SCREEN EFFECTS
	// ==========================================

	/** Flash screen */
	UFUNCTION(BlueprintCallable, Category = "Visual|Screen")
	void FlashScreen(FLinearColor Color, float Duration = 0.1f);

	/** Fade to color */
	UFUNCTION(BlueprintCallable, Category = "Visual|Screen")
	void FadeToColor(FLinearColor Color, float Duration = 0.5f);

	/** Fade from color */
	UFUNCTION(BlueprintCallable, Category = "Visual|Screen")
	void FadeFromColor(FLinearColor Color, float Duration = 0.5f);

	/** Pulse vignette */
	UFUNCTION(BlueprintCallable, Category = "Visual|Screen")
	void PulseVignette(float Intensity, float Duration = 0.3f);

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Post-process material for retro effects */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* RetroPostProcessMaterial;

	/** Speed lines material */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* SpeedLinesMaterial;

	// ==========================================
	// STATE
	// ==========================================

	/** Current visual preset */
	EMGVisualPreset CurrentPreset = EMGVisualPreset::Modern;

	/** Current settings */
	FMGPostProcessSettings CurrentSettings;

	/** Post-process component */
	UPROPERTY()
	UPostProcessComponent* PostProcessComponent;

	/** Retro material instance */
	UPROPERTY()
	UMaterialInstanceDynamic* RetroMaterialInstance;

	/** Speed lines material instance */
	UPROPERTY()
	UMaterialInstanceDynamic* SpeedLinesMaterialInstance;

	/** Current speed lines intensity */
	float CurrentSpeedLinesIntensity = 0.0f;

	/** Target speed lines intensity */
	float TargetSpeedLinesIntensity = 0.0f;

	/** Fade state */
	bool bIsFading = false;
	FLinearColor FadeColor;
	float FadeDuration = 0.0f;
	float FadeElapsed = 0.0f;
	bool bFadingIn = false;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize post-process component */
	void SetupPostProcess();

	/** Update post-process settings */
	void UpdatePostProcess();

	/** Update retro effects */
	void UpdateRetroEffects();

	/** Update speed lines */
	void UpdateSpeedLines(float DeltaTime);

	/** Update fade */
	void UpdateFade(float DeltaTime);

	/** Get preset defaults */
	FMGPostProcessSettings GetPS1Preset() const;
	FMGPostProcessSettings GetPS2Preset() const;
	FMGPostProcessSettings GetModernPreset() const;
	FMGPostProcessSettings GetArcadePreset() const;
	FMGPostProcessSettings GetNoirPreset() const;
};
