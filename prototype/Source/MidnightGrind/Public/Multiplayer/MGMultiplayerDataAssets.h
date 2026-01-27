// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMultiplayerDataAssets.h
 * @brief Multiplayer configuration data assets for Midnight Grind racing game.
 *
 * This file contains data asset definitions for configuring multiplayer systems:
 * - Matchmaking brackets (skill-based player grouping)
 * - Ranked season settings (competitive play seasons)
 * - Race rewards configuration (cash, reputation, bonuses)
 * - Network settings (send rates, latency thresholds)
 * - Anti-cheat configuration (validation rules)
 *
 * @section usage_mp Usage
 * Create multiplayer data assets in the Unreal Editor:
 * Right-click in Content Browser > Miscellaneous > Data Asset > [Asset Type]
 *
 * The multiplayer system uses two main assets:
 * 1. UMGMultiplayerSettingsAsset: Core matchmaking, rewards, and network config
 * 2. UMGAntiCheatConfigAsset: Server-side validation thresholds
 *
 * @section matchmaking_mp Matchmaking System
 * Players are grouped into brackets based on reputation/rank:
 * - Quick Match: Casual races with flexible skill matching
 * - Ranked: Competitive races with strict skill brackets
 *
 * @section rewards_mp Reward System
 * Race rewards scale by finishing position and include:
 * - Base cash and reputation per position
 * - Bonuses for clean races, personal bests, track records
 * - Ranked mode multipliers for increased rewards
 *
 * @see UMGMultiplayerSubsystem
 * @see UMGMultiplayerSettingsAsset
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Multiplayer/MGMultiplayerSubsystem.h"
#include "MGMultiplayerDataAssets.generated.h"

// ============================================================================
// STRUCTURE DEFINITIONS - MATCHMAKING
// ============================================================================

/**
 * Matchmaking tier/bracket
 *
 * Defines a skill bracket for grouping players in matchmaking.
 * Multiple brackets can be configured for both quick match and
 * ranked modes to ensure fair competition.
 */
USTRUCT(BlueprintType)
struct FMGMatchmakingBracket
{
	GENERATED_BODY()

	/// Unique identifier for this bracket (e.g., "Bronze", "Silver", "Gold")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BracketID;

	/// Localized display name for UI (e.g., "Bronze League")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Minimum reputation required to queue for this bracket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinReputation = 0;

	/// Maximum reputation allowed in this bracket (players above move up)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxReputation = 1000;

	/// Minimum players needed to start a race in this bracket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayers = 2;

	/// Maximum players allowed per race in this bracket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/// Seconds to wait before expanding search to adjacent brackets
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxWaitTime = 60.0f;
};

// ============================================================================
// STRUCTURE DEFINITIONS - RANKED PLAY
// ============================================================================

/**
 * Ranked season settings
 *
 * Configures a competitive ranked season with tiers and thresholds.
 * Seasons have defined start/end dates and reset player ranks
 * at the beginning of each new season.
 */
USTRUCT(BlueprintType)
struct FMGRankedSeason
{
	GENERATED_BODY()

	/// Unique identifier for this season (e.g., "S1_2024", "Season_Winter")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	/// Localized season name displayed in UI (e.g., "Season 1: Street Kings")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonName;

	/// Date and time when season begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	/// Date and time when season ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	/// Number of races required before receiving a rank (calibration period)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlacementMatches = 10;

	/// Ordered list of rank tier names (e.g., "Bronze", "Silver", "Gold", "Platinum", "Diamond")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> RankTiers;

	/// Points required to reach each tier (index matches RankTiers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> TierThresholds;
};

// ============================================================================
// STRUCTURE DEFINITIONS - REWARDS
// ============================================================================

/**
 * Race rewards configuration
 *
 * Defines the cash and reputation awarded for race performance.
 * Configure different reward amounts per finishing position and
 * bonus multipliers for special achievements.
 */
USTRUCT(BlueprintType)
struct FMGRaceRewards
{
	GENERATED_BODY()

