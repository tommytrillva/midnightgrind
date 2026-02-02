// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGGarageSubsystem.h
 * @brief Core garage management subsystem for player vehicle collection
 *
 * The Garage Subsystem is the central hub for all player vehicle ownership and
 * customization in Midnight Grind. It manages the player's collection of vehicles,
 * tracks installed parts, handles paint configurations, and calculates performance
 * statistics.
 *
 * ## Key Responsibilities
 * - **Vehicle Collection**: Adding, removing, selling, and selecting vehicles
 * - **Parts Management**: Installing/removing aftermarket parts with compatibility checks
 * - **Paint System**: Managing multi-layer paint configurations (primary, secondary, accent)
 * - **Statistics**: Calculating Performance Index (PI) and performance class (D-X)
 * - **Vehicle Health**: Tracking wear and damage across all components
 * - **Build Export/Import**: Sharing vehicle configurations via JSON or build codes
 *
 * ## Architecture
 * This is a GameInstanceSubsystem, meaning it persists across level loads and
 * maintains state for the entire game session. Vehicle data is marked with SaveGame
 * for persistence.
 *
 * ## Usage Example
 * @code
 * UMGGarageSubsystem* Garage = GetGameInstance()->GetSubsystem<UMGGarageSubsystem>();
 *
 * // Add a new vehicle
 * FGuid NewVehicleId;
 * FMGGarageResult Result = Garage->AddVehicle(VehicleModelData, NewVehicleId);
 *
 * // Install a part
 * if (Garage->IsPartCompatible(NewVehicleId, TurboPart))
 * {
 *     Garage->InstallPart(NewVehicleId, TurboPart);
 * }
 *
 * // Get performance stats
 * FMGVehicleSpecs Stats = Garage->GetVehicleStats(NewVehicleId);
 * @endcode
 *
 * ## Related Subsystems
 * - UMGTuningSubsystem: Fine-tuning vehicle parameters (suspension, gearing, etc.)
 * - UMGPartsCatalogSubsystem: Part database and pricing lookups
 * - UMGDynoSubsystem: Power measurement and verification
 * - UMGLiveryEditorSubsystem: Visual customization beyond paint
 *
 * @see FMGOwnedVehicle for the primary vehicle data structure
 * @see EMGPartSlot for available customization slots
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/MG_VHCL_Data.h"
#include "Vehicle/MGStatCalculator.h"
#include "Vehicle/MG_VHCL_WearSubsystem.h"
#include "Data/MGPartsCatalog.h"
#include "MGGarageSubsystem.generated.h"

class AMGVehiclePawn;
class UMGVehicleModelData;
class UMGPartData;

// ============================================================================
// PART SLOT ENUMERATION
// ============================================================================

/**
 * @enum EMGPartSlot
 * @brief Defines all available slots where aftermarket parts can be installed
 *
 * Part slots are organized by vehicle system (engine, drivetrain, suspension, etc.).
 * Each slot can hold exactly one part at a time. Some parts may require other
 * parts to be installed first (prerequisites).
 *
 * ## Slot Categories
 * - **Engine**: Core power-producing components (block, head, camshaft, intake, exhaust)
 * - **Forced Induction**: Turbo/supercharger systems for boost
 * - **Drivetrain**: Power delivery (clutch, transmission, differential)
 * - **Suspension**: Handling components (springs, dampers, sway bars)
 * - **Brakes**: Stopping power (rotors, calipers, lines)
 * - **Wheels & Tires**: Contact patch and wheel setup
 * - **Aero**: Downforce and drag management
 * - **Body**: Visual and weight reduction parts
 * - **Special**: Nitrous, roll cage, and other unique systems
 */
