// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGLiveEventsSubsystem.h
 * Live Events Subsystem - Time-Limited Events, Challenges, and Community Goals
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem is the central hub for all live service event functionality in
 * Midnight Grind. It manages time-limited events, tracks player progress through
 * challenges, handles community-wide goals, and coordinates featured playlists
 * with bonus multipliers.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. LIVE EVENTS (FMGLiveEvent)
 *    Time-limited gameplay experiences that appear on a schedule:
 *    - Weekend specials (Friday-Sunday bonus events)
 *    - Weekly challenges (reset every Monday)
 *    - Holiday celebrations (Halloween, Christmas themes)
 *    - Flash events (2-6 hours, creates urgency)
 *    - Collaboration events (cross-promotion with other brands)
 *
 * 2. CHALLENGES (FMGEventChallenge)
 *    Specific objectives players must complete within an event:
 *    - "Win 5 races" -> Type: WinRaces, TargetValue: 5
 *    - "Drift 10,000 meters" -> Type: DriftDistance, TargetValue: 10000
 *    - Challenges contain one or more Objectives (FMGChallengeObjective)
 *    - Each tracks progress toward TargetValue
 *    - Rewards are granted when all objectives complete
 *
 * 3. COMMUNITY GOALS (FMGCommunityGoal)
 *    Server-wide objectives where ALL players contribute:
 *    - "Community drives 1 billion meters"
 *    - Progress is synced from server
 *    - Rewards unlock at tier thresholds (25%, 50%, 75%, 100%)
 *    - Everyone who participates gets the unlocked rewards
 *
 * 4. FEATURED PLAYLISTS (FMGFeaturedPlaylist)
 *    Curated collections of races with bonus multipliers:
 *    - Specific tracks/weather/time of day combinations
 *    - XP multiplier (e.g., 1.5x) and Cash multiplier
 *    - Rotate based on current events
 *    - Incentivize players to try specific content
 *
 * 5. EVENT LIFECYCLE (EMGEventStatus)
 *    Events automatically progress through states:
 *    Upcoming -> Active -> EndingSoon -> Completed/Expired
 *    - UI uses status to show countdown timers, warnings
 *    - EndingSoon triggers "1 hour left!" notifications
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *     +-------------------+
 *     | Server Backend    |
 *     | (Event Config)    |
 *     +-------------------+
 *              |
 *              | RefreshEvents()
 *              v
 *     +------------------------+
 *     | UMGLiveEventsSubsystem |  <-- This file
 *     | (GameInstanceSubsystem)|
 *     +------------------------+
 *              |
 *    +---------+---------+
 *    |         |         |
 *    v         v         v
 * [Events] [Challenges] [Community]
 *    |         |         |
 *    +---------+---------+
 *              |
 *              | ReportChallengeProgress()
 *              v
 *     +-------------------+
 *     | Race Results      |
 *     | (Gameplay System) |
 *     +-------------------+
 *              |
 *              | Delegates
 *              v
 *     +-------------------+
 *     | UI Widgets        |
 *     | (Progress/Rewards)|
 *     +-------------------+
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. At login: RefreshEvents() syncs event data from server
 * 2. Tick(): UpdateEventStatuses() checks times, transitions states
 * 3. Player views Events screen -> GetActiveEvents() returns current events
 * 4. Player completes race -> ReportChallengeProgress(WinRaces, 1, TrackID, VehicleID)
 * 5. Subsystem updates matching challenges -> OnChallengeProgress fires
 * 6. When objective completes -> OnChallengeCompleted fires
 * 7. Player clicks "Claim" -> ClaimChallengeReward() grants items
 * 8. At midnight UTC -> GenerateDailyChallenges() refreshes dailies
 *
 * DELEGATES (Events you can listen to):
 * ------------------------------------
 * - OnEventStarted: New event became active
 * - OnEventEnded: Event expired or completed
 * - OnChallengeProgress: Challenge progress updated (for progress bars)
 * - OnChallengeCompleted: Challenge finished, show claim button
 * - OnCommunityGoalProgress: Server synced new community progress
 * - OnCommunityGoalTierReached: Community unlocked a reward tier
 * - OnDailyChallengesRefreshed: New daily challenges available
 *
 * CHALLENGE TYPES (EMGChallengeType):
 * -----------------------------------
 * Racing:      WinRaces, CompleteRaces, AchievePosition, WinStreak
 * Performance: BeatLapTime, ReachTopSpeed, PerfectLaps
 * Accumulative: DriveDistance, DriftDistance, NearMisses, Overtakes
 * Specific:    UseVehicle, RaceOnTrack
 * Economy:     EarnCurrency
 * Community:   CommunityTotal (aggregates all players)
 *
 * IMPORTANT NOTES:
 * ----------------
 * - This is a GameInstanceSubsystem (persists across level loads)
 * - All public methods are game thread only
 * - Server sync is async but callbacks run on game thread
 * - Progress is saved locally and synced to server
 * - CreateMockEvents() generates test data for development
 *
 * @see UMGSeasonPassSubsystem - For season pass progression
 * @see UMGLiveEventsManager - Alternative implementation in LiveOps
 * @see UMGEventCalendarSubsystem - For event scheduling
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLiveEventsSubsystem.generated.h"

