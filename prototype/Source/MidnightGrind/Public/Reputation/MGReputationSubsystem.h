// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGReputationSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Reputation Subsystem for Midnight Grind, managing the
 * "Street Cred" system. Reputation represents a player's standing in the
 * underground racing community - how respected they are among other racers.
 *
 * WHAT IS REPUTATION?
 * -------------------
 * In many games, reputation (or "rep") is a progression system that:
 * - Shows how experienced/skilled a player is
 * - Unlocks exclusive content at certain thresholds
 * - Affects matchmaking (experienced players face each other)
 * - Provides bragging rights and social status
 *
 * Think of it like street racing films (Fast & Furious): new racers have to
 * earn respect by winning races and proving themselves in different ways.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. REPUTATION TIERS (EMGReputationTier):
 *    As players earn reputation, they climb through ranks:
 *
 *    +---------------------------------------------------+
 *    |  LEGEND    |  Top of the food chain, feared/respected |
 *    +------------+----------------------------------------+
 *    |  ELITE     |  Highly skilled, known in the scene      |
 *    +------------+----------------------------------------+
 *    |  RESPECTED |  Proven racer, people know your name     |
 *    +------------+----------------------------------------+
 *    |  REGULAR   |  Active participant, reliable            |
 *    +------------+----------------------------------------+
 *    |  ROOKIE    |  New but showing potential               |
 *    +------------+----------------------------------------+
 *    |  UNKNOWN   |  Just starting, fresh face               |
 *    +---------------------------------------------------+
 *
 *    Each tier requires exponentially more reputation than the last.
 *
 * 2. REPUTATION CATEGORIES (EMGReputationCategory):
 *    Players can be respected for different things:
 *
 *    - OVERALL: Combined score across all categories
 *    - RACING: Winning races, podium finishes
 *    - TECHNICAL: Clean driving, consistency, no crashes
 *    - SOCIAL: Helping crew members, community participation
 *    - CREATIVE: Making popular liveries, sharing content
 *    - COMPETITIVE: Tournament performance, ranked play
 *
 *    This allows players with different play styles to gain recognition.
 *    A player who never wins but creates amazing liveries can still be
 *    "Legendary" in the Creative category.
 *
 * 3. REPUTATION GAINS (FMGReputationGain):
 *    Every action that earns reputation is logged:
 *    - What category
 *    - How much was earned
 *    - What triggered it (race win, tournament placement, etc.)
 *    - When it happened
 *
 *    This creates an activity feed: "Earned +500 Racing Rep for 1st place"
 *
 * 4. REPUTATION UNLOCKS (FMGReputationUnlock):
 *    Reaching certain tiers unlocks exclusive rewards:
 *    - Unique vehicles only Legends can buy
 *    - Special customization options
 *    - Access to exclusive events
 *    - VIP matchmaking pools
 *
 *    These rewards incentivize players to keep building reputation.
 *
 * 5. REPUTATION TITLES (FMGReputationTitle):
 *    Titles are earned at reputation milestones:
 *    - "Rookie Racer" at Rookie tier
 *    - "Street Regular" at Regular tier
 *    - "Racing Legend" at Legend tier
 *
 *    Titles are tied to specific categories, so you might be a
 *    "Creative Legend" while only being "Racing Respected."
 *
 * HOW REPUTATION IS EARNED:
 * -------------------------
 *
 * Racing Activities:
 *    - Win a race: +100 to +500 (based on field size/difficulty)
 *    - Podium finish: +50 to +200
 *    - Clean race bonus: +25 (Technical category)
 *    - New personal best: +10 (Technical category)
 *
 * Social Activities:
 *    - Join a crew: +50 (one-time)
 *    - Complete crew mission: +25
 *    - Help crew member win: +10
 *
 * Creative Activities:
 *    - Share a livery: +10
 *    - Livery gets downloaded: +1 per download
 *    - Featured creation: +500
 *
 * Competitive Activities:
 *    - Enter tournament: +25
 *    - Win tournament match: +100
 *    - Win tournament: +1000
 *
 * ARCHITECTURE OVERVIEW:
 * ----------------------
 *
 *    +------------------+
 *    |  Game Systems    |  (Race results, tournaments, social actions)
 *    +--------+---------+
 *             |
 *             v  "Player finished 1st in a race"
 *    +------------------+
 *    |   Reputation     |
 *    |   Subsystem      |  <- THIS FILE
 *    +--------+---------+
 *             |
 *             v  "Earned +200 Racing Rep, check unlocks"
 *    +------------------+
 *    |  Unlock System   |  (Titles, vehicles, events)
 *    +------------------+
 *             |
 *             v  "New title unlocked!"
 *    +------------------+
 *    |    UI System     |  (Show notification, update display)
 *    +------------------+
 *
 * DELEGATE PATTERN:
 * -----------------
 * Events broadcast when reputation changes:
 *
 *    OnReputationGained: Any rep earned
 *       -> UI shows "+200 Racing Rep" toast
 *       -> Sound effect plays
 *
 *    OnTierReached: Player reached new tier
 *       -> Big celebration animation
 *       -> New perks activated
 *       -> Matchmaking pool updated
 *
 *    OnUnlockEarned: New content available
 *       -> Notification sent
 *       -> Item added to inventory
 *
 *    OnTitleUnlocked: New title available
 *       -> Notification sent
 *       -> Title can be equipped
 *
 * CODE EXAMPLE:
 * -------------
 *    // Get the subsystem
 *    UMGReputationSubsystem* RepSys =
 *        GetGameInstance()->GetSubsystem<UMGReputationSubsystem>();
 *
 *    // After winning a race
 *    RepSys->OnRaceCompleted(
 *        1,      // Position (1st place)
 *        12,     // Total racers
 *        true    // Was it a clean race (no crashes)?
 *    );
 *
 *    // After a tournament result
 *    RepSys->OnTournamentResult(
 *        1,      // Position (winner)
 *        64      // Total participants
 *    );
 *
 *    // Check current tier
 *    EMGReputationTier RacingTier = RepSys->GetTier(EMGReputationCategory::Racing);
 *    if (RacingTier >= EMGReputationTier::Elite)
 *    {
 *        // Show elite-only content
 *    }
 *
 *    // Equip a reputation title
 *    RepSys->EquipTitle(TEXT("Title_RacingLegend"));
 *
 * MATCHMAKING IMPLICATIONS:
 * -------------------------
 * Reputation affects who you play with:
 * - Unknown/Rookie: Protected lobbies with other new players
 * - Regular/Respected: Standard matchmaking pool
 * - Elite/Legend: Competitive lobbies, face other top players
 *
 * This protects new players from being stomped by veterans while
 * giving experienced players challenging competition.
 *
 * DECAY AND PROTECTION:
 * ---------------------
 * (Implementation note - not in current version)
 * Future versions might include:
 * - Reputation decay for inactivity
 * - Protection against losing tiers from bad luck
 * - Season resets that partially reset reputation
 *
 * RELATED FILES:
 * --------------
 * - MGReputationSubsystem.cpp: Implementation of all functions
 * - MGMatchmakingSubsystem: Uses reputation for skill-based matching
 * - MGPlayerTitleSubsystem: Displays equipped reputation titles
 * - MGCrewSubsystem: Social reputation from crew activities
 * - MGLeaderboardSubsystem: Rankings based on reputation
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGReputationSubsystem.generated.h"

