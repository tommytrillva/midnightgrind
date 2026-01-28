// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGRivalSubsystem.h
 * @brief Player Rivalry and Nemesis Management System
 *
 * @section overview Overview
 * The Rival Subsystem manages player-to-player rivalries, tracking race history,
 * head-to-head statistics, public callouts/challenges, and nemesis designations.
 * This creates emergent competitive narratives between players that evolve over time.
 *
 * @section beginners Key Concepts for Beginners
 *
 * @subsection what_is_rivalry What is a Rivalry?
 * A rivalry is a tracked competitive relationship between two players. When you
 * repeatedly race against the same person, the system builds a history that tracks:
 * - Win/loss records
 * - Winning/losing streaks
 * - How close races were
 * - Special moments (photo finishes, comebacks)
 *
 * Unlike friends (social connections) or crews (teams), rivalries are adversarial
 * relationships that add stakes and drama to races.
 *
 * @subsection rivalry_levels Rivalry Levels (EMGRivalryLevel)
 * Relationships progress through intensity levels:
 * - None: No history yet
 * - Noticed (1-2 races): "Oh, I've seen this racer before"
 * - Competitor (3-5 races): Regular opponent
 * - Rival (6-10 races): Serious competition brewing
 * - Nemesis (10+ heated races): Intense ongoing battle
 * - Legend (50+ races): Long-standing epic rivalry
 *
 * @subsection heat_level Heat Level
 * Heat is a 0-100 value measuring how "hot" a rivalry currently is:
 * - Increases from close races, comebacks, and callouts
 * - Decays over time if players don't race
 * - High heat = more dramatic music, effects, and rewards
 *
 * @subsection disposition Rivalry Disposition (EMGRivalryDisposition)
 * The tone of the rivalry:
 * - Neutral: Just racing, no particular feeling
 * - Friendly: Respectful competition, clean racing
 * - Heated: Aggressive competition, maybe some trash talk
 * - Hostile: Grudge match, anything goes
 *
 * @subsection nemesis_system Nemesis System
 * Players can designate ONE rival as their official "Nemesis":
 * - Manual designation via DesignateNemesis()
 * - Auto-suggested based on race history (GetSuggestedNemesis)
 * - Mutual nemeses (both designated each other) get bonus rewards
 * - Nemesis races have special UI, music, and higher stakes
 *
 * @subsection callouts Public Callouts
 * Callouts are public challenges one player issues to another:
 * - Challenge: Standard race challenge
 * - PinkSlip: Racing for vehicle ownership
 * - ShowOff: Bragging about accomplishment
 * - Taunt: Trash talk to provoke response
 * - Respect: Acknowledging a worthy opponent
 *
 * Callouts can have wagers (cash, REP, or vehicles for pink slips).
 * Other players can spectate high-profile callouts.
 *
 * @section data_structures Key Data Structures
 *
 * @subsection race_outcome FMGRivalryRaceOutcome
 * Records the details of a single race between rivals:
 * - bWon: Did this player win?
 * - MarginSeconds: By how much? (negative = lost by)
 * - bWasClose: Within 0.5 seconds
 * - bCameFromBehind: Dramatic comeback win
 *
 * @subsection h2h_stats FMGHeadToHeadStats
 * Aggregated statistics between two players:
 * - TotalRaces, Wins, Losses
 * - CurrentStreak, LongestWinStreak, LongestLossStreak
 * - AverageMarginSeconds, ClosestRace, PhotoFinishes
 * - ComebackWins, ComebackLosses
 *
 * @subsection rivalry_data FMGRivalryData
 * Complete relationship data between two players:
 * - Player identities
 * - Level and Disposition
 * - Heat level
 * - Head-to-head stats
 * - Recent race history
 * - Milestone achievements
 *
 * @section usage Usage Examples
 *
 * @subsection recording_races Recording Rival Races
 * @code
 * UMGRivalSubsystem* RivalSystem = GetGameInstance()->GetSubsystem<UMGRivalSubsystem>();
 *
 * // After a race completes
 * FMGRivalryRaceOutcome Outcome;
 * Outcome.RaceID = RaceGuid;
 * Outcome.Timestamp = FDateTime::UtcNow();
 * Outcome.RaceType = TEXT("Sprint");
 * Outcome.bWon = PlayerFinishedFirst;
 * Outcome.MarginSeconds = PlayerTime - OpponentTime; // Positive if we won
 * Outcome.bWasClose = FMath::Abs(Outcome.MarginSeconds) < 0.5f;
 * Outcome.bWasClean = !HadContactDuringRace;
 * Outcome.bCameFromBehind = WasBehindAtLastCheckpoint && PlayerFinishedFirst;
 *
 * RivalSystem->RecordRivalRace(LocalPlayerID, OpponentID, WinnerID, Outcome);
 * // This automatically creates/updates the rivalry
 * @endcode
 *
 * @subsection querying_rivalry Querying Rivalry Info
 * @code
 * // Get rivalry with specific player
 * FMGRivalryData Rivalry;
 * if (RivalSystem->GetRivalry(LocalPlayerID, OpponentID, Rivalry))
 * {
 *     UE_LOG(LogTemp, Log, TEXT("Rivalry Level: %d"), (int)Rivalry.Level);
 *     UE_LOG(LogTemp, Log, TEXT("Heat: %.1f"), Rivalry.HeatLevel);
 *     UE_LOG(LogTemp, Log, TEXT("Record: %d-%d"), Rivalry.Player1Stats.Wins, Rivalry.Player1Stats.Losses);
 * }
 *
 * // Get your top 5 rivals
 * TArray<FMGRivalryData> TopRivals = RivalSystem->GetTopRivalries(LocalPlayerID, 5);
 *
 * // Get head-to-head stats
 * FMGHeadToHeadStats Stats = RivalSystem->GetHeadToHeadStats(LocalPlayerID, OpponentID);
 * UE_LOG(LogTemp, Log, TEXT("Win streak vs this rival: %d"), Stats.CurrentStreak);
 * @endcode
 *
 * @subsection nemesis_management Managing Nemesis
 * @code
 * // Set someone as your nemesis
 * if (RivalSystem->DesignateNemesis(LocalPlayerID, ArchRivalID))
 * {
 *     // Check if it's mutual
 *     if (RivalSystem->AreMutualNemeses(LocalPlayerID, ArchRivalID))
 *     {
 *         ShowNotification("MUTUAL NEMESIS! Double rewards active!");
 *     }
 * }
 *
 * // Get system's suggested nemesis based on race history
 * FGuid SuggestedNemesis = RivalSystem->GetSuggestedNemesis(LocalPlayerID);
 * @endcode
 *
 * @subsection issuing_callouts Issuing Callouts
 * @code
 * // Issue a public challenge with wagers
 * FGuid CalloutID = RivalSystem->IssueCallout(
 *     LocalPlayerID,
 *     OpponentID,
 *     EMGCalloutType::Challenge,
 *     TEXT("You got lucky last time. Let's settle this."),
 *     10000,  // Cash wager
 *     500     // REP wager
 * );
 *
 * // Issue a pink slip challenge (racing for cars)
 * FGuid PinkSlipID = RivalSystem->IssuePinkSlipCallout(
 *     LocalPlayerID,
 *     OpponentID,
 *     MyVehicleID,
 *     TEXT("Think your ride can beat mine? Put up or shut up.")
 * );
 *
 * // Respond to a callout
 * RivalSystem->RespondToCallout(CalloutID, LocalPlayerID, EMGCalloutResponse::Accepted);
 *
 * // Spectate a public callout
 * RivalSystem->SpectateCallout(HotCalloutID, LocalPlayerID);
 * @endcode
 *
 * @subsection bonuses Rivalry Bonuses
 * @code
 * // Get bonus multipliers for racing a rival
 * float REPBonus = RivalSystem->GetRivalREPBonus(LocalPlayerID, OpponentID);
 * float CashBonus = RivalSystem->GetRivalCashBonus(LocalPlayerID, OpponentID);
 * float StreakBonus = RivalSystem->GetStreakBonus(LocalPlayerID, OpponentID);
 *
 * // Apply bonuses to race rewards
 * int32 FinalREP = BaseREP * (1.0f + REPBonus + StreakBonus);
 * int32 FinalCash = BaseCash * (1.0f + CashBonus + StreakBonus);
 * @endcode
 *
 * @section events Events/Delegates
 * - OnRivalryCreated: New rivalry established
 * - OnRivalryLevelChanged: Rivalry escalated (e.g., Competitor -> Rival)
 * - OnCalloutReceived: Someone challenged you
 * - OnCalloutResponded: Your challenge was accepted/declined
 * - OnRivalRaceComplete: Race against rival finished
 * - OnNemesisDesignated: You or someone set a nemesis
 *
 * @section leaderboards Rivalry Leaderboards
 * The system tracks global rivalry statistics:
 * - GetHottestRivalries(): Most heated current rivalries
 * - GetMostRacesRivalries(): Longest-running rivalries
 * - GetClosestRivalries(): Most evenly matched (50/50 win rate)
 *
 * @section achievements Rivalry Achievements
 * CheckRivalryAchievements() evaluates milestone progress:
 * - First rival race
 * - Win streaks against rivals
 * - Unique rivals raced
 * - Nemesis victories
 * - Callout wins
 *
 * @see MGRivalsIntegration.h For career/narrative integration
 * @see MGSocialSubsystem.h For friends/crew (non-adversarial social)
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRivalSubsystem.generated.h"

