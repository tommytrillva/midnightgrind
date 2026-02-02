// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MG_FUEL_Subsystem.h
 * @brief Fuel Management Subsystem for Midnight Grind Racing
 *
 * This subsystem provides comprehensive fuel simulation and management for all vehicles
 * in the game. It handles fuel consumption calculations, tank management, refueling
 * operations, and strategic fuel planning for races.
 *
 * Key Features:
 * - Multiple fuel types with different performance characteristics
 * - Dynamic fuel consumption based on driving style and conditions
 * - Fuel mode switching for different race strategies (economy, performance, etc.)
 * - Pit stop integration for refueling operations
 * - Telemetry tracking for fuel efficiency analysis
 * - Race strategy planning and fuel calculations
 *
 * Usage:
 * Access via UGameInstance::GetSubsystem<UMGFuelSubsystem>() to register vehicles,
 * update fuel consumption each frame, and query fuel state for UI/AI.
 *
 * @see UMGPitStopSubsystem for refueling during pit stops
 * @see FMGVehicleFuelState for current fuel status of a vehicle
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MG_FUEL_Subsystem.generated.h"

// ============================================================================
// FUEL TYPE ENUMERATIONS
// ============================================================================

/**
 * @brief Available fuel types with different performance characteristics
 *
 * Each fuel type affects power output, efficiency, and consumption rate.
 * Some vehicles may only be compatible with specific fuel types.
 */
UENUM(BlueprintType)
enum class EMGFuelType : uint8
{
	Regular,        /// Standard unleaded fuel - balanced performance
	Premium,        /// Higher octane for better power, slightly more expensive
	Racing,         /// High-performance fuel for maximum power output
	Diesel,         /// For diesel engines - more torque, better efficiency
	Electric,       /// Battery power for electric vehicles
	Hybrid,         /// Combined fuel/electric for hybrid vehicles
	Nitromethane,   /// Extreme performance fuel for drag racing
	E85             /// Ethanol blend - eco-friendly alternative
};

/**
 * @brief Current fuel level state thresholds
 *
 * Used to trigger warnings and adjust AI/UI behavior based on remaining fuel.
 */
UENUM(BlueprintType)
enum class EMGFuelState : uint8
{
	Full,       /// Tank is completely full (90-100%)
	Adequate,   /// Sufficient fuel for normal operation (25-90%)
	Low,        /// Fuel warning threshold reached (10-25%)
	Critical,   /// Very low fuel - immediate pit stop recommended (<10%)
	Empty,      /// No fuel remaining - vehicle cannot move
	Reserved    /// Running on reserve tank (if equipped)
};

/**
 * @brief Fuel consumption modes for strategic driving
 *
 * Players can switch between modes to balance performance vs fuel economy.
 * AI will automatically adjust based on race situation.
 */
UENUM(BlueprintType)
enum class EMGFuelMode : uint8
{
	Standard,    /// Normal driving mode - balanced consumption
	Economy,     /// Reduced power for maximum fuel efficiency
	Performance, /// Full power with higher fuel consumption
	Qualifying,  /// Maximum performance for single lap (very high consumption)
	Attack,      /// Aggressive mode for overtaking (temporary high power)
	Defend,      /// Slightly reduced to maintain position and save fuel
	Limp         /// Minimal power to barely reach the pits
};

/**
 * @brief Fuel-related alert types for player notifications
 *
 * Broadcast via delegates to update HUD and trigger audio cues.
 */
UENUM(BlueprintType)
enum class EMGFuelAlert : uint8
{
	None,                /// No active fuel alerts
	LowFuel,             /// Fuel below 25% - consider pitting soon
	CriticalFuel,        /// Fuel below 10% - pit immediately
	FuelSaveRecommended, /// Strategic recommendation to conserve fuel
	PitWindowOpen,       /// Optimal pit stop window is now open
	WontFinish           /// Not enough fuel to complete remaining laps
};

// ============================================================================
// FUEL DATA STRUCTURES
// ============================================================================

/**
 * @brief Configuration data for a specific fuel type
 *
 * Defines the properties and multipliers for each available fuel type.
 * Configure via data assets or runtime for different fuel characteristics.
 */
USTRUCT(BlueprintType)
struct FMGFuelTypeData
{
	GENERATED_BODY()

	/// The enum type this data defines
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelType FuelType = EMGFuelType::Regular;

	/// Unique identifier for this fuel configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FuelID;

