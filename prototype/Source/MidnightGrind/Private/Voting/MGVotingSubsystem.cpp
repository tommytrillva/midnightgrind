// Copyright Midnight Grind. All Rights Reserved.

#include "Voting/MGVotingSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGVotingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Config.DefaultVoteDuration = 30.0f;
	Config.MapVoteDuration = 45.0f;
	Config.KickVoteDuration = 20.0f;
	Config.KickVoteThreshold = 0.6f;
	Config.SkipVoteThreshold = 0.5f;
	Config.MinPlayersForVote = 2;
	Config.VoteCooldown = 60.0f;
	Config.MaxMapOptions = 4;
	Config.bRandomizeMapOrder = true;
	Config.bExcludeRecentMaps = true;
	Config.RecentMapsToExclude = 3;
	Config.bHostCanOverride = true;

	ActiveVote.State = EMGVoteState::Inactive;
}

void UMGVotingSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VoteTickHandle);
	}

	Super::Deinitialize();
}

bool UMGVotingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FGuid UMGVotingSubsystem::StartVote(EMGVoteType Type, const TArray<FMGVoteOption>& Options, float Duration)
{
	if (!CanStartVote(Type))
	{
		return FGuid();
	}

	if (Options.Num() < 2)
	{
		return FGuid();
	}

	ActiveVote = FMGVoteSession();
	ActiveVote.VoteID = FGuid::NewGuid();
	ActiveVote.VoteType = Type;
	ActiveVote.State = EMGVoteState::Active;
	ActiveVote.Options = Options;
	ActiveVote.InitiatorID = LocalPlayerID;
	ActiveVote.StartTime = FDateTime::UtcNow();
	ActiveVote.TotalVoters = Players.Num();

	if (Duration <= 0.0f)
	{
		switch (Type)
		{
		case EMGVoteType::MapSelection:
			ActiveVote.Duration = Config.MapVoteDuration;
			break;
		case EMGVoteType::KickPlayer:
			ActiveVote.Duration = Config.KickVoteDuration;
			break;
		default:
			ActiveVote.Duration = Config.DefaultVoteDuration;
			break;
		}
	}
	else
	{
		ActiveVote.Duration = Duration;
	}

	ActiveVote.TimeRemaining = ActiveVote.Duration;

	switch (Type)
	{
	case EMGVoteType::KickPlayer:
		ActiveVote.PassThreshold = Config.KickVoteThreshold;
		break;
	case EMGVoteType::SkipRace:
	case EMGVoteType::RestartRace:
		ActiveVote.PassThreshold = Config.SkipVoteThreshold;
		break;
	default:
		ActiveVote.PassThreshold = 0.5f;
		break;
	}

	PlayerVotes.Empty();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			VoteTickHandle,
			this,
			&UMGVotingSubsystem::OnVoteTick,
			0.1f,
			true
		);
	}

	OnVoteStarted.Broadcast(ActiveVote);

	return ActiveVote.VoteID;
}

bool UMGVotingSubsystem::CancelVote(FGuid VoteID)
{
	if (ActiveVote.VoteID != VoteID || ActiveVote.State != EMGVoteState::Active)
	{
		return false;
	}

	if (ActiveVote.InitiatorID != LocalPlayerID && !bIsHost)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VoteTickHandle);
	}

	ActiveVote.State = EMGVoteState::Cancelled;
	OnVoteCancelled.Broadcast(VoteID);

	return true;
}

bool UMGVotingSubsystem::ForceEndVote(FGuid VoteID)
{
	if (!bIsHost && !Config.bHostCanOverride)
	{
		return false;
	}

	if (ActiveVote.VoteID != VoteID || ActiveVote.State != EMGVoteState::Active)
	{
		return false;
	}

	ProcessVoteEnd();
	return true;
}

bool UMGVotingSubsystem::IsVoteActive() const
{
	return ActiveVote.State == EMGVoteState::Active;
}

