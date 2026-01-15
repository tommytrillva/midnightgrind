// Copyright Midnight Grind. All Rights Reserved.

#include "Tutorial/MGTutorialSubsystem.h"
#include "TimerManager.h"

void UMGTutorialSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadTutorialDefinitions();
	LoadTooltipDefinitions();
	LoadProgress();
}

void UMGTutorialSubsystem::Deinitialize()
{
	SaveProgress();
	Super::Deinitialize();
}

// ==========================================
// TUTORIALS
// ==========================================

void UMGTutorialSubsystem::StartTutorial(FName SequenceID)
{
	FMGTutorialSequence Sequence = GetSequence(SequenceID);
	if (Sequence.SequenceID.IsNone())
	{
		return;
	}

	// Check prerequisites
	if (!Sequence.RequiredSequence.IsNone() && !IsSequenceCompleted(Sequence.RequiredSequence))
	{
		return;
	}

	// Check if already completed (for one-time tutorials)
	if (Sequence.bOneTime && IsSequenceCompleted(SequenceID))
	{
		return;
	}

	CurrentSequence = Sequence;
	CurrentStepIndex = 0;
	bTutorialActive = true;

	OnTutorialStarted.Broadcast(SequenceID, Sequence);

	SetupCurrentStep();
}

void UMGTutorialSubsystem::StopTutorial()
{
	if (!bTutorialActive)
	{
		return;
	}

	// Clear timer
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);
	}

	bTutorialActive = false;
	CurrentSequence = FMGTutorialSequence();
	CurrentStepIndex = 0;
}

void UMGTutorialSubsystem::SkipStep()
{
	if (!bTutorialActive)
	{
		return;
	}

	FMGTutorialStep CurrentStep = GetCurrentStep();
	if (!CurrentStep.bCanSkip)
	{
		return;
	}

	NextStep();
}

void UMGTutorialSubsystem::NextStep()
{
	if (!bTutorialActive)
	{
		return;
	}

	CurrentStepIndex++;

	if (CurrentStepIndex >= CurrentSequence.Steps.Num())
	{
		// Tutorial complete
		FName SequenceID = CurrentSequence.SequenceID;
		CompletedSequences.Add(SequenceID);
		SaveProgress();

		OnTutorialCompleted.Broadcast(SequenceID);
		StopTutorial();
		return;
	}

	SetupCurrentStep();
}

void UMGTutorialSubsystem::PreviousStep()
{
	if (!bTutorialActive || CurrentStepIndex <= 0)
	{
		return;
	}

	CurrentStepIndex--;
	SetupCurrentStep();
}

void UMGTutorialSubsystem::CompleteStep()
{
	if (!bTutorialActive)
	{
		return;
	}

	NextStep();
}

FMGTutorialStep UMGTutorialSubsystem::GetCurrentStep() const
{
	if (!bTutorialActive || CurrentStepIndex >= CurrentSequence.Steps.Num())
	{
		return FMGTutorialStep();
	}

	return CurrentSequence.Steps[CurrentStepIndex];
}

int32 UMGTutorialSubsystem::GetTotalSteps() const
{
	return CurrentSequence.Steps.Num();
}

bool UMGTutorialSubsystem::IsSequenceCompleted(FName SequenceID) const
{
	return CompletedSequences.Contains(SequenceID);
}

TArray<FName> UMGTutorialSubsystem::GetCompletedSequences() const
{
	return CompletedSequences.Array();
}

void UMGTutorialSubsystem::ResetTutorialProgress()
{
	CompletedSequences.Empty();
	SeenTooltips.Empty();
	bIsFirstTimeUser = true;
	SaveProgress();
}

// ==========================================
// TOOLTIPS
// ==========================================

void UMGTutorialSubsystem::ShowTooltip(FName TooltipID)
{
	FMGTooltip Tooltip = GetTooltip(TooltipID);
	if (Tooltip.TooltipID.IsNone())
	{
		return;
	}

	// Check if already seen (for one-time tooltips)
	if (Tooltip.bShowOnce && HasTooltipBeenSeen(TooltipID))
	{
		return;
	}

	CurrentTooltip = Tooltip;
	OnTooltipTriggered.Broadcast(Tooltip);
}

void UMGTutorialSubsystem::HideTooltip()
{
	if (!CurrentTooltip.TooltipID.IsNone())
	{
		MarkTooltipSeen(CurrentTooltip.TooltipID);
		CurrentTooltip = FMGTooltip();
	}
}

void UMGTutorialSubsystem::CheckTooltipTriggers(FName StatID, int32 Value)
{
	if (!bShowTutorialPrompts)
	{
		return;
	}

	for (const FMGTooltip& Tooltip : Tooltips)
	{
		if (Tooltip.TriggerStat == StatID && Value >= Tooltip.TriggerThreshold)
		{
			if (!HasTooltipBeenSeen(Tooltip.TooltipID))
			{
				ShowTooltip(Tooltip.TooltipID);
				break;
			}
		}
	}
}