	/// Cash awarded per position (index 0 = 1st place, index 1 = 2nd place, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> CashByPosition;

	/// Reputation awarded per position (index 0 = 1st place, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ReputationByPosition;

	/// Bonus cash for completing race without wall collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRaceBonus = 100;

	/// Bonus cash for beating personal best lap time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PersonalBestBonus = 50;

	/// Bonus cash for setting a new track record
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackRecordBonus = 500;

	/// Multiplier applied to all rewards in ranked mode (e.g., 1.5 = 50% bonus)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RankedMultiplier = 1.5f;

	FMGRaceRewards()
	{
		// Default rewards: 1st through 8th place
		CashByPosition = { 1000, 750, 500, 400, 300, 200, 150, 100 };
		ReputationByPosition = { 100, 75, 50, 40, 30, 20, 15, 10 };
	}
};

// ============================================================================
// STRUCTURE DEFINITIONS - NETWORK
// ============================================================================

/**
 * Network settings
 *
 * Configures network parameters for multiplayer races including
 * update rates, interpolation, and connection quality thresholds.
 */
USTRUCT(BlueprintType)
struct FMGNetworkSettings
{
	GENERATED_BODY()

	/// Position/state updates sent per second to server
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultSendRate = 30.0f;

	/// Milliseconds of delay for position interpolation (smooths network jitter)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpolationDelay = 100.0f;

	/// Ping threshold in ms that triggers "High Ping" warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighPingThreshold = 150;

	/// Maximum allowed ping in ms before player is disconnected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAllowedPing = 500;

	/// Packet loss percentage that triggers connection warning (0.05 = 5%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PacketLossWarningThreshold = 0.05f;

	/// Seconds to allow for reconnection after disconnect before DNF
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReconnectionTimeout = 30.0f;
};

// ============================================================================
// DATA ASSET CLASSES
// ============================================================================

