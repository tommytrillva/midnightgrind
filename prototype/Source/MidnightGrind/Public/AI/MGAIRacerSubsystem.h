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
 * @brief Spawn configuration for AI racers
 *
 * Configures how AI opponents are spawned and their base behavior.
 * Per GDD Pillar 5 (Unified Challenge), AI uses skill-based catch-up
 * instead of rubber-banding that violates physics.
 */
USTRUCT(BlueprintType)
struct FMGAISpawnConfig
{
	GENERATED_BODY()

	/** Number of AI racers to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 RacerCount = 7;

	/** Minimum skill level for driver selection (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSkill = 0.4f;

	/** Maximum skill level for driver selection (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxSkill = 0.9f;

	/**
	 * Difficulty modifier affecting AI decision quality
	 * 0.5 = Easy (more mistakes), 1.0 = Normal, 1.5 = Hard (optimal)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float DifficultyModifier = 1.0f;

	/**
	 * Enable skill-based catch-up system
	 * When enabled, AI will take calculated risks to catch up or drive
	 * conservatively when leading. This does NOT provide physics cheats.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	bool bEnableSkillBasedCatchUp = true;

	/**
	 * @deprecated Use bEnableSkillBasedCatchUp instead
	 * Legacy rubber banding flag - mapped to skill-based catch-up
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (DeprecatedProperty, DeprecationMessage = "Use bEnableSkillBasedCatchUp instead"))
	bool bEnableRubberBanding = true;

	/**
	 * @deprecated No longer used - skill-based catch-up has fixed behavior
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (DeprecatedProperty, DeprecationMessage = "No longer used"))
	float RubberBandStrength = 0.3f;

	/** Include rival driver (targets player specifically) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bIncludeRival = false;

	/** Specific driver profiles to include in race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<UMGAIDriverProfile*> RequiredDrivers;

	/** Restrict AI to specific vehicle class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
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

	// Tickable requirements for UWorldSubsystem
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMGAIRacerSubsystem, STATGROUP_Tickables); }
	virtual bool IsTickable() const override { return !IsTemplate() && bIsTickEnabled; }
	virtual bool IsTickableWhenPaused() const override { return false; }

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

	/**
	 * Enable/disable skill-based catch-up for all AI
	 * When enabled, AI will take calculated risks to catch up or drive
	 * conservatively when leading. Does NOT provide physics advantages.
	 * @param bEnabled Enable catch-up behavior
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllSkillBasedCatchUp(bool bEnabled);

	/**
	 * @deprecated Use SetAllSkillBasedCatchUp instead
	 * Enable/disable skill-based catch-up for all AI
	 * @param bEnabled Enable catch-up behavior
	 * @param Strength Deprecated - no longer used
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control", meta = (DeprecatedFunction, DeprecationMessage = "Use SetAllSkillBasedCatchUp instead"))
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
	// TICK CONTROL
	// ==========================================

	/** Whether subsystem ticking is enabled */
	bool bIsTickEnabled = true;

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
