// Copyright Midnight Grind. All Rights Reserved.

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleRespawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompleted, int32, LapNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointPassed, int32, CheckpointIndex, float, LapTime);

/**
 * Camera mode for vehicle
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
 * Runtime vehicle state for HUD and game systems
 */
USTRUCT(BlueprintType)
struct FMGVehicleRuntimeState
{
	GENERATED_BODY()

	// Speed
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float SpeedMPH = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float SpeedKPH = 0.0f;

	// Engine
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float RPM = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float RPMPercent = 0.0f; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentGear = 0;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bRevLimiter = false;

	// Boost/Nitrous
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float BoostPSI = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float NitrousPercent = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bNitrousActive = false;

	// Drift
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDrifting = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float DriftAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float DriftScore = 0.0f;

	// Race
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentLap = 0;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 RacePosition = 0;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float CurrentLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float BestLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float TotalRaceTime = 0.0f;

	// Vehicle Health
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float EngineHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	float BodyHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bEngineStalled = false;
};

/**
 * Main player-controlled vehicle pawn for MIDNIGHT GRIND
 * Integrates Chaos vehicle physics with arcade-tuned handling
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API AMGVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
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

	/** Get the MG vehicle movement component */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	UMGVehicleMovementComponent* GetMGVehicleMovement() const;

	/** Load and apply a vehicle configuration */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Configuration")
	void LoadVehicleConfiguration(const FMGVehicleData& Configuration);

	/** Get current vehicle configuration */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Configuration")
	const FMGVehicleData& GetVehicleConfiguration() const { return VehicleConfiguration; }

	/** Get current runtime state (for HUD) */
	UFUNCTION(BlueprintPure, Category = "Vehicle|State")
	FMGVehicleRuntimeState GetRuntimeState() const { return RuntimeState; }

	// ==========================================
	// CAMERA
	// ==========================================

	/** Switch camera mode */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void SetCameraMode(EMGCameraMode NewMode);

	/** Get current camera mode */
	UFUNCTION(BlueprintPure, Category = "Vehicle|Camera")
	EMGCameraMode GetCameraMode() const { return CurrentCameraMode; }

	/** Cycle to next camera mode */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void CycleCamera();

	/** Look behind */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void SetLookBehind(bool bLookBehind);

	// ==========================================
	// RACE STATE
	// ==========================================

	/** Set current lap */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void SetCurrentLap(int32 Lap);

	/** Set race position */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void SetRacePosition(int32 Position);

	/** Record checkpoint pass */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void RecordCheckpoint(int32 CheckpointIndex);

	/** Reset lap timer */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void ResetLapTimer();

	/** Respawn vehicle at last checkpoint */
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Race")
	void RespawnAtCheckpoint();

	// ==========================================
	// INPUT CONFIGURATION
	// ==========================================

	/** Input mapping context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> VehicleMappingContext;

	/** Input priority */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 InputPriority = 0;

	// Input Actions
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ThrottleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> SteeringAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> HandbrakeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> NitrousAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftUpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftDownAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> CameraCycleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookBehindAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ResetVehicleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseAction;

	// ==========================================
	// CAMERA SETTINGS
	// ==========================================

	/** Chase camera distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float ChaseCameraDistance = 600.0f;

	/** Chase camera height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float ChaseCameraHeight = 200.0f;

	/** Camera lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float CameraLagSpeed = 10.0f;

	/** Camera rotation lag speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float CameraRotationLagSpeed = 10.0f;

	/** Speed-based FOV increase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float SpeedFOVMultiplier = 0.1f;

	/** Base FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float BaseFOV = 90.0f;

	/** Max FOV at high speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Chase")
	float MaxFOV = 110.0f;

	/** Camera shake when drifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float DriftCameraShakeIntensity = 0.5f;

	/** Camera shake on nitrous */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float NitrousCameraShakeIntensity = 1.0f;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when vehicle respawns */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnVehicleRespawn OnVehicleRespawn;

	/** Called when lap is completed */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnLapCompleted OnLapCompleted;

	/** Called when checkpoint is passed */
	UPROPERTY(BlueprintAssignable, Category = "Vehicle|Events")
	FOnCheckpointPassed OnCheckpointPassed;

protected:
	// ==========================================
	// INPUT HANDLERS
	// ==========================================

	/** Handle throttle input */
	void HandleThrottle(const FInputActionValue& Value);
	void HandleThrottleReleased(const FInputActionValue& Value);

	/** Handle brake input */
	void HandleBrake(const FInputActionValue& Value);
	void HandleBrakeReleased(const FInputActionValue& Value);

	/** Handle steering input */
	void HandleSteering(const FInputActionValue& Value);

	/** Handle handbrake */
	void HandleHandbrake(const FInputActionValue& Value);
	void HandleHandbrakeReleased(const FInputActionValue& Value);

	/** Handle nitrous */
	void HandleNitrous(const FInputActionValue& Value);
	void HandleNitrousReleased(const FInputActionValue& Value);

	/** Handle gear shifts */
	void HandleShiftUp(const FInputActionValue& Value);
	void HandleShiftDown(const FInputActionValue& Value);

	/** Handle camera */
	void HandleCameraCycle(const FInputActionValue& Value);
	void HandleLookBehind(const FInputActionValue& Value);
	void HandleLookBehindReleased(const FInputActionValue& Value);

	/** Handle reset */
	void HandleResetVehicle(const FInputActionValue& Value);

	/** Handle pause */
	void HandlePause(const FInputActionValue& Value);

	// ==========================================
	// UPDATE METHODS
	// ==========================================

	/** Update runtime state from movement component */
	virtual void UpdateRuntimeState(float DeltaTime);

	/** Update camera based on speed and state */
	virtual void UpdateCamera(float DeltaTime);

	/** Update audio based on engine state */
	virtual void UpdateAudio(float DeltaTime);

	/** Update VFX based on vehicle state */
	virtual void UpdateVFX(float DeltaTime);

	// ==========================================
	// BLUEPRINT EVENTS
	// ==========================================

	/** Called when gear changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnGearChanged(int32 NewGear);

	/** Called when drift starts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnDriftStarted();

	/** Called when drift ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnDriftEnded(float TotalScore);

	/** Called when nitrous activates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnNitrousActivated();

	/** Called when nitrous deactivates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnNitrousDeactivated();

	/** Called when boost spools up */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnBoostSpooled();

	/** Called on collision */
	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle|Events")
	void OnVehicleCollision(const FHitResult& HitResult, float ImpactForce);

	// ==========================================
	// STATE
	// ==========================================

	/** Current vehicle configuration */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	FMGVehicleData VehicleConfiguration;

	/** Current runtime state */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	FMGVehicleRuntimeState RuntimeState;

	/** Current camera mode */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	EMGCameraMode CurrentCameraMode = EMGCameraMode::Chase;

	/** Is looking behind */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	bool bIsLookingBehind = false;

	/** Last checkpoint transform for respawn */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle|State")
	FTransform LastCheckpointTransform;

	/** Previous gear for change detection */
	int32 PreviousGear = 0;

	/** Was drifting last frame */
	bool bWasDrifting = false;

	/** Target FOV (for smooth transitions) */
	float TargetFOV = 90.0f;

private:
	/** Cached MG movement component */
	UPROPERTY()
	TObjectPtr<UMGVehicleMovementComponent> MGVehicleMovement;

	/** Setup component hierarchy */
	void SetupComponents();

	/** Bind to movement component events */
	void BindMovementEvents();
};
