// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGNotificationWidgets.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/PanelWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"

// ==========================================
// UMGNotificationWidgetBase
// ==========================================

void UMGNotificationWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	NotificationSubsystem = GetGameInstance()->GetSubsystem<UMGNotificationSubsystem>();
}

void UMGNotificationWidgetBase::SetNotificationData(const FMGNotificationData& Data)
{
	NotificationData = Data;
	UpdateDisplay();
}

void UMGNotificationWidgetBase::Show()
{
	SetVisibility(ESlateVisibility::Visible);

	if (ShowAnimation)
	{
		PlayAnimation(ShowAnimation);
	}
}

void UMGNotificationWidgetBase::Hide()
{
	if (HideAnimation)
	{
		PlayAnimation(HideAnimation);
		// Wait for animation then remove
		FTimerHandle TimerHandle;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(TimerHandle, this, &UMGNotificationWidgetBase::OnHideAnimationFinished, HideAnimation->GetEndTime(), false);
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
		RemoveFromParent();
	}
}

void UMGNotificationWidgetBase::Dismiss()
{
	if (NotificationSubsystem)
	{
		NotificationSubsystem->DismissCurrentNotification();
	}
}

void UMGNotificationWidgetBase::UpdateDisplay_Implementation()
{
	// Base implementation - override in derived classes
}

void UMGNotificationWidgetBase::OnHideAnimationFinished()
{
	SetVisibility(ESlateVisibility::Collapsed);
	RemoveFromParent();
}

// ==========================================
// UMGToastNotificationWidget
// ==========================================

void UMGToastNotificationWidget::SetNotificationData(const FMGNotificationData& Data)
{
	Super::SetNotificationData(Data);
	ElapsedTime = 0.0f;
}

void UMGToastNotificationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (NotificationData.Duration > 0.0f)
	{
		ElapsedTime += InDeltaTime;
		UpdateTimeProgress();
	}
}