/**
 * Reputation System - Street Cred
 * - Unified reputation across single and multiplayer
 * - Earned through all online activities
 * - Affects matchmaking and visibility
 * - Unlocks exclusive content at reputation tiers
 * - Different reputation categories for different play styles
 */

UENUM(BlueprintType)
enum class EMGReputationTier : uint8
{
	Unknown,       // New player
	Rookie,        // Starting out
	Regular,       // Active racer
	Respected,     // Known in the scene
	Elite,         // Top tier
	Legend         // The best of the best
};

UENUM(BlueprintType)
enum class EMGReputationCategory : uint8
{
	Overall,       // Combined reputation
	Racing,        // Race wins/podiums
	Technical,     // Clean racing, consistency
	Social,        // Crew activities, community
	Creative,      // Liveries, tracks created
	Competitive    // Tournament performance
};

USTRUCT(BlueprintType)
struct FMGReputationGain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationCategory Category = EMGReputationCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

USTRUCT(BlueprintType)
struct FMGReputationLevel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationCategory Category = EMGReputationCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier Tier = EMGReputationTier::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ReputationToNextTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TierProgressPercent = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGReputationUnlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Rookie;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationCategory RequiredCategory = EMGReputationCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;
};

USTRUCT(BlueprintType)
struct FMGReputationTitle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TitleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Rookie;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationCategory RequiredCategory = EMGReputationCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEquipped = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnReputationGained, EMGReputationCategory, Category, int64, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTierReached, EMGReputationCategory, Category, EMGReputationTier, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnUnlockEarned, const FMGReputationUnlock&, Unlock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTitleUnlocked, const FMGReputationTitle&, Title);

