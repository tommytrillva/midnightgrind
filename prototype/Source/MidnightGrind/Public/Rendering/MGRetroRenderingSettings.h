// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "MGRetroRenderingSettings.generated.h"

/**
 * PS1/PS2 era visual effect intensity presets
 */
UENUM(BlueprintType)
enum class EMGRetroIntensity : uint8
{
	/** Subtle retro touches, modern clarity */
	Subtle		UMETA(DisplayName = "Subtle"),

	/** Balanced PS2-era look */
	Medium		UMETA(DisplayName = "Medium"),

	/** Strong PS1-era look */
	Authentic	UMETA(DisplayName = "Authentic"),

	/** Extreme lo-fi, very stylized */
	Extreme		UMETA(DisplayName = "Extreme"),

	/** Custom user settings */
	Custom		UMETA(DisplayName = "Custom")
};

/**
 * CRT display simulation type
 */
UENUM(BlueprintType)
enum class EMGCRTType : uint8
{
	/** No CRT effect */
	None		UMETA(DisplayName = "None"),

	/** Standard consumer CRT */
	Standard	UMETA(DisplayName = "Standard CRT"),

	/** Arcade cabinet style */
	Arcade		UMETA(DisplayName = "Arcade Monitor"),

	/** PVM/BVM professional monitor */
	PVM			UMETA(DisplayName = "PVM/Professional"),

	/** Composite video artifacts */
	Composite	UMETA(DisplayName = "Composite Video"),

	/** RF video with noise */
	RF			UMETA(DisplayName = "RF Connection")
};

/**
 * Dithering pattern type
 */
UENUM(BlueprintType)
enum class EMGDitherPattern : uint8
{
	/** No dithering */
	None		UMETA(DisplayName = "None"),

	/** Classic PS1 ordered dither (Bayer 4x4) */
	Bayer4x4	UMETA(DisplayName = "Bayer 4x4"),

	/** Finer Bayer pattern */
	Bayer8x8	UMETA(DisplayName = "Bayer 8x8"),

	/** Blue noise dithering */
	BlueNoise	UMETA(DisplayName = "Blue Noise"),

	/** Halftone dots */
	Halftone	UMETA(DisplayName = "Halftone")
};

/**
 * Configuration for PS1/PS2 era rendering style
 */
USTRUCT(BlueprintType)
struct FMGRetroRenderConfig
{
	GENERATED_BODY()

	// ==========================================
	// RESOLUTION & RENDERING
	// ==========================================

	/** Enable low-resolution rendering (render at lower res, upscale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution")
	bool bEnableLowResolution = true;

	/** Target resolution scale (0.25 = 1/4 native, authentic PS1 look) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ResolutionScale = 0.5f;

	/** Use point (nearest-neighbor) filtering for pixelated upscale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution")
	bool bPointFilterUpscale = true;

	// ==========================================
	// VERTEX EFFECTS (PS1 Characteristic)
	// ==========================================

	/** Enable vertex position snapping (PS1 wobble) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertex Effects")
	bool bEnableVertexSnap = true;

	/** Vertex snap grid size (lower = more wobble). PS1: ~160 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertex Effects", meta = (ClampMin = "32", ClampMax = "1024"))
	float VertexSnapGridSize = 160.0f;

	/** Enable vertex jitter (slight random movement per frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertex Effects")
	bool bEnableVertexJitter = true;

	/** Jitter intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertex Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VertexJitterIntensity = 0.3f;

	// ==========================================
	// TEXTURE EFFECTS
	// ==========================================

	/** Enable affine texture mapping simulation (PS1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Effects")
	bool bEnableAffineMapping = true;

	/** Affine distortion intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AffineMappingIntensity = 0.5f;

	/** Enable texture LOD bias (blurrier textures at distance, like PS1/PS2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Effects")
	bool bEnableLODBias = true;

	/** Texture LOD bias amount (higher = blurrier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Effects", meta = (ClampMin = "0.0", ClampMax = "4.0"))
	float TextureLODBias = 1.0f;

	// ==========================================
	// COLOR & DITHERING
	// ==========================================

	/** Enable color quantization (limited color palette) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	bool bEnableColorQuantization = true;

	/** Color depth per channel (PS1: 5-bit = 32 levels, PS2: 8-bit = 256) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "4", ClampMax = "256"))
	int32 ColorLevelsPerChannel = 32;

	/** Dithering pattern type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	EMGDitherPattern DitherPattern = EMGDitherPattern::Bayer4x4;

	/** Dithering intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DitherIntensity = 0.5f;

	/** Dither spread (affects color banding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DitherSpread = 0.5f;

	// ==========================================
	// CRT SIMULATION
	// ==========================================

	/** CRT type to simulate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT")
	EMGCRTType CRTType = EMGCRTType::Standard;

	/** Scanline intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScanlineIntensity = 0.3f;

	/** Scanline count multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT", meta = (ClampMin = "0.5", ClampMax = "4.0"))
	float ScanlineScale = 1.0f;

	/** CRT curvature (0 = flat, 1 = curved like classic CRT) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CRTCurvature = 0.1f;

	/** Chromatic aberration (color fringing at edges) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChromaticAberration = 0.2f;

	/** Enable phosphor glow (bloom on bright areas) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT")
	bool bEnablePhosphorGlow = true;

	/** Phosphor glow intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float PhosphorGlowIntensity = 0.5f;

	/** Enable vignette (darkened corners) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT")
	bool bEnableVignette = true;

	/** Vignette intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CRT", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VignetteIntensity = 0.3f;

	// ==========================================
	// NOISE & ARTIFACTS
	// ==========================================

	/** Enable film grain / static noise */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	bool bEnableNoise = true;

