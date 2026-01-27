// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPenaltySubsystem.h
 * @brief Race Penalty and Rules Enforcement Subsystem
 *
 * This subsystem manages all racing penalties, violations, and rule enforcement for
 * Midnight Grind. It handles the complete lifecycle of penalties from detection through
 * resolution, including track limits monitoring, collision fault determination, and
 * penalty serving mechanics.
 *
 * Key responsibilities:
 * - Detecting and recording rule violations (track limits, collisions, false starts, etc.)
 * - Issuing appropriate penalties based on violation severity and race rules
 * - Tracking penalty state and ensuring timely service
 * - Managing driver incident history for progressive disciplinary action
 * - Broadcasting penalty events to UI and other game systems
 *
 * Integration Points:
 * - Works with MGCautionSubsystem for incident-triggered caution flags
 * - Feeds data to MGReportSubsystem for player conduct tracking
 * - Coordinates with race timing systems for lap time deletion
 *
 * @see UMGCautionSubsystem
 * @see UMGReportSubsystem
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPenaltySubsystem.generated.h"

//=============================================================================
// Penalty Type Enumerations
//=============================================================================

/**
 * @brief Types of penalties that can be issued to drivers
 *
 * Penalties are ordered roughly by severity, from lightest (Warning) to
 * most severe (Exclusion). Different racing series may use different
 * subsets of these penalties.
 */
UENUM(BlueprintType)
enum class EMGPenaltyType : uint8
{
	None,                 ///< No penalty - used as default/cleared state
	Warning,              ///< Official warning, no time or position impact
	TimeAdded,            ///< Time added to race result (e.g., +5 seconds)
	DriveThrough,         ///< Must drive through pit lane without stopping
	StopAndGo,            ///< Must stop in pit box for specified duration
	PositionPenalty,      ///< Drop positions during race (e.g., give back position)
	GridPenalty,          ///< Grid penalty for next race start
	Disqualification,     ///< Removed from current race results
	Exclusion,            ///< Excluded from event (more severe than DQ)
	PointsDeduction,      ///< Championship points removed
	FinePenalty,          ///< Monetary fine (in-game currency)
	LicensePoints         ///< Penalty points on racing license
};

/**
 * @brief Categories of rule violations that trigger penalties
 *
 * These represent the various ways a driver can break racing rules.
 * Each violation type maps to specific penalty responses based on
 * the configured FMGPenaltyRules.
 */
UENUM(BlueprintType)
enum class EMGViolationType : uint8
{
	None,                   ///< No violation - default state
	TrackLimits,            ///< Exceeding track boundaries (4 wheels off)
	CuttingCorner,          ///< Cutting inside a corner to gain advantage
	GainingAdvantage,       ///< General unfair advantage (off-track or otherwise)
	IllegalOvertake,        ///< Passing under yellow flags or during caution
	Collision,              ///< Contact with another vehicle deemed at-fault
	DangerousDriving,       ///< Reckless driving endangering others
	UnsafeRejoin,           ///< Rejoining track unsafely after going off
	Blocking,               ///< Excessive defensive moves or blocking
	IgnoringFlags,          ///< Not respecting flag signals (blue, yellow, etc.)
	FalseStart,             ///< Moving before race start signal
	PitSpeeding,            ///< Exceeding pit lane speed limit
	PitLaneViolation,       ///< Other pit lane rule breaches
	UnservedPenalty,        ///< Failing to serve a penalty within allowed laps
	TechnicalInfringement,  ///< Vehicle not meeting technical regulations
	UnsportsmanlikeConduct, ///< General unsporting behavior
	TeamOrders              ///< Illegal team orders affecting race outcome
};

/**
 * @brief Current state of a penalty in its lifecycle
 *
 * Penalties progress through these states from issue to resolution.
 */
