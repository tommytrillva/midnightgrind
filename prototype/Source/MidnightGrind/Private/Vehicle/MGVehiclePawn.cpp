// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Vehicle/MGVehicleDamageSystem.h"
#include "VFX/MGVehicleVFXComponent.h"
#include "Audio/MGEngineAudioComponent.h"
#include "Audio/MGVehicleSFXComponent.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AMGVehiclePawn::AMGVehiclePawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMGVehicleMovementComponent>(AWheeledVehiclePawn::VehicleMovementComponentName))
{
	// Get our custom movement component
	MGVehicleMovement = Cast<UMGVehicleMovementComponent>(GetVehicleMovementComponent());

	SetupComponents();

	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void AMGVehiclePawn::SetupComponents()
{
	// Create spring arm for chase camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = ChaseCameraDistance;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, ChaseCameraHeight);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritYaw = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraLagSpeed = CameraLagSpeed;
	SpringArm->CameraRotationLagSpeed = CameraRotationLagSpeed;
	SpringArm->CameraLagMaxDistance = 100.0f;

	// Create main chase camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->FieldOfView = BaseFOV;
	Camera->bUsePawnControlRotation = false;

	// Create hood camera
	HoodCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HoodCamera"));
	HoodCamera->SetupAttachment(GetMesh());
	HoodCamera->SetRelativeLocation(FVector(100.0f, 0.0f, 120.0f));
	HoodCamera->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));
	HoodCamera->FieldOfView = 100.0f;
	HoodCamera->bAutoActivate = false;

	// Create interior camera
	InteriorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InteriorCamera"));
	InteriorCamera->SetupAttachment(GetMesh());
	InteriorCamera->SetRelativeLocation(FVector(30.0f, -30.0f, 110.0f));
	InteriorCamera->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));
	InteriorCamera->FieldOfView = 90.0f;
	InteriorCamera->bAutoActivate = false;

	// Create engine audio component
	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(GetMesh());
	EngineAudio->bAutoActivate = false;

	// Create exhaust VFX
	ExhaustVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExhaustVFX"));
	ExhaustVFX->SetupAttachment(GetMesh());
	ExhaustVFX->SetRelativeLocation(FVector(-200.0f, 0.0f, 30.0f));
	ExhaustVFX->bAutoActivate = false;

	// Create tire smoke VFX
	TireSmokeVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TireSmokeVFX"));
	TireSmokeVFX->SetupAttachment(GetMesh());
	TireSmokeVFX->bAutoActivate = false;

	// Create nitrous VFX
	NitrousVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NitrousVFX"));
	NitrousVFX->SetupAttachment(GetMesh());
	NitrousVFX->SetRelativeLocation(FVector(-200.0f, 0.0f, 30.0f));
	NitrousVFX->bAutoActivate = false;

	// Create vehicle VFX component (handles wear/damage effects)
	VehicleVFX = CreateDefaultSubobject<UMGVehicleVFXComponent>(TEXT("VehicleVFX"));

	// Create engine audio component (handles RPM/load audio)
	VehicleEngineAudio = CreateDefaultSubobject<UMGEngineAudioComponent>(TEXT("VehicleEngineAudio"));

	// Create damage system component (handles collision damage, visual damage)
	VehicleDamageSystem = CreateDefaultSubobject<UMGVehicleDamageSystem>(TEXT("VehicleDamageSystem"));

	// Create vehicle SFX component (handles collision, scrape, tire sounds)
	VehicleSFX = CreateDefaultSubobject<UMGVehicleSFXComponent>(TEXT("VehicleSFX"));
}

void AMGVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	// Store initial transform as checkpoint
	LastCheckpointTransform = GetActorTransform();

	// Bind to movement component events
	BindMovementEvents();

	// Bind damage system events for VFX/SFX feedback
	if (VehicleDamageSystem)
	{
		VehicleDamageSystem->OnDamageTaken.AddDynamic(this, &AMGVehiclePawn::HandleDamageTaken);
		VehicleDamageSystem->OnComponentDamaged.AddDynamic(this, &AMGVehiclePawn::HandleComponentDamaged);
		VehicleDamageSystem->OnComponentBroken.AddDynamic(this, &AMGVehiclePawn::HandleComponentBroken);
		VehicleDamageSystem->OnVisualDamageUpdated.AddDynamic(this, &AMGVehiclePawn::HandleVisualDamageUpdated);
		VehicleDamageSystem->OnScrapeStart.AddDynamic(this, &AMGVehiclePawn::HandleScrapeStart);
		VehicleDamageSystem->OnScrapeEnd.AddDynamic(this, &AMGVehiclePawn::HandleScrapeEnd);
	}

	// Activate engine audio
	if (EngineAudio)
	{
		EngineAudio->Activate();
	}

	// Set initial camera
	SetCameraMode(EMGCameraMode::Chase);
}

void AMGVehiclePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind damage system delegates
	if (VehicleDamageSystem)
	{
		VehicleDamageSystem->OnDamageTaken.RemoveDynamic(this, &AMGVehiclePawn::HandleDamageTaken);
		VehicleDamageSystem->OnComponentDamaged.RemoveDynamic(this, &AMGVehiclePawn::HandleComponentDamaged);
		VehicleDamageSystem->OnComponentBroken.RemoveDynamic(this, &AMGVehiclePawn::HandleComponentBroken);
		VehicleDamageSystem->OnVisualDamageUpdated.RemoveDynamic(this, &AMGVehiclePawn::HandleVisualDamageUpdated);
		VehicleDamageSystem->OnScrapeStart.RemoveDynamic(this, &AMGVehiclePawn::HandleScrapeStart);
		VehicleDamageSystem->OnScrapeEnd.RemoveDynamic(this, &AMGVehiclePawn::HandleScrapeEnd);
	}

	// Unbind movement component delegates
	if (MGVehicleMovement)
	{
		MGVehicleMovement->OnGearChanged.RemoveDynamic(this, &AMGVehiclePawn::OnGearChanged);
		MGVehicleMovement->OnClutchOverheating.RemoveDynamic(this, &AMGVehiclePawn::HandleClutchOverheat);
		MGVehicleMovement->OnClutchBurnout.RemoveDynamic(this, &AMGVehiclePawn::HandleClutchBurnout);
		MGVehicleMovement->OnTireBlowout.RemoveDynamic(this, &AMGVehiclePawn::HandleTireBlowout);
		MGVehicleMovement->OnMoneyShift.RemoveDynamic(this, &AMGVehiclePawn::HandleMoneyShift);
	}

	Super::EndPlay(EndPlayReason);
}

void AMGVehiclePawn::NotifyHit(
	UPrimitiveComponent* MyComp,
	AActor* Other,
	UPrimitiveComponent* OtherComp,
	bool bSelfMoved,
	FVector HitLocation,
	FVector HitNormal,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Route collision to damage system
	if (VehicleDamageSystem)
	{
		// Calculate impact force from impulse magnitude
		float ImpactForce = NormalImpulse.Size();

		// Apply collision damage
		VehicleDamageSystem->ApplyCollisionDamage(Hit, ImpactForce, Other);
	}
}

void AMGVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update all systems
	UpdateRuntimeState(DeltaTime);
	UpdateCamera(DeltaTime);
	UpdateAudio(DeltaTime);
	UpdateVFX(DeltaTime);

	// Update lap timer
	RuntimeState.CurrentLapTime += DeltaTime;
	RuntimeState.TotalRaceTime += DeltaTime;

	// Update HUD with vehicle telemetry (only for locally controlled player)
	if (IsLocallyControlled())
	{
		UpdateHUDTelemetry();
	}
}

void AMGVehiclePawn::UpdateHUDTelemetry()
{
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			// Build telemetry data from runtime state
			FMGVehicleTelemetry Telemetry;
			Telemetry.SpeedMPH = RuntimeState.SpeedMPH;
			Telemetry.SpeedKPH = RuntimeState.SpeedKPH;
			Telemetry.RPM = RuntimeState.RPM;
			Telemetry.MaxRPM = 8000.0f; // Typical redline
			Telemetry.CurrentGear = RuntimeState.CurrentGear;
			Telemetry.NOSAmount = RuntimeState.NitrousPercent / 100.0f; // Convert 0-100 to 0-1
			Telemetry.bNOSActive = RuntimeState.bNitrousActive;
			Telemetry.bIsDrifting = RuntimeState.bIsDrifting;
			Telemetry.DriftAngle = RuntimeState.DriftAngle;

			HUDSubsystem->UpdateVehicleTelemetry(Telemetry);

			// Update drift score data if drifting
			if (RuntimeState.bIsDrifting || RuntimeState.DriftScore > 0.0f)
			{
				FMGDriftScoreData DriftData;
				DriftData.CurrentDriftScore = static_cast<int32>(RuntimeState.DriftScore);
				DriftData.TotalDriftScore = static_cast<int32>(RuntimeState.DriftScore);
				DriftData.bInDriftChain = RuntimeState.bIsDrifting;
				DriftData.DriftMultiplier = FMath::Clamp(FMath::Abs(RuntimeState.DriftAngle) / 45.0f, 1.0f, 3.0f);

				HUDSubsystem->UpdateDriftScore(DriftData);
			}
		}
	}
}

void AMGVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Get enhanced input component
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogTemp, Error, TEXT("MGVehiclePawn requires Enhanced Input Component!"));
		return;
	}

	// Bind throttle
	if (ThrottleAction)
	{
		EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AMGVehiclePawn::HandleThrottle);
		EnhancedInput->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AMGVehiclePawn::HandleThrottleReleased);
	}

	// Bind brake
	if (BrakeAction)
	{
		EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AMGVehiclePawn::HandleBrake);
		EnhancedInput->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AMGVehiclePawn::HandleBrakeReleased);
	}

	// Bind steering
	if (SteeringAction)
	{
		EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AMGVehiclePawn::HandleSteering);
		EnhancedInput->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AMGVehiclePawn::HandleSteering);
	}

	// Bind handbrake
	if (HandbrakeAction)
	{
		EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &AMGVehiclePawn::HandleHandbrake);
		EnhancedInput->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AMGVehiclePawn::HandleHandbrakeReleased);
	}

	// Bind nitrous
	if (NitrousAction)
	{
		EnhancedInput->BindAction(NitrousAction, ETriggerEvent::Triggered, this, &AMGVehiclePawn::HandleNitrous);
		EnhancedInput->BindAction(NitrousAction, ETriggerEvent::Completed, this, &AMGVehiclePawn::HandleNitrousReleased);
	}

	// Bind gear shifts
	if (ShiftUpAction)
	{
		EnhancedInput->BindAction(ShiftUpAction, ETriggerEvent::Started, this, &AMGVehiclePawn::HandleShiftUp);
	}
	if (ShiftDownAction)
	{
		EnhancedInput->BindAction(ShiftDownAction, ETriggerEvent::Started, this, &AMGVehiclePawn::HandleShiftDown);
	}

	// Bind camera controls
	if (CameraCycleAction)
	{
		EnhancedInput->BindAction(CameraCycleAction, ETriggerEvent::Started, this, &AMGVehiclePawn::HandleCameraCycle);
	}
	if (LookBehindAction)
	{
		EnhancedInput->BindAction(LookBehindAction, ETriggerEvent::Triggered, this, &AMGVehiclePawn::HandleLookBehind);
		EnhancedInput->BindAction(LookBehindAction, ETriggerEvent::Completed, this, &AMGVehiclePawn::HandleLookBehindReleased);
	}

	// Bind reset
	if (ResetVehicleAction)
	{
		EnhancedInput->BindAction(ResetVehicleAction, ETriggerEvent::Started, this, &AMGVehiclePawn::HandleResetVehicle);
	}

	// Bind pause
	if (PauseAction)
	{
		EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AMGVehiclePawn::HandlePause);
	}
}

void AMGVehiclePawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Add input mapping context
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (VehicleMappingContext)
			{
				Subsystem->AddMappingContext(VehicleMappingContext, InputPriority);
			}
		}
	}
}

void AMGVehiclePawn::UnPossessed()
{
	// Remove input mapping context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (VehicleMappingContext)
			{
				Subsystem->RemoveMappingContext(VehicleMappingContext);
			}
		}
	}

	Super::UnPossessed();
}

// ==========================================
// COMPONENT ACCESS
// ==========================================

UMGVehicleMovementComponent* AMGVehiclePawn::GetMGVehicleMovement() const
{
	return MGVehicleMovement;
}

void AMGVehiclePawn::LoadVehicleConfiguration(const FMGVehicleData& Configuration)
{
	VehicleConfiguration = Configuration;

	if (MGVehicleMovement)
	{
		MGVehicleMovement->ApplyVehicleConfiguration(Configuration);
	}
}

// ==========================================
// CAMERA
// ==========================================

