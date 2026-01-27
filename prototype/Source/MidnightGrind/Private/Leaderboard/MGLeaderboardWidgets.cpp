// Copyright Midnight Grind. All Rights Reserved.

#include "Leaderboard/MGLeaderboardWidgets.h"
#include "Social/MGLeaderboardSubsystem.h"
#include "Kismet/GameplayStatics.h"

// ==========================================
// UMGLeaderboardEntryWidget
// ==========================================

void UMGLeaderboardEntryWidget::SetEntryData(const FMGLeaderboardEntry& Entry, EMGLeaderboardType Type)
{
	EntryData = Entry;
	LeaderboardType = Type;
	UpdateDisplay();
}

void UMGLeaderboardEntryWidget::SetSelected(bool bSelected)
{
	if (bIsSelected != bSelected)
	{
		bIsSelected = bSelected;
		OnSelectionChanged(bSelected);
	}
}

void UMGLeaderboardEntryWidget::UpdateDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGLeaderboardEntryWidget::OnSelectionChanged_Implementation(bool bSelected)
{
	// Blueprint implementation
}

// ==========================================
// UMGLeaderboardFilterWidget
// ==========================================

void UMGLeaderboardFilterWidget::SetFilters(EMGLeaderboardType Type, EMGLeaderboardScope Scope)
{
	CurrentType = Type;
	CurrentScope = Scope;
	UpdateFilterDisplay();
}

void UMGLeaderboardFilterWidget::SetAvailableTypes(const TArray<EMGLeaderboardType>& Types)
{
	AvailableTypes = Types;
	UpdateFilterDisplay();
}

void UMGLeaderboardFilterWidget::SetAvailableScopes(const TArray<EMGLeaderboardScope>& Scopes)
{
	AvailableScopes = Scopes;
	UpdateFilterDisplay();
}

void UMGLeaderboardFilterWidget::UpdateFilterDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGLeaderboardFilterWidget::SelectType(EMGLeaderboardType Type)
{
	if (CurrentType != Type && AvailableTypes.Contains(Type))
	{
		CurrentType = Type;
		UpdateFilterDisplay();
		OnFilterChanged.Broadcast(CurrentType, CurrentScope);
	}
}

void UMGLeaderboardFilterWidget::SelectScope(EMGLeaderboardScope Scope)
{
	if (CurrentScope != Scope && AvailableScopes.Contains(Scope))
	{
		CurrentScope = Scope;
		UpdateFilterDisplay();
		OnFilterChanged.Broadcast(CurrentType, CurrentScope);
	}
}

// ==========================================
// UMGPlayerRankWidget
// ==========================================

void UMGPlayerRankWidget::SetPlayerData(int32 Rank, float Score, int32 TotalPlayers, EMGLeaderboardType Type)
{
	PlayerRank = Rank;
	PlayerScore = Score;
	this->TotalPlayers = TotalPlayers;
	DisplayType = Type;

	if (TotalPlayers > 0)
	{
		PercentileRank = (1.0f - (static_cast<float>(Rank) / static_cast<float>(TotalPlayers))) * 100.0f;
	}
	else
	{
		PercentileRank = 0.0f;
	}

	UpdateRankDisplay();
}

void UMGPlayerRankWidget::SetPersonalBest(const FMGPersonalBest& PersonalBest)
{
	PersonalBestData = PersonalBest;
	UpdateRankDisplay();
}

void UMGPlayerRankWidget::UpdateRankDisplay_Implementation()
{
	// Blueprint implementation
}

// ==========================================
// UMGTrackSelectorWidget
// ==========================================

void UMGTrackSelectorWidget::SetAvailableTracks(const TArray<FName>& Tracks)
{
	AvailableTracks = Tracks;

	// Select first track if none selected
	if (SelectedTrack.IsNone() && Tracks.Num() > 0)
	{
		SelectedTrack = Tracks[0];
	}

	UpdateTrackDisplay();
}

