// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGMenuSubsystem.h"
#include "UI/MGMenuWidgets.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UMGMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load saved settings
	LoadSettings();
}

void UMGMenuSubsystem::Deinitialize()
{
	// Clean up widgets
	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}

	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
		SettingsWidget = nullptr;
	}

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
	}

	Super::Deinitialize();
}

// ==========================================
// MENU CONTROL
// ==========================================

void UMGMenuSubsystem::ShowMainMenu()
{
	if (!MainMenuClass)
	{
		return;
	}

	if (!MainMenuWidget)
	{
		MainMenuWidget = CreateWidget<UMGMainMenuWidget>(GetGameInstance(), MainMenuClass);
	}

	if (MainMenuWidget && !MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->AddToViewport(100);
	}

	SetMenuState(EMGMenuState::MainMenu);

	// Set input mode to UI
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetGameInstance(), 0))
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UMGMenuSubsystem::HideMainMenu()
{
	if (MainMenuWidget && MainMenuWidget->IsInViewport())
	{
		MainMenuWidget->RemoveFromParent();
	}

	if (CurrentMenuState == EMGMenuState::MainMenu)
	{
		SetMenuState(EMGMenuState::InGame);
	}
}

void UMGMenuSubsystem::TogglePauseMenu()
{
	if (CurrentMenuState == EMGMenuState::Paused)
	{
		HidePauseMenu();
	}
	else if (CurrentMenuState == EMGMenuState::InGame)
	{
		ShowPauseMenu();
	}
}

void UMGMenuSubsystem::ShowPauseMenu()
{
	if (CurrentMenuState != EMGMenuState::InGame)
	{
		return;
	}

	if (!PauseMenuClass)
	{
		return;
	}

	if (!PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UMGPauseMenuWidget>(GetGameInstance(), PauseMenuClass);
	}

	if (PauseMenuWidget && !PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->AddToViewport(200);
	}

	// Pause game
	UGameplayStatics::SetGamePaused(GetGameInstance(), true);

	SetMenuState(EMGMenuState::Paused);

	// Set input mode
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetGameInstance(), 0))
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void UMGMenuSubsystem::HidePauseMenu()
{
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromParent();
	}

	// Unpause game
	UGameplayStatics::SetGamePaused(GetGameInstance(), false);

	SetMenuState(EMGMenuState::InGame);

	// Return to game input
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetGameInstance(), 0))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
}

void UMGMenuSubsystem::ShowSettings(EMGSettingsCategory InitialCategory)
{
	if (!SettingsClass)
	{
		return;
	}

	if (!SettingsWidget)
	{
		SettingsWidget = CreateWidget<UMGSettingsWidget>(GetGameInstance(), SettingsClass);
	}

	if (SettingsWidget)
	{
		if (!SettingsWidget->IsInViewport())
		{
			SettingsWidget->AddToViewport(300);
		}
		SettingsWidget->SetInitialCategory(InitialCategory);
	}

	SetMenuState(EMGMenuState::Settings);
}

void UMGMenuSubsystem::HideSettings()
{
	if (SettingsWidget && SettingsWidget->IsInViewport())
	{
		SettingsWidget->RemoveFromParent();
	}

	// Return to previous state
	if (MainMenuWidget && MainMenuWidget->IsInViewport())
	{
		SetMenuState(EMGMenuState::MainMenu);
	}
	else if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		SetMenuState(EMGMenuState::Paused);
	}
	else
	{
		SetMenuState(EMGMenuState::InGame);
	}
}

void UMGMenuSubsystem::ShowLoadingScreen(const FText& LoadingText)
{
	if (!LoadingScreenClass)
	{
		return;
	}

	if (!LoadingScreenWidget)
	{
		LoadingScreenWidget = CreateWidget<UMGLoadingScreenWidget>(GetGameInstance(), LoadingScreenClass);
	}

	if (LoadingScreenWidget)
	{
		if (!LoadingScreenWidget->IsInViewport())
		{
			LoadingScreenWidget->AddToViewport(1000);
		}
		LoadingScreenWidget->SetLoadingText(LoadingText);
	}

	SetMenuState(EMGMenuState::Loading);
}

void UMGMenuSubsystem::HideLoadingScreen()
{
	if (LoadingScreenWidget && LoadingScreenWidget->IsInViewport())
	{
		LoadingScreenWidget->RemoveFromParent();
	}

	SetMenuState(EMGMenuState::InGame);
}

void UMGMenuSubsystem::UpdateLoadingProgress(float Progress)
{
	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->SetProgress(Progress);
	}

	OnLoadingProgress.Broadcast(Progress);
}

// ==========================================
// SETTINGS
// ==========================================

