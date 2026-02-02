// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * @file MGRaceHUDSubsystem.h
 * @brief Central management subsystem for all racing UI elements
 *
 * =============================================================================
 * @section Overview
 * This file defines the Race HUD Subsystem, which serves as the central hub
 * for managing all racing-related UI elements. It provides a unified interface
 * for updating vehicle telemetry, race status, notifications, damage feedback,
 * and minimap data that HUD widgets can subscribe to and display.
 *
 * The subsystem is a WorldSubsystem, meaning it exists per-world and
 * automatically initializes when a game world is created. This makes it
 * ideal for race-specific UI that should reset between race sessions.
 *
 * Key responsibilities:
 * - Vehicle telemetry data (speed, RPM, gear, NOS)
 * - Race status tracking (position, laps, times, gaps)
 * - Drift scoring display
 * - In-race notification queue management
 * - Damage feedback and impact effects
 * - Minimap position updates
 * - HUD display mode control
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **HUD Modes**: The HUD can operate in different modes:
 *   - Full: All elements visible (normal racing)
 *   - Minimal: Just speed and position (clean view)
 *   - Hidden: No HUD (cinematic)
 *   - PhotoMode: Special UI for photo mode
 *   - Replay: Replay-specific controls visible
 *
 * - **Telemetry Data (FMGVehicleTelemetry)**: Real-time vehicle data including
 *   speed (KPH/MPH), RPM, gear, NOS amount, throttle/brake positions, and
 *   drift state. Updated every frame by the vehicle pawn.
 *
 * - **Race Status (FMGRaceStatus)**: Current race state including position,
 *   lap number, lap times, gaps to other racers, and pace comparison.
 *
 * - **Notification System**: This subsystem has its own lightweight notification
 *   queue separate from MGNotificationManager. These are racing-specific
 *   notifications like position changes, lap times, and drift scores.
 *
 * - **Damage Feedback**: Visual indicators for vehicle damage including
 *   engine health, damage vignette, and impact flash effects.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Vehicle Pawn]                    [Race Manager]
 *        |                                  |
 *        | UpdateVehicleTelemetry()         | UpdateRaceStatus()
 *        v                                  v
 *   +--------------------------------------------------+
 *   |            UMGRaceHUDSubsystem                   |
 *   +--------------------------------------------------+
 *   | CurrentTelemetry   | CurrentRaceStatus          |
 *   | CurrentDriftData   | ActiveNotifications        |
 *   | CurrentDamageData  | CurrentHUDMode             |
 *   +--------------------------------------------------+
 *        |           |            |            |
 *        v           v            v            v
 *   [Speedometer] [Position] [Minimap] [Notifications]
 *   [Tachometer]  [Lap Info] [Damage]  [Drift Score]
 *
 *   Widgets subscribe to delegates like OnPositionChanged, OnLapCompleted,
 *   OnDamageStateChanged to react to state changes.
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Get the HUD subsystem from world
 * UMGRaceHUDSubsystem* HUDSubsystem = GetWorld()->GetSubsystem<UMGRaceHUDSubsystem>();
 *
 * // Update vehicle telemetry (called from vehicle pawn)
 * FMGVehicleTelemetry Telemetry;
 * Telemetry.SpeedKPH = VehicleMovement->GetForwardSpeed() * 0.036f;
 * Telemetry.SpeedMPH = Telemetry.SpeedKPH * 0.621371f;
 * Telemetry.RPM = Engine->GetCurrentRPM();
 * Telemetry.CurrentGear = Transmission->GetCurrentGear();
 * Telemetry.NOSAmount = NOSComponent->GetNOSPercent();
 * HUDSubsystem->UpdateVehicleTelemetry(Telemetry);
 *
 * // Update race status (called from race manager)
 * FMGRaceStatus Status;
 * Status.CurrentPosition = GetPlayerPosition();
 * Status.TotalRacers = GetTotalRacers();
 * Status.CurrentLap = GetPlayerLap();
 * Status.TotalLaps = GetTotalLaps();
 * Status.CurrentLapTime = GetCurrentLapTime();
 * HUDSubsystem->UpdateRaceStatus(Status);
 *
 * // Show race-specific notifications
 * HUDSubsystem->ShowPositionChange(3, 2);  // Moved from 3rd to 2nd
 * HUDSubsystem->ShowLapNotification(2, 45.5f, true, false);  // Lap 2, best lap
 * HUDSubsystem->ShowDriftScorePopup(5000, 2.5f);  // 5000 pts at 2.5x multiplier
 *
 * // Subscribe to events in widgets
 * HUDSubsystem->OnPositionChanged.AddDynamic(this, &UMyWidget::HandlePositionChange);
 * HUDSubsystem->OnDamageStateChanged.AddDynamic(this, &UMyWidget::HandleDamageUpdate);
 *
 * // Control HUD visibility
 * HUDSubsystem->SetHUDMode(EMGHUDMode::Minimal);  // Clean view
 * HUDSubsystem->ToggleHUD();  // Toggle visibility
 * HUDSubsystem->SetElementVisibility(TEXT("Minimap"), false);  // Hide specific element
 *
 * // Trigger damage feedback
 * FMGImpactFeedback Impact;
 * Impact.Intensity = 0.8f;
 * Impact.bShowVignette = true;
 * Impact.bTriggerShake = true;
 * HUDSubsystem->TriggerImpactFeedback(Impact);
 *
 * // Get current state for widgets
 * FMGVehicleTelemetry CurrentTelemetry = HUDSubsystem->GetVehicleTelemetry();
 * FMGRaceStatus CurrentStatus = HUDSubsystem->GetRaceStatus();
 * bool bCritical = HUDSubsystem->IsVehicleCriticallyDamaged();
 * @endcode
 *
 * =============================================================================
 * @section NotificationQueue Notification Queue System
 *
 * The subsystem maintains its own notification queue for race-specific feedback.
 * This is separate from the game-wide MGNotificationManager to allow:
 * - Racing-optimized display (corner popups, floating text)
 * - Independent timing and animation
 * - Category-based filtering and dismissal
 *
 * @code
 * // Show generic notification
 * HUDSubsystem->ShowNotification(
 *     FText::FromString("Near Miss!"),
 *     2.0f,                              // Duration
 *     FLinearColor(1.0f, 0.8f, 0.0f)     // Gold color
 * );
 *
 * // Show advanced notification with all options
 * FMGHUDNotification Notif;
 * Notif.Message = FText::FromString("Perfect Drift Chain!");
 * Notif.Duration = 3.0f;
 * Notif.Priority = EMGHUDNotificationPriority::High;
 * Notif.Category = TEXT("Drift");
 * Notif.Progress = 0.75f;  // For progress-style notifications
 * int32 NotifID = HUDSubsystem->ShowNotificationAdvanced(Notif);
 *
 * // Update progress
 * HUDSubsystem->UpdateNotificationProgress(NotifID, 1.0f);
 *
 * // Dismiss by category
 * HUDSubsystem->DismissNotificationsByCategory(TEXT("Drift"));
 * @endcode
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/TimerManager.h"
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
 * Damage feedback data for HUD
 */
