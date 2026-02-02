// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "Customization/MGCustomizationSubsystem.h"
/**
 * @file MG_VHCL_ConfigApplicator.h
 * @brief Vehicle customization system for applying visual and performance configurations.
 *
 * @section Overview
 * This file defines the configuration applicator that handles all vehicle customization
 * in MIDNIGHT GRIND. It manages paint jobs, vinyl/decal layers, tuning parameters,
 * parts installation, and visual modifications like window tint and underglow.
 *
 * @section Architecture
 * The configuration system uses a layered architecture:
 *
 * 1. **FMGVehicleConfig**: Complete vehicle configuration (paint, vinyls, tuning, parts)
 * 2. **UMGVehicleConfigApplicator**: Applies configs to vehicle pawns
 * 3. **Preview Mode**: Temporary changes for garage preview before committing
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Paint Types**: Different paint finishes affect how light reflects:
 * - Solid: Flat, single-color finish
 * - Metallic: Contains metal flakes that sparkle
 * - Pearlescent: Color shifts depending on viewing angle
 * - Matte: Non-reflective, flat finish
 * - Chrome: Mirror-like reflective surface
 *
 * **Dynamic Material Instance**: A runtime copy of a material that can have
 * its parameters modified without affecting the original. Used for changing
 * car colors, adding decals, etc.
 *
 * **Tuning Parameters**: Values that affect vehicle physics without changing parts:
 * - Ride height, spring stiffness, damper settings
 * - Brake bias, differential settings
 * - Tire pressure, camber, toe angles
 *
 * @section Usage Example Usage
 * @code
 * // Create an applicator
 * UMGVehicleConfigApplicator* Applicator = NewObject<UMGVehicleConfigApplicator>();
 *
 * // Apply a complete configuration
 * FMGVehicleConfig Config;
 * Config.Paint.PrimaryColor = FLinearColor::Red;
 * Config.Paint.PaintType = EMGPaintType::Metallic;
 * Config.Tuning.RideHeight = -0.5f; // Lowered
 * Applicator->ApplyFullConfig(VehiclePawn, Config);
 *
 * // Preview mode for garage
 * Applicator->BeginPreview(VehiclePawn);
 * Applicator->ApplyColor(VehiclePawn, FLinearColor::Blue, EMGPaintType::Pearlescent);
 * // Player decides to keep or cancel...
 * Applicator->EndPreview(VehiclePawn, true); // true = keep changes
 * @endcode
 *
 * @see AMGVehiclePawn The vehicle pawn that receives configurations
 * @see UMGVehicleMovementComponent Where tuning affects physics
 */

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Customization/MGCustomizationSubsystem.h"
#include "Data/MGPartsCatalog.h"
#include "MG_VHCL_ConfigApplicator.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class AMGVehiclePawn;
class UMGVehicleMovementComponent;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;

// ============================================================================
// PAINT TYPE ENUMERATION
// ============================================================================

/**
 * @brief Automotive paint finish types.
 *
 * Each paint type has different visual properties that affect how light
 * interacts with the vehicle surface. The paint type is used to configure
 * material parameters like metallic intensity, roughness, and clear coat.
 *
 * **UENUM(BlueprintType)** makes this enum usable in Blueprint graphs
 * and exposes it to the Unreal reflection system.
 */
UENUM(BlueprintType)
enum class EMGPaintType : uint8
{
	Solid,
	Metallic,
	Pearlescent,
	Matte,
	Chrome,
	Chameleon,
	Candy,
	Satin
};

// ============================================================================
// PAINT CONFIGURATION
// ============================================================================

// MOVED TO Customization/MGCustomizationSubsystem.h
// FMGPaintConfig is now defined in the canonical customization subsystem header.
// Include "Customization/MGCustomizationSubsystem.h" to use FMGPaintConfig.
/*
USTRUCT(BlueprintType)
struct FMGPaintConfig
{
	GENERATED_BODY()

	// Primary color
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	// Secondary color (for two-tone)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::White;

	// Paint type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPaintType PaintType = EMGPaintType::Solid;

	// Metallic intensity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MetallicIntensity = 0.0f;

	// Clear coat intensity (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ClearCoatIntensity = 0.5f;

	// Pearl shift color (for pearlescent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PearlShiftColor = FLinearColor::Blue;

	// Is two-tone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTwoTone = false;
};
*/

// ============================================================================
// VINYL/DECAL LAYER
// ============================================================================

