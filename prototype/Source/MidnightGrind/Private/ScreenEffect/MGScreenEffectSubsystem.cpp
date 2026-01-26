// Copyright Midnight Grind. All Rights Reserved.

#include "ScreenEffect/MGScreenEffectSubsystem.h"
#include "Kismet/GameplayStatics.h"

UMGScreenEffectSubsystem::UMGScreenEffectSubsystem()
    : CurrentSpeedEffectIntensity(0.0f)
    , CurrentDamageEffectIntensity(0.0f)
    , CurrentBoostEffectIntensity(0.0f)
    , bBoostActive(false)
    , bShakeActive(false)
    , ShakeIntensity(0.0f)
    , ShakeDuration(0.0f)
    , ShakeElapsedTime(0.0f)
    , ShakeFrequency(20.0f)
    , ShakeType(EMGScreenShakeType::None)
    , CurrentShakeOffset(FVector2D::ZeroVector)
    , bTransitioning(false)
    , bFadingIn(false)
    , TransitionProgress(0.0f)
    , TransitionDuration(0.5f)
    , TransitionColor(FLinearColor::Black)
    , OutputRadialBlur(0.0f)
    , OutputChromatic(0.0f)
    , OutputVignette(0.0f)
    , OutputVignetteColor(FLinearColor::Black)
    , OutputDesaturation(0.0f)
    , OutputBloom(1.0f)
    , OutputTint(FLinearColor::White)
    , OutputGlitch(0.0f)
    , OutputScanlines(0.0f)
    , GlobalEffectScale(1.0f)
    , bEffectsEnabled(true)
{
}

void UMGScreenEffectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeDefaultParams();
    InitializePresets();

    // Enable all categories by default
    CategoryEnabled.Add(EMGScreenEffectCategory::Speed, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Damage, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Boost, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Impact, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Environment, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Transition, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Feedback, true);
    CategoryEnabled.Add(EMGScreenEffectCategory::Y2K, true);

    UE_LOG(LogTemp, Log, TEXT("ScreenEffect: Subsystem initialized"));
}

void UMGScreenEffectSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UMGScreenEffectSubsystem::InitializeDefaultParams()
{
    SpeedParams = FMGSpeedEffectParams();
    DamageParams = FMGDamageEffectParams();
    BoostParams = FMGBoostEffectParams();
    Y2KParams = FMGY2KEffectParams();
    TransitionParams = FMGTransitionParams();

    // Initialize impact params for each type
    FMGImpactEffectParams LightBump;
    LightBump.ShakeIntensity = 0.3f;
    LightBump.ShakeDuration = 0.15f;
    LightBump.ChromaticPunch = 0.01f;
    LightBump.FlashIntensity = 0.2f;
    ImpactParamsMap.Add(EMGImpactEffectType::LightBump, LightBump);

    FMGImpactEffectParams MediumCollision;
    MediumCollision.ShakeIntensity = 0.6f;
    MediumCollision.ShakeDuration = 0.25f;
    MediumCollision.ChromaticPunch = 0.02f;
    MediumCollision.FlashIntensity = 0.4f;
    MediumCollision.DistortionAmount = 0.015f;
    ImpactParamsMap.Add(EMGImpactEffectType::MediumCollision, MediumCollision);

    FMGImpactEffectParams HeavyCrash;
    HeavyCrash.ShakeIntensity = 1.0f;
    HeavyCrash.ShakeDuration = 0.4f;
    HeavyCrash.ChromaticPunch = 0.04f;
    HeavyCrash.FlashIntensity = 0.7f;
    HeavyCrash.FlashDuration = 0.15f;
    HeavyCrash.DistortionAmount = 0.03f;
    HeavyCrash.ZoomPunch = 0.04f;
    HeavyCrash.FlashColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
    ImpactParamsMap.Add(EMGImpactEffectType::HeavyCrash, HeavyCrash);

    FMGImpactEffectParams WallScrape;
    WallScrape.ShakeIntensity = 0.2f;
    WallScrape.ShakeDuration = 0.1f;
    WallScrape.ShakeType = EMGScreenShakeType::Directional;
    WallScrape.ChromaticPunch = 0.005f;
    WallScrape.FlashIntensity = 0.1f;
    ImpactParamsMap.Add(EMGImpactEffectType::WallScrape, WallScrape);

    FMGImpactEffectParams VehicleContact;
    VehicleContact.ShakeIntensity = 0.4f;
    VehicleContact.ShakeDuration = 0.2f;
    VehicleContact.ChromaticPunch = 0.015f;
    VehicleContact.FlashIntensity = 0.3f;
    ImpactParamsMap.Add(EMGImpactEffectType::VehicleContact, VehicleContact);

    FMGImpactEffectParams Landing;
    Landing.ShakeIntensity = 0.5f;
    Landing.ShakeDuration = 0.3f;
    Landing.ShakeType = EMGScreenShakeType::Subtle;
    Landing.ChromaticPunch = 0.01f;
    Landing.ZoomPunch = 0.02f;
    Landing.bEnableFlash = false;
    ImpactParamsMap.Add(EMGImpactEffectType::Landing, Landing);
}