/**
 * Rivalry intensity level
 */
UENUM(BlueprintType)
enum class EMGRivalryLevel : uint8
{
	None UMETA(DisplayName = "None"),
	Noticed UMETA(DisplayName = "Noticed"),       // 1-2 races
	Competitor UMETA(DisplayName = "Competitor"), // 3-5 races
	Rival UMETA(DisplayName = "Rival"),           // 6-10 races
	Nemesis UMETA(DisplayName = "Nemesis"),       // 10+ races, heated
	Legend UMETA(DisplayName = "Legend")          // Long-standing rivalry
};

/**
 * Rivalry disposition
 */
UENUM(BlueprintType)
enum class EMGRivalryDisposition : uint8
{
	Neutral UMETA(DisplayName = "Neutral"),
	Friendly UMETA(DisplayName = "Friendly Rivalry"),
	Heated UMETA(DisplayName = "Heated Rivalry"),
	Hostile UMETA(DisplayName = "Hostile")
};

/**
 * Callout type
 */
UENUM(BlueprintType)
enum class EMGCalloutType : uint8
{
	Challenge UMETA(DisplayName = "Race Challenge"),
	PinkSlip UMETA(DisplayName = "Pink Slip Challenge"),
	ShowOff UMETA(DisplayName = "Show Off"),
	Taunt UMETA(DisplayName = "Taunt"),
	Respect UMETA(DisplayName = "Pay Respect")
};