UENUM(BlueprintType)
enum class EMGPartSlot : uint8
{
	// ----- Engine Components -----
	/// The main engine block - determines displacement and base power potential
	EngineBlock			UMETA(DisplayName = "Engine Block"),
	/// Cylinder head - affects airflow and rev ceiling
	CylinderHead		UMETA(DisplayName = "Cylinder Head"),
	/// Camshaft - controls valve timing and power band characteristics
	Camshaft			UMETA(DisplayName = "Camshaft"),
	/// Intake manifold - distributes air/fuel mixture to cylinders
	IntakeManifold		UMETA(DisplayName = "Intake Manifold"),
	/// Throttle body - controls air intake volume
	ThrottleBody		UMETA(DisplayName = "Throttle Body"),
	/// Air filter/cold air intake - improves airflow into engine
	AirFilter			UMETA(DisplayName = "Air Filter/Intake"),
	/// Exhaust manifold/headers - collects exhaust gases from cylinders
	ExhaustManifold		UMETA(DisplayName = "Exhaust Manifold/Headers"),
	/// Full exhaust system - cat-back or turbo-back setups
	ExhaustSystem		UMETA(DisplayName = "Exhaust System"),
	/// Fuel injectors - deliver precise fuel amounts
	FuelInjectors		UMETA(DisplayName = "Fuel Injectors"),
	/// Fuel pump - supplies adequate fuel pressure
	FuelPump			UMETA(DisplayName = "Fuel Pump"),
	/// ECU tune - engine management calibration
	ECU					UMETA(DisplayName = "ECU/Tune"),

	// ----- Forced Induction -----
	/// Turbocharger - exhaust-driven forced induction
	Turbo				UMETA(DisplayName = "Turbocharger"),
	/// Supercharger - belt-driven forced induction
	Supercharger		UMETA(DisplayName = "Supercharger"),
	/// Intercooler - cools compressed intake air
	Intercooler			UMETA(DisplayName = "Intercooler"),
	/// Wastegate - controls boost pressure
	Wastegate			UMETA(DisplayName = "Wastegate"),
	/// Blow off valve - relieves pressure between shifts
	BlowOffValve		UMETA(DisplayName = "Blow Off Valve"),

	// ----- Drivetrain -----
	/// Clutch - transfers power to transmission, affects launches
	Clutch				UMETA(DisplayName = "Clutch"),
	/// Transmission - gear ratios and shift characteristics
	Transmission		UMETA(DisplayName = "Transmission"),
	/// Differential - power distribution and lock behavior
	Differential		UMETA(DisplayName = "Differential"),
	/// Driveshaft - power transfer, weight reduction
	Driveshaft			UMETA(DisplayName = "Driveshaft"),

	// ----- Suspension -----
	/// Front springs - affects front ride height and handling
	FrontSprings		UMETA(DisplayName = "Front Springs"),
	/// Rear springs - affects rear ride height and handling
	RearSprings			UMETA(DisplayName = "Rear Springs"),
	/// Front dampers/shocks - controls front suspension movement
	FrontDampers		UMETA(DisplayName = "Front Dampers"),
	/// Rear dampers/shocks - controls rear suspension movement
	RearDampers			UMETA(DisplayName = "Rear Dampers"),
	/// Front anti-roll bar - reduces body roll in corners
	FrontSwayBar		UMETA(DisplayName = "Front Sway Bar"),
	/// Rear anti-roll bar - reduces body roll in corners
	RearSwayBar			UMETA(DisplayName = "Rear Sway Bar"),

	// ----- Brakes -----
	/// Front brake rotors - primary stopping power
	FrontRotors			UMETA(DisplayName = "Front Rotors"),
	/// Rear brake rotors - secondary stopping power
	RearRotors			UMETA(DisplayName = "Rear Rotors"),
	/// Front brake calipers - clamp force on front rotors
	FrontCalipers		UMETA(DisplayName = "Front Calipers"),
	/// Rear brake calipers - clamp force on rear rotors
	RearCalipers		UMETA(DisplayName = "Rear Calipers"),
	/// Brake lines - stainless steel for better pedal feel
	BrakeLines			UMETA(DisplayName = "Brake Lines"),

	// ----- Wheels & Tires -----
	/// Front wheels - affects weight and fitment
	FrontWheels			UMETA(DisplayName = "Front Wheels"),
	/// Rear wheels - affects weight and fitment
	RearWheels			UMETA(DisplayName = "Rear Wheels"),
	/// Front tires - grip and compound selection
	FrontTires			UMETA(DisplayName = "Front Tires"),
	/// Rear tires - grip and compound selection
	RearTires			UMETA(DisplayName = "Rear Tires"),

	// ----- Aerodynamics -----
	/// Front splitter - generates front downforce
	FrontSplitter		UMETA(DisplayName = "Front Splitter"),
	/// Rear wing - generates rear downforce
	RearWing			UMETA(DisplayName = "Rear Wing"),
	/// Rear diffuser - accelerates air under the car
	Diffuser			UMETA(DisplayName = "Diffuser"),
	/// Side skirts - manages airflow along body sides
	SideSkirts			UMETA(DisplayName = "Side Skirts"),

