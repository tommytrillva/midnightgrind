// Copyright Midnight Grind. All Rights Reserved.

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
 * FMGVehicleStats Stats = Garage->GetVehicleStats(NewVehicleId);
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

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/MGVehicleData.h"
#include "Vehicle/MGStatCalculator.h"
#include "Vehicle/MGVehicleWearSubsystem.h"
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
	CylinderHead		UMETA(DisplayName = "Cylinder Head"),
	Camshaft			UMETA(DisplayName = "Camshaft"),
	IntakeManifold		UMETA(DisplayName = "Intake Manifold"),
	ThrottleBody		UMETA(DisplayName = "Throttle Body"),
	AirFilter			UMETA(DisplayName = "Air Filter/Intake"),
	ExhaustManifold		UMETA(DisplayName = "Exhaust Manifold/Headers"),
	ExhaustSystem		UMETA(DisplayName = "Exhaust System"),
	FuelInjectors		UMETA(DisplayName = "Fuel Injectors"),
	FuelPump			UMETA(DisplayName = "Fuel Pump"),
	ECU					UMETA(DisplayName = "ECU/Tune"),

	// Forced Induction
	Turbo				UMETA(DisplayName = "Turbocharger"),
	Supercharger		UMETA(DisplayName = "Supercharger"),
	Intercooler			UMETA(DisplayName = "Intercooler"),
	Wastegate			UMETA(DisplayName = "Wastegate"),
	BlowOffValve		UMETA(DisplayName = "Blow Off Valve"),

	// Drivetrain
	Clutch				UMETA(DisplayName = "Clutch"),
	Transmission		UMETA(DisplayName = "Transmission"),
	Differential		UMETA(DisplayName = "Differential"),
	Driveshaft			UMETA(DisplayName = "Driveshaft"),

	// Suspension
	FrontSprings		UMETA(DisplayName = "Front Springs"),
	RearSprings			UMETA(DisplayName = "Rear Springs"),
	FrontDampers		UMETA(DisplayName = "Front Dampers"),
	RearDampers			UMETA(DisplayName = "Rear Dampers"),
	FrontSwayBar		UMETA(DisplayName = "Front Sway Bar"),
	RearSwayBar			UMETA(DisplayName = "Rear Sway Bar"),

	// Brakes
	FrontRotors			UMETA(DisplayName = "Front Rotors"),
	RearRotors			UMETA(DisplayName = "Rear Rotors"),
	FrontCalipers		UMETA(DisplayName = "Front Calipers"),
	RearCalipers		UMETA(DisplayName = "Rear Calipers"),
	BrakeLines			UMETA(DisplayName = "Brake Lines"),

	// Wheels & Tires
	FrontWheels			UMETA(DisplayName = "Front Wheels"),
	RearWheels			UMETA(DisplayName = "Rear Wheels"),
	FrontTires			UMETA(DisplayName = "Front Tires"),
	RearTires			UMETA(DisplayName = "Rear Tires"),

	// Aero
	FrontSplitter		UMETA(DisplayName = "Front Splitter"),
	RearWing			UMETA(DisplayName = "Rear Wing"),
	Diffuser			UMETA(DisplayName = "Diffuser"),
	SideSkirts			UMETA(DisplayName = "Side Skirts"),

	// Body
	Hood				UMETA(DisplayName = "Hood"),
	FrontBumper			UMETA(DisplayName = "Front Bumper"),
	RearBumper			UMETA(DisplayName = "Rear Bumper"),
	Fenders				UMETA(DisplayName = "Fenders"),

	// Special
	Nitrous				UMETA(DisplayName = "Nitrous System"),
	RollCage			UMETA(DisplayName = "Roll Cage"),

	None				UMETA(Hidden)
};

/**
 * Paint finish types
 */
UENUM(BlueprintType)
enum class EMGPaintFinish : uint8
{
	Matte				UMETA(DisplayName = "Matte"),
	Satin				UMETA(DisplayName = "Satin"),
	Gloss				UMETA(DisplayName = "Gloss"),
	Metallic			UMETA(DisplayName = "Metallic"),
	Pearl				UMETA(DisplayName = "Pearl"),
	Chrome				UMETA(DisplayName = "Chrome"),
	Candy				UMETA(DisplayName = "Candy"),
	Chameleon			UMETA(DisplayName = "Chameleon/Color Shift")
};

/**
 * Paint configuration for a vehicle
 */
USTRUCT(BlueprintType)
struct FMGPaintConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor SecondaryColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor AccentColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	EMGPaintFinish FinishType = EMGPaintFinish::Metallic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	float MetallicIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	float ClearcoatIntensity = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Paint")
	FLinearColor FlakeColor = FLinearColor::White;
};

/**
 * Part data struct for garage operations (simplified from UMGPartData)
 */
USTRUCT(BlueprintType)
struct FMGPartData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FText Manufacturer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EMGPartSlot Slot = EMGPartSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EMGPartTier Tier = EMGPartTier::Stock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	int64 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FMGPartModifiers Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	TArray<UMGVehicleModelData*> CompatibleVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	TSoftObjectPtr<UStaticMesh> VisualMesh;
};

/**
 * An installed part with metadata
 */
USTRUCT(BlueprintType)
struct FMGInstalledPart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FMGPartData PartData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FDateTime InstallDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	float Condition = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	int32 MileageAtInstall = 0;

	FMGInstalledPart()
	{
		InstallDate = FDateTime::Now();
	}
};

/**
 * Represents a vehicle owned by the player in their garage
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

/**
 * Result of a garage operation
 */
USTRUCT(BlueprintType)
struct FMGGarageResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	FText ErrorMessage;

	UPROPERTY(BlueprintReadOnly)
	int64 CostOrRefund = 0;

	static FMGGarageResult Success(int64 Cost = 0)
	{
		FMGGarageResult Result;
		Result.bSuccess = true;
		Result.CostOrRefund = Cost;
		return Result;
	}

	static FMGGarageResult Failure(const FText& Error)
	{
		FMGGarageResult Result;
		Result.bSuccess = false;
		Result.ErrorMessage = Error;
		return Result;
	}
};

/** Delegate for garage events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleChanged, const FGuid&, VehicleId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartInstalled, const FGuid&, VehicleId, EMGPartSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPartRemoved, const FGuid&, VehicleId, EMGPartSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleAdded, const FGuid&, VehicleId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleRemoved, const FGuid&, VehicleId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleSelected, const FGuid&, VehicleId);

/**
 * Game Instance Subsystem for managing the player's garage
 * Handles vehicle collection, customization, and stat calculations
 */
UCLASS()
class MIDNIGHTGRIND_API UMGGarageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

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
	FMGVehicleStats PreviewPartInstallation(const FGuid& VehicleId, const FMGPartData& Part) const;

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
	FMGVehicleStats GetVehicleStats(const FGuid& VehicleId) const;

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
	TMap<FGuid, FMGVehicleStats> CachedStats;

	/** Find vehicle index by ID */
	int32 FindVehicleIndex(const FGuid& VehicleId) const;

	/** Get mutable reference to vehicle (internal use) */
	FMGOwnedVehicle* GetVehicleMutable(const FGuid& VehicleId);

	/** Mark stats cache as dirty */
	void InvalidateStatsCache(const FGuid& VehicleId);

	/** Apply a part's modifiers to vehicle data for stat calculation */
	void ApplyPartToVehicleData(FMGVehicleData& VehicleData, const FMGPartData& Part) const;
};
