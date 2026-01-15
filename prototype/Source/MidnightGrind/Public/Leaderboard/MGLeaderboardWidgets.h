// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGLeaderboardSubsystem.h"
#include "MGLeaderboardWidgets.generated.h"

class UTextBlock;
class UImage;
class UButton;
class UScrollBox;
class UVerticalBox;
class UHorizontalBox;

/**
 * Single leaderboard entry row widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGLeaderboardEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set entry data */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void SetEntryData(const FMGLeaderboardEntry& Entry, EMGLeaderboardType Type);

	/** Get entry data */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	FMGLeaderboardEntry GetEntryData() const { return EntryData; }

	/** Is this entry selected */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	bool IsSelected() const { return bIsSelected; }

	/** Set selected state */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void SetSelected(bool bSelected);

protected:
	/** Entry data */
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGLeaderboardEntry EntryData;

	/** Leaderboard type */
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	EMGLeaderboardType LeaderboardType;

	/** Is selected */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsSelected = false;

	/** Update visual display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** On selection changed */
	UFUNCTION(BlueprintNativeEvent, Category = "Events")
	void OnSelectionChanged(bool bSelected);
};

/**
 * Leaderboard filter options widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGLeaderboardFilterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Delegate for filter changes */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFilterChanged, EMGLeaderboardType, Type, EMGLeaderboardScope, Scope);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFilterChanged OnFilterChanged;

	/** Set current filters */
	UFUNCTION(BlueprintCallable, Category = "Filter")
	void SetFilters(EMGLeaderboardType Type, EMGLeaderboardScope Scope);

	/** Get current type filter */
	UFUNCTION(BlueprintPure, Category = "Filter")
	EMGLeaderboardType GetTypeFilter() const { return CurrentType; }

	/** Get current scope filter */
	UFUNCTION(BlueprintPure, Category = "Filter")
	EMGLeaderboardScope GetScopeFilter() const { return CurrentScope; }

	/** Set available types */
	UFUNCTION(BlueprintCallable, Category = "Filter")
	void SetAvailableTypes(const TArray<EMGLeaderboardType>& Types);

	/** Set available scopes */
	UFUNCTION(BlueprintCallable, Category = "Filter")
	void SetAvailableScopes(const TArray<EMGLeaderboardScope>& Scopes);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EMGLeaderboardType CurrentType = EMGLeaderboardType::LapTime;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EMGLeaderboardScope CurrentScope = EMGLeaderboardScope::Global;

	UPROPERTY(BlueprintReadOnly, Category = "Config")
	TArray<EMGLeaderboardType> AvailableTypes;

	UPROPERTY(BlueprintReadOnly, Category = "Config")
	TArray<EMGLeaderboardScope> AvailableScopes;

	/** Update filter display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateFilterDisplay();

	/** Called when type button pressed */
	UFUNCTION(BlueprintCallable, Category = "Filter")
	void SelectType(EMGLeaderboardType Type);

	/** Called when scope button pressed */
	UFUNCTION(BlueprintCallable, Category = "Filter")
	void SelectScope(EMGLeaderboardScope Scope);
};

