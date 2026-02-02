// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGPlayerProgression.h
 * @brief Core player progression system for Midnight Grind
 *
 * This file defines the primary progression subsystem that tracks all aspects of
 * player advancement in the game. The progression system is the backbone of the
 * Midnight Grind experience, managing:
 *
 * - **Player Leveling**: XP-based level progression with an exponential curve
 * - **Crew Reputation**: Faction standing with five distinct racing crews
 * - **Content Unlocks**: Vehicles, parts, tracks, and features earned through play
 * - **Racing Statistics**: Comprehensive tracking of player performance metrics
 *
 * ## Progression Flow
 * 1. Player completes races and earns XP/reputation
 * 2. XP accumulates toward level thresholds
 * 3. Reputation changes affect crew standing and unlock eligibility
 * 4. Level/reputation milestones trigger content unlocks
 * 5. Statistics are updated and persisted for leaderboards/achievements
 *
 * ## Integration Points
 * - MGContentGatingSubsystem: Uses reputation/level data for access control
 * - MGTransactionPipeline: Receives XP/reputation rewards from race completions
 * - MGOnlineProfile: Syncs progression data with backend services
 * - Save System: All data marked with SaveGame specifier for persistence
 *
 * @see UMGContentGatingSubsystem for REP-based content gating
 * @see UMGTransactionPipeline for reward distribution
 * @see UMGOnlineProfileSubsystem for server synchronization
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/MG_VHCL_Data.h"
#include "Core/MGSharedTypes.h"
#include "Progression/MGContentGatingSubsystem.h"
#include "RaceDirector/MGRaceDirectorSubsystem.h"
#include "MGPlayerProgression.generated.h"

class UMGVehicleModelData;

// =============================================================================
// ENUMERATIONS - Crew & Reputation Types
// =============================================================================

/**
 * @brief Racing crews/factions in the Midnight Grind world
 *
 * Each crew represents a distinct racing culture with unique aesthetics,
 * preferred vehicle types, and race styles. Players build reputation with
 * crews by participating in their events and can eventually unlock
 * crew-exclusive content.
 *
 * @note Players can have standing with multiple crews simultaneously.
 *       The "primary crew" is determined by highest reputation.
 */
UENUM(BlueprintType)
enum class EMGCrew : uint8
{
	None			UMETA(DisplayName = "Unaffiliated"),        ///< No crew affiliation
	Midnight		UMETA(DisplayName = "Midnight Runners"),	///< Street racing purists - focus on raw skill
	Velocity		UMETA(DisplayName = "Team Velocity"),		///< Import tuners - JDM and precision driving
	Chrome			UMETA(DisplayName = "Chrome Kings"),		///< Muscle car enthusiasts - American power
	Shadow			UMETA(DisplayName = "Shadow Syndicate"),	///< Underground elite - high-stakes racing
	Apex			UMETA(DisplayName = "Apex Racing"),			///< Professional racers - track-focused excellence
};

// EMGReputationTier - REMOVED (duplicate)
// Canonical definition in: Progression/MGContentGatingSubsystem.h

/**
 * @brief Categories of unlockable content
 *
 * Used to classify unlocks for filtering in UI and determining
 * which subsystem handles the unlock (Garage, Inventory, etc.).
 */
UENUM(BlueprintType)
enum class EMGUnlockType : uint8
{
	Vehicle,	///< New vehicle available for purchase/acquisition
	Part,		///< Performance or cosmetic part unlocked
	Track,		///< New track/route accessible
	RaceType,	///< New race mode (drift, drag, etc.) unlocked
	Crew,		///< Access to a new crew's content
	Cosmetic,	///< Visual customization option (paint, vinyl, etc.)
	Feature		///< Game feature unlock (tuning shop, pink slips, etc.)
};

// =============================================================================
// STRUCTURES - Unlock System
// =============================================================================

/**
 * @brief Represents a single piece of content that has been unlocked
 *
 * This struct is created when a player earns a new unlock and is stored
 * in their progression data. It contains both identification data and
 * display metadata for UI presentation.
 */
USTRUCT(BlueprintType)
struct FMGUnlock
{
	GENERATED_BODY()

	/** Unique identifier for this unlock, used for lookups and persistence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FName UnlockID;

	/** Category of unlock for filtering and routing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	EMGUnlockType Type = EMGUnlockType::Vehicle;

	/** Localized name shown in unlock notifications */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FText DisplayName;

