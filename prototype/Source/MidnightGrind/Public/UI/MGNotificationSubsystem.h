// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGNotificationSubsystem.h
 * @brief Extended notification subsystem with specialized notification types
 *
 * =============================================================================
 * @section Overview
 * This file defines an extended notification subsystem that provides specialized
 * notifications for common game events. While MGNotificationManager handles basic
 * notification queuing, this subsystem adds:
 *
 * - Achievement unlock celebrations with rarity display
 * - Level up announcements with unlocked rewards
 * - Race result summaries with stats breakdown
 * - Challenge completion with reward showcase
 * - Currency gain/loss notifications
 *
 * The system also manages notification history for later review and provides
 * filtering/grouping capabilities for the notification center UI.
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Specialized Notification Types**: Each game event type (achievement,
 *   level up, race result) has a dedicated struct with all relevant data.
 *
 * - **Display Styles (EMGNotificationStyle)**: Notifications can appear as:
 *   - Toast: Small corner popup for quick, non-intrusive messages
 *   - Banner: Horizontal strip at screen top/bottom for moderate importance
 *   - Popup: Center-screen popup for rewards and confirmations
 *   - FullScreen: Dramatic announcements (level up, race results)
 *   - Minimal: Ultra-simple text-only display for subtle feedback
 *
 * - **Priority System (EMGNotificationPriority)**: Controls display order:
 *   - Low: Informational, can be suppressed
 *   - Normal: Standard notifications
 *   - High: Important events, shown promptly
 *   - Critical: Must-see (race finish, unlock), interrupts queue
 *   - System: Errors and warnings
 *
 * - **Reward Display (FMGRewardDisplayData)**: Standardizes how rewards are
 *   shown, with icon, name, quantity, rarity color, and currency type.
 *
 * - **History Tracking (FMGNotificationHistoryEntry)**: All shown notifications
 *   are stored in history with timestamps, interaction data, and action taken
 *   for later review in the notification center.
 *
 * - **Do Not Disturb Mode**: When enabled, only Critical and System priority
 *   notifications will be shown. Useful during intense gameplay.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Game Events]
 *        |
 *        +-- Achievement Unlocked ---> ShowAchievementNotification()
 *        |
 *        +-- Player Leveled Up ------> ShowLevelUpNotification()
 *        |
 *        +-- Race Finished ----------> ShowRaceResultNotification()
 *        |
 *        +-- Challenge Complete -----> ShowChallengeCompleteNotification()
 *        |
 *        +-- Currency Earned --------> ShowCurrencyNotification()
 *        |
 *        v
 *   [MGNotificationSubsystem]
 *        |
 *        +-- ShouldShowNotification() --> Check DND, priority filter
 *        |
 *        +-- SortQueue() --> Priority-based ordering
 *        |
 *        +-- ShowNotification() --> Display via widget
 *        |
 *        +-- AddToHistory() --> Store for review
 *        |
 *        v
 *   [Notification Widgets] (Toast, Banner, Popup, FullScreen)
 *        |
 *        +-- OnNotificationShown delegate
 *        +-- OnNotificationDismissed delegate
 *        +-- OnNotificationAction delegate (for button clicks)
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Get the notification subsystem
 * UMGNotificationSubsystem* NotifSys = GetGameInstance()->GetSubsystem<UMGNotificationSubsystem>();
 *
 * // === ACHIEVEMENT NOTIFICATION ===
 * FMGAchievementNotification Achievement;
 * Achievement.AchievementID = TEXT("ACH_SPEED_DEMON");
 * Achievement.Name = FText::FromString("Speed Demon");
 * Achievement.Description = FText::FromString("Reach 200 MPH");
 * Achievement.Points = 50;
 * Achievement.RarityPercent = 5.0f; // Only 5% of players have this
 * Achievement.bIsSecret = false;
 * NotifSys->ShowAchievementNotification(Achievement);
 *
 * // === LEVEL UP NOTIFICATION ===
 * FMGLevelUpNotification LevelUp;
 * LevelUp.NewLevel = 15;
 * LevelUp.LevelTitle = FText::FromString("Street Legend");
 *
 * // Add unlocked rewards
 * FMGRewardDisplayData VehicleReward;
 * VehicleReward.RewardName = FText::FromString("1999 Nissan Skyline R34");
 * VehicleReward.Icon = VehicleIcon;
 * VehicleReward.RarityColor = FLinearColor(1.0f, 0.5f, 0.0f); // Rare orange
 * LevelUp.UnlockedRewards.Add(VehicleReward);
 *
 * LevelUp.UnlockedFeatures.Add(FText::FromString("Online Racing"));
 * LevelUp.UnlockedFeatures.Add(FText::FromString("Custom Liveries"));
 * NotifSys->ShowLevelUpNotification(LevelUp);
 *
 * // === RACE RESULT NOTIFICATION ===
 * FMGRaceResultNotification RaceResult;
 * RaceResult.Position = 1;
 * RaceResult.TotalRacers = 8;
 * RaceResult.FinishTime = 185.5f;
 * RaceResult.BestLapTime = 45.2f;
 * RaceResult.bIsPersonalBest = true;
 * RaceResult.bIsTrackRecord = false;
 * RaceResult.CashEarned = 10000;
 * RaceResult.XPEarned = 500;
 * RaceResult.ReputationEarned = 100;
 * NotifSys->ShowRaceResultNotification(RaceResult);
 *
 * // === SIMPLE NOTIFICATIONS ===
 * // Queue a simple notification
 * FGuid NotifID = NotifSys->QueueSimpleNotification(
 *     FText::FromString("Connection Restored"),
 *     FText::FromString("You are back online."),
 *     EMGNotificationType::Info
 * );
 *
 * // Show currency change
 * NotifSys->ShowCurrencyNotification(TEXT("Cash"), 5000, true);  // Gained $5000
 * NotifSys->ShowCurrencyNotification(TEXT("Cash"), -1000, false); // Lost $1000
 *
 * // Show error/warning
 * NotifSys->ShowErrorNotification(
 *     FText::FromString("Save Failed"),
 *     FText::FromString("Unable to save progress. Please try again.")
 * );
 *
 * // === NOTIFICATION QUEUE MANAGEMENT ===
 * // Cancel a queued notification
 * NotifSys->CancelNotification(NotifID);
 *
 * // Dismiss current notification
 * NotifSys->DismissCurrentNotification();
 *
 * // Check queue status
 * int32 QueueSize = NotifSys->GetQueueSize();
 * bool bShowing = NotifSys->IsShowingNotification();
 * FMGNotificationData Current = NotifSys->GetCurrentNotification();
 *
 * // === HISTORY AND FILTERING ===
 * // Query notification history
 * TArray<FMGNotificationHistoryEntry> History = NotifSys->GetNotificationHistory();
 * int32 Unread = NotifSys->GetUnreadCount();
 *
 * // Filter history by type
 * TArray<FMGNotificationHistoryEntry> Achievements =
 *     NotifSys->GetHistoryByType(EMGNotificationType::Success);
 *
 * // Filter by category
 * TArray<FMGNotificationHistoryEntry> RaceNotifs =
 *     NotifSys->GetHistoryByCategory(TEXT("Race"));
 *
 * // Mark notifications as read
 * NotifSys->MarkNotificationRead(SomeNotificationID);
 * NotifSys->MarkAllAsRead();
 *
 * // === SETTINGS ===
 * // Enable/disable notifications
 * NotifSys->SetNotificationsEnabled(true);
 * NotifSys->SetSoundsEnabled(true);
 *
 * // Enable Do Not Disturb mode
 * NotifSys->SetDoNotDisturb(true);  // Only Critical/System show
 *
 * // Filter by minimum priority
 * NotifSys->SetMinimumPriority(EMGNotificationPriority::High);
 *
 * // === EVENT SUBSCRIPTIONS ===
 * // Subscribe to notification events
 * NotifSys->OnNotificationQueued.AddDynamic(this, &UMyClass::HandleQueued);
 * NotifSys->OnNotificationShown.AddDynamic(this, &UMyClass::HandleShown);
 * NotifSys->OnNotificationDismissed.AddDynamic(this, &UMyClass::HandleDismissed);
 * NotifSys->OnNotificationAction.AddDynamic(this, &UMyClass::HandleAction);
 * NotifSys->OnAchievementUnlocked.AddDynamic(this, &UMyClass::HandleAchievement);
 * NotifSys->OnLevelUp.AddDynamic(this, &UMyClass::HandleLevelUp);
 * @endcode
 *
 * =============================================================================
 * @section Difference Difference from MGNotificationManager
 *
 * This subsystem (MGNotificationSubsystem) and MGNotificationManager serve
 * different purposes:
 *
 * | Feature              | MGNotificationManager    | MGNotificationSubsystem      |
 * |----------------------|--------------------------|------------------------------|
 * | Purpose              | Basic queue & display    | Specialized game events      |
 * | Notification Types   | Generic FMGNotification  | Achievement, LevelUp, Race   |
 * | Data Richness        | Title + Body + Icon      | Full event data with rewards |
 * | History              | Basic                    | Rich with filtering          |
 * | Best For             | Simple toasts/alerts     | Game milestone celebrations  |
 *
 * Use MGNotificationManager for simple feedback messages.
 * Use MGNotificationSubsystem for milestone events that deserve rich displays.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNotificationSubsystem.generated.h"