USTRUCT(BlueprintType)
struct FMGDamageHUDData
{
	GENERATED_BODY()

	/** Overall vehicle damage (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float OverallDamage = 0.0f;

	/** Engine health (0-1, 1 = healthy) */
	UPROPERTY(BlueprintReadWrite)
	float EngineHealth = 1.0f;

	/** Is engine smoking */
	UPROPERTY(BlueprintReadWrite)
	bool bEngineSmoking = false;

	/** Is engine on fire */
	UPROPERTY(BlueprintReadWrite)
	bool bEngineOnFire = false;

	/** Are headlights broken */
	UPROPERTY(BlueprintReadWrite)
	bool bHeadlightsBroken = false;

	/** Are taillights broken */
	UPROPERTY(BlueprintReadWrite)
	bool bTaillightsBroken = false;

	/** Is currently scraping */
	UPROPERTY(BlueprintReadWrite)
	bool bIsScraping = false;

	/** Is vehicle limping (critically damaged) */
	UPROPERTY(BlueprintReadWrite)
	bool bIsLimping = false;
};

/**
 * Impact feedback for HUD effects
 */
USTRUCT(BlueprintType)
struct FMGImpactFeedback
{
	GENERATED_BODY()

	/** Impact intensity (0-1) */
	UPROPERTY(BlueprintReadWrite)
	float Intensity = 0.0f;

