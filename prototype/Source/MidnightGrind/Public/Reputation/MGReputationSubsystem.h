// Copyright Midnight Grind. All Rights Reserved.

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