class UTexture2D;

// ============================================================================
// ENUMERATIONS - Event Classification Types
// ============================================================================

/**
 * @brief Categorizes live events by their duration and purpose.
 *
 * Event types determine default durations, UI styling, and notification behavior.
 * The game designer configures these in the backend, and the client uses them
 * to appropriately display and track each event.
 */
UENUM(BlueprintType)
enum class EMGEventType : uint8
{
	Weekend,        ///< Weekend special events (Friday-Sunday), typically with bonus rewards
	Weekly,         ///< Weekly challenges that reset every Monday, core engagement driver
	Daily,          ///< Daily challenges refreshing at midnight UTC, quick tasks
	LimitedTime,    ///< Special events lasting days to weeks, often with exclusive rewards
	CommunityGoal,  ///< Server-wide collaborative events where all players contribute
	Holiday,        ///< Seasonal celebrations (Halloween, Christmas, etc.) with themed content
	Collaboration,  ///< Cross-promotion events with other brands or games
	Flash           ///< Ultra-short events (2-6 hours), creates urgency and excitement
};

/**
 * @brief Tracks the current lifecycle state of an event.
 *
 * Events progress through states automatically based on time. The UI uses these
 * states to show appropriate messaging (e.g., countdown timers, "ending soon" warnings).
 */
UENUM(BlueprintType)
enum class EMGEventStatus : uint8
{
	Upcoming,    ///< Event scheduled but not started; show "Coming Soon" in UI
	Active,      ///< Event is live and players can participate normally
	EndingSoon,  ///< Less than 1 hour remaining; show urgent countdown timer
	Completed,   ///< Player finished the event; rewards claimed or pending
	Expired      ///< Event ended without player completion; no rewards available
};

/**
 * @brief Defines what type of gameplay action a challenge tracks.
 *
 * Challenge types map to specific game statistics and determine how progress
 * is calculated. When reporting progress via ReportChallengeProgress(), the
 * subsystem matches the type to update relevant objectives.
 *
 * Design Note: New challenge types require corresponding tracking code in
 * the race results system and the UpdateObjectiveProgress() method.
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
	WinRaces,        ///< Count of first-place finishes
	CompleteRaces,   ///< Count of races finished (any position)
	AchievePosition, ///< Finish at or above a target position (e.g., top 3)
	BeatLapTime,     ///< Complete a lap faster than target time (in seconds)
	DriveDistance,   ///< Cumulative distance driven (in meters)
	ReachTopSpeed,   ///< Hit target speed at any point during race (km/h)
	DriftDistance,   ///< Cumulative meters drifted while sideways
	NearMisses,      ///< Count of close passes to obstacles/other cars
	Overtakes,       ///< Count of successful passes of opponent vehicles
	UseVehicle,      ///< Complete races using a specific vehicle ID
	RaceOnTrack,     ///< Complete races on a specific track ID
	WinStreak,       ///< Consecutive wins without losing (resets on loss)
	PerfectLaps,     ///< Laps completed without hitting walls/objects
	EarnCurrency,    ///< Cumulative in-game currency earned from races
	CommunityTotal   ///< Aggregates contributions from all players server-wide
};

// ============================================================================
// DATA STRUCTURES - Rewards and Objectives
// ============================================================================

/**
 * @brief Describes a single reward item that can be earned from events.
 *
 * Rewards can be currency, cosmetic items, XP boosts, or exclusive event items.
 * The RewardType field determines how the reward is processed when claimed.
 * Exclusive rewards (bIsExclusive=true) are only available during the event
 * and may never return, creating collector value.
 */
