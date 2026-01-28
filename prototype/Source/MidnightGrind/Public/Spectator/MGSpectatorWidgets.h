// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSpectatorWidgets.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file contains all the UI widgets (visual elements) for spectator mode.
 * These widgets display information like race standings, driver info, lap times,
 * and camera controls while watching a race.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. WHAT IS A WIDGET?
 *    - A Widget is a UI element (text, button, image, panel, etc.)
 *    - UUserWidget is the base class for custom UI in Unreal's UMG system
 *    - Widgets are designed in the Widget Blueprint editor (visual designer)
 *    - The C++ class provides logic, the Blueprint provides visual layout
 *
 * 2. ABSTRACT vs BLUEPRINTABLE:
 *    - "Abstract" means you can't create instances directly - you MUST create
 *      a Blueprint subclass
 *    - "Blueprintable" allows creation of Blueprint subclasses
 *    - Together they mean: "Make a Blueprint from this, don't use the C++ directly"
 *    - This lets designers customize visuals without touching code
 *
 * 3. BINDWIDGET AND BINDWIDGETOPTIONAL:
 *    - meta = (BindWidget) links a C++ variable to a widget in the Blueprint
 *    - When you name a widget "SpeedText" in the Blueprint, and have a
 *      UTextBlock* SpeedText with BindWidget, they're automatically connected
 *    - "Optional" means it won't crash if the widget doesn't exist
 *
 * 4. BLUEPRINTNATIVEEVENT:
 *    - A function that has C++ implementation but CAN be overridden in Blueprint
 *    - The C++ version is named FunctionName_Implementation
 *    - Allows designers to customize behavior without touching C++
 *
 * 5. WIDGET LIFECYCLE:
 *    - NativeConstruct() - Called when widget is created and added to viewport
 *    - NativeTick() - Called every frame (use sparingly for performance)
 *    - NativeDestruct() - Called when widget is removed/destroyed
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Player Controller]
 *           |
 *           | creates/manages
 *           v
 *    [HUD Widget] (UMGSpectatorHUDWidget)
 *           |
 *           | contains child widgets
 *           v
 *    [Target Info] [Standings] [Controls] [Speedometer] [Lap Timer]
 *           |
 *           | listens to events from
 *           v
 *    [MGSpectatorSubsystem] --> OnTargetChanged, OnCameraModeChanged, etc.
 *
 * TYPICAL WORKFLOW FOR DESIGNERS:
 * 1. Create a Widget Blueprint that inherits from one of these classes
 * 2. Design the visual layout in the Widget Blueprint editor
 * 3. Name widgets to match the BindWidget variables (e.g., "SpeedText")
 * 4. Optionally override BlueprintNativeEvent functions for custom behavior
 * 5. The HUD widget class then creates and manages these child widgets
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGSpectatorSubsystem.h"
#include "MGSpectatorWidgets.generated.h"

// Forward declarations - Common UMG widget types we reference
class UTextBlock;              // Displays text
class UImage;                  // Displays images/icons
class UProgressBar;            // Displays progress (like a speed gauge)
class UVerticalBox;            // Container that stacks children vertically
class UHorizontalBox;          // Container that stacks children horizontally
class UButton;                 // Clickable button
class UMGSpectatorSubsystem;   // The spectator manager we get data from

/**
 * UMGSpectatorHUDWidget - The main container widget for all spectator UI.
 *
 * This is the "root" widget that contains all other spectator UI elements.
 * Think of it as the TV broadcast overlay you see during a race:
 * - Driver name and position
 * - Race standings on the side
 * - Lap times
 * - Camera mode indicator
 *
 * ABSTRACT CLASS NOTE:
 * You cannot use this class directly. Create a Widget Blueprint that
 * inherits from this class, then design your UI layout there.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Update all displays */
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void UpdateDisplay();

	/** Set overlay visibility */
	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void SetOverlayVisibility(bool bVisible);

