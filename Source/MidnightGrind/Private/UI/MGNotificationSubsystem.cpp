// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGNotificationSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UMGNotificationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize with empty history
	NotificationHistory.Empty();
	NotificationQueue.Empty();
	bIsShowingNotification = false;
}

void UMGNotificationSubsystem::Deinitialize()
{
	NotificationQueue.Empty();
	NotificationHistory.Empty();

	Super::Deinitialize();
}

void UMGNotificationSubsystem::Tick(float MGDeltaTime)
{
	// Update current notification timer
	if (bIsShowingNotification && CurrentNotification.Duration > 0.0f)
	{
		CurrentNotificationTimer += DeltaTime;

		if (CurrentNotificationTimer >= CurrentNotification.Duration)
		{
			HideCurrentNotification();
		}
	}

	// Process queue if not showing
	if (!bIsShowingNotification)
	{
		ProcessQueue();
	}
}

// ==========================================
// QUEUE MANAGEMENT
// ==========================================

FGuid UMGNotificationSubsystem::QueueNotification(const FMGNotificationData& Notification)
{
	if (!bNotificationsEnabled)
	{
		return FGuid();
	}

	FMGNotificationData NewNotification = Notification;

	// Generate ID if not set
	if (!NewNotification.NotificationID.IsValid())
	{
		NewNotification.NotificationID = FGuid::NewGuid();
	}

	// Set timestamp
	NewNotification.Timestamp = FDateTime::Now();

	// Apply defaults if not set
	if (NewNotification.Duration <= 0.0f)
	{
		NewNotification.Duration = GetDefaultDuration(NewNotification.Type);
	}

	// Check if should show
	if (!ShouldShowNotification(NewNotification))
	{
		return FGuid();
	}

	// Add to queue
	NotificationQueue.Add(NewNotification);

	// Sort by priority
	SortQueue();

	// Broadcast event
	OnNotificationQueued.Broadcast(NewNotification);

	return NewNotification.NotificationID;
}

FGuid UMGNotificationSubsystem::QueueSimpleNotification(FText Title, FText Message, EMGNotificationType Type)
{
	FMGNotificationData Notification;
	Notification.Title = Title;
	Notification.Message = Message;
	Notification.Type = Type;
	Notification.Style = GetDefaultStyle(Type);
	Notification.Priority = EMGNotificationPriority::Normal;

	return QueueNotification(Notification);
}

bool UMGNotificationSubsystem::CancelNotification(FGuid NotificationID)
{
	int32 Index = NotificationQueue.IndexOfByPredicate([NotificationID](const FMGNotificationData& N)
	{
		return N.NotificationID == NotificationID;
	});

	if (Index != INDEX_NONE)
	{
		NotificationQueue.RemoveAt(Index);
		return true;
	}

	return false;
}

void UMGNotificationSubsystem::DismissCurrentNotification()
{
	if (bIsShowingNotification && CurrentNotification.bCanDismiss)
	{
		HideCurrentNotification();
	}
}

void UMGNotificationSubsystem::DismissAllNotifications()
{
	// Clear queue
	NotificationQueue.Empty();

	// Dismiss current
	if (bIsShowingNotification)
	{
		HideCurrentNotification();
	}
}

// ==========================================
// SPECIALIZED NOTIFICATIONS
// ==========================================

void UMGNotificationSubsystem::ShowRewardNotification(FText Title, const TArray<FMGRewardDisplayData>& Rewards)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Reward;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Style = EMGNotificationStyle::Popup;
	Notification.Title = Title;
	Notification.Message = FText::Format(NSLOCTEXT("Notifications", "RewardsEarned", "You earned {0} rewards!"), FText::AsNumber(Rewards.Num()));
	Notification.Rewards = Rewards;
	Notification.Duration = 8.0f;

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowAchievementNotification(const FMGAchievementNotification& Achievement)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Success;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Style = EMGNotificationStyle::Banner;
	Notification.Title = NSLOCTEXT("Notifications", "AchievementUnlocked", "Achievement Unlocked");
	Notification.Message = Achievement.Name;
	Notification.Icon = Achievement.Icon;
	Notification.Duration = 6.0f;
	Notification.Category = TEXT("Achievement");

	// Add custom data
	Notification.CustomData.Add(TEXT("AchievementID"), Achievement.AchievementID.ToString());
	Notification.CustomData.Add(TEXT("Points"), FString::FromInt(Achievement.Points));
	Notification.CustomData.Add(TEXT("Description"), Achievement.Description.ToString());

	QueueNotification(Notification);

	// Broadcast achievement event
	OnAchievementUnlocked.Broadcast(Achievement);
}

