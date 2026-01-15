// Copyright Midnight Grind. All Rights Reserved.

#include "MidnightGrindEditorModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "MidnightGrindEditor"

void FMidnightGrindEditorModule::StartupModule()
{
	// Register custom asset types
	RegisterAssetTypes();

	// Register custom editor modes
	RegisterEditorModes();

	// Register menu extensions
	RegisterMenuExtensions();

	UE_LOG(LogTemp, Log, TEXT("MidnightGrindEditor module started"));
}

void FMidnightGrindEditorModule::ShutdownModule()
{
	// Unregister everything
	UnregisterMenuExtensions();
	UnregisterEditorModes();
	UnregisterAssetTypes();

	UE_LOG(LogTemp, Log, TEXT("MidnightGrindEditor module shutdown"));
}

void FMidnightGrindEditorModule::RegisterAssetTypes()
{
	// TODO: Register custom asset types for:
	// - Vehicle Data Assets
	// - Part Definition Assets
	// - Track Definition Assets
	// - Audio Bank Assets
}

void FMidnightGrindEditorModule::UnregisterAssetTypes()
{
	// TODO: Unregister custom asset types
}

void FMidnightGrindEditorModule::RegisterEditorModes()
{
	// TODO: Register custom editor modes for:
	// - Track Editor
	// - Vehicle Setup Editor
	// - Environment Placement Editor
}

void FMidnightGrindEditorModule::UnregisterEditorModes()
{
	// TODO: Unregister custom editor modes
}

void FMidnightGrindEditorModule::RegisterMenuExtensions()
{
	// TODO: Add menu items for:
	// - MIDNIGHT GRIND tools menu
	// - Quick access to vehicle testing
	// - Batch asset operations
}

void FMidnightGrindEditorModule::UnregisterMenuExtensions()
{
	// TODO: Remove menu extensions
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMidnightGrindEditorModule, MidnightGrindEditor)