USTRUCT(BlueprintType)
struct FMGEventReward
{
	GENERATED_BODY()

	/** Reward ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	/** Reward type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardType;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	/** Is exclusive to event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsExclusive = false;

	/** Rarity color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RarityColor = FLinearColor::White;
};

/**
 * @brief A single trackable objective within a challenge.
 *
 * Objectives are the atomic units of challenge progress. A challenge may have
 * multiple objectives that must all be completed. Each objective tracks progress
 * toward a TargetValue and may have additional requirements like specific tracks
 * or vehicles.
 *
 * Example: "Win 3 races on Night City Circuit" would have:
 * - Type = WinRaces, TargetValue = 3, RequiredTrack = "NightCityCircuit"
 */
USTRUCT(BlueprintType)
struct FMGChallengeObjective
{
	GENERATED_BODY()

	/** Objective ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType Type = EMGChallengeType::CompleteRaces;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Target value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 1;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentValue = 0;

	/** Required track (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	/** Required vehicle (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;

	/** Required weather (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredWeather;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;
};

/**
 * @brief A complete challenge with objectives, rewards, and tracking state.
 *
 * Challenges are the main engagement loop for events. Players complete objectives
 * to finish challenges, then claim rewards. Challenges have difficulty ratings
 * to help players choose appropriate tasks and set expectations.
 *
 * Workflow: Objectives completed -> bIsCompleted = true -> Player claims -> bIsClaimed = true
 */
USTRUCT(BlueprintType)
struct FMGEventChallenge
{
	GENERATED_BODY()

	/** Challenge ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeObjective> Objectives;

	/** Rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> Rewards;

	/** XP reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPReward = 0;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	/** Is claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsClaimed = false;

	/** Difficulty (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Difficulty = 1;

	/** Sort order */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

/**
 * @brief Server-wide collaborative goal that all players contribute to.
 *
 * Community goals create shared experiences where the entire player base works
 * toward massive targets (e.g., "Community drives 1 billion meters"). Progress
 * is synced from the server and rewards unlock at tier thresholds.
 *
 * The TierThresholds array defines milestones (e.g., [250000, 500000, 750000, 1000000])
 * and TierRewards contains corresponding rewards for each milestone reached.
 */
USTRUCT(BlueprintType)
struct FMGCommunityGoal
{
	GENERATED_BODY()

	/** Goal ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GoalID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType Type = EMGChallengeType::CommunityTotal;

	/** Community target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CommunityTarget = 1000000;

	/** Current community progress */
	UPROPERTY(BlueprintReadOnly)
	int64 CommunityProgress = 0;

	/** Player's contribution */
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerContribution = 0;

	/** Tier thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int64> TierThresholds;

	/** Tier rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> TierRewards;

	/** Current tier reached */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentTier = 0;

	/** Is goal completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;
};

// ============================================================================
// DATA STRUCTURES - Playlists and Race Configuration
// ============================================================================

/**
 * @brief A single race configuration within a featured playlist.
 *
 * Playlist entries define the specific race setup: track, weather, time of day,
 * lap count, and any vehicle restrictions. This allows event designers to create
 * curated racing experiences with specific conditions.
 */
USTRUCT(BlueprintType)
struct FMGPlaylistEntry
{
	GENERATED_BODY()

