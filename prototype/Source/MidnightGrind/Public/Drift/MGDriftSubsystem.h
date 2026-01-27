// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGDriftSubsystem.h
 * @brief Drift Gameplay Mechanics Subsystem
 *
 * This subsystem manages all drift-related gameplay mechanics in Midnight Grind.
 * Drifting is a core mechanic where players slide their vehicle sideways through
 * corners to accumulate points and build multipliers.
 *
 * Key Features:
 * - Real-time drift detection based on slip angle and speed thresholds
 * - Progressive scoring system with grades (D through SSS)
 * - Chain combos for linking multiple drifts together
 * - Bonus points for advanced techniques (tandem, counter-steer, wall taps)
 * - Drift zones with unique multipliers and leaderboards
 * - Session and career statistics tracking
 *
 * How Drifting Works:
 * 1. Player initiates a drift when slip angle exceeds MinDriftAngle at speed
 * 2. Points accumulate based on angle, speed, and duration
 * 3. Bonuses trigger for special maneuvers (wall proximity, tandem with AI)
 * 4. Grade improves as point thresholds are reached
 * 5. Chain extends if new drift starts within ChainTimeWindow
 * 6. Drift ends when angle drops below threshold or player crashes
 *
 * @see FMGActiveDrift for current drift state
 * @see FMGDriftResult for completed drift data
 * @see FMGDriftConfig for tuning parameters
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDriftSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Drift performance grade based on accumulated points.
 *
 * Grades provide visual feedback to players about drift quality.
 * Higher grades unlock achievements and provide bonus rewards.
 */
UENUM(BlueprintType)
enum class EMGDriftGrade : uint8
{
	None,   ///< No drift in progress or below minimum threshold
	D,      ///< Basic drift (1,000+ points)
	C,      ///< Decent drift (2,500+ points)
	B,      ///< Good drift (5,000+ points)
	A,      ///< Great drift (10,000+ points)
	S,      ///< Excellent drift (25,000+ points)
	SS,     ///< Outstanding drift (50,000+ points)
	SSS     ///< Perfect/legendary drift (100,000+ points)
};

/**
 * @brief Types of drift zones with unique scoring characteristics.
 *
 * Different zone types offer varied challenges and point multipliers.
 * Players can master specific zone types for leaderboard placement.
 */
UENUM(BlueprintType)
enum class EMGDriftZoneType : uint8
{
	None,        ///< Not in a drift zone
	Standard,    ///< Normal corner, base multiplier
	Hairpin,     ///< Sharp 180-degree turn, high difficulty bonus
	SwitchBack,  ///< S-curve requiring direction changes
	Downhill,    ///< Gravity-assisted, speed bonus potential
	Touge,       ///< Mountain pass style, narrow with drops
	Highway,     ///< High-speed sweeping curves
	Parking      ///< Tight spaces for donut/figure-8 practice
};

/**
 * @brief Special bonus types that can trigger during a drift.
 *
 * These bonuses reward skillful driving and add variety to scoring.
 * Multiple bonuses can be active simultaneously for massive points.
 */
UENUM(BlueprintType)
enum class EMGDriftChainBonus : uint8
{
	None,        ///< No special bonus active
	Tandem,      ///< Drifting alongside another vehicle
	Counter,     ///< Quick direction change mid-drift
	Manji,       ///< Side-to-side weaving while drifting
	Feint,       ///< Initiating drift with opposite steering input
	WallTap,     ///< Intentionally touching wall during drift
	DonutEntry,  ///< Entering corner with a full rotation
	CloseCall,   ///< Near miss with obstacle while drifting
	Overtake     ///< Passing another vehicle during drift
};

//=============================================================================
// Data Structures - Drift State
//=============================================================================

/**
 * @brief Current state of an active drift in progress.
 *
 * This structure is updated every frame while drifting.
 * Use GetActiveDrift() to query the current state.
 */
USTRUCT(BlueprintType)
struct FMGActiveDrift
{
	GENERATED_BODY()

	/// True when vehicle is currently in a drift state
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDrifting = false;

	/// Current slip angle in degrees (0 = straight, 90 = perpendicular)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentAngle = 0.0f;

	/// Current vehicle speed in km/h during drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// How long this drift has been active in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftDuration = 0.0f;

	/// Total distance covered while drifting in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftDistance = 0.0f;

	/// Accumulated points for this drift (before final multiplier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPoints = 0;

