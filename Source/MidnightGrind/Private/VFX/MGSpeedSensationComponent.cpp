// Copyright Midnight Grind. All Rights Reserved.

#include "VFX/MGSpeedSensationComponent.h"
#include "VFX/MGCameraVFXComponent.h"
#include "VFX/MG_VHCL_VFXComponent.h"
#include "PostProcess/MGPostProcessSubsystem.h"
#include "ScreenEffect/MGScreenEffectSubsystem.h"
#include "Audio/MGEngineAudioComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Curves/CurveFloat.h"

UMGSpeedSensationComponent::UMGSpeedSensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // Update after physics
	
	// Initialize default category settings
	for (int32 i = 0; i < (int32)EMGSpeedEffectCategory::HUDDistortion + 1; i++)
	{
		EMGSpeedEffectCategory Category = (EMGSpeedEffectCategory)i;
		CategoryIntensities.Add(Category, 1.0f);
		CategoryEnabled.Add(Category, true);
	}
}

void UMGSpeedSensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply default profile
	SetEffectProfile(CurrentProfile);
	
	// Cache references
	InitializeReferences();
}

void UMGSpeedSensationComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up any active effects
	if (CachedScreenEffectSubsystem)
	{
		CachedScreenEffectSubsystem->StopAllEffects();
	}
	
	Super::EndPlay(EndPlayReason);
}

void UMGSpeedSensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bEffectsPaused)
	{
		return;
	}
	
	// Calculate current speed
	float CurrentSpeed = bUseManualSpeed ? ManualSpeedKPH : GetCurrentSpeedKPH();
	
	// Calculate speed intensity
	TargetSpeedIntensity = CalculateSpeedIntensity(CurrentSpeed);
	
	// Smooth interpolation
	CurrentSpeedIntensity = FMath::FInterpTo(CurrentSpeedIntensity, TargetSpeedIntensity, DeltaTime, 3.0f);
	
	// Update contextual effects
	UpdateBoost(DeltaTime);
	UpdateProximityPulse(DeltaTime);
	
	// Update all effect systems
	UpdateEffects(DeltaTime);
	
	// Check for threshold crossing
	CheckThresholdCrossing();
	
	// Notify intensity changes
	if (FMath::Abs(CurrentSpeedIntensity - LastNotifiedIntensity) > 0.05f)
	{
		NotifyIntensityChange(CurrentSpeedIntensity);
		LastNotifiedIntensity = CurrentSpeedIntensity;
	}
}

// ==========================================
// PROFILE MANAGEMENT
// ==========================================

void UMGSpeedSensationComponent::SetEffectProfile(EMGSpeedSensationProfile Profile)
{
	CurrentProfile = Profile;
	Config = CreateConfigForProfile(Profile);
}

void UMGSpeedSensationComponent::SetCustomConfiguration(const FMGSpeedSensationConfig& NewConfig)
{
	Config = NewConfig;
	CurrentProfile = EMGSpeedSensationProfile::Custom;
}

// ==========================================
// INTENSITY CONTROL
// ==========================================

void UMGSpeedSensationComponent::SetGlobalIntensityScale(float Scale)
{
	Config.GlobalIntensityScale = FMath::Clamp(Scale, 0.0f, 2.0f);
}

void UMGSpeedSensationComponent::SetCategoryIntensity(EMGSpeedEffectCategory Category, float Intensity)
{
	CategoryIntensities.Add(Category, FMath::Clamp(Intensity, 0.0f, 2.0f));
}

float UMGSpeedSensationComponent::GetCategoryIntensity(EMGSpeedEffectCategory Category) const
{
	const float* Found = CategoryIntensities.Find(Category);
	return Found ? *Found : 1.0f;
}

void UMGSpeedSensationComponent::SetCategoryEnabled(EMGSpeedEffectCategory Category, bool bEnabled)
{
	CategoryEnabled.Add(Category, bEnabled);
}

bool UMGSpeedSensationComponent::IsCategoryEnabled(EMGSpeedEffectCategory Category) const
{
	const bool* Found = CategoryEnabled.Find(Category);
	return Found ? *Found : true;
}

// ==========================================
// CONTEXTUAL MODIFIERS
// ==========================================

void UMGSpeedSensationComponent::SetEnvironmentMultiplier(float Multiplier)
{
	EnvironmentMultiplier = FMath::Clamp(Multiplier, 0.0f, 3.0f);
}