	/** Impact direction (normalized, in screen space) */
	UPROPERTY(BlueprintReadWrite)
	FVector2D Direction = FVector2D::ZeroVector;

	/** Should show vignette flash */
	UPROPERTY(BlueprintReadWrite)
	bool bShowVignette = false;

	/** Should trigger screen shake */
	UPROPERTY(BlueprintReadWrite)
	bool bTriggerShake = false;
};

/**
 * HUD notification priority for display ordering
 */
UENUM(BlueprintType)
enum class EMGHUDNotificationPriority : uint8
{
	/** Low priority - informational */
	Low,
	/** Normal priority - standard gameplay feedback */
	Normal,
	/** High priority - important events */
	High,
	/** Critical priority - warnings and errors */
	Critical
};

/**
 * A queued notification to display
 */
USTRUCT(BlueprintType)
struct FMGHUDNotification
{
	GENERATED_BODY()

	/** Unique ID for this notification */
	UPROPERTY(BlueprintReadOnly)
	int32 NotificationID = 0;

	/** Display message */
	UPROPERTY(BlueprintReadWrite)
	FText Message;

	/** Display duration in seconds */
	UPROPERTY(BlueprintReadWrite)
	float Duration = 2.0f;

	/** Remaining time before removal */
	UPROPERTY(BlueprintReadOnly)
	float RemainingTime = 0.0f;

