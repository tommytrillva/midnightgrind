// Copyright Midnight Grind. All Rights Reserved.

#include "GameMode/MGGameModeSubsystem.h"

void UMGGameModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeModes();
	InitializePlaylists();

	// Set default mode
	SetGameMode(EMGGameModeType::CircuitRace);
}

void UMGGameModeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMGGameModeSubsystem::SetGameMode(EMGGameModeType ModeType)
{
	for (const FMGGameModeInfo& Mode : AvailableModes)
	{
		if (Mode.DefaultRules.ModeType == ModeType)
		{
			CurrentMode = Mode;
			CurrentRules = Mode.DefaultRules;
			OnGameModeChanged.Broadcast(CurrentMode);
			OnRulesChanged.Broadcast(CurrentRules);
			return;
		}
	}
}

void UMGGameModeSubsystem::SetGameModeByID(FName ModeID)
{
	for (const FMGGameModeInfo& Mode : AvailableModes)
	{
		if (Mode.ModeID == ModeID)
		{
			CurrentMode = Mode;
			CurrentRules = Mode.DefaultRules;
			OnGameModeChanged.Broadcast(CurrentMode);
			OnRulesChanged.Broadcast(CurrentRules);
			return;
		}
	}

	// Check custom modes
	for (const FMGGameModeInfo& Mode : CustomModes)
	{
		if (Mode.ModeID == ModeID)
		{
			CurrentMode = Mode;
			CurrentRules = Mode.DefaultRules;
			OnGameModeChanged.Broadcast(CurrentMode);
			OnRulesChanged.Broadcast(CurrentRules);
			return;
		}
	}
}

