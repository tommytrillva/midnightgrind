// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGPitStopSubsystem.h
 * @brief Pit Stop Management Subsystem for Midnight Grind Racing
 *
 * This subsystem manages all pit stop operations during races, including vehicle
 * servicing, pit lane traffic management, crew operations, and strategy planning.
 *
 * Key Features:
 * - Complete pit stop workflow (entry, stop, service, release, exit)
 * - Multiple service types (refuel, tire change, repairs, adjustments)
 * - Pit crew simulation with skill levels and fatigue
 * - Pit lane management with speed limits and violations
 * - Tire compound selection and inventory tracking
 * - Race strategy integration and optimization
 *
 * Usage:
 * 1. Configure pit lane with SetPitLaneConfig()
 * 2. Request pit stops via RequestPitStop()
 * 3. Track state changes via OnPitStopStateChanged delegate
 * 4. Monitor completion via OnPitStopCompleted delegate
 *
 * @see UMGFuelSubsystem for fuel-related pit stop operations
 * @see FMGPitStopRequest for configuring pit stop services
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPitStopSubsystem.generated.h"

// ============================================================================
// PIT STOP ENUMERATIONS
// ============================================================================

/**
 * @brief Types of services available during a pit stop
 *
 * Each service takes time and may require specific crew members.
 * Services can be combined in a single pit stop.
 */
UENUM(BlueprintType)
enum class EMGPitStopService : uint8
{
	None,           /// No service requested
	Refuel,         /// Add fuel to vehicle
	TireChange,     /// Replace tires (front, rear, or all)
	RepairDamage,   /// Fix vehicle damage
	AdjustSetup,    /// Modify aero/suspension settings
	DriverChange,   /// Swap drivers (endurance races)
	PenaltyServe,   /// Serve a time penalty
	QuickService,   /// Fast refuel + tire change combo
	FullService     /// Complete service including repairs
};

/**
 * @brief Current state of a pit stop operation
 *
 * Tracks the vehicle's progress through the pit stop workflow.
 */
UENUM(BlueprintType)
enum class EMGPitStopState : uint8
{
	Available,   /// No pit stop in progress
	Approaching, /// Vehicle approaching pit lane entrance
	InPitLane,   /// Vehicle in pit lane, heading to box
	Stopping,    /// Vehicle decelerating into pit box
	Servicing,   /// Crew performing requested services
	Departing,   /// Services complete, vehicle leaving box
	Cooldown,    /// Box on cooldown before next use
	Closed       /// Pit lane closed (safety car, etc.)
};

/**
 * @brief Pit crew member roles
 *
 * Each role has specific responsibilities and timing.
 * Crew members can have different skill levels.
 */
UENUM(BlueprintType)
enum class EMGPitCrewRole : uint8
{
	JackOperator,      /// Raises/lowers vehicle for tire changes
	FuelMan,           /// Handles refueling operations
	TireChangerFL,     /// Front-left tire specialist
	TireChangerFR,     /// Front-right tire specialist
	TireChangerRL,     /// Rear-left tire specialist
	TireChangerRR,     /// Rear-right tire specialist
	BodyRepair,        /// Repairs bodywork damage
	SetupEngineer,     /// Adjusts vehicle setup
	LollipopMan,       /// Signals driver for release
	FireExtinguisher   /// Safety crew member
};

/**
 * @brief Available tire compounds
 *
 * Each compound has different performance and durability characteristics.
 * Strategy involves choosing the right compound for track conditions.
 */
UENUM(BlueprintType)
enum class EMGTireCompound : uint8
{
	UltraSoft,     /// Maximum grip, shortest life
	Soft,          /// High grip, short life
	Medium,        /// Balanced grip and durability
	Hard,          /// Long life, less grip
	Intermediate,  /// Light rain compound
	FullWet,       /// Heavy rain compound
	AllSeason,     /// General purpose for free roam
	Drift          /// Special compound for drift events
};

/**
 * @brief Pit lane rule violations
 *
 * Violations result in time penalties or disqualification.
 */
