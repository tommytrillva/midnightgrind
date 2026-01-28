// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Test Race Configuration - MVP Testing Utility

/**
 * =============================================================================
 * MGTestRaceConfig.h - Test Race Configuration System
 * =============================================================================
 *
 * PURPOSE:
 * This file provides a convenient way to set up and run test races during
 * development. Instead of manually configuring races every time you want to
 * test something, you can create "presets" (pre-configured race setups) that
 * can be quickly loaded and executed.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. DATA ASSET (UDataAsset):
 *    - A Data Asset is a special type of asset in Unreal Engine that stores
 *      configuration data that designers can edit in the Editor.
 *    - UMGTestRaceConfig inherits from UDataAsset, meaning you can create
 *      instances of it in the Content Browser (Right-click > Miscellaneous >
 *      Data Asset > MGTestRaceConfig).
 *    - This separates data from code, allowing non-programmers to create
 *      test scenarios without touching C++.
 *
 * 2. STRUCTS (USTRUCT):
 *    - FMGTestRacePreset is a "struct" - a container that groups related data.
 *    - The "F" prefix is Unreal convention for structs (F = "Fast" or just
 *      convention from Unreal's history).
 *    - GENERATED_BODY() is required macro that generates boilerplate code
 *      for Unreal's reflection system (allows Blueprint access, serialization).
 *
 * 3. UPROPERTY SPECIFIERS:
 *    - EditAnywhere: Can be edited in the Details panel in the Editor.
 *    - BlueprintReadWrite: Can be read AND modified from Blueprints.
 *    - BlueprintReadOnly: Can only be read from Blueprints, not modified.
 *    - Category: Organizes properties into collapsible groups in the Editor.
 *
 * 4. UFUNCTION SPECIFIERS:
 *    - BlueprintPure: A function that doesn't modify state (like a getter).
 *      Shows as a node without execution pins in Blueprints.
 *    - BlueprintCallable: A function that can be called from Blueprints.
 *      Shows as a node with execution pins.
 *    - CallInEditor: Adds a button in the Details panel to call the function.
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Test Race Config Data Asset]
 *              |
 *              v
 *    [UMGTestRaceRunner] -- Creates presets programmatically
 *              |
 *              v
 *    [MGRaceFlowSubsystem] -- Actually starts and manages the race
 *              |
 *              v
 *    [Spawns vehicles, starts countdown, tracks positions, etc.]
 *
 * COMMON USE CASES:
 * - QA testing specific race scenarios repeatedly
 * - Debugging AI behavior with specific configurations
 * - Performance testing with stress test presets (many AI opponents)
 * - Quick iteration during development without manual setup
 *
 * EXAMPLE USAGE IN BLUEPRINTS:
 *   1. Create a MGTestRaceConfig Data Asset in Content Browser
 *   2. Add presets with different configurations
 *   3. Use UMGTestRaceRunner::RunFromPreset() to start a race
 *
 * EXAMPLE USAGE IN C++:
 *   UMGTestRaceRunner::RunMinimalTest(GetWorld());
 *   // or
 *   FMGTestRacePreset Preset = UMGTestRaceRunner::CreateDriftTestPreset();
 *   UMGTestRaceRunner::RunFromPreset(GetWorld(), Preset);
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "MGTestRaceConfig.generated.h"

/**
 * Test Race Preset
 *
 * A pre-configured race setup that can be quickly loaded for testing.
 * Contains all the settings needed to start a race, plus testing-specific
 * options like invincibility and debug visualization.
 *
 * Think of this as a "saved game" for race configuration - you set it up
 * once, save it, and can load it instantly any time you need to test.
 *
 * The "F" prefix is Unreal convention for structs (as opposed to "U" for
 * UObject classes and "A" for Actor classes).
 */
USTRUCT(BlueprintType)
struct FMGTestRacePreset
{
	GENERATED_BODY()

