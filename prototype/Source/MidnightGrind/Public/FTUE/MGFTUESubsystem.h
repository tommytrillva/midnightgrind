// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGFTUESubsystem.h - First-Time User Experience (FTUE) Onboarding System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the FTUE Subsystem, which manages the new player onboarding
 * experience in Midnight Grind. It ensures first-time players are guided through
 * the game's core features in a structured, rewarding way without overwhelming them.
 *
 * Think of this as the "game concierge" - it introduces new players to features
 * one at a time and keeps more advanced features locked until they're ready.
 *
 *
 * KEY CONCEPTS & TERMINOLOGY:
 * ---------------------------
 *
 * 1. ONBOARDING STAGE: A major milestone in the new player journey.
 *    The stages progress in order:
 *    - Welcome: Initial game introduction and setup
 *    - ChooseFirstCar: Selecting a starter vehicle from the initial lineup
 *    - FirstRace: Completing the tutorial/first race
 *    - JoinMultiplayer: Entering an online race for the first time
 *    - CustomizeCar: Using the garage customization features
 *    - JoinOrCreateCrew: Experiencing the social/crew system
 *    - CompleteChallenge: Learning about daily/weekly challenges
 *    - ExploreSeason: Understanding the season pass system
 *    - Completed: All onboarding finished, full game unlocked
 *
 * 2. ONBOARDING STEP: Configuration for displaying one stage's UI.
 *    - Title/Description: What to show the player
 *    - Instruction: Action prompt ("Tap a car to select it")
 *    - TargetWidgetID: Which UI element to spotlight/highlight
 *    - RewardGrindCash: Currency reward for completing this step
 *    - UnlockFeature: What game feature to unlock when completed
 *
 * 3. CONTEXTUAL HINT: Smart tooltips that appear during gameplay.
 *    - Triggered by specific contexts (first garage visit, low nitro, etc.)
 *    - MaxShowCount prevents annoying experienced players
 *    - Can be permanently dismissed by the player
 *
 * 4. PROGRESSIVE UNLOCKING: Features hidden until the right moment.
 *    - New players don't see "Multiplayer" button until they've done basics
 *    - Prevents information overload and confusion
 *    - Use IsFeatureUnlocked() to check before showing UI elements
 *
 * 5. GRINDCASH: In-game currency awarded for completing steps.
 *    - Incentivizes completing the onboarding
 *    - Gives new players starting capital
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 * This is a UGameInstanceSubsystem that:
 * - Persists for the entire game session
 * - Tracks new player status and onboarding progress
 * - Controls feature visibility throughout the UI
 * - Saves/loads progress to local storage
 *
 * Key relationships:
 * - MGTutorialSubsystem: FTUE = WHAT to do; Tutorial = HOW to do it
 *   (FTUE says "Complete a race", Tutorial teaches racing controls)
 * - MGCampaignSubsystem: Campaign missions unlock after FTUE stages
 * - UI System: All menus check IsFeatureUnlocked() before showing options
 * - Economy System: FTUE grants starting GrindCash rewards
 *
 *
 * FTUE vs TUTORIAL - IMPORTANT DISTINCTION:
 * -----------------------------------------
 * - FTUE: HIGH-LEVEL journey stages ("Choose car" -> "First race" -> "Multiplayer")
 *   Controls WHAT features are available and WHEN
 *
 * - Tutorial: DETAILED interactive lessons within a specific feature
 *   Teaches HOW to use accelerate, brake, drift, etc.
 *
 * The FTUE stage "FirstRace" might trigger the "BasicControls" Tutorial sequence.
 *
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // In UI: Check if a button should be visible
 * if (FTUESubsystem->IsFeatureUnlocked("Multiplayer"))
 * {
 *     ShowMultiplayerButton();
 * }
 *
 * // When player completes an onboarding action:
 * FTUESubsystem->CompleteCurrentStep();
 *
 * // For returning players who want to skip:
 * if (PlayerRequestsSkip && !FTUESubsystem->IsOnboardingComplete())
 * {
 *     FTUESubsystem->SkipOnboarding();
 * }
 *
 * // Trigger hints when player encounters new mechanics:
 * FTUESubsystem->TriggerHint("Garage.PaintShop");
 *
 * // Listen for feature unlocks to update UI:
 * FTUESubsystem->OnFeatureUnlocked.AddDynamic(this, &UMainMenu::OnFeatureUnlocked);
 *
 *
 * DETAILED DOCUMENTATION BELOW:
 * -----------------------------
 * The following sections use Doxygen-style comments for API documentation.
 * @see FMGOnboardingStep for step configuration
 * @see FMGContextualHint for hint configuration
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGFTUESubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Stages of the new player onboarding flow
 *
 * These stages represent major milestones in the onboarding experience.
 * Each stage unlocks new features and provides rewards upon completion.
 * Stages are designed to be completed in order, though some may be skipped.
 */
