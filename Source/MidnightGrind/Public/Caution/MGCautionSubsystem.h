// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGCautionSubsystem.h
 * @brief Race Caution and Safety Car Management Subsystem
 *
 * This subsystem manages all caution periods, safety car procedures, flag displays,
 * and race neutralization for Midnight Grind. It implements realistic motorsport
 * caution protocols including local yellows, full-course cautions, virtual safety
 * car (VSC), physical safety car, and red flag procedures.
 *
 * Key responsibilities:
 * - Deploying and managing caution periods in response to incidents
 * - Controlling safety car behavior and pace
 * - Managing Virtual Safety Car delta time enforcement
 * - Displaying appropriate flags to drivers
 * - Handling race restarts after caution periods
 * - Tracking caution statistics for race analysis
 *
 * Caution Hierarchy (from least to most severe):
 * 1. Local Yellow - Single marshal sector, no passing in that zone
 * 2. Virtual Safety Car (VSC) - Reduced speed, maintain delta to reference time
 * 3. Full Course Yellow / Safety Car - Field bunched behind safety car
 * 4. Red Flag - Race stopped, all vehicles return to pit lane
 *
 * Integration Points:
 * - Works with MGPenaltySubsystem to penalize caution violations
 * - Coordinates with race timing for delta calculations
 * - Feeds data to UI for flag and caution displays
 *
 * @see UMGPenaltySubsystem
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCautionSubsystem.generated.h"

//=============================================================================
// Caution Type Enumerations
//=============================================================================

/**
 * @brief Types of caution periods that can be deployed
 *
 * These represent different levels of race neutralization, each with
 * specific rules for driver behavior and race control procedures.
 */
UENUM(BlueprintType)
enum class EMGCautionType : uint8
{
	None,              ///< No caution active - green flag racing
	LocalYellow,       ///< Single-zone yellow flag, no passing in that zone only
	FullCourseYellow,  ///< All-sector yellow flags, no passing anywhere
	SafetyCar,         ///< Physical safety car deployed, field follows at reduced pace
	VirtualSafetyCar,  ///< VSC mode - drivers must maintain delta to reference time
	RedFlag,           ///< Race stopped - all vehicles must stop or return to pits
	Code60             ///< Speed limited to 60 km/h (endurance racing style)
};

/**
 * @brief Reasons why a caution period may be deployed
 *
 * Used for caution period records and helps determine appropriate
 * caution type and duration.
 */
UENUM(BlueprintType)
enum class EMGCautionReason : uint8
{
	None,              ///< No reason specified
	Accident,          ///< Vehicle crash or spin requiring assistance
	Debris,            ///< Track debris from vehicle damage or parts failure
	VehicleStopped,    ///< Stranded vehicle in dangerous location
	MedicalEmergency,  ///< Medical situation requiring track access
	WeatherConditions, ///< Rain, fog, or other unsafe weather
	TrackInvasion,     ///< Unauthorized person or animal on track
	OilOnTrack,        ///< Fluid spill creating hazardous conditions
	UnsafeConditions,  ///< General unsafe track conditions
	RaceControl,       ///< Race control decision (no specific incident)
	Steward            ///< Steward-ordered caution
};

/**
 * @brief Current state within a caution period lifecycle
 *
 * Cautions progress through these states from deployment to restart.
 */
UENUM(BlueprintType)
enum class EMGCautionState : uint8
{
	Clear,             ///< No caution - normal racing conditions
	CautionDeployed,   ///< Caution just deployed, vehicles slowing
	CatchingUp,        ///< Field catching up to safety car
	Bunched,           ///< Field bunched together behind leader/SC
	RestartPending,    ///< Preparing for race restart
	GreenFlagPending   ///< Green flag imminent, final restart preparations
};

/**
 * @brief Types of flags that can be displayed to drivers
 *
 * Standard motorsport flag signals with their meanings.
 */
