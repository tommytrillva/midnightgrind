// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGRaceOverlayWidget.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UMGRaceOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Subscribe to HUD Subsystem notification events if available
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			HUDSubsystem->OnNotificationAdded.AddDynamic(this, &UMGRaceOverlayWidget::OnHUDNotificationAdded);
			HUDSubsystem->OnNotificationRemoved.AddDynamic(this, &UMGRaceOverlayWidget::OnHUDNotificationRemoved);
			HUDSubsystem->OnAllNotificationsCleared.AddDynamic(this, &UMGRaceOverlayWidget::OnHUDNotificationsCleared);
		}
	}
}

void UMGRaceOverlayWidget::NativeDestruct()
{
	// Unsubscribe from HUD Subsystem events
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			HUDSubsystem->OnNotificationAdded.RemoveDynamic(this, &UMGRaceOverlayWidget::OnHUDNotificationAdded);
			HUDSubsystem->OnNotificationRemoved.RemoveDynamic(this, &UMGRaceOverlayWidget::OnHUDNotificationRemoved);
			HUDSubsystem->OnAllNotificationsCleared.RemoveDynamic(this, &UMGRaceOverlayWidget::OnHUDNotificationsCleared);
		}
	}

	Super::NativeDestruct();
}

void UMGRaceOverlayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateCountdown(InDeltaTime);
	UpdateNotifications(InDeltaTime);
	UpdateWrongWayFlash(InDeltaTime);
}

// ==========================================
// COUNTDOWN
// ==========================================

void UMGRaceOverlayWidget::StartCountdown(int32 StartValue, float IntervalSeconds)
{
	CountdownState.bActive = true;
	CountdownState.CurrentValue = StartValue;
	CountdownState.Timer = 0.0f;
	CountdownState.IntervalTime = IntervalSeconds;

	// Show first value
	OnCountdownValueChanged(StartValue);
	OnCountdownTick.Broadcast(StartValue);

	PlaySound(CountdownTickSound);
}

void UMGRaceOverlayWidget::CancelCountdown()
{
	CountdownState.bActive = false;
}

// ==========================================
// NOTIFICATIONS
// ==========================================

int32 UMGRaceOverlayWidget::ShowNotification(const FMGNotificationData& Data)
{
	FMGNotificationData NewData = Data;
	NewData.NotificationID = NextNotificationID++;
	NewData.QueuedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Check if we need to remove old notifications
	while (ActiveNotifications.Num() >= MaxVisibleNotifications)
	{
		// Remove lowest priority or oldest
		int32 RemoveIndex = 0;
		EMGNotificationPriority LowestPriority = EMGNotificationPriority::Critical;

		for (int32 i = 0; i < ActiveNotifications.Num(); ++i)
		{
			if (ActiveNotifications[i].Priority < LowestPriority)
			{
				LowestPriority = ActiveNotifications[i].Priority;
				RemoveIndex = i;
			}
			else if (ActiveNotifications[i].Priority == LowestPriority)
			{
				// Same priority, compare time
				if (ActiveNotifications[i].QueuedTime < ActiveNotifications[RemoveIndex].QueuedTime)
				{
					RemoveIndex = i;
				}
			}
		}

		HideNotification(ActiveNotifications[RemoveIndex].NotificationID);
	}

	ActiveNotifications.Add(NewData);
	DisplayNotification(NewData);
	OnNotificationShown.Broadcast(NewData);

	if (NewData.Sound)
	{
		PlaySound(NewData.Sound);
	}

	return NewData.NotificationID;
}

int32 UMGRaceOverlayWidget::ShowTextNotification(const FText& Text, float Duration, FLinearColor Color)
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::Generic;
	Data.Priority = EMGNotificationPriority::Medium;
	Data.MainText = Text;
	Data.Color = Color;
	Data.Duration = Duration;

	return ShowNotification(Data);
}