void UMGSpeedSensationComponent::TriggerProximityPulse(float Intensity, float Duration)
{
	ProximityPulseIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);
	ProximityPulseDuration = Duration;
	ProximityPulseElapsed = 0.0f;
}

void UMGSpeedSensationComponent::ApplySpeedBoost(const FMGSpeedBoostParams& BoostParams)
{
	CurrentBoostParams = BoostParams;
	bBoostActive = true;
	BoostElapsedTime = 0.0f;
	
	OnSpeedBoostApplied.Broadcast(BoostParams.GlobalMultiplier);
}

void UMGSpeedSensationComponent::BoostIntensity(float Multiplier, float Duration)
{
	FMGSpeedBoostParams Params;
	Params.Duration = Duration;
	Params.GlobalMultiplier = Multiplier;
	Params.FOVMultiplier = Multiplier;
	Params.MotionBlurMultiplier = Multiplier;
	Params.SpeedLinesMultiplier = Multiplier;
	Params.ParticleTrailMultiplier = Multiplier * 2.0f;
	Params.ChromaticMultiplier = Multiplier;
	
	ApplySpeedBoost(Params);
}

void UMGSpeedSensationComponent::StopBoost()
{
	if (bBoostActive)
	{
		// Fade out over remaining fade time
		bBoostActive = false;
	}
}

// ==========================================
// MANUAL CONTROL
// ==========================================

void UMGSpeedSensationComponent::SetManualSpeed(float SpeedKPH)
{
	bUseManualSpeed = true;
	ManualSpeedKPH = SpeedKPH;
}

void UMGSpeedSensationComponent::ClearManualSpeed()
{
	bUseManualSpeed = false;
	ManualSpeedKPH = 0.0f;
}

void UMGSpeedSensationComponent::PauseEffects()
{
	bEffectsPaused = true;
}

void UMGSpeedSensationComponent::ResumeEffects()
{
	bEffectsPaused = false;
}

// ==========================================
// UTILITY
// ==========================================

float UMGSpeedSensationComponent::GetCurrentSpeedKPH() const
{
	if (CachedOwnerPawn)
	{
		FVector Velocity = CachedOwnerPawn->GetVelocity();
		float SpeedCmPerSec = Velocity.Size();
		return SpeedCmPerSec * 0.036f; // Convert cm/s to km/h
	}
	
	return 0.0f;
}

void UMGSpeedSensationComponent::ResetToDefaults()
{
	SetEffectProfile(EMGSpeedSensationProfile::Modern);
	
	// Reset all category overrides
	for (auto& Pair : CategoryIntensities)
	{
		Pair.Value = 1.0f;
	}
	
	for (auto& Pair : CategoryEnabled)
	{
		Pair.Value = true;
	}
	
	EnvironmentMultiplier = 1.0f;
	StopBoost();
	ClearManualSpeed();
}

TArray<FString> UMGSpeedSensationComponent::GetAvailableProfiles() const
{
	TArray<FString> Profiles;
	Profiles.Add(TEXT("Modern"));
	Profiles.Add(TEXT("Arcade"));
	Profiles.Add(TEXT("Simulation"));
	Profiles.Add(TEXT("Y2K Cyberpunk"));
	Profiles.Add(TEXT("Cinematic"));
	return Profiles;
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void UMGSpeedSensationComponent::InitializeReferences()
{
	// Cache owner pawn
	CachedOwnerPawn = Cast<APawn>(GetOwner());
	
	if (!CachedOwnerPawn)
	{
		return;
	}
	
	// Find camera VFX component on owner
	CachedCameraVFX = CachedOwnerPawn->FindComponentByClass<UMGCameraVFXComponent>();
	
	// Find vehicle VFX component
	CachedVehicleVFX = CachedOwnerPawn->FindComponentByClass<UMGVehicleVFXComponent>();
	
	// Find engine audio component
	CachedEngineAudio = CachedOwnerPawn->FindComponentByClass<UMGEngineAudioComponent>();
	
	// Get subsystems from game instance
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GameInstance)
	{
		CachedPostProcessSubsystem = GameInstance->GetSubsystem<UMGPostProcessSubsystem>();
		CachedScreenEffectSubsystem = GameInstance->GetSubsystem<UMGScreenEffectSubsystem>();
	}
}

