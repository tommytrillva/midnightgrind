// Copyright Midnight Grind. All Rights Reserved.
// ============================================================================
// MIDNIGHTGRIND.H - PRIMARY GAME MODULE HEADER
// ============================================================================
//
// PURPOSE:
//   This is the main entry point header for the Midnight Grind game module.
//   Every Unreal Engine game/plugin needs a module class that handles
//   initialization and shutdown. This file defines that module.
//
// WHAT IS A MODULE?
//   In Unreal Engine, a "module" is a self-contained unit of code that can be
//   loaded and unloaded. Your game is a module, and it can have dependencies
//   on other modules (like the Engine module, Slate for UI, etc.).
//
// WHEN IS THIS USED?
//   - StartupModule() is called when the game first loads
//   - ShutdownModule() is called when the game exits
//   - Get() and IsAvailable() let other code safely access this module
//
// KEY CONCEPTS FOR NEW DEVELOPERS:
//   1. DECLARE_LOG_CATEGORY_EXTERN - Creates a log category so you can filter
//      game-specific logs in the Output Log (e.g., UE_LOG(LogMidnightGrind, Log, TEXT("Hello"));)
//   2. IModuleInterface - Base class all modules must inherit from
//   3. FModuleManager - Unreal's system for loading/unloading modules
//
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
