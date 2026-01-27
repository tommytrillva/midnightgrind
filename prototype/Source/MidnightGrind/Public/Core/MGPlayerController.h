// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MGPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UMGInputRemapSubsystem;
class UMGSessionSubsystem;
class UMGChatSubsystem;
class AMGVehiclePawn;
struct FMGNearMissEvent;
struct FMGDriftResult;
struct FMGJumpResult;
enum class EMGTrickType : uint8;
enum class EMGFuelAlert : uint8;
enum class EMGTirePosition : uint8;
enum class EMGTireCondition : uint8;

/**
 * Vehicle input state - replicated for multiplayer
 */
USTRUCT(BlueprintType)
struct FMGVehicleInputState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Throttle = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Brake = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Steering = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool bHandbrake = false;

	UPROPERTY(BlueprintReadWrite)
	bool bNitro = false;

	UPROPERTY(BlueprintReadWrite)
	int32 GearShift = 0; // -1 down, 0 none, 1 up

	UPROPERTY(BlueprintReadWrite)
	bool bLookBack = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHorn = false;

	UPROPERTY(BlueprintReadWrite)
	FVector LookDirection = FVector::ForwardVector;
};

/**
 * Player controller input mode
 */
UENUM(BlueprintType)
enum class EMGInputMode : uint8
{
	/** Normal driving */
	Driving,
	/** In menu/UI */
	Menu,
	/** Spectating */
	Spectating,
	/** Replay playback */
	Replay,
	/** Photo mode */
	PhotoMode,
	/** Chat input */
	Chat
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputModeChanged, EMGInputMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehiclePossessed, AMGVehiclePawn*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleUnpossessed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickChatSent, int32, QuickChatIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetVehicleRequested);

/**
 * Player Controller
 * Handles player input, camera control, and multiplayer communication
 *
 * Features:
 * - Enhanced Input System integration
 * - Multiplayer input replication
 * - Input remapping support
 * - Camera management
 * - Quick chat system
 * - Spectator mode
 */
UCLASS()
class MIDNIGHTGRIND_API AMGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMGPlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInputModeChanged OnInputModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnVehiclePossessed OnVehiclePossessed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnVehicleUnpossessed OnVehicleUnpossessed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnQuickChatSent OnQuickChatSent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnResetVehicleRequested OnResetVehicleRequested;

	// ==========================================
	// INPUT MODE
	// ==========================================

	/** Get current input mode */
	UFUNCTION(BlueprintPure, Category = "Input")
	EMGInputMode GetCurrentInputMode() const { return CurrentInputMode; }

	/** Set input mode */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetInputMode(EMGInputMode NewMode);

	/** Get current vehicle input */
	UFUNCTION(BlueprintPure, Category = "Input")
	FMGVehicleInputState GetVehicleInput() const { return VehicleInput; }

	/** Is input enabled for driving */
	UFUNCTION(BlueprintPure, Category = "Input")
	bool CanDrive() const;

	// ==========================================
	// VEHICLE CONTROL
	// ==========================================

	/** Get controlled vehicle */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	AMGVehiclePawn* GetVehicle() const { return ControlledVehicle; }

	/** Request vehicle reset (respawn) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void RequestVehicleReset();

	/** Request camera change */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void CycleCamera();

	/** Get current camera index */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	int32 GetCurrentCameraIndex() const { return CurrentCameraIndex; }

	// ==========================================
	// SPECTATING
	// ==========================================

	/** Enter spectator mode */
	UFUNCTION(BlueprintCallable, Category = "Spectating")
	void EnterSpectatorMode();

	/** Exit spectator mode */
	UFUNCTION(BlueprintCallable, Category = "Spectating")
	void ExitSpectatorMode();

	/** Spectate next player */
	UFUNCTION(BlueprintCallable, Category = "Spectating")
	void SpectateNextPlayer();

	/** Spectate previous player */
	UFUNCTION(BlueprintCallable, Category = "Spectating")
	void SpectatePreviousPlayer();

	/** Get spectate target */
	UFUNCTION(BlueprintPure, Category = "Spectating")
	APlayerState* GetSpectateTarget() const { return SpectateTarget; }

	/** Is in spectator mode */
	UFUNCTION(BlueprintPure, Category = "Spectating")
	bool IsSpectating() const { return CurrentInputMode == EMGInputMode::Spectating; }

	// ==========================================
	// QUICK CHAT
	// ==========================================

	/** Send quick chat message */
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendQuickChat(int32 Index);

	/** Server RPC for quick chat */
	UFUNCTION(Server, Reliable, Category = "Chat")
	void ServerSendQuickChat(int32 Index);

	// ==========================================
	// PAUSE/MENU
	// ==========================================

	/** Toggle pause menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void TogglePauseMenu();

	/** Open map */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void OpenMap();

	/** Is pause menu open */
	UFUNCTION(BlueprintPure, Category = "Menu")
	bool IsPauseMenuOpen() const { return bPauseMenuOpen; }

	// ==========================================
	// NETWORK
	// ==========================================

	/** Get network latency (ping) in ms */
	UFUNCTION(BlueprintPure, Category = "Network")
	float GetNetworkLatency() const;

	/** Is local controller */
	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsLocalController() const { return IsLocalPlayerController(); }

