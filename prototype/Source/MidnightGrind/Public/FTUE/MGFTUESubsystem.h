// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGFTUESubsystem.generated.h"

/**
 * First-Time User Experience System
 * - Guides new players through the game
 * - Progressive feature unlocking
 * - Contextual hints and tutorials
 * - Tracks onboarding completion
 */

UENUM(BlueprintType)
enum class EMGOnboardingStage : uint8
{
	Welcome,
	ChooseFirstCar,
	FirstRace,
	JoinMultiplayer,
	CustomizeCar,
	JoinOrCreateCrew,
	CompleteChallenge,
	ExploreSeason,
	Completed
};

USTRUCT(BlueprintType)
struct FMGOnboardingStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGOnboardingStage Stage = EMGOnboardingStage::Welcome;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Instruction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkippable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetWidgetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 RewardGrindCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockFeature;
};

USTRUCT(BlueprintType)
struct FMGContextualHint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HintID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText HintText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TriggerContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxShowCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentShowCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDismissed = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnOnboardingStageChanged, EMGOnboardingStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnOnboardingCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHintTriggered, const FMGContextualHint&, Hint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnFeatureUnlocked, FName, FeatureID);

UCLASS()
class MIDNIGHTGRIND_API UMGFTUESubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Onboarding Flow
	UFUNCTION(BlueprintPure, Category = "FTUE")
	EMGOnboardingStage GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "FTUE")
	FMGOnboardingStep GetCurrentStep() const;

	UFUNCTION(BlueprintPure, Category = "FTUE")
	float GetOnboardingProgress() const;

	UFUNCTION(BlueprintCallable, Category = "FTUE")
	void CompleteCurrentStep();

	UFUNCTION(BlueprintCallable, Category = "FTUE")
	void SkipCurrentStep();

	UFUNCTION(BlueprintCallable, Category = "FTUE")
	void SkipOnboarding();

	UFUNCTION(BlueprintPure, Category = "FTUE")
	bool IsOnboardingComplete() const { return CurrentStage == EMGOnboardingStage::Completed; }

	UFUNCTION(BlueprintPure, Category = "FTUE")
	bool IsNewPlayer() const { return bIsNewPlayer; }

	// Contextual Hints
	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void TriggerHint(FName Context);

	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void DismissHint(FName HintID);

	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void DismissAllHints();

	UFUNCTION(BlueprintPure, Category = "FTUE|Hints")
	bool ShouldShowHints() const { return bShowHints; }

	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void SetShowHints(bool bShow);

	// Feature Unlocking
	UFUNCTION(BlueprintPure, Category = "FTUE|Features")
	bool IsFeatureUnlocked(FName FeatureID) const;

	UFUNCTION(BlueprintPure, Category = "FTUE|Features")
	TArray<FName> GetLockedFeatures() const;

	UFUNCTION(BlueprintCallable, Category = "FTUE|Features")
	void UnlockFeature(FName FeatureID);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnOnboardingStageChanged OnOnboardingStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnOnboardingCompleted OnOnboardingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnHintTriggered OnHintTriggered;

	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnFeatureUnlocked OnFeatureUnlocked;

protected:
	void LoadFTUEData();
	void SaveFTUEData();
	void InitializeOnboardingSteps();
	void InitializeHints();
	void AdvanceStage();
	void GrantStepReward(const FMGOnboardingStep& Step);

private:
	EMGOnboardingStage CurrentStage = EMGOnboardingStage::Welcome;

	UPROPERTY()
	TArray<FMGOnboardingStep> OnboardingSteps;

	UPROPERTY()
	TArray<FMGContextualHint> Hints;

	TSet<FName> UnlockedFeatures;
	bool bIsNewPlayer = true;
	bool bShowHints = true;
};
