// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCareerSubsystem.generated.h"

/**
 * Career System - Integrated with Multiplayer
 * - Your career exists in the online world
 * - Progress against real players, not just AI
 * - Story events happen during live multiplayer races
 * - Rivals are actual players you've raced against
 * - Reputation earned through all online activities
 */

UENUM(BlueprintType)
enum class EMGCareerChapter : uint8
{
	Newcomer,      // Chapter 1: Breaking into the scene
	Rising,        // Chapter 2: Making a name
	Contender,     // Chapter 3: Challenging the best
	Champion,      // Chapter 4: Proving dominance
	Legend         // Chapter 5: Cementing legacy
};

UENUM(BlueprintType)
enum class EMGCareerMilestone : uint8
{
	FirstRace,
	FirstWin,
	FirstPodium,
	JoinedCrew,
	DefeatedRival,
	WonTournament,
	ReachedContender,
	BecameChampion,
	EarnedLegendStatus
};

USTRUCT(BlueprintType)
struct FMGCareerObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCareerChapter Chapter = EMGCareerChapter::Newcomer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMainObjective = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 GrindCashReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ReputationReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockReward;
};

USTRUCT(BlueprintType)
struct FMGCareerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Podiums = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalsDefeated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TournamentsWon = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceKM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRaceTimeHours = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestWinStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentWinStreak = 0;
};

USTRUCT(BlueprintType)
struct FMGCareerProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCareerChapter CurrentChapter = EMGCareerChapter::Newcomer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChapterProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChapterProgressRequired = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGCareerMilestone> CompletedMilestones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCareerStats Stats;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChapterAdvanced, EMGCareerChapter, NewChapter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMilestoneReached, EMGCareerMilestone, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnObjectiveCompleted, const FMGCareerObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCareerProgressUpdated, int32, Progress, int32, Required);

UCLASS()
class MIDNIGHTGRIND_API UMGCareerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Career Progress
	UFUNCTION(BlueprintPure, Category = "Career")
	FMGCareerProgress GetProgress() const { return Progress; }

	UFUNCTION(BlueprintPure, Category = "Career")
	EMGCareerChapter GetCurrentChapter() const { return Progress.CurrentChapter; }

	UFUNCTION(BlueprintPure, Category = "Career")
	float GetChapterProgressPercent() const;

	UFUNCTION(BlueprintPure, Category = "Career")
	FText GetChapterName(EMGCareerChapter Chapter) const;

	// Objectives
	UFUNCTION(BlueprintPure, Category = "Career")
	TArray<FMGCareerObjective> GetCurrentObjectives() const;

	UFUNCTION(BlueprintPure, Category = "Career")
	TArray<FMGCareerObjective> GetMainObjectives() const;

	UFUNCTION(BlueprintCallable, Category = "Career")
	void UpdateObjectiveProgress(FName ObjectiveID, int32 Progress);

	// Race Integration (called by race system)
	UFUNCTION(BlueprintCallable, Category = "Career|Race")
	void OnRaceCompleted(int32 Position, int32 TotalRacers, bool bWasCleanRace, const TArray<FString>& DefeatedRivals);

	UFUNCTION(BlueprintCallable, Category = "Career|Race")
	void OnTournamentWon(FName TournamentID);

	UFUNCTION(BlueprintCallable, Category = "Career|Race")
	void OnCrewJoined(FName CrewID);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Career")
	FMGCareerStats GetStats() const { return Progress.Stats; }

	UFUNCTION(BlueprintCallable, Category = "Career")
	void AddDistance(float DistanceKM);

	UFUNCTION(BlueprintCallable, Category = "Career")
	void AddRaceTime(float TimeHours);

	// Milestones
	UFUNCTION(BlueprintPure, Category = "Career")
	bool HasCompletedMilestone(EMGCareerMilestone Milestone) const;

	UFUNCTION(BlueprintPure, Category = "Career")
	TArray<EMGCareerMilestone> GetPendingMilestones() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnChapterAdvanced OnChapterAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnMilestoneReached OnMilestoneReached;

	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnObjectiveCompleted OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Career|Events")
	FMGOnCareerProgressUpdated OnCareerProgressUpdated;

protected:
	void LoadCareerData();
	void SaveCareerData();
	void InitializeObjectives();
	void CheckChapterAdvancement();
	void CheckMilestones();
	void CompleteMilestone(EMGCareerMilestone Milestone);
	void AdvanceChapter();
	void GrantObjectiveReward(const FMGCareerObjective& Objective);

private:
	UPROPERTY()
	FMGCareerProgress Progress;

	UPROPERTY()
	TArray<FMGCareerObjective> Objectives;
};
