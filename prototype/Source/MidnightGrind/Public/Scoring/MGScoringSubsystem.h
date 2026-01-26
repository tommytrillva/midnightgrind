// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGScoringSubsystem.generated.h"

/**
 * Score event type
 */
UENUM(BlueprintType)
enum class EMGScoreEventType : uint8
{
	None				UMETA(DisplayName = "None"),
	RacePosition		UMETA(DisplayName = "Race Position"),
	Drift				UMETA(DisplayName = "Drift"),
	NearMiss			UMETA(DisplayName = "Near Miss"),
	Takedown			UMETA(DisplayName = "Takedown"),
	Airtime				UMETA(DisplayName = "Airtime"),
	Nitro				UMETA(DisplayName = "Nitro"),
	SpeedTrap			UMETA(DisplayName = "Speed Trap"),
	Destruction			UMETA(DisplayName = "Destruction"),
	Overtake			UMETA(DisplayName = "Overtake"),
	CleanSection		UMETA(DisplayName = "Clean Section"),
	PerfectLanding		UMETA(DisplayName = "Perfect Landing"),
	Trick				UMETA(DisplayName = "Trick"),
	Combo				UMETA(DisplayName = "Combo"),
	Bonus				UMETA(DisplayName = "Bonus"),
	Challenge			UMETA(DisplayName = "Challenge"),
	TimeBonus			UMETA(DisplayName = "Time Bonus"),
	StyleBonus			UMETA(DisplayName = "Style Bonus")
};

/**
 * Score category
 */
UENUM(BlueprintType)
enum class EMGScoreCategory : uint8
{
	Racing				UMETA(DisplayName = "Racing"),
	Style				UMETA(DisplayName = "Style"),
	Combat				UMETA(DisplayName = "Combat"),
	Exploration			UMETA(DisplayName = "Exploration"),
	Technical			UMETA(DisplayName = "Technical"),
	Bonus				UMETA(DisplayName = "Bonus"),
	Penalty				UMETA(DisplayName = "Penalty")
};

/**
 * Score grade
 */
UENUM(BlueprintType)
enum class EMGScoreGrade : uint8
{
	F					UMETA(DisplayName = "F"),
	D					UMETA(DisplayName = "D"),
	C					UMETA(DisplayName = "C"),
	B					UMETA(DisplayName = "B"),
	A					UMETA(DisplayName = "A"),
	S					UMETA(DisplayName = "S"),
	SS					UMETA(DisplayName = "SS"),
	SSS					UMETA(DisplayName = "SSS")
};

/**
 * Score event
 */
USTRUCT(BlueprintType)
struct FMGScoreEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreEventType Type = EMGScoreEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreCategory Category = EMGScoreCategory::Racing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsChainEvent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainIndex = 0;
};

/**
 * Score chain
 */
USTRUCT(BlueprintType)
struct FMGScoreChain
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGScoreEvent> Events;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBasePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalFinalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChainTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainLength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;
};

/**
 * Player score data
 */
USTRUCT(BlueprintType)
struct FMGPlayerScore
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGScoreCategory, int32> CategoryScores;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGScoreEventType, int32> EventTypeCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGScoreEventType, int32> EventTypePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentChainLength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestChain = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestSingleEvent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreGrade CurrentGrade = EMGScoreGrade::F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGScoreEvent> RecentEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRecentEvents = 10;
};

/**
 * Score event definition
 */
USTRUCT(BlueprintType)
struct FMGScoreEventDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreEventType Type = EMGScoreEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreCategory Category = EMGScoreCategory::Racing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPoints = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPoints = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainBonusPerEvent = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChainBonus = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExtendsChainTimer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainTimeExtension = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> IconAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DisplayColor = FLinearColor::White;
};

/**
 * Score grade threshold
 */
USTRUCT(BlueprintType)
struct FMGScoreGradeThreshold
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreGrade Grade = EMGScoreGrade::F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinMultiplierAverage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinChainLength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText GradeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GradeColor = FLinearColor::White;
};

/**
 * Race score summary
 */
USTRUCT(BlueprintType)
struct FMGRaceScoreSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RaceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacingScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StyleScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CombatScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PenaltyScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGScoreGrade FinalGrade = EMGScoreGrade::F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalEvents = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestChain = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestSingleEvent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGScoreEventType, int32> EventBreakdown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LeaderboardRank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Score multiplier source
 */
USTRUCT(BlueprintType)
struct FMGScoreMultiplierSource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SourceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MultiplierValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RemainingTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPermanent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsStackable = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScoreEvent, const FString&, PlayerId, const FMGScoreEvent&, Event, int32, NewTotal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChainStarted, const FString&, PlayerId, EMGScoreEventType, StartType, int32, BasePoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnChainExtended, const FString&, PlayerId, int32, ChainLength, float, Multiplier, int32, ChainPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChainEnded, const FString&, PlayerId, int32, FinalLength, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMultiplierChanged, const FString&, PlayerId, float, OldMultiplier, float, NewMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGradeChanged, const FString&, PlayerId, EMGScoreGrade, OldGrade, EMGScoreGrade, NewGrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMilestoneScore, const FString&, PlayerId, int32, Milestone, int32, TotalScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreSummary, const FString&, PlayerId, const FMGRaceScoreSummary&, Summary);