void UMGNotificationSubsystem::ShowLevelUpNotification(const FMGLevelUpNotification& LevelUpData)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::LevelUp;
	Notification.Priority = EMGNotificationPriority::Critical;
	Notification.Style = EMGNotificationStyle::FullScreen;
	Notification.Title = NSLOCTEXT("Notifications", "LevelUp", "LEVEL UP!");
	Notification.Message = FText::Format(NSLOCTEXT("Notifications", "NewLevel", "You are now Level {0}"), FText::AsNumber(LevelUpData.NewLevel));
	Notification.Rewards = LevelUpData.UnlockedRewards;
	Notification.Duration = 0.0f; // Requires manual dismiss
	Notification.Category = TEXT("Progression");

	// Add unlocked features to custom data
	for (int32 i = 0; i < LevelUpData.UnlockedFeatures.Num(); i++)
	{
		Notification.CustomData.Add(*FString::Printf(TEXT("Feature%d"), i), LevelUpData.UnlockedFeatures[i].ToString());
	}

	QueueNotification(Notification);

	// Broadcast level up event
	OnLevelUp.Broadcast(LevelUpData);
}

void UMGNotificationSubsystem::ShowRaceResultNotification(const FMGRaceResultNotification& RaceResult)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::RaceResult;
	Notification.Priority = EMGNotificationPriority::Critical;
	Notification.Style = EMGNotificationStyle::FullScreen;

	// Title based on position
	if (RaceResult.Position == 1)
	{
		Notification.Title = NSLOCTEXT("Notifications", "Victory", "VICTORY!");
	}
	else if (RaceResult.Position <= 3)
	{
		Notification.Title = NSLOCTEXT("Notifications", "Podium", "PODIUM FINISH!");
	}
	else
	{
		Notification.Title = NSLOCTEXT("Notifications", "RaceComplete", "RACE COMPLETE");
	}

	Notification.Message = FText::Format(
		NSLOCTEXT("Notifications", "Position", "Position: {0} of {1}"),
		FormatPositionText(RaceResult.Position),
		FText::AsNumber(RaceResult.TotalRacers)
	);

	Notification.Duration = 0.0f; // Manual dismiss

	// Add rewards
	if (RaceResult.CashEarned > 0)
	{
		FMGRewardDisplayData CashReward;
		CashReward.RewardName = NSLOCTEXT("Notifications", "Cash", "Cash");
		CashReward.Quantity = RaceResult.CashEarned;
		CashReward.bIsCurrency = true;
		CashReward.CurrencyType = TEXT("Cash");
		Notification.Rewards.Add(CashReward);
	}

	if (RaceResult.XPEarned > 0)
	{
		FMGRewardDisplayData XPReward;
		XPReward.RewardName = NSLOCTEXT("Notifications", "XP", "Experience");
		XPReward.Quantity = RaceResult.XPEarned;
		XPReward.bIsCurrency = true;
		XPReward.CurrencyType = TEXT("XP");
		Notification.Rewards.Add(XPReward);
	}

	if (RaceResult.ReputationEarned > 0)
	{
		FMGRewardDisplayData RepReward;
		RepReward.RewardName = NSLOCTEXT("Notifications", "Rep", "Reputation");
		RepReward.Quantity = RaceResult.ReputationEarned;
		RepReward.bIsCurrency = true;
		RepReward.CurrencyType = TEXT("Reputation");
		Notification.Rewards.Add(RepReward);
	}

	// Custom data for detailed display
	Notification.CustomData.Add(TEXT("Position"), FString::FromInt(RaceResult.Position));
	Notification.CustomData.Add(TEXT("FinishTime"), FString::SanitizeFloat(RaceResult.FinishTime));
	Notification.CustomData.Add(TEXT("BestLap"), FString::SanitizeFloat(RaceResult.BestLapTime));
	Notification.CustomData.Add(TEXT("PersonalBest"), RaceResult.bIsPersonalBest ? TEXT("true") : TEXT("false"));
	Notification.CustomData.Add(TEXT("TrackRecord"), RaceResult.bIsTrackRecord ? TEXT("true") : TEXT("false"));

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowUnlockNotification(FText ItemName, FText ItemDescription, UTexture2D* ItemIcon)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Unlock;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Style = EMGNotificationStyle::Popup;
	Notification.Title = NSLOCTEXT("Notifications", "NewUnlock", "NEW UNLOCK!");
	Notification.Message = ItemName;
	Notification.Icon = ItemIcon;
	Notification.Duration = 6.0f;
	Notification.Category = TEXT("Unlock");

	Notification.CustomData.Add(TEXT("Description"), ItemDescription.ToString());

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowChallengeCompleteNotification(FText ChallengeName, const TArray<FMGRewardDisplayData>& Rewards)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::ChallengeComplete;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Style = EMGNotificationStyle::Banner;
	Notification.Title = NSLOCTEXT("Notifications", "ChallengeComplete", "CHALLENGE COMPLETE!");
	Notification.Message = ChallengeName;
	Notification.Rewards = Rewards;
	Notification.Duration = 6.0f;
	Notification.Category = TEXT("Challenge");

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowCurrencyNotification(FName CurrencyType, int32 Amount, bool bIsGain)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Economy;
	Notification.Priority = EMGNotificationPriority::Normal;
	Notification.Style = EMGNotificationStyle::Toast;

	FText CurrencyName;
	if (CurrencyType == TEXT("Cash"))
	{
		CurrencyName = NSLOCTEXT("Currency", "Cash", "Cash");
	}
	else if (CurrencyType == TEXT("Gold"))
	{
		CurrencyName = NSLOCTEXT("Currency", "Gold", "Gold");
	}
	else if (CurrencyType == TEXT("Reputation"))
	{
		CurrencyName = NSLOCTEXT("Currency", "Rep", "Reputation");
	}
	else
	{
		CurrencyName = FText::FromName(CurrencyType);
	}

	if (bIsGain)
	{
		Notification.Title = FText::Format(NSLOCTEXT("Notifications", "CurrencyGain", "+{0}"), FText::AsNumber(Amount));
	}
	else
	{
		Notification.Title = FText::Format(NSLOCTEXT("Notifications", "CurrencySpend", "-{0}"), FText::AsNumber(Amount));
	}

	Notification.Message = CurrencyName;
	Notification.Duration = 3.0f;
	Notification.Category = TEXT("Currency");

	Notification.CustomData.Add(TEXT("CurrencyType"), CurrencyType.ToString());
	Notification.CustomData.Add(TEXT("Amount"), FString::FromInt(Amount));
	Notification.CustomData.Add(TEXT("IsGain"), bIsGain ? TEXT("true") : TEXT("false"));

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowMultiplayerNotification(FText PlayerName, FText Action)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Multiplayer;
	Notification.Priority = EMGNotificationPriority::Normal;
	Notification.Style = EMGNotificationStyle::Minimal;
	Notification.Title = PlayerName;
	Notification.Message = Action;
	Notification.Duration = 4.0f;
	Notification.Category = TEXT("Multiplayer");

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowErrorNotification(FText Title, FText ErrorMessage)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Error;
	Notification.Priority = EMGNotificationPriority::System;
	Notification.Style = EMGNotificationStyle::Banner;
	Notification.Title = Title;
	Notification.Message = ErrorMessage;
	Notification.Duration = 8.0f;
	Notification.Category = TEXT("System");

	// Add dismiss action
	FMGNotificationAction DismissAction;
	DismissAction.ActionID = TEXT("Dismiss");
	DismissAction.ButtonText = NSLOCTEXT("Notifications", "OK", "OK");
	DismissAction.bIsPrimary = true;
	Notification.Actions.Add(DismissAction);

	QueueNotification(Notification);
}