protected:
	// ==========================================
	// WIDGETS
	// ==========================================

	/** Target info panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UUserWidget* TargetInfoPanel;

	/** Standings panel */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UUserWidget* StandingsPanel;

	/** Camera info text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* CameraInfoText;

	/** Controls hint text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* ControlsHintText;

	/** Mini-map widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UUserWidget* MiniMapWidget;

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	/** Handle target changed */
	UFUNCTION()
	void OnTargetChanged(const FMGSpectatorTarget& NewTarget);

	/** Handle camera mode changed */
	UFUNCTION()
	void OnCameraModeChanged(EMGSpectatorCameraMode NewMode);

	/** Update target info display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateTargetInfoDisplay();

	/** Update standings display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateStandingsDisplay();

	/** Update camera info display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateCameraInfoDisplay();
};

/**
 * UMGSpectatorTargetInfoWidget - Displays detailed info about the currently watched driver.
 *
 * This widget shows who you're currently spectating:
 * - Driver/player name
 * - Current race position (1st, 2nd, etc.)
 * - Current speed
 * - Lap number
 * - Best lap time
 * - Team color indicator
 * - AI badge (if watching an AI driver)
 *
 * DESIGN TIP:
 * Position this prominently at the bottom of the screen (like TV broadcasts)
 * so viewers always know who they're watching.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorTargetInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Set target data */
	UFUNCTION(BlueprintCallable, Category = "Target")
	void SetTargetData(const FMGSpectatorTarget& Target);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGSpectatorTarget TargetData;

	/** Driver name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* DriverNameText;

	/** Position text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* PositionText;

	/** Speed text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* SpeedText;

	/** Lap text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* LapText;

	/** Best lap time text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* BestLapText;

	/** Team color indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* TeamColorImage;

	/** AI indicator */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* AIIndicator;

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();
};

/**
 * UMGSpectatorStandingsWidget - Shows the complete race standings/leaderboard.
 *
 * Displays a list of all drivers in order of their race position.
 * Like the "tower" graphic on the side of TV broadcasts showing P1, P2, P3, etc.
 *
 * HOW IT WORKS:
 * - Creates a list of StandingsEntry widgets (one per driver)
 * - Updates positions in real-time as the race progresses
 * - Highlights the currently spectated driver
 * - Clicking an entry can switch to spectating that driver
 *
 * PERFORMANCE NOTE:
 * The widget pool (StandingsEntryWidgets) reuses widgets instead of
 * creating/destroying them every update. This is more efficient.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorStandingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Refresh standings */
	UFUNCTION(BlueprintCallable, Category = "Standings")
	void RefreshStandings();

protected:
	/** Standings entry widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UUserWidget> StandingsEntryWidgetClass;

	/** Standings container */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UVerticalBox* StandingsContainer;

	/** Current standings entries */
	UPROPERTY()
	TArray<UUserWidget*> StandingsEntryWidgets;

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay(const TArray<FMGSpectatorTarget>& Targets);

	/** Handle target changed (highlight current) */
	UFUNCTION()
	void OnTargetChanged(const FMGSpectatorTarget& NewTarget);
};

/**
 * UMGSpectatorStandingsEntryWidget - A single row in the standings list.
 *
 * Each entry represents one driver showing:
 * - Position number (1st, 2nd, etc.)
 * - Driver name
 * - Gap to leader (e.g., "+3.5s")
 * - Team color bar
 *
 * INTERACTION:
 * Clicking this entry broadcasts OnClicked, which the parent standings
 * widget uses to switch spectator target to this driver.
 *
 * HIGHLIGHTING:
 * The currently spectated driver's entry is visually highlighted
 * so viewers can easily spot them in the standings.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorStandingsEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntryClicked, const FMGSpectatorTarget&, Target);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEntryClicked OnClicked;

	/** Set target data */
	UFUNCTION(BlueprintCallable, Category = "Entry")
	void SetTargetData(const FMGSpectatorTarget& Target);

	/** Set highlighted */
	UFUNCTION(BlueprintCallable, Category = "Entry")
	void SetHighlighted(bool bHighlight);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGSpectatorTarget TargetData;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsHighlighted = false;

	/** Position text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* PositionText;

	/** Name text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* NameText;

	/** Gap text (to leader) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* GapText;

	/** Team color bar */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* TeamColorBar;

	/** Background image */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UImage* BackgroundImage;

	/** Click button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* ClickButton;

	virtual void NativeConstruct() override;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Handle click */
	UFUNCTION()
	void HandleClick();
};

