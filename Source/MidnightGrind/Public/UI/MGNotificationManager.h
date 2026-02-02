// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * @file MGNotificationManager.h
 * @brief Game-wide notification system for alerts, rewards, and toasts
 *
 * =============================================================================
 * @section Overview
 * This file defines the notification manager subsystem that handles all in-game
 * notifications, from simple toast messages to elaborate reward showcases. It
 * provides a centralized system for queuing, prioritizing, and displaying
 * notifications to the player.
 *
 * The notification system supports:
 * - Priority-based queuing (critical notifications jump the queue)
 * - Multiple notification types with distinct visual styles
 * - Auto-dismiss with configurable durations
 * - Action buttons for interactive notifications
 * - Notification history for later review
 * - Separate toast system for quick messages
 * - In-race specific notifications (position changes, lap times)
 *
 * This manager is a GameInstanceSubsystem, meaning it persists across level
 * changes and is ideal for game-wide notifications.
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Priority Queue**: Notifications are shown in priority order, not FIFO.
 *   Critical notifications (errors, unlocks) show immediately while low-priority
 *   ones wait their turn. The queue is re-sorted after each addition.
 *
 * - **Notification Types (EMGNotificationType)**: Different types have distinct
 *   visual styles, colors, and sounds:
 *   - Info: General information (blue)
 *   - Success: Completion/success (green)
 *   - Warning: Caution needed (yellow)
 *   - Error: Something went wrong (red)
 *   - Reward: Currency/item earned (gold)
 *   - Challenge: Challenge completed (purple)
 *   - Achievement: Achievement unlocked (platinum)
 *   - LevelUp: Player leveled up (rainbow)
 *   - Unlock: New content unlocked (cyan)
 *   - Rival: Rival-related event (orange)
 *   - Social: Friend/crew events (blue)
 *   - System: System messages (gray)
 *
 * - **Toast vs Notification**: Toasts (FMGToast) are simple, brief text messages
 *   that appear and disappear quickly. Full notifications (FMGNotification) have
 *   titles, bodies, icons, durations, and optional action buttons.
 *
 * - **Do Not Disturb Mode**: When enabled, only Critical priority notifications
 *   will be shown. Useful during races or cinematics.
 *
 * - **Notification Gap**: A small delay (0.5s by default) between notifications
 *   prevents overwhelming the player with rapid-fire messages.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Game Systems] ----> QueueNotification() ----> [MGNotificationManager]
 *                                                         |
 *   Achievement System --->                               |
 *   Level System --------->  Quick Methods                +-- NotificationQueue
 *   Race System ---------->  (ShowReward, etc.)           |     (priority sorted)
 *   Economy System ------->                               |
 *   Social System -------->                               +-- ToastQueue
 *                                                         |     (FIFO)
 *                                                         |
 *                                                         +-- CurrentNotification
 *                                                         |
 *                                                         +-- NotificationHistory
 *                                                         |     (last 100)
 *                                                         |
 *                                                         v
 *                                          [Delegates: OnShown, OnDismissed, OnAction]
 *                                                         |
 *                                                         v
 *                                                   [UI Widgets Subscribe]
 *
 * =============================================================================
 * @section Usage
 * @code
 * // === GETTING THE MANAGER ===
 * UMGNotificationManager* NotifMgr = GetGameInstance()->GetSubsystem<UMGNotificationManager>();
 *
 * // === SIMPLE TOASTS ===
 * // Quick toast message (default 3 seconds)
 * NotifMgr->ShowToast(FText::FromString("Settings saved!"));
 *
 * // Toast with type and duration
 * NotifMgr->ShowToast(
 *     FText::FromString("Connection lost!"),
 *     EMGNotificationType::Warning,
 *     5.0f  // 5 second duration
 * );
 *
 * // Toast with format string
 * NotifMgr->ShowToastFormat(
 *     NSLOCTEXT("UI", "WelcomeFormat", "Welcome back, {0}!"),
 *     PlayerName,
 *     EMGNotificationType::Info
 * );
 *
 * // === QUICK NOTIFICATION METHODS ===
 * // Reward notification
 * NotifMgr->ShowReward(
 *     FText::FromString("Race Complete!"),
 *     5000,   // Credits
 *     250     // XP
 * );
 *
 * // Challenge completed
 * NotifMgr->ShowChallengeComplete(
 *     FText::FromString("Speed Demon Challenge"),
 *     10000   // Reward credits
 * );
 *
 * // Achievement unlocked
 * NotifMgr->ShowAchievement(
 *     FText::FromString("First Victory"),
 *     FText::FromString("Win your first race")
 * );
 *
 * // Level up
 * NotifMgr->ShowLevelUp(15);  // Reached level 15
 *
 * // Item unlocked
 * NotifMgr->ShowUnlock(
 *     FText::FromString("Neon Underglow"),
 *     TEXT("Customization")  // Item type
 * );
 *
 * // Rival event
 * NotifMgr->ShowRivalEvent(
 *     FText::FromString("Razor"),
 *     FText::FromString("challenged you to a race!")
 * );
 *
 * // Error/Success messages
 * NotifMgr->ShowError(FText::FromString("Purchase failed. Insufficient funds."));
 * NotifMgr->ShowSuccess(FText::FromString("Vehicle purchased!"));
 *
 * // === IN-RACE NOTIFICATIONS ===
 * // Position change
 * NotifMgr->ShowPositionChange(3, 2);  // Moved from 3rd to 2nd place
 *
 * // Lap complete
 * NotifMgr->ShowLapComplete(
 *     2,          // Lap number
 *     45.678f,    // Lap time in seconds
 *     true        // Was best lap
 * );
 *
 * // Drift score
 * NotifMgr->ShowDriftScore(
 *     5000,                           // Score
 *     FText::FromString("INSANE")     // Tier name
 * );
 *
 * // Combo
 * NotifMgr->ShowCombo(
 *     5,      // Combo count
 *     2.5f    // Multiplier
 * );
 *
 * // Near miss
 * NotifMgr->ShowNearMiss(100);  // Bonus points
 *
 * // === CUSTOM NOTIFICATIONS ===
 * // Create a fully customized notification
 * FMGNotification Notif;
 * Notif.Type = EMGNotificationType::Reward;
 * Notif.Priority = EMGNotificationPriority::High;
 * Notif.Title = FText::FromString("Daily Login Bonus!");
 * Notif.Body = FText::FromString("You received 1000 credits");
 * Notif.IconID = TEXT("Icon_Credits");
 * Notif.Amount = 1000;
 * Notif.Duration = 5.0f;
 * Notif.bDismissable = true;
 * Notif.bAutoDismiss = true;
 *
 * // Add action button
 * Notif.bHasAction = true;
 * Notif.ActionText = FText::FromString("View Rewards");
 * Notif.ActionID = TEXT("OpenRewardsScreen");
 *
 * NotifMgr->QueueNotification(Notif);
 *
 * // Show immediately (bypass queue)
 * NotifMgr->ShowNotificationImmediately(Notif);
 *
 * // === QUEUE MANAGEMENT ===
 * // Check current state
 * bool bShowing = NotifMgr->IsNotificationShowing();
 * int32 QueueSize = NotifMgr->GetQueueSize();
 * FMGNotification Current = NotifMgr->GetCurrentNotification();
 *
 * // Dismiss notifications
 * NotifMgr->DismissCurrentNotification();
 * NotifMgr->DismissNotification(SomeNotificationID);
 * NotifMgr->ClearAllNotifications();
 *
 * // === HISTORY ===
 * // Get recent notifications
 * TArray<FMGNotification> History = NotifMgr->GetNotificationHistory(20);
 * int32 UnreadCount = NotifMgr->GetUnreadCount();
 * NotifMgr->MarkAllAsRead();
 *
 * // === SETTINGS ===
 * // Enable/disable notifications
 * NotifMgr->SetNotificationsEnabled(true);
 * NotifMgr->SetToastsEnabled(true);
 *
 * // Do Not Disturb mode (only Critical priority)
 * NotifMgr->SetDoNotDisturb(true);
 *
 * // === EVENT SUBSCRIPTIONS ===
 * // Subscribe to notification lifecycle events
 * NotifMgr->OnNotificationQueued.AddDynamic(this, &UMyWidget::HandleQueued);
 * NotifMgr->OnNotificationShown.AddDynamic(this, &UMyWidget::HandleShown);
 * NotifMgr->OnNotificationDismissed.AddDynamic(this, &UMyWidget::HandleDismissed);
 *
 * // Handle action button clicks
 * NotifMgr->OnNotificationAction.AddDynamic(this, &UMyWidget::HandleAction);
 *
 * void UMyWidget::HandleAction(const FMGNotification& Notif, FName ActionID)
 * {
 *     if (ActionID == TEXT("OpenRewardsScreen"))
 *     {
 *         OpenRewardsScreen();
 *     }
 * }
 * @endcode
 *
 * =============================================================================
 * @section Difference Difference from MGNotificationSubsystem
 *
 * | Feature              | MGNotificationManager    | MGNotificationSubsystem      |
 * |----------------------|--------------------------|------------------------------|
 * | Subsystem Type       | GameInstanceSubsystem    | GameInstanceSubsystem        |
 * | Purpose              | Basic queue & display    | Specialized game events      |
 * | Data Structure       | FMGNotification          | Achievement, LevelUp structs |
 * | Quick Methods        | ShowReward, ShowError    | ShowAchievementNotification  |
 * | Toast Support        | Yes (separate queue)     | No                           |
 * | In-Race Notifs       | Yes (position, lap)      | No                           |
 * | History Filtering    | Basic (by count)         | By type, category            |
 * | Best For             | Simple feedback          | Milestone celebrations       |
 *
 * Use MGNotificationManager for everyday feedback (toasts, errors, in-race).
 * Use MGNotificationSubsystem for milestone events needing rich display.
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGNotificationManager.generated.h"