void AMGVehiclePawn::SetCameraMode(EMGCameraMode NewMode)
{
	CurrentCameraMode = NewMode;

	// Deactivate all cameras first
	if (Camera) Camera->Deactivate();
	if (HoodCamera) HoodCamera->Deactivate();
	if (InteriorCamera) InteriorCamera->Deactivate();

	// Activate the selected camera
	switch (NewMode)
	{
	case EMGCameraMode::Chase:
		if (Camera) Camera->Activate();
		if (SpringArm)
		{
			SpringArm->TargetArmLength = ChaseCameraDistance;
			SpringArm->SocketOffset = FVector(0.0f, 0.0f, ChaseCameraHeight);
		}
		break;

	case EMGCameraMode::Hood:
		if (HoodCamera) HoodCamera->Activate();
		break;

	case EMGCameraMode::Bumper:
		if (Camera) Camera->Activate();
		if (SpringArm)
		{
			SpringArm->TargetArmLength = 0.0f;
			SpringArm->SocketOffset = FVector(200.0f, 0.0f, 80.0f);
		}
		break;

	case EMGCameraMode::Interior:
		if (InteriorCamera) InteriorCamera->Activate();
		break;

	case EMGCameraMode::Cinematic:
		if (Camera) Camera->Activate();
		if (SpringArm)
		{
			SpringArm->TargetArmLength = ChaseCameraDistance * 1.5f;
			SpringArm->SocketOffset = FVector(0.0f, 200.0f, ChaseCameraHeight * 0.5f);
		}
		break;
	}
}

void AMGVehiclePawn::CycleCamera()
{
	int32 CurrentIndex = static_cast<int32>(CurrentCameraMode);
	int32 NextIndex = (CurrentIndex + 1) % 5; // 5 camera modes
	SetCameraMode(static_cast<EMGCameraMode>(NextIndex));
}

void AMGVehiclePawn::SetLookBehind(bool bLookBehind)
{
	bIsLookingBehind = bLookBehind;

	if (SpringArm)
	{
		if (bLookBehind)
		{
			SpringArm->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
		}
		else
		{
			SpringArm->SetRelativeRotation(FRotator::ZeroRotator);
		}
	}
}

// ==========================================
// RACE STATE
// ==========================================

void AMGVehiclePawn::SetCurrentLap(int32 Lap)
{
	int32 PreviousLap = RuntimeState.CurrentLap;
	RuntimeState.CurrentLap = Lap;

	if (Lap > PreviousLap && PreviousLap > 0)
	{
		// Lap completed
		if (RuntimeState.CurrentLapTime < RuntimeState.BestLapTime || RuntimeState.BestLapTime <= 0.0f)
		{
			RuntimeState.BestLapTime = RuntimeState.CurrentLapTime;
		}

		OnLapCompleted.Broadcast(PreviousLap);
		ResetLapTimer();
	}
}

void AMGVehiclePawn::SetRacePosition(int32 Position)
{
	RuntimeState.RacePosition = Position;
}

void AMGVehiclePawn::RecordCheckpoint(int32 CheckpointIndex)
{
	LastCheckpointTransform = GetActorTransform();
	OnCheckpointPassed.Broadcast(CheckpointIndex, RuntimeState.CurrentLapTime);
}

void AMGVehiclePawn::ResetLapTimer()
{
	RuntimeState.CurrentLapTime = 0.0f;
}

void AMGVehiclePawn::RespawnAtCheckpoint()
{
	// Stop all momentum
	if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		PrimComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		PrimComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	// Teleport to checkpoint
	SetActorTransform(LastCheckpointTransform);

	// Broadcast event
	OnVehicleRespawn.Broadcast();
}

void AMGVehiclePawn::ApplyTireDamage(float DamageAmount)
{
	TireHealth = FMath::Clamp(TireHealth - DamageAmount, 0.0f, 100.0f);

	// Apply handling penalty when tires are damaged
	if (MGVehicleMovement && TireHealth < 100.0f)
	{
		// Calculate grip reduction based on tire health
		float GripMultiplier = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, 100.0f),
			FVector2D(0.3f, 1.0f),
			TireHealth
		);

		// Apply to movement component
		MGVehicleMovement->SetTireGripMultiplier(GripMultiplier);

		// Max speed reduction when tires are very damaged
		if (TireHealth < 30.0f)
		{
			float SpeedMultiplier = FMath::GetMappedRangeValueClamped(
				FVector2D(0.0f, 30.0f),
				FVector2D(0.5f, 1.0f),
				TireHealth
			);
			MGVehicleMovement->SetMaxSpeedMultiplier(SpeedMultiplier);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Vehicle tire damage applied: %.1f, Health now: %.1f"), DamageAmount, TireHealth);
}

// ==========================================
// INPUT HANDLERS
// ==========================================

void AMGVehiclePawn::HandleThrottle(const FInputActionValue& Value)
{
	float ThrottleValue = Value.Get<float>();
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetThrottleInput(ThrottleValue);
	}
}

void AMGVehiclePawn::HandleThrottleReleased(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetThrottleInput(0.0f);
	}
}

void AMGVehiclePawn::HandleBrake(const FInputActionValue& Value)
{
	float BrakeValue = Value.Get<float>();
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetBrakeInput(BrakeValue);
	}
}