UCLASS()
class MIDNIGHTGRIND_API UMGReputationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Reputation Queries
	UFUNCTION(BlueprintPure, Category = "Reputation")
	FMGReputationLevel GetReputationLevel(EMGReputationCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Reputation")
	int64 GetReputation(EMGReputationCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Reputation")
	EMGReputationTier GetTier(EMGReputationCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Reputation")
	FText GetTierName(EMGReputationTier Tier) const;

	// Reputation Earning
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void AddReputation(EMGReputationCategory Category, int64 Amount, const FString& Source);

	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnRaceCompleted(int32 Position, int32 TotalRacers, bool bWasCleanRace);

	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnTournamentResult(int32 Position, int32 TotalParticipants);

	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnCrewActivity(const FString& ActivityType);

	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void OnCreationShared(bool bIsLivery, int32 Downloads);

	// Unlocks
	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	TArray<FMGReputationUnlock> GetAllUnlocks() const { return Unlocks; }

	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	TArray<FMGReputationUnlock> GetUnlockedItems() const;

	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	TArray<FMGReputationUnlock> GetPendingUnlocks() const;

	UFUNCTION(BlueprintPure, Category = "Reputation|Unlocks")
	bool HasUnlock(FName UnlockID) const;

	// Titles
	UFUNCTION(BlueprintPure, Category = "Reputation|Titles")
	TArray<FMGReputationTitle> GetAllTitles() const { return Titles; }

	UFUNCTION(BlueprintPure, Category = "Reputation|Titles")
	TArray<FMGReputationTitle> GetUnlockedTitles() const;

	UFUNCTION(BlueprintCallable, Category = "Reputation|Titles")
	void EquipTitle(FName TitleID);

	UFUNCTION(BlueprintPure, Category = "Reputation|Titles")
	FMGReputationTitle GetEquippedTitle() const;

	// History
	UFUNCTION(BlueprintPure, Category = "Reputation")
	TArray<FMGReputationGain> GetRecentGains(int32 Count = 20) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnReputationGained OnReputationGained;

	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnTierReached OnTierReached;

	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnUnlockEarned OnUnlockEarned;

	UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
	FMGOnTitleUnlocked OnTitleUnlocked;

protected:
	void LoadReputationData();
	void SaveReputationData();
	void InitializeUnlocks();
	void InitializeTitles();
	void UpdateTier(EMGReputationCategory Category);
	void CheckUnlocks();
	void CheckTitles();
	int64 GetReputationForTier(EMGReputationTier Tier) const;

private:
	UPROPERTY()
	TMap<EMGReputationCategory, FMGReputationLevel> ReputationLevels;

	UPROPERTY()
	TArray<FMGReputationUnlock> Unlocks;

	UPROPERTY()
	TArray<FMGReputationTitle> Titles;

	UPROPERTY()
	TArray<FMGReputationGain> GainHistory;

	FName EquippedTitleID;
	int32 MaxGainHistory = 100;
};
