// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGTutorialSubsystem.h - Interactive Tutorial & Contextual Help System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Tutorial Subsystem, which manages step-by-step tutorials
 * and contextual tooltips throughout Midnight Grind. It teaches players how to
 * play the game through interactive sequences and provides ongoing help hints.
 *
 * Think of this as the "driving instructor" - it walks players through game
 * mechanics with hands-on practice, not just text explanations.
 *
 *
 * KEY CONCEPTS & TERMINOLOGY:
 * ---------------------------
 *
 * 1. TUTORIAL SEQUENCE: A complete lesson covering one topic.
 *    - Example: "Basic Controls", "Drifting Fundamentals", "Garage Customization"
 *    - Contains multiple ordered STEPS that the player progresses through
 *    - Can have prerequisites (must complete another sequence first)
 *    - Grants rewards upon completion
 *
 * 2. TUTORIAL STEP: A single instruction or interaction within a sequence.
 *    Four types of steps:
 *    - Instruction: Text/image explaining something (read and continue)
 *    - Interactive: Requires player input (press this button, perform this action)
 *    - Demonstration: Shows something happening with a timer (watch this)
 *    - Checkpoint: Requires completing a real gameplay action (now you do it)
 *
 * 3. TUTORIAL CATEGORY: Groups tutorials by topic.
 *    - Onboarding: First-time user basics
 *    - Controls: Input and handling
 *    - Racing: Race mechanics and strategy
 *    - Advanced: Expert techniques (drifting, drafting, etc.)
 *    - Multiplayer: Online features
 *    - Garage: Vehicle customization
 *
 * 4. TOOLTIP: A small contextual hint that appears during gameplay.
 *    - Less intrusive than full tutorials
 *    - Triggered by player actions or game state
 *    - Can be configured to show only once (bShowOnce)
 *    - Example: "TIP: Hold brake while turning to initiate a drift"
 *
 * 5. HIGHLIGHT: Visual emphasis on UI elements or world locations.
 *    - HighlightWidget: Spotlights a UI element (button, menu, etc.)
 *    - HighlightLocation: Points to a spot in the 3D world
 *    - Helps players know WHERE to look/click
 *
 * 6. REQUIRED INPUT: For Interactive steps, what the player must do.
 *    - RequiredInput: The input action name (e.g., "Accelerate", "Brake")
 *    - RequiredHoldTime: How long they must hold it (for gas/brake tutorials)
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 * This is a UGameInstanceSubsystem that:
 * - Persists throughout the game session
 * - Tracks which tutorials and tooltips have been seen
 * - Communicates with UI to display tutorial overlays
 * - Receives input events to detect player actions
 *
 * Key relationships:
 * - MGFTUESubsystem: FTUE = high-level onboarding flow; Tutorial = detailed teaching
 * - Input System: Reports player inputs for interactive steps
 * - UI System: Displays tutorial widgets, tooltips, highlights
 * - Audio System: Plays voice-over for tutorial steps
 *
 *
 * TUTORIAL vs FTUE - IMPORTANT DISTINCTION:
 * -----------------------------------------
 * - FTUE (First-Time User Experience): High-level onboarding STAGES
 *   "Choose your first car" -> "Complete first race" -> "Join multiplayer"
 *
 * - TUTORIAL: Detailed interactive LESSONS within those stages
 *   "Here's how to steer" -> "Now press the gas" -> "Try braking here"
 *
 * The FTUE subsystem might trigger a Tutorial sequence as part of its flow.
 *
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // Start a tutorial when player enters a new area:
 * if (!TutorialSubsystem->IsSequenceCompleted("Tutorial_Garage"))
 * {
 *     TutorialSubsystem->StartTutorial("Tutorial_Garage");
 * }
 *
 * // Report player input for interactive steps:
 * TutorialSubsystem->ReportInput("Accelerate", true, 0.5f); // Pressed for 0.5 sec
 *
 * // Trigger tooltip when player does something for the first time:
 * TutorialSubsystem->ShowTooltip("Tip_NitroBoost");
 *
 * // Listen for tutorial events in UI:
 * TutorialSubsystem->OnTutorialStepChanged.AddDynamic(this, &UMyWidget::UpdateTutorialUI);
 *
 * // Check if we should skip tutorials for returning players:
 * if (!TutorialSubsystem->IsFirstTimeUser())
 * {
 *     // Maybe skip or offer to skip tutorials
 * }
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTutorialSubsystem.generated.h"