	/** Track ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Weather override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeatherOverride;

	/** Time of day override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TimeOfDayOverride;

	/** Lap count override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 0;

	/** Vehicle restrictions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AllowedVehicles;

	/** Is reverse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReverse = false;
};

/**
 * @brief A curated collection of races with bonus rewards for event participation.
 *
 * Featured playlists incentivize players to try specific content by offering XP
 * and cash multipliers. They are displayed prominently in the event UI and rotate
 * based on the current active events.
 */
USTRUCT(BlueprintType)
struct FMGFeaturedPlaylist
{
	GENERATED_BODY()

	/** Playlist ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Playlist entries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPlaylistEntry> Entries;

	/** XP multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XPMultiplier = 1.0f;

	/** Cash multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashMultiplier = 1.0f;

	/** Is featured */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;
};

// ============================================================================
// DATA STRUCTURES - Event Container
// ============================================================================

/**
 * @brief Complete data structure for a live event with all associated content.
 *
 * FMGLiveEvent is the top-level container for event data. It holds metadata
 * (name, times, type), visual assets (banner, theme color), and all gameplay
 * content (challenges, community goals, playlists, rewards).
 *
 * Events are loaded from the server and stored in the AllEvents array. The
 * subsystem automatically updates Status based on current time.
 */
USTRUCT(BlueprintType)
struct FMGLiveEvent
{
	GENERATED_BODY()

	/** Event ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	/** Event type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEventType Type = EMGEventType::Weekly;

	/** Status */
	UPROPERTY(BlueprintReadOnly)
	EMGEventStatus Status = EMGEventStatus::Upcoming;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Banner image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* BannerImage = nullptr;

	/** Theme color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ThemeColor = FLinearColor::White;

	/** Start time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	/** End time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	/** Challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventChallenge> Challenges;

	/** Community goals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCommunityGoal> CommunityGoals;

	/** Featured playlists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGFeaturedPlaylist> FeaturedPlaylists;

	/** Completion rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> CompletionRewards;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Total event XP earned */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalXPEarned = 0;

	/** Is participated */
	UPROPERTY(BlueprintReadOnly)
	bool bHasParticipated = false;
};

/**
 * @brief Container for a day's worth of challenges with bonus reward for completion.
 *
 * Daily challenges reset at midnight UTC. Completing all challenges in a day
 * awards a bonus reward and contributes to the player's daily streak.
 */
USTRUCT(BlueprintType)
struct FMGDailyChallenges
{
	GENERATED_BODY()

	/** Date for these challenges */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Date;

	/** Challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventChallenge> Challenges;

	/** Bonus reward for completing all */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEventReward BonusReward;

	/** All completed */
	UPROPERTY(BlueprintReadOnly)
	bool bAllCompleted = false;

