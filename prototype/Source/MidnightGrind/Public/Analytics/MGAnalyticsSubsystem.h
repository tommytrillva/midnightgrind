// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGAnalyticsSubsystem.h
 * =============================================================================
 *
 * PURPOSE:
 * --------
 * This file defines the Analytics Subsystem for Midnight Grind - a comprehensive
 * system that tracks player behavior, game performance, and balance data. Analytics
 * help developers understand how players interact with the game, identify issues,
 * and make data-driven decisions for game improvements.
 *
 * KEY CONCEPTS:
 * -------------
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    - This class inherits from UGameInstanceSubsystem, meaning it exists for the
 *      entire lifetime of the game session (from launch to quit).
 *    - Unlike World Subsystems, it persists across level loads, making it ideal
 *      for tracking session-wide statistics.
 *    - Unreal automatically creates and destroys this subsystem - you don't need
 *      to spawn it manually.
 *
 * 2. ANALYTICS EVENTS:
 *    - Events are discrete actions or occurrences that we want to track (e.g.,
 *      "player started a race", "player purchased a vehicle").
 *    - Each event has properties (string key-value pairs) and metrics (numeric values).
 *    - Events are batched and uploaded periodically to reduce network overhead.
 *
 * 3. HEAT MAPS:
 *    - Visual representations of where events occur on a track.
 *    - Each point has a location, intensity, and event type.
 *    - Used to identify problem areas (frequent crashes) or popular spots (overtakes).
 *
 * 4. FUNNELS:
 *    - Track player progression through a sequence of steps (e.g., tutorial stages).
 *    - Help identify where players drop off or get stuck.
 *    - Conversion rate = (users who completed step) / (users who reached step).
 *
 * 5. BALANCE DATA:
 *    - Tracks vehicle performance statistics to ensure fair gameplay.
 *    - Win rates, usage rates, and average positions help identify overpowered
 *      or underpowered vehicles that need tuning.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 * - The Analytics Subsystem is accessed via the Game Instance:
 *     UMGAnalyticsSubsystem* Analytics = GameInstance->GetSubsystem<UMGAnalyticsSubsystem>();
 *
 * - Other systems (Gameplay, Economy, Progression) call into this subsystem to
 *   record events when important actions occur.
 *
 * - Data flows: Game Events -> Analytics Subsystem -> Event Queue -> Backend Server
 *
 * - Works alongside:
 *     - TelemetrySubsystem: Captures real-time vehicle data during races
 *     - CrashReportingSubsystem: Handles errors and crashes specifically
 *
 * USAGE EXAMPLES:
 * ---------------
 * // Track a simple event
 * AnalyticsSubsystem->TrackEvent("MainMenuOpened", EMGAnalyticsCategory::Engagement);
 *
 * // Track a race completion with full data
 * FMGRaceAnalytics RaceData;
 * RaceData.TrackID = "DowntownCircuit";
 * RaceData.FinalPosition = 1;
 * RaceData.RaceTime = 125.5f;
 * AnalyticsSubsystem->TrackRaceEnd(RaceData);
 *
 * // Add a heat map point when player crashes
 * AnalyticsSubsystem->AddHeatMapPoint("DowntownCircuit", "Crash", CrashLocation, 1.0f);
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAnalyticsSubsystem.generated.h"

/**
 * Analytics Event Category
 *
 * Categories help organize events for filtering and analysis. When viewing analytics
 * dashboards, you can filter by category to focus on specific aspects of the game.
 *
 * Choose the most appropriate category when logging events - this makes data analysis
 * much easier later.
 */
UENUM(BlueprintType)
enum class EMGAnalyticsCategory : uint8
{
	Gameplay		UMETA(DisplayName = "Gameplay"),		// Racing events: starts, finishes, crashes, overtakes
	Economy			UMETA(DisplayName = "Economy"),		// Currency: earning, spending, purchases
	Social			UMETA(DisplayName = "Social"),			// Multiplayer: crew actions, friend interactions
	Progression		UMETA(DisplayName = "Progression"),	// Level ups, achievements, unlocks
	Technical		UMETA(DisplayName = "Technical"),		// Performance metrics, load times, errors
	Engagement		UMETA(DisplayName = "Engagement"),		// Session data, retention, UI interactions
	Monetization	UMETA(DisplayName = "Monetization"),	// Real-money transactions, ad views
	Error			UMETA(DisplayName = "Error")			// Errors that don't cause crashes but should be tracked
};

