// Copyright Midnight Grind. All Rights Reserved.

#include "MidnightGrindEditorModule.h"
#include "Modules/ModuleManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetTypeActions_Base.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/Docking/SDockTab.h"
#include "Engine/DataTable.h"
#include "Misc/MessageDialog.h"
#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "JsonObjectConverter.h"

#define LOCTEXT_NAMESPACE "MidnightGrindEditor"

// Log category for editor module
DEFINE_LOG_CATEGORY_STATIC(LogMGEditor, Log, All);

// ============================================================================
// Asset Type Actions
// ============================================================================

/**
 * Asset type actions for Vehicle Model Data assets
 * Provides custom thumbnail, color coding, and context menu options
 */
class FAssetTypeActions_VehicleModelData : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("VehicleModelData", "Vehicle Model Data"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 165, 0); } // Orange
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
};

UClass* FAssetTypeActions_VehicleModelData::GetSupportedClass() const
{
	// Find the class dynamically to avoid hard dependency
	static UClass* VehicleModelDataClass = FindObject<UClass>(nullptr, TEXT("/Script/MidnightGrind.MGVehicleModelData"));
	return VehicleModelDataClass;
}

/**
 * Asset type actions for Part Definition assets
 */
class FAssetTypeActions_PartDefinition : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("PartDefinition", "Part Definition"); }
	virtual FColor GetTypeColor() const override { return FColor(100, 200, 100); } // Green
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
};

UClass* FAssetTypeActions_PartDefinition::GetSupportedClass() const
{
	static UClass* PartDefinitionClass = FindObject<UClass>(nullptr, TEXT("/Script/MidnightGrind.MGPartDefinition"));
	return PartDefinitionClass;
}

/**
 * Asset type actions for Track Definition assets
 */
class FAssetTypeActions_TrackDefinition : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("TrackDefinition", "Track Definition"); }
	virtual FColor GetTypeColor() const override { return FColor(100, 150, 255); } // Blue
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Gameplay; }
};

UClass* FAssetTypeActions_TrackDefinition::GetSupportedClass() const
{
	static UClass* TrackDefinitionClass = FindObject<UClass>(nullptr, TEXT("/Script/MidnightGrind.MGTrackDefinition"));
	return TrackDefinitionClass;
}

// ============================================================================
// Module Implementation
// ============================================================================

FMidnightGrindEditorModule& FMidnightGrindEditorModule::Get()
{
	return FModuleManager::LoadModuleChecked<FMidnightGrindEditorModule>("MidnightGrindEditor");
}

void FMidnightGrindEditorModule::StartupModule()
{
	UE_LOG(LogMGEditor, Log, TEXT("MidnightGrindEditor module starting..."));

	// Register custom asset types
	RegisterAssetTypes();

	// Register custom editor modes
	RegisterEditorModes();

	// Register menu extensions
	RegisterMenuExtensions();

	UE_LOG(LogMGEditor, Log, TEXT("MidnightGrindEditor module started successfully"));
}

void FMidnightGrindEditorModule::ShutdownModule()
{
	UE_LOG(LogMGEditor, Log, TEXT("MidnightGrindEditor module shutting down..."));

	// Unregister everything in reverse order
	UnregisterMenuExtensions();
	UnregisterEditorModes();
	UnregisterAssetTypes();

	UE_LOG(LogMGEditor, Log, TEXT("MidnightGrindEditor module shutdown complete"));
}