void UMGScreenEffectSubsystem::InitializePresets()
{
    // Default preset
    FMGEffectPreset Default;
    Default.PresetName = TEXT("Default");
    EffectPresets.Add(TEXT("Default"), Default);

    // Arcade preset - more intense effects
    FMGEffectPreset Arcade;
    Arcade.PresetName = TEXT("Arcade");
    Arcade.SpeedParams.LineOpacity = 0.6f;
    Arcade.SpeedParams.RadialBlurAmount = 0.03f;
    Arcade.BoostParams.ChromaticStrength = 0.03f;
    Arcade.BoostParams.BloomIntensity = 2.0f;
    Arcade.Y2KParams.GlitchIntensity = 0.4f;
    Arcade.Y2KParams.ScanlineIntensity = 0.3f;
    EffectPresets.Add(TEXT("Arcade"), Arcade);

    // Minimal preset - reduced effects
    FMGEffectPreset Minimal;
    Minimal.PresetName = TEXT("Minimal");
    Minimal.SpeedParams.bEnableSpeedLines = false;
    Minimal.SpeedParams.RadialBlurAmount = 0.01f;
    Minimal.DamageParams.RedTintIntensity = 0.2f;
    Minimal.bEnableY2K = false;
    EffectPresets.Add(TEXT("Minimal"), Minimal);

    // Y2K Heavy preset - maximum retro aesthetics
    FMGEffectPreset Y2KHeavy;
    Y2KHeavy.PresetName = TEXT("Y2K Heavy");
    Y2KHeavy.Y2KParams.GlitchIntensity = 0.5f;
    Y2KHeavy.Y2KParams.ScanlineIntensity = 0.4f;
    Y2KHeavy.Y2KParams.RGBSplitAmount = 0.01f;
    Y2KHeavy.Y2KParams.NoiseAmount = 0.1f;
    Y2KHeavy.SpeedParams.Style = EMGSpeedLineStyle::Neon;
    Y2KHeavy.BoostParams.Style = EMGBoostEffectStyle::NeonGlow;
    EffectPresets.Add(TEXT("Y2K Heavy"), Y2KHeavy);

    // Performance preset - minimal GPU impact
    FMGEffectPreset Performance;
    Performance.PresetName = TEXT("Performance");
    Performance.SpeedParams.bEnableRadialBlur = false;
    Performance.SpeedParams.bEnableMotionBlur = false;
    Performance.BoostParams.bEnableChromatic = false;
    Performance.DamageParams.bEnableChromatic = false;
    Performance.bEnableY2K = false;
    EffectPresets.Add(TEXT("Performance"), Performance);

    CurrentPresetName = TEXT("Default");
}

void UMGScreenEffectSubsystem::UpdateEffects(float DeltaTime)
{
    if (!bEffectsEnabled)
    {
        return;
    }

    UpdateActiveEffects(DeltaTime);
    UpdateShake(DeltaTime);
    UpdateTransition(DeltaTime);
    CleanupExpiredEffects();
    CalculateOutputValues();
}

void UMGScreenEffectSubsystem::UpdateSpeedEffect(float CurrentSpeed)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Speed))
    {
        CurrentSpeedEffectIntensity = 0.0f;
        return;
    }

    // Calculate intensity based on speed thresholds
    if (CurrentSpeed <= SpeedParams.MinSpeedThreshold)
    {
        CurrentSpeedEffectIntensity = 0.0f;
    }
    else if (CurrentSpeed >= SpeedParams.MaxSpeedThreshold)
    {
        CurrentSpeedEffectIntensity = 1.0f;
    }
    else
    {
        float Range = SpeedParams.MaxSpeedThreshold - SpeedParams.MinSpeedThreshold;
        CurrentSpeedEffectIntensity = (CurrentSpeed - SpeedParams.MinSpeedThreshold) / Range;
    }

    CurrentSpeedEffectIntensity *= GlobalEffectScale;
}

void UMGScreenEffectSubsystem::SetSpeedEffectParams(const FMGSpeedEffectParams& Params)
{
    SpeedParams = Params;
}

FMGSpeedEffectParams UMGScreenEffectSubsystem::GetSpeedEffectParams() const
{
    return SpeedParams;
}

float UMGScreenEffectSubsystem::GetCurrentSpeedEffectIntensity() const
{
    return CurrentSpeedEffectIntensity;
}

