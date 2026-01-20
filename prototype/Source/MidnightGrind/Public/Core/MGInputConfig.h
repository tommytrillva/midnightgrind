// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MGInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Input Configuration Data Asset
 * Defines all input actions and mapping contexts for the vehicle
 *
 * Usage:
 * 1. Create this as a Data Asset in Content/Input/DA_VehicleInputConfig
 * 2. Assign to Default Vehicle in Project Settings or Game Instance
 * 3. Input actions will be auto-assigned to vehicle pawns
 *
 * Required Input Actions:
 * - IA_Throttle (Axis1D) - RT/W key
 * - IA_Brake (Axis1D) - LT/S key
 * - IA_Steering (Axis1D) - Left Stick X / A-D keys
 * - IA_Handbrake (Digital) - A button / Space
 * - IA_Nitrous (Digital) - B button / Shift
 * - IA_ShiftUp (Digital) - RB / E key
 * - IA_ShiftDown (Digital) - LB / Q key
 * - IA_CameraToggle (Digital) - Y button / C key
 * - IA_LookBehind (Digital) - RS Click / Tab
 * - IA_Reset (Digital) - Start / R key
 * - IA_Pause (Digital) - Start / Escape
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGInputConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UMGInputConfig();

	// ==========================================
	// MAPPING CONTEXTS
	// ==========================================

	/** Main vehicle input mapping context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contexts")
	TSoftObjectPtr<UInputMappingContext> VehicleContext;

	/** Menu/UI input mapping context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Contexts")
	TSoftObjectPtr<UInputMappingContext> MenuContext;

	// ==========================================
	// VEHICLE INPUT ACTIONS
	// ==========================================

	/** Throttle (0-1 axis) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_Throttle;

	/** Brake (0-1 axis) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_Brake;

	/** Steering (-1 to 1 axis) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_Steering;

	/** Handbrake (digital) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_Handbrake;

	/** Nitrous (digital hold) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_Nitrous;

	/** Shift up (digital) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_ShiftUp;

	/** Shift down (digital) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Vehicle")
	TSoftObjectPtr<UInputAction> IA_ShiftDown;

	// ==========================================
	// CAMERA INPUT ACTIONS
	// ==========================================

	/** Cycle camera modes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Camera")
	TSoftObjectPtr<UInputAction> IA_CameraToggle;

	/** Look behind (hold) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Camera")
	TSoftObjectPtr<UInputAction> IA_LookBehind;

	// ==========================================
	// GAME INPUT ACTIONS
	// ==========================================

	/** Reset/respawn vehicle */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Game")
	TSoftObjectPtr<UInputAction> IA_Reset;

	/** Pause game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actions|Game")
	TSoftObjectPtr<UInputAction> IA_Pause;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get the primary asset id for this config */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Load all input actions synchronously */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void LoadAllInputActions();

	/** Check if all required actions are assigned */
	UFUNCTION(BlueprintPure, Category = "Input")
	bool AreAllActionsAssigned() const;

	/** Get list of missing action names */
	UFUNCTION(BlueprintPure, Category = "Input")
	TArray<FString> GetMissingActionNames() const;
};

/**
 * Developer Settings for Input Configuration
 * Access via Project Settings -> Game -> Midnight Grind Input
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Midnight Grind Input"))
class MIDNIGHTGRIND_API UMGInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMGInputSettings();

	/** Default input configuration asset */
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Input",
		meta = (AllowedClasses = "/Script/MidnightGrind.MGInputConfig"))
	FSoftObjectPath DefaultInputConfig;

	/** Get the default input config (loads if needed) */
	UFUNCTION(BlueprintPure, Category = "Input")
	static UMGInputConfig* GetDefaultInputConfig();

	/** Get the CDO */
	static UMGInputSettings* Get();

#if WITH_EDITOR
	virtual FText GetSectionText() const override { return NSLOCTEXT("MidnightGrind", "InputSettingsName", "Midnight Grind Input"); }
	virtual FText GetSectionDescription() const override { return NSLOCTEXT("MidnightGrind", "InputSettingsDesc", "Configure default input actions and mapping contexts"); }
#endif
};
