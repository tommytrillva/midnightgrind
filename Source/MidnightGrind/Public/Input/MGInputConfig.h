// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGInputConfig.h
 * @brief Input configuration data assets for vehicle and menu controls.
 *
 * @section Overview
 * This file defines data assets that configure how player input is mapped to
 * game actions in Midnight Grind. It uses Unreal Engine 5's Enhanced Input System
 * to provide flexible, rebindable controls for both driving and menu navigation.
 *
 * Data assets are created in the Unreal Editor and assigned to vehicles/UI systems.
 * This separates input configuration from code, making it easy to:
 * - Create different control schemes (casual vs. simulation)
 * - Support multiple input devices
 * - Allow player rebinding without code changes
 *
 * @section Architecture
 *
 * Enhanced Input System Overview:
 * ```
 * UInputAction (abstract action like "Accelerate")
 *     |
 *     +-- Bound to keys via UInputMappingContext
 *     |
 *     +-- Triggers callbacks in UMGVehicleInputHandler
 * ```
 *
 * Config Classes:
 * - **UMGVehicleInputConfig**: Driving controls (throttle, brake, steering, etc.)
 * - **UMGMenuInputConfig**: UI navigation (confirm, back, tab switching)
 * - **FMGInputActionBinding**: Single action with its default bindings
 * - **UMGInputUtility**: Helper functions for icons and input processing
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **Data Asset**:
 * A UDataAsset is an object stored as a .uasset file. Unlike Blueprints, it's
 * just data - no logic. Create one via: Content Browser > Right-Click >
 * Miscellaneous > Data Asset > Select Class.
 *
 * **Input Action (UInputAction)**:
 * Represents an abstract action like "Accelerate" or "Brake". Actions can have:
 * - Value type: Bool (pressed/released), Axis1D (trigger), Axis2D (stick)
 * - Triggers: Conditions for when action fires (pressed, released, held)
 * - Modifiers: Transform input (negate, dead zone, scale)
 *
 * **Input Mapping Context (UInputMappingContext)**:
 * Maps Input Actions to physical controls. Multiple contexts can be active
 * with different priorities. Example mappings:
 * ```
 * ThrottleAction -> Gamepad Right Trigger
 * ThrottleAction -> Keyboard W Key
 * SteeringAction -> Gamepad Left Stick X
 * SteeringAction -> Keyboard A/D Keys
 * ```
 *
 * **Input Priority**:
 * When multiple mapping contexts are active, priority determines which wins.
 * Higher priority = checked first. Vehicle context (priority 1) vs Menu (priority 2).
 *
 * **FKey**:
 * Unreal's type for representing any input key (keyboard key, gamepad button,
 * mouse button, etc.). Examples: EKeys::Gamepad_RightTrigger, EKeys::W
 *
 * **Soft Object Pointer (TSoftObjectPtr)**:
 * A reference that doesn't load the asset until needed. Used for icons to
 * avoid loading all button textures into memory at once.
 *
 * @section Usage Example Usage
 *
 * @code
 * // In the Editor:
 * // 1. Create Data Asset: Right-click > Miscellaneous > Data Asset
 * // 2. Select UMGVehicleInputConfig as the class
 * // 3. Fill in Input Actions and Mapping Context
 * // 4. Assign to your Vehicle Blueprint
 *
 * // In Code - Using the config:
 * UPROPERTY(EditDefaultsOnly, Category = "Input")
 * TObjectPtr<UMGVehicleInputConfig> InputConfig;
 *
 * void AMyVehicle::SetupPlayerInput(UInputComponent* PlayerInputComponent)
 * {
 *     if (APlayerController* PC = Cast<APlayerController>(GetController()))
 *     {
 *         // Add the mapping context
 *         if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
 *             ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
 *         {
 *             Subsystem->AddMappingContext(InputConfig->MappingContext, InputConfig->InputPriority);
 *         }
 *
 *         // Bind actions
 *         if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
 *         {
 *             EIC->BindAction(InputConfig->ThrottleAction, ETriggerEvent::Triggered,
 *                             this, &AMyVehicle::OnThrottle);
 *         }
 *     }
 * }
 *
 * // Getting default bindings for UI:
 * FKey DefaultBrake = UMGVehicleInputConfig::GetDefaultGamepadBinding(TEXT("Brake"));
 * // Returns Gamepad_LeftTrigger
 *
 * // Getting key icons for prompt display:
 * TSoftObjectPtr<UTexture2D> Icon = UMGInputUtility::GetKeyIcon(EKeys::Gamepad_FaceButton_Bottom, true);
 * // Returns Xbox A button or PlayStation X button texture
 *
 * // Checking input device type for UI adaptation:
 * if (UMGInputUtility::IsUsingGamepad(GetWorld()))
 * {
 *     ShowGamepadPrompts();
 * }
 * else
 * {
 *     ShowKeyboardPrompts();
 * }
 *
 * // Applying dead zone to raw input:
 * float RawStick = GetGamepadStickValue();
 * float Processed = UMGInputUtility::ApplyDeadZone(RawStick, 0.15f);
 *
 * // Applying sensitivity curve:
 * float WithCurve = UMGInputUtility::ApplySensitivityCurve(Processed, 1.5f, 2.0f);
 * @endcode
 *
 * @section DefaultBindings Default Control Schemes
 *
 * **Gamepad (Xbox/PlayStation)**:
 * | Action      | Xbox          | PlayStation  |
 * |-------------|---------------|--------------|
 * | Throttle    | RT            | R2           |
 * | Brake       | LT            | L2           |
 * | Steering    | Left Stick    | Left Stick   |
 * | Handbrake   | A / X         | X / Square   |
 * | NOS         | B / Circle    | Circle       |
 * | Shift Up    | RB            | R1           |
 * | Shift Down  | LB            | L1           |
 * | Camera      | Y / Triangle  | Triangle     |
 * | Look Back   | RS Click      | R3           |
 *
 * **Keyboard**:
 * | Action      | Primary | Alternate |
 * |-------------|---------|-----------|
 * | Throttle    | W       | Up Arrow  |
 * | Brake       | S       | Down Arrow|
 * | Steer Left  | A       | Left Arrow|
 * | Steer Right | D       | Right Arrow|
 * | Handbrake   | Space   | -         |
 * | NOS         | Left Shift | -      |
 * | Shift Up    | E       | -         |
 * | Shift Down  | Q       | -         |
 * | Camera      | C       | -         |
 * | Look Back   | R       | -         |
 *
 * @see UMGVehicleInputHandler For input processing and assists
 * @see UInputAction Unreal's Enhanced Input action class
 * @see UInputMappingContext Unreal's input mapping system
 */

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
