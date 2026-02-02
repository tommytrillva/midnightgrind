// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGPinkSlipConfirmationWidget.h"
#include "PinkSlip/MGPinkSlipSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

void UMGPinkSlipConfirmationWidget::ShowConfirmation(const FMGPinkSlipConfirmationData& ConfirmData)
{
	CurrentConfirmData = ConfirmData;

	// Notify Blueprint implementation
	OnConfirmationDataSet();

	// Add to viewport if not already
	if (!IsInViewport())
	{
		AddToViewport(100); // High Z-order for confirmation dialogs
	}
}

void UMGPinkSlipConfirmationWidget::OnConfirmClicked_Implementation()
{
	UMGPinkSlipSubsystem* PinkSlipSubsystem = GetPinkSlipSubsystem();
	if (PinkSlipSubsystem)
	{
		// Submit positive confirmation
		PinkSlipSubsystem->SubmitConfirmation(true);

		// Check if more confirmations needed
		if (PinkSlipSubsystem->IsConfirmationComplete())
		{
			// All confirmations received - race can begin
			// Remove from viewport
			RemoveFromParent();
		}
		// If not complete, the subsystem will broadcast OnConfirmationRequired
		// which should trigger ShowConfirmation again with next step
	}
}

void UMGPinkSlipConfirmationWidget::OnCancelClicked_Implementation()
{
	UMGPinkSlipSubsystem* PinkSlipSubsystem = GetPinkSlipSubsystem();
	if (PinkSlipSubsystem)
	{
		// Cancel the confirmation process
		PinkSlipSubsystem->CancelConfirmation();
	}

	// Remove from viewport
	RemoveFromParent();
}

FText UMGPinkSlipConfirmationWidget::GetFormattedTotalValue() const
{
	// Format as currency
	return FText::AsCurrency(CurrentConfirmData.TotalValueAtStake);
}

FText UMGPinkSlipConfirmationWidget::GetStepText() const
{
	return FText::Format(
		NSLOCTEXT("PinkSlip", "StepFormat", "Confirmation {0} of {1}"),
		FText::AsNumber(CurrentConfirmData.ConfirmationStep),
		FText::AsNumber(CurrentConfirmData.TotalConfirmations)
	);
}

FText UMGPinkSlipConfirmationWidget::GetConfirmButtonText() const
{
	if (CurrentConfirmData.bIsFinalConfirmation)
	{
		return NSLOCTEXT("PinkSlip", "FinalConfirm", "I ACCEPT THE RISK - RACE FOR PINKS");
	}

	switch (CurrentConfirmData.ConfirmationStep)
	{
	case 1:
		return NSLOCTEXT("PinkSlip", "Confirm1Button", "I Understand - Continue");
	case 2:
		return NSLOCTEXT("PinkSlip", "Confirm2Button", "Yes, I'm Sure - Final Warning");
	default:
		return NSLOCTEXT("PinkSlip", "ConfirmDefault", "Confirm");
	}
}

FText UMGPinkSlipConfirmationWidget::GetCancelButtonText() const
{
	if (CurrentConfirmData.bIsFinalConfirmation)
	{
		return NSLOCTEXT("PinkSlip", "FinalCancel", "Back Out Safely");
	}

	return NSLOCTEXT("PinkSlip", "Cancel", "Cancel");
}

UMGPinkSlipSubsystem* UMGPinkSlipConfirmationWidget::GetPinkSlipSubsystem() const
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (GameInstance)
	{
		return GameInstance->GetSubsystem<UMGPinkSlipSubsystem>();
	}
	return nullptr;
}
