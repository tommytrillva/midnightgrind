// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/MGVehicleData.h"
#include "MGPlayerProgression.generated.h"

class UMGVehicleModelData;

/**
 * Crew/faction types in the game world
 */
UENUM(BlueprintType)
enum class EMGCrew : uint8
{
	None			UMETA(DisplayName = "Unaffiliated"),
	Midnight		UMETA(DisplayName = "Midnight Runners"),	// Street racing purists
	Velocity		UMETA(DisplayName = "Team Velocity"),		// Import tuners
	Chrome			UMETA(DisplayName = "Chrome Kings"),		// Muscle car enthusiasts
	Shadow			UMETA(DisplayName = "Shadow Syndicate"),	// Underground elite
	Apex			UMETA(DisplayName = "Apex Racing"),			// Professional racers
};

/**
 * Reputation tier with a crew
 */
UENUM(BlueprintType)
enum class EMGReputationTier : uint8
{
	Unknown		UMETA(DisplayName = "Unknown"),		// 0-99
	Rookie		UMETA(DisplayName = "Rookie"),		// 100-499
	Known		UMETA(DisplayName = "Known"),		// 500-1499
	Respected	UMETA(DisplayName = "Respected"),	// 1500-3999
	Feared		UMETA(DisplayName = "Feared"),		// 4000-7999
	Legend		UMETA(DisplayName = "Legend")		// 8000+
};

/**
 * Unlock types
 */
UENUM(BlueprintType)
enum class EMGUnlockType : uint8
{
	Vehicle,
	Part,
	Track,
	RaceType,
	Crew,
	Cosmetic,
	Feature
};

/**
 * A single unlock entry
 */
USTRUCT(BlueprintType)
struct FMGUnlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FName UnlockID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	EMGUnlockType Type = EMGUnlockType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlock")
	FDateTime UnlockedAt;

	FMGUnlock()
	{
		UnlockedAt = FDateTime::Now();
	}
};

/**
 * Unlock requirement
 */
USTRUCT(BlueprintType)
struct FMGUnlockRequirement
{
	GENERATED_BODY()

	/** Unique ID of the unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FName UnlockID;

	/** Type of unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	EMGUnlockType Type = EMGUnlockType::Vehicle;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FText DisplayName;

	/** Required player level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	int32 RequiredLevel = 0;

	/** Required reputation with specific crew */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	EMGCrew RequiredCrew = EMGCrew::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	int32 RequiredCrewReputation = 0;

	/** Required total wins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	int32 RequiredWins = 0;

	/** Required races completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	int32 RequiredRaces = 0;

	/** Specific race/event that must be won */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FName RequiredRaceID;

	/** Other unlocks that must be acquired first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	TArray<FName> RequiredUnlocks;
};

/**
 * Reputation with a single crew
 */
USTRUCT(BlueprintType)
struct FMGCrewReputation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGCrew Crew = EMGCrew::None;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 ReputationPoints = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGReputationTier Tier = EMGReputationTier::Unknown;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RacesForCrew = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 WinsForCrew = 0;
};

/**
 * Race statistics
 */
USTRUCT(BlueprintType)
struct FMGRaceStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 TotalRaces = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 TotalWins = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 TotalPodiums = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 PinkSlipWins = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 PinkSlipLosses = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 CircuitRaces = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 SprintRaces = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 DriftEvents = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 DragRaces = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 TimeTrials = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	float TotalDistanceDrivenKm = 0.0f;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	float TopSpeedAchievedMPH = 0.0f;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	float BestDriftScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	float TotalDriftScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 PerfectLaps = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 NearMisses = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	FTimespan TotalPlayTime;

	float GetWinRate() const
	{
		return TotalRaces > 0 ? (float)TotalWins / (float)TotalRaces : 0.0f;
	}
};

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

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, OldLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCreditsChanged, int64, NewAmount, int64, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReputationChanged, EMGCrew, Crew, int32, NewReputation, EMGReputationTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockAcquired, const FMGUnlock&, Unlock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnXPGained, int64, Amount, int64, TotalXP);

/**
 * Game Instance Subsystem for player progression
 * Handles levels, XP, reputation, unlocks, and statistics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPlayerProgression : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// LEVEL & XP
	// ==========================================

	/** Get current player level */
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