class UMGNotificationWidget;
class USoundBase;

// =============================================================================
// Enums and Structs
// =============================================================================

/**
 * Notification priority
 * Determines display order and whether notification can be suppressed
 */
UENUM(BlueprintType)
enum class EMGNotificationPriority : uint8
{
	/** Low priority - informational */
	Low,
	/** Normal priority - standard notifications */
	Normal,
	/** High priority - important events */
	High,
	/** Critical - must see (race finish, unlock) */
	Critical,
	/** System - errors, warnings */
	System
};

/**
 * Notification type
 */
UENUM(BlueprintType)
enum class EMGNotificationType : uint8
{
	/** Generic info */
	Info,
	/** Success/achievement */
	Success,
	/** Warning */
	Warning,
	/** Error */
	Error,
	/** Reward earned */
	Reward,
	/** Level up */
	LevelUp,
	/** Unlock (vehicle, part, etc) */
	Unlock,
	/** Challenge complete */
	ChallengeComplete,
	/** Race result */
	RaceResult,
	/** Multiplayer event */
	Multiplayer,
	/** Season/event */
	Season,
	/** Economy (purchase, currency) */
	Economy,
	/** Social (friend, crew) */
	Social,
	/** System message */
	System
};

/**
 * Notification display style
 */
UENUM(BlueprintType)
enum class EMGNotificationStyle : uint8
{
	/** Small toast at corner */
	Toast,
	/** Banner across top/bottom */
	Banner,
	/** Center popup */
	Popup,
	/** Full screen announcement */
	FullScreen,
	/** Minimal - just icon and text */
	Minimal
};