void UMGScreenEffectSubsystem::UpdateDamageEffect(float HealthPercent)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Damage))
    {
        CurrentDamageEffectIntensity = 0.0f;
        return;
    }

    // Calculate intensity based on health
    if (HealthPercent >= DamageParams.HealthThreshold)
    {
        CurrentDamageEffectIntensity = 0.0f;
    }
    else
    {
        CurrentDamageEffectIntensity = 1.0f - (HealthPercent / DamageParams.HealthThreshold);
    }

    // Apply pulse if enabled
    if (DamageParams.bEnablePulse && CurrentDamageEffectIntensity > 0.0f)
    {
        float PulseTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        float Pulse = (FMath::Sin(PulseTime * DamageParams.PulseRate * 2.0f * PI) + 1.0f) * 0.5f;
        CurrentDamageEffectIntensity *= FMath::Lerp(0.7f, 1.0f, Pulse);
    }

    CurrentDamageEffectIntensity *= GlobalEffectScale;
}

void UMGScreenEffectSubsystem::SetDamageEffectParams(const FMGDamageEffectParams& Params)
{
    DamageParams = Params;
}

FMGDamageEffectParams UMGScreenEffectSubsystem::GetDamageEffectParams() const
{
    return DamageParams;
}

void UMGScreenEffectSubsystem::TriggerDamageFlash(float Intensity)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Damage))
    {
        return;
    }

    // Create temporary flash effect
    PlayEffect(TEXT("DamageFlash"), 0.2f, Intensity * GlobalEffectScale, EMGScreenEffectCategory::Damage);
}

void UMGScreenEffectSubsystem::StartBoostEffect(float Intensity)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Boost))
    {
        return;
    }

    bBoostActive = true;
    CurrentBoostEffectIntensity = Intensity * GlobalEffectScale;
}

void UMGScreenEffectSubsystem::StopBoostEffect(float FadeOutTime)
{
    bBoostActive = false;
    // Fade out handled in update
}

void UMGScreenEffectSubsystem::UpdateBoostEffect(float BoostAmount)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Boost) || !bBoostActive)
    {
        CurrentBoostEffectIntensity = FMath::FInterpTo(CurrentBoostEffectIntensity, 0.0f,
                                                        GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f, 5.0f);
        return;
    }

    CurrentBoostEffectIntensity = BoostAmount * BoostParams.IntensityScale * GlobalEffectScale;
}

void UMGScreenEffectSubsystem::SetBoostEffectParams(const FMGBoostEffectParams& Params)
{
    BoostParams = Params;
}

FMGBoostEffectParams UMGScreenEffectSubsystem::GetBoostEffectParams() const
{
    return BoostParams;
}

bool UMGScreenEffectSubsystem::IsBoostEffectActive() const
{
    return bBoostActive && CurrentBoostEffectIntensity > 0.01f;
}

void UMGScreenEffectSubsystem::TriggerImpact(EMGImpactEffectType Type, float Intensity, FVector2D Direction)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Impact))
    {
        return;
    }

    const FMGImpactEffectParams* Params = ImpactParamsMap.Find(Type);
    if (!Params)
    {
        Params = ImpactParamsMap.Find(EMGImpactEffectType::MediumCollision);
        if (!Params)
        {
            return;
        }
    }

    Intensity *= GlobalEffectScale;

    // Start screen shake
    if (Params->bEnableShake)
    {
        StartScreenShake(Params->ShakeIntensity * Intensity, Params->ShakeDuration, Params->ShakeType);
    }

    // Trigger flash effect
    if (Params->bEnableFlash)
    {
        FMGActiveEffect Flash;
        Flash.EffectId = FGuid::NewGuid();
        Flash.EffectName = TEXT("ImpactFlash");
        Flash.Category = EMGScreenEffectCategory::Impact;
        Flash.Intensity = Params->FlashIntensity * Intensity;
        Flash.Duration = Params->FlashDuration;
        Flash.TintColor = Params->FlashColor;
        Flash.IntensityCurve = EMGIntensityCurve::EaseOut;
        Flash.Priority = 10;

        ActiveEffects.Add(Flash.EffectId, Flash);
        OnEffectStarted.Broadcast(Flash.EffectId);
    }

    // Chromatic punch
    if (Params->ChromaticPunch > 0.0f)
    {
        FMGActiveEffect Chromatic;
        Chromatic.EffectId = FGuid::NewGuid();
        Chromatic.EffectName = TEXT("ChromaticPunch");
        Chromatic.Category = EMGScreenEffectCategory::Impact;
        Chromatic.Intensity = Params->ChromaticPunch * Intensity;
        Chromatic.Duration = Params->ShakeDuration;
        Chromatic.IntensityCurve = EMGIntensityCurve::EaseOut;

        ActiveEffects.Add(Chromatic.EffectId, Chromatic);
    }
}