void UMGToastNotificationWidget::UpdateDisplay_Implementation()
{
	if (TitleText)
	{
		TitleText->SetText(NotificationData.Title);
	}

	if (MessageText)
	{
		MessageText->SetText(NotificationData.Message);
	}

	if (IconImage && NotificationData.Icon)
	{
		IconImage->SetBrushFromTexture(NotificationData.Icon);
	}
	else if (IconImage && NotificationSubsystem)
	{
		// Use default type icon
		UTexture2D* TypeIcon = NotificationSubsystem->GetNotificationTypeIcon(NotificationData.Type);
		if (TypeIcon)
		{
			IconImage->SetBrushFromTexture(TypeIcon);
		}
	}

	// Set accent color based on type
	if (AccentImage && NotificationSubsystem)
	{
		FLinearColor TypeColor = NotificationSubsystem->GetNotificationTypeColor(NotificationData.Type);
		AccentImage->SetColorAndOpacity(TypeColor);
	}

	// Initialize progress bar
	if (TimeProgressBar)
	{
		TimeProgressBar->SetPercent(1.0f);
		TimeProgressBar->SetVisibility(NotificationData.Duration > 0.0f ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UMGToastNotificationWidget::UpdateTimeProgress()
{
	if (TimeProgressBar && NotificationData.Duration > 0.0f)
	{
		float Progress = 1.0f - (ElapsedTime / NotificationData.Duration);
		TimeProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

// ==========================================
// UMGBannerNotificationWidget
// ==========================================

void UMGBannerNotificationWidget::SetNotificationData(const FMGNotificationData& Data)
{
	Super::SetNotificationData(Data);
}

void UMGBannerNotificationWidget::UpdateDisplay_Implementation()
{
	if (TitleText)
	{
		TitleText->SetText(NotificationData.Title);
	}

	if (MessageText)
	{
		MessageText->SetText(NotificationData.Message);
	}

	if (IconImage && NotificationData.Icon)
	{
		IconImage->SetBrushFromTexture(NotificationData.Icon);
	}

	// Set background color based on type
	if (BackgroundImage && NotificationSubsystem)
	{
		FLinearColor TypeColor = NotificationSubsystem->GetNotificationTypeColor(NotificationData.Type);
		// Use a darker version for background
		TypeColor = TypeColor * 0.3f;
		TypeColor.A = 0.9f;
		BackgroundImage->SetColorAndOpacity(TypeColor);
	}

	// Create action buttons
	CreateActionButtons();
}

void UMGBannerNotificationWidget::CreateActionButtons_Implementation()
{
	if (!ActionButtonsBox)
	{
		return;
	}

	ActionButtonsBox->ClearChildren();

	// Actions would be created here with proper button widgets
	// This is a blueprint native event so specific button creation
	// should be done in Blueprint subclass
}

void UMGBannerNotificationWidget::OnActionClicked(FName ActionID)
{
	if (NotificationSubsystem)
	{
		NotificationSubsystem->HandleNotificationAction(ActionID);
	}
}

// ==========================================
// UMGPopupNotificationWidget
// ==========================================

void UMGPopupNotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UMGPopupNotificationWidget::OnCloseClicked);
	}
}

void UMGPopupNotificationWidget::SetNotificationData(const FMGNotificationData& Data)
{
	Super::SetNotificationData(Data);
}

void UMGPopupNotificationWidget::UpdateDisplay_Implementation()
{
	if (TitleText)
	{
		TitleText->SetText(NotificationData.Title);
	}

	if (MessageText)
	{
		MessageText->SetText(NotificationData.Message);
	}

	if (IconImage && NotificationData.Icon)
	{
		IconImage->SetBrushFromTexture(NotificationData.Icon);
	}

	// Show close button if can dismiss
	if (CloseButton)
	{
		CloseButton->SetVisibility(NotificationData.bCanDismiss ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	// Populate rewards if any
	PopulateRewards();
}

void UMGPopupNotificationWidget::PopulateRewards_Implementation()
{
	if (!RewardsContainer)
	{
		return;
	}

	RewardsContainer->ClearChildren();

	if (NotificationData.Rewards.Num() == 0)
	{
		RewardsContainer->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	RewardsContainer->SetVisibility(ESlateVisibility::Visible);

	// Create reward items - detailed implementation in Blueprint
	for (const FMGRewardDisplayData& Reward : NotificationData.Rewards)
	{
		if (RewardItemWidgetClass)
		{
			UMGRewardItemWidget* RewardWidget = CreateWidget<UMGRewardItemWidget>(this, RewardItemWidgetClass);
			if (RewardWidget)
			{
				RewardWidget->SetRewardData(Reward);
				RewardsContainer->AddChild(RewardWidget);
			}
		}
	}
}

void UMGPopupNotificationWidget::OnCloseClicked()
{
	Dismiss();
}

// ==========================================
// UMGRewardItemWidget
// ==========================================

void UMGRewardItemWidget::SetRewardData(const FMGRewardDisplayData& Reward)
{
	RewardData = Reward;
	UpdateDisplay();
}

void UMGRewardItemWidget::UpdateDisplay_Implementation()
{
	if (RewardIcon && RewardData.Icon)
	{
		RewardIcon->SetBrushFromTexture(RewardData.Icon);
	}

	if (RewardNameText)
	{
		RewardNameText->SetText(RewardData.RewardName);
	}

	if (QuantityText)
	{
		if (RewardData.Quantity > 1 || RewardData.bIsCurrency)
		{
			QuantityText->SetText(FText::Format(NSLOCTEXT("Notifications", "Quantity", "x{0}"), FText::AsNumber(RewardData.Quantity)));
			QuantityText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			QuantityText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (RarityBorder)
	{
		RarityBorder->SetColorAndOpacity(RewardData.RarityColor);
	}
}

// ==========================================
// UMGFullScreenNotificationWidget
// ==========================================

void UMGFullScreenNotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &UMGFullScreenNotificationWidget::OnContinueClicked);
	}
}

void UMGFullScreenNotificationWidget::SetNotificationData(const FMGNotificationData& Data)
{
	Super::SetNotificationData(Data);

	// Check if this is a victory/good result
	if (Data.Type == EMGNotificationType::RaceResult)
	{
		FString* PosStr = Data.CustomData.Find(TEXT("Position"));
		if (PosStr)
		{
			int32 Position = FCString::Atoi(**PosStr);
			if (Position <= 3)
			{
				PlayCelebration();
			}
		}
	}
	else if (Data.Type == EMGNotificationType::LevelUp)
	{
		PlayCelebration();
	}
}

void UMGFullScreenNotificationWidget::UpdateDisplay_Implementation()
{
	if (TitleText)
	{
		TitleText->SetText(NotificationData.Title);
	}

	if (SubtitleText)
	{
		SubtitleText->SetText(NotificationData.Message);
	}

	// Show position for race results
	if (PositionText && NotificationData.Type == EMGNotificationType::RaceResult)
	{
		FString* PosStr = NotificationData.CustomData.Find(TEXT("Position"));
		if (PosStr)
		{
			int32 Position = FCString::Atoi(**PosStr);
			PositionText->SetText(UMGNotificationSubsystem::FormatPositionText(Position));
			PositionText->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else if (PositionText)
	{
		PositionText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Show new record indicator
	if (NewRecordIndicator)
	{
		FString* PersonalBest = NotificationData.CustomData.Find(TEXT("PersonalBest"));
		FString* TrackRecord = NotificationData.CustomData.Find(TEXT("TrackRecord"));
		bool bShowRecord = (PersonalBest && *PersonalBest == TEXT("true")) || (TrackRecord && *TrackRecord == TEXT("true"));
		NewRecordIndicator->SetVisibility(bShowRecord ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	PopulateRewards();
	PopulateStats();
}

void UMGFullScreenNotificationWidget::PopulateRewards_Implementation()
{
	if (!RewardsContainer)
	{
		return;
	}

	RewardsContainer->ClearChildren();

	if (NotificationData.Rewards.Num() == 0)
	{
		return;
	}

	for (const FMGRewardDisplayData& Reward : NotificationData.Rewards)
	{
		if (RewardItemWidgetClass)
		{
			UMGRewardItemWidget* RewardWidget = CreateWidget<UMGRewardItemWidget>(this, RewardItemWidgetClass);
			if (RewardWidget)
			{
				RewardWidget->SetRewardData(Reward);
				RewardsContainer->AddChild(RewardWidget);
			}
		}
	}
}

void UMGFullScreenNotificationWidget::PopulateStats_Implementation()
{
	if (!StatsContainer)
	{
		return;
	}

	StatsContainer->ClearChildren();

	// Stats would be created as text widgets here
	// Implementation details left for Blueprint subclass
}

void UMGFullScreenNotificationWidget::OnContinueClicked()
{
	Dismiss();
}

void UMGFullScreenNotificationWidget::PlayCelebration_Implementation()
{
	// Play celebration effects - particles, sounds, etc
	// Implementation in Blueprint
}

// ==========================================
// UMGMinimalNotificationWidget
// ==========================================

void UMGMinimalNotificationWidget::SetNotificationData(const FMGNotificationData& Data)
{
	Super::SetNotificationData(Data);
}

void UMGMinimalNotificationWidget::UpdateDisplay_Implementation()
{
	if (MessageText)
	{
		// Combine title and message for minimal display
		FText CombinedText = FText::Format(NSLOCTEXT("Notifications", "MinimalFormat", "{0}: {1}"),
			NotificationData.Title, NotificationData.Message);
		MessageText->SetText(CombinedText);
	}

	if (SmallIcon && NotificationData.Icon)
	{
		SmallIcon->SetBrushFromTexture(NotificationData.Icon);
	}
	else if (SmallIcon && NotificationSubsystem)
	{
		// Apply type color to icon
		FLinearColor TypeColor = NotificationSubsystem->GetNotificationTypeColor(NotificationData.Type);
		SmallIcon->SetColorAndOpacity(TypeColor);
	}
}

// ==========================================
// UMGAchievementNotificationWidget
// ==========================================

void UMGAchievementNotificationWidget::SetAchievementData(const FMGAchievementNotification& Achievement)
{
	AchievementData = Achievement;
	UpdateAchievementDisplay();
	PlayUnlockAnimation();
}

void UMGAchievementNotificationWidget::UpdateAchievementDisplay_Implementation()
{
	if (AchievementIcon && AchievementData.Icon)
	{
		AchievementIcon->SetBrushFromTexture(AchievementData.Icon);
	}

	if (AchievementNameText)
	{
		AchievementNameText->SetText(AchievementData.Name);
	}

	if (AchievementDescText)
	{
		AchievementDescText->SetText(AchievementData.Description);
	}

	if (PointsText)
	{
		PointsText->SetText(FText::Format(NSLOCTEXT("Achievements", "Points", "{0}G"), FText::AsNumber(AchievementData.Points)));
	}

	if (RarityText)
	{
		FText RarityDisplay;
		if (AchievementData.RarityPercent < 5.0f)
		{
			RarityDisplay = NSLOCTEXT("Achievements", "UltraRare", "Ultra Rare");
		}
		else if (AchievementData.RarityPercent < 15.0f)
		{
			RarityDisplay = NSLOCTEXT("Achievements", "Rare", "Rare");
		}
		else if (AchievementData.RarityPercent < 50.0f)
		{
			RarityDisplay = NSLOCTEXT("Achievements", "Uncommon", "Uncommon");
		}
		else
		{
			RarityDisplay = NSLOCTEXT("Achievements", "Common", "Common");
		}

		RarityText->SetText(FText::Format(NSLOCTEXT("Achievements", "RarityFormat", "{0} ({1}%)"),
			RarityDisplay, FText::AsNumber(FMath::RoundToInt(AchievementData.RarityPercent))));
	}
}

void UMGAchievementNotificationWidget::PlayUnlockAnimation_Implementation()
{
	// Play unlock animation and sound effects
	// Implementation in Blueprint
}

// ==========================================
// UMGNotificationContainerWidget
// ==========================================

void UMGNotificationContainerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NotificationSubsystem = GetGameInstance()->GetSubsystem<UMGNotificationSubsystem>();

	if (NotificationSubsystem)
	{
		NotificationSubsystem->OnNotificationShown.AddDynamic(this, &UMGNotificationContainerWidget::OnNotificationShown);
		NotificationSubsystem->OnNotificationDismissed.AddDynamic(this, &UMGNotificationContainerWidget::OnNotificationDismissed);
	}
}

void UMGNotificationContainerWidget::NativeDestruct()
{
	if (NotificationSubsystem)
	{
		NotificationSubsystem->OnNotificationShown.RemoveDynamic(this, &UMGNotificationContainerWidget::OnNotificationShown);
		NotificationSubsystem->OnNotificationDismissed.RemoveDynamic(this, &UMGNotificationContainerWidget::OnNotificationDismissed);
	}

	Super::NativeDestruct();
}

void UMGNotificationContainerWidget::OnNotificationShown(const FMGNotificationData& Notification)
{
	UMGNotificationWidgetBase* Widget = CreateNotificationWidget(Notification);

	if (Widget)
	{
		UPanelWidget* Container = GetContainerForStyle(Notification.Style);
		if (Container)
		{
			Container->AddChild(Widget);
			ActiveWidgets.Add(Widget);
			Widget->Show();
		}
	}
}

void UMGNotificationContainerWidget::OnNotificationDismissed(const FMGNotificationData& Notification)
{
	// Find and remove the widget
	for (int32 i = ActiveWidgets.Num() - 1; i >= 0; i--)
	{
		if (ActiveWidgets[i] && ActiveWidgets[i]->GetNotificationData().NotificationID == Notification.NotificationID)
		{
			ActiveWidgets[i]->Hide();
			ActiveWidgets.RemoveAt(i);
			break;
		}
	}
}

UMGNotificationWidgetBase* UMGNotificationContainerWidget::CreateNotificationWidget(const FMGNotificationData& Notification)
{
	TSubclassOf<UMGNotificationWidgetBase> WidgetClass;

	switch (Notification.Style)
	{
	case EMGNotificationStyle::Toast:
		WidgetClass = ToastWidgetClass;
		break;
	case EMGNotificationStyle::Banner:
		WidgetClass = BannerWidgetClass;
		break;
	case EMGNotificationStyle::Popup:
		WidgetClass = PopupWidgetClass;
		break;
	case EMGNotificationStyle::FullScreen:
		WidgetClass = FullScreenWidgetClass;
		break;
	case EMGNotificationStyle::Minimal:
		WidgetClass = MinimalWidgetClass;
		break;
	default:
		WidgetClass = ToastWidgetClass;
		break;
	}

	if (!WidgetClass)
	{
		return nullptr;
	}

	UMGNotificationWidgetBase* Widget = CreateWidget<UMGNotificationWidgetBase>(this, WidgetClass);
	if (Widget)
	{
		Widget->SetNotificationData(Notification);
	}

	return Widget;
}

UPanelWidget* UMGNotificationContainerWidget::GetContainerForStyle(EMGNotificationStyle Style) const
{
	switch (Style)
	{
	case EMGNotificationStyle::Toast:
		return ToastContainer;
	case EMGNotificationStyle::Banner:
		return TopBannerContainer;
	case EMGNotificationStyle::Popup:
	case EMGNotificationStyle::Minimal:
		return CenterContainer;
	case EMGNotificationStyle::FullScreen:
		return FullScreenContainer;
	default:
		return ToastContainer;
	}
}

void UMGNotificationContainerWidget::RemoveNotificationWidget(UMGNotificationWidgetBase* Widget)
{
	if (Widget)
	{
		ActiveWidgets.Remove(Widget);
		Widget->RemoveFromParent();
	}
}

// ==========================================
// UMGNotificationHistoryWidget
// ==========================================

void UMGNotificationHistoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NotificationSubsystem = GetGameInstance()->GetSubsystem<UMGNotificationSubsystem>();

	RefreshHistory();
}

void UMGNotificationHistoryWidget::RefreshHistory()
{
	if (!NotificationSubsystem)
	{
		return;
	}

	TArray<FMGNotificationHistoryEntry> Entries;

	if (bIsFiltering)
	{
		Entries = NotificationSubsystem->GetHistoryByType(CurrentTypeFilter);
	}
	else
	{
		Entries = NotificationSubsystem->GetNotificationHistory();
	}

	// Update unread count
	if (UnreadCountText)
	{
		int32 UnreadCount = NotificationSubsystem->GetUnreadCount();
		if (UnreadCount > 0)
		{
			UnreadCountText->SetText(FText::Format(NSLOCTEXT("Notifications", "UnreadCount", "{0} unread"), FText::AsNumber(UnreadCount)));
			UnreadCountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			UnreadCountText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Show empty message if no history
	if (EmptyHistoryText)
	{
		EmptyHistoryText->SetVisibility(Entries.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	UpdateDisplay(Entries);
}

void UMGNotificationHistoryWidget::FilterByType(EMGNotificationType Type)
{
	CurrentTypeFilter = Type;
	bIsFiltering = true;
	RefreshHistory();
}

void UMGNotificationHistoryWidget::ClearFilter()
{
	bIsFiltering = false;
	RefreshHistory();
}

void UMGNotificationHistoryWidget::ClearAllHistory()
{
	if (NotificationSubsystem)
	{
		NotificationSubsystem->ClearHistory();
		RefreshHistory();
	}
}

void UMGNotificationHistoryWidget::UpdateDisplay_Implementation(const TArray<FMGNotificationHistoryEntry>& Entries)
{
	if (!HistoryListContainer)
	{
		return;
	}

	HistoryListContainer->ClearChildren();

	// Create history items - detailed implementation in Blueprint
}
