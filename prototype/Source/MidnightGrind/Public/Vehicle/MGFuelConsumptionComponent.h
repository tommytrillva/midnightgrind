// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGFuelConsumptionComponent.h
 * @brief Realistic fuel consumption component for vehicle simulation
 *
 * This component provides detailed fuel consumption modeling including:
 * - Throttle-based consumption curves
 * - RPM and boost multipliers
 * - Driving style analysis (aggressive vs economical)
 * - Weight reduction as fuel is consumed
 * - Fuel starvation in high-G corners with low fuel
 * - Integration with economy system for refueling costs
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGFuelConsumptionComponent.generated.h"

class UMGVehicleMovementComponent;
class UMGFuelSubsystem;
class UMGEconomySubsystem;

/**
 * @brief Fuel tank configuration parameters
 *
 * Defines the physical characteristics of the vehicle's fuel tank,
 * including capacity, weight, and pickup geometry for starvation simulation.
 */
USTRUCT(BlueprintType)
struct FMGFuelTankConfiguration
{
	GENERATED_BODY()

	/** Tank capacity in gallons (US) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float CapacityGallons = 15.0f;

	/** Current fuel level in gallons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.0"))
	float CurrentFuelGallons = 15.0f;

	/** Fuel weight per gallon in pounds (gasoline ~6.0 lbs/gal, E85 ~6.6, diesel ~7.1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "5.0", ClampMax = "8.0"))
	float FuelWeightPerGallon = 6.0f;

	/** Tank shape affects starvation behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank")
	bool bHasBaffles = true;

	/** Fuel pickup height from tank bottom in inches (affects when starvation begins) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float PickupHeightInches = 1.0f;

	/** Reserve fuel capacity that triggers low fuel warning (gallons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float ReserveCapacityGallons = 2.0f;

	/** Critical fuel level that may cause starvation (gallons) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank", meta = (ClampMin = "0.25", ClampMax = "2.0"))
	float CriticalLevelGallons = 0.5f;

	/**
	 * @brief Calculate current fuel weight in kilograms
	 * @return Current fuel weight in kg
	 */
	float GetFuelWeightKg() const
	{
		// Convert from lbs to kg (1 lb = 0.453592 kg)
		return CurrentFuelGallons * FuelWeightPerGallon * 0.453592f;
	}

	/**
	 * @brief Get fuel percentage remaining
	 * @return Percentage of fuel remaining (0.0 to 1.0)
	 */
	float GetFuelPercentage() const
	{
		return (CapacityGallons > 0.0f) ? (CurrentFuelGallons / CapacityGallons) : 0.0f;
	}

	/**
	 * @brief Check if fuel is at reserve level
	 * @return True if fuel is at or below reserve level
	 */
	bool IsAtReserve() const
	{
		return CurrentFuelGallons <= ReserveCapacityGallons;
	}

	/**
	 * @brief Check if fuel is critically low
	 * @return True if fuel is at critical starvation-risk level
	 */
	bool IsCritical() const
	{
		return CurrentFuelGallons <= CriticalLevelGallons;
	}
};

/**
 * @brief Driving style metrics for consumption analysis
 *
 * Tracks driver behavior patterns that affect fuel consumption.
 * Aggressive driving (high throttle variance, late braking, etc.) increases consumption.
 */
USTRUCT(BlueprintType)
struct FMGDrivingStyleMetrics
{
	GENERATED_BODY()

	/** Average throttle position over sample window (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageThrottle = 0.0f;

	/** Throttle variance (higher = more aggressive) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float ThrottleVariance = 0.0f;

	/** Time spent at wide-open-throttle (WOT) as percentage */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float WOTPercentage = 0.0f;

	/** Number of hard accelerations per minute */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float HardAccelerationsPerMinute = 0.0f;

	/** Calculated aggression score (0.0 = economical, 1.0 = aggressive) */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AggressionScore = 0.5f;

	/** Fuel consumption multiplier based on driving style */
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float StyleConsumptionMultiplier = 1.0f;
};

/**
 * @brief Fuel starvation state information
 *
 * Tracks conditions that can cause fuel starvation (fuel not reaching engine)
 * due to lateral G-forces and low fuel level.
 */
USTRUCT(BlueprintType)
struct FMGFuelStarvationState
{
	GENERATED_BODY()

	/** Is fuel starvation currently occurring */
	UPROPERTY(BlueprintReadOnly, Category = "Starvation")
	bool bIsStarving = false;

	/** Starvation severity (0.0 = none, 1.0 = complete) */
	UPROPERTY(BlueprintReadOnly, Category = "Starvation")
	float StarvationSeverity = 0.0f;