FMGSpeedSensationConfig UMGSpeedSensationComponent::CreateConfigForProfile(EMGSpeedSensationProfile Profile) const
{
	FMGSpeedSensationConfig NewConfig;
	
	switch (Profile)
	{
	case EMGSpeedSensationProfile::Modern:
		NewConfig.ProfileName = "Modern";
		NewConfig.MinSpeedThreshold = 80.0f;
		NewConfig.MaxSpeedThreshold = 300.0f;
		NewConfig.GlobalIntensityScale = 1.0f;
		
		NewConfig.FOVSettings.BaseFOV = 90.0f;
		NewConfig.FOVSettings.MaxFOVIncrease = 12.0f;
		NewConfig.FOVSettings.FOVInterpSpeed = 3.0f;
		NewConfig.FOVSettings.FOVCurve = EMGSpeedCurveType::EaseOut;
		
		NewConfig.ShakeSettings.bEnableSpeedShake = true;
		NewConfig.ShakeSettings.ShakeStartSpeed = 150.0f;
		NewConfig.ShakeSettings.MaxShakeIntensity = 0.12f;
		NewConfig.ShakeSettings.ShakeFrequency = 25.0f;
		
		NewConfig.MotionBlurSettings.bEnableMotionBlur = true;
		NewConfig.MotionBlurSettings.BaseBlurAmount = 0.3f;
		NewConfig.MotionBlurSettings.MaxBlurIncrease = 0.3f;
		NewConfig.MotionBlurSettings.bEnableRadialBlur = true;
		NewConfig.MotionBlurSettings.MaxRadialBlurStrength = 0.2f;
		
		NewConfig.SpeedLinesSettings.bEnableSpeedLines = true;
		NewConfig.SpeedLinesSettings.SpeedLineStyle = "Radial";
		NewConfig.SpeedLinesSettings.LineDensity = 32;
		NewConfig.SpeedLinesSettings.MaxLineOpacity = 0.4f;
		NewConfig.SpeedLinesSettings.bPeripheralOnly = true;
		NewConfig.SpeedLinesSettings.ClearCenterRadius = 0.35f;
		
		NewConfig.ChromaticSettings.bEnableChromatic = true;
		NewConfig.ChromaticSettings.BaseIntensity = 0.0f;
		NewConfig.ChromaticSettings.MaxIntensityIncrease = 0.5f;
		NewConfig.ChromaticSettings.bRadialDistribution = true;
		NewConfig.ChromaticSettings.CenterClearRadius = 0.45f;
		
		NewConfig.VignetteSettings.bEnableVignette = true;
		NewConfig.VignetteSettings.BaseIntensity = 0.1f;
		NewConfig.VignetteSettings.MaxIntensityIncrease = 0.25f;
		NewConfig.VignetteSettings.VignetteColor = FLinearColor::Black;
		
		NewConfig.ParticleSettings.bEnableParticleTrails = true;
		NewConfig.ParticleSettings.MaxSpawnRateMultiplier = 2.5f;
		
		NewConfig.AudioSettings.bEnableAudioEffects = true;
		NewConfig.AudioSettings.bScaleWindIntensity = true;
		NewConfig.AudioSettings.MaxWindMultiplier = 1.3f;
		break;
		
	case EMGSpeedSensationProfile::Arcade:
		NewConfig.ProfileName = "Arcade";
		NewConfig.MinSpeedThreshold = 60.0f;
		NewConfig.MaxSpeedThreshold = 280.0f;
		NewConfig.GlobalIntensityScale = 1.3f;
		
		NewConfig.FOVSettings.BaseFOV = 90.0f;
		NewConfig.FOVSettings.MaxFOVIncrease = 20.0f;
		NewConfig.FOVSettings.FOVInterpSpeed = 4.0f;
		NewConfig.FOVSettings.FOVCurve = EMGSpeedCurveType::Linear;
		
		NewConfig.ShakeSettings.bEnableSpeedShake = true;
		NewConfig.ShakeSettings.ShakeStartSpeed = 120.0f;
		NewConfig.ShakeSettings.MaxShakeIntensity = 0.18f;
		NewConfig.ShakeSettings.ShakeFrequency = 30.0f;
		
		NewConfig.MotionBlurSettings.bEnableMotionBlur = true;
		NewConfig.MotionBlurSettings.BaseBlurAmount = 0.4f;
		NewConfig.MotionBlurSettings.MaxBlurIncrease = 0.5f;
		NewConfig.MotionBlurSettings.bEnableRadialBlur = true;
		NewConfig.MotionBlurSettings.MaxRadialBlurStrength = 0.35f;
		
		NewConfig.SpeedLinesSettings.bEnableSpeedLines = true;
		NewConfig.SpeedLinesSettings.SpeedLineStyle = "Anime";
		NewConfig.SpeedLinesSettings.LineDensity = 48;
		NewConfig.SpeedLinesSettings.MaxLineOpacity = 0.6f;
		NewConfig.SpeedLinesSettings.bPeripheralOnly = false;
		NewConfig.SpeedLinesSettings.ClearCenterRadius = 0.2f;
		
		NewConfig.ChromaticSettings.bEnableChromatic = true;
		NewConfig.ChromaticSettings.BaseIntensity = 0.1f;
		NewConfig.ChromaticSettings.MaxIntensityIncrease = 0.8f;
		
		NewConfig.VignetteSettings.bEnableVignette = true;
		NewConfig.VignetteSettings.BaseIntensity = 0.15f;
		NewConfig.VignetteSettings.MaxIntensityIncrease = 0.35f;
		
		NewConfig.ParticleSettings.bEnableParticleTrails = true;
		NewConfig.ParticleSettings.MaxSpawnRateMultiplier = 4.0f;
		NewConfig.ParticleSettings.TrailColor = FLinearColor(1.0f, 0.6f, 0.0f, 0.8f);
		
		NewConfig.AudioSettings.MaxWindMultiplier = 1.6f;
		break;
		
	case EMGSpeedSensationProfile::Simulation:
		NewConfig.ProfileName = "Simulation";
		NewConfig.MinSpeedThreshold = 100.0f;
		NewConfig.MaxSpeedThreshold = 320.0f;
		NewConfig.GlobalIntensityScale = 0.7f;
		
		NewConfig.FOVSettings.BaseFOV = 90.0f;
		NewConfig.FOVSettings.MaxFOVIncrease = 8.0f;
		NewConfig.FOVSettings.FOVInterpSpeed = 2.0f;
		NewConfig.FOVSettings.FOVCurve = EMGSpeedCurveType::EaseInOut;
		
		NewConfig.ShakeSettings.bEnableSpeedShake = true;
		NewConfig.ShakeSettings.ShakeStartSpeed = 180.0f;
		NewConfig.ShakeSettings.MaxShakeIntensity = 0.08f;
		NewConfig.ShakeSettings.ShakeFrequency = 20.0f;
		NewConfig.ShakeSettings.bDirectionalShake = true;
		
		NewConfig.MotionBlurSettings.bEnableMotionBlur = true;
		NewConfig.MotionBlurSettings.BaseBlurAmount = 0.25f;
		NewConfig.MotionBlurSettings.MaxBlurIncrease = 0.25f;
		NewConfig.MotionBlurSettings.bEnableRadialBlur = false;
		
		NewConfig.SpeedLinesSettings.bEnableSpeedLines = false; // No speed lines in sim mode
		
		NewConfig.ChromaticSettings.bEnableChromatic = false; // No chromatic in sim
		
		NewConfig.VignetteSettings.bEnableVignette = true;
		NewConfig.VignetteSettings.BaseIntensity = 0.05f;
		NewConfig.VignetteSettings.MaxIntensityIncrease = 0.15f;
		
		NewConfig.ParticleSettings.bEnableParticleTrails = false; // No particles in sim
		
		NewConfig.AudioSettings.MaxWindMultiplier = 1.2f;
		break;
		
	case EMGSpeedSensationProfile::Y2KCyberpunk:
		NewConfig.ProfileName = "Y2K Cyberpunk";
		NewConfig.MinSpeedThreshold = 70.0f;
		NewConfig.MaxSpeedThreshold = 290.0f;
		NewConfig.GlobalIntensityScale = 1.2f;
		
		NewConfig.FOVSettings.BaseFOV = 90.0f;
		NewConfig.FOVSettings.MaxFOVIncrease = 18.0f;
		NewConfig.FOVSettings.FOVInterpSpeed = 3.5f;
		
		NewConfig.ShakeSettings.bEnableSpeedShake = true;
		NewConfig.ShakeSettings.MaxShakeIntensity = 0.15f;
		
		NewConfig.MotionBlurSettings.bEnableMotionBlur = true;
		NewConfig.MotionBlurSettings.BaseBlurAmount = 0.35f;
		NewConfig.MotionBlurSettings.MaxBlurIncrease = 0.4f;
		NewConfig.MotionBlurSettings.bEnableRadialBlur = true;
		NewConfig.MotionBlurSettings.MaxRadialBlurStrength = 0.3f;
		
		NewConfig.SpeedLinesSettings.bEnableSpeedLines = true;
		NewConfig.SpeedLinesSettings.SpeedLineStyle = "Neon";
		NewConfig.SpeedLinesSettings.LineDensity = 40;
		NewConfig.SpeedLinesSettings.MaxLineOpacity = 0.55f;
		NewConfig.SpeedLinesSettings.LineColor = FLinearColor(0.0f, 1.0f, 0.8f, 0.7f); // Cyan
		
		NewConfig.ChromaticSettings.bEnableChromatic = true;
		NewConfig.ChromaticSettings.BaseIntensity = 0.2f;
		NewConfig.ChromaticSettings.MaxIntensityIncrease = 1.0f;
		NewConfig.ChromaticSettings.bRadialDistribution = true;
		
		NewConfig.VignetteSettings.bEnableVignette = true;
		NewConfig.VignetteSettings.BaseIntensity = 0.2f;
		NewConfig.VignetteSettings.MaxIntensityIncrease = 0.3f;
		NewConfig.VignetteSettings.VignetteColor = FLinearColor(0.1f, 0.0f, 0.2f, 1.0f); // Purple tint
		
		NewConfig.ParticleSettings.bEnableParticleTrails = true;
		NewConfig.ParticleSettings.MaxSpawnRateMultiplier = 3.5f;
		NewConfig.ParticleSettings.TrailColor = FLinearColor(1.0f, 0.0f, 0.8f, 0.8f); // Magenta
		
		NewConfig.AudioSettings.MaxWindMultiplier = 1.5f;
		NewConfig.AudioSettings.bEnableDopplerShift = true;
		NewConfig.AudioSettings.DopplerIntensity = 0.7f;
		break;
		
	case EMGSpeedSensationProfile::Cinematic:
		NewConfig.ProfileName = "Cinematic";
		NewConfig.MinSpeedThreshold = 85.0f;
		NewConfig.MaxSpeedThreshold = 310.0f;
		NewConfig.GlobalIntensityScale = 1.1f;
		
		NewConfig.FOVSettings.BaseFOV = 90.0f;
		NewConfig.FOVSettings.MaxFOVIncrease = 16.0f;
		NewConfig.FOVSettings.FOVInterpSpeed = 2.5f;
		NewConfig.FOVSettings.FOVCurve = EMGSpeedCurveType::EaseInOut;
		
		NewConfig.ShakeSettings.bEnableSpeedShake = true;
		NewConfig.ShakeSettings.MaxShakeIntensity = 0.1f;
		
		NewConfig.MotionBlurSettings.bEnableMotionBlur = true;
		NewConfig.MotionBlurSettings.BaseBlurAmount = 0.4f;
		NewConfig.MotionBlurSettings.MaxBlurIncrease = 0.5f;
		NewConfig.MotionBlurSettings.bEnableRadialBlur = true;
		NewConfig.MotionBlurSettings.MaxRadialBlurStrength = 0.25f;
		
		NewConfig.SpeedLinesSettings.bEnableSpeedLines = true;
		NewConfig.SpeedLinesSettings.LineDensity = 24;
		NewConfig.SpeedLinesSettings.MaxLineOpacity = 0.3f;
		NewConfig.SpeedLinesSettings.bPeripheralOnly = true;
		NewConfig.SpeedLinesSettings.ClearCenterRadius = 0.4f;
		
		NewConfig.ChromaticSettings.bEnableChromatic = true;
		NewConfig.ChromaticSettings.MaxIntensityIncrease = 0.6f;
		
		NewConfig.VignetteSettings.bEnableVignette = true;
		NewConfig.VignetteSettings.BaseIntensity = 0.25f;
		NewConfig.VignetteSettings.MaxIntensityIncrease = 0.35f;
		
		NewConfig.ParticleSettings.bEnableParticleTrails = true;
		NewConfig.ParticleSettings.MaxSpawnRateMultiplier = 2.0f;
		
		break;
		
	default:
		// Custom or unknown - return current config
		return Config;
	}
	
	return NewConfig;
}

