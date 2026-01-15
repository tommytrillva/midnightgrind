// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Editor module for MIDNIGHT GRIND
 * Provides custom editor tools, asset types, and workflow utilities
 */
class FMidnightGrindEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register custom asset types */
	void RegisterAssetTypes();
	void UnregisterAssetTypes();

	/** Register custom editor modes */
	void RegisterEditorModes();
	void UnregisterEditorModes();

	/** Register menu extensions */
	void RegisterMenuExtensions();
	void UnregisterMenuExtensions();
};