void UMGGameModeSubsystem::SetRules(const FMGGameModeRules& Rules)
{
	CurrentRules = Rules;
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::SetLapCount(int32 Laps)
{
	CurrentRules.LapCount = FMath::Clamp(Laps, 1, 99);
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::SetTrafficMode(EMGTrafficMode Traffic)
{
	CurrentRules.Traffic = Traffic;
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::SetCatchUpMode(EMGCatchUpMode CatchUp)
{
	CurrentRules.CatchUp = CatchUp;
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::SetCollisionsEnabled(bool bEnabled)
{
	CurrentRules.bAllowCollisions = bEnabled;
	CurrentRules.bGhostMode = !bEnabled;
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::SetNitroEnabled(bool bEnabled)
{
	CurrentRules.bAllowNitro = bEnabled;
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::SetPerformanceCap(int32 MaxPI)
{
	CurrentRules.PerformanceCapPI = FMath::Max(0, MaxPI);
	CurrentRules.bRestrictedCarClass = MaxPI > 0;
	OnRulesChanged.Broadcast(CurrentRules);
}

void UMGGameModeSubsystem::ResetToDefaultRules()
{
	CurrentRules = CurrentMode.DefaultRules;
	OnRulesChanged.Broadcast(CurrentRules);
}

TArray<FMGPlaylistEntry> UMGGameModeSubsystem::GetFeaturedPlaylists() const
{
	TArray<FMGPlaylistEntry> Featured;
	for (const FMGPlaylistEntry& Playlist : Playlists)
	{
		if (Playlist.bIsFeatured)
		{
			Featured.Add(Playlist);
		}
	}
	return Featured;
}

void UMGGameModeSubsystem::SelectPlaylist(FName PlaylistID)
{
	for (const FMGPlaylistEntry& Playlist : Playlists)
	{
		if (Playlist.PlaylistID == PlaylistID)
		{
			CurrentPlaylist = Playlist;
			CurrentRules = Playlist.RuleOverrides;

			// Set mode to first in playlist
			if (Playlist.IncludedModes.Num() > 0)
			{
				SetGameMode(Playlist.IncludedModes[0]);
				CurrentRules = Playlist.RuleOverrides;
			}
			return;
		}
	}
}

void UMGGameModeSubsystem::EliminatePlayer(const FString& PlayerID)
{
	if (!EliminationState.EliminatedPlayers.Contains(PlayerID))
	{
		EliminationState.EliminatedPlayers.Add(PlayerID);
		OnPlayerEliminated.Broadcast(PlayerID);
	}
}

void UMGGameModeSubsystem::UpdateEliminationTimer(float MGDeltaTime)
{
	if (CurrentRules.ModeType != EMGGameModeType::Elimination)
		return;

	EliminationState.TimeUntilElimination -= DeltaTime;

	if (EliminationState.TimeUntilElimination <= 0.0f)
	{
		// Eliminate last place
		if (!EliminationState.PlayerInLastPlace.IsEmpty())
		{
			EliminatePlayer(EliminationState.PlayerInLastPlace);
		}

		// Reset timer for next elimination
		EliminationState.TimeUntilElimination = 30.0f; // 30 seconds between eliminations
	}
}

bool UMGGameModeSubsystem::IsPlayerEliminated(const FString& PlayerID) const
{
	return EliminationState.EliminatedPlayers.Contains(PlayerID);
}

FMGDriftScoring UMGGameModeSubsystem::GetDriftScore(const FString& PlayerID) const
{
	if (const FMGDriftScoring* Score = DriftScores.Find(PlayerID))
	{
		return *Score;
	}
	return FMGDriftScoring();
}

void UMGGameModeSubsystem::UpdateDriftScore(const FString& PlayerID, float DriftAngle, float Speed, float MGDeltaTime)
{
	FMGDriftScoring& Score = DriftScores.FindOrAdd(PlayerID);

	Score.DriftAngle = DriftAngle;
	Score.DriftSpeed = Speed;

	// Calculate points based on angle and speed
	float AngleFactor = FMath::Clamp(FMath::Abs(DriftAngle) / 90.0f, 0.0f, 1.0f);
	float SpeedFactor = FMath::Clamp(Speed / 200.0f, 0.0f, 1.0f); // Assuming 200 km/h max drift speed

	int64 PointsThisFrame = static_cast<int64>(AngleFactor * SpeedFactor * 100.0f * DeltaTime * Score.ComboMultiplier);

	Score.CurrentCombo += PointsThisFrame;

	// Increase combo multiplier based on sustained drift
	Score.ComboTimer += DeltaTime;
	if (Score.ComboTimer >= 2.0f)
	{
		Score.ComboMultiplier = FMath::Min(Score.ComboMultiplier + 1, 10);
		Score.ComboTimer = 0.0f;
	}

	OnDriftScoreUpdate.Broadcast(Score, PlayerID);
}

void UMGGameModeSubsystem::EndDriftCombo(const FString& PlayerID)
{
	if (FMGDriftScoring* Score = DriftScores.Find(PlayerID))
	{
		// Add combo to total
		Score->TotalScore += Score->CurrentCombo;

		// Reset combo
		Score->CurrentCombo = 0;
		Score->ComboMultiplier = 1;
		Score->ComboTimer = 0.0f;
		Score->DriftAngle = 0.0f;
		Score->DriftSpeed = 0.0f;

		OnDriftScoreUpdate.Broadcast(*Score, PlayerID);
	}
}

TArray<FMGDriftLeaderboardEntry> UMGGameModeSubsystem::GetDriftLeaderboard() const
{
	TArray<FMGDriftLeaderboardEntry> Leaderboard;

	for (const auto& Pair : DriftScores)
	{
		FMGDriftLeaderboardEntry Entry;
		Entry.PlayerID = Pair.Key;
		Entry.Score = Pair.Value.TotalScore;
		Leaderboard.Add(Entry);
	}

	// Sort by score descending
	Leaderboard.Sort([](const FMGDriftLeaderboardEntry& A, const FMGDriftLeaderboardEntry& B)
	{
		return A.Score > B.Score;
	});

	return Leaderboard;
}

FName UMGGameModeSubsystem::CreateCustomMode(const FMGGameModeInfo& ModeInfo)
{
	FMGGameModeInfo NewMode = ModeInfo;
	NewMode.bIsOfficial = false;

	if (NewMode.ModeID.IsNone())
	{
		NewMode.ModeID = FName(*FString::Printf(TEXT("Custom_%s"), *FGuid::NewGuid().ToString()));
	}

	CustomModes.Add(NewMode);
	return NewMode.ModeID;
}

void UMGGameModeSubsystem::SaveCustomMode(FName ModeID)
{
	// Would save to local storage
}

void UMGGameModeSubsystem::DeleteCustomMode(FName ModeID)
{
	CustomModes.RemoveAll([ModeID](const FMGGameModeInfo& Mode)
	{
		return Mode.ModeID == ModeID;
	});
}

TArray<FMGGameModeInfo> UMGGameModeSubsystem::GetCustomModes() const
{
	return CustomModes;
}

void UMGGameModeSubsystem::InitializeModes()
{
	AvailableModes.Empty();

	// Circuit Race
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::CircuitRace));

	// Sprint Race
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::SprintRace));

	// Drift
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::Drift));

	// Time Attack
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::TimeAttack));

	// Elimination
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::Elimination));

	// King of the Hill
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::KingOfTheHill));

	// Tag
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::Tag));

	// Drag
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::Drag));

	// Touge
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::Touge));

	// Freeroam Race
	AvailableModes.Add(CreateModeInfo(EMGGameModeType::FreeroamRace));
}