	/** Current lateral G-force */
	UPROPERTY(BlueprintReadOnly, Category = "Starvation")
	float LateralGForce = 0.0f;

	/** Threshold G-force where starvation begins at current fuel level */
	UPROPERTY(BlueprintReadOnly, Category = "Starvation")
	float StarvationThresholdG = 999.0f;

	/** Time spent in starvation this session */
	UPROPERTY(BlueprintReadOnly, Category = "Starvation")
	float TotalStarvationTime = 0.0f;

	/** Number of starvation events this session */
	UPROPERTY(BlueprintReadOnly, Category = "Starvation")
	int32 StarvationEventCount = 0;

	/**
	 * @brief Calculate power reduction factor due to starvation
	 * @return Power multiplier (1.0 = full power, 0.0 = no fuel)
	 */
	float GetPowerReductionFactor() const
	{
		if (!bIsStarving) return 1.0f;
		return FMath::Max(0.0f, 1.0f - StarvationSeverity);
	}
};

/**
 * @brief Real-time fuel consumption telemetry
 */
USTRUCT(BlueprintType)
struct FMGFuelConsumptionTelemetry
{
	GENERATED_BODY()

	/** Instantaneous consumption rate (gallons per hour) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float InstantGPH = 0.0f;

	/** Average consumption rate (gallons per hour) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float AverageGPH = 0.0f;

	/** Miles per gallon (current) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float InstantMPG = 0.0f;

	/** Miles per gallon (average for session) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float AverageMPG = 0.0f;

	/** Estimated range remaining in miles */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float EstimatedRangeMiles = 0.0f;

	/** Total fuel consumed this session (gallons) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float SessionFuelConsumed = 0.0f;

	/** Total distance traveled this session (miles) */
	UPROPERTY(BlueprintReadOnly, Category = "Telemetry")
	float SessionDistanceMiles = 0.0f;
};

// ==========================================
// DELEGATES
// ==========================================

/** Broadcast when fuel level changes significantly */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelLevelChanged, float, CurrentGallons, float, PercentRemaining);

/** Broadcast when fuel reaches reserve level */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelLowWarning, float, GallonsRemaining);

/** Broadcast when fuel reaches critical level */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelCriticalWarning, float, GallonsRemaining);

/** Broadcast when fuel starvation begins */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelStarvationStarted, float, LateralG, float, FuelPercent);

/** Broadcast when fuel starvation ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFuelStarvationEnded);

/** Broadcast when vehicle runs out of fuel */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFuelEmpty);

/** Broadcast when vehicle weight changes due to fuel consumption */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFuelWeightChanged, float, CurrentWeightKg, float, WeightReductionKg);

/** Broadcast when refueling completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRefuelComplete, float, GallonsAdded, int64, CostCredits);

/**
 * @class UMGFuelConsumptionComponent
 * @brief Vehicle component that simulates realistic fuel consumption
 *
 * Provides comprehensive fuel simulation including:
 * - Physics-based consumption model (throttle, RPM, boost, drag)
 * - Driving style analysis affecting economy
 * - Dynamic weight reduction as fuel is consumed
 * - Fuel starvation simulation in hard corners
 * - Integration with economy system for refueling costs
 *
 * @note This component works in conjunction with UMGVehicleMovementComponent
 *       and UMGFuelSubsystem for a complete fuel simulation system.
 */
