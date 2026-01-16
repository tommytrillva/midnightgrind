// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAccessibilitySubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGColorBlindMode : uint8
{
	None			UMETA(DisplayName = "None"),
	Deuteranopia	UMETA(DisplayName = "Deuteranopia (Green-Blind)"),
	Protanopia		UMETA(DisplayName = "Protanopia (Red-Blind)"),
	Tritanopia		UMETA(DisplayName = "Tritanopia (Blue-Blind)"),
	Achromatopsia	UMETA(DisplayName = "Achromatopsia (Total Color Blindness)")
};

UENUM(BlueprintType)
enum class EMGTextSize : uint8
{
	Small		UMETA(DisplayName = "Small"),
	Medium		UMETA(DisplayName = "Medium"),
	Large		UMETA(DisplayName = "Large"),
	ExtraLarge	UMETA(DisplayName = "Extra Large")
};

USTRUCT(BlueprintType)
struct FMGAccessibilitySettings
{
	GENERATED_BODY()

	// Visual
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGColorBlindMode ColorBlindMode = EMGColorBlindMode::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ColorBlindIntensity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGTextSize TextSize = EMGTextSize::Medium;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float UIScale = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHighContrastUI = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Brightness = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Contrast = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bReduceMotion = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDisableScreenShake = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDisableFlashing = false;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSubtitlesEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGTextSize SubtitleSize = EMGTextSize::Medium;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSubtitleBackground = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSpeakerNames = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMonoAudio = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bVisualizeAudio = false;

	// Controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoAccelerate = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoSteering = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHoldToAccelerate = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SteeringSensitivity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bInvertYAxis = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float InputDeadzone = 0.15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bOneHandedMode = false;

	// Gameplay
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bExtendedTimers = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimerMultiplier = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSimplifiedControls = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoNitro = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCollisionAssist = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBrakingAssist = false;

	// Screen Reader
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bScreenReaderEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScreenReaderSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDescribeUIElements = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAnnounceNotifications = true;
};

USTRUCT(BlueprintType)
struct FMGInputRemapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActionName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey PrimaryKey;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey SecondaryKey;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey GamepadKey;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessibilitySettingsChanged, const FMGAccessibilitySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenReaderSpeak, const FString&, Text);

UCLASS()
class MIDNIGHTGRIND_API UMGAccessibilitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable) FOnAccessibilitySettingsChanged OnAccessibilitySettingsChanged;
	UPROPERTY(BlueprintAssignable) FOnScreenReaderSpeak OnScreenReaderSpeak;

	// Settings
	UFUNCTION(BlueprintCallable) void SetAccessibilitySettings(const FMGAccessibilitySettings& Settings);
	UFUNCTION(BlueprintPure) FMGAccessibilitySettings GetAccessibilitySettings() const { return CurrentSettings; }
	UFUNCTION(BlueprintCallable) void ResetToDefaults();
	UFUNCTION(BlueprintCallable) void ApplyPreset(const FString& PresetName);

	// Visual
	UFUNCTION(BlueprintCallable) void SetColorBlindMode(EMGColorBlindMode Mode, float Intensity = 1.0f);
	UFUNCTION(BlueprintCallable) void SetTextSize(EMGTextSize Size);
	UFUNCTION(BlueprintCallable) void SetUIScale(float Scale);
	UFUNCTION(BlueprintCallable) void SetHighContrast(bool bEnabled);
	UFUNCTION(BlueprintCallable) void SetReduceMotion(bool bEnabled);
	UFUNCTION(BlueprintPure) FLinearColor GetAccessibleColor(FLinearColor OriginalColor) const;

	// Audio
	UFUNCTION(BlueprintCallable) void SetSubtitles(bool bEnabled, EMGTextSize Size = EMGTextSize::Medium);
	UFUNCTION(BlueprintCallable) void SetMonoAudio(bool bEnabled);
	UFUNCTION(BlueprintCallable) void SetVisualizeAudio(bool bEnabled);

	// Controls
	UFUNCTION(BlueprintCallable) void SetAutoAccelerate(bool bEnabled);
	UFUNCTION(BlueprintCallable) void SetAutoSteering(bool bEnabled);
	UFUNCTION(BlueprintCallable) void SetOneHandedMode(bool bEnabled);
	UFUNCTION(BlueprintCallable) void RemapInput(FName ActionName, FKey NewKey, bool bIsGamepad = false);
	UFUNCTION(BlueprintPure) TArray<FMGInputRemapping> GetInputRemappings() const { return InputRemappings; }
	UFUNCTION(BlueprintCallable) void ResetInputRemappings();

	// Screen Reader
	UFUNCTION(BlueprintCallable) void Speak(const FString& Text, bool bInterrupt = false);
	UFUNCTION(BlueprintCallable) void SpeakUIElement(const FString& ElementType, const FString& ElementName, const FString& Description);
	UFUNCTION(BlueprintCallable) void StopSpeaking();
	UFUNCTION(BlueprintPure) bool IsScreenReaderActive() const { return CurrentSettings.bScreenReaderEnabled; }

	// Presets
	UFUNCTION(BlueprintPure) TArray<FString> GetAvailablePresets() const;

protected:
	UPROPERTY() FMGAccessibilitySettings CurrentSettings;
	UPROPERTY() TArray<FMGInputRemapping> InputRemappings;
	UPROPERTY() TArray<FString> SpeechQueue;
	UPROPERTY() bool bIsSpeaking = false;

	void ApplyColorBlindFilter();
	void ApplyUIScaling();
	void UpdateSubtitleSettings();
	void LoadSettings();
	void SaveSettings();
	FLinearColor TransformColorForColorBlindness(FLinearColor Color, EMGColorBlindMode Mode, float Intensity) const;
};