void UMGGameModeSubsystem::InitializePlaylists()
{
	Playlists.Empty();

	// Quick Race
	FMGPlaylistEntry QuickRace;
	QuickRace.PlaylistID = FName(TEXT("Playlist_QuickRace"));
	QuickRace.PlaylistName = FText::FromString(TEXT("Quick Race"));
	QuickRace.IncludedModes.Add(EMGGameModeType::CircuitRace);
	QuickRace.IncludedModes.Add(EMGGameModeType::SprintRace);
	QuickRace.bIsFeatured = true;
	Playlists.Add(QuickRace);

	// Ranked Circuit
	FMGPlaylistEntry RankedCircuit;
	RankedCircuit.PlaylistID = FName(TEXT("Playlist_Ranked"));
	RankedCircuit.PlaylistName = FText::FromString(TEXT("Ranked Racing"));
	RankedCircuit.IncludedModes.Add(EMGGameModeType::CircuitRace);
	RankedCircuit.bIsRanked = true;
	RankedCircuit.bIsFeatured = true;
	RankedCircuit.RuleOverrides.CatchUp = EMGCatchUpMode::Disabled;
	RankedCircuit.RuleOverrides.Traffic = EMGTrafficMode::None;
	Playlists.Add(RankedCircuit);

	// Drift League
	FMGPlaylistEntry DriftLeague;
	DriftLeague.PlaylistID = FName(TEXT("Playlist_Drift"));
	DriftLeague.PlaylistName = FText::FromString(TEXT("Drift League"));
	DriftLeague.IncludedModes.Add(EMGGameModeType::Drift);
	DriftLeague.bIsFeatured = true;
	Playlists.Add(DriftLeague);

	// Party Mode
	FMGPlaylistEntry PartyMode;
	PartyMode.PlaylistID = FName(TEXT("Playlist_Party"));
	PartyMode.PlaylistName = FText::FromString(TEXT("Party Mix"));
	PartyMode.IncludedModes.Add(EMGGameModeType::Elimination);
	PartyMode.IncludedModes.Add(EMGGameModeType::Tag);
	PartyMode.IncludedModes.Add(EMGGameModeType::KingOfTheHill);
	PartyMode.bIsFeatured = true;
	Playlists.Add(PartyMode);

	// Drag Strip
	FMGPlaylistEntry DragStrip;
	DragStrip.PlaylistID = FName(TEXT("Playlist_Drag"));
	DragStrip.PlaylistName = FText::FromString(TEXT("Drag Racing"));
	DragStrip.IncludedModes.Add(EMGGameModeType::Drag);
	Playlists.Add(DragStrip);

	// Touge Battle
	FMGPlaylistEntry TougeBattle;
	TougeBattle.PlaylistID = FName(TEXT("Playlist_Touge"));
	TougeBattle.PlaylistName = FText::FromString(TEXT("Mountain Pass"));
	TougeBattle.IncludedModes.Add(EMGGameModeType::Touge);
	TougeBattle.RuleOverrides.MaxRacers = 2;
	TougeBattle.RuleOverrides.Traffic = EMGTrafficMode::OncomingOnly;
	Playlists.Add(TougeBattle);
}

