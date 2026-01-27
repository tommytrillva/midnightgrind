// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRivalSubsystem.generated.h"

/**
 * Rival threat level - how dangerous they are
 */
UENUM(BlueprintType)
enum class EMGRivalThreatLevel : uint8
{
	Nuisance,       // Easy to beat, annoying at best
	Contender,      // Legitimate competition
	Dangerous,      // Serious threat, hard to beat
	Nemesis,        // Your personal nemesis, major story rival
	Legend          // Legendary racer, end-game challenge
};

/**
 * Rival personality - affects their behavior and trash talk
 */
UENUM(BlueprintType)
enum class EMGRivalPersonality : uint8
{
	Cocky,          // Arrogant, underestimates you
	Aggressive,     // Dirty racer, tries to wreck you
	Calculating,    // Strategic, always planning
	Respectful,     // Honors clean racing
	Silent,         // Says nothing, lets driving speak
	Vengeful,       // Holds grudges, seeks revenge
	Showboat        // Flashy, loves attention
};

/**
 * Rivalry intensity - how heated is this rivalry
 */
UENUM(BlueprintType)
enum class EMGRivalryIntensity : uint8
{
	Acquaintance,   // Just met, no history
	Competitive,    // Healthy competition
	Heated,         // Personal beef developing
	Bitter,         // Deep rivalry, high stakes
	LifeLong        // Career-defining rivalry
};

/**
 * Rivalry event types
 */
UENUM(BlueprintType)
enum class EMGRivalryEventType : uint8
{
	FirstEncounter,     // First race against this rival
	PlayerWon,          // You beat them
	RivalWon,           // They beat you
	CloseFinish,        // Finish within 0.5 seconds
	TrashTalkReceived,  // They said something
	TrashTalkSent,      // You taunted them
	WreckCaused,        // Collision during race
	PinkSlipWon,        // Won their car
	PinkSlipLost,       // Lost your car to them
	RevengeAchieved,    // Beat them after losing streak
	DominationAchieved, // Won 5+ consecutive against them
	RivalryBegan,       // Rivalry officially started
	RivalryEnded        // Rivalry concluded (left scene, etc.)
};

/**
 * A significant event in the rivalry history
 */
USTRUCT(BlueprintType)
struct FMGRivalryEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalryEventType EventType = EMGRivalryEventType::FirstEncounter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalPosition = 0;
};

/**
 * Rival character with personality and history
 */
USTRUCT(BlueprintType)
struct FMGRival
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RivalID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Nickname;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Backstory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalThreatLevel ThreatLevel = EMGRivalThreatLevel::Contender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalPersonality Personality = EMGRivalPersonality::Cocky;

	// Their signature vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SignatureVehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VehicleDescription;

	// Racing style/strengths
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorneringSkill = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StraightLineSkill = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DirtyTacticsTendency = 0.2f;

	// What crew they belong to (if any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrewID;

	// Are they a story/scripted rival or dynamically generated?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsStoryRival = false;

	// Trash talk lines
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> PreRaceLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> WinLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> LoseLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> CloseRaceLines;
};

/**
 * Player's relationship with a specific rival
 */
USTRUCT(BlueprintType)
struct FMGRivalry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RivalID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalryIntensity Intensity = EMGRivalryIntensity::Acquaintance;

	// Win/loss record
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	// Streak tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayerStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRivalStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestPlayerStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestRivalStreak = 0;

	// Close finish tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PhotoFinishes = 0;

	// Pink slip history
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PinkSlipsWonFromThem = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PinkSlipsLostToThem = 0;

	// Respect/animosity meter (-100 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RespectLevel = 0;

	// When did rivalry start?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstEncounter;

	// Event history
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRivalryEvent> History;

	// Is this an active rivalry?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	// Has the player "defeated" this rival (story completion)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDefeated = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalEncountered, FName, RivalID, bool, bFirstEncounter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRivalryUpdated, FName, RivalID, EMGRivalryIntensity, NewIntensity, int32, RespectLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRivalDefeated, FName, RivalID, bool, bPinkSlipWon, int32, TotalWinsAgainstThem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalTrashTalk, FName, RivalID, FText, TrashTalkLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRivalStreakEvent, FName, RivalID, bool, bPlayerStreak, int32, StreakCount);

