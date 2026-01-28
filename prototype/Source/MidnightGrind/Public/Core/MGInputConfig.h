// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGInputConfig.h - Input Configuration Data Asset
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines a Data Asset that stores all the input action references
 * for the game. Instead of hardcoding input bindings in code, we store them
 * in a configurable asset that designers can edit in the Unreal Editor.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. DATA ASSETS (UDataAsset / UPrimaryDataAsset):
 *    - A Data Asset is a way to store configuration data in an asset file
 *    - Unlike Blueprints, Data Assets don't have logic - just data
 *    - Designers can edit them in the editor without touching code
 *    - UPrimaryDataAsset is used for assets managed by the Asset Manager
 *
 * 2. ENHANCED INPUT SYSTEM:
 *    - Unreal Engine 5's modern input system (replaces the old input system)
 *    - UInputAction: Represents a single action (e.g., "Jump", "Throttle")
 *    - UInputMappingContext: Maps physical inputs (keys/buttons) to actions
 *    - Allows one action to have multiple bindings (keyboard + gamepad)
 *
 * 3. SOFT OBJECT POINTERS (TSoftObjectPtr):
 *    - A "soft" reference that doesn't force the asset to load immediately
 *    - Assets load on-demand when you actually need them
 *    - Reduces memory usage and startup time
 *    - Use .LoadSynchronous() or async loading to get the actual object
 *
 * 4. DEVELOPER SETTINGS (UDeveloperSettings):
 *    - Creates a section in Project Settings for this module
 *    - Allows setting default values that persist across sessions
 *    - Access via GetDefault<UMGInputSettings>() or UMGInputSettings::Get()
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 * - UMGInputConfig is created as a Data Asset in the Content Browser
 * - UMGInputSettings (in Project Settings) points to the default config
 * - AMGPlayerController loads this config at startup
 * - The config provides references to all input actions needed for driving
 *
 * UNREAL ENGINE MACROS USED:
 * --------------------------
 * - UCLASS(): Makes this class visible to Unreal's reflection system
 * - UPROPERTY(): Exposes variables to the editor and serialization
 * - UFUNCTION(): Exposes functions to Blueprints
 * - GENERATED_BODY(): Required macro that generates boilerplate code
 * - BlueprintType: Allows this class to be used in Blueprints
 * - EditDefaultsOnly: Property editable only in the Class Defaults
 * - BlueprintReadOnly: Readable in Blueprints but not writable
 *
 * USAGE EXAMPLE:
 * --------------
 * @code
 * // In PlayerController or Pawn setup:
 * UMGInputConfig* Config = UMGInputSettings::GetDefaultInputConfig();
 * if (Config)
 * {
 *     // Load the throttle action synchronously
 *     UInputAction* Throttle = Config->IA_Throttle.LoadSynchronous();
 *     // Bind to this action...
 * }
 * @endcode
 *
 * REQUIRED INPUT ACTIONS (create these in Content/Input/):
 * --------------------------------------------------------
 * - IA_Throttle (Axis1D) - RT/W key - Accelerate the vehicle
 * - IA_Brake (Axis1D) - LT/S key - Brake/reverse
 * - IA_Steering (Axis1D) - Left Stick X / A-D keys - Turn left/right
 * - IA_Handbrake (Digital) - A button / Space - Drift/handbrake turn
 * - IA_Nitrous (Digital) - B button / Shift - Nitro boost
 * - IA_ShiftUp (Digital) - RB / E key - Manual transmission shift up
 * - IA_ShiftDown (Digital) - LB / Q key - Manual transmission shift down
 * - IA_CameraToggle (Digital) - Y button / C key - Cycle camera views
 * - IA_LookBehind (Digital) - RS Click / Tab - Rear view mirror
 * - IA_Reset (Digital) - Start / R key - Respawn vehicle
 * - IA_Pause (Digital) - Start / Escape - Pause menu
 *
 * @see AMGPlayerController - Uses this config to set up input bindings
 * @see UMGInputRemapSubsystem - Handles runtime input remapping
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MGInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;
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
 *
 * This class creates a settings section in Project Settings that allows
 * designers to configure the default input asset without touching code.
 *
 * HOW TO ACCESS IN PROJECT SETTINGS:
 * Go to: Edit -> Project Settings -> Game -> Midnight Grind Input
 *
 * KEY UNREAL CONCEPTS:
 * - config = Game: Saves to DefaultGame.ini (or Game.ini in Saved/Config)
 * - defaultconfig: Uses the default config file, not a per-platform override
 * - UDeveloperSettings: Base class for Project Settings sections
 *
 * ACCESS FROM CODE:
 * @code
 * // Get the settings singleton:
 * UMGInputSettings* Settings = UMGInputSettings::Get();
 *
 * // Or get the default input config directly:
 * UMGInputConfig* Config = UMGInputSettings::GetDefaultInputConfig();
 * @endcode
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