void UMGScreenEffectSubsystem::SetImpactEffectParams(EMGImpactEffectType Type, const FMGImpactEffectParams& Params)
{
    ImpactParamsMap.Add(Type, Params);
}

FMGImpactEffectParams UMGScreenEffectSubsystem::GetImpactEffectParams(EMGImpactEffectType Type) const
{
    if (const FMGImpactEffectParams* Params = ImpactParamsMap.Find(Type))
    {
        return *Params;
    }
    return FMGImpactEffectParams();
}

void UMGScreenEffectSubsystem::StartScreenShake(float Intensity, float Duration, EMGScreenShakeType Type)
{
    bShakeActive = true;
    ShakeIntensity = Intensity * GlobalEffectScale;
    ShakeDuration = Duration;
    ShakeElapsedTime = 0.0f;
    ShakeType = Type;

    switch (Type)
    {
        case EMGScreenShakeType::Subtle:
            ShakeFrequency = 15.0f;
            break;
        case EMGScreenShakeType::Normal:
            ShakeFrequency = 20.0f;
            break;
        case EMGScreenShakeType::Intense:
            ShakeFrequency = 30.0f;
            break;
        case EMGScreenShakeType::Rotational:
            ShakeFrequency = 10.0f;
            break;
        default:
            ShakeFrequency = 20.0f;
            break;
    }
}

void UMGScreenEffectSubsystem::StopScreenShake()
{
    bShakeActive = false;
    CurrentShakeOffset = FVector2D::ZeroVector;
}

FVector2D UMGScreenEffectSubsystem::GetCurrentShakeOffset() const
{
    return CurrentShakeOffset;
}

bool UMGScreenEffectSubsystem::IsScreenShaking() const
{
    return bShakeActive && ShakeElapsedTime < ShakeDuration;
}

FGuid UMGScreenEffectSubsystem::StartY2KEffect(EMGY2KEffectType Type, float Duration, float Intensity)
{
    if (!IsCategoryEnabled(EMGScreenEffectCategory::Y2K))
    {
        return FGuid();
    }

    FMGActiveEffect Effect;
    Effect.EffectId = FGuid::NewGuid();
    Effect.EffectName = UEnum::GetDisplayValueAsText(Type).ToString();
    Effect.Category = EMGScreenEffectCategory::Y2K;
    Effect.Intensity = Intensity * GlobalEffectScale;
    Effect.Duration = Duration;
    Effect.FadeInTime = 0.1f;
    Effect.FadeOutTime = 0.2f;
    Effect.IntensityCurve = EMGIntensityCurve::EaseInOut;

    // Special handling for certain Y2K effects
    switch (Type)
    {
        case EMGY2KEffectType::PixelBurst:
            Effect.IntensityCurve = EMGIntensityCurve::Bounce;
            break;
        case EMGY2KEffectType::VHSGlitch:
            Effect.IntensityCurve = EMGIntensityCurve::Flicker;
            break;
        case EMGY2KEffectType::HologramFlicker:
            Effect.bLooping = true;
            break;
        default:
            break;
    }

    ActiveEffects.Add(Effect.EffectId, Effect);
    OnEffectStarted.Broadcast(Effect.EffectId);

    return Effect.EffectId;
}

void UMGScreenEffectSubsystem::StopY2KEffect(const FGuid& EffectId)
{
    StopEffect(EffectId, 0.2f);
}

void UMGScreenEffectSubsystem::StopAllY2KEffects()
{
    TArray<FGuid> ToRemove;

    for (const auto& Pair : ActiveEffects)
    {
        if (Pair.Value.Category == EMGScreenEffectCategory::Y2K)
        {
            ToRemove.Add(Pair.Key);
        }
    }

    for (const FGuid& Id : ToRemove)
    {
        StopEffect(Id, 0.1f);
    }
}

void UMGScreenEffectSubsystem::SetY2KEffectParams(const FMGY2KEffectParams& Params)
{
    Y2KParams = Params;
}

FMGY2KEffectParams UMGScreenEffectSubsystem::GetY2KEffectParams() const
{
    return Y2KParams;
}

void UMGScreenEffectSubsystem::FadeToColor(const FLinearColor& Color, float Duration)
{
    bTransitioning = true;
    bFadingIn = false;
    TransitionProgress = 0.0f;
    TransitionDuration = Duration;
    TransitionColor = Color;

    OnTransitionStarted.Broadcast(false);
}

void UMGScreenEffectSubsystem::FadeFromColor(const FLinearColor& Color, float Duration)
{
    bTransitioning = true;
    bFadingIn = true;
    TransitionProgress = 1.0f;
    TransitionDuration = Duration;
    TransitionColor = Color;

    OnTransitionStarted.Broadcast(true);
}