float UMGSpeedSensationComponent::CalculateSpeedIntensity(float SpeedKPH) const
{
	if (SpeedKPH <= Config.MinSpeedThreshold)
	{
		return 0.0f;
	}
	
	if (SpeedKPH >= Config.MaxSpeedThreshold)
	{
		return 1.0f;
	}
	
	// Normalize speed to 0-1 range
	float NormalizedSpeed = (SpeedKPH - Config.MinSpeedThreshold) / (Config.MaxSpeedThreshold - Config.MinSpeedThreshold);
	
	return FMath::Clamp(NormalizedSpeed, 0.0f, 1.0f);
}

float UMGSpeedSensationComponent::ApplyCurve(float Value, EMGSpeedCurveType CurveType, UCurveFloat* CustomCurve) const
{
	if (CurveType == EMGSpeedCurveType::Custom && CustomCurve)
	{
		return CustomCurve->GetFloatValue(Value);
	}
	
	switch (CurveType)
	{
	case EMGSpeedCurveType::Linear:
		return Value;
		
	case EMGSpeedCurveType::EaseIn:
		return Value * Value;
		
	case EMGSpeedCurveType::EaseOut:
		return 1.0f - (1.0f - Value) * (1.0f - Value);
		
	case EMGSpeedCurveType::EaseInOut:
		return Value < 0.5f ? 
			2.0f * Value * Value : 
			1.0f - FMath::Pow(-2.0f * Value + 2.0f, 2.0f) / 2.0f;
		
	case EMGSpeedCurveType::Exponential:
		return FMath::Pow(Value, 2.5f);
		
	default:
		return Value;
	}
}