/**
 * Heat Map Data Point
 *
 * A single data point in a heat map visualization. Heat maps are used to visualize
 * where specific events occur on a race track. By aggregating many data points,
 * designers can see patterns like:
 * - Where players frequently crash (collision hot spots)
 * - Popular overtaking zones
 * - Areas where players slow down unexpectedly
 *
 * The "Intensity" value can be used to weight the visualization - higher intensity
 * points appear more prominently (useful for severe crashes vs minor bumps).
 */
USTRUCT(BlueprintType)
struct FMGHeatMapPoint
{
	GENERATED_BODY()

	/** World-space location where the event occurred (X, Y, Z coordinates on the track) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	FVector Location = FVector::ZeroVector;

	/** How "significant" this point is (0.0 to 1.0+). Higher = more prominent in visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	float Intensity = 1.0f;

	/** What type of event this point represents (e.g., "Crash", "Overtake", "Drift") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	FName EventType;

	/** When this event occurred - useful for filtering heat maps by time period */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	FDateTime Timestamp;
};

/**
 * Heat Map Data for a Track
 *
 * Contains all heat map data for a specific race track. This structure organizes
 * event points by type, making it easy to visualize different aspects of player
 * behavior on the track.
 *
 * Example use case: A level designer notices many CrashPoints clustered at a
 * particular corner. This indicates the corner may be too difficult or have
 * misleading visual cues, prompting a redesign.
 */
USTRUCT(BlueprintType)
struct FMGTrackHeatMap
{
	GENERATED_BODY()

	/** Unique identifier for the track (must match the track's asset name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	FName TrackID;

	/** Locations where vehicle collisions occurred - helps identify dangerous sections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	TArray<FMGHeatMapPoint> CrashPoints;

	/** Locations where players successfully overtook opponents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	TArray<FMGHeatMapPoint> OvertakePoints;

	/** Locations where players executed drifts - shows popular drift zones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	TArray<FMGHeatMapPoint> DriftPoints;

	/** Locations where players activated nitro boost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	TArray<FMGHeatMapPoint> NitroPoints;

	/** Locations where players unexpectedly slowed down - may indicate confusing sections */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HeatMap")
	TArray<FMGHeatMapPoint> SlowdownPoints;
};

/**
 * Session Analytics Data
 *
 * Captures a summary of everything that happened during a single play session
 * (from game launch to quit). This is crucial for understanding player engagement
 * and retention metrics.
 *
 * Key metrics derived from session data:
 * - Average session length: Are players engaged or leaving quickly?
 * - Races per session: How much are players actually playing?
 * - Win rate: Is the difficulty appropriate?
 * - Currency flow: Is the economy balanced?
 *
 * A "session" starts when Initialize() is called (game launch) and ends when
 * Deinitialize() is called (game quit).
 */
USTRUCT(BlueprintType)
struct FMGSessionAnalytics
{
	GENERATED_BODY()

	/** Unique identifier for this session (GUID format). Links all events from one play session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SessionID;

	/** Timestamp when the session started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FDateTime SessionStart;

	/** Timestamp when the session ended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FDateTime SessionEnd;

	/** Total duration of the session in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	float SessionDuration = 0.0f; // Seconds

	/** Number of races the player finished (not abandoned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 RacesCompleted = 0;

	/** Number of races the player won (1st place) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 RacesWon = 0;

	/** Total in-game currency earned this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CashEarned = 0;

	/** Total in-game currency spent this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CashSpent = 0;

	/** Total experience points earned this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 XPEarned = 0;

	/** Number of level-ups achieved this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 LevelsGained = 0;

	/** Number of vehicles purchased this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 VehiclesPurchased = 0;

	/** Number of achievements unlocked this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 AchievementsUnlocked = 0;

	/** Platform the player is on (e.g., "Windows", "PlayStation", "Xbox") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString Platform;

	/** Device specifications string (GPU, RAM, etc.) for performance analysis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString DeviceInfo;

	/** Average frames per second during gameplay - helps identify performance issues */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	float AverageFPS = 0.0f;