UENUM(BlueprintType)
enum class EMGPenaltyState : uint8
{
	Pending,    ///< Penalty issued but not yet announced to driver
	Announced,  ///< Driver has been notified of penalty
	Active,     ///< Penalty window is open, must be served
	Served,     ///< Penalty has been completed/applied
	Cancelled,  ///< Penalty was cancelled (e.g., by race control)
	Appealed    ///< Penalty is under appeal review
};

/**
 * @brief Severity levels for track limits violations
 *
 * Determines how the system responds to track limit breaches.
 * More severe violations may result in immediate penalties rather
 * than accumulating warnings.
 */
UENUM(BlueprintType)
enum class EMGTrackLimitsSeverity : uint8
{
	Minor,      ///< Slight breach, no advantage gained - counts toward warning limit
	Moderate,   ///< Clear breach with potential minor advantage
	Severe,     ///< Significant breach with clear advantage gained
	Deliberate  ///< Intentional corner cutting - immediate penalty
};

//=============================================================================
// Penalty Data Structures
//=============================================================================

/**
 * @brief Complete penalty record containing all details of an issued penalty
 *
 * This struct represents a single penalty issued to a driver, tracking
 * everything from the violation that caused it to when/if it was served.
 * Used for both active penalty management and historical records.
 */
USTRUCT(BlueprintType)
struct FMGPenalty
{
	GENERATED_BODY()

	/// Unique identifier for this penalty instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PenaltyID;

	/// ID of the vehicle/driver receiving the penalty
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Type of penalty issued (time, drive-through, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPenaltyType Type = EMGPenaltyType::None;

	/// What rule violation triggered this penalty
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGViolationType Violation = EMGViolationType::None;

	/// Current state in the penalty lifecycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPenaltyState State = EMGPenaltyState::Pending;

	/// Time value in seconds (for TimeAdded, StopAndGo duration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeValue = 0.0f;

	/// Number of positions to drop (for PositionPenalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PositionValue = 0;

	/// Grid positions for next race (for GridPenalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridValue = 0;

	/// Championship or license points affected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsValue = 0;

	/// Fine amount in game currency (for FinePenalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FineValue = 0.0f;

	/// Lap number when penalty was issued
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapIssued = 0;

	/// Specific lap to serve penalty (0 = any lap in window)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapToServe = 0;

	/// Number of laps allowed to serve penalty before escalation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapsToServe = 3;

	/// Game time when penalty was issued
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IssuedTime = 0.0f;

	/// Game time when penalty was served (0 if not served)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ServedTime = 0.0f;

	/// World location where the incident occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector IncidentLocation = FVector::ZeroVector;

	/// Human-readable description of the penalty reason
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// ID of other vehicle involved (for collisions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OtherVehicleInvolved;

	/// Whether penalty applies automatically at race end
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoApplied = true;

	/// Whether this penalty can be appealed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAppealable = true;
};

/**
 * @brief Record of a single track limits violation
 *
 * Tracks each instance where a driver exceeded track limits,
 * used for warning accumulation and lap time deletion decisions.
 */
USTRUCT(BlueprintType)
struct FMGTrackLimitsViolation
{
	GENERATED_BODY()

	/// ID of the offending vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Corner/turn number where violation occurred (track-specific)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CornerNumber = 0;

	/// Lap number when violation occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	/// How severe the violation was
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrackLimitsSeverity Severity = EMGTrackLimitsSeverity::Minor;

	/// Estimated time gained from the violation (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeGained = 0.0f;

	/// World position where the violation occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ViolationPosition = FVector::ZeroVector;

	/// Game time when violation was detected
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	/// Whether this violation caused the lap time to be deleted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLapTimeDeleted = false;
};

/**
 * @brief Data captured from a collision between two vehicles
 *
 * Contains all physics and contextual data needed to determine fault
 * and appropriate penalty response for vehicle-to-vehicle contact.
 */
USTRUCT(BlueprintType)
struct FMGCollisionData
{
	GENERATED_BODY()

	/// First vehicle involved in collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Vehicle1ID;