void UMGNotificationSubsystem::ShowWarningNotification(FText Title, FText WarningMessage)
{
	FMGNotificationData Notification;
	Notification.Type = EMGNotificationType::Warning;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Style = EMGNotificationStyle::Toast;
	Notification.Title = Title;
	Notification.Message = WarningMessage;
	Notification.Duration = 6.0f;
	Notification.Category = TEXT("System");

	QueueNotification(Notification);
}

// ==========================================
// NOTIFICATION INTERACTION
// ==========================================

void UMGNotificationSubsystem::HandleNotificationAction(FName ActionID)
{
	if (!bIsShowingNotification)
	{
		return;
	}

	// Find action
	const FMGNotificationAction* Action = CurrentNotification.Actions.FindByPredicate([ActionID](const FMGNotificationAction& A)
	{
		return A.ActionID == ActionID;
	});

	if (Action)
	{
		// Broadcast action event
		OnNotificationAction.Broadcast(CurrentNotification, ActionID);

		// Update history
		for (FMGNotificationHistoryEntry& Entry : NotificationHistory)
		{
			if (Entry.NotificationData.NotificationID == CurrentNotification.NotificationID)
			{
				Entry.bWasInteracted = true;
				Entry.ActionTaken = ActionID;
				break;
			}
		}

		// Dismiss if configured
		if (Action->bDismissOnAction)
		{
			HideCurrentNotification();
		}
	}
}