void AMGVehiclePawn::HandleBrakeReleased(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetBrakeInput(0.0f);
	}
}

void AMGVehiclePawn::HandleSteering(const FInputActionValue& Value)
{
	float SteerValue = Value.Get<float>();
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetSteeringInput(SteerValue);
	}
}

void AMGVehiclePawn::HandleHandbrake(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetHandbrakeInput(true);
	}
}

void AMGVehiclePawn::HandleHandbrakeReleased(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->SetHandbrakeInput(false);
	}
}

void AMGVehiclePawn::HandleNitrous(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->ActivateNitrous();
	}
}

void AMGVehiclePawn::HandleNitrousReleased(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->DeactivateNitrous();
	}
}

void AMGVehiclePawn::HandleShiftUp(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->ShiftUp();
	}
}

void AMGVehiclePawn::HandleShiftDown(const FInputActionValue& Value)
{
	if (MGVehicleMovement)
	{
		MGVehicleMovement->ShiftDown();
	}
}

void AMGVehiclePawn::HandleCameraCycle(const FInputActionValue& Value)
{
	CycleCamera();
}

void AMGVehiclePawn::HandleLookBehind(const FInputActionValue& Value)
{
	SetLookBehind(true);
}

void AMGVehiclePawn::HandleLookBehindReleased(const FInputActionValue& Value)
{
	SetLookBehind(false);
}

void AMGVehiclePawn::HandleResetVehicle(const FInputActionValue& Value)
{
	RespawnAtCheckpoint();
}

void AMGVehiclePawn::HandlePause(const FInputActionValue& Value)
{
	// Pause is typically handled by the game mode or player controller
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetPause(!UGameplayStatics::IsGamePaused(GetWorld()));
	}
}

// ==========================================
// UPDATE METHODS
// ==========================================

void AMGVehiclePawn::UpdateRuntimeState(float DeltaTime)
{
	if (!MGVehicleMovement)
	{
		return;
	}

	// Get engine state
	FMGEngineState EngineState = MGVehicleMovement->GetEngineState();

	// Update speed
	RuntimeState.SpeedMPH = MGVehicleMovement->GetSpeedMPH();
	RuntimeState.SpeedKPH = MGVehicleMovement->GetSpeedKPH();

	// Update engine
	RuntimeState.RPM = EngineState.CurrentRPM;
	RuntimeState.RPMPercent = EngineState.CurrentRPM / 8000.0f; // Normalize to typical redline
	RuntimeState.CurrentGear = MGVehicleMovement->GetCurrentGear();
	RuntimeState.bRevLimiter = EngineState.bRevLimiterActive;

	// Update boost/nitrous
	RuntimeState.BoostPSI = EngineState.CurrentBoostPSI;
	RuntimeState.NitrousPercent = EngineState.NitrousRemaining;
	RuntimeState.bNitrousActive = EngineState.bNitrousActive;

	// Update drift state
	FMGDriftState DriftState = MGVehicleMovement->GetDriftState();
	RuntimeState.bIsDrifting = DriftState.bIsDrifting;
	RuntimeState.DriftAngle = DriftState.DriftAngle;
	RuntimeState.DriftScore = DriftState.DriftScore;

	// Check for gear change
	if (RuntimeState.CurrentGear != PreviousGear)
	{
		OnGearChanged(RuntimeState.CurrentGear);
		PreviousGear = RuntimeState.CurrentGear;
	}

	// Check for drift state change
	if (RuntimeState.bIsDrifting && !bWasDrifting)
	{
		OnDriftStarted();
	}
	else if (!RuntimeState.bIsDrifting && bWasDrifting)
	{
		OnDriftEnded(RuntimeState.DriftScore);
	}
	bWasDrifting = RuntimeState.bIsDrifting;
}

void AMGVehiclePawn::UpdateCamera(float DeltaTime)
{
	if (!Camera || !SpringArm)
	{
		return;
	}

	// Speed-based FOV
	float SpeedPercent = FMath::Clamp(RuntimeState.SpeedMPH / 150.0f, 0.0f, 1.0f); // 150 mph = max
	TargetFOV = FMath::Lerp(BaseFOV, MaxFOV, SpeedPercent * SpeedFOVMultiplier);

	// Nitrous FOV boost
	if (RuntimeState.bNitrousActive)
	{
		TargetFOV += 10.0f;
	}

	// Smooth FOV transition
	float CurrentFOV = Camera->FieldOfView;
	Camera->FieldOfView = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 5.0f);

	// Drift camera shake (subtle)
	if (RuntimeState.bIsDrifting && DriftCameraShakeIntensity > 0.0f)
	{
		float ShakeAmount = FMath::Abs(RuntimeState.DriftAngle) / 90.0f * DriftCameraShakeIntensity;
		FVector ShakeOffset = FVector(
			FMath::RandRange(-ShakeAmount, ShakeAmount),
			FMath::RandRange(-ShakeAmount, ShakeAmount),
			FMath::RandRange(-ShakeAmount * 0.5f, ShakeAmount * 0.5f)
		);
		SpringArm->AddRelativeLocation(ShakeOffset);
	}
}