bool UMGVotingSubsystem::CanStartVote(EMGVoteType Type) const
{
	if (ActiveVote.State == EMGVoteState::Active)
	{
		return false;
	}

	if (Players.Num() < Config.MinPlayersForVote)
	{
		return false;
	}

	if (UWorld* World = GetWorld())
	{
		float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastVoteTime < Config.VoteCooldown)
		{
			return false;
		}
	}

	return true;
}

bool UMGVotingSubsystem::CastVote(FName OptionID)
{
	if (ActiveVote.State != EMGVoteState::Active)
	{
		return false;
	}

	bool bValidOption = false;
	for (const FMGVoteOption& Option : ActiveVote.Options)
	{
		if (Option.OptionID == OptionID)
		{
			bValidOption = true;
			break;
		}
	}

	if (!bValidOption)
	{
		return false;
	}

	if (PlayerVotes.Contains(LocalPlayerID) && !ActiveVote.bAllowVoteChange)
	{
		return false;
	}

	FMGPlayerVote Vote;
	Vote.PlayerID = LocalPlayerID;
	Vote.OptionID = OptionID;
	Vote.VoteTime = FDateTime::UtcNow();
	Vote.bAbstained = false;

	PlayerVotes.Add(LocalPlayerID, Vote);
	UpdateVoteCounts();

	OnVoteUpdated.Broadcast(ActiveVote, OptionID);

	return true;
}

bool UMGVotingSubsystem::ChangeVote(FName NewOptionID)
{
	if (!ActiveVote.bAllowVoteChange)
	{
		return false;
	}

	return CastVote(NewOptionID);
}

bool UMGVotingSubsystem::Abstain()
{
	if (ActiveVote.State != EMGVoteState::Active || !ActiveVote.bAllowAbstain)
	{
		return false;
	}

	FMGPlayerVote Vote;
	Vote.PlayerID = LocalPlayerID;
	Vote.OptionID = NAME_None;
	Vote.VoteTime = FDateTime::UtcNow();
	Vote.bAbstained = true;

	PlayerVotes.Add(LocalPlayerID, Vote);

	return true;
}

bool UMGVotingSubsystem::HasVoted() const
{
	return PlayerVotes.Contains(LocalPlayerID);
}

FName UMGVotingSubsystem::GetMyVote() const
{
	if (const FMGPlayerVote* Vote = PlayerVotes.Find(LocalPlayerID))
	{
		return Vote->OptionID;
	}
	return NAME_None;
}

FGuid UMGVotingSubsystem::StartMapVote(const TArray<FMGMapVoteData>& MapOptions)
{
	TArray<FMGVoteOption> Options;

	for (const FMGMapVoteData& Map : MapOptions)
	{
		FMGVoteOption Option;
		Option.OptionID = Map.MapID;
		Option.DisplayName = Map.MapName;
		Option.Thumbnail = Map.MapPreview;
		Option.Metadata = Map.MapPath;
		Options.Add(Option);
	}

	FGuid VoteID = StartVote(EMGVoteType::MapSelection, Options, Config.MapVoteDuration);

	if (VoteID.IsValid())
	{
		ActiveVote.VoteTitle = FText::FromString(TEXT("Vote for Next Track"));
		ActiveVote.VoteDescription = FText::FromString(TEXT("Select the track for the next race"));
	}

	return VoteID;
}

FGuid UMGVotingSubsystem::StartRandomMapVote(int32 NumOptions)
{
	TArray<FMGMapVoteData> SelectedMaps = SelectRandomMaps(NumOptions);
	return StartMapVote(SelectedMaps);
}

void UMGVotingSubsystem::RegisterMap(const FMGMapVoteData& MapData)
{
	for (int32 i = 0; i < AvailableMaps.Num(); ++i)
	{
		if (AvailableMaps[i].MapID == MapData.MapID)
		{
			AvailableMaps[i] = MapData;
			return;
		}
	}

	AvailableMaps.Add(MapData);
}