void UMGSpeedSensationComponent::UpdateEffects(float DeltaTime)
{
	// Calculate base intensity (without boost)
	float BaseIntensity = CurrentSpeedIntensity;
	
	UpdateCameraEffects(BaseIntensity);
	UpdatePostProcessEffects(BaseIntensity);
	UpdateScreenEffects(BaseIntensity);
	UpdateParticleEffects(BaseIntensity);
	UpdateAudioEffects(BaseIntensity);
}

void UMGSpeedSensationComponent::UpdateCameraEffects(float IntensityOverride)
{
	if (!CachedCameraVFX)
	{
		return;
	}
	
	float BaseIntensity = IntensityOverride >= 0.0f ? IntensityOverride : CurrentSpeedIntensity;
	
	// FOV
	if (IsCategoryEnabled(EMGSpeedEffectCategory::CameraFOV) && Config.FOVSettings.MaxFOVIncrease > 0.0f)
	{
		float FOVIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::CameraFOV, BaseIntensity);
		FOVIntensity = ApplyCurve(FOVIntensity, Config.FOVSettings.FOVCurve, Config.FOVSettings.CustomFOVCurve);
		
		float TargetFOV = Config.FOVSettings.BaseFOV + (Config.FOVSettings.MaxFOVIncrease * FOVIntensity);
		
		// Apply boost multiplier
		if (bBoostActive)
		{
			float BoostAlpha = CalculateBoostAlpha();
			TargetFOV += (Config.FOVSettings.MaxFOVIncrease * 0.3f * CurrentBoostParams.FOVMultiplier * BoostAlpha);
		}
		
		CachedCameraVFX->SetBaseFOV(TargetFOV);
	}
	
	// Screen Shake
	if (IsCategoryEnabled(EMGSpeedEffectCategory::ScreenShake) && Config.ShakeSettings.bEnableSpeedShake)
	{
		float CurrentSpeed = bUseManualSpeed ? ManualSpeedKPH : GetCurrentSpeedKPH();
		
		if (CurrentSpeed >= Config.ShakeSettings.ShakeStartSpeed)
		{
			float ShakeIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::ScreenShake, BaseIntensity);
			ShakeIntensity *= Config.ShakeSettings.MaxShakeIntensity;
			
			// Start continuous shake if not already active
			CachedCameraVFX->StartContinuousShake(EMGCameraShakeType::SpeedVibration, ShakeIntensity);
		}
		else
		{
			// Stop shake when below threshold
			CachedCameraVFX->StopContinuousShake();
		}
	}
}

