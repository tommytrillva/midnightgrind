// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGTournamentSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Tournament Subsystem for Midnight Grind, which manages
 * all aspects of competitive bracket-based tournaments in the game. Think of it
 * as the backbone for organizing racing competitions where players compete in
 * structured elimination or point-based formats.
 *
 * WHAT IS A TOURNAMENT?
 * ---------------------
 * A tournament is an organized competitive event where multiple players (or teams)
 * compete through a series of matches to determine a winner. Unlike casual racing,
 * tournaments have:
 *   - Fixed schedules (registration period, check-in, start times)
 *   - Bracket structures (who plays whom and in what order)
 *   - Elimination rules (lose and you're out, or accumulate points)
 *   - Prize pools (rewards for top placements)
 *
 * KEY CONCEPTS AND TERMINOLOGY:
 * -----------------------------
 *
 * 1. TOURNAMENT FORMATS (EMGTournamentFormat):
 *    - Single Elimination: Lose once and you're out. Fast and dramatic.
 *    - Double Elimination: Must lose twice to be eliminated. Has winners/losers brackets.
 *    - Round Robin: Everyone plays everyone. Best record wins.
 *    - Swiss System: Pairs players with similar records each round.
 *    - Group Stage: Divide into groups, top finishers advance to knockouts.
 *
 * 2. TOURNAMENT STATES (EMGTournamentState):
 *    - Announced: Tournament is visible but registration hasn't opened
 *    - Registration: Players can sign up
 *    - CheckIn: Registered players confirm they're ready to play
 *    - InProgress: Matches are being played
 *    - Finals: Final match(es) are happening
 *    - Completed/Cancelled: Tournament has ended
 *
 * 3. BRACKETS:
 *    A visual representation of the tournament structure showing all matches
 *    and how winners advance. In double elimination, there are:
 *    - Winners Bracket: Where everyone starts
 *    - Losers Bracket: Where you go after your first loss
 *    - Grand Finals: Winner of winners vs winner of losers
 *
 * 4. SEEDING:
 *    Assigning positions to players based on skill rating so top players
 *    don't face each other in early rounds.
 *
 * 5. PARTICIPANT:
 *    Can be a solo player, a team, or a crew depending on tournament type.
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UGameInstanceSubsystem, meaning it persists across level loads
 *   and exists for the entire game session.
 * - It works with UMGOnlineSubsystem for network/backend communication.
 * - Tournaments generate matches that use the core racing systems.
 * - Results feed into the Skill Rating system (UMGSkillRatingSubsystem).
 * - Works alongside the Esports Subsystem for professional/broadcast features.
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. Browse available tournaments (GetAvailableTournaments)
 * 2. Check requirements and register (CanRegisterForTournament, RegisterForTournament)
 * 3. Wait for check-in window and check in (CheckInForTournament)
 * 4. Receive match notifications (OnMatchReady delegate)
 * 5. Play matches and report results (GetCurrentMatch, ReportMatchResult)
 * 6. Advance through bracket or get eliminated
 * 7. Receive prizes if placed well (OnTournamentCompleted delegate)
 *
 * DELEGATES (Event Notifications):
 * --------------------------------
 * - OnTournamentStateChanged: Fires when tournament moves to new state
 * - OnTournamentRegistration: Fires when registration succeeds/fails
 * - OnMatchReady: Fires when your next match is ready to start
 * - OnMatchCompleted: Fires when any match finishes
 * - OnTournamentCompleted: Fires when tournament ends with final standings
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ---------------------------
 * - USTRUCT: A data container (like a C struct with Unreal reflection support)
 * - UENUM: An enumeration type exposed to Blueprints
 * - UPROPERTY: Member variable that Unreal tracks (for serialization, GC, Blueprints)
 * - UFUNCTION: Function exposed to Unreal reflection system
 * - BlueprintPure: Function with no side effects, callable from Blueprints
 * - BlueprintCallable: Function that may have side effects, callable from Blueprints
 * - GameInstanceSubsystem: A singleton that lives as long as the game instance
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTournamentSubsystem.generated.h"

class UMGOnlineSubsystem;

//=============================================================================
// ENUMS - Tournament Configuration Types
//=============================================================================

/**
 * Tournament Format Types
 *
 * Determines the structure of competition and how players advance or are eliminated.
 * Each format has different characteristics:
 *
 * - SingleElimination: Quick, high-stakes. One loss = out.
 *   Common for smaller tournaments or when time is limited.
 *   Example: 16 players = 4 rounds (16 -> 8 -> 4 -> 2 -> 1)
 *
 * - DoubleElimination: More forgiving. Two losses = out.
 *   Creates Winners Bracket (undefeated) and Losers Bracket (one loss).
 *   Grand Finals: Winners champ vs Losers champ.
 *
 * - RoundRobin: Everyone plays everyone. Best record wins.
 *   Fair but time-consuming. Best for small groups (4-8 players).
 *
 * - Swiss: Smart pairing system. Players with similar records face each other.
 *   Efficient for large player counts. Common in chess tournaments.
 *
 * - GroupStage: Divide into groups (round robin within), top finishers
 *   advance to knockout rounds. Used in World Cup, etc.
 */
UENUM(BlueprintType)
enum class EMGTournamentFormat : uint8
{
	SingleElimination	UMETA(DisplayName = "Single Elimination"),
	DoubleElimination	UMETA(DisplayName = "Double Elimination"),
	RoundRobin			UMETA(DisplayName = "Round Robin"),
	Swiss				UMETA(DisplayName = "Swiss System"),
	GroupStage			UMETA(DisplayName = "Group Stage + Knockout")
};

/**
 * Tournament State
 *
 * Represents the lifecycle of a tournament from announcement to completion.
 * Tournaments progress through these states in order (with some exceptions).
 *
 * State Flow:
 * Announced -> Registration -> CheckIn -> InProgress <-> Intermission -> Finals -> Completed
 *                                                                               \-> Cancelled
 *
 * - Announced: Tournament visible but can't register yet. Used for hype/marketing.
 * - Registration: Players can sign up. Usually opens days before the event.
 * - CheckIn: Registered players confirm attendance. Reduces no-shows.
 * - InProgress: Active matches being played.
 * - Intermission: Break between rounds (for broadcasts, player rest).
 * - Finals: The championship match(es). Grand Finals, etc.
 * - Completed: Tournament finished. Results finalized, prizes distributed.
 * - Cancelled: Tournament will not occur. Refunds issued if applicable.
 */
UENUM(BlueprintType)
enum class EMGTournamentState : uint8
{
	Announced		UMETA(DisplayName = "Announced"),
	Registration	UMETA(DisplayName = "Registration Open"),
	CheckIn			UMETA(DisplayName = "Check-In"),
	InProgress		UMETA(DisplayName = "In Progress"),
	Intermission	UMETA(DisplayName = "Intermission"),
	Finals			UMETA(DisplayName = "Finals"),
	Completed		UMETA(DisplayName = "Completed"),
	Cancelled		UMETA(DisplayName = "Cancelled")
};

/**
 * Match State
 *
 * Represents the lifecycle of an individual match within a tournament.
 * A match is a single competitive encounter between two participants.
 *
 * - Pending: Match exists but participants not yet determined
 *   (waiting for earlier round results).
 * - ReadyToStart: Both participants known and ready. Match can begin.
 * - InProgress: Match is actively being played.
 * - Completed: Match finished with a winner determined normally.
 * - Disputed: Result is contested. Requires admin intervention.
 * - Forfeited: A participant didn't show up or gave up.
 * - Bye: Only one participant. They advance automatically.
 *   (Happens in brackets with non-power-of-2 participant counts)
 */
UENUM(BlueprintType)
enum class EMGMatchState : uint8
{
	Pending			UMETA(DisplayName = "Pending"),
	ReadyToStart	UMETA(DisplayName = "Ready to Start"),
	InProgress		UMETA(DisplayName = "In Progress"),
	Completed		UMETA(DisplayName = "Completed"),
	Disputed		UMETA(DisplayName = "Disputed"),
	Forfeited		UMETA(DisplayName = "Forfeited"),
	Bye				UMETA(DisplayName = "Bye")
};

/**
 * Tournament Entry Type
 *
 * Defines what kind of participants can enter the tournament.
 *
 * - Solo: Individual players compete. Most common type.
 * - Team: Pre-formed teams compete. TeamSize in tournament data specifies members.
 *   Example: 2v2 relay racing.
 * - Crew: Crew-based competition. Players must be in a crew to enter.
 *   Represents the crew in inter-crew rivalries.
 */
UENUM(BlueprintType)
enum class EMGTournamentEntryType : uint8
{
	Solo			UMETA(DisplayName = "Solo"),
	Team			UMETA(DisplayName = "Team"),
	Crew			UMETA(DisplayName = "Crew")
};

/**
 * Tournament Tier (determines prize pools and prestige)
 *
 * Higher tiers = bigger prizes, more prestige, tougher competition.
 * Championship Points from higher tiers count more toward rankings.
 *
 * Tier Hierarchy (lowest to highest):
 * - Community: Player-organized events. Small prizes, casual.
 * - Weekly: Regular scheduled events. Modest rewards.
 * - Monthly: End-of-month championship. Good rewards.
 * - Seasonal: Every 3 months. Significant rewards and prestige.
 * - Major: Big competitive events. Large prize pools.
 * - Championship: The ultimate events. Massive rewards, exclusive titles.
 */
UENUM(BlueprintType)
enum class EMGTournamentTier : uint8
{
	Community		UMETA(DisplayName = "Community"),
	Weekly			UMETA(DisplayName = "Weekly"),
	Monthly			UMETA(DisplayName = "Monthly Championship"),
	Seasonal		UMETA(DisplayName = "Seasonal Cup"),
	Major			UMETA(DisplayName = "Major"),
	Championship	UMETA(DisplayName = "World Championship")
};

/**
 * Bracket Side (for double elimination)
 *
 * In double elimination, there are two parallel brackets:
 *
 * - Winners: Where everyone starts. You stay here until your first loss.
 *   Winning here is ideal - shorter path to championship.
 *
 * - Losers: Where you go after losing in Winners bracket.
 *   You get a second chance but have a longer road to the finals.
 *   Lose here = eliminated from tournament.
 *
 * - GrandFinals: The championship match.
 *   Winners bracket champion vs Losers bracket champion.
 *   Note: Losers champ may need to beat Winners champ twice
 *   (since Winners champ hasn't lost yet) - called "bracket reset".
 */
UENUM(BlueprintType)
enum class EMGBracketSide : uint8
{
	Winners		UMETA(DisplayName = "Winners Bracket"),
	Losers		UMETA(DisplayName = "Losers Bracket"),
	GrandFinals	UMETA(DisplayName = "Grand Finals")
};

//=============================================================================
// STRUCTS - Tournament Data Containers
//=============================================================================

/**
 * Tournament Participant (player or team)
 *
 * Represents a single entry in the tournament. This could be:
 * - A solo player
 * - A team of players
 * - A crew representation
 *
 * Contains all the data needed to identify, track, and display a participant
 * throughout the tournament lifecycle.
 */
USTRUCT(BlueprintType)
struct FMGTournamentParticipant
{
	GENERATED_BODY()

	/** Participant ID (player ID or team/crew ID) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	FString ParticipantID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	FText DisplayName;

	/** Team/Crew name if applicable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	FText TeamName;

	/** Team member IDs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	TArray<FString> MemberIDs;

	/** Seed number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	int32 Seed = 0;

	/** Current tournament wins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	int32 Wins = 0;

	/** Current tournament losses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	int32 Losses = 0;

	/** Points (for round robin/swiss) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	int32 Points = 0;

	/** Has checked in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	bool bCheckedIn = false;

	/** Is eliminated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	bool bEliminated = false;

	/** Final placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	int32 FinalPlacement = 0;

	/** Selected vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Participant")
	FName VehicleID;
};

/**
 * Tournament Match
 *
 * Represents a single scheduled encounter between two participants.
 * Tracks the state, participants, results, and bracket position of the match.
 *
 * Key fields:
 * - MatchID: Unique identifier for this match
 * - Round/MatchNumber: Position in the bracket (for display)
 * - BracketSide: Winners, Losers, or Grand Finals
 * - Participant1ID/Participant2ID: Who is competing
 * - BestOf: Number of games in series (e.g., BestOf=3 means first to 2 wins)
 * - NextMatchWinnerID/NextMatchLoserID: Where participants go after this match
 */
USTRUCT(BlueprintType)
struct FMGTournamentMatch
{
	GENERATED_BODY()

	/** Match ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString MatchID;

	/** Round number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	int32 Round = 1;

	/** Match number within round */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	int32 MatchNumber = 1;

	/** Bracket side */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	EMGBracketSide BracketSide = EMGBracketSide::Winners;

	/** Group ID for group stage tournaments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString GroupID;

	/** Participant 1 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString Participant1ID;

	/** Participant 2 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString Participant2ID;

	/** Match state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	EMGMatchState State = EMGMatchState::Pending;

	/** Winner ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString WinnerID;

	/** Loser ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString LoserID;

	/** Score for participant 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	int32 Score1 = 0;

	/** Score for participant 2 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	int32 Score2 = 0;

	/** Best of N games */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	int32 BestOf = 1;

	/** Track ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FName TrackID;

	/** Scheduled start time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FDateTime ScheduledTime;

	/** Actual start time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FDateTime StartTime;

	/** End time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FDateTime EndTime;

	/** Next match ID for winner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString NextMatchWinnerID;

	/** Next match ID for loser (double elim) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString NextMatchLoserID;
};

/**
 * Tournament Bracket Round
 *
 * Groups matches that occur at the same stage of the tournament.
 * Used for bracket visualization and to determine when to advance.
 *
 * Example rounds in a 16-player single elimination:
 * - Round 1 (8 matches): "Round of 16"
 * - Round 2 (4 matches): "Quarterfinals"
 * - Round 3 (2 matches): "Semifinals"
 * - Round 4 (1 match): "Finals"
 *
 * The RoundName field contains a display-friendly name like "Quarterfinals".
 */
USTRUCT(BlueprintType)
struct FMGBracketRound
{
	GENERATED_BODY()

	/** Round number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	int32 RoundNumber = 1;

	/** Round name (e.g., "Quarter Finals") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	FText RoundName;

	/** Bracket side */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	EMGBracketSide BracketSide = EMGBracketSide::Winners;

	/** Matches in this round */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	TArray<FString> MatchIDs;

	/** Best of N for this round */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round")
	int32 BestOf = 1;
};

/**
 * Tournament Group (for group stage)
 *
 * In GroupStage format, participants are divided into groups (like World Cup).
 * Within each group, everyone plays everyone (round robin).
 * Top finishers from each group advance to knockout rounds.
 *
 * Example with 16 players:
 * - 4 groups of 4 players each (Group A, B, C, D)
 * - Each group plays round robin (6 matches per group)
 * - Top 2 from each group advance to knockout (8 players)
 * - Quarterfinals -> Semifinals -> Finals
 */
USTRUCT(BlueprintType)
struct FMGTournamentGroup
{
	GENERATED_BODY()

	/** Group ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FString GroupID;

	/** Group name (e.g., "Group A") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FText GroupName;

	/** Participants in this group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	TArray<FString> ParticipantIDs;

	/** Matches in this group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	TArray<FString> MatchIDs;

	/** Number of players that advance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	int32 AdvancingCount = 2;
};

/**
 * Tournament Prize
 *
 * Defines rewards given to participants based on their final placement.
 * Tournaments can have multiple prize entries for different placements.
 *
 * Reward types:
 * - CashReward: In-game currency
 * - XPReward: Experience points for progression
 * - ReputationReward: Street reputation (affects unlock availability)
 * - ChampionshipPoints: Points toward seasonal rankings
 * - ItemRewards: Specific items (parts, cosmetics)
 * - TitleReward: Display title (e.g., "Weekly Champion")
 * - VehicleReward: Exclusive vehicle unlock
 */
USTRUCT(BlueprintType)
struct FMGTournamentPrize
{
	GENERATED_BODY()

	/** Placement (1st, 2nd, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	int32 Placement = 1;

	/** Cash reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	int32 CashReward = 0;

	/** XP reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	int32 XPReward = 0;

	/** Reputation points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	int32 ReputationReward = 0;

	/** Championship points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	int32 ChampionshipPoints = 0;

	/** Exclusive item rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	TArray<FName> ItemRewards;

	/** Title/badge reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	FName TitleReward;

	/** Vehicle unlock reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prize")
	FName VehicleReward;
};

/**
 * Tournament Entry Requirements
 *
 * Defines who is eligible to enter a tournament. Acts as a gatekeeper
 * to ensure fair competition and appropriate skill levels.
 *
 * Examples of restrictions:
 * - Level 20-50 only (keeps veterans from stomping newbies)
 * - Gold rank or higher (competitive events)
 * - Muscle cars only (themed tournaments)
 * - Entry fee of 5000 credits (prize pool contribution)
 * - Crew members only (crew events)
 * - North America region only (latency concerns)
 */
USTRUCT(BlueprintType)
struct FMGTournamentRequirements
{
	GENERATED_BODY()

	/** Minimum player level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MinLevel = 1;

	/** Maximum player level (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MaxLevel = 0;

	/** Minimum rank tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MinRankTier = 0;

	/** Required vehicle class (empty = any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FName RequiredVehicleClass;

	/** Allowed vehicle IDs (empty = all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> AllowedVehicles;

	/** Entry fee (in-game cash) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 EntryFee = 0;

	/** Requires crew membership */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bRequiresCrew = false;

	/** Invite only */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	bool bInviteOnly = false;

	/** Region lock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	FString RegionLock;
};

/**
 * Tournament Schedule
 *
 * Contains all timing information for a tournament. Used to:
 * - Notify players when to register, check in, and play
 * - Automatically transition tournament states
 * - Display countdown timers in UI
 *
 * Timeline Example:
 * Day 1: Registration opens
 * Day 3: Registration closes
 * Day 4 (6:00 PM): Check-in opens
 * Day 4 (6:30 PM): Check-in closes, tournament starts
 * Day 4 (10:00 PM): Estimated end (depends on match lengths)
 */
USTRUCT(BlueprintType)
struct FMGTournamentSchedule
{
	GENERATED_BODY()

	/** Registration opens */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FDateTime RegistrationStart;

	/** Registration closes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FDateTime RegistrationEnd;

	/** Check-in opens */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FDateTime CheckInStart;

	/** Check-in closes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FDateTime CheckInEnd;

	/** Tournament start time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FDateTime TournamentStart;

	/** Estimated end time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FDateTime EstimatedEnd;

	/** Minutes between rounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	int32 MinutesBetweenRounds = 10;

	/** Check-in duration (minutes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	int32 CheckInDuration = 30;
};

/**
 * Tournament Data
 *
 * The main data structure containing ALL information about a tournament.
 * This is the primary container passed around and stored for tournaments.
 *
 * Think of this as the "tournament document" - everything you need to know
 * is in here: settings, schedule, participants, matches, brackets, etc.
 *
 * Key sections:
 * - Identity: ID, name, description
 * - Classification: Tier, format, entry type, state
 * - Timing: Schedule struct
 * - Rules: Requirements, max/min participants, team size
 * - Rewards: Prize pool and prize distribution
 * - Progression: Tracks, rounds, matches, brackets
 * - Metadata: Featured status, streaming, organizer
 */
USTRUCT(BlueprintType)
struct FMGTournamentData
{
	GENERATED_BODY()

	/** Tournament ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FString TournamentID;

	/** Tournament name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FText TournamentName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FText Description;

	/** Tournament tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	EMGTournamentTier Tier = EMGTournamentTier::Community;

	/** Tournament format */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	EMGTournamentFormat Format = EMGTournamentFormat::SingleElimination;

	/** Entry type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	EMGTournamentEntryType EntryType = EMGTournamentEntryType::Solo;

	/** Current state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	EMGTournamentState State = EMGTournamentState::Announced;

	/** Schedule */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FMGTournamentSchedule Schedule;

	/** Entry requirements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FMGTournamentRequirements Requirements;

	/** Max participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	int32 MaxParticipants = 32;

	/** Min participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	int32 MinParticipants = 4;

	/** Team size (for team tournaments) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	int32 TeamSize = 1;

	/** Total prize pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	int32 TotalPrizePool = 0;

	/** Prizes by placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FMGTournamentPrize> Prizes;

	/** Track pool for matches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FName> TrackPool;

	/** Best of N for different rounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TMap<int32, int32> RoundBestOf;

	/** All participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FMGTournamentParticipant> Participants;

	/** All matches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FMGTournamentMatch> Matches;

	/** Winners bracket rounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FMGBracketRound> WinnersBracket;

	/** Losers bracket rounds (double elim) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FMGBracketRound> LosersBracket;

	/** Groups (group stage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	TArray<FMGTournamentGroup> Groups;

	/** Current round */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	int32 CurrentRound = 0;

	/** Is featured tournament */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	bool bIsFeatured = false;

	/** Has stream/spectators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	bool bHasStream = false;

	/** Created timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FDateTime CreatedAt;

	/** Organizer ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tournament")
	FString OrganizerID;
};

/**
 * Player Tournament Stats
 *
 * Aggregated statistics tracking a player's tournament history.
 * Used for profile displays, achievements, and bragging rights.
 *
 * Tracked metrics:
 * - Participation: Total tournaments entered
 * - Success: Wins, top 3 finishes, match record
 * - Streaks: Current and best winning streaks
 * - Earnings: Total prize money won
 * - Rankings: Championship points accumulated
 * - Achievements: Best placement in each tier
 */
USTRUCT(BlueprintType)
struct FMGPlayerTournamentStats
{
	GENERATED_BODY()

	/** Total tournaments entered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TournamentsEntered = 0;

	/** Tournaments won */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TournamentsWon = 0;

	/** Top 3 finishes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TopThreeFinishes = 0;

	/** Total match wins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalMatchWins = 0;

	/** Total match losses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalMatchLosses = 0;

	/** Current win streak */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CurrentWinStreak = 0;

	/** Best win streak */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 BestWinStreak = 0;

	/** Total earnings from tournaments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalEarnings = 0;

	/** Championship points earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 ChampionshipPoints = 0;

	/** Best placement by tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	TMap<EMGTournamentTier, int32> BestPlacementByTier;
};

//=============================================================================
// DELEGATES - Event Notifications
//=============================================================================
/**
 * Delegates allow other systems to subscribe to tournament events.
 * In Blueprints, these appear as bindable events you can hook into.
 *
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE:
 * - "Dynamic": Works with Blueprints
 * - "Multicast": Multiple listeners can subscribe
 * - "Delegate": Function pointer wrapper
 * - OneParam/TwoParams: Number of parameters passed to listeners
 */

/** Fired when a tournament changes state (registration opened, started, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTournamentStateChanged, const FMGTournamentData&, Tournament);

/** Fired when a registration attempt completes (success or failure) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTournamentRegistration, const FString&, TournamentID, bool, bSuccess);

/** Fired when a match is ready to start and waiting for players */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchReady, const FMGTournamentMatch&, Match);

/** Fired when a match finishes (with results) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchCompleted, const FMGTournamentMatch&, Match);

/** Fired when a tournament concludes with final standings and prizes distributed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTournamentCompleted, const FString&, TournamentID, const TArray<FMGTournamentParticipant>&, FinalStandings);

//=============================================================================
// SUBSYSTEM CLASS
//=============================================================================

/**
 * Tournament Subsystem
 *
 * The main subsystem that manages all tournament functionality.
 * Inherits from UGameInstanceSubsystem, meaning:
 * - One instance exists per game instance
 * - Persists across level loads (unlike per-level actors)
 * - Accessed via: GetGameInstance()->GetSubsystem<UMGTournamentSubsystem>()
 *
 * Responsibilities:
 * - Tournament creation, browsing, and lifecycle management
 * - Player registration and check-in
 * - Match scheduling and result tracking
 * - Bracket generation and advancement
 * - Prize distribution
 * - Statistics tracking
 *
 * @see UGameInstanceSubsystem for subsystem lifecycle details
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTournamentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Called when the subsystem is created (game starts).
	 * Sets up initial state, timers, and mock data for testing.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called when the subsystem is destroyed (game ends).
	 * Cleans up timers and any pending operations.
	 */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTournamentStateChanged OnTournamentStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTournamentRegistration OnTournamentRegistration;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchReady OnMatchReady;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMatchCompleted OnMatchCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTournamentCompleted OnTournamentCompleted;

	// ==========================================
	// TOURNAMENT BROWSING
	// ==========================================

	/** Get all available tournaments */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	TArray<FMGTournamentData> GetAvailableTournaments() const;

	/** Get tournaments by state */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	TArray<FMGTournamentData> GetTournamentsByState(EMGTournamentState State) const;

	/** Get featured tournaments */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	TArray<FMGTournamentData> GetFeaturedTournaments() const;

	/** Get tournament by ID */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool GetTournament(const FString& TournamentID, FMGTournamentData& OutTournament) const;

	/** Get tournaments player is registered for */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	TArray<FMGTournamentData> GetRegisteredTournaments() const;

	// ==========================================
	// REGISTRATION
	// ==========================================

	/** Check if can register for tournament */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool CanRegisterForTournament(const FString& TournamentID, FText& OutReason) const;

	/** Register for tournament */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool RegisterForTournament(const FString& TournamentID, FName VehicleID);

	/** Register team for tournament */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool RegisterTeamForTournament(const FString& TournamentID, const TArray<FString>& TeamMemberIDs, const FText& TeamName);

	/** Unregister from tournament */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool UnregisterFromTournament(const FString& TournamentID);

	/** Is player registered */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	bool IsRegisteredForTournament(const FString& TournamentID) const;

	/** Get registered participant count */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	int32 GetRegisteredCount(const FString& TournamentID) const;

	// ==========================================
	// CHECK-IN
	// ==========================================

	/** Check in for tournament */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool CheckInForTournament(const FString& TournamentID);

	/** Is player checked in */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	bool IsCheckedIn(const FString& TournamentID) const;

	/** Get time until check-in opens */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	FTimespan GetTimeUntilCheckIn(const FString& TournamentID) const;

	/** Get time until check-in closes */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	FTimespan GetTimeUntilCheckInCloses(const FString& TournamentID) const;

	// ==========================================
	// MATCH MANAGEMENT
	// ==========================================

	/** Get current match for player */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool GetCurrentMatch(FMGTournamentMatch& OutMatch) const;

	/** Get match by ID */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool GetMatch(const FString& TournamentID, const FString& MatchID, FMGTournamentMatch& OutMatch) const;

	/** Get matches for round */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	TArray<FMGTournamentMatch> GetMatchesForRound(const FString& TournamentID, int32 Round, EMGBracketSide Side = EMGBracketSide::Winners) const;

	/** Report match ready (both players present) */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	void ReportMatchReady(const FString& TournamentID, const FString& MatchID);

	/** Report match result */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	void ReportMatchResult(const FString& TournamentID, const FString& MatchID, const FString& WinnerID, int32 Score1, int32 Score2);

	/** Forfeit match */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	void ForfeitMatch(const FString& TournamentID, const FString& MatchID);

	// ==========================================
	// BRACKET QUERIES
	// ==========================================

	/** Get bracket data */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	TArray<FMGBracketRound> GetBracket(const FString& TournamentID, EMGBracketSide Side = EMGBracketSide::Winners) const;

	/** Get participant data */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool GetParticipant(const FString& TournamentID, const FString& ParticipantID, FMGTournamentParticipant& OutParticipant) const;

	/** Get standings (sorted by placement/points) */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	TArray<FMGTournamentParticipant> GetStandings(const FString& TournamentID) const;

	/** Get group standings */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	TArray<FMGTournamentParticipant> GetGroupStandings(const FString& TournamentID, const FString& GroupID) const;

	/** Is player eliminated */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	bool IsPlayerEliminated(const FString& TournamentID) const;

	/** Get player placement */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	int32 GetPlayerPlacement(const FString& TournamentID) const;

	// ==========================================
	// TOURNAMENT CREATION (for community tournaments)
	// ==========================================

	/** Create community tournament */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	FString CreateTournament(const FMGTournamentData& TournamentData);

	/** Cancel tournament (organizer only) */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool CancelTournament(const FString& TournamentID);

	/** Start tournament (organizer only) */
	UFUNCTION(BlueprintCallable, Category = "Tournament")
	bool StartTournament(const FString& TournamentID);

	// ==========================================
	// STATS
	// ==========================================

	/** Get player tournament stats */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	FMGPlayerTournamentStats GetPlayerTournamentStats() const { return PlayerStats; }

	/** Get tournament history */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	TArray<FMGTournamentData> GetTournamentHistory(int32 Count = 20) const;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get round name */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	static FText GetRoundName(int32 TotalRounds, int32 CurrentRound, EMGBracketSide Side);

	/** Get time until tournament starts */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	FTimespan GetTimeUntilStart(const FString& TournamentID) const;

	/** Calculate prize for placement */
	UFUNCTION(BlueprintPure, Category = "Tournament")
	FMGTournamentPrize GetPrizeForPlacement(const FString& TournamentID, int32 Placement) const;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** All tournaments */
	UPROPERTY()
	TArray<FMGTournamentData> Tournaments;

	/** Registered tournament IDs */
	UPROPERTY()
	TArray<FString> RegisteredTournamentIDs;

	/** Current active match tournament ID */
	UPROPERTY()
	FString ActiveMatchTournamentID;

	/** Current active match ID */
	UPROPERTY()
	FString ActiveMatchID;

	/** Player tournament stats */
	UPROPERTY()
	FMGPlayerTournamentStats PlayerStats;

	/** Tournament history */
	UPROPERTY()
	TArray<FMGTournamentData> TournamentHistory;

	UPROPERTY()
	UMGOnlineSubsystem* OnlineSubsystem;

	/** Local player ID */
	UPROPERTY()
	FString LocalPlayerID;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Generate bracket for tournament */
	void GenerateBracket(FMGTournamentData& Tournament);

	/** Generate single elimination bracket */
	void GenerateSingleEliminationBracket(FMGTournamentData& Tournament);

	/** Generate double elimination bracket */
	void GenerateDoubleEliminationBracket(FMGTournamentData& Tournament);

	/** Generate round robin schedule */
	void GenerateRoundRobinSchedule(FMGTournamentData& Tournament);

	/** Generate group stage */
	void GenerateGroupStage(FMGTournamentData& Tournament);

	/** Advance winner to next match */
	void AdvanceWinner(FMGTournamentData& Tournament, FMGTournamentMatch& CompletedMatch);

	/** Move loser to losers bracket (double elim) */
	void MoveToLosersBracket(FMGTournamentData& Tournament, FMGTournamentMatch& CompletedMatch);

	/** Check if round is complete */
	bool IsRoundComplete(const FMGTournamentData& Tournament, int32 Round, EMGBracketSide Side) const;

	/** Advance to next round */
	void AdvanceToNextRound(FMGTournamentData& Tournament);

	/** Complete tournament */
	void CompleteTournament(FMGTournamentData& Tournament);

	/** Calculate final placements */
	void CalculateFinalPlacements(FMGTournamentData& Tournament);

	/** Distribute prizes */
	void DistributePrizes(const FMGTournamentData& Tournament);

	/** Seed participants */
	void SeedParticipants(FMGTournamentData& Tournament);

	/** Get seeded matchups */
	TArray<TPair<int32, int32>> GetSeededMatchups(int32 ParticipantCount) const;

	/** Update player stats from tournament */
	void UpdatePlayerStats(const FMGTournamentData& Tournament);

	/** Load mock tournaments */
	void LoadMockTournaments();

	/** Update tournament state based on schedule */
	void UpdateTournamentStates();

	/** Get participant index */
	int32 GetParticipantIndex(const FMGTournamentData& Tournament, const FString& ParticipantID) const;

	/** Get match index */
	int32 GetMatchIndex(const FMGTournamentData& Tournament, const FString& MatchID) const;

	/** Timer for state updates */
	FTimerHandle StateUpdateTimerHandle;
};
