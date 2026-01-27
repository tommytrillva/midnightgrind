// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputRemap/MGInputRemapSubsystem.h"
#include "Checkpoint/MGCheckpointSubsystem.h"
#include "NearMiss/MGNearMissSubsystem.h"
#include "Drift/MGDriftSubsystem.h"
#include "Airtime/MGAirtimeSubsystem.h"
#include "Fuel/MGFuelSubsystem.h"
#include "Tire/MGTireSubsystem.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

AMGPlayerController::AMGPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AMGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get input remap subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		InputRemapSubsystem = GameInstance->GetSubsystem<UMGInputRemapSubsystem>();
	}

	// Set up default input mapping
	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		// Bind to checkpoint subsystem's wrong-way detection
		if (UWorld* World = GetWorld())
		{
			if (UMGCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UMGCheckpointSubsystem>())
			{
				CheckpointSubsystem->OnWrongWay.AddDynamic(this, &AMGPlayerController::OnWrongWayDetected);
			}

			// Bind to near miss subsystem for HUD popups
			if (UMGNearMissSubsystem* NearMissSubsystem = World->GetSubsystem<UMGNearMissSubsystem>())
			{
				NearMissSubsystem->OnNearMissOccurred.AddDynamic(this, &AMGPlayerController::OnNearMissDetected);
			}
		}

		// Bind to drift subsystem for score popups (GameInstance subsystem)
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMGDriftSubsystem* DriftSubsystem = GI->GetSubsystem<UMGDriftSubsystem>())
			{
				DriftSubsystem->OnDriftEnded.AddDynamic(this, &AMGPlayerController::OnDriftEnded);
			}

			// Bind to airtime subsystem for jump/trick popups
			if (UMGAirtimeSubsystem* AirtimeSubsystem = GI->GetSubsystem<UMGAirtimeSubsystem>())
			{
				AirtimeSubsystem->OnJumpEnded.AddDynamic(this, &AMGPlayerController::OnJumpEnded);
				AirtimeSubsystem->OnTrickCompleted.AddDynamic(this, &AMGPlayerController::OnTrickCompleted);
			}

			// Bind to fuel subsystem for low fuel warnings
			if (UMGFuelSubsystem* FuelSubsystem = GI->GetSubsystem<UMGFuelSubsystem>())
			{
				FuelSubsystem->OnFuelAlert.AddDynamic(this, &AMGPlayerController::OnFuelAlert);
				FuelSubsystem->OnFuelEmpty.AddDynamic(this, &AMGPlayerController::OnFuelEmpty);
			}

			// Bind to tire subsystem for puncture warnings
			if (UMGTireSubsystem* TireSubsystem = GI->GetSubsystem<UMGTireSubsystem>())
			{
				TireSubsystem->OnTirePunctured.AddDynamic(this, &AMGPlayerController::OnTirePunctured);
				TireSubsystem->OnTireConditionChanged.AddDynamic(this, &AMGPlayerController::OnTireConditionChanged);
			}
		}
	}
}

void AMGPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from subsystem delegates
	if (UWorld* World = GetWorld())
	{
		if (UMGCheckpointSubsystem* CheckpointSubsystem = World->GetSubsystem<UMGCheckpointSubsystem>())
		{
			CheckpointSubsystem->OnWrongWay.RemoveDynamic(this, &AMGPlayerController::OnWrongWayDetected);
		}

		if (UMGNearMissSubsystem* NearMissSubsystem = World->GetSubsystem<UMGNearMissSubsystem>())
		{
			NearMissSubsystem->OnNearMissOccurred.RemoveDynamic(this, &AMGPlayerController::OnNearMissDetected);
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGDriftSubsystem* DriftSubsystem = GI->GetSubsystem<UMGDriftSubsystem>())
		{
			DriftSubsystem->OnDriftEnded.RemoveDynamic(this, &AMGPlayerController::OnDriftEnded);
		}

		if (UMGAirtimeSubsystem* AirtimeSubsystem = GI->GetSubsystem<UMGAirtimeSubsystem>())
		{
			AirtimeSubsystem->OnJumpEnded.RemoveDynamic(this, &AMGPlayerController::OnJumpEnded);
			AirtimeSubsystem->OnTrickCompleted.RemoveDynamic(this, &AMGPlayerController::OnTrickCompleted);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Driving inputs
		if (AccelerateAction)
		{
			EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnAccelerate);
			EnhancedInputComponent->BindAction(AccelerateAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnAccelerateReleased);
		}

		if (BrakeAction)
		{
			EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnBrake);
			EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnBrakeReleased);
		}

		if (SteerAction)
		{
			EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnSteer);
			EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnSteer);
		}

		if (HandbrakeAction)
		{
			EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AMGPlayerController::OnHandbrake);
			EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnHandbrakeReleased);
		}

		if (NitroAction)
		{
			EnhancedInputComponent->BindAction(NitroAction, ETriggerEvent::Started, this, &AMGPlayerController::OnNitro);
			EnhancedInputComponent->BindAction(NitroAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnNitroReleased);
		}

		if (ShiftUpAction)
		{
			EnhancedInputComponent->BindAction(ShiftUpAction, ETriggerEvent::Started, this, &AMGPlayerController::OnShiftUp);
		}

		if (ShiftDownAction)
		{
			EnhancedInputComponent->BindAction(ShiftDownAction, ETriggerEvent::Started, this, &AMGPlayerController::OnShiftDown);
		}

		if (LookBackAction)
		{
			EnhancedInputComponent->BindAction(LookBackAction, ETriggerEvent::Started, this, &AMGPlayerController::OnLookBack);
			EnhancedInputComponent->BindAction(LookBackAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnLookBackReleased);
		}

		if (HornAction)
		{
			EnhancedInputComponent->BindAction(HornAction, ETriggerEvent::Started, this, &AMGPlayerController::OnHorn);
			EnhancedInputComponent->BindAction(HornAction, ETriggerEvent::Completed, this, &AMGPlayerController::OnHornReleased);
		}

		if (ResetVehicleAction)
		{
			EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Started, this, &AMGPlayerController::OnResetVehicle);
		}

		if (CycleCameraAction)
		{
			EnhancedInputComponent->BindAction(CycleCameraAction, ETriggerEvent::Started, this, &AMGPlayerController::OnCycleCamera);
		}

		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AMGPlayerController::OnPause);
		}

		if (MapAction)
		{
			EnhancedInputComponent->BindAction(MapAction, ETriggerEvent::Started, this, &AMGPlayerController::OnMap);
		}

		// Quick chat
		if (QuickChat1Action)
		{
			EnhancedInputComponent->BindAction(QuickChat1Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat1);
		}
		if (QuickChat2Action)
		{
			EnhancedInputComponent->BindAction(QuickChat2Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat2);
		}
		if (QuickChat3Action)
		{
			EnhancedInputComponent->BindAction(QuickChat3Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat3);
		}
		if (QuickChat4Action)
		{
			EnhancedInputComponent->BindAction(QuickChat4Action, ETriggerEvent::Started, this, &AMGPlayerController::OnQuickChat4);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGPlayerController::OnLook);
		}

		if (RewindAction)
		{
			EnhancedInputComponent->BindAction(RewindAction, ETriggerEvent::Started, this, &AMGPlayerController::OnRewind);
		}
	}
}

void AMGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Check if it's a vehicle
	if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(InPawn))
	{
		ControlledVehicle = Vehicle;
		SetInputMode(EMGInputMode::Driving);
		OnVehiclePossessed.Broadcast(Vehicle);
	}
}

void AMGPlayerController::OnUnPossess()
{
	if (ControlledVehicle)
	{
		ControlledVehicle = nullptr;
		OnVehicleUnpossessed.Broadcast();
	}

	Super::OnUnPossess();
}

void AMGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (IsLocalController() && CanDrive())
	{
		ApplyVehicleInput();

		// Send input to server
		if (!HasAuthority())
		{
			ServerUpdateVehicleInput(VehicleInput);
		}
	}
}

void AMGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGPlayerController, ControlledVehicle);
	DOREPLIFETIME(AMGPlayerController, VehicleInput);
	DOREPLIFETIME(AMGPlayerController, bRaceStarted);
}

void AMGPlayerController::SetInputMode(EMGInputMode NewMode)
{
	if (CurrentInputMode != NewMode)
	{
		CurrentInputMode = NewMode;
		UpdateInputMappingContext();
		OnInputModeChanged.Broadcast(NewMode);
	}
}

bool AMGPlayerController::CanDrive() const
{
	return CurrentInputMode == EMGInputMode::Driving &&
	       bRaceStarted &&
	       ControlledVehicle != nullptr;
}

void AMGPlayerController::RequestVehicleReset()
{
	if (HasAuthority())
	{
		OnResetVehicleRequested.Broadcast();
	}
	else
	{
		ServerRequestVehicleReset();
	}
}

void AMGPlayerController::CycleCamera()
{
	CurrentCameraIndex = (CurrentCameraIndex + 1) % NumCameras;
	// Camera switching is handled by the vehicle pawn
}

void AMGPlayerController::EnterSpectatorMode()
{
	SetInputMode(EMGInputMode::Spectating);

	// Find first available spectate target
	TArray<APlayerState*> Targets = GetSpectateTargets();
	if (Targets.Num() > 0)
	{
		SpectateTarget = Targets[0];
	}
}

void AMGPlayerController::ExitSpectatorMode()
{
	SpectateTarget = nullptr;
	SetInputMode(EMGInputMode::Driving);
}