/**
 * Callout response
 */
UENUM(BlueprintType)
enum class EMGCalloutResponse : uint8
{
	Pending UMETA(DisplayName = "Pending"),
	Accepted UMETA(DisplayName = "Accepted"),
	Declined UMETA(DisplayName = "Declined"),
	Ignored UMETA(DisplayName = "Ignored"),
	Expired UMETA(DisplayName = "Expired")
};

/**
 * Race outcome for rivalry tracking
 */
USTRUCT(BlueprintType)
struct FMGRivalryRaceOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MarginSeconds = 0.0f; // Positive = won by, Negative = lost by

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasClose = false; // Within 0.5 seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasClean = false; // No contact

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCameFromBehind = false; // Overtook in final stretch

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID;
};

/**
 * Public callout/challenge
 */
USTRUCT(BlueprintType)
struct FMGRivalryCallout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CalloutID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ChallengerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChallengerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCalloutType CalloutType = EMGCalloutType::Challenge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message; // Custom taunt/message

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCalloutResponse Response = EMGCalloutResponse::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	// Wager details
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CashWager = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 REPWager = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleWager; // For pink slip

	// Race specifications
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID;

	// Public visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPublic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ViewCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> Spectators; // Players watching
};

/**
 * Head-to-head stats
 */
USTRUCT(BlueprintType)
struct FMGHeadToHeadStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Losses = 0;

	// Streaks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0; // Positive = win streak, Negative = loss streak

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestWinStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestLossStreak = 0;

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageMarginSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestWinMargin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClosestRace = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PhotoFinishes = 0; // Races within 0.1 seconds

	// Race type breakdown
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> WinsByRaceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> LossesByRaceType;

	// Comebacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComebackWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComebackLosses = 0;
};

/**
 * Rivalry relationship data
 */
USTRUCT(BlueprintType)
struct FMGRivalryData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RivalryID;

	// Players involved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid Player1ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Player1Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid Player2ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Player2Name;

	// Rivalry state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalryLevel Level = EMGRivalryLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalryDisposition Disposition = EMGRivalryDisposition::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRace;

	// Stats from player1's perspective
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGHeadToHeadStats Player1Stats;

	// Heat/intensity (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatLevel = 0.0f;

	// Recent race history
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRivalryRaceOutcome> RecentRaces;

	// Rivalry milestones
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFirstWin = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFirst3Peat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFirst10Races = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPhotographFinish = false;

	// Public rivalry (both players acknowledge)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMutual = false;

	// Designated nemesis (manual selection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDesignatedNemesis = false;
};

/**
 * Rivalry leaderboard entry
 */
USTRUCT(BlueprintType)
struct FMGRivalryLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Player1Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Player2Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Player1Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Player2Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRivalryLevel Level = EMGRivalryLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatLevel = 0.0f;
};

