// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Vehicle/MGVehiclePawn.h"
#include "MGRacingHUD.generated.h"

class AMGVehiclePawn;
class UTextBlock;
class UImage;
class UProgressBar;
class UCanvasPanel;

/**
 * Speedometer style options
 */
UENUM(BlueprintType)
enum class EMGSpeedometerStyle : uint8
{
	Digital		UMETA(DisplayName = "Digital"),
	Analog		UMETA(DisplayName = "Analog Gauge"),
	Bar			UMETA(DisplayName = "Bar Graph"),
	Minimal		UMETA(DisplayName = "Minimal")
};

/**
 * Configuration for HUD layout
 */
USTRUCT(BlueprintType)
struct FMGHUDConfig
{
	GENERATED_BODY()

	/** Speedometer style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	EMGSpeedometerStyle SpeedometerStyle = EMGSpeedometerStyle::Digital;

	/** Show tachometer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowTachometer = true;

	/** Show gear indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowGear = true;

	/** Show boost/turbo gauge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowBoostGauge = true;

	/** Show nitrous bar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowNitrousBar = true;

	/** Show lap counter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowLapCounter = true;

	/** Show position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowPosition = true;

	/** Show lap timer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowLapTimer = true;

	/** Show drift score */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowDriftScore = true;

	/** Show minimap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bShowMinimap = true;

	/** Use MPH (false = KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	bool bUseMPH = true;

	/** HUD scale multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float HUDScale = 1.0f;

	/** HUD opacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HUDOpacity = 1.0f;
};

/**
 * Main racing HUD widget - displays vehicle state during gameplay
 * Designed with PS1/PS2 era aesthetic - bold, chunky, readable
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGRacingHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UMGRacingHUD(const FObjectInitializer& ObjectInitializer);

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Set the vehicle to display data for */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetVehicle(AMGVehiclePawn* Vehicle);

	/** Get the current vehicle */
	UFUNCTION(BlueprintPure, Category = "HUD")
	AMGVehiclePawn* GetVehicle() const { return TargetVehicle.Get(); }

	/** Apply HUD configuration */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ApplyConfig(const FMGHUDConfig& Config);

	/** Get current configuration */
	UFUNCTION(BlueprintPure, Category = "HUD")
	FMGHUDConfig GetConfig() const { return HUDConfig; }

	// ==========================================
	// DATA ACCESS (for Blueprint binding)
	// ==========================================

	/** Get current speed value */
	UFUNCTION(BlueprintPure, Category = "HUD|Speed")
	float GetCurrentSpeed() const;

	/** Get current speed as formatted text */
	UFUNCTION(BlueprintPure, Category = "HUD|Speed")
	FText GetSpeedText() const;

	/** Get speed unit text (MPH/KPH) */
	UFUNCTION(BlueprintPure, Category = "HUD|Speed")
	FText GetSpeedUnitText() const;

	/** Get current RPM (0-1 normalized) */
	UFUNCTION(BlueprintPure, Category = "HUD|Engine")
	float GetRPMPercent() const;

	/** Get current RPM as text */
	UFUNCTION(BlueprintPure, Category = "HUD|Engine")
	FText GetRPMText() const;

	/** Get current gear text */
	UFUNCTION(BlueprintPure, Category = "HUD|Engine")
	FText GetGearText() const;

	/** Get current gear as number (for gear indicator) */
	UFUNCTION(BlueprintPure, Category = "HUD|Engine")
	int32 GetCurrentGear() const;

	/** Is rev limiter active? (for flash effect) */
	UFUNCTION(BlueprintPure, Category = "HUD|Engine")
	bool IsRevLimiterActive() const;

	/** Get boost pressure (0-1 normalized) */
	UFUNCTION(BlueprintPure, Category = "HUD|Boost")
	float GetBoostPercent() const;

	/** Get boost PSI text */
	UFUNCTION(BlueprintPure, Category = "HUD|Boost")
	FText GetBoostText() const;

	/** Get nitrous remaining (0-1) */
	UFUNCTION(BlueprintPure, Category = "HUD|Nitrous")
	float GetNitrousPercent() const;

	/** Is nitrous currently active? */
	UFUNCTION(BlueprintPure, Category = "HUD|Nitrous")
	bool IsNitrousActive() const;

	// ==========================================
	// RACE DATA
	// ==========================================

	/** Get current lap number */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	int32 GetCurrentLap() const;

	/** Get total lap count */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	int32 GetTotalLaps() const;

	/** Get lap text (e.g., "2/5") */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	FText GetLapText() const;

	/** Get race position */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	int32 GetRacePosition() const;

	/** Get position text with suffix (1st, 2nd, etc.) */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	FText GetPositionText() const;

	/** Get current lap time as formatted text */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	FText GetLapTimeText() const;

	/** Get best lap time as formatted text */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	FText GetBestLapTimeText() const;

	/** Get total race time as formatted text */
	UFUNCTION(BlueprintPure, Category = "HUD|Race")
	FText GetTotalTimeText() const;

	// ==========================================
	// DRIFT DATA
	// ==========================================

	/** Is currently drifting? */
	UFUNCTION(BlueprintPure, Category = "HUD|Drift")
	bool IsDrifting() const;

	/** Get drift angle in degrees */
	UFUNCTION(BlueprintPure, Category = "HUD|Drift")
	float GetDriftAngle() const;

