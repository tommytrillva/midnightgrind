// Copyright Midnight Grind. All Rights Reserved.

/*
 * =============================================================================
 * MGAIRacerSubsystem.h - AI Racer Management Subsystem
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the UMGAIRacerSubsystem class, which is a "World Subsystem"
 * responsible for managing ALL AI opponents in a race. Think of it as the
 * "AI Manager" that handles spawning, tracking, and controlling every computer-
 * controlled racer in the game.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. WORLD SUBSYSTEM:
 *    - A World Subsystem is an Unreal Engine feature that creates ONE instance
 *      of a class per game world (level). It automatically initializes when the
 *      level loads and cleans up when the level unloads.
 *    - This is perfect for race management because we need exactly one manager
 *      coordinating all AI racers in each race/level.
 *    - Access it from anywhere with: GetWorld()->GetSubsystem<UMGAIRacerSubsystem>()
 *
 * 2. DRIVER PROFILES:
 *    - Each AI racer has a "Driver Profile" (UMGAIDriverProfile) that defines
 *      their personality, skill level, aggression, and driving style.
 *    - This creates varied and interesting opponents rather than identical bots.
 *
 * 3. SKILL-BASED CATCH-UP (NOT RUBBER BANDING):
 *    - Traditional racing games use "rubber banding" where AI cars magically
 *      get faster when behind - this feels unfair to players.
 *    - Midnight Grind uses "skill-based catch-up" instead: AI takes more risks
 *      when behind and drives conservatively when ahead. No physics cheats!
 *    - This aligns with GDD Pillar 5: "Unified Challenge" - AI follows the
 *      same physics rules as the player.
 *
 * 4. GRID POSITIONS:
 *    - "Grid position" refers to where each racer starts on the starting grid.
 *    - Position 0 = Pole position (front of the grid), higher numbers = further back.
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
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
 *
 * TYPICAL USAGE FLOW:
 * -------------------
 * 1. Race GameMode gets the subsystem: GetWorld()->GetSubsystem<UMGAIRacerSubsystem>()
 * 2. Configure it: SetDriverRoster(), SetVehicleClass(), SetRacingLine()
 * 3. Set spawn positions: SetSpawnTransforms()
 * 4. Spawn AI: SpawnAIRacers(Config)
 * 5. Start race: StartAllRacing()
 * 6. During race: Subsystem auto-updates positions via Tick()
 * 7. End race: StopAllRacing(), ClearAllRacers()
 *
 * EXAMPLE USAGE (Blueprint/C++):
 * ------------------------------
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
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGAIRacerController.h"
#include "MGAIRacerSubsystem.generated.h"

// Forward declarations - tells the compiler these classes exist without including their headers
// This speeds up compilation and reduces dependencies
class UMGAIDriverProfile;
class UMGAIDriverRoster;
class AMGAIRacerController;

/*
 * FMGAISpawnConfig - Configuration for spawning AI racers
 * --------------------------------------------------------
 * This struct bundles all the settings needed to spawn a group of AI opponents.
 * It's passed to SpawnAIRacers() to control how many opponents spawn and how
 * they should behave.
 *
 * DESIGN NOTE (GDD Reference):
 * Per GDD Pillar 5 (Unified Challenge), AI uses skill-based catch-up instead of
 * rubber-banding that violates physics. This means AI opponents follow the same
 * physics rules as the player - they just make smarter or riskier decisions.
 */
USTRUCT(BlueprintType)
struct FMGAISpawnConfig
{
	GENERATED_BODY()