/**
 * Multiplayer Settings Asset
 *
 * Master configuration for all multiplayer racing settings including
 * matchmaking, lobby defaults, rewards, ranked play, and networking.
 *
 * @section creating_mp Creating Multiplayer Settings
 * 1. Right-click in Content Browser
 * 2. Select Miscellaneous > Data Asset
 * 3. Choose MGMultiplayerSettingsAsset
 * 4. Configure brackets, rewards, and network settings
 *
 * @section brackets_mp Configuring Brackets
 * Create multiple brackets covering the full reputation range.
 * Brackets can overlap slightly to allow flexible matchmaking.
 * Quick match brackets are typically broader; ranked brackets are stricter.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMultiplayerSettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// MATCHMAKING
	// ==========================================
	// Skill bracket configuration for player matching.

	/// Skill brackets for quick match (casual) races
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	TArray<FMGMatchmakingBracket> QuickMatchBrackets;

	/// Skill brackets for ranked (competitive) races
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	TArray<FMGMatchmakingBracket> RankedBrackets;

	/// If true, allows matching players across different server regions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	bool bEnableCrossRegion = false;

	/// Preferred server regions in priority order (e.g., "us-east", "eu-west")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	TArray<FString> RegionPriority;

	// ==========================================
	// LOBBY
	// ==========================================
	// Default settings for race lobbies.

	/// Default number of laps when creating a lobby
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	int32 DefaultLapCount = 3;

	/// Default maximum players when creating a lobby
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	int32 DefaultMaxPlayers = 8;

	/// Seconds of countdown before race starts
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	float RaceCountdownTime = 5.0f;

	/// If true, race starts immediately when all players are ready
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bAutoStartWhenReady = true;

	/// If true, allows players to join during countdown phase
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bAllowLateJoin = true;

	// ==========================================
	// REWARDS
	// ==========================================
	// Cash and reputation payout configuration.

	/// Base rewards configuration per position
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	FMGRaceRewards RaceRewards;

	/// Reward multiplier for players who DNF (Did Not Finish), 0.5 = 50% of base
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	float DNFPenalty = 0.5f;

	/// Reward multiplier for players who disconnect mid-race (0 = no rewards)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	float DisconnectPenalty = 0.0f;

	// ==========================================
	// RANKED
	// ==========================================
	// Competitive ranked mode settings.

	/// Configuration for the current ranked season
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	FMGRankedSeason CurrentSeason;

	/// Ranked points earned for winning a race
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	int32 RankedPointsPerWin = 25;

	/// Ranked points lost when losing a race
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	int32 RankedPointsPerLoss = 20;

	/// Point bonus multiplier for consecutive wins (e.g., 1.5 = 50% extra on streak)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	float WinStreakMultiplier = 1.5f;

	// ==========================================
	// NETWORK
	// ==========================================
	// Connection and synchronization settings.

	/// Network configuration for multiplayer synchronization
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network")
	FMGNetworkSettings NetworkSettings;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/**
	 * Finds the appropriate matchmaking bracket for a player's reputation.
	 * @param Reputation The player's current reputation points
	 * @param bRanked True to check ranked brackets, false for quick match
	 * @return The matching bracket, or default bracket if none found
	 */
	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	FMGMatchmakingBracket GetBracketForReputation(int32 Reputation, bool bRanked) const;

	/**
	 * Calculates total rewards for a race result.
	 * @param Position Finishing position (0 = 1st place)
	 * @param bRanked True if this was a ranked race
	 * @param bCleanRace True if player had no wall collisions
	 * @param bPersonalBest True if player achieved personal best
	 * @param OutCash [out] Total cash reward
	 * @param OutReputation [out] Total reputation reward
	 */
	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	void CalculateRewards(int32 Position, bool bRanked, bool bCleanRace, bool bPersonalBest, int32& OutCash, int32& OutReputation) const;

	UMGMultiplayerSettingsAsset()
	{
		// Set up a default open bracket for quick play
		FMGMatchmakingBracket DefaultBracket;
		DefaultBracket.BracketID = FName("Default");
		DefaultBracket.DisplayName = FText::FromString("Open Bracket");
		DefaultBracket.MinReputation = 0;
		DefaultBracket.MaxReputation = 999999;
		QuickMatchBrackets.Add(DefaultBracket);

		// Default region priority (adjust based on player base distribution)
		RegionPriority.Add("us-east");
		RegionPriority.Add("us-west");
		RegionPriority.Add("eu-west");
		RegionPriority.Add("asia-east");
	}
};

// ============================================================================
// ANTI-CHEAT CONFIGURATION
// ============================================================================

/**
 * Anti-cheat configuration
 *
 * Defines server-side validation thresholds and enforcement rules
 * for detecting and handling cheating in multiplayer races.
 *
 * @section validation_ac Validation Checks
 * The anti-cheat system validates:
 * - Vehicle speed against expected maximums
 * - Position changes for impossible teleportation
 * - Checkpoint times for impossibly fast sectors
 *
 * @section enforcement_ac Enforcement
 * Suspicious activity is logged and tracked. Players exceeding
 * the AutoBanThreshold are automatically banned from ranked play.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAntiCheatConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// VALIDATION
	// ==========================================
	// Thresholds for detecting suspicious behavior.

	/// Master toggle for server-side validation checks
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bEnableServerValidation = true;

	/// Maximum allowed speed as multiplier of vehicle top speed (1.1 = 10% tolerance)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	float MaxSpeedVariance = 1.1f;

	/// Maximum distance in cm a vehicle can move between updates (teleport detection)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	float MaxTeleportDistance = 1000.0f;

	/// Tolerance in seconds for checkpoint time validation (accounts for latency)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	float CheckpointTimeTolerance = 2.0f;

	// ==========================================
	// ENFORCEMENT
	// ==========================================
	// Actions taken when cheating is detected.

	/// Number of suspicious incidents before automatic ban
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enforcement")
	int32 AutoBanThreshold = 5;

	/// If true, all suspicious activity is logged for review
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enforcement")
	bool bLogSuspiciousActivity = true;
};