	/// Current score multiplier (increases with bonuses and chains)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/// Number of consecutive drifts in current chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	/// Current drift grade based on point thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftGrade CurrentGrade = EMGDriftGrade::None;

	/// True if drifting in reverse (special technique)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReverse = false;

	/// Closest wall distance recorded during drift (for wall tap bonus)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinWallDistance = 9999.0f;

	/// List of bonuses triggered during this drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDriftChainBonus> ActiveBonuses;
};

/**
 * @brief Final results of a completed drift.
 *
 * Created when a drift ends (successfully or failed).
 * Used for scoring, statistics, and UI display.
 */
USTRUCT(BlueprintType)
struct FMGDriftResult
{
	GENERATED_BODY()

	/// Points before any multipliers applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	/// Additional points from bonuses
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusPoints = 0;

	/// Final score after all multipliers (BasePoints + BonusPoints) * Multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	/// Final multiplier value at drift end
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalMultiplier = 1.0f;

	/// Total drift duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	/// Total distance drifted in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	/// Maximum slip angle achieved during drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAngle = 0.0f;

	/// Maximum speed reached during drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	/// Final grade achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftGrade Grade = EMGDriftGrade::None;

	/// Chain count at drift completion
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	/// All bonuses earned during this drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDriftChainBonus> Bonuses;

	/// True if drift was executed flawlessly (no corrections needed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPerfect = false;

	/// True if drift ended due to collision/spinout
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFailed = false;
};

//=============================================================================
// Data Structures - Zones and Leaderboards
//=============================================================================

/**
 * @brief Definition of a drift zone area in the game world.
 *
 * Drift zones are designated areas where scoring is enhanced.
 * Each zone has target scores for medals and leaderboard placement.
 */
USTRUCT(BlueprintType)
struct FMGDriftZone
{
	GENERATED_BODY()

	/// Unique identifier for this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	/// Display name shown to players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ZoneName;

	/// Type of zone affecting difficulty and scoring
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftZoneType ZoneType = EMGDriftZoneType::Standard;

	/// Score multiplier applied within this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsMultiplier = 1.0f;

	/// Points needed for bronze medal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 10000;

	/// Points needed for gold medal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldScore = 25000;

	/// Points needed for platinum medal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumScore = 50000;

	/// World position of zone center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ZoneCenter = FVector::ZeroVector;

	/// Radius of zone activation area in centimeters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoneRadius = 1000.0f;
};

/**
 * @brief Statistics tracked during a single gameplay session.
 *
 * Reset when starting a new race or free roam session.
 */
USTRUCT(BlueprintType)
struct FMGDriftSessionStats
{
	GENERATED_BODY()

	/// Cumulative points earned this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	/// Number of individual drifts performed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDrifts = 0;

	/// Highest chain combo achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestChain = 0;

	/// Best single drift score
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestSingleDrift = 0;

	/// Combined distance of all drifts in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDriftDistance = 0.0f;

	/// Combined time spent drifting in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDriftTime = 0.0f;

	/// Highest slip angle achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDriftAngle = 0.0f;

	/// Highest speed while drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDriftSpeed = 0.0f;

	/// Number of flawless drifts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectDrifts = 0;

	/// Number of crashed/spinout drifts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FailedDrifts = 0;

	/// Count of drifts per grade achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDriftGrade, int32> GradeCounts;

	/// Count of each bonus type triggered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDriftChainBonus, int32> BonusCounts;
};

/**
 * @brief Single entry in a drift zone leaderboard.
 */
USTRUCT(BlueprintType)
struct FMGDriftLeaderboardEntry
{
	GENERATED_BODY()

	/// Position on leaderboard (1 = first place)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	/// Display name of the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	/// Unique player identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	/// Score achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score = 0;

	/// Vehicle used for this score
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// When this score was set
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SetAt;
};

//=============================================================================
// Data Structures - Configuration
//=============================================================================

/**
 * @brief Configuration parameters for drift detection and scoring.
 *
 * Adjust these values to tune the feel and balance of drifting.
 * Can be modified at runtime for difficulty settings.
 */
USTRUCT(BlueprintType)
struct FMGDriftConfig
{
	GENERATED_BODY()

	/// Minimum slip angle (degrees) required to register as drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDriftAngle = 15.0f;

	/// Minimum speed (km/h) required to register as drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDriftSpeed = 50.0f;

	/// Time window (seconds) to chain consecutive drifts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainTimeWindow = 2.0f;