	/// Second vehicle involved in collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Vehicle2ID;

	/// World location of impact point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactLocation = FVector::ZeroVector;

	/// Direction and magnitude of impact force
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactForce = FVector::ZeroVector;

	/// Relative speed between vehicles at impact (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	/// How much the vehicles were overlapping (0-1, for side-by-side racing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlapPercentage = 0.0f;

	/// Corner where collision occurred (0 = straight)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CornerNumber = 0;

	/// Lap number when collision occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	/// Vehicle determined to be at fault (may be None for racing incident)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AtFaultDriver;

	/// True if collision was deemed a racing incident (no penalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRacingIncident = false;

	/// Game time when collision occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;
};

//=============================================================================
// Configuration Structures
//=============================================================================

/**
 * @brief Configuration settings for penalty rule enforcement
 *
 * Defines thresholds, limits, and behaviors for the penalty system.
 * Can be adjusted per race series or game mode.
 */
USTRUCT(BlueprintType)
struct FMGPenaltyRules
{
	GENERATED_BODY()

	/// Master toggle for track limits enforcement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceTrackLimits = true;

	/// Number of track limit warnings before penalty (per driver)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLimitsWarnings = 3;

	/// Time penalty (seconds) after exceeding warning limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackLimitsTimeAdded = 5.0f;

	/// Automatically delete lap times with track limit violations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDeleteLapTimes = true;

	/// Penalize deliberate corner cutting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceCornerCutting = true;

	/// Monitor and penalize at-fault collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceCollisions = true;

	/// Minimum relative speed (km/h) for collision to be penalizable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionSpeedThreshold = 30.0f;

	/// Penalize movement before race start signal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceFalseStart = true;

	/// Movement threshold (seconds early) for false start
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FalseStartThreshold = 0.1f;

	/// Speed limit in pit lane (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitSpeedLimit = 60.0f;

	/// Time penalty for pit lane speeding (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitSpeedPenalty = 5.0f;

	/// Enforce blue flags for lapped traffic
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceBlueFlags = true;

	/// Number of ignored blue flags before penalty
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BlueFlagIgnoreLimit = 3;

	/// Allow drivers to appeal penalties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowAppeals = true;

	/// Default laps to serve drive-through/stop-go penalties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PenaltyServeLaps = 3;
};

/**
 * @brief Aggregated incident statistics for a single driver
 *
 * Tracks all incidents, warnings, and penalties for a driver across
 * a session. Used for progressive penalty escalation and safety ratings.
 */
USTRUCT(BlueprintType)
struct FMGDriverIncidents
{
	GENERATED_BODY()

	/// ID of the driver/vehicle these stats belong to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Total track limits violations this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLimitsViolations = 0;

	/// Active track limits warnings (resets after penalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLimitsWarnings = 0;

	/// Collisions where this driver was at fault
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollisionsAtFault = 0;

	/// Collisions where this driver was the victim
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollisionsVictim = 0;

	/// Collisions deemed racing incidents (no fault)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacingIncidents = 0;

	/// Total official warnings received
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Warnings = 0;

	/// Total penalties issued
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Penalties = 0;

	/// Cumulative time penalties (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimePenalties = 0.0f;

	/// License points accumulated this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LicensePoints = 0;

	/// Complete history of penalties for this driver
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPenalty> PenaltyHistory;

	/// Complete history of track limits violations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTrackLimitsViolation> TrackLimitsHistory;

	/// Complete history of collisions involving this driver
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCollisionData> CollisionHistory;
};

/**
 * @brief User preferences for penalty system behavior
 *
 * Controls how penalties are displayed and applied. Some settings
 * may only be available in certain game modes (e.g., casual vs competitive).
 */
USTRUCT(BlueprintType)
struct FMGPenaltySettings
{
	GENERATED_BODY()

	/// Master toggle for entire penalty system
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnablePenalties = true;

