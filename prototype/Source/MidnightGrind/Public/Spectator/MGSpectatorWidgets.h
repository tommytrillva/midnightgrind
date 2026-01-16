// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGSpectatorSubsystem.h"
#include "MGSpectatorWidgets.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UVerticalBox;
class UHorizontalBox;
class UButton;
class UMGSpectatorSubsystem;

/**
 * Spectator HUD Widget
 * Main overlay for spectator mode
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
 * Target Info Widget
 * Displays current spectator target information
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
 * Race Standings Widget
 * Shows all driver positions
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
 * Standings Entry Widget
 * Single entry in standings list
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
 * Camera Controls Widget
 * UI for changing camera modes and targets
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
 * Speedometer Widget
 * Shows current target speed
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
 * Lap Timer Widget
 * Shows current and best lap times
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
 * Camera Cut Indicator Widget
 * Shows when camera cuts happen
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