class UUserWidget;
class UTexture2D;

/**
 * Tutorial Step Type - How the player interacts with each step
 *
 * Different step types require different UI treatment and player engagement.
 * The tutorial system uses this to know when to auto-advance vs wait for
 * player action.
 */
UENUM(BlueprintType)
enum class EMGTutorialStepType : uint8
{
	/**
	 * Text/image instruction - Read and press continue.
	 * Shows information, player advances when ready.
	 * Example: "Racing in Midnight Grind is about speed AND style."
	 */
	Instruction,

	/**
	 * Interactive prompt - Wait for specific input.
	 * Tutorial pauses until player performs the required action.
	 * Example: "Press and HOLD the gas trigger" (waits for input).
	 */
	Interactive,

	/**
	 * Timed demonstration - Show something, auto-advance.
	 * Plays an animation or demonstration, then moves on.
	 * Example: Shows AI car performing a drift for 5 seconds.
	 */
	Demonstration,

	/**
	 * Checkpoint - Player must complete a real gameplay action.
	 * More open-ended than Interactive; tests actual gameplay.
	 * Example: "Now drift around this corner" (measures drift).
	 */
	Checkpoint
};

/**
 * Tutorial Category - Groups tutorials by game area or skill level
 *
 * Categories help organize tutorials in the menu and determine when
 * tutorials should be suggested to players. They also control unlock
 * requirements (e.g., Advanced tutorials need Racing tutorials first).
 */
UENUM(BlueprintType)
enum class EMGTutorialCategory : uint8
{
	/** First-time player basics - Mandatory for new players. */
	Onboarding,

	/** Input and handling - Gas, brake, steering, camera. */
	Controls,

	/** Race mechanics - Starts, drafting, positioning, finish. */
	Racing,

	/** Expert techniques - Drifting, advanced drafting, shortcuts. */
	Advanced,

	/** Online features - Lobbies, matchmaking, social. */
	Multiplayer,

	/** Vehicle customization - Parts, tuning, visuals. */
	Garage
};

/**
 * Tutorial Step Definition - Configuration for a single tutorial step
 *
 * Each step in a tutorial sequence has content (what to show), interaction
 * requirements (what the player must do), and presentation options
 * (highlights, voice-over, timing).
 */
USTRUCT(BlueprintType)
struct FMGTutorialStep
{
	GENERATED_BODY()

	/** Unique ID within the sequence (e.g., "Step_PressGas"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StepID;

	/** How this step behaves (instruction, interactive, demo, checkpoint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTutorialStepType Type = EMGTutorialStepType::Instruction;

	/** Headline text (e.g., "Acceleration"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Full instruction text explaining what to do and why. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Optional image to display (controller diagram, technique illustration). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Image;

	/** For Interactive steps: specific prompt for the required input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText InputPrompt;

	/** For Interactive steps: which input action must be performed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredInput;

	/** For Interactive steps: how long to hold the input (0 = just press). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RequiredHoldTime = 0.0f;

	/** For Instruction/Demo steps: auto-advance after this many seconds (0 = wait for player). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoAdvanceDelay = 0.0f;

	/** UI widget to spotlight/highlight (draws attention to buttons, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HighlightWidget;

	/** 3D world position to highlight (for pointing at objects in the scene). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HighlightLocation = FVector::ZeroVector;

	/** If true, player can skip this step. Some critical steps cannot be skipped. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanSkip = true;

	/** Optional voice-over audio that plays with this step. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* VoiceOver;
};

/**
 * Tutorial Sequence Definition - A complete tutorial lesson
 *
 * A sequence is a complete tutorial that teaches one topic (e.g., "Basic Drifting").
 * It contains multiple ordered steps and can have prerequisites and rewards.
 */
USTRUCT(BlueprintType)
struct FMGTutorialSequence
{
	GENERATED_BODY()

