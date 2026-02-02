// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MidnightGrindModule.h
 * @brief Primary Game Module Definition for Midnight Grind
 *
 * @section overview Overview for New Developers
 *
 * This file defines the main game module for Midnight Grind. In Unreal Engine,
 * a "module" is a unit of code that can be loaded/unloaded and has its own
 * lifecycle. Every Unreal project has at least one game module - this is ours.
 *
 * @section what_is_module What is a Module?
 *
 * Think of a module like a DLL (Windows) or shared library (Mac/Linux). It's
 * a compiled unit of code that Unreal can manage independently. The engine
 * calls specific functions on your module at key moments:
 *
 * - StartupModule(): Called when your game code is first loaded
 * - ShutdownModule(): Called when your game code is being unloaded
 * - IsGameModule(): Tells Unreal what type of module this is
 *
 * @section module_types Module Types in Unreal
 *
 * Unreal distinguishes between several module types:
 *
 * 1. GAME MODULES (IsGameModule() returns true):
 *    - Your actual game code
 *    - Always included in packaged builds
 *    - This is what we are!
 *
 * 2. PLUGIN MODULES:
 *    - Optional extensions/features
 *    - Can be enabled/disabled
 *    - Lives in the Plugins folder
 *
 * 3. ENGINE MODULES:
 *    - Core Unreal functionality
 *    - You don't write these
 *
 * @section concepts Key Concepts
 *
 * 1. IModuleInterface:
 *    The base class all modules inherit from. Provides the lifecycle hooks
 *    that Unreal calls. Our FMidnightGrindModule inherits from this.
 *
 * 2. StartupModule():
 *    Called once when the module is loaded. This happens early in engine
 *    startup, before any game content loads. Use this for:
 *    - Registering custom console commands
 *    - Setting up static data structures
 *    - Logging initialization messages
 *    - Registering with engine systems
 *
 * 3. ShutdownModule():
 *    Called when the module is unloaded (engine shutdown). Use this for:
 *    - Cleaning up static/global resources
 *    - Unregistering from engine systems
 *    - Final logging/analytics
 *
 * 4. IMPLEMENT_PRIMARY_GAME_MODULE Macro:
 *    In the .cpp file, this macro registers our module as THE primary
 *    game module. It connects our class to Unreal's module system.
 *
 * @section when_called When Are These Functions Called?
 *
 * Engine Startup:
 *   1. Engine core initializes
 *   2. Engine modules load
 *   3. Plugin modules load
 *   4. >> StartupModule() called << (we're here)
 *   5. Game content loads
 *   6. World/level loads
 *   7. Gameplay begins
 *
 * Engine Shutdown:
 *   1. Gameplay ends
 *   2. World unloads
 *   3. >> ShutdownModule() called << (we're here)
 *   4. Engine shuts down
 *
 * @section usage Usage Example
 *
 * @code
 * // In MidnightGrindModule.cpp:
 *
 * #include "MidnightGrindModule.h"
 * #include "Modules/ModuleManager.h"
 *
 * void FMidnightGrindModule::StartupModule()
 * {
 *     // Module is loading - do early initialization here
 *     UE_LOG(LogTemp, Log, TEXT("Midnight Grind module starting up!"));
 *
 *     // Example: Register a custom console command
 *     // IConsoleManager::Get().RegisterConsoleCommand(...);
 * }
 *
 * void FMidnightGrindModule::ShutdownModule()
 * {
 *     // Module is unloading - clean up here
 *     UE_LOG(LogTemp, Log, TEXT("Midnight Grind module shutting down."));
 * }
 *
 * // This macro is REQUIRED - it tells Unreal this is the game module
 * IMPLEMENT_PRIMARY_GAME_MODULE(FMidnightGrindModule, MidnightGrind, "MidnightGrind");
 * @endcode
 *
 * @section best_practices Best Practices
 *
 * 1. Keep StartupModule() fast - it blocks engine startup
 * 2. Don't access game content in StartupModule() - it hasn't loaded yet
 * 3. Use subsystems (UGameInstanceSubsystem) for most game logic, not the module
 * 4. The module is for engine-level initialization, not gameplay code
 * 5. If you need to do heavy initialization, defer it to later
 *
 * @section common_mistakes Common Mistakes
 *
 * 1. Trying to spawn actors in StartupModule() - world doesn't exist yet
 * 2. Accessing UObjects before they're loaded - use soft references
 * 3. Doing too much work in StartupModule() - slows game launch
 * 4. Forgetting IMPLEMENT_PRIMARY_GAME_MODULE in the .cpp file
 *
 * @section game_description Game Description
 *
 * Midnight Grind is an arcade street racing game inspired by the visual
 * style of PS1/PS2 era racing games. Think classic Need for Speed Underground
 * meets Initial D with a retro aesthetic. Features include:
 *
 * - Pink slip racing (bet your car!)
 * - Deep vehicle customization
 * - Insurance system for risk management
 * - Dynamic difficulty adjustment
 * - Rival/nemesis system
 *
 * @section related Related Files
 *
 * - MidnightGrindModule.cpp - Implementation of StartupModule/ShutdownModule
 * - MidnightGrind.Build.cs - Build configuration (dependencies, defines)
 * - MidnightGrind.Target.cs - Target configuration (platform settings)
 *
 * @note This file may coexist with MidnightGrind.h at the Source root.
 *       Having the module definition in the Public folder allows other
 *       modules to reference it if needed.
 *
 * @see IModuleInterface - Base class documentation
 * @see https://docs.unrealengine.com/en-US/ProgrammingAndScripting/ProgrammingWithCPP/ModuleQuickStart
 */

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Main game module for Midnight Grind.
 *
 * This class serves as the entry point for the game's C++ code. Unreal Engine
 * instantiates this class during startup and calls its lifecycle methods.
 *
 * For most game logic, prefer using UGameInstanceSubsystem derivatives
 * (like UMGWagerSubsystem, UMGInsuranceSubsystem, etc.) rather than putting
 * code directly in this module class.
 *
 * @see UGameInstanceSubsystem for runtime game logic
 * @see IModuleInterface for the base class interface
 */
class FMidnightGrindModule : public IModuleInterface
{
public:
	// ========================================================================
	// LIFECYCLE METHODS - Called automatically by Unreal Engine
	// ========================================================================

	/**
	 * Called when the game module is loaded
	 * Typically happens during engine startup before any game content loads
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the game module is unloaded
	 * Happens during engine shutdown - clean up any static/global resources here
	 */
	virtual void ShutdownModule() override;

	/**
	 * Identifies this as a game module (not a plugin or engine module)
	 *
	 * This affects how Unreal treats the module during:
	 * - Hot reload (game modules are reloaded differently)
	 * - Packaging (game modules are always included)
	 * - Editor behavior
	 */
	virtual bool IsGameModule() const override { return true; }
};
