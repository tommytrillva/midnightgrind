// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MidnightGrind.h
 * @brief Primary game module header for the Midnight Grind project.
 *
 * This file defines the main game module class and logging category for the
 * Midnight Grind game. It serves as the central entry point for module-level
 * initialization and provides singleton access to the module instance.
 *
 * @section main_module Main Game Module
 *
 * This is the **primary game module** for Midnight Grind. In Unreal Engine's
 * architecture, every game or plugin is organized into one or more "modules."
 * The main game module (defined here) is the first module loaded when your
 * game starts and the last to be unloaded when it shuts down.
 *
 * The module system provides:
 * - Clean separation of code into logical units
 * - Dependency management between different parts of the engine and game
 * - Hot-reload support during development (Editor only)
 * - Control over initialization and cleanup timing
 *
 * @section ue_modules Understanding Unreal Engine Modules (For Beginners)
 *
 * @subsection what_is_module What is a Module?
 *
 * A **module** in Unreal Engine is a self-contained unit of C++ code that:
 * - Compiles into a single DLL (Windows) or shared library (Mac/Linux)
 * - Has explicit dependencies on other modules
 * - Can be loaded/unloaded at runtime (in some cases)
 * - Defines its own public API through header files
 *
 * Think of modules like "packages" or "libraries" in other programming
 * environments. Your game's source code lives in at least one module (this one),
 * and you can create additional modules for organization (e.g., a separate
 * module for AI, networking, etc.).
 *
 * @subsection module_components Module Components
 *
 * Every Unreal module consists of these key files:
 *
 * | File | Purpose |
 * |------|---------|
 * | `ModuleName.h` | Module header (this file) - declares the module class |
 * | `ModuleName.cpp` | Module implementation - defines startup/shutdown logic |
 * | `ModuleName.Build.cs` | Build configuration - specifies dependencies |
 *
 * @subsection module_lifecycle Module Lifecycle
 *
 * 1. **Loading**: Engine loads the module DLL into memory
 * 2. **StartupModule()**: Called to initialize the module (your custom code runs here)
 * 3. **Runtime**: Module is active, all classes and functions are available
 * 4. **ShutdownModule()**: Called during engine shutdown for cleanup
 * 5. **Unloading**: Module DLL is removed from memory
 *
 * @section log_category_section Understanding DECLARE_LOG_CATEGORY_EXTERN
 *
 * Unreal Engine has a powerful logging system that allows filtering and
 * categorization of log messages. The macro `DECLARE_LOG_CATEGORY_EXTERN`
 * creates a custom log category for your game.
 *
 * @subsection log_syntax Syntax Breakdown
 *
 * @code{.cpp}
 * DECLARE_LOG_CATEGORY_EXTERN(CategoryName, DefaultVerbosity, CompileTimeVerbosity);
 * @endcode
 *
 * - **CategoryName**: The identifier used in UE_LOG calls (e.g., `LogMidnightGrind`)
 * - **DefaultVerbosity**: What level shows by default (Log, Warning, Error, etc.)
 * - **CompileTimeVerbosity**: Maximum level that gets compiled in (use `All` for development)
 *
 * @subsection log_usage How to Use the Log Category
 *
 * @code{.cpp}
 * // Basic logging
 * UE_LOG(LogMidnightGrind, Log, TEXT("Player spawned at location: %s"), *Location.ToString());
 *
 * // Warning (shows in yellow in Output Log)
 * UE_LOG(LogMidnightGrind, Warning, TEXT("Health is critically low!"));
 *
 * // Error (shows in red)
 * UE_LOG(LogMidnightGrind, Error, TEXT("Failed to load save game!"));
 *
 * // Verbose (only shows when verbosity is increased)
 * UE_LOG(LogMidnightGrind, Verbose, TEXT("Tick called with DeltaTime: %f"), DeltaTime);
 * @endcode
 *
 * @subsection log_filtering Filtering Logs
 *
 * In the Output Log window, you can filter to show only `LogMidnightGrind`
 * messages, making it easy to focus on your game's specific output without
 * noise from engine subsystems.
 *
 * @section architecture Module Architecture Overview
 *
 * @subsection arch_hierarchy Where This Module Fits
 *
 * @verbatim
 * Unreal Engine Architecture
 * ==========================
 *
 *     +------------------+
 *     |   Unreal Engine  |  (Core, CoreUObject, Engine modules)
 *     +--------+---------+
 *              |
 *              v
 *     +------------------+
 *     |  Game Framework  |  (GameplayAbilities, EnhancedInput, etc.)
 *     +--------+---------+
 *              |
 *              v
 *     +------------------+
 *     |  MidnightGrind   |  <-- THIS MODULE (Your game code)
 *     +------------------+
 *              |
 *              v
 *     +------------------+
 *     |   Game Content   |  (Blueprints, Assets, Levels)
 *     +------------------+
 * @endverbatim
 *
 * @subsection arch_dependencies Module Dependencies
 *
 * This module depends on several Unreal modules (defined in MidnightGrind.Build.cs):
 * - **Core**: Fundamental types, containers, and utilities
 * - **CoreUObject**: UObject system, reflection, serialization
 * - **Engine**: Actors, Components, World, GameFramework classes
 *
 * @subsection arch_extension Extending the Module
 *
 * As your game grows, you might:
 * - Add global managers in StartupModule() (e.g., custom subsystems)
 * - Register console commands for debugging
 * - Initialize third-party libraries or services
 * - Create additional modules for large feature sets
 *
 * @see FMidnightGrindModule The main module class implementation
 * @see IModuleInterface Unreal's base interface for all modules
 * @see FModuleManager The engine system that manages module loading
 *
 * @author Midnight Grind Team
 */