/**
 * UMGSpectatorControlsWidget - UI controls for spectator camera and targets.
 *
 * Provides interactive buttons for:
 * - Switching camera modes (Free, Chase, Orbit, etc.)
 * - Cycling through targets (previous/next driver)
 * - Toggling auto-director mode
 * - Exiting spectator mode
 *
 * CONTROLLER/KEYBOARD SUPPORT:
 * While this provides on-screen buttons, most users will use keyboard
 * shortcuts or gamepad buttons. This widget is more for mouse/touch users
 * or as visual feedback for what controls do.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorControlsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/** Camera mode buttons */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UHorizontalBox* CameraModeButtons;

	/** Previous target button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* PrevTargetButton;

	/** Next target button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* NextTargetButton;

	/** Auto-director toggle button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* AutoDirectorButton;

	/** Exit spectator button */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UButton* ExitButton;

	/** Current camera mode text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* CameraModeText;

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	/** Handle prev target click */
	UFUNCTION()
	void OnPrevTargetClicked();

	/** Handle next target click */
	UFUNCTION()
	void OnNextTargetClicked();

	/** Handle auto-director click */
	UFUNCTION()
	void OnAutoDirectorClicked();

	/** Handle exit click */
	UFUNCTION()
	void OnExitClicked();

	/** Handle camera mode changed */
	UFUNCTION()
	void OnCameraModeChanged(EMGSpectatorCameraMode NewMode);

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();
};

/**
 * UMGSpectatorSpeedometerWidget - Displays the current speed of the spectated vehicle.
 *
 * Shows speed in a visually appealing format:
 * - Large numeric speed value
 * - Unit label (KM/H or MPH)
 * - Optional speed bar/gauge for visual representation
 *
 * SMOOTH DISPLAY:
 * Uses DisplaySpeed vs CurrentSpeed for smooth animation - the display
 * value interpolates toward the actual speed rather than jumping instantly.
 * This looks more natural and prevents flickering numbers.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorSpeedometerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Set speed value */
	UFUNCTION(BlueprintCallable, Category = "Speedometer")
	void SetSpeed(float SpeedKMH);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float CurrentSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float DisplaySpeed = 0.0f;

	/** Speed text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* SpeedText;

	/** Unit text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* UnitText;

	/** Speed needle/bar */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UProgressBar* SpeedBar;

	/** Max speed for display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaxDisplaySpeed = 400.0f;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();
};

/**
 * UMGSpectatorLapTimerWidget - Displays lap timing information.
 *
 * Shows:
 * - Current lap number out of total (e.g., "Lap 3/5")
 * - Current lap elapsed time (counting up)
 * - Best lap time (personal best for this driver)
 * - Delta time (how current lap compares to best - green if faster, red if slower)
 *
 * TIME FORMATTING:
 * Use the static FormatLapTime() helper to convert seconds to MM:SS.mmm format.
 * Example: 93.456 seconds becomes "1:33.456"
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSpectatorLapTimerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Set lap data */
	UFUNCTION(BlueprintCallable, Category = "Lap Timer")
	void SetLapData(int32 CurrentLap, int32 TotalLaps, float CurrentLapTime, float BestLapTime);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 CurrentLap = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 TotalLaps = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float CurrentLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float BestLapTime = 0.0f;

	/** Current lap text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* CurrentLapText;

	/** Current time text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* CurrentTimeText;

	/** Best time text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* BestTimeText;

	/** Delta time text (vs best) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* DeltaTimeText;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Format time as string */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatLapTime(float TimeSeconds);
};

/**
 * UMGCameraCutIndicatorWidget - Visual feedback when camera switches.
 *
 * When the auto-director (or player) switches cameras, this widget briefly
 * appears showing what camera is now active. Like a TV broadcast showing
 * "CAMERA 3" or "HELICOPTER CAM" overlay.
 *
 * ANIMATION:
 * Override PlayShowAnimation/PlayHideAnimation in your Blueprint to create
 * smooth fade in/out effects. The widget auto-hides after DisplayDuration seconds.
 *
 * USAGE:
 * The widget automatically subscribes to OnCameraCut events from the
 * SpectatorSubsystem - you don't need to manually trigger it.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGCameraCutIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Show cut indicator */
	UFUNCTION(BlueprintCallable, Category = "Indicator")
	void ShowCutIndicator(const FMGCameraCut& CutInfo);

protected:
	/** Cut info text */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UTextBlock* CutInfoText;

	/** Display duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DisplayDuration = 2.0f;

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	FTimerHandle HideTimerHandle;

	/** Handle camera cut */
	UFUNCTION()
	void OnCameraCut(const FMGCameraCut& CutInfo);

	/** Hide indicator */
	void HideIndicator();

	/** Play show animation */
	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
	void PlayShowAnimation();

	/** Play hide animation */
	UFUNCTION(BlueprintNativeEvent, Category = "Animation")
	void PlayHideAnimation();
};