void UMGNotificationSubsystem::MarkNotificationRead(FGuid NotificationID)
{
	for (FMGNotificationHistoryEntry& Entry : NotificationHistory)
	{
		if (Entry.NotificationData.NotificationID == NotificationID)
		{
			Entry.NotificationData.bIsRead = true;
			break;
		}
	}
}

void UMGNotificationSubsystem::MarkAllAsRead()
{
	for (FMGNotificationHistoryEntry& Entry : NotificationHistory)
	{
		Entry.NotificationData.bIsRead = true;
	}
}

// ==========================================
// NOTIFICATION HISTORY
// ==========================================

int32 UMGNotificationSubsystem::GetUnreadCount() const
{
	int32 Count = 0;
	for (const FMGNotificationHistoryEntry& Entry : NotificationHistory)
	{
		if (!Entry.NotificationData.bIsRead)
		{
			Count++;
		}
	}
	return Count;
}

TArray<FMGNotificationHistoryEntry> UMGNotificationSubsystem::GetHistoryByType(EMGNotificationType Type) const
{
	TArray<FMGNotificationHistoryEntry> FilteredHistory;
	for (const FMGNotificationHistoryEntry& Entry : NotificationHistory)
	{
		if (Entry.NotificationData.Type == Type)
		{
			FilteredHistory.Add(Entry);
		}
	}
	return FilteredHistory;
}

TArray<FMGNotificationHistoryEntry> UMGNotificationSubsystem::GetHistoryByCategory(FName Category) const
{
	TArray<FMGNotificationHistoryEntry> FilteredHistory;
	for (const FMGNotificationHistoryEntry& Entry : NotificationHistory)
	{
		if (Entry.NotificationData.Category == Category)
		{
			FilteredHistory.Add(Entry);
		}
	}
	return FilteredHistory;
}

void UMGNotificationSubsystem::ClearHistory()
{
	NotificationHistory.Empty();
}

// ==========================================
// SETTINGS
// ==========================================

void UMGNotificationSubsystem::SetNotificationsEnabled(bool bEnabled)
{
	bNotificationsEnabled = bEnabled;

	if (!bEnabled)
	{
		DismissAllNotifications();
	}
}

void UMGNotificationSubsystem::SetDoNotDisturb(bool bEnabled)
{
	bDoNotDisturb = bEnabled;

	if (bEnabled)
	{
		// Only allow critical notifications
		MinimumPriority = EMGNotificationPriority::Critical;
	}
	else
	{
		MinimumPriority = EMGNotificationPriority::Low;
	}
}

// ==========================================
// UTILITY
// ==========================================

UTexture2D* UMGNotificationSubsystem::GetNotificationTypeIcon(EMGNotificationType Type) const
{
	if (const UTexture2D* const* Icon = TypeIcons.Find(Type))
	{
		return const_cast<UTexture2D*>(*Icon);
	}
	return nullptr;
}

FLinearColor UMGNotificationSubsystem::GetNotificationTypeColor(EMGNotificationType Type) const
{
	switch (Type)
	{
	case EMGNotificationType::Success:
	case EMGNotificationType::ChallengeComplete:
		return FLinearColor(0.2f, 0.8f, 0.3f); // Green

	case EMGNotificationType::Warning:
		return FLinearColor(1.0f, 0.8f, 0.0f); // Yellow

	case EMGNotificationType::Error:
		return FLinearColor(0.9f, 0.2f, 0.2f); // Red

	case EMGNotificationType::Reward:
	case EMGNotificationType::Economy:
		return FLinearColor(1.0f, 0.85f, 0.0f); // Gold

	case EMGNotificationType::LevelUp:
	case EMGNotificationType::Unlock:
		return FLinearColor(0.6f, 0.4f, 1.0f); // Purple

	case EMGNotificationType::RaceResult:
		return FLinearColor(0.3f, 0.7f, 1.0f); // Blue

	case EMGNotificationType::Multiplayer:
	case EMGNotificationType::Social:
		return FLinearColor(0.3f, 0.85f, 0.9f); // Cyan

	case EMGNotificationType::Season:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange

	case EMGNotificationType::System:
		return FLinearColor(0.5f, 0.5f, 0.5f); // Gray

	default:
		return FLinearColor::White;
	}
}

