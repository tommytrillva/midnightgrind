// Copyright Midnight Grind. All Rights Reserved.

// MGPostProcessSubsystem.cpp
// Midnight Grind - Y2K Visual Effects and Post-Processing System

#include "PostProcess/MGPostProcessSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"

void UMGPostProcessSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize default profiles
    InitializeDefaultProfiles();

    // Start with modern style
    SetVisualStyle(EMGVisualStyle::Modern);

    // Start effect update timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGPostProcessSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            EffectUpdateHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->UpdateEffects(1.0f / 60.0f);
                }
            },
            1.0f / 60.0f,
            true
        );
    }
}

void UMGPostProcessSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EffectUpdateHandle);
        World->GetTimerManager().ClearTimer(TemporaryEffectHandle);
    }

    Super::Deinitialize();
}

void UMGPostProcessSubsystem::InitializeDefaultProfiles()
{
    // Create profile for each visual style
    for (int32 i = 0; i <= static_cast<int32>(EMGVisualStyle::Custom); ++i)
    {
        EMGVisualStyle Style = static_cast<EMGVisualStyle>(i);
        FMGPostProcessProfile Profile = CreateProfileForStyle(Style);
        Profile.ProfileName = FName(*UEnum::GetDisplayValueAsText(Style).ToString());
        SavedProfiles.Add(Profile.ProfileName, Profile);
    }
}