void UMGSpeedSensationComponent::UpdatePostProcessEffects(float IntensityOverride)
{
	if (!CachedPostProcessSubsystem)
	{
		return;
	}
	
	float BaseIntensity = IntensityOverride >= 0.0f ? IntensityOverride : CurrentSpeedIntensity;
	
	// Motion Blur
	if (IsCategoryEnabled(EMGSpeedEffectCategory::MotionBlur) && Config.MotionBlurSettings.bEnableMotionBlur)
	{
		float BlurIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::MotionBlur, BaseIntensity);
		
		// Apply boost multiplier
		if (bBoostActive)
		{
			BlurIntensity *= CurrentBoostParams.MotionBlurMultiplier * CalculateBoostAlpha();
		}
		
		float TargetBlur = Config.MotionBlurSettings.BaseBlurAmount + 
			(Config.MotionBlurSettings.MaxBlurIncrease * BlurIntensity);
		
		CachedPostProcessSubsystem->SetMotionBlurAmount(TargetBlur);
	}
	
	// Radial Blur (via speed effects)
	if (IsCategoryEnabled(EMGSpeedEffectCategory::RadialBlur) && Config.MotionBlurSettings.bEnableRadialBlur)
	{
		float RadialIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::RadialBlur, BaseIntensity);
		
		if (bBoostActive)
		{
			RadialIntensity *= (1.0f + CurrentBoostParams.GlobalMultiplier) * CalculateBoostAlpha();
		}
		
		float CurrentSpeed = bUseManualSpeed ? ManualSpeedKPH : GetCurrentSpeedKPH();
		CachedPostProcessSubsystem->UpdateSpeedEffect(CurrentSpeed * RadialIntensity);
	}
	
	// Chromatic Aberration
	if (IsCategoryEnabled(EMGSpeedEffectCategory::ChromaticAberration) && Config.ChromaticSettings.bEnableChromatic)
	{
		float ChromaticIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::ChromaticAberration, BaseIntensity);
		
		if (bBoostActive)
		{
			ChromaticIntensity *= CurrentBoostParams.ChromaticMultiplier * CalculateBoostAlpha();
		}
		
		float TargetChromatic = Config.ChromaticSettings.BaseIntensity + 
			(Config.ChromaticSettings.MaxIntensityIncrease * ChromaticIntensity);
		
		CachedPostProcessSubsystem->SetChromaticAberrationIntensity(TargetChromatic);
	}
	
	// Vignette
	if (IsCategoryEnabled(EMGSpeedEffectCategory::Vignette) && Config.VignetteSettings.bEnableVignette)
	{
		float VignetteIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::Vignette, BaseIntensity);
		
		float TargetVignette = Config.VignetteSettings.BaseIntensity + 
			(Config.VignetteSettings.MaxIntensityIncrease * VignetteIntensity);
		
		CachedPostProcessSubsystem->SetVignetteIntensity(TargetVignette);
	}
}

