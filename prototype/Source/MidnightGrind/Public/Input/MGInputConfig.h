// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "MGInputConfig.generated.h"

/**
 * Input action configuration for a single action
 * Used to define default bindings
 */
USTRUCT(BlueprintType)
struct FMGInputActionBinding
{
	GENERATED_BODY()

	/** The input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> Action;

	/** Gamepad binding */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FKey GamepadKey;

	/** Keyboard binding (primary) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FKey KeyboardKey;

	/** Keyboard binding (alternate) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FKey KeyboardKeyAlt;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	FText DisplayName;

	/** Can this binding be remapped? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	bool bAllowRemap = true;
};

/**
 * Data asset containing all vehicle input configuration
 * Create one of these in the editor and assign to vehicles
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Input mapping context for vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Input priority (higher = takes precedence) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 InputPriority = 1;

	// ==========================================
	// DRIVING ACTIONS
	// ==========================================

	/** Throttle/Accelerate */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Driving")
	TObjectPtr<UInputAction> ThrottleAction;

	/** Brake/Reverse */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Driving")
	TObjectPtr<UInputAction> BrakeAction;

	/** Steering (axis) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Driving")
	TObjectPtr<UInputAction> SteeringAction;

	/** Handbrake/E-brake */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Driving")
	TObjectPtr<UInputAction> HandbrakeAction;

	/** Nitrous/Boost */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Driving")
	TObjectPtr<UInputAction> NitrousAction;

	// ==========================================
	// TRANSMISSION ACTIONS
	// ==========================================

	/** Shift up */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Transmission")
	TObjectPtr<UInputAction> ShiftUpAction;

	/** Shift down */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Transmission")
	TObjectPtr<UInputAction> ShiftDownAction;

	// ==========================================
	// CAMERA ACTIONS
	// ==========================================

	/** Cycle camera view */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> CameraCycleAction;

	/** Look behind */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> LookBehindAction;

	/** Free look (right stick) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Camera")
	TObjectPtr<UInputAction> FreeLookAction;

	// ==========================================
	// GAME ACTIONS
	// ==========================================

	/** Reset/respawn vehicle */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Game")
	TObjectPtr<UInputAction> ResetVehicleAction;

	/** Pause menu */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Game")
	TObjectPtr<UInputAction> PauseAction;

	/** Map/GPS */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Game")
	TObjectPtr<UInputAction> MapAction;

	// ==========================================
	// DEFAULT BINDINGS
	// ==========================================

	/** Get default gamepad binding for an action */
	UFUNCTION(BlueprintPure, Category = "Input")
	static FKey GetDefaultGamepadBinding(FName ActionName);

	/** Get default keyboard binding for an action */
	UFUNCTION(BlueprintPure, Category = "Input")
	static FKey GetDefaultKeyboardBinding(FName ActionName);
};

/**
 * Data asset for menu/UI input configuration
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMenuInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Input mapping context for menus */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Navigate (D-pad/stick) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Navigation")
	TObjectPtr<UInputAction> NavigateAction;

	/** Confirm/Select */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Navigation")
	TObjectPtr<UInputAction> ConfirmAction;

	/** Back/Cancel */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Navigation")
	TObjectPtr<UInputAction> BackAction;

	/** Tab left (LB) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Navigation")
	TObjectPtr<UInputAction> TabLeftAction;

	/** Tab right (RB) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Navigation")
	TObjectPtr<UInputAction> TabRightAction;

	/** Special action (Y button) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Navigation")
	TObjectPtr<UInputAction> SpecialAction;
};

/**
 * Utility class for input helpers
 */
UCLASS()
class MIDNIGHTGRIND_API UMGInputUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Get the icon texture for a key (gamepad buttons, keyboard keys) */
	UFUNCTION(BlueprintPure, Category = "Input|Icons")
	static TSoftObjectPtr<UTexture2D> GetKeyIcon(FKey Key, bool bGamepad);

	/** Get display text for a key */
	UFUNCTION(BlueprintPure, Category = "Input|Icons")
	static FText GetKeyDisplayText(FKey Key);

	/** Check if player is using gamepad */
	UFUNCTION(BlueprintPure, Category = "Input", meta = (WorldContext = "WorldContextObject"))
	static bool IsUsingGamepad(const UObject* WorldContextObject);

	/** Apply dead zone to axis input */
	UFUNCTION(BlueprintPure, Category = "Input")
	static float ApplyDeadZone(float Value, float DeadZone, float MaxValue = 1.0f);

	/** Apply sensitivity curve to input */
	UFUNCTION(BlueprintPure, Category = "Input")
	static float ApplySensitivityCurve(float Value, float Sensitivity, float Exponent = 2.0f);
};