void UMGScreenEffectSubsystem::FadeOut(float Duration)
{
    FadeToColor(FLinearColor::Black, Duration);
}

void UMGScreenEffectSubsystem::FadeIn(float Duration)
{
    FadeFromColor(FLinearColor::Black, Duration);
}

void UMGScreenEffectSubsystem::SetTransitionParams(const FMGTransitionParams& Params)
{
    TransitionParams = Params;
}

bool UMGScreenEffectSubsystem::IsTransitioning() const
{
    return bTransitioning;
}

float UMGScreenEffectSubsystem::GetTransitionProgress() const
{
    return TransitionProgress;
}

FGuid UMGScreenEffectSubsystem::PlayEffect(const FString& EffectName, float Duration, float Intensity,
                                            EMGScreenEffectCategory Category)
{
    if (!IsCategoryEnabled(Category))
    {
        return FGuid();
    }

    FMGActiveEffect Effect;
    Effect.EffectId = FGuid::NewGuid();
    Effect.EffectName = EffectName;
    Effect.Category = Category;
    Effect.Intensity = Intensity * GlobalEffectScale;
    Effect.Duration = Duration;

    ActiveEffects.Add(Effect.EffectId, Effect);
    OnEffectStarted.Broadcast(Effect.EffectId);

    return Effect.EffectId;
}

void UMGScreenEffectSubsystem::StopEffect(const FGuid& EffectId, float FadeOutTime)
{
    if (FMGActiveEffect* Effect = ActiveEffects.Find(EffectId))
    {
        Effect->FadeOutTime = FadeOutTime;
        Effect->Duration = Effect->ElapsedTime + FadeOutTime;
    }
}

void UMGScreenEffectSubsystem::StopAllEffects()
{
    ActiveEffects.Empty();
    CurrentSpeedEffectIntensity = 0.0f;
    CurrentDamageEffectIntensity = 0.0f;
    CurrentBoostEffectIntensity = 0.0f;
    bBoostActive = false;
    StopScreenShake();
}

void UMGScreenEffectSubsystem::PauseEffect(const FGuid& EffectId)
{
    if (FMGActiveEffect* Effect = ActiveEffects.Find(EffectId))
    {
        Effect->bPaused = true;
    }
}

void UMGScreenEffectSubsystem::ResumeEffect(const FGuid& EffectId)
{
    if (FMGActiveEffect* Effect = ActiveEffects.Find(EffectId))
    {
        Effect->bPaused = false;
    }
}

void UMGScreenEffectSubsystem::SetEffectIntensity(const FGuid& EffectId, float Intensity)
{
    if (FMGActiveEffect* Effect = ActiveEffects.Find(EffectId))
    {
        float OldIntensity = Effect->Intensity;
        Effect->Intensity = Intensity * GlobalEffectScale;

        if (!FMath::IsNearlyEqual(OldIntensity, Effect->Intensity))
        {
            OnEffectIntensityChanged.Broadcast(EffectId, Effect->Intensity);
        }
    }
}

bool UMGScreenEffectSubsystem::IsEffectActive(const FGuid& EffectId) const
{
    return ActiveEffects.Contains(EffectId);
}

TArray<FMGActiveEffect> UMGScreenEffectSubsystem::GetActiveEffects() const
{
    TArray<FMGActiveEffect> Effects;
    ActiveEffects.GenerateValueArray(Effects);
    return Effects;
}

void UMGScreenEffectSubsystem::ApplyPreset(const FString& PresetName)
{
    if (const FMGEffectPreset* Preset = EffectPresets.Find(PresetName))
    {
        SpeedParams = Preset->SpeedParams;
        DamageParams = Preset->DamageParams;
        BoostParams = Preset->BoostParams;
        Y2KParams = Preset->Y2KParams;

        SetCategoryEnabled(EMGScreenEffectCategory::Speed, Preset->bEnableSpeed);
        SetCategoryEnabled(EMGScreenEffectCategory::Damage, Preset->bEnableDamage);
        SetCategoryEnabled(EMGScreenEffectCategory::Boost, Preset->bEnableBoost);
        SetCategoryEnabled(EMGScreenEffectCategory::Impact, Preset->bEnableImpact);
        SetCategoryEnabled(EMGScreenEffectCategory::Y2K, Preset->bEnableY2K);

        CurrentPresetName = PresetName;

        UE_LOG(LogTemp, Log, TEXT("ScreenEffect: Applied preset '%s'"), *PresetName);
    }
}

