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
 */
USTRUCT(BlueprintType)
struct FMGSessionAnalytics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FDateTime SessionStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FDateTime SessionEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	float SessionDuration = 0.0f; // Seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 RacesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 RacesWon = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CashEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CashSpent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 XPEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 LevelsGained = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 VehiclesPurchased = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 AchievementsUnlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString Platform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString DeviceInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	float AverageFPS = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 CrashCount = 0;
};

/**
 * Race Analytics Data
 */
USTRUCT(BlueprintType)
struct FMGRaceAnalytics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString RaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 FinalPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float RaceTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 TotalLaps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 Collisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float DriftDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 NitroUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 Overtakes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsOnline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 RacerCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bRageQuit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FDateTime RaceTimestamp;
};

/**
 * Balance Analytics Data
 */
USTRUCT(BlueprintType)
struct FMGBalanceAnalytics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	int32 TimesUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float WinRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float AveragePosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float AverageLapTime = 0.0f;
};

/**
 * Funnel Step
 */
USTRUCT(BlueprintType)
struct FMGFunnelStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	FString StepName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	int32 UsersReached = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	int32 UsersCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	float ConversionRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Funnel")
	float AverageTimeSeconds = 0.0f;
};

/**
 * Analytics Event
 */
USTRUCT(BlueprintType)
struct FMGAnalyticsEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EMGAnalyticsCategory Category = EMGAnalyticsCategory::Gameplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FString, FString> Properties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	TMap<FString, float> Metrics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString PlayerID;
};

/**
 * Performance Metrics
 */
USTRUCT(BlueprintType)
struct FMGPerformanceMetrics
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageFPS = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MinFPS = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxFPS = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageFrameTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float GPU_Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float CPU_Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int64 MemoryUsedMB = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 DrawCalls = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 TrianglesRendered = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float LoadTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 HitchCount = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalyticsEventSent, const FMGAnalyticsEvent&, Event);

