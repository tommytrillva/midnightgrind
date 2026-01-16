// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNotificationSubsystem.generated.h"

class UMGNotificationWidget;
class USoundBase;

/**
 * Notification priority
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
