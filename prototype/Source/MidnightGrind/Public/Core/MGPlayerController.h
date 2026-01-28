// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGPlayerController.h - Player Input and Control Management
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * The Player Controller is the bridge between the human player and the game.
 * It receives input from keyboard/gamepad, processes it, and sends commands
 * to the vehicle (Pawn). It also manages camera control, UI input modes,
 * spectator functionality, and multiplayer input replication.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. PLAYER CONTROLLER (APlayerController):
 *    - Represents the "player" - the human sitting at the computer
 *    - Persists across Pawn deaths/respawns (unlike the Pawn itself)
 *    - Handles input processing before passing to the Pawn
 *    - Manages possession (which Pawn the player controls)
 *    - One PlayerController per human player in the game
 *
 *    IMPORTANT DISTINCTION:
 *    - PlayerController = The player's "soul" - handles input, UI, cameras
 *    - Pawn = The player's "body" - the vehicle they're driving
 *    - PlayerState = The player's "stats" - score, name, replicated data
 *
 * 2. ENHANCED INPUT SYSTEM:
 *    - Unreal Engine 5's modern, flexible input system
 *    - Input Actions: Named actions (Accelerate, Brake, Steer)
 *    - Input Mapping Contexts: Which keys/buttons trigger which actions
 *    - FInputActionValue: The value from an input (axis values, booleans)
 *
 *    INPUT FLOW:
 *    Physical Input -> Mapping Context -> Input Action -> Handler Function
 *
 * 3. INPUT MODES:
 *    - Driving: Full vehicle control, minimal UI
 *    - Menu: Cursor visible, UI-focused input
 *    - Spectating: Camera control only, no vehicle
 *    - Replay: Playback controls, timeline scrubbing
 *    - PhotoMode: Camera positioning, no vehicle physics
 *    - Chat: Text input focused
 *
 * 4. NETWORK REPLICATION (Multiplayer):
 *    - Server: Has authority, validates all actions
 *    - Client: Predicts locally, sends requests to server
 *    - RPCs (Remote Procedure Calls): Functions that run on server/client
 *      - Server RPC: Client calls, server executes (e.g., ServerUpdateVehicleInput)
 *      - Client RPC: Server calls, client executes (e.g., ClientOnRaceStarted)
 *
 * 5. VEHICLE INPUT STATE (FMGVehicleInputState):
 *    - Captures all current input values in one struct
 *    - Replicated to server for authoritative physics
 *    - Includes: throttle, brake, steering, nitro, handbrake, gear shifts
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *   [Human Player]
 *         |
 *         v (input)
 *   [AMGPlayerController] <-- This file
 *         |
 *         +-- Possesses --> [AMGVehiclePawn] (the car)
 *         |
 *         +-- References --> [AMGPlayerState] (multiplayer data)
 *         |
 *         +-- Uses --> [UMGInputRemapSubsystem] (rebindable controls)
 *
 *   In Multiplayer:
 *   [Client PlayerController] --ServerRPC--> [Server] --Replication--> [All Clients]
 *
 * EVENT HANDLERS:
 * ---------------
 * This controller subscribes to MANY game events (near misses, drifts, jumps,
 * takedowns, etc.) to provide feedback to the player. These handlers are at
 * the bottom of the file and demonstrate how to:
 * - Subscribe to subsystem events
 * - Filter events for the local player
 * - Trigger UI feedback (HUD notifications, sounds)
 *
 * COMMON PATTERNS:
 * ----------------
 * @code
 * // Get the local player's controller
 * AMGPlayerController* PC = Cast<AMGPlayerController>(GetWorld()->GetFirstPlayerController());
 *
 * // Check if we can drive
 * if (PC && PC->CanDrive())
 * {
 *     // Vehicle controls are active
 * }
 *
 * // Switch to spectator mode
 * PC->EnterSpectatorMode();
 * PC->SpectateNextPlayer();
 *
 * // Send a quick chat message
 * PC->SendQuickChat(0); // "Good race!"
 * @endcode
 *
 * @see AMGVehiclePawn The vehicle being controlled
 * @see AMGPlayerState The player's replicated state
 * @see UMGInputConfig The input configuration asset
 * =============================================================================
 */

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
enum class EMGTakedownType : uint8;
enum class EMGPitLaneViolation : uint8;
enum class EMGPursuitIntensity : uint8;
struct FMGTakedownEvent;
struct FMGPitStopResult;
struct FMGBonusDefinition;
struct FMGPursuitUnit;
enum class EMGSpeedtrapRating : uint8;
struct FMGDestructionEvent;
struct FMGScoreEvent;
struct FMGAchievementDefinition;
enum class EMGStreakType : uint8;
enum class EMGStreakTier : uint8;
enum class EMGPrestigeRank : uint8;
enum class EMGDamageState : uint8;
struct FMGCheckpointPassage;
struct FMGLapData;
struct FMGStuntEvent;
enum class EMGPowerupType : uint8;
enum class EMGWeatherType : uint8;
enum class EMGCautionType : uint8;
enum class EMGCautionReason : uint8;
struct FMGSafetyCarState;
enum class EMGPenaltyType : uint8;
struct FMGPenalty;
enum class EMGHeatLevel : uint8;
struct FMGBountyCompletionResult;
struct FMGRaceEvent;
enum class EMGRaceEventType : uint8;
enum class EMGLicenseCategory : uint8;
enum class EMGLicenseTier : uint8;
enum class EMGTestGrade : uint8;
struct FMGContract;
struct FMGContractObjective;
struct FMGChallenge;
enum class EMGCurrencyType : uint8;
struct FMGEarningMultiplier;
struct FMGRewardClaimResult;
enum class EMGStreakMilestone : uint8;
struct FMGDailyReward;
enum class EMGReputationCategory : uint8;
enum class EMGReputationTier : uint8;
struct FMGReputationUnlock;
struct FMGGhostComparator;
enum class EMGGhostComparison : uint8;
enum class EMGCareerChapter : uint8;
enum class EMGCareerMilestone : uint8;
struct FMGCareerObjective;
struct FMGRival;
enum class EMGTimeOfDay : uint8;
enum class EMGDraftingZone : uint8;

