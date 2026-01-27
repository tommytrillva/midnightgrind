// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGRivalsSubsystem.h
 * @brief Dynamic rivalry system that turns real players into meaningful opponents
 *
 * The Rivals System creates persistent, narrative-driven relationships with other
 * players you encounter in races. Unlike static NPC rivals, these are real players
 * whose repeated encounters build into genuine rivalries over time.
 *
 * @section rivals_concept_sec Core Concept
 * Every player you race against is tracked. As you encounter the same players
 * repeatedly, the system builds a rivalry profile based on:
 * - Win/loss record between you
 * - Frequency of encounters
 * - Closeness of race finishes
 * - Streaks and dramatic moments
 *
 * @section intensity_sec Rivalry Intensity Levels
 * Rivalries evolve through five intensity stages:
 * - **Neutral**: Just another racer in the field
 * - **Acquaintance**: You've raced a few times, starting to recognize them
 * - **Competitor**: Regular opponent, competitive dynamic forming
 * - **Rival**: True rivalry established, races feel personal
 * - **Nemesis**: Ultimate rival - only one player can hold this designation
 *
 * @section nemesis_sec The Nemesis System
 * Each player can designate one rival as their Nemesis - their ultimate opponent.
 * The Nemesis relationship provides:
 * - Special UI treatment and notifications
 * - Bonus rewards for defeating your Nemesis
 * - Matchmaking priority to enable rematch opportunities
 * - Story progression tied to Nemesis encounters
 *
 * @section crew_integration_sec Crew Integration
 * When a rival joins your crew (or vice versa), the rivalry can transform into
 * an alliance, turning a former opponent into a teammate.
 *
 * @see UMGSocialSubsystem For crew membership that affects rivalries
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRivalsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGRivalryIntensity : uint8
{
	Neutral,       // Just another racer
	Acquaintance,  // Raced a few times
	Competitor,    // Regular competitor
	Rival,         // True rivalry
	Nemesis        // Ultimate rival
};

UENUM(BlueprintType)
enum class EMGRivalStatus : uint8
{
	Active,
	Dormant,       // Haven't raced in a while
	Defeated,      // You're dominant
	Dominant,      // They're dominant
	Ally           // Joined same crew
};

USTRUCT(BlueprintType)
struct FMGRivalRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 YourWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TheirWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRaceDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0; // Positive = your streak, negative = their streak
};

USTRUCT(BlueprintType)
struct FMGRival
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RivalPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RivalName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RivalCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalryIntensity Intensity = EMGRivalryIntensity::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalStatus Status = EMGRivalStatus::Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRivalRecord Record;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FavoriteVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FavoriteTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalryScore = 0; // Determines intensity

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstEncounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNemesis = false;
};

USTRUCT(BlueprintType)
struct FMGRivalEncounter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RivalPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 YourPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TheirPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinishTimeDifference = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasCloseRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNewRivalDiscovered, const FMGRival&, Rival);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRivalryIntensified, const FMGRival&, Rival, EMGRivalryIntensity, NewIntensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNemesisDesignated, const FMGRival&, Nemesis);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRivalDefeated, const FMGRival&, Rival, bool, bWasCloseRace);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRivalBecameAlly, const FMGRival&, FormerRival);

UCLASS()
class MIDNIGHTGRIND_API UMGRivalsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Rival Management
	UFUNCTION(BlueprintPure, Category = "Rivals")
	TArray<FMGRival> GetAllRivals() const { return Rivals; }

	UFUNCTION(BlueprintPure, Category = "Rivals")
	TArray<FMGRival> GetActiveRivals() const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	FMGRival GetRival(const FString& PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	bool IsRival(const FString& PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	FMGRival GetNemesis() const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	bool HasNemesis() const;

	// Race Integration
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void RecordEncounter(const FMGRivalEncounter& Encounter);

	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void OnRaceWithPlayer(const FString& PlayerID, const FText& PlayerName, int32 YourPosition, int32 TheirPosition, float TimeDiff);

	// Rivalry Queries
	UFUNCTION(BlueprintPure, Category = "Rivals")
	TArray<FMGRival> GetRivalsByIntensity(EMGRivalryIntensity MinIntensity) const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	FMGRivalRecord GetRecordAgainst(const FString& PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	int32 GetTotalRivalryWins() const;

	// Nemesis System
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void SetNemesis(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void ClearNemesis();

	// Crew Integration
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void OnPlayerJoinedSameCrew(const FString& PlayerID);

	// Matchmaking Hints
	UFUNCTION(BlueprintPure, Category = "Rivals")
	TArray<FString> GetPreferredOpponents() const;

	UFUNCTION(BlueprintPure, Category = "Rivals")
	bool ShouldPrioritizeRivalMatch() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FMGOnNewRivalDiscovered OnNewRivalDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FMGOnRivalryIntensified OnRivalryIntensified;

	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FMGOnNemesisDesignated OnNemesisDesignated;

	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FMGOnRivalDefeated OnRivalDefeated;

	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FMGOnRivalBecameAlly OnRivalBecameAlly;

protected:
	void LoadRivalData();
	void SaveRivalData();
	FMGRival* FindRival(const FString& PlayerID);
	void UpdateRivalryIntensity(FMGRival& Rival);
	void CheckForNewNemesis();
	int32 CalculateRivalryScore(const FMGRival& Rival) const;

private:
	UPROPERTY()
	TArray<FMGRival> Rivals;

	FString CurrentNemesisID;
	int32 MaxRivals = 50;
};
