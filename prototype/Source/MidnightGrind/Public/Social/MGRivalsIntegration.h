// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRivalsIntegration.generated.h"

class UMGRivalsSubsystem;
class UMGCareerSubsystem;
class UMGProgressionSubsystem;
class AMGRaceGameMode;

/**
 * Rivalry intensity level
 */
UENUM(BlueprintType)
enum class EMGRivalryIntensity : uint8
{
	/** Casual competitor */
	Acquaintance,
	/** Regular opponent */
	Competitor,
	/** Heated rivalry */
	Rival,
	/** Intense nemesis */
	Nemesis,
	/** Ultimate enemy */
	ArchNemesis
};

/**
 * Rivalry event type
 */
UENUM(BlueprintType)
enum class EMGRivalryEvent : uint8
{
	/** First encounter */
	FirstMeet,
	/** Beat rival in race */
	Victory,
	/** Lost to rival */
	Defeat,
	/** Close finish (within 0.5s) */
	PhotoFinish,
	/** Dominated (won by 10+ seconds) */
	Domination,
	/** Got dominated */
	Humiliation,
	/** Won pink slip from rival */
	PinkSlipVictory,
	/** Lost pink slip to rival */
	PinkSlipLoss,
	/** Consecutive wins against */
	WinStreak,
	/** Consecutive losses to */
	LossStreak,
	/** Denied rival's win streak */
	StreakBreaker,
	/** Passed rival at finish line */
	LastSecondPass,
	/** Got passed at finish line */
	LastSecondLoss,
	/** Both crashed (mutual takedown) */
	MutualDestruction
};

/**
 * Rival encounter data
 */
USTRUCT(BlueprintType)
struct FMGRivalEncounter
{
	GENERATED_BODY()

	/** Rival ID */
	UPROPERTY(BlueprintReadOnly)
	FName RivalID;

	/** Track where encounter happened */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Event type */
	UPROPERTY(BlueprintReadOnly)
	EMGRivalryEvent EventType = EMGRivalryEvent::FirstMeet;

	/** Player's finishing position */
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerPosition = 0;

	/** Rival's finishing position */
	UPROPERTY(BlueprintReadOnly)
	int32 RivalPosition = 0;

	/** Time gap (positive = player ahead) */
	UPROPERTY(BlueprintReadOnly)
	float TimeGap = 0.0f;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Rivalry points change */
	UPROPERTY(BlueprintReadOnly)
	int32 RivalryPointsDelta = 0;
};

/**
 * Rival profile
 */
USTRUCT(BlueprintType)
struct FMGRivalProfile
{
	GENERATED_BODY()

	/** Rival unique ID */
	UPROPERTY(BlueprintReadOnly)
	FName RivalID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Crew affiliation */
	UPROPERTY(BlueprintReadOnly)
	FName CrewID;

	/** Signature vehicle */
	UPROPERTY(BlueprintReadOnly)
	FName SignatureVehicleID;

	/** Current intensity level */
	UPROPERTY(BlueprintReadOnly)
	EMGRivalryIntensity Intensity = EMGRivalryIntensity::Acquaintance;

	/** Rivalry points (affects intensity) */
	UPROPERTY(BlueprintReadOnly)
	int32 RivalryPoints = 0;

	/** Wins against this rival */
	UPROPERTY(BlueprintReadOnly)
	int32 WinsAgainst = 0;

	/** Losses to this rival */
	UPROPERTY(BlueprintReadOnly)
	int32 LossesTo = 0;

	/** Current streak (positive = wins, negative = losses) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentStreak = 0;

	/** Best streak ever */
	UPROPERTY(BlueprintReadOnly)
	int32 BestStreak = 0;

	/** Total encounters */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalEncounters = 0;

	/** Pink slips won from rival */
	UPROPERTY(BlueprintReadOnly)
	int32 PinkSlipsWon = 0;

	/** Pink slips lost to rival */
	UPROPERTY(BlueprintReadOnly)
	int32 PinkSlipsLost = 0;

	/** Last encounter time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime LastEncounter;

	/** Rival's taunt/catchphrase */
	UPROPERTY(BlueprintReadOnly)
	FText Catchphrase;

	/** Preferred race types */
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> PreferredRaceTypes;

	/** Favorite tracks */
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> FavoriteTracks;
};

/**
 * Rivalry milestone
 */
USTRUCT(BlueprintType)
struct FMGRivalryMilestone
{
	GENERATED_BODY()

	/** Milestone ID */
	UPROPERTY(BlueprintReadOnly)
	FName MilestoneID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Description */
	UPROPERTY(BlueprintReadOnly)
	FText Description;

	/** Rival this is for */
	UPROPERTY(BlueprintReadOnly)
	FName RivalID;

	/** Reward credits */
	UPROPERTY(BlueprintReadOnly)
	int64 RewardCredits = 0;

	/** Reward reputation */
	UPROPERTY(BlueprintReadOnly)
	int32 RewardReputation = 0;

	/** Unlock item (if any) */
	UPROPERTY(BlueprintReadOnly)
	FName UnlockItemID;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bCompleted = false;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalryIntensityChanged, FName, RivalID, EMGRivalryIntensity, NewIntensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRivalEncounter, const FMGRivalEncounter&, Encounter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRivalryMilestoneComplete, const FMGRivalryMilestone&, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewRivalDiscovered, const FMGRivalProfile&, Rival);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalTaunt, FName, RivalID, FText, TauntMessage);