void UMGRaceOverlayWidget::HideNotification(int32 NotificationID)
{
	int32 Index = ActiveNotifications.IndexOfByPredicate([NotificationID](const FMGNotificationData& Data)
	{
		return Data.NotificationID == NotificationID;
	});

	if (Index != INDEX_NONE)
	{
		ActiveNotifications.RemoveAt(Index);
		RemoveNotification(NotificationID);
		OnNotificationHidden.Broadcast(NotificationID);
	}
}

void UMGRaceOverlayWidget::ClearAllNotifications()
{
	for (const FMGNotificationData& Data : ActiveNotifications)
	{
		RemoveNotification(Data.NotificationID);
		OnNotificationHidden.Broadcast(Data.NotificationID);
	}
	ActiveNotifications.Empty();
}

int32 UMGRaceOverlayWidget::GetActiveNotificationCount() const
{
	return ActiveNotifications.Num();
}

// ==========================================
// POSITION CHANGES
// ==========================================

void UMGRaceOverlayWidget::ShowPositionChange(int32 OldPosition, int32 NewPosition)
{
	FMGNotificationData Data;
	Data.Priority = EMGNotificationPriority::High;
	Data.Duration = 1.5f;

	if (NewPosition < OldPosition)
	{
		// Position gained
		Data.Type = EMGNotificationType::PositionGain;
		Data.MainText = FText::Format(NSLOCTEXT("RaceOverlay", "PositionUp", "+{0}"), FText::AsNumber(OldPosition - NewPosition));
		Data.SubText = FText::Format(NSLOCTEXT("RaceOverlay", "NowPosition", "Now P{0}"), FText::AsNumber(NewPosition));
		Data.Color = PositionGainColor;
		Data.Sound = PositionGainSound;
	}
	else
	{
		// Position lost
		Data.Type = EMGNotificationType::PositionLoss;
		Data.MainText = FText::Format(NSLOCTEXT("RaceOverlay", "PositionDown", "-{0}"), FText::AsNumber(NewPosition - OldPosition));
		Data.SubText = FText::Format(NSLOCTEXT("RaceOverlay", "NowPosition", "Now P{0}"), FText::AsNumber(NewPosition));
		Data.Color = PositionLossColor;
		Data.Sound = PositionLossSound;
	}

	ShowNotification(Data);
}

// ==========================================
// LAP NOTIFICATIONS
// ==========================================

void UMGRaceOverlayWidget::ShowLapComplete(int32 LapNumber, float LapTime, bool bIsBestLap)
{
	FMGNotificationData Data;
	Data.Duration = 2.0f;

	if (bIsBestLap)
	{
		Data.Type = EMGNotificationType::BestLap;
		Data.Priority = EMGNotificationPriority::High;
		Data.MainText = NSLOCTEXT("RaceOverlay", "BestLap", "BEST LAP!");
		Data.SubText = FormatTime(LapTime);
		Data.Color = BestLapColor;
		Data.Sound = BestLapSound;
	}
	else
	{
		Data.Type = EMGNotificationType::LapComplete;
		Data.Priority = EMGNotificationPriority::Medium;
		Data.MainText = FText::Format(NSLOCTEXT("RaceOverlay", "LapComplete", "LAP {0}"), FText::AsNumber(LapNumber));
		Data.SubText = FormatTime(LapTime);
		Data.Color = FLinearColor::White;
	}

	ShowNotification(Data);
}

void UMGRaceOverlayWidget::ShowFinalLap()
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::FinalLap;
	Data.Priority = EMGNotificationPriority::Critical;
	Data.MainText = NSLOCTEXT("RaceOverlay", "FinalLap", "FINAL LAP!");
	Data.Color = FinalLapColor;
	Data.Duration = 2.5f;
	Data.Sound = FinalLapSound;

	ShowNotification(Data);
}

// ==========================================
// BONUS POPUPS
// ==========================================

void UMGRaceOverlayWidget::ShowNearMissBonus(int32 Points)
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::NearMiss;
	Data.Priority = EMGNotificationPriority::Medium;
	Data.MainText = NSLOCTEXT("RaceOverlay", "NearMiss", "NEAR MISS!");
	Data.SubText = FText::Format(NSLOCTEXT("RaceOverlay", "BonusPoints", "+{0}"), FText::AsNumber(Points));
	Data.Color = NearMissColor;
	Data.Duration = 1.0f;

	ShowNotification(Data);
}