	// ----- Body -----
	/// Hood - venting and weight reduction options
	Hood				UMETA(DisplayName = "Hood"),
	/// Front bumper - aero and visual customization
	FrontBumper			UMETA(DisplayName = "Front Bumper"),
	/// Rear bumper - aero and visual customization
	RearBumper			UMETA(DisplayName = "Rear Bumper"),
	/// Fenders - wider options for larger tires
	Fenders				UMETA(DisplayName = "Fenders"),

	// ----- Special Systems -----
	/// Nitrous oxide system - temporary power boost
	Nitrous				UMETA(DisplayName = "Nitrous System"),
	/// Roll cage - chassis stiffening and safety
	RollCage			UMETA(DisplayName = "Roll Cage"),

	/// No slot specified (used for validation)
	None				UMETA(Hidden)
};

// ============================================================================
// PAINT SYSTEM
// ============================================================================

// EMGPaintFinish - REMOVED (duplicate)
// Canonical definition in: Customization/MGCustomizationSubsystem.h

/**
 * @struct FMGPaintConfiguration
 * @brief Complete paint configuration for a vehicle
 *
 * Stores all paint-related settings including colors, finish type, and
 * material properties. Used by the garage and livery systems.
 *
 * ## Color Layers
 * - **Primary**: Main body color
 * - **Secondary**: Accent panels, mirrors, trim
 * - **Accent**: Small details, stripes, highlights
 */
USTRUCT(BlueprintType)
struct FMGPaintConfiguration
{
	GENERATED_BODY()

	/// Main body color of the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor PrimaryColor = FLinearColor::White;

	/// Secondary color for accent panels and trim
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor SecondaryColor = FLinearColor::Black;

	/// Accent color for details and highlights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor AccentColor = FLinearColor::Red;

	/// Type of paint finish (matte, gloss, metallic, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	EMGPaintFinish FinishType = EMGPaintFinish::Metallic;

	/// Intensity of metallic flakes (0.0 = none, 1.0 = maximum sparkle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	float MetallicIntensity = 0.5f;

	/// Clearcoat layer intensity (0.0 = flat, 1.0 = wet look)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	float ClearcoatIntensity = 0.8f;

	/// Color of metallic flakes (visible in metallic/pearl finishes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor FlakeColor = FLinearColor::White;
};

// ============================================================================
// PART DATA STRUCTURES
// ============================================================================

/**
 * @struct FMGPartData
 * @brief Lightweight part data for garage operations
 *
 * This is a simplified representation of part data used for runtime operations.
 * For full part definitions, see the Parts Catalog subsystem and UMGPartData asset.
 *
 * ## Part Identification
 * Parts are identified by a unique FName PartID (e.g., "TURBO_T3_SMALL").
 * The same part may be compatible with multiple vehicles.
 */
USTRUCT(BlueprintType)
struct FMGPartData
{
	GENERATED_BODY()

	/// Unique identifier for this part (e.g., "INTAKE_COLD_AIR_V1")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartID;

	/// Human-readable name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FText DisplayName;

	/// Brand/manufacturer name (e.g., "HKS", "Garrett", "Brembo")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FText Manufacturer;

	/// Which slot this part installs into
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EMGPartSlot Slot = EMGPartSlot::None;

	/// Quality tier affecting price and performance gains
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EMGPartTier Tier = EMGPartTier::Stock;

	/// Purchase price in credits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	int64 Price = 0;

	/// Performance modifiers applied when installed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FMGPartModifiers Modifiers;

	/// List of vehicles this part can be installed on (empty = universal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	TArray<UMGVehicleModelData*> CompatibleVehicles;

	/// Optional 3D mesh for visual representation in garage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	TSoftObjectPtr<UStaticMesh> VisualMesh;
};

// FMGInstalledPart - MOVED TO Data/MGPartsCatalog.h
// Canonical definition in: Data/MGPartsCatalog.h

// ============================================================================
// OWNED VEHICLE DATA
// ============================================================================

