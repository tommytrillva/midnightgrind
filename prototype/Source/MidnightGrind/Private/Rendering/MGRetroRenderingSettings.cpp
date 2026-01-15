// Copyright Midnight Grind. All Rights Reserved.

#include "Rendering/MGRetroRenderingSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Kismet/KismetMaterialLibrary.h"

// ==========================================
// UMGRetroRenderingComponent
// ==========================================

UMGRetroRenderingComponent::UMGRetroRenderingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void UMGRetroRenderingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bApplyOnBeginPlay)
	{
		ApplyConfiguration();
	}
}

void UMGRetroRenderingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRetroEffectsEnabled)
	{
		return;
	}

	// Update time-based effects
	TimeAccumulator += DeltaTime;

	// Update animated material parameters (noise, jitter, etc.)
	if (RetroParameterCollection)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			// Update time parameter for animated effects
			UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("Time"), TimeAccumulator);

			// Update jitter offset (changes each frame for PS1 wobble effect)
			if (RenderConfig.bEnableVertexJitter)
			{
				float JitterX = FMath::FRandRange(-1.0f, 1.0f) * RenderConfig.VertexJitterIntensity;
				float JitterY = FMath::FRandRange(-1.0f, 1.0f) * RenderConfig.VertexJitterIntensity;
				UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("JitterOffsetX"), JitterX);
				UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("JitterOffsetY"), JitterY);
			}
		}
	}
}

void UMGRetroRenderingComponent::ApplyPreset(EMGRetroIntensity Preset)
{
	RenderConfig = UMGRetroRenderingUtility::GetPresetConfig(Preset);
	ApplyConfiguration();
}

void UMGRetroRenderingComponent::SetConfiguration(const FMGRetroRenderConfig& NewConfig)
{
	RenderConfig = NewConfig;
	ApplyConfiguration();
}

void UMGRetroRenderingComponent::SetRetroEffectsEnabled(bool bEnabled)
{
	bRetroEffectsEnabled = bEnabled;

	if (bEnabled)
	{
		ApplyConfiguration();
	}
	else
	{
		// Reset to default rendering
		if (RetroParameterCollection)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("EffectsEnabled"), 0.0f);
			}
		}
	}
}