	/** Number of game crashes/errors during this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CrashCount = 0;
};

/**
 * Race Analytics Data
 *
 * Comprehensive data about a single race. This is one of the most important
 * analytics structures as racing is the core gameplay loop. This data helps answer:
 * - Is the AI difficulty appropriate?
 * - Are certain tracks too hard or too easy?
 * - Which vehicles perform best on which tracks?
 * - Why are players rage-quitting?
 *
 * Passed to TrackRaceEnd() when a race completes. The race result screen or
 * race manager should populate this structure.
 */
USTRUCT(BlueprintType)
struct FMGRaceAnalytics
{
	GENERATED_BODY()

	/** Unique identifier for this specific race instance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString RaceID;

	/** Which track/circuit was raced */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName TrackID;

	/** Which vehicle the player used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName VehicleID;

	/** Player's finishing position (1 = first place) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 FinalPosition = 0;

	/** Total time to complete the race in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float RaceTime = 0.0f;

	/** Fastest single lap time achieved during the race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float BestLapTime = 0.0f;

	/** Total number of laps in the race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 TotalLaps = 0;

	/** Number of collisions with walls, objects, or other vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 Collisions = 0;

	/** Total distance traveled while drifting (in Unreal units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float DriftDistance = 0.0f;

	/** Number of times nitro boost was activated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 NitroUses = 0;

	/** Number of times player passed another racer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 Overtakes = 0;

	/** True if this was a multiplayer race, false for single-player/AI races */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsOnline = false;

	/** Total number of racers in this race (including player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 RacerCount = 0;

	/** True if player quit before finishing - important for difficulty tuning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bRageQuit = false;

	/** When this race occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FDateTime RaceTimestamp;
};

/**
 * Balance Analytics Data
 *
 * Tracks aggregated statistics for a specific vehicle to help with game balance.
 * Good game balance means all vehicles feel viable and fun to use. If one vehicle
 * has a significantly higher win rate, it may need to be nerfed. If a vehicle is
 * never used, it may be too weak or uninteresting.
 *
 * Ideal targets for a balanced game:
 * - Win rate: ~50% for most vehicles (adjusted for skill class)
 * - Usage distribution: Varied across vehicles (not everyone using one car)
 * - Average position: Should cluster around middle positions
 */
USTRUCT(BlueprintType)
struct FMGBalanceAnalytics
{
	GENERATED_BODY()

	/** Which vehicle this data is about */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	FName VehicleID;

	/** How many times players have selected this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	int32 TimesUsed = 0;

	/** Total wins achieved with this vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	int32 Wins = 0;

	/** Wins / TimesUsed - a high win rate (>60%) may indicate overpowered vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float WinRate = 0.0f;

	/** Average finishing position (1.0 = always wins, 8.0 = always last in 8-racer field) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float AveragePosition = 0.0f;

	/** Average lap time in seconds - helps compare raw speed across vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float AverageLapTime = 0.0f;
};

/**
 * Funnel Step
 *
 * A single step in a funnel analysis. Funnels track how players progress through
 * a sequence of steps, showing where they drop off.
 *
 * Example: Tutorial Funnel
 *   Step 1: "TutorialStarted" - 1000 users reached, 950 completed (95% conversion)
 *   Step 2: "LearnedAccelerate" - 950 users reached, 920 completed (97% conversion)
 *   Step 3: "LearnedDrift" - 920 users reached, 600 completed (65% conversion) <- Problem!
 *
 * In this example, the drift tutorial has a significant drop-off, indicating it
 * may be too difficult or unclear and needs improvement.
 */
USTRUCT(BlueprintType)
struct FMGFunnelStep
{
	GENERATED_BODY()

	/** Human-readable name for this step (e.g., "CompletedFirstRace") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	FString StepName;

	/** Number of unique users who started/reached this step */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	int32 UsersReached = 0;

	/** Number of users who successfully completed this step */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	int32 UsersCompleted = 0;

	/** UsersCompleted / UsersReached - shows what percentage make it through */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	float ConversionRate = 0.0f;

	/** Average time users spend on this step - long times may indicate confusion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	float AverageTimeSeconds = 0.0f;
};

/**
 * Analytics Event
 *
 * The core data structure for tracking player actions. Every meaningful action
 * in the game can be represented as an analytics event. Events are queued locally
 * and periodically uploaded to the analytics backend in batches.
 *
 * Best practices for event naming:
 * - Use PascalCase: "RaceStarted", "VehiclePurchased", "AchievementUnlocked"
 * - Be specific: "VehiclePurchased" not "Purchase"
 * - Use past tense for completed actions: "RaceCompleted" not "RaceComplete"
 *
 * Properties vs Metrics:
 * - Properties: String values for filtering/grouping (VehicleName, TrackID)
 * - Metrics: Numeric values for aggregation/math (LapTime, CashSpent)
 */
USTRUCT(BlueprintType)
struct FMGAnalyticsEvent
{
	GENERATED_BODY()

