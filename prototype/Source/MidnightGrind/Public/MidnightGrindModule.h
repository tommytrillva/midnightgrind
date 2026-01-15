// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Main game module for MIDNIGHT GRIND
 * Arcade street racing with PS1/PS2 era aesthetics
 */
class FMidnightGrindModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Check if module is loaded and ready */
	virtual bool IsGameModule() const override { return true; }
};
