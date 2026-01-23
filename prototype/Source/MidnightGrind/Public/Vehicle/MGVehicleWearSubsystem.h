// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGVehicleWearSubsystem.generated.h"

class AMGVehiclePawn;
class UMGEconomySubsystem;

/**
 * Tire wear state
 */
UENUM(BlueprintType)
enum class EMGTireWearState : uint8
{
	New UMETA(DisplayName = "New (100-80%)"),
	Good UMETA(DisplayName = "Good (80-50%)"),
	Worn UMETA(DisplayName = "Worn (50-25%)"),
	Critical UMETA(DisplayName = "Critical (25-10%)"),
	Destroyed UMETA(DisplayName = "Destroyed (<10%)")
};

/**
 * Part condition state
 */
UENUM(BlueprintType)
enum class EMGPartConditionState : uint8
{
	Excellent UMETA(DisplayName = "Excellent (100-90%)"),
	Good UMETA(DisplayName = "Good (90-70%)"),
	Fair UMETA(DisplayName = "Fair (70-50%)"),
	Poor UMETA(DisplayName = "Poor (50-25%)"),
	Critical UMETA(DisplayName = "Critical (<25%)")
};

/**
 * Consumable type
 */
UENUM(BlueprintType)
enum class EMGConsumableType : uint8
{
	Nitrous UMETA(DisplayName = "Nitrous"),
	Fuel UMETA(DisplayName = "Fuel"),
	BrakePads UMETA(DisplayName = "Brake Pads"),
	Tires UMETA(DisplayName = "Tires"),
	EngineOil UMETA(DisplayName = "Engine Oil"),
	Coolant UMETA(DisplayName = "Coolant")
};

/**
 * Individual tire wear data
 */
USTRUCT(BlueprintType)
struct FMGTireWearData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Condition = 100.0f; // 0-100%

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 80.0f; // Celsius (optimal: 80-100)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WearRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireWearState WearState = EMGTireWearState::New;

	// Accumulated stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceKM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftDistanceKM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BurnoutCount = 0;
};

/**
 * Complete tire set wear data
 */
USTRUCT(BlueprintType)
struct FMGTireSetWearData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireWearData FrontLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireWearData FrontRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireWearData RearLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireWearData RearRight;

	// Average condition
	float GetAverageCondition() const
	{
		return (FrontLeft.Condition + FrontRight.Condition + RearLeft.Condition + RearRight.Condition) / 4.0f;
	}

	// Worst tire condition
	float GetWorstCondition() const
	{
		return FMath::Min(FMath::Min(FrontLeft.Condition, FrontRight.Condition),
			FMath::Min(RearLeft.Condition, RearRight.Condition));
	}
};

/**
 * Engine wear data
 */
USTRUCT(BlueprintType)
struct FMGEngineWearData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Condition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 90.0f; // Celsius (optimal: 80-100)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OilLevel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OilCondition = 100.0f; // Degrades over time/distance

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CoolantLevel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RedlineHits = 0; // Hits at rev limiter

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverheatTime = 0.0f; // Seconds spent overheating

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverheating = false;
};

/**
 * Brake wear data
 */
USTRUCT(BlueprintType)
struct FMGBrakeWearData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontPadCondition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearPadCondition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontRotorCondition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RearRotorCondition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FluidCondition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 100.0f; // Can fade when hot

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBrakeFade = false;
};

/**
 * Complete vehicle wear state
 */
USTRUCT(BlueprintType)
struct FMGVehicleWearState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTireSetWearData Tires;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEngineWearData Engine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGBrakeWearData Brakes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitrousRemaining = 100.0f; // Percent of bottle

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelLevel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BodyCondition = 100.0f; // Collision damage

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalMileage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SessionMileage = 0;
};

/**
 * Repair cost estimate
 */