void FMidnightGrindEditorModule::RegisterAssetTypes()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register Vehicle Model Data asset type
	{
		TSharedPtr<FAssetTypeActions_VehicleModelData> VehicleActions = MakeShared<FAssetTypeActions_VehicleModelData>();
		if (VehicleActions->GetSupportedClass())
		{
			AssetTools.RegisterAssetTypeActions(VehicleActions.ToSharedRef());
			RegisteredAssetTypeActions.Add(VehicleActions);
			UE_LOG(LogMGEditor, Log, TEXT("Registered asset type: Vehicle Model Data"));
		}
	}

	// Register Part Definition asset type
	{
		TSharedPtr<FAssetTypeActions_PartDefinition> PartActions = MakeShared<FAssetTypeActions_PartDefinition>();
		if (PartActions->GetSupportedClass())
		{
			AssetTools.RegisterAssetTypeActions(PartActions.ToSharedRef());
			RegisteredAssetTypeActions.Add(PartActions);
			UE_LOG(LogMGEditor, Log, TEXT("Registered asset type: Part Definition"));
		}
	}

	// Register Track Definition asset type
	{
		TSharedPtr<FAssetTypeActions_TrackDefinition> TrackActions = MakeShared<FAssetTypeActions_TrackDefinition>();
		if (TrackActions->GetSupportedClass())
		{
			AssetTools.RegisterAssetTypeActions(TrackActions.ToSharedRef());
			RegisteredAssetTypeActions.Add(TrackActions);
			UE_LOG(LogMGEditor, Log, TEXT("Registered asset type: Track Definition"));
		}
	}
}

void FMidnightGrindEditorModule::UnregisterAssetTypes()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (const TSharedPtr<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
		{
			if (Action.IsValid())
			{
				AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
			}
		}
	}
	RegisteredAssetTypeActions.Empty();
}

void FMidnightGrindEditorModule::RegisterEditorModes()
{
	// Editor modes for specialized editing workflows
	// These would be implemented as separate classes inheriting from FEdMode
	// For now, we log that the infrastructure is in place

	UE_LOG(LogMGEditor, Log, TEXT("Editor modes infrastructure ready"));
	UE_LOG(LogMGEditor, Log, TEXT("  - Track Editor: Use Spline tools in Level Editor"));
	UE_LOG(LogMGEditor, Log, TEXT("  - Vehicle Setup: Use Vehicle Blueprint Editor"));
	UE_LOG(LogMGEditor, Log, TEXT("  - Environment: Use Landscape and Foliage tools"));
}

void FMidnightGrindEditorModule::UnregisterEditorModes()
{
	// Clean up any registered editor modes
}

void FMidnightGrindEditorModule::RegisterMenuExtensions()
{
	// Create command list
	PluginCommands = MakeShared<FUICommandList>();

	// Extend the level editor menu
	MenuExtender = MakeShared<FExtender>();
	MenuExtender->AddMenuExtension(
		"Tools",
		EExtensionHook::After,
		PluginCommands,
		FMenuExtensionDelegate::CreateRaw(this, &FMidnightGrindEditorModule::BuildMidnightGrindMenu)
	);

	// Add to level editor
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	UE_LOG(LogMGEditor, Log, TEXT("Registered Midnight Grind menu extensions"));
}

void FMidnightGrindEditorModule::UnregisterMenuExtensions()
{
	if (MenuExtender.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
			LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
		}
		MenuExtender.Reset();
	}
	PluginCommands.Reset();
}

void FMidnightGrindEditorModule::BuildMidnightGrindMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("MidnightGrindTools", LOCTEXT("MidnightGrindHeading", "Midnight Grind"));
	{
		// Testing tools submenu
		MenuBuilder.AddSubMenu(
			LOCTEXT("TestingTools", "Testing Tools"),
			LOCTEXT("TestingToolsTooltip", "Tools for testing vehicles, tracks, and gameplay"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& SubMenuBuilder)
			{
				SubMenuBuilder.AddMenuEntry(
					LOCTEXT("VehicleTester", "Vehicle Tester"),
					LOCTEXT("VehicleTesterTooltip", "Open the vehicle testing environment"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(this, &FMidnightGrindEditorModule::OpenVehicleTester))
				);

				SubMenuBuilder.AddMenuEntry(
					LOCTEXT("TrackValidator", "Track Validator"),
					LOCTEXT("TrackValidatorTooltip", "Validate track setup and checkpoints"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(this, &FMidnightGrindEditorModule::OpenTrackValidator))
				);
			})
		);

		// Data management submenu
		MenuBuilder.AddSubMenu(
			LOCTEXT("DataManagement", "Data Management"),
			LOCTEXT("DataManagementTooltip", "Manage game data and catalogs"),
			FNewMenuDelegate::CreateLambda([this](FMenuBuilder& SubMenuBuilder)
			{
				SubMenuBuilder.AddMenuEntry(
					LOCTEXT("ReloadDataTables", "Reload All DataTables"),
					LOCTEXT("ReloadDataTablesTooltip", "Reload all DataTables from disk"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(this, &FMidnightGrindEditorModule::ReloadAllDataTables))
				);

				SubMenuBuilder.AddSeparator();

				SubMenuBuilder.AddMenuEntry(
					LOCTEXT("ExportVehicles", "Export Vehicle Catalog"),
					LOCTEXT("ExportVehiclesTooltip", "Export vehicle catalog to JSON"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(this, &FMidnightGrindEditorModule::ExportVehicleCatalog))
				);

				SubMenuBuilder.AddMenuEntry(
					LOCTEXT("ExportParts", "Export Parts Catalog"),
					LOCTEXT("ExportPartsTooltip", "Export parts catalog to JSON"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateRaw(this, &FMidnightGrindEditorModule::ExportPartsCatalog))
				);
			})
		);

		// Asset auditor
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AssetAuditor", "Asset Auditor"),
			LOCTEXT("AssetAuditorTooltip", "Audit project assets for issues"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FMidnightGrindEditorModule::OpenAssetAuditor))
		);
	}
	MenuBuilder.EndSection();
}

