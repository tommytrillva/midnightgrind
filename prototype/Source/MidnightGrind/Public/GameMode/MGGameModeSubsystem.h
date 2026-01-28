// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGGameModeSubsystem.h
 * @brief Race Type Configuration and Game Mode Rule Management System
 * @author Midnight Grind Team
 * @date 2024
 *
 * @section overview_sec Overview
 * This subsystem manages all race types and rule configurations in Midnight Grind.
 * It defines what kinds of races exist (circuit, drift, drag, etc.) and what
 * rules apply to each (lap count, traffic, collisions, nitro, etc.).
 *
 * @note This is NOT Unreal's AGameMode class (which handles spawning and game flow).
 * This subsystem specifically handles race TYPE configuration and rules.
 *
 * @section quickstart_sec Quick Start Example
 * @code
 * // Get the subsystem
 * UMGGameModeSubsystem* GameMode = GetGameInstance()->GetSubsystem<UMGGameModeSubsystem>();
 *
 * // Set up a drift race
 * GameMode->SetGameMode(EMGGameModeType::Drift);
 * GameMode->SetTrafficMode(EMGTrafficMode::None);
 * GameMode->SetCollisionsEnabled(false);  // Ghost mode for drift
 *
 * // Set up a standard circuit race
 * GameMode->SetGameMode(EMGGameModeType::CircuitRace);
 * GameMode->SetLapCount(5);
 * GameMode->SetNitroEnabled(true);
 * GameMode->SetCatchUpMode(EMGCatchUpMode::Subtle);
 *
 * // Get current configuration
 * FMGGameModeRules Rules = GameMode->GetCurrentRules();
 * UE_LOG(LogGame, Log, TEXT("Laps: %d, Traffic: %d"), Rules.LapCount, (int32)Rules.Traffic);
 *
 * // Use playlists for quickplay
 * TArray<FMGPlaylistEntry> Featured = GameMode->GetFeaturedPlaylists();
 * GameMode->SelectPlaylist(Featured[0].PlaylistID);
 * @endcode
 *
 * @section concepts_sec Key Concepts for Beginners
 *
 * @subsection modes_subsec Game Mode Types (EMGGameModeType)
 * | Mode Type     | Description                                        |
 * |---------------|----------------------------------------------------|
 * | CircuitRace   | Traditional lap-based racing around a track        |
 * | SprintRace    | Point A to Point B, no laps                        |
 * | Drift         | Score points by drifting, position doesn't matter  |
 * | TimeAttack    | Race against the clock, not other players          |
 * | Elimination   | Last place gets eliminated each lap                |
 * | KingOfTheHill | Earn points by leading the race                    |
 * | Tag           | One player is "it", earn points while tagged       |
 * | Cops          | Pursuit mode - evade or chase                      |
 * | Drag          | Quarter-mile straight-line speed runs              |
 * | Touge         | Japanese mountain pass battles                     |
 * | Custom        | Player-created rule combinations                   |
 *
 * @subsection rules_subsec Game Mode Rules (FMGGameModeRules)
 * Rules are the configurable settings for each race:
 * - **LapCount**: Number of laps (0 for point-to-point races)
 * - **MaxRacers**: Maximum players allowed in the race
 * - **Traffic**: None/Light/Normal/Heavy AI traffic on roads
 * - **CatchUp**: Rubber-banding intensity to keep races close
 * - **bAllowNitro**: Can players use NOS boost?
 * - **bAllowCollisions**: Can cars physically interact?
 * - **bGhostMode**: Cars pass through each other
 * - **PerformanceCapPI**: Maximum vehicle performance index allowed
 *
 * @subsection playlist_subsec Playlists (FMGPlaylistEntry)
 * Curated combinations of modes and tracks for matchmaking:
 * - Quickplay uses playlists to find appropriate matches
 * - Can be ranked (competitive) or casual
 * - Featured playlists rotate based on events (see EventCalendar)
 * - RuleOverrides customize rules per playlist
 *
 * @section modespecific_sec Mode-Specific Features
 *
 * @subsection elim_subsec Elimination Mode
 * @code
 * // During race, update elimination timer
 * GameMode->UpdateEliminationTimer(DeltaTime);
 *
 * // When someone is eliminated
 * GameMode->EliminatePlayer(PlayerID);
 *
 * // Check elimination state
 * FMGEliminationState State = GameMode->GetEliminationState();
 * if (State.TimeUntilElimination < 10.0f)
 * {
 *     ShowEliminationWarning(State.PlayerInLastPlace);
 * }
 * @endcode
 *
 * @subsection drift_subsec Drift Mode
 * @code
 * // Update drift score during gameplay
 * GameMode->UpdateDriftScore(PlayerID, DriftAngle, Speed, DeltaTime);
 *
 * // When drift ends (straightened out or crashed)
 * GameMode->EndDriftCombo(PlayerID);
 *
 * // Get current scoring
 * FMGDriftScoring Score = GameMode->GetDriftScore(PlayerID);
 * ShowCombo(Score.CurrentCombo, Score.ComboMultiplier);
 * @endcode
 *
 * @section events_subsec Delegates/Events
 * - **OnGameModeChanged**: Fires when mode type changes
 * - **OnRulesChanged**: Fires when any rule is modified
 * - **OnPlayerEliminated**: Fires when a player is eliminated
 * - **OnDriftScoreUpdate**: Fires when drift score changes
 *
 * @section related_sec Related Files
 * - MGGameModeSubsystem.cpp: Implementation
 * - MGEventCalendarSubsystem.h: Featured playlist rotation
 * - MGMatchmakingSubsystem.h: Uses playlists for online matching
 *
 * @see EMGGameModeType, EMGTrafficMode, EMGCatchUpMode
 * @see FMGGameModeRules, FMGPlaylistEntry, FMGEliminationState, FMGDriftScoring
 */

