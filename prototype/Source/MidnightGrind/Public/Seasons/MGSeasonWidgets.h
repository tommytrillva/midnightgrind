// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSeasonWidgets.h
 * Season UI Widgets - Visual Components for Season Pass and Events
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file contains all the UI widget classes used to display season pass
 * progression, live events, and challenge information to players. These widgets
 * connect to UMGSeasonSubsystem to show real-time data and handle user
 * interactions like claiming rewards and joining events.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. UNREAL UI SYSTEM (UMG - Unreal Motion Graphics)
 *    - UUserWidget: Base class for all UI elements in Unreal
 *    - Blueprintable widgets: C++ defines structure, Blueprint handles visuals
 *    - Widget binding: Connect C++ variables to UI elements via BindWidget
 *    - NativeConstruct/Destruct: Called when widget is created/destroyed
 *
 * 2. ABSTRACT CLASSES
 *    All widgets here are "Abstract" meaning they MUST be subclassed in
 *    Blueprint to be used. This allows designers to customize appearance
 *    while programmers define behavior. Example:
 *    - C++ defines: "UMGSeasonRewardWidget has RewardData and UpdateDisplay()"
 *    - Blueprint implements: "UpdateDisplay sets icon, text, shows/hides elements"
 *
 * 3. BLUEPRINT NATIVE EVENTS
 *    Functions marked "BlueprintNativeEvent" have:
 *    - C++ implementation (the _Implementation suffix version)
 *    - Blueprint can override to add custom behavior
 *    - Example: UpdateDisplay() is called from C++, but visuals set in Blueprint
 *
 * WIDGET CLASSES EXPLAINED:
 * -------------------------
 *
 * UMGSeasonRewardWidget
 * ---------------------
 * Displays a single reward item in the season pass grid.
 * - Shows icon, name, rarity color
 * - Shows locked/unlocked/claimed states
 * - Has "Claim" button for available rewards
 * - Fires OnClaimed delegate when player collects reward
 *
 * UMGSeasonProgressWidget
 * -----------------------
 * Shows the player's current tier progress bar.
 * - Displays current tier number
 * - Shows XP progress toward next tier
 * - Animates when XP is gained
 * - Shows tier-up celebration when leveling
 * - Auto-updates by listening to subsystem delegates
 *
 * UMGSeasonPassWidget
 * -------------------
 * The main season pass overview screen.
 * - Displays all 100 tiers in a scrollable grid
 * - Shows free track and premium track rewards
 * - Highlights current tier position
 * - Has "Claim All" button for convenience
 * - Shows premium upgrade purchase option
 *
 * UMGEventObjectiveWidget
 * -----------------------
 * Shows a single objective within an event.
 * - Displays description (e.g., "Win 5 races")
 * - Shows progress bar (3/5 completed)
 * - Checkmark when completed
 *
 * UMGEventCardWidget
 * ------------------
 * A clickable card representing one live event.
 * - Shows event name, icon, time remaining
 * - Displays completion percentage
 * - Click to open event details
 * - Fires OnSelected delegate when clicked
 *
 * UMGEventDetailWidget
 * --------------------
 * Full-screen event details view.
 * - Shows event description and theme
 * - Lists all objectives with progress
 * - Displays available rewards
 * - Has "Join Event" button to start participating
 *
 * UMGEventsHubWidget
 * ------------------
 * Main events browser with tabs.
 * - Active tab: Currently running events
 * - Upcoming tab: Events starting soon
 * - Completed tab: Past events (for history)
 * - Creates event cards dynamically
 * - Opens detail widget when card is selected
 *
 * UMGChallengesWidget
 * -------------------
 * Shows daily and weekly challenge lists.
 * - Displays countdown timers until reset
 * - Groups challenges by daily/weekly
 * - Updates progress in real-time
 * - Ticks every frame to update timers
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *     +------------------+
 *     | UMGSeasonSubsystem|  (Data source)
 *     +------------------+
 *              |
 *              | Delegates & Data
 *              v
 *     +------------------+
 *     | UI Widgets       |  <-- This file
 *     | (Season/Events)  |
 *     +------------------+
 *              |
 *              | User input
 *              v
 *     +------------------+
 *     | Player Actions   |
 *     | (Claim, Join)    |
 *     +------------------+
 *
 * TYPICAL USAGE FLOW (Season Pass Screen):
 * ----------------------------------------
 * 1. UMGSeasonPassWidget::NativeConstruct() called when menu opens
 * 2. Widget gets UMGSeasonSubsystem reference from GameInstance
 * 3. Widget subscribes to OnSeasonChanged, OnTierUp delegates
 * 4. UpdateDisplay() creates reward widgets for each tier
 * 5. Player scrolls to current tier (ScrollToCurrentTier())
 * 6. Player clicks reward -> OnRewardClaimed fires
 * 7. Widget calls subsystem->ClaimTierReward()
 * 8. Widget refreshes to show claimed state
 *
 * IMPORTANT PATTERNS:
 * -------------------
 * - Always get subsystem reference in NativeConstruct(), clear in NativeDestruct()
 * - Bind to delegates in Construct, unbind in Destruct (prevent crashes)
 * - Use UPROPERTY() on all widget references to prevent garbage collection
 * - BlueprintReadOnly properties are set in C++, displayed in Blueprint
 * - EditDefaultsOnly properties are set in Blueprint defaults (like widget classes)
 *
 * @see UMGSeasonSubsystem - The data source for all these widgets
 * @see UUserWidget - Unreal's base UI widget class
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MGSeasonSubsystem.h"
#include "MGSeasonWidgets.generated.h"

class UTextBlock;
class UProgressBar;
class UImage;
class UButton;
class UScrollBox;