	/// Use more aggressive/stricter rule enforcement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStrictRules = false;

	/// Show on-screen penalty notifications
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowPenaltyNotifications = true;

	/// Show warning notifications (pre-penalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowWarnings = true;

	/// Automatically serve penalties when possible (vs manual pit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoServePenalties = false;

	/// Duration to ghost vehicles after contact (0 = disabled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GhostingOnContact = 0.0f;

	/// Multiplier for slowdown-style penalties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowdownPenaltyMultiplier = 1.0f;
};

//=============================================================================
// Delegate Declarations
//=============================================================================

/// Broadcast when a new penalty is issued to a driver
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPenaltyIssued, FName, VehicleID, const FMGPenalty&, Penalty);

/// Broadcast when a driver successfully serves a penalty
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPenaltyServed, FName, VehicleID, const FMGPenalty&, Penalty);

/// Broadcast when a penalty is cancelled by race control
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPenaltyCancelled, FName, VehicleID, FGuid, PenaltyID);

/// Broadcast when a warning (not penalty) is issued
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWarningIssued, FName, VehicleID, EMGViolationType, Violation);

/// Broadcast each time track limits are exceeded
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrackLimitsViolation, FName, VehicleID, const FMGTrackLimitsViolation&, Violation);

/// Broadcast when a collision is detected between vehicles
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollisionDetected, const FMGCollisionData&, Collision);

/// Broadcast when a driver is disqualified from the race
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisqualification, FName, VehicleID);