	/// Base points earned per second of drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePointsPerSecond = 100.0f;

	/// How much drift angle increases the multiplier (per degree)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleMultiplierScale = 0.02f;

	/// How much speed increases the multiplier (per km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplierScale = 0.01f;

	/// Distance from wall (cm) to trigger proximity bonus
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallProximityBonusDistance = 100.0f;

	/// Multiplier boost when near walls
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallProximityMultiplier = 1.5f;

	/// Maximum distance (cm) to another drifting vehicle for tandem
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TandemBonusDistance = 300.0f;

	/// Multiplier boost during tandem drifting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TandemMultiplier = 2.0f;

	//--- Grade Point Thresholds ---//

	/// Points required for SSS grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SSSThreshold = 100000;

	/// Points required for SS grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SSThreshold = 50000;

	/// Points required for S grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SThreshold = 25000;

	/// Points required for A grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AThreshold = 10000;

	/// Points required for B grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BThreshold = 5000;

	/// Points required for C grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CThreshold = 2500;

	/// Points required for D grade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DThreshold = 1000;
};

//=============================================================================
// Delegates (Event Callbacks)
//=============================================================================

/// Fired when a new drift begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftStarted, const FMGActiveDrift&, Drift);

/// Fired when a drift ends (successfully or failed)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftEnded, const FMGDriftResult&, Result);

/// Fired when drift grade improves or drops
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftGradeChanged, EMGDriftGrade, OldGrade, EMGDriftGrade, NewGrade);

/// Fired when a bonus is triggered during drift
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftBonusTriggered, EMGDriftChainBonus, Bonus, int32, Points);

/// Fired when chain is extended with a new drift
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftChainExtended, int32, ChainCount, float, Multiplier);

/// Fired specifically when a drift fails (crash/spinout)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftFailed, const FMGDriftResult&, Result);

/// Fired when entering a designated drift zone
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftZoneEntered, const FMGDriftZone&, Zone, bool, bNewHighScore);

