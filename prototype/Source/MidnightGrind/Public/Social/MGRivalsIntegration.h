// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGRivalsIntegration.h
 * @brief Rivals System Integration with Career, Matchmaking, and Narrative
 *
 * @section overview Overview
 * This file defines the Rivals Integration Manager, which connects the rivalry
 * tracking system to other game systems: career progression, matchmaking,
 * and narrative/story events. While MGRivalSubsystem tracks the raw rivalry data,
 * this subsystem makes rivalries meaningful by integrating them into gameplay.
 *
 * @section beginners Key Concepts for Beginners
 *
 * @subsection purpose Why Integration Matters
 * Raw rivalry data (wins, losses, streaks) is useful but doesn't create
 * memorable experiences by itself. Integration makes rivalries feel alive:
 * - Rivals appear in your races (matchmaking integration)
 * - Story missions feature your nemesis (career integration)
 * - Characters react to rivalry events (narrative integration)
 * - Special rewards unlock from rivalry milestones (progression)
 *
 * @subsection intensity Rivalry Intensity (EMGRivalryIntensity)
 * A simplified progression of how serious a rivalry has become:
 * - Acquaintance: Casual competitor, just met
 * - Competitor: Regular opponent, starting to remember them
 * - Rival: Real competition, racing is personal now
 * - Nemesis: Intense hatred/respect, major storylines
 * - ArchNemesis: Ultimate rival, legendary status
 *
 * Intensity is determined by Rivalry Points, accumulated through encounters.
 *
 * @subsection events Rivalry Events (EMGRivalryEvent)
 * Specific memorable moments that affect rivalry intensity:
 * - FirstMeet: Initial encounter
 * - Victory/Defeat: Win or loss against rival
 * - PhotoFinish: Within 0.5 seconds - dramatic!
 * - Domination: Won by 10+ seconds - humiliating
 * - PinkSlipVictory/Loss: Won or lost a vehicle
 * - WinStreak/LossStreak: Consecutive results
 * - StreakBreaker: Ended rival's win streak
 * - LastSecondPass/Loss: Passed or got passed at finish
 * - MutualDestruction: Both crashed out (creates grudge)
 *
 * @subsection points Rivalry Points
 * Events award rivalry points that determine intensity:
 * - Normal victory: +10 points
 * - Photo finish: +25 points (dramatic)
 * - Domination: +15 points
 * - Pink slip win: +50 points (high stakes)
 * - Streak breaker: +20 points
 * - Last second pass: +30 points
 *
 * Thresholds: Competitor=100, Rival=300, Nemesis=600, ArchNemesis=1000
 *
 * @section data_structures Key Data Structures
 *
 * @subsection encounter FMGRivalEncounter
 * Records a single race encounter with a rival:
 * - Where it happened (TrackID)
 * - What happened (EventType)
 * - Positions and time gap
 * - Points earned/lost
 *
 * @subsection profile FMGRivalProfile
 * Complete profile of a rival:
 * - Identity (ID, name, crew)
 * - Stats (wins, losses, streak)
 * - Personality (catchphrase, preferred races)
 * - History (encounter count, last seen)
 *
 * @subsection milestone FMGRivalryMilestone
 * Achievements for rivalry progress:
 * - "Beat [Rival] 5 times in a row"
 * - "Win a pink slip from your nemesis"
 * - Rewards credits, reputation, and unlocks
 *
 * @section integration_areas Integration Areas
 *
 * @subsection matchmaking Matchmaking Integration
 * @code
 * // When setting up a race, consider including rivals
 * TArray<FName> RivalsToInclude = RivalsIntegration->GetRivalsForMatchmaking(2);
 *
 * // The system prioritizes:
 * // 1. Current nemesis (if available)
 * // 2. High-intensity rivals
 * // 3. Players with active callouts
 * // 4. Recent opponents
 *
 * // Check if a specific rival should be forced into the race
 * if (RivalsIntegration->ShouldIncludeRivalInRace(NemesisID))
 * {
 *     // Nemesis is due for a showdown - add them!
 *     AddParticipant(NemesisID);
 * }
 * @endcode
 *
 * @subsection career Career Integration
 * @code
 * // Get the best rival to feature in a career event
 * FName SuggestedRival = RivalsIntegration->GetSuggestedRivalForEvent(EventID);
 *
 * // The system considers:
 * // - Event difficulty vs rival skill
 * // - Narrative arc (building towards confrontation)
 * // - Time since last encounter
 * // - Whether milestone would complete
 * @endcode
 *
 * @subsection narrative Narrative Integration
 * @code
 * // Check for pending story moments
 * TArray<FName> PendingNarratives = RivalsIntegration->GetPendingRivalryNarratives();
 * // Returns: ["NemesisBossRace", "RevengeOpportunity", "FinalShowdown"]
 *
 * // Get the current story state for a rivalry
 * FString StoryState = RivalsIntegration->GetRivalryStoryState(RivalID);
 * // Returns: "BuildingTension" / "ReadyForShowdown" / "PostVictory"
 *
 * // Trigger a taunt (rival sends you a message)
 * RivalsIntegration->TriggerRivalTaunt(RivalID, EMGRivalryEvent::Victory);
 * // Generates contextual trash talk based on recent events
 * @endcode
 *
 * @section usage Usage Examples
 *
 * @subsection processing_results Processing Race Results
 * @code
 * // After a race ends, process results for rivalry updates
 * void ARaceGameMode::OnRaceComplete()
 * {
 *     UMGRivalsIntegration* Rivals = GetGameInstance()->GetSubsystem<UMGRivalsIntegration>();
 *
 *     // ProcessRaceResults automatically:
 *     // 1. Detects rivalries (repeated encounters)
 *     // 2. Creates new rivals if criteria met
 *     // 3. Records encounters
 *     // 4. Awards rivalry points
 *     // 5. Updates intensity levels
 *     // 6. Checks milestones
 *     // 7. Triggers narrative events
 *     Rivals->ProcessRaceResults(RaceResults);
 * }
 * @endcode
 *
 * @subsection recording_events Recording Specific Events
 * @code
 * // Record a specific notable moment
 * Rivals->RecordRivalryEvent(
 *     RivalID,
 *     EMGRivalryEvent::LastSecondPass,
 *     0.1f  // Time gap (positive = we're ahead)
 * );
 *
 * // Record a pink slip victory
 * Rivals->RecordRivalryEvent(RivalID, EMGRivalryEvent::PinkSlipVictory);
 * @endcode
 *
 * @subsection querying_rivals Querying Rival Data
 * @code
 * // Get all known rivals
 * TArray<FMGRivalProfile> AllRivals = Rivals->GetAllRivals();
 *
 * // Get rivals at specific intensity
 * TArray<FMGRivalProfile> Nemeses = Rivals->GetRivalsByIntensity(EMGRivalryIntensity::Nemesis);
 *
 * // Get your top 5 rivals
 * TArray<FMGRivalProfile> TopRivals = Rivals->GetTopRivals(5);
 *
 * // Get encounter history
 * TArray<FMGRivalEncounter> History = Rivals->GetEncounterHistory(RivalID, 10);
 *
 * // Get THE nemesis (highest intensity rival)
 * FMGRivalProfile Nemesis = Rivals->GetNemesis();
 * @endcode
 *
 * @subsection milestones Working with Milestones
 * @code
 * // Get available milestones to work toward
 * TArray<FMGRivalryMilestone> Available = Rivals->GetAvailableMilestones();
 *
 * // Get completed milestones
 * TArray<FMGRivalryMilestone> Completed = Rivals->GetCompletedMilestones();
 *
 * // Manually check milestones (usually automatic)
 * Rivals->CheckMilestones(RivalID);
 * @endcode
 *
 * @section events Events/Delegates
 * - OnRivalryIntensityChanged: Rival leveled up (Competitor -> Rival)
 * - OnRivalEncounter: Any encounter processed
 * - OnRivalryMilestoneComplete: Achievement unlocked
 * - OnNewRivalDiscovered: First time encountering this rival
 * - OnRivalTaunt: Rival sent a taunt message
 *
 * @section taunts Dynamic Taunts
 * The system generates contextual taunts based on:
 * - Recent race results
 * - Current streak
 * - Rivalry intensity
 * - Rival's personality/catchphrase
 *
 * @code
 * // System might generate:
 * // After your loss: "Better luck next time, slowpoke!"
 * // After their loss streak: "You're getting predictable..."
 * // Before nemesis race: "This ends tonight."
 * @endcode
 *
 * @see MGRivalSubsystem.h For raw rivalry data tracking
 * @see MGCareerSubsystem.h For career mode integration
 * @see MGProgressionSubsystem.h For rewards and unlocks
 */

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