void UMGSpeedSensationComponent::UpdateScreenEffects(float IntensityOverride)
{
	if (!CachedScreenEffectSubsystem)
	{
		return;
	}
	
	float BaseIntensity = IntensityOverride >= 0.0f ? IntensityOverride : CurrentSpeedIntensity;
	
	// Speed Lines
	if (IsCategoryEnabled(EMGSpeedEffectCategory::SpeedLines) && Config.SpeedLinesSettings.bEnableSpeedLines)
	{
		float LineIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::SpeedLines, BaseIntensity);
		
		if (bBoostActive)
		{
			LineIntensity *= CurrentBoostParams.SpeedLinesMultiplier * CalculateBoostAlpha();
		}
		
		float CurrentSpeed = bUseManualSpeed ? ManualSpeedKPH : GetCurrentSpeedKPH();
		CachedScreenEffectSubsystem->UpdateSpeedEffect(CurrentSpeed * LineIntensity);
	}
}

void UMGSpeedSensationComponent::UpdateParticleEffects(float IntensityOverride)
{
	if (!CachedVehicleVFX || !Config.ParticleSettings.bEnableParticleTrails)
	{
		return;
	}
	
	if (!IsCategoryEnabled(EMGSpeedEffectCategory::ParticleTrails))
	{
		return;
	}
	
	float BaseIntensity = IntensityOverride >= 0.0f ? IntensityOverride : CurrentSpeedIntensity;
	float ParticleIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::ParticleTrails, BaseIntensity);
	
	if (bBoostActive)
	{
		ParticleIntensity *= CurrentBoostParams.ParticleTrailMultiplier * CalculateBoostAlpha();
	}
	
	// Update particle spawn rate based on speed
	// This would interface with the vehicle VFX component's speed trail system
	// Implementation depends on specific VFX component API
}

