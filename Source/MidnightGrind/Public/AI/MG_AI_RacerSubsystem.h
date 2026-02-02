// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MG_AI_RacerSubsystem.h
 * @brief AI Racer Management Subsystem - Central hub for all AI opponents
 *
 * This file defines the UMGAIRacerSubsystem class, which is a "World Subsystem"
 * responsible for managing ALL AI opponents in a race. Think of it as the
 * "AI Manager" that handles spawning, tracking, and controlling every computer-
 * controlled racer in the game.
 *
 * @section concepts KEY CONCEPTS FOR NEW DEVELOPERS
 *
 * @subsection world_subsystem WORLD SUBSYSTEM
 * - A World Subsystem is an Unreal Engine feature that creates ONE instance
 *   of a class per game world (level). It automatically initializes when the
 *   level loads and cleans up when the level unloads.
 * - This is perfect for race management because we need exactly one manager
 *   coordinating all AI racers in each race/level.
 * - Access it from anywhere with: GetWorld()->GetSubsystem<UMGAIRacerSubsystem>()
 *
 * @subsection driver_profiles DRIVER PROFILES
 * - Each AI racer has a "Driver Profile" (UMGAIDriverProfile) that defines
 *   their personality, skill level, aggression, and driving style.
 * - This creates varied and interesting opponents rather than identical bots.
 *
 * @subsection skill_catchup SKILL-BASED CATCH-UP (NOT RUBBER BANDING)
 * - Traditional racing games use "rubber banding" where AI cars magically
 *   get faster when behind - this feels unfair to players.
 * - Midnight Grind uses "skill-based catch-up" instead: AI takes more risks
 *   when behind and drives conservatively when ahead. No physics cheats!
 * - This aligns with GDD Pillar 5: "Unified Challenge" - AI follows the
 *   same physics rules as the player.
 *
 * @subsection grid_positions GRID POSITIONS
 * - "Grid position" refers to where each racer starts on the starting grid.
 * - Position 0 = Pole position (front of the grid), higher numbers = further back.
 *
 * @section architecture HOW IT FITS INTO THE GAME ARCHITECTURE
 * @verbatim
 *   [Race GameMode]
 *         |
 *         v
 *   [UMGAIRacerSubsystem] <-- You are here
 *         |
 *         +---> [AMGAIRacerController] (one per AI car)
 *         |           |
 *         |           v
 *         |     [Vehicle Pawn] (the actual car)
 *         |
 *         +---> [UMGAIDriverProfile] (personality data)
 *         |
 *         +---> [Racing Line Data] (path to follow)
 * @endverbatim
 *
 * @section usage TYPICAL USAGE FLOW
 * 1. Race GameMode gets the subsystem: GetWorld()->GetSubsystem<UMGAIRacerSubsystem>()
 * 2. Configure it: SetDriverRoster(), SetVehicleClass(), SetRacingLine()
 * 3. Set spawn positions: SetSpawnTransforms()
 * 4. Spawn AI: SpawnAIRacers(Config)
 * 5. Start race: StartAllRacing()
 * 6. During race: Subsystem auto-updates positions via Tick()
 * 7. End race: StopAllRacing(), ClearAllRacers()
 *
 * @code
 * // In your Race GameMode:
 * UMGAIRacerSubsystem* AISubsystem = GetWorld()->GetSubsystem<UMGAIRacerSubsystem>();
 *
 * FMGAISpawnConfig Config;
 * Config.RacerCount = 7;           // 7 AI opponents
 * Config.MinSkill = 0.5f;          // Medium to high skill
 * Config.MaxSkill = 0.9f;
 * Config.DifficultyModifier = 1.0f; // Normal difficulty
 * Config.bEnableSkillBasedCatchUp = true;
 *
 * AISubsystem->SpawnAIRacers(Config);
 * AISubsystem->StartAllRacing();
 * @endcode
 *
 * @see UMGAIDriverProfile - Driver personality data asset
 * @see AMGAIRacerController - The AI controller for each racer
 * @see FMGAISpawnConfig - Spawn configuration struct
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGAIRacerController.h"
#include "MG_AI_RacerSubsystem.generated.h"

// Forward declarations - tells the compiler these classes exist without including their headers
// This speeds up compilation and reduces dependencies
class UMGAIDriverProfile;
class UMGAIDriverRoster;
class AMGAIRacerController;

/**
 * Configuration for spawning AI racers
 *
 * This struct bundles all the settings needed to spawn a group of AI opponents.
 * It's passed to SpawnAIRacers() to control how many opponents spawn and how
 * they should behave.
 *
 * Design Note (GDD Reference):
 * Per GDD Pillar 5 (Unified Challenge), AI uses skill-based catch-up instead of
 * rubber-banding that violates physics. This means AI opponents follow the same
 * physics rules as the player - they just make smarter or riskier decisions.
 */
USTRUCT(BlueprintType)
struct FMGAISpawnConfig
{
	GENERATED_BODY()

