// Copyright Midnight Grind. All Rights Reserved.

#include "Visual/MGPostProcessSubsystem.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"

void UMGPostProcessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SetupPostProcess();

	// Start with modern preset
	SetVisualPreset(EMGVisualPreset::Modern);
}

void UMGPostProcessSubsystem::Deinitialize()
{
	if (PostProcessComponent)
	{
		PostProcessComponent->DestroyComponent();
		PostProcessComponent = nullptr;
	}

	Super::Deinitialize();
}

void UMGPostProcessSubsystem::Tick(float DeltaTime)
{
	UpdateSpeedLines(DeltaTime);
	UpdateFade(DeltaTime);
}

// ==========================================
// PRESETS
// ==========================================

void UMGPostProcessSubsystem::SetVisualPreset(EMGVisualPreset Preset)
{
	CurrentPreset = Preset;

	FMGPostProcessSettings PresetSettings = GetPresetSettings(Preset);
	ApplySettings(PresetSettings);

	OnVisualPresetChanged.Broadcast(Preset);
}

FMGPostProcessSettings UMGPostProcessSubsystem::GetPresetSettings(EMGVisualPreset Preset) const
{
	switch (Preset)
	{
		case EMGVisualPreset::PS1:
			return GetPS1Preset();
		case EMGVisualPreset::PS2:
			return GetPS2Preset();
		case EMGVisualPreset::Modern:
			return GetModernPreset();
		case EMGVisualPreset::Arcade:
			return GetArcadePreset();
		case EMGVisualPreset::Noir:
			return GetNoirPreset();
		case EMGVisualPreset::Custom:
			return CurrentSettings;
		default:
			return GetModernPreset();
	}
}

// ==========================================
// SETTINGS
// ==========================================

void UMGPostProcessSubsystem::ApplySettings(const FMGPostProcessSettings& Settings)
{
	CurrentSettings = Settings;
	UpdatePostProcess();
	UpdateRetroEffects();

	OnPostProcessSettingsChanged.Broadcast(Settings);
}

void UMGPostProcessSubsystem::ResetToPresetDefaults()
{
	SetVisualPreset(CurrentPreset);
}

// ==========================================
// INDIVIDUAL EFFECTS
// ==========================================