	/**
	 * Display name for this preset (e.g., "Drift Test", "Stress Test")
	 * Shown in UI menus and test reports.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FText PresetName;

	/**
	 * Detailed description of what this test scenario is for.
	 * Example: "Tests drift physics with 3 AI on wet roads"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FText Description;

	/**
	 * The actual race configuration (laps, AI count, track, mode, etc.)
	 * This struct is defined in MGRaceFlowSubsystem.h and contains
	 * everything the race system needs to set up and start a race.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FMGRaceSetupRequest Setup;

	/**
	 * Seconds to wait before automatically starting the race.
	 * Set to 0 if you want to start manually (e.g., with a button press).
	 * Useful for automated testing where no human input is available.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float AutoStartDelay = 3.0f;

	/**
	 * Whether to show debug overlays (checkpoints, racing lines, AI paths).
	 * Very useful for debugging, but can clutter the screen.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bEnableDebugVis = true;

	/**
	 * Skip the "3-2-1-GO!" countdown sequence.
	 * Saves time when running many tests, but some bugs only appear
	 * during the countdown phase.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bSkipCountdown = false;

	/**
	 * Make the player immune to damage.
	 * Useful when you want to test other systems without dying.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bPlayerInvincible = false;

	/**
	 * AI racers won't attack or ram the player.
	 * Useful for testing player mechanics in isolation, or when
	 * you need to study AI behavior without interference.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	bool bPassiveAI = false;
};

/**
 * Test Race Configuration Data Asset
 *
 * A Data Asset that stores a collection of test race presets. Create instances
 * of this in the Content Browser to define your test scenarios.
 *
 * HOW TO CREATE:
 * 1. Right-click in Content Browser
 * 2. Select Miscellaneous > Data Asset
 * 3. Choose MGTestRaceConfig
 * 4. Name it (e.g., "DA_TestRacePresets")
 * 5. Double-click to edit and add presets
 *
 * WHY USE DATA ASSETS:
 * - Designers can create test scenarios without coding
 * - Presets are saved with the project (version controlled)
 * - Can have multiple configuration files for different test suites
 * - Editor support: search, duplicate, organize in folders
 *
 * MIDNIGHTGRIND_API: This macro exports the class for use by other modules.
 * Required for any class that needs to be accessed from outside this module.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTestRaceConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Array of test presets stored in this Data Asset.
	 * Add, remove, and edit these in the Editor's Details panel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	TArray<FMGTestRacePreset> TestPresets;

	/**
	 * Which preset to use by default (0-based index).
	 * Used when no specific preset is requested.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Presets")
	int32 DefaultPresetIndex = 0;

	/**
	 * Get a preset by its array index.
	 * @param Index The 0-based index (0 = first preset)
	 * @return The preset at that index (or default if out of bounds)
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	FMGTestRacePreset GetPreset(int32 Index) const;

	/**
	 * Find a preset by its display name.
	 * @param Name The PresetName to search for
	 * @param OutPreset [out] The found preset (if successful)
	 * @return True if a preset with that name was found
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	bool GetPresetByName(const FText& Name, FMGTestRacePreset& OutPreset) const;

	/**
	 * Get a list of all preset names for UI display.
	 * @return Array of preset names (for dropdown menus, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	TArray<FText> GetPresetNames() const;

	/** Default constructor - initializes empty preset list */
	UMGTestRaceConfig();

#if WITH_EDITOR
	/**
	 * Generate a set of default test presets.
	 * Adds common test scenarios (minimal, full, drift, stress, etc.)
	 *
	 * This function has a button in the Editor (CallInEditor) that you can
	 * click to populate the presets. Only available in Editor builds.
	 */
	UFUNCTION(CallInEditor, Category = "Test")
	void GenerateDefaultPresets();
#endif
};

