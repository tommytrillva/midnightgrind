// Copyright Midnight Grind. All Rights Reserved.

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
