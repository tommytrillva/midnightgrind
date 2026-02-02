// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// Updated Stage 51: Race Flow Integration

/**
 * =============================================================================
 * @file MGRaceResultsWidget.h
 * @brief Race results screen widget displaying final standings, times, and rewards
 *
 * =============================================================================
 * @section Overview
 * This file defines the race results widget that appears after a race ends.
 * It displays the final standings, lap times, rewards earned, and provides
 * navigation options for the player. The widget integrates with both the
 * legacy RaceGameMode results and the newer RaceFlowSubsystem for MVP races.
 *
 * Key features:
 * - Final position and standings display
 * - Time and gap calculations for all racers
 * - Reward breakdown (cash, reputation, XP)
 * - Personal best and track record tracking
 * - Win streak and career statistics display
 * - Pink slip race results (vehicle won/lost)
 * - Animated reveal of results rows
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **FMGResultRowData**: Represents one row in the results table. Contains
 *   position, driver name, vehicle, times, and special flags (player highlight,
 *   DNF status, best lap indicator).
 *
 * - **Dual Data Sources**: The widget can receive data from either:
 *   1. FMGRaceResults (from MGRaceGameMode) - Legacy race system
 *   2. FMGRaceFlowResult (from MGRaceFlowSubsystem) - MVP race flow system
 *   Use DisplayResults() for legacy, DisplayFlowResults() for MVP.
 *
 * - **History Integration**: Connects with MGRaceHistorySubsystem to show
 *   personal bests, win streaks, and track-specific statistics.
 *
 * - **Pink Slip Races**: Special high-stakes races where vehicles are wagered.
 *   The widget shows if the player won or lost a vehicle.
 *
 * - **Animated Row Reveal**: Results are revealed one row at a time using
 *   PlayRowRevealAnimation() for dramatic effect.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Race Ends]
 *        |
 *        +-- RaceGameMode.OnRaceFinished --> DisplayResults(FMGRaceResults)
 *        |
 *        +-- RaceFlowSubsystem.OnRaceComplete --> DisplayFlowResults(FMGRaceFlowResult)
 *        |
 *        v
 *   [MGRaceResultsWidget]
 *        |
 *        +-- ProcessResultsData() --> Creates FMGResultRowData array
 *        |
 *        +-- CreateUIElements() --> Builds result rows
 *        |
 *        +-- UpdateHistoryStatsDisplay() --> Shows PB, streak, career
 *        |
 *        +-- PlayVictoryAnimation() or PlayDefeatAnimation()
 *        |
 *        v
 *   [User Actions]
 *        |
 *        +-- OnContinue --> ContinueToGarage()
 *        +-- OnRestart --> RestartRace()
 *        +-- OnQuit --> HandleQuit()
 *
 * =============================================================================
 * @section Usage
 * @code
 * // Create and display results widget (typically done by RaceFlowSubsystem)
 * UMGRaceResultsWidget* ResultsWidget = CreateWidget<UMGRaceResultsWidget>(
 *     GetWorld(), ResultsWidgetClass);
 * ResultsWidget->AddToViewport();
 *
 * // Method 1: Display from RaceFlowSubsystem automatically
 * ResultsWidget->DisplayFromFlowSubsystem();
 *
 * // Method 2: Display with specific flow result data
 * FMGRaceFlowResult FlowResult;
 * FlowResult.Position = 1;
 * FlowResult.CashEarned = 15000;
 * FlowResult.ReputationEarned = 500;
 * FlowResult.XPEarned = 250;
 * ResultsWidget->DisplayFlowResults(FlowResult);
 *
 * // Method 3: Display with legacy race results
 * FMGRaceResults LegacyResults;
 * LegacyResults.bPlayerWon = true;
 * LegacyResults.FinalStandings = StandingsArray;
 * ResultsWidget->DisplayResults(LegacyResults);
 *
 * // Subscribe to navigation events
 * ResultsWidget->OnContinue.AddDynamic(this, &AMyController::HandleResultsContinue);
 * ResultsWidget->OnRestart.AddDynamic(this, &AMyController::HandleResultsRestart);
 *
 * // Query result data
 * TArray<FMGResultRowData> Rows = ResultsWidget->GetResultRows();
 * bool bWon = ResultsWidget->DidPlayerWin();
 * bool bNewPB = ResultsWidget->IsNewPersonalBest();
 * int32 Streak = ResultsWidget->GetCurrentWinStreak();
 *
 * // Check pink slip results
 * if (ResultsWidget->WonPinkSlipVehicle())
 * {
 *     FText VehicleName = ResultsWidget->GetPinkSlipVehicleText();
 *     // Show "You won: [VehicleName]!"
 * }
 * @endcode
 *
 * =============================================================================
 * @section BlueprintIntegration Blueprint Integration
 *
 * For Blueprint designers creating the visual layout:
 *
 * 1. Create a Blueprint widget inheriting from MGRaceResultsWidget
 * 2. Add the following widgets with matching names (BindWidgetOptional):
 *    - RootCanvas: Main canvas panel
 *    - HeaderText: "RACE RESULTS" or "YOU WIN!"
 *    - SubHeaderText: Track name or additional info
 *    - ResultsListBox: Vertical box for result rows
 *    - CreditsText, ReputationText, BestLapText: Reward displays
 *    - PersonalBestText, WinStreakText, CareerStatsText: History stats
 *    - PromptText: "Press A to Continue" etc.
 *
 * 3. Implement BlueprintImplementableEvents:
 *    - OnResultsReady: Called when data is loaded
 *    - PlayVictoryAnimation: Celebration effects for wins
 *    - PlayDefeatAnimation: Consolation effects for losses
 *    - PlayRowRevealAnimation(int32 RowIndex): Per-row reveal
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/MGRaceGameMode.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "Race/MGRaceHistorySubsystem.h"
#include "MGRaceResultsWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UButton;
class UCanvasPanel;
class UMGRaceFlowSubsystem;
class UMGRaceHistorySubsystem;

/**
 * Individual racer result row
 */