	/// Localized display name for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Energy per unit volume (affects range per liter)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnergyDensity = 1.0f;

	/// Engine power multiplier when using this fuel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerMultiplier = 1.0f;

	/// Fuel efficiency multiplier (higher = less consumption)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EfficiencyMultiplier = 1.0f;

	/// Price per liter in game currency
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CostPerLiter = 1.0f;

	/// Whether vehicle needs upgraded fuel system
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresSpecialTank = false;

	/// Fuel octane rating (affects knock resistance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OctaneRating = 95.0f;

	/// Environmental impact score (for eco-mode scoring)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnvironmentalImpact = 1.0f;

	/// Color used for fuel visualization in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FuelColor = FLinearColor::Green;
};

/**
 * @brief Real-time fuel state for a registered vehicle
 *
 * Contains all current fuel-related values for a vehicle.
 * Updated each tick by UpdateFuelConsumption().
 */
USTRUCT(BlueprintType)
struct FMGVehicleFuelState
{
	GENERATED_BODY()

	/// Unique identifier linking to vehicle data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Current fuel type loaded in the tank
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelType CurrentFuelType = EMGFuelType::Regular;

	/// Current fuel amount in liters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFuel = 50.0f;

	/// Maximum tank capacity in liters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TankCapacity = 60.0f;

	/// Fuel percentage (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelPercentage = 0.833f;

	/// Current fuel level state enum
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelState State = EMGFuelState::Adequate;

	/// Active fuel consumption mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelMode FuelMode = EMGFuelMode::Standard;

	/// Current consumption rate in liters per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumptionRate = 0.0f;

	/// Instant consumption reading (real-time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InstantConsumption = 0.0f;

	/// Rolling average consumption rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageConsumption = 0.0f;

	/// Total fuel consumed since last reset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalFuelUsed = 0.0f;

	/// Total distance traveled in current session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceTraveled = 0.0f;

	/// Estimated remaining range in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedRange = 0.0f;

	/// Estimated laps remaining at current consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedLapsRemaining = 0.0f;

	/// Current fuel weight in kg (affects vehicle physics)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelWeight = 0.0f;

	/// Fuel temperature in Celsius (can affect performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelTemperature = 25.0f;

	/// Whether fuel save mode is currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFuelSaveActive = false;

	/// Target fuel save percentage (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelSavePercentage = 0.0f;
};

/**
 * @brief Factors that affect fuel consumption rate
 *
 * Each factor multiplies the base consumption to calculate actual usage.
 * Configure per vehicle or use global defaults.
 */
USTRUCT(BlueprintType)
struct FMGFuelConsumptionFactors
{
	GENERATED_BODY()

	/// Base fuel consumption in liters per second at idle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseConsumption = 0.1f;

	/// Multiplier applied based on throttle input (0-1 mapped)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleMultiplier = 1.0f;

	/// Consumption increase at higher speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	/// Engine RPM influence on consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPMMultiplier = 1.0f;

	/// Gear ratio efficiency factor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GearMultiplier = 1.0f;

	/// Massive consumption increase when nitro is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroMultiplier = 3.0f;

	/// Fuel used while stationary with engine running
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleConsumption = 0.01f;

	/// Reduced consumption when drafting (slipstream)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DraftingBonus = 0.9f;

	/// Increased consumption on uphill sections
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InclineMultiplier = 1.0f;

	/// Weather effects on consumption (rain, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeatherMultiplier = 1.0f;

	/// Worn tires increase fuel consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TireWearMultiplier = 1.0f;

	/// Vehicle damage increases consumption
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;
};

/**
 * @brief Settings for each fuel consumption mode
 *
 * Defines how each EMGFuelMode affects vehicle performance and consumption.
 */
USTRUCT(BlueprintType)
struct FMGFuelModeSettings
{
	GENERATED_BODY()

	/// The mode these settings apply to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFuelMode Mode = EMGFuelMode::Standard;

	/// Display name for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ModeName;

	/// Engine power multiplier in this mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PowerMultiplier = 1.0f;

	/// Fuel consumption multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConsumptionMultiplier = 1.0f;

	/// Rev limiter adjustment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRPMMultiplier = 1.0f;

	/// Throttle sensitivity adjustment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleResponseMultiplier = 1.0f;

	/// Whether nitro boost is available in this mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowNitro = true;

	/// Target fuel economy percentage (for auto fuel save)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelSaveTarget = 0.0f;
};

