// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGLODSubsystem.h
 * @brief Level of Detail (LOD) Management System for Performance Optimization
 * @author Midnight Grind Team
 * @version 1.0
 */

#pragma once
/**
 * @file MGLODSubsystem.h
 * @brief Level of Detail (LOD) Management System for Performance Optimization
 *
 * @section overview Overview
 * This subsystem manages Level of Detail (LOD) settings across different object
 * categories in Midnight Grind. LOD is a performance optimization technique where
 * objects farther from the camera use simpler (less detailed) versions to save
 * GPU processing power while maintaining visual quality where it matters most.
 *
 * @section key_concepts Key Concepts for Beginners
 *
 * 1. WHAT IS LOD (Level of Detail)?
 *    LOD is a technique to improve game performance:
 *    - Objects close to the camera use high-detail models (LOD0)
 *    - Objects farther away use progressively simpler models (LOD1-4)
 *    - Objects very far away are completely hidden (Culled)
 *
 *    WHY: A car with 100,000 triangles looks great up close, but at
 *    500 meters away, a 1,000 triangle version looks the same and
 *    renders 100x faster!
 *
 * 2. LOD LEVELS (EMGLODLevel):
 *    @code
 *    Distance from camera:
 *    |--LOD0--|---LOD1---|----LOD2----|-----LOD3-----|------LOD4------|--Culled-->
 *    0m      50m        100m         200m           400m             800m
 *       ^                                                              ^
 *    Full detail                                                 Not rendered
 *    (100% triangles)                                           (0 triangles)
 *    @endcode
 *
 *    - LOD0: Full detail - all triangles, all textures
 *    - LOD1: High - slightly reduced (maybe 70% triangles)
 *    - LOD2: Medium - noticeable reduction (maybe 40%)
 *    - LOD3: Low - simple shapes (maybe 15%)
 *    - LOD4: Very Low - basic silhouettes (maybe 5%)
 *    - Culled: Not rendered at all (0%)
 *
 * 3. OBJECT CATEGORIES (EMGLODCategory):
 *    Different object types can have different LOD distances:
 *    - Vehicle: Cars need detail longer (player looks at them)
 *    - Environment: Buildings can simplify sooner
 *    - Props: Street furniture, barriers, signs
 *    - Characters: Pedestrians, drivers
 *    - Effects: Particles, decals
 *    - UI: 3D UI elements in the world
 *
 * 4. LOD SETTINGS (FMGLODSettings):
 *    Per-category configuration:
 *    - LOD1Distance: Distance where LOD1 kicks in (default 50m)
 *    - LOD2Distance: Distance where LOD2 kicks in (default 100m)
 *    - LOD3Distance: Distance where LOD3 kicks in (default 200m)
 *    - LOD4Distance: Distance where LOD4 kicks in (default 400m)
 *    - CullDistance: Distance where object disappears (default 800m)
 *    - LODBias: Offset all distances (+10 = everything switches later)
 *    - bForceLOD: Override to lock everything at one LOD level
 *
 * 5. LOD BIAS:
 *    A global or per-category offset to LOD distances:
 *    - Positive bias (+10): Objects stay detailed longer (better quality)
 *    - Negative bias (-10): Objects simplify sooner (better performance)
 *    - Use SetGlobalLODBias() for quick quality/performance tradeoff
 *
 * 6. SPEED-BASED LOD SCALING:
 *    When driving fast, objects pass by quickly - you don't notice detail:
 *    - At 200 km/h, objects can use lower LODs (they're blurry anyway)
 *    - At 0 km/h (parked), use full detail
 *    - UpdateSpeedFactor() adjusts LOD distances based on current speed
 *    - This is automatic when bSpeedBasedScaling is true
 *
 * 7. QUALITY PRESETS:
 *    ApplyQualityPreset() applies predefined LOD configurations:
 *    - Level 0: Ultra Low (mobile, old hardware)
 *    - Level 1: Low
 *    - Level 2: Medium
 *    - Level 3: High
 *    - Level 4: Ultra (powerful hardware)
 *
 * 8. LOD STATS (FMGLODStats):
 *    Real-time statistics for debugging and profiling:
 *    - TotalObjects: How many LOD objects exist
 *    - LOD0Count through LOD4Count: Objects at each level
 *    - CulledCount: Objects not being rendered
 *    - TotalTriangles: Current triangle count
 *
 * @section usage_examples Code Examples
 *
 * @code
 * // Get the LOD subsystem
 * UMGLODSubsystem* LOD = GetGameInstance()->GetSubsystem<UMGLODSubsystem>();
 *
 * // Apply a quality preset based on user settings
 * int32 QualityLevel = GameSettings->GetGraphicsQuality(); // 0-4
 * LOD->ApplyQualityPreset(QualityLevel);
 *
 * // Customize LOD distances for vehicles (want more detail)
 * FMGLODSettings VehicleSettings;
 * VehicleSettings.LOD1Distance = 75.0f;  // Keep full detail to 75m
 * VehicleSettings.LOD2Distance = 150.0f;
 * VehicleSettings.LOD3Distance = 300.0f;
 * VehicleSettings.LOD4Distance = 500.0f;
 * VehicleSettings.CullDistance = 1000.0f;
 * LOD->SetLODSettings(EMGLODCategory::Vehicle, VehicleSettings);
 *
 * // Customize LOD for environment (can be less detailed)
 * FMGLODSettings EnvSettings;
 * EnvSettings.LOD1Distance = 30.0f;  // Simplify buildings sooner
 * EnvSettings.LOD2Distance = 60.0f;
 * EnvSettings.LOD3Distance = 120.0f;
 * EnvSettings.LOD4Distance = 250.0f;
 * EnvSettings.CullDistance = 500.0f;
 * LOD->SetLODSettings(EMGLODCategory::Environment, EnvSettings);
 *
 * // Force all props to LOD2 for debugging
 * FMGLODSettings DebugSettings = LOD->GetLODSettings(EMGLODCategory::Props);
 * DebugSettings.bForceLOD = true;
 * DebugSettings.ForcedLOD = EMGLODLevel::LOD2;
 * LOD->SetLODSettings(EMGLODCategory::Props, DebugSettings);
 *
 * // Enable speed-based scaling
 * LOD->SetSpeedBasedLODScaling(true);
 *
 * // Call every frame with current vehicle speed
 * void AMyVehicle::Tick(float MGDeltaTime)
 * {
 *     float SpeedKPH = GetVelocity().Size() * 0.036f; // Convert to km/h
 *     LOD->UpdateSpeedFactor(SpeedKPH);
 * }
 *
 * // Calculate LOD level for a specific object
 * float DistanceToCamera = (ObjectLocation - CameraLocation).Size();
 * EMGLODLevel Level = LOD->CalculateLOD(EMGLODCategory::Vehicle, DistanceToCamera);
 *
 * // Display LOD stats for debugging
 * FMGLODStats Stats = LOD->GetLODStats();
 * UE_LOG(LogGame, Log, TEXT("LOD0: %d, Culled: %d, Triangles: %lld"),
 *        Stats.LOD0Count, Stats.CulledCount, Stats.TotalTriangles);
 *
 * // Listen for settings changes
 * LOD->OnLODSettingsChanged.AddDynamic(this, &MyClass::HandleLODChanged);
 * @endcode
 *
 * @section performance_tips Performance Tips
 * - Higher LOD distances = better quality but worse performance
 * - Lower cull distances = fewer objects rendered = better performance
 * - Speed-based scaling is free performance during gameplay
 * - Monitor FMGLODStats to find bottlenecks
 * - Vehicles at LOD0/LOD1 are usually the most expensive objects
 *
 * @see UMGGraphicsSubsystem - Overall graphics quality settings
 * @see UMGAssetCacheSubsystem - Asset loading that works with LOD
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLODSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGLODLevel : uint8
{
	LOD0,  // Full detail
	LOD1,  // High
	LOD2,  // Medium
	LOD3,  // Low
	LOD4,  // Very Low
	Culled // Not rendered
};

UENUM(BlueprintType)
enum class EMGLODCategory : uint8
{
	Vehicle,
	Environment,
	Props,
	Characters,
	Effects,
	UI
};

USTRUCT(BlueprintType)
struct FMGLODSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float LOD1Distance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float LOD2Distance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float LOD3Distance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float LOD4Distance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float CullDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	float LODBias = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	bool bForceLOD = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
	EMGLODLevel ForcedLOD = EMGLODLevel::LOD0;
};

USTRUCT(BlueprintType)
struct FMGLODStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 TotalObjects = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 LOD0Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 LOD1Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 LOD2Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 LOD3Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 LOD4Count = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int32 CulledCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "LOD|Stats")
	int64 TotalTriangles = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLODSettingsChanged, EMGLODCategory, Category, const FMGLODSettings&, Settings);

UCLASS()
class MIDNIGHTGRIND_API UMGLODSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Settings per category
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetLODSettings(EMGLODCategory Category, const FMGLODSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "LOD")
	FMGLODSettings GetLODSettings(EMGLODCategory Category) const;

	// Quality presets
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void ApplyQualityPreset(int32 QualityLevel);

	// Runtime LOD control
	UFUNCTION(BlueprintPure, Category = "LOD")
	EMGLODLevel CalculateLOD(EMGLODCategory Category, float Distance) const;

	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetGlobalLODBias(float Bias);

	UFUNCTION(BlueprintPure, Category = "LOD")
	float GetGlobalLODBias() const { return GlobalLODBias; }

	// Distance scaling for speed
	UFUNCTION(BlueprintCallable, Category = "LOD")
	void SetSpeedBasedLODScaling(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "LOD")
	void UpdateSpeedFactor(float CurrentSpeed);

	// Stats
	UFUNCTION(BlueprintPure, Category = "LOD")
	FMGLODStats GetLODStats() const { return CurrentStats; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "LOD|Events")
	FMGOnLODSettingsChanged OnLODSettingsChanged;

protected:
	void UpdateStats();

private:
	UPROPERTY()
	TMap<EMGLODCategory, FMGLODSettings> CategorySettings;

	FMGLODStats CurrentStats;
	float GlobalLODBias = 0.0f;
	float SpeedFactor = 1.0f;
	bool bSpeedBasedScaling = true;
};