	/** Noise intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NoiseIntensity = 0.1f;

	/** Enable color noise (colored static vs grayscale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	bool bColoredNoise = false;

	/** Enable interlacing effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	bool bEnableInterlacing = false;

	/** Interlacing intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InterlacingIntensity = 0.5f;

	// ==========================================
	// LIGHTING & FOG
	// ==========================================

	/** Enable vertex lighting (per-vertex instead of per-pixel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	bool bEnableVertexLighting = true;

	/** Enable draw distance fog (like PS1/PS2 fog) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	bool bEnableDistanceFog = true;

	/** Fog start distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "100.0"))
	float FogStartDistance = 2000.0f;

	/** Fog density */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.5f;

	/** Fog color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	FLinearColor FogColor = FLinearColor(0.05f, 0.02f, 0.1f, 1.0f); // Dark purple, night vibes

	// ==========================================
	// SPECIAL EFFECTS
	// ==========================================

	/** Enable neon glow post-process (for the midnight aesthetic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special")
	bool bEnableNeonGlow = true;

	/** Neon glow threshold (brightness level to trigger glow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float NeonGlowThreshold = 0.8f;

	/** Neon glow intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float NeonGlowIntensity = 1.5f;

	/** Enable light streaks on neon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special")
	bool bEnableLightStreaks = true;

	/** Light streak length */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LightStreakLength = 0.3f;
};

/**
 * Component that manages PS1/PS2 era retro rendering effects
 * Attach to a camera or use globally to achieve the MIDNIGHT GRIND aesthetic
 */
UCLASS(ClassGroup = (MidnightGrind), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGRetroRenderingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGRetroRenderingComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Apply a preset intensity level */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering")
	void ApplyPreset(EMGRetroIntensity Preset);

	/** Get current configuration */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	FMGRetroRenderConfig GetConfiguration() const { return RenderConfig; }

	/** Set full configuration */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering")
	void SetConfiguration(const FMGRetroRenderConfig& NewConfig);

	/** Enable/disable all retro effects */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering")
	void SetRetroEffectsEnabled(bool bEnabled);

	/** Check if retro effects are enabled */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	bool AreRetroEffectsEnabled() const { return bRetroEffectsEnabled; }

	// ==========================================
	// INDIVIDUAL EFFECT CONTROL
	// ==========================================

	/** Set resolution scale */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Resolution")
	void SetResolutionScale(float Scale);

	/** Set vertex snap enabled */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Vertex")
	void SetVertexSnapEnabled(bool bEnabled);

	/** Set vertex snap grid size */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Vertex")
	void SetVertexSnapGridSize(float GridSize);

	/** Set color quantization levels */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Color")
	void SetColorLevels(int32 Levels);

	/** Set dither pattern */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Color")
	void SetDitherPattern(EMGDitherPattern Pattern);

	/** Set dither intensity */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Color")
	void SetDitherIntensity(float Intensity);

	/** Set CRT type */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|CRT")
	void SetCRTType(EMGCRTType Type);

	/** Set scanline intensity */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|CRT")
	void SetScanlineIntensity(float Intensity);

	/** Set neon glow intensity */
	UFUNCTION(BlueprintCallable, Category = "Retro Rendering|Special")
	void SetNeonGlowIntensity(float Intensity);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Current retro render configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FMGRetroRenderConfig RenderConfig;

	/** Post-process material for retro effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TObjectPtr<UMaterialInterface> RetroPostProcessMaterial;

	/** Material parameter collection for shader parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TObjectPtr<UMaterialParameterCollection> RetroParameterCollection;

	/** Whether retro effects are currently enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bRetroEffectsEnabled = true;

	/** Auto-apply settings on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bApplyOnBeginPlay = true;

protected:
	/** Apply current configuration to materials and render settings */
	virtual void ApplyConfiguration();

	/** Update material parameter collection with current settings */
	virtual void UpdateMaterialParameters();

	/** Configure post-process volume */
	virtual void ConfigurePostProcess();

private:
	/** Cached reference to post-process volume */
	UPROPERTY()
	TWeakObjectPtr<APostProcessVolume> CachedPostProcessVolume;

	/** Time accumulator for animated effects */
	float TimeAccumulator = 0.0f;
};

/**
 * Static utility class for retro rendering calculations
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRetroRenderingUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Get preset configuration for an intensity level */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	static FMGRetroRenderConfig GetPresetConfig(EMGRetroIntensity Preset);

	/** Calculate Bayer dither value for a position */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	static float CalculateBayerDither(int32 X, int32 Y, int32 Size = 4);

	/** Quantize a color to limited palette */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	static FLinearColor QuantizeColor(const FLinearColor& Color, int32 LevelsPerChannel);

	/** Apply vertex snap to a position (CPU-side, for preview) */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	static FVector SnapVertexPosition(const FVector& Position, float GridSize);

	/** Calculate affine texture coordinate distortion */
	UFUNCTION(BlueprintPure, Category = "Retro Rendering")
	static FVector2D CalculateAffineUV(const FVector2D& UV, float Depth, float Intensity);
};
