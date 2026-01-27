// Copyright Midnight Grind. All Rights Reserved.
// ============================================================================
// MIDNIGHTGRINDMODULE.H - ALTERNATIVE MODULE DEFINITION
// ============================================================================
//
// NOTE: This appears to be a duplicate/alternative module definition.
//       The primary one is in MidnightGrind.h at the Source root.
//       This file may be kept for organizational purposes in the Public folder.
//
// PURPOSE:
//   Defines the game module class that Unreal Engine uses to manage
//   the lifecycle of the Midnight Grind game code.
//
// GAME DESCRIPTION:
//   Midnight Grind is an arcade street racing game inspired by the visual
//   style of PS1/PS2 era racing games. Think classic Need for Speed Underground
//   meets Initial D with a retro aesthetic.
//
// ============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Main game module for MIDNIGHT GRIND
 * Arcade street racing with PS1/PS2 era aesthetics
 *
 * WHAT THIS CLASS DOES:
 *   1. Tells Unreal this is a "game module" (not just a plugin/library)
 *   2. Provides hooks for initialization (StartupModule) and cleanup (ShutdownModule)
 *
 * WHY IsGameModule() RETURNS TRUE:
 *   Unreal Engine distinguishes between different module types:
 *   - Game modules: Your actual game code
 *   - Plugin modules: Optional extensions
 *   - Engine modules: Core Unreal functionality
 *   Returning true from IsGameModule() tells Unreal this IS the game.
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
