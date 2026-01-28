// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGMenuWidgets.h
 * @brief Collection of core menu widgets (Main Menu, Pause, Settings, Results)
 *
 * =============================================================================
 * @section Overview
 * This file defines the primary menu widgets used throughout the game's UI flow.
 * These widgets handle navigation between game modes, settings management, and
 * post-race results display. All widgets follow the game's Y2K neon aesthetic
 * and support both gamepad and mouse/keyboard navigation.
 *
 * Widgets included in this file:
 * - **UMGMainMenuWidget**: The game's entry point UI with play, garage, settings
 * - **UMGPauseMenuWidget**: In-race pause menu for resume/restart/quit
 * - **UMGSettingsWidget**: Multi-tab settings interface (graphics, audio, controls)
 * - **UMGResultsWidget**: Post-race results display with stats and rewards
 * - **UMGNotificationWidget**: Pop-up notifications for achievements/events
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Menu Subsystem Integration**: All menu widgets communicate through
 *   UMGMenuSubsystem to manage game state, handle transitions, and persist
 *   settings. Never modify game state directly from widgets.
 *
 * - **Abstract/Blueprintable Pattern**: These base classes define the core
 *   functionality in C++, while Blueprint subclasses handle visual layout
 *   and animations. This separates logic from presentation.
 *
 * - **BlueprintImplementableEvent**: Functions marked with this specifier are
 *   "hooks" for Blueprint to add custom animations, sounds, and visual effects
 *   without modifying C++ code.
 *
 * - **Settings Flow**: Settings changes are staged in PendingSettings until
 *   explicitly applied. This allows preview and revert functionality.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Game State]
 *        |
 *        v
 *   [MGMenuSubsystem] <----> [Menu Widgets]
 *        |                        |
 *        |                        +-- UMGMainMenuWidget
 *        |                        +-- UMGPauseMenuWidget
 *        |                        +-- UMGSettingsWidget
 *        |                        +-- UMGResultsWidget
 *        |                        +-- UMGNotificationWidget
 *        v
 *   [Level Transitions / Settings Persistence]
 *
 * =============================================================================
 * @section Usage
 *
 * @subsection MainMenuUsage Main Menu
 * @code
 * // Main menu is typically created by the game mode
 * UMGMainMenuWidget* MainMenu = CreateWidget<UMGMainMenuWidget>(
 *     GetWorld(), MainMenuClass);
 * MainMenu->AddToViewport();
 *
 * // Button handlers are connected in Blueprint or via delegates
 * // The widget calls MenuSubsystem methods to trigger transitions:
 * // - OnQuickPlayClicked() -> MenuSubsystem->StartQuickPlay()
 * // - OnGarageClicked() -> MenuSubsystem->OpenGarage()
 * @endcode
 *
 * @subsection PauseMenuUsage Pause Menu
 * @code
 * // Pause menu is shown when player pauses during a race
 * void AMyController::TogglePause()
 * {
 *     if (IsPaused())
 *     {
 *         PauseWidget->OnResumeClicked();
 *     }
 *     else
 *     {
 *         PauseWidget = CreateWidget<UMGPauseMenuWidget>(GetWorld(), PauseClass);
 *         PauseWidget->AddToViewport();
 *         SetPause(true);
 *     }
 * }
 * @endcode
 *
 * @subsection SettingsUsage Settings Widget
 * @code
 * // Create settings widget, optionally jumping to a specific category
 * UMGSettingsWidget* Settings = CreateWidget<UMGSettingsWidget>(
 *     GetWorld(), SettingsClass);
 * Settings->SetInitialCategory(EMGSettingsCategory::Audio);
 * Settings->AddToViewport();
 *
 * // Settings flow: Edit -> Preview -> Apply or Revert
 * // Changes are staged in PendingSettings until ApplySettings() is called
 * // RevertSettings() restores to last saved values
 * // ResetToDefaults() clears all customizations
 * @endcode
 *
 * @subsection ResultsUsage Results Widget
 * @code
 * // Show race results after finishing
 * UMGResultsWidget* Results = CreateWidget<UMGResultsWidget>(
 *     GetWorld(), ResultsClass);
 * Results->ShowResults(
 *     Position,        // e.g., 1 for first place
 *     TotalTime,       // Race completion time in seconds
 *     BestLapTime,     // Best lap time in seconds
 *     CashEarned,      // Currency reward
 *     ReputationEarned // XP/reputation reward
 * );
 * Results->AddToViewport();
 * @endcode
 *
 * @subsection NotificationUsage Notification Widget
 * @code
 * // Show a notification (achievement, position change, etc.)
 * NotificationWidget->ShowNotification(
 *     FText::FromString("Achievement Unlocked!"),
 *     FText::FromString("Complete your first race"),
 *     AchievementIcon
 * );
 *
 * // Auto-hides after duration, or hide manually:
 * NotificationWidget->HideNotification();
 * @endcode
 *
 * =============================================================================
 * @section UnrealMacros Unreal Engine Macros Explained
 *
 * - UCLASS(Abstract, Blueprintable): Cannot instantiate this C++ class directly;
 *   must create a Blueprint subclass. The Blueprint handles visual layout.
 *
 * - UFUNCTION(BlueprintImplementableEvent): A function with no C++ body that
 *   Blueprints can implement. If not implemented, calling it does nothing.
 *
 * - UPROPERTY(BlueprintReadOnly): Exposes the variable to Blueprint for reading
 *   but prevents modification from Blueprint graphs.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/MGMenuSubsystem.h"