	/** Get current drift score */
	UFUNCTION(BlueprintPure, Category = "HUD|Drift")
	float GetDriftScore() const;

	/** Get drift score as formatted text */
	UFUNCTION(BlueprintPure, Category = "HUD|Drift")
	FText GetDriftScoreText() const;

	/** Get drift multiplier */
	UFUNCTION(BlueprintPure, Category = "HUD|Drift")
	float GetDriftMultiplier() const;

	// ==========================================
	// VISUAL HELPERS
	// ==========================================

	/** Get tachometer needle rotation (0-270 degrees typical) */
	UFUNCTION(BlueprintPure, Category = "HUD|Visual")
	float GetTachNeedleRotation() const;

	/** Get speedometer needle rotation */
	UFUNCTION(BlueprintPure, Category = "HUD|Visual")
	float GetSpeedNeedleRotation() const;

	/** Get RPM zone color (green/yellow/red) */
	UFUNCTION(BlueprintPure, Category = "HUD|Visual")
	FLinearColor GetRPMZoneColor() const;

	/** Get gear color based on optimal shift point */
	UFUNCTION(BlueprintPure, Category = "HUD|Visual")
	FLinearColor GetGearColor() const;

	/** Should show shift indicator? */
	UFUNCTION(BlueprintPure, Category = "HUD|Visual")
	bool ShouldShowShiftIndicator() const;

	/** Format time as MM:SS.mmm */
	UFUNCTION(BlueprintPure, Category = "HUD|Utility")
	static FText FormatLapTime(float TimeInSeconds);

	/** Get ordinal suffix (st, nd, rd, th) */
	UFUNCTION(BlueprintPure, Category = "HUD|Utility")
	static FText GetOrdinalSuffix(int32 Number);

	// ==========================================
	// EVENTS/ANIMATIONS
	// ==========================================

	/** Trigger lap complete animation */
	UFUNCTION(BlueprintCallable, Category = "HUD|Animation")
	void TriggerLapCompleteAnimation(int32 LapNumber, float LapTime, bool bNewBest);

	/** Trigger checkpoint animation */
	UFUNCTION(BlueprintCallable, Category = "HUD|Animation")
	void TriggerCheckpointAnimation(float SplitTime, bool bAhead);

	/** Trigger drift score popup */
	UFUNCTION(BlueprintCallable, Category = "HUD|Animation")
	void TriggerDriftScorePopup(float Score);

	/** Trigger nitrous flash */
	UFUNCTION(BlueprintCallable, Category = "HUD|Animation")
	void TriggerNitrousFlash();

	/** Trigger countdown display */
	UFUNCTION(BlueprintCallable, Category = "HUD|Animation")
	void ShowCountdown(int32 CountdownValue);

	/** Trigger GO! display */
	UFUNCTION(BlueprintCallable, Category = "HUD|Animation")
	void ShowGoSignal();

protected:
	// ==========================================
	// BLUEPRINT IMPLEMENTABLE EVENTS
	// ==========================================

	/** Called when vehicle state updates (for custom animations) */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnVehicleStateUpdated();

	/** Called when lap completes */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnLapComplete(int32 LapNumber, float LapTime, bool bNewBest);

	/** Called when checkpoint passed */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnCheckpointPassed(float SplitTime, bool bAhead);

	/** Called when drift score awarded */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnDriftScoreAwarded(float Score);

	/** Called when nitrous activates */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnNitrousActivate();

	/** Called when nitrous depletes */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnNitrousDeplete();

	/** Called to update countdown */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnCountdownTick(int32 Value);

	/** Called when race starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|Events")
	void OnRaceStart();

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** HUD configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FMGHUDConfig HUDConfig;

	/** Total laps in current race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 TotalLapCount = 3;

	/** Max speed for speedometer (affects needle range) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxDisplaySpeed = 200.0f;

	/** Max RPM for tachometer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxDisplayRPM = 9000.0f;

	/** Redline RPM (for red zone) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float RedlineRPM = 7500.0f;

	/** Optimal shift point (for shift indicator) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float OptimalShiftRPM = 7000.0f;

	// ==========================================
	// COLORS (PS1/PS2 style - bold and saturated)
	// ==========================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor SpeedColor = FLinearColor(0.0f, 1.0f, 0.8f, 1.0f); // Cyan

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor RPMNormalColor = FLinearColor(0.0f, 1.0f, 0.3f, 1.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor RPMWarningColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Yellow

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor RPMRedlineColor = FLinearColor(1.0f, 0.0f, 0.2f, 1.0f); // Red

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor GearColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ShiftColor = FLinearColor(1.0f, 0.0f, 0.5f, 1.0f); // Hot pink

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor BoostColor = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f); // Blue

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NitrousColor = FLinearColor(1.0f, 0.0f, 0.8f, 1.0f); // Magenta

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor DriftColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor PositiveColor = FLinearColor(0.0f, 1.0f, 0.3f, 1.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor NegativeColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); // Red

private:
	/** Cached vehicle reference */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> TargetVehicle;

	/** Cached runtime state */
	FMGVehicleRuntimeState CachedState;

	/** Previous nitrous state for activation detection */
	bool bWasNitrousActive = false;

	/** Update cached state from vehicle */
	void UpdateCachedState();

	/** Bind to vehicle events */
	void BindVehicleEvents();
};