	/** Unique identifier (e.g., "Tutorial_BasicDrifting"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SequenceID;

	/** Player-facing name shown in tutorial menu (e.g., "Basic Drifting"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SequenceName;

	/** Which category this tutorial belongs to (for menu organization). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTutorialCategory Category = EMGTutorialCategory::Onboarding;

	/** Ordered array of steps in this tutorial. Played in sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTutorialStep> Steps;

	/** Must complete this sequence before this one is available. Empty = no prereq. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredSequence;

	/** GrindCash reward for completing this tutorial. Incentivizes completion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionReward = 0;

	/** If true, this tutorial is marked "done" after completion and won't repeat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOneTime = true;
};

/**
 * Tooltip Definition - A small contextual hint
 *
 * Tooltips are less intrusive than full tutorials. They appear as small
 * hints when players encounter new features or make common mistakes.
 * They can be triggered by game state (stats) or shown manually.
 */
USTRUCT(BlueprintType)
struct FMGTooltip
{
	GENERATED_BODY()

	/** Unique identifier (e.g., "Tip_NitroEmpty"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TooltipID;

	/** Short headline (e.g., "Out of Nitro!"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Helpful explanation (e.g., "Drift to refill your nitro bar."). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Widget to point at when showing this tooltip (optional). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetWidget;

	/** If true, only show this tooltip once ever. If false, can repeat. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowOnce = true;

	/** Stat that triggers this tooltip (e.g., "NitroUseAttempts"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TriggerStat;

	/** Value of TriggerStat that triggers this tooltip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TriggerThreshold = 0;
};

/**
 * =============================================================================
 * DELEGATE DECLARATIONS
 * =============================================================================
 *
 * Events for the UI system to display tutorial content. The Tutorial subsystem
 * fires these events; the UI widgets listen and update their displays.
 */

/** Broadcast when a tutorial sequence begins. UI should show tutorial overlay. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStarted, FName, SequenceID, const FMGTutorialSequence&, Sequence);

/** Broadcast when tutorial sequence ends. UI should hide tutorial overlay. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialCompleted, FName, SequenceID);

/** Broadcast when advancing to a new step. UI should update content display. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepChanged, int32, StepIndex, const FMGTutorialStep&, Step);

/** Broadcast when a tooltip should appear. UI should show small hint popup. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTooltipTriggered, const FMGTooltip&, Tooltip);

/**
 * =============================================================================
 * UMGTutorialSubsystem - Interactive Tutorial & Contextual Help Management
 * =============================================================================
 *
 * This subsystem manages detailed interactive tutorials and contextual tooltips.
 * It handles the "teaching" part of the game - walking players through controls,
 * mechanics, and features with hands-on practice.
 *
 * Key capabilities:
 * - Tutorial sequences with multiple step types (instruction, interactive, checkpoint)
 * - Interactive prompts that detect and validate player input
 * - Contextual tooltips that appear based on game state
 * - Progress tracking to remember what's been completed
 * - Voice-over and visual highlight support
 *
 * As a UGameInstanceSubsystem:
 * - Automatically created when game starts
 * - Access via: GetGameInstance()->GetSubsystem<UMGTutorialSubsystem>()
 * - Persists across level loads
 *
 * Relationship with other systems:
 * - MGFTUESubsystem handles HIGH-LEVEL onboarding (what features to introduce)
 * - MGTutorialSubsystem handles DETAILED teaching (how to use each feature)
 * - The FTUE may trigger tutorial sequences as part of its flow
 *
 * The class is organized into sections:
 * - Events: Delegates for UI updates
 * - Tutorials: Sequence playback and navigation
 * - Tooltips: Contextual hint management
 * - Onboarding: First-time user status
 * - Input: Receiving player input for interactive steps
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTutorialSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called by engine when subsystem is created. Loads definitions and progress. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called by engine when subsystem is destroyed. Saves progress. */
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTutorialStarted OnTutorialStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTutorialCompleted OnTutorialCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTutorialStepChanged OnTutorialStepChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTooltipTriggered OnTooltipTriggered;

