// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGContentGatingSubsystem.generated.h"

/**
 * Reputation tier per PRD Section 4.2
 */
UENUM(BlueprintType)
enum class EMGReputationTier : uint8
{
	Unknown UMETA(DisplayName = "Unknown (Tier 0)"),
	Rookie UMETA(DisplayName = "Rookie (Tier 1)"),
	Known UMETA(DisplayName = "Known (Tier 2)"),
	Respected UMETA(DisplayName = "Respected (Tier 3)"),
	Feared UMETA(DisplayName = "Feared (Tier 4)"),
	Legend UMETA(DisplayName = "Legend (Tier 5)")
};

/**
 * Performance class for pink slip restrictions
 */
UENUM(BlueprintType)
enum class EMGPinkSlipClass : uint8
{
	D UMETA(DisplayName = "D Class"),
	C UMETA(DisplayName = "C Class"),
	B UMETA(DisplayName = "B Class"),
	A UMETA(DisplayName = "A Class"),
	S UMETA(DisplayName = "S Class"),
	X UMETA(DisplayName = "X Class")
};

/**
 * Content type that can be gated
 */
UENUM(BlueprintType)
enum class EMGGatedContentType : uint8
{
	Location UMETA(DisplayName = "Map Location"),
	RaceType UMETA(DisplayName = "Race Type"),
	PinkSlipClass UMETA(DisplayName = "Pink Slip Class"),
	Vehicle UMETA(DisplayName = "Vehicle Purchase"),
	Part UMETA(DisplayName = "Part Purchase"),
	Feature UMETA(DisplayName = "Game Feature")
};

/**
 * Unlock requirement
 */
USTRUCT(BlueprintType)
struct FMGUnlockRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredREP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAchievementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredRaceWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredPreviousUnlock; // Must unlock something else first

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockDescription;
};

/**
 * Gated content definition
 */
USTRUCT(BlueprintType)
struct FMGGatedContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGatedContentType ContentType = EMGGatedContentType::Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGUnlockRequirement Requirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowWhenLocked = true; // Show in UI but greyed out

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LockedHintText;
};

/**
 * Location unlock data per PRD
 */
USTRUCT(BlueprintType)
struct FMGLocationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Unknown;

	// Associated race types available in this location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AvailableRaceTypes;

	// Is this a secret/hidden location?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSecretLocation = false;
};

/**
 * Race type unlock data
 */
USTRUCT(BlueprintType)
struct FMGRaceTypeUnlockData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceTypeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier RequiredTier = EMGReputationTier::Unknown;

	// Some race types require specific wins first
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCircuitWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredSprintWins = 0;
};

/**
 * Player unlock state
 */
USTRUCT(BlueprintType)
struct FMGPlayerUnlockState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentREP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReputationTier CurrentTier = EMGReputationTier::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> UnlockedLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> UnlockedRaceTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<FName> UnlockedAchievements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> RaceTypeWinCounts;

	// Pink slip eligibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSet<EMGPinkSlipClass> UnlockedPinkSlipClasses;

	// Total stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaceWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPinkSlipWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCash = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierUnlocked, FGuid, PlayerID, EMGReputationTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnContentUnlocked, FGuid, PlayerID, FName, ContentID, EMGGatedContentType, ContentType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnREPChanged, FGuid, PlayerID, int32, NewREP);

/**
 * Content Gating Subsystem
 * Manages REP-based progression and content unlocks
 * Per PRD Section 4.2: Reputation System
 */