void UMGScreenEffectSubsystem::SavePreset(const FString& PresetName)
{
    FMGEffectPreset Preset;
    Preset.PresetName = PresetName;
    Preset.SpeedParams = SpeedParams;
    Preset.DamageParams = DamageParams;
    Preset.BoostParams = BoostParams;
    Preset.Y2KParams = Y2KParams;
    Preset.bEnableSpeed = IsCategoryEnabled(EMGScreenEffectCategory::Speed);
    Preset.bEnableDamage = IsCategoryEnabled(EMGScreenEffectCategory::Damage);
    Preset.bEnableBoost = IsCategoryEnabled(EMGScreenEffectCategory::Boost);
    Preset.bEnableImpact = IsCategoryEnabled(EMGScreenEffectCategory::Impact);
    Preset.bEnableY2K = IsCategoryEnabled(EMGScreenEffectCategory::Y2K);

    EffectPresets.Add(PresetName, Preset);
}

TArray<FString> UMGScreenEffectSubsystem::GetAvailablePresets() const
{
    TArray<FString> Names;
    EffectPresets.GetKeys(Names);
    return Names;
}

FMGEffectPreset UMGScreenEffectSubsystem::GetCurrentPreset() const
{
    if (const FMGEffectPreset* Preset = EffectPresets.Find(CurrentPresetName))
    {
        return *Preset;
    }
    return FMGEffectPreset();
}

void UMGScreenEffectSubsystem::SetGlobalEffectScale(float Scale)
{
    GlobalEffectScale = FMath::Clamp(Scale, 0.0f, 2.0f);
}

float UMGScreenEffectSubsystem::GetGlobalEffectScale() const
{
    return GlobalEffectScale;
}

void UMGScreenEffectSubsystem::SetEffectsEnabled(bool bEnabled)
{
    bEffectsEnabled = bEnabled;

    if (!bEnabled)
    {
        StopAllEffects();
    }
}

bool UMGScreenEffectSubsystem::AreEffectsEnabled() const
{
    return bEffectsEnabled;
}

void UMGScreenEffectSubsystem::SetCategoryEnabled(EMGScreenEffectCategory Category, bool bEnabled)
{
    CategoryEnabled.Add(Category, bEnabled);
}

bool UMGScreenEffectSubsystem::IsCategoryEnabled(EMGScreenEffectCategory Category) const
{
    if (const bool* Enabled = CategoryEnabled.Find(Category))
    {
        return *Enabled;
    }
    return true;
}

float UMGScreenEffectSubsystem::GetRadialBlurAmount() const
{
    return OutputRadialBlur;
}

float UMGScreenEffectSubsystem::GetChromaticAberrationAmount() const
{
    return OutputChromatic;
}

float UMGScreenEffectSubsystem::GetVignetteIntensity() const
{
    return OutputVignette;
}

FLinearColor UMGScreenEffectSubsystem::GetVignetteColor() const
{
    return OutputVignetteColor;
}

float UMGScreenEffectSubsystem::GetDesaturationAmount() const
{
    return OutputDesaturation;
}

float UMGScreenEffectSubsystem::GetBloomMultiplier() const
{
    return OutputBloom;
}

FLinearColor UMGScreenEffectSubsystem::GetScreenTint() const
{
    return OutputTint;
}

float UMGScreenEffectSubsystem::GetGlitchIntensity() const
{
    return OutputGlitch;
}

float UMGScreenEffectSubsystem::GetScanlineIntensity() const
{
    return OutputScanlines;
}

void UMGScreenEffectSubsystem::UpdateActiveEffects(float DeltaTime)
{
    for (auto& Pair : ActiveEffects)
    {
        FMGActiveEffect& Effect = Pair.Value;

        if (Effect.bPaused)
        {
            continue;
        }

        Effect.ElapsedTime += DeltaTime;

        // Handle looping
        if (Effect.bLooping && Effect.ElapsedTime >= Effect.Duration)
        {
            Effect.ElapsedTime = FMath::Fmod(Effect.ElapsedTime, Effect.Duration);
        }
    }
}