	/** Name of the event - should describe what happened (e.g., "RaceCompleted") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventName;

	/** Category for organizing and filtering events */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EMGAnalyticsCategory Category = EMGAnalyticsCategory::Gameplay;

	/** String key-value pairs for categorical data (e.g., "VehicleName" -> "Speedster") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FString, FString> Properties;

	/** Numeric key-value pairs for measurable data (e.g., "LapTime" -> 45.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FString, float> Metrics;

	/** When this event occurred (automatically set when event is created) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FDateTime Timestamp;

	/** Links this event to the current play session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString SessionID;

	/** Unique identifier for the player (for cross-session analysis) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString PlayerID;
};

/**
 * Performance Metrics
 *
 * Captures technical performance data at a point in time. This data is crucial
 * for identifying performance issues across different player hardware configurations.
 *
 * Key performance indicators:
 * - FPS: Target 60+ for smooth gameplay. Below 30 is problematic.
 * - Frame time: 16.67ms = 60 FPS, 33.33ms = 30 FPS
 * - Hitches: Sudden frame drops (stutters) that disrupt gameplay feel
 *
 * This data, combined with DeviceInfo from session analytics, helps identify:
 * - Which hardware configurations struggle
 * - Which tracks/scenarios cause performance issues
 * - Whether optimization efforts are working
 */
USTRUCT(BlueprintType)
struct FMGPerformanceMetrics
{
	GENERATED_BODY()

	/** Average frames per second during the sample period */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageFPS = 0.0f;

	/** Lowest FPS observed - important for detecting worst-case scenarios */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MinFPS = 0.0f;

	/** Highest FPS observed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxFPS = 0.0f;

	/** Average time per frame in milliseconds (1000 / FPS) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageFrameTime = 0.0f;

	/** Time spent on GPU operations per frame (rendering) in milliseconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float GPU_Time = 0.0f;

	/** Time spent on CPU operations per frame (game logic) in milliseconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float CPU_Time = 0.0f;

	/** Total memory usage in megabytes - watch for memory leaks over time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int64 MemoryUsedMB = 0;

	/** Number of draw calls per frame - too many = batching problems */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 DrawCalls = 0;

	/** Total triangles rendered per frame - indicates scene complexity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 TrianglesRendered = 0;

	/** Time to load the current level/scene in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float LoadTime = 0.0f;

	/** Number of frame hitches (sudden drops) - causes stuttering gameplay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 HitchCount = 0;
};

// ============================================================================
// DELEGATES
// ============================================================================

/**
 * Delegate broadcast whenever an analytics event is sent.
 * Useful for debugging or creating real-time analytics dashboards during development.
 *
 * Usage in Blueprint or C++:
 *   AnalyticsSubsystem->OnAnalyticsEventSent.AddDynamic(this, &UMyClass::HandleEventSent);
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalyticsEventSent, const FMGAnalyticsEvent&, Event);

// ============================================================================
// MAIN ANALYTICS SUBSYSTEM CLASS
// ============================================================================

/**
 * UMGAnalyticsSubsystem
 *
 * The central hub for all game analytics. This subsystem automatically starts
 * when the game launches and stops when the game quits. It provides functions
 * to track every type of meaningful player action.
 *
 * SUBSYSTEM LIFECYCLE:
 * - Initialize(): Called automatically when GameInstance is created
 *   - Generates a new SessionID
 *   - Starts performance monitoring timers
 *   - Records session start timestamp
 *
 * - Deinitialize(): Called automatically when game shuts down
 *   - Flushes any pending events to the server
 *   - Records session end timestamp
 *   - Uploads final session summary
 *
 * ACCESSING THE SUBSYSTEM:
 *   // From any Actor or Object with a World context
 *   UGameInstance* GI = GetGameInstance();
 *   UMGAnalyticsSubsystem* Analytics = GI->GetSubsystem<UMGAnalyticsSubsystem>();
 *
 *   // From Blueprint: Use "Get Analytics Subsystem" node
 *
 * THREAD SAFETY:
 *   This subsystem is designed to be called from the game thread only.
 *   Do not call analytics functions from background threads.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAnalyticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Called by Unreal when the subsystem is created.
	 * Sets up timers, generates session ID, and prepares for tracking.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called by Unreal when the subsystem is destroyed (game shutdown).
	 * Flushes pending events and cleans up resources.
	 */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS (Delegates)
	// ==========================================