/// Fired when reaching a point milestone (e.g., 10000, 25000)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftScoreMilestone, int32, Score, FName, MilestoneName);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Core subsystem managing all drift gameplay mechanics.
 *
 * This subsystem handles:
 * - Drift state detection and tracking
 * - Point calculation and grade progression
 * - Chain combo management
 * - Drift zone registration and leaderboards
 * - Session and career statistics
 *
 * Access this subsystem from any Blueprint or C++ code via:
 * @code
 * UMGDriftSubsystem* DriftSystem = GetGameInstance()->GetSubsystem<UMGDriftSubsystem>();
 * @endcode
 *
 * @note This is a GameInstanceSubsystem, so data persists across level loads.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDriftSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// Lifecycle
	//=========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//=========================================================================
	// Core Drift Functions
	//=========================================================================

	/**
	 * @brief Updates the drift state based on vehicle physics.
	 *
	 * Call this every frame from your vehicle pawn to maintain drift tracking.
	 *
	 * @param DeltaTime Frame delta time in seconds
	 * @param SlipAngle Current slip angle in degrees
	 * @param Speed Current speed in km/h
	 * @param Position Current world position
	 * @param Velocity Current velocity vector
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void UpdateDriftState(float DeltaTime, float SlipAngle, float Speed, const FVector& Position, const FVector& Velocity);

	/**
	 * @brief Manually starts a drift (usually called automatically by UpdateDriftState).
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void StartDrift();

	/**
	 * @brief Ends the current drift and calculates final score.
	 * @param bFailed True if drift ended due to crash/spinout
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void EndDrift(bool bFailed = false);

	/**
	 * @brief Resets all session statistics (call when starting new race).
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void ResetSession();

	/** @return True if currently in a drift state */
	UFUNCTION(BlueprintPure, Category = "Drift|Core")
	bool IsDrifting() const { return ActiveDrift.bIsDrifting; }

	/** @return Current active drift data */
	UFUNCTION(BlueprintPure, Category = "Drift|Core")
	FMGActiveDrift GetActiveDrift() const { return ActiveDrift; }

	/** @return Seconds since last drift ended (for chain timing) */
	UFUNCTION(BlueprintPure, Category = "Drift|Core")
	float GetTimeSinceLastDrift() const;

	//=========================================================================
	// Scoring Functions
	//=========================================================================

	/** @return Current drift score (before final multiplier) */
	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	int32 GetCurrentScore() const { return ActiveDrift.CurrentPoints; }

	/** @return Total points earned this session */
	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	int32 GetSessionScore() const { return SessionStats.TotalPoints; }

	/** @return Current score multiplier */
	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	float GetCurrentMultiplier() const { return ActiveDrift.Multiplier; }

	/** @return Current drift grade */
	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	EMGDriftGrade GetCurrentGrade() const { return ActiveDrift.CurrentGrade; }

	/**
	 * @brief Determines grade based on point value.
	 * @param Points Score to evaluate
	 * @return Corresponding grade
	 */
	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	EMGDriftGrade CalculateGradeFromPoints(int32 Points) const;

	//=========================================================================
	// Bonus Functions
	//=========================================================================

	/**
	 * @brief Triggers a bonus during the current drift.
	 * @param Bonus Type of bonus to activate
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void TriggerBonus(EMGDriftChainBonus Bonus);

	/**
	 * @brief Updates wall proximity for wall tap bonus calculation.
	 * @param Distance Distance to nearest wall in centimeters
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void SetWallProximity(float Distance);

	/**
	 * @brief Updates tandem drift status.
	 * @param bHasTandemPartner True if another vehicle is drifting nearby
	 * @param PartnerDistance Distance to tandem partner in centimeters
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void SetTandemPartner(bool bHasTandemPartner, float PartnerDistance);

	/**
	 * @brief Registers an overtake during drift for bonus points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void RegisterOvertake();

	/**
	 * @brief Registers a close call during drift for bonus points.
	 * @param Distance Distance to obstacle in centimeters
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void RegisterCloseCall(float Distance);

	/**
	 * @brief Gets the point value for a specific bonus type.
	 * @param Bonus Bonus type to query
	 * @return Base points awarded for this bonus
	 */
	UFUNCTION(BlueprintPure, Category = "Drift|Bonus")
	int32 GetBonusPoints(EMGDriftChainBonus Bonus) const;

	//=========================================================================
	// Chain Management
	//=========================================================================

	/** @return Current chain count */
	UFUNCTION(BlueprintPure, Category = "Drift|Chain")
	int32 GetChainCount() const { return ActiveDrift.ChainCount; }

	/** @return True if chain timer is still active */
	UFUNCTION(BlueprintPure, Category = "Drift|Chain")
	bool IsChainActive() const;

	/**
	 * @brief Extends the current chain (called automatically when new drift starts in time).
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Chain")
	void ExtendChain();

	/**
	 * @brief Breaks the current chain (called when chain timer expires).
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Chain")
	void BreakChain();

	//=========================================================================
	// Zone Functions
	//=========================================================================

	/**
	 * @brief Call when player enters a drift zone.
	 * @param Zone The drift zone being entered
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Zones")
	void EnterDriftZone(const FMGDriftZone& Zone);

	/**
	 * @brief Call when player exits a drift zone.
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Zones")
	void ExitDriftZone();

	/** @return True if currently in a drift zone */
	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	bool IsInDriftZone() const { return bInDriftZone; }

	/** @return Current drift zone data */
	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	FMGDriftZone GetCurrentDriftZone() const { return CurrentZone; }

	/**
	 * @brief Registers a new drift zone in the system.
	 * @param Zone Zone definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Zones")
	void RegisterDriftZone(const FMGDriftZone& Zone);

	/** @return All registered drift zones */
	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	TArray<FMGDriftZone> GetAllDriftZones() const;

	/**
	 * @brief Gets the high score for a specific zone.
	 * @param ZoneID Zone to query
	 * @return Best score achieved in this zone
	 */
	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	int32 GetZoneHighScore(FName ZoneID) const;

	//=========================================================================
	// Statistics Functions
	//=========================================================================

	/** @return Current session statistics */
	UFUNCTION(BlueprintPure, Category = "Drift|Stats")
	FMGDriftSessionStats GetSessionStats() const { return SessionStats; }

	/** @return All-time career statistics */
	UFUNCTION(BlueprintPure, Category = "Drift|Stats")
	FMGDriftSessionStats GetCareerStats() const { return CareerStats; }

	/**
	 * @brief Adds session stats to career totals (call at end of session).
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Stats")
	void MergeSessionToCareer();

	//=========================================================================
	// Leaderboard Functions
	//=========================================================================

	/**
	 * @brief Gets leaderboard entries for a zone.
	 * @param ZoneID Zone to query
	 * @param MaxEntries Maximum entries to return
	 * @return Sorted leaderboard entries
	 */
	UFUNCTION(BlueprintPure, Category = "Drift|Leaderboard")
	TArray<FMGDriftLeaderboardEntry> GetZoneLeaderboard(FName ZoneID, int32 MaxEntries = 50) const;

	/**
	 * @brief Submits a score to a zone leaderboard.
	 * @param ZoneID Zone to submit to
	 * @param Score Score achieved
	 * @param VehicleID Vehicle used
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Leaderboard")
	void SubmitZoneScore(FName ZoneID, int32 Score, FName VehicleID);

	/**
	 * @brief Gets player's rank on a zone leaderboard.
	 * @param ZoneID Zone to query
	 * @return Player's rank (0 if not on board)
	 */
	UFUNCTION(BlueprintPure, Category = "Drift|Leaderboard")
	int32 GetZoneLeaderboardPosition(FName ZoneID) const;

	//=========================================================================
	// Configuration Functions
	//=========================================================================

	/**
	 * @brief Updates drift configuration parameters.
	 * @param NewConfig New configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Drift|Config")
	void SetDriftConfig(const FMGDriftConfig& NewConfig);

	/** @return Current drift configuration */
	UFUNCTION(BlueprintPure, Category = "Drift|Config")
	FMGDriftConfig GetDriftConfig() const { return Config; }

	//=========================================================================
	// Delegates (Bindable Events)
	//=========================================================================

	/// Broadcast when a drift begins
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftStarted OnDriftStarted;

	/// Broadcast when a drift ends
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftEnded OnDriftEnded;

	/// Broadcast when grade changes
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftGradeChanged OnDriftGradeChanged;

	/// Broadcast when a bonus triggers
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftBonusTriggered OnDriftBonusTriggered;

	/// Broadcast when chain extends
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftChainExtended OnDriftChainExtended;

	/// Broadcast when drift fails
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftFailed OnDriftFailed;

	/// Broadcast when entering a drift zone
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftZoneEntered OnDriftZoneEntered;

	/// Broadcast when hitting a score milestone
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftScoreMilestone OnDriftScoreMilestone;