void AMGVehiclePawn::UpdateAudio(float DeltaTime)
{
	// Get engine state for audio params
	FMGEngineState EngineState = MGVehicleMovement ? MGVehicleMovement->GetEngineState() : FMGEngineState();

	// Update legacy engine audio component
	if (EngineAudio)
	{
		// Update engine sound pitch based on RPM
		float PitchMultiplier = FMath::Lerp(0.5f, 2.0f, RuntimeState.RPMPercent);
		EngineAudio->SetPitchMultiplier(PitchMultiplier);

		// Volume based on throttle
		float VolumeMultiplier = FMath::Lerp(0.3f, 1.0f, EngineState.ThrottlePosition);
		EngineAudio->SetVolumeMultiplier(VolumeMultiplier);
	}

	// Update new engine audio component with all parameters
	if (VehicleEngineAudio)
	{
		VehicleEngineAudio->SetRPM(RuntimeState.RPM);
		VehicleEngineAudio->SetThrottle(EngineState.ThrottlePosition);
		VehicleEngineAudio->SetLoad(EngineState.EngineLoad);
		VehicleEngineAudio->SetGear(RuntimeState.CurrentGear);

		// Set boost level (normalized from PSI, assuming 30 PSI max)
		const float NormalizedBoost = FMath::Clamp(RuntimeState.BoostPSI / 30.0f, 0.0f, 1.0f);
		VehicleEngineAudio->SetBoost(NormalizedBoost);
	}
}

void AMGVehiclePawn::UpdateVFX(float DeltaTime)
{
	// Exhaust flames on throttle lift at high RPM
	if (ExhaustVFX && MGVehicleMovement)
	{
		FMGEngineState EngineState = MGVehicleMovement->GetEngineState();
		bool bShouldFlame = EngineState.ThrottlePosition < 0.2f && RuntimeState.RPMPercent > 0.7f;

		if (bShouldFlame && !ExhaustVFX->IsActive())
		{
			ExhaustVFX->Activate();
		}
		else if (!bShouldFlame && ExhaustVFX->IsActive())
		{
			ExhaustVFX->Deactivate();
		}
	}

	// Tire smoke during drift
	if (TireSmokeVFX)
	{
		if (RuntimeState.bIsDrifting && !TireSmokeVFX->IsActive())
		{
			TireSmokeVFX->Activate();
		}
		else if (!RuntimeState.bIsDrifting && TireSmokeVFX->IsActive())
		{
			TireSmokeVFX->Deactivate();
		}
	}

	// Nitrous VFX
	if (NitrousVFX)
	{
		if (RuntimeState.bNitrousActive && !NitrousVFX->IsActive())
		{
			NitrousVFX->Activate();
			OnNitrousActivated();
		}
		else if (!RuntimeState.bNitrousActive && NitrousVFX->IsActive())
		{
			NitrousVFX->Deactivate();
			OnNitrousDeactivated();
		}
	}

	// Brake glow VFX (from VehicleVFX component)
	if (VehicleVFX && MGVehicleMovement)
	{
		const float BrakeGlow = MGVehicleMovement->GetBrakeGlowIntensity();
		if (BrakeGlow > 0.05f)
		{
			// Apply same glow to all 4 wheels for now
			// Could be per-wheel if we tracked per-wheel brake temps
			for (int32 i = 0; i < 4; ++i)
			{
				VehicleVFX->SetBrakeGlowIntensity(i, BrakeGlow);
			}
		}
	}
}

void AMGVehiclePawn::BindMovementEvents()
{
	if (MGVehicleMovement)
	{
		// Bind to movement component delegates
		MGVehicleMovement->OnGearChanged.AddDynamic(this, &AMGVehiclePawn::OnGearChanged);

		// Bind wear system events to VFX component
		MGVehicleMovement->OnClutchOverheating.AddDynamic(this, &AMGVehiclePawn::HandleClutchOverheat);
		MGVehicleMovement->OnClutchBurnout.AddDynamic(this, &AMGVehiclePawn::HandleClutchBurnout);
		MGVehicleMovement->OnTireBlowout.AddDynamic(this, &AMGVehiclePawn::HandleTireBlowout);
		MGVehicleMovement->OnMoneyShift.AddDynamic(this, &AMGVehiclePawn::HandleMoneyShift);
	}
}