void UMGRaceOverlayWidget::ShowDriftScore(int32 Score, float Multiplier, int32 ChainCount)
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::DriftBonus;
	Data.Priority = EMGNotificationPriority::Medium;

	if (Multiplier > 1.0f)
	{
		Data.MainText = FText::Format(NSLOCTEXT("RaceOverlay", "DriftScoreMulti", "DRIFT x{0}"), FText::AsNumber(FMath::FloorToInt(Multiplier * 10.0f) / 10.0f));
	}
	else
	{
		Data.MainText = NSLOCTEXT("RaceOverlay", "DriftScore", "DRIFT!");
	}

	Data.SubText = FText::Format(NSLOCTEXT("RaceOverlay", "ScoreValue", "{0}"), FText::AsNumber(Score));
	Data.Color = DriftScoreColor;
	Data.Duration = 1.0f;

	ShowNotification(Data);
}

void UMGRaceOverlayWidget::ShowBonus(const FText& BonusName, int32 Points, FLinearColor Color)
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::Generic;
	Data.Priority = EMGNotificationPriority::Medium;
	Data.MainText = BonusName;
	Data.SubText = FText::Format(NSLOCTEXT("RaceOverlay", "BonusPoints", "+{0}"), FText::AsNumber(Points));
	Data.Color = Color;
	Data.Duration = 1.0f;

	ShowNotification(Data);
}

// ==========================================
// WARNINGS
// ==========================================

void UMGRaceOverlayWidget::ShowWrongWay(bool bShow)
{
	if (bShowingWrongWay == bShow)
	{
		return;
	}

	bShowingWrongWay = bShow;
	WrongWayFlashTimer = 0.0f;

	UpdateWrongWayDisplay(bShow);
}

// ==========================================
// RACE END
// ==========================================

void UMGRaceOverlayWidget::ShowRaceFinish(int32 FinalPosition, float TotalTime, bool bNewRecord)
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::RaceFinish;
	Data.Priority = EMGNotificationPriority::Critical;
	Data.Duration = 5.0f;

	Data.MainText = FText::Format(NSLOCTEXT("RaceOverlay", "FinishedPosition", "FINISHED {0}{1}"),
		FText::AsNumber(FinalPosition),
		GetOrdinalSuffix(FinalPosition));

	Data.SubText = FormatTime(TotalTime);

	if (FinalPosition == 1)
	{
		Data.Color = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f); // Gold
	}
	else if (FinalPosition <= 3)
	{
		Data.Color = FLinearColor(0.753f, 0.753f, 0.753f, 1.0f); // Silver
	}
	else
	{
		Data.Color = FLinearColor::White;
	}

	ShowNotification(Data);
	DisplayRaceFinish(FinalPosition, TotalTime, bNewRecord);

	if (bNewRecord)
	{
		// Show new record notification after a delay
		FMGNotificationData RecordData;
		RecordData.Type = EMGNotificationType::NewRecord;
		RecordData.Priority = EMGNotificationPriority::Critical;
		RecordData.MainText = NSLOCTEXT("RaceOverlay", "NewRecord", "NEW RECORD!");
		RecordData.Color = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
		RecordData.Duration = 3.0f;

		// Would delay this slightly in practice
		ShowNotification(RecordData);
	}
}

void UMGRaceOverlayWidget::ShowVictory()
{
	FMGNotificationData Data;
	Data.Type = EMGNotificationType::RaceFinish;
	Data.Priority = EMGNotificationPriority::Critical;
	Data.MainText = NSLOCTEXT("RaceOverlay", "Victory", "VICTORY!");
	Data.Color = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f);
	Data.Duration = 3.0f;

	ShowNotification(Data);
}

// ==========================================
// BLUEPRINT IMPLEMENTABLE DEFAULTS
// ==========================================

