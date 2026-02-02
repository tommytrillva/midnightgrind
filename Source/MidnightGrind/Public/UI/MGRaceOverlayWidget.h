// Copyright Midnight Grind. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "UI/MGNotificationSubsystem.h"
#include "MGRaceOverlayWidget.generated.h"

// EMGNotificationPriority - REMOVED (duplicate)
// Canonical definition in: UI/MGNotificationManager.h

// EMGNotificationType - REMOVED (duplicate)
// Canonical definition in: UI/MGNotificationManager.h

// FMGNotificationData - MOVED TO UI/MGNotificationSubsystem.h
// Canonical definition in: UI/MGNotificationSubsystem.h

/**
 * Countdown state
 */
USTRUCT(BlueprintType)
struct FMGCountdownState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentValue = 3;

	UPROPERTY(BlueprintReadOnly)
	float Timer = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float IntervalTime = 1.0f;
};

/**
 * Race Overlay Widget
 * Handles countdown, notifications, position changes, and other overlays
 *
 * Features:
 * - Race countdown with "GO!"
 * - Position change notifications
 * - Lap notifications
 * - Bonus popups (near miss, drift)
 * - Wrong way warning
 * - Final lap indicator
 * - Race results
 */
UCLASS(Abstract)
class MIDNIGHTGRIND_API UMGRaceOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ==========================================
	// COUNTDOWN
	// ==========================================

	/** Start race countdown */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Countdown")
	void StartCountdown(int32 StartValue = 3, float IntervalSeconds = 1.0f);

	/** Cancel countdown */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Countdown")
	void CancelCountdown();

	/** Is countdown active */
	UFUNCTION(BlueprintPure, Category = "Overlay|Countdown")
	bool IsCountdownActive() const { return CountdownState.bActive; }

	/** Get current countdown value */
	UFUNCTION(BlueprintPure, Category = "Overlay|Countdown")
	int32 GetCountdownValue() const { return CountdownState.CurrentValue; }

	// ==========================================
	// NOTIFICATIONS
	// ==========================================

	/** Show a notification */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Notifications")
	int32 ShowNotification(const FMGNotificationData& Data);

	/** Show simple text notification */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Notifications")
	int32 ShowTextNotification(const FText& Text, float Duration = 2.0f, FLinearColor Color = FLinearColor::White);

	/** Hide notification by ID */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Notifications")
	void HideNotification(int32 NotificationID);

	/** Clear all notifications */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Notifications")
	void ClearAllNotifications();

	/** Get active notification count */
	UFUNCTION(BlueprintPure, Category = "Overlay|Notifications")
	int32 GetActiveNotificationCount() const;

	// ==========================================
	// POSITION CHANGES
	// ==========================================

	/** Show position change */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Position")
	void ShowPositionChange(int32 OldPosition, int32 NewPosition);

	// ==========================================
	// LAP NOTIFICATIONS
	// ==========================================

	/** Show lap completion */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Laps")
	void ShowLapComplete(int32 LapNumber, float LapTime, bool bIsBestLap);

	/** Show final lap warning */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Laps")
	void ShowFinalLap();

	// ==========================================
	// BONUS POPUPS
	// ==========================================

	/** Show near miss bonus */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Bonus")
	void ShowNearMissBonus(int32 Points);

	/** Show drift score popup */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Bonus")
	void ShowDriftScore(int32 Score, float Multiplier, int32 ChainCount);

	/** Show generic bonus */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Bonus")
	void ShowBonus(const FText& BonusName, int32 Points, FLinearColor Color);

	// ==========================================
	// WARNINGS
	// ==========================================

	/** Show wrong way warning */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Warnings")
	void ShowWrongWay(bool bShow);

	/** Is wrong way showing */
	UFUNCTION(BlueprintPure, Category = "Overlay|Warnings")
	bool IsShowingWrongWay() const { return bShowingWrongWay; }

	// ==========================================
	// RACE END
	// ==========================================

	/** Show race finish */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Finish")
	void ShowRaceFinish(int32 FinalPosition, float TotalTime, bool bNewRecord);

	/** Show victory celebration */
	UFUNCTION(BlueprintCallable, Category = "Overlay|Finish")
	void ShowVictory();

	// ==========================================
	// EVENTS (for Blueprint binding)
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, Value);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownComplete);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationShown, const FMGNotificationData&, Data);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationHidden, int32, NotificationID);

	UPROPERTY(BlueprintAssignable, Category = "Overlay|Events")
	FOnCountdownTick OnCountdownTick;

	UPROPERTY(BlueprintAssignable, Category = "Overlay|Events")
	FOnCountdownComplete OnCountdownComplete;

	UPROPERTY(BlueprintAssignable, Category = "Overlay|Events")
	FOnNotificationShown OnNotificationShown;

	UPROPERTY(BlueprintAssignable, Category = "Overlay|Events")
	FOnNotificationHidden OnNotificationHidden;