	/*
	 * Number of AI racers to spawn.
	 * Typical values: 5-11 for a full race grid
	 * Performance consideration: More AI = more CPU usage
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 RacerCount = 7;

	/*
	 * Minimum skill level for driver selection.
	 * Range: 0.0 (beginner) to 1.0 (pro)
	 *
	 * The subsystem will only select drivers from the roster whose skill
	 * level falls between MinSkill and MaxSkill. This lets you create
	 * appropriate competition for the player's current skill level.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinSkill = 0.4f;

	/*
	 * Maximum skill level for driver selection.
	 * See MinSkill above for details.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxSkill = 0.9f;

	/*
	 * Difficulty modifier affecting AI decision quality.
	 *
	 * This is a multiplier applied to AI skill calculations:
	 * - 0.5 = Easy: AI makes more mistakes, slower reactions
	 * - 1.0 = Normal: AI drives at their natural skill level
	 * - 1.5 = Hard: AI makes optimal decisions, minimal mistakes
	 *
	 * IMPORTANT: This affects DECISION MAKING, not physics. AI doesn't
	 * get faster cars or better grip - they just drive smarter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float DifficultyModifier = 1.0f;

	/*
	 * Enable skill-based catch-up system.
	 *
	 * When enabled, AI racers will:
	 * - Take calculated risks when behind (later braking, tighter lines)
	 * - Drive more conservatively when leading (preserve their lead)
	 *
	 * This creates exciting close races without "cheating" physics.
	 * It's the difference between an AI that "gets lucky" vs one that
	 * "drives harder when it needs to."
	 *
	 * IMPORTANT: This does NOT provide physics advantages like speed boosts!
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
	bool bEnableSkillBasedCatchUp = true;

	/*
	 * DEPRECATED: Use bEnableSkillBasedCatchUp instead.
	 *
	 * This was the old "rubber banding" flag. We renamed it because
	 * "rubber banding" implies physics cheats, which we don't do.
	 * Keeping it for backward compatibility - it maps to skill-based catch-up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (DeprecatedProperty, DeprecationMessage = "Use bEnableSkillBasedCatchUp instead"))
	bool bEnableRubberBanding = true;

	/*
	 * DEPRECATED: No longer used.
	 *
	 * The skill-based catch-up system has fixed, balanced behavior and
	 * doesn't need a "strength" parameter.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty", meta = (DeprecatedProperty, DeprecationMessage = "No longer used"))
	float RubberBandStrength = 0.3f;

	/*
	 * Include a rival driver in the race.
	 *
	 * A "rival" is a special AI that specifically targets the player:
	 * - They'll try to overtake the player
	 * - They'll defend harder against the player
	 * - They may have story significance
	 *
	 * Used for career mode "boss" races and story events.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	bool bIncludeRival = false;

	/*
	 * Specific driver profiles that MUST be included in the race.
	 *
	 * Use this to ensure certain story characters or named rivals
	 * appear in specific races, regardless of skill filtering.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TArray<UMGAIDriverProfile*> RequiredDrivers;

	/*
	 * Restrict AI to a specific vehicle class.
	 *
	 * Example: "Muscle", "Import", "Exotic"
	 * Leave empty (NAME_None) to allow any vehicle class.
	 * Used for class-restricted race events.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	FName VehicleClassRestriction;
};

/*
 * FMGAIRacerInfo - Runtime information about a spawned AI racer
 * --------------------------------------------------------------
 * This struct holds all the live data about an AI racer that's currently
 * in the race. It's created when an AI spawns and updated throughout the race.
 *
 * Use this to query information about AI racers (positions, vehicles, etc.)
 */
USTRUCT(BlueprintType)
struct FMGAIRacerInfo
{
	GENERATED_BODY()

	// The AI Controller driving this racer (the "brain")
	UPROPERTY(BlueprintReadOnly)
	AMGAIRacerController* Controller = nullptr;

	// The Vehicle Pawn (the actual car actor in the world)
	UPROPERTY(BlueprintReadOnly)
	APawn* Vehicle = nullptr;

	// The driver's personality/skill profile
	UPROPERTY(BlueprintReadOnly)
	UMGAIDriverProfile* Profile = nullptr;

	// Starting grid position (0 = pole position)
	UPROPERTY(BlueprintReadOnly)
	int32 GridPosition = 0;

	// Current race position (1 = first place, updated during race)
	UPROPERTY(BlueprintReadOnly)
	int32 RacePosition = 0;

	// The ID of the vehicle model being used (e.g., "Mustang_69")
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	// Whether this racer is currently active in the race
	// False = crashed out, finished, or otherwise inactive
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;
};

/*
 * EVENT DELEGATES
 * ----------------
 * Delegates are Unreal's way of implementing the "Observer Pattern" - they let
 * other systems subscribe to events without tight coupling.
 *
 * Blueprint users can bind to these events in the Event Graph.
 * C++ users can use AddDynamic() to subscribe.
 *
 * Example (Blueprint): Bind OnAIRacerSpawned to update your race UI
 * Example (C++): AISubsystem->OnAllAIRacersSpawned.AddDynamic(this, &MyClass::HandleRaceReady);
 */

// Fired when a single AI racer is spawned. Provides info about the new racer.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIRacerSpawned, const FMGAIRacerInfo&, RacerInfo);

// Fired when an AI racer is removed from the race (finished, crashed, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIRacerRemoved, const FMGAIRacerInfo&, RacerInfo);

// Fired when ALL requested AI racers have finished spawning
// Use this to know when the race grid is complete and ready
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllAIRacersSpawned);

// Fired when all AI racers have been cleared (end of race cleanup)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllAIRacersCleared);