// ============================================================================
// INPUT STATE STRUCT - Captures all vehicle control inputs
// ============================================================================

/**
 * Vehicle input state - replicated for multiplayer
 *
 * This struct captures the complete state of all vehicle controls at a
 * point in time. It's sent to the server each frame (via unreliable RPC)
 * so the server can simulate the vehicle authoritatively.
 *
 * WHY A STRUCT?
 * - Groups all inputs together for easy transmission
 * - Single replicated property instead of many
 * - Can be interpolated/predicted on clients
 *
 * VALUES:
 * - Throttle/Brake: 0.0 to 1.0 (how hard you're pressing)
 * - Steering: -1.0 (full left) to 1.0 (full right)
 * - Booleans: Digital on/off states
 * - GearShift: -1 = shift down, 0 = none, 1 = shift up
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

// ============================================================================
// INPUT MODE ENUM - What type of input is currently active
// ============================================================================

/**
 * Player controller input mode
 *
 * Different game situations require different input handling:
 * - Driving needs precise analog control
 * - Menus need mouse/cursor support
 * - Spectating needs camera-only controls
 *
 * When the mode changes, the controller swaps Input Mapping Contexts
 * to change which actions are available.
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

// ============================================================================
// DELEGATES - Events broadcast by the controller
// ============================================================================

/**
 * Controller event delegates
 *
 * These events let other systems (especially UI) react to controller
 * state changes. For example:
 * - OnInputModeChanged: Update HUD visibility
 * - OnVehiclePossessed: Initialize vehicle-specific UI
 * - OnQuickChatSent: Show chat bubble animation
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputModeChanged, EMGInputMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehiclePossessed, AMGVehiclePawn*, Vehicle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVehicleUnpossessed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickChatSent, int32, QuickChatIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResetVehicleRequested);

// ============================================================================
// MAIN PLAYER CONTROLLER CLASS
// ============================================================================

/**
 * Player Controller - The bridge between human input and game systems
 *
 * RESPONSIBILITIES:
 * -----------------
 * 1. INPUT PROCESSING
 *    - Receives raw input from Enhanced Input System
 *    - Converts to game actions (accelerate, steer, etc.)
 *    - Applies input mode filtering (can't drive in menus)
 *
 * 2. VEHICLE CONTROL
 *    - Possesses/unpossesses vehicle pawns
 *    - Sends input to vehicle for physics simulation
 *    - Handles camera switching
 *
 * 3. MULTIPLAYER
 *    - Replicates input to server (Server RPCs)
 *    - Receives game events from server (Client RPCs)
 *    - Manages local prediction vs server authority
 *
 * 4. UI COORDINATION
 *    - Toggles pause menu
 *    - Manages input mode (game vs UI)
 *    - Broadcasts events for HUD updates
 *
 * 5. SPECTATING
 *    - Allows watching other players after finishing/crashing
 *    - Cycles through active racers
 *
 * LIFECYCLE:
 * ----------
 * BeginPlay() -> SetupInputComponent() -> OnPossess() -> PlayerTick() -> OnUnPossess() -> EndPlay()
 *
 * IMPORTANT OVERRIDES:
 * - SetupInputComponent(): Binds input actions to handler functions
 * - OnPossess(): Called when taking control of a vehicle
 * - PlayerTick(): Called every frame, applies input to vehicle
 * - GetLifetimeReplicatedProps(): Defines what replicates in multiplayer
 *
 * @see APlayerController The Unreal base class
 * @see AMGVehiclePawn The vehicle being controlled
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
	// INPUT HANDLERS - Enhanced Input callbacks
	// ==========================================
	// These functions are bound to Input Actions in SetupInputComponent().
	// They receive FInputActionValue which contains the input data:
	// - For digital inputs: Get<bool>()
	// - For 1D axis: Get<float>()
	// - For 2D axis: Get<FVector2D>()
	// - For 3D axis: Get<FVector>()

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
	// INTERNAL HELPERS
	// ==========================================
	// Private implementation functions that support the public API.
	// These handle the "how" while public functions define the "what".

	/** Apply vehicle input to pawn - called every tick */
	void ApplyVehicleInput();

	/** Update input mapping context based on mode */
	void UpdateInputMappingContext();

	/** Find spectate targets */
	TArray<APlayerState*> GetSpectateTargets() const;

	// ==========================================
	// NETWORK RPCs - Remote Procedure Calls
	// ==========================================
	// RPCs are functions that execute on a different machine (client/server).
	// - Server: Client calls this, server executes it
	// - Client: Server calls this, specific client executes it
	// - NetMulticast: Server calls, all clients execute
	//
	// Reliable: Guaranteed delivery (use for important game events)
	// Unreliable: May be dropped (use for frequent updates like input)

	/** Server RPC to update vehicle input - called frequently, unreliable is fine */
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

	// ==========================================
	// GAME EVENT HANDLERS - Subsystem callbacks
	// ==========================================
	// The controller subscribes to events from various game subsystems.
	// When events occur, these handlers are called to provide player feedback.
	//
	// PATTERN: Each handler typically:
	// 1. Checks if the event is for the local player (GetLocalPlayerId())
	// 2. Triggers HUD notifications, sounds, or visual effects
	// 3. Updates any local state tracking
	//
	// These are bound in BeginPlay() using AddDynamic() on subsystem delegates.

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

	/** Handle takedown dealt from collision subsystem */
	UFUNCTION()
	void OnTakedownDealt(const FString& AttackerId, const FMGTakedownEvent& Takedown);

	/** Handle takedown chain from collision subsystem */
	UFUNCTION()
	void OnTakedownChain(const FString& PlayerId, int32 ChainCount, float ChainMultiplier);

	/** Handle revenge completed from collision subsystem */
	UFUNCTION()
	void OnRevengeComplete(const FString& AttackerId, const FString& OriginalAttackerId);

	/** Handle pit stop completed from pit stop subsystem */
	UFUNCTION()
	void OnPitStopCompleted(FName VehicleID, const FMGPitStopResult& Result);

	/** Handle pit lane violation from pit stop subsystem */
	UFUNCTION()
	void OnPitLaneViolation(FName VehicleID, EMGPitLaneViolation Violation);

	/** Handle bonus collected from bonus subsystem */
	UFUNCTION()
	void OnBonusCollected(const FString& PlayerId, const FMGBonusDefinition& Bonus, int32 PointsAwarded);

	/** Handle combo bonus from bonus subsystem */
	UFUNCTION()
	void OnComboBonusTriggered(const FString& PlayerId, int32 ComboLevel, int32 BonusPoints);

	/** Handle secret found from bonus subsystem */
	UFUNCTION()
	void OnSecretBonusFound(const FString& PlayerId, const FString& SecretId);

	/** Handle pursuit started from pursuit subsystem */
	UFUNCTION()
	void OnPursuitStarted(const FString& PlayerId, EMGPursuitIntensity Intensity);

	/** Handle pursuit ended from pursuit subsystem */
	UFUNCTION()
	void OnPursuitEnded(const FString& PlayerId, bool bEscaped, int32 FinalBounty);

	/** Handle unit disabled from pursuit subsystem */
	UFUNCTION()
	void OnUnitDisabled(const FString& PlayerId, const FMGPursuitUnit& Unit);

	/** Handle roadblock evaded from pursuit subsystem */
	UFUNCTION()
	void OnRoadblockEvaded(const FString& PlayerId, const FString& RoadblockId);

	/** Handle speedtrap recorded from speedtrap subsystem */
	UFUNCTION()
	void OnSpeedtrapRecorded(const FString& SpeedtrapId, float RecordedValue, EMGSpeedtrapRating Rating);

	/** Handle new personal best at speedtrap */
	UFUNCTION()
	void OnSpeedtrapNewPersonalBest(const FString& SpeedtrapId, float OldBest, float NewBest);

	/** Handle speedtrap discovered */
	UFUNCTION()
	void OnSpeedtrapDiscovered(const FString& SpeedtrapId, int32 TotalDiscovered);

	/** Handle destructible destroyed from destruction subsystem */
	UFUNCTION()
	void OnDestructibleDestroyed(const FString& PlayerId, const FMGDestructionEvent& Event);

	/** Handle destruction combo updated from destruction subsystem */
	UFUNCTION()
	void OnDestructionComboUpdated(const FString& PlayerId, int32 ComboCount, float Multiplier);

	/** Handle spectacular destruction from destruction subsystem */
	UFUNCTION()
	void OnSpectacularDestruction(const FString& PlayerId, int32 BonusPoints);

	/** Handle slipstream entered from aerodynamics subsystem */
	UFUNCTION()
	void OnSlipstreamEntered(const FString& FollowerId, const FString& LeaderId, float Distance);

	/** Handle slingshot ready from aerodynamics subsystem */
	UFUNCTION()
	void OnSlingshotReady(const FString& VehicleId, float BoostAmount, float Duration);

	/** Handle slingshot used from aerodynamics subsystem */
	UFUNCTION()
	void OnSlingshotUsed(const FString& VehicleId, float SpeedGained);

	/** Handle score event from scoring subsystem */
	UFUNCTION()
	void OnScoreEvent(const FString& PlayerId, const FMGScoreEvent& Event, int32 NewTotal);

	/** Handle chain extended from scoring subsystem */
	UFUNCTION()
	void OnChainExtended(const FString& PlayerId, int32 ChainLength, float Multiplier, int32 ChainPoints);

	/** Get local player ID for filtering events */
	FString GetLocalPlayerId() const;

	/** Handle achievement unlocked from achievement subsystem */
	UFUNCTION()
	void OnAchievementUnlocked(const FMGAchievementDefinition& Achievement, int32 TierUnlocked);

	/** Handle streak tier up from streak subsystem */
	UFUNCTION()
	void OnStreakTierUp(const FString& PlayerId, EMGStreakType Type, EMGStreakTier NewTier);

	/** Handle new streak record from streak subsystem */
	UFUNCTION()
	void OnNewStreakRecord(const FString& PlayerId, EMGStreakType Type);

	/** Handle prestige rank up from prestige subsystem */
	UFUNCTION()
	void OnPrestigeRankUp(const FString& PlayerId, EMGPrestigeRank OldRank, EMGPrestigeRank NewRank);

	/** Handle prestige level up from prestige subsystem */
	UFUNCTION()
	void OnPrestigeLevelUp(const FString& PlayerId, int32 OldLevel, int32 NewLevel);

	/** Handle damage state changed from collision subsystem */
	UFUNCTION()
	void OnDamageStateChanged(const FString& VehicleId, EMGDamageState OldState, EMGDamageState NewState);

	/** Handle checkpoint passed from checkpoint subsystem */
	UFUNCTION()
	void OnCheckpointPassed(const FMGCheckpointPassage& Passage, int32 CheckpointsRemaining, float DeltaTime);

	/** Handle lap completed from checkpoint subsystem */
	UFUNCTION()
	void OnLapCompleted(const FMGLapData& LapData, int32 LapsRemaining, bool bIsBestLap);

	/** Handle nitro depleted from nitro boost subsystem */
	UFUNCTION()
	void OnNitroDepleted();

	/** Handle nitro overheat from nitro boost subsystem */
	UFUNCTION()
	void OnNitroOverheat();

	/** Handle stunt completed from stunt subsystem */
	UFUNCTION()
	void OnStuntCompleted(const FMGStuntEvent& Event, int32 TotalPoints);

	/** Handle rampage activated from takedown subsystem */
	UFUNCTION()
	void OnRampageActivated(float Duration, float Multiplier);

	/** Handle powerup collected from powerup subsystem */
	UFUNCTION()
	void OnPowerupCollected(const FString& PlayerId, EMGPowerupType PowerupType, int32 SlotIndex);

	/** Handle powerup hit from powerup subsystem */
	UFUNCTION()
	void OnPowerupHit(const FString& SourceId, const FString& TargetId, EMGPowerupType PowerupType);

	/** Handle engine overheat from vehicle wear subsystem */
	UFUNCTION()
	void OnEngineOverheat(FGuid VehicleID);

	/** Handle weather transition from weather subsystem */
	UFUNCTION()
	void OnWeatherTransitionStarted(EMGWeatherType FromType, EMGWeatherType ToType);

	/** Handle caution deployed from caution subsystem */
	UFUNCTION()
	void OnCautionDeployed(EMGCautionType Type, EMGCautionReason Reason);

	/** Handle caution ended from caution subsystem */
	UFUNCTION()
	void OnCautionEnded(EMGCautionType Type);

	/** Handle safety car deployed from caution subsystem */
	UFUNCTION()
	void OnSafetyCarDeployed(const FMGSafetyCarState& State);

	/** Handle safety car coming in from caution subsystem */
	UFUNCTION()
	void OnSafetyCarIn();

	/** Handle penalty issued from penalty subsystem */
	UFUNCTION()
	void OnPenaltyIssued(const FMGPenalty& Penalty);

	/** Handle penalty served from penalty subsystem */
	UFUNCTION()
	void OnPenaltyServed(const FMGPenalty& Penalty);

	/** Handle heat level changed from heat level subsystem */
	UFUNCTION()
	void OnHeatLevelChanged(EMGHeatLevel OldLevel, EMGHeatLevel NewLevel);

	/** Handle pursuit evaded from heat level subsystem */
	UFUNCTION()
	void OnPursuitEvaded(float Duration, int32 BountyEarned);

	/** Handle player busted from heat level subsystem */
	UFUNCTION()
	void OnPlayerBusted(int32 TotalCost, float PursuitDuration);

	/** Handle helicopter deployed from heat level subsystem */
	UFUNCTION()
	void OnHelicopterDeployed();

	/** Handle bounty completed from bounty subsystem */
	UFUNCTION()
	void OnBountyCompleted(const FString& PlayerId, const FMGBountyCompletionResult& Result);

	/** Handle bounty failed from bounty subsystem */
	UFUNCTION()
	void OnBountyFailed(const FString& PlayerId, const FString& BountyId, const FString& Reason);

	/** Handle bounty objective completed from bounty subsystem */
	UFUNCTION()
	void OnBountyObjectiveCompleted(const FString& PlayerId, const FString& BountyId, const FString& ObjectiveId);

	/** Handle dramatic moment from race director */
	UFUNCTION()
	void OnDramaticMoment(const FMGRaceEvent& Event);

	/** Handle lead change from race director */
	UFUNCTION()
	void OnLeadChange(const FGuid& NewLeaderId, int32 TotalChanges);

	/** Handle license upgrade from license subsystem */
	UFUNCTION()
	void OnLicenseUpgraded(EMGLicenseCategory Category, EMGLicenseTier NewTier);

	/** Handle license test completed from license subsystem */
	UFUNCTION()
	void OnLicenseTestCompleted(const FString& TestId, EMGTestGrade Grade, float Time);

	/** Handle contract completed from contract subsystem */
	UFUNCTION()
	void OnContractCompleted(const FMGContract& Contract);

	/** Handle contract objective completed from contract subsystem */
	UFUNCTION()
	void OnContractObjectiveCompleted(FName ContractID, const FMGContractObjective& Objective);

	/** Handle sponsor level up from contract subsystem */
	UFUNCTION()
	void OnSponsorLevelUp(FName SponsorID, int32 NewLevel);

	/** Handle challenge completed from challenge subsystem */
	UFUNCTION()
	void OnChallengeCompleted(const FMGChallenge& Challenge);

	/** Handle currency changed from currency subsystem */
	UFUNCTION()
	void OnCurrencyChanged(EMGCurrencyType Type, int64 NewBalance, int64 Delta);

	/** Handle multiplier activated from currency subsystem */
	UFUNCTION()
	void OnMultiplierActivated(const FMGEarningMultiplier& Multiplier);

	/** Handle daily reward claimed from daily rewards subsystem */
	UFUNCTION()
	void OnDailyRewardClaimed(const FMGRewardClaimResult& Result);

	/** Handle streak milestone from daily rewards subsystem */
	UFUNCTION()
	void OnStreakMilestoneReached(EMGStreakMilestone Milestone, const TArray<FMGDailyReward>& Rewards);

	/** Handle reputation tier reached from reputation subsystem */
	UFUNCTION()
	void OnReputationTierReached(EMGReputationCategory Category, EMGReputationTier Tier);

	/** Handle reputation unlock earned from reputation subsystem */
	UFUNCTION()
	void OnReputationUnlockEarned(const FMGReputationUnlock& Unlock);

	/** Handle new personal best from ghost subsystem */
	UFUNCTION()
	void OnGhostNewPersonalBest(FName TrackID, float NewTime);

	/** Handle ghost comparison from ghost subsystem */
	UFUNCTION()
	void OnGhostComparison(const FMGGhostComparator& Comparison, EMGGhostComparison Status);

	/** Handle shortcut discovered from shortcut subsystem */
	UFUNCTION()
	void OnShortcutDiscovered(const FString& ShortcutId, int32 DiscoveryPoints);

	/** Handle shortcut completed from shortcut subsystem */
	UFUNCTION()
	void OnShortcutCompleted(const FString& ShortcutId, float TimeTaken, float TimeSaved);

	/** Handle shortcut mastered from shortcut subsystem */
	UFUNCTION()
	void OnShortcutMastered(const FString& ShortcutId, int32 BonusPoints);

	/** Handle secret shortcut found from shortcut subsystem */
	UFUNCTION()
	void OnSecretShortcutFound(const FString& ShortcutId, int32 BonusPoints);

	/** Handle career chapter advanced from career subsystem */
	UFUNCTION()
	void OnCareerChapterAdvanced(EMGCareerChapter NewChapter);

	/** Handle career milestone reached from career subsystem */
	UFUNCTION()
	void OnCareerMilestoneReached(EMGCareerMilestone Milestone);

	/** Handle career objective completed from career subsystem */
	UFUNCTION()
	void OnCareerObjectiveCompleted(const FMGCareerObjective& Objective);

	/** Handle new rival discovered from rivals subsystem */
	UFUNCTION()
	void OnNewRivalDiscovered(const FMGRival& Rival);

	/** Handle rival defeated from rivals subsystem */
	UFUNCTION()
	void OnRivalDefeated(const FMGRival& Rival, bool bWasCloseRace);

	/** Handle nemesis designated from rivals subsystem */
	UFUNCTION()
	void OnNemesisDesignated(const FMGRival& Nemesis);

	// ==========================================
	// TIME OF DAY HANDLERS
	// ==========================================

	/** Handle time of day period change */
	UFUNCTION()
	void OnTimeOfDayChanged(EMGTimeOfDay OldTime, EMGTimeOfDay NewTime);

	/** Handle midnight reached */
	UFUNCTION()
	void OnMidnightReached(int32 GameDay);

	// ==========================================
	// SLIPSTREAM HANDLERS
	// ==========================================

	/** Handle entering slipstream draft zone */
	UFUNCTION()
	void OnSlipstreamEntered(AActor* LeadVehicle, EMGDraftingZone Zone);

	/** Handle exiting slipstream */
	UFUNCTION()
	void OnSlipstreamExited();

	/** Handle slingshot boost ready */
	UFUNCTION()
	void OnSlingshotReady();

	/** Handle slingshot boost activated */
	UFUNCTION()
	void OnSlingshotActivated(float BonusSpeed);
};