/**
 * @brief Historical fuel telemetry data for a vehicle
 *
 * Tracks consumption patterns over time for strategy analysis.
 */
USTRUCT(BlueprintType)
struct FMGFuelTelemetry
{
	GENERATED_BODY()

	/// Vehicle this telemetry belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Rolling history of consumption readings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> ConsumptionHistory;

	/// Fuel used per completed lap
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> LapConsumption;

	/// Highest recorded consumption rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakConsumption = 0.0f;

	/// Most efficient consumption rate achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowestConsumption = 0.0f;

	/// Most fuel-efficient lap (least fuel used)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestEfficiencyLap = 0.0f;

	/// Least efficient lap (most fuel used)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorstEfficiencyLap = 0.0f;

	/// Total fuel used in current session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalFuelUsedSession = 0.0f;

	/// Total distance covered in session
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceSession = 0.0f;

	/// Overall efficiency (distance per liter)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SessionEfficiency = 0.0f;
};

/**
 * @brief Race fuel strategy configuration
 *
 * Defines the planned fuel management approach for a race.
 * Used by AI and displayed to player for strategy planning.
 */
USTRUCT(BlueprintType)
struct FMGFuelStrategy
{
	GENERATED_BODY()

	/// Strategy identifier/name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StrategyName;

	/// Initial fuel load at race start (liters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartingFuel = 0.0f;

	/// Minimum fuel to have at finish line
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetFuelAtFinish = 2.0f;

	/// Lap numbers where pit stops are planned
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PlannedPitLaps;

	/// Fuel amounts to add at each pit stop
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> PlannedFuelLoads;

	/// Fuel mode to use for each stint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGFuelMode> PlannedFuelModes;

	/// Expected consumption per lap based on track data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedConsumptionPerLap = 0.0f;

	/// Pit earlier than optimal to gain track position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUndercut = false;

	/// Whether fuel saving is required to make strategy work
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFuelSaveRequired = false;

	/// Lap number to start fuel saving (if required)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FuelSaveFromLap = 0;
};

/**
 * @brief Global fuel simulation settings
 *
 * Controls fuel system behavior and realism level.
 */
USTRUCT(BlueprintType)
struct FMGFuelSettings
{
	GENERATED_BODY()

	/// Master toggle for fuel simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateFuel = true;

	/// Global consumption rate modifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalConsumptionMultiplier = 1.0f;

	/// Whether fuel weight affects vehicle handling
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFuelAffectsWeight = true;

	/// Weight per liter of fuel in kg
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelWeightPerLiter = 0.75f;

	/// Enable/disable fuel warning notifications
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowFuelAlerts = true;

	/// Percentage threshold for low fuel warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowFuelThreshold = 0.25f;

	/// Percentage threshold for critical fuel warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalFuelThreshold = 0.1f;

	/// Auto-switch to economy mode when low
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoFuelMode = false;

	/// Show real-time consumption on HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowConsumptionHUD = true;

	/// Show estimated range on HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRangeEstimate = true;

	/// Liters per second during pit stop refueling
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RefuelRate = 10.0f;
};

// ============================================================================
// FUEL EVENT DELEGATES
// ============================================================================

/// Fired when a vehicle's fuel state category changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFuelStateChanged, FName, VehicleID, EMGFuelState, OldState, EMGFuelState, NewState);
/// Fired when a fuel alert is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelAlert, FName, VehicleID, EMGFuelAlert, Alert);
/// Fired when driver changes fuel mode
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelModeChanged, FName, VehicleID, EMGFuelMode, NewMode);
/// Fired each time fuel is consumed (for detailed tracking)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelConsumed, FName, VehicleID, float, AmountConsumed);
/// Fired when fuel is added via refueling
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelAdded, FName, VehicleID, float, AmountAdded);
/// Fired when vehicle runs completely out of fuel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelEmpty, FName, VehicleID);
/// Fired at lap completion with fuel usage for that lap
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLapFuelUsage, FName, VehicleID, int32, LapNumber, float, FuelUsed);

// ============================================================================
// FUEL SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Main fuel management subsystem
 *
 * Provides all fuel-related functionality including consumption simulation,
 * refueling, strategy planning, and telemetry tracking.
 *
 * Lifecycle:
 * 1. Register vehicles with RegisterVehicle()
 * 2. Call UpdateFuelConsumption() each tick with driving parameters
 * 3. Query state via GetFuelState(), GetEstimatedRange(), etc.
 * 4. Handle refueling via AddFuel() or StartRefueling()
 *
 * @note This is a GameInstanceSubsystem - persists across level loads
 */