class UMGNotificationWidget;
class UMGToastWidget;

// =============================================================================
// Enums and Structs
// =============================================================================

/**
 * Notification type - determines visual style
 * Each type can have its own colors, icons, and sounds
 */
UENUM(BlueprintType)
enum class EMGNotificationType : uint8
{
	/** General info */
	Info,
	/** Success/completion */
	Success,
	/** Warning */
	Warning,
	/** Error */
	Error,
	/** Reward earned */
	Reward,
	/** Challenge completed */
	Challenge,
	/** Achievement unlocked */
	Achievement,
	/** Level up */
	LevelUp,
	/** Unlock */
	Unlock,
	/** Rival event */
	Rival,
	/** Social (friend online, invite) */
	Social,
	/** System message */
	System
};

// MOVED TO MGSharedTypes.h
// /**
//  * Notification priority
//  */
// UENUM(BlueprintType)
// enum class EMGNotificationPriority : uint8
// {
// 	Low,
// 	Normal,
// 	High,
// 	Critical
// };

/**
 * Notification data
 */
USTRUCT(BlueprintType)
struct FMGNotification
{
	GENERATED_BODY()

	/** Unique notification ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid NotificationID;

	/** Type for visual styling */
	UPROPERTY(BlueprintReadWrite)
	EMGNotificationType Type = EMGNotificationType::Info;