UENUM(BlueprintType)
enum class EMGPitLaneViolation : uint8
{
	None,              /// No violation
	Speeding,          /// Exceeded pit lane speed limit
	UnsafeRelease,     /// Released into path of another car
	CrossingLine,      /// Crossed pit lane boundary line
	EquipmentContact,  /// Hit pit crew or equipment
	WrongBox,          /// Stopped at wrong pit box
	IgnoringRedLight   /// Left pit box before green light
};

// ============================================================================
// PIT STOP DATA STRUCTURES
// ============================================================================

/**
 * @brief Configuration for a requested pit stop
 *
 * Specifies what services the pit crew should perform.
 * Submit via RequestPitStop() before entering pit lane.
 */
USTRUCT(BlueprintType)
struct FMGPitStopRequest
{
	GENERATED_BODY()

	/// Vehicle requesting the pit stop
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// List of services to perform
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> RequestedServices;

	/// Tire compound to install (if changing tires)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompound NewTireCompound = EMGTireCompound::Medium;

	/// Fuel amount to add in liters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelAmount = 0.0f;

	/// Whether to change front tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChangeFrontTires = true;

	/// Whether to change rear tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChangeRearTires = true;

	/// Repair front wing damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairFrontWing = false;

	/// Repair rear wing damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairRearWing = false;

	/// Repair bodywork damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairBodywork = false;

	/// Front wing angle adjustment (-5 to +5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontWingAdjustment = 0;

	/// Rear wing angle adjustment (-5 to +5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearWingAdjustment = 0;

	/// Higher priority gets serviced first if queue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriorityLevel = 1.0f;
};

/**
 * @brief Results of a completed pit stop
 *
 * Contains timing breakdown and details of work performed.
 * Stored in history for post-race analysis.
 */
USTRUCT(BlueprintType)
struct FMGPitStopResult
{
	GENERATED_BODY()

	/// Vehicle that completed the stop
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Total time from pit entry to exit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	/// Time vehicle was stationary in box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StationaryTime = 0.0f;

	/// Time spent traveling through pit lane
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitLaneTime = 0.0f;

	/// Services that were successfully completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> CompletedServices;

	/// Actual fuel added (may differ from request)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelAdded = 0.0f;

	/// Number of tires changed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TiresChanged = 0;

	/// Amount of damage repaired
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageRepaired = 0.0f;

	/// Whether an error occurred during servicing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHadError = false;

	/// Description of error if one occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ErrorDescription;

	/// Any pit lane violation committed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPitLaneViolation Violation = EMGPitLaneViolation::None;

	/// Time penalty applied for violation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimePenalty = 0.0f;

	/// Lap number when pit stop occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	/// When the pit stop completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * @brief Individual pit crew member data
 *
 * Crew skill affects service time and error probability.
 */
USTRUCT(BlueprintType)
struct FMGPitCrewMember
{
	GENERATED_BODY()

	/// Assigned role on the pit crew
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPitCrewRole Role = EMGPitCrewRole::JackOperator;

	/// Unique identifier for this crew member
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrewMemberID;

	/// Skill level (0.0-2.0, 1.0 = average)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillLevel = 1.0f;

	/// Current fatigue level (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Fatigue = 0.0f;

	/// Chance of making an error (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ErrorChance = 0.05f;

	/// Base time to perform their task in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseServiceTime = 2.0f;

	/// Whether crew member is ready to work
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = true;
};

/**
 * @brief Configuration for a single pit box
 *
 * Each team has their own pit box with assigned crew and inventory.
 */
USTRUCT(BlueprintType)
struct FMGPitBoxConfig
{
	GENERATED_BODY()

	/// Position number along pit lane (1, 2, 3, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoxNumber = 0;

	/// World location of the pit box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxLocation = FVector::ZeroVector;

	/// Direction pit box faces
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator BoxRotation = FRotator::ZeroRotator;

	/// Exact stopping position for vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StopPosition = FVector::ZeroVector;

	/// Assigned pit crew members
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPitCrewMember> Crew;