/**
 * Player rank summary widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPlayerRankWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set player data */
	UFUNCTION(BlueprintCallable, Category = "Rank")
	void SetPlayerData(int32 Rank, float Score, int32 TotalPlayers, EMGLeaderboardType Type);

	/** Set personal best data */
	UFUNCTION(BlueprintCallable, Category = "Rank")
	void SetPersonalBest(const FMGPersonalBest& PersonalBest);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 PlayerRank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float PlayerScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 TotalPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float PercentileRank = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	EMGLeaderboardType DisplayType;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGPersonalBest PersonalBestData;

	/** Update rank display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateRankDisplay();
};

/**
 * Track selector for leaderboards
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGTrackSelectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackSelected, FName, TrackID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrackSelected OnTrackSelected;

	/** Set available tracks */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetAvailableTracks(const TArray<FName>& Tracks);

	/** Select track */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SelectTrack(FName TrackID);

	/** Get selected track */
	UFUNCTION(BlueprintPure, Category = "Track")
	FName GetSelectedTrack() const { return SelectedTrack; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FName> AvailableTracks;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	FName SelectedTrack;

	/** Update track display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateTrackDisplay();
};

/**
 * Ghost action widget (race against ghost)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGGhostActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceGhostRequested, const FString&, GhostID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWatchGhostRequested, const FString&, GhostID);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceGhostRequested OnRaceGhostRequested;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWatchGhostRequested OnWatchGhostRequested;

	/** Set ghost data */
	UFUNCTION(BlueprintCallable, Category = "Ghost")
	void SetGhostData(const FMGLeaderboardEntry& Entry);

	/** Is ghost available */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	bool IsGhostAvailable() const { return bGhostAvailable; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGLeaderboardEntry GhostEntry;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bGhostAvailable = false;

	/** Update ghost display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateGhostDisplay();

	/** Race against ghost */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RaceAgainstGhost();

	/** Watch ghost replay */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void WatchGhostReplay();
};

/**
 * Main leaderboard screen widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGLeaderboardScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Show leaderboard for track */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void ShowLeaderboard(FName TrackID, EMGLeaderboardType Type = EMGLeaderboardType::LapTime);

	/** Refresh current leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void RefreshLeaderboard();

	/** Select entry at index */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void SelectEntry(int32 Index);

	/** Get selected entry */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	FMGLeaderboardEntry GetSelectedEntry() const;

	/** Navigate to player's position */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void NavigateToPlayer();

	/** Navigate to top */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void NavigateToTop();

protected:
	/** Current track */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FName CurrentTrack;

	/** Current type */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EMGLeaderboardType CurrentType;

	/** Current scope */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EMGLeaderboardScope CurrentScope;

	/** Current entries */
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FMGLeaderboardEntry> CurrentEntries;

	/** Selected entry index */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 SelectedIndex = -1;

	/** Is loading */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsLoading = false;

	/** Entry widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGLeaderboardEntryWidget> EntryWidgetClass;

	/** Entry widget pool */
	UPROPERTY()
	TArray<UMGLeaderboardEntryWidget*> EntryWidgets;

	/** Leaderboard subsystem reference */
	UPROPERTY()
	UMGLeaderboardSubsystem* LeaderboardSubsystem;

	/** Handle query result */
	UFUNCTION()
	void OnQueryComplete(const FMGLeaderboardResult& Result);

	/** Update entries display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateEntriesDisplay();

	/** Show loading indicator */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void ShowLoading(bool bShow);

	/** On entry selected */
	UFUNCTION(BlueprintNativeEvent, Category = "Events")
	void OnEntrySelected(int32 Index, const FMGLeaderboardEntry& Entry);

	/** On filter changed (from filter widget) */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleFilterChanged(EMGLeaderboardType Type, EMGLeaderboardScope Scope);

	/** On track changed (from track selector) */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleTrackChanged(FName TrackID);

	/** Create entry widget */
	UMGLeaderboardEntryWidget* CreateEntryWidget();
};

/**
 * Post-race leaderboard comparison widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPostRaceLeaderboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Show post-race comparison */
	UFUNCTION(BlueprintCallable, Category = "PostRace")
	void ShowComparison(FName TrackID, float PlayerTime, int32 PlayerPosition);

	/** Set submission result */
	UFUNCTION(BlueprintCallable, Category = "PostRace")
	void SetSubmissionResult(const FMGScoreSubmissionResult& Result);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FName TrackID;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float PlayerTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 PlayerPosition = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGScoreSubmissionResult SubmissionResult;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FMGLeaderboardEntry> NearbyEntries;

	/** Update comparison display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateComparisonDisplay();

	/** Show personal best celebration */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void ShowPersonalBestCelebration();

	/** Show rank improvement */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void ShowRankImprovement(int32 OldRank, int32 NewRank);
};