	/** Priority */
	UPROPERTY(BlueprintReadWrite)
	EMGNotificationPriority Priority = EMGNotificationPriority::Normal;

	/** Title text */
	UPROPERTY(BlueprintReadWrite)
	FText Title;

	/** Body/description text */
	UPROPERTY(BlueprintReadWrite)
	FText Body;

	/** Optional icon ID */
	UPROPERTY(BlueprintReadWrite)
	FName IconID;

	/** Optional item/reward ID (for reward notifications) */
	UPROPERTY(BlueprintReadWrite)
	FName ItemID;

	/** Optional amount (for rewards) */
	UPROPERTY(BlueprintReadWrite)
	int64 Amount = 0;

	/** Display duration in seconds (0 = until dismissed) */
	UPROPERTY(BlueprintReadWrite)
	float Duration = 5.0f;

	/** Can be dismissed by player */
	UPROPERTY(BlueprintReadWrite)
	bool bDismissable = true;

	/** Auto-dismiss after duration */
	UPROPERTY(BlueprintReadWrite)
	bool bAutoDismiss = true;

	/** Has action button */
	UPROPERTY(BlueprintReadWrite)
	bool bHasAction = false;

	/** Action button text */
	UPROPERTY(BlueprintReadWrite)
	FText ActionText;