/**
 * Manages rival characters and player rivalries
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRivalSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnRivalEncountered OnRivalEncountered;

	UPROPERTY(BlueprintAssignable)
	FOnRivalryUpdated OnRivalryUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnRivalDefeated OnRivalDefeated;

	UPROPERTY(BlueprintAssignable)
	FOnRivalTrashTalk OnRivalTrashTalk;

	UPROPERTY(BlueprintAssignable)
	FOnRivalStreakEvent OnRivalStreakEvent;

	// ==================== Rival Discovery ====================

	/** Get all known rivals */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	TArray<FMGRival> GetAllRivals() const;

	/** Get story rivals only */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	TArray<FMGRival> GetStoryRivals() const;

	/** Get rival by ID */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	bool GetRival(FName RivalID, FMGRival& OutRival) const;

	/** Get rivals by threat level */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	TArray<FMGRival> GetRivalsByThreatLevel(EMGRivalThreatLevel ThreatLevel) const;

	/** Get rivals in a specific crew */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	TArray<FMGRival> GetRivalsInCrew(FName CrewID) const;

	// ==================== Rivalry Management ====================

	/** Get player's rivalry with a specific rival */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	FMGRivalry GetRivalry(FName RivalID) const;

	/** Get all active rivalries */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	TArray<FMGRivalry> GetActiveRivalries() const;

	/** Get the player's nemesis (most intense rivalry) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	FName GetCurrentNemesis() const;

	/** Get rivals the player has defeated */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	TArray<FName> GetDefeatedRivals() const;

	/** Check if player has faced this rival before */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	bool HasFacedRival(FName RivalID) const;

	// ==================== Race Integration ====================

	/** Called when a race completes with rival involved */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	void OnRaceCompleted(FName RaceID, FName RivalID, int32 PlayerPosition, int32 RivalPosition, float TimeDifference, bool bWasPinkSlip);

	/** Record a collision/contact during race */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	void RecordRaceContact(FName RivalID, bool bPlayerCausedIt);

	// ==================== Trash Talk ====================

	/** Get appropriate trash talk line for situation */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	FText GetTrashTalkLine(FName RivalID, EMGRivalryEventType Context) const;

	/** Trigger trash talk event */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	void TriggerTrashTalk(FName RivalID, EMGRivalryEventType Context);

	// ==================== Respect/Animosity ====================

	/** Get respect level with rival (-100 to 100) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	int32 GetRespectLevel(FName RivalID) const;

	/** Modify respect level (positive = respect, negative = animosity) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	void ModifyRespect(FName RivalID, int32 Amount);

	/** Is this rival your friend or enemy? */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	bool IsRivalFriendly(FName RivalID) const;

	// ==================== Statistics ====================

	/** Get win/loss ratio against a rival */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	float GetWinRatioAgainst(FName RivalID) const;

	/** Get total races against all rivals */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	int32 GetTotalRivalRaces() const;

	/** Get most-raced rival */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	FName GetMostFrequentRival() const;

	/** Get current domination streaks */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	void GetDominationStreaks(TMap<FName, int32>& OutPlayerStreaks, TMap<FName, int32>& OutRivalStreaks) const;

	// ==================== Story Progression ====================

	/** Mark a story rival as defeated */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	void DefeatStoryRival(FName RivalID);

	/** Check if a story rival can be challenged */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	bool CanChallengeStoryRival(FName RivalID) const;

	/** Get the next story rival to face */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	FName GetNextStoryRival() const;

	// ==================== Utility ====================

	/** Get rivalry intensity name for UI */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	static FText GetIntensityDisplayName(EMGRivalryIntensity Intensity);

	/** Get threat level name for UI */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Rivals")
	static FText GetThreatDisplayName(EMGRivalThreatLevel ThreatLevel);

protected:
	void InitializeStoryRivals();
	void UpdateRivalryIntensity(FName RivalID);
	void CheckForDefeat(FName RivalID);
	void RecordRivalryEvent(FName RivalID, EMGRivalryEventType EventType, const FText& Description, int32 PlayerPos = 0, int32 RivalPos = 0);

private:
	// All rival characters
	UPROPERTY()
	TMap<FName, FMGRival> Rivals;

	// Player's rivalries
	UPROPERTY()
	TMap<FName, FMGRivalry> Rivalries;

	// Story rival progression order
	UPROPERTY()
	TArray<FName> StoryRivalOrder;

	// Current story rival index
	UPROPERTY()
	int32 CurrentStoryRivalIndex = 0;
};
