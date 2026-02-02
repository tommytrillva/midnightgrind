// Copyright Midnight Grind. All Rights Reserved.

#include "Accessibility/MGAccessibilitySubsystem.h"
#include "GameFramework/GameUserSettings.h"

void UMGAccessibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettings();
}

void UMGAccessibilitySubsystem::Deinitialize()
{
	SaveSettings();
	Super::Deinitialize();
}

void UMGAccessibilitySubsystem::SetAccessibilitySettings(const FMGAccessibilitySettings& Settings)
{
	CurrentSettings = Settings;
	ApplyColorBlindFilter();
	ApplyUIScaling();
	UpdateSubtitleSettings();
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
	SaveSettings();
}

void UMGAccessibilitySubsystem::ResetToDefaults()
{
	CurrentSettings = FMGAccessibilitySettings();
	SetAccessibilitySettings(CurrentSettings);
}

void UMGAccessibilitySubsystem::ApplyPreset(const FString& PresetName)
{
	if (PresetName == TEXT("VisuallyImpaired"))
	{
		CurrentSettings.TextSize = EMGTextSize::ExtraLarge;
		CurrentSettings.UIScale = 1.5f;
		CurrentSettings.bHighContrastUI = true;
		CurrentSettings.bScreenReaderEnabled = true;
		CurrentSettings.bSubtitlesEnabled = true;
		CurrentSettings.SubtitleSize = EMGTextSize::ExtraLarge;
	}
	else if (PresetName == TEXT("HearingImpaired"))
	{
		CurrentSettings.bSubtitlesEnabled = true;
		CurrentSettings.SubtitleSize = EMGTextSize::Large;
		CurrentSettings.bSpeakerNames = true;
		CurrentSettings.bVisualizeAudio = true;
	}
	else if (PresetName == TEXT("MotorImpaired"))
	{
		CurrentSettings.bAutoAccelerate = true;
		CurrentSettings.bAutoSteering = true;
		CurrentSettings.bSimplifiedControls = true;
		CurrentSettings.bExtendedTimers = true;
		CurrentSettings.bCollisionAssist = true;
	}
	else if (PresetName == TEXT("PhotosensitiveEpilepsy"))
	{
		CurrentSettings.bDisableFlashing = true;
		CurrentSettings.bDisableScreenShake = true;
		CurrentSettings.bReduceMotion = true;
	}

	SetAccessibilitySettings(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetColorBlindMode(EMGColorBlindMode Mode, float Intensity)
{
	CurrentSettings.ColorBlindMode = Mode;
	CurrentSettings.ColorBlindIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	ApplyColorBlindFilter();
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetTextSize(EMGTextSize Size)
{
	CurrentSettings.TextSize = Size;
	ApplyUIScaling();
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetUIScale(float Scale)
{
	CurrentSettings.UIScale = FMath::Clamp(Scale, 0.5f, 2.0f);
	ApplyUIScaling();
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetHighContrast(bool bEnabled)
{
	CurrentSettings.bHighContrastUI = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetReduceMotion(bool bEnabled)
{
	CurrentSettings.bReduceMotion = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

FLinearColor UMGAccessibilitySubsystem::GetAccessibleColor(FLinearColor OriginalColor) const
{
	return TransformColorForColorBlindness(OriginalColor, CurrentSettings.ColorBlindMode, CurrentSettings.ColorBlindIntensity);
}

void UMGAccessibilitySubsystem::SetSubtitles(bool bEnabled, EMGTextSize Size)
{
	CurrentSettings.bSubtitlesEnabled = bEnabled;
	CurrentSettings.SubtitleSize = Size;
	UpdateSubtitleSettings();
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetMonoAudio(bool bEnabled)
{
	CurrentSettings.bMonoAudio = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetVisualizeAudio(bool bEnabled)
{
	CurrentSettings.bVisualizeAudio = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetAutoAccelerate(bool bEnabled)
{
	CurrentSettings.bAutoAccelerate = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetAutoSteering(bool bEnabled)
{
	CurrentSettings.bAutoSteering = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::SetOneHandedMode(bool bEnabled)
{
	CurrentSettings.bOneHandedMode = bEnabled;
	OnAccessibilitySettingsChanged.Broadcast(CurrentSettings);
}

void UMGAccessibilitySubsystem::RemapInput(FName ActionName, FKey NewKey, bool bIsGamepad)
{
	FMGInputRemapping* Existing = InputRemappings.FindByPredicate([ActionName](const FMGInputRemapping& R) {
		return R.ActionName == ActionName;
	});

	if (Existing)
	{
		if (bIsGamepad) Existing->GamepadKey = NewKey;
		else Existing->PrimaryKey = NewKey;
	}
	else
	{
		FMGInputRemapping NewMapping;
		NewMapping.ActionName = ActionName;
		if (bIsGamepad) NewMapping.GamepadKey = NewKey;
		else NewMapping.PrimaryKey = NewKey;
		InputRemappings.Add(NewMapping);
	}
}

void UMGAccessibilitySubsystem::ResetInputRemappings()
{
	InputRemappings.Empty();
}

void UMGAccessibilitySubsystem::Speak(const FString& Text, bool bInterrupt)
{
	if (!CurrentSettings.bScreenReaderEnabled) return;

	if (bInterrupt)
	{
		SpeechQueue.Empty();
		bIsSpeaking = false;
	}

	SpeechQueue.Add(Text);
	OnScreenReaderSpeak.Broadcast(Text);
}

void UMGAccessibilitySubsystem::SpeakUIElement(const FString& ElementType, const FString& ElementName, const FString& Description)
{
	if (!CurrentSettings.bScreenReaderEnabled || !CurrentSettings.bDescribeUIElements) return;

	FString Text = FString::Printf(TEXT("%s: %s. %s"), *ElementType, *ElementName, *Description);
	Speak(Text, false);
}

void UMGAccessibilitySubsystem::StopSpeaking()
{
	SpeechQueue.Empty();
	bIsSpeaking = false;
}

TArray<FString> UMGAccessibilitySubsystem::GetAvailablePresets() const
{
	return { TEXT("VisuallyImpaired"), TEXT("HearingImpaired"), TEXT("MotorImpaired"), TEXT("PhotosensitiveEpilepsy") };
}

void UMGAccessibilitySubsystem::ApplyColorBlindFilter()
{
	// Would apply post-process color correction
}

void UMGAccessibilitySubsystem::ApplyUIScaling()
{
	// Would update UI scale
}

void UMGAccessibilitySubsystem::UpdateSubtitleSettings()
{
	// Would update subtitle widget settings
}

void UMGAccessibilitySubsystem::LoadSettings()
{
	// Would load from save file
}

void UMGAccessibilitySubsystem::SaveSettings()
{
	// Would save to save file
}

FLinearColor UMGAccessibilitySubsystem::TransformColorForColorBlindness(FLinearColor Color, EMGColorBlindMode Mode, float Intensity) const
{
	if (Mode == EMGColorBlindMode::None) return Color;

	// Simulate color blindness using color transformation matrices
	FLinearColor Result = Color;

	switch (Mode)
	{
		case EMGColorBlindMode::Deuteranopia:
			Result.R = Color.R * 0.625f + Color.G * 0.375f;
			Result.G = Color.R * 0.7f + Color.G * 0.3f;
			Result.B = Color.B;
			break;
		case EMGColorBlindMode::Protanopia:
			Result.R = Color.R * 0.567f + Color.G * 0.433f;
			Result.G = Color.R * 0.558f + Color.G * 0.442f;
			Result.B = Color.B;
			break;
		case EMGColorBlindMode::Tritanopia:
			Result.R = Color.R * 0.95f + Color.G * 0.05f;
			Result.G = Color.G * 0.433f + Color.B * 0.567f;
			Result.B = Color.G * 0.475f + Color.B * 0.525f;
			break;
		case EMGColorBlindMode::Achromatopsia:
			float Gray = Color.R * 0.299f + Color.G * 0.587f + Color.B * 0.114f;
			Result.R = Result.G = Result.B = Gray;
			break;
	}

	return FMath::Lerp(Color, Result, Intensity);
}