/**
 * Scoring Subsystem
 * Manages central scoring, combos, chains, and multipliers
 */
UCLASS()
class MIDNIGHTGRIND_API UMGScoringSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnScoreEvent OnScoreEvent;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnChainStarted OnChainStarted;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnChainExtended OnChainExtended;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnChainEnded OnChainEnded;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnMultiplierChanged OnMultiplierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnGradeChanged OnGradeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnMilestoneScore OnMilestoneScore;

	UPROPERTY(BlueprintAssignable, Category = "Scoring|Events")
	FOnScoreSummary OnScoreSummary;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Scoring|Registration")
	void RegisterScoreEventType(const FMGScoreEventDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Registration")
	void RegisterGradeThreshold(const FMGScoreGradeThreshold& Threshold);

	// Score Events
	UFUNCTION(BlueprintCallable, Category = "Scoring|Events")
	FMGScoreEvent AddScore(const FString& PlayerId, EMGScoreEventType Type, int32 BasePoints = 0);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Events")
	FMGScoreEvent AddScoreWithMultiplier(const FString& PlayerId, EMGScoreEventType Type, int32 BasePoints, float ExtraMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Events")
	void AddPenalty(const FString& PlayerId, int32 PenaltyPoints, const FText& Reason);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Events")
	void AddBonusPoints(const FString& PlayerId, int32 BonusPoints, const FText& Reason);

	// Chain Management
	UFUNCTION(BlueprintCallable, Category = "Scoring|Chain")
	void StartChain(const FString& PlayerId, EMGScoreEventType StartType, int32 BasePoints);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Chain")
	void ExtendChain(const FString& PlayerId, EMGScoreEventType Type, int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Chain")
	void EndChain(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Chain")
	void BreakChain(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Scoring|Chain")
	FMGScoreChain GetActiveChain(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Chain")
	bool HasActiveChain(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Chain")
	float GetChainTimeRemaining(const FString& PlayerId) const;

	// Multipliers
	UFUNCTION(BlueprintCallable, Category = "Scoring|Multiplier")
	void AddMultiplierSource(const FString& PlayerId, const FMGScoreMultiplierSource& Source);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Multiplier")
	void RemoveMultiplierSource(const FString& PlayerId, const FString& SourceId);

	UFUNCTION(BlueprintPure, Category = "Scoring|Multiplier")
	float GetTotalMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Multiplier")
	TArray<FMGScoreMultiplierSource> GetActiveMultipliers(const FString& PlayerId) const;

	// Player Score
	UFUNCTION(BlueprintPure, Category = "Scoring|Score")
	FMGPlayerScore GetPlayerScore(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Score")
	int32 GetTotalScore(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Score")
	int32 GetCategoryScore(const FString& PlayerId, EMGScoreCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Score")
	EMGScoreGrade GetCurrentGrade(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Score")
	TArray<FMGScoreEvent> GetRecentEvents(const FString& PlayerId) const;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Scoring|Session")
	void StartScoringSession(const FString& PlayerId, const FString& RaceId);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Session")
	FMGRaceScoreSummary EndScoringSession(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Scoring|Session")
	void ResetPlayerScore(const FString& PlayerId);

	// Grade Calculation
	UFUNCTION(BlueprintPure, Category = "Scoring|Grade")
	EMGScoreGrade CalculateGrade(int32 Score, float AverageMultiplier, int32 LongestChain) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Grade")
	FMGScoreGradeThreshold GetGradeThreshold(EMGScoreGrade Grade) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Grade")
	int32 GetScoreForGrade(EMGScoreGrade Grade) const;

	// Definitions
	UFUNCTION(BlueprintPure, Category = "Scoring|Definition")
	FMGScoreEventDefinition GetEventDefinition(EMGScoreEventType Type) const;

	UFUNCTION(BlueprintPure, Category = "Scoring|Definition")
	TArray<FMGScoreEventDefinition> GetAllEventDefinitions() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Scoring|Update")
	void UpdateScoringSystem(float DeltaTime);

protected:
	void TickChains(float DeltaTime);
	void TickMultipliers(float DeltaTime);
	void UpdateGrade(const FString& PlayerId);
	void CheckMilestones(const FString& PlayerId, int32 NewTotal);
	int32 CalculateFinalPoints(int32 BasePoints, float Multiplier, const FMGScoreEventDefinition& Definition) const;
	FString GenerateEventId() const;
	FString GenerateChainId() const;

private:
	UPROPERTY()
	TMap<EMGScoreEventType, FMGScoreEventDefinition> EventDefinitions;

	UPROPERTY()
	TMap<EMGScoreGrade, FMGScoreGradeThreshold> GradeThresholds;

	UPROPERTY()
	TMap<FString, FMGPlayerScore> PlayerScores;

	UPROPERTY()
	TMap<FString, FMGScoreChain> ActiveChains;

	UPROPERTY()
	TMap<FString, TArray<FMGScoreMultiplierSource>> PlayerMultipliers;

	UPROPERTY()
	TMap<FString, FString> ActiveRaceIds;

	UPROPERTY()
	TMap<FString, TSet<int32>> ReachedMilestones;

	UPROPERTY()
	TArray<int32> ScoreMilestones;

	UPROPERTY()
	int32 EventCounter = 0;

	UPROPERTY()
	int32 ChainCounter = 0;

	FTimerHandle ScoringTickTimer;
};