/**
 * Analytics Subsystem
 * Tracks player behavior, performance, and balance data
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAnalyticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAnalyticsEventSent OnAnalyticsEventSent;

	// ==========================================
	// EVENT TRACKING
	// ==========================================

	/** Track custom event */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEvent(const FString& EventName, EMGAnalyticsCategory Category);

	/** Track event with properties */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEventWithProperties(const FString& EventName, EMGAnalyticsCategory Category,
		const TMap<FString, FString>& Properties);

	/** Track event with metrics */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEventWithMetrics(const FString& EventName, EMGAnalyticsCategory Category,
		const TMap<FString, float>& Metrics);

	/** Track full event */
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackFullEvent(const FMGAnalyticsEvent& Event);

	// ==========================================
	// GAMEPLAY TRACKING
	// ==========================================

	/** Track race start */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackRaceStart(FName TrackID, FName VehicleID, bool bIsOnline, int32 RacerCount);

	/** Track race end */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackRaceEnd(const FMGRaceAnalytics& RaceData);

	/** Track crash/collision */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackCrash(FVector Location, FName TrackID, float Speed);

	/** Track overtake */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackOvertake(FVector Location, FName TrackID);

	/** Track drift */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackDrift(FVector Location, FName TrackID, float DriftScore);

	/** Track nitro use */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Gameplay")
	void TrackNitroUse(FVector Location, FName TrackID);

	// ==========================================
	// ECONOMY TRACKING
	// ==========================================

	/** Track currency earned */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Economy")
	void TrackCurrencyEarned(const FString& CurrencyType, int32 Amount, const FString& Source);

	/** Track currency spent */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Economy")
	void TrackCurrencySpent(const FString& CurrencyType, int32 Amount, const FString& ItemType, FName ItemID);

	/** Track purchase */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Economy")
	void TrackPurchase(FName ItemID, const FString& ItemType, int32 Price, const FString& CurrencyType);

	// ==========================================
	// PROGRESSION TRACKING
	// ==========================================

	/** Track level up */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackLevelUp(int32 NewLevel, float TotalPlayTime);

	/** Track achievement unlocked */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackAchievementUnlocked(FName AchievementID, float TotalPlayTime);

	/** Track tutorial step */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackTutorialStep(const FString& StepName, bool bCompleted, float TimeSpent);

	/** Track funnel step */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Progression")
	void TrackFunnelStep(const FString& FunnelName, const FString& StepName, bool bCompleted);

	// ==========================================
	// SOCIAL TRACKING
	// ==========================================

	/** Track social action */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Social")
	void TrackSocialAction(const FString& ActionType, const FString& Context);

	/** Track crew action */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Social")
	void TrackCrewAction(const FString& ActionType, FName CrewID);

	// ==========================================
	// TECHNICAL TRACKING
	// ==========================================

	/** Track error */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Technical")
	void TrackError(const FString& ErrorType, const FString& ErrorMessage, const FString& StackTrace);

	/** Track performance snapshot */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Technical")
	void TrackPerformanceSnapshot(const FMGPerformanceMetrics& Metrics);

	/** Track loading time */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Technical")
	void TrackLoadingTime(const FString& LoadType, float LoadTime);

	// ==========================================
	// HEAT MAPS
	// ==========================================

	/** Get track heat map */
	UFUNCTION(BlueprintCallable, Category = "Analytics|HeatMap")
	FMGTrackHeatMap GetTrackHeatMap(FName TrackID) const;

	/** Add heat map point */
	UFUNCTION(BlueprintCallable, Category = "Analytics|HeatMap")
	void AddHeatMapPoint(FName TrackID, FName EventType, FVector Location, float Intensity = 1.0f);

	/** Clear heat map data */
	UFUNCTION(BlueprintCallable, Category = "Analytics|HeatMap")
	void ClearHeatMapData(FName TrackID);

	// ==========================================
	// BALANCE DATA
	// ==========================================

	/** Get vehicle balance data */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Balance")
	FMGBalanceAnalytics GetVehicleBalanceData(FName VehicleID) const;

	/** Get all vehicle balance data */
	UFUNCTION(BlueprintPure, Category = "Analytics|Balance")
	TArray<FMGBalanceAnalytics> GetAllVehicleBalanceData() const;

	/** Get track balance data (win rates by starting position) */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Balance")
	TMap<int32, float> GetTrackStartPositionWinRates(FName TrackID) const;

	// ==========================================
	// SESSION DATA
	// ==========================================

	/** Get current session data */
	UFUNCTION(BlueprintPure, Category = "Analytics|Session")
	FMGSessionAnalytics GetCurrentSessionData() const { return CurrentSession; }

	/** Get session ID */
	UFUNCTION(BlueprintPure, Category = "Analytics|Session")
	FString GetSessionID() const { return CurrentSession.SessionID; }

	/** Get total play time (all sessions) */
	UFUNCTION(BlueprintPure, Category = "Analytics|Session")
	float GetTotalPlayTime() const { return TotalPlayTime; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set analytics enabled */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Config")
	void SetAnalyticsEnabled(bool bEnabled);

	/** Is analytics enabled */
	UFUNCTION(BlueprintPure, Category = "Analytics|Config")
	bool IsAnalyticsEnabled() const { return bAnalyticsEnabled; }

	/** Set batch upload interval */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Config")
	void SetBatchUploadInterval(float Seconds);

	/** Force upload pending events */
	UFUNCTION(BlueprintCallable, Category = "Analytics|Config")
	void FlushEvents();

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Analytics enabled */
	UPROPERTY()
	bool bAnalyticsEnabled = true;

	/** Current session data */
	UPROPERTY()
	FMGSessionAnalytics CurrentSession;

	/** Pending events to upload */
	UPROPERTY()
	TArray<FMGAnalyticsEvent> PendingEvents;

	/** Heat map data per track */
	UPROPERTY()
	TMap<FName, FMGTrackHeatMap> TrackHeatMaps;

	/** Vehicle balance data */
	UPROPERTY()
	TMap<FName, FMGBalanceAnalytics> VehicleBalanceData;

	/** Funnel data */
	UPROPERTY()
	TMap<FString, TArray<FMGFunnelStep>> FunnelData;

	/** Total play time across all sessions */
	UPROPERTY()
	float TotalPlayTime = 0.0f;

	/** Batch upload interval */
	UPROPERTY()
	float BatchUploadInterval = 60.0f;

	/** Performance sampling interval */
	UPROPERTY()
	float PerformanceSampleInterval = 5.0f;

	/** Timer handles */
	FTimerHandle BatchUploadTimerHandle;
	FTimerHandle PerformanceSampleTimerHandle;

	/** Player ID */
	UPROPERTY()
	FString PlayerID;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Start new session */
	void StartSession();

	/** End session */
	void EndSession();

	/** Queue event for upload */
	void QueueEvent(const FMGAnalyticsEvent& Event);

	/** Upload pending events */
	void UploadPendingEvents();

	/** Sample performance metrics */
	void SamplePerformanceMetrics();

	/** Update session duration */
	void UpdateSessionDuration();

	/** Generate session ID */
	FString GenerateSessionID() const;

	/** Get device info string */
	FString GetDeviceInfo() const;

	/** Update vehicle balance stats */
	void UpdateVehicleBalanceStats(FName VehicleID, int32 Position, float LapTime);
};