	/**
	 * Broadcast whenever an analytics event is queued.
	 * Bind to this to monitor analytics in real-time during development.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAnalyticsEventSent OnAnalyticsEventSent;

	// ==========================================
	// EVENT TRACKING - Core Functions
	// ==========================================
	// These functions are the primary way to record analytics events.
	// Choose the appropriate function based on what data you need to include.

	/**
	 * Track a simple event with just a name and category.
	 * Use for binary events like "MainMenuOpened" or "SettingsChanged".
	 *
	 * @param EventName - Name describing what happened
	 * @param Category - Category for filtering
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEvent(const FString& EventName, EMGAnalyticsCategory Category);

	/**
	 * Track an event with additional string properties.
	 * Use when you need categorical context like vehicle name or track ID.
	 *
	 * @param EventName - Name describing what happened
	 * @param Category - Category for filtering
	 * @param Properties - String key-value pairs (e.g., "VehicleName" -> "Speedster")
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEventWithProperties(const FString& EventName, EMGAnalyticsCategory Category,
		const TMap<FString, FString>& Properties);

	/**
	 * Track an event with numeric metrics.
	 * Use when you need measurable data like times, distances, or counts.
	 *
	 * @param EventName - Name describing what happened
	 * @param Category - Category for filtering
	 * @param Metrics - Numeric key-value pairs (e.g., "LapTime" -> 45.5f)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEventWithMetrics(const FString& EventName, EMGAnalyticsCategory Category,
		const TMap<FString, float>& Metrics);

	/**
	 * Track a fully-populated event structure.
	 * Use when you need complete control over all event fields.
	 *
	 * @param Event - Complete event data structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackFullEvent(const FMGAnalyticsEvent& Event);

	// ==========================================
	// GAMEPLAY TRACKING - Racing Events
	// ==========================================
	// Specialized functions for tracking racing-specific events.
	// These handle the most common gameplay scenarios automatically.

	/**
	 * Call when a race begins (after countdown, when control is given to player).
	 * Creates heat map entries and starts race timing.
	 *
	 * @param TrackID - Identifier for the track being raced
	 * @param VehicleID - Identifier for the player's vehicle
	 * @param bIsOnline - True if multiplayer, false if single-player/AI
	 * @param RacerCount - Total number of participants
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackRaceStart(FName TrackID, FName VehicleID, bool bIsOnline, int32 RacerCount);

	/**
	 * Call when a race ends (player finishes or quits).
	 * This is critical for balance analysis - make sure to set bRageQuit if player quits early.
	 *
	 * @param RaceData - Complete race analytics data structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackRaceEnd(const FMGRaceAnalytics& RaceData);

	/**
	 * Call when player's vehicle collides with something.
	 * Adds a point to the crash heat map for this track.
	 *
	 * @param Location - World position where crash occurred
	 * @param TrackID - Which track this happened on
	 * @param Speed - Vehicle speed at impact (for severity analysis)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackCrash(FVector Location, FName TrackID, float Speed);

	/**
	 * Call when player successfully passes another racer.
	 * Adds a point to the overtake heat map.
	 *
	 * @param Location - World position where overtake occurred
	 * @param TrackID - Which track this happened on
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackOvertake(FVector Location, FName TrackID);

	/**
	 * Call when player executes a drift.
	 * Adds a point to the drift heat map.
	 *
	 * @param Location - World position where drift started
	 * @param TrackID - Which track this happened on
	 * @param DriftScore - Points earned from drift (indicates quality)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackDrift(FVector Location, FName TrackID, float DriftScore);

	/**
	 * Call when player activates nitro boost.
	 * Adds a point to the nitro heat map.
	 *
	 * @param Location - World position where nitro was activated
	 * @param TrackID - Which track this happened on
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackNitroUse(FVector Location, FName TrackID);

	// ==========================================
	// ECONOMY TRACKING - Currency and Purchases
	// ==========================================
	// Track the flow of in-game currency. This data is essential for
	// balancing the game economy and identifying issues like:
	// - Players accumulating too much currency (nothing to spend on)
	// - Players unable to afford progression (paywall feeling)
	// - Certain sources being too rewarding or too stingy

	/**
	 * Call when player earns currency from any source.
	 *
	 * @param CurrencyType - Type of currency (e.g., "Cash", "Tokens", "Premium")
	 * @param Amount - How much was earned
	 * @param Source - Where it came from (e.g., "RaceReward", "Achievement", "DailyBonus")
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Economy")
	void TrackCurrencyEarned(const FString& CurrencyType, int32 Amount, const FString& Source);

	/**
	 * Call when player spends currency.
	 *
	 * @param CurrencyType - Type of currency spent
	 * @param Amount - How much was spent
	 * @param ItemType - Category of item (e.g., "Vehicle", "Upgrade", "Cosmetic")
	 * @param ItemID - Specific item identifier
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Economy")
	void TrackCurrencySpent(const FString& CurrencyType, int32 Amount, const FString& ItemType, FName ItemID);

	/**
	 * Call when a purchase transaction completes successfully.
	 * Similar to TrackCurrencySpent but focuses on the item rather than currency.
	 *
	 * @param ItemID - What was purchased
	 * @param ItemType - Category of item
	 * @param Price - Cost of the item
	 * @param CurrencyType - What currency was used
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Economy")
	void TrackPurchase(FName ItemID, const FString& ItemType, int32 Price, const FString& CurrencyType);

	// ==========================================
	// PROGRESSION TRACKING - Player Advancement
	// ==========================================
	// Track how players progress through the game. This data helps identify:
	// - If leveling is too fast or slow
	// - Which achievements are too hard or too easy
	// - Where players get stuck in tutorials

	/**
	 * Call when player gains a level.
	 *
	 * @param NewLevel - The new level the player reached
	 * @param TotalPlayTime - Total time played across all sessions (for progression curve analysis)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackLevelUp(int32 NewLevel, float TotalPlayTime);

	/**
	 * Call when player unlocks an achievement.
	 *
	 * @param AchievementID - Which achievement was unlocked
	 * @param TotalPlayTime - Time played when unlocked (shows difficulty/accessibility)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackAchievementUnlocked(FName AchievementID, float TotalPlayTime);

	/**
	 * Call when player starts or completes a tutorial step.
	 * Essential for improving new player experience.
	 *
	 * @param StepName - Identifier for the tutorial step
	 * @param bCompleted - True if completed, false if just started
	 * @param TimeSpent - How long player spent on this step (long times = confusion)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackTutorialStep(const FString& StepName, bool bCompleted, float TimeSpent);

	/**
	 * Call to record a step in a conversion funnel.
	 * Funnels track sequences like: Download -> Tutorial -> FirstRace -> FirstPurchase
	 *
	 * @param FunnelName - Name of the funnel (e.g., "NewPlayerOnboarding")
	 * @param StepName - Which step in the funnel
	 * @param bCompleted - True if player completed the step
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackFunnelStep(const FString& FunnelName, const FString& StepName, bool bCompleted);

	// ==========================================
	// SOCIAL TRACKING - Multiplayer Interactions
	// ==========================================
	// Track social features usage. Helps understand:
	// - Are social features being used?
	// - Which social features are most popular?
	// - How do social features impact retention?

	/**
	 * Call when player performs a social action.
	 *
	 * @param ActionType - What they did (e.g., "AddFriend", "SendGift", "InviteToRace")
	 * @param Context - Additional context (e.g., "FromLeaderboard", "FromRaceResult")
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Social")
	void TrackSocialAction(const FString& ActionType, const FString& Context);

	/**
	 * Call when player performs a crew/clan-related action.
	 *
	 * @param ActionType - What they did (e.g., "JoinCrew", "LeaveCrew", "CrewRace")
	 * @param CrewID - Which crew was involved
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Social")
	void TrackCrewAction(const FString& ActionType, FName CrewID);

	// ==========================================
	// TECHNICAL TRACKING - Performance and Errors
	// ==========================================
	// Track technical metrics. Unlike CrashReportingSubsystem which handles
	// fatal errors, this tracks non-fatal issues and performance data.

	/**
	 * Call when a non-fatal error occurs that should be tracked.
	 * For fatal errors/crashes, use CrashReportingSubsystem instead.
	 *
	 * @param ErrorType - Category of error (e.g., "NetworkTimeout", "AssetLoadFailed")
	 * @param ErrorMessage - Human-readable description
	 * @param StackTrace - Call stack if available (can be empty)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Technical")
	void TrackError(const FString& ErrorType, const FString& ErrorMessage, const FString& StackTrace);

	/**
	 * Call to record current performance metrics.
	 * Called automatically on a timer, but can be called manually for specific moments.
	 *
	 * @param Metrics - Performance data structure with FPS, memory, etc.
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Technical")
	void TrackPerformanceSnapshot(const FMGPerformanceMetrics& Metrics);

	/**
	 * Call when a loading operation completes.
	 * Helps identify slow loading areas that need optimization.
	 *
	 * @param LoadType - What was loaded (e.g., "Level", "Vehicle", "MainMenu")
	 * @param LoadTime - How long it took in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Technical")
	void TrackLoadingTime(const FString& LoadType, float LoadTime);

	// ==========================================
	// HEAT MAPS - Spatial Data Visualization
	// ==========================================
	// Heat maps show WHERE things happen on tracks. Used for level design
	// decisions and identifying problem areas.

	/**
	 * Retrieve all heat map data for a specific track.
	 * Use this to visualize the data or export for analysis.
	 *
	 * @param TrackID - Which track to get data for
	 * @return Heat map data structure with all event points
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|HeatMap")
	FMGTrackHeatMap GetTrackHeatMap(FName TrackID) const;

	/**
	 * Add a new point to a track's heat map.
	 * Usually called by the specific tracking functions (TrackCrash, TrackDrift, etc.)
	 * but can be called directly for custom event types.
	 *
	 * @param TrackID - Which track this occurred on
	 * @param EventType - Type of event (determines which array it goes in)
	 * @param Location - World position
	 * @param Intensity - Weight/significance of this point (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|HeatMap")
	void AddHeatMapPoint(FName TrackID, FName EventType, FVector Location, float Intensity = 1.0f);

	/**
	 * Clear all heat map data for a track.
	 * Useful when starting fresh after track redesign.
	 *
	 * @param TrackID - Which track to clear
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|HeatMap")
	void ClearHeatMapData(FName TrackID);

	// ==========================================
	// BALANCE DATA - Game Balance Analysis
	// ==========================================
	// Query aggregated data for game balance tuning. This data helps
	// designers identify overpowered/underpowered vehicles and track issues.

	/**
	 * Get balance statistics for a specific vehicle.
	 *
	 * @param VehicleID - Which vehicle to query
	 * @return Balance data (win rate, usage, average position, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Balance")
	FMGBalanceAnalytics GetVehicleBalanceData(FName VehicleID) const;

	/**
	 * Get balance data for ALL vehicles.
	 * Useful for generating comparison reports or balance dashboards.
	 *
	 * @return Array of balance data for each vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Analytics|Balance")
	TArray<FMGBalanceAnalytics> GetAllVehicleBalanceData() const;

	/**
	 * Get win rates by starting grid position for a track.
	 * Helps identify if certain starting positions give unfair advantages.
	 * Ideal: All positions should have roughly equal win rates (~12.5% for 8 racers).
	 *
	 * @param TrackID - Which track to analyze
	 * @return Map of starting position (1-8) to win rate (0.0-1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Balance")
	TMap<int32, float> GetTrackStartPositionWinRates(FName TrackID) const;

	// ==========================================
	// SESSION DATA - Current Session Info
	// ==========================================
	// Query information about the current play session. Useful for
	// displaying stats to the player or correlating with other systems.

	/**
	 * Get complete data for the current play session.
	 *
	 * @return Session analytics structure with all tracked data
	 */
	UFUNCTION(BlueprintPure, Category = "Analytics|Session")
	FMGSessionAnalytics GetCurrentSessionData() const { return CurrentSession; }