void UMGScreenEffectSubsystem::UpdateShake(float DeltaTime)
{
    if (!bShakeActive)
    {
        CurrentShakeOffset = FMath::VInterpTo(CurrentShakeOffset, FVector2D::ZeroVector, DeltaTime, 10.0f);
        return;
    }

    ShakeElapsedTime += DeltaTime;

    if (ShakeElapsedTime >= ShakeDuration)
    {
        bShakeActive = false;
        CurrentShakeOffset = FVector2D::ZeroVector;
        return;
    }

    // Calculate decay
    float Decay = 1.0f - (ShakeElapsedTime / ShakeDuration);
    float CurrentIntensity = ShakeIntensity * Decay;

    // Calculate shake offset based on type
    float Time = ShakeElapsedTime * ShakeFrequency;

    switch (ShakeType)
    {
        case EMGScreenShakeType::Subtle:
            CurrentShakeOffset.X = FMath::Sin(Time * 1.1f) * CurrentIntensity * 0.005f;
            CurrentShakeOffset.Y = FMath::Cos(Time * 0.9f) * CurrentIntensity * 0.005f;
            break;

        case EMGScreenShakeType::Normal:
            CurrentShakeOffset.X = FMath::Sin(Time) * CurrentIntensity * 0.01f;
            CurrentShakeOffset.Y = FMath::Cos(Time * 1.3f) * CurrentIntensity * 0.01f;
            break;

        case EMGScreenShakeType::Intense:
            CurrentShakeOffset.X = (FMath::Sin(Time) + FMath::Sin(Time * 2.3f) * 0.5f) * CurrentIntensity * 0.02f;
            CurrentShakeOffset.Y = (FMath::Cos(Time * 1.5f) + FMath::Cos(Time * 3.1f) * 0.5f) * CurrentIntensity * 0.02f;
            break;

        case EMGScreenShakeType::Rotational:
            CurrentShakeOffset.X = FMath::Sin(Time) * CurrentIntensity * 0.015f;
            CurrentShakeOffset.Y = CurrentShakeOffset.X;
            break;

        case EMGScreenShakeType::Directional:
            CurrentShakeOffset.X = FMath::Sin(Time) * CurrentIntensity * 0.015f;
            CurrentShakeOffset.Y = 0.0f;
            break;

        default:
            CurrentShakeOffset = FVector2D::ZeroVector;
            break;
    }
}

void UMGScreenEffectSubsystem::UpdateTransition(float DeltaTime)
{
    if (!bTransitioning)
    {
        return;
    }

    float ProgressDelta = DeltaTime / TransitionDuration;

    if (bFadingIn)
    {
        TransitionProgress -= ProgressDelta;

        if (TransitionProgress <= 0.0f)
        {
            TransitionProgress = 0.0f;
            bTransitioning = false;
            OnTransitionComplete.Broadcast(true);
        }
    }
    else
    {
        TransitionProgress += ProgressDelta;

        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bTransitioning = false;
            OnTransitionComplete.Broadcast(false);
        }
    }
}

void UMGScreenEffectSubsystem::CalculateOutputValues()
{
    // Reset outputs
    OutputRadialBlur = 0.0f;
    OutputChromatic = 0.0f;
    OutputVignette = 0.0f;
    OutputVignetteColor = FLinearColor::Black;
    OutputDesaturation = 0.0f;
    OutputBloom = 1.0f;
    OutputTint = FLinearColor::White;
    OutputGlitch = 0.0f;
    OutputScanlines = 0.0f;

    // Speed effects
    if (SpeedParams.bEnableRadialBlur)
    {
        OutputRadialBlur += SpeedParams.RadialBlurAmount * CurrentSpeedEffectIntensity;
    }
    OutputVignette += SpeedParams.VignetteIntensity * CurrentSpeedEffectIntensity;

    // Damage effects
    OutputVignette += DamageParams.VignetteRadius * CurrentDamageEffectIntensity;
    OutputVignetteColor = FLinearColor::LerpUsingHSV(OutputVignetteColor, DamageParams.DamageColor, CurrentDamageEffectIntensity);

    if (DamageParams.bEnableDesaturation)
    {
        OutputDesaturation += DamageParams.DesaturationAmount * CurrentDamageEffectIntensity;
    }

    if (DamageParams.bEnableChromatic)
    {
        OutputChromatic += DamageParams.ChromaticAberration * CurrentDamageEffectIntensity;
    }

    // Boost effects
    if (BoostParams.bEnableChromatic)
    {
        OutputChromatic += BoostParams.ChromaticStrength * CurrentBoostEffectIntensity;
    }

    if (BoostParams.bEnableBloom)
    {
        OutputBloom += BoostParams.BloomIntensity * CurrentBoostEffectIntensity;
    }

    OutputTint = FLinearColor::LerpUsingHSV(OutputTint, BoostParams.BoostColor, CurrentBoostEffectIntensity * 0.3f);

    // Y2K effects (base settings)
    if (IsCategoryEnabled(EMGScreenEffectCategory::Y2K))
    {
        if (Y2KParams.bEnableGlitch)
        {
            OutputGlitch = Y2KParams.GlitchIntensity;
        }
        if (Y2KParams.bEnableScanlines)
        {
            OutputScanlines = Y2KParams.ScanlineIntensity;
        }
        OutputChromatic += Y2KParams.RGBSplitAmount;
    }

    // Add active effects contribution
    for (const auto& Pair : ActiveEffects)
    {
        const FMGActiveEffect& Effect = Pair.Value;

        if (Effect.bPaused)
        {
            continue;
        }

        // Calculate effective intensity with fade
        float T = Effect.Duration > 0.0f ? Effect.ElapsedTime / Effect.Duration : 1.0f;
        float FadeIn = FMath::Clamp(Effect.ElapsedTime / FMath::Max(0.001f, Effect.FadeInTime), 0.0f, 1.0f);
        float FadeOut = FMath::Clamp((Effect.Duration - Effect.ElapsedTime) / FMath::Max(0.001f, Effect.FadeOutTime), 0.0f, 1.0f);
        float EffectiveIntensity = Effect.Intensity * FadeIn * FadeOut * ApplyIntensityCurve(T, Effect.IntensityCurve);

        switch (Effect.Category)
        {
            case EMGScreenEffectCategory::Impact:
                if (Effect.EffectName == TEXT("ImpactFlash"))
                {
                    OutputTint = FLinearColor::LerpUsingHSV(OutputTint, Effect.TintColor, EffectiveIntensity);
                }
                else if (Effect.EffectName == TEXT("ChromaticPunch"))
                {
                    OutputChromatic += EffectiveIntensity;
                }
                break;

            case EMGScreenEffectCategory::Damage:
                if (Effect.EffectName == TEXT("DamageFlash"))
                {
                    OutputVignetteColor = FLinearColor::LerpUsingHSV(OutputVignetteColor,
                                                                      DamageParams.DamageColor, EffectiveIntensity);
                }
                break;

            case EMGScreenEffectCategory::Y2K:
                OutputGlitch = FMath::Max(OutputGlitch, EffectiveIntensity);
                break;

            default:
                break;
        }
    }

    // Transition overlay
    if (bTransitioning)
    {
        float CurvedProgress = ApplyIntensityCurve(TransitionProgress, TransitionParams.Curve);
        OutputTint = FLinearColor::LerpUsingHSV(OutputTint, TransitionColor, CurvedProgress);
    }

    // Clamp outputs
    OutputRadialBlur = FMath::Clamp(OutputRadialBlur, 0.0f, 0.1f);
    OutputChromatic = FMath::Clamp(OutputChromatic, 0.0f, 0.1f);
    OutputVignette = FMath::Clamp(OutputVignette, 0.0f, 1.0f);
    OutputDesaturation = FMath::Clamp(OutputDesaturation, 0.0f, 1.0f);
    OutputBloom = FMath::Clamp(OutputBloom, 0.5f, 5.0f);
    OutputGlitch = FMath::Clamp(OutputGlitch, 0.0f, 1.0f);
    OutputScanlines = FMath::Clamp(OutputScanlines, 0.0f, 1.0f);
}