	/// Available tires by compound type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGTireCompound, int32> TireInventory;

	/// Maximum fuel capacity at this box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelCapacity = 100.0f;

	/// Current fuel available
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFuel = 100.0f;

	/// Equipment quality multiplier (affects speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EquipmentQuality = 1.0f;

	/// Whether a vehicle is currently in this box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOccupied = false;

	/// Vehicle currently assigned to this box
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AssignedVehicle;
};

/**
 * @brief Pit lane configuration for a track
 *
 * Defines pit lane geometry, rules, and pit box locations.
 */
USTRUCT(BlueprintType)
struct FMGPitLaneConfig
{
	GENERATED_BODY()

	/// Track this pit lane belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/// Maximum allowed speed in pit lane (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 60.0f;

	/// Total pit lane length in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaneLength = 300.0f;

	/// World location of pit entry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EntryPoint = FVector::ZeroVector;

	/// World location of pit exit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ExitPoint = FVector::ZeroVector;

	/// All pit boxes on this lane
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPitBoxConfig> PitBoxes;

	/// Whether pit lane is currently open
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPitLaneOpen = true;

	/// Whether speeding is enforced
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSpeedLimitEnforcement = true;

	/// Time penalty for speeding (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedingPenaltyTime = 5.0f;

	/// Whether vehicles have pit limiter function
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPitLimiter = true;

	/// Whether traffic light controls pit exit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTrafficLight = true;
};

/**
 * @brief Active pit stop in progress
 *
 * Tracks the current state of an ongoing pit stop operation.
 */
USTRUCT(BlueprintType)
struct FMGActivePitStop
{
	GENERATED_BODY()

	/// Vehicle being serviced
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Pit box index being used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AssignedBox = 0;

	/// Current state in pit stop workflow
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPitStopState CurrentState = EMGPitStopState::Available;

	/// Original request parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPitStopRequest Request;

	/// When current state began
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StateStartTime = 0.0f;

	/// Total elapsed time in pit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	/// Predicted time until release
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedTimeRemaining = 0.0f;

	/// Services that are done
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> CompletedServices;

	/// Services still in queue
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> PendingServices;

	/// Progress of current service (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentServiceProgress = 0.0f;

	/// Whether jack has raised the car
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJackRaised = false;

	/// Whether lollipop man gave green light
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGreenLightGiven = false;
};

/**
 * @brief Pit stop strategy for race planning
 *
 * Defines planned pit stops and tire/fuel strategy.
 */
USTRUCT(BlueprintType)
struct FMGPitStrategy
{
	GENERATED_BODY()

	/// Strategy name/identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StrategyName;

	/// Number of planned pit stops
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlannedStops = 1;

	/// Lap numbers for each planned stop
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PlannedStopLaps;

	/// Tire compound for each stint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGTireCompound> PlannedCompounds;

	/// Fuel load for each stint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> PlannedFuelLoads;

	/// Prioritize position over optimal timing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptimizeForPosition = true;

	/// Adjust strategy for weather changes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReactToWeather = true;

	/// Pit early to gain track position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUndercut = false;

	/// Pit late on fresh tires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOvercut = false;

	/// Minimum laps before tire change
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLapsOnTire = 10;

	/// Tire wear level to trigger pit (0.0-1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TireWearThreshold = 0.2f;

	/// Minimum fuel at end of stint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelReserveTarget = 2.0f;
};

/**
 * @brief Pit stop statistics for a vehicle
 *
 * Tracks performance metrics across all pit stops.
 */
USTRUCT(BlueprintType)
struct FMGPitStopStats
{
	GENERATED_BODY()

	/// Vehicle these stats belong to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Number of pit stops made
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPitStops = 0;

	/// Fastest pit stop time (stationary)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestPitStop = 0.0f;

	/// Average pit stop time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AveragePitStop = 0.0f;

	/// Number of crew errors
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PitStopErrors = 0;

	/// Number of speed limit violations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpeedingViolations = 0;