/**
 * Notification action
 */
USTRUCT(BlueprintType)
struct FMGNotificationAction
{
	GENERATED_BODY()

	/** Action ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActionID;

	/** Button text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ButtonText;

	/** Is primary action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPrimary = false;

	/** Auto-dismiss after action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDismissOnAction = true;
};

/**
 * Reward display data
 */
USTRUCT(BlueprintType)
struct FMGRewardDisplayData
{
	GENERATED_BODY()

	/** Reward name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RewardName;

	/** Reward description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Reward icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Reward quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	/** Reward rarity color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RarityColor = FLinearColor::White;

	/** Is currency reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCurrency = false;

	/** Currency type if applicable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrencyType;
};

/**
 * Notification data
 */
USTRUCT(BlueprintType)
struct FMGNotificationData
{
	GENERATED_BODY()

	/** Unique notification ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid NotificationID;

	/** Notification type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNotificationType Type = EMGNotificationType::Info;

	/** Priority */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNotificationPriority Priority = EMGNotificationPriority::Normal;

	/** Display style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNotificationStyle Style = EMGNotificationStyle::Toast;

	/** Title text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Message text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	/** Icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Duration in seconds (0 = until dismissed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;

	/** Can be dismissed manually */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanDismiss = true;

	/** Should play sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlaySound = true;

	/** Custom sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* CustomSound = nullptr;

	/** Actions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGNotificationAction> Actions;

	/** Reward data (for reward notifications) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRewardDisplayData> Rewards;

	/** Progress value (0-1, for progress notifications) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Progress = -1.0f;

	/** Category for grouping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Category;

	/** Custom data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomData;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Is read */
	UPROPERTY(BlueprintReadOnly)
	bool bIsRead = false;

	FMGNotificationData()
	{
		NotificationID = FGuid::NewGuid();
		Timestamp = FDateTime::Now();
	}
};

/**
 * Achievement notification data
 */
USTRUCT(BlueprintType)
struct FMGAchievementNotification
{
	GENERATED_BODY()

	/** Achievement ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AchievementID;

	/** Achievement name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	/** Achievement description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Achievement icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Gamerscore/points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Points = 0;

	/** Rarity percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RarityPercent = 50.0f;

	/** Is secret achievement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSecret = false;
};

/**
 * Level up notification data
 */
USTRUCT(BlueprintType)
struct FMGLevelUpNotification
{
	GENERATED_BODY()

	/** New level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NewLevel = 1;

	/** Level name/title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LevelTitle;

	/** Rewards unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRewardDisplayData> UnlockedRewards;

	/** New features unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> UnlockedFeatures;
};

/**
 * Race result notification
 */
USTRUCT(BlueprintType)
struct FMGRaceResultNotification
{
	GENERATED_BODY()