/**
 * Rival notification
 */
USTRUCT(BlueprintType)
struct FMGRivalNotification
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid NotificationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RivalID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RivalName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRead = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalryCreated, FGuid, PlayerID, FMGRivalryData, Rivalry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalryLevelChanged, FGuid, RivalryID, EMGRivalryLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCalloutReceived, FGuid, PlayerID, FMGRivalryCallout, Callout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCalloutResponded, FGuid, CalloutID, FGuid, ResponderID, EMGCalloutResponse, Response);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRivalRaceComplete, FGuid, RivalryID, FGuid, WinnerID, FMGRivalryRaceOutcome, Outcome);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNemesisDesignated, FGuid, PlayerID, FGuid, NemesisID);

/**
 * Rival Subsystem
 * Manages player rivalries, callouts, and nemesis relationships
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRivalSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// RIVALRY TRACKING
	// ==========================================

	/**
	 * Record race between two players
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry")
	void RecordRivalRace(FGuid Player1ID, FGuid Player2ID, FGuid WinnerID, const FMGRivalryRaceOutcome& Outcome);

	/**
	 * Get rivalry between two players
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry")
	bool GetRivalry(FGuid Player1ID, FGuid Player2ID, FMGRivalryData& OutRivalry) const;

	/**
	 * Get all rivalries for player
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry")
	TArray<FMGRivalryData> GetPlayerRivalries(FGuid PlayerID) const;

	/**
	 * Get top rivalries for player (most intense)
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry")
	TArray<FMGRivalryData> GetTopRivalries(FGuid PlayerID, int32 MaxCount = 5) const;

	/**
	 * Get head-to-head stats
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry")
	FMGHeadToHeadStats GetHeadToHeadStats(FGuid PlayerID, FGuid OpponentID) const;

	// ==========================================
	// NEMESIS SYSTEM
	// ==========================================

	/**
	 * Designate player as nemesis
	 */
	UFUNCTION(BlueprintCallable, Category = "Nemesis")
	bool DesignateNemesis(FGuid PlayerID, FGuid NemesisID);

	/**
	 * Remove nemesis designation
	 */
	UFUNCTION(BlueprintCallable, Category = "Nemesis")
	bool RemoveNemesis(FGuid PlayerID);

	/**
	 * Get player's designated nemesis
	 */
	UFUNCTION(BlueprintPure, Category = "Nemesis")
	FGuid GetDesignatedNemesis(FGuid PlayerID) const;

	/**
	 * Get suggested nemesis (auto-detected based on race history)
	 */
	UFUNCTION(BlueprintPure, Category = "Nemesis")
	FGuid GetSuggestedNemesis(FGuid PlayerID) const;

	/**
	 * Check if players are mutual nemeses
	 */
	UFUNCTION(BlueprintPure, Category = "Nemesis")
	bool AreMutualNemeses(FGuid Player1ID, FGuid Player2ID) const;

	// ==========================================
	// CALLOUTS
	// ==========================================

	/**
	 * Issue public callout
	 */
	UFUNCTION(BlueprintCallable, Category = "Callout")
	FGuid IssueCallout(FGuid ChallengerID, FGuid TargetID, EMGCalloutType CalloutType, const FString& Message, int32 CashWager, int32 REPWager);

	/**
	 * Issue pink slip callout
	 */
	UFUNCTION(BlueprintCallable, Category = "Callout")
	FGuid IssuePinkSlipCallout(FGuid ChallengerID, FGuid TargetID, FGuid ChallengerVehicleID, const FString& Message);

	/**
	 * Respond to callout
	 */
	UFUNCTION(BlueprintCallable, Category = "Callout")
	bool RespondToCallout(FGuid CalloutID, FGuid ResponderID, EMGCalloutResponse Response);

	/**
	 * Get pending callouts for player
	 */
	UFUNCTION(BlueprintPure, Category = "Callout")
	TArray<FMGRivalryCallout> GetPendingCallouts(FGuid PlayerID) const;

	/**
	 * Get outgoing callouts for player
	 */
	UFUNCTION(BlueprintPure, Category = "Callout")
	TArray<FMGRivalryCallout> GetOutgoingCallouts(FGuid PlayerID) const;

	/**
	 * Get recent public callouts (for spectating)
	 */
	UFUNCTION(BlueprintPure, Category = "Callout")
	TArray<FMGRivalryCallout> GetRecentPublicCallouts(int32 MaxCount = 20) const;

	/**
	 * Spectate callout
	 */
	UFUNCTION(BlueprintCallable, Category = "Callout")
	bool SpectateCallout(FGuid CalloutID, FGuid SpectatorID);

	// ==========================================
	// RIVALRY BONUSES
	// ==========================================

	/**
	 * Get REP bonus for racing rival
	 */
	UFUNCTION(BlueprintPure, Category = "Bonus")
	float GetRivalREPBonus(FGuid PlayerID, FGuid OpponentID) const;

	/**
	 * Get cash bonus for racing rival
	 */
	UFUNCTION(BlueprintPure, Category = "Bonus")
	float GetRivalCashBonus(FGuid PlayerID, FGuid OpponentID) const;

	/**
	 * Get streak bonus
	 */
	UFUNCTION(BlueprintPure, Category = "Bonus")
	float GetStreakBonus(FGuid PlayerID, FGuid OpponentID) const;

	// ==========================================
	// LEADERBOARDS
	// ==========================================

	/**
	 * Get hottest rivalries leaderboard
	 */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	TArray<FMGRivalryLeaderboardEntry> GetHottestRivalries(int32 MaxEntries = 50);

	/**
	 * Get most races rivalries
	 */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	TArray<FMGRivalryLeaderboardEntry> GetMostRacesRivalries(int32 MaxEntries = 50);

	/**
	 * Get closest rivalries (most evenly matched)
	 */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	TArray<FMGRivalryLeaderboardEntry> GetClosestRivalries(int32 MaxEntries = 50);

	// ==========================================
	// NOTIFICATIONS
	// ==========================================

	/**
	 * Get rival notifications for player
	 */
	UFUNCTION(BlueprintPure, Category = "Notifications")
	TArray<FMGRivalNotification> GetRivalNotifications(FGuid PlayerID, bool bUnreadOnly = false) const;

	/**
	 * Mark notification as read
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void MarkNotificationRead(FGuid NotificationID);

	/**
	 * Mark all notifications as read
	 */
	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void MarkAllNotificationsRead(FGuid PlayerID);

	// ==========================================
	// ACHIEVEMENTS
	// ==========================================

	/**
	 * Check for rivalry achievements
	 */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FName> CheckRivalryAchievements(FGuid PlayerID) const;

	/**
	 * Get rivalry milestones progress
	 */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	void GetMilestoneProgress(FGuid PlayerID, int32& OutTotalRaces, int32& OutUniqueRivals, int32& OutNemesisWins, int32& OutCalloutWins) const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRivalryCreated OnRivalryCreated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRivalryLevelChanged OnRivalryLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCalloutReceived OnCalloutReceived;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCalloutResponded OnCalloutResponded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRivalRaceComplete OnRivalRaceComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNemesisDesignated OnNemesisDesignated;