void UMGMenuSubsystem::ApplySettings(const FMGGameSettings& NewSettings)
{
	CurrentSettings = NewSettings;

	ApplyGraphicsSettings();
	ApplyAudioSettings();

	OnSettingsApplied.Broadcast();
}

void UMGMenuSubsystem::ApplyAndSaveSettings(const FMGGameSettings& NewSettings)
{
	ApplySettings(NewSettings);
	SaveSettings();
}

void UMGMenuSubsystem::ResetSettingsToDefaults()
{
	CurrentSettings = FMGGameSettings();
	ApplySettings(CurrentSettings);
}

void UMGMenuSubsystem::SaveSettings()
{
	FString SavePath = GetSettingsSavePath();

	// Serialize to JSON
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Graphics
	JsonObject->SetNumberField(TEXT("ResolutionIndex"), CurrentSettings.ResolutionIndex);
	JsonObject->SetNumberField(TEXT("FullscreenMode"), CurrentSettings.FullscreenMode);
	JsonObject->SetBoolField(TEXT("VSyncEnabled"), CurrentSettings.bVSyncEnabled);
	JsonObject->SetNumberField(TEXT("FrameRateLimit"), CurrentSettings.FrameRateLimit);
	JsonObject->SetNumberField(TEXT("GraphicsQuality"), CurrentSettings.GraphicsQuality);
	JsonObject->SetBoolField(TEXT("MotionBlurEnabled"), CurrentSettings.bMotionBlurEnabled);
	JsonObject->SetNumberField(TEXT("MotionBlurIntensity"), CurrentSettings.MotionBlurIntensity);

	// Audio
	JsonObject->SetNumberField(TEXT("MasterVolume"), CurrentSettings.MasterVolume);
	JsonObject->SetNumberField(TEXT("MusicVolume"), CurrentSettings.MusicVolume);
	JsonObject->SetNumberField(TEXT("SFXVolume"), CurrentSettings.SFXVolume);
	JsonObject->SetNumberField(TEXT("EngineVolume"), CurrentSettings.EngineVolume);

	// Controls
	JsonObject->SetNumberField(TEXT("SteeringSensitivity"), CurrentSettings.SteeringSensitivity);
	JsonObject->SetBoolField(TEXT("VibrationEnabled"), CurrentSettings.bVibrationEnabled);
	JsonObject->SetBoolField(TEXT("AutomaticTransmission"), CurrentSettings.bAutomaticTransmission);
	JsonObject->SetBoolField(TEXT("TractionControl"), CurrentSettings.bTractionControl);

	// Gameplay
	JsonObject->SetNumberField(TEXT("DefaultCamera"), CurrentSettings.DefaultCamera);
	JsonObject->SetBoolField(TEXT("ShowSpeedometer"), CurrentSettings.bShowSpeedometer);
	JsonObject->SetNumberField(TEXT("SpeedUnits"), CurrentSettings.SpeedUnits);
	JsonObject->SetBoolField(TEXT("ShowMinimap"), CurrentSettings.bShowMinimap);
	JsonObject->SetBoolField(TEXT("ShowRacingLine"), CurrentSettings.bShowRacingLine);

	// Accessibility
	JsonObject->SetNumberField(TEXT("ColorBlindMode"), CurrentSettings.ColorBlindMode);
	JsonObject->SetNumberField(TEXT("HUDScale"), CurrentSettings.HUDScale);
	JsonObject->SetBoolField(TEXT("ReduceScreenShake"), CurrentSettings.bReduceScreenShake);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(OutputString, *SavePath);
}