	/**
	 * Get just the session ID (unique identifier for this session).
	 * Useful for correlating events across different systems.
	 *
	 * @return GUID string for the current session
	 */
	UFUNCTION(BlueprintPure, Category = "Analytics|Session")
	FString GetSessionID() const { return CurrentSession.SessionID; }

	/**
	 * Get total play time across ALL sessions (lifetime).
	 * Loaded from save data at startup.
	 *
	 * @return Total seconds played
	 */
	UFUNCTION(BlueprintPure, Category = "Analytics|Session")
	float GetTotalPlayTime() const { return TotalPlayTime; }

	// ==========================================
	// CONFIGURATION - Analytics Settings
	// ==========================================
	// Control how analytics behaves. Players should be able to opt out
	// of analytics collection for privacy reasons.

	/**
	 * Enable or disable analytics collection.
	 * When disabled, tracking functions do nothing. Respects player privacy preferences.
	 *
	 * @param bEnabled - True to enable, false to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Config")
	void SetAnalyticsEnabled(bool bEnabled);

	/**
	 * Check if analytics is currently enabled.
	 *
	 * @return True if analytics collection is active
	 */
	UFUNCTION(BlueprintPure, Category = "Analytics|Config")
	bool IsAnalyticsEnabled() const { return bAnalyticsEnabled; }