	/** Finishing position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 1;

	/** Total racers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRacers = 8;

	/** Finish time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinishTime = 0.0f;

	/** Best lap time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	/** Is new personal best */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	/** Is new track record */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTrackRecord = false;

	/** Cash earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CashEarned = 0;

	/** XP earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPEarned = 0;

	/** Reputation earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationEarned = 0;
};

/**
 * Notification history entry
 */
USTRUCT(BlueprintType)
struct FMGNotificationHistoryEntry
{
	GENERATED_BODY()

	/** Original notification data */
	UPROPERTY(BlueprintReadOnly)
	FMGNotificationData NotificationData;

	/** When it was shown */
	UPROPERTY(BlueprintReadOnly)
	FDateTime ShownTime;

	/** When it was dismissed */
	UPROPERTY(BlueprintReadOnly)
	FDateTime DismissedTime;

	/** Was interacted with */
	UPROPERTY(BlueprintReadOnly)
	bool bWasInteracted = false;

	/** Action taken if any */
	UPROPERTY(BlueprintReadOnly)
	FName ActionTaken;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationQueued, const FMGNotificationData&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationShown, const FMGNotificationData&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNotificationDismissed, const FMGNotificationData&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNotificationAction, const FMGNotificationData&, Notification, FName, ActionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, const FMGAchievementNotification&, Achievement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, const FMGLevelUpNotification&, LevelUpData);

/**
 * Notification Subsystem
 * Manages in-game notifications, alerts, and reward popups
 *
 * Features:
 * - Priority-based queue
 * - Multiple display styles
 * - Reward showcases
 * - Achievement popups
 * - Race result summaries
 * - Notification history
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNotificationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNotificationQueued OnNotificationQueued;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNotificationShown OnNotificationShown;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNotificationDismissed OnNotificationDismissed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNotificationAction OnNotificationAction;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelUp OnLevelUp;

	// ==========================================
	// QUEUE MANAGEMENT
	// ==========================================

	/** Queue a notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	FGuid QueueNotification(const FMGNotificationData& Notification);

	/** Queue simple notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	FGuid QueueSimpleNotification(FText Title, FText Message, EMGNotificationType Type = EMGNotificationType::Info);

	/** Cancel queued notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	bool CancelNotification(FGuid NotificationID);

	/** Dismiss current notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void DismissCurrentNotification();

	/** Dismiss all notifications */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void DismissAllNotifications();

	/** Get queue size */
	UFUNCTION(BlueprintPure, Category = "Notifications")
	int32 GetQueueSize() const { return NotificationQueue.Num(); }

	/** Is showing notification */
	UFUNCTION(BlueprintPure, Category = "Notifications")
	bool IsShowingNotification() const { return bIsShowingNotification; }

	/** Get current notification */
	UFUNCTION(BlueprintPure, Category = "Notifications")
	FMGNotificationData GetCurrentNotification() const { return CurrentNotification; }

	// ==========================================
	// SPECIALIZED NOTIFICATIONS
	// ==========================================

	/** Show reward notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowRewardNotification(FText Title, const TArray<FMGRewardDisplayData>& Rewards);

	/** Show achievement notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowAchievementNotification(const FMGAchievementNotification& Achievement);

	/** Show level up notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowLevelUpNotification(const FMGLevelUpNotification& LevelUpData);

	/** Show race result notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowRaceResultNotification(const FMGRaceResultNotification& RaceResult);

	/** Show unlock notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowUnlockNotification(FText ItemName, FText ItemDescription, UTexture2D* ItemIcon);

	/** Show challenge complete notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowChallengeCompleteNotification(FText ChallengeName, const TArray<FMGRewardDisplayData>& Rewards);

	/** Show currency notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowCurrencyNotification(FName CurrencyType, int32 Amount, bool bIsGain = true);

	/** Show multiplayer notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowMultiplayerNotification(FText PlayerName, FText Action);

	/** Show error notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowErrorNotification(FText Title, FText ErrorMessage);

	/** Show warning notification */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowWarningNotification(FText Title, FText WarningMessage);

	// ==========================================
	// NOTIFICATION INTERACTION
	// ==========================================

	/** Handle notification action */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void HandleNotificationAction(FName ActionID);

	/** Mark notification as read */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void MarkNotificationRead(FGuid NotificationID);

	/** Mark all as read */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void MarkAllAsRead();

	// ==========================================
	// NOTIFICATION HISTORY
	// ==========================================

	/** Get notification history */
	UFUNCTION(BlueprintPure, Category = "History")
	TArray<FMGNotificationHistoryEntry> GetNotificationHistory() const { return NotificationHistory; }