protected:
	//=========================================================================
	// Internal Helper Functions
	//=========================================================================

	/// Calculates and accumulates points each frame
	void CalculatePoints(float DeltaTime);

	/// Updates the current grade based on points
	void UpdateGrade();

	/// Recalculates the score multiplier
	void UpdateMultiplier();

	/// Checks and broadcasts milestone achievements
	void CheckMilestones(int32 OldScore, int32 NewScore);

	/// Creates the final drift result structure
	FMGDriftResult BuildDriftResult(bool bFailed);

	/// Persists drift data to save game
	void SaveDriftData();

	/// Loads drift data from save game
	void LoadDriftData();

	//=========================================================================
	// Internal State
	//=========================================================================

	/// Current drift state
	UPROPERTY()
	FMGActiveDrift ActiveDrift;

	/// Current session statistics
	UPROPERTY()
	FMGDriftSessionStats SessionStats;

	/// All-time career statistics
	UPROPERTY()
	FMGDriftSessionStats CareerStats;

	/// Current configuration
	UPROPERTY()
	FMGDriftConfig Config;

	/// Current drift zone (if any)
	UPROPERTY()
	FMGDriftZone CurrentZone;

	/// All registered drift zones
	UPROPERTY()
	TArray<FMGDriftZone> RegisteredZones;

	/// Best scores per zone
	UPROPERTY()
	TMap<FName, int32> ZoneHighScores;

	/// Leaderboard entries per zone
	UPROPERTY()
	TMap<FName, TArray<FMGDriftLeaderboardEntry>> ZoneLeaderboards;

	/// True if currently in a drift zone
	UPROPERTY()
	bool bInDriftZone = false;

	/// Timestamp when last drift ended
	UPROPERTY()
	float LastDriftEndTime = 0.0f;

	/// Maximum angle achieved in current drift
	UPROPERTY()
	float MaxAngleThisDrift = 0.0f;

	/// Maximum speed achieved in current drift
	UPROPERTY()
	float MaxSpeedThisDrift = 0.0f;

	/// True if tandem partner detected
	UPROPERTY()
	bool bHasTandemPartner = false;

	/// Distance to tandem partner
	UPROPERTY()
	float TandemPartnerDistance = 0.0f;

	/// Last milestone score reached
	UPROPERTY()
	int32 LastMilestoneReached = 0;
};