FMGPostProcessProfile UMGPostProcessSubsystem::CreateProfileForStyle(EMGVisualStyle Style)
{
    FMGPostProcessProfile Profile;
    Profile.VisualStyle = Style;

    // Default settings (Modern)
    Profile.Bloom.Intensity = 0.5f;
    Profile.Bloom.Threshold = 0.8f;
    Profile.MotionBlur.Amount = 0.5f;
    Profile.Vignette.Intensity = 0.4f;
    Profile.ColorGrading.Mode = EMGColorGradingMode::Neutral;
    Profile.SpeedEffects.bEnabled = true;
    Profile.NitroEffects.bEnabled = true;

    switch (Style)
    {
    case EMGVisualStyle::PS2Authentic:
        Profile.Pixelation.Mode = EMGPixelationMode::PS2;
        Profile.Bloom.Intensity = 0.3f;
        Profile.Bloom.SizeScale = 8.0f;
        Profile.FilmGrain.Intensity = 0.1f;
        Profile.Vignette.Intensity = 0.5f;
        Profile.ColorGrading.Saturation = 1.1f;
        Profile.ColorGrading.Contrast = 1.1f;
        Profile.CRT.bEnabled = false;
        Profile.MotionBlur.Amount = 0.3f;
        break;

    case EMGVisualStyle::PS1Retro:
        Profile.Pixelation.Mode = EMGPixelationMode::PS1;
        Profile.Pixelation.bDithering = true;
        Profile.Pixelation.bColorReduction = true;
        Profile.Pixelation.ColorPalette = 32;
        Profile.Pixelation.bAffineTextureMapping = true;
        Profile.Pixelation.bVertexSnapping = true;
        Profile.Bloom.Intensity = 0.2f;
        Profile.FilmGrain.Intensity = 0.2f;
        Profile.Vignette.Intensity = 0.6f;
        Profile.ColorGrading.Saturation = 0.9f;
        Profile.CRT.bEnabled = true;
        Profile.CRT.Scanlines.bEnabled = true;
        Profile.CRT.Scanlines.Intensity = 0.2f;
        Profile.MotionBlur.Amount = 0.0f;
        break;

    case EMGVisualStyle::Y2KNeon:
        Profile.Bloom.Intensity = 2.0f;
        Profile.Bloom.Threshold = 0.5f;
        Profile.Bloom.Tint = FLinearColor(1.0f, 0.8f, 1.0f, 1.0f);
        Profile.ChromaticAberration.Intensity = 1.5f;
        Profile.Vignette.Intensity = 0.5f;
        Profile.Vignette.Color = FLinearColor(0.1f, 0.0f, 0.2f, 1.0f);
        Profile.ColorGrading.Mode = EMGColorGradingMode::Neon;
        Profile.ColorGrading.Saturation = 1.3f;
        Profile.ColorGrading.Contrast = 1.2f;
        Profile.ColorGrading.HighlightTint = FLinearColor(1.0f, 0.8f, 1.0f, 1.0f);
        Profile.SpeedEffects.SpeedLineIntensity = 0.7f;
        Profile.NitroEffects.ScreenTintColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
        break;

    case EMGVisualStyle::CRTArcade:
        Profile.CRT.bEnabled = true;
        Profile.CRT.Curvature = 0.15f;
        Profile.CRT.CornerRadius = 0.08f;
        Profile.CRT.bShowBezel = true;
        Profile.CRT.PhosphorBleed = 0.1f;
        Profile.CRT.Scanlines.bEnabled = true;
        Profile.CRT.Scanlines.Intensity = 0.4f;
        Profile.CRT.Scanlines.LineCount = 480;
        Profile.Pixelation.Mode = EMGPixelationMode::Subtle;
        Profile.Bloom.Intensity = 0.8f;
        Profile.Vignette.Intensity = 0.6f;
        Profile.ColorGrading.Saturation = 1.2f;
        break;

    case EMGVisualStyle::VHSNostalgia:
        Profile.VHS.bEnabled = true;
        Profile.VHS.Intensity = 0.7f;
        Profile.VHS.ColorBleed = 0.4f;
        Profile.VHS.StaticNoise = 0.15f;
        Profile.VHS.Jitter = 0.15f;
        Profile.VHS.bTrackingLines = true;
        Profile.VHS.bDateTimeStamp = true;
        Profile.CRT.bEnabled = true;
        Profile.CRT.Curvature = 0.1f;
        Profile.CRT.Scanlines.bEnabled = true;
        Profile.CRT.Scanlines.Intensity = 0.3f;
        Profile.FilmGrain.Intensity = 0.3f;
        Profile.ColorGrading.Mode = EMGColorGradingMode::Vintage;
        Profile.ColorGrading.Saturation = 0.85f;
        Profile.Vignette.Intensity = 0.7f;
        break;

    case EMGVisualStyle::Cyberpunk:
        Profile.Bloom.Intensity = 1.5f;
        Profile.Bloom.Tint = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
        Profile.ChromaticAberration.Intensity = 2.0f;
        Profile.Glitch.bEnabled = true;
        Profile.Glitch.Intensity = 0.3f;
        Profile.Glitch.Frequency = 0.5f;
        Profile.ColorGrading.Mode = EMGColorGradingMode::HighContrast;
        Profile.ColorGrading.ShadowTint = FLinearColor(0.0f, 0.05f, 0.1f, 1.0f);
        Profile.ColorGrading.HighlightTint = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
        Profile.Vignette.Color = FLinearColor(0.0f, 0.1f, 0.1f, 1.0f);
        Profile.NitroEffects.ScreenTintColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
        break;

    case EMGVisualStyle::NightCity:
        Profile.Bloom.Intensity = 1.8f;
        Profile.Bloom.Threshold = 0.6f;
        Profile.ColorGrading.Mode = EMGColorGradingMode::Cool;
        Profile.ColorGrading.Temperature = -0.3f;
        Profile.ColorGrading.Contrast = 1.3f;
        Profile.ColorGrading.Saturation = 1.1f;
        Profile.ColorGrading.ShadowTint = FLinearColor(0.0f, 0.0f, 0.1f, 1.0f);
        Profile.Vignette.Intensity = 0.6f;
        Profile.Vignette.Color = FLinearColor(0.0f, 0.0f, 0.1f, 1.0f);
        Profile.ChromaticAberration.Intensity = 0.5f;
        Profile.FilmGrain.Intensity = 0.05f;
        break;

    case EMGVisualStyle::SunsetDrive:
        Profile.Bloom.Intensity = 1.2f;
        Profile.Bloom.Tint = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
        Profile.ColorGrading.Mode = EMGColorGradingMode::Warm;
        Profile.ColorGrading.Temperature = 0.4f;
        Profile.ColorGrading.Saturation = 1.2f;
        Profile.ColorGrading.ShadowTint = FLinearColor(0.1f, 0.05f, 0.15f, 1.0f);
        Profile.ColorGrading.HighlightTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        Profile.Vignette.Intensity = 0.5f;
        Profile.Vignette.Color = FLinearColor(0.2f, 0.05f, 0.1f, 1.0f);
        Profile.FilmGrain.Intensity = 0.08f;
        Profile.NitroEffects.ScreenTintColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
        break;

    case EMGVisualStyle::Modern:
    case EMGVisualStyle::Custom:
    default:
        // Keep default modern settings
        break;
    }

    return Profile;
}

