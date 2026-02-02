// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGNotificationManager.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGNotificationManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set up tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGNotificationManager::OnTick,
			0.05f, // 20 Hz for smooth animations
			true
		);
	}
}

void UMGNotificationManager::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	Super::Deinitialize();
}

// ==========================================
// NOTIFICATIONS
// ==========================================

void UMGNotificationManager::QueueNotification(const FMGNotification& Notification)
{
	if (!bNotificationsEnabled)
	{
		return;
	}

	// Check do not disturb
	if (bDoNotDisturb && Notification.Priority != EMGNotificationPriority::Critical)
	{
		return;
	}

	FMGNotification NewNotification = Notification;
	NewNotification.NotificationID = FGuid::NewGuid();
	NewNotification.Timestamp = FDateTime::UtcNow();

	NotificationQueue.Add(NewNotification);
	SortQueue();

	OnNotificationQueued.Broadcast(NewNotification);

	// Process queue if nothing showing
	if (!bNotificationActive && GapTimer <= 0.0f)
	{
		ProcessQueue();
	}
}

void UMGNotificationManager::ShowNotificationImmediately(const FMGNotification& Notification)
{
	// Dismiss current if any
	if (bNotificationActive)
	{
		DismissCurrentNotification();
	}

	// Show immediately
	CurrentNotification = Notification;
	CurrentNotification.bShown = true;
	CurrentNotificationTimeRemaining = Notification.Duration;
	bNotificationActive = true;

	OnNotificationShown.Broadcast(CurrentNotification);
}

void UMGNotificationManager::DismissCurrentNotification()
{
	if (!bNotificationActive)
	{
		return;
	}

	CurrentNotification.bDismissed = true;
	bNotificationActive = false;

	// Add to history
	NotificationHistory.Add(CurrentNotification);
	while (NotificationHistory.Num() > MaxHistorySize)
	{
		NotificationHistory.RemoveAt(0);
	}

	OnNotificationDismissed.Broadcast(CurrentNotification);

	// Start gap timer
	GapTimer = NotificationGap;
}

void UMGNotificationManager::DismissNotification(const FGuid& NotificationID)
{
	// Check if it's the current notification
	if (bNotificationActive && CurrentNotification.NotificationID == NotificationID)
	{
		DismissCurrentNotification();
		return;
	}

	// Remove from queue
	NotificationQueue.RemoveAll([&NotificationID](const FMGNotification& N)
	{
		return N.NotificationID == NotificationID;
	});
}

void UMGNotificationManager::ClearAllNotifications()
{
	NotificationQueue.Empty();

	if (bNotificationActive)
	{
		DismissCurrentNotification();
	}
}

// ==========================================
// QUICK NOTIFICATIONS
// ==========================================

void UMGNotificationManager::ShowReward(FText Title, int64 Credits, int32 XP)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Reward;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Title = Title;
	Notification.Amount = Credits;

	if (XP > 0)
	{
		Notification.Body = FText::Format(
			NSLOCTEXT("MG", "RewardBody", "+{0} Credits  |  +{1} XP"),
			FText::AsNumber(Credits),
			FText::AsNumber(XP)
		);
	}
	else
	{
		Notification.Body = FText::Format(
			NSLOCTEXT("MG", "RewardBodyCreditsOnly", "+{0} Credits"),
			FText::AsNumber(Credits)
		);
	}

	Notification.Duration = 4.0f;
	Notification.IconID = FName(TEXT("Credits"));

	QueueNotification(Notification);
}

void UMGNotificationManager::ShowChallengeComplete(FText ChallengeName, int64 RewardCredits)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Challenge;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Title = NSLOCTEXT("MG", "ChallengeComplete", "CHALLENGE COMPLETE");
	Notification.Body = ChallengeName;
	Notification.Amount = RewardCredits;
	Notification.Duration = 5.0f;
	Notification.IconID = FName(TEXT("Challenge"));

	QueueNotification(Notification);
}

void UMGNotificationManager::ShowAchievement(FText AchievementName, FText Description)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Achievement;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Title = NSLOCTEXT("MG", "AchievementUnlocked", "ACHIEVEMENT UNLOCKED");
	Notification.Body = FText::Format(
		NSLOCTEXT("MG", "AchievementFormat", "{0}\n{1}"),
		AchievementName,
		Description
	);
	Notification.Duration = 6.0f;
	Notification.IconID = FName(TEXT("Achievement"));

	QueueNotification(Notification);
}

void UMGNotificationManager::ShowLevelUp(int32 NewLevel)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::LevelUp;
	Notification.Priority = EMGNotificationPriority::Critical;
	Notification.Title = NSLOCTEXT("MG", "LevelUp", "LEVEL UP!");
	Notification.Body = FText::Format(
		NSLOCTEXT("MG", "LevelUpBody", "You are now Level {0}"),
		FText::AsNumber(NewLevel)
	);
	Notification.Amount = NewLevel;
	Notification.Duration = 6.0f;
	Notification.bHasAction = true;
	Notification.ActionText = NSLOCTEXT("MG", "ViewRewards", "View Rewards");
	Notification.ActionID = FName(TEXT("ViewLevelRewards"));
	Notification.IconID = FName(TEXT("LevelUp"));

	// Show immediately for level ups
	ShowNotificationImmediately(Notification);
}