protected:
	// ==========================================
	// BLUEPRINT IMPLEMENTABLE
	// ==========================================

	/** Called when countdown value changes */
	UFUNCTION(BlueprintNativeEvent, Category = "Overlay|Countdown")
	void OnCountdownValueChanged(int32 NewValue);

	/** Called when countdown shows "GO!" */
	UFUNCTION(BlueprintNativeEvent, Category = "Overlay|Countdown")
	void OnCountdownGo();

	/** Called to display a notification */
	UFUNCTION(BlueprintNativeEvent, Category = "Overlay|Notifications")
	void DisplayNotification(const FMGNotificationData& Data);

	/** Called when a notification should be removed */
	UFUNCTION(BlueprintNativeEvent, Category = "Overlay|Notifications")
	void RemoveNotification(int32 NotificationID);

	/** Called to update wrong way display */
	UFUNCTION(BlueprintNativeEvent, Category = "Overlay|Warnings")
	void UpdateWrongWayDisplay(bool bShow);

	/** Called to display race finish */
	UFUNCTION(BlueprintNativeEvent, Category = "Overlay|Finish")
	void DisplayRaceFinish(int32 Position, float Time, bool bNewRecord);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Maximum notifications to show at once */
	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	int32 MaxVisibleNotifications = 3;

	/** Notification sounds */
	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	USoundBase* PositionGainSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	USoundBase* PositionLossSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	USoundBase* BestLapSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	USoundBase* FinalLapSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	USoundBase* CountdownTickSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Config")
	USoundBase* CountdownGoSound = nullptr;

	/** Colors */
	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor PositionGainColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor PositionLossColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor BestLapColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor FinalLapColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor NearMissColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor DriftScoreColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Overlay|Colors")
	FLinearColor WrongWayColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	// ==========================================
	// STATE
	// ==========================================

	/** Countdown state */
	FMGCountdownState CountdownState;

	/** Active notifications */
	UPROPERTY()
	TArray<FMGNotificationData> ActiveNotifications;

	/** Next notification ID */
	int32 NextNotificationID = 0;

	/** Is showing wrong way */
	bool bShowingWrongWay = false;

	/** Wrong way flash timer */
	float WrongWayFlashTimer = 0.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update countdown logic */
	void UpdateCountdown(float MGDeltaTime);

	/** Update notification timers */
	void UpdateNotifications(float MGDeltaTime);

	/** Update wrong way flash */
	void UpdateWrongWayFlash(float MGDeltaTime);

	/** Play sound if valid */
	void PlaySound(USoundBase* Sound);

	/** Format time for display */
	FText FormatTime(float TimeSeconds) const;

	/** Get ordinal suffix (1st, 2nd, 3rd, etc.) */
	FText GetOrdinalSuffix(int32 Number) const;

	// ==========================================
	// HUD SUBSYSTEM INTEGRATION
	// ==========================================

	/** Called when HUD subsystem adds a notification */
	UFUNCTION()
	void OnHUDNotificationAdded(const FMGHUDNotification& HUDNotification);

	/** Called when HUD subsystem removes a notification */
	UFUNCTION()
	void OnHUDNotificationRemoved(int32 NotificationID);

	/** Called when HUD subsystem clears all notifications */
	UFUNCTION()
	void OnHUDNotificationsCleared();

	/** Convert HUD notification to overlay notification */
	FMGNotificationData ConvertHUDNotification(const FMGHUDNotification& HUDNotification) const;
};
