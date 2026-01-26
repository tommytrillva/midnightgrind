// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGRaceHUDSubsystem.generated.h"

class UMGRaceHUDWidget;
class UMGMinimapWidget;
class UMGRaceOverlayWidget;

/**
 * HUD display mode
 */
UENUM(BlueprintType)
enum class EMGHUDMode : uint8
{
	/** Full HUD visible */
	Full,
	/** Minimal HUD (speed, position only) */
	Minimal,
	/** Cinematic mode (no HUD) */
	Hidden,
	/** Photo mode HUD */
	PhotoMode,
	/** Replay mode HUD */
	Replay
};

/**
 * Vehicle telemetry data for HUD display
 */
USTRUCT(BlueprintType)
struct FMGVehicleTelemetry
{
	GENERATED_BODY()

	/** Current speed in KPH */
	UPROPERTY(BlueprintReadWrite)
	float SpeedKPH = 0.0f;

	/** Current speed in MPH */
	UPROPERTY(BlueprintReadWrite)
	float SpeedMPH = 0.0f;

	/** Engine RPM */
	UPROPERTY(BlueprintReadWrite)
	float RPM = 0.0f;

	/** Max RPM */
	UPROPERTY(BlueprintReadWrite)
	float MaxRPM = 8000.0f;

	/** Current gear (0 = reverse, 1 = first, etc.) */
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentGear = 1;

	/** Total gears (excluding reverse) */
	UPROPERTY(BlueprintReadWrite)
	int32 TotalGears = 6;

	/** NOS amount (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float NOSAmount = 1.0f;

	/** Is NOS active */
	UPROPERTY(BlueprintReadWrite)
	bool bNOSActive = false;

	/** Throttle position (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float ThrottlePosition = 0.0f;

	/** Brake position (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float BrakePosition = 0.0f;

	/** Steering angle (-1 to 1) */
	UPROPERTY(BlueprintReadWrite)
	float SteeringAngle = 0.0f;

	/** Is drifting */
	UPROPERTY(BlueprintReadWrite)
	bool bIsDrifting = false;

	/** Drift angle in degrees */
	UPROPERTY(BlueprintReadWrite)
	float DriftAngle = 0.0f;
};

/**
 * Race status data for HUD display
 */
USTRUCT(BlueprintType)
struct FMGRaceStatus
{
	GENERATED_BODY()

	/** Current position (1 = first) */
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentPosition = 1;

	/** Total racers */
	UPROPERTY(BlueprintReadWrite)
	int32 TotalRacers = 8;

	/** Current lap */
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentLap = 1;

	/** Total laps */
	UPROPERTY(BlueprintReadWrite)
	int32 TotalLaps = 3;

	/** Current lap time */
	UPROPERTY(BlueprintReadWrite)
	float CurrentLapTime = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadWrite)
	float BestLapTime = 0.0f;

	/** Total race time */
	UPROPERTY(BlueprintReadWrite)
	float TotalRaceTime = 0.0f;

	/** Time difference to leader (negative = ahead) */
	UPROPERTY(BlueprintReadWrite)
	float GapToLeader = 0.0f;

	/** Time difference to next position (negative = ahead) */
	UPROPERTY(BlueprintReadWrite)
	float GapToNext = 0.0f;

	/** Is on personal best pace */
	UPROPERTY(BlueprintReadWrite)
	bool bOnPBPace = false;

	/** Is final lap */
	UPROPERTY(BlueprintReadWrite)
	bool bFinalLap = false;

	/** Race progress (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float RaceProgress = 0.0f;

	/** Checkpoint progress within current lap (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float CheckpointProgress = 0.0f;
};

/**
 * Drift scoring data
 */
USTRUCT(BlueprintType)
struct FMGDriftScoreData
{
	GENERATED_BODY()

	/** Current drift score (during drift) */
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentDriftScore = 0;

	/** Current drift multiplier */
	UPROPERTY(BlueprintReadWrite)
	float DriftMultiplier = 1.0f;