	/** Bonus claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bBonusClaimed = false;
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// Broadcast when a new event becomes active (Start time reached)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventStarted, const FMGLiveEvent&, Event);

/// Broadcast when an event ends (End time reached or all content completed)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventEnded, const FMGLiveEvent&, Event);

/// Broadcast when challenge progress updates; use for UI progress bars
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChallengeProgress, FName, ChallengeID, const FMGEventChallenge&, Challenge);

/// Broadcast when all objectives in a challenge are completed; show claim prompt
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FMGEventChallenge&, Challenge);

/// Broadcast when community goal progress updates from server
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityGoalProgress, FName, GoalID, const FMGCommunityGoal&, Goal);

/// Broadcast when community goal reaches a new tier; unlock tier rewards
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityGoalTierReached, FName, GoalID, int32, Tier);

/// Broadcast at midnight UTC when daily challenges refresh
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDailyChallengesRefreshed);

// ============================================================================
// LIVE EVENTS SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGLiveEventsSubsystem
 * @brief Manages time-limited events, challenges, and community goals.
 *
 * This GameInstanceSubsystem is the primary interface for all live event functionality.
 * It persists for the lifetime of the game instance and automatically handles event
 * lifecycle, progress tracking, and reward distribution.
 *
 * ## Integration Points:
 * - Race completion system should call ReportChallengeProgress() with race stats
 * - UI should bind to delegates for real-time updates
 * - Login flow should call RefreshEvents() to sync latest event data
 *
 * ## Thread Safety:
 * All public methods are designed to be called from the game thread only.
 * Server sync operations are asynchronous but callbacks execute on game thread.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLiveEventsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Called when the subsystem is created; initializes event data and starts tick timer
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when the game instance is destroyed; saves progress and cleans up
	virtual void Deinitialize() override;

	/// Called every frame; updates event statuses and checks for expirations
	virtual void Tick(float DeltaTime);

	// ==========================================
	// BLUEPRINT DELEGATE PROPERTIES
	// Bind to these in UI widgets for real-time event updates
	// ==========================================

	/// Fired when a new event becomes active; update event list UI
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventStarted OnEventStarted;

	/// Fired when an event ends; show summary and final rewards
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventEnded OnEventEnded;

	/// Fired on any challenge progress; update progress bars and counters
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeProgress OnChallengeProgress;

	/// Fired when challenge is complete; show completion fanfare and claim button
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeCompleted OnChallengeCompleted;

	/// Fired when community goal progress syncs from server
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCommunityGoalProgress OnCommunityGoalProgress;

	/// Fired when community unlocks a new reward tier; celebration moment
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCommunityGoalTierReached OnCommunityGoalTierReached;

	/// Fired at midnight UTC when new daily challenges are available
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDailyChallengesRefreshed OnDailyChallengesRefreshed;

	// ==========================================
	// LIVE EVENTS - Query and Management
	// Core functions for accessing event data
	// ==========================================

	/**
	 * @brief Returns all events currently in Active or EndingSoon status.
	 * @return Array of active events sorted by end time (soonest first)
	 */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	TArray<FMGLiveEvent> GetActiveEvents() const;

	/**
	 * @brief Returns events in Upcoming status (not yet started).
	 * @return Array of upcoming events sorted by start time (soonest first)
	 */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	TArray<FMGLiveEvent> GetUpcomingEvents() const;

	/**
	 * @brief Retrieves a specific event by its unique identifier.
	 * @param EventID Unique identifier for the event
	 * @return The event data, or empty struct if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	FMGLiveEvent GetEventByID(FName EventID) const;

	/**
	 * @brief Checks if an event is currently active and playable.
	 * @param EventID Unique identifier for the event
	 * @return True if event status is Active or EndingSoon
	 */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	bool IsEventActive(FName EventID) const;

	/**
	 * @brief Gets the time remaining before an event ends.
	 * @param EventID Unique identifier for the event
	 * @return Time remaining as FTimespan; zero if event not found or ended
	 */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	FTimespan GetEventTimeRemaining(FName EventID) const;

	/**
	 * @brief Syncs event data from the server.
	 * Call this at login and periodically to ensure fresh data.
	 * Results are asynchronous; listen to delegates for updates.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live Events")
	void RefreshEvents();

	// ==========================================
	// CHALLENGES - Progress Tracking and Rewards
	// The main gameplay loop for event participation
	// ==========================================

	/**
	 * @brief Gets current progress for a specific challenge.
	 * @param EventID Parent event identifier
	 * @param ChallengeID Challenge identifier within the event
	 * @return Challenge data with current progress values
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FMGEventChallenge GetChallengeProgress(FName EventID, FName ChallengeID) const;

	/**
	 * @brief Reports gameplay progress toward challenges.
	 *
	 * Call this after races with the player's stats. The subsystem will find
	 * all matching challenges and update their progress appropriately.
	 *
	 * @param Type The type of achievement being reported
	 * @param Value The value to add (e.g., distance driven, races won)
	 * @param TrackID Optional track ID for track-specific challenges
	 * @param VehicleID Optional vehicle ID for vehicle-specific challenges
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void ReportChallengeProgress(EMGChallengeType Type, int32 Value, FName TrackID = NAME_None, FName VehicleID = NAME_None);

	/**
	 * @brief Claims rewards for a completed challenge.
	 * @param EventID Parent event identifier
	 * @param ChallengeID Challenge identifier to claim
	 * @return True if rewards were successfully claimed
	 */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	bool ClaimChallengeReward(FName EventID, FName ChallengeID);