	/** Display color */
	UPROPERTY(BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	/** Priority for ordering */
	UPROPERTY(BlueprintReadWrite)
	EMGHUDNotificationPriority Priority = EMGHUDNotificationPriority::Normal;

	/** Optional icon name */
	UPROPERTY(BlueprintReadWrite)
	FName IconName;

	/** Whether notification should stack with duplicates or replace */
	UPROPERTY(BlueprintReadWrite)
	bool bStackable = true;

	/** Progress value (0-1) for progress-style notifications, -1 for no progress */
	UPROPERTY(BlueprintReadWrite)
	float Progress = -1.0f;

	/** Category tag for filtering/grouping */
	UPROPERTY(BlueprintReadWrite)
	FName Category;

	/** Animation state (0 = entering, 1 = visible, fading out when < Duration threshold) */
	UPROPERTY(BlueprintReadOnly)
	float AnimationAlpha = 0.0f;

	/** World time when notification was created */
	UPROPERTY(BlueprintReadOnly)
	float CreationTime = 0.0f;

	FMGHUDNotification() = default;

	FMGHUDNotification(const FText& InMessage, float InDuration, FLinearColor InColor)
		: Message(InMessage), Duration(InDuration), RemainingTime(InDuration), Color(InColor) {}
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
 * - Damage feedback indicators
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

	/** Show airtime/jump popup */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowAirtimePopup(float AirtimeSeconds, int32 Score);

	/** Show trick completed popup */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowTrickPopup(const FText& TrickName, int32 Score);

	/** Show generic notification */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ShowNotification(const FText& Message, float Duration = 2.0f, FLinearColor Color = FLinearColor::White);

	/** Show notification with full options */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	int32 ShowNotificationAdvanced(const FMGHUDNotification& Notification);

	/** Update an existing notification's progress */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void UpdateNotificationProgress(int32 NotificationID, float Progress);

	/** Dismiss a specific notification */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void DismissNotification(int32 NotificationID);

	/** Dismiss all notifications in a category */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void DismissNotificationsByCategory(FName Category);

	/** Clear all notifications */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void ClearAllNotifications();

	/** Get all active notifications */
	UFUNCTION(BlueprintPure, Category = "HUD|Notifications")
	TArray<FMGHUDNotification> GetActiveNotifications() const { return ActiveNotifications; }

	/** Get notification count */
	UFUNCTION(BlueprintPure, Category = "HUD|Notifications")
	int32 GetActiveNotificationCount() const { return ActiveNotifications.Num(); }

	/** Set maximum simultaneous notifications (oldest will be removed) */
	UFUNCTION(BlueprintCallable, Category = "HUD|Notifications")
	void SetMaxNotifications(int32 MaxCount);

	/** Get maximum notifications */
	UFUNCTION(BlueprintPure, Category = "HUD|Notifications")
	int32 GetMaxNotifications() const { return MaxActiveNotifications; }

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
	// DAMAGE FEEDBACK
	// ==========================================

	/** Update damage state for HUD indicators */
	UFUNCTION(BlueprintCallable, Category = "HUD|Damage")
	void UpdateDamageState(const FMGDamageHUDData& DamageData);

	/** Get current damage state */
	UFUNCTION(BlueprintPure, Category = "HUD|Damage")
	FMGDamageHUDData GetDamageState() const { return CurrentDamageData; }

	/** Trigger impact feedback (flash, shake, vignette) */
	UFUNCTION(BlueprintCallable, Category = "HUD|Damage")
	void TriggerImpactFeedback(const FMGImpactFeedback& Feedback);

	/** Show damage warning notification */
	UFUNCTION(BlueprintCallable, Category = "HUD|Damage")
	void ShowDamageWarning(const FText& Message, float Duration = 2.0f);

	/** Set damage vignette intensity (0-1) */
	UFUNCTION(BlueprintCallable, Category = "HUD|Damage")
	void SetDamageVignetteIntensity(float Intensity);

	/** Get damage vignette intensity */
	UFUNCTION(BlueprintPure, Category = "HUD|Damage")
	float GetDamageVignetteIntensity() const { return DamageVignetteIntensity; }

	/** Is vehicle in critical damage state */
	UFUNCTION(BlueprintPure, Category = "HUD|Damage")
	bool IsVehicleCriticallyDamaged() const { return CurrentDamageData.bIsLimping || CurrentDamageData.bEngineOnFire; }

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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageStateChanged, const FMGDamageHUDData&, DamageData);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImpactReceived, const FMGImpactFeedback&, Feedback);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationAdded, const FMGHUDNotification&, Notification);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationRemoved, int32, NotificationID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNotificationProgressUpdated, int32, NotificationID, float, Progress);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllNotificationsCleared);

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnHUDModeChanged OnHUDModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnPositionChanged OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnLapCompleted OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnDamageStateChanged OnDamageStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnImpactReceived OnImpactReceived;

	/** Fired when a notification is added to the queue */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Notifications")
	FOnNotificationAdded OnNotificationAdded;

	/** Fired when a notification expires or is dismissed */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Notifications")
	FOnNotificationRemoved OnNotificationRemoved;

	/** Fired when a notification's progress is updated */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Notifications")
	FOnNotificationProgressUpdated OnNotificationProgressUpdated;

	/** Fired when all notifications are cleared */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Notifications")
	FOnAllNotificationsCleared OnAllNotificationsCleared;

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

	/** Current damage state */
	FMGDamageHUDData CurrentDamageData;

	/** Damage vignette intensity (0-1) */
	float DamageVignetteIntensity = 0.0f;

	/** Target vignette intensity for smooth interpolation */
	float TargetVignetteIntensity = 0.0f;

	/** Current impact flash alpha */
	float ImpactFlashAlpha = 0.0f;

	// ==========================================
	// NOTIFICATION STATE
	// ==========================================

	/** Active notifications being displayed */
	UPROPERTY()
	TArray<FMGHUDNotification> ActiveNotifications;

	/** Next notification ID to assign */
	int32 NextNotificationID = 1;

	/** Maximum simultaneous notifications */
	int32 MaxActiveNotifications = 5;

	/** Notification fade in duration */
	float NotificationFadeInDuration = 0.2f;

	/** Notification fade out duration */
	float NotificationFadeOutDuration = 0.3f;

	/** Timer handle for notification tick */
	FTimerHandle NotificationTickHandle;

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

	/** Process notification timers and animations */
	void TickNotifications();

	/** Start the notification tick timer */
	void StartNotificationTicker();

	/** Stop the notification tick timer */
	void StopNotificationTicker();

	/** Find notification by ID */
	FMGHUDNotification* FindNotificationByID(int32 NotificationID);

	/** Remove expired notifications */
	void RemoveExpiredNotifications();

	/** Enforce max notification limit */
	void EnforceNotificationLimit();

	/** Sort notifications by priority */
	void SortNotificationsByPriority();
};