	/** Total drift score this race */
	UPROPERTY(BlueprintReadWrite)
	int32 TotalDriftScore = 0;

	/** Is currently in a drift chain */
	UPROPERTY(BlueprintReadWrite)
	bool bInDriftChain = false;

	/** Drift chain count */
	UPROPERTY(BlueprintReadWrite)
	int32 DriftChainCount = 0;

	/** Time remaining to continue chain */
	UPROPERTY(BlueprintReadWrite)
	float ChainTimeRemaining = 0.0f;
};

/**
 * Race HUD Subsystem
 * Central management for all racing UI elements
 *
 * Features:
 * - Vehicle telemetry display (speed, RPM, gear)
 * - Race status (position, laps, times)
 * - Minimap integration
 * - Notification overlay
 * - Multiple display modes
 * - Customizable HUD layouts
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceHUDSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	// ==========================================
	// HUD CONTROL
	// ==========================================

	/** Set HUD display mode */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHUDMode(EMGHUDMode Mode);

	/** Get current HUD mode */
	UFUNCTION(BlueprintPure, Category = "HUD")
	EMGHUDMode GetHUDMode() const { return CurrentHUDMode; }

	/** Show/hide specific HUD elements */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetElementVisibility(FName ElementName, bool bVisible);

	/** Toggle HUD visibility */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleHUD();

	// ==========================================
	// DATA UPDATES
	// ==========================================

	/** Update vehicle telemetry */
	UFUNCTION(BlueprintCallable, Category = "HUD|Data")
	void UpdateVehicleTelemetry(const FMGVehicleTelemetry& Telemetry);

	/** Get current telemetry */
	UFUNCTION(BlueprintPure, Category = "HUD|Data")
	FMGVehicleTelemetry GetVehicleTelemetry() const { return CurrentTelemetry; }

	/** Update race status */
	UFUNCTION(BlueprintCallable, Category = "HUD|Data")
	void UpdateRaceStatus(const FMGRaceStatus& Status);

	/** Get current race status */
	UFUNCTION(BlueprintPure, Category = "HUD|Data")
	FMGRaceStatus GetRaceStatus() const { return CurrentRaceStatus; }

	/** Update drift score data */
	UFUNCTION(BlueprintCallable, Category = "HUD|Data")
	void UpdateDriftScore(const FMGDriftScoreData& DriftData);

	/** Get drift score data */
	UFUNCTION(BlueprintPure, Category = "HUD|Data")
	FMGDriftScoreData GetDriftScoreData() const { return CurrentDriftData; }

	// ==========================================
	// NOTIFICATIONS
	// ==========================================

	/** Show position change notification */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowPositionChange(int32 OldPosition, int32 NewPosition);

	/** Show lap notification */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowLapNotification(int32 LapNumber, float LapTime, bool bIsBestLap, bool bIsFinalLap);

	/** Show near miss bonus */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowNearMissBonus(int32 BonusPoints);

	/** Show drift score popup */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowDriftScorePopup(int32 Score, float Multiplier);

	/** Show generic notification */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowNotification(const FText& Message, float Duration = 2.0f, FLinearColor Color = FLinearColor::White);

	/** Show countdown */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowCountdown(int32 CountdownValue);

	/** Show "GO!" */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowRaceStart();

	/** Show wrong way warning */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowWrongWayWarning(bool bShow);

	// ==========================================
	// RACE EVENTS
	// ==========================================

	/** Called when race starts */
	UFUNCTION(BlueprintCallable, Category = "HUD|Events")
	void OnRaceStart();

	/** Called when race ends */
	UFUNCTION(BlueprintCallable, Category = "HUD|Events")
	void OnRaceEnd(bool bPlayerWon);

	/** Called when player finishes */
	UFUNCTION(BlueprintCallable, Category = "HUD|Events")
	void OnPlayerFinished(int32 FinalPosition, float FinalTime);

	/** Called when entering photo mode */
	UFUNCTION(BlueprintCallable, Category = "HUD|Events")
	void OnEnterPhotoMode();

	/** Called when exiting photo mode */
	UFUNCTION(BlueprintCallable, Category = "HUD|Events")
	void OnExitPhotoMode();

	// ==========================================
	// MINIMAP
	// ==========================================

	/** Update player position on minimap */
	UFUNCTION(BlueprintCallable, Category = "HUD|Minimap")
	void UpdateMinimapPlayerPosition(FVector2D Position, float Rotation);

	/** Update opponent position on minimap */
	UFUNCTION(BlueprintCallable, Category = "HUD|Minimap")
	void UpdateMinimapOpponentPosition(int32 OpponentIndex, FVector2D Position, float Rotation);

	/** Set minimap track data */
	UFUNCTION(BlueprintCallable, Category = "HUD|Minimap")
	void SetMinimapTrackData(UTexture2D* TrackTexture, FVector2D TrackBoundsMin, FVector2D TrackBoundsMax);

	/** Set minimap zoom level */
	UFUNCTION(BlueprintCallable, Category = "HUD|Minimap")
	void SetMinimapZoom(float ZoomLevel);

	// ==========================================
	// SETTINGS
	// ==========================================

	/** Set speed display unit (true = MPH, false = KPH) */
	UFUNCTION(BlueprintCallable, Category = "HUD|Settings")
	void SetSpeedUnitMPH(bool bUseMPH);

	/** Is using MPH */
	UFUNCTION(BlueprintPure, Category = "HUD|Settings")
	bool IsUsingMPH() const { return bDisplayMPH; }

	/** Set tachometer style */
	UFUNCTION(BlueprintCallable, Category = "HUD|Settings")
	void SetTachometerStyle(int32 StyleIndex);

	/** Set HUD scale */
	UFUNCTION(BlueprintCallable, Category = "HUD|Settings")
	void SetHUDScale(float Scale);

	/** Set HUD opacity */
	UFUNCTION(BlueprintCallable, Category = "HUD|Settings")
	void SetHUDOpacity(float Opacity);

	// ==========================================
	// EVENTS/DELEGATES
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHUDModeChanged, EMGHUDMode, NewMode);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPositionChanged, int32, OldPosition, int32, NewPosition);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompleted, int32, LapNumber);

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnHUDModeChanged OnHUDModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnPositionChanged OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnLapCompleted OnLapCompleted;

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Current HUD mode */
	EMGHUDMode CurrentHUDMode = EMGHUDMode::Full;

	/** Previous HUD mode (for restoration) */
	EMGHUDMode PreviousHUDMode = EMGHUDMode::Full;

	/** Current vehicle telemetry */
	FMGVehicleTelemetry CurrentTelemetry;

	/** Current race status */
	FMGRaceStatus CurrentRaceStatus;

	/** Current drift data */
	FMGDriftScoreData CurrentDriftData;

	/** Display speed in MPH */
	bool bDisplayMPH = false;

	/** Current tachometer style */
	int32 TachometerStyle = 0;

	/** HUD scale (0.5 - 2.0) */
	float HUDScale = 1.0f;

	/** HUD opacity (0 - 1) */
	float HUDOpacity = 1.0f;

	/** Element visibility map */
	TMap<FName, bool> ElementVisibility;

	/** Is race active */
	bool bRaceActive = false;

	/** Is showing wrong way */
	bool bShowingWrongWay = false;

	// ==========================================
	// WIDGETS (References)
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGRaceHUDWidget> RaceHUDWidget;

	UPROPERTY()
	TWeakObjectPtr<UMGMinimapWidget> MinimapWidget;

	UPROPERTY()
	TWeakObjectPtr<UMGRaceOverlayWidget> OverlayWidget;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Refresh all HUD elements */
	void RefreshHUD();

	/** Apply HUD mode settings */
	void ApplyHUDMode(EMGHUDMode Mode);
};