/**
 * Season tier reward widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSeasonRewardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRewardClaimed, int32, Tier, bool, bPremium);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardClaimed OnClaimed;

	/** Set reward data */
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void SetRewardData(const FMGSeasonReward& Reward, bool bIsUnlocked, bool bCanClaim);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGSeasonReward RewardData;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bCanClaim = false;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Claim reward */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ClaimReward();
};

/**
 * Season progress bar widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSeasonProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Update progress display */
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void UpdateProgress();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 CurrentTier = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 CurrentXP = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	int32 XPRequired = 1000;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	float TierProgress = 0.0f;

	UPROPERTY()
	UMGSeasonSubsystem* SeasonSubsystem;

	/** Handle XP gained */
	UFUNCTION()
	void OnXPGained(int32 XPGained, int32 TotalXP);

	/** Handle tier up */
	UFUNCTION()
	void OnTierUp(int32 NewTier, const TArray<FMGSeasonReward>& Rewards);

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Play XP gain animation */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void PlayXPGainAnimation(int32 Amount);

	/** Play tier up animation */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void PlayTierUpAnimation(int32 NewTier);
};

/**
 * Season pass overview widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSeasonPassWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Scroll to current tier */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ScrollToCurrentTier();

	/** Claim all available rewards */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void ClaimAllRewards();

	/** Purchase premium pass */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	void PurchasePremiumPass();

protected:
	/** Reward widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGSeasonRewardWidget> RewardWidgetClass;

	/** Reward widgets */
	UPROPERTY()
	TArray<UMGSeasonRewardWidget*> RewardWidgets;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGSeasonData SeasonData;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGSeasonProgress Progress;

	UPROPERTY()
	UMGSeasonSubsystem* SeasonSubsystem;

	/** Handle season change */
	UFUNCTION()
	void OnSeasonChanged(const FMGSeasonData& NewSeason);

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Update tier display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateTierDisplay(int32 Tier);

	/** Show premium pass purchase */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void ShowPremiumPurchasePrompt();

	/** Handle reward claimed */
	UFUNCTION()
	void OnRewardClaimed(int32 Tier, bool bPremium);
};

/**
 * Event objective widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGEventObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set objective data */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveData(const FMGEventObjective& Objective);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGEventObjective ObjectiveData;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();
};

/**
 * Event card widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGEventCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventSelected, const FMGEventData&, Event);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventSelected OnSelected;

	/** Set event data */
	UFUNCTION(BlueprintCallable, Category = "Event")
	void SetEventData(const FMGEventData& Event);

	/** Get event data */
	UFUNCTION(BlueprintPure, Category = "Event")
	FMGEventData GetEventData() const { return EventData; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGEventData EventData;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FTimespan TimeRemaining;

	/** Objective widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGEventObjectiveWidget> ObjectiveWidgetClass;

	/** Objective widgets */
	UPROPERTY()
	TArray<UMGEventObjectiveWidget*> ObjectiveWidgets;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Update objectives display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateObjectivesDisplay();

	/** Handle click */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void HandleClick();

	/** Update time remaining */
	void UpdateTimeRemaining();
};

/**
 * Event detail widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGEventDetailWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Show event details */
	UFUNCTION(BlueprintCallable, Category = "Event")
	void ShowEvent(const FMGEventData& Event);

	/** Join event */
	UFUNCTION(BlueprintCallable, Category = "Event")
	void JoinEvent();

	/** Close */
	UFUNCTION(BlueprintCallable, Category = "Event")
	void Close();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FMGEventData EventData;

	UPROPERTY()
	UMGSeasonSubsystem* SeasonSubsystem;

	/** Objective widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGEventObjectiveWidget> ObjectiveWidgetClass;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Handle objective progress */
	UFUNCTION()
	void OnObjectiveProgress(const FMGEventData& Event, const FMGEventObjective& Objective);

	/** Handle event completed */
	UFUNCTION()
	void OnEventCompleted(const FMGEventData& Event);
};

/**
 * Events hub widget
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGEventsHubWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Refresh events */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void RefreshEvents();

	/** Show active tab */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ShowActiveTab();

	/** Show upcoming tab */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ShowUpcomingTab();

	/** Show completed tab */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ShowCompletedTab();

protected:
	/** Current tab */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 CurrentTab = 0;

	/** Event card widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGEventCardWidget> EventCardWidgetClass;

	/** Event cards */
	UPROPERTY()
	TArray<UMGEventCardWidget*> EventCards;

	/** Event detail widget */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Widgets")
	UMGEventDetailWidget* EventDetailWidget;

	UPROPERTY()
	UMGSeasonSubsystem* SeasonSubsystem;

	/** Handle event started */
	UFUNCTION()
	void OnEventStarted(const FMGEventData& Event);

	/** Handle event ended */
	UFUNCTION()
	void OnEventEnded(const FMGEventData& Event);

	/** Update events display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateEventsDisplay(const TArray<FMGEventData>& Events);

	/** Handle event card selected */
	UFUNCTION()
	void OnEventCardSelected(const FMGEventData& Event);

	/** Create event card widget */
	UMGEventCardWidget* CreateEventCardWidget();
};

/**
 * Challenge widget (daily/weekly)
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGChallengesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Refresh challenges */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void RefreshChallenges();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FMGEventObjective> DailyChallenges;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	TArray<FMGEventObjective> WeeklyChallenges;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FTimespan DailyResetTime;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FTimespan WeeklyResetTime;

	UPROPERTY()
	UMGSeasonSubsystem* SeasonSubsystem;

	/** Objective widget class */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGEventObjectiveWidget> ObjectiveWidgetClass;

	/** Update display */
	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	void UpdateDisplay();

	/** Update timers */
	void UpdateTimers();
};
