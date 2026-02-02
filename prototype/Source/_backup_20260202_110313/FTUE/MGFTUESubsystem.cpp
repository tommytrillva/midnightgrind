// Copyright Midnight Grind. All Rights Reserved.

#include "FTUE/MGFTUESubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/Paths.h"

void UMGFTUESubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeOnboardingSteps();
	InitializeHints();
	LoadFTUEData();
}

void UMGFTUESubsystem::Deinitialize()
{
	SaveFTUEData();
	Super::Deinitialize();
}

FMGOnboardingStep UMGFTUESubsystem::GetCurrentStep() const
{
	for (const FMGOnboardingStep& Step : OnboardingSteps)
	{
		if (Step.Stage == CurrentStage)
			return Step;
	}
	return FMGOnboardingStep();
}

float UMGFTUESubsystem::GetOnboardingProgress() const
{
	int32 Completed = 0;
	for (const FMGOnboardingStep& Step : OnboardingSteps)
	{
		if (Step.bCompleted)
			Completed++;
	}
	return OnboardingSteps.Num() > 0 ? (float)Completed / (float)OnboardingSteps.Num() * 100.0f : 100.0f;
}

void UMGFTUESubsystem::CompleteCurrentStep()
{
	for (FMGOnboardingStep& Step : OnboardingSteps)
	{
		if (Step.Stage == CurrentStage && !Step.bCompleted)
		{
			Step.bCompleted = true;
			GrantStepReward(Step);

			if (!Step.UnlockFeature.IsNone())
			{
				UnlockFeature(Step.UnlockFeature);
			}

			AdvanceStage();
			break;
		}
	}
}

void UMGFTUESubsystem::SkipCurrentStep()
{
	FMGOnboardingStep CurrentStep = GetCurrentStep();
	if (CurrentStep.bSkippable)
	{
		AdvanceStage();
	}
}

void UMGFTUESubsystem::SkipOnboarding()
{
	// Unlock all features
	UnlockFeature(FName(TEXT("Multiplayer")));
	UnlockFeature(FName(TEXT("Customization")));
	UnlockFeature(FName(TEXT("Crew")));
	UnlockFeature(FName(TEXT("Tournament")));
	UnlockFeature(FName(TEXT("SeasonPass")));

	CurrentStage = EMGOnboardingStage::Completed;
	bIsNewPlayer = false;
	OnOnboardingCompleted.Broadcast();
	SaveFTUEData();
}

void UMGFTUESubsystem::TriggerHint(FName Context)
{
	if (!bShowHints)
		return;

	for (FMGContextualHint& Hint : Hints)
	{
		if (Hint.TriggerContext == Context && !Hint.bDismissed && Hint.CurrentShowCount < Hint.MaxShowCount)
		{
			Hint.CurrentShowCount++;
			OnHintTriggered.Broadcast(Hint);
			break;
		}
	}
}

void UMGFTUESubsystem::DismissHint(FName HintID)
{
	for (FMGContextualHint& Hint : Hints)
	{
		if (Hint.HintID == HintID)
		{
			Hint.bDismissed = true;
			break;
		}
	}
	SaveFTUEData();
}

void UMGFTUESubsystem::DismissAllHints()
{
	for (FMGContextualHint& Hint : Hints)
	{
		Hint.bDismissed = true;
	}
	SaveFTUEData();
}

void UMGFTUESubsystem::SetShowHints(bool bShow)
{
	bShowHints = bShow;
	SaveFTUEData();
}

bool UMGFTUESubsystem::IsFeatureUnlocked(FName FeatureID) const
{
	return UnlockedFeatures.Contains(FeatureID) || IsOnboardingComplete();
}