	/** Number of AI racers to spawn (typical: 5-11 for full grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 RacerCount = 7;

	/** Minimum skill level for driver selection (0.0 = beginner, 1.0 = pro) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSkill = 0.4f;

	/** Maximum skill level for driver selection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxSkill = 0.9f;

	/**
	 * Difficulty modifier affecting AI decision quality
	 * 0.5 = Easy (more mistakes), 1.0 = Normal, 1.5 = Hard (optimal decisions)
	 * Affects decision-making only, not physics
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float DifficultyModifier = 1.0f;

	/**
	 * Enable skill-based catch-up system
	 * AI takes calculated risks when behind, drives conservatively when leading
	 * Does NOT provide physics advantages - just smarter decisions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	bool bEnableSkillBasedCatchUp = true;

	/** @deprecated Use bEnableSkillBasedCatchUp instead */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (DeprecatedProperty, DeprecationMessage = "Use bEnableSkillBasedCatchUp instead"))
	bool bEnableRubberBanding = true;

	/** @deprecated No longer used - skill-based catch-up has fixed behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (DeprecatedProperty, DeprecationMessage = "No longer used"))
	float RubberBandStrength = 0.3f;

	/** Include a rival driver that specifically targets the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bIncludeRival = false;

	/** Specific driver profiles that MUST be included in the race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<UMGAIDriverProfile*> RequiredDrivers;

	/** Restrict AI to a specific vehicle class (e.g., "Muscle", "Import") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FName VehicleClassRestriction;
};

/**
 * Runtime information about a spawned AI racer
 *
 * This struct holds all the live data about an AI racer that's currently
 * in the race. It's created when an AI spawns and updated throughout the race.
 * Use this to query information about AI racers (positions, vehicles, etc.)
 */
USTRUCT(BlueprintType)
struct FMGAIRacerInfo
{
	GENERATED_BODY()

	/** The AI Controller driving this racer (the "brain") */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	AMGAIRacerController* Controller = nullptr;

	/** The Vehicle Pawn (the actual car actor in the world) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	APawn* Vehicle = nullptr;

	/** The driver's personality/skill profile */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	UMGAIDriverProfile* Profile = nullptr;

	/** Starting grid position (0 = pole position) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	int32 GridPosition = 0;

	/** Current race position (1 = first place, updated during race) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	int32 RacePosition = 0;

	/** The ID of the vehicle model being used (e.g., "Mustang_69") */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	FName VehicleID;

	/** Whether this racer is currently active (false = crashed out/finished) */
	UPROPERTY(BlueprintReadOnly, Category = "Racer Info")
	bool bIsActive = false;
};

/**
 * @name Event Delegates
 * Delegates for AI racer lifecycle events.
 * Blueprint users can bind in Event Graph, C++ uses AddDynamic().
 * @{
 */

/** Fired when a single AI racer is spawned */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIRacerSpawned, const FMGAIRacerInfo&, RacerInfo);

/** Fired when an AI racer is removed from the race */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIRacerRemoved, const FMGAIRacerInfo&, RacerInfo);

/** Fired when ALL requested AI racers have finished spawning */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllAIRacersSpawned);

/** Fired when all AI racers have been cleared */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllAIRacersCleared);

/** @} */

