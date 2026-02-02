// Copyright Midnight Grind. All Rights Reserved.

#include "Seasons/MGSeasonWidgets.h"
#include "Seasons/MGSeasonSubsystem.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// UMGSeasonRewardWidget
// ==========================================

void UMGSeasonRewardWidget::SetRewardData(const FMGSeasonReward& Reward, bool bUnlocked, bool bClaim)
{
	RewardData = Reward;
	bIsUnlocked = bUnlocked;
	bCanClaim = bClaim;
	UpdateDisplay();
}

void UMGSeasonRewardWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGSeasonRewardWidget::ClaimReward()
{
	if (bCanClaim)
	{
		OnClaimed.Broadcast(RewardData.Tier, RewardData.bIsPremium);
	}
}

// ==========================================
// UMGSeasonProgressWidget
// ==========================================

void UMGSeasonProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SeasonSubsystem = GI->GetSubsystem<UMGSeasonSubsystem>();
		if (SeasonSubsystem)
		{
			SeasonSubsystem->OnSeasonXPGained.AddDynamic(this, &UMGSeasonProgressWidget::OnXPGained);
			SeasonSubsystem->OnSeasonTierUp.AddDynamic(this, &UMGSeasonProgressWidget::OnTierUp);
		}
	}

	UpdateProgress();
}

void UMGSeasonProgressWidget::NativeDestruct()
{
	if (SeasonSubsystem)
	{
		SeasonSubsystem->OnSeasonXPGained.RemoveDynamic(this, &UMGSeasonProgressWidget::OnXPGained);
		SeasonSubsystem->OnSeasonTierUp.RemoveDynamic(this, &UMGSeasonProgressWidget::OnTierUp);
	}
	Super::NativeDestruct();
}

void UMGSeasonProgressWidget::UpdateProgress()
{
	if (SeasonSubsystem)
	{
		FMGSeasonProgress Progress = SeasonSubsystem->GetSeasonProgress();
		FMGSeasonData Season = SeasonSubsystem->GetCurrentSeason();

		CurrentTier = Progress.CurrentTier;
		CurrentXP = Progress.CurrentXP;
		XPRequired = Season.XPPerTier;
		TierProgress = SeasonSubsystem->GetTierProgress();
	}

	UpdateDisplay();
}

void UMGSeasonProgressWidget::OnXPGained(int32 XPGained, int32 TotalXP)
{
	UpdateProgress();
	PlayXPGainAnimation(XPGained);
}

void UMGSeasonProgressWidget::OnTierUp(int32 NewTier, const TArray<FMGSeasonReward>& Rewards)
{
	CurrentTier = NewTier;
	UpdateDisplay();
	PlayTierUpAnimation(NewTier);
}

void UMGSeasonProgressWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGSeasonProgressWidget::PlayXPGainAnimation_Implementation(int32 Amount)
{
	// Blueprint implementation
}

void UMGSeasonProgressWidget::PlayTierUpAnimation_Implementation(int32 NewTier)
{
	// Blueprint implementation
}

// ==========================================
// UMGSeasonPassWidget
// ==========================================

void UMGSeasonPassWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SeasonSubsystem = GI->GetSubsystem<UMGSeasonSubsystem>();
		if (SeasonSubsystem)
		{
			SeasonSubsystem->OnSeasonChanged.AddDynamic(this, &UMGSeasonPassWidget::OnSeasonChanged);

			SeasonData = SeasonSubsystem->GetCurrentSeason();
			Progress = SeasonSubsystem->GetSeasonProgress();
		}
	}

	UpdateDisplay();
}

void UMGSeasonPassWidget::NativeDestruct()
{
	if (SeasonSubsystem)
	{
		SeasonSubsystem->OnSeasonChanged.RemoveDynamic(this, &UMGSeasonPassWidget::OnSeasonChanged);
	}
	Super::NativeDestruct();
}

void UMGSeasonPassWidget::ScrollToCurrentTier()
{
	// Blueprint implementation - scroll to current tier in list
}

void UMGSeasonPassWidget::ClaimAllRewards()
{
	if (SeasonSubsystem)
	{
		SeasonSubsystem->ClaimAllRewards();
		Progress = SeasonSubsystem->GetSeasonProgress();
		UpdateDisplay();
	}
}