float UMGScreenEffectSubsystem::ApplyIntensityCurve(float T, EMGIntensityCurve Curve) const
{
    T = FMath::Clamp(T, 0.0f, 1.0f);

    switch (Curve)
    {
        case EMGIntensityCurve::Linear:
            return T;

        case EMGIntensityCurve::EaseIn:
            return T * T;

        case EMGIntensityCurve::EaseOut:
            return 1.0f - (1.0f - T) * (1.0f - T);

        case EMGIntensityCurve::EaseInOut:
            return T < 0.5f ? 2.0f * T * T : 1.0f - FMath::Pow(-2.0f * T + 2.0f, 2.0f) / 2.0f;

        case EMGIntensityCurve::Bounce:
            {
                float n1 = 7.5625f;
                float d1 = 2.75f;
                float x = 1.0f - T;

                if (x < 1.0f / d1)
                {
                    return 1.0f - n1 * x * x;
                }
                else if (x < 2.0f / d1)
                {
                    x -= 1.5f / d1;
                    return 1.0f - (n1 * x * x + 0.75f);
                }
                else if (x < 2.5f / d1)
                {
                    x -= 2.25f / d1;
                    return 1.0f - (n1 * x * x + 0.9375f);
                }
                else
                {
                    x -= 2.625f / d1;
                    return 1.0f - (n1 * x * x + 0.984375f);
                }
            }

        case EMGIntensityCurve::Elastic:
            {
                float c4 = (2.0f * PI) / 3.0f;
                return T == 0.0f ? 0.0f :
                       T == 1.0f ? 1.0f :
                       FMath::Pow(2.0f, -10.0f * T) * FMath::Sin((T * 10.0f - 0.75f) * c4) + 1.0f;
            }

        case EMGIntensityCurve::Pulse:
            return FMath::Sin(T * PI);

        case EMGIntensityCurve::Flicker:
            return T + FMath::FRand() * 0.2f - 0.1f;

        default:
            return T;
    }
}

void UMGScreenEffectSubsystem::CleanupExpiredEffects()
{
    TArray<FGuid> ToRemove;

    for (const auto& Pair : ActiveEffects)
    {
        if (!Pair.Value.bLooping && Pair.Value.ElapsedTime >= Pair.Value.Duration)
        {
            ToRemove.Add(Pair.Key);
        }
    }

    for (const FGuid& Id : ToRemove)
    {
        ActiveEffects.Remove(Id);
        OnEffectEnded.Broadcast(Id);
    }
}