void UMGTutorialSubsystem::MarkTooltipSeen(FName TooltipID)
{
	SeenTooltips.Add(TooltipID);
	SaveProgress();
}

bool UMGTutorialSubsystem::HasTooltipBeenSeen(FName TooltipID) const
{
	return SeenTooltips.Contains(TooltipID);
}

// ==========================================
// ONBOARDING
// ==========================================

void UMGTutorialSubsystem::CompleteOnboarding()
{
	bIsFirstTimeUser = false;
	SaveProgress();
}

void UMGTutorialSubsystem::SetTutorialPromptsEnabled(bool bEnabled)
{
	bShowTutorialPrompts = bEnabled;
	SaveProgress();
}

// ==========================================
// INPUT
// ==========================================

void UMGTutorialSubsystem::ReportInput(FName InputAction, bool bPressed, float HoldTime)
{
	if (!bTutorialActive)
	{
		return;
	}

	FMGTutorialStep CurrentStep = GetCurrentStep();
	if (CurrentStep.Type != EMGTutorialStepType::Interactive)
	{
		return;
	}

	if (CurrentStep.RequiredInput == InputAction)
	{
		if (CurrentStep.RequiredHoldTime > 0.0f)
		{
			if (HoldTime >= CurrentStep.RequiredHoldTime)
			{
				CompleteStep();
			}
		}
		else if (bPressed)
		{
			CompleteStep();
		}
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGTutorialSubsystem::LoadTutorialDefinitions()
{
	// Basic controls tutorial
	{
		FMGTutorialSequence Sequence;
		Sequence.SequenceID = FName("BasicControls");
		Sequence.SequenceName = FText::FromString("Basic Controls");
		Sequence.Category = EMGTutorialCategory::Controls;
		Sequence.CompletionReward = 500;

		// Acceleration
		{
			FMGTutorialStep Step;
			Step.StepID = FName("Accelerate");
			Step.Type = EMGTutorialStepType::Interactive;
			Step.Title = FText::FromString("Acceleration");
			Step.Description = FText::FromString("Press and hold the accelerator to speed up");
			Step.InputPrompt = FText::FromString("Hold RT / R2 / W");
			Step.RequiredInput = FName("Accelerate");
			Step.RequiredHoldTime = 1.0f;
			Sequence.Steps.Add(Step);
		}

		// Braking
		{
			FMGTutorialStep Step;
			Step.StepID = FName("Brake");
			Step.Type = EMGTutorialStepType::Interactive;
			Step.Title = FText::FromString("Braking");
			Step.Description = FText::FromString("Press the brake to slow down");
			Step.InputPrompt = FText::FromString("Press LT / L2 / S");
			Step.RequiredInput = FName("Brake");
			Sequence.Steps.Add(Step);
		}

		// Steering
		{
			FMGTutorialStep Step;
			Step.StepID = FName("Steer");
			Step.Type = EMGTutorialStepType::Instruction;
			Step.Title = FText::FromString("Steering");
			Step.Description = FText::FromString("Use the left stick or A/D keys to steer your vehicle");
			Step.AutoAdvanceDelay = 3.0f;
			Sequence.Steps.Add(Step);
		}

		TutorialSequences.Add(Sequence);
	}

	// Racing tutorial
	{
		FMGTutorialSequence Sequence;
		Sequence.SequenceID = FName("RacingBasics");
		Sequence.SequenceName = FText::FromString("Racing Basics");
		Sequence.Category = EMGTutorialCategory::Racing;
		Sequence.RequiredSequence = FName("BasicControls");
		Sequence.CompletionReward = 1000;

		// NOS
		{
			FMGTutorialStep Step;
			Step.StepID = FName("NOS");
			Step.Type = EMGTutorialStepType::Interactive;
			Step.Title = FText::FromString("Nitrous Oxide");
			Step.Description = FText::FromString("Press the NOS button for a speed boost");
			Step.InputPrompt = FText::FromString("Press A / X / Space");
			Step.RequiredInput = FName("NOS");
			Sequence.Steps.Add(Step);
		}

		// Drifting
		{
			FMGTutorialStep Step;
			Step.StepID = FName("Drift");
			Step.Type = EMGTutorialStepType::Instruction;
			Step.Title = FText::FromString("Drifting");
			Step.Description = FText::FromString("Release the accelerator and turn sharply to initiate a drift. Drifting charges your NOS!");
			Step.AutoAdvanceDelay = 4.0f;
			Sequence.Steps.Add(Step);
		}

		// Checkpoints
		{
			FMGTutorialStep Step;
			Step.StepID = FName("Checkpoints");
			Step.Type = EMGTutorialStepType::Instruction;
			Step.Title = FText::FromString("Checkpoints");
			Step.Description = FText::FromString("Pass through all checkpoints to complete a lap. Missing a checkpoint will invalidate your lap!");
			Step.AutoAdvanceDelay = 4.0f;
			Sequence.Steps.Add(Step);
		}

		TutorialSequences.Add(Sequence);
	}

	// Advanced techniques
	{
		FMGTutorialSequence Sequence;
		Sequence.SequenceID = FName("AdvancedTechniques");
		Sequence.SequenceName = FText::FromString("Advanced Techniques");
		Sequence.Category = EMGTutorialCategory::Advanced;
		Sequence.RequiredSequence = FName("RacingBasics");
		Sequence.CompletionReward = 2000;

		// Perfect start
		{
			FMGTutorialStep Step;
			Step.StepID = FName("PerfectStart");
			Step.Type = EMGTutorialStepType::Instruction;
			Step.Title = FText::FromString("Perfect Start");
			Step.Description = FText::FromString("Time your acceleration at the start of the race for a speed boost!");
			Step.AutoAdvanceDelay = 3.0f;
			Sequence.Steps.Add(Step);
		}

		// Slipstream
		{
			FMGTutorialStep Step;
			Step.StepID = FName("Slipstream");
			Step.Type = EMGTutorialStepType::Instruction;
			Step.Title = FText::FromString("Slipstreaming");
			Step.Description = FText::FromString("Drive close behind opponents to draft and gain speed for an overtake");
			Step.AutoAdvanceDelay = 4.0f;
			Sequence.Steps.Add(Step);
		}

		// Racing line
		{
			FMGTutorialStep Step;
			Step.StepID = FName("RacingLine");
			Step.Type = EMGTutorialStepType::Instruction;
			Step.Title = FText::FromString("The Racing Line");
			Step.Description = FText::FromString("Follow the optimal racing line to maintain speed through corners");
			Step.AutoAdvanceDelay = 4.0f;
			Sequence.Steps.Add(Step);
		}

		TutorialSequences.Add(Sequence);
	}
}

void UMGTutorialSubsystem::LoadTooltipDefinitions()
{
	// First race tooltip
	{
		FMGTooltip Tooltip;
		Tooltip.TooltipID = FName("FirstRace");
		Tooltip.Title = FText::FromString("Ready to Race?");
		Tooltip.Description = FText::FromString("Select Quick Play to jump into your first race!");
		Tooltip.bShowOnce = true;
		Tooltips.Add(Tooltip);
	}

	// Low NOS tooltip
	{
		FMGTooltip Tooltip;
		Tooltip.TooltipID = FName("NOSTip");
		Tooltip.Title = FText::FromString("Need More NOS?");
		Tooltip.Description = FText::FromString("Drift to recharge your NOS meter!");
		Tooltip.TriggerStat = FName("NOSUsed");
		Tooltip.TriggerThreshold = 5;
		Tooltip.bShowOnce = true;
		Tooltips.Add(Tooltip);
	}

	// Garage tooltip
	{
		FMGTooltip Tooltip;
		Tooltip.TooltipID = FName("GarageTip");
		Tooltip.Title = FText::FromString("Visit the Garage");
		Tooltip.Description = FText::FromString("Check out the garage to customize your vehicle!");
		Tooltip.TriggerStat = FName("RacesCompleted");
		Tooltip.TriggerThreshold = 3;
		Tooltip.bShowOnce = true;
		Tooltips.Add(Tooltip);
	}
}

void UMGTutorialSubsystem::LoadProgress()
{
	// Would load from save data
	// For now, start fresh
	bIsFirstTimeUser = true;
}

void UMGTutorialSubsystem::SaveProgress()
{
	// Would save to persistent storage
}

void UMGTutorialSubsystem::OnAutoAdvanceTimer()
{
	NextStep();
}

void UMGTutorialSubsystem::SetupCurrentStep()
{
	FMGTutorialStep Step = GetCurrentStep();
	if (Step.StepID.IsNone())
	{
		return;
	}

	// Clear any existing timer
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoAdvanceTimerHandle);

		// Set auto-advance timer if needed
		if (Step.AutoAdvanceDelay > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				AutoAdvanceTimerHandle,
				this,
				&UMGTutorialSubsystem::OnAutoAdvanceTimer,
				Step.AutoAdvanceDelay,
				false
			);
		}
	}

	OnTutorialStepChanged.Broadcast(CurrentStepIndex, Step);
}

FMGTutorialSequence UMGTutorialSubsystem::GetSequence(FName SequenceID) const
{
	for (const FMGTutorialSequence& Sequence : TutorialSequences)
	{
		if (Sequence.SequenceID == SequenceID)
		{
			return Sequence;
		}
	}
	return FMGTutorialSequence();
}

FMGTooltip UMGTutorialSubsystem::GetTooltip(FName TooltipID) const
{
	for (const FMGTooltip& Tooltip : Tooltips)
	{
		if (Tooltip.TooltipID == TooltipID)
		{
			return Tooltip;
		}
	}
	return FMGTooltip();
}