	/**
	 * @brief Gets all challenges that are complete but not yet claimed.
	 * Use this to show a notification badge for pending rewards.
	 * @return Array of completed but unclaimed challenges across all events
	 */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGEventChallenge> GetUnclaimedChallenges() const;

	// ==========================================
	// DAILY CHALLENGES - Recurring Daily Content
	// Refreshes at midnight UTC each day
	// ==========================================

	/**
	 * @brief Gets the current day's challenge set.
	 * @return Today's challenges with progress and bonus reward info
	 */
	UFUNCTION(BlueprintPure, Category = "Daily")
	FMGDailyChallenges GetDailyChallenges() const;

	/**
	 * @brief Gets the player's consecutive days completing all daily challenges.
	 * Streak increases player engagement and may unlock bonus multipliers.
	 * @return Number of consecutive days with all dailies completed
	 */
	UFUNCTION(BlueprintPure, Category = "Daily")
	int32 GetDailyStreak() const { return DailyStreak; }

	/**
	 * @brief Claims the bonus reward for completing all daily challenges.
	 * Only available when all challenges for the day are complete.
	 * @return True if bonus was claimed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Daily")
	bool ClaimDailyBonusReward();

	// ==========================================
	// COMMUNITY GOALS
	// ==========================================

	/** Get community goal progress */
	UFUNCTION(BlueprintPure, Category = "Community")
	FMGCommunityGoal GetCommunityGoalProgress(FName EventID, FName GoalID) const;

	/** Contribute to community goal */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void ContributeToCommunityGoal(FName EventID, FName GoalID, int32 Contribution);

	/** Get community goal percentage */
	UFUNCTION(BlueprintPure, Category = "Community")
	float GetCommunityGoalPercentage(FName EventID, FName GoalID) const;

	// ==========================================
	// PLAYLISTS
	// ==========================================

	/** Get featured playlists */
	UFUNCTION(BlueprintPure, Category = "Playlists")
	TArray<FMGFeaturedPlaylist> GetFeaturedPlaylists() const;

	/** Get playlist by ID */
	UFUNCTION(BlueprintPure, Category = "Playlists")
	FMGFeaturedPlaylist GetPlaylistByID(FName PlaylistID) const;

	/** Get current playlist multipliers */
	UFUNCTION(BlueprintPure, Category = "Playlists")
	void GetPlaylistMultipliers(FName PlaylistID, float& OutXPMultiplier, float& OutCashMultiplier) const;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get event type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetEventTypeDisplayName(EMGEventType Type);

	/** Get event status display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetEventStatusDisplayName(EMGEventStatus Status);

	/** Get challenge type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetChallengeTypeDisplayName(EMGChallengeType Type);

	/** Format time remaining */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatTimeRemaining(FTimespan TimeRemaining);

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** All events */
	UPROPERTY()
	TArray<FMGLiveEvent> AllEvents;

	/** Daily challenges */
	UPROPERTY()
	FMGDailyChallenges DailyChallenges;

	/** Daily streak */
	int32 DailyStreak = 0;

	/** Last daily completion date */
	FDateTime LastDailyCompletion;

	/** Featured playlists */
	UPROPERTY()
	TArray<FMGFeaturedPlaylist> FeaturedPlaylists;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update event statuses */
	void UpdateEventStatuses();

	/** Check for expired events */
	void CheckExpiredEvents();

	/** Generate daily challenges */
	void GenerateDailyChallenges();

	/** Check challenge completion */
	void CheckChallengeCompletion(FMGEventChallenge& Challenge);

	/** Check community goal tiers */
	void CheckCommunityGoalTiers(FMGCommunityGoal& Goal, FName EventID);

	/** Update objective progress */
	void UpdateObjectiveProgress(FMGChallengeObjective& Objective, EMGChallengeType Type, int32 Value, FName TrackID, FName VehicleID);

	/** Create mock events (for testing) */
	void CreateMockEvents();

	/** Save progress */
	void SaveProgress();

	/** Load progress */
	void LoadProgress();
};