void UMGMenuSubsystem::LoadSettings()
{
	FString SavePath = GetSettingsSavePath();
	FString JsonString;

	if (!FFileHelper::LoadFileToString(JsonString, *SavePath))
	{
		// No saved settings, use defaults
		CurrentSettings = FMGGameSettings();
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		CurrentSettings = FMGGameSettings();
		return;
	}

	// Graphics
	CurrentSettings.ResolutionIndex = JsonObject->GetIntegerField(TEXT("ResolutionIndex"));
	CurrentSettings.FullscreenMode = JsonObject->GetIntegerField(TEXT("FullscreenMode"));
	CurrentSettings.bVSyncEnabled = JsonObject->GetBoolField(TEXT("VSyncEnabled"));
	CurrentSettings.FrameRateLimit = JsonObject->GetIntegerField(TEXT("FrameRateLimit"));
	CurrentSettings.GraphicsQuality = JsonObject->GetIntegerField(TEXT("GraphicsQuality"));
	CurrentSettings.bMotionBlurEnabled = JsonObject->GetBoolField(TEXT("MotionBlurEnabled"));
	CurrentSettings.MotionBlurIntensity = JsonObject->GetNumberField(TEXT("MotionBlurIntensity"));

	// Audio
	CurrentSettings.MasterVolume = JsonObject->GetNumberField(TEXT("MasterVolume"));
	CurrentSettings.MusicVolume = JsonObject->GetNumberField(TEXT("MusicVolume"));
	CurrentSettings.SFXVolume = JsonObject->GetNumberField(TEXT("SFXVolume"));
	CurrentSettings.EngineVolume = JsonObject->GetNumberField(TEXT("EngineVolume"));

	// Controls
	CurrentSettings.SteeringSensitivity = JsonObject->GetNumberField(TEXT("SteeringSensitivity"));
	CurrentSettings.bVibrationEnabled = JsonObject->GetBoolField(TEXT("VibrationEnabled"));
	CurrentSettings.bAutomaticTransmission = JsonObject->GetBoolField(TEXT("AutomaticTransmission"));
	CurrentSettings.bTractionControl = JsonObject->GetBoolField(TEXT("TractionControl"));

	// Gameplay
	CurrentSettings.DefaultCamera = JsonObject->GetIntegerField(TEXT("DefaultCamera"));
	CurrentSettings.bShowSpeedometer = JsonObject->GetBoolField(TEXT("ShowSpeedometer"));
	CurrentSettings.SpeedUnits = JsonObject->GetIntegerField(TEXT("SpeedUnits"));
	CurrentSettings.bShowMinimap = JsonObject->GetBoolField(TEXT("ShowMinimap"));
	CurrentSettings.bShowRacingLine = JsonObject->GetBoolField(TEXT("ShowRacingLine"));

	// Accessibility
	CurrentSettings.ColorBlindMode = JsonObject->GetIntegerField(TEXT("ColorBlindMode"));
	CurrentSettings.HUDScale = JsonObject->GetNumberField(TEXT("HUDScale"));
	CurrentSettings.bReduceScreenShake = JsonObject->GetBoolField(TEXT("ReduceScreenShake"));

	// Apply loaded settings
	ApplySettings(CurrentSettings);
}

// ==========================================
// NAVIGATION
// ==========================================

void UMGMenuSubsystem::StartGame()
{
	HideMainMenu();
	ShowLoadingScreen(FText::FromString(TEXT("Loading...")));

	// Would load the appropriate level here
	// UGameplayStatics::OpenLevel(...)
}

void UMGMenuSubsystem::ReturnToMainMenu()
{
	HidePauseMenu();
	ShowLoadingScreen(FText::FromString(TEXT("Returning to Main Menu...")));

	// Would load main menu level
	// UGameplayStatics::OpenLevel(...)
}

void UMGMenuSubsystem::QuitGame()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetGameInstance(), 0))
	{
		UKismetSystemLibrary::QuitGame(GetGameInstance(), PC, EQuitPreference::Quit, false);
	}
}

void UMGMenuSubsystem::ResumeGame()
{
	HidePauseMenu();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGMenuSubsystem::SetMenuState(EMGMenuState NewState)
{
	if (CurrentMenuState != NewState)
	{
		CurrentMenuState = NewState;
		OnMenuStateChanged.Broadcast(NewState);
	}
}

void UMGMenuSubsystem::ApplyGraphicsSettings()
{
	UGameUserSettings* UserSettings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!UserSettings)
	{
		return;
	}

	// Apply resolution
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	if (Resolutions.IsValidIndex(CurrentSettings.ResolutionIndex))
	{
		FIntPoint Resolution = Resolutions[CurrentSettings.ResolutionIndex];
		UserSettings->SetScreenResolution(Resolution);
	}

	// Fullscreen mode
	EWindowMode::Type WindowMode = EWindowMode::Fullscreen;
	switch (CurrentSettings.FullscreenMode)
	{
		case 0: WindowMode = EWindowMode::Windowed; break;
		case 1: WindowMode = EWindowMode::WindowedFullscreen; break;
		case 2: WindowMode = EWindowMode::Fullscreen; break;
	}
	UserSettings->SetFullscreenMode(WindowMode);

	// VSync
	UserSettings->SetVSyncEnabled(CurrentSettings.bVSyncEnabled);

	// Frame rate
	UserSettings->SetFrameRateLimit(CurrentSettings.FrameRateLimit);

	// Apply settings
	UserSettings->ApplySettings(false);
}

void UMGMenuSubsystem::ApplyAudioSettings()
{
	// Would set audio mix levels here
	// This would integrate with the Audio System we created earlier
}

FString UMGMenuSubsystem::GetSettingsSavePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("Settings.json");
}