FText UMGNotificationSubsystem::GetPriorityDisplayName(EMGNotificationPriority Priority)
{
	switch (Priority)
	{
	case EMGNotificationPriority::Low:
		return NSLOCTEXT("Notifications", "PriorityLow", "Low");
	case EMGNotificationPriority::Normal:
		return NSLOCTEXT("Notifications", "PriorityNormal", "Normal");
	case EMGNotificationPriority::High:
		return NSLOCTEXT("Notifications", "PriorityHigh", "High");
	case EMGNotificationPriority::Critical:
		return NSLOCTEXT("Notifications", "PriorityCritical", "Critical");
	case EMGNotificationPriority::System:
		return NSLOCTEXT("Notifications", "PrioritySystem", "System");
	default:
		return FText::GetEmpty();
	}
}

FText UMGNotificationSubsystem::GetTypeDisplayName(EMGNotificationType Type)
{
	switch (Type)
	{
	case EMGNotificationType::Info:
		return NSLOCTEXT("Notifications", "TypeInfo", "Info");
	case EMGNotificationType::Success:
		return NSLOCTEXT("Notifications", "TypeSuccess", "Success");
	case EMGNotificationType::Warning:
		return NSLOCTEXT("Notifications", "TypeWarning", "Warning");
	case EMGNotificationType::Error:
		return NSLOCTEXT("Notifications", "TypeError", "Error");
	case EMGNotificationType::Reward:
		return NSLOCTEXT("Notifications", "TypeReward", "Reward");
	case EMGNotificationType::LevelUp:
		return NSLOCTEXT("Notifications", "TypeLevelUp", "Level Up");
	case EMGNotificationType::Unlock:
		return NSLOCTEXT("Notifications", "TypeUnlock", "Unlock");
	case EMGNotificationType::ChallengeComplete:
		return NSLOCTEXT("Notifications", "TypeChallenge", "Challenge");
	case EMGNotificationType::RaceResult:
		return NSLOCTEXT("Notifications", "TypeRaceResult", "Race Result");
	case EMGNotificationType::Multiplayer:
		return NSLOCTEXT("Notifications", "TypeMultiplayer", "Multiplayer");
	case EMGNotificationType::Season:
		return NSLOCTEXT("Notifications", "TypeSeason", "Season");
	case EMGNotificationType::Economy:
		return NSLOCTEXT("Notifications", "TypeEconomy", "Economy");
	case EMGNotificationType::Social:
		return NSLOCTEXT("Notifications", "TypeSocial", "Social");
	case EMGNotificationType::System:
		return NSLOCTEXT("Notifications", "TypeSystem", "System");
	default:
		return FText::GetEmpty();
	}
}

FText UMGNotificationSubsystem::FormatPositionText(int32 Position)
{
	FString Suffix;
	int32 Mod100 = Position % 100;

	if (Mod100 >= 11 && Mod100 <= 13)
	{
		Suffix = TEXT("th");
	}
	else
	{
		switch (Position % 10)
		{
		case 1: Suffix = TEXT("st"); break;
		case 2: Suffix = TEXT("nd"); break;
		case 3: Suffix = TEXT("rd"); break;
		default: Suffix = TEXT("th"); break;
		}
	}

	return FText::FromString(FString::Printf(TEXT("%d%s"), Position, *Suffix));
}

// ==========================================
// INTERNAL
// ==========================================

void UMGNotificationSubsystem::ProcessQueue()
{
	if (NotificationQueue.Num() == 0)
	{
		return;
	}

	// Get next notification
	FMGNotificationData NextNotification = NotificationQueue[0];
	NotificationQueue.RemoveAt(0);

	ShowNotification(NextNotification);
}

void UMGNotificationSubsystem::ShowNotification(const FMGNotificationData& Notification)
{
	CurrentNotification = Notification;
	CurrentNotificationTimer = 0.0f;
	bIsShowingNotification = true;

	// Mark current notification time
	FMGNotificationHistoryEntry HistoryEntry;
	HistoryEntry.NotificationData = Notification;
	HistoryEntry.ShownTime = FDateTime::Now();
	NotificationHistory.Insert(HistoryEntry, 0);

	// Trim history
	while (NotificationHistory.Num() > MaxHistorySize)
	{
		NotificationHistory.RemoveAt(NotificationHistory.Num() - 1);
	}

	// Play sound
	PlayNotificationSound(Notification);

	// Broadcast event
	OnNotificationShown.Broadcast(Notification);
}

