// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGDevCommands.h - Developer Console Commands and Cheats
 * =============================================================================
 *
 * PURPOSE:
 * This file provides a collection of console commands that developers and QA
 * testers can use to quickly test game functionality without playing through
 * the game normally. These are "cheat codes" for development purposes.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. GAME INSTANCE SUBSYSTEM (UGameInstanceSubsystem):
 *    - A subsystem is a singleton-like object that Unreal manages automatically.
 *    - GameInstanceSubsystem persists for the entire game session (survives
 *      level transitions), unlike WorldSubsystem which is per-level.
 *    - You don't create these manually - Unreal creates ONE instance when the
 *      game starts and destroys it when the game ends.
 *    - Access via: GetGameInstance()->GetSubsystem<UMGDevCommands>()
 *
 * 2. EXEC FUNCTIONS:
 *    - The "Exec" specifier in UFUNCTION makes the function callable from
 *      the in-game console (press ~ or ` key to open console).
 *    - Console command format: ClassName.FunctionName Parameters
 *      Example: "MG.SpawnVehicle JDM_Mid"
 *    - This is how debug commands in most games work under the hood.
 *
 * 3. CONSOLE COMMAND ORGANIZATION:
 *    - Commands are grouped by category (Vehicle, Race, Economy, Cheats, etc.)
 *    - The Category in UFUNCTION helps organize them in the Editor.
 *    - "Dev|Vehicle" creates a nested category: Dev > Vehicle
 *
 * 4. FORWARD DECLARATIONS:
 *    - "class AMGVehiclePawn;" at the top is a forward declaration.
 *    - It tells the compiler "this class exists" without including its header.
 *    - This speeds up compilation and reduces header dependencies.
 *    - The actual #include goes in the .cpp file where you use the class.
 *
 * 5. TOGGLE PATTERN:
 *    - Many commands (GodMode, ShowDebug, etc.) are "toggles" - calling them
 *      once enables the feature, calling again disables it.
 *    - State is tracked in protected bool variables (bGodMode, bShowDebug, etc.)
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Developer Types in Console]
 *           |
 *           v
 *    [UMGDevCommands] -- Parses command, validates parameters
 *           |
 *           +---> [MGVehicleFactory] -- For spawning vehicles
 *           +---> [MGRaceFlowSubsystem] -- For race manipulation
 *           +---> [MGEconomySubsystem] -- For currency cheats
 *           +---> [Player Vehicle] -- For god mode, nitrous, etc.
 *
 * COMMON USE CASES:
 * - QuickRace: Skip menus and start racing immediately
 * - GodMode: Test without worrying about damage
 * - SpawnAI: Test with specific number of opponents
 * - TimeScale: Slow down time to debug physics issues
 * - ShowDebug: Visual debugging for checkpoints, racing lines, AI
 *
 * HOW TO USE:
 * 1. Press ~ (tilde) or ` (backtick) to open the console
 * 2. Type the command, e.g., "MG.GodMode"
 * 3. Press Enter
 *
 * CONSOLE COMMANDS QUICK REFERENCE:
 * - MG.SpawnVehicle <preset> - Spawn a vehicle for player
 * - MG.SpawnAI <count> - Spawn AI opponents
 * - MG.StartRace - Start the race immediately
 * - MG.FinishRace - Force finish the race
 * - MG.SetLap <lap> - Set current lap number
 * - MG.AddCredits <amount> - Add credits
 * - MG.GodMode - Toggle invincibility
 * - MG.UnlimitedNitrous - Toggle unlimited nitrous
 * - MG.TimeScale <scale> - Set time scale
 * - MG.ShowDebug - Toggle debug display
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/MGVehicleFactory.h"
#include "MGDevCommands.generated.h"

/**
 * Forward declaration of AMGVehiclePawn.
 * This tells the compiler the class exists without including its full header.
 * The "A" prefix indicates this is an Actor class (Unreal naming convention).
 */
class AMGVehiclePawn;

/**
 * Developer Commands Subsystem
 *
 * Provides console commands and cheats for testing gameplay. This subsystem
 * is automatically created when the game starts and persists across level
 * transitions.
 *
 * All commands can be executed via:
 * - The in-game console (press ~)
 * - Blueprints (all functions are BlueprintCallable)
 * - C++ code (get the subsystem and call the function directly)
 */
/**
 * Developer Commands Subsystem
 *
 * Provides console commands (cheats) for testing gameplay.
 *
 * USAGE: Open console with ~ key and type commands like:
 *   MG.GodMode
 *   MG.AddCredits 100000
 *   MG.SpawnAI 5
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDevCommands : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Called when the subsystem is created (game start).
	 * Registers console commands and initializes state.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called when the subsystem is destroyed (game shutdown).
	 * Cleans up console commands.
	 */
	virtual void Deinitialize() override;

	// ========================================================================
	// VEHICLE COMMANDS
	// ========================================================================
	// Commands for spawning, teleporting, and resetting vehicles.
	// Useful for testing vehicle mechanics without going through menus.

	/**
	 * Spawn a vehicle at the player start location.
	 * Console: MG.SpawnVehicle [preset]
	 *
	 * @param Preset The vehicle type to spawn (defaults to JDM_Mid)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void SpawnVehicle(EMGVehiclePreset Preset = EMGVehiclePreset::JDM_Mid);

	/**
	 * Spawn AI opponent vehicles.
	 * Console: MG.SpawnAI [count]
	 *
	 * @param Count Number of AI to spawn (defaults to 5)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void SpawnAI(int32 Count = 5);

	/**
	 * Remove all AI vehicles from the world.
	 * Console: MG.DespawnAllAI
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void DespawnAllAI();

	/**
	 * Teleport player vehicle back to the starting position.
	 * Console: MG.TeleportToStart
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void TeleportToStart();

	/**
	 * Reset player vehicle (fix stuck state, restore orientation).
	 * Console: MG.ResetVehicle
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void ResetVehicle();

	// ========================================================================
	// RACE COMMANDS
	// ========================================================================
	// Commands for controlling the race state.
	// Useful for testing specific race phases or quickly finishing races.

	/**
	 * Start the race countdown (3-2-1-GO!).
	 * Console: MG.StartRace
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void StartRace();

	/**
	 * Immediately finish the race (player wins).
	 * Console: MG.FinishRace
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void FinishRace();

	/**
	 * Restart the current race from the beginning.
	 * Console: MG.RestartRace
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void RestartRace();

	/**
	 * Jump to a specific lap number.
	 * Console: MG.SetLap [number]
	 *
	 * @param LapNumber The lap to jump to (1 = first lap)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void SetLap(int32 LapNumber);

	/**
	 * Set the player's race position.
	 * Console: MG.SetPosition [position]
	 *
	 * @param Position The position to set (1 = first place)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void SetPosition(int32 Position);

	/**
	 * Skip directly to the results screen.
	 * Console: MG.SkipToResults
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Race")
	void SkipToResults();

	// ========================================================================
	// ECONOMY COMMANDS
	// ========================================================================
	// Commands for manipulating player currency, XP, and unlocks.
	// Useful for testing purchases, upgrades, and progression.

	/**
	 * Add credits (primary currency) to the player.
	 * Console: MG.AddCredits [amount]
	 *
	 * @param Amount Credits to add (defaults to 100,000)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void AddCredits(int32 Amount = 100000);

	/**
	 * Add experience points to the player.
	 * Console: MG.AddXP [amount]
	 *
	 * @param Amount XP to add (defaults to 10,000)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void AddXP(int32 Amount = 10000);

	/**
	 * Set the player's level directly.
	 * Console: MG.SetLevel [level]
	 *
	 * @param Level The level to set (1 = beginner)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void SetLevel(int32 Level);

	/**
	 * Instantly unlock all vehicles in the garage.
	 * Console: MG.UnlockAllVehicles
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void UnlockAllVehicles();

	// ========================================================================
	// CHEAT COMMANDS
	// ========================================================================
	// Classic cheat codes for making testing easier.
	// All toggles - call once to enable, call again to disable.

	/**
	 * Toggle invincibility (no damage).
	 * Console: MG.GodMode
	 *
	 * State tracked in: bGodMode
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void GodMode();

	/**
	 * Toggle infinite nitrous boost.
	 * Console: MG.UnlimitedNitrous
	 *
	 * State tracked in: bUnlimitedNitrous
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void UnlimitedNitrous();

	/**
	 * Toggle super speed mode (faster acceleration/top speed).
	 * Console: MG.SuperSpeed
	 *
	 * State tracked in: bSuperSpeed
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void SuperSpeed();

	/**
	 * Set the game's time scale (slow motion or fast forward).
	 * Console: MG.TimeScale [scale]
	 *
	 * @param Scale Time multiplier (0.5 = half speed, 2.0 = double speed)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void TimeScale(float Scale = 1.0f);

	/**
	 * Freeze/unfreeze all AI vehicles.
	 * Console: MG.FreezeAI
	 *
	 * State tracked in: bAIFrozen
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Cheats")
	void FreezeAI();

	// ========================================================================
	// DEBUG COMMANDS
	// ========================================================================
	// Visual debugging tools and state inspection.
	// Essential for understanding what the game is doing internally.

	/**
	 * Toggle the debug HUD overlay (shows internal state info).
	 * Console: MG.ShowDebug
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void ShowDebug();

	/**
	 * Toggle checkpoint visualization (shows checkpoint triggers).
	 * Console: MG.ShowCheckpoints
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void ShowCheckpoints();

	/**
	 * Toggle racing line visualization (optimal path around track).
	 * Console: MG.ShowRacingLine
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void ShowRacingLine();

	/**
	 * Print the current race state to the console.
	 * Console: MG.PrintRaceState
	 *
	 * Outputs: Current phase, lap count, positions, timer, etc.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void PrintRaceState();

	/**
	 * Print player vehicle statistics to the console.
	 * Console: MG.PrintVehicleStats
	 *
	 * Outputs: Speed, RPM, gear, tire grip, damage state, etc.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Debug")
	void PrintVehicleStats();

	// ========================================================================
	// AI DEBUG COMMANDS
	// ========================================================================
	// Tools for understanding and manipulating AI behavior.
	// Use these when AI isn't behaving as expected.

	/**
	 * Toggle AI debug visualization.
	 * Console: MG.ShowAIDebug
	 *
	 * Shows: Mood indicators, current state, target waypoints, decision trees
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void ShowAIDebug();

	/**
	 * Print all AI controller states to the console.
	 * Console: MG.PrintAIStates
	 *
	 * Outputs: State, mood, speed, target, personality for each AI
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void PrintAIStates();

	/**
	 * Set difficulty level for all AI racers.
	 * Console: MG.SetAIDifficulty [0.0-1.0]
	 *
	 * @param Difficulty 0.0 = very easy, 0.5 = medium, 1.0 = hard
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void SetAIDifficulty(float Difficulty = 0.5f);

	/**
	 * Reset all AI moods to neutral.
	 * Console: MG.ResetAIMoods
	 *
	 * Useful when AI is stuck in aggressive/defensive patterns.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|AI")
	void ResetAIMoods();

	// ========================================================================
	// VEHICLE DEBUG COMMANDS
	// ========================================================================
	// Detailed vehicle inspection and repair tools.
	// Use these to debug physics issues and damage systems.

	/**
	 * Print the vehicle's damage state to console.
	 * Console: MG.PrintDamageState
	 *
	 * Outputs: Component health %, total damage, active effects
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void PrintDamageState();

	/**
	 * Print vehicle physics state to console.
	 * Console: MG.PrintPhysicsState
	 *
	 * Outputs: Suspension compression, weight transfer, grip per wheel
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void PrintPhysicsState();

	/**
	 * Toggle tire debug visualization.
	 * Console: MG.ShowTireDebug
	 *
	 * Shows: Tire contact patches, grip forces, slip angles
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void ShowTireDebug();

	/**
	 * Fully repair the player's vehicle.
	 * Console: MG.RepairVehicle
	 *
	 * Restores all components to 100% health.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Vehicle")
	void RepairVehicle();

	// ========================================================================
	// ECONOMY DEBUG COMMANDS
	// ========================================================================
	// Tools for inspecting and testing the economy system.
	// Use these to verify transactions and balances.

	/**
	 * Print player's economic state to console.
	 * Console: MG.PrintEconomyState
	 *
	 * Outputs: Credits, XP, level, multipliers, bonuses
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void PrintEconomyState();

	/**
	 * Simulate a purchase without actually spending.
	 * Console: MG.SimulatePurchase [amount]
	 *
	 * Shows: Whether player can afford it, taxes, final cost
	 * @param Amount The hypothetical purchase amount
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void SimulatePurchase(int32 Amount = 10000);

	/**
	 * Print recent transaction history to console.
	 * Console: MG.PrintTransactions [count]
	 *
	 * @param Count Number of recent transactions to show
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Economy")
	void PrintTransactions(int32 Count = 10);

	// ========================================================================
	// WEATHER DEBUG COMMANDS
	// ========================================================================
	// Control weather and time for testing different conditions.
	// Weather affects road grip, visibility, and AI behavior.

	/**
	 * Set the current weather type.
	 * Console: MG.SetWeather [type]
	 *
	 * @param WeatherType 0=Clear, 1=Cloudy, 2=Rain, 3=Storm, 4=Fog, 5=Snow
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Weather")
	void SetWeather(int32 WeatherType = 0);

	/**
	 * Set the time of day.
	 * Console: MG.SetTimeOfDay [hour]
	 *
	 * @param Hour Time in 24-hour format (0=midnight, 12=noon, 18=evening)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Weather")
	void SetTimeOfDay(float Hour = 12.0f);

	/**
	 * Print current weather state to console.
	 * Console: MG.PrintWeatherState
	 *
	 * Outputs: Weather type, intensity, road wetness, visibility
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Weather")
	void PrintWeatherState();

	/**
	 * Toggle instant vs gradual weather transitions.
	 * Console: MG.ToggleInstantWeather
	 *
	 * When enabled: Weather changes instantly (good for testing)
	 * When disabled: Weather transitions smoothly (realistic)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Weather")
	void ToggleInstantWeather();

	// ========================================================================
	// PERFORMANCE DEBUG COMMANDS
	// ========================================================================
	// Tools for identifying performance bottlenecks.
	// Use these when the game is running slowly.

	/**
	 * Print tick times for all subsystems to console.
	 * Console: MG.PrintTickTimes
	 *
	 * Outputs: Time spent in each subsystem's Tick() function (in ms)
	 * Look for subsystems taking > 1ms - they may need optimization.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Performance")
	void PrintTickTimes();

	/**
	 * Print memory usage breakdown to console.
	 * Console: MG.PrintMemoryUsage
	 *
	 * Outputs: Memory used by textures, meshes, audio, etc.
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Performance")
	void PrintMemoryUsage();

	/**
	 * Toggle the performance overlay HUD.
	 * Console: MG.ShowPerformance
	 *
	 * Shows: FPS, frame time, GPU time, draw calls, memory
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Performance")
	void ShowPerformance();

	// ========================================================================
	// QUICK TEST
	// ========================================================================
	// One-command shortcuts to start racing immediately.
	// The fastest way to get into gameplay for testing.

	/**
	 * Instantly start a race with specified settings.
	 * Console: MG.QuickRace [AICount] [Laps]
	 *
	 * Skips all menus and setup - just starts racing.
	 *
	 * @param AICount Number of AI opponents (defaults to 5)
	 * @param Laps Number of laps (defaults to 3)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Quick")
	void QuickRace(int32 AICount = 5, int32 Laps = 3);

	/**
	 * Instantly start a time trial (solo, no AI).
	 * Console: MG.QuickTimeTrial [Laps]
	 *
	 * Good for testing track flow and vehicle handling.
	 *
	 * @param Laps Number of laps (defaults to 3)
	 */
	UFUNCTION(Exec, BlueprintCallable, Category = "Dev|Quick")
	void QuickTimeTrial(int32 Laps = 3);

protected:
	// ========================================================================
	// STATE TRACKING
	// ========================================================================
	// These booleans track the state of toggle commands.
	// When you call GodMode() once, bGodMode becomes true.
	// Call it again, and bGodMode becomes false.

	/** Is god mode (invincibility) currently active? */
	bool bGodMode = false;

	/** Is unlimited nitrous currently active? */
	bool bUnlimitedNitrous = false;

	/** Is super speed mode currently active? */
	bool bSuperSpeed = false;

	/** Are AI vehicles currently frozen? */
	bool bAIFrozen = false;

	/** Is the debug HUD currently showing? */
	bool bShowDebug = false;

	/** Are checkpoints currently visualized? */
	bool bShowCheckpoints = false;

	/** Is the racing line currently visible? */
	bool bShowRacingLine = false;

	/** Is AI debug info currently visible? */
	bool bShowAIDebug = false;

	/** Is tire debug visualization currently active? */
	bool bShowTireDebug = false;

	/** Are weather transitions instant (vs gradual)? */
	bool bInstantWeather = false;

	/** Is the performance overlay currently showing? */
	bool bShowPerformance = false;

	// ========================================================================
	// HELPER FUNCTIONS
	// ========================================================================
	// Internal utility functions used by the commands above.

	/**
	 * Get a pointer to the player's current vehicle.
	 * @return The player's vehicle pawn, or nullptr if not in a vehicle
	 */
	AMGVehiclePawn* GetPlayerVehicle() const;

	/**
	 * Log a command execution (for debugging/telemetry).
	 * @param Command The command string that was executed
	 */
	void LogCommand(const FString& Command);
};