	/**
	 * Set how often pending events are uploaded to the server.
	 * Events are queued locally and uploaded in batches to reduce network calls.
	 * Default is 60 seconds. Lower values = more up-to-date data but more network usage.
	 *
	 * @param Seconds - Time between batch uploads
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Config")
	void SetBatchUploadInterval(float Seconds);

	/**
	 * Force immediate upload of all pending events.
	 * Called automatically on shutdown, but can be called manually if needed
	 * (e.g., before a potential crash point).
	 */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Config")
	void FlushEvents();

protected:
	// ==========================================
	// PROTECTED DATA - Internal State
	// ==========================================
	// These variables store the subsystem's state. They are protected
	// so derived classes can access them if needed.

	/** Master switch - when false, all tracking functions are no-ops */
	UPROPERTY()
	bool bAnalyticsEnabled = true;

	/** Accumulated data for the current play session */
	UPROPERTY()
	FMGSessionAnalytics CurrentSession;

	/** Events waiting to be uploaded to the server */
	UPROPERTY()
	TArray<FMGAnalyticsEvent> PendingEvents;

	/** Heat map data indexed by track ID */
	UPROPERTY()
	TMap<FName, FMGTrackHeatMap> TrackHeatMaps;

	/** Balance statistics indexed by vehicle ID */
	UPROPERTY()
	TMap<FName, FMGBalanceAnalytics> VehicleBalanceData;