UENUM(BlueprintType)
enum class EMGOnboardingStage : uint8
{
	Welcome,           ///< Initial welcome screen and game introduction
	ChooseFirstCar,    ///< Player selects their starter vehicle
	FirstRace,         ///< Complete the tutorial race
	JoinMultiplayer,   ///< Join an online multiplayer race
	CustomizeCar,      ///< Visit the garage and customize a vehicle
	JoinOrCreateCrew,  ///< Join an existing crew or create a new one
	CompleteChallenge, ///< Complete any daily or weekly challenge
	ExploreSeason,     ///< View the season pass and track progression
	Completed          ///< All onboarding stages finished
};

//=============================================================================
// Data Structures
//=============================================================================

/**
 * @brief Configuration for a single onboarding step
 *
 * Each step in the onboarding flow has associated UI content, rewards,
 * and navigation information. Steps can optionally unlock game features
 * that were previously hidden from new players.
 */
USTRUCT(BlueprintType)
struct FMGOnboardingStep
{
	GENERATED_BODY()

	/// The stage this step belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGOnboardingStage Stage = EMGOnboardingStage::Welcome;

	/// Short title displayed in the onboarding UI (e.g., "Choose Your Ride")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/// Detailed description of what the player will learn/do
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Action instruction shown at the bottom (e.g., "Tap a car to select it")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Instruction;

	/// Whether this step has been completed by the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	/// Whether players can skip this step without completing it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkippable = false;

	/// Widget ID to highlight during this step (for spotlight effects)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetWidgetID;

	/// GrindCash reward granted upon completion (0 = no reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 RewardGrindCash = 0;

	/// Feature ID to unlock upon completion (empty = no unlock)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockFeature;
};

/**
 * @brief Configuration for a contextual help hint
 *
 * Contextual hints are small tooltips that appear when players encounter
 * new mechanics. They help players understand features without requiring
 * them to read extensive documentation.
 *
 * Hints have a maximum show count to prevent annoying experienced players.
 * Players can also permanently dismiss hints they no longer want to see.
 */
USTRUCT(BlueprintType)
struct FMGContextualHint
{
	GENERATED_BODY()

	/// Unique identifier for this hint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HintID;

	/// The help text to display to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText HintText;

	/// Context that triggers this hint (e.g., "Garage.FirstVisit", "Race.LowNitro")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TriggerContext;

	/// Maximum times to show this hint before auto-suppressing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxShowCount = 3;

	/// Number of times this hint has been shown to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentShowCount = 0;

	/// True if the player has manually dismissed this hint permanently
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDismissed = false;
};

//=============================================================================
// Delegate Declarations
//=============================================================================

/// Broadcast when the onboarding advances to a new stage
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnOnboardingStageChanged, EMGOnboardingStage, NewStage);

/// Broadcast when all onboarding stages are completed
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnOnboardingCompleted);

/// Broadcast when a contextual hint should be displayed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHintTriggered, const FMGContextualHint&, Hint);

/// Broadcast when a game feature is unlocked through onboarding
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnFeatureUnlocked, FName, FeatureID);

//=============================================================================
// FTUE Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing the first-time user experience
 *
 * This subsystem tracks onboarding progress, manages contextual hints,
 * and controls progressive feature unlocking. It ensures new players
 * have a guided introduction while allowing experienced players to skip.
 *
 * ## Usage Example (Blueprint)
 * @code
 * // Check if player is new and needs onboarding
 * if (FTUESubsystem->IsNewPlayer() && !FTUESubsystem->IsOnboardingComplete())
 * {
 *     // Show onboarding UI for current step
 *     FMGOnboardingStep CurrentStep = FTUESubsystem->GetCurrentStep();
 *     ShowOnboardingWidget(CurrentStep);
 * }
 *
 * // When player completes an action
 * FTUESubsystem->CompleteCurrentStep();
 * @endcode
 *
 * ## Usage Example (C++)
 * @code
 * if (UMGFTUESubsystem* FTUE = GetGameInstance()->GetSubsystem<UMGFTUESubsystem>())
 * {
 *     // Check if a feature is available
 *     if (FTUE->IsFeatureUnlocked("Multiplayer"))
 *     {
 *         ShowMultiplayerButton();
 *     }
 *
 *     // Trigger a contextual hint
 *     FTUE->TriggerHint("Garage.PaintShop");
 * }
 * @endcode
 *
 * @note Bind to OnFeatureUnlocked to update UI when new features become available
 */