USTRUCT(BlueprintType)
struct FMGResultRowData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(BlueprintReadWrite)
	FText DriverName;

	UPROPERTY(BlueprintReadWrite)
	FText VehicleName;

	UPROPERTY(BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float BestLap = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float GapToWinner = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	bool bIsPlayer = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsDNF = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasBestLap = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResultsContinue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResultsRestart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResultsQuit);

/**
 * Race Results Screen Widget
 * Displays final standings, times, and rewards after race completion
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceResultsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ==========================================
	// DISPLAY
	// ==========================================

	/** Display race results (from RaceGameMode) */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void DisplayResults(const FMGRaceResults& Results);

	/** Display results from RaceFlowSubsystem (MVP integration) */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void DisplayFlowResults(const FMGRaceFlowResult& FlowResult);

	/** Auto-populate from RaceFlowSubsystem's last result */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void DisplayFromFlowSubsystem();

	/** Show with animation */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void ShowResults();

	/** Hide results */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void HideResults();

	/** Get processed result rows for display */
	UFUNCTION(BlueprintPure, Category = "Results")
	TArray<FMGResultRowData> GetResultRows() const { return ResultRows; }

	// ==========================================
	// FLOW SUBSYSTEM INTEGRATION
	// ==========================================

	/** Continue to garage via flow subsystem */
	UFUNCTION(BlueprintCallable, Category = "Results|Flow")
	void ContinueToGarage();

	/** Restart race via flow subsystem */
	UFUNCTION(BlueprintCallable, Category = "Results|Flow")
	void RestartRace();

	/** Get cached flow result */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	FMGRaceFlowResult GetFlowResult() const { return CachedFlowResult; }

	/** Get cash earned from flow result */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	int64 GetCashEarned() const { return CachedFlowResult.CashEarned; }

	/** Get reputation earned from flow result */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	int32 GetRepEarned() const { return CachedFlowResult.ReputationEarned; }

	/** Get XP earned from flow result */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	int32 GetXPFromFlow() const { return CachedFlowResult.XPEarned; }

	/** Did player win pink slip vehicle? */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	bool WonPinkSlipVehicle() const { return !CachedFlowResult.PinkSlipWonVehicleID.IsNone(); }

	/** Did player lose pink slip vehicle? */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	bool LostPinkSlipVehicle() const { return !CachedFlowResult.PinkSlipLostVehicleID.IsNone(); }

	/** Get pink slip vehicle name won/lost */
	UFUNCTION(BlueprintPure, Category = "Results|Flow")
	FText GetPinkSlipVehicleText() const;

	// ==========================================
	// REWARDS
	// ==========================================

	/** Get credits earned text */
	UFUNCTION(BlueprintPure, Category = "Results")
	FText GetCreditsEarnedText() const;

	/** Get reputation earned text */
	UFUNCTION(BlueprintPure, Category = "Results")
	FText GetReputationEarnedText() const;

	/** Get XP earned text */
	UFUNCTION(BlueprintPure, Category = "Results")
	FText GetXPEarnedText() const;

	/** Check if player won */
	UFUNCTION(BlueprintPure, Category = "Results")
	bool DidPlayerWin() const { return CachedResults.bPlayerWon; }

	// ==========================================
	// HISTORY STATS
	// ==========================================

	/** Get formatted win streak text */
	UFUNCTION(BlueprintPure, Category = "Results|History")
	FText GetWinStreakText() const;

	/** Get track personal best comparison text */
	UFUNCTION(BlueprintPure, Category = "Results|History")
	FText GetPersonalBestText() const;

	/** Get career stats text (wins/races) */
	UFUNCTION(BlueprintPure, Category = "Results|History")
	FText GetCareerStatsText() const;

	/** Was this a new personal best? */
	UFUNCTION(BlueprintPure, Category = "Results|History")
	bool IsNewPersonalBest() const { return bIsNewPB; }

	/** Get current win streak */
	UFUNCTION(BlueprintPure, Category = "Results|History")
	int32 GetCurrentWinStreak() const;

	/** Get track win rate text */
	UFUNCTION(BlueprintPure, Category = "Results|History")
	FText GetTrackWinRateText() const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Results|Events")
	FOnResultsContinue OnContinue;

	UPROPERTY(BlueprintAssignable, Category = "Results|Events")
	FOnResultsRestart OnRestart;

	UPROPERTY(BlueprintAssignable, Category = "Results|Events")
	FOnResultsQuit OnQuit;