FMGMapVoteData UMGVotingSubsystem::GetMapData(FName MapID) const
{
	for (const FMGMapVoteData& Map : AvailableMaps)
	{
		if (Map.MapID == MapID)
		{
			return Map;
		}
	}
	return FMGMapVoteData();
}

FGuid UMGVotingSubsystem::StartKickVote(FName TargetPlayerID, const FString& Reason)
{
	if (TargetPlayerID == LocalPlayerID)
	{
		return FGuid();
	}

	FMGVoteOption YesOption;
	YesOption.OptionID = FName(TEXT("Yes"));
	YesOption.DisplayName = FText::FromString(TEXT("Yes"));

	FMGVoteOption NoOption;
	NoOption.OptionID = FName(TEXT("No"));
	NoOption.DisplayName = FText::FromString(TEXT("No"));
	NoOption.bIsDefault = true;

	TArray<FMGVoteOption> Options;
	Options.Add(YesOption);
	Options.Add(NoOption);

	FGuid VoteID = StartVote(EMGVoteType::KickPlayer, Options, Config.KickVoteDuration);

	if (VoteID.IsValid())
	{
		FString* TargetName = Players.Find(TargetPlayerID);
		ActiveVote.VoteTitle = FText::FromString(FString::Printf(TEXT("Kick %s?"),
			TargetName ? **TargetName : TEXT("Unknown")));
		ActiveVote.VoteDescription = FText::FromString(Reason.IsEmpty() ? TEXT("No reason given") : Reason);
		ActiveVote.PassThreshold = Config.KickVoteThreshold;
	}

	return VoteID;
}

FGuid UMGVotingSubsystem::StartSkipVote()
{
	FMGVoteOption YesOption;
	YesOption.OptionID = FName(TEXT("Yes"));
	YesOption.DisplayName = FText::FromString(TEXT("Skip"));

	FMGVoteOption NoOption;
	NoOption.OptionID = FName(TEXT("No"));
	NoOption.DisplayName = FText::FromString(TEXT("Continue"));
	NoOption.bIsDefault = true;

	TArray<FMGVoteOption> Options;
	Options.Add(YesOption);
	Options.Add(NoOption);

	FGuid VoteID = StartVote(EMGVoteType::SkipRace, Options);

	if (VoteID.IsValid())
	{
		ActiveVote.VoteTitle = FText::FromString(TEXT("Skip Current Race?"));
		ActiveVote.PassThreshold = Config.SkipVoteThreshold;
	}

	return VoteID;
}

FGuid UMGVotingSubsystem::StartRestartVote()
{
	FMGVoteOption YesOption;
	YesOption.OptionID = FName(TEXT("Yes"));
	YesOption.DisplayName = FText::FromString(TEXT("Restart"));

	FMGVoteOption NoOption;
	NoOption.OptionID = FName(TEXT("No"));
	NoOption.DisplayName = FText::FromString(TEXT("Continue"));
	NoOption.bIsDefault = true;

	TArray<FMGVoteOption> Options;
	Options.Add(YesOption);
	Options.Add(NoOption);

	FGuid VoteID = StartVote(EMGVoteType::RestartRace, Options);

	if (VoteID.IsValid())
	{
		ActiveVote.VoteTitle = FText::FromString(TEXT("Restart Race?"));
	}

	return VoteID;
}

void UMGVotingSubsystem::SetConfig(const FMGVotingConfig& NewConfig)
{
	Config = NewConfig;
}

FMGVoteOption UMGVotingSubsystem::GetWinningOption() const
{
	if (ActiveVote.Options.Num() == 0)
	{
		return FMGVoteOption();
	}

	FMGVoteOption Winner = ActiveVote.Options[0];
	for (const FMGVoteOption& Option : ActiveVote.Options)
	{
		if (Option.VoteCount > Winner.VoteCount)
		{
			Winner = Option;
		}
	}

	return Winner;
}