TArray<FName> UMGFTUESubsystem::GetLockedFeatures() const
{
	TArray<FName> Locked;
	TArray<FName> AllFeatures = { FName(TEXT("Multiplayer")), FName(TEXT("Customization")), FName(TEXT("Crew")), FName(TEXT("Tournament")), FName(TEXT("SeasonPass")) };

	for (const FName& Feature : AllFeatures)
	{
		if (!IsFeatureUnlocked(Feature))
			Locked.Add(Feature);
	}
	return Locked;
}

void UMGFTUESubsystem::UnlockFeature(FName FeatureID)
{
	if (!UnlockedFeatures.Contains(FeatureID))
	{
		UnlockedFeatures.Add(FeatureID);
		OnFeatureUnlocked.Broadcast(FeatureID);
		SaveFTUEData();
	}
}

void UMGFTUESubsystem::LoadFTUEData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("FTUE");
	FString FilePath = SaveDir / TEXT("FTUEProgress.sav");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		// No save file - this is a new player
		bIsNewPlayer = true;
		CurrentStage = EMGOnboardingStage::Welcome;
		return;
	}

	FMemoryReader Archive(FileData, true);

	int32 Version = 0;
	Archive << Version;

	if (Version >= 1)
	{
		// Load current stage
		int32 StageInt = 0;
		Archive << StageInt;
		CurrentStage = static_cast<EMGOnboardingStage>(StageInt);

		// Load player flags
		Archive << bIsNewPlayer;
		Archive << bShowHints;

		// Load completed step stages
		int32 CompletedCount = 0;
		Archive << CompletedCount;
		TSet<int32> CompletedStages;
		for (int32 i = 0; i < CompletedCount; i++)
		{
			int32 CompletedStage;
			Archive << CompletedStage;
			CompletedStages.Add(CompletedStage);
		}

		// Apply completion to steps
		for (FMGOnboardingStep& Step : OnboardingSteps)
		{
			Step.bCompleted = CompletedStages.Contains(static_cast<int32>(Step.Stage));
		}

		// Load hint data
		int32 HintCount = 0;
		Archive << HintCount;
		for (int32 i = 0; i < HintCount; i++)
		{
			FName HintID;
			int32 ShowCount;
			bool bDismissed;
			Archive << HintID;
			Archive << ShowCount;
			Archive << bDismissed;

			// Apply to matching hint
			for (FMGContextualHint& Hint : Hints)
			{
				if (Hint.HintID == HintID)
				{
					Hint.CurrentShowCount = ShowCount;
					Hint.bDismissed = bDismissed;
					break;
				}
			}
		}

		// Load unlocked features
		int32 FeatureCount = 0;
		Archive << FeatureCount;
		for (int32 i = 0; i < FeatureCount; i++)
		{
			FName FeatureID;
			Archive << FeatureID;
			UnlockedFeatures.Add(FeatureID);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("FTUE data loaded - Stage: %d, NewPlayer: %s"),
		static_cast<int32>(CurrentStage), bIsNewPlayer ? TEXT("Yes") : TEXT("No"));
}

void UMGFTUESubsystem::SaveFTUEData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("FTUE");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("FTUEProgress.sav");

	FBufferArchive Archive;

	int32 Version = 1;
	Archive << Version;

	// Save current stage
	int32 StageInt = static_cast<int32>(CurrentStage);
	Archive << StageInt;

	// Save player flags
	Archive << bIsNewPlayer;
	Archive << bShowHints;

	// Save completed steps
	TArray<int32> CompletedStages;
	for (const FMGOnboardingStep& Step : OnboardingSteps)
	{
		if (Step.bCompleted)
		{
			CompletedStages.Add(static_cast<int32>(Step.Stage));
		}
	}
	int32 CompletedCount = CompletedStages.Num();
	Archive << CompletedCount;
	for (int32 StageVal : CompletedStages)
	{
		Archive << StageVal;
	}

	// Save hint data
	int32 HintCount = Hints.Num();
	Archive << HintCount;
	for (const FMGContextualHint& Hint : Hints)
	{
		FName HintID = Hint.HintID;
		int32 ShowCount = Hint.CurrentShowCount;
		bool bDismissed = Hint.bDismissed;
		Archive << HintID;
		Archive << ShowCount;
		Archive << bDismissed;
	}

	// Save unlocked features
	int32 FeatureCount = UnlockedFeatures.Num();
	Archive << FeatureCount;
	for (const FName& FeatureID : UnlockedFeatures)
	{
		FName Feature = FeatureID;
		Archive << Feature;
	}

	if (FFileHelper::SaveArrayToFile(Archive, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("FTUE data saved - Stage: %d"), static_cast<int32>(CurrentStage));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save FTUE data"));
	}
}

