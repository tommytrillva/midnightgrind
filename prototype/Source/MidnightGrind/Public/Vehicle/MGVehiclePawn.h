// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehiclePawn.h
 * @brief Main player-controlled vehicle pawn for the MIDNIGHT GRIND racing game.
 *
 * This file contains the core vehicle pawn class that integrates Chaos vehicle physics
 * with arcade-tuned handling. It provides camera systems, input handling, race state
 * management, and damage systems.
 *
 * @see UMGVehicleMovementComponent
 * @see FMGVehicleData
 */

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "InputActionValue.h"
#include "MGVehicleData.h"
#include "MGVehiclePawn.generated.h"

class UMGVehicleMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAudioComponent;
class UNiagaraComponent;

/** @brief Broadcast when the vehicle respawns at a checkpoint. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleRespawn);

/** @brief Broadcast when a lap is completed. @param LapNumber The lap that was just completed (1-indexed). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompleted, int32, LapNumber);

/** @brief Broadcast when a checkpoint is passed. @param CheckpointIndex Zero-based checkpoint index. @param LapTime Current lap time when checkpoint was passed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointPassed, int32, CheckpointIndex, float, LapTime);

/**
 * @brief Camera mode enumeration for vehicle view options.
 *
 * Defines the available camera perspectives for the player vehicle.
 * Each mode provides a different viewing experience suited for
 * various gameplay situations.
 */
UENUM(BlueprintType)
enum class EMGCameraMode : uint8
{
	Chase		UMETA(DisplayName = "Chase Camera"),
	Hood		UMETA(DisplayName = "Hood Camera"),
	Bumper		UMETA(DisplayName = "Bumper Camera"),
	Interior	UMETA(DisplayName = "Interior Camera"),
	Cinematic	UMETA(DisplayName = "Cinematic Camera")
};

/**
 * @brief Runtime vehicle state structure for HUD and game systems.
 *
 * Contains all real-time telemetry data for the vehicle, including speed,
 * engine state, boost/nitrous status, drift information, race progress,
 * and vehicle health. This data is updated every frame and used by the
 * HUD subsystem for display.
 *
 * @note All values are read-only and computed by the vehicle movement component.
 */
USTRUCT(BlueprintType)
struct FMGVehicleRuntimeState
{
	GENERATED_BODY()

	// ==========================================
	// SPEED
	// ==========================================

	/** @brief Current speed in miles per hour. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float SpeedMPH = 0.0f;

	/** @brief Current speed in kilometers per hour. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float SpeedKPH = 0.0f;

	// ==========================================
	// ENGINE
	// ==========================================

	/** @brief Current engine RPM. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float RPM = 0.0f;

	/** @brief Engine RPM as percentage of redline (0.0 to 1.0). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float RPMPercent = 0.0f;

	/** @brief Current gear (0 = neutral, -1 = reverse, 1+ = forward gears). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentGear = 0;

	/** @brief True if the rev limiter is currently active. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bRevLimiter = false;

	// ==========================================
	// BOOST/NITROUS
	// ==========================================

	/** @brief Current turbo boost pressure in PSI. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float BoostPSI = 0.0f;

	/** @brief Remaining nitrous as percentage (0.0 to 100.0). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float NitrousPercent = 100.0f;

	/** @brief True if nitrous is currently being used. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bNitrousActive = false;

	// ==========================================
	// DRIFT
	// ==========================================

	/** @brief True if the vehicle is currently in a drift state. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDrifting = false;

	/** @brief Current drift angle in degrees. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float DriftAngle = 0.0f;

	/** @brief Accumulated drift score for current drift session. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float DriftScore = 0.0f;

	// ==========================================
	// RACE
	// ==========================================

	/** @brief Current lap number (1-indexed). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentLap = 0;

	/** @brief Current position in the race (1 = first place). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 RacePosition = 0;

	/** @brief Current lap elapsed time in seconds. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float CurrentLapTime = 0.0f;

	/** @brief Best lap time achieved this race in seconds. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float BestLapTime = 0.0f;

	/** @brief Total elapsed race time in seconds. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TotalRaceTime = 0.0f;

	// ==========================================
	// VEHICLE HEALTH
	// ==========================================

	/** @brief Engine health percentage (0.0 to 100.0). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float EngineHealth = 100.0f;

	/** @brief Body/chassis health percentage (0.0 to 100.0). */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float BodyHealth = 100.0f;