USTRUCT(BlueprintType)
struct FMGRepairEstimate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TireReplacementCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BrakePadCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BrakeRotorCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 OilChangeCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CoolantTopOffCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 EngineRepairCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BodyRepairCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 NitrousRefillCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalCost = 0;

	void CalculateTotal()
	{
		TotalCost = TireReplacementCost + BrakePadCost + BrakeRotorCost +
			OilChangeCost + CoolantTopOffCost + EngineRepairCost + BodyRepairCost + NitrousRefillCost;
	}
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireConditionChanged, FGuid, VehicleID, EMGTireWearState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEngineOverheat, FGuid, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBrakeFade, FGuid, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNitrousEmpty, FGuid, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartFailure, FGuid, VehicleID, FName, PartName);

/**
 * Vehicle Wear and Consumables Subsystem
 * Manages tire wear, engine condition, brake fade, and consumables
 * Per PRD Section 4.5: Economic Sinks
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleWearSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// WEAR TRACKING
	// ==========================================

	/**
	 * Register vehicle for wear tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear")
	void RegisterVehicle(FGuid VehicleID);

	/**
	 * Unregister vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear")
	void UnregisterVehicle(FGuid VehicleID);

	/**
	 * Get current wear state
	 */
	UFUNCTION(BlueprintPure, Category = "Wear")
	bool GetWearState(FGuid VehicleID, FMGVehicleWearState& OutState) const;

	/**
	 * Update wear state from gameplay
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear")
	void UpdateWearFromGameplay(FGuid VehicleID, float DeltaTime, const FVector& Velocity,
		float ThrottleInput, float BrakeInput, float SteeringInput, bool bDrifting, bool bNOSActive);

	// ==========================================
	// TIRE WEAR
	// ==========================================

	/**
	 * Apply tire wear from driving
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Tires")
	void ApplyTireWear(FGuid VehicleID, float WheelSlip, float Speed, bool bDrifting, float DeltaTime);

	/**
	 * Apply tire wear from burnout
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Tires")
	void ApplyBurnoutWear(FGuid VehicleID, float Duration);

	/**
	 * Get tire grip multiplier based on condition
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Tires")
	float GetTireGripMultiplier(FGuid VehicleID) const;

	/**
	 * Get individual tire grip
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Tires")
	float GetIndividualTireGrip(FGuid VehicleID, int32 WheelIndex) const;

	/**
	 * Check if tires need replacement
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Tires")
	bool NeedsTireReplacement(FGuid VehicleID) const;

	// ==========================================
	// ENGINE WEAR
	// ==========================================

	/**
	 * Apply engine wear from driving
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Engine")
	void ApplyEngineWear(FGuid VehicleID, float RPM, float MaxRPM, float Throttle, float DeltaTime);

	/**
	 * Apply redline damage
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Engine")
	void ApplyRedlineDamage(FGuid VehicleID);

	/**
	 * Check engine overheating
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Engine")
	bool IsEngineOverheating(FGuid VehicleID) const;

	/**
	 * Get engine power multiplier based on condition
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Engine")
	float GetEnginePowerMultiplier(FGuid VehicleID) const;

	// ==========================================
	// BRAKE WEAR
	// ==========================================

	/**
	 * Apply brake wear
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Brakes")
	void ApplyBrakeWear(FGuid VehicleID, float BrakeForce, float Speed, float DeltaTime);

	/**
	 * Check for brake fade
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Brakes")
	bool HasBrakeFade(FGuid VehicleID) const;

	/**
	 * Get brake effectiveness multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Brakes")
	float GetBrakeEffectiveness(FGuid VehicleID) const;

	// ==========================================
	// CONSUMABLES
	// ==========================================

	/**
	 * Use nitrous
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Consumables")
	bool UseNitrous(FGuid VehicleID, float Amount);

	/**
	 * Get remaining nitrous
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Consumables")
	float GetNitrousRemaining(FGuid VehicleID) const;

	/**
	 * Refill nitrous (costs money)
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Consumables")
	bool RefillNitrous(FGuid OwnerID, FGuid VehicleID);

	/**
	 * Use fuel
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Consumables")
	void UseFuel(FGuid VehicleID, float Amount);

	/**
	 * Get remaining fuel
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Consumables")
	float GetFuelRemaining(FGuid VehicleID) const;

	// ==========================================
	// COLLISION DAMAGE
	// ==========================================

	/**
	 * Apply collision damage
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Collision")
	void ApplyCollisionDamage(FGuid VehicleID, float ImpactForce, const FVector& ImpactPoint);

	/**
	 * Get body condition
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Collision")
	float GetBodyCondition(FGuid VehicleID) const;

	// ==========================================
	// REPAIRS & MAINTENANCE
	// ==========================================

	/**
	 * Get repair cost estimate
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Repair")
	FMGRepairEstimate GetRepairEstimate(FGuid VehicleID) const;

	/**
	 * Get cost to replace tires
	 */
	UFUNCTION(BlueprintPure, Category = "Wear|Repair")
	int64 GetTireReplacementCost(FGuid VehicleID) const;

	/**
	 * Replace tires
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Repair")
	bool ReplaceTires(FGuid OwnerID, FGuid VehicleID);

	/**
	 * Perform oil change
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Repair")
	bool PerformOilChange(FGuid OwnerID, FGuid VehicleID);

	/**
	 * Replace brake pads
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Repair")
	bool ReplaceBrakePads(FGuid OwnerID, FGuid VehicleID);

	/**
	 * Full repair (all systems)
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Repair")
	bool PerformFullRepair(FGuid OwnerID, FGuid VehicleID);

	/**
	 * Quick repair (body damage only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Wear|Repair")
	bool PerformQuickRepair(FGuid OwnerID, FGuid VehicleID);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Wear|Events")
	FOnTireConditionChanged OnTireConditionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wear|Events")
	FOnEngineOverheat OnEngineOverheat;

	UPROPERTY(BlueprintAssignable, Category = "Wear|Events")
	FOnBrakeFade OnBrakeFade;

	UPROPERTY(BlueprintAssignable, Category = "Wear|Events")
	FOnNitrousEmpty OnNitrousEmpty;

	UPROPERTY(BlueprintAssignable, Category = "Wear|Events")
	FOnPartFailure OnPartFailure;

protected:
	// Tracked vehicles
	UPROPERTY()
	TMap<FGuid, FMGVehicleWearState> VehicleWearStates;

	// Economy subsystem reference
	UPROPERTY()
	UMGEconomySubsystem* EconomySubsystem;

	// ==========================================
	// INTERNAL
	// ==========================================

	EMGTireWearState GetTireWearState(float Condition) const;
	void UpdateTireState(FMGTireWearData& Tire);
	void UpdateEngineState(FMGEngineWearData& Engine, float DeltaTime);
	void UpdateBrakeState(FMGBrakeWearData& Brakes, float DeltaTime);

	// Wear rate constants
	static constexpr float BaseTireWearRate = 0.001f; // % per second at full slip
	static constexpr float DriftWearMultiplier = 3.0f;
	static constexpr float BurnoutWearMultiplier = 10.0f;
	static constexpr float BaseEngineWearRate = 0.0001f; // % per second
	static constexpr float RedlineWearRate = 0.01f; // % per hit
	static constexpr float BaseBrakeWearRate = 0.0005f; // % per second at full brake

	// Temperature constants
	static constexpr float EngineOverheatTemp = 120.0f;
	static constexpr float BrakeFadeTemp = 400.0f;
	static constexpr float TireOverheatTemp = 120.0f;

	// Cost constants (base costs, scaled by part tier)
	static constexpr int64 BaseTireCost = 500;
	static constexpr int64 BaseOilChangeCost = 100;
	static constexpr int64 BaseBrakePadCost = 300;
	static constexpr int64 BaseNitrousRefillCost = 200;
	static constexpr int64 BaseBodyRepairCostPerPercent = 50;
};