void UMGSeasonPassWidget::PurchasePremiumPass()
{
	ShowPremiumPurchasePrompt();
}

void UMGSeasonPassWidget::OnSeasonChanged(const FMGSeasonData& NewSeason)
{
	SeasonData = NewSeason;
	if (SeasonSubsystem)
	{
		Progress = SeasonSubsystem->GetSeasonProgress();
	}
	UpdateDisplay();
}

void UMGSeasonPassWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGSeasonPassWidget::UpdateTierDisplay_Implementation(int32 Tier)
{
	// Blueprint implementation
}

void UMGSeasonPassWidget::ShowPremiumPurchasePrompt_Implementation()
{
	// Blueprint implementation
}

void UMGSeasonPassWidget::OnRewardClaimed(int32 Tier, bool bPremium)
{
	if (SeasonSubsystem)
	{
		SeasonSubsystem->ClaimTierReward(Tier, bPremium);
		Progress = SeasonSubsystem->GetSeasonProgress();
		UpdateTierDisplay(Tier);
	}
}

// ==========================================
// UMGEventObjectiveWidget
// ==========================================

void UMGEventObjectiveWidget::SetObjectiveData(const FMGEventObjective& Objective)
{
	ObjectiveData = Objective;
	UpdateDisplay();
}

void UMGEventObjectiveWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

// ==========================================
// UMGEventCardWidget
// ==========================================

void UMGEventCardWidget::SetEventData(const FMGEventData& Event)
{
	EventData = Event;
	UpdateTimeRemaining();
	UpdateDisplay();
	UpdateObjectivesDisplay();
}

void UMGEventCardWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGEventCardWidget::UpdateObjectivesDisplay_Implementation()
{
	// Blueprint implementation - create/update objective widgets
}

void UMGEventCardWidget::HandleClick()
{
	OnSelected.Broadcast(EventData);
}

void UMGEventCardWidget::UpdateTimeRemaining()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UMGSeasonSubsystem* Subsystem = GI->GetSubsystem<UMGSeasonSubsystem>())
		{
			TimeRemaining = Subsystem->GetEventTimeRemaining(EventData.EventID);
		}
	}
}

// ==========================================
// UMGEventDetailWidget
// ==========================================

void UMGEventDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SeasonSubsystem = GI->GetSubsystem<UMGSeasonSubsystem>();
		if (SeasonSubsystem)
		{
			SeasonSubsystem->OnEventObjectiveProgress.AddDynamic(this, &UMGEventDetailWidget::OnObjectiveProgress);
			SeasonSubsystem->OnEventCompleted.AddDynamic(this, &UMGEventDetailWidget::OnEventCompleted);
		}
	}
}

void UMGEventDetailWidget::NativeDestruct()
{
	if (SeasonSubsystem)
	{
		SeasonSubsystem->OnEventObjectiveProgress.RemoveDynamic(this, &UMGEventDetailWidget::OnObjectiveProgress);
		SeasonSubsystem->OnEventCompleted.RemoveDynamic(this, &UMGEventDetailWidget::OnEventCompleted);
	}
	Super::NativeDestruct();
}

void UMGEventDetailWidget::ShowEvent(const FMGEventData& Event)
{
	EventData = Event;
	UpdateDisplay();
	SetVisibility(ESlateVisibility::Visible);
}

void UMGEventDetailWidget::JoinEvent()
{
	if (SeasonSubsystem && !EventData.bIsParticipating)
	{
		SeasonSubsystem->JoinEvent(EventData.EventID);
		EventData.bIsParticipating = true;
		UpdateDisplay();
	}
}

void UMGEventDetailWidget::Close()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UMGEventDetailWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGEventDetailWidget::OnObjectiveProgress(const FMGEventData& Event, const FMGEventObjective& Objective)
{
	if (Event.EventID == EventData.EventID)
	{
		EventData = Event;
		UpdateDisplay();
	}
}

void UMGEventDetailWidget::OnEventCompleted(const FMGEventData& Event)
{
	if (Event.EventID == EventData.EventID)
	{
		EventData = Event;
		UpdateDisplay();
		// Show completion celebration
	}
}

// ==========================================
// UMGEventsHubWidget
// ==========================================

void UMGEventsHubWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SeasonSubsystem = GI->GetSubsystem<UMGSeasonSubsystem>();
		if (SeasonSubsystem)
		{
			SeasonSubsystem->OnEventStarted.AddDynamic(this, &UMGEventsHubWidget::OnEventStarted);
			SeasonSubsystem->OnEventEnded.AddDynamic(this, &UMGEventsHubWidget::OnEventEnded);
		}
	}

	RefreshEvents();
}

void UMGEventsHubWidget::NativeDestruct()
{
	if (SeasonSubsystem)
	{
		SeasonSubsystem->OnEventStarted.RemoveDynamic(this, &UMGEventsHubWidget::OnEventStarted);
		SeasonSubsystem->OnEventEnded.RemoveDynamic(this, &UMGEventsHubWidget::OnEventEnded);
	}
	Super::NativeDestruct();
}

void UMGEventsHubWidget::RefreshEvents()
{
	if (!SeasonSubsystem)
	{
		return;
	}

	TArray<FMGEventData> Events;
	switch (CurrentTab)
	{
		case 0: Events = SeasonSubsystem->GetActiveEvents(); break;
		case 1: Events = SeasonSubsystem->GetUpcomingEvents(); break;
		case 2: Events = SeasonSubsystem->GetCompletedEvents(); break;
	}

	UpdateEventsDisplay(Events);
}

void UMGEventsHubWidget::ShowActiveTab()
{
	CurrentTab = 0;
	RefreshEvents();
}

void UMGEventsHubWidget::ShowUpcomingTab()
{
	CurrentTab = 1;
	RefreshEvents();
}

void UMGEventsHubWidget::ShowCompletedTab()
{
	CurrentTab = 2;
	RefreshEvents();
}

void UMGEventsHubWidget::OnEventStarted(const FMGEventData& Event)
{
	if (CurrentTab == 0 || CurrentTab == 1)
	{
		RefreshEvents();
	}
}

void UMGEventsHubWidget::OnEventEnded(const FMGEventData& Event)
{
	RefreshEvents();
}

void UMGEventsHubWidget::UpdateEventsDisplay_Implementation(const TArray<FMGEventData>& Events)
{
	// Ensure enough card widgets
	while (EventCards.Num() < Events.Num())
	{
		UMGEventCardWidget* Card = CreateEventCardWidget();
		if (Card)
		{
			Card->OnSelected.AddDynamic(this, &UMGEventsHubWidget::OnEventCardSelected);
			EventCards.Add(Card);
		}
	}

	// Update visible cards
	for (int32 i = 0; i < Events.Num(); i++)
	{
		if (i < EventCards.Num() && EventCards[i])
		{
			EventCards[i]->SetEventData(Events[i]);
			EventCards[i]->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Hide unused
	for (int32 i = Events.Num(); i < EventCards.Num(); i++)
	{
		if (EventCards[i])
		{
			EventCards[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMGEventsHubWidget::OnEventCardSelected(const FMGEventData& Event)
{
	if (EventDetailWidget)
	{
		EventDetailWidget->ShowEvent(Event);
	}
}

UMGEventCardWidget* UMGEventsHubWidget::CreateEventCardWidget()
{
	if (!EventCardWidgetClass)
	{
		return nullptr;
	}
	return CreateWidget<UMGEventCardWidget>(this, EventCardWidgetClass);
}

// ==========================================
// UMGChallengesWidget
// ==========================================

void UMGChallengesWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		SeasonSubsystem = GI->GetSubsystem<UMGSeasonSubsystem>();
	}

	RefreshChallenges();
}

void UMGChallengesWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateTimers();
}

void UMGChallengesWidget::RefreshChallenges()
{
	if (SeasonSubsystem)
	{
		DailyChallenges = SeasonSubsystem->GetDailyChallenges();
		WeeklyChallenges = SeasonSubsystem->GetWeeklyChallenges();
	}

	UpdateTimers();
	UpdateDisplay();
}

void UMGChallengesWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGChallengesWidget::UpdateTimers()
{
	if (SeasonSubsystem)
	{
		DailyResetTime = SeasonSubsystem->GetDailyResetTime();
		WeeklyResetTime = SeasonSubsystem->GetWeeklyResetTime();
	}
}