	/// Total penalty time from violations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeLostToPenalties = 0.0f;

	/// Complete history of pit stops
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPitStopResult> PitStopHistory;
};

// ============================================================================
// PIT STOP EVENT DELEGATES
// ============================================================================

/// Fired when a pit stop is requested
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStopRequested, FName, VehicleID, const FMGPitStopRequest&, Request);
/// Fired when pit stop state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStopStateChanged, FName, VehicleID, EMGPitStopState, NewState);
/// Fired when pit stop is fully complete
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStopCompleted, FName, VehicleID, const FMGPitStopResult&, Result);
/// Fired when an individual service begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPitServiceStarted, FName, VehicleID, EMGPitStopService, Service, float, EstimatedTime);
/// Fired when an individual service completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitServiceCompleted, FName, VehicleID, EMGPitStopService, Service);
/// Fired when a pit lane violation occurs
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitLaneViolation, FName, VehicleID, EMGPitLaneViolation, Violation);
/// Fired when pit strategy is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStrategyUpdated, FName, VehicleID, const FMGPitStrategy&, Strategy);
/// Fired when pit lane opens or closes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPitLaneStatusChanged, bool, bIsOpen);

// ============================================================================
// PIT STOP SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Main pit stop management subsystem
 *
 * Handles all pit stop operations during races including:
 * - Pit stop workflow management
 * - Crew and service simulation
 * - Pit lane traffic and violations
 * - Strategy planning and optimization
 *
 * Typical usage flow:
 * 1. SetPitLaneConfig() at race start
 * 2. RequestPitStop() when player wants to pit
 * 3. EnterPitLane() when crossing pit entry
 * 4. ArriveAtPitBox() when stopping in box
 * 5. BeginServicing() starts the services
 * 6. ReleaseFromPitBox() when complete
 * 7. ExitPitLane() when leaving pit
 *
 * @note This is a GameInstanceSubsystem - persists across level loads
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPitStopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ========================================================================
	// SUBSYSTEM LIFECYCLE
	// ========================================================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ========================================================================
	// PIT STOP REQUESTS
	// ========================================================================

	/// Submit a pit stop request (call before entering pit lane)
	UFUNCTION(BlueprintCallable, Category = "PitStop|Request")
	bool RequestPitStop(FName VehicleID, const FMGPitStopRequest& Request);

	/// Cancel a pending pit stop request
	UFUNCTION(BlueprintCallable, Category = "PitStop|Request")
	void CancelPitStopRequest(FName VehicleID);

	/// Update an existing pit stop request
	UFUNCTION(BlueprintCallable, Category = "PitStop|Request")
	void ModifyPitStopRequest(FName VehicleID, const FMGPitStopRequest& NewRequest);

	/// Check if vehicle has a pending request
	UFUNCTION(BlueprintPure, Category = "PitStop|Request")
	bool HasPendingPitStop(FName VehicleID) const;

	/// Get the pending request details
	UFUNCTION(BlueprintPure, Category = "PitStop|Request")
	FMGPitStopRequest GetPendingRequest(FName VehicleID) const;

	/// Calculate estimated time for a pit stop request
	UFUNCTION(BlueprintPure, Category = "PitStop|Request")
	float EstimatePitStopTime(const FMGPitStopRequest& Request) const;

	// ========================================================================
	// PIT STOP STATE MANAGEMENT
	// ========================================================================

	/// Called when vehicle enters pit lane
	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void EnterPitLane(FName VehicleID);

	/// Called when vehicle stops at pit box
	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void ArriveAtPitBox(FName VehicleID);

	/// Called to start servicing (crew begins work)
	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void BeginServicing(FName VehicleID);

	/// Called to release vehicle from pit box
	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void ReleaseFromPitBox(FName VehicleID);

	/// Called when vehicle leaves pit lane
	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void ExitPitLane(FName VehicleID);

	/// Get current pit stop state for a vehicle
	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	EMGPitStopState GetPitStopState(FName VehicleID) const;

	/// Get full active pit stop data
	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	FMGActivePitStop GetActivePitStop(FName VehicleID) const;

	/// Check if vehicle is anywhere in pit lane
	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	bool IsVehicleInPitLane(FName VehicleID) const;

	/// Check if vehicle is currently being serviced
	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	bool IsVehicleBeingServiced(FName VehicleID) const;

	// ========================================================================
	// PIT LANE MANAGEMENT
	// ========================================================================

	/// Configure the pit lane for a track
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void SetPitLaneConfig(const FMGPitLaneConfig& Config);

	/// Get current pit lane configuration
	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	FMGPitLaneConfig GetPitLaneConfig() const { return PitLaneConfig; }

	/// Open pit lane for normal operations
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void OpenPitLane();

	/// Close pit lane (safety car, etc.)
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void ClosePitLane();

	/// Check if pit lane is open
	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	bool IsPitLaneOpen() const { return PitLaneConfig.bPitLaneOpen; }

	/// Get current pit lane speed limit
	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	float GetPitLaneSpeedLimit() const { return PitLaneConfig.SpeedLimit; }

	/// Report vehicle speed for violation checking
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void ReportVehicleSpeed(FName VehicleID, float CurrentSpeed);

	/// Get next available pit box index
	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	int32 GetAvailablePitBox() const;

	/// Assign a specific pit box to a vehicle
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void AssignPitBox(FName VehicleID, int32 BoxIndex);

	// ========================================================================
	// PIT BOX MANAGEMENT
	// ========================================================================

	/// Configure a specific pit box
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void ConfigurePitBox(int32 BoxIndex, const FMGPitBoxConfig& Config);

	/// Get configuration for a pit box
	UFUNCTION(BlueprintPure, Category = "PitStop|PitBox")
	FMGPitBoxConfig GetPitBoxConfig(int32 BoxIndex) const;

	/// Set skill level for a crew member
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void SetCrewMemberSkill(int32 BoxIndex, EMGPitCrewRole Role, float SkillLevel);

	/// Add tires to a pit box inventory
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void RefillTireInventory(int32 BoxIndex, EMGTireCompound Compound, int32 Amount);

	/// Add fuel to a pit box
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void RefuelPitBox(int32 BoxIndex, float Amount);

	/// Get tire count for a compound at a box
	UFUNCTION(BlueprintPure, Category = "PitStop|PitBox")
	int32 GetTireInventory(int32 BoxIndex, EMGTireCompound Compound) const;

	/// Get current fuel at a pit box
	UFUNCTION(BlueprintPure, Category = "PitStop|PitBox")
	float GetPitBoxFuel(int32 BoxIndex) const;

	// ========================================================================
	// STRATEGY MANAGEMENT
	// ========================================================================

	/// Set pit strategy for a vehicle
	UFUNCTION(BlueprintCallable, Category = "PitStop|Strategy")
	void SetPitStrategy(FName VehicleID, const FMGPitStrategy& Strategy);

	/// Get current pit strategy
	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	FMGPitStrategy GetPitStrategy(FName VehicleID) const;

	/// Calculate optimal strategy based on conditions
	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	FMGPitStrategy CalculateOptimalStrategy(FName VehicleID, int32 RemainingLaps, float CurrentFuel, float TireWear) const;

	/// Get recommended next pit lap
	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	int32 GetRecommendedPitLap(FName VehicleID) const;

	/// Get recommended tire compound
	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	EMGTireCompound GetRecommendedCompound(FName VehicleID) const;

	/// Update strategy for weather changes
	UFUNCTION(BlueprintCallable, Category = "PitStop|Strategy")
	void UpdateStrategyForWeather(FName VehicleID, bool bRaining);

	/// Update strategy based on race position
	UFUNCTION(BlueprintCallable, Category = "PitStop|Strategy")
	void UpdateStrategyForPosition(FName VehicleID, int32 CurrentPosition, int32 GapToAhead, int32 GapToBehind);

	// ========================================================================
	// QUICK ACTIONS (Convenience Functions)
	// ========================================================================

	/// Request quick fuel-only stop
	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestQuickFuel(FName VehicleID, float Amount);

	/// Request quick tire-only stop
	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestQuickTires(FName VehicleID, EMGTireCompound Compound);

	/// Request full service pit stop
	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestFullService(FName VehicleID);

	/// Request minimal/fastest possible service
	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestMinimalService(FName VehicleID);

	// ========================================================================
	// STATISTICS
	// ========================================================================

	/// Get pit stop statistics for a vehicle
	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	FMGPitStopStats GetPitStopStats(FName VehicleID) const;

	/// Get complete pit stop history for a vehicle
	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	TArray<FMGPitStopResult> GetPitStopHistory(FName VehicleID) const;

	/// Get the fastest pit stop time in the race
	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	float GetFastestPitStop() const;

	/// Get most recent pit stop for a vehicle
	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	FMGPitStopResult GetLastPitStop(FName VehicleID) const;

	/// Clear all race statistics
	UFUNCTION(BlueprintCallable, Category = "PitStop|Stats")
	void ResetRaceStats();

	// ========================================================================
	// EVENT DELEGATES
	// ========================================================================

	/// Broadcast when pit stop is requested
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStopRequested OnPitStopRequested;

	/// Broadcast when pit stop state changes
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStopStateChanged OnPitStopStateChanged;

	/// Broadcast when pit stop is fully complete
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStopCompleted OnPitStopCompleted;

	/// Broadcast when individual service starts
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitServiceStarted OnPitServiceStarted;

	/// Broadcast when individual service completes
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitServiceCompleted OnPitServiceCompleted;

	/// Broadcast when pit lane violation occurs
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitLaneViolation OnPitLaneViolation;

	/// Broadcast when strategy is updated
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStrategyUpdated OnPitStrategyUpdated;

	/// Broadcast when pit lane opens/closes
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitLaneStatusChanged OnPitLaneStatusChanged;

