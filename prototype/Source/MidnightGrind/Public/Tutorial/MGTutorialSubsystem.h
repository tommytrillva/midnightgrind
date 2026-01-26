// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTutorialSubsystem.generated.h"

class UUserWidget;
class UTexture2D;

/**
 * Tutorial step type
 */
UENUM(BlueprintType)
enum class EMGTutorialStepType : uint8
{
	/** Text/image instruction */
	Instruction,
	/** Interactive prompt (wait for input) */
	Interactive,
	/** Timed demonstration */
	Demonstration,
	/** Checkpoint (player must complete action) */
	Checkpoint
};

/**
 * Tutorial category
 */
UENUM(BlueprintType)
enum class EMGTutorialCategory : uint8
{
	/** First-time user experience */
	Onboarding,
	/** Basic controls */
	Controls,
	/** Racing basics */
	Racing,
	/** Advanced techniques */
	Advanced,
	/** Multiplayer */
	Multiplayer,
	/** Garage/customization */
	Garage
};

/**
 * Tutorial step definition
 */
USTRUCT(BlueprintType)
struct FMGTutorialStep
{
	GENERATED_BODY()

	/** Step identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StepID;

	/** Step type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTutorialStepType Type = EMGTutorialStepType::Instruction;

	/** Title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Description/instruction text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Optional image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Image;

	/** Input prompt text (for interactive steps) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText InputPrompt;

	/** Required input action name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredInput;

	/** Minimum hold time for input (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RequiredHoldTime = 0.0f;

	/** Auto-advance delay (0 = manual) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoAdvanceDelay = 0.0f;

	/** Highlight UI element */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HighlightWidget;

	/** World location to highlight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HighlightLocation = FVector::ZeroVector;

	/** Can be skipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanSkip = true;

	/** Voice-over sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* VoiceOver;
};

/**
 * Tutorial sequence definition
 */
USTRUCT(BlueprintType)
struct FMGTutorialSequence
{
	GENERATED_BODY()

	/** Sequence identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SequenceID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SequenceName;

	/** Category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTutorialCategory Category = EMGTutorialCategory::Onboarding;

	/** Steps in sequence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTutorialStep> Steps;

	/** Required sequence to complete first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredSequence;

	/** Reward for completion (cash) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionReward = 0;

	/** Is this a one-time tutorial */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOneTime = true;
};

/**
 * Tooltip definition
 */
USTRUCT(BlueprintType)
struct FMGTooltip
{
	GENERATED_BODY()

	/** Tooltip identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TooltipID;

	/** Title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Associated widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetWidget;

	/** Show only once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowOnce = true;

	/** Trigger condition stat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TriggerStat;

	/** Trigger threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TriggerThreshold = 0;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStarted, FName, SequenceID, const FMGTutorialSequence&, Sequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialCompleted, FName, SequenceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTutorialStepChanged, int32, StepIndex, const FMGTutorialStep&, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTooltipTriggered, const FMGTooltip&, Tooltip);

/**
 * Tutorial Subsystem
 * Manages tutorials and contextual help
 *
 * Features:
 * - Tutorial sequences with steps
 * - Interactive prompts
 * - Contextual tooltips
 * - Progress tracking
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTutorialSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
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
