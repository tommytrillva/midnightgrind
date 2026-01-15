// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMenuSubsystem.generated.h"

class UUserWidget;
class UMGMainMenuWidget;
class UMGPauseMenuWidget;
class UMGSettingsWidget;
class UMGLoadingScreenWidget;

/**
 * Menu state
 */
UENUM(BlueprintType)
enum class EMGMenuState : uint8
{
	None,
	MainMenu,
	Paused,
	Settings,
	Loading,
	InGame
};

/**
 * Settings category
 */
UENUM(BlueprintType)
enum class EMGSettingsCategory : uint8
{
	Graphics,
	Audio,
	Controls,
	Gameplay,
	Accessibility
};

/**
 * Game settings structure
 */
USTRUCT(BlueprintType)
struct FMGGameSettings
{
	GENERATED_BODY()

	// ==========================================
	// GRAPHICS
	// ==========================================

	/** Resolution index */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 ResolutionIndex = 0;

	/** Fullscreen mode (0=Windowed, 1=Borderless, 2=Fullscreen) */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 FullscreenMode = 2;

	/** VSync enabled */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	bool bVSyncEnabled = true;

	/** Frame rate limit (0=unlimited) */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 FrameRateLimit = 60;

	/** Graphics quality (0=Low, 1=Medium, 2=High, 3=Ultra, 4=Custom) */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 GraphicsQuality = 2;

	/** Texture quality */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 TextureQuality = 2;

	/** Shadow quality */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 ShadowQuality = 2;

	/** Anti-aliasing quality */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 AntiAliasingQuality = 2;

	/** Post-process quality */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 PostProcessQuality = 2;

	/** Effects quality */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 EffectsQuality = 2;

	/** Motion blur enabled */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	bool bMotionBlurEnabled = true;

	/** Motion blur intensity */
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	float MotionBlurIntensity = 0.5f;

	// ==========================================
	// AUDIO
	// ==========================================

	/** Master volume */
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.0f;

	/** Music volume */
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 0.8f;

	/** SFX volume */
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float SFXVolume = 1.0f;

	/** Engine volume */
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float EngineVolume = 1.0f;

	/** Voice chat volume */
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float VoiceChatVolume = 0.8f;

	/** Enable voice chat */
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	bool bVoiceChatEnabled = true;

	// ==========================================
	// CONTROLS
	// ==========================================

	/** Steering sensitivity */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	float SteeringSensitivity = 1.0f;

	/** Controller vibration */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	bool bVibrationEnabled = true;

	/** Vibration intensity */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	float VibrationIntensity = 1.0f;

	/** Invert Y axis (camera) */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	bool bInvertYAxis = false;

	/** Automatic transmission */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	bool bAutomaticTransmission = true;

	/** Traction control assist */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	bool bTractionControl = true;

	/** Stability assist */
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	bool bStabilityAssist = true;

	// ==========================================
	// GAMEPLAY
	// ==========================================

	/** Camera style (0=Chase, 1=Hood, 2=Bumper, 3=Cockpit) */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	int32 DefaultCamera = 0;

	/** Show speedometer */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	bool bShowSpeedometer = true;

	/** Speedometer units (0=KPH, 1=MPH) */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	int32 SpeedUnits = 0;

	/** Show minimap */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	bool bShowMinimap = true;

	/** Show racing line */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	bool bShowRacingLine = false;

	/** Show opponent names */
	UPROPERTY(BlueprintReadWrite, Category = "Gameplay")
	bool bShowOpponentNames = true;

	// ==========================================
	// ACCESSIBILITY
	// ==========================================

	/** Color blind mode (0=Off, 1=Deuteranopia, 2=Protanopia, 3=Tritanopia) */
	UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
	int32 ColorBlindMode = 0;

	/** HUD scale */
	UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
	float HUDScale = 1.0f;

	/** Subtitle size */
	UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
	float SubtitleSize = 1.0f;

	/** Reduce screen shake */
	UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
	bool bReduceScreenShake = false;

	/** High contrast mode */
	UPROPERTY(BlueprintReadWrite, Category = "Accessibility")
	bool bHighContrastMode = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuStateChanged, EMGMenuState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsApplied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingProgress, float, Progress);

/**
 * Menu Subsystem
 * Central management for game menus
 *
 * Features:
 * - Menu state management
 * - Settings persistence
 * - Loading screen control
 * - Pause handling
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMenuSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMenuStateChanged OnMenuStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSettingsApplied OnSettingsApplied;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadingProgress OnLoadingProgress;

	// ==========================================
	// MENU CONTROL
	// ==========================================

	/** Show main menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowMainMenu();

	/** Hide main menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void HideMainMenu();

	/** Toggle pause menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void TogglePauseMenu();

	/** Show pause menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowPauseMenu();

	/** Hide pause menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void HidePauseMenu();

	/** Show settings menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowSettings(EMGSettingsCategory InitialCategory = EMGSettingsCategory::Graphics);

	/** Hide settings menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void HideSettings();

	/** Show loading screen */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void ShowLoadingScreen(const FText& LoadingText = FText());

	/** Hide loading screen */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void HideLoadingScreen();

	/** Update loading progress */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void UpdateLoadingProgress(float Progress);

	/** Get current menu state */
	UFUNCTION(BlueprintPure, Category = "Menu")
	EMGMenuState GetMenuState() const { return CurrentMenuState; }

	/** Is game paused via menu */
	UFUNCTION(BlueprintPure, Category = "Menu")
	bool IsGamePaused() const { return CurrentMenuState == EMGMenuState::Paused; }

	// ==========================================
	// SETTINGS
	// ==========================================

	/** Get current settings */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FMGGameSettings GetSettings() const { return CurrentSettings; }

	/** Apply settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySettings(const FMGGameSettings& NewSettings);

	/** Apply and save settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAndSaveSettings(const FMGGameSettings& NewSettings);

	/** Reset settings to defaults */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetSettingsToDefaults();

	/** Save settings to disk */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	/** Load settings from disk */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();

	// ==========================================
	// NAVIGATION
	// ==========================================

	/** Start new game */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void StartGame();

	/** Return to main menu */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ReturnToMainMenu();

	/** Quit game */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void QuitGame();

	/** Resume game from pause */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ResumeGame();

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Main menu widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGMainMenuWidget> MainMenuClass;

	/** Pause menu widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGPauseMenuWidget> PauseMenuClass;

	/** Settings widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGSettingsWidget> SettingsClass;

	/** Loading screen widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGLoadingScreenWidget> LoadingScreenClass;

	// ==========================================
	// STATE
	// ==========================================

	/** Current menu state */
	EMGMenuState CurrentMenuState = EMGMenuState::None;

	/** Current settings */
	FMGGameSettings CurrentSettings;

	/** Active menu widgets */
	UPROPERTY()
	UMGMainMenuWidget* MainMenuWidget;

	UPROPERTY()
	UMGPauseMenuWidget* PauseMenuWidget;

	UPROPERTY()
	UMGSettingsWidget* SettingsWidget;

	UPROPERTY()
	UMGLoadingScreenWidget* LoadingScreenWidget;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Set menu state */
	void SetMenuState(EMGMenuState NewState);

	/** Apply graphics settings */
	void ApplyGraphicsSettings();

	/** Apply audio settings */
	void ApplyAudioSettings();

	/** Get settings save path */
	FString GetSettingsSavePath() const;
};
