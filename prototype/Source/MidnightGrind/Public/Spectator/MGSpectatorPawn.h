// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSpectatorPawn.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines the spectator camera system used when watching races or
 * replays. Think of it like a TV broadcast camera that can fly around freely
 * or follow vehicles during a race.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. PAWN vs ACTOR:
 *    - An Actor is any object that can exist in the game world
 *    - A Pawn is a special Actor that can be "possessed" (controlled) by a player
 *    - When you enter spectator mode, you stop controlling your car and instead
 *      control this "ghost camera" that can fly around
 *
 * 2. SPECTATOR MODE:
 *    - Used when watching other players race, during replays, or after your
 *      car is eliminated
 *    - Unlike driving, spectating lets you fly the camera anywhere to get
 *      the best viewing angle
 *
 * 3. CAMERA COMPONENT:
 *    - The "eye" through which the player sees the world
 *    - Has properties like Field of View (FOV) which controls zoom level
 *    - Lower FOV = zoomed in (telephoto lens), Higher FOV = zoomed out (wide angle)
 *
 * 4. INPUT BINDING:
 *    - SetupPlayerInputComponent() connects keyboard/gamepad buttons to actions
 *    - For example: pressing W might call MoveForward(), pressing Shift activates
 *      fast mode
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Player Controller]
 *           |
 *           v (possesses)
 *    [MGSpectatorPawn] <---> [MGSpectatorSubsystem]
 *           |                       |
 *           v                       v
 *    [Camera Component]      [Target Tracking]
 *                            [Camera Modes]
 *                            [Auto-Director]
 *
 * When spectator mode is activated:
 * 1. The SpectatorSubsystem spawns this pawn
 * 2. The player controller "possesses" (takes control of) this pawn
 * 3. Player input now moves the spectator camera instead of a vehicle
 * 4. The subsystem provides additional features like target tracking
 *
 * USAGE EXAMPLE (in Blueprints):
 *    // Get reference to spectator pawn
 *    SpectatorPawn->SetFieldOfView(60.0f);  // Zoom in
 *    SpectatorPawn->bFastMode = true;        // Move faster
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "MGSpectatorPawn.generated.h"

// Forward declarations - these tell the compiler these classes exist without
// including their full header files. This speeds up compilation.
class UCameraComponent;        // The actual camera that renders what the player sees
class USpringArmComponent;     // A "boom arm" that can be used to position cameras (not currently used here)
class UInputComponent;         // Handles mapping player inputs to functions
class UMGSpectatorSubsystem;   // The manager that coordinates all spectator features

/**
 * AMGSpectatorPawn - The controllable camera pawn for spectator mode.
 *
 * This pawn gives players a free-flying camera to watch the action. It supports:
 * - WASD/stick movement in any direction
 * - Mouse/right stick to look around
 * - Speed modifiers (shift for fast, ctrl for slow)
 * - Zoom in/out functionality
 * - Integration with the spectator subsystem for target tracking
 *
 * The "A" prefix means this is an Actor-derived class (Unreal naming convention).
 */
UCLASS()
class MIDNIGHTGRIND_API AMGSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	AMGSpectatorPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ==========================================
	// CAMERA
	// ==========================================

	/** Get camera component */
	UFUNCTION(BlueprintPure, Category = "Camera")
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

	/** Set field of view */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetFieldOfView(float FOV);

	/** Get field of view */
	UFUNCTION(BlueprintPure, Category = "Camera")
	float GetFieldOfView() const;

	/** Reset camera to default FOV */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ResetFieldOfView();

	// ==========================================
	// MOVEMENT
	// ==========================================

	/** Movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 1000.0f;

	/** Fast movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FastMoveSpeed = 3000.0f;

	/** Slow movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SlowMoveSpeed = 200.0f;

	/** Look sensitivity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float LookSensitivity = 1.0f;

	/** Is fast mode active */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bFastMode = false;

	/** Is slow mode active */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bSlowMode = false;

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	// ==========================================
	// SUBSYSTEM REFERENCE
	// ==========================================

	UPROPERTY()
	UMGSpectatorSubsystem* SpectatorSubsystem;

	// ==========================================
	// INPUT
	// ==========================================

	/** Move forward/backward */
	void MoveForward(float Value);

	/** Move right/left */
	void MoveRight(float Value);

	/** Move up/down */
	void MoveUp(float Value);

	/** Look up/down */
	void LookUp(float Value);

	/** Look right/left */
	void Turn(float Value);

	/** Start fast mode */
	void StartFastMode();

	/** Stop fast mode */
	void StopFastMode();

	/** Start slow mode */
	void StartSlowMode();

	/** Stop slow mode */
	void StopSlowMode();

	/** Cycle camera mode */
	void CycleCameraMode();

	/** Cycle target next */
	void CycleTargetNext();

	/** Cycle target previous */
	void CycleTargetPrevious();

	/** Zoom in */
	void ZoomIn();

	/** Zoom out */
	void ZoomOut();

	/** Toggle auto-director */
	void ToggleAutoDirector();

	/** Exit spectator mode */
	void ExitSpectator();

	// ==========================================
	// STATE
	// ==========================================

	/** Default FOV */
	float DefaultFOV = 90.0f;

	/** Current FOV */
	float CurrentFOV = 90.0f;

	/** Min FOV (zoomed in) */
	float MinFOV = 30.0f;

	/** Max FOV (zoomed out) */
	float MaxFOV = 120.0f;

	/** FOV zoom speed */
	float FOVZoomSpeed = 30.0f;

	/** Target FOV for smooth zoom */
	float TargetFOV = 90.0f;
};

/**
 * AMGSpectatorCameraActor - A pre-placed camera in the level for broadcast-style shots.
 *
 * WHAT THIS IS:
 * Unlike the SpectatorPawn which players control freely, this is a FIXED camera
 * that level designers place around the track. Think of TV cameras at a real race:
 * - Cameras on the pit wall
 * - Cameras at corners
 * - Helicopter cameras
 *
 * WHY IT EXISTS:
 * The auto-director system can switch between these cameras to create a
 * professional TV broadcast feel. Each camera has settings that define:
 * - Where along the track it should be active (TrackRange)
 * - How important it is compared to other cameras (Priority)
 * - Whether it should zoom to follow vehicles (bIsZoomCamera)
 *
 * HOW TO USE (for Level Designers):
 * 1. Drag this actor into your level from the Content Browser
 * 2. Position it where you want a camera shot
 * 3. Adjust the rotation to frame the shot nicely
 * 4. Set TrackRange to define which part of the track this camera covers
 *    (e.g., TrackRange(500, 800) means active from 500m to 800m along the track)
 * 5. If bAutoRegister is true, it will automatically be available for the
 *    broadcast/director camera modes
 *
 * ARCHITECTURE NOTE:
 * These cameras are registered with the SpectatorSubsystem when the level loads.
 * The subsystem then selects the best camera based on where the target vehicle
 * is on the track.
 */
UCLASS()
class MIDNIGHTGRIND_API AMGSpectatorCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AMGSpectatorCameraActor();

	virtual void BeginPlay() override;

	/** Get camera settings */
	UFUNCTION(BlueprintPure, Category = "Camera")
	FMGBroadcastCameraPoint GetCameraSettings() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	/** Camera priority */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 Priority = 1;

	/** Track range for this camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FVector2D TrackRange = FVector2D(0.0f, 100.0f);

	/** Is zoom camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsZoomCamera = false;

	/** Auto-track targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoTrack = true;

	/** Auto-register on begin play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoRegister = true;
};