void UMGNotificationManager::ShowUnlock(FText ItemName, FName ItemType)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Unlock;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Title = NSLOCTEXT("MG", "NewUnlock", "NEW UNLOCK");
	Notification.Body = ItemName;
	Notification.ItemID = FName(*ItemName.ToString());
	Notification.Duration = 5.0f;
	Notification.IconID = ItemType;

	QueueNotification(Notification);
}

void UMGNotificationManager::ShowRivalEvent(FText RivalName, FText EventDescription)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Rival;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Title = RivalName;
	Notification.Body = EventDescription;
	Notification.Duration = 4.0f;
	Notification.IconID = FName(TEXT("Rival"));

	QueueNotification(Notification);
}

void UMGNotificationManager::ShowError(FText ErrorMessage)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Error;
	Notification.Priority = EMGNotificationPriority::High;
	Notification.Title = NSLOCTEXT("MG", "Error", "ERROR");
	Notification.Body = ErrorMessage;
	Notification.Duration = 5.0f;
	Notification.IconID = FName(TEXT("Error"));

	QueueNotification(Notification);
}

void UMGNotificationManager::ShowSuccess(FText Message)
{
	FMGNotification Notification;
	Notification.Type = EMGNotificationType::Success;
	Notification.Priority = EMGNotificationPriority::Normal;
	Notification.Title = Message;
	Notification.Duration = 3.0f;
	Notification.IconID = FName(TEXT("Success"));

	QueueNotification(Notification);
}

// ==========================================
// TOASTS
// ==========================================

void UMGNotificationManager::ShowToast(FText Message, EMGNotificationType Type, float Duration)
{
	if (!bToastsEnabled)
	{
		return;
	}

	FMGToast Toast;
	Toast.Message = Message;
	Toast.Type = Type;
	Toast.Duration = Duration;

	ToastQueue.Add(Toast);
}

void UMGNotificationManager::ShowToastFormat(const FText& Format, const FText& Arg1, EMGNotificationType Type)
{
	FText Message = FText::Format(Format, Arg1);
	ShowToast(Message, Type, 3.0f);
}

// ==========================================
// IN-RACE NOTIFICATIONS
// ==========================================

void UMGNotificationManager::ShowPositionChange(int32 OldPosition, int32 NewPosition)
{
	if (NewPosition < OldPosition)
	{
		// Moved up
		ShowToast(
			FText::Format(NSLOCTEXT("MG", "PositionUp", "▲ {0}"), FText::AsNumber(NewPosition)),
			EMGNotificationType::Success,
			2.0f
		);
	}
	else
	{
		// Moved down
		ShowToast(
			FText::Format(NSLOCTEXT("MG", "PositionDown", "▼ {0}"), FText::AsNumber(NewPosition)),
			EMGNotificationType::Warning,
			2.0f
		);
	}
}

void UMGNotificationManager::ShowLapComplete(int32 LapNumber, float LapTime, bool bBestLap)
{
	int32 Minutes = static_cast<int32>(LapTime) / 60;
	int32 Seconds = static_cast<int32>(LapTime) % 60;
	int32 Milliseconds = static_cast<int32>((LapTime - FMath::FloorToFloat(LapTime)) * 1000);

	FText TimeText = FText::Format(
		NSLOCTEXT("MG", "LapTimeFormat", "{0}:{1}.{2}"),
		FText::AsNumber(Minutes),
		FText::Format(NSLOCTEXT("MG", "TwoDigit", "{0}"), FText::AsNumber(Seconds)),
		FText::Format(NSLOCTEXT("MG", "ThreeDigit", "{0}"), FText::AsNumber(Milliseconds))
	);

	if (bBestLap)
	{
		ShowToast(
			FText::Format(NSLOCTEXT("MG", "BestLap", "★ BEST LAP: {0}"), TimeText),
			EMGNotificationType::Success,
			3.0f
		);
	}
	else
	{
		ShowToast(
			FText::Format(NSLOCTEXT("MG", "LapComplete", "LAP {0}: {1}"),
				FText::AsNumber(LapNumber), TimeText),
			EMGNotificationType::Info,
			2.0f
		);
	}
}

void UMGNotificationManager::ShowDriftScore(int32 Score, FText TierName)
{
	ShowToast(
		FText::Format(NSLOCTEXT("MG", "DriftScore", "{0}! +{1}"), TierName, FText::AsNumber(Score)),
		EMGNotificationType::Success,
		2.0f
	);
}

void UMGNotificationManager::ShowCombo(int32 ComboCount, float Multiplier)
{
	ShowToast(
		FText::Format(NSLOCTEXT("MG", "Combo", "{0}x COMBO! ({1}x)"),
			FText::AsNumber(ComboCount),
			FText::AsNumber(Multiplier)),
		EMGNotificationType::Success,
		1.5f
	);
}