void UMGTrackSelectorWidget::SelectTrack(FName TrackID)
{
	if (SelectedTrack != TrackID && AvailableTracks.Contains(TrackID))
	{
		SelectedTrack = TrackID;
		UpdateTrackDisplay();
		OnTrackSelected.Broadcast(TrackID);
	}
}

void UMGTrackSelectorWidget::UpdateTrackDisplay_Implementation()
{
	// Blueprint implementation
}

// ==========================================
// UMGGhostActionWidget
// ==========================================

void UMGGhostActionWidget::SetGhostData(const FMGLeaderboardEntry& Entry)
{
	GhostEntry = Entry;
	bGhostAvailable = Entry.bHasGhost && !Entry.GhostReplayID.IsEmpty();
	UpdateGhostDisplay();
}

void UMGGhostActionWidget::UpdateGhostDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGGhostActionWidget::RaceAgainstGhost()
{
	if (bGhostAvailable)
	{
		OnRaceGhostRequested.Broadcast(GhostEntry.GhostReplayID);
	}
}

void UMGGhostActionWidget::WatchGhostReplay()
{
	if (bGhostAvailable)
	{
		OnWatchGhostRequested.Broadcast(GhostEntry.GhostReplayID);
	}
}

// ==========================================
// UMGLeaderboardScreenWidget
// ==========================================

void UMGLeaderboardScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get leaderboard subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>();
		if (LeaderboardSubsystem)
		{
			LeaderboardSubsystem->OnLeaderboardQueryComplete.AddDynamic(this, &UMGLeaderboardScreenWidget::OnQueryComplete);
		}
	}
}

void UMGLeaderboardScreenWidget::NativeDestruct()
{
	if (LeaderboardSubsystem)
	{
		LeaderboardSubsystem->OnLeaderboardQueryComplete.RemoveDynamic(this, &UMGLeaderboardScreenWidget::OnQueryComplete);
	}

	Super::NativeDestruct();
}

void UMGLeaderboardScreenWidget::ShowLeaderboard(FName TrackID, EMGLeaderboardType Type)
{
	CurrentTrack = TrackID;
	CurrentType = Type;
	CurrentScope = EMGLeaderboardScope::Global;
	SelectedIndex = -1;

	RefreshLeaderboard();
}

void UMGLeaderboardScreenWidget::RefreshLeaderboard()
{
	if (!LeaderboardSubsystem)
	{
		return;
	}

	bIsLoading = true;
	ShowLoading(true);

	FMGLeaderboardQuery Query;
	Query.Type = CurrentType;
	Query.Scope = CurrentScope;
	Query.TrackID = CurrentTrack;
	Query.StartRank = 1;
	Query.MaxEntries = 50;
	Query.bAroundPlayer = false;

	LeaderboardSubsystem->QueryLeaderboard(Query);
}

void UMGLeaderboardScreenWidget::SelectEntry(int32 Index)
{
	if (Index < 0 || Index >= CurrentEntries.Num())
	{
		return;
	}

	// Deselect previous
	if (SelectedIndex >= 0 && SelectedIndex < EntryWidgets.Num())
	{
		EntryWidgets[SelectedIndex]->SetSelected(false);
	}

	SelectedIndex = Index;

	// Select new
	if (SelectedIndex >= 0 && SelectedIndex < EntryWidgets.Num())
	{
		EntryWidgets[SelectedIndex]->SetSelected(true);
	}

	OnEntrySelected(Index, CurrentEntries[Index]);
}

FMGLeaderboardEntry UMGLeaderboardScreenWidget::GetSelectedEntry() const
{
	if (SelectedIndex >= 0 && SelectedIndex < CurrentEntries.Num())
	{
		return CurrentEntries[SelectedIndex];
	}
	return FMGLeaderboardEntry();
}

void UMGLeaderboardScreenWidget::NavigateToPlayer()
{
	if (!LeaderboardSubsystem)
	{
		return;
	}

	bIsLoading = true;
	ShowLoading(true);

	LeaderboardSubsystem->QueryAroundPlayer(CurrentType, CurrentTrack, 10);
}

