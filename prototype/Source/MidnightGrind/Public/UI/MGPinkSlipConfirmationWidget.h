// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGPinkSlipConfirmationWidget.h
 * @brief Triple-confirmation dialog for high-stakes pink slip races
 *
 * =============================================================================
 * @section Overview
 * This file defines the base widget class for pink slip race confirmations.
 * Pink slip races are winner-takes-all wagers where the loser permanently
 * loses their vehicle. Due to the severe consequences, this widget implements
 * a mandatory triple-confirmation process with increasingly stern warnings.
 *
 * The widget is designed to be extended in Blueprint for visual presentation
 * while this base class handles all confirmation logic and flow.
 *
 * =============================================================================
 * @section KeyConcepts Key Concepts
 *
 * - **Triple Confirmation**: Three separate "Are you sure?" dialogs must be
 *   acknowledged before a pink slip race can begin. This prevents accidental
 *   wagers and ensures the player understands the stakes.
 *
 * - **Escalating Warnings**: Each confirmation step shows progressively more
 *   urgent warnings. The final confirmation requires extra acknowledgment.
 *
 * - **Blueprint Extension**: The C++ base handles logic; Blueprint subclasses
 *   implement the visual design and animations.
 *
 * - **Permanent Consequences**: If the player loses, the vehicle is permanently
 *   transferred to the opponent. There is no undo or buyback.
 *
 * =============================================================================
 * @section Architecture
 *
 *   [Pink Slip Challenge] ---> [UMGPinkSlipSubsystem]
 *                                      |
 *                                      v
 *                              [UMGPinkSlipConfirmationWidget]
 *                                      |
 *                                      +-- Step 1: "You will wager your vehicle"
 *                                      |
 *                                      +-- Step 2: "If you lose, it's gone forever"
 *                                      |
 *                                      +-- Step 3: "FINAL WARNING - No undo!"
 *                                      |
 *                                      v
 *                              [Race Begins or Cancel]
 *
 * =============================================================================
 * @section Usage
 * @code
 * // This widget is typically shown by the PinkSlipSubsystem, not directly
 * // Blueprint designers should extend this class and implement visuals
 *
 * // In Blueprint, bind to confirmation data:
 * // - GetConfirmationData() returns current FMGPinkSlipConfirmationData
 * // - IsFinalConfirmation() returns true on step 3
 * // - GetFormattedTotalValue() returns formatted currency string
 * // - GetStepText() returns "1 of 3", "2 of 3", etc.
 *
 * // When player clicks confirm button:
 * OnConfirmClicked(); // Base class handles progression to next step
 *
 * // When player clicks cancel button:
 * OnCancelClicked(); // Base class handles cleanup
 *
 * // Blueprint event for updating visuals:
 * UFUNCTION(BlueprintImplementableEvent)
 * void OnConfirmationDataSet(); // Override to update UI elements
 * @endcode
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PinkSlip/MGPinkSlipSubsystem.h"
#include "MGPinkSlipConfirmationWidget.generated.h"

/**
 * @class UMGPinkSlipConfirmationWidget
 * @brief Base class for pink slip confirmation dialogs
 *
 * This widget handles the mandatory triple-confirmation process
 * required before a pink slip race can begin. It presents
 * increasingly stern warnings to ensure the player understands
 * the permanence of the wager.
 *
 * UI designers should extend this class in Blueprint and implement
 * the visual presentation. The base class handles all logic.
 *
 * Flow:
 * 1. ShowConfirmation() called with FMGPinkSlipConfirmationData
 * 2. Player sees warning, vehicle info, and stakes
 * 3. Player clicks Confirm or Cancel
 * 4. OnConfirm/OnCancel called, broadcasts result
 * 5. If more confirmations needed, next dialog shows automatically
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPinkSlipConfirmationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * @brief Display the confirmation dialog
	 * @param ConfirmData All data needed for the confirmation step
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip")
	void ShowConfirmation(const FMGPinkSlipConfirmationData& ConfirmData);

	/**
	 * @brief Called when player confirms (clicks Yes/Confirm)
	 *
	 * Subclasses can override to add effects. Must call Super.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pink Slip")
	void OnConfirmClicked();

	/**
	 * @brief Called when player cancels (clicks No/Cancel)
	 *
	 * Subclasses can override to add effects. Must call Super.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pink Slip")
	void OnCancelClicked();

	/**
	 * @brief Get the current confirmation data
	 * @return Current confirmation data being displayed
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	const FMGPinkSlipConfirmationData& GetConfirmationData() const { return CurrentConfirmData; }

	/**
	 * @brief Check if this is the final confirmation
	 * @return true if confirming will start the race
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	bool IsFinalConfirmation() const { return CurrentConfirmData.bIsFinalConfirmation; }

protected:
	/**
	 * @brief Called when confirmation data is set
	 *
	 * Override in Blueprint to update visual elements.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Pink Slip")
	void OnConfirmationDataSet();

	/**
	 * @brief Get formatted text for total value at stake
	 * @return Formatted currency string
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	FText GetFormattedTotalValue() const;

	/**
	 * @brief Get confirmation step text (e.g., "1 of 3")
	 * @return Step indicator text
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	FText GetStepText() const;

	/**
	 * @brief Get confirm button text based on step
	 * @return Localized button text
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	FText GetConfirmButtonText() const;

	/**
	 * @brief Get cancel button text
	 * @return Localized button text
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	FText GetCancelButtonText() const;

	/** Current confirmation data */
	UPROPERTY(BlueprintReadOnly, Category = "Pink Slip")
	FMGPinkSlipConfirmationData CurrentConfirmData;

private:
	/** Get pink slip subsystem */
	UMGPinkSlipSubsystem* GetPinkSlipSubsystem() const;
};