void UMGNotificationManager::ShowNearMiss(int32 BonusPoints)
{
	ShowToast(
		FText::Format(NSLOCTEXT("MG", "NearMiss", "NEAR MISS! +{0}"), FText::AsNumber(BonusPoints)),
		EMGNotificationType::Success,
		1.5f
	);
}

// ==========================================
// HISTORY
// ==========================================

TArray<FMGNotification> UMGNotificationManager::GetNotificationHistory(int32 Count) const
{
	TArray<FMGNotification> Result;

	int32 StartIndex = FMath::Max(0, NotificationHistory.Num() - Count);
	for (int32 i = NotificationHistory.Num() - 1; i >= StartIndex; --i)
	{
		Result.Add(NotificationHistory[i]);
	}

	return Result;
}

int32 UMGNotificationManager::GetUnreadCount() const
{
	int32 Count = 0;
	for (const FMGNotification& N : NotificationHistory)
	{
		if (!N.bDismissed)
		{
			Count++;
		}
	}
	return Count;
}

void UMGNotificationManager::MarkAllAsRead()
{
	for (FMGNotification& N : NotificationHistory)
	{
		N.bDismissed = true;
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGNotificationManager::ProcessQueue()
{
	if (NotificationQueue.Num() > 0 && !bNotificationActive)
	{
		ShowNextNotification();
	}
}

void UMGNotificationManager::ShowNextNotification()
{
	if (NotificationQueue.Num() == 0)
	{
		return;
	}

	CurrentNotification = NotificationQueue[0];
	NotificationQueue.RemoveAt(0);

	CurrentNotification.bShown = true;
	CurrentNotificationTimeRemaining = CurrentNotification.Duration;
	bNotificationActive = true;

	OnNotificationShown.Broadcast(CurrentNotification);
}

FLinearColor UMGNotificationManager::GetNotificationColor(EMGNotificationType Type) const
{
	// Y2K neon colors
	switch (Type)
	{
	case EMGNotificationType::Info:
		return FLinearColor(0.0f, 1.0f, 0.976f, 1.0f); // Cyan #00FFF9
	case EMGNotificationType::Success:
		return FLinearColor(0.0f, 1.0f, 0.4f, 1.0f);   // Green
	case EMGNotificationType::Warning:
		return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);   // Yellow #FFFF00
	case EMGNotificationType::Error:
		return FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);   // Red
	case EMGNotificationType::Reward:
		return FLinearColor(1.0f, 0.843f, 0.0f, 1.0f); // Gold
	case EMGNotificationType::Challenge:
		return FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);   // Pink #FF0099
	case EMGNotificationType::Achievement:
		return FLinearColor(0.8f, 0.5f, 1.0f, 1.0f);   // Purple
	case EMGNotificationType::LevelUp:
		return FLinearColor(1.0f, 0.843f, 0.0f, 1.0f); // Gold
	case EMGNotificationType::Unlock:
		return FLinearColor(0.0f, 1.0f, 0.976f, 1.0f); // Cyan
	case EMGNotificationType::Rival:
		return FLinearColor(1.0f, 0.0f, 0.6f, 1.0f);   // Pink
	case EMGNotificationType::Social:
		return FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);   // Light blue
	case EMGNotificationType::System:
		return FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);   // Gray
	default:
		return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void UMGNotificationManager::OnTick()
{
	const float MGDeltaTime = 0.05f;

	// Update gap timer
	if (GapTimer > 0.0f)
	{
		GapTimer -= DeltaTime;
		if (GapTimer <= 0.0f)
		{
			ProcessQueue();
		}
	}

	// Update current notification
	if (bNotificationActive)
	{
		if (CurrentNotification.bAutoDismiss && CurrentNotification.Duration > 0.0f)
		{
			CurrentNotificationTimeRemaining -= DeltaTime;
			if (CurrentNotificationTimeRemaining <= 0.0f)
			{
				DismissCurrentNotification();
			}
		}
	}

	// Update toast
	if (bToastActive)
	{
		ToastTimeRemaining -= DeltaTime;
		if (ToastTimeRemaining <= 0.0f)
		{
			bToastActive = false;

			// Show next toast if any
			if (ToastQueue.Num() > 0)
			{
				FMGToast Toast = ToastQueue[0];
				ToastQueue.RemoveAt(0);
				ToastTimeRemaining = Toast.Duration;
				bToastActive = true;
				// Would trigger UI update here
			}
		}
	}
	else if (ToastQueue.Num() > 0)
	{
		FMGToast Toast = ToastQueue[0];
		ToastQueue.RemoveAt(0);
		ToastTimeRemaining = Toast.Duration;
		bToastActive = true;
		// Would trigger UI update here
	}
}

void UMGNotificationManager::SortQueue()
{
	// Sort by priority (highest first), then by timestamp (oldest first)
	NotificationQueue.Sort([](const FMGNotification& A, const FMGNotification& B)
	{
		if (A.Priority != B.Priority)
		{
			return static_cast<int32>(A.Priority) > static_cast<int32>(B.Priority);
		}
		return A.Timestamp < B.Timestamp;
	});
}