TArray<FMGVoteOption> UMGVotingSubsystem::GetSortedResults() const
{
	TArray<FMGVoteOption> Sorted = ActiveVote.Options;
	Sorted.Sort([](const FMGVoteOption& A, const FMGVoteOption& B)
	{
		return A.VoteCount > B.VoteCount;
	});
	return Sorted;
}

float UMGVotingSubsystem::GetOptionVotePercentage(FName OptionID) const
{
	int32 TotalVotes = GetTotalVotesCast();
	if (TotalVotes == 0)
	{
		return 0.0f;
	}

	for (const FMGVoteOption& Option : ActiveVote.Options)
	{
		if (Option.OptionID == OptionID)
		{
			return (static_cast<float>(Option.VoteCount) / static_cast<float>(TotalVotes)) * 100.0f;
		}
	}

	return 0.0f;
}

int32 UMGVotingSubsystem::GetTotalVotesCast() const
{
	int32 Total = 0;
	for (const FMGVoteOption& Option : ActiveVote.Options)
	{
		Total += Option.VoteCount;
	}
	return Total;
}

void UMGVotingSubsystem::SetLocalPlayer(FName PlayerID, const FString& PlayerName, bool bIsHostPlayer)
{
	LocalPlayerID = PlayerID;
	LocalPlayerName = PlayerName;
	bIsHost = bIsHostPlayer;

	AddPlayer(PlayerID, PlayerName);
}

void UMGVotingSubsystem::AddPlayer(FName PlayerID, const FString& PlayerName)
{
	Players.Add(PlayerID, PlayerName);

	if (ActiveVote.State == EMGVoteState::Active)
	{
		ActiveVote.TotalVoters = Players.Num();
	}
}

void UMGVotingSubsystem::RemovePlayer(FName PlayerID)
{
	Players.Remove(PlayerID);
	PlayerVotes.Remove(PlayerID);

	if (ActiveVote.State == EMGVoteState::Active)
	{
		ActiveVote.TotalVoters = Players.Num();
		UpdateVoteCounts();
	}
}

void UMGVotingSubsystem::ReceiveVoteStart(const FMGVoteSession& Vote)
{
	ActiveVote = Vote;
	PlayerVotes.Empty();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			VoteTickHandle,
			this,
			&UMGVotingSubsystem::OnVoteTick,
			0.1f,
			true
		);
	}

	OnVoteStarted.Broadcast(ActiveVote);
}

void UMGVotingSubsystem::ReceiveVoteCast(FName PlayerID, FName OptionID)
{
	FMGPlayerVote Vote;
	Vote.PlayerID = PlayerID;
	Vote.OptionID = OptionID;
	Vote.VoteTime = FDateTime::UtcNow();

	PlayerVotes.Add(PlayerID, Vote);
	UpdateVoteCounts();

	OnVoteUpdated.Broadcast(ActiveVote, OptionID);
}

void UMGVotingSubsystem::ReceiveVoteEnd(FGuid VoteID, FName WinningOption, bool bPassed)
{
	if (ActiveVote.VoteID != VoteID)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VoteTickHandle);
	}

	ActiveVote.WinningOptionID = WinningOption;
	ActiveVote.State = bPassed ? EMGVoteState::Passed : EMGVoteState::Failed;

	OnVoteEnded.Broadcast(ActiveVote, bPassed);

	if (bPassed && ActiveVote.VoteType == EMGVoteType::MapSelection)
	{
		FMGMapVoteData SelectedMap = GetMapData(WinningOption);
		OnMapVoteResult.Broadcast(SelectedMap);
		AddToRecentMaps(WinningOption);
	}
}

void UMGVotingSubsystem::OnVoteTick()
{
	if (ActiveVote.State != EMGVoteState::Active)
	{
		return;
	}

	float DeltaTime = 0.1f;
	ActiveVote.TimeRemaining -= DeltaTime;

	OnVoteTimeUpdate.Broadcast(ActiveVote.TimeRemaining);

	if (ActiveVote.TimeRemaining <= 0.0f)
	{
		ProcessVoteEnd();
	}
}