protected:
	// ========================================================================
	// INTERNAL METHODS
	// ========================================================================

	/// Timer callback for pit stop updates
	void OnPitStopTick();
	/// Update all active pit stops
	void UpdateActivePitStops(float MGDeltaTime);
	/// Process a single service on a vehicle
	void ProcessService(FMGActivePitStop& PitStop, float MGDeltaTime);
	/// Calculate time for a specific service
	float CalculateServiceTime(EMGPitStopService Service, int32 BoxIndex) const;
	/// Check if crew member made an error
	bool CheckForCrewError(int32 BoxIndex, EMGPitCrewRole Role) const;
	/// Apply penalty for a violation
	void ApplyPenalty(FName VehicleID, EMGPitLaneViolation Violation);
	/// Finalize a pit stop and return results
	FMGPitStopResult CompletePitStop(const FMGActivePitStop& PitStop);
	/// Save pit stop data to save game
	void SavePitStopData();
	/// Load pit stop data from save game
	void LoadPitStopData();

	// ========================================================================
	// DATA STORAGE
	// ========================================================================

	/// Current pit lane configuration
	UPROPERTY()
	FMGPitLaneConfig PitLaneConfig;

	/// Pending pit stop requests by vehicle
	UPROPERTY()
	TMap<FName, FMGPitStopRequest> PendingRequests;

	/// Currently active pit stops
	UPROPERTY()
	TMap<FName, FMGActivePitStop> ActivePitStops;

	/// Pit strategies by vehicle
	UPROPERTY()
	TMap<FName, FMGPitStrategy> VehicleStrategies;

	/// Pit stop statistics by vehicle
	UPROPERTY()
	TMap<FName, FMGPitStopStats> VehicleStats;

	/// Complete race pit stop history
	UPROPERTY()
	TArray<FMGPitStopResult> RacePitStopHistory;

	/// Fastest pit stop time in current race
	UPROPERTY()
	float FastestPitStopTime = 0.0f;

	/// Vehicle with fastest pit stop
	UPROPERTY()
	FName FastestPitStopVehicle;

	/// Timer handle for pit stop updates
	FTimerHandle PitStopTickHandle;
};