/*
 * =============================================================================
 * UMGAIRacerSubsystem - The Main AI Racer Manager Class
 * =============================================================================
 *
 * This World Subsystem is the central hub for all AI racer management.
 * One instance exists per game world (level).
 *
 * RESPONSIBILITIES:
 * - Spawning AI racers from driver profiles
 * - Assigning grid positions
 * - Distributing racing lines to AI controllers
 * - Tracking race positions for all AI
 * - Skill-based matchmaking (selecting appropriate opponents)
 * - Starting/stopping/pausing AI racing behavior
 *
 * ACCESS PATTERN:
 * UMGAIRacerSubsystem* AISubsystem = GetWorld()->GetSubsystem<UMGAIRacerSubsystem>();
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAIRacerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*
	 * UWorldSubsystem lifecycle overrides.
	 * These are called automatically by Unreal Engine.
	 */

	// Called when the subsystem is created (level load)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Called when the subsystem is destroyed (level unload)
	virtual void Deinitialize() override;

	// Called every frame to update AI positions and state
	virtual void Tick(float DeltaTime) override;

	// Return true to create this subsystem (we always want it)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	/*
	 * Tickable interface requirements.
	 * These methods tell Unreal's tick system how to handle this subsystem.
	 */

	// Statistics tracking ID for profiling
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMGAIRacerSubsystem, STATGROUP_Tickables); }

	// Should this subsystem tick? Only if it's not a template and ticking is enabled
	virtual bool IsTickable() const override { return !IsTemplate() && bIsTickEnabled; }

	// Don't tick when the game is paused
	virtual bool IsTickableWhenPaused() const override { return false; }

	// ==========================================
	// EVENTS - Subscribe to these to react to AI activities
	// ==========================================

	// Fires when an AI racer spawns - use for UI updates, announcer calls, etc.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAIRacerSpawned OnAIRacerSpawned;

	// Fires when an AI racer is removed - use for cleanup, scoring, etc.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAIRacerRemoved OnAIRacerRemoved;

	// Fires when all AI are spawned - use to enable "Start Race" button
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllAIRacersSpawned OnAllAIRacersSpawned;

	// Fires when all AI are cleared - use for post-race cleanup
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllAIRacersCleared OnAllAIRacersCleared;

	// ==========================================
	// CONFIGURATION - Call these before spawning AI
	// ==========================================

	/*
	 * Set the roster of available drivers.
	 * The subsystem will select drivers from this roster based on spawn config.
	 * @param Roster - Data asset containing all available AI drivers
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverRoster(UMGAIDriverRoster* Roster);

	/*
	 * Set the vehicle Blueprint class to spawn for AI.
	 * @param VehicleClass - The Pawn class to spawn (usually your vehicle BP)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetVehicleClass(TSubclassOf<APawn> VehicleClass);

	/*
	 * Set the racing line for AI to follow.
	 * All spawned AI will receive a copy of this racing line.
	 * @param RacingLine - Array of racing line points (from UMGRacingLineGenerator)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine);

	/*
	 * Set the spawn transforms (grid positions).
	 * Each transform defines where an AI car will spawn.
	 * Index 0 = first AI spawn point (usually 2nd on grid if player is 1st)
	 * @param Transforms - Array of world transforms for spawn locations
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetSpawnTransforms(const TArray<FTransform>& Transforms);

	// ==========================================
	// SPAWNING - Create and remove AI racers
	// ==========================================

	/*
	 * Spawn multiple AI racers based on configuration.
	 * This is the main spawning method - handles driver selection, grid assignment, etc.
	 * @param Config - Spawn configuration (count, skill range, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void SpawnAIRacers(const FMGAISpawnConfig& Config);

	/*
	 * Spawn a single AI racer with specific parameters.
	 * Use this for more control over individual racer setup.
	 * @param Profile - The driver profile to use
	 * @param SpawnTransform - Where to spawn the vehicle
	 * @param GridPosition - Starting grid position
	 * @return Info struct for the spawned racer
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	FMGAIRacerInfo SpawnSingleRacer(UMGAIDriverProfile* Profile, const FTransform& SpawnTransform, int32 GridPosition);

	/*
	 * Remove all AI racers from the race.
	 * Call this when cleaning up after a race or resetting.
	 * Despawns all vehicles and clears the active racers array.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void ClearAllRacers();

	/*
	 * Remove a specific racer by their controller.
	 * Use this when an individual AI crashes out or DNFs.
	 * @param Controller - The AI controller to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawn")
	void RemoveRacer(AMGAIRacerController* Controller);

	// ==========================================
	// CONTROL - Start, stop, and modify AI behavior
	// ==========================================

	/*
	 * Start all AI racing.
	 * Call this when the race countdown reaches zero.
	 * AI will begin following racing lines and competing.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StartAllRacing();

	/*
	 * Stop all AI racing.
	 * AI will stop driving but vehicles remain in place.
	 * Use when the race ends or for cutscenes.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StopAllRacing();

	/*
	 * Pause or unpause all AI.
	 * Use for pause menus or slow-motion effects.
	 * @param bPause - True to pause, false to resume
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void PauseAllAI(bool bPause);

	/*
	 * Adjust difficulty for all AI mid-race.
	 * @param DifficultyMultiplier - New difficulty (0.5=easy, 1.0=normal, 1.5=hard)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllDifficulty(float DifficultyMultiplier);

	/*
	 * Enable/disable skill-based catch-up for all AI.
	 *
	 * When enabled, AI will:
	 * - Take calculated risks to catch up when behind
	 * - Drive conservatively when leading
	 *
	 * IMPORTANT: This does NOT provide physics advantages!
	 * @param bEnabled - Whether to enable catch-up behavior
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void SetAllSkillBasedCatchUp(bool bEnabled);

	/*
	 * DEPRECATED: Use SetAllSkillBasedCatchUp instead.
	 * Kept for backward compatibility.
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control", meta = (DeprecatedFunction, DeprecationMessage = "Use SetAllSkillBasedCatchUp instead"))
	void SetAllRubberBanding(bool bEnabled, float Strength = 0.3f);

	// ==========================================
	// QUERIES - Get information about AI racers
	// ==========================================

	// Get array of all active AI racers
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	TArray<FMGAIRacerInfo> GetAllRacers() const { return ActiveRacers; }

	// Get count of active AI racers
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	int32 GetRacerCount() const { return ActiveRacers.Num(); }

	// Get a specific racer by array index
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerByIndex(int32 Index) const;

	// Get a racer by their AI controller
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerByController(AMGAIRacerController* Controller) const;

	// Get the racer currently in a specific race position
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAIRacerInfo GetRacerInPosition(int32 Position) const;

	// Check if all AI spawning is complete
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool AreAllRacersSpawned() const { return bAllSpawned; }

	// Check if any AI is still actively racing
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool IsAnyRacing() const;

	// ==========================================
	// POSITION TRACKING - Race position management
	// ==========================================

	/*
	 * Update race positions for all AI.
	 * Called by the race manager when positions change.
	 * @param Positions - Array of new positions indexed by racer
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Position")
	void UpdateRacePositions(const TArray<int32>& Positions);

	// Get the AI currently in first place
	UFUNCTION(BlueprintPure, Category = "AI|Position")
	FMGAIRacerInfo GetLeadingAI() const;

	// Get the AI closest to the player's vehicle
	UFUNCTION(BlueprintPure, Category = "AI|Position")
	FMGAIRacerInfo GetClosestToPlayer() const;

protected:
	// ==========================================
	// TICK CONTROL
	// ==========================================

	// Master switch for enabling/disabling subsystem ticking
	bool bIsTickEnabled = true;

	// ==========================================
	// CONFIGURATION - Internal state
	// ==========================================

	// The roster of available AI drivers to select from
	UPROPERTY()
	UMGAIDriverRoster* DriverRoster;

	// The vehicle Blueprint class to spawn for AI
	UPROPERTY()
	TSubclassOf<APawn> AIVehicleClass;

	// The AI Controller class to use (can be overridden for custom AI behavior)
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<AMGAIRacerController> AIControllerClass;

	// The racing line all AI will follow
	TArray<FMGAIRacingLinePoint> RacingLinePoints;

	// Grid spawn positions (one transform per AI car)
	TArray<FTransform> GridSpawnTransforms;

	// ==========================================
	// STATE - Current runtime state
	// ==========================================

	// Array of all currently active AI racers
	UPROPERTY()
	TArray<FMGAIRacerInfo> ActiveRacers;

	// The configuration used for the current spawn
	FMGAISpawnConfig CurrentConfig;

	// True once all AI have finished spawning
	bool bAllSpawned = false;

	// True if AI is currently paused
	bool bAIPaused = false;

	// ==========================================
	// INTERNAL - Private helper methods
	// ==========================================

	/*
	 * Select drivers from the roster that match the spawn config.
	 * Filters by skill range and includes any required drivers.
	 */
	TArray<UMGAIDriverProfile*> SelectDriversForRace(const FMGAISpawnConfig& Config);

	/*
	 * Assign grid positions to the selected drivers.
	 * May shuffle or sort based on skill for realistic grids.
	 */
	void AssignGridPositions(TArray<UMGAIDriverProfile*>& Drivers);

	/*
	 * Select an appropriate vehicle for a driver.
	 * Respects class restrictions and driver preferences.
	 */
	FName SelectVehicleForDriver(UMGAIDriverProfile* Driver, const FMGAISpawnConfig& Config);

	/*
	 * Get the world transform for a specific grid position.
	 * Returns identity if position is out of range.
	 */
	FTransform GetSpawnTransformForPosition(int32 GridPosition) const;

	/*
	 * Clean up any racers whose vehicles have been destroyed.
	 * Called periodically to maintain array integrity.
	 */
	void CleanupDestroyedRacers();
};
