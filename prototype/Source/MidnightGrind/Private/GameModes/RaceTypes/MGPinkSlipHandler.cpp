// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGPinkSlipHandler.h"
#include "GameModes/MGRaceConfiguration.h"

UMGPinkSlipHandler::UMGPinkSlipHandler()
{
}

void UMGPinkSlipHandler::InitializeRace(const FMGRaceConfiguration& Config)
{
	Super::InitializeRace(Config);

	CurrentState = EMGPinkSlipState::WaitingConfirmation;
	bRaceComplete = false;
	WinnerIndex = -1;
	DisconnectedParticipant = -1;
	DisconnectGraceRemaining = 0.0f;
	VoidReason = FText::GetEmpty();

	// Reset participants
	Participants[0] = FMGPinkSlipParticipant();
	Participants[1] = FMGPinkSlipParticipant();
}

void UMGPinkSlipHandler::StartRace()
{
	// Don't start unless both verified and confirmed
	if (!AreBothVerified() || !AreBothConfirmed())
	{
		return;
	}

	Super::StartRace();
	SetState(EMGPinkSlipState::Racing);

	// Start inner race handler if set
	if (InnerRaceHandler)
	{
		InnerRaceHandler->StartRace();
	}
}

void UMGPinkSlipHandler::UpdateRace(float DeltaTime)
{
	Super::UpdateRace(DeltaTime);

	if (bRaceComplete)
	{
		return;
	}

	// Handle disconnect grace period
	UpdateDisconnect(DeltaTime);

	// Update inner race
	if (CurrentState == EMGPinkSlipState::Racing && InnerRaceHandler)
	{
		InnerRaceHandler->UpdateRace(DeltaTime);

		// Check if inner race completed
		if (InnerRaceHandler->IsRaceComplete())
		{
			TArray<FMGRaceResult> InnerResults = InnerRaceHandler->GetResults();
			if (InnerResults.Num() > 0)
			{
				// Winner is first position
				WinnerIndex = InnerResults[0].ParticipantIndex;
				SetState(EMGPinkSlipState::ProcessingTransfer);
				ProcessTransfer();
			}
		}
	}
}

void UMGPinkSlipHandler::EndRace()
{
	Super::EndRace();

	if (InnerRaceHandler)
	{
		InnerRaceHandler->EndRace();
	}
}

bool UMGPinkSlipHandler::IsRaceComplete() const
{
	return bRaceComplete;
}

TArray<FMGRaceResult> UMGPinkSlipHandler::GetResults() const
{
	if (InnerRaceHandler)
	{
		return InnerRaceHandler->GetResults();
	}

	TArray<FMGRaceResult> Results;
	for (int32 i = 0; i < 2; ++i)
	{
		FMGRaceResult Result;
		Result.ParticipantIndex = i;
		Result.Position = (i == WinnerIndex) ? 1 : 2;
		Result.bFinished = bRaceComplete;
		Results.Add(Result);
	}
	return Results;
}

FText UMGPinkSlipHandler::GetRaceTypeName() const
{
	return NSLOCTEXT("MG", "PinkSlip", "Pink Slip");
}

void UMGPinkSlipHandler::SetChallenger(const FString& PlayerID, FGuid VehicleID)
{
	Participants[0].PlayerID = PlayerID;
	Participants[0].VehicleID = VehicleID;
	Participants[0].bIsChallenger = true;

	// Would fetch vehicle info here
	Participants[0].VehicleName = NSLOCTEXT("MG", "UnknownVehicle", "Vehicle");
	Participants[0].VehiclePI = 500; // Placeholder
	Participants[0].EstimatedValue = 50000; // Placeholder
}

void UMGPinkSlipHandler::SetDefender(const FString& PlayerID, FGuid VehicleID)
{
	Participants[1].PlayerID = PlayerID;
	Participants[1].VehicleID = VehicleID;
	Participants[1].bIsChallenger = false;

	// Would fetch vehicle info here
	Participants[1].VehicleName = NSLOCTEXT("MG", "UnknownVehicle", "Vehicle");
	Participants[1].VehiclePI = 500; // Placeholder
	Participants[1].EstimatedValue = 50000; // Placeholder
}

void UMGPinkSlipHandler::ConfirmWager(int32 ParticipantIndex)
{
	if (ParticipantIndex < 0 || ParticipantIndex >= 2)
	{
		return;
	}

	// Must be verified first
	if (Participants[ParticipantIndex].VerificationStatus != EMGPinkSlipVerification::Passed)
	{
		return;
	}

	Participants[ParticipantIndex].bConfirmed = true;
	OnConfirmed.Broadcast(ParticipantIndex);

	// Check if both confirmed - move to verification
	if (AreBothConfirmed())
	{
		SetState(EMGPinkSlipState::Verification);
	}
}

EMGPinkSlipVerification UMGPinkSlipHandler::VerifyParticipant(int32 ParticipantIndex)
{
	if (ParticipantIndex < 0 || ParticipantIndex >= 2)
	{
		return EMGPinkSlipVerification::Pending;
	}

	FMGPinkSlipParticipant& Participant = Participants[ParticipantIndex];

	// Check PI difference
	int32 OtherIndex = 1 - ParticipantIndex;
	if (Participants[OtherIndex].VehiclePI > 0)
	{
		int32 PIDiff = FMath::Abs(Participant.VehiclePI - Participants[OtherIndex].VehiclePI);
		if (PIDiff > MaxPIDifference)
		{
			Participant.VerificationStatus = EMGPinkSlipVerification::PIOutOfRange;
			OnVerified.Broadcast(ParticipantIndex, Participant.VerificationStatus);
			return Participant.VerificationStatus;
		}
	}

	// Would check other requirements here:
	// - REP tier
	// - Is only vehicle
	// - Trade lock
	// - Account standing
	// - Disconnects
	// - Cooldown

	// For now, assume passed
	Participant.VerificationStatus = EMGPinkSlipVerification::Passed;
	OnVerified.Broadcast(ParticipantIndex, Participant.VerificationStatus);
	return Participant.VerificationStatus;
}