void UMGPostProcessSubsystem::SetBloomIntensity(float Intensity)
{
	CurrentSettings.BloomIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

void UMGPostProcessSubsystem::SetMotionBlurIntensity(float Intensity)
{
	CurrentSettings.MotionBlurIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

void UMGPostProcessSubsystem::SetChromaticAberration(float Intensity)
{
	CurrentSettings.ChromaticAberration = FMath::Clamp(Intensity, 0.0f, 1.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

void UMGPostProcessSubsystem::SetVignetteIntensity(float Intensity)
{
	CurrentSettings.VignetteIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

void UMGPostProcessSubsystem::SetFilmGrainIntensity(float Intensity)
{
	CurrentSettings.FilmGrainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

void UMGPostProcessSubsystem::SetSaturation(float Saturation)
{
	CurrentSettings.Saturation = FMath::Clamp(Saturation, 0.0f, 2.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

void UMGPostProcessSubsystem::SetContrast(float Contrast)
{
	CurrentSettings.Contrast = FMath::Clamp(Contrast, 0.5f, 1.5f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdatePostProcess();
}

// ==========================================
// SPEED EFFECTS
// ==========================================

void UMGPostProcessSubsystem::UpdateSpeedEffects(float SpeedKPH, float MaxSpeedKPH)
{
	if (MaxSpeedKPH <= 0.0f)
	{
		TargetSpeedLinesIntensity = 0.0f;
		return;
	}

	float SpeedRatio = SpeedKPH / MaxSpeedKPH;

	// Speed lines start at 70% speed
	if (SpeedRatio > 0.7f)
	{
		TargetSpeedLinesIntensity = (SpeedRatio - 0.7f) / 0.3f;
	}
	else
	{
		TargetSpeedLinesIntensity = 0.0f;
	}

	// Dynamic motion blur based on speed
	float DynamicMotionBlur = CurrentSettings.MotionBlurIntensity * FMath::Clamp(SpeedRatio, 0.0f, 1.0f);

	// Dynamic chromatic aberration at high speed
	float DynamicChroma = CurrentSettings.ChromaticAberration;
	if (SpeedRatio > 0.8f)
	{
		DynamicChroma += (SpeedRatio - 0.8f) * 0.5f;
	}

	// Would apply these to post-process settings
}

void UMGPostProcessSubsystem::SetSpeedLinesIntensity(float Intensity)
{
	TargetSpeedLinesIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGPostProcessSubsystem::SetSpeedRadialBlur(float Intensity)
{
	if (SpeedLinesMaterialInstance)
	{
		SpeedLinesMaterialInstance->SetScalarParameterValue(TEXT("RadialBlurIntensity"), Intensity);
	}
}

// ==========================================
// RETRO EFFECTS
// ==========================================

void UMGPostProcessSubsystem::SetRetroEffectsEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		if (CurrentPreset == EMGVisualPreset::Modern)
		{
			SetVisualPreset(EMGVisualPreset::PS2);
		}
	}
	else
	{
		SetVisualPreset(EMGVisualPreset::Modern);
	}
}

void UMGPostProcessSubsystem::SetRetroSettings(const FMGRetroSettings& Settings)
{
	CurrentSettings.RetroSettings = Settings;
	CurrentPreset = EMGVisualPreset::Custom;
	UpdateRetroEffects();
}

void UMGPostProcessSubsystem::SetResolutionScale(float Scale)
{
	CurrentSettings.RetroSettings.ResolutionScale = FMath::Clamp(Scale, 0.25f, 1.0f);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdateRetroEffects();
}

void UMGPostProcessSubsystem::SetColorDepth(int32 Depth)
{
	CurrentSettings.RetroSettings.ColorDepth = FMath::Clamp(Depth, 2, 8);
	CurrentPreset = EMGVisualPreset::Custom;
	UpdateRetroEffects();
}

// ==========================================
// SCREEN EFFECTS
// ==========================================

void UMGPostProcessSubsystem::FlashScreen(FLinearColor Color, float Duration)
{
	FadeColor = Color;
	FadeDuration = Duration;
	FadeElapsed = 0.0f;
	bIsFading = true;
	bFadingIn = false; // Flash out then back
}

void UMGPostProcessSubsystem::FadeToColor(FLinearColor Color, float Duration)
{
	FadeColor = Color;
	FadeDuration = Duration;
	FadeElapsed = 0.0f;
	bIsFading = true;
	bFadingIn = true;
}

void UMGPostProcessSubsystem::FadeFromColor(FLinearColor Color, float Duration)
{
	FadeColor = Color;
	FadeDuration = Duration;
	FadeElapsed = 0.0f;
	bIsFading = true;
	bFadingIn = false;
}

void UMGPostProcessSubsystem::PulseVignette(float Intensity, float Duration)
{
	// Would animate vignette intensity
}

// ==========================================
// INTERNAL
// ==========================================

void UMGPostProcessSubsystem::SetupPostProcess()
{
	// Create retro material instance
	if (RetroPostProcessMaterial)
	{
		RetroMaterialInstance = UMaterialInstanceDynamic::Create(RetroPostProcessMaterial, this);
	}

	// Create speed lines material instance
	if (SpeedLinesMaterial)
	{
		SpeedLinesMaterialInstance = UMaterialInstanceDynamic::Create(SpeedLinesMaterial, this);
	}
}

void UMGPostProcessSubsystem::UpdatePostProcess()
{
	// Would update post-process volume settings
	// This integrates with UE5's post-process system
}

void UMGPostProcessSubsystem::UpdateRetroEffects()
{
	if (!RetroMaterialInstance)
	{
		return;
	}

	const FMGRetroSettings& Retro = CurrentSettings.RetroSettings;

	RetroMaterialInstance->SetScalarParameterValue(TEXT("VertexJitter"), Retro.bEnableVertexJitter ? Retro.VertexJitterIntensity : 0.0f);
	RetroMaterialInstance->SetScalarParameterValue(TEXT("ColorDepth"), Retro.bEnableColorBanding ? Retro.ColorDepth : 8.0f);
	RetroMaterialInstance->SetScalarParameterValue(TEXT("DitherIntensity"), Retro.bEnableDithering ? Retro.DitherIntensity : 0.0f);
	RetroMaterialInstance->SetScalarParameterValue(TEXT("ScanlineIntensity"), Retro.bEnableScanlines ? Retro.ScanlineIntensity : 0.0f);
	RetroMaterialInstance->SetScalarParameterValue(TEXT("CRTCurvature"), Retro.bEnableCRTCurvature ? Retro.CRTCurvature : 0.0f);
}

void UMGPostProcessSubsystem::UpdateSpeedLines(float DeltaTime)
{
	// Smooth interpolation of speed lines
	float InterpSpeed = 5.0f;
	CurrentSpeedLinesIntensity = FMath::FInterpTo(CurrentSpeedLinesIntensity, TargetSpeedLinesIntensity, DeltaTime, InterpSpeed);

	if (SpeedLinesMaterialInstance)
	{
		SpeedLinesMaterialInstance->SetScalarParameterValue(TEXT("Intensity"), CurrentSpeedLinesIntensity);
	}
}

void UMGPostProcessSubsystem::UpdateFade(float DeltaTime)
{
	if (!bIsFading)
	{
		return;
	}

	FadeElapsed += DeltaTime;

	float Alpha = FMath::Clamp(FadeElapsed / FadeDuration, 0.0f, 1.0f);

	if (!bFadingIn)
	{
		Alpha = 1.0f - Alpha;
	}

	// Would apply fade color to screen

	if (FadeElapsed >= FadeDuration)
	{
		bIsFading = false;
	}
}

// ==========================================
// PRESET DEFINITIONS
// ==========================================

FMGPostProcessSettings UMGPostProcessSubsystem::GetPS1Preset() const
{
	FMGPostProcessSettings Settings;

	// Heavy retro effects
	Settings.BloomIntensity = 0.2f;
	Settings.ChromaticAberration = 0.1f;
	Settings.VignetteIntensity = 0.4f;
	Settings.FilmGrainIntensity = 0.3f;
	Settings.MotionBlurIntensity = 0.0f;
	Settings.Saturation = 0.9f;
	Settings.Contrast = 1.1f;

	// PS1-specific
	Settings.RetroSettings.bEnableVertexJitter = true;
	Settings.RetroSettings.VertexJitterIntensity = 0.7f;
	Settings.RetroSettings.bEnableAffineMapping = true;
	Settings.RetroSettings.bEnableResolutionDownscale = true;
	Settings.RetroSettings.ResolutionScale = 0.5f;
	Settings.RetroSettings.bEnableColorBanding = true;
	Settings.RetroSettings.ColorDepth = 5;
	Settings.RetroSettings.bEnableDithering = true;
	Settings.RetroSettings.DitherIntensity = 0.5f;

	return Settings;
}

FMGPostProcessSettings UMGPostProcessSubsystem::GetPS2Preset() const
{
	FMGPostProcessSettings Settings;

	// Moderate retro effects
	Settings.BloomIntensity = 0.4f;
	Settings.ChromaticAberration = 0.05f;
	Settings.VignetteIntensity = 0.3f;
	Settings.FilmGrainIntensity = 0.1f;
	Settings.MotionBlurIntensity = 0.3f;
	Settings.Saturation = 1.0f;
	Settings.Contrast = 1.05f;

	// PS2-specific (lighter than PS1)
	Settings.RetroSettings.bEnableVertexJitter = true;
	Settings.RetroSettings.VertexJitterIntensity = 0.2f;
	Settings.RetroSettings.bEnableResolutionDownscale = true;
	Settings.RetroSettings.ResolutionScale = 0.75f;
	Settings.RetroSettings.bEnableColorBanding = false;
	Settings.RetroSettings.bEnableDithering = false;

	return Settings;
}

FMGPostProcessSettings UMGPostProcessSubsystem::GetModernPreset() const
{
	FMGPostProcessSettings Settings;

	Settings.BloomIntensity = 0.5f;
	Settings.ChromaticAberration = 0.0f;
	Settings.VignetteIntensity = 0.2f;
	Settings.FilmGrainIntensity = 0.0f;
	Settings.MotionBlurIntensity = 0.5f;
	Settings.Saturation = 1.0f;
	Settings.Contrast = 1.0f;

	// No retro effects
	Settings.RetroSettings = FMGRetroSettings();

	return Settings;
}

FMGPostProcessSettings UMGPostProcessSubsystem::GetArcadePreset() const
{
	FMGPostProcessSettings Settings;

	// Vibrant arcade look
	Settings.BloomIntensity = 0.7f;
	Settings.ChromaticAberration = 0.02f;
	Settings.VignetteIntensity = 0.25f;
	Settings.FilmGrainIntensity = 0.0f;
	Settings.MotionBlurIntensity = 0.4f;
	Settings.Saturation = 1.2f;
	Settings.Contrast = 1.1f;

	// Light scanlines for CRT feel
	Settings.RetroSettings.bEnableScanlines = true;
	Settings.RetroSettings.ScanlineIntensity = 0.15f;

	return Settings;
}

FMGPostProcessSettings UMGPostProcessSubsystem::GetNoirPreset() const
{
	FMGPostProcessSettings Settings;

	// Stylized noir look
	Settings.BloomIntensity = 0.3f;
	Settings.ChromaticAberration = 0.0f;
	Settings.VignetteIntensity = 0.5f;
	Settings.FilmGrainIntensity = 0.2f;
	Settings.MotionBlurIntensity = 0.3f;
	Settings.Saturation = 0.3f; // Near grayscale
	Settings.Contrast = 1.3f;

	return Settings;
}