/**
 * AI Racer Management Subsystem
 *
 * This World Subsystem is the central hub for all AI racer management.
 * One instance exists per game world (level).
 *
 * Responsibilities:
 * - Spawning AI racers from driver profiles
 * - Assigning grid positions
 * - Distributing racing lines to AI controllers
 * - Tracking race positions for all AI
 * - Skill-based matchmaking (selecting appropriate opponents)
 * - Starting/stopping/pausing AI racing behavior
 *
 * Access Pattern:
 * @code
 * UMGAIRacerSubsystem* AISubsystem = GetWorld()->GetSubsystem<UMGAIRacerSubsystem>();
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAIRacerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	//~ End USubsystem Interface

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float MGDeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMGAIRacerSubsystem, STATGROUP_Tickables); }
	virtual bool IsTickable() const override { return !IsTemplate() && bIsTickEnabled; }
	virtual bool IsTickableWhenPaused() const override { return false; }
	//~ End FTickableGameObject Interface

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fires when an AI racer spawns */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAIRacerSpawned OnAIRacerSpawned;

	/** Fires when an AI racer is removed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAIRacerRemoved OnAIRacerRemoved;

	/** Fires when all AI are spawned */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllAIRacersSpawned OnAllAIRacersSpawned;

	/** Fires when all AI are cleared */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllAIRacersCleared OnAllAIRacersCleared;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * Set the roster of available drivers
	 * @param Roster Data asset containing all available AI drivers
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverRoster(UMGAIDriverRoster* Roster);

	/**
	 * Set the vehicle Blueprint class to spawn for AI
	 * @param VehicleClass The Pawn class to spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetVehicleClass(TSubclassOf<APawn> VehicleClass);

	/**
	 * Set the racing line for AI to follow
	 * @param RacingLine Array of racing line points
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine);

	/**
	 * Set the spawn transforms (grid positions)
	 * @param Transforms Array of world transforms for spawn locations
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetSpawnTransforms(const TArray<FTransform>& Transforms);

	// ==========================================
	// SPAWNING
	// ==========================================

	/**
	 * Spawn multiple AI racers based on configuration
	 * @param Config Spawn configuration (count, skill range, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void SpawnAIRacers(const FMGAISpawnConfig& Config);

	/**
	 * Spawn a single AI racer with specific parameters
	 * @param Profile The driver profile to use
	 * @param SpawnTransform Where to spawn the vehicle
	 * @param GridPosition Starting grid position
	 * @return Info struct for the spawned racer
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	FMGAIRacerInfo SpawnSingleRacer(UMGAIDriverProfile* Profile, const FTransform& SpawnTransform, int32 GridPosition);

	/** Remove all AI racers from the race */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void ClearAllRacers();

	/**
	 * Remove a specific racer by their controller
	 * @param Controller The AI controller to remove
	 */
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

	/**
	 * Pause or unpause all AI
	 * @param bPause True to pause, false to resume
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void PauseAllAI(bool bPause);

	/**
	 * Adjust difficulty for all AI mid-race
	 * @param DifficultyMultiplier New difficulty (0.5=easy, 1.0=normal, 1.5=hard)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllDifficulty(float DifficultyMultiplier);

	/**
	 * Enable/disable skill-based catch-up for all AI
	 * @param bEnabled Whether to enable catch-up behavior
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllSkillBasedCatchUp(bool bEnabled);

	/** @deprecated Use SetAllSkillBasedCatchUp instead */
	UFUNCTION(BlueprintCallable, Category = "AI|Control", meta = (DeprecatedFunction, DeprecationMessage = "Use SetAllSkillBasedCatchUp instead"))
	void SetAllRubberBanding(bool bEnabled, float Strength = 0.3f);

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get array of all active AI racers */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	TArray<FMGAIRacerInfo> GetAllRacers() const { return ActiveRacers; }

	/** Get count of active AI racers */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	int32 GetRacerCount() const { return ActiveRacers.Num(); }

	/** Get a specific racer by array index */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerByIndex(int32 Index) const;

	/** Get a racer by their AI controller */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerByController(AMGAIRacerController* Controller) const;

	/** Get the racer currently in a specific race position */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerInPosition(int32 Position) const;

	/** Check if all AI spawning is complete */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool AreAllRacersSpawned() const { return bAllSpawned; }

	/** Check if any AI is still actively racing */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool IsAnyRacing() const;

	// ==========================================
	// POSITION TRACKING
	// ==========================================

	/**
	 * Update race positions for all AI
	 * @param Positions Array of new positions indexed by racer
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Position")
	void UpdateRacePositions(const TArray<int32>& Positions);

	/** Get the AI currently in first place */
	UFUNCTION(BlueprintPure, Category = "AI|Position")
	FMGAIRacerInfo GetLeadingAI() const;

	/** Get the AI closest to the player's vehicle */
	UFUNCTION(BlueprintPure, Category = "AI|Position")
	FMGAIRacerInfo GetClosestToPlayer() const;

protected:
	// ==========================================
	// TICK CONTROL
	// ==========================================

	/** Master switch for enabling/disabling subsystem ticking */
	bool bIsTickEnabled = true;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** The roster of available AI drivers to select from */
	UPROPERTY()
	UMGAIDriverRoster* DriverRoster;

	/** The vehicle Blueprint class to spawn for AI */
	UPROPERTY()
	TSubclassOf<APawn> AIVehicleClass;

	/** The AI Controller class to use */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AMGAIRacerController> AIControllerClass;

	/** The racing line all AI will follow */
	TArray<FMGAIRacingLinePoint> RacingLinePoints;

	/** Grid spawn positions (one transform per AI car) */
	TArray<FTransform> GridSpawnTransforms;

	// ==========================================
	// STATE
	// ==========================================

	/** Array of all currently active AI racers */
	UPROPERTY()
	TArray<FMGAIRacerInfo> ActiveRacers;

	/** The configuration used for the current spawn */
	FMGAISpawnConfig CurrentConfig;

	/** True once all AI have finished spawning */
	bool bAllSpawned = false;

	/** True if AI is currently paused */
	bool bAIPaused = false;

	// ==========================================
	// INTERNAL HELPERS
	// ==========================================

	/** Select drivers from the roster that match the spawn config */
	TArray<UMGAIDriverProfile*> SelectDriversForRace(const FMGAISpawnConfig& Config);

	/** Assign grid positions to the selected drivers */
	void AssignGridPositions(TArray<UMGAIDriverProfile*>& Drivers);

	/** Select an appropriate vehicle for a driver */
	FName SelectVehicleForDriver(UMGAIDriverProfile* Driver, const FMGAISpawnConfig& Config);

	/** Get the world transform for a specific grid position */
	FTransform GetSpawnTransformForPosition(int32 GridPosition) const;

	/** Clean up any racers whose vehicles have been destroyed */
	void CleanupDestroyedRacers();
};