UCLASS()
class MIDNIGHTGRIND_API UMGFuelSubsystem : public UGameInstanceSubsystem
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
	// VEHICLE REGISTRATION
	// ========================================================================

	/// Register a new vehicle for fuel tracking
	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void RegisterVehicle(FName VehicleID, float TankCapacity, EMGFuelType FuelType = EMGFuelType::Regular);

	/// Remove a vehicle from fuel tracking
	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void UnregisterVehicle(FName VehicleID);

	/// Set current fuel amount directly (for loading saves, etc.)
	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void SetVehicleFuel(FName VehicleID, float FuelAmount);

	/// Change vehicle's tank capacity (for upgrades)
	UFUNCTION(BlueprintCallable, Category = "Fuel|Vehicle")
	void SetTankCapacity(FName VehicleID, float Capacity);

	// ========================================================================
	// FUEL STATE QUERIES
	// ========================================================================

	/// Get complete fuel state for a vehicle
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	FMGVehicleFuelState GetFuelState(FName VehicleID) const;

	/// Get current fuel amount in liters
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetCurrentFuel(FName VehicleID) const;

	/// Get fuel as percentage of tank capacity (0.0-1.0)
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetFuelPercentage(FName VehicleID) const;

	/// Get current fuel state enum (Full, Low, Critical, etc.)
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	EMGFuelState GetFuelStatus(FName VehicleID) const;

	/// Get estimated range remaining in meters
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetEstimatedRange(FName VehicleID) const;

	/// Get estimated laps remaining at current consumption rate
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetEstimatedLapsRemaining(FName VehicleID) const;

	/// Check if vehicle can complete specified number of laps
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	bool CanFinishRace(FName VehicleID, int32 RemainingLaps) const;

	/// Get current fuel weight in kg (for physics)
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetFuelWeight(FName VehicleID) const;

	// ========================================================================
	// FUEL CONSUMPTION
	// ========================================================================

	/// Main update function - call each frame with current driving parameters
	UFUNCTION(BlueprintCallable, Category = "Fuel|Consumption")
	void UpdateFuelConsumption(FName VehicleID, float Throttle, float Speed, float RPM, int32 Gear, bool bNitroActive, float DeltaTime);

	/// Consume a specific amount of fuel directly
	UFUNCTION(BlueprintCallable, Category = "Fuel|Consumption")
	void ConsumeFuel(FName VehicleID, float Amount);

	/// Get real-time instantaneous consumption rate
	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	float GetInstantConsumption(FName VehicleID) const;

	/// Get rolling average consumption rate
	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	float GetAverageConsumption(FName VehicleID) const;

	/// Get average fuel used per lap
	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	float GetConsumptionPerLap(FName VehicleID) const;

	/// Set custom consumption factors for a vehicle
	UFUNCTION(BlueprintCallable, Category = "Fuel|Consumption")
	void SetConsumptionFactors(FName VehicleID, const FMGFuelConsumptionFactors& Factors);

	/// Get current consumption factors for a vehicle
	UFUNCTION(BlueprintPure, Category = "Fuel|Consumption")
	FMGFuelConsumptionFactors GetConsumptionFactors(FName VehicleID) const;

	// ========================================================================
	// FUEL MODE MANAGEMENT
	// ========================================================================

	/// Switch to a specific fuel mode
	UFUNCTION(BlueprintCallable, Category = "Fuel|Mode")
	void SetFuelMode(FName VehicleID, EMGFuelMode Mode);

	/// Get current fuel mode
	UFUNCTION(BlueprintPure, Category = "Fuel|Mode")
	EMGFuelMode GetFuelMode(FName VehicleID) const;

	/// Cycle through available fuel modes
	UFUNCTION(BlueprintCallable, Category = "Fuel|Mode")
	void CycleFuelMode(FName VehicleID, bool bForward = true);

	/// Get settings for a specific fuel mode
	UFUNCTION(BlueprintPure, Category = "Fuel|Mode")
	FMGFuelModeSettings GetFuelModeSettings(EMGFuelMode Mode) const;

	/// Customize fuel mode settings
	UFUNCTION(BlueprintCallable, Category = "Fuel|Mode")
	void SetFuelModeSettings(EMGFuelMode Mode, const FMGFuelModeSettings& Settings);

	/// Get list of all available fuel modes
	UFUNCTION(BlueprintPure, Category = "Fuel|Mode")
	TArray<EMGFuelMode> GetAvailableFuelModes() const;

	// ========================================================================
	// FUEL SAVE MODE
	// ========================================================================

	/// Enable fuel save mode with target percentage reduction
	UFUNCTION(BlueprintCallable, Category = "Fuel|Save")
	void ActivateFuelSave(FName VehicleID, float TargetPercentage);

	/// Disable fuel save mode
	UFUNCTION(BlueprintCallable, Category = "Fuel|Save")
	void DeactivateFuelSave(FName VehicleID);

	/// Check if fuel save is currently active
	UFUNCTION(BlueprintPure, Category = "Fuel|Save")
	bool IsFuelSaveActive(FName VehicleID) const;

	/// Get current fuel save reduction amount
	UFUNCTION(BlueprintPure, Category = "Fuel|Save")
	float GetFuelSaveAmount(FName VehicleID) const;

	// ========================================================================
	// REFUELING OPERATIONS
	// ========================================================================

	/// Add fuel instantly (cheat/debug)
	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void AddFuel(FName VehicleID, float Amount);

	/// Fill tank to capacity instantly
	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void FillTank(FName VehicleID);

	/// Begin timed refueling operation
	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void StartRefueling(FName VehicleID, float TargetAmount);

	/// Interrupt refueling before completion
	UFUNCTION(BlueprintCallable, Category = "Fuel|Refuel")
	void StopRefueling(FName VehicleID);

	/// Check if vehicle is currently refueling
	UFUNCTION(BlueprintPure, Category = "Fuel|Refuel")
	bool IsRefueling(FName VehicleID) const;

	/// Get refueling completion percentage (0.0-1.0)
	UFUNCTION(BlueprintPure, Category = "Fuel|Refuel")
	float GetRefuelingProgress(FName VehicleID) const;

	/// Calculate time needed to add specified fuel amount
	UFUNCTION(BlueprintPure, Category = "Fuel|Refuel")
	float CalculateRefuelTime(float Amount) const;

	// ========================================================================
	// FUEL TYPE MANAGEMENT
	// ========================================================================

	/// Register a new fuel type configuration
	UFUNCTION(BlueprintCallable, Category = "Fuel|Type")
	void RegisterFuelType(const FMGFuelTypeData& FuelData);

	/// Get data for a specific fuel type
	UFUNCTION(BlueprintPure, Category = "Fuel|Type")
	FMGFuelTypeData GetFuelTypeData(EMGFuelType Type) const;

	/// Change a vehicle's fuel type
	UFUNCTION(BlueprintCallable, Category = "Fuel|Type")
	void SetVehicleFuelType(FName VehicleID, EMGFuelType Type);

	/// Get vehicle's current fuel type
	UFUNCTION(BlueprintPure, Category = "Fuel|Type")
	EMGFuelType GetVehicleFuelType(FName VehicleID) const;

	/// Get all registered fuel types
	UFUNCTION(BlueprintPure, Category = "Fuel|Type")
	TArray<FMGFuelTypeData> GetAllFuelTypes() const;

	// ========================================================================
	// RACE STRATEGY
	// ========================================================================

	/// Set the fuel strategy for a vehicle
	UFUNCTION(BlueprintCallable, Category = "Fuel|Strategy")
	void SetFuelStrategy(FName VehicleID, const FMGFuelStrategy& Strategy);

	/// Get current fuel strategy
	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	FMGFuelStrategy GetFuelStrategy(FName VehicleID) const;

	/// Calculate optimal strategy based on track/race data
	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	FMGFuelStrategy CalculateOptimalStrategy(FName VehicleID, int32 TotalLaps, float LapLength) const;

	/// Calculate fuel needed for specified number of laps
	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	float CalculateRequiredFuel(FName VehicleID, int32 Laps) const;

	/// Get recommended lap for next pit stop
	UFUNCTION(BlueprintPure, Category = "Fuel|Strategy")
	int32 GetRecommendedPitLap(FName VehicleID, int32 RemainingLaps) const;

	// ========================================================================
	// TELEMETRY
	// ========================================================================

	/// Get complete telemetry data for a vehicle
	UFUNCTION(BlueprintPure, Category = "Fuel|Telemetry")
	FMGFuelTelemetry GetFuelTelemetry(FName VehicleID) const;

	/// Record fuel usage for a completed lap
	UFUNCTION(BlueprintCallable, Category = "Fuel|Telemetry")
	void RecordLapFuelUsage(FName VehicleID, int32 LapNumber);

	/// Clear all telemetry data for a vehicle
	UFUNCTION(BlueprintCallable, Category = "Fuel|Telemetry")
	void ResetTelemetry(FName VehicleID);

	// ========================================================================
	// LAP TRACKING
	// ========================================================================

	/// Called when vehicle crosses start line
	UFUNCTION(BlueprintCallable, Category = "Fuel|Lap")
	void OnLapStarted(FName VehicleID, int32 LapNumber);

	/// Called when vehicle completes a lap
	UFUNCTION(BlueprintCallable, Category = "Fuel|Lap")
	void OnLapCompleted(FName VehicleID, int32 LapNumber);

	// ========================================================================
	// GLOBAL SETTINGS
	// ========================================================================

	/// Update global fuel simulation settings
	UFUNCTION(BlueprintCallable, Category = "Fuel|Settings")
	void SetFuelSettings(const FMGFuelSettings& NewSettings);

	/// Get current global fuel settings
	UFUNCTION(BlueprintPure, Category = "Fuel|Settings")
	FMGFuelSettings GetFuelSettings() const { return Settings; }

	// ========================================================================
	// EVENT DELEGATES
	// ========================================================================

	/// Broadcast when fuel state category changes (Full -> Low, etc.)
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelStateChanged OnFuelStateChanged;

	/// Broadcast when fuel alert is triggered
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelAlert OnFuelAlert;

	/// Broadcast when fuel mode is changed
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelModeChanged OnFuelModeChanged;

	/// Broadcast on each fuel consumption update
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelConsumed OnFuelConsumed;

	/// Broadcast when fuel is added
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelAdded OnFuelAdded;

	/// Broadcast when tank reaches empty
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelEmpty OnFuelEmpty;

	/// Broadcast at lap completion with fuel usage data
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnLapFuelUsage OnLapFuelUsage;

