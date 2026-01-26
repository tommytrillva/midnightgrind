// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGAIRacerController.h"
#include "MGAIRacerSubsystem.generated.h"

class UMGAIDriverProfile;
class UMGAIDriverRoster;
class AMGAIRacerController;

/**
 * Spawn configuration for AI racers
 */
USTRUCT(BlueprintType)
struct FMGAISpawnConfig
{
	GENERATED_BODY()

	/** Number of AI racers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacerCount = 7;

	/** Minimum skill level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSkill = 0.4f;

	/** Maximum skill level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxSkill = 0.9f;

	/** Difficulty modifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float DifficultyModifier = 1.0f;

	/** Enable rubber banding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRubberBanding = true;

	/** Rubber band strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RubberBandStrength = 0.3f;

	/** Include rival driver */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeRival = false;

	/** Specific drivers to include */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UMGAIDriverProfile*> RequiredDrivers;

	/** Vehicle class restriction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClassRestriction;
};

/**
 * Spawned AI racer info
 */
USTRUCT(BlueprintType)
struct FMGAIRacerInfo
{
	GENERATED_BODY()

	/** AI Controller */
	UPROPERTY(BlueprintReadOnly)
	AMGAIRacerController* Controller = nullptr;

	/** Vehicle pawn */
	UPROPERTY(BlueprintReadOnly)
	APawn* Vehicle = nullptr;

	/** Driver profile */
	UPROPERTY(BlueprintReadOnly)
	UMGAIDriverProfile* Profile = nullptr;

	/** Grid position */
	UPROPERTY(BlueprintReadOnly)
	int32 GridPosition = 0;

	/** Current race position */
	UPROPERTY(BlueprintReadOnly)
	int32 RacePosition = 0;

	/** Vehicle ID */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Is active */
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIRacerSpawned, const FMGAIRacerInfo&, RacerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIRacerRemoved, const FMGAIRacerInfo&, RacerInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllAIRacersSpawned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllAIRacersCleared);

/**
 * AI Racer Subsystem
 * Manages spawning and control of all AI racers
 *
 * Features:
 * - Automatic AI spawning
 * - Grid position assignment
 * - Skill-based matchmaking
 * - Racing line distribution
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAIRacerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAIRacerSpawned OnAIRacerSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAIRacerRemoved OnAIRacerRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllAIRacersSpawned OnAllAIRacersSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllAIRacersCleared OnAllAIRacersCleared;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set driver roster */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverRoster(UMGAIDriverRoster* Roster);

	/** Set vehicle blueprint class */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetVehicleClass(TSubclassOf<APawn> VehicleClass);

	/** Set racing line for AI */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine);

	/** Set spawn transforms (grid positions) */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetSpawnTransforms(const TArray<FTransform>& Transforms);

	// ==========================================
	// SPAWNING
	// ==========================================

	/** Spawn AI racers with configuration */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void SpawnAIRacers(const FMGAISpawnConfig& Config);

	/** Spawn single AI racer */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	FMGAIRacerInfo SpawnSingleRacer(UMGAIDriverProfile* Profile, const FTransform& SpawnTransform, int32 GridPosition);

	/** Clear all AI racers */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void ClearAllRacers();

	/** Remove specific racer */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void RemoveRacer(AMGAIRacerController* Controller);

	// ==========================================
	// CONTROL
	// ==========================================

	/** Start all AI racing */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StartAllRacing();

	/** Stop all AI racing */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StopAllRacing();

	/** Pause all AI */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void PauseAllAI(bool bPause);

	/** Set difficulty for all AI */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllDifficulty(float DifficultyMultiplier);

	/** Enable/disable rubber banding for all */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllRubberBanding(bool bEnabled, float Strength = 0.3f);

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get all active racers */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	TArray<FMGAIRacerInfo> GetAllRacers() const { return ActiveRacers; }

	/** Get racer count */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	int32 GetRacerCount() const { return ActiveRacers.Num(); }

	/** Get racer by index */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerByIndex(int32 Index) const;

	/** Get racer by controller */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerByController(AMGAIRacerController* Controller) const;

	/** Get racer in position */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerInPosition(int32 Position) const;

	/** Are all racers spawned */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool AreAllRacersSpawned() const { return bAllSpawned; }

	/** Is any AI still racing */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool IsAnyRacing() const;

	// ==========================================
	// POSITION TRACKING
	// ==========================================

	/** Update race positions (called by race manager) */
	UFUNCTION(BlueprintCallable, Category = "AI|Position")
	void UpdateRacePositions(const TArray<int32>& Positions);

	/** Get AI in lead */
	UFUNCTION(BlueprintPure, Category = "AI|Position")
	FMGAIRacerInfo GetLeadingAI() const;

	/** Get AI closest to player */
	UFUNCTION(BlueprintPure, Category = "AI|Position")
	FMGAIRacerInfo GetClosestToPlayer() const;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Driver roster */
	UPROPERTY()
	UMGAIDriverRoster* DriverRoster;

	/** Vehicle class to spawn */
	UPROPERTY()
	TSubclassOf<APawn> AIVehicleClass;

	/** AI Controller class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AMGAIRacerController> AIControllerClass;

	/** Racing line points */
	TArray<FMGAIRacingLinePoint> RacingLinePoints;

	/** Spawn transforms */
	TArray<FTransform> GridSpawnTransforms;

	// ==========================================
	// STATE
	// ==========================================

	/** Active racers */
	UPROPERTY()
	TArray<FMGAIRacerInfo> ActiveRacers;

	/** Current spawn config */
	FMGAISpawnConfig CurrentConfig;

	/** All racers spawned flag */
	bool bAllSpawned = false;

	/** AI paused flag */
	bool bAIPaused = false;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Select drivers for race */
	TArray<UMGAIDriverProfile*> SelectDriversForRace(const FMGAISpawnConfig& Config);

	/** Assign grid positions */
	void AssignGridPositions(TArray<UMGAIDriverProfile*>& Drivers);

	/** Select vehicle for driver */
	FName SelectVehicleForDriver(UMGAIDriverProfile* Driver, const FMGAISpawnConfig& Config);

	/** Get spawn transform for grid position */
	FTransform GetSpawnTransformForPosition(int32 GridPosition) const;

	/** Cleanup destroyed racers */
	void CleanupDestroyedRacers();
};