// ==========================================
// WEAR EVENT HANDLERS
// ==========================================

void AMGVehiclePawn::HandleClutchOverheat(float Temperature, float WearLevel)
{
	if (VehicleVFX)
	{
		// Intensity based on how much over the safe temp we are
		// Safe temp ~120C, danger at ~200C
		const float SafeTemp = 120.0f;
		const float DangerTemp = 200.0f;
		const float Intensity = FMath::Clamp((Temperature - SafeTemp) / (DangerTemp - SafeTemp), 0.0f, 1.0f);

		VehicleVFX->TriggerClutchOverheatSmoke(Intensity);
	}
}

void AMGVehiclePawn::HandleClutchBurnout()
{
	if (VehicleVFX)
	{
		// Full intensity smoke for burnout
		VehicleVFX->TriggerClutchOverheatSmoke(1.0f);

		// Also trigger engine damage smoke (clutch failure can cause smoke)
		VehicleVFX->TriggerEngineDamageSmoke(0); // Light oil smoke
	}
}

void AMGVehiclePawn::HandleTireBlowout(int32 WheelIndex, EMGPressureLossCause Cause)
{
	if (VehicleVFX)
	{
		VehicleVFX->TriggerTireBlowout(WheelIndex);
	}
}

void AMGVehiclePawn::HandleMoneyShift(float OverRevAmount)
{
	if (VehicleVFX)
	{
		VehicleVFX->TriggerTransmissionGrind();

		// If severe money shift, also trigger engine damage smoke
		if (OverRevAmount > 1000.0f)
		{
			VehicleVFX->TriggerEngineDamageSmoke(1); // Coolant steam from stress
		}
	}
}

// ==========================================
// DAMAGE EVENT HANDLERS
// ==========================================

void AMGVehiclePawn::HandleDamageTaken(const FMGDamageEvent& DamageEvent)
{
	// Trigger collision impact VFX
	if (VehicleVFX)
	{
		VehicleVFX->TriggerCollisionImpact(
			DamageEvent.ImpactLocation,
			DamageEvent.ImpactNormal,
			DamageEvent.ImpactForce
		);

		// Spawn debris on significant impacts
		if (DamageEvent.DamageDealt > 10.0f)
		{
			int32 DebrisCount = FMath::Clamp(FMath::FloorToInt(DamageEvent.DamageDealt / 10.0f), 3, 15);
			VehicleVFX->SpawnDebris(DamageEvent.ImpactLocation, -DamageEvent.ImpactNormal, DebrisCount);
		}
	}

	// Trigger collision SFX
	if (VehicleSFX)
	{
		VehicleSFX->OnCollision(
			DamageEvent.ImpactForce,
			DamageEvent.ImpactLocation,
			DamageEvent.ImpactNormal
		);
	}

	// Call Blueprint event for additional effects
	OnVehicleCollision(FHitResult(), DamageEvent.ImpactForce);
}

void AMGVehiclePawn::HandleComponentDamaged(EMGDamageComponent Component, float NewHealth)
{
	// Update VFX based on component damage
	if (VehicleVFX)
	{
		// Engine damage triggers smoke
		if (Component == EMGDamageComponent::Engine)
		{
			if (NewHealth < 30.0f)
			{
				// Heavy damage - severe smoke
				VehicleVFX->TriggerEngineDamageSmoke(2);
			}
			else if (NewHealth < 60.0f)
			{
				// Medium damage - coolant/steam
				VehicleVFX->TriggerEngineDamageSmoke(1);
			}
			else if (NewHealth < 80.0f)
			{
				// Light damage - oil leak smoke
				VehicleVFX->TriggerEngineDamageSmoke(0);
			}
		}

		// Cooling system damage causes overheating smoke
		if (Component == EMGDamageComponent::Cooling && NewHealth < 50.0f)
		{
			float Intensity = 1.0f - (NewHealth / 50.0f);
			VehicleVFX->TriggerEngineDamageSmoke(FMath::Clamp(FMath::FloorToInt(Intensity * 3.0f), 0, 2));
		}
	}

	// Update engine audio for damage effects (misfiring, knocking)
	if (VehicleEngineAudio && Component == EMGDamageComponent::Engine)
	{
		// Convert health to damage level (0 = healthy, 1 = destroyed)
		float DamageLevel = 1.0f - (NewHealth / 100.0f);
		VehicleEngineAudio->SetEngineDamageLevel(DamageLevel);
	}

	// Update runtime state for HUD
	if (Component == EMGDamageComponent::Engine)
	{
		RuntimeState.EngineHealth = NewHealth;
	}
	else if (Component == EMGDamageComponent::Body)
	{
		RuntimeState.BodyHealth = NewHealth;
	}
}

