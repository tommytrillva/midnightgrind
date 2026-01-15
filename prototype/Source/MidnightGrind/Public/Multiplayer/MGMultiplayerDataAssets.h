// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Multiplayer/MGMultiplayerSubsystem.h"
#include "MGMultiplayerDataAssets.generated.h"

/**
 * Matchmaking tier/bracket
 */
USTRUCT(BlueprintType)
struct FMGMatchmakingBracket
{
	GENERATED_BODY()

	/** Bracket name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BracketID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Minimum reputation to enter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinReputation = 0;

	/** Maximum reputation for bracket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxReputation = 1000;

	/** Minimum players to start match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayers = 2;

	/** Maximum players in match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	/** Maximum wait time before expanding search */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxWaitTime = 60.0f;
};

/**
 * Ranked season settings
 */
USTRUCT(BlueprintType)
struct FMGRankedSeason
{
	GENERATED_BODY()

	/** Season identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	/** Season name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SeasonName;

	/** Season start date */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	/** Season end date */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	/** Placement matches required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlacementMatches = 10;

	/** Rank tiers (Bronze, Silver, Gold, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> RankTiers;

	/** Points per rank tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> TierThresholds;
};

/**
 * Race rewards configuration
 */
USTRUCT(BlueprintType)
struct FMGRaceRewards
{
	GENERATED_BODY()

	/** Cash per position (1st, 2nd, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> CashByPosition;

	/** Reputation per position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ReputationByPosition;

	/** Bonus for clean race (no wall hits) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRaceBonus = 100;

	/** Bonus for personal best */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PersonalBestBonus = 50;

	/** Bonus for track record */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackRecordBonus = 500;

	/** Multiplier for ranked matches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RankedMultiplier = 1.5f;

	FMGRaceRewards()
	{
		// Default rewards
		CashByPosition = { 1000, 750, 500, 400, 300, 200, 150, 100 };
		ReputationByPosition = { 100, 75, 50, 40, 30, 20, 15, 10 };
	}
};

/**
 * Network settings
 */
USTRUCT(BlueprintType)
struct FMGNetworkSettings
{
	GENERATED_BODY()

	/** Default send rate (Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultSendRate = 30.0f;

	/** Interpolation delay (ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpolationDelay = 100.0f;

	/** Max ping before warning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighPingThreshold = 150;

	/** Max ping before kick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAllowedPing = 500;

	/** Packet loss threshold for warning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PacketLossWarningThreshold = 0.05f;

	/** Reconnection timeout */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReconnectionTimeout = 30.0f;
};

/**
 * Multiplayer Settings Asset
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMultiplayerSettingsAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// MATCHMAKING
	// ==========================================

	/** Quick match brackets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	TArray<FMGMatchmakingBracket> QuickMatchBrackets;

	/** Ranked brackets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	TArray<FMGMatchmakingBracket> RankedBrackets;

	/** Enable cross-region matchmaking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	bool bEnableCrossRegion = false;

	/** Preferred regions priority */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Matchmaking")
	TArray<FString> RegionPriority;

	// ==========================================
	// LOBBY
	// ==========================================

	/** Default lap count */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	int32 DefaultLapCount = 3;

	/** Default max players */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	int32 DefaultMaxPlayers = 8;

	/** Countdown time before race */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	float RaceCountdownTime = 5.0f;

	/** Auto-start when all ready */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bAutoStartWhenReady = true;

	/** Allow late join during countdown */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lobby")
	bool bAllowLateJoin = true;

	// ==========================================
	// REWARDS
	// ==========================================

	/** Race rewards configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	FMGRaceRewards RaceRewards;

	/** DNF penalty percentage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	float DNFPenalty = 0.5f;

	/** Disconnect penalty percentage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	float DisconnectPenalty = 0.0f;

	// ==========================================
	// RANKED
	// ==========================================

	/** Current ranked season */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	FMGRankedSeason CurrentSeason;

	/** Points gained per win */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	int32 RankedPointsPerWin = 25;

	/** Points lost per loss */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	int32 RankedPointsPerLoss = 20;

	/** Streak bonus multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ranked")
	float WinStreakMultiplier = 1.5f;

	// ==========================================
	// NETWORK
	// ==========================================

	/** Network settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Network")
	FMGNetworkSettings NetworkSettings;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/** Get matchmaking bracket for reputation */
	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	FMGMatchmakingBracket GetBracketForReputation(int32 Reputation, bool bRanked) const;

	/** Calculate rewards for position */
	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	void CalculateRewards(int32 Position, bool bRanked, bool bCleanRace, bool bPersonalBest, int32& OutCash, int32& OutReputation) const;

	UMGMultiplayerSettingsAsset()
	{
		// Default quick match bracket
		FMGMatchmakingBracket DefaultBracket;
		DefaultBracket.BracketID = FName("Default");
		DefaultBracket.DisplayName = FText::FromString("Open Bracket");
		DefaultBracket.MinReputation = 0;
		DefaultBracket.MaxReputation = 999999;
		QuickMatchBrackets.Add(DefaultBracket);

		// Default regions
		RegionPriority.Add("us-east");
		RegionPriority.Add("us-west");
		RegionPriority.Add("eu-west");
		RegionPriority.Add("asia-east");
	}
};

/**
 * Anti-cheat configuration
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAntiCheatConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Enable server-side validation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	bool bEnableServerValidation = true;

	/** Maximum speed variance before flag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	float MaxSpeedVariance = 1.1f;

	/** Maximum position teleport distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	float MaxTeleportDistance = 1000.0f;

	/** Checkpoint time validation tolerance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Validation")
	float CheckpointTimeTolerance = 2.0f;

	/** Auto-ban threshold (suspicious incidents) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enforcement")
	int32 AutoBanThreshold = 5;

	/** Log suspicious activity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enforcement")
	bool bLogSuspiciousActivity = true;
};