/**
 * @struct FMGOwnedVehicle
 * @brief Complete data for a vehicle owned by the player
 *
 * This is the primary data structure for player vehicles. It contains:
 * - Identity: Unique ID and reference to base vehicle model
 * - Customization: Installed parts and paint configuration
 * - Performance: Calculated PI and class
 * - Statistics: Odometer, race history, investment tracking
 * - Wear State: Condition of all components (tires, engine, brakes, etc.)
 *
 * ## Persistence
 * All properties marked with SaveGame will be saved/loaded automatically.
 * Transient properties like cached stats are recalculated on load.
 *
 * ## Vehicle Health System
 * Each component has a condition percentage (0-100). When condition drops
 * below 50%, performance begins to degrade. Components can be repaired
 * or replaced at shops.
 */
USTRUCT(BlueprintType)
struct FMGOwnedVehicle
{
	GENERATED_BODY()

	/** Unique ID for this vehicle instance */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid VehicleId;

	/** Reference to base vehicle model data asset */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	TSoftObjectPtr<UMGVehicleModelData> VehicleModelData;

	/** Custom name given by player */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FString CustomName;

	/** Installed parts by slot */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	TMap<EMGPartSlot, FMGInstalledPart> InstalledParts;

	/** Current paint configuration */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FMGPaintConfiguration Paint;

	/** Calculated performance index */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 PerformanceIndex = 0;

	/** Performance class (D-X) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGPerformanceClass PerformanceClass = EMGPerformanceClass::D;

	/** Total money invested in this vehicle */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int64 TotalInvestment = 0;

	/** Odometer (total distance driven in cm) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float Odometer = 0.0f;

	/** Number of races completed */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RacesCompleted = 0;

	/** Number of races won */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RacesWon = 0;

	/** Date acquired */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime DateAcquired;

	/** Is this the currently selected vehicle? */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bIsSelected = false;

	// ==========================================
	// WEAR & DAMAGE STATE (Persisted)
	// ==========================================

	/** Tire wear state for all four tires */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FMGTireSetWearData TireWear;

	/** Engine wear and condition */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FMGEngineWearData EngineWear;

	/** Brake condition (0-100%) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	float BrakeCondition = 100.0f;

	/** Clutch condition (0-100%) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	float ClutchCondition = 100.0f;

	/** Transmission condition (0-100%) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	float TransmissionCondition = 100.0f;

	/** Suspension condition (0-100%) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	float SuspensionCondition = 100.0f;

	/** Body damage level (0 = none, 1 = totaled) */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	float BodyDamage = 0.0f;

	/** Has unrepaired damage that affects performance */
	bool HasDamage() const
	{
		return TireWear.GetWorstCondition() < 50.0f ||
			   EngineWear.Condition < 50.0f ||
			   BrakeCondition < 50.0f ||
			   ClutchCondition < 50.0f ||
			   TransmissionCondition < 50.0f ||
			   SuspensionCondition < 50.0f ||
			   BodyDamage > 0.25f;
	}

	/** Overall vehicle health (average of all components) */
	float GetOverallHealth() const
	{
		float Health = TireWear.GetAverageCondition() +
					   EngineWear.Condition +
					   BrakeCondition +
					   ClutchCondition +
					   TransmissionCondition +
					   SuspensionCondition +
					   (1.0f - BodyDamage) * 100.0f;
		return Health / 7.0f;
	}

	FMGOwnedVehicle()
	{
		VehicleId = FGuid::NewGuid();
		DateAcquired = FDateTime::Now();
	}

	bool IsValid() const { return VehicleId.IsValid() && !VehicleModelData.IsNull(); }
};

// ============================================================================
// OPERATION RESULT
// ============================================================================

/**
 * @struct FMGGarageResult
 * @brief Result of a garage operation (install, remove, purchase, etc.)
 *
 * Provides a standardized way to return success/failure status along with
 * relevant information like error messages and costs.
 *
 * ## Usage Pattern
 * @code
 * FMGGarageResult Result = Garage->InstallPart(VehicleId, Part);
 * if (Result.bSuccess)
 * {
 *     UE_LOG(LogGarage, Log, TEXT("Part installed, cost: %lld"), Result.CostOrRefund);
 * }
 * else
 * {
 *     ShowErrorUI(Result.ErrorMessage);
 * }
 * @endcode
 */
USTRUCT(BlueprintType)
struct FMGGarageResult
{
	GENERATED_BODY()