	/** Funnel progression data indexed by funnel name */
	UPROPERTY()
	TMap<FString, TArray<FMGFunnelStep>> FunnelData;

	/** Lifetime play time in seconds (loaded from save data) */
	UPROPERTY()
	float TotalPlayTime = 0.0f;

	/** Seconds between batch uploads (default 60) */
	UPROPERTY()
	float BatchUploadInterval = 60.0f;

	/** Seconds between performance samples (default 5) */
	UPROPERTY()
	float PerformanceSampleInterval = 5.0f;

	/** Handle for the batch upload timer - used to cancel on shutdown */
	FTimerHandle BatchUploadTimerHandle;

	/** Handle for the performance sampling timer */
	FTimerHandle PerformanceSampleTimerHandle;

	/** Unique identifier for this player (persistent across sessions) */
	UPROPERTY()
	FString PlayerID;

	// ==========================================
	// INTERNAL FUNCTIONS - Implementation Details
	// ==========================================
	// These functions are used internally by the subsystem.
	// You generally don't need to call these directly.

	/** Initialize a new session with timestamp and generated ID */
	void StartSession();

	/** Finalize the current session and upload remaining data */
	void EndSession();

	/**
	 * Add an event to the pending queue for batch upload.
	 * Automatically adds SessionID, PlayerID, and Timestamp.
	 */
	void QueueEvent(const FMGAnalyticsEvent& Event);

	/** Send all pending events to the analytics backend */
	void UploadPendingEvents();

	/** Capture current FPS, memory, etc. and queue as an event */
	void SamplePerformanceMetrics();

	/** Update the session duration field based on current time */
	void UpdateSessionDuration();

	/** Create a unique GUID string for session identification */
	FString GenerateSessionID() const;

	/** Collect hardware/platform info for performance correlation */
	FString GetDeviceInfo() const;

	/**
	 * Update the aggregated balance statistics for a vehicle.
	 * Called after each race to track win rates, positions, etc.
	 */
	void UpdateVehicleBalanceStats(FName VehicleID, int32 Position, float LapTime);
};