void UMGFTUESubsystem::InitializeOnboardingSteps()
{
	OnboardingSteps.Empty();

	FMGOnboardingStep Welcome;
	Welcome.Stage = EMGOnboardingStage::Welcome;
	Welcome.Title = FText::FromString(TEXT("Welcome to the Streets"));
	Welcome.Description = FText::FromString(TEXT("The underground racing scene awaits"));
	Welcome.Instruction = FText::FromString(TEXT("Press any button to continue"));
	Welcome.RewardGrindCash = 1000;
	OnboardingSteps.Add(Welcome);

	FMGOnboardingStep ChooseCar;
	ChooseCar.Stage = EMGOnboardingStage::ChooseFirstCar;
	ChooseCar.Title = FText::FromString(TEXT("Choose Your Ride"));
	ChooseCar.Description = FText::FromString(TEXT("Pick your first car - don't worry, you can unlock more later"));
	ChooseCar.Instruction = FText::FromString(TEXT("Select a starter vehicle"));
	ChooseCar.RewardGrindCash = 500;
	OnboardingSteps.Add(ChooseCar);

	FMGOnboardingStep FirstRace;
	FirstRace.Stage = EMGOnboardingStage::FirstRace;
	FirstRace.Title = FText::FromString(TEXT("Hit the Streets"));
	FirstRace.Description = FText::FromString(TEXT("Time to prove yourself"));
	FirstRace.Instruction = FText::FromString(TEXT("Complete your first race"));
	FirstRace.RewardGrindCash = 2000;
	FirstRace.UnlockFeature = FName(TEXT("Multiplayer"));
	OnboardingSteps.Add(FirstRace);

	FMGOnboardingStep JoinMP;
	JoinMP.Stage = EMGOnboardingStage::JoinMultiplayer;
	JoinMP.Title = FText::FromString(TEXT("Race Real Opponents"));
	JoinMP.Description = FText::FromString(TEXT("Jump into online racing"));
	JoinMP.Instruction = FText::FromString(TEXT("Complete an online race"));
	JoinMP.bSkippable = true;
	JoinMP.RewardGrindCash = 1500;
	JoinMP.UnlockFeature = FName(TEXT("Customization"));
	OnboardingSteps.Add(JoinMP);

	FMGOnboardingStep Customize;
	Customize.Stage = EMGOnboardingStage::CustomizeCar;
	Customize.Title = FText::FromString(TEXT("Make It Yours"));
	Customize.Description = FText::FromString(TEXT("Customize your ride"));
	Customize.Instruction = FText::FromString(TEXT("Apply any customization to your car"));
	Customize.bSkippable = true;
	Customize.RewardGrindCash = 1000;
	Customize.UnlockFeature = FName(TEXT("Crew"));
	OnboardingSteps.Add(Customize);

	FMGOnboardingStep JoinCrew;
	JoinCrew.Stage = EMGOnboardingStage::JoinOrCreateCrew;
	JoinCrew.Title = FText::FromString(TEXT("Find Your Crew"));
	JoinCrew.Description = FText::FromString(TEXT("Racing is better together"));
	JoinCrew.Instruction = FText::FromString(TEXT("Join or create a crew"));
	JoinCrew.bSkippable = true;
	JoinCrew.RewardGrindCash = 2000;
	JoinCrew.UnlockFeature = FName(TEXT("Tournament"));
	OnboardingSteps.Add(JoinCrew);

	FMGOnboardingStep Challenge;
	Challenge.Stage = EMGOnboardingStage::CompleteChallenge;
	Challenge.Title = FText::FromString(TEXT("Chase the Challenge"));
	Challenge.Description = FText::FromString(TEXT("Complete challenges for bonus rewards"));
	Challenge.Instruction = FText::FromString(TEXT("Complete any daily challenge"));
	Challenge.bSkippable = true;
	Challenge.RewardGrindCash = 1000;
	Challenge.UnlockFeature = FName(TEXT("SeasonPass"));
	OnboardingSteps.Add(Challenge);

	FMGOnboardingStep Season;
	Season.Stage = EMGOnboardingStage::ExploreSeason;
	Season.Title = FText::FromString(TEXT("Season Pass"));
	Season.Description = FText::FromString(TEXT("Check out the season rewards"));
	Season.Instruction = FText::FromString(TEXT("View the Season Pass"));
	Season.bSkippable = true;
	Season.RewardGrindCash = 500;
	OnboardingSteps.Add(Season);
}

