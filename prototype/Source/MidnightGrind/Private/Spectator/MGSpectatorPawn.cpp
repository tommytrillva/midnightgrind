// Copyright Midnight Grind. All Rights Reserved.

#include "Spectator/MGSpectatorPawn.h"
#include "Spectator/MGSpectatorSubsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"

// ==========================================
// AMGSpectatorPawn
// ==========================================

AMGSpectatorPawn::AMGSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetFieldOfView(90.0f);
	CameraComponent->bUsePawnControlRotation = true;

	// Set defaults
	bAddDefaultMovementBindings = false;
}

void AMGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	// Get spectator subsystem
	UWorld* World = GetWorld();
	if (World)
	{
		SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();
	}

	// Store default FOV
	DefaultFOV = CameraComponent->FieldOfView;
	CurrentFOV = DefaultFOV;
	TargetFOV = DefaultFOV;
}

void AMGSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth FOV changes
	if (!FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f))
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 10.0f);
		CameraComponent->SetFieldOfView(CurrentFOV);
	}
}

void AMGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement
	PlayerInputComponent->BindAxis("MoveForward", this, &AMGSpectatorPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMGSpectatorPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AMGSpectatorPawn::MoveUp);

	// Look
	PlayerInputComponent->BindAxis("Turn", this, &AMGSpectatorPawn::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMGSpectatorPawn::LookUp);

	// Speed modifiers
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMGSpectatorPawn::StartFastMode);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMGSpectatorPawn::StopFastMode);
	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &AMGSpectatorPawn::StartSlowMode);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &AMGSpectatorPawn::StopSlowMode);

	// Camera controls
	PlayerInputComponent->BindAction("CycleCameraMode", IE_Pressed, this, &AMGSpectatorPawn::CycleCameraMode);
	PlayerInputComponent->BindAction("NextTarget", IE_Pressed, this, &AMGSpectatorPawn::CycleTargetNext);
	PlayerInputComponent->BindAction("PreviousTarget", IE_Pressed, this, &AMGSpectatorPawn::CycleTargetPrevious);

	// Zoom
	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &AMGSpectatorPawn::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &AMGSpectatorPawn::ZoomOut);

	// Other controls
	PlayerInputComponent->BindAction("ToggleAutoDirector", IE_Pressed, this, &AMGSpectatorPawn::ToggleAutoDirector);
	PlayerInputComponent->BindAction("ExitSpectator", IE_Pressed, this, &AMGSpectatorPawn::ExitSpectator);
}

// ==========================================
// CAMERA
// ==========================================

void AMGSpectatorPawn::SetFieldOfView(float FOV)
{
	TargetFOV = FMath::Clamp(FOV, MinFOV, MaxFOV);
}

float AMGSpectatorPawn::GetFieldOfView() const
{
	return CurrentFOV;
}

void AMGSpectatorPawn::ResetFieldOfView()
{
	TargetFOV = DefaultFOV;
}

// ==========================================
// INPUT
// ==========================================

void AMGSpectatorPawn::MoveForward(float Value)
{
	if (Value == 0.0f || !SpectatorSubsystem)
	{
		return;
	}

	// Only move in free cam mode
	if (SpectatorSubsystem->GetCameraMode() != EMGSpectatorCameraMode::FreeCam)
	{
		return;
	}

	float Speed = MoveSpeed;
	if (bFastMode)
	{
		Speed = FastMoveSpeed;
	}
	else if (bSlowMode)
	{
		Speed = SlowMoveSpeed;
	}

	FVector Direction = GetActorForwardVector();
	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	AddMovementInput(Direction, Value * Speed * DeltaSeconds);
}