protected:
	// ==========================================
	// BLUEPRINT EVENTS
	// ==========================================

	/** Called when results are ready to display */
	UFUNCTION(BlueprintImplementableEvent, Category = "Results|Events")
	void OnResultsReady();

	/** Play victory animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Results|Animation")
	void PlayVictoryAnimation();

	/** Play defeat animation */
	UFUNCTION(BlueprintImplementableEvent, Category = "Results|Animation")
	void PlayDefeatAnimation();

	/** Play reveal animation for row */
	UFUNCTION(BlueprintImplementableEvent, Category = "Results|Animation")
	void PlayRowRevealAnimation(int32 RowIndex);

	// ==========================================
	// UI ELEMENTS
	// ==========================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCanvasPanel* RootCanvas;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* HeaderText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* SubHeaderText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UVerticalBox* ResultsListBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* CreditsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* ReputationText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* BestLapText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* PromptText;

	// ==========================================
	// HISTORY STATS UI
	// ==========================================

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* PersonalBestText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* WinStreakText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* CareerStatsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TrackRecordText;

	// ==========================================
	// STATE
	// ==========================================

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	FMGRaceResults CachedResults;

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	FMGRaceFlowResult CachedFlowResult;

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	TArray<FMGResultRowData> ResultRows;

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	int32 CurrentSelection = 0;

	/** Cached flow subsystem reference */
	UPROPERTY()
	TWeakObjectPtr<UMGRaceFlowSubsystem> RaceFlowSubsystem;

	/** Cached race history subsystem reference */
	UPROPERTY()
	TWeakObjectPtr<UMGRaceHistorySubsystem> RaceHistorySubsystem;

	/** Was this race a new personal best? */
	bool bIsNewPB = false;

	/** Cached track stats for display */
	FMGTrackStats CachedTrackStats;

	/** Cached lifetime stats for display */
	FMGLifetimeStats CachedLifetimeStats;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor PlayerHighlightColor = FSlateColor(FLinearColor(0.0f, 1.0f, 0.9f, 1.0f));

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor WinnerColor = FSlateColor(FLinearColor(1.0f, 0.843f, 0.0f, 1.0f));

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FSlateColor DNFColor = FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));

private:
	/** Process race results into display rows */
	void ProcessResultsData(const FMGRaceResults& Results);

	/** Create UI elements programmatically */
	void CreateUIElements();

	/** Create a result row widget */
	UWidget* CreateResultRow(const FMGResultRowData& RowData);

	/** Format time string */
	FText FormatTime(float Seconds) const;

	/** Format gap string */
	FText FormatGap(float Seconds) const;

	/** Handle continue action */
	UFUNCTION()
	void HandleContinue();

	/** Handle restart action */
	UFUNCTION()
	void HandleRestart();

	/** Handle quit action */
	UFUNCTION()
	void HandleQuit();

	/** Row reveal timer */
	FTimerHandle RowRevealTimerHandle;
	int32 CurrentRevealRow = 0;
	void RevealNextRow();

	/** Update history stats display */
	void UpdateHistoryStatsDisplay(const FString& TrackId, float PlayerTime);

	/** Create history stats UI elements */
	void CreateHistoryStatsUI();

	/** Get race history subsystem */
	UMGRaceHistorySubsystem* GetHistorySubsystem();
};