void UMGFTUESubsystem::InitializeHints()
{
	Hints.Empty();

	FMGContextualHint DriftHint;
	DriftHint.HintID = FName(TEXT("Hint_Drift"));
	DriftHint.HintText = FText::FromString(TEXT("Hold the handbrake while turning to initiate a drift"));
	DriftHint.TriggerContext = FName(TEXT("CornerApproach"));
	DriftHint.MaxShowCount = 3;
	Hints.Add(DriftHint);

	FMGContextualHint BoostHint;
	BoostHint.HintID = FName(TEXT("Hint_Boost"));
	BoostHint.HintText = FText::FromString(TEXT("Fill your boost meter by drifting and drafting"));
	BoostHint.TriggerContext = FName(TEXT("BoostReady"));
	BoostHint.MaxShowCount = 2;
	Hints.Add(BoostHint);

	FMGContextualHint RivalHint;
	RivalHint.HintID = FName(TEXT("Hint_Rival"));
	RivalHint.HintText = FText::FromString(TEXT("Players you race against repeatedly become rivals"));
	RivalHint.TriggerContext = FName(TEXT("SameOpponentTwice"));
	RivalHint.MaxShowCount = 1;
	Hints.Add(RivalHint);

	FMGContextualHint CrewHint;
	CrewHint.HintID = FName(TEXT("Hint_Crew"));
	CrewHint.HintText = FText::FromString(TEXT("Joining a crew unlocks crew challenges and bonuses"));
	CrewHint.TriggerContext = FName(TEXT("MainMenu"));
	CrewHint.MaxShowCount = 2;
	Hints.Add(CrewHint);
}

void UMGFTUESubsystem::AdvanceStage()
{
	int32 CurrentIndex = (int32)CurrentStage;
	int32 NextIndex = CurrentIndex + 1;

	if (NextIndex >= (int32)EMGOnboardingStage::Completed)
	{
		CurrentStage = EMGOnboardingStage::Completed;
		bIsNewPlayer = false;
		OnOnboardingCompleted.Broadcast();
	}
	else
	{
		CurrentStage = (EMGOnboardingStage)NextIndex;
		OnOnboardingStageChanged.Broadcast(CurrentStage);
	}

	SaveFTUEData();
}

void UMGFTUESubsystem::GrantStepReward(const FMGOnboardingStep& Step)
{
	if (Step.RewardGrindCash > 0)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMGCurrencySubsystem* Currency = GI->GetSubsystem<UMGCurrencySubsystem>())
			{
				Currency->EarnCurrency(EMGCurrencyType::GrindCash, Step.RewardGrindCash, EMGEarnSource::FirstTimeBonus, TEXT("Onboarding reward"));
			}
		}
	}
}