/**
 * @brief Single vinyl/decal layer configuration.
 *
 * Vinyls are custom graphics that can be placed on the vehicle body.
 * Multiple layers can be stacked to create complex designs. Each layer
 * has position, scale, rotation, color, and placement settings.
 *
 * Placement values:
 * - 0 = Left side
 * - 1 = Right side
 * - 2 = Both sides
 * - 3 = Hood
 * - 4 = Roof
 * - 5 = Trunk
 */
USTRUCT(BlueprintType)
struct FMGVinylLayer
{
	GENERATED_BODY()

	/** Vinyl asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VinylID;

	/** Position offset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Position = FVector2D::ZeroVector;

	/** Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D Scale = FVector2D(1.0f, 1.0f);

	/** Rotation in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rotation = 0.0f;

	/** Primary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	/** Which side (0 = left, 1 = right, 2 = both, 3 = hood, 4 = roof, 5 = trunk) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Placement = 0;

	/** Mirror on opposite side */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMirrored = true;

	/** Is visible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisible = true;
};

// ============================================================================
// TUNING CONFIGURATION
// ============================================================================

/**
 * @brief Complete vehicle tuning parameters.
 *
 * These parameters adjust vehicle behavior without swapping physical parts.
 * All values are normalized (-1 to +1 or 0 to 1) for consistent UI sliders.
 *
 * @section TuningCategories Tuning Categories
 *
 * **Engine Tuning**: Power output and RPM range adjustments
 * **Transmission Tuning**: Gear ratios and final drive
 * **Suspension Tuning**: Ride height, spring rates, dampers, anti-roll bars
 * **Steering Tuning**: Ratio and sensitivity
 * **Brake Tuning**: Bias and force
 * **Differential Tuning**: Lock percentage and AWD torque split
 * **Tire Tuning**: Pressure and alignment (camber, toe)
 * **Aero Tuning**: Front and rear downforce
 * **NOS Tuning**: Boost strength and duration
 *
 * @note Values are relative adjustments. A PowerAdjust of 0.1 means +10%
 * power from the base vehicle specification.
 */
USTRUCT(BlueprintType)
struct FMGTuningConfig
{
	GENERATED_BODY()

	// ==========================================
	// ENGINE TUNING
	// ==========================================

	/** Engine power adjustment (-100 to +100%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float PowerAdjust = 0.0f;

	/** Torque curve bias (low-end vs high-end) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float TorqueBias = 0.0f;

	/** Rev limiter adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-0.2", ClampMax = "0.2"))
	float RevLimiterAdjust = 0.0f;

	// ==========================================
	// TRANSMISSION TUNING
	// ==========================================

	/** Final drive ratio adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-0.5", ClampMax = "0.5"))
	float FinalDriveAdjust = 0.0f;

	/** Gear spacing (close vs wide ratio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float GearSpacing = 0.0f;

	// ==========================================
	// SUSPENSION TUNING
	// ==========================================

	/** Ride height (-1 = lowered, +1 = raised) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RideHeight = 0.0f;

	/** Spring stiffness (soft to stiff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SpringStiffness = 0.0f;

	/** Damper strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float DamperStrength = 0.0f;

	/** Anti-roll bar stiffness front */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AntiRollFront = 0.5f;

	/** Anti-roll bar stiffness rear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AntiRollRear = 0.5f;

	// ==========================================
	// STEERING TUNING
	// ==========================================

	/** Steering ratio (quick vs slow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float SteeringRatio = 0.0f;

	/** Steering sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float SteeringSensitivity = 1.0f;

	// ==========================================
	// BRAKE TUNING
	// ==========================================

	/** Brake bias (front to rear, 0.5 = balanced) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrakeBias = 0.6f;

	/** Brake force multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float BrakeForce = 1.0f;

	// ==========================================
	// DIFFERENTIAL TUNING
	// ==========================================

	/** Diff lock percentage (0 = open, 1 = locked) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DiffLock = 0.3f;

	/** AWD torque split (0 = rear, 1 = front) - only for AWD */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TorqueSplit = 0.4f;

	// ==========================================
	// TIRE TUNING
	// ==========================================

	/** Tire pressure front (affects grip and wear) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float TirePressureFront = 0.0f;

	/** Tire pressure rear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float TirePressureRear = 0.0f;

	/** Camber front (-1 = negative, +1 = positive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float CamberFront = 0.0f;

	/** Camber rear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float CamberRear = 0.0f;

	/** Toe front */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float ToeFront = 0.0f;

	/** Toe rear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float ToeRear = 0.0f;

	// ==========================================
	// AERO TUNING
	// ==========================================

	/** Downforce front */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DownforceFront = 0.0f;