	/** @brief True if the engine has stalled and needs restart. */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bEngineStalled = false;
};

/**
 * @brief Main player-controlled vehicle pawn for MIDNIGHT GRIND racing game.
 *
 * This class serves as the primary vehicle actor that players control during races.
 * It integrates Unreal Engine's Chaos vehicle physics system with arcade-tuned
 * handling characteristics to provide responsive and fun gameplay.
 *
 * Key features include:
 * - Multiple camera modes (chase, hood, bumper, interior, cinematic)
 * - Enhanced Input System integration for controller/keyboard support
 * - Race state tracking (laps, checkpoints, position)
 * - Visual and audio feedback systems
 * - Damage and tire degradation systems
 *
 * @note This class is abstract and should be subclassed in Blueprints to
 *       assign visual assets and configure vehicle-specific parameters.
 *
 * Example usage in Blueprint:
 * @code
 * // Create a new Blueprint child class of AMGVehiclePawn
 * // Assign skeletal mesh, audio cues, and VFX
 * // Configure camera distances and input mappings
 * @endcode
 *
 * @see UMGVehicleMovementComponent For physics and handling configuration
 * @see FMGVehicleData For vehicle specification data
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API AMGVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	/**
	 * @brief Constructs the vehicle pawn with custom movement component.
	 * @param ObjectInitializer Unreal object initializer for component creation.
	 */
	AMGVehiclePawn(const FObjectInitializer& ObjectInitializer);

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	//~ End AActor Interface

	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Spring arm for chase camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	/** Main gameplay camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	/** Hood camera position */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> HoodCamera;

	/** Interior camera position */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> InteriorCamera;

	/** Engine audio component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> EngineAudio;

	/** Exhaust VFX (Niagara) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ExhaustVFX;

	/** Tire smoke VFX */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> TireSmokeVFX;

	/** Nitrous VFX */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NitrousVFX;

	// ==========================================
	// VEHICLE CONFIGURATION
	// ==========================================

	/**
	 * @brief Get the MIDNIGHT GRIND vehicle movement component.
	 *
	 * Provides access to the custom movement component that handles
	 * vehicle physics, drift mechanics, and turbo simulation.
	 *
	 * @return Pointer to the UMGVehicleMovementComponent, or nullptr if not found.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	UMGVehicleMovementComponent* GetMGVehicleMovement() const;

	/**
	 * @brief Load and apply a complete vehicle configuration.
	 *
	 * Applies all vehicle data including engine specs, drivetrain settings,
	 * suspension tuning, and cosmetic configuration to this vehicle instance.
	 *
	 * @param Configuration The vehicle data structure containing all specs.
	 *
	 * @note This should typically be called once during initialization or
	 *       when switching to a different vehicle build.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Configuration")
	void LoadVehicleConfiguration(const FMGVehicleData& Configuration);

	/**
	 * @brief Get the current vehicle configuration data.
	 * @return Const reference to the active vehicle configuration.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Configuration")
	const FMGVehicleData& GetVehicleConfiguration() const { return VehicleConfiguration; }

	/**
	 * @brief Get the current runtime state for HUD display.
	 *
	 * Returns a copy of the current vehicle telemetry including speed,
	 * RPM, gear, boost, drift state, and race progress.
	 *
	 * @return Copy of the current runtime state structure.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGVehicleRuntimeState GetRuntimeState() const { return RuntimeState; }

	// ==========================================
	// CAMERA
	// ==========================================

	/**
	 * @brief Switch to a specific camera mode.
	 *
	 * Transitions the camera to the specified view mode with smooth
	 * interpolation. Different modes are suited for different gameplay situations.
	 *
	 * @param NewMode The camera mode to switch to.
	 *
	 * @see EMGCameraMode for available camera modes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void SetCameraMode(EMGCameraMode NewMode);

	/**
	 * @brief Get the current camera mode.
	 * @return The currently active camera mode.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Camera")
	EMGCameraMode GetCameraMode() const { return CurrentCameraMode; }

	/**
	 * @brief Cycle to the next camera mode in sequence.
	 *
	 * Cycles through camera modes in order: Chase -> Hood -> Bumper ->
	 * Interior -> Cinematic -> Chase. Called when player presses camera button.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void CycleCamera();

	/**
	 * @brief Enable or disable the look-behind view.
	 *
	 * Temporarily rotates the camera 180 degrees to view behind the vehicle.
	 * Used for checking opponent positions or reversing.
	 *
	 * @param bLookBehind True to look behind, false to return to forward view.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void SetLookBehind(bool bLookBehind);

	// ==========================================
	// RACE STATE
	// ==========================================

	/**
	 * @brief Set the current lap number.
	 *
	 * Called by the race manager when the vehicle crosses the start/finish line.
	 *
	 * @param Lap The new lap number (1-indexed).
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void SetCurrentLap(int32 Lap);

	/**
	 * @brief Set the current race position.
	 *
	 * Called by the race manager to update this vehicle's position in the race.
	 *
	 * @param Position The current position (1 = first place).
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void SetRacePosition(int32 Position);

	/**
	 * @brief Record passing through a checkpoint.
	 *
	 * Updates the last checkpoint transform for respawn purposes and
	 * records split times. Broadcasts the OnCheckpointPassed event.
	 *
	 * @param CheckpointIndex Zero-based index of the checkpoint passed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void RecordCheckpoint(int32 CheckpointIndex);

	/**
	 * @brief Reset the lap timer to zero.
	 *
	 * Called at the start of each new lap to begin timing fresh.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void ResetLapTimer();

	/**
	 * @brief Respawn the vehicle at the last passed checkpoint.
	 *
	 * Teleports the vehicle to the last recorded checkpoint transform,
	 * resets velocity, and broadcasts the OnVehicleRespawn event.
	 * Used when the vehicle is stuck or flipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void RespawnAtCheckpoint();

	// ==========================================
	// DAMAGE
	// ==========================================

	/**
	 * @brief Apply damage to the vehicle's tires.
	 *
	 * Reduces tire health which affects grip and maximum speed.
	 * Used by hazards like spike strips, curb impacts, and collisions.
	 *
	 * @param DamageAmount Amount of damage to apply (subtracted from health).
	 *
	 * @note Tire health is clamped to 0-100 range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Damage")
	void ApplyTireDamage(float DamageAmount);

	/**
	 * @brief Get the current tire health percentage.
	 * @return Tire health value (0.0 = destroyed, 100.0 = perfect condition).
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	float GetTireHealth() const { return TireHealth; }

	/**
	 * @brief Check if the tires are completely flat/destroyed.
	 * @return True if tire health is zero or below, false otherwise.
	 */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Damage")
	bool AreTiresFlat() const { return TireHealth <= 0.0f; }

	// ==========================================
	// INPUT CONFIGURATION
	// ==========================================

	/** @brief Enhanced Input mapping context for vehicle controls. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> VehicleMappingContext;

	/** @brief Priority level for input mapping context (higher = takes precedence). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 InputPriority = 0;

	// ==========================================
	// INPUT ACTIONS
	// ==========================================

	/** @brief Throttle/accelerate input action (analog 0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ThrottleAction;

	/** @brief Brake input action (analog 0-1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> BrakeAction;

	/** @brief Steering input action (analog -1 to 1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> SteeringAction;

	/** @brief Handbrake/e-brake input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> HandbrakeAction;

	/** @brief Nitrous activation input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> NitrousAction;

	/** @brief Manual upshift input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftUpAction;

	/** @brief Manual downshift input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftDownAction;

	/** @brief Camera view cycle input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> CameraCycleAction;

	/** @brief Look behind (rear view) input action (digital hold). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookBehindAction;

	/** @brief Vehicle reset/respawn input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ResetVehicleAction;

	/** @brief Pause menu input action (digital). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseAction;

	// ==========================================
	// CAMERA SETTINGS
	// ==========================================

	/** @brief Distance from vehicle to chase camera in Unreal units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float ChaseCameraDistance = 600.0f;

	/** @brief Vertical offset for chase camera above vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float ChaseCameraHeight = 200.0f;

	/** @brief Speed at which camera position follows vehicle movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float CameraLagSpeed = 10.0f;

	/** @brief Speed at which camera rotation follows vehicle rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float CameraRotationLagSpeed = 10.0f;

	/**
	 * @brief Multiplier for speed-based FOV increase.
	 *
	 * Higher values create more dramatic speed sensation at high velocities.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float SpeedFOVMultiplier = 0.1f;

	/** @brief Base field of view angle in degrees when stationary. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float BaseFOV = 90.0f;

	/** @brief Maximum field of view angle at top speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float MaxFOV = 110.0f;

	/** @brief Camera shake intensity multiplier when drifting (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float DriftCameraShakeIntensity = 0.5f;

	/** @brief Camera shake intensity multiplier when nitrous is active (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float NitrousCameraShakeIntensity = 1.0f;

	// ==========================================
	// EVENTS
	// ==========================================

	/**
	 * @brief Event broadcast when the vehicle respawns at a checkpoint.
	 *
	 * Subscribe to this event in Blueprint to trigger respawn effects,
	 * reset UI elements, or notify game systems.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnVehicleRespawn OnVehicleRespawn;

	/**
	 * @brief Event broadcast when a lap is completed.
	 *
	 * Subscribe to this event to update lap counters, trigger celebrations,
	 * or determine race completion.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnLapCompleted OnLapCompleted;

	/**
	 * @brief Event broadcast when a checkpoint is passed.
	 *
	 * Provides checkpoint index and current lap time for split time displays
	 * and race progress tracking.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnCheckpointPassed OnCheckpointPassed;

protected:
	// ==========================================
	// INPUT HANDLERS
	// ==========================================

	/**
	 * @brief Handle throttle input pressed/held.
	 * @param Value Input action value containing analog throttle amount.
	 */
	void HandleThrottle(const FInputActionValue& Value);

	/**
	 * @brief Handle throttle input released.
	 * @param Value Input action value (typically zero).
	 */
	void HandleThrottleReleased(const FInputActionValue& Value);

	/**
	 * @brief Handle brake input pressed/held.
	 * @param Value Input action value containing analog brake amount.
	 */
	void HandleBrake(const FInputActionValue& Value);

	/**
	 * @brief Handle brake input released.
	 * @param Value Input action value (typically zero).
	 */
	void HandleBrakeReleased(const FInputActionValue& Value);

	/**
	 * @brief Handle steering input.
	 * @param Value Input action value containing steering axis (-1 to 1).
	 */
	void HandleSteering(const FInputActionValue& Value);

	/**
	 * @brief Handle handbrake input engaged.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleHandbrake(const FInputActionValue& Value);

	/**
	 * @brief Handle handbrake input released.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleHandbrakeReleased(const FInputActionValue& Value);

	/**
	 * @brief Handle nitrous activation.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleNitrous(const FInputActionValue& Value);

	/**
	 * @brief Handle nitrous deactivation.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleNitrousReleased(const FInputActionValue& Value);

	/**
	 * @brief Handle manual upshift request.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleShiftUp(const FInputActionValue& Value);

	/**
	 * @brief Handle manual downshift request.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleShiftDown(const FInputActionValue& Value);

	/**
	 * @brief Handle camera mode cycle request.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleCameraCycle(const FInputActionValue& Value);

	/**
	 * @brief Handle look-behind activation.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleLookBehind(const FInputActionValue& Value);

	/**
	 * @brief Handle look-behind deactivation.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleLookBehindReleased(const FInputActionValue& Value);

	/**
	 * @brief Handle vehicle reset/respawn request.
	 * @param Value Input action value (digital trigger).
	 */
	void HandleResetVehicle(const FInputActionValue& Value);

	/**
	 * @brief Handle pause menu request.
	 * @param Value Input action value (digital trigger).
	 */
	void HandlePause(const FInputActionValue& Value);

	// ==========================================
	// UPDATE METHODS
	// ==========================================

	/**
	 * @brief Update runtime state from the movement component.
	 *
	 * Pulls current speed, RPM, gear, boost, and drift data from
	 * the movement component and populates the RuntimeState structure.
	 *
	 * @param DeltaTime Frame delta time in seconds.
	 */
	virtual void UpdateRuntimeState(float DeltaTime);

	/**
	 * @brief Update camera position, rotation, and FOV.
	 *
	 * Handles speed-based FOV scaling, drift camera effects,
	 * and smooth interpolation between camera modes.
	 *
	 * @param DeltaTime Frame delta time in seconds.
	 */
	virtual void UpdateCamera(float DeltaTime);

	/**
	 * @brief Update engine audio based on current state.
	 *
	 * Modulates engine audio pitch and volume based on RPM,
	 * throttle position, and special states (nitrous, rev limiter).
	 *
	 * @param DeltaTime Frame delta time in seconds.
	 */
	virtual void UpdateAudio(float DeltaTime);

	/**
	 * @brief Update visual effects based on vehicle state.
	 *
	 * Controls exhaust flames, tire smoke, nitrous flame effects,
	 * and other Niagara particle systems.
	 *
	 * @param DeltaTime Frame delta time in seconds.
	 */
	virtual void UpdateVFX(float DeltaTime);

	/**
	 * @brief Send current vehicle telemetry to the HUD subsystem.
	 *
	 * Pushes the RuntimeState data to the HUD for display of
	 * speed, RPM, gear, boost, and race information.
	 */
	void UpdateHUDTelemetry();

	// ==========================================
	// BLUEPRINT EVENTS
	// ==========================================

	/**
	 * @brief Blueprint event called when the current gear changes.
	 *
	 * Implement in Blueprint to play gear shift sounds or animations.
	 *
	 * @param NewGear The new gear number (-1 = reverse, 0 = neutral, 1+ = forward).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnGearChanged(int32 NewGear);

	/**
	 * @brief Blueprint event called when a drift begins.
	 *
	 * Implement in Blueprint to start drift effects, sounds, or UI elements.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnDriftStarted();

	/**
	 * @brief Blueprint event called when a drift ends.
	 *
	 * Implement in Blueprint to finalize drift scoring display
	 * and play completion effects.
	 *
	 * @param TotalScore The final score earned from this drift.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnDriftEnded(float TotalScore);

	/**
	 * @brief Blueprint event called when nitrous is activated.
	 *
	 * Implement in Blueprint to start nitrous flame effects and audio.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnNitrousActivated();

	/**
	 * @brief Blueprint event called when nitrous is deactivated.
	 *
	 * Implement in Blueprint to stop nitrous effects gracefully.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnNitrousDeactivated();

	/**
	 * @brief Blueprint event called when turbo reaches full boost.
	 *
	 * Implement in Blueprint to play spool-up completion sound
	 * or display boost indicator.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnBoostSpooled();

	/**
	 * @brief Blueprint event called when the vehicle collides with something.
	 *
	 * Implement in Blueprint to play impact sounds, spawn debris,
	 * or apply damage based on impact force.
	 *
	 * @param HitResult Details about the collision including location and normal.
	 * @param ImpactForce Magnitude of the collision force.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnVehicleCollision(const FHitResult& HitResult, float ImpactForce);

	// ==========================================
	// STATE
	// ==========================================

	/** @brief Complete vehicle configuration data including engine, drivetrain, and tuning. */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	FMGVehicleData VehicleConfiguration;

	/** @brief Current frame's runtime telemetry state for HUD and game systems. */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	FMGVehicleRuntimeState RuntimeState;

	/** @brief Currently active camera viewing mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	EMGCameraMode CurrentCameraMode = EMGCameraMode::Chase;

	/** @brief True if the look-behind view is currently active. */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	bool bIsLookingBehind = false;

	/** @brief Transform of the last checkpoint passed, used for respawn positioning. */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	FTransform LastCheckpointTransform;

	/** @brief Previous frame's gear for detecting gear changes. */
	int32 PreviousGear = 0;

	/** @brief Whether the vehicle was drifting in the previous frame. */
	bool bWasDrifting = false;

	/** @brief Target FOV for smooth interpolation during speed changes. */
	float TargetFOV = 90.0f;

	/** @brief Current tire condition percentage (0.0 = flat, 100.0 = new). */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	float TireHealth = 100.0f;

private:
	/** @brief Cached pointer to the custom movement component for fast access. */
	UPROPERTY()
	TObjectPtr<UMGVehicleMovementComponent> MGVehicleMovement;

	/**
	 * @brief Initialize and configure all vehicle subcomponents.
	 *
	 * Creates spring arm, cameras, audio components, and VFX systems.
	 * Called during construction.
	 */
	void SetupComponents();

	/**
	 * @brief Subscribe to movement component delegate events.
	 *
	 * Binds to gear change, drift, nitrous, and boost events
	 * from the movement component.
	 */
	void BindMovementEvents();
};
