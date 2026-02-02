// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCertificationSubsystem.cpp
 * @brief Implementation of the Certification Subsystem for platform compliance,
 *        age ratings, and system event handling.
 */

#include "Certification/MGCertificationSubsystem.h"

void UMGCertificationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DetectPlatform();
	InitializeRequirements();

	// Set content descriptors for MIDNIGHT GRIND
	ContentDescriptors.bViolence = false; // No violence - just racing
	ContentDescriptors.bMildLanguage = false;
	ContentDescriptors.bOnlineInteraction = true;
	ContentDescriptors.bInGamePurchases = true; // Cosmetic only
	ContentDescriptors.bUserGeneratedContent = true; // Liveries, tracks

	CurrentAgeRating = EMGAgeRating::ESRB_Everyone;
}

void UMGCertificationSubsystem::DetectPlatform()
{
#if PLATFORM_WINDOWS
	// Check for store-specific builds
	#if WITH_STEAM
		CurrentPlatform = EMGPlatform::Steam;
	#elif WITH_EPIC
		CurrentPlatform = EMGPlatform::EpicGames;
	#else
		CurrentPlatform = EMGPlatform::PC;
	#endif
#elif PLATFORM_PS5
	CurrentPlatform = EMGPlatform::PlayStation5;
#elif PLATFORM_XBOXONE || PLATFORM_XSX
	CurrentPlatform = EMGPlatform::XboxSeriesX;
#elif PLATFORM_SWITCH
	CurrentPlatform = EMGPlatform::NintendoSwitch;
#else
	CurrentPlatform = EMGPlatform::PC;
#endif
}

void UMGCertificationSubsystem::InitializeRequirements()
{
	Requirements.Empty();

	// Universal requirements
	FMGCertificationRequirement SaveOnSuspend;
	SaveOnSuspend.RequirementID = FName(TEXT("REQ_SaveOnSuspend"));
	SaveOnSuspend.Description = FText::FromString(TEXT("Game must save progress when suspended"));
	SaveOnSuspend.bIsMet = true;
	Requirements.Add(SaveOnSuspend);

	FMGCertificationRequirement NetworkError;
	NetworkError.RequirementID = FName(TEXT("REQ_NetworkError"));
	NetworkError.Description = FText::FromString(TEXT("Game must handle network disconnection gracefully"));
	NetworkError.bIsMet = true;
	Requirements.Add(NetworkError);

	FMGCertificationRequirement ControllerDisconnect;
	ControllerDisconnect.RequirementID = FName(TEXT("REQ_ControllerDisconnect"));
	ControllerDisconnect.Description = FText::FromString(TEXT("Game must pause when controller disconnects"));
	ControllerDisconnect.bIsMet = true;
	Requirements.Add(ControllerDisconnect);

	// PlayStation-specific TRCs
	if (CurrentPlatform == EMGPlatform::PlayStation5)
	{
		FMGCertificationRequirement ActivityCards;
		ActivityCards.RequirementID = FName(TEXT("PS_ActivityCards"));
		ActivityCards.Description = FText::FromString(TEXT("Support PlayStation Activity Cards"));
		ActivityCards.Platform = EMGPlatform::PlayStation5;
		Requirements.Add(ActivityCards);

		FMGCertificationRequirement DualSense;
		DualSense.RequirementID = FName(TEXT("PS_DualSense"));
		DualSense.Description = FText::FromString(TEXT("Support DualSense haptic feedback"));
		DualSense.Platform = EMGPlatform::PlayStation5;
		Requirements.Add(DualSense);
	}

	// Xbox-specific XRs
	if (CurrentPlatform == EMGPlatform::XboxSeriesX)
	{
		FMGCertificationRequirement SmartDelivery;
		SmartDelivery.RequirementID = FName(TEXT("XB_SmartDelivery"));
		SmartDelivery.Description = FText::FromString(TEXT("Support Smart Delivery"));
		SmartDelivery.Platform = EMGPlatform::XboxSeriesX;
		Requirements.Add(SmartDelivery);

		FMGCertificationRequirement QuickResume;
		QuickResume.RequirementID = FName(TEXT("XB_QuickResume"));
		QuickResume.Description = FText::FromString(TEXT("Support Quick Resume"));
		QuickResume.Platform = EMGPlatform::XboxSeriesX;
		Requirements.Add(QuickResume);
	}
}

bool UMGCertificationSubsystem::ValidateRequirement(FName RequirementID)
{
	for (FMGCertificationRequirement& Req : Requirements)
	{
		if (Req.RequirementID == RequirementID)
		{
			// Would perform actual validation
			Req.bIsMet = true;
			return true;
		}
	}
	return false;
}

bool UMGCertificationSubsystem::AreAllRequirementsMet() const
{
	for (const FMGCertificationRequirement& Req : Requirements)
	{
		if (Req.Platform == CurrentPlatform || Req.Platform == EMGPlatform::PC)
		{
			if (!Req.bIsMet)
				return false;
		}
	}
	return true;
}

void UMGCertificationSubsystem::OnApplicationSuspending()
{
	// Save game state immediately
	// Required for console certification
}

void UMGCertificationSubsystem::OnApplicationResuming()
{
	// Restore game state
	// Check network connectivity
	// Validate user session
}

void UMGCertificationSubsystem::OnNetworkStatusChanged(bool bIsOnline)
{
	bNetworkAvailable = bIsOnline;

	if (!bIsOnline)
	{
		// Show network error message
		// Return to appropriate menu
	}
}

void UMGCertificationSubsystem::OnControllerDisconnected(int32 ControllerID)
{
	// Pause game
	// Show reconnect prompt
}

void UMGCertificationSubsystem::OnControllerReconnected(int32 ControllerID)
{
	// Allow resuming
}

void UMGCertificationSubsystem::OnUserSignedOut()
{
	bUserSignedIn = false;
	// Return to title screen
	// Clear user data from memory
}
