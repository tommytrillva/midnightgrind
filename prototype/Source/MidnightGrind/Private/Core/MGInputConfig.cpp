// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGInputConfig.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/AssetManager.h"

// ==========================================
// UMGInputConfig
// ==========================================

UMGInputConfig::UMGInputConfig()
{
}

FPrimaryAssetId UMGInputConfig::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("InputConfig"), GetFName());
}

void UMGInputConfig::LoadAllInputActions()
{
	// Load all soft references synchronously
	if (!IA_Throttle.IsNull()) IA_Throttle.LoadSynchronous();
	if (!IA_Brake.IsNull()) IA_Brake.LoadSynchronous();
	if (!IA_Steering.IsNull()) IA_Steering.LoadSynchronous();
	if (!IA_Handbrake.IsNull()) IA_Handbrake.LoadSynchronous();
	if (!IA_Nitrous.IsNull()) IA_Nitrous.LoadSynchronous();
	if (!IA_ShiftUp.IsNull()) IA_ShiftUp.LoadSynchronous();
	if (!IA_ShiftDown.IsNull()) IA_ShiftDown.LoadSynchronous();
	if (!IA_CameraToggle.IsNull()) IA_CameraToggle.LoadSynchronous();
	if (!IA_LookBehind.IsNull()) IA_LookBehind.LoadSynchronous();
	if (!IA_Reset.IsNull()) IA_Reset.LoadSynchronous();
	if (!IA_Pause.IsNull()) IA_Pause.LoadSynchronous();

	if (!VehicleContext.IsNull()) VehicleContext.LoadSynchronous();
	if (!MenuContext.IsNull()) MenuContext.LoadSynchronous();

	UE_LOG(LogTemp, Log, TEXT("UMGInputConfig: Loaded all input actions"));
}

bool UMGInputConfig::AreAllActionsAssigned() const
{
	return GetMissingActionNames().Num() == 0;
}

TArray<FString> UMGInputConfig::GetMissingActionNames() const
{
	TArray<FString> Missing;

	if (IA_Throttle.IsNull()) Missing.Add(TEXT("IA_Throttle"));
	if (IA_Brake.IsNull()) Missing.Add(TEXT("IA_Brake"));
	if (IA_Steering.IsNull()) Missing.Add(TEXT("IA_Steering"));
	if (IA_Handbrake.IsNull()) Missing.Add(TEXT("IA_Handbrake"));
	if (IA_Nitrous.IsNull()) Missing.Add(TEXT("IA_Nitrous"));
	if (IA_ShiftUp.IsNull()) Missing.Add(TEXT("IA_ShiftUp"));
	if (IA_ShiftDown.IsNull()) Missing.Add(TEXT("IA_ShiftDown"));
	if (IA_CameraToggle.IsNull()) Missing.Add(TEXT("IA_CameraToggle"));
	if (IA_LookBehind.IsNull()) Missing.Add(TEXT("IA_LookBehind"));
	if (IA_Reset.IsNull()) Missing.Add(TEXT("IA_Reset"));
	if (IA_Pause.IsNull()) Missing.Add(TEXT("IA_Pause"));
	if (VehicleContext.IsNull()) Missing.Add(TEXT("VehicleContext"));

	return Missing;
}

// ==========================================
// UMGInputSettings
// ==========================================

UMGInputSettings::UMGInputSettings()
{
	// Default path for input config
	DefaultInputConfig = FSoftObjectPath(TEXT("/Game/Input/DA_VehicleInputConfig.DA_VehicleInputConfig"));
}

UMGInputSettings* UMGInputSettings::Get()
{
	return GetMutableDefault<UMGInputSettings>();
}

UMGInputConfig* UMGInputSettings::GetDefaultInputConfig()
{
	UMGInputSettings* Settings = Get();
	if (!Settings)
	{
		return nullptr;
	}

	if (Settings->DefaultInputConfig.IsValid())
	{
		return Cast<UMGInputConfig>(Settings->DefaultInputConfig.TryLoad());
	}

	return nullptr;
}