void AMGVehiclePawn::HandleComponentBroken(EMGDamageComponent Component)
{
	// Trigger breakdown VFX and SFX
	if (VehicleVFX)
	{
		switch (Component)
		{
		case EMGDamageComponent::Engine:
			// Engine failure - heavy smoke/possible fire
			VehicleVFX->TriggerEngineDamageSmoke(2);
			RuntimeState.bEngineStalled = true;
			// Max engine damage audio
			if (VehicleEngineAudio)
			{
				VehicleEngineAudio->SetEngineDamageLevel(1.0f);
			}
			break;

		case EMGDamageComponent::Transmission:
			VehicleVFX->TriggerTransmissionGrind();
			break;

		case EMGDamageComponent::Cooling:
			// Radiator blown - steam everywhere
			VehicleVFX->TriggerEngineDamageSmoke(2);
			break;

		case EMGDamageComponent::Wheels:
			// Note: Tire blowouts handled separately via wear system
			break;

		default:
			break;
		}
	}

	// Update runtime state
	if (Component == EMGDamageComponent::Engine)
	{
		RuntimeState.EngineHealth = 0.0f;
		RuntimeState.bEngineStalled = true;
	}
	else if (Component == EMGDamageComponent::Body)
	{
		RuntimeState.BodyHealth = 0.0f;
	}
}

void AMGVehiclePawn::HandleVisualDamageUpdated(const FMGVisualDamageState& VisualState)
{
	if (VehicleVFX)
	{
		// Convert visual damage state to VFX damage state
		FMGVehicleDamageVFXState VFXState;

		// Calculate zone damages for VFX
		if (VisualState.ZoneDeformation.Contains(EMGDamageZone::Front))
		{
			VFXState.FrontDamage = VisualState.ZoneDeformation[EMGDamageZone::Front];
		}
		if (VisualState.ZoneDeformation.Contains(EMGDamageZone::Rear))
		{
			VFXState.RearDamage = VisualState.ZoneDeformation[EMGDamageZone::Rear];
		}
		if (VisualState.ZoneDeformation.Contains(EMGDamageZone::Left))
		{
			VFXState.LeftDamage = VisualState.ZoneDeformation[EMGDamageZone::Left];
		}
		if (VisualState.ZoneDeformation.Contains(EMGDamageZone::Right))
		{
			VFXState.RightDamage = VisualState.ZoneDeformation[EMGDamageZone::Right];
		}

		// Calculate overall damage from all zones
		float TotalDeformation = 0.0f;
		int32 ZoneCount = 0;
		for (const auto& Pair : VisualState.ZoneDeformation)
		{
			TotalDeformation += Pair.Value;
			ZoneCount++;
		}
		VFXState.OverallDamage = ZoneCount > 0 ? TotalDeformation / ZoneCount : 0.0f;

		// Engine smoke state
		VFXState.bEngineSmoking = VisualState.bIsSmoking;
		VFXState.bOnFire = VisualState.bIsOnFire;

		// Apply to VFX component
		VehicleVFX->SetDamageState(VFXState);

		// Update light damage state
		VehicleVFX->SetHeadlightsBroken(VisualState.bHeadlightsBroken);
		VehicleVFX->SetTaillightsBroken(VisualState.bTaillightsBroken);

		// Glass break sound if windows damaged significantly
		if (VehicleSFX && VisualState.WindowDamage > 0.5f)
		{
			VehicleSFX->PlayGlassBreak(GetActorLocation());
		}
	}
}

void AMGVehiclePawn::HandleScrapeStart(FVector ContactPoint, float Intensity)
{
	// Start scrape VFX (sparks)
	if (VehicleVFX)
	{
		FVector Direction = GetVelocity().GetSafeNormal();
		VehicleVFX->StartScrapeSparks(ContactPoint, Direction);
	}

	// Start scrape SFX (metal grinding)
	if (VehicleSFX)
	{
		VehicleSFX->StartScrape(Intensity);
	}
}

void AMGVehiclePawn::HandleScrapeEnd()
{
	// Stop scrape VFX
	if (VehicleVFX)
	{
		VehicleVFX->StopScrapeSparks();
	}

	// Stop scrape SFX
	if (VehicleSFX)
	{
		VehicleSFX->StopScrape();
	}
}