	/** Action callback ID */
	UPROPERTY(BlueprintReadWrite)
	FName ActionID;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Has been shown */
	UPROPERTY(BlueprintReadOnly)
	bool bShown = false;

	/** Has been dismissed */
	UPROPERTY(BlueprintReadOnly)
	bool bDismissed = false;

	FMGNotification()
	{
		NotificationID = FGuid::NewGuid();
		Timestamp = FDateTime::UtcNow();
	}
};

/**
 * Toast message (simple text notification)
 */
USTRUCT(BlueprintType)
struct FMGToast
{
	GENERATED_BODY()

	/** Message text */
	UPROPERTY(BlueprintReadWrite)
	FText Message;

	/** Type for styling */
	UPROPERTY(BlueprintReadWrite)
	EMGNotificationType Type = EMGNotificationType::Info;

	/** Duration in seconds */
	UPROPERTY(BlueprintReadWrite)
	float Duration = 3.0f;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationQueued, const FMGNotification&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationShown, const FMGNotification&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationDismissed, const FMGNotification&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNotificationAction, const FMGNotification&, Notification, FName, ActionID);

/**
 * Notification Manager
 * Handles in-game notifications, toasts, and reward popups
 *
 * Features:
 * - Priority queue for notifications
 * - Different visual styles per type
 * - Auto-dismiss and manual dismiss
 * - Action callbacks
 * - Notification history
 * - Y2K neon styling support
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNotificationManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// NOTIFICATIONS
	// ==========================================

	/**
	 * Queue a notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void QueueNotification(const FMGNotification& Notification);

	/**
	 * Show notification immediately (bypasses queue)
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowNotificationImmediately(const FMGNotification& Notification);

	/**
	 * Dismiss current notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void DismissCurrentNotification();

	/**
	 * Dismiss notification by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void DismissNotification(const FGuid& NotificationID);

	/**
	 * Clear all notifications
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ClearAllNotifications();

	// ==========================================
	// QUICK NOTIFICATIONS
	// ==========================================

	/**
	 * Show reward notification
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowReward(FText Title, int64 Credits, int32 XP = 0);

	/**
	 * Show challenge completed
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowChallengeComplete(FText ChallengeName, int64 RewardCredits);

	/**
	 * Show achievement unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowAchievement(FText AchievementName, FText Description);

	/**
	 * Show level up
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowLevelUp(int32 NewLevel);

	/**
	 * Show item unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowUnlock(FText ItemName, FName ItemType);

	/**
	 * Show rival event
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowRivalEvent(FText RivalName, FText EventDescription);

	/**
	 * Show error
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowError(FText ErrorMessage);

	/**
	 * Show success
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Quick")
	void ShowSuccess(FText Message);

	// ==========================================
	// TOASTS
	// ==========================================

	/**
	 * Show simple toast message
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Toast")
	void ShowToast(FText Message, EMGNotificationType Type = EMGNotificationType::Info, float Duration = 3.0f);

	/**
	 * Show toast with format
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Toast")
	void ShowToastFormat(const FText& Format, const FText& Arg1, EMGNotificationType Type = EMGNotificationType::Info);

	// ==========================================
	// IN-RACE NOTIFICATIONS
	// ==========================================

	/**
	 * Show position change
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Race")
	void ShowPositionChange(int32 OldPosition, int32 NewPosition);

	/**
	 * Show lap complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Race")
	void ShowLapComplete(int32 LapNumber, float LapTime, bool bBestLap);

	/**
	 * Show drift score
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Race")
	void ShowDriftScore(int32 Score, FText TierName);

	/**
	 * Show combo
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Race")
	void ShowCombo(int32 ComboCount, float Multiplier);

	/**
	 * Show near miss
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Race")
	void ShowNearMiss(int32 BonusPoints);

	// ==========================================
	// HISTORY
	// ==========================================

	/**
	 * Get notification history
	 */
	UFUNCTION(BlueprintPure, Category = "Notifications|History")
	TArray<FMGNotification> GetNotificationHistory(int32 Count = 20) const;

	/**
	 * Get unread count
	 */
	UFUNCTION(BlueprintPure, Category = "Notifications|History")
	int32 GetUnreadCount() const;

	/**
	 * Mark all as read
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|History")
	void MarkAllAsRead();

	// ==========================================
	// STATE
	// ==========================================

	/**
	 * Is notification showing
	 */
	UFUNCTION(BlueprintPure, Category = "Notifications|State")
	bool IsNotificationShowing() const { return bNotificationActive; }

	/**
	 * Get current notification
	 */
	UFUNCTION(BlueprintPure, Category = "Notifications|State")
	FMGNotification GetCurrentNotification() const { return CurrentNotification; }

	/**
	 * Get queue size
	 */
	UFUNCTION(BlueprintPure, Category = "Notifications|State")
	int32 GetQueueSize() const { return NotificationQueue.Num(); }

	// ==========================================
	// SETTINGS
	// ==========================================

	/**
	 * Set notifications enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Settings")
	void SetNotificationsEnabled(bool bEnabled) { bNotificationsEnabled = bEnabled; }

	/**
	 * Set toasts enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Settings")
	void SetToastsEnabled(bool bEnabled) { bToastsEnabled = bEnabled; }

	/**
	 * Set do not disturb (only critical notifications)
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications|Settings")
	void SetDoNotDisturb(bool bDND) { bDoNotDisturb = bDND; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Notification queued */
	UPROPERTY(BlueprintAssignable, Category = "Notifications|Events")
	FOnNotificationQueued OnNotificationQueued;

	/** Notification shown */
	UPROPERTY(BlueprintAssignable, Category = "Notifications|Events")
	FOnNotificationShown OnNotificationShown;

	/** Notification dismissed */
	UPROPERTY(BlueprintAssignable, Category = "Notifications|Events")
	FOnNotificationDismissed OnNotificationDismissed;

	/** Notification action triggered */
	UPROPERTY(BlueprintAssignable, Category = "Notifications|Events")
	FOnNotificationAction OnNotificationAction;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Process notification queue */
	void ProcessQueue();

	/** Show next notification from queue */
	void ShowNextNotification();

	/** Get color for notification type */
	FLinearColor GetNotificationColor(EMGNotificationType Type) const;

	/** Timer tick */
	void OnTick();

	/** Sort queue by priority */
	void SortQueue();

private:
	/** Notification queue */
	TArray<FMGNotification> NotificationQueue;

	/** Notification history */
	TArray<FMGNotification> NotificationHistory;

	/** Current active notification */
	FMGNotification CurrentNotification;

	/** Toast queue */
	TArray<FMGToast> ToastQueue;

	/** Is notification currently showing */
	bool bNotificationActive = false;

	/** Time remaining on current notification */
	float CurrentNotificationTimeRemaining = 0.0f;

	/** Is toast currently showing */
	bool bToastActive = false;

	/** Toast time remaining */
	float ToastTimeRemaining = 0.0f;

	/** Settings */
	bool bNotificationsEnabled = true;
	bool bToastsEnabled = true;
	bool bDoNotDisturb = false;

	/** Timer handle */
	FTimerHandle TickTimer;

	/** Max history size */
	static constexpr int32 MaxHistorySize = 100;

	/** Time between notifications */
	static constexpr float NotificationGap = 0.5f;

	/** Gap timer */
	float GapTimer = 0.0f;
};