/// Broadcast when a lap time is deleted due to violation
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapTimeDeleted, FName, VehicleID, int32, LapNumber);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Game Instance Subsystem for managing race penalties and rule enforcement
 *
 * UMGPenaltySubsystem is the central authority for all penalty-related operations
 * in Midnight Grind. It monitors driver behavior, issues penalties for violations,
 * tracks incident history, and provides queries for penalty state.
 *
 * The subsystem operates passively - it must be informed of incidents by other
 * systems (physics, race director, etc.) and responds by issuing appropriate
 * penalties based on configured rules.
 *
 * Usage Example:
 * @code
 * // Get the penalty subsystem
 * UMGPenaltySubsystem* PenaltySys = GetGameInstance()->GetSubsystem<UMGPenaltySubsystem>();
 *
 * // Report a track limits violation
 * PenaltySys->ReportTrackLimitsViolation(VehicleID, 5, CurrentLap, VehicleLocation);
 *
 * // Check if driver has pending penalties
 * if (PenaltySys->HasPendingPenalty(VehicleID))
 * {
 *     // Show penalty notification
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPenaltySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Subsystem Lifecycle
	//-------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//-------------------------------------------------------------------------
	// Penalty Issue Functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Issues a penalty to a driver for a specific violation
	 * @param VehicleID The vehicle/driver receiving the penalty
	 * @param Type The type of penalty to issue
	 * @param Violation The rule violation that caused this penalty
	 * @param TimeValue Time value for time-based penalties (seconds)
	 * @return The created penalty record
	 */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	FMGPenalty IssuePenalty(FName VehicleID, EMGPenaltyType Type, EMGViolationType Violation, float TimeValue = 0.0f);

	/** Issues an official warning without time/position penalty */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueWarning(FName VehicleID, EMGViolationType Violation);

	/** Issues a drive-through penalty (pass through pit lane) */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueDriveThrough(FName VehicleID, EMGViolationType Violation);

	/** Issues a stop-and-go penalty with specified stop duration */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueStopAndGo(FName VehicleID, EMGViolationType Violation, float Duration = 10.0f);

	/** Issues a time penalty to be added to race result */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueTimePenalty(FName VehicleID, EMGViolationType Violation, float Seconds);

	/** Issues a position drop penalty during the race */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssuePositionPenalty(FName VehicleID, EMGViolationType Violation, int32 Positions);

	/** Disqualifies driver from current race */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueDisqualification(FName VehicleID, EMGViolationType Violation);

	//-------------------------------------------------------------------------
	// Penalty Service Functions
	//-------------------------------------------------------------------------

	/** Marks a penalty as served (called when driver completes penalty) */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Serve")
	void ServePenalty(FName VehicleID, FGuid PenaltyID);

	/** Cancels a penalty (race control decision) */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Serve")
	void CancelPenalty(FName VehicleID, FGuid PenaltyID);

	/** Submits an appeal for a penalty */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Serve")
	void AppealPenalty(FName VehicleID, FGuid PenaltyID);

	//-------------------------------------------------------------------------
	// Penalty Query Functions
	//-------------------------------------------------------------------------

	/** Returns all unserved penalties for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	TArray<FMGPenalty> GetPendingPenalties(FName VehicleID) const;

	/** Returns complete penalty history for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	TArray<FMGPenalty> GetAllPenalties(FName VehicleID) const;

	/** Checks if driver has any unserved penalties */
	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	bool HasPendingPenalty(FName VehicleID) const;

	/** Gets the most severe pending penalty for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	FMGPenalty GetMostSeverePenalty(FName VehicleID) const;

	//-------------------------------------------------------------------------
	// Track Limits Functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Reports a track limits violation
	 * @param VehicleID The offending vehicle
	 * @param CornerNumber Track corner number (for statistics)
	 * @param LapNumber Current lap number
	 * @param Position World position of violation
	 */
	UFUNCTION(BlueprintCallable, Category = "Penalty|TrackLimits")
	void ReportTrackLimitsViolation(FName VehicleID, int32 CornerNumber, int32 LapNumber, const FVector& Position);

	/** Sets severity and time gained for most recent track limits violation */
	UFUNCTION(BlueprintCallable, Category = "Penalty|TrackLimits")
	void SetTrackLimitsSeverity(FName VehicleID, EMGTrackLimitsSeverity Severity, float TimeGained);

	/** Gets total track limits violations for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|TrackLimits")
	int32 GetTrackLimitsCount(FName VehicleID) const;

	/** Gets remaining warnings before track limits penalty */
	UFUNCTION(BlueprintPure, Category = "Penalty|TrackLimits")
	int32 GetTrackLimitsWarningsRemaining(FName VehicleID) const;

	/** Manually deletes a lap time due to violation */
	UFUNCTION(BlueprintCallable, Category = "Penalty|TrackLimits")
	void DeleteLapTime(FName VehicleID, int32 LapNumber);

	//-------------------------------------------------------------------------
	// Collision Functions
	//-------------------------------------------------------------------------

	/** Reports a collision between vehicles for analysis */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Collision")
	void ReportCollision(const FMGCollisionData& Collision);

	/** Analyzes collision data to determine fault */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Collision")
	void DetermineFault(FMGCollisionData& Collision);

	/** Returns all collisions from current session */
	UFUNCTION(BlueprintPure, Category = "Penalty|Collision")
	TArray<FMGCollisionData> GetCollisionHistory() const;

	/** Gets number of at-fault collisions for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Collision")
	int32 GetCollisionsAtFault(FName VehicleID) const;

	//-------------------------------------------------------------------------
	// Race Event Functions
	//-------------------------------------------------------------------------

	/** Checks for false start based on reaction time */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void CheckFalseStart(FName VehicleID, float ReactionTime);

	/** Checks if vehicle exceeded pit lane speed limit */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void CheckPitSpeeding(FName VehicleID, float Speed);

	/** Records a blue flag ignore incident */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void CheckBlueFlagIgnore(FName VehicleID);

	/** Clears blue flag count (e.g., when leader unlaps) */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void ClearBlueFlagCount(FName VehicleID);

	//-------------------------------------------------------------------------
	// Driver Incidents Functions
	//-------------------------------------------------------------------------

	/** Gets complete incident record for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Incidents")
	FMGDriverIncidents GetDriverIncidents(FName VehicleID) const;

	/** Resets all incidents for a specific driver */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Incidents")
	void ResetDriverIncidents(FName VehicleID);

	/** Resets incidents for all drivers (new session) */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Incidents")
	void ResetAllIncidents();

	/** Gets cumulative time penalties for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Incidents")
	float GetTotalTimePenalties(FName VehicleID) const;

	/** Gets total warning count for a driver */
	UFUNCTION(BlueprintPure, Category = "Penalty|Incidents")
	int32 GetTotalWarnings(FName VehicleID) const;

	//-------------------------------------------------------------------------
	// Rules Configuration
	//-------------------------------------------------------------------------

	/** Sets the penalty rules configuration */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Rules")
	void SetPenaltyRules(const FMGPenaltyRules& NewRules);

	/** Gets current penalty rules configuration */
	UFUNCTION(BlueprintPure, Category = "Penalty|Rules")
	FMGPenaltyRules GetPenaltyRules() const { return Rules; }

	//-------------------------------------------------------------------------
	// Settings Configuration
	//-------------------------------------------------------------------------

	/** Sets user penalty preferences */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Settings")
	void SetPenaltySettings(const FMGPenaltySettings& NewSettings);

	/** Gets current user penalty preferences */
	UFUNCTION(BlueprintPure, Category = "Penalty|Settings")
	FMGPenaltySettings GetPenaltySettings() const { return Settings; }

	//-------------------------------------------------------------------------
	// Lap Event Callbacks
	//-------------------------------------------------------------------------

	/** Called when a driver completes a lap - processes pending penalties */
	UFUNCTION(BlueprintCallable, Category = "Penalty|Lap")
	void OnLapCompleted(FName VehicleID, int32 LapNumber);

	//-------------------------------------------------------------------------
	// Event Delegates
	//-------------------------------------------------------------------------

	/// Fires when a new penalty is issued
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnPenaltyIssued OnPenaltyIssued;

	/// Fires when a penalty is served
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnPenaltyServed OnPenaltyServed;

	/// Fires when a penalty is cancelled
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnPenaltyCancelled OnPenaltyCancelled;

	/// Fires when a warning is issued
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnWarningIssued OnWarningIssued;

	/// Fires on track limits violation
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnTrackLimitsViolation OnTrackLimitsViolation;

	/// Fires when collision is detected
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnCollisionDetected OnCollisionDetected;

	/// Fires when driver is disqualified
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnDisqualification OnDisqualification;

	/// Fires when lap time is deleted
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnLapTimeDeleted OnLapTimeDeleted;

protected:
	//-------------------------------------------------------------------------
	// Internal Implementation
	//-------------------------------------------------------------------------

	/** Periodic update for penalty state management */
	void OnPenaltyTick();

	/** Checks for penalties that weren't served in time */
	void CheckUnservedPenalties();

	/** Processes track limits violation accumulation */
	void ProcessTrackLimits(FName VehicleID);

	/** Gets or creates incident record for a driver */
	FMGDriverIncidents& GetOrCreateIncidents(FName VehicleID);

	//-------------------------------------------------------------------------
	// Internal State
	//-------------------------------------------------------------------------

	/// Incident records per driver
	UPROPERTY()
	TMap<FName, FMGDriverIncidents> DriverIncidents;

	/// Blue flag ignore counts per driver
	UPROPERTY()
	TMap<FName, int32> BlueFlagCounts;

	/// All collisions this session
	UPROPERTY()
	TArray<FMGCollisionData> AllCollisions;

	/// Current penalty rule configuration
	UPROPERTY()
	FMGPenaltyRules Rules;

	/// Current user penalty settings
	UPROPERTY()
	FMGPenaltySettings Settings;

	/// Current race lap (for penalty timing)
	UPROPERTY()
	int32 CurrentLap = 0;

	/// Timer handle for penalty tick updates
	FTimerHandle PenaltyTickHandle;
};