UCLASS(ClassGroup = (MidnightGrind), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGFuelConsumptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGFuelConsumptionComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Configure the fuel tank parameters
	 * @param Configuration Tank configuration struct
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Configuration")
	void SetTankConfiguration(const FMGFuelTankConfiguration& Configuration);

	/**
	 * @brief Get current tank configuration
	 * @return Current tank configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Configuration")
	const FMGFuelTankConfiguration& GetTankConfiguration() const { return TankConfig; }

	/**
	 * @brief Set the base consumption rate for this vehicle
	 * @param GallonsPerHour Base consumption rate at idle
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Configuration")
	void SetBaseConsumptionRate(float GallonsPerHour);

	// ==========================================
	// FUEL LEVEL QUERIES
	// ==========================================

	/**
	 * @brief Get current fuel level in gallons
	 * @return Current fuel in gallons
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetCurrentFuelGallons() const { return TankConfig.CurrentFuelGallons; }

	/**
	 * @brief Get current fuel percentage
	 * @return Fuel percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetFuelPercentage() const { return TankConfig.GetFuelPercentage(); }

	/**
	 * @brief Get current fuel weight in kilograms
	 * @return Current fuel weight in kg
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	float GetFuelWeightKg() const { return TankConfig.GetFuelWeightKg(); }

	/**
	 * @brief Check if fuel is at reserve level
	 * @return True if at or below reserve
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	bool IsAtReserve() const { return TankConfig.IsAtReserve(); }

	/**
	 * @brief Check if fuel is critically low
	 * @return True if at critical level
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	bool IsCritical() const { return TankConfig.IsCritical(); }

	/**
	 * @brief Check if tank is empty
	 * @return True if fuel is depleted
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|State")
	bool IsEmpty() const { return TankConfig.CurrentFuelGallons <= 0.0f; }

	// ==========================================
	// CONSUMPTION TELEMETRY
	// ==========================================

	/**
	 * @brief Get current consumption telemetry
	 * @return Consumption telemetry struct
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Telemetry")
	const FMGFuelConsumptionTelemetry& GetConsumptionTelemetry() const { return Telemetry; }

	/**
	 * @brief Get driving style metrics
	 * @return Driving style metrics struct
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Telemetry")
	const FMGDrivingStyleMetrics& GetDrivingStyleMetrics() const { return DrivingStyle; }

	/**
	 * @brief Get fuel starvation state
	 * @return Starvation state struct
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Telemetry")
	const FMGFuelStarvationState& GetStarvationState() const { return StarvationState; }

	/**
	 * @brief Get estimated range in miles
	 * @return Estimated range based on current consumption
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Telemetry")
	float GetEstimatedRangeMiles() const { return Telemetry.EstimatedRangeMiles; }

	// ==========================================
	// FUEL OPERATIONS
	// ==========================================

	/**
	 * @brief Consume fuel (called internally by tick)
	 * @param GallonsToConsume Amount of fuel to consume
	 * @return Actual amount consumed (may be less if tank empties)
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Operations")
	float ConsumeFuel(float GallonsToConsume);

	/**
	 * @brief Add fuel to tank (refueling)
	 * @param GallonsToAdd Amount to add
	 * @return Actual amount added (may be less if tank fills)
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Operations")
	float AddFuel(float GallonsToAdd);

	/**
	 * @brief Fill tank completely
	 * @return Amount of fuel added
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Operations")
	float FillTank();

	/**
	 * @brief Purchase fuel using economy system
	 * @param GallonsToAdd Amount to purchase
	 * @param bFullTank If true, fill the entire tank
	 * @return True if purchase successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Operations")
	bool PurchaseFuel(float GallonsToAdd, bool bFullTank = false);

	/**
	 * @brief Get the cost to add specified fuel
	 * @param GallonsToAdd Amount of fuel
	 * @return Cost in credits
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Operations")
	int64 GetRefuelCost(float GallonsToAdd) const;

	/**
	 * @brief Get cost to fill the tank
	 * @return Cost in credits to fill tank
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Operations")
	int64 GetFillTankCost() const;

	/**
	 * @brief Reset fuel session tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Fuel|Operations")
	void ResetSessionTracking();

	// ==========================================
	// POWER REDUCTION
	// ==========================================

	/**
	 * @brief Get current power reduction from fuel starvation
	 * @return Power multiplier (1.0 = full power, 0.0 = no fuel)
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Power")
	float GetFuelStarvationPowerMultiplier() const;

	/**
	 * @brief Check if fuel starvation is occurring
	 * @return True if fuel starvation is active
	 */
	UFUNCTION(BlueprintPure, Category = "Fuel|Power")
	bool IsFuelStarving() const { return StarvationState.bIsStarving; }

	// ==========================================
	// TUNING PARAMETERS
	// ==========================================

	/** Base fuel consumption at idle (gallons per hour) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Base", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float IdleConsumptionGPH = 0.3f;

	/** Consumption multiplier at wide-open-throttle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Throttle", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float WOTConsumptionMultiplier = 5.0f;

	/** How much RPM affects consumption (0 = none, 1 = linear with RPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Engine", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RPMConsumptionFactor = 0.5f;

	/** Boost consumption multiplier per PSI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Engine", meta = (ClampMin = "0.01", ClampMax = "0.2"))
	float BoostConsumptionPerPSI = 0.05f;

	/** Nitrous consumption multiplier when active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Engine", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float NitrousConsumptionMultiplier = 2.5f;

	/** Maximum driving style consumption penalty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Style", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxAggressionPenalty = 0.3f;

	/** G-force threshold where starvation risk begins at 25% fuel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Starvation", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float StarvationBaseGThreshold = 1.2f;

	/** How quickly starvation effect builds up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Starvation", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float StarvationBuildupRate = 2.0f;

	/** How quickly starvation effect dissipates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Starvation", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float StarvationRecoveryRate = 5.0f;

	/** Price per gallon of fuel in credits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Economy", meta = (ClampMin = "1"))
	int32 FuelPricePerGallon = 5;

	/** Premium fuel price multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Economy", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float PremiumFuelPriceMultiplier = 1.3f;

	/** Racing fuel price multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Economy", meta = (ClampMin = "1.0", ClampMax = "5.0"))
	float RacingFuelPriceMultiplier = 3.0f;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Broadcast when fuel level changes significantly */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelLevelChanged OnFuelLevelChanged;

	/** Broadcast when fuel reaches reserve level */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelLowWarning OnFuelLowWarning;

	/** Broadcast when fuel reaches critical level */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelCriticalWarning OnFuelCriticalWarning;

	/** Broadcast when fuel starvation begins */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelStarvationStarted OnFuelStarvationStarted;

	/** Broadcast when fuel starvation ends */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelStarvationEnded OnFuelStarvationEnded;

	/** Broadcast when vehicle runs out of fuel */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelEmpty OnFuelEmpty;

	/** Broadcast when weight changes due to fuel consumption */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnFuelWeightChanged OnFuelWeightChanged;

	/** Broadcast when refueling completes */
	UPROPERTY(BlueprintAssignable, Category = "Fuel|Events")
	FOnRefuelComplete OnRefuelComplete;

protected:
	// ==========================================
	// INTERNAL UPDATE METHODS
	// ==========================================

	/**
	 * @brief Calculate consumption for this frame
	 * @param DeltaTime Frame delta time
	 * @return Gallons consumed this frame
	 */
	virtual float CalculateFrameConsumption(float DeltaTime);

	/**
	 * @brief Update driving style metrics
	 * @param DeltaTime Frame delta time
	 */
	virtual void UpdateDrivingStyleMetrics(float DeltaTime);

	/**
	 * @brief Update fuel starvation simulation
	 * @param DeltaTime Frame delta time
	 */
	virtual void UpdateFuelStarvation(float DeltaTime);

	/**
	 * @brief Update consumption telemetry
	 * @param DeltaTime Frame delta time
	 * @param FrameConsumption Fuel consumed this frame
	 */
	virtual void UpdateTelemetry(float DeltaTime, float FrameConsumption);

	/**
	 * @brief Update weight effects on vehicle
	 */
	virtual void UpdateWeightEffects();

	/**
	 * @brief Check and broadcast fuel warnings
	 */
	virtual void CheckFuelWarnings();

	/**
	 * @brief Calculate starvation G threshold at current fuel level
	 * @return G-force threshold for starvation
	 */
	float CalculateStarvationThreshold() const;

	/**
	 * @brief Get current lateral G-force
	 * @return Lateral acceleration in G
	 */
	float GetLateralGForce() const;

	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/** Tank configuration */
	UPROPERTY()
	FMGFuelTankConfiguration TankConfig;

	/** Consumption telemetry */
	UPROPERTY()
	FMGFuelConsumptionTelemetry Telemetry;

	/** Driving style metrics */
	UPROPERTY()
	FMGDrivingStyleMetrics DrivingStyle;

	/** Starvation state */
	UPROPERTY()
	FMGFuelStarvationState StarvationState;

	/** Cached movement component reference */
	UPROPERTY()
	TObjectPtr<UMGVehicleMovementComponent> MovementComponent;

	/** Starting fuel weight for weight delta calculation */
	float InitialFuelWeightKg = 0.0f;

	/** Previous fuel percentage for change detection */
	float PreviousFuelPercentage = 1.0f;

	/** Has low warning been triggered this session */
	bool bLowWarningTriggered = false;

	/** Has critical warning been triggered this session */
	bool bCriticalWarningTriggered = false;

	/** Has empty event been triggered */
	bool bEmptyEventTriggered = false;

	/** Throttle history for style analysis (circular buffer) */
	TArray<float> ThrottleHistory;
	int32 ThrottleHistoryIndex = 0;
	static constexpr int32 ThrottleHistorySamples = 60;

	/** Consumption history for averaging (circular buffer) */
	TArray<float> ConsumptionHistory;
	int32 ConsumptionHistoryIndex = 0;
	static constexpr int32 ConsumptionHistorySamples = 30;

	/** Time accumulator for hard acceleration detection */
	float HardAccelTimer = 0.0f;
	int32 HardAccelCount = 0;

	/** Previous velocity for acceleration calculation */
	FVector PreviousVelocity = FVector::ZeroVector;
};