#include "MGMenuWidgets.generated.h"

class UMGMenuSubsystem;
class UButton;
class UTextBlock;
class UImage;
class UProgressBar;
class UWidgetSwitcher;

/**
 * Main Menu Widget
 * Entry point for the game
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// ==========================================
	// ACTIONS
	// ==========================================

	/** Quick play button */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnQuickPlayClicked();

	/** Garage button */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnGarageClicked();

	/** Multiplayer button */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnMultiplayerClicked();

	/** Settings button */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnSettingsClicked();

	/** Quit button */
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnQuitClicked();

protected:
	/** Menu subsystem reference */
	UPROPERTY(BlueprintReadOnly, Category = "MainMenu")
	UMGMenuSubsystem* MenuSubsystem;

	/** Called when a menu item is selected */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void OnMenuItemSelected(int32 ItemIndex);

	/** Play menu animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void PlayIntroAnimation();

	/** Play exit animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void PlayExitAnimation();
};

/**
 * Pause Menu Widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// ==========================================
	// ACTIONS
	// ==========================================

	/** Resume game */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnResumeClicked();

	/** Restart race */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnRestartClicked();

	/** Settings */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnSettingsClicked();

	/** Return to main menu */
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnMainMenuClicked();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "PauseMenu")
	UMGMenuSubsystem* MenuSubsystem;

	/** Confirm dialog for exit */
	UFUNCTION(BlueprintImplementableEvent, Category = "PauseMenu")
	void ShowExitConfirmation();
};

/**
 * Settings Widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Set initial category */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetInitialCategory(EMGSettingsCategory Category);

	// ==========================================
	// CATEGORY NAVIGATION
	// ==========================================

	/** Switch to graphics tab */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ShowGraphicsSettings();

	/** Switch to audio tab */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ShowAudioSettings();

	/** Switch to controls tab */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ShowControlsSettings();

	/** Switch to gameplay tab */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ShowGameplaySettings();

	/** Switch to accessibility tab */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ShowAccessibilitySettings();

	// ==========================================
	// SETTINGS ACTIONS
	// ==========================================

	/** Apply current settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySettings();

	/** Revert to saved settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RevertSettings();

	/** Reset to defaults */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetToDefaults();

	/** Close settings */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void CloseSettings();

	/** Get pending settings */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FMGGameSettings GetPendingSettings() const { return PendingSettings; }

	/** Update pending setting value */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void UpdatePendingSettings(const FMGGameSettings& NewSettings);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	UMGMenuSubsystem* MenuSubsystem;

	/** Settings being edited (not yet applied) */
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	FMGGameSettings PendingSettings;

	/** Original settings (for revert) */
	FMGGameSettings OriginalSettings;

	/** Current category */
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
	EMGSettingsCategory CurrentCategory;

	/** Called when category changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void OnCategoryChanged(EMGSettingsCategory NewCategory);

	/** Called when settings are modified */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void OnSettingsModified();

	/** Show unsaved changes dialog */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void ShowUnsavedChangesDialog();
};

// NOTE: UMGLoadingScreenWidget moved to MGLoadingScreenWidget.h

/**
 * Results Screen Widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGResultsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Show race results */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void ShowResults(int32 Position, float TotalTime, float BestLapTime, int32 CashEarned, int32 ReputationEarned);

	/** Continue to next screen */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void OnContinueClicked();

	/** View replay */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void OnReplayClicked();

	/** Rematch */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void OnRematchClicked();

protected:
	/** Results data */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 FinalPosition = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float FinalTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float BestLap = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 Cash = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 Reputation = 0;

	/** Is personal best */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bIsPersonalBest = false;

	/** Called to animate results */
	UFUNCTION(BlueprintImplementableEvent, Category = "Results")
	void PlayResultsAnimation();
};

/**
 * Notification Widget
 * For in-game notifications and achievements
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGNotificationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Show notification */
	UFUNCTION(BlueprintCallable, Category = "Notification")
	void ShowNotification(const FText& Title, const FText& Message, UTexture2D* Icon = nullptr);

	/** Hide notification */
	UFUNCTION(BlueprintCallable, Category = "Notification")
	void HideNotification();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Notification")
	FText NotificationTitle;

	UPROPERTY(BlueprintReadOnly, Category = "Notification")
	FText NotificationMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Notification")
	UTexture2D* NotificationIcon;

	/** Play show animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Notification")
	void PlayShowAnimation();

	/** Play hide animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Notification")
	void PlayHideAnimation();
};