FMGGameModeInfo UMGGameModeSubsystem::CreateModeInfo(EMGGameModeType Type) const
{
	FMGGameModeInfo Info;
	Info.DefaultRules.ModeType = Type;
	Info.bIsOfficial = true;

	switch (Type)
	{
	case EMGGameModeType::CircuitRace:
		Info.ModeID = FName(TEXT("Mode_Circuit"));
		Info.DisplayName = FText::FromString(TEXT("Circuit Race"));
		Info.Description = FText::FromString(TEXT("Traditional lap-based racing. First to cross the finish line wins."));
		Info.DefaultRules.LapCount = 3;
		Info.DefaultRules.MaxRacers = 8;
		break;

	case EMGGameModeType::SprintRace:
		Info.ModeID = FName(TEXT("Mode_Sprint"));
		Info.DisplayName = FText::FromString(TEXT("Sprint"));
		Info.Description = FText::FromString(TEXT("Point-to-point racing through the city streets."));
		Info.DefaultRules.LapCount = 1;
		Info.DefaultRules.MaxRacers = 8;
		break;

	case EMGGameModeType::Drift:
		Info.ModeID = FName(TEXT("Mode_Drift"));
		Info.DisplayName = FText::FromString(TEXT("Drift Zone"));
		Info.Description = FText::FromString(TEXT("Score points by drifting. Highest score wins."));
		Info.DefaultRules.MaxRacers = 6;
		Info.DefaultRules.bAllowCollisions = false;
		Info.DefaultRules.bGhostMode = true;
		Info.DefaultRules.TimeLimit = 180.0f;
		break;

	case EMGGameModeType::TimeAttack:
		Info.ModeID = FName(TEXT("Mode_TimeAttack"));
		Info.DisplayName = FText::FromString(TEXT("Time Attack"));
		Info.Description = FText::FromString(TEXT("Race against the clock. Set the fastest lap time."));
		Info.DefaultRules.MaxRacers = 1;
		Info.DefaultRules.LapCount = 5;
		Info.DefaultRules.Traffic = EMGTrafficMode::None;
		Info.DefaultRules.bGhostMode = true;
		break;

	case EMGGameModeType::Elimination:
		Info.ModeID = FName(TEXT("Mode_Elimination"));
		Info.DisplayName = FText::FromString(TEXT("Elimination"));
		Info.Description = FText::FromString(TEXT("Last place is eliminated each lap. Survive to win."));
		Info.DefaultRules.LapCount = 8;
		Info.DefaultRules.MaxRacers = 8;
		Info.DefaultRules.MinRacers = 4;
		break;

	case EMGGameModeType::KingOfTheHill:
		Info.ModeID = FName(TEXT("Mode_KingOfHill"));
		Info.DisplayName = FText::FromString(TEXT("King of the Hill"));
		Info.Description = FText::FromString(TEXT("Stay in first place to score points. Highest score wins."));
		Info.DefaultRules.TimeLimit = 300.0f;
		Info.DefaultRules.MaxRacers = 6;
		break;

	case EMGGameModeType::Tag:
		Info.ModeID = FName(TEXT("Mode_Tag"));
		Info.DisplayName = FText::FromString(TEXT("Tag"));
		Info.Description = FText::FromString(TEXT("One player is 'it'. Be 'it' the longest to win."));
		Info.DefaultRules.TimeLimit = 300.0f;
		Info.DefaultRules.MaxRacers = 6;
		Info.DefaultRules.Traffic = EMGTrafficMode::Light;
		break;

	case EMGGameModeType::Drag:
		Info.ModeID = FName(TEXT("Mode_Drag"));
		Info.DisplayName = FText::FromString(TEXT("Drag Race"));
		Info.Description = FText::FromString(TEXT("Quarter mile straight-line racing. Perfect your launch."));
		Info.DefaultRules.MaxRacers = 2;
		Info.DefaultRules.Traffic = EMGTrafficMode::None;
		Info.DefaultRules.bAllowNitro = false;
		break;

	case EMGGameModeType::Touge:
		Info.ModeID = FName(TEXT("Mode_Touge"));
		Info.DisplayName = FText::FromString(TEXT("Touge Battle"));
		Info.Description = FText::FromString(TEXT("Mountain pass racing. Fall behind and lose."));
		Info.DefaultRules.MaxRacers = 2;
		Info.DefaultRules.LapCount = 1;
		Info.DefaultRules.Traffic = EMGTrafficMode::OncomingOnly;
		break;

	case EMGGameModeType::FreeroamRace:
		Info.ModeID = FName(TEXT("Mode_Freeroam"));
		Info.DisplayName = FText::FromString(TEXT("Freeroam Race"));
		Info.Description = FText::FromString(TEXT("Impromptu street race. Challenge anyone, anywhere."));
		Info.DefaultRules.MaxRacers = 8;
		Info.DefaultRules.Traffic = EMGTrafficMode::Normal;
		Info.bIsRanked = false;
		break;

	default:
		Info.ModeID = FName(TEXT("Mode_Custom"));
		Info.DisplayName = FText::FromString(TEXT("Custom"));
		Info.Description = FText::FromString(TEXT("Create your own rules."));
		break;
	}

	return Info;
}