/**
 * ============================================================================
 * MGGameModeSubsystem.h
 * ============================================================================
 *
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file manages all race types and rule configurations in Midnight Grind.
 * It defines what kinds of races exist (circuit, drift, drag, etc.) and what
 * rules apply to each (laps, traffic, collisions, etc.).
 *
 * NOTE: This is NOT the same as Unreal's AGameMode class. That handles
 * spawning and game flow. This subsystem handles race TYPE configuration.
 *
 * KEY CONCEPTS:
 *
 * 1. GAME MODE TYPES (EMGGameModeType)
 *    The core race types available in the game:
 *    - CircuitRace: Traditional lap-based racing around a track
 *    - SprintRace: Point A to Point B, no laps
 *    - Drift: Score points by drifting, not by position
 *    - TimeAttack: Race against the clock, not other players
 *    - Elimination: Last place gets eliminated each lap
 *    - KingOfTheHill: Earn points by leading the race
 *    - Tag: One player is "it", earn points while "it"
 *    - Cops: Pursuit mode - evade or chase
 *    - Drag: Quarter-mile straight-line speed runs
 *    - Touge: Japanese mountain pass battles
 *    - Custom: Player-created rule combinations
 *
 * 2. GAME MODE RULES (FMGGameModeRules)
 *    Rules are the "settings" for a race. Key options include:
 *    - LapCount: How many laps (0 for point-to-point)
 *    - MaxRacers: Maximum players in the race
 *    - Traffic: None/Light/Normal/Heavy - AI cars on road
 *    - CatchUp: Rubber-banding to keep races close
 *    - bAllowNitro: Can players use NOS boost?
 *    - bAllowCollisions: Can cars hit each other?
 *    - bGhostMode: Cars pass through each other
 *    - PerformanceCapPI: Maximum car performance allowed
 *
 * 3. PLAYLISTS (FMGPlaylistEntry)
 *    Playlists are curated combinations of modes and tracks:
 *    - Quickplay uses playlists to find matches
 *    - Can be ranked or casual
 *    - Featured playlists rotate (see EventCalendar)
 *    - RuleOverrides customize rules per playlist
 *
 * 4. MODE-SPECIFIC STATE
 *    Some modes need extra tracking:
 *    - FMGEliminationState: Who's been eliminated, countdown timer
 *    - FMGDriftScoring: Current combo, multiplier, total score
 *    These are accessed via GetEliminationState() / GetDriftScore()
 *
 * 5. CUSTOM MODES
 *    Players can create their own modes:
 *    - CreateCustomMode() saves a new mode configuration
 *    - Custom modes can be shared with friends
 *    - GetCustomModes() returns player-created modes
 *
 * COMMON USE CASES:
 *
 * For Race Setup:
 * - SetGameMode(EMGGameModeType::Drift) to change race type
 * - SetLapCount(5) to configure laps
 * - SetTrafficMode(EMGTrafficMode::Heavy) for challenge
 * - GetCurrentRules() to read active configuration
 *
 * For Lobby UI:
 * - GetAvailableModes() to populate mode selector
 * - GetPlaylists() for quickplay playlist list
 * - GetFeaturedPlaylists() for highlighted playlists
 *
 * For In-Race:
 * - Elimination: Call UpdateEliminationTimer(), EliminatePlayer()
 * - Drift: Call UpdateDriftScore(), EndDriftCombo()
 * - Subscribe to OnPlayerEliminated for elimination announcements
 * - Subscribe to OnDriftScoreUpdate for score popups
 *
 * For Rule Modification:
 * - SetRules() applies complete rule set
 * - Individual setters (SetLapCount, SetNitroEnabled, etc.) for tweaks
 * - ResetToDefaultRules() restores mode's default settings
 *
 * DELEGATES:
 * - OnGameModeChanged: Fires when mode type changes
 * - OnRulesChanged: Fires when any rule is modified
 * - OnPlayerEliminated: Fires when a player is eliminated
 * - OnDriftScoreUpdate: Fires when drift score changes
 *
 * ============================================================================
 *
 * Game Mode System - Race Type Management
 * - Multiple race types with unique rules
 * - Dynamic rule modification
 * - Custom game mode creation
 * - Playlist management for quickplay
 * - Tournament mode integration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGGameModeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGGameModeType : uint8
{
	CircuitRace,         // Traditional lap-based racing
	SprintRace,          // Point-to-point race
	Drift,               // Scoring based on drift performance
	TimeAttack,          // Beat the clock
	Elimination,         // Last place eliminated each lap
	KingOfTheHill,       // Hold position at front
	Tag,                 // Be "it" the longest
	Cops,                // Evade or catch
	Drag,                // 1/4 mile sprint
	Touge,               // Mountain pass battle
	FreeroamRace,        // Impromptu street race
	Tournament,          // Multi-race series
	Custom               // User-defined rules
};

UENUM(BlueprintType)
enum class EMGTrafficMode : uint8
{
	None,                // No traffic
	Light,               // Sparse traffic
	Normal,              // Standard traffic
	Heavy,               // Dense traffic
	OncomingOnly         // Traffic in opposite lanes only
};

UENUM(BlueprintType)
enum class EMGCatchUpMode : uint8
{
	Disabled,            // No rubber banding
	Subtle,              // Slight speed adjustment
	Moderate,            // Noticeable but fair
	Aggressive           // Strong catch-up
};

USTRUCT(BlueprintType)
struct FMGGameModeRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGameModeType ModeType = EMGGameModeType::CircuitRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRacers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinRacers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f; // 0 = no limit

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowNitro = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCollisions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGhostMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrafficMode Traffic = EMGTrafficMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCatchUpMode CatchUp = EMGCatchUpMode::Subtle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDamageEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlipstreamEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRestrictedCarClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredCarClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerformanceCapPI = 0; // 0 = no cap

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTeamRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamCount = 2;
};

USTRUCT(BlueprintType)
struct FMGGameModeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGameModeRules DefaultRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOfficial = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinReputationTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ModeIcon;
};

USTRUCT(BlueprintType)
struct FMGPlaylistEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlaylistName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGGameModeType> IncludedModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> IncludedTracks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGameModeRules RuleOverrides;
};

USTRUCT(BlueprintType)
struct FMGEliminationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EliminatedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilElimination = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerInLastPlace;
};

USTRUCT(BlueprintType)
struct FMGDriftScoring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboTimer = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnGameModeChanged, const FMGGameModeInfo&, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRulesChanged, const FMGGameModeRules&, NewRules);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerEliminated, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnDriftScoreUpdate, const FMGDriftScoring&, Score, const FString&, PlayerID);

UCLASS()
class MIDNIGHTGRIND_API UMGGameModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Mode Selection
	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetGameMode(EMGGameModeType ModeType);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetGameModeByID(FName ModeID);

	UFUNCTION(BlueprintPure, Category = "GameMode")
	FMGGameModeInfo GetCurrentMode() const { return CurrentMode; }

	UFUNCTION(BlueprintPure, Category = "GameMode")
	FMGGameModeRules GetCurrentRules() const { return CurrentRules; }

	UFUNCTION(BlueprintPure, Category = "GameMode")
	TArray<FMGGameModeInfo> GetAvailableModes() const { return AvailableModes; }

	// Rule Modification
	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetRules(const FMGGameModeRules& Rules);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetLapCount(int32 Laps);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetTrafficMode(EMGTrafficMode Traffic);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetCatchUpMode(EMGCatchUpMode CatchUp);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetCollisionsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetNitroEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetPerformanceCap(int32 MaxPI);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void ResetToDefaultRules();

	// Playlists
	UFUNCTION(BlueprintPure, Category = "GameMode|Playlist")
	TArray<FMGPlaylistEntry> GetPlaylists() const { return Playlists; }

	UFUNCTION(BlueprintPure, Category = "GameMode|Playlist")
	TArray<FMGPlaylistEntry> GetFeaturedPlaylists() const;

	UFUNCTION(BlueprintCallable, Category = "GameMode|Playlist")
	void SelectPlaylist(FName PlaylistID);

	UFUNCTION(BlueprintPure, Category = "GameMode|Playlist")
	FMGPlaylistEntry GetCurrentPlaylist() const { return CurrentPlaylist; }

	// Elimination Mode
	UFUNCTION(BlueprintPure, Category = "GameMode|Elimination")
	FMGEliminationState GetEliminationState() const { return EliminationState; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Elimination")
	void EliminatePlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Elimination")
	void UpdateEliminationTimer(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "GameMode|Elimination")
	bool IsPlayerEliminated(const FString& PlayerID) const;

	// Drift Mode
	UFUNCTION(BlueprintPure, Category = "GameMode|Drift")
	FMGDriftScoring GetDriftScore(const FString& PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "GameMode|Drift")
	void UpdateDriftScore(const FString& PlayerID, float DriftAngle, float Speed, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Drift")
	void EndDriftCombo(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "GameMode|Drift")
	TArray<TPair<FString, int64>> GetDriftLeaderboard() const;

	// Custom Modes
	UFUNCTION(BlueprintCallable, Category = "GameMode|Custom")
	FName CreateCustomMode(const FMGGameModeInfo& ModeInfo);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Custom")
	void SaveCustomMode(FName ModeID);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Custom")
	void DeleteCustomMode(FName ModeID);

	UFUNCTION(BlueprintPure, Category = "GameMode|Custom")
	TArray<FMGGameModeInfo> GetCustomModes() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnGameModeChanged OnGameModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnRulesChanged OnRulesChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnPlayerEliminated OnPlayerEliminated;

	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnDriftScoreUpdate OnDriftScoreUpdate;

protected:
	void InitializeModes();
	void InitializePlaylists();
	FMGGameModeInfo CreateModeInfo(EMGGameModeType Type) const;

private:
	UPROPERTY()
	TArray<FMGGameModeInfo> AvailableModes;

	UPROPERTY()
	TArray<FMGGameModeInfo> CustomModes;

	UPROPERTY()
	TArray<FMGPlaylistEntry> Playlists;

	FMGGameModeInfo CurrentMode;
	FMGGameModeRules CurrentRules;
	FMGPlaylistEntry CurrentPlaylist;
	FMGEliminationState EliminationState;
	TMap<FString, FMGDriftScoring> DriftScores;
};