	/** Localized description for unlock details screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FText Description;

	/** Icon texture for UI display (soft reference for lazy loading) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Timestamp when unlock was acquired (for sorting/display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FDateTime UnlockedAt;

	FMGUnlock()
	{
		UnlockedAt = FDateTime::Now();
	}
};

// FMGUnlockRequirement - MOVED TO Progression/MGContentGatingSubsystem.h
// Canonical definition in: Progression/MGContentGatingSubsystem.h

// =============================================================================
// STRUCTURES - Reputation System
// =============================================================================

/**
 * @brief Player's standing with a single racing crew
 *
 * Tracks both the raw reputation points and derived tier, along with
 * crew-specific statistics. This data determines access to crew content
 * and is displayed on the player's profile.
 */
USTRUCT(BlueprintType)
struct FMGCrewReputation
{
	GENERATED_BODY()

	/** Which crew this reputation is for */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGCrew Crew = EMGCrew::None;

	/** Raw reputation points (determines tier) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 ReputationPoints = 0;

	/** Calculated tier based on ReputationPoints thresholds */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGReputationTier Tier = EMGReputationTier::Unknown;

	/** Total races completed for this crew */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RacesForCrew = 0;

	/** Total races won for this crew */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 WinsForCrew = 0;
};

// =============================================================================
// STRUCTURES - Statistics Tracking
// =============================================================================

// FMGRaceStatistics - MOVED TO RaceDirector/MGRaceDirectorSubsystem.h
// Canonical definition in: RaceDirector/MGRaceDirectorSubsystem.h

/**
 * Player level progression data
 */
USTRUCT(BlueprintType)
struct FMGLevelProgression
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 CurrentLevel = 1;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int64 CurrentXP = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int64 TotalXPEarned = 0;

	/** Get XP required for next level */
	int64 GetXPForNextLevel() const
	{
		// XP curve: 1000 * level^1.5
		return static_cast<int64>(1000.0 * FMath::Pow(CurrentLevel, 1.5));
	}

	/** Get XP progress toward next level (0-1) */
	float GetLevelProgress() const
	{
		int64 Required = GetXPForNextLevel();
		return Required > 0 ? FMath::Clamp((float)CurrentXP / (float)Required, 0.0f, 1.0f) : 0.0f;
	}
};

// =============================================================================
// DELEGATES - Event Broadcasting
// =============================================================================

/** Broadcast when the player gains a level */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, OldLevel);

/** Broadcast when player credits change (deposit or withdrawal) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreditsChanged, int64, NewAmount, int64, Delta);

/** Broadcast when reputation with any crew changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReputationChanged, EMGCrew, Crew, int32, NewReputation, EMGReputationTier, NewTier);

/** Broadcast when a new unlock is granted to the player */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockAcquired, const FMGUnlock&, Unlock);