UENUM(BlueprintType)
enum class EMGFlagType : uint8
{
	None,            ///< No flag displayed
	Green,           ///< Track clear, racing conditions
	Yellow,          ///< Caution, no passing, danger ahead
	DoubleYellow,    ///< Severe hazard, significant speed reduction required
	Blue,            ///< Faster car approaching, allow to pass
	White,           ///< Slow vehicle on track
	Red,             ///< Session stopped, return to pit lane
	Black,           ///< Disqualified, must pit immediately
	BlackOrange,     ///< Mechanical issue, must pit for inspection (meatball flag)
	BlackWhite,      ///< Warning for unsportsmanlike conduct
	Checkered,       ///< Session/race complete
	SafetyCarBoard   ///< Safety car deployed indicator
};

/**
 * @brief Phases of safety car deployment procedure
 *
 * Tracks the safety car's progress from deployment to withdrawal.
 */
UENUM(BlueprintType)
enum class EMGSafetyCarPhase : uint8
{
	NotDeployed,      ///< Safety car in pit lane, not active
	Deploying,        ///< Safety car leaving pit lane
	PickingUpLeader,  ///< Safety car finding and catching the race leader
	Leading,          ///< Safety car at front of field, leading pack
	InLap,            ///< Safety car preparing to enter pit lane
	PitEntry          ///< Safety car entering pit lane, restart imminent
};

//=============================================================================
// Caution Data Structures
//=============================================================================

/**
 * @brief Defines a track zone for localized caution management
 *
 * Tracks are divided into zones for local yellow flag deployment.
 * Each zone can have independent flag status.
 */
USTRUCT(BlueprintType)
struct FMGCautionZone
{
	GENERATED_BODY()

	/// Index of this zone (0 to NumZones-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ZoneIndex = 0;

	/// Track distance where this zone begins (meters from start/finish)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistance = 0.0f;

	/// Track distance where this zone ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndDistance = 0.0f;

	/// Currently displayed flag in this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFlagType ActiveFlag = EMGFlagType::None;

	/// Reason for caution in this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionReason Reason = EMGCautionReason::None;

	/// World location of incident causing caution
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector IncidentLocation = FVector::ZeroVector;

	/// Speed limit enforced in this zone (0 = no limit beyond general caution)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 0.0f;

	/// Whether passing is prohibited in this zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNoOvertaking = true;

	/// When this zone's caution was activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ActivatedTime;
};

/**
 * @brief Complete record of a caution period
 *
 * Stores all details about a caution from deployment to conclusion,
 * used for both active management and historical records.
 */
USTRUCT(BlueprintType)
struct FMGCautionPeriod
{
	GENERATED_BODY()

	/// Sequential caution number this session (1st caution = 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CautionNumber = 0;

	/// Type of caution deployed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionType Type = EMGCautionType::None;

	/// What triggered this caution
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionReason Reason = EMGCautionReason::None;

	/// Current state in caution lifecycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionState State = EMGCautionState::Clear;

	/// Lap number when caution was deployed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartLap = 0;

	/// Lap number when caution ended (0 if still active)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndLap = 0;

	/// Game time when caution was deployed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	/// Game time when caution ended
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	/// Total duration of caution period (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	/// Number of laps completed under caution
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapsUnderCaution = 0;

	/// Vehicles involved in the incident (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> InvolvedVehicles;

	/// World location of the incident
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector IncidentLocation = FVector::ZeroVector;

	/// Human-readable description of caution reason
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Whether pit lane is open during this caution
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPitLaneOpen = true;

	/// Whether lapped cars can un-lap themselves
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLappedCarsCanUnlap = true;
};

/**
 * @brief State data for the physical safety car
 *
 * Tracks safety car position, speed, and behavior during deployment.
 */
USTRUCT(BlueprintType)
struct FMGSafetyCarState
{
	GENERATED_BODY()

	/// Whether safety car is currently deployed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDeployed = false;

	/// Current phase in safety car procedure
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSafetyCarPhase Phase = EMGSafetyCarPhase::NotDeployed;