	/// Whether the operation completed successfully
	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	/// Human-readable error message (only valid if bSuccess is false)
	UPROPERTY(BlueprintReadOnly)
	FText ErrorMessage;

	/// Cost charged or refund given (positive = cost, negative = refund)
	UPROPERTY(BlueprintReadOnly)
	int64 CostOrRefund = 0;

	/** Create a successful result with optional cost */
	static FMGGarageResult Success(int64 Cost = 0)
	{
		FMGGarageResult Result;
		Result.bSuccess = true;
		Result.CostOrRefund = Cost;
		return Result;
	}

	/** Create a failure result with error message */
	static FMGGarageResult Failure(const FText& Error)
	{
		FMGGarageResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = Error;
		return Result;
	}
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/** Broadcast when any property of a vehicle changes (parts, paint, stats, etc.) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleChanged, const FGuid&, VehicleId);

/** Broadcast when a part is successfully installed on a vehicle */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartInstalled, const FGuid&, VehicleId, EMGPartSlot, Slot);

/** Broadcast when a part is removed from a vehicle */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartRemoved, const FGuid&, VehicleId, EMGPartSlot, Slot);

/** Broadcast when a new vehicle is added to the garage */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleAdded, const FGuid&, VehicleId);

/** Broadcast when a vehicle is removed from the garage (sold or deleted) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleRemoved, const FGuid&, VehicleId);

/** Broadcast when the player selects a different vehicle for racing */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleSelected, const FGuid&, VehicleId);

// ============================================================================
// GARAGE SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGGarageSubsystem
 * @brief Game Instance Subsystem for managing the player's vehicle garage
 *
 * The garage subsystem is the central authority for all vehicle ownership and
 * customization. It provides a complete API for:
 *
 * - Managing the player's vehicle collection (add, remove, sell, select)
 * - Installing and removing aftermarket parts with compatibility validation
 * - Applying paint configurations
 * - Calculating vehicle performance statistics and PI ratings
 * - Tracking vehicle wear and maintenance state
 * - Exporting/importing vehicle builds for sharing
 * - Spawning configured vehicles into the world
 *
 * ## Thread Safety
 * All operations are designed to run on the game thread. Do not call from
 * worker threads.
 *
 * ## Events
 * Subscribe to delegate events (OnVehicleChanged, OnPartInstalled, etc.) to
 * react to garage state changes in UI and other systems.
 *
 * @see FMGOwnedVehicle for vehicle data structure
 * @see UMGTuningSubsystem for fine-tuning parameters
 */