void UMGNotificationSubsystem::HideCurrentNotification()
{
	if (!bIsShowingNotification)
	{
		return;
	}

	// Update history with dismiss time
	for (FMGNotificationHistoryEntry& Entry : NotificationHistory)
	{
		if (Entry.NotificationData.NotificationID == CurrentNotification.NotificationID)
		{
			Entry.DismissedTime = FDateTime::Now();
			break;
		}
	}

	// Broadcast dismiss event
	OnNotificationDismissed.Broadcast(CurrentNotification);

	// Reset state
	bIsShowingNotification = false;
	CurrentNotification = FMGNotificationData();
	CurrentNotificationTimer = 0.0f;
}

void UMGNotificationSubsystem::AddToHistory(const FMGNotificationData& Notification)
{
	FMGNotificationHistoryEntry Entry;
	Entry.NotificationData = Notification;
	Entry.ShownTime = FDateTime::Now();

	NotificationHistory.Insert(Entry, 0);

	// Trim history
	while (NotificationHistory.Num() > MaxHistorySize)
	{
		NotificationHistory.RemoveAt(NotificationHistory.Num() - 1);
	}
}

void UMGNotificationSubsystem::PlayNotificationSound(const FMGNotificationData& Notification)
{
	if (!bSoundsEnabled || !Notification.bPlaySound)
	{
		return;
	}

	USoundBase* Sound = Notification.CustomSound;

	// Use default sound if no custom
	if (!Sound)
	{
		if (USoundBase* const* DefaultSound = DefaultSounds.Find(Notification.Type))
		{
			Sound = *DefaultSound;
		}
	}

	if (Sound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}
}

void UMGNotificationSubsystem::SortQueue()
{
	// Sort by priority (higher priority first)
	NotificationQueue.Sort([](const FMGNotificationData& A, const FMGNotificationData& B)
	{
		return static_cast<int32>(A.Priority) > static_cast<int32>(B.Priority);
	});
}

bool UMGNotificationSubsystem::ShouldShowNotification(const FMGNotificationData& Notification) const
{
	// Check if notifications are enabled
	if (!bNotificationsEnabled)
	{
		return false;
	}

	// Check priority filter
	if (static_cast<int32>(Notification.Priority) < static_cast<int32>(MinimumPriority))
	{
		return false;
	}

	// Do not disturb allows only critical
	if (bDoNotDisturb && Notification.Priority != EMGNotificationPriority::Critical)
	{
		return false;
	}

	return true;
}

float UMGNotificationSubsystem::GetDefaultDuration(EMGNotificationType Type) const
{
	switch (Type)
	{
	case EMGNotificationType::Info:
	case EMGNotificationType::Multiplayer:
	case EMGNotificationType::Social:
		return 4.0f;

	case EMGNotificationType::Success:
	case EMGNotificationType::Warning:
	case EMGNotificationType::Economy:
		return 5.0f;

	case EMGNotificationType::Reward:
	case EMGNotificationType::Unlock:
	case EMGNotificationType::ChallengeComplete:
		return 6.0f;

	case EMGNotificationType::Error:
	case EMGNotificationType::System:
		return 8.0f;

	case EMGNotificationType::LevelUp:
	case EMGNotificationType::RaceResult:
		return 0.0f; // Manual dismiss

	case EMGNotificationType::Season:
		return 7.0f;

	default:
		return 5.0f;
	}
}

EMGNotificationStyle UMGNotificationSubsystem::GetDefaultStyle(EMGNotificationType Type) const
{
	switch (Type)
	{
	case EMGNotificationType::Info:
	case EMGNotificationType::Economy:
	case EMGNotificationType::Social:
		return EMGNotificationStyle::Toast;

	case EMGNotificationType::Success:
	case EMGNotificationType::Warning:
	case EMGNotificationType::ChallengeComplete:
	case EMGNotificationType::Season:
		return EMGNotificationStyle::Banner;

	case EMGNotificationType::Error:
	case EMGNotificationType::System:
		return EMGNotificationStyle::Banner;

	case EMGNotificationType::Reward:
	case EMGNotificationType::Unlock:
		return EMGNotificationStyle::Popup;

	case EMGNotificationType::LevelUp:
	case EMGNotificationType::RaceResult:
		return EMGNotificationStyle::FullScreen;

	case EMGNotificationType::Multiplayer:
		return EMGNotificationStyle::Minimal;

	default:
		return EMGNotificationStyle::Toast;
	}
}