protected:
	// All rivalries (keyed by combined player ID string)
	UPROPERTY()
	TMap<FString, FMGRivalryData> Rivalries;

	// Player nemesis mapping
	UPROPERTY()
	TMap<FGuid, FGuid> PlayerNemeses;

	// Active callouts
	UPROPERTY()
	TMap<FGuid, FMGRivalryCallout> ActiveCallouts;

	// Player notifications
	UPROPERTY()
	TMap<FGuid, TArray<FMGRivalNotification>> PlayerNotifications;

	// ==========================================
	// INTERNAL
	// ==========================================

	FString GetRivalryKey(FGuid Player1ID, FGuid Player2ID) const;
	FMGRivalryData& GetOrCreateRivalry(FGuid Player1ID, FGuid Player2ID, const FString& Player1Name, const FString& Player2Name);
	void UpdateRivalryLevel(FMGRivalryData& Rivalry);
	void UpdateHeatLevel(FMGRivalryData& Rivalry, const FMGRivalryRaceOutcome& Outcome);
	void AddRivalNotification(FGuid PlayerID, FGuid RivalID, const FString& RivalName, const FString& Message);
	void ProcessExpiredCallouts();
	void CheckRivalryMilestones(FMGRivalryData& Rivalry, FGuid PlayerID);

	// Heat decay per day
	static constexpr float HeatDecayPerDay = 2.0f;

	// Heat gain amounts
	static constexpr float HeatGainClose = 10.0f;
	static constexpr float HeatGainComeback = 8.0f;
	static constexpr float HeatGainNormal = 3.0f;
	static constexpr float HeatGainFromCallout = 15.0f;

	// Rivalry level thresholds
	static constexpr int32 NoticedThreshold = 1;
	static constexpr int32 CompetitorThreshold = 3;
	static constexpr int32 RivalThreshold = 6;
	static constexpr int32 NemesisThreshold = 10;
	static constexpr int32 LegendThreshold = 50;
};