void UMGPostProcessSubsystem::SetVisualStyle(EMGVisualStyle Style)
{
    if (CurrentStyle != Style)
    {
        CurrentStyle = Style;
        CurrentProfile = CreateProfileForStyle(Style);
        ApplyEffectsToPostProcessVolume();
        OnVisualStyleChanged.Broadcast(Style);
        OnProfileChanged.Broadcast(CurrentProfile);
    }
}

void UMGPostProcessSubsystem::ApplyProfile(const FMGPostProcessProfile& Profile)
{
    CurrentProfile = Profile;
    CurrentStyle = Profile.VisualStyle;
    ApplyEffectsToPostProcessVolume();
    OnProfileChanged.Broadcast(CurrentProfile);
}

void UMGPostProcessSubsystem::ApplyProfileByName(const FName& ProfileName)
{
    if (const FMGPostProcessProfile* Profile = SavedProfiles.Find(ProfileName))
    {
        ApplyProfile(*Profile);
    }
}

TArray<FName> UMGPostProcessSubsystem::GetAvailableProfileNames() const
{
    TArray<FName> Names;
    SavedProfiles.GetKeys(Names);
    return Names;
}

void UMGPostProcessSubsystem::SaveProfileAs(const FName& ProfileName)
{
    FMGPostProcessProfile ProfileToSave = CurrentProfile;
    ProfileToSave.ProfileName = ProfileName;
    SavedProfiles.Add(ProfileName, ProfileToSave);
}

void UMGPostProcessSubsystem::DeleteProfile(const FName& ProfileName)
{
    SavedProfiles.Remove(ProfileName);
}