/**
 * Rivals Integration Manager
 * Connects the rivals system to career, matchmaking, and narrative
 *
 * Features:
 * - Tracks rivalry encounters and history
 * - Manages rivalry intensity escalation
 * - Triggers narrative events based on rivalry state
 * - Influences matchmaking to include rivals
 * - Handles rivalry milestones and rewards
 * - Generates dynamic rival taunts
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRivalsIntegration : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// ENCOUNTER PROCESSING
	// ==========================================

	/**
	 * Process race results for rivalry updates
	 * @param RaceResults The completed race results
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void ProcessRaceResults(const struct FMGRaceResults& RaceResults);

	/**
	 * Record a specific rivalry event
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void RecordRivalryEvent(FName RivalID, EMGRivalryEvent EventType, float TimeGap = 0.0f);

	// ==========================================
	// RIVAL QUERIES
	// ==========================================

	/**
	 * Get all known rivals
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	TArray<FMGRivalProfile> GetAllRivals() const;

	/**
	 * Get rivals by intensity
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	TArray<FMGRivalProfile> GetRivalsByIntensity(EMGRivalryIntensity Intensity) const;

	/**
	 * Get top rivals (highest intensity)
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	TArray<FMGRivalProfile> GetTopRivals(int32 Count = 5) const;

	/**
	 * Get rival profile
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	FMGRivalProfile GetRivalProfile(FName RivalID) const;

	/**
	 * Get encounter history with rival
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	TArray<FMGRivalEncounter> GetEncounterHistory(FName RivalID, int32 Count = 10) const;

	/**
	 * Get current nemesis (highest intensity rival)
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	FMGRivalProfile GetNemesis() const;

	/**
	 * Is this racer a rival
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Query")
	bool IsRival(FName RacerID) const;

	// ==========================================
	// MATCHMAKING INFLUENCE
	// ==========================================

	/**
	 * Get rivals to potentially include in matchmaking
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivals|Matchmaking")
	TArray<FName> GetRivalsForMatchmaking(int32 MaxCount = 2) const;

	/**
	 * Should force include rival in next race
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Matchmaking")
	bool ShouldIncludeRivalInRace(FName RivalID) const;

	/**
	 * Get suggested rival for career event
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Career")
	FName GetSuggestedRivalForEvent(FName EventID) const;

	// ==========================================
	// NARRATIVE INTEGRATION
	// ==========================================

	/**
	 * Get pending rivalry narrative events
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Narrative")
	TArray<FName> GetPendingRivalryNarratives() const;

	/**
	 * Trigger rival taunt
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivals|Narrative")
	void TriggerRivalTaunt(FName RivalID, EMGRivalryEvent Context);

	/**
	 * Get rivalry story state
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Narrative")
	FString GetRivalryStoryState(FName RivalID) const;

	// ==========================================
	// MILESTONES
	// ==========================================

	/**
	 * Get available rivalry milestones
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Milestones")
	TArray<FMGRivalryMilestone> GetAvailableMilestones() const;

	/**
	 * Get completed milestones
	 */
	UFUNCTION(BlueprintPure, Category = "Rivals|Milestones")
	TArray<FMGRivalryMilestone> GetCompletedMilestones() const;

	/**
	 * Check and award milestones
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivals|Milestones")
	void CheckMilestones(FName RivalID);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Rivalry intensity changed */
	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FOnRivalryIntensityChanged OnRivalryIntensityChanged;

	/** Rival encounter occurred */
	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FOnRivalEncounter OnRivalEncounter;

	/** Rivalry milestone completed */
	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FOnRivalryMilestoneComplete OnRivalryMilestoneComplete;

	/** New rival discovered */
	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FOnNewRivalDiscovered OnNewRivalDiscovered;

	/** Rival sent a taunt */
	UPROPERTY(BlueprintAssignable, Category = "Rivals|Events")
	FOnRivalTaunt OnRivalTaunt;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update rivalry intensity based on points */
	void UpdateRivalryIntensity(FName RivalID);

	/** Calculate rivalry points for event */
	int32 CalculateRivalryPoints(EMGRivalryEvent EventType, float TimeGap) const;

	/** Generate taunt message */
	FText GenerateTaunt(FName RivalID, EMGRivalryEvent Context) const;

	/** Check if should create rivalry */
	bool ShouldCreateRivalry(FName RacerID, const struct FMGRaceResults& Results) const;

	/** Create new rival entry */
	void CreateRival(FName RacerID, const FText& DisplayName);

	/** Get intensity threshold */
	int32 GetIntensityThreshold(EMGRivalryIntensity Intensity) const;

	/** Cache subsystems */
	void CacheSubsystems();

private:
	/** Rival profiles */
	UPROPERTY()
	TMap<FName, FMGRivalProfile> RivalProfiles;

	/** Encounter history */
	TMap<FName, TArray<FMGRivalEncounter>> EncounterHistory;

	/** Completed milestones */
	TSet<FName> CompletedMilestones;

	/** Pending narrative triggers */
	TArray<FName> PendingNarratives;

	/** Subsystem references */
	TWeakObjectPtr<UMGCareerSubsystem> CareerSubsystem;
	TWeakObjectPtr<UMGProgressionSubsystem> ProgressionSubsystem;

	/** Intensity thresholds */
	static constexpr int32 CompetitorThreshold = 100;
	static constexpr int32 RivalThreshold = 300;
	static constexpr int32 NemesisThreshold = 600;
	static constexpr int32 ArchNemesisThreshold = 1000;

	/** Max encounters to keep per rival */
	static constexpr int32 MaxEncounterHistory = 50;
};