/** Broadcast when XP is earned (before level-up check) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGained, int64, Amount, int64, TotalXP);

/**
 * @brief Game Instance Subsystem for player progression
 *
 * Central hub for all player advancement tracking in Midnight Grind.
 * Handles levels, XP, reputation, unlocks, and statistics.
 *
 * ## Responsibilities
 * - Level/XP progression with exponential scaling
 * - Crew reputation tracking and tier calculation
 * - Content unlock management and requirement checking
 * - Race statistics accumulation
 * - Player profile data
 *
 * ## Usage
 * @code
 * UMGPlayerProgression* Progression = GetGameInstance()->GetSubsystem<UMGPlayerProgression>();
 * Progression->AddXP(500);  // Award XP
 * Progression->AddCrewReputation(EMGCrew::Midnight, 100);  // Build crew standing
 * @endcode
 *
 * @see UMGContentGatingSubsystem for REP-based content gating
 * @see UMGRaceRewardsProcessor for race reward calculations
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPlayerProgression : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// LEVEL & XP
	// ==========================================

	/**
	 * @brief Get current player level
	 * @return Current level (1+)
	 */
	UFUNCTION(BlueprintPure, Category = "Progression|Level")
	int32 GetCurrentLevel() const { return LevelProgression.CurrentLevel; }

	/** Get current XP */
	UFUNCTION(BlueprintPure, Category = "Progression|Level")
	int64 GetCurrentXP() const { return LevelProgression.CurrentXP; }

	/** Get XP needed for next level */
	UFUNCTION(BlueprintPure, Category = "Progression|Level")
	int64 GetXPForNextLevel() const { return LevelProgression.GetXPForNextLevel(); }

	/** Get level progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Progression|Level")
	float GetLevelProgress() const { return LevelProgression.GetLevelProgress(); }

	/** Add XP to the player */
	UFUNCTION(BlueprintCallable, Category = "Progression|Level")
	void AddXP(int64 Amount, bool bNotify = true);

	/** Get level progression data */
	UFUNCTION(BlueprintPure, Category = "Progression|Level")
	const FMGLevelProgression& GetLevelProgression() const { return LevelProgression; }

	// ==========================================
	// REPUTATION
	// ==========================================

	/** Get reputation with a specific crew */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	int32 GetCrewReputation(EMGCrew Crew) const;

	/** Get reputation tier with a crew */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	EMGReputationTier GetCrewReputationTier(EMGCrew Crew) const;

	/** Add reputation with a crew */
	UFUNCTION(BlueprintCallable, Category = "Progression|Reputation")
	void AddCrewReputation(EMGCrew Crew, int32 Amount);

	/** Get total reputation across all crews */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	int32 GetTotalReputation() const;

	/** Get the player's primary crew (highest reputation) */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	EMGCrew GetPrimaryCrew() const;

	/** Get all crew reputations */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	TArray<FMGCrewReputation> GetAllCrewReputations() const;

	/** Get display name for reputation tier */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	static FText GetReputationTierName(EMGReputationTier Tier);

	/** Get display name for crew */
	UFUNCTION(BlueprintPure, Category = "Progression|Reputation")
	static FText GetCrewName(EMGCrew Crew);

	// ==========================================
	// UNLOCKS
	// ==========================================

	/** Check if something is unlocked */
	UFUNCTION(BlueprintPure, Category = "Progression|Unlocks")
	bool IsUnlocked(FName UnlockID) const;

	/** Check if player meets requirements for an unlock */
	UFUNCTION(BlueprintPure, Category = "Progression|Unlocks")
	bool MeetsUnlockRequirements(const FMGUnlockRequirement& Requirement) const;

	/** Grant an unlock to the player */
	UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
	bool GrantUnlock(const FMGUnlock& Unlock);

	/** Get all unlocks */
	UFUNCTION(BlueprintPure, Category = "Progression|Unlocks")
	TArray<FMGUnlock> GetAllUnlocks() const { return Unlocks; }

	/** Get unlocks by type */
	UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
	TArray<FMGUnlock> GetUnlocksByType(EMGUnlockType Type) const;

	/** Check and grant any newly available unlocks */
	UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
	TArray<FMGUnlock> CheckAndGrantNewUnlocks();

	// ==========================================
	// STATISTICS
	// ==========================================

	/** Get race statistics */
	UFUNCTION(BlueprintPure, Category = "Progression|Stats")
	const FMGRaceStatistics& GetRaceStatistics() const { return Statistics; }

	/** Record a race result */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void RecordRaceResult(int32 Position, int32 TotalRacers, EMGCrew RaceCrew, FName RaceTypeID);

	/** Add distance driven */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void AddDistanceDriven(float DistanceKm);

	/** Record a top speed */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void RecordTopSpeed(float SpeedMPH);

	/** Record drift score */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void RecordDriftScore(float Score);

	/** Record a pink slip result */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void RecordPinkSlipResult(bool bWon);

	/** Add play time */
	UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
	void AddPlayTime(float Seconds);

	// ==========================================
	// PLAYER PROFILE
	// ==========================================

	/** Get player display name */
	UFUNCTION(BlueprintPure, Category = "Progression|Profile")
	FString GetPlayerName() const { return PlayerName; }

	/** Set player display name */
	UFUNCTION(BlueprintCallable, Category = "Progression|Profile")
	void SetPlayerName(const FString& Name);

	/** Get player card title */
	UFUNCTION(BlueprintPure, Category = "Progression|Profile")
	FText GetPlayerTitle() const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnLevelUp OnLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnReputationChanged OnReputationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnUnlockAcquired OnUnlockAcquired;

	UPROPERTY(BlueprintAssignable, Category = "Progression|Events")
	FOnXPGained OnXPGained;

protected:
	/** Check for level up after XP gain */
	void CheckLevelUp();

	/** Calculate reputation tier from points */
	static EMGReputationTier CalculateReputationTier(int32 ReputationPoints);

	/** Get or create crew reputation entry */
	FMGCrewReputation& GetOrCreateCrewReputation(EMGCrew Crew);

	// ==========================================
	// DATA
	// ==========================================

	/** Player display name */
	UPROPERTY(SaveGame)
	FString PlayerName = TEXT("Racer");

	/** Level progression */
	UPROPERTY(SaveGame)
	FMGLevelProgression LevelProgression;

	/** Reputation with each crew */
	UPROPERTY(SaveGame)
	TMap<EMGCrew, FMGCrewReputation> CrewReputations;

	/** Unlocked content */
	UPROPERTY(SaveGame)
	TArray<FMGUnlock> Unlocks;

	/** Race statistics */
	UPROPERTY(SaveGame)
	FMGRaceStatistics Statistics;

	/** Registered unlock requirements */
	UPROPERTY()
	TArray<FMGUnlockRequirement> UnlockRequirements;
};