bool UMGPinkSlipHandler::VerifyBoth()
{
	EMGPinkSlipVerification Status0 = VerifyParticipant(0);
	EMGPinkSlipVerification Status1 = VerifyParticipant(1);

	return Status0 == EMGPinkSlipVerification::Passed &&
		Status1 == EMGPinkSlipVerification::Passed;
}

void UMGPinkSlipHandler::SetInnerRaceHandler(UMGRaceTypeHandler* Handler)
{
	InnerRaceHandler = Handler;
}

FMGPinkSlipParticipant UMGPinkSlipHandler::GetParticipant(int32 Index) const
{
	if (Index >= 0 && Index < 2)
	{
		return Participants[Index];
	}
	return FMGPinkSlipParticipant();
}

bool UMGPinkSlipHandler::AreBothConfirmed() const
{
	return Participants[0].bConfirmed && Participants[1].bConfirmed;
}

bool UMGPinkSlipHandler::AreBothVerified() const
{
	return Participants[0].VerificationStatus == EMGPinkSlipVerification::Passed &&
		Participants[1].VerificationStatus == EMGPinkSlipVerification::Passed;
}

void UMGPinkSlipHandler::ReportDisconnect(int32 ParticipantIndex)
{
	if (ParticipantIndex < 0 || ParticipantIndex >= 2)
	{
		return;
	}

	if (CurrentState != EMGPinkSlipState::Racing)
	{
		return;
	}

	DisconnectedParticipant = ParticipantIndex;
	DisconnectGraceRemaining = DisconnectGracePeriod;

	OnDisconnect.Broadcast(ParticipantIndex, false);
}

void UMGPinkSlipHandler::ReportReconnect(int32 ParticipantIndex)
{
	if (DisconnectedParticipant == ParticipantIndex)
	{
		DisconnectedParticipant = -1;
		DisconnectGraceRemaining = 0.0f;
	}
}

void UMGPinkSlipHandler::VoidRace(FText Reason)
{
	VoidReason = Reason;
	SetState(EMGPinkSlipState::Voided);
	bRaceComplete = true;
	WinnerIndex = -1;

	// Both keep their vehicles
}

void UMGPinkSlipHandler::SetState(EMGPinkSlipState NewState)
{
	if (CurrentState != NewState)
	{
		EMGPinkSlipState OldState = CurrentState;
		CurrentState = NewState;
		OnStateChanged.Broadcast(OldState, NewState);
	}
}

void UMGPinkSlipHandler::ProcessTransfer()
{
	if (WinnerIndex < 0 || WinnerIndex >= 2)
	{
		return;
	}

	int32 LoserIndex = 1 - WinnerIndex;

	// Build transfer record
	TransferRecord.TransferID = FGuid::NewGuid();
	TransferRecord.Timestamp = FDateTime::UtcNow();
	TransferRecord.WinnerID = Participants[WinnerIndex].PlayerID;
	TransferRecord.LoserID = Participants[LoserIndex].PlayerID;
	TransferRecord.VehicleID = Participants[LoserIndex].VehicleID;
	TransferRecord.VehicleName = Participants[LoserIndex].VehicleName;
	TransferRecord.VehicleValue = Participants[LoserIndex].EstimatedValue;
	TransferRecord.RaceType = InnerRaceHandler ? FName(*InnerRaceHandler->GetRaceTypeName().ToString()) : FName(TEXT("Unknown"));
	TransferRecord.bWasAgainstAI = Participants[LoserIndex].bIsAI;

	// Apply cooldown to loser
	ApplyCooldown(Participants[LoserIndex].PlayerID);

	// Apply trade lock to won vehicle
	ApplyTradeLock(Participants[LoserIndex].VehicleID);

	// Record transfer
	RecordTransfer();

	SetState(EMGPinkSlipState::Complete);
	bRaceComplete = true;

	OnTransferComplete.Broadcast(TransferRecord);
}

void UMGPinkSlipHandler::ApplyCooldown(const FString& PlayerID)
{
	// Would integrate with progression/cooldown system
	// For now, just log
}

void UMGPinkSlipHandler::ApplyTradeLock(FGuid VehicleID)
{
	// Would integrate with garage/trade system
	// Lock vehicle for WonVehicleTradeLockDays
}

void UMGPinkSlipHandler::UpdateDisconnect(float DeltaTime)
{
	if (DisconnectedParticipant < 0)
	{
		return;
	}

	DisconnectGraceRemaining -= DeltaTime;

	if (DisconnectGraceRemaining <= 0.0f)
	{
		// Grace period expired - disconnected player loses
		WinnerIndex = 1 - DisconnectedParticipant;
		OnDisconnect.Broadcast(DisconnectedParticipant, true);

		SetState(EMGPinkSlipState::ProcessingTransfer);
		ProcessTransfer();
	}
}

void UMGPinkSlipHandler::RecordTransfer()
{
	// Would save to persistent storage and send to server
}