protected:
	// ==========================================
	// INPUT CONFIGURATION
	// ==========================================

	/** Default input mapping context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Driving input mapping context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DrivingMappingContext;

	/** Menu input mapping context */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MenuMappingContext;

	// Input Actions
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> AccelerateAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> BrakeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> SteerAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> HandbrakeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> NitroAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftUpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ShiftDownAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookBackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> HornAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> ResetVehicleAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> CycleCameraAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> MapAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> QuickChat1Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> QuickChat2Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> QuickChat3Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> QuickChat4Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> RewindAction;

	// ==========================================
	// STATE
	// ==========================================

	/** Current input mode */
	EMGInputMode CurrentInputMode = EMGInputMode::Menu;

	/** Controlled vehicle */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Vehicle")
	TObjectPtr<AMGVehiclePawn> ControlledVehicle;

	/** Current vehicle input state */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Input")
	FMGVehicleInputState VehicleInput;

	/** Current camera index */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	int32 CurrentCameraIndex = 0;

	/** Number of cameras on current vehicle */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	int32 NumCameras = 3;

	/** Spectate target */
	UPROPERTY(BlueprintReadOnly, Category = "Spectating")
	TObjectPtr<APlayerState> SpectateTarget;

	/** Is pause menu open */
	bool bPauseMenuOpen = false;

	/** Is race started (controls can be disabled before race) */
	UPROPERTY(Replicated)
	bool bRaceStarted = false;

	/** Subsystem references */
	UPROPERTY()
	TObjectPtr<UMGInputRemapSubsystem> InputRemapSubsystem;

	// ==========================================
	// INPUT HANDLERS
	// ==========================================

	void OnAccelerate(const FInputActionValue& Value);
	void OnAccelerateReleased(const FInputActionValue& Value);
	void OnBrake(const FInputActionValue& Value);
	void OnBrakeReleased(const FInputActionValue& Value);
	void OnSteer(const FInputActionValue& Value);
	void OnHandbrake(const FInputActionValue& Value);
	void OnHandbrakeReleased(const FInputActionValue& Value);
	void OnNitro(const FInputActionValue& Value);
	void OnNitroReleased(const FInputActionValue& Value);
	void OnShiftUp(const FInputActionValue& Value);
	void OnShiftDown(const FInputActionValue& Value);
	void OnLookBack(const FInputActionValue& Value);
	void OnLookBackReleased(const FInputActionValue& Value);
	void OnHorn(const FInputActionValue& Value);
	void OnHornReleased(const FInputActionValue& Value);
	void OnResetVehicle(const FInputActionValue& Value);
	void OnCycleCamera(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);
	void OnMap(const FInputActionValue& Value);
	void OnQuickChat1(const FInputActionValue& Value);
	void OnQuickChat2(const FInputActionValue& Value);
	void OnQuickChat3(const FInputActionValue& Value);
	void OnQuickChat4(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void OnRewind(const FInputActionValue& Value);

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Apply vehicle input to pawn */
	void ApplyVehicleInput();

	/** Update input mapping context based on mode */
	void UpdateInputMappingContext();

	/** Find spectate targets */
	TArray<APlayerState*> GetSpectateTargets() const;

	/** Server RPC to update vehicle input */
	UFUNCTION(Server, Unreliable)
	void ServerUpdateVehicleInput(const FMGVehicleInputState& Input);

	/** Server RPC to request reset */
	UFUNCTION(Server, Reliable)
	void ServerRequestVehicleReset();

	/** Client callback when race starts */
	UFUNCTION(Client, Reliable)
	void ClientOnRaceStarted();

	/** Client callback when race ends */
	UFUNCTION(Client, Reliable)
	void ClientOnRaceEnded();

	/** Handle wrong-way detection from checkpoint subsystem */
	UFUNCTION()
	void OnWrongWayDetected(bool bIsWrongWay);

	/** Handle near miss events from near miss subsystem */
	UFUNCTION()
	void OnNearMissDetected(const FMGNearMissEvent& Event, int32 TotalPoints);

	/** Handle drift end events from drift subsystem */
	UFUNCTION()
	void OnDriftEnded(const FMGDriftResult& Result);

	/** Handle jump end events from airtime subsystem */
	UFUNCTION()
	void OnJumpEnded(const FString& PlayerId, const FMGJumpResult& Result);

	/** Handle trick completed events from airtime subsystem */
	UFUNCTION()
	void OnTrickCompleted(const FString& PlayerId, EMGTrickType Trick, int32 Score);

	/** Handle fuel alert from fuel subsystem */
	UFUNCTION()
	void OnFuelAlert(FName VehicleID, EMGFuelAlert Alert);

	/** Handle fuel empty from fuel subsystem */
	UFUNCTION()
	void OnFuelEmpty(FName VehicleID);

	/** Handle tire puncture from tire subsystem */
	UFUNCTION()
	void OnTirePunctured(FName VehicleID, EMGTirePosition Position);

	/** Handle tire condition change from tire subsystem */
	UFUNCTION()
	void OnTireConditionChanged(FName VehicleID, EMGTirePosition Position, EMGTireCondition NewCondition);
};