void UMGPostProcessSubsystem::SetBloomSettings(const FMGBloomSettings& Settings)
{
    CurrentProfile.Bloom = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetBloomIntensity(float Intensity)
{
    CurrentProfile.Bloom.Intensity = FMath::Clamp(Intensity, 0.0f, 10.0f);
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetMotionBlurSettings(const FMGMotionBlurSettings& Settings)
{
    CurrentProfile.MotionBlur = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetMotionBlurAmount(float Amount)
{
    CurrentProfile.MotionBlur.Amount = FMath::Clamp(Amount, 0.0f, 2.0f);
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetChromaticAberrationSettings(const FMGChromaticAberrationSettings& Settings)
{
    CurrentProfile.ChromaticAberration = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetChromaticAberrationIntensity(float Intensity)
{
    CurrentProfile.ChromaticAberration.Intensity = FMath::Clamp(Intensity, 0.0f, 10.0f);
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetFilmGrainSettings(const FMGFilmGrainSettings& Settings)
{
    CurrentProfile.FilmGrain = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetFilmGrainIntensity(float Intensity)
{
    CurrentProfile.FilmGrain.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetVignetteSettings(const FMGVignetteSettings& Settings)
{
    CurrentProfile.Vignette = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetVignetteIntensity(float Intensity)
{
    CurrentProfile.Vignette.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetColorGradingSettings(const FMGColorGradingSettings& Settings)
{
    CurrentProfile.ColorGrading = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetColorGradingMode(EMGColorGradingMode Mode)
{
    CurrentProfile.ColorGrading.Mode = Mode;

    // Apply preset values for mode
    switch (Mode)
    {
    case EMGColorGradingMode::Warm:
        CurrentProfile.ColorGrading.Temperature = 0.3f;
        CurrentProfile.ColorGrading.Saturation = 1.1f;
        break;
    case EMGColorGradingMode::Cool:
        CurrentProfile.ColorGrading.Temperature = -0.3f;
        CurrentProfile.ColorGrading.Saturation = 1.0f;
        break;
    case EMGColorGradingMode::Neon:
        CurrentProfile.ColorGrading.Saturation = 1.4f;
        CurrentProfile.ColorGrading.Contrast = 1.2f;
        break;
    case EMGColorGradingMode::Desaturated:
        CurrentProfile.ColorGrading.Saturation = 0.5f;
        break;
    case EMGColorGradingMode::HighContrast:
        CurrentProfile.ColorGrading.Contrast = 1.5f;
        break;
    case EMGColorGradingMode::Vintage:
        CurrentProfile.ColorGrading.Saturation = 0.8f;
        CurrentProfile.ColorGrading.Temperature = 0.1f;
        CurrentProfile.ColorGrading.ShadowTint = FLinearColor(0.1f, 0.05f, 0.0f, 1.0f);
        break;
    case EMGColorGradingMode::Cinema:
        CurrentProfile.ColorGrading.Contrast = 1.2f;
        CurrentProfile.ColorGrading.Saturation = 0.95f;
        CurrentProfile.ColorGrading.ShadowTint = FLinearColor(0.0f, 0.0f, 0.05f, 1.0f);
        break;
    default:
        CurrentProfile.ColorGrading.Temperature = 0.0f;
        CurrentProfile.ColorGrading.Saturation = 1.0f;
        CurrentProfile.ColorGrading.Contrast = 1.0f;
        break;
    }

    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetCRTSettings(const FMGCRTSettings& Settings)
{
    CurrentProfile.CRT = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::EnableCRTEffect(bool bEnabled)
{
    CurrentProfile.CRT.bEnabled = bEnabled;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetVHSSettings(const FMGVHSSettings& Settings)
{
    CurrentProfile.VHS = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::EnableVHSEffect(bool bEnabled)
{
    CurrentProfile.VHS.bEnabled = bEnabled;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetGlitchSettings(const FMGGlitchSettings& Settings)
{
    CurrentProfile.Glitch = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::TriggerGlitch(float Duration, float Intensity)
{
    GlitchTimeRemaining = Duration;
    CurrentProfile.Glitch.bEnabled = true;
    CurrentProfile.Glitch.Intensity = Intensity;
    ApplyEffectsToPostProcessVolume();
    OnEffectTriggered.Broadcast(EMGScreenEffect::GlitchEffect, Intensity);
}

void UMGPostProcessSubsystem::SetPixelationSettings(const FMGPixelationSettings& Settings)
{
    CurrentProfile.Pixelation = Settings;
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetPixelationMode(EMGPixelationMode Mode)
{
    CurrentProfile.Pixelation.Mode = Mode;

    switch (Mode)
    {
    case EMGPixelationMode::Subtle:
        CurrentProfile.Pixelation.CustomWidth = 720;
        CurrentProfile.Pixelation.CustomHeight = 405;
        break;
    case EMGPixelationMode::PS2:
        CurrentProfile.Pixelation.CustomWidth = 640;
        CurrentProfile.Pixelation.CustomHeight = 480;
        break;
    case EMGPixelationMode::PS1:
        CurrentProfile.Pixelation.CustomWidth = 320;
        CurrentProfile.Pixelation.CustomHeight = 240;
        break;
    case EMGPixelationMode::Extreme:
        CurrentProfile.Pixelation.CustomWidth = 160;
        CurrentProfile.Pixelation.CustomHeight = 120;
        break;
    default:
        break;
    }

    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetSpeedEffectSettings(const FMGSpeedEffectSettings& Settings)
{
    CurrentProfile.SpeedEffects = Settings;
}

void UMGPostProcessSubsystem::UpdateSpeedEffect(float CurrentSpeed)
{
    if (!CurrentProfile.SpeedEffects.bEnabled)
    {
        CurrentSpeedEffectIntensity = 0.0f;
        return;
    }

    float SpeedRange = CurrentProfile.SpeedEffects.MaxSpeedForEffect - CurrentProfile.SpeedEffects.SpeedThreshold;
    if (SpeedRange <= 0.0f)
    {
        return;
    }

    float NewIntensity = 0.0f;
    if (CurrentSpeed > CurrentProfile.SpeedEffects.SpeedThreshold)
    {
        float NormalizedSpeed = (CurrentSpeed - CurrentProfile.SpeedEffects.SpeedThreshold) / SpeedRange;
        NewIntensity = FMath::Clamp(NormalizedSpeed, 0.0f, 1.0f);
    }

    // Smooth transition
    float RampSpeed = 1.0f / FMath::Max(0.01f, CurrentProfile.SpeedEffects.EffectRampUpTime);
    if (NewIntensity > CurrentSpeedEffectIntensity)
    {
        CurrentSpeedEffectIntensity = FMath::FInterpTo(CurrentSpeedEffectIntensity, NewIntensity, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f, RampSpeed);
    }
    else
    {
        CurrentSpeedEffectIntensity = FMath::FInterpTo(CurrentSpeedEffectIntensity, NewIntensity, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f, RampSpeed * 2.0f);
    }

    OnSpeedEffectIntensityChanged.Broadcast(CurrentSpeedEffectIntensity);
}

void UMGPostProcessSubsystem::SetNitroEffectSettings(const FMGNitroEffectSettings& Settings)
{
    CurrentProfile.NitroEffects = Settings;
}

void UMGPostProcessSubsystem::ActivateNitroEffect(bool bActive)
{
    bNitroEffectActive = bActive;
    OnEffectTriggered.Broadcast(EMGScreenEffect::NitroBlur, bActive ? 1.0f : 0.0f);
}

void UMGPostProcessSubsystem::PulseNitroEffect()
{
    if (bNitroEffectActive && CurrentProfile.NitroEffects.bEnabled)
    {
        // Trigger pulse effect
        float PulseIntensity = 0.5f + 0.5f * FMath::Sin(GetWorld() ? GetWorld()->GetTimeSeconds() * CurrentProfile.NitroEffects.PulseFrequency * 2.0f * PI : 0.0f);

        if (CurrentProfile.NitroEffects.bRadialBlurPulse)
        {
            RadialBlurPulse(PulseIntensity * 0.3f, 0.1f);
        }
    }
}

void UMGPostProcessSubsystem::TriggerDriftEffect(float DriftAngle)
{
    float NormalizedAngle = FMath::Clamp(FMath::Abs(DriftAngle) / 45.0f, 0.0f, 1.0f);
    OnEffectTriggered.Broadcast(EMGScreenEffect::DriftSmoke, NormalizedAngle);
}

void UMGPostProcessSubsystem::TriggerCollisionEffect(float ImpactForce)
{
    float NormalizedForce = FMath::Clamp(ImpactForce / 1000.0f, 0.0f, 1.0f);

    // Flash screen red briefly
    FlashScreen(FLinearColor(1.0f, 0.2f, 0.1f, 0.5f * NormalizedForce), 0.1f);

    // Trigger glitch based on impact
    if (NormalizedForce > 0.3f)
    {
        TriggerGlitch(NormalizedForce * 0.3f, NormalizedForce);
    }

    OnEffectTriggered.Broadcast(EMGScreenEffect::ImpactFlash, NormalizedForce);
}

void UMGPostProcessSubsystem::TriggerBoostEffect(float Duration)
{
    RadialBlurPulse(0.5f, Duration * 0.5f);
    OnEffectTriggered.Broadcast(EMGScreenEffect::RadialBlur, 0.5f);
}

void UMGPostProcessSubsystem::FlashScreen(FLinearColor Color, float Duration)
{
    FlashColor = Color;
    FlashTimeRemaining = Duration;
}

void UMGPostProcessSubsystem::ShakeScreen(float Intensity, float Duration)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Store shake parameters for tick-based effect
    ShakeIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    ShakeTimeRemaining = Duration;
    bShakeActive = true;

    // Also apply visual effects during shake
    // Add chromatic aberration for impact feel
    float CAIntensity = Intensity * 0.5f;
    CurrentProfile.ChromaticAberration.Intensity = FMath::Min(1.0f, CAIntensity);

    // Boost vignette slightly
    float VignetteBoost = Intensity * 0.2f;
    CurrentProfile.Vignette.Intensity = FMath::Min(1.0f, CurrentProfile.Vignette.Intensity + VignetteBoost);

    ApplyEffectsToPostProcessVolume();

    // Set timer to restore effects
    TWeakObjectPtr<UMGPostProcessSubsystem> WeakThis(this);
    World->GetTimerManager().SetTimer(
        ShakeTimerHandle,
        [WeakThis]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->bShakeActive = false;
                WeakThis->ShakeIntensity = 0.0f;

                // Restore chromatic aberration
                WeakThis->CurrentProfile.ChromaticAberration.Intensity = 0.0f;

                // Restore vignette to normal
                WeakThis->ApplyEffectsToPostProcessVolume();
            }
        },
        Duration,
        false
    );

    // Also request actual camera shake through player controller
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && PC->PlayerCameraManager)
    {
        // Use built-in camera shake with oscillation pattern
        PC->PlayerCameraManager->StartCameraShake(
            nullptr, // No specific shake class - using parameters
            Intensity,
            ECameraShakePlaySpace::CameraLocal,
            FRotator::ZeroRotator
        );
    }

    OnEffectTriggered.Broadcast(EMGScreenEffect::CameraShake, Intensity);
}

void UMGPostProcessSubsystem::PulseVignette(float Intensity, float Duration)
{
    // Temporarily boost vignette
    float OriginalIntensity = CurrentProfile.Vignette.Intensity;

    if (UWorld* World = GetWorld())
    {
        CurrentProfile.Vignette.Intensity = FMath::Min(1.0f, OriginalIntensity + Intensity);
        ApplyEffectsToPostProcessVolume();

        TWeakObjectPtr<UMGPostProcessSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            TemporaryEffectHandle,
            [WeakThis, OriginalIntensity]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->CurrentProfile.Vignette.Intensity = OriginalIntensity;
                    WeakThis->ApplyEffectsToPostProcessVolume();
                }
            },
            Duration,
            false
        );
    }
}

void UMGPostProcessSubsystem::RadialBlurPulse(float Intensity, float Duration)
{
    OnEffectTriggered.Broadcast(EMGScreenEffect::RadialBlur, Intensity);
}

void UMGPostProcessSubsystem::SetTimeOfDay(EMGTimeOfDay Time)
{
    CurrentTimeOfDay = Time;
    CurrentProfile.ColorGrading = GetColorGradingForTimeOfDay(Time);
    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::SetTimeOfDayBlend(EMGTimeOfDay FromTime, EMGTimeOfDay ToTime, float Alpha)
{
    FMGColorGradingSettings FromSettings = GetColorGradingForTimeOfDay(FromTime);
    FMGColorGradingSettings ToSettings = GetColorGradingForTimeOfDay(ToTime);

    // Blend settings
    CurrentProfile.ColorGrading.Temperature = FMath::Lerp(FromSettings.Temperature, ToSettings.Temperature, Alpha);
    CurrentProfile.ColorGrading.Tint = FMath::Lerp(FromSettings.Tint, ToSettings.Tint, Alpha);
    CurrentProfile.ColorGrading.Saturation = FMath::Lerp(FromSettings.Saturation, ToSettings.Saturation, Alpha);
    CurrentProfile.ColorGrading.Contrast = FMath::Lerp(FromSettings.Contrast, ToSettings.Contrast, Alpha);
    CurrentProfile.ColorGrading.Gamma = FMath::Lerp(FromSettings.Gamma, ToSettings.Gamma, Alpha);
    CurrentProfile.ColorGrading.Gain = FMath::Lerp(FromSettings.Gain, ToSettings.Gain, Alpha);
    CurrentProfile.ColorGrading.ShadowTint = FLinearColor::LerpUsingHSV(FromSettings.ShadowTint, ToSettings.ShadowTint, Alpha);
    CurrentProfile.ColorGrading.MidtoneTint = FLinearColor::LerpUsingHSV(FromSettings.MidtoneTint, ToSettings.MidtoneTint, Alpha);
    CurrentProfile.ColorGrading.HighlightTint = FLinearColor::LerpUsingHSV(FromSettings.HighlightTint, ToSettings.HighlightTint, Alpha);

    ApplyEffectsToPostProcessVolume();
}

FMGColorGradingSettings UMGPostProcessSubsystem::GetColorGradingForTimeOfDay(EMGTimeOfDay Time)
{
    FMGColorGradingSettings Settings;
    Settings.Mode = EMGColorGradingMode::Custom;

    switch (Time)
    {
    case EMGTimeOfDay::Dawn:
        Settings.Temperature = 0.2f;
        Settings.Saturation = 0.9f;
        Settings.Gamma = 1.1f;
        Settings.ShadowTint = FLinearColor(0.1f, 0.05f, 0.15f, 1.0f);
        Settings.HighlightTint = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
        break;

    case EMGTimeOfDay::Morning:
        Settings.Temperature = 0.1f;
        Settings.Saturation = 1.0f;
        Settings.Gamma = 1.0f;
        Settings.HighlightTint = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
        break;

    case EMGTimeOfDay::Noon:
        Settings.Temperature = 0.0f;
        Settings.Saturation = 1.0f;
        Settings.Contrast = 1.1f;
        Settings.Gain = 1.05f;
        break;

    case EMGTimeOfDay::Afternoon:
        Settings.Temperature = 0.15f;
        Settings.Saturation = 1.05f;
        Settings.HighlightTint = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
        break;

    case EMGTimeOfDay::Sunset:
        Settings.Temperature = 0.5f;
        Settings.Saturation = 1.2f;
        Settings.Contrast = 1.15f;
        Settings.ShadowTint = FLinearColor(0.15f, 0.05f, 0.2f, 1.0f);
        Settings.HighlightTint = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
        break;

    case EMGTimeOfDay::Dusk:
        Settings.Temperature = 0.1f;
        Settings.Saturation = 0.9f;
        Settings.Gamma = 1.1f;
        Settings.ShadowTint = FLinearColor(0.1f, 0.0f, 0.2f, 1.0f);
        Settings.HighlightTint = FLinearColor(0.9f, 0.7f, 0.8f, 1.0f);
        break;

    case EMGTimeOfDay::Night:
        Settings.Temperature = -0.2f;
        Settings.Saturation = 0.8f;
        Settings.Contrast = 1.2f;
        Settings.Gamma = 1.2f;
        Settings.Gain = 0.9f;
        Settings.ShadowTint = FLinearColor(0.0f, 0.0f, 0.1f, 1.0f);
        Settings.HighlightTint = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
        break;

    case EMGTimeOfDay::Midnight:
        Settings.Temperature = -0.3f;
        Settings.Saturation = 0.7f;
        Settings.Contrast = 1.3f;
        Settings.Gamma = 1.3f;
        Settings.Gain = 0.85f;
        Settings.ShadowTint = FLinearColor(0.0f, 0.0f, 0.15f, 1.0f);
        Settings.HighlightTint = FLinearColor(0.7f, 0.75f, 1.0f, 1.0f);
        break;

    default:
        break;
    }

    return Settings;
}

void UMGPostProcessSubsystem::UpdateEffects(float MGDeltaTime)
{
    UpdateTemporaryEffects(DeltaTime);

    if (bNitroEffectActive)
    {
        PulseNitroEffect();
    }
}

void UMGPostProcessSubsystem::UpdateTemporaryEffects(float MGDeltaTime)
{
    // Update glitch timer
    if (GlitchTimeRemaining > 0.0f)
    {
        GlitchTimeRemaining -= DeltaTime;
        if (GlitchTimeRemaining <= 0.0f)
        {
            CurrentProfile.Glitch.bEnabled = false;
            ApplyEffectsToPostProcessVolume();
        }
    }

    // Update flash timer
    if (FlashTimeRemaining > 0.0f)
    {
        FlashTimeRemaining -= DeltaTime;
        // Flash effect would be applied to overlay
    }
}

void UMGPostProcessSubsystem::ResetToDefaults()
{
    SetVisualStyle(EMGVisualStyle::Modern);
}

void UMGPostProcessSubsystem::DisableAllEffects()
{
    CurrentProfile.Bloom.Intensity = 0.0f;
    CurrentProfile.MotionBlur.Amount = 0.0f;
    CurrentProfile.ChromaticAberration.Intensity = 0.0f;
    CurrentProfile.FilmGrain.Intensity = 0.0f;
    CurrentProfile.Vignette.Intensity = 0.0f;
    CurrentProfile.CRT.bEnabled = false;
    CurrentProfile.VHS.bEnabled = false;
    CurrentProfile.Glitch.bEnabled = false;
    CurrentProfile.Pixelation.Mode = EMGPixelationMode::None;
    CurrentProfile.SpeedEffects.bEnabled = false;
    CurrentProfile.NitroEffects.bEnabled = false;

    ApplyEffectsToPostProcessVolume();
}

void UMGPostProcessSubsystem::ApplyEffectsToPostProcessVolume()
{
    // In a real implementation, this would find the post-process volume
    // and apply the settings to it. For now, we just track the state.

    // Effects would be applied to:
    // - UPostProcessComponent
    // - Post-process materials
    // - Camera settings
}

void UMGPostProcessSubsystem::ProcessSpeedEffects(float Speed, float MGDeltaTime)
{
    UpdateSpeedEffect(Speed);
}