void UMGRetroRenderingComponent::SetResolutionScale(float Scale)
{
	RenderConfig.ResolutionScale = FMath::Clamp(Scale, 0.1f, 1.0f);
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetVertexSnapEnabled(bool bEnabled)
{
	RenderConfig.bEnableVertexSnap = bEnabled;
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetVertexSnapGridSize(float GridSize)
{
	RenderConfig.VertexSnapGridSize = FMath::Clamp(GridSize, 32.0f, 1024.0f);
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetColorLevels(int32 Levels)
{
	RenderConfig.ColorLevelsPerChannel = FMath::Clamp(Levels, 4, 256);
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetDitherPattern(EMGDitherPattern Pattern)
{
	RenderConfig.DitherPattern = Pattern;
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetDitherIntensity(float Intensity)
{
	RenderConfig.DitherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetCRTType(EMGCRTType Type)
{
	RenderConfig.CRTType = Type;
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetScanlineIntensity(float Intensity)
{
	RenderConfig.ScanlineIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::SetNeonGlowIntensity(float Intensity)
{
	RenderConfig.NeonGlowIntensity = FMath::Clamp(Intensity, 0.0f, 3.0f);
	UpdateMaterialParameters();
}

void UMGRetroRenderingComponent::ApplyConfiguration()
{
	UpdateMaterialParameters();
	ConfigurePostProcess();
}

void UMGRetroRenderingComponent::UpdateMaterialParameters()
{
	if (!RetroParameterCollection)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Effects enabled
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("EffectsEnabled"),
		bRetroEffectsEnabled ? 1.0f : 0.0f);

	// Resolution
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ResolutionScale"),
		RenderConfig.ResolutionScale);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("PointFilter"),
		RenderConfig.bPointFilterUpscale ? 1.0f : 0.0f);

	// Vertex effects
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("VertexSnapEnabled"),
		RenderConfig.bEnableVertexSnap ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("VertexSnapGridSize"),
		RenderConfig.VertexSnapGridSize);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("VertexJitterEnabled"),
		RenderConfig.bEnableVertexJitter ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("VertexJitterIntensity"),
		RenderConfig.VertexJitterIntensity);

	// Texture effects
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("AffineEnabled"),
		RenderConfig.bEnableAffineMapping ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("AffineIntensity"),
		RenderConfig.AffineMappingIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("TextureLODBias"),
		RenderConfig.bEnableLODBias ? RenderConfig.TextureLODBias : 0.0f);

	// Color quantization
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ColorQuantEnabled"),
		RenderConfig.bEnableColorQuantization ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ColorLevels"),
		static_cast<float>(RenderConfig.ColorLevelsPerChannel));

	// Dithering
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("DitherPattern"),
		static_cast<float>(RenderConfig.DitherPattern));
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("DitherIntensity"),
		RenderConfig.DitherIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("DitherSpread"),
		RenderConfig.DitherSpread);

	// CRT effects
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("CRTType"),
		static_cast<float>(RenderConfig.CRTType));
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ScanlineIntensity"),
		RenderConfig.ScanlineIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ScanlineScale"),
		RenderConfig.ScanlineScale);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("CRTCurvature"),
		RenderConfig.CRTCurvature);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ChromaticAberration"),
		RenderConfig.ChromaticAberration);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("PhosphorGlowEnabled"),
		RenderConfig.bEnablePhosphorGlow ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("PhosphorGlowIntensity"),
		RenderConfig.PhosphorGlowIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("VignetteEnabled"),
		RenderConfig.bEnableVignette ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("VignetteIntensity"),
		RenderConfig.VignetteIntensity);

	// Noise
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("NoiseEnabled"),
		RenderConfig.bEnableNoise ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("NoiseIntensity"),
		RenderConfig.NoiseIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("ColoredNoise"),
		RenderConfig.bColoredNoise ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("InterlacingEnabled"),
		RenderConfig.bEnableInterlacing ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("InterlacingIntensity"),
		RenderConfig.InterlacingIntensity);

	// Fog
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("DistanceFogEnabled"),
		RenderConfig.bEnableDistanceFog ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("FogStartDistance"),
		RenderConfig.FogStartDistance);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("FogDensity"),
		RenderConfig.FogDensity);
	UKismetMaterialLibrary::SetVectorParameterValue(World, RetroParameterCollection, TEXT("FogColor"),
		RenderConfig.FogColor);

	// Neon glow
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("NeonGlowEnabled"),
		RenderConfig.bEnableNeonGlow ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("NeonGlowThreshold"),
		RenderConfig.NeonGlowThreshold);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("NeonGlowIntensity"),
		RenderConfig.NeonGlowIntensity);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("LightStreaksEnabled"),
		RenderConfig.bEnableLightStreaks ? 1.0f : 0.0f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, RetroParameterCollection, TEXT("LightStreakLength"),
		RenderConfig.LightStreakLength);
}

void UMGRetroRenderingComponent::ConfigurePostProcess()
{
	// Find or create post-process volume
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// TODO: Find/create and configure post-process volume with retro material
	// This would typically be done in Blueprint or through level design
}

// ==========================================
// UMGRetroRenderingUtility
// ==========================================