	/** Downforce rear (from spoiler) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DownforceRear = 0.0f;

	// ==========================================
	// NOS TUNING
	// ==========================================

	/** NOS boost strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float NOSPower = 1.0f;

	/** NOS duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float NOSDuration = 1.0f;
};

// MOVED TO Data/MGPartsCatalog.h
// FMGInstalledPart is now defined in the canonical parts catalog header.
// Include "Data/MGPartsCatalog.h" to use FMGInstalledPart.
/*
USTRUCT(BlueprintType)
struct FMGInstalledPart
{
	GENERATED_BODY()

	// Part ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartID;

	// Slot this part is installed in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotID;

	// Part condition (0-1, affects performance)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Condition = 1.0f;
};
*/

/**
 * Complete vehicle configuration
 */
USTRUCT(BlueprintType)
struct FMGVehicleConfig
{
	GENERATED_BODY()

	/** Vehicle data asset ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/** Paint configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPaintConfig Paint;

	/** Vinyl layers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGVinylLayer> Vinyls;

	/** Tuning parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGTuningConfig Tuning;

	/** Installed parts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGInstalledPart> InstalledParts;

	/** Wheel selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WheelID;

	/** Wheel color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WheelColor = FLinearColor::Gray;

	/** Window tint (0 = clear, 1 = limo) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindowTint = 0.0f;

	/** Headlight color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HeadlightColor = FLinearColor::White;

	/** Taillight color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TaillightColor = FLinearColor::Red;

	/** Underglow color (none if alpha = 0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor UnderglowColor = FLinearColor(0.0f, 1.0f, 1.0f, 0.0f);

	/** License plate text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LicensePlate;
};

/**
 * Delegate for config application
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConfigApplied, AMGVehiclePawn*, Vehicle, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTuningChanged, AMGVehiclePawn*, Vehicle, const FMGTuningConfig&, NewTuning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPaintChanged, AMGVehiclePawn*, Vehicle, const FMGPaintConfig&, NewPaint);

// ============================================================================
// VEHICLE CONFIGURATION APPLICATOR CLASS
// ============================================================================

/**
 * @class UMGVehicleConfigApplicator
 * @brief Applies visual and performance configurations to vehicle pawns.
 *
 * This utility class handles all aspects of vehicle customization, from
 * paint jobs to physics tuning. It supports a preview mode for garage
 * interfaces where players can see changes before committing.
 *
 * @section Features Features
 * - **Paint Application**: Colors, metallic finishes, pearlescent effects
 * - **Tuning Application**: Suspension, brakes, differential, aero settings
 * - **Vinyl Management**: Add, update, remove, and layer decals
 * - **Parts Visuals**: Show installed parts (spoilers, body kits, etc.)
 * - **Preview Mode**: Temporary changes for garage preview
 *
 * @section UnrealMacros Unreal Engine Macro Explanations
 *
 * **UCLASS(BlueprintType)**
 * - BlueprintType: This class can be used as a variable type in Blueprints
 * - Unlike components, this is a UObject that can be created on-demand
 *
 * **DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(...)**
 * - Creates a delegate (event) that Blueprint can bind to
 * - TwoParams means it passes two parameters to bound functions
 * - Dynamic: Can be bound at runtime (not just compile time)
 * - Multicast: Multiple functions can bind to the same delegate
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleConfigApplicator : public UObject
{
	GENERATED_BODY()

public:
	UMGVehicleConfigApplicator();

	// ==========================================
	// FULL CONFIG APPLICATION
	// ==========================================

	/**
	 * Apply complete configuration to vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config")
	bool ApplyFullConfig(AMGVehiclePawn* Vehicle, const FMGVehicleConfig& Config);

	/**
	 * Reset vehicle to stock configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config")
	void ResetToStock(AMGVehiclePawn* Vehicle);

	/**
	 * Get current configuration from vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config")
	FMGVehicleConfig GetCurrentConfig(AMGVehiclePawn* Vehicle) const;

	// ==========================================
	// PAINT APPLICATION
	// ==========================================

	/**
	 * Apply paint configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Paint")
	void ApplyPaint(AMGVehiclePawn* Vehicle, const FMGPaintConfig& PaintConfig);

	/**
	 * Apply single color (quick method)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Paint")
	void ApplyColor(AMGVehiclePawn* Vehicle, FLinearColor Color, EMGPaintType PaintType = EMGPaintType::Metallic);

	/**
	 * Get current paint config
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Config|Paint")
	FMGPaintConfig GetCurrentPaint(AMGVehiclePawn* Vehicle) const;

	// ==========================================
	// VINYL/DECAL APPLICATION
	// ==========================================

	/**
	 * Apply vinyl layers
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Vinyl")
	void ApplyVinyls(AMGVehiclePawn* Vehicle, const TArray<FMGVinylLayer>& Vinyls);

	/**
	 * Add single vinyl layer
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Vinyl")
	int32 AddVinyl(AMGVehiclePawn* Vehicle, const FMGVinylLayer& Vinyl);

	/**
	 * Update vinyl layer
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Vinyl")
	void UpdateVinyl(AMGVehiclePawn* Vehicle, int32 LayerIndex, const FMGVinylLayer& Vinyl);

	/**
	 * Remove vinyl layer
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Vinyl")
	void RemoveVinyl(AMGVehiclePawn* Vehicle, int32 LayerIndex);

	/**
	 * Clear all vinyls
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Vinyl")
	void ClearAllVinyls(AMGVehiclePawn* Vehicle);

	// ==========================================
	// TUNING APPLICATION
	// ==========================================

	/**
	 * Apply tuning configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Tuning")
	void ApplyTuning(AMGVehiclePawn* Vehicle, const FMGTuningConfig& TuningConfig);

	/**
	 * Apply single tuning value
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Tuning")
	void ApplyTuningValue(AMGVehiclePawn* Vehicle, FName ParameterName, float Value);

	/**
	 * Reset tuning to defaults
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Tuning")
	void ResetTuning(AMGVehiclePawn* Vehicle);

	/**
	 * Get current tuning config
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Config|Tuning")
	FMGTuningConfig GetCurrentTuning(AMGVehiclePawn* Vehicle) const;

	// ==========================================
	// PARTS APPLICATION
	// ==========================================

	/**
	 * Apply installed parts (visual and stats)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Parts")
	void ApplyParts(AMGVehiclePawn* Vehicle, const TArray<FMGInstalledPart>& Parts);

	/**
	 * Apply wheel selection
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Parts")
	void ApplyWheels(AMGVehiclePawn* Vehicle, FName WheelID, FLinearColor WheelColor);

	// ==========================================
	// VISUAL CUSTOMIZATION
	// ==========================================

	/**
	 * Apply window tint
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Visual")
	void ApplyWindowTint(AMGVehiclePawn* Vehicle, float TintAmount);

	/**
	 * Apply light colors
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Visual")
	void ApplyLightColors(AMGVehiclePawn* Vehicle, FLinearColor HeadlightColor, FLinearColor TaillightColor);

	/**
	 * Apply underglow
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Visual")
	void ApplyUnderglow(AMGVehiclePawn* Vehicle, FLinearColor Color);

	/**
	 * Apply license plate
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Visual")
	void ApplyLicensePlate(AMGVehiclePawn* Vehicle, const FString& PlateText);

	// ==========================================
	// PREVIEW MODE
	// ==========================================

	/**
	 * Begin preview mode (changes aren't persisted)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Preview")
	void BeginPreview(AMGVehiclePawn* Vehicle);

	/**
	 * End preview and optionally apply changes
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle Config|Preview")
	void EndPreview(AMGVehiclePawn* Vehicle, bool bApplyChanges);

	/**
	 * Is in preview mode
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle Config|Preview")
	bool IsInPreviewMode() const { return bPreviewMode; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Config fully applied */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle Config|Events")
	FOnConfigApplied OnConfigApplied;