UCLASS()
class MIDNIGHTGRIND_API UMGGarageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// VEHICLE COLLECTION
	// ==========================================

	/** Add a new vehicle to the garage */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	FMGGarageResult AddVehicle(UMGVehicleModelData* VehicleModelData, FGuid& OutVehicleId);

	/** Remove a vehicle from the garage */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	FMGGarageResult RemoveVehicle(const FGuid& VehicleId);

	/** Sell a vehicle (removes and returns credits) */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	FMGGarageResult SellVehicle(const FGuid& VehicleId, int64& OutSellPrice);

	/** Get all owned vehicles */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	TArray<FMGOwnedVehicle> GetAllVehicles() const;

	/** Get a specific vehicle by ID */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	bool GetVehicle(const FGuid& VehicleId, FMGOwnedVehicle& OutVehicle) const;

	/** Get the currently selected vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	bool GetSelectedVehicle(FMGOwnedVehicle& OutVehicle) const;

	/** Select a vehicle for racing */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	FMGGarageResult SelectVehicle(const FGuid& VehicleId);

	/** Get number of vehicles owned */
	UFUNCTION(BlueprintPure, Category = "Garage")
	int32 GetVehicleCount() const { return OwnedVehicles.Num(); }

	/** Check if player owns a specific base vehicle type */
	UFUNCTION(BlueprintPure, Category = "Garage")
	bool OwnsVehicleType(UMGVehicleModelData* VehicleModelData) const;

	/** Check if player has any vehicles, give starter if not */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	void EnsureStarterVehicle();

	/** Does player have the starter vehicle? */
	UFUNCTION(BlueprintPure, Category = "Garage")
	bool HasStarterVehicle() const { return OwnedVehicles.Num() > 0; }

	/** Add a vehicle by ID (for MVP - creates placeholder data) */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	FMGGarageResult AddVehicleByID(FName VehicleID, FGuid& OutVehicleId);

	// ==========================================
	// CUSTOMIZATION - PARTS
	// ==========================================

	/** Install a part on a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Parts")
	FMGGarageResult InstallPart(const FGuid& VehicleId, const FMGPartData& Part);

	/** Remove a part from a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Parts")
	FMGGarageResult RemovePart(const FGuid& VehicleId, EMGPartSlot Slot, FMGPartData& OutRemovedPart);

	/** Get the part installed in a specific slot */
	UFUNCTION(BlueprintCallable, Category = "Garage|Parts")
	bool GetInstalledPart(const FGuid& VehicleId, EMGPartSlot Slot, FMGInstalledPart& OutPart) const;

	/** Get all installed parts on a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Parts")
	TMap<EMGPartSlot, FMGInstalledPart> GetAllInstalledParts(const FGuid& VehicleId) const;

	/** Check if a part is compatible with a vehicle */
	UFUNCTION(BlueprintPure, Category = "Garage|Parts")
	bool IsPartCompatible(const FGuid& VehicleId, const FMGPartData& Part) const;

	/** Preview stats if a part were installed */
	UFUNCTION(BlueprintCallable, Category = "Garage|Parts")
	FMGVehicleSpecs PreviewPartInstallation(const FGuid& VehicleId, const FMGPartData& Part) const;

	// ==========================================
	// CUSTOMIZATION - PAINT
	// ==========================================

	/** Apply a paint configuration to a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Paint")
	FMGGarageResult ApplyPaint(const FGuid& VehicleId, const FMGPaintConfiguration& Paint);

	/** Apply just the primary color */
	UFUNCTION(BlueprintCallable, Category = "Garage|Paint")
	FMGGarageResult SetPrimaryColor(const FGuid& VehicleId, const FLinearColor& Color);

	/** Apply just the secondary color */
	UFUNCTION(BlueprintCallable, Category = "Garage|Paint")
	FMGGarageResult SetSecondaryColor(const FGuid& VehicleId, const FLinearColor& Color);

	/** Get the current paint configuration */
	UFUNCTION(BlueprintCallable, Category = "Garage|Paint")
	bool GetPaintConfiguration(const FGuid& VehicleId, FMGPaintConfiguration& OutPaint) const;

	// ==========================================
	// STATS & CALCULATIONS
	// ==========================================

	/** Recalculate all stats for a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Stats")
	void RecalculateVehicleStats(const FGuid& VehicleId);

	/** Get calculated stats for a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Stats")
	FMGVehicleSpecs GetVehicleStats(const FGuid& VehicleId) const;

	/** Get the performance index for a vehicle */
	UFUNCTION(BlueprintPure, Category = "Garage|Stats")
	int32 GetPerformanceIndex(const FGuid& VehicleId) const;

	/** Get the performance class for a vehicle */
	UFUNCTION(BlueprintPure, Category = "Garage|Stats")
	EMGPerformanceClass GetPerformanceClass(const FGuid& VehicleId) const;

	/** Calculate sell value for a vehicle */
	UFUNCTION(BlueprintPure, Category = "Garage|Stats")
	int64 CalculateSellValue(const FGuid& VehicleId) const;

	// ==========================================
	// VEHICLE CONFIG EXPORT/IMPORT
	// ==========================================

	/**
	 * Export a vehicle's configuration to a JSON string.
	 * Includes all parts, tuning settings, and paint.
	 * Can be shared with other players to recreate the build.
	 *
	 * @param VehicleId The vehicle to export
	 * @param OutJsonString The exported JSON configuration
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Garage|Export")
	bool ExportVehicleBuild(const FGuid& VehicleId, FString& OutJsonString) const;

	/**
	 * Import a vehicle configuration from a JSON string.
	 * Applies the imported parts, tuning, and paint to an existing vehicle.
	 * Missing parts will be skipped (player must own them).
	 *
	 * @param VehicleId The vehicle to apply the config to
	 * @param JsonString The JSON configuration to import
	 * @param bRequireOwnedParts If true, only install parts the player owns
	 * @return Result with success/failure and any error message
	 */
	UFUNCTION(BlueprintCallable, Category = "Garage|Export")
	FMGGarageResult ImportVehicleBuild(const FGuid& VehicleId, const FString& JsonString, bool bRequireOwnedParts = true);

	/**
	 * Export a vehicle's configuration to a file.
	 * @param VehicleId The vehicle to export
	 * @param FilePath Full path to save the JSON file
	 * @return True if export succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Garage|Export")
	bool ExportVehicleBuildToFile(const FGuid& VehicleId, const FString& FilePath) const;

	/**
	 * Import a vehicle configuration from a file.
	 * @param VehicleId The vehicle to apply the config to
	 * @param FilePath Full path to the JSON file
	 * @param bRequireOwnedParts If true, only install parts the player owns
	 * @return Result with success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Garage|Export")
	FMGGarageResult ImportVehicleBuildFromFile(const FGuid& VehicleId, const FString& FilePath, bool bRequireOwnedParts = true);

	/**
	 * Get a shareable build code (compressed hash).
	 * Short code that can be easily shared and imported.
	 * @param VehicleId The vehicle to generate a code for
	 * @return Short alphanumeric code, or empty if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Garage|Export")
	FString GetBuildCode(const FGuid& VehicleId) const;

	/**
	 * Apply a build from a shared code.
	 * @param VehicleId The vehicle to apply the build to
	 * @param BuildCode The build code to apply
	 * @return Result with success/failure
	 */
	UFUNCTION(BlueprintCallable, Category = "Garage|Export")
	FMGGarageResult ApplyBuildCode(const FGuid& VehicleId, const FString& BuildCode);

	// ==========================================
	// VEHICLE SPAWNING
	// ==========================================

	/** Spawn the selected vehicle at a transform */
	UFUNCTION(BlueprintCallable, Category = "Garage|Spawn")
	AMGVehiclePawn* SpawnSelectedVehicle(const FTransform& SpawnTransform);

	/** Spawn a specific vehicle by ID */
	UFUNCTION(BlueprintCallable, Category = "Garage|Spawn")
	AMGVehiclePawn* SpawnVehicle(const FGuid& VehicleId, const FTransform& SpawnTransform);

	/** Apply customization to an already-spawned vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Spawn")
	void ApplyCustomizationToVehicle(AMGVehiclePawn* Vehicle, const FGuid& VehicleId);

	// ==========================================
	// VEHICLE STATS TRACKING
	// ==========================================

	/** Update odometer for a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage|Tracking")
	void AddOdometerDistance(const FGuid& VehicleId, float DistanceInCm);

	/** Record a race result */
	UFUNCTION(BlueprintCallable, Category = "Garage|Tracking")
	void RecordRaceResult(const FGuid& VehicleId, bool bWon);

	/** Rename a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Garage")
	FMGGarageResult RenameVehicle(const FGuid& VehicleId, const FString& NewName);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when any vehicle property changes */
	UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
	FOnVehicleChanged OnVehicleChanged;

	/** Called when a part is installed */
	UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
	FOnPartInstalled OnPartInstalled;

	/** Called when a part is removed */
	UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
	FOnPartRemoved OnPartRemoved;

	/** Called when a vehicle is added to the garage */
	UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
	FOnVehicleAdded OnVehicleAdded;

	/** Called when a vehicle is removed from the garage */
	UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
	FOnVehicleRemoved OnVehicleRemoved;

	/** Called when a different vehicle is selected */
	UPROPERTY(BlueprintAssignable, Category = "Garage|Events")
	FOnVehicleSelected OnVehicleSelected;

protected:
	/** All vehicles owned by the player */
	UPROPERTY(SaveGame)
	TArray<FMGOwnedVehicle> OwnedVehicles;

	/** Currently selected vehicle ID */
	UPROPERTY(SaveGame)
	FGuid SelectedVehicleId;

	/** Cached calculated stats per vehicle */
	UPROPERTY(Transient)
	TMap<FGuid, FMGVehicleSpecs> CachedStats;

	/** Find vehicle index by ID */
	int32 FindVehicleIndex(const FGuid& VehicleId) const;

	/** Get mutable reference to vehicle (internal use) */
	FMGOwnedVehicle* GetVehicleMutable(const FGuid& VehicleId);

	/** Mark stats cache as dirty */
	void InvalidateStatsCache(const FGuid& VehicleId);

	/** Apply a part's modifiers to vehicle data for stat calculation */
	void ApplyPartToVehicleData(FMGVehicleData& VehicleData, const FMGPartData& Part) const;
};