// ============================================================================
// INCLUDES
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Creates a log category named "LogMidnightGrind" that can be used throughout the project
// Usage: UE_LOG(LogMidnightGrind, Warning, TEXT("Something happened!"));
// The "Log" parameter sets the default verbosity, "All" means compile all log levels
DECLARE_LOG_CATEGORY_EXTERN(LogMidnightGrind, Log, All);

/**
 * Main game module for MIDNIGHT GRIND
 *
 * This class is automatically instantiated by Unreal Engine when the game loads.
 * It provides lifecycle hooks (startup/shutdown) and a way for other systems
 * to check if the module is loaded.
 */
class FMidnightGrindModule : public IModuleInterface
{
public:
	// ========================================================================
	// LIFECYCLE METHODS
	// ========================================================================
	// These are called automatically by Unreal Engine at the appropriate times

	/**
	 * Called when the module is first loaded (game startup)
	 * Use this to initialize any global/static data your game needs
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is being unloaded (game shutdown)
	 * Use this to clean up any global resources
	 */
	virtual void ShutdownModule() override;

	// ========================================================================
	// STATIC ACCESS METHODS
	// ========================================================================
	// These provide a safe way to access the module from anywhere in code

	/**
	 * Singleton-like access to this module's interface
	 *
	 * HOW IT WORKS:
	 *   Uses FModuleManager to find and return the loaded module instance.
	 *   "LoadModuleChecked" means it will crash if the module isn't found,
	 *   so only use this when you KNOW the module should be loaded.
	 *
	 * USAGE EXAMPLE:
	 *   FMidnightGrindModule& MyModule = FMidnightGrindModule::Get();
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static FMidnightGrindModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMidnightGrindModule>("MidnightGrind");
	}

	/**
	 * Checks to see if this module is loaded and ready
	 *
	 * WHY USE THIS?
	 *   Before calling Get(), you might want to check if the module is loaded
	 *   to avoid crashes. This is especially useful during engine startup/shutdown
	 *   when module load order isn't guaranteed.
	 *
	 * USAGE EXAMPLE:
	 *   if (FMidnightGrindModule::IsAvailable())
	 *   {
	 *       FMidnightGrindModule& Module = FMidnightGrindModule::Get();
	 *   }
	 *
	 * @return True if the module is loaded, false otherwise
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("MidnightGrind");
	}
};