protected:
	// ========================================================================
	// INTERNAL METHODS
	// ========================================================================

	/// Timer callback for periodic fuel updates
	void OnFuelTick();
	/// Update all active refueling operations
	void UpdateRefueling(float DeltaTime);
	/// Update all vehicle fuel states
	void UpdateFuelStates();
	/// Check and fire fuel alerts for a vehicle
	void CheckFuelAlerts(FName VehicleID);
	/// Calculate fuel state enum from percentage
	EMGFuelState CalculateFuelState(float Percentage) const;
	/// Create default fuel type configurations
	void InitializeDefaultFuelTypes();
	/// Create default fuel mode settings
	void InitializeDefaultFuelModes();
	/// Persist fuel data to save game
	void SaveFuelData();
	/// Load fuel data from save game
	void LoadFuelData();

	// ========================================================================
	// DATA STORAGE
	// ========================================================================

	/// Current fuel state for each registered vehicle
	UPROPERTY()
	TMap<FName, FMGVehicleFuelState> VehicleFuelStates;

	/// Consumption factor configuration per vehicle
	UPROPERTY()
	TMap<FName, FMGFuelConsumptionFactors> VehicleConsumptionFactors;

	/// Telemetry history per vehicle
	UPROPERTY()
	TMap<FName, FMGFuelTelemetry> VehicleTelemetry;

	/// Race strategy per vehicle
	UPROPERTY()
	TMap<FName, FMGFuelStrategy> VehicleStrategies;

	/// Fuel level at start of each lap (for per-lap tracking)
	UPROPERTY()
	TMap<FName, float> VehicleLapStartFuel;

	/// Registered fuel type configurations
	UPROPERTY()
	TMap<EMGFuelType, FMGFuelTypeData> FuelTypes;

	/// Fuel mode settings
	UPROPERTY()
	TMap<EMGFuelMode, FMGFuelModeSettings> FuelModes;

	/// Vehicles currently being refueled
	UPROPERTY()
	TMap<FName, bool> RefuelingVehicles;

	/// Target fuel amount for each refueling vehicle
	UPROPERTY()
	TMap<FName, float> RefuelingTargets;

	/// Current refueling progress per vehicle
	UPROPERTY()
	TMap<FName, float> RefuelingProgress;

	/// Global fuel simulation settings
	UPROPERTY()
	FMGFuelSettings Settings;

	/// Timer handle for periodic fuel tick
	FTimerHandle FuelTickHandle;
};
