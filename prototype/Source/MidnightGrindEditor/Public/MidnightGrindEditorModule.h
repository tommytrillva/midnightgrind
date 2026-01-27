// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Framework/MultiBox/MultiBoxExtender.h"

class FUICommandList;
class FExtender;
class IAssetTypeActions;

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

	/** Get the module instance */
	static FMidnightGrindEditorModule& Get();

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

	/** Build the Midnight Grind tools menu */
	void BuildMidnightGrindMenu(FMenuBuilder& MenuBuilder);

	/** Menu command handlers */
	void OpenVehicleTester();
	void OpenTrackValidator();
	void OpenAssetAuditor();
	void ReloadAllDataTables();
	void ExportVehicleCatalog();
	void ExportPartsCatalog();

	/** Registered asset type actions for cleanup */
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;

	/** Menu extender handle */
	TSharedPtr<FExtender> MenuExtender;

	/** UI command list for menu actions */
	TSharedPtr<FUICommandList> PluginCommands;
};
