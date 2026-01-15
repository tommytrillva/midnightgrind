// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMidnightGrind, Log, All);

/**
 * Main game module for MIDNIGHT GRIND
 */
class FMidnightGrindModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static FMidnightGrindModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FMidnightGrindModule>("MidnightGrind");
	}

	/**
	 * Checks to see if this module is loaded and ready
	 * @return True if the module is loaded
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("MidnightGrind");
	}
};