FMGRetroRenderConfig UMGRetroRenderingUtility::GetPresetConfig(EMGRetroIntensity Preset)
{
	FMGRetroRenderConfig Config;

	switch (Preset)
	{
	case EMGRetroIntensity::Subtle:
		// Light retro touches, mostly modern look
		Config.bEnableLowResolution = false;
		Config.ResolutionScale = 1.0f;
		Config.bEnableVertexSnap = false;
		Config.bEnableVertexJitter = false;
		Config.bEnableAffineMapping = false;
		Config.bEnableColorQuantization = true;
		Config.ColorLevelsPerChannel = 128;
		Config.DitherPattern = EMGDitherPattern::None;
		Config.DitherIntensity = 0.0f;
		Config.CRTType = EMGCRTType::None;
		Config.ScanlineIntensity = 0.0f;
		Config.bEnableVignette = true;
		Config.VignetteIntensity = 0.15f;
		Config.bEnableNoise = true;
		Config.NoiseIntensity = 0.05f;
		Config.bEnableNeonGlow = true;
		Config.NeonGlowIntensity = 1.0f;
		break;

	case EMGRetroIntensity::Medium:
		// Balanced PS2-era look
		Config.bEnableLowResolution = true;
		Config.ResolutionScale = 0.75f;
		Config.bPointFilterUpscale = false; // Bilinear for PS2 smoothness
		Config.bEnableVertexSnap = false;
		Config.bEnableVertexJitter = true;
		Config.VertexJitterIntensity = 0.15f;
		Config.bEnableAffineMapping = false;
		Config.bEnableColorQuantization = true;
		Config.ColorLevelsPerChannel = 64;
		Config.DitherPattern = EMGDitherPattern::Bayer8x8;
		Config.DitherIntensity = 0.3f;
		Config.DitherSpread = 0.3f;
		Config.CRTType = EMGCRTType::Standard;
		Config.ScanlineIntensity = 0.2f;
		Config.CRTCurvature = 0.05f;
		Config.ChromaticAberration = 0.1f;
		Config.bEnablePhosphorGlow = true;
		Config.PhosphorGlowIntensity = 0.3f;
		Config.bEnableVignette = true;
		Config.VignetteIntensity = 0.25f;
		Config.bEnableNoise = true;
		Config.NoiseIntensity = 0.08f;
		Config.bEnableNeonGlow = true;
		Config.NeonGlowIntensity = 1.2f;
		break;

	case EMGRetroIntensity::Authentic:
		// Strong PS1-era look - THE TARGET AESTHETIC
		Config.bEnableLowResolution = true;
		Config.ResolutionScale = 0.5f;
		Config.bPointFilterUpscale = true; // Chunky pixels
		Config.bEnableVertexSnap = true;
		Config.VertexSnapGridSize = 160.0f;
		Config.bEnableVertexJitter = true;
		Config.VertexJitterIntensity = 0.3f;
		Config.bEnableAffineMapping = true;
		Config.AffineMappingIntensity = 0.5f;
		Config.bEnableLODBias = true;
		Config.TextureLODBias = 1.5f;
		Config.bEnableColorQuantization = true;
		Config.ColorLevelsPerChannel = 32; // PS1 5-bit color
		Config.DitherPattern = EMGDitherPattern::Bayer4x4;
		Config.DitherIntensity = 0.5f;
		Config.DitherSpread = 0.5f;
		Config.CRTType = EMGCRTType::Standard;
		Config.ScanlineIntensity = 0.35f;
		Config.ScanlineScale = 1.0f;
		Config.CRTCurvature = 0.1f;
		Config.ChromaticAberration = 0.2f;
		Config.bEnablePhosphorGlow = true;
		Config.PhosphorGlowIntensity = 0.5f;
		Config.bEnableVignette = true;
		Config.VignetteIntensity = 0.3f;
		Config.bEnableNoise = true;
		Config.NoiseIntensity = 0.12f;
		Config.bEnableDistanceFog = true;
		Config.FogStartDistance = 2000.0f;
		Config.FogDensity = 0.5f;
		Config.bEnableNeonGlow = true;
		Config.NeonGlowIntensity = 1.5f;
		Config.bEnableLightStreaks = true;
		Config.LightStreakLength = 0.3f;
		break;

	case EMGRetroIntensity::Extreme:
		// Very lo-fi, stylized
		Config.bEnableLowResolution = true;
		Config.ResolutionScale = 0.25f;
		Config.bPointFilterUpscale = true;
		Config.bEnableVertexSnap = true;
		Config.VertexSnapGridSize = 100.0f;
		Config.bEnableVertexJitter = true;
		Config.VertexJitterIntensity = 0.5f;
		Config.bEnableAffineMapping = true;
		Config.AffineMappingIntensity = 0.8f;
		Config.bEnableLODBias = true;
		Config.TextureLODBias = 3.0f;
		Config.bEnableColorQuantization = true;
		Config.ColorLevelsPerChannel = 16;
		Config.DitherPattern = EMGDitherPattern::Bayer4x4;
		Config.DitherIntensity = 0.7f;
		Config.DitherSpread = 0.7f;
		Config.CRTType = EMGCRTType::Composite;
		Config.ScanlineIntensity = 0.5f;
		Config.CRTCurvature = 0.15f;
		Config.ChromaticAberration = 0.4f;
		Config.bEnablePhosphorGlow = true;
		Config.PhosphorGlowIntensity = 0.8f;
		Config.bEnableVignette = true;
		Config.VignetteIntensity = 0.4f;
		Config.bEnableNoise = true;
		Config.NoiseIntensity = 0.2f;
		Config.bEnableInterlacing = true;
		Config.InterlacingIntensity = 0.4f;
		Config.bEnableDistanceFog = true;
		Config.FogStartDistance = 1500.0f;
		Config.FogDensity = 0.7f;
		Config.bEnableNeonGlow = true;
		Config.NeonGlowIntensity = 2.0f;
		Config.bEnableLightStreaks = true;
		Config.LightStreakLength = 0.5f;
		break;

	case EMGRetroIntensity::Custom:
	default:
		// Return default config
		break;
	}

	return Config;
}