UCLASS()
class MIDNIGHTGRIND_API UMGContentGatingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// REP MANAGEMENT
	// ==========================================

	/**
	 * Add REP to player (from race win, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "REP")
	void AddREP(FGuid PlayerID, int32 Amount, const FString& Reason);

	/**
	 * Remove REP from player (from loss, bust, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "REP")
	void RemoveREP(FGuid PlayerID, int32 Amount, const FString& Reason);

	/**
	 * Get player's current REP
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	int32 GetPlayerREP(FGuid PlayerID) const;

	/**
	 * Get player's current tier
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	EMGReputationTier GetPlayerTier(FGuid PlayerID) const;

	/**
	 * Get REP required for next tier
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	int32 GetREPForNextTier(EMGReputationTier CurrentTier) const;

	/**
	 * Get progress toward next tier (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "REP")
	float GetTierProgress(FGuid PlayerID) const;

	// ==========================================
	// REP REWARDS (per PRD Section 4.2)
	// ==========================================

	/**
	 * Calculate REP reward for race win
	 */
	UFUNCTION(BlueprintPure, Category = "REP|Rewards")
	int32 CalculateRaceWinREP(FName RaceType, float WinMargin, bool bCleanRace, bool bComeback, int32 OpponentCount) const;

	/**
	 * Calculate REP loss for race loss
	 */
	UFUNCTION(BlueprintPure, Category = "REP|Rewards")
	int32 CalculateRaceLossREP(FName RaceType, float LossMargin) const;

	/**
	 * Calculate REP loss for police bust
	 */
	UFUNCTION(BlueprintPure, Category = "REP|Rewards")
	int32 CalculateBustREPLoss(int32 HeatLevel) const;

	// ==========================================
	// CONTENT ACCESS
	// ==========================================

	/**
	 * Check if player can access location
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool CanAccessLocation(FGuid PlayerID, FName LocationID) const;

	/**
	 * Check if player can participate in race type
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool CanAccessRaceType(FGuid PlayerID, FName RaceTypeID) const;

	/**
	 * Check if player can participate in pink slip of given class
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool CanAccessPinkSlipClass(FGuid PlayerID, EMGPinkSlipClass VehicleClass) const;

	/**
	 * Check if generic content is unlocked
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	bool IsContentUnlocked(FGuid PlayerID, FName ContentID) const;

	/**
	 * Get unlock requirements for content
	 */
	UFUNCTION(BlueprintPure, Category = "Gating")
	FMGUnlockRequirement GetUnlockRequirement(FName ContentID) const;

	// ==========================================
	// UNLOCKS
	// ==========================================

	/**
	 * Get all unlocked locations for player
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FName> GetUnlockedLocations(FGuid PlayerID) const;

	/**
	 * Get all unlocked race types for player
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FName> GetUnlockedRaceTypes(FGuid PlayerID) const;

	/**
	 * Get locations available at tier
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FMGLocationData> GetLocationsByTier(EMGReputationTier Tier) const;

	/**
	 * Get next unlockable content for player
	 */
	UFUNCTION(BlueprintPure, Category = "Unlocks")
	TArray<FMGGatedContent> GetNextUnlockableContent(FGuid PlayerID) const;

	// ==========================================
	// PLAYER STATE
	// ==========================================

	/**
	 * Initialize player unlock state
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void InitializePlayer(FGuid PlayerID);

	/**
	 * Get full player unlock state
	 */
	UFUNCTION(BlueprintPure, Category = "State")
	bool GetPlayerUnlockState(FGuid PlayerID, FMGPlayerUnlockState& OutState) const;

	/**
	 * Record race win for unlock tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void RecordRaceWin(FGuid PlayerID, FName RaceType);

	/**
	 * Record pink slip win
	 */
	UFUNCTION(BlueprintCallable, Category = "State")
	void RecordPinkSlipWin(FGuid PlayerID, EMGPinkSlipClass VehicleClass);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTierUnlocked OnTierUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnContentUnlocked OnContentUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnREPChanged OnREPChanged;

protected:
	// Player states
	UPROPERTY()
	TMap<FGuid, FMGPlayerUnlockState> PlayerStates;

	// Content definitions
	UPROPERTY()
	TArray<FMGLocationData> LocationDefinitions;

	UPROPERTY()
	TArray<FMGRaceTypeUnlockData> RaceTypeDefinitions;

	UPROPERTY()
	TArray<FMGGatedContent> GatedContentDefinitions;

	// ==========================================
	// INTERNAL
	// ==========================================

	void SetupDefaultContent();
	void SetupLocations();
	void SetupRaceTypes();
	void SetupPinkSlipClasses();
	EMGReputationTier CalculateTierFromREP(int32 REP) const;
	void CheckAndUnlockContent(FGuid PlayerID);
	void UnlockTierContent(FGuid PlayerID, EMGReputationTier Tier);

	// REP thresholds per tier (cumulative)
	static constexpr int32 TierThresholds[] = {
		0,      // Tier 0: Unknown
		100,    // Tier 1: Rookie
		500,    // Tier 2: Known
		1500,   // Tier 3: Respected
		5000,   // Tier 4: Feared
		15000   // Tier 5: Legend
	};
};