	/// Current world position of safety car
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentPosition = FVector::ZeroVector;

	/// Current speed of safety car (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// Target cruising speed for safety car (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetSpeed = 80.0f;

	/// Vehicle ID of current race leader
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LeaderVehicle;

	/// Gap between safety car and leader (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToLeader = 0.0f;

	/// Number of laps safety car has led the field
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapsLed = 0;

	/// Whether safety car lights are on (flashing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLightsOn = true;

	/// Whether conditions are clear for SC to withdraw
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReadyToWithdraw = false;

	/// Distance along track (meters from start/finish)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongTrack = 0.0f;
};

/**
 * @brief State data for Virtual Safety Car mode
 *
 * VSC requires drivers to maintain a delta time to a reference lap.
 * This tracks compliance and delta values for all vehicles.
 */
USTRUCT(BlueprintType)
struct FMGVirtualSafetyCarState
{
	GENERATED_BODY()

	/// Whether VSC is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive = false;

	/// Target delta time all drivers should maintain (seconds behind reference)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetDelta = 0.0f;

	/// Maximum speed during VSC (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 0.0f;

	/// Current delta for each vehicle (positive = slower than reference)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> VehicleDeltas;

	/// Whether each vehicle is complying with VSC requirements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, bool> VehicleCompliance;

	/// Minimum allowed delta (going faster than reference triggers penalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDelta = -0.5f;

	/// Maximum allowed delta (too slow affects competitors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDelta = 1.0f;

	/// Whether VSC is about to end
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEndingSoon = false;
};

/**
 * @brief Configuration for race restart procedure
 *
 * Defines how the race will restart after a caution period ends.
 */
USTRUCT(BlueprintType)
struct FMGRestartProcedure
{
	GENERATED_BODY()

	/// Whether restart uses double-file formation (NASCAR style)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDoubleFileRestart = false;

	/// Whether restart is rolling (true) or standing (false)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRollingStart = true;

	/// Track distance where restart zone begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartZoneStart = 0.0f;

	/// Track distance where restart zone ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartZoneEnd = 0.0f;

	/// Vehicle ID designated as restart leader
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RestartLeader;

	/// Whether the leader controls restart timing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeaderControlsRestart = true;

	/// Minimum speed for rolling restart (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRestartSpeed = 60.0f;

	/// Maximum speed for rolling restart (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRestartSpeed = 100.0f;

	/// Laps remaining under caution before restart
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WarningLapsRemaining = 1;

	/// Whether conditions are ready for green flag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGreenFlagReady = false;
};

//=============================================================================
// Configuration Structures
//=============================================================================

/**
 * @brief Configuration settings for caution system behavior
 *
 * Controls thresholds, durations, and automatic deployment rules.
 */
USTRUCT(BlueprintType)
struct FMGCautionSettings
{
	GENERATED_BODY()

	/// Master toggle for caution system
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableCautions = true;

	/// Automatically deploy safety car for major incidents
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDeploySafetyCar = true;

	/// Automatically deploy VSC for minor incidents
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDeployVSC = true;

	/// Target speed for safety car (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SafetyCarSpeed = 80.0f;

	/// Speed limit during VSC (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VSCSpeedLimit = 60.0f;

	/// Minimum laps under safety car before restart
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLapsUnderSC = 2;

	/// Maximum laps under safety car before red flag consideration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLapsUnderSC = 5;

	/// Allow lapped cars to unlap during caution
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowLappedCarsToUnlap = true;

	/// Close pit lane during red flag
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClosePitOnRedFlag = true;

	/// Time required to clear debris (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DebrisCleanupTime = 30.0f;

	/// Time before safety response arrives at incident (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccidentResponseTime = 10.0f;

	/// Use double-file restarts (NASCAR style)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDoubleFileRestarts = false;

	/// Distance before restart zone to show warning (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartWarningDistance = 500.0f;
};