float UMGRetroRenderingUtility::CalculateBayerDither(int32 X, int32 Y, int32 Size)
{
	// 4x4 Bayer matrix (classic PS1 dither)
	static const float Bayer4x4[4][4] = {
		{ 0.0f / 16.0f,  8.0f / 16.0f,  2.0f / 16.0f, 10.0f / 16.0f },
		{ 12.0f / 16.0f, 4.0f / 16.0f, 14.0f / 16.0f,  6.0f / 16.0f },
		{ 3.0f / 16.0f, 11.0f / 16.0f,  1.0f / 16.0f,  9.0f / 16.0f },
		{ 15.0f / 16.0f, 7.0f / 16.0f, 13.0f / 16.0f,  5.0f / 16.0f }
	};

	// 8x8 Bayer matrix
	static const float Bayer8x8[8][8] = {
		{ 0.0f / 64.0f, 32.0f / 64.0f,  8.0f / 64.0f, 40.0f / 64.0f,  2.0f / 64.0f, 34.0f / 64.0f, 10.0f / 64.0f, 42.0f / 64.0f },
		{ 48.0f / 64.0f, 16.0f / 64.0f, 56.0f / 64.0f, 24.0f / 64.0f, 50.0f / 64.0f, 18.0f / 64.0f, 58.0f / 64.0f, 26.0f / 64.0f },
		{ 12.0f / 64.0f, 44.0f / 64.0f,  4.0f / 64.0f, 36.0f / 64.0f, 14.0f / 64.0f, 46.0f / 64.0f,  6.0f / 64.0f, 38.0f / 64.0f },
		{ 60.0f / 64.0f, 28.0f / 64.0f, 52.0f / 64.0f, 20.0f / 64.0f, 62.0f / 64.0f, 30.0f / 64.0f, 54.0f / 64.0f, 22.0f / 64.0f },
		{ 3.0f / 64.0f, 35.0f / 64.0f, 11.0f / 64.0f, 43.0f / 64.0f,  1.0f / 64.0f, 33.0f / 64.0f,  9.0f / 64.0f, 41.0f / 64.0f },
		{ 51.0f / 64.0f, 19.0f / 64.0f, 59.0f / 64.0f, 27.0f / 64.0f, 49.0f / 64.0f, 17.0f / 64.0f, 57.0f / 64.0f, 25.0f / 64.0f },
		{ 15.0f / 64.0f, 47.0f / 64.0f,  7.0f / 64.0f, 39.0f / 64.0f, 13.0f / 64.0f, 45.0f / 64.0f,  5.0f / 64.0f, 37.0f / 64.0f },
		{ 63.0f / 64.0f, 31.0f / 64.0f, 55.0f / 64.0f, 23.0f / 64.0f, 61.0f / 64.0f, 29.0f / 64.0f, 53.0f / 64.0f, 21.0f / 64.0f }
	};

	if (Size <= 4)
	{
		return Bayer4x4[Y % 4][X % 4];
	}
	else
	{
		return Bayer8x8[Y % 8][X % 8];
	}
}

FLinearColor UMGRetroRenderingUtility::QuantizeColor(const FLinearColor& Color, int32 LevelsPerChannel)
{
	float Levels = static_cast<float>(FMath::Max(2, LevelsPerChannel));
	float Step = 1.0f / (Levels - 1.0f);

	FLinearColor Result;
	Result.R = FMath::RoundToFloat(Color.R / Step) * Step;
	Result.G = FMath::RoundToFloat(Color.G / Step) * Step;
	Result.B = FMath::RoundToFloat(Color.B / Step) * Step;
	Result.A = Color.A;

	return Result;
}

FVector UMGRetroRenderingUtility::SnapVertexPosition(const FVector& Position, float GridSize)
{
	if (GridSize <= 0.0f)
	{
		return Position;
	}

	return FVector(
		FMath::RoundToFloat(Position.X / GridSize) * GridSize,
		FMath::RoundToFloat(Position.Y / GridSize) * GridSize,
		FMath::RoundToFloat(Position.Z / GridSize) * GridSize
	);
}

FVector2D UMGRetroRenderingUtility::CalculateAffineUV(const FVector2D& UV, float Depth, float Intensity)
{
	// PS1 didn't have perspective-correct texture mapping
	// This simulates the "warping" effect by not dividing by depth
	if (Depth <= 0.0f || Intensity <= 0.0f)
	{
		return UV;
	}

	// Blend between perspective-correct (modern) and affine (PS1)
	FVector2D AffineUV = UV * Depth;
	return FMath::Lerp(UV, AffineUV, Intensity);
}