void UMGRaceOverlayWidget::OnCountdownValueChanged_Implementation(int32 NewValue)
{
	// Override in Blueprint
}

void UMGRaceOverlayWidget::OnCountdownGo_Implementation()
{
	// Override in Blueprint
}

void UMGRaceOverlayWidget::DisplayNotification_Implementation(const FMGNotificationData& Data)
{
	// Override in Blueprint
}

void UMGRaceOverlayWidget::RemoveNotification_Implementation(int32 NotificationID)
{
	// Override in Blueprint
}

void UMGRaceOverlayWidget::UpdateWrongWayDisplay_Implementation(bool bShow)
{
	// Override in Blueprint
}

void UMGRaceOverlayWidget::DisplayRaceFinish_Implementation(int32 Position, float Time, bool bNewRecord)
{
	// Override in Blueprint
}

// ==========================================
// INTERNAL
// ==========================================

void UMGRaceOverlayWidget::UpdateCountdown(float MGDeltaTime)
{
	if (!CountdownState.bActive)
	{
		return;
	}

	CountdownState.Timer += DeltaTime;

	if (CountdownState.Timer >= CountdownState.IntervalTime)
	{
		CountdownState.Timer = 0.0f;
		CountdownState.CurrentValue--;

		if (CountdownState.CurrentValue > 0)
		{
			OnCountdownValueChanged(CountdownState.CurrentValue);
			OnCountdownTick.Broadcast(CountdownState.CurrentValue);
			PlaySound(CountdownTickSound);
		}
		else if (CountdownState.CurrentValue == 0)
		{
			// Show "GO!"
			OnCountdownGo();
			PlaySound(CountdownGoSound);
		}
		else
		{
			// Countdown complete
			CountdownState.bActive = false;
			OnCountdownComplete.Broadcast();
		}
	}
}

void UMGRaceOverlayWidget::UpdateNotifications(float MGDeltaTime)
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Check for expired notifications
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		const FMGNotificationData& Data = ActiveNotifications[i];
		float ElapsedTime = CurrentTime - Data.QueuedTime;

		if (ElapsedTime >= Data.Duration)
		{
			HideNotification(Data.NotificationID);
		}
	}
}

void UMGRaceOverlayWidget::UpdateWrongWayFlash(float MGDeltaTime)
{
	if (!bShowingWrongWay)
	{
		return;
	}

	WrongWayFlashTimer += DeltaTime;

	// Flash at 2Hz
	float FlashPeriod = 0.5f;
	bool bFlashOn = FMath::Fmod(WrongWayFlashTimer, FlashPeriod) < (FlashPeriod * 0.5f);

	// Could update flash visibility here
}

void UMGRaceOverlayWidget::PlaySound(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}
}

FText UMGRaceOverlayWidget::FormatTime(float TimeSeconds) const
{
	int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeSeconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(Seconds);
	int32 Milliseconds = FMath::FloorToInt((Seconds - WholeSeconds) * 1000.0f);

	return FText::FromString(FString::Printf(TEXT("%d:%02d.%03d"), Minutes, WholeSeconds, Milliseconds));
}

FText UMGRaceOverlayWidget::GetOrdinalSuffix(int32 Number) const
{
	if (Number >= 11 && Number <= 13)
	{
		return NSLOCTEXT("RaceOverlay", "OrdinalTh", "th");
	}

	switch (Number % 10)
	{
	case 1:
		return NSLOCTEXT("RaceOverlay", "OrdinalSt", "st");
	case 2:
		return NSLOCTEXT("RaceOverlay", "OrdinalNd", "nd");
	case 3:
		return NSLOCTEXT("RaceOverlay", "OrdinalRd", "rd");
	default:
		return NSLOCTEXT("RaceOverlay", "OrdinalTh", "th");
	}
}

// ==========================================
// HUD SUBSYSTEM INTEGRATION
// ==========================================