/**
 * @brief Aggregate statistics for caution periods this session
 */
USTRUCT(BlueprintType)
struct FMGCautionStats
{
	GENERATED_BODY()

	/// Total number of caution periods this session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCautions = 0;

	/// Number of full safety car deployments
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SafetyCarPeriods = 0;

	/// Number of VSC deployments
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VSCPeriods = 0;

	/// Number of red flags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RedFlags = 0;

	/// Total laps completed under any caution
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLapsUnderCaution = 0;

	/// Total time spent under caution (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeUnderCaution = 0.0f;

	/// Complete history of all caution periods
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCautionPeriod> CautionHistory;
};

//=============================================================================
// Delegate Declarations
//=============================================================================

/// Broadcast when a caution period begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCautionDeployed, EMGCautionType, Type, EMGCautionReason, Reason);

/// Broadcast when a caution period ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCautionEnded, EMGCautionType, Type);

/// Broadcast when caution state transitions
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCautionStateChanged, EMGCautionState, OldState, EMGCautionState, NewState);

/// Broadcast when physical safety car is deployed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSafetyCarDeployed, const FMGSafetyCarState&, State);

/// Broadcast when safety car enters pit lane
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSafetyCarIn);

/// Broadcast when a flag is shown to a specific driver
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFlagDisplayed, FName, VehicleID, EMGFlagType, Flag);

/// Broadcast when race goes green (caution ends)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGreenFlag);

/// Broadcast when red flag is deployed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedFlag);

/// Broadcast as restart approaches (laps remaining)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRestartWarning, int32, LapsRemaining, bool, bFinalWarning);