/**
 * Test Race Runner
 *
 * A utility class with static functions for quickly starting test races.
 * Use this when you want to run a test with ONE LINE of code instead of
 * manually configuring everything.
 *
 * This class provides two types of functions:
 * 1. Quick Test Functions (Run*): Start a race immediately
 * 2. Preset Factories (Create*): Create presets programmatically
 *
 * STATIC FUNCTIONS:
 * All functions are "static" which means you don't need an instance of
 * UMGTestRaceRunner - just call them directly:
 *   UMGTestRaceRunner::RunMinimalTest(GetWorld());
 *
 * WORLD CONTEXT:
 * The WorldContextObject parameter is how Unreal gets access to the game
 * world from a static function. In C++, pass GetWorld(). In Blueprints,
 * Unreal automatically provides this.
 *
 * BLUEPRINTABLE:
 * This class can be subclassed in Blueprints if you want to add custom
 * test race configurations.
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGTestRaceRunner : public UObject
{
	GENERATED_BODY()

public:
	// ========================================================================
	// QUICK TEST FUNCTIONS
	// ========================================================================
	// Call these to immediately start a test race.
	// They create a preset internally and run it.

	/**
	 * Run the simplest possible test race.
	 * Good for quick sanity checks that the race system is working.
	 * Config: 2 laps, 3 AI, easy difficulty, debug vis enabled
	 *
	 * @param WorldContextObject Context for accessing the world (auto-filled in BP)
	 * @return True if the race started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunMinimalTest(UObject* WorldContextObject);

	/**
	 * Run a comprehensive test with all systems active.
	 * Tests the full race experience including weather, damage, etc.
	 * Config: 3 laps, 7 AI, medium difficulty
	 *
	 * @param WorldContextObject Context for accessing the world
	 * @return True if the race started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunFullTest(UObject* WorldContextObject);

	/**
	 * Run a test focused on drift mechanics.
	 * Tests drift scoring, physics, and related systems.
	 *
	 * @param WorldContextObject Context for accessing the world
	 * @return True if the race started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunDriftTest(UObject* WorldContextObject);

	/**
	 * Run a drag race test (straight-line acceleration).
	 * Tests drag race mechanics, reaction times, gear shifting.
	 *
	 * @param WorldContextObject Context for accessing the world
	 * @return True if the race started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunDragTest(UObject* WorldContextObject);

	/**
	 * Run a stress test with maximum AI opponents.
	 * Used to find performance issues and edge cases.
	 * Config: Max AI count, hard difficulty
	 *
	 * @param WorldContextObject Context for accessing the world
	 * @return True if the race started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunStressTest(UObject* WorldContextObject);

	/**
	 * Run a race using a custom preset.
	 * Use this when the built-in tests don't fit your needs.
	 *
	 * @param WorldContextObject Context for accessing the world
	 * @param Preset The test configuration to use
	 * @return True if the race started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Test", meta = (WorldContext = "WorldContextObject"))
	static bool RunFromPreset(UObject* WorldContextObject, const FMGTestRacePreset& Preset);

	// ========================================================================
	// PRESET FACTORIES
	// ========================================================================
	// These create preset structs but DON'T start races.
	// Use them to get a preset, modify it, then pass to RunFromPreset().
	//
	// BlueprintPure: No side effects, just returns data.
	// These show up in Blueprints without execution pins.

	/**
	 * Create a minimal test preset.
	 * Config: 2 laps, 3 AI, easy difficulty
	 * Good starting point for quick tests.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateMinimalTestPreset();

	/**
	 * Create a full test preset.
	 * Config: 3 laps, 7 AI, medium difficulty
	 * Comprehensive test of all race features.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateFullTestPreset();

	/**
	 * Create a drift-focused test preset.
	 * Optimized for testing drift mechanics.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateDriftTestPreset();

	/**
	 * Create a drag race test preset.
	 * Optimized for testing straight-line racing.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateDragTestPreset();

	/**
	 * Create a sprint race test preset.
	 * Point-to-point race (no laps).
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateSprintTestPreset();

	/**
	 * Create a pink slip race test preset.
	 * High-stakes race where loser loses their car.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreatePinkSlipTestPreset();

	/**
	 * Create a stress test preset.
	 * Config: Maximum AI, hard difficulty
	 * Use to find performance limits.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateStressTestPreset();

	/**
	 * Create a benchmark preset.
	 * Standardized configuration for measuring performance.
	 * Results can be compared across builds/machines.
	 */
	UFUNCTION(BlueprintPure, Category = "Test")
	static FMGTestRacePreset CreateBenchmarkPreset();
};