void UMGVotingSubsystem::ProcessVoteEnd()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VoteTickHandle);
		LastVoteTime = World->GetTimeSeconds();
	}

	UpdateVoteCounts();
	bool bPassed = DetermineResult();

	ActiveVote.State = bPassed ? EMGVoteState::Passed : EMGVoteState::Failed;
	ActiveVote.WinningOptionID = GetWinningOption().OptionID;

	OnVoteEnded.Broadcast(ActiveVote, bPassed);

	if (bPassed && ActiveVote.VoteType == EMGVoteType::MapSelection)
	{
		FMGMapVoteData SelectedMap = GetMapData(ActiveVote.WinningOptionID);
		OnMapVoteResult.Broadcast(SelectedMap);
		AddToRecentMaps(ActiveVote.WinningOptionID);
	}
}

void UMGVotingSubsystem::UpdateVoteCounts()
{
	for (FMGVoteOption& Option : ActiveVote.Options)
	{
		Option.VoteCount = 0;
		Option.Voters.Empty();
	}

	for (const auto& Pair : PlayerVotes)
	{
		if (Pair.Value.bAbstained)
		{
			continue;
		}

		for (FMGVoteOption& Option : ActiveVote.Options)
		{
			if (Option.OptionID == Pair.Value.OptionID)
			{
				Option.VoteCount++;
				Option.Voters.Add(Pair.Key);
				break;
			}
		}
	}
}

bool UMGVotingSubsystem::DetermineResult()
{
	int32 TotalVotes = GetTotalVotesCast();

	if (TotalVotes == 0)
	{
		// No votes - use default option if available
		for (const FMGVoteOption& Option : ActiveVote.Options)
		{
			if (Option.bIsDefault)
			{
				return false;
			}
		}
		return false;
	}

	// For yes/no votes
	if (ActiveVote.VoteType == EMGVoteType::KickPlayer ||
		ActiveVote.VoteType == EMGVoteType::SkipRace ||
		ActiveVote.VoteType == EMGVoteType::RestartRace)
	{
		for (const FMGVoteOption& Option : ActiveVote.Options)
		{
			if (Option.OptionID == FName(TEXT("Yes")))
			{
				float YesPercent = static_cast<float>(Option.VoteCount) / static_cast<float>(ActiveVote.TotalVoters);
				return YesPercent >= ActiveVote.PassThreshold;
			}
		}
	}

	// For map votes, always passes (highest vote wins)
	if (ActiveVote.VoteType == EMGVoteType::MapSelection)
	{
		return true;
	}

	return true;
}

void UMGVotingSubsystem::AddToRecentMaps(FName MapID)
{
	RecentMaps.Insert(MapID, 0);

	while (RecentMaps.Num() > Config.RecentMapsToExclude)
	{
		RecentMaps.RemoveAt(RecentMaps.Num() - 1);
	}
}

TArray<FMGMapVoteData> UMGVotingSubsystem::SelectRandomMaps(int32 Count)
{
	TArray<FMGMapVoteData> Available;

	for (const FMGMapVoteData& Map : AvailableMaps)
	{
		if (Config.bExcludeRecentMaps && RecentMaps.Contains(Map.MapID))
		{
			continue;
		}
		Available.Add(Map);
	}

	if (Config.bRandomizeMapOrder)
	{
		for (int32 i = Available.Num() - 1; i > 0; --i)
		{
			int32 j = FMath::RandRange(0, i);
			Available.Swap(i, j);
		}
	}

	TArray<FMGMapVoteData> Selected;
	for (int32 i = 0; i < FMath::Min(Count, Available.Num()); ++i)
	{
		Selected.Add(Available[i]);
	}

	return Selected;
}