/// Broadcast when a driver violates VSC delta requirements
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVSCDeltaViolation, FName, VehicleID, float, Delta);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Game Instance Subsystem for managing race cautions and safety procedures
 *
 * UMGCautionSubsystem is responsible for all race neutralization mechanics in
 * Midnight Grind. It handles everything from local yellow flags through full
 * race stoppages, implementing realistic motorsport caution procedures.
 *
 * The subsystem responds to incidents reported by other systems and deploys
 * appropriate caution measures. It manages the complete lifecycle of caution
 * periods including deployment, safety car behavior, restart procedures, and
 * statistical tracking.
 *
 * Typical Caution Flow:
 * 1. Incident detected (collision, debris, stopped vehicle)
 * 2. Appropriate caution type deployed based on severity
 * 3. Vehicles slow/bunch under caution rules
 * 4. Incident cleared by safety crews
 * 5. Restart procedure initiated
 * 6. Green flag racing resumes
 *
 * Usage Example:
 * @code
 * UMGCautionSubsystem* CautionSys = GetGameInstance()->GetSubsystem<UMGCautionSubsystem>();
 *
 * // Report an incident
 * CautionSys->ReportIncident(VehicleID, IncidentLocation, EMGCautionReason::Accident);
 *
 * // Check if we're under caution
 * if (CautionSys->IsCautionActive())
 * {
 *     // Enforce caution rules
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCautionSubsystem : public UGameInstanceSubsystem
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
	// Caution Control Functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Deploys a caution period
	 * @param Type Type of caution to deploy
	 * @param Reason What triggered the caution
	 * @param IncidentLocation World position of incident
	 */
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void DeployCaution(EMGCautionType Type, EMGCautionReason Reason, const FVector& IncidentLocation);

	/** Ends the current caution period and initiates restart */
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void EndCaution();

	/** Upgrades current caution to more severe type */
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void EscalateCaution(EMGCautionType NewType);

	/** Deploys a local yellow flag in a specific zone */
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void DeployLocalYellow(int32 ZoneIndex, EMGCautionReason Reason, const FVector& Location);

	/** Clears a local yellow in a specific zone */
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void ClearLocalYellow(int32 ZoneIndex);

	/** Clears all local yellow flags */
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void ClearAllLocalYellows();

	/** Returns whether any caution is currently active */
	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	bool IsCautionActive() const;

	/** Returns the type of currently active caution */
	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	EMGCautionType GetActiveCautionType() const;

	/** Returns the current caution period details */
	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	FMGCautionPeriod GetCurrentCaution() const { return CurrentCaution; }

	/** Returns current state within the caution lifecycle */
	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	EMGCautionState GetCautionState() const { return CurrentState; }

	//-------------------------------------------------------------------------
	// Safety Car Functions
	//-------------------------------------------------------------------------

	/** Deploys the physical safety car */
	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void DeploySafetyCar(EMGCautionReason Reason);

	/** Signals safety car to return to pit lane */
	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void BringSafetyCarIn();

	/** Updates safety car position and speed (called by SC vehicle) */
	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void UpdateSafetyCarPosition(const FVector& Position, float Speed);

	/** Returns current safety car state */
	UFUNCTION(BlueprintPure, Category = "Caution|SafetyCar")
	FMGSafetyCarState GetSafetyCarState() const { return SafetyCarState; }

	/** Returns whether safety car is currently deployed */
	UFUNCTION(BlueprintPure, Category = "Caution|SafetyCar")
	bool IsSafetyCarDeployed() const { return SafetyCarState.bDeployed; }

	/** Returns current safety car speed */
	UFUNCTION(BlueprintPure, Category = "Caution|SafetyCar")
	float GetSafetyCarSpeed() const { return SafetyCarState.CurrentSpeed; }

	/** Manually sets safety car phase */
	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void SetSafetyCarPhase(EMGSafetyCarPhase Phase);

	//-------------------------------------------------------------------------
	// Virtual Safety Car Functions
	//-------------------------------------------------------------------------

	/** Deploys Virtual Safety Car mode */
	UFUNCTION(BlueprintCallable, Category = "Caution|VSC")
	void DeployVirtualSafetyCar();

	/** Ends Virtual Safety Car mode */
	UFUNCTION(BlueprintCallable, Category = "Caution|VSC")
	void EndVirtualSafetyCar();

	/**
	 * @brief Updates a vehicle's delta to VSC reference time
	 * @param VehicleID Vehicle being updated
	 * @param Delta Current delta (positive = slower than reference)
	 */
	UFUNCTION(BlueprintCallable, Category = "Caution|VSC")
	void UpdateVSCDelta(FName VehicleID, float Delta);

	/** Returns current VSC state */
	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	FMGVirtualSafetyCarState GetVSCState() const { return VSCState; }

	/** Returns whether VSC is currently active */
	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	bool IsVSCActive() const { return VSCState.bActive; }

	/** Returns a vehicle's current VSC delta */
	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	float GetVSCDelta(FName VehicleID) const;

	/** Returns whether a vehicle is complying with VSC rules */
	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	bool IsVehicleVSCCompliant(FName VehicleID) const;

	//-------------------------------------------------------------------------
	// Red Flag Functions
	//-------------------------------------------------------------------------

	/** Deploys red flag (stops the race) */
	UFUNCTION(BlueprintCallable, Category = "Caution|RedFlag")
	void DeployRedFlag(EMGCautionReason Reason);

	/** Initiates restart procedure after red flag */
	UFUNCTION(BlueprintCallable, Category = "Caution|RedFlag")
	void RestartFromRedFlag();

	/** Returns whether red flag is currently active */
	UFUNCTION(BlueprintPure, Category = "Caution|RedFlag")
	bool IsRedFlagActive() const;

	//-------------------------------------------------------------------------
	// Flag Management Functions
	//-------------------------------------------------------------------------

	/** Shows a specific flag to a driver */
	UFUNCTION(BlueprintCallable, Category = "Caution|Flags")
	void ShowFlag(FName VehicleID, EMGFlagType Flag);

	/** Clears flag display for a driver */
	UFUNCTION(BlueprintCallable, Category = "Caution|Flags")
	void ClearFlag(FName VehicleID);

	/** Gets the flag currently shown to a driver */
	UFUNCTION(BlueprintPure, Category = "Caution|Flags")
	EMGFlagType GetVehicleFlag(FName VehicleID) const;

	/** Gets all vehicles being shown a specific flag type */
	UFUNCTION(BlueprintPure, Category = "Caution|Flags")
	TArray<FName> GetVehiclesWithFlag(EMGFlagType Flag) const;

	//-------------------------------------------------------------------------
	// Zone Management Functions
	//-------------------------------------------------------------------------

	/** Configures track zones for local caution management */
	UFUNCTION(BlueprintCallable, Category = "Caution|Zones")
	void ConfigureZones(int32 NumZones, float TrackLength);

	/** Returns all configured caution zones */
	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	TArray<FMGCautionZone> GetCautionZones() const { return CautionZones; }

	/** Gets the zone at a given track distance */
	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	FMGCautionZone GetZoneAtDistance(float Distance) const;

	/** Returns whether a specific zone is under local caution */
	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	bool IsZoneUnderCaution(int32 ZoneIndex) const;

	/** Gets the speed limit for a specific zone */
	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	float GetZoneSpeedLimit(int32 ZoneIndex) const;

	//-------------------------------------------------------------------------
	// Restart Functions
	//-------------------------------------------------------------------------

	/** Initiates restart preparation (bunches field) */
	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void PrepareRestart();

	/** Triggers the race restart (green flag) */
	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void InitiateRestart();

	/** Sets which vehicle leads the restart */
	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void SetRestartLeader(FName VehicleID);

	/** Returns current restart procedure configuration */
	UFUNCTION(BlueprintPure, Category = "Caution|Restart")
	FMGRestartProcedure GetRestartProcedure() const { return RestartProcedure; }

	/** Returns whether a restart is pending */
	UFUNCTION(BlueprintPure, Category = "Caution|Restart")
	bool IsRestartPending() const;

	/** Aborts a pending restart (returns to caution) */
	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void AbortRestart();

	//-------------------------------------------------------------------------
	// Pit Lane Control Functions
	//-------------------------------------------------------------------------

	/** Opens or closes pit lane during caution */
	UFUNCTION(BlueprintCallable, Category = "Caution|Pit")
	void SetPitLaneOpen(bool bOpen);

	/** Returns whether pit lane is open */
	UFUNCTION(BlueprintPure, Category = "Caution|Pit")
	bool IsPitLaneOpen() const { return bPitLaneOpen; }

	/** Allows lapped cars to unlap themselves */
	UFUNCTION(BlueprintCallable, Category = "Caution|Pit")
	void AllowLappedCarsToUnlap();

	/** Returns list of currently lapped cars */
	UFUNCTION(BlueprintPure, Category = "Caution|Pit")
	TArray<FName> GetLappedCars() const { return LappedCars; }

	//-------------------------------------------------------------------------
	// Statistics Functions
	//-------------------------------------------------------------------------

	/** Returns aggregate caution statistics */
	UFUNCTION(BlueprintPure, Category = "Caution|Stats")
	FMGCautionStats GetCautionStats() const { return CautionStats; }

	/** Returns complete caution history */
	UFUNCTION(BlueprintPure, Category = "Caution|Stats")
	TArray<FMGCautionPeriod> GetCautionHistory() const { return CautionStats.CautionHistory; }

	/** Resets all caution statistics */
	UFUNCTION(BlueprintCallable, Category = "Caution|Stats")
	void ResetStats();

	//-------------------------------------------------------------------------
	// Settings Functions
	//-------------------------------------------------------------------------

	/** Sets caution system configuration */
	UFUNCTION(BlueprintCallable, Category = "Caution|Settings")
	void SetCautionSettings(const FMGCautionSettings& NewSettings);

	/** Returns current caution settings */
	UFUNCTION(BlueprintPure, Category = "Caution|Settings")
	FMGCautionSettings GetCautionSettings() const { return Settings; }

	//-------------------------------------------------------------------------
	// Event Reporting Functions
	//-------------------------------------------------------------------------

	/** Reports an incident for potential caution deployment */
	UFUNCTION(BlueprintCallable, Category = "Caution|Events")
	void ReportIncident(FName VehicleID, const FVector& Location, EMGCautionReason Reason);

	/** Reports debris on track */
	UFUNCTION(BlueprintCallable, Category = "Caution|Events")
	void ReportDebris(const FVector& Location);

	/** Reports a stopped vehicle */
	UFUNCTION(BlueprintCallable, Category = "Caution|Events")
	void ReportStoppedVehicle(FName VehicleID, const FVector& Location);

	//-------------------------------------------------------------------------
	// Event Delegates
	//-------------------------------------------------------------------------

	/// Fires when caution is deployed
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnCautionDeployed OnCautionDeployed;

	/// Fires when caution ends
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnCautionEnded OnCautionEnded;

	/// Fires when caution state changes
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnCautionStateChanged OnCautionStateChanged;

	/// Fires when safety car is deployed
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnSafetyCarDeployed OnSafetyCarDeployed;

	/// Fires when safety car enters pit
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnSafetyCarIn OnSafetyCarIn;

	/// Fires when flag is shown to driver
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnFlagDisplayed OnFlagDisplayed;

	/// Fires on green flag
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnGreenFlag OnGreenFlag;

	/// Fires on red flag
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnRedFlag OnRedFlag;

	/// Fires as restart approaches
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnRestartWarning OnRestartWarning;

	/// Fires when driver violates VSC delta
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnVSCDeltaViolation OnVSCDeltaViolation;

protected:
	//-------------------------------------------------------------------------
	// Internal Implementation
	//-------------------------------------------------------------------------

	/** Periodic update for caution state management */
	void OnCautionTick();

	/** Updates safety car AI behavior */
	void UpdateSafetyCar(float MGDeltaTime);

	/** Updates VSC delta tracking */
	void UpdateVSC(float MGDeltaTime);

	/** Updates restart procedure progress */
	void UpdateRestartProcedure(float MGDeltaTime);

	/** Transitions caution to new state */
	void SetCautionState(EMGCautionState NewState);

	/** Gets zone index for a given track distance */
	int32 GetZoneIndex(float Distance) const;

	/** Records completed caution period to history */
	void RecordCautionPeriod();

	//-------------------------------------------------------------------------
	// Internal State
	//-------------------------------------------------------------------------

	/// Current active caution period
	UPROPERTY()
	FMGCautionPeriod CurrentCaution;

	/// Current state in caution lifecycle
	UPROPERTY()
	EMGCautionState CurrentState = EMGCautionState::Clear;

	/// Physical safety car state
	UPROPERTY()
	FMGSafetyCarState SafetyCarState;

	/// Virtual safety car state
	UPROPERTY()
	FMGVirtualSafetyCarState VSCState;

	/// Current restart procedure configuration
	UPROPERTY()
	FMGRestartProcedure RestartProcedure;

	/// Track zones for local caution management
	UPROPERTY()
	TArray<FMGCautionZone> CautionZones;

	/// Current flag state per vehicle
	UPROPERTY()
	TMap<FName, EMGFlagType> VehicleFlags;

	/// Aggregate caution statistics
	UPROPERTY()
	FMGCautionStats CautionStats;

	/// System configuration settings
	UPROPERTY()
	FMGCautionSettings Settings;

	/// Whether pit lane is currently open
	UPROPERTY()
	bool bPitLaneOpen = true;

	/// List of lapped vehicles
	UPROPERTY()
	TArray<FName> LappedCars;

	/// Total track length for zone calculations
	UPROPERTY()
	float TrackLength = 5000.0f;

	/// Running count of cautions this session
	UPROPERTY()
	int32 CautionCounter = 0;

	/// Timer handle for periodic updates
	FTimerHandle CautionTickHandle;
};