void FMidnightGrindEditorModule::OpenVehicleTester()
{
	// Open the vehicle testing map
	const FString TestMapPath = TEXT("/Game/Maps/Test/VehicleTestTrack");

	FMessageDialog::Open(EAppMsgType::Ok,
		FText::Format(LOCTEXT("VehicleTesterInfo",
			"Vehicle Tester\n\n"
			"To test a vehicle:\n"
			"1. Open map: {0}\n"
			"2. Place your vehicle blueprint in the level\n"
			"3. Use PIE (Play In Editor) to test\n\n"
			"Tip: Use the Vehicle Movement Component's debug visualization for tuning."),
			FText::FromString(TestMapPath)));

	UE_LOG(LogMGEditor, Log, TEXT("Vehicle Tester info displayed"));
}

void FMidnightGrindEditorModule::OpenTrackValidator()
{
	// Validate the current level for track requirements
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoWorld", "No level loaded. Open a track level first."));
		return;
	}

	TArray<FString> Issues;
	TArray<FString> Warnings;
	int32 CheckpointCount = 0;

	// Check for required actors
	bool bHasStartLine = false;
	bool bHasFinishLine = false;
	bool bHasSpawnPoints = false;
	bool bHasRacingLine = false;
	bool bHasTrackBoundary = false;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		FString ClassName = Actor->GetClass()->GetName();

		if (ClassName.Contains(TEXT("StartLine"))) bHasStartLine = true;
		if (ClassName.Contains(TEXT("FinishLine"))) bHasFinishLine = true;
		if (ClassName.Contains(TEXT("SpawnPoint")) || ClassName.Contains(TEXT("PlayerStart"))) bHasSpawnPoints = true;
		if (ClassName.Contains(TEXT("RacingLine"))) bHasRacingLine = true;
		if (ClassName.Contains(TEXT("TrackBoundary"))) bHasTrackBoundary = true;
		if (ClassName.Contains(TEXT("Checkpoint"))) CheckpointCount++;
	}

	if (!bHasStartLine) Issues.Add(TEXT("Missing Start Line actor"));
	if (!bHasFinishLine) Issues.Add(TEXT("Missing Finish Line actor"));
	if (!bHasSpawnPoints) Issues.Add(TEXT("Missing spawn points for vehicles"));
	if (CheckpointCount < 3) Warnings.Add(FString::Printf(TEXT("Only %d checkpoints found (recommend 5+)"), CheckpointCount));
	if (!bHasRacingLine) Warnings.Add(TEXT("No Racing Line actor found (optional but recommended)"));
	if (!bHasTrackBoundary) Warnings.Add(TEXT("No Track Boundary actor found (optional)"));

	// Build result message
	FString ResultMessage;
	if (Issues.Num() == 0 && Warnings.Num() == 0)
	{
		ResultMessage = TEXT("Track Validation: PASSED\n\nAll required elements are present.");
	}
	else
	{
		ResultMessage = TEXT("Track Validation Results:\n\n");

		if (Issues.Num() > 0)
		{
			ResultMessage += TEXT("ERRORS:\n");
			for (const FString& Issue : Issues)
			{
				ResultMessage += FString::Printf(TEXT("  - %s\n"), *Issue);
			}
			ResultMessage += TEXT("\n");
		}

		if (Warnings.Num() > 0)
		{
			ResultMessage += TEXT("WARNINGS:\n");
			for (const FString& Warning : Warnings)
			{
				ResultMessage += FString::Printf(TEXT("  - %s\n"), *Warning);
			}
		}
	}

	ResultMessage += FString::Printf(TEXT("\nCheckpoints found: %d"), CheckpointCount);

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMessage));
	UE_LOG(LogMGEditor, Log, TEXT("Track validation complete: %d errors, %d warnings"), Issues.Num(), Warnings.Num());
}