	/** Tuning changed */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle Config|Events")
	FOnTuningChanged OnTuningChanged;

	/** Paint changed */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle Config|Events")
	FOnPaintChanged OnPaintChanged;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Create or get dynamic material for vehicle body */
	UMaterialInstanceDynamic* GetOrCreateBodyMaterial(AMGVehiclePawn* Vehicle);

	/** Apply paint parameters to material */
	void SetPaintMaterialParameters(UMaterialInstanceDynamic* Material, const FMGPaintConfig& Paint);

	/** Apply tuning to movement component */
	void ApplyTuningToMovement(UMGVehicleMovementComponent* Movement, const FMGTuningConfig& Tuning);

	/** Calculate stat modifiers from parts */
	void CalculatePartModifiers(const TArray<FMGInstalledPart>& Parts);

private:
	/** Cached configuration before preview */
	FMGVehicleConfig PreviewCachedConfig;

	/** Preview mode active */
	bool bPreviewMode = false;

	/** Vehicle being previewed */
	TWeakObjectPtr<AMGVehiclePawn> PreviewVehicle;

	/** Cached vinyl layers for current vehicle */
	TArray<FMGVinylLayer> CachedVinyls;

	/** Material parameter names */
	static const FName BaseColorParam;
	static const FName SecondaryColorParam;
	static const FName MetallicParam;
	static const FName RoughnessParam;
	static const FName ClearCoatParam;
	static const FName PearlColorParam;
};