void UMGSpeedSensationComponent::UpdateAudioEffects(float IntensityOverride)
{
	if (!CachedEngineAudio || !Config.AudioSettings.bEnableAudioEffects)
	{
		return;
	}
	
	if (!IsCategoryEnabled(EMGSpeedEffectCategory::AudioDoppler))
	{
		return;
	}
	
	float BaseIntensity = IntensityOverride >= 0.0f ? IntensityOverride : CurrentSpeedIntensity;
	float AudioIntensity = CalculateFinalIntensity(EMGSpeedEffectCategory::AudioDoppler, BaseIntensity);
	
	// Update wind intensity
	if (Config.AudioSettings.bScaleWindIntensity)
	{
		float WindMultiplier = 1.0f + (AudioIntensity * (Config.AudioSettings.MaxWindMultiplier - 1.0f));
		// Would call: CachedEngineAudio->SetWindIntensityMultiplier(WindMultiplier);
	}
}

void UMGSpeedSensationComponent::UpdateBoost(float DeltaTime)
{
	if (!bBoostActive)
	{
		return;
	}
	
	BoostElapsedTime += DeltaTime;
	
	// Check if boost duration expired
	if (BoostElapsedTime >= CurrentBoostParams.Duration)
	{
		bBoostActive = false;
	}
}

void UMGSpeedSensationComponent::UpdateProximityPulse(float DeltaTime)
{
	if (ProximityPulseIntensity <= 0.0f)
	{
		return;
	}
	
	ProximityPulseElapsed += DeltaTime;
	
	if (ProximityPulseElapsed >= ProximityPulseDuration)
	{
		ProximityPulseIntensity = 0.0f;
		ProximityPulseElapsed = 0.0f;
	}
	else
	{
		// Fade out over duration
		float Alpha = 1.0f - (ProximityPulseElapsed / ProximityPulseDuration);
		ProximityPulseIntensity *= Alpha;
	}
}

float UMGSpeedSensationComponent::CalculateFinalIntensity(EMGSpeedEffectCategory Category, float BaseIntensity) const
{
	// Start with base intensity
	float FinalIntensity = BaseIntensity;
	
	// Apply global scale
	FinalIntensity *= Config.GlobalIntensityScale;
	
	// Apply category-specific intensity
	FinalIntensity *= GetCategoryIntensity(Category);
	
	// Apply environment multiplier
	FinalIntensity *= EnvironmentMultiplier;
	
	// Add proximity pulse
	FinalIntensity += ProximityPulseIntensity;
	
	return FMath::Clamp(FinalIntensity, 0.0f, 2.0f);
}

float UMGSpeedSensationComponent::CalculateBoostAlpha() const
{
	if (!bBoostActive)
	{
		return 0.0f;
	}
	
	float TotalDuration = CurrentBoostParams.Duration + CurrentBoostParams.FadeOutTime;
	
	if (BoostElapsedTime < CurrentBoostParams.Duration)
	{
		// Full intensity during active phase
		return 1.0f;
	}
	else
	{
		// Fade out
		float FadeElapsed = BoostElapsedTime - CurrentBoostParams.Duration;
		return 1.0f - (FadeElapsed / CurrentBoostParams.FadeOutTime);
	}
}

void UMGSpeedSensationComponent::CheckThresholdCrossing()
{
	float CurrentSpeed = bUseManualSpeed ? ManualSpeedKPH : GetCurrentSpeedKPH();
	bool bInHighSpeed = CurrentSpeed >= Config.MinSpeedThreshold;
	
	if (bInHighSpeed != bWasInHighSpeed)
	{
		OnSpeedThresholdCrossed.Broadcast(bInHighSpeed);
		bWasInHighSpeed = bInHighSpeed;
	}
}

void UMGSpeedSensationComponent::NotifyIntensityChange(float NewIntensity)
{
	OnSpeedIntensityChanged.Broadcast(NewIntensity);
}