void UMGLeaderboardScreenWidget::NavigateToTop()
{
	if (!LeaderboardSubsystem)
	{
		return;
	}

	bIsLoading = true;
	ShowLoading(true);

	LeaderboardSubsystem->QueryTopEntries(CurrentType, CurrentTrack, 50);
}

void UMGLeaderboardScreenWidget::OnQueryComplete(const FMGLeaderboardResult& Result)
{
	bIsLoading = false;
	ShowLoading(false);

	if (!Result.bSuccess)
	{
		// Handle error
		return;
	}

	CurrentEntries = Result.Entries;
	SelectedIndex = -1;

	UpdateEntriesDisplay();

	// Auto-select local player if present
	for (int32 i = 0; i < CurrentEntries.Num(); i++)
	{
		if (CurrentEntries[i].bIsLocalPlayer)
		{
			SelectEntry(i);
			break;
		}
	}
}

void UMGLeaderboardScreenWidget::UpdateEntriesDisplay_Implementation()
{
	// Ensure we have enough widgets
	while (EntryWidgets.Num() < CurrentEntries.Num())
	{
		UMGLeaderboardEntryWidget* Widget = CreateEntryWidget();
		if (Widget)
		{
			EntryWidgets.Add(Widget);
		}
	}

	// Update visible widgets
	for (int32 i = 0; i < CurrentEntries.Num(); i++)
	{
		if (i < EntryWidgets.Num() && EntryWidgets[i])
		{
			EntryWidgets[i]->SetEntryData(CurrentEntries[i], CurrentType);
			EntryWidgets[i]->SetVisibility(ESlateVisibility::Visible);
		}
	}

	// Hide unused widgets
	for (int32 i = CurrentEntries.Num(); i < EntryWidgets.Num(); i++)
	{
		if (EntryWidgets[i])
		{
			EntryWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMGLeaderboardScreenWidget::ShowLoading_Implementation(bool bShow)
{
	// Blueprint implementation
}

void UMGLeaderboardScreenWidget::OnEntrySelected_Implementation(int32 Index, const FMGLeaderboardEntry& Entry)
{
	// Blueprint implementation
}

void UMGLeaderboardScreenWidget::HandleFilterChanged(EMGLeaderboardType Type, EMGLeaderboardScope Scope)
{
	CurrentType = Type;
	CurrentScope = Scope;
	RefreshLeaderboard();
}

void UMGLeaderboardScreenWidget::HandleTrackChanged(FName TrackID)
{
	CurrentTrack = TrackID;
	RefreshLeaderboard();
}

UMGLeaderboardEntryWidget* UMGLeaderboardScreenWidget::CreateEntryWidget()
{
	if (!EntryWidgetClass)
	{
		return nullptr;
	}

	return CreateWidget<UMGLeaderboardEntryWidget>(this, EntryWidgetClass);
}

// ==========================================
// UMGPostRaceLeaderboardWidget
// ==========================================

void UMGPostRaceLeaderboardWidget::ShowComparison(FName InTrackID, float InPlayerTime, int32 InPlayerPosition)
{
	TrackID = InTrackID;
	PlayerTime = InPlayerTime;
	PlayerPosition = InPlayerPosition;

	// Query nearby entries would happen here
	UpdateComparisonDisplay();
}

void UMGPostRaceLeaderboardWidget::SetSubmissionResult(const FMGScoreSubmissionResult& Result)
{
	SubmissionResult = Result;

	UpdateComparisonDisplay();

	if (Result.bIsPersonalBest)
	{
		ShowPersonalBestCelebration();
	}

	if (Result.RankImprovement > 0)
	{
		ShowRankImprovement(Result.OldRank, Result.NewRank);
	}
}

void UMGPostRaceLeaderboardWidget::UpdateComparisonDisplay_Implementation()
{
	// Blueprint implementation
}

void UMGPostRaceLeaderboardWidget::ShowPersonalBestCelebration_Implementation()
{
	// Blueprint implementation - play celebration animation
}

void UMGPostRaceLeaderboardWidget::ShowRankImprovement_Implementation(int32 OldRank, int32 NewRank)
{
	// Blueprint implementation - show rank change animation
}
