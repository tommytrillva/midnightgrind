// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameModes/MGRaceGameMode.h"
#include "MGRaceResultsWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UButton;
class UCanvasPanel;

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

	/** Display race results */
	UFUNCTION(BlueprintCallable, Category = "Results")
	void DisplayResults(const FMGRaceResults& Results);

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
	// STATE
	// ==========================================

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	FMGRaceResults CachedResults;

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	TArray<FMGResultRowData> ResultRows;

	UPROPERTY(BlueprintReadOnly, Category = "Results|State")
	int32 CurrentSelection = 0;

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
};