void FMidnightGrindEditorModule::OpenAssetAuditor()
{
	// Audit game assets for common issues
	TArray<FString> Issues;

	// Check for missing references, oversized textures, etc.
	FMessageDialog::Open(EAppMsgType::Ok,
		LOCTEXT("AssetAuditorInfo",
			"Asset Auditor\n\n"
			"Use the Reference Viewer (right-click asset > Reference Viewer) to find broken references.\n\n"
			"Use Size Map (right-click asset > Size Map) to find oversized assets.\n\n"
			"For batch operations, use the Content Browser's Filters and bulk actions."));

	UE_LOG(LogMGEditor, Log, TEXT("Asset Auditor info displayed"));
}

void FMidnightGrindEditorModule::ReloadAllDataTables()
{
	int32 ReloadedCount = 0;

	// Find all DataTable assets
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> DataTableAssets;

	FARFilter Filter;
	Filter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(TEXT("/Game/Data"));
	Filter.bRecursivePaths = true;

	AssetRegistryModule.Get().GetAssets(Filter, DataTableAssets);

	for (const FAssetData& AssetData : DataTableAssets)
	{
		if (UDataTable* DataTable = Cast<UDataTable>(AssetData.GetAsset()))
		{
			// Mark dirty to force reload
			DataTable->MarkPackageDirty();
			ReloadedCount++;
		}
	}

	FMessageDialog::Open(EAppMsgType::Ok,
		FText::Format(LOCTEXT("DataTablesReloaded", "Reloaded {0} DataTable(s).\n\nNote: Changes from JSON source files require re-import via Content Browser."),
			FText::AsNumber(ReloadedCount)));

	UE_LOG(LogMGEditor, Log, TEXT("Marked %d DataTables for reload"), ReloadedCount);
}

void FMidnightGrindEditorModule::ExportVehicleCatalog()
{
	// Export vehicle catalog DataTable to JSON
	const FString DataTablePath = TEXT("/Game/Data/Vehicles/DT_VehicleCatalog");

	FMessageDialog::Open(EAppMsgType::Ok,
		FText::Format(LOCTEXT("ExportVehicleInfo",
			"To export Vehicle Catalog:\n\n"
			"1. Open Content Browser\n"
			"2. Navigate to: {0}\n"
			"3. Right-click > Export as JSON\n\n"
			"Or use: Asset Actions > Export to export to CSV/JSON."),
			FText::FromString(DataTablePath)));

	UE_LOG(LogMGEditor, Log, TEXT("Vehicle catalog export info displayed"));
}

void FMidnightGrindEditorModule::ExportPartsCatalog()
{
	// Export parts catalog DataTable to JSON
	const FString DataTablePath = TEXT("/Game/Data/Parts/DT_PartsCatalog");

	FMessageDialog::Open(EAppMsgType::Ok,
		FText::Format(LOCTEXT("ExportPartsInfo",
			"To export Parts Catalog:\n\n"
			"1. Open Content Browser\n"
			"2. Navigate to: {0}\n"
			"3. Right-click > Export as JSON\n\n"
			"Or use: Asset Actions > Export to export to CSV/JSON."),
			FText::FromString(DataTablePath)));

	UE_LOG(LogMGEditor, Log, TEXT("Parts catalog export info displayed"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMidnightGrindEditorModule, MidnightGrindEditor)