	/** Get unread count */
	UFUNCTION(BlueprintPure, Category = "History")
	int32 GetUnreadCount() const;

	/** Get history by type */
	UFUNCTION(BlueprintPure, Category = "History")
	TArray<FMGNotificationHistoryEntry> GetHistoryByType(EMGNotificationType Type) const;

	/** Get history by category */
	UFUNCTION(BlueprintPure, Category = "History")
	TArray<FMGNotificationHistoryEntry> GetHistoryByCategory(FName Category) const;

	/** Clear history */
	UFUNCTION(BlueprintCallable, Category = "History")
	void ClearHistory();

	// ==========================================
	// SETTINGS
	// ==========================================

	/** Set notifications enabled */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetNotificationsEnabled(bool bEnabled);

	/** Are notifications enabled */
	UFUNCTION(BlueprintPure, Category = "Settings")
	bool AreNotificationsEnabled() const { return bNotificationsEnabled; }

	/** Set sounds enabled */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetSoundsEnabled(bool bEnabled) { bSoundsEnabled = bEnabled; }

	/** Are sounds enabled */
	UFUNCTION(BlueprintPure, Category = "Settings")
	bool AreSoundsEnabled() const { return bSoundsEnabled; }

	/** Set do not disturb */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetDoNotDisturb(bool bEnabled);

	/** Is do not disturb active */
	UFUNCTION(BlueprintPure, Category = "Settings")
	bool IsDoNotDisturbActive() const { return bDoNotDisturb; }

	/** Set notification filter */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMinimumPriority(EMGNotificationPriority MinPriority) { MinimumPriority = MinPriority; }

	/** Get minimum priority */
	UFUNCTION(BlueprintPure, Category = "Settings")
	EMGNotificationPriority GetMinimumPriority() const { return MinimumPriority; }

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get notification type icon */
	UFUNCTION(BlueprintPure, Category = "Utility")
	UTexture2D* GetNotificationTypeIcon(EMGNotificationType Type) const;

	/** Get notification type color */
	UFUNCTION(BlueprintPure, Category = "Utility")
	FLinearColor GetNotificationTypeColor(EMGNotificationType Type) const;

	/** Get priority display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetPriorityDisplayName(EMGNotificationPriority Priority);

	/** Get type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetTypeDisplayName(EMGNotificationType Type);

	/** Format position text */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatPositionText(int32 Position);

protected:
	// ==========================================
	// QUEUE
	// ==========================================

	/** Notification queue */
	UPROPERTY()
	TArray<FMGNotificationData> NotificationQueue;

	/** Current notification */
	UPROPERTY()
	FMGNotificationData CurrentNotification;

	/** Is showing notification */
	bool bIsShowingNotification = false;

	/** Current notification timer */
	float CurrentNotificationTimer = 0.0f;

	// ==========================================
	// HISTORY
	// ==========================================

	/** Notification history */
	UPROPERTY()
	TArray<FMGNotificationHistoryEntry> NotificationHistory;

	/** Max history size */
	int32 MaxHistorySize = 100;

	// ==========================================
	// SETTINGS
	// ==========================================

	/** Are notifications enabled */
	bool bNotificationsEnabled = true;

	/** Are sounds enabled */
	bool bSoundsEnabled = true;

	/** Do not disturb mode */
	bool bDoNotDisturb = false;

	/** Minimum priority to show */
	EMGNotificationPriority MinimumPriority = EMGNotificationPriority::Low;

	// ==========================================
	// DEFAULT SOUNDS
	// ==========================================

	/** Default notification sounds by type */
	UPROPERTY()
	TMap<EMGNotificationType, USoundBase*> DefaultSounds;

	// ==========================================
	// TYPE ICONS
	// ==========================================

	/** Notification type icons */
	UPROPERTY()
	TMap<EMGNotificationType, UTexture2D*> TypeIcons;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Process notification queue */
	void ProcessQueue();

	/** Show notification */
	void ShowNotification(const FMGNotificationData& Notification);

	/** Hide current notification */
	void HideCurrentNotification();

	/** Add to history */
	void AddToHistory(const FMGNotificationData& Notification);

	/** Play notification sound */
	void PlayNotificationSound(const FMGNotificationData& Notification);

	/** Sort queue by priority */
	void SortQueue();

	/** Should show notification */
	bool ShouldShowNotification(const FMGNotificationData& Notification) const;

	/** Get default duration for type */
	float GetDefaultDuration(EMGNotificationType Type) const;

	/** Get default style for type */
	EMGNotificationStyle GetDefaultStyle(EMGNotificationType Type) const;
};