void AMGPlayerController::SpectateNextPlayer()
{
	if (CurrentInputMode != EMGInputMode::Spectating)
	{
		return;
	}

	TArray<APlayerState*> Targets = GetSpectateTargets();
	if (Targets.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = SpectateTarget ? Targets.IndexOfByKey(SpectateTarget) : -1;
	int32 NextIndex = (CurrentIndex + 1) % Targets.Num();
	SpectateTarget = Targets[NextIndex];
}

void AMGPlayerController::SpectatePreviousPlayer()
{
	if (CurrentInputMode != EMGInputMode::Spectating)
	{
		return;
	}

	TArray<APlayerState*> Targets = GetSpectateTargets();
	if (Targets.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = SpectateTarget ? Targets.IndexOfByKey(SpectateTarget) : 0;
	int32 PrevIndex = (CurrentIndex - 1 + Targets.Num()) % Targets.Num();
	SpectateTarget = Targets[PrevIndex];
}

void AMGPlayerController::SendQuickChat(int32 Index)
{
	if (Index >= 1 && Index <= 4)
	{
		ServerSendQuickChat(Index);
		OnQuickChatSent.Broadcast(Index);
	}
}

void AMGPlayerController::ServerSendQuickChat_Implementation(int32 Index)
{
	// Broadcast to all players via game state or chat subsystem
	// Implementation depends on chat system
}

void AMGPlayerController::TogglePauseMenu()
{
	bPauseMenuOpen = !bPauseMenuOpen;

	if (bPauseMenuOpen)
	{
		SetInputMode(EMGInputMode::Menu);
	}
	else
	{
		SetInputMode(EMGInputMode::Driving);
	}
}

void AMGPlayerController::OpenMap()
{
	// Open map UI - handled by UI subsystem
}

float AMGPlayerController::GetNetworkLatency() const
{
	if (PlayerState)
	{
		return PlayerState->GetPingInMilliseconds();
	}
	return 0.0f;
}

// ==========================================
// INPUT HANDLERS
// ==========================================

void AMGPlayerController::OnAccelerate(const FInputActionValue& Value)
{
	VehicleInput.Throttle = Value.Get<float>();
}

void AMGPlayerController::OnAccelerateReleased(const FInputActionValue& Value)
{
	VehicleInput.Throttle = 0.0f;
}

void AMGPlayerController::OnBrake(const FInputActionValue& Value)
{
	VehicleInput.Brake = Value.Get<float>();
}

void AMGPlayerController::OnBrakeReleased(const FInputActionValue& Value)
{
	VehicleInput.Brake = 0.0f;
}

void AMGPlayerController::OnSteer(const FInputActionValue& Value)
{
	VehicleInput.Steering = Value.Get<float>();
}

void AMGPlayerController::OnHandbrake(const FInputActionValue& Value)
{
	VehicleInput.bHandbrake = true;
}

void AMGPlayerController::OnHandbrakeReleased(const FInputActionValue& Value)
{
	VehicleInput.bHandbrake = false;
}

void AMGPlayerController::OnNitro(const FInputActionValue& Value)
{
	VehicleInput.bNitro = true;
}

void AMGPlayerController::OnNitroReleased(const FInputActionValue& Value)
{
	VehicleInput.bNitro = false;
}

void AMGPlayerController::OnShiftUp(const FInputActionValue& Value)
{
	VehicleInput.GearShift = 1;
}

void AMGPlayerController::OnShiftDown(const FInputActionValue& Value)
{
	VehicleInput.GearShift = -1;
}

void AMGPlayerController::OnLookBack(const FInputActionValue& Value)
{
	VehicleInput.bLookBack = true;
}

void AMGPlayerController::OnLookBackReleased(const FInputActionValue& Value)
{
	VehicleInput.bLookBack = false;
}

void AMGPlayerController::OnHorn(const FInputActionValue& Value)
{
	VehicleInput.bHorn = true;
}

void AMGPlayerController::OnHornReleased(const FInputActionValue& Value)
{
	VehicleInput.bHorn = false;
}

void AMGPlayerController::OnResetVehicle(const FInputActionValue& Value)
{
	RequestVehicleReset();
}

void AMGPlayerController::OnCycleCamera(const FInputActionValue& Value)
{
	CycleCamera();
}

void AMGPlayerController::OnPause(const FInputActionValue& Value)
{
	TogglePauseMenu();
}

void AMGPlayerController::OnMap(const FInputActionValue& Value)
{
	OpenMap();
}

void AMGPlayerController::OnQuickChat1(const FInputActionValue& Value)
{
	SendQuickChat(1);
}

void AMGPlayerController::OnQuickChat2(const FInputActionValue& Value)
{
	SendQuickChat(2);
}

void AMGPlayerController::OnQuickChat3(const FInputActionValue& Value)
{
	SendQuickChat(3);
}

void AMGPlayerController::OnQuickChat4(const FInputActionValue& Value)
{
	SendQuickChat(4);
}

void AMGPlayerController::OnLook(const FInputActionValue& Value)
{
	FVector2D LookValue = Value.Get<FVector2D>();
	VehicleInput.LookDirection = FVector(LookValue.X, LookValue.Y, 0.0f).GetSafeNormal();
}

void AMGPlayerController::OnRewind(const FInputActionValue& Value)
{
	// Rewind functionality - integrated with replay subsystem
}

// ==========================================
// INTERNAL
// ==========================================

void AMGPlayerController::ApplyVehicleInput()
{
	// Input application to vehicle pawn is handled in the pawn's tick
	// VehicleInput struct is already populated by input handlers

	// Clear one-shot inputs
	VehicleInput.GearShift = 0;
}

void AMGPlayerController::UpdateInputMappingContext()
{
	if (!IsLocalController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Remove current contexts
		if (DrivingMappingContext)
		{
			Subsystem->RemoveMappingContext(DrivingMappingContext);
		}
		if (MenuMappingContext)
		{
			Subsystem->RemoveMappingContext(MenuMappingContext);
		}

		// Add appropriate context
		switch (CurrentInputMode)
		{
		case EMGInputMode::Driving:
			if (DrivingMappingContext)
			{
				Subsystem->AddMappingContext(DrivingMappingContext, 1);
			}
			break;

		case EMGInputMode::Menu:
		case EMGInputMode::PhotoMode:
		case EMGInputMode::Chat:
			if (MenuMappingContext)
			{
				Subsystem->AddMappingContext(MenuMappingContext, 1);
			}
			break;

		case EMGInputMode::Spectating:
		case EMGInputMode::Replay:
			// Spectating uses a subset of driving controls
			if (DrivingMappingContext)
			{
				Subsystem->AddMappingContext(DrivingMappingContext, 1);
			}
			break;
		}
	}
}

TArray<APlayerState*> AMGPlayerController::GetSpectateTargets() const
{
	TArray<APlayerState*> Targets;

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			// Don't include ourselves
			if (PS && PS != PlayerState)
			{
				Targets.Add(PS);
			}
		}
	}

	return Targets;
}