void UMGRaceOverlayWidget::OnHUDNotificationAdded(const FMGHUDNotification& HUDNotification)
{
	// Convert HUD notification to overlay notification format and display
	FMGNotificationData OverlayData = ConvertHUDNotification(HUDNotification);
	ShowNotification(OverlayData);
}

void UMGRaceOverlayWidget::OnHUDNotificationRemoved(int32 NotificationID)
{
	// Note: The overlay manages its own notification lifetimes,
	// so we don't necessarily need to force remove here unless
	// the HUD subsystem explicitly dismisses a notification early
	HideNotification(NotificationID);
}

void UMGRaceOverlayWidget::OnHUDNotificationsCleared()
{
	ClearAllNotifications();
}

FMGNotificationData UMGRaceOverlayWidget::ConvertHUDNotification(const FMGHUDNotification& HUDNotification) const
{
	FMGNotificationData OverlayData;

	// Map fields
	OverlayData.MainText = HUDNotification.Message;
	OverlayData.Color = HUDNotification.Color;
	OverlayData.Duration = HUDNotification.Duration;
	OverlayData.NotificationID = HUDNotification.NotificationID;
	OverlayData.QueuedTime = HUDNotification.CreationTime;

	// Map priority - HUD uses EMGHUDNotificationPriority, overlay uses EMGNotificationPriority
	switch (HUDNotification.Priority)
	{
	case EMGHUDNotificationPriority::Low:
		OverlayData.Priority = EMGNotificationPriority::Low;
		break;
	case EMGHUDNotificationPriority::Normal:
		OverlayData.Priority = EMGNotificationPriority::Medium;
		break;
	case EMGHUDNotificationPriority::High:
		OverlayData.Priority = EMGNotificationPriority::High;
		break;
	case EMGHUDNotificationPriority::Critical:
		OverlayData.Priority = EMGNotificationPriority::Critical;
		break;
	}

	// Determine notification type based on category
	FName Category = HUDNotification.Category;
	if (Category == FName("PositionChange"))
	{
		// Check color to determine gain vs loss
		if (HUDNotification.Color.G > HUDNotification.Color.R)
		{
			OverlayData.Type = EMGNotificationType::PositionGain;
		}
		else
		{
			OverlayData.Type = EMGNotificationType::PositionLoss;
		}
	}
	else if (Category == FName("LapComplete"))
	{
		// Check for final lap or best lap by examining the message
		FString MessageStr = HUDNotification.Message.ToString().ToUpper();
		if (MessageStr.Contains(TEXT("FINAL")))
		{
			OverlayData.Type = EMGNotificationType::FinalLap;
		}
		else if (MessageStr.Contains(TEXT("BEST")))
		{
			OverlayData.Type = EMGNotificationType::BestLap;
		}
		else
		{
			OverlayData.Type = EMGNotificationType::LapComplete;
		}
	}
	else if (Category == FName("Countdown"))
	{
		FString MessageStr = HUDNotification.Message.ToString().ToUpper();
		if (MessageStr.Contains(TEXT("GO")))
		{
			OverlayData.Type = EMGNotificationType::RaceStart;
		}
		else
		{
			OverlayData.Type = EMGNotificationType::Countdown;
		}
	}
	else if (Category == FName("WrongWay"))
	{
		OverlayData.Type = EMGNotificationType::WrongWay;
	}
	else if (Category == FName("RaceFinish") || Category == FName("RaceResult"))
	{
		OverlayData.Type = EMGNotificationType::RaceFinish;
	}
	else if (Category == FName("Bonus"))
	{
		FString MessageStr = HUDNotification.Message.ToString().ToUpper();
		if (MessageStr.Contains(TEXT("NEAR MISS")))
		{
			OverlayData.Type = EMGNotificationType::NearMiss;
		}
		else
		{
			OverlayData.Type = EMGNotificationType::Generic;
		}
	}
	else if (Category == FName("DriftScore"))
	{
		OverlayData.Type = EMGNotificationType::DriftBonus;
	}
	else
	{
		OverlayData.Type = EMGNotificationType::Generic;
	}

	return OverlayData;
}