	// ==========================================
	// TUTORIALS
	// ==========================================

	/** Start a tutorial sequence */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorial(FName SequenceID);

	/** Stop current tutorial */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StopTutorial();

	/** Skip current step */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SkipStep();

	/** Advance to next step */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void NextStep();

	/** Go to previous step */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void PreviousStep();

	/** Complete current step (for checkpoints) */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void CompleteStep();

	/** Is tutorial active */
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTutorialActive() const { return bTutorialActive; }

	/** Get current step */
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	FMGTutorialStep GetCurrentStep() const;

	/** Get current step index */
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	/** Get total step count */
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetTotalSteps() const;

	/** Is sequence completed */
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsSequenceCompleted(FName SequenceID) const;

	/** Get all completed sequences */
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	TArray<FName> GetCompletedSequences() const;

	/** Reset tutorial progress */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ResetTutorialProgress();

	// ==========================================
	// TOOLTIPS
	// ==========================================

	/** Show tooltip */
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowTooltip(FName TooltipID);

	/** Hide current tooltip */
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void HideTooltip();

	/** Trigger tooltip check (based on stat) */
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void CheckTooltipTriggers(FName StatID, int32 Value);

	/** Mark tooltip as seen */
	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void MarkTooltipSeen(FName TooltipID);

	/** Has tooltip been seen */
	UFUNCTION(BlueprintPure, Category = "Tooltip")
	bool HasTooltipBeenSeen(FName TooltipID) const;

	// ==========================================
	// ONBOARDING
	// ==========================================

	/** Is first time user */
	UFUNCTION(BlueprintPure, Category = "Onboarding")
	bool IsFirstTimeUser() const { return bIsFirstTimeUser; }

	/** Mark onboarding complete */
	UFUNCTION(BlueprintCallable, Category = "Onboarding")
	void CompleteOnboarding();

	/** Should show tutorial prompts */
	UFUNCTION(BlueprintPure, Category = "Onboarding")
	bool ShouldShowTutorialPrompts() const { return bShowTutorialPrompts; }

	/** Set tutorial prompts enabled */
	UFUNCTION(BlueprintCallable, Category = "Onboarding")
	void SetTutorialPromptsEnabled(bool bEnabled);

	// ==========================================
	// INPUT
	// ==========================================

	/** Report input for interactive steps */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ReportInput(FName InputAction, bool bPressed, float HoldTime = 0.0f);

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Tutorial sequences */
	UPROPERTY()
	TArray<FMGTutorialSequence> TutorialSequences;

	/** Tooltips */
	UPROPERTY()
	TArray<FMGTooltip> Tooltips;

	/** Completed sequences */
	UPROPERTY()
	TSet<FName> CompletedSequences;

	/** Seen tooltips */
	UPROPERTY()
	TSet<FName> SeenTooltips;

	// ==========================================
	// STATE
	// ==========================================

	/** Is tutorial currently active */
	bool bTutorialActive = false;

	/** Current sequence */
	FMGTutorialSequence CurrentSequence;

	/** Current step index */
	int32 CurrentStepIndex = 0;

	/** Is first time user */
	bool bIsFirstTimeUser = true;

	/** Show tutorial prompts */
	bool bShowTutorialPrompts = true;

	/** Current tooltip showing */
	FMGTooltip CurrentTooltip;

	/** Timer handle for auto-advance */
	FTimerHandle AutoAdvanceTimerHandle;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Load tutorial definitions */
	void LoadTutorialDefinitions();

	/** Load tooltip definitions */
	void LoadTooltipDefinitions();

	/** Load saved progress */
	void LoadProgress();

	/** Save progress */
	void SaveProgress();

	/** Handle auto-advance timer */
	void OnAutoAdvanceTimer();

	/** Setup current step */
	void SetupCurrentStep();

	/** Get sequence by ID */
	FMGTutorialSequence GetSequence(FName SequenceID) const;

	/** Get tooltip by ID */
	FMGTooltip GetTooltip(FName TooltipID) const;
};