UCLASS()
class MIDNIGHTGRIND_API UMGFTUESubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Lifecycle
	//-------------------------------------------------------------------------

	/// Called when game instance creates this subsystem; loads saved progress
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when game instance shuts down; saves current progress
	virtual void Deinitialize() override;

	//=========================================================================
	// Onboarding Flow
	//=========================================================================

	/**
	 * @brief Returns the current onboarding stage
	 * @return The stage the player is currently on
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE")
	EMGOnboardingStage GetCurrentStage() const { return CurrentStage; }

	/**
	 * @brief Returns configuration for the current onboarding step
	 * @return Step data including title, description, and rewards
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE")
	FMGOnboardingStep GetCurrentStep() const;

	/**
	 * @brief Returns overall onboarding completion percentage
	 * @return Value from 0.0 to 1.0 representing progress
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE")
	float GetOnboardingProgress() const;

	/**
	 * @brief Marks the current onboarding step as completed
	 * @note Grants rewards, unlocks features, and advances to next stage
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE")
	void CompleteCurrentStep();

	/**
	 * @brief Skips the current step without completing it
	 * @note Only works if the current step has bSkippable = true
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE")
	void SkipCurrentStep();

	/**
	 * @brief Skips all remaining onboarding and unlocks all features
	 * @note Use with caution - this skips all tutorials and rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE")
	void SkipOnboarding();

	/**
	 * @brief Checks if all onboarding stages have been completed
	 * @return True if onboarding is finished
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE")
	bool IsOnboardingComplete() const { return CurrentStage == EMGOnboardingStage::Completed; }

	/**
	 * @brief Checks if this is a new player (hasn't completed any onboarding)
	 * @return True if player has never progressed past Welcome stage
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE")
	bool IsNewPlayer() const { return bIsNewPlayer; }

	//=========================================================================
	// Contextual Hints
	//=========================================================================

	/**
	 * @brief Triggers a contextual hint for a specific game context
	 * @param Context The context identifier (e.g., "Garage.FirstVisit")
	 * @note Hint will only show if not dismissed and under MaxShowCount
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void TriggerHint(FName Context);

	/**
	 * @brief Permanently dismisses a specific hint
	 * @param HintID The hint to dismiss
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void DismissHint(FName HintID);

	/**
	 * @brief Dismisses all active hints
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void DismissAllHints();

	/**
	 * @brief Checks if hints are enabled globally
	 * @return True if hints should be displayed
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE|Hints")
	bool ShouldShowHints() const { return bShowHints; }

	/**
	 * @brief Enables or disables all contextual hints
	 * @param bShow True to enable hints, false to suppress all hints
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE|Hints")
	void SetShowHints(bool bShow);

	//=========================================================================
	// Feature Unlocking
	//=========================================================================

	/**
	 * @brief Checks if a game feature has been unlocked
	 * @param FeatureID Identifier for the feature (e.g., "Multiplayer", "Crews")
	 * @return True if the feature is unlocked and accessible
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE|Features")
	bool IsFeatureUnlocked(FName FeatureID) const;

	/**
	 * @brief Returns all features that are still locked
	 * @return Array of feature IDs that haven't been unlocked yet
	 */
	UFUNCTION(BlueprintPure, Category = "FTUE|Features")
	TArray<FName> GetLockedFeatures() const;

	/**
	 * @brief Manually unlocks a feature (for debugging or special cases)
	 * @param FeatureID The feature to unlock
	 * @note Triggers OnFeatureUnlocked event
	 */
	UFUNCTION(BlueprintCallable, Category = "FTUE|Features")
	void UnlockFeature(FName FeatureID);

	//=========================================================================
	// Events
	//=========================================================================

	/// Broadcast when onboarding advances to a new stage
	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnOnboardingStageChanged OnOnboardingStageChanged;

	/// Broadcast when all onboarding is completed
	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnOnboardingCompleted OnOnboardingCompleted;

	/// Broadcast when a contextual hint should be displayed
	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnHintTriggered OnHintTriggered;

	/// Broadcast when a game feature is unlocked
	UPROPERTY(BlueprintAssignable, Category = "FTUE|Events")
	FMGOnFeatureUnlocked OnFeatureUnlocked;

protected:
	//-------------------------------------------------------------------------
	// Internal Methods
	//-------------------------------------------------------------------------

	/// Loads FTUE progress from local storage
	void LoadFTUEData();

	/// Saves current progress to local storage
	void SaveFTUEData();

	/// Initializes the default onboarding step configurations
	void InitializeOnboardingSteps();

	/// Initializes the default contextual hint configurations
	void InitializeHints();

	/// Advances to the next onboarding stage
	void AdvanceStage();

	/// Grants rewards (GrindCash, unlocks) for a completed step
	void GrantStepReward(const FMGOnboardingStep& Step);

private:
	//-------------------------------------------------------------------------
	// Internal State
	//-------------------------------------------------------------------------

	/// Current onboarding stage
	EMGOnboardingStage CurrentStage = EMGOnboardingStage::Welcome;

	/// Configuration for all onboarding steps
	UPROPERTY()
	TArray<FMGOnboardingStep> OnboardingSteps;

	/// Configuration for all contextual hints
	UPROPERTY()
	TArray<FMGContextualHint> Hints;

	/// Set of feature IDs that have been unlocked
	TSet<FName> UnlockedFeatures;

	/// True if this is a brand new player
	bool bIsNewPlayer = true;

	/// Global toggle for showing contextual hints
	bool bShowHints = true;
};