void AMGSpectatorPawn::MoveRight(float Value)
{
	if (Value == 0.0f || !SpectatorSubsystem)
	{
		return;
	}

	if (SpectatorSubsystem->GetCameraMode() != EMGSpectatorCameraMode::FreeCam)
	{
		return;
	}

	float Speed = MoveSpeed;
	if (bFastMode)
	{
		Speed = FastMoveSpeed;
	}
	else if (bSlowMode)
	{
		Speed = SlowMoveSpeed;
	}

	FVector Direction = GetActorRightVector();
	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	AddMovementInput(Direction, Value * Speed * DeltaSeconds);
}

void AMGSpectatorPawn::MoveUp(float Value)
{
	if (Value == 0.0f || !SpectatorSubsystem)
	{
		return;
	}

	if (SpectatorSubsystem->GetCameraMode() != EMGSpectatorCameraMode::FreeCam)
	{
		return;
	}

	float Speed = MoveSpeed;
	if (bFastMode)
	{
		Speed = FastMoveSpeed;
	}
	else if (bSlowMode)
	{
		Speed = SlowMoveSpeed;
	}

	FVector Direction = FVector::UpVector;
	UWorld* World = GetWorld();
	float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
	AddMovementInput(Direction, Value * Speed * DeltaSeconds);
}

void AMGSpectatorPawn::LookUp(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->AddPitchInput(Value * LookSensitivity);
	}
}

void AMGSpectatorPawn::Turn(float Value)
{
	if (Value == 0.0f)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->AddYawInput(Value * LookSensitivity);
	}
}

void AMGSpectatorPawn::StartFastMode()
{
	bFastMode = true;
}

void AMGSpectatorPawn::StopFastMode()
{
	bFastMode = false;
}

void AMGSpectatorPawn::StartSlowMode()
{
	bSlowMode = true;
}

void AMGSpectatorPawn::StopSlowMode()
{
	bSlowMode = false;
}

void AMGSpectatorPawn::CycleCameraMode()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->CycleNextCameraMode();
	}
}

void AMGSpectatorPawn::CycleTargetNext()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->CycleNextTarget();
	}
}

void AMGSpectatorPawn::CycleTargetPrevious()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->CyclePreviousTarget();
	}
}

void AMGSpectatorPawn::ZoomIn()
{
	TargetFOV = FMath::Max(TargetFOV - FOVZoomSpeed, MinFOV);
}

void AMGSpectatorPawn::ZoomOut()
{
	TargetFOV = FMath::Min(TargetFOV + FOVZoomSpeed, MaxFOV);
}

void AMGSpectatorPawn::ToggleAutoDirector()
{
	if (SpectatorSubsystem)
	{
		SpectatorSubsystem->EnableAutoDirector(!SpectatorSubsystem->IsAutoDirectorEnabled());
	}
}

void AMGSpectatorPawn::ExitSpectator()
{
	if (SpectatorSubsystem)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			SpectatorSubsystem->ExitSpectatorMode(PC);
		}
	}
}

// ==========================================
// AMGSpectatorCameraActor
// ==========================================

AMGSpectatorCameraActor::AMGSpectatorCameraActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create camera component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	RootComponent = CameraComponent;
}

void AMGSpectatorCameraActor::BeginPlay()
{
	Super::BeginPlay();

	// Auto-register with spectator subsystem
	if (bAutoRegister)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UMGSpectatorSubsystem* SpectatorSubsystem = World->GetSubsystem<UMGSpectatorSubsystem>();
			if (SpectatorSubsystem)
			{
				SpectatorSubsystem->RegisterBroadcastCamera(GetCameraSettings());
			}
		}
	}
}

FMGBroadcastCameraPoint AMGSpectatorCameraActor::GetCameraSettings() const
{
	FMGBroadcastCameraPoint Settings;
	Settings.Location = GetActorLocation();
	Settings.Rotation = GetActorRotation();
	Settings.FieldOfView = CameraComponent->FieldOfView;
	Settings.Priority = Priority;
	Settings.TrackRange = TrackRange;
	Settings.bIsZoomCamera = bIsZoomCamera;
	Settings.bAutoTrack = bAutoTrack;

	return Settings;
}