void AMGPlayerController::ServerUpdateVehicleInput_Implementation(const FMGVehicleInputState& Input)
{
	VehicleInput = Input;
}

void AMGPlayerController::ServerRequestVehicleReset_Implementation()
{
	OnResetVehicleRequested.Broadcast();
}

void AMGPlayerController::ClientOnRaceStarted_Implementation()
{
	bRaceStarted = true;
	SetInputMode(EMGInputMode::Driving);
}

void AMGPlayerController::ClientOnRaceEnded_Implementation()
{
	bRaceStarted = false;
	SetInputMode(EMGInputMode::Menu);
}

void AMGPlayerController::OnWrongWayDetected(bool bIsWrongWay)
{
	// Forward wrong-way status to HUD subsystem
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			HUDSubsystem->ShowWrongWayWarning(bIsWrongWay);
		}
	}
}

void AMGPlayerController::OnNearMissDetected(const FMGNearMissEvent& Event, int32 TotalPoints)
{
	// Forward near miss to HUD subsystem
	if (UWorld* World = GetWorld())
	{
		if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
		{
			HUDSubsystem->ShowNearMissBonus(Event.BasePoints);
		}
	}
}

void AMGPlayerController::OnDriftEnded(const FMGDriftResult& Result)
{
	// Forward drift score to HUD subsystem
	if (Result.TotalPoints > 0 && !Result.bFailed)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				HUDSubsystem->ShowDriftScorePopup(Result.TotalPoints, Result.Multiplier);
			}
		}
	}
}

void AMGPlayerController::OnJumpEnded(const FString& PlayerId, const FMGJumpResult& Result)
{
	// Forward jump result to HUD subsystem
	if (Result.TotalScore > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				HUDSubsystem->ShowAirtimePopup(Result.AirtimeDuration, Result.TotalScore);
			}
		}
	}
}

void AMGPlayerController::OnTrickCompleted(const FString& PlayerId, EMGTrickType Trick, int32 Score)
{
	// Forward trick to HUD subsystem
	if (Score > 0)
	{
		if (UWorld* World = GetWorld())
		{
			if (UMGRaceHUDSubsystem* HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>())
			{
				// Convert trick type to display name
				FText TrickName;
				switch (Trick)
				{
					case EMGTrickType::Flip: TrickName = FText::FromString(TEXT("FLIP")); break;
					case EMGTrickType::Barrel: TrickName = FText::FromString(TEXT("BARREL ROLL")); break;
					case EMGTrickType::Spin: TrickName = FText::FromString(TEXT("SPIN")); break;
					case EMGTrickType::Corkscrew: TrickName = FText::FromString(TEXT("CORKSCREW")); break;
					default: TrickName = FText::FromString(TEXT("TRICK")); break;
				}
				HUDSubsystem->ShowTrickPopup(TrickName, Score);
			}
		}
	}
}
