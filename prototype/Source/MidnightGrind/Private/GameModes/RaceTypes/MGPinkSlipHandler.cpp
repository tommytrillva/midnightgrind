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

	// Triple confirmation check
	if (bRequireTripleConfirmation)
	{
		if (ChallengerConfirmations < 3 || DefenderConfirmations < 3)
		{
			return;
		}
	}

	Super::StartRace();
	SetState(EMGPinkSlipState::Racing);

	// Record start time
	RaceStartTime = FDateTime::UtcNow();

	// Broadcast dramatic moment - keys on the table
	BroadcastDramaticMoment(EMGPinkSlipMoment::KeysOnTheTable);

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
	ChallengerConfirmations = 0;

	// Fetch actual vehicle info
	FetchVehicleInfo(0);

	// Broadcast challenge issued
	BroadcastDramaticMoment(EMGPinkSlipMoment::ChallengeIssued, 0);
}

void UMGPinkSlipHandler::SetDefender(const FString& PlayerID, FGuid VehicleID)
{
	Participants[1].PlayerID = PlayerID;
	Participants[1].VehicleID = VehicleID;
	Participants[1].bIsChallenger = false;
	DefenderConfirmations = 0;

	// Fetch actual vehicle info
	FetchVehicleInfo(1);
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

	// Increment confirmation count for triple confirmation
	if (ParticipantIndex == 0)
	{
		ChallengerConfirmations++;
	}
	else
	{
		DefenderConfirmations++;
	}

	// Check if this is their final confirmation
	const int32 RequiredConfirmations = bRequireTripleConfirmation ? 3 : 1;
	const int32 CurrentConfirmations = (ParticipantIndex == 0) ? ChallengerConfirmations : DefenderConfirmations;

	if (CurrentConfirmations >= RequiredConfirmations)
	{
		Participants[ParticipantIndex].bConfirmed = true;
		OnConfirmed.Broadcast(ParticipantIndex);
	}

	// Check if both have fully confirmed
	UpdateConfirmationState();
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

	// Notify witnesses
	for (const FMGPinkSlipWitness& Witness : Witnesses)
	{
		// Would send notification to witness
	}
}

// ==========================================
// WITNESS SYSTEM
// ==========================================

void UMGPinkSlipHandler::AddWitness(const FString& PlayerID, const FString& DisplayName)
{
	// Check max witnesses
	if (Witnesses.Num() >= MaxWitnesses)
	{
		return;
	}

	// Check if already a witness
	for (const FMGPinkSlipWitness& Witness : Witnesses)
	{
		if (Witness.PlayerID == PlayerID)
		{
			return;
		}
	}

	// Check if participant (can't witness own race)
	if (PlayerID == Participants[0].PlayerID || PlayerID == Participants[1].PlayerID)
	{
		return;
	}

	FMGPinkSlipWitness NewWitness;
	NewWitness.PlayerID = PlayerID;
	NewWitness.DisplayName = DisplayName;
	NewWitness.JoinedTime = FDateTime::UtcNow();

	Witnesses.Add(NewWitness);
	OnWitnessJoined.Broadcast(NewWitness);
}

void UMGPinkSlipHandler::RemoveWitness(const FString& PlayerID)
{
	Witnesses.RemoveAll([&PlayerID](const FMGPinkSlipWitness& Witness) {
		return Witness.PlayerID == PlayerID;
	});
}

// ==========================================
// REMATCH SYSTEM
// ==========================================

bool UMGPinkSlipHandler::IsRematchAvailable() const
{
	// Rematch available only after completion, within window, and if loser still has cars
	return CurrentState == EMGPinkSlipState::Complete &&
		RematchWindowRemaining > 0.0f &&
		WinnerIndex >= 0;
}

void UMGPinkSlipHandler::RequestRematch()
{
	if (!IsRematchAvailable())
	{
		return;
	}

	bRematchRequested = true;
}

void UMGPinkSlipHandler::AcceptRematch()
{
	if (!bRematchRequested || !IsRematchAvailable())
	{
		return;
	}

	bRematchAccepted = true;

	// Would trigger rematch initialization
	// Swap vehicles since winner now has loser's car
}

void UMGPinkSlipHandler::DeclineRematch()
{
	bRematchRequested = false;
	RematchWindowRemaining = 0.0f;
}

// ==========================================
// DRAMA/PRESENTATION
// ==========================================

int64 UMGPinkSlipHandler::GetTotalValueAtStake() const
{
	return Participants[0].EstimatedValue + Participants[1].EstimatedValue;
}

void UMGPinkSlipHandler::BroadcastDramaticMoment(EMGPinkSlipMoment Moment, int32 ParticipantIndex)
{
	OnDramaticMoment.Broadcast(Moment, ParticipantIndex);
}

void UMGPinkSlipHandler::CheckPhotoFinish()
{
	if (ParticipantFinishTimes[0] <= 0.0f || ParticipantFinishTimes[1] <= 0.0f)
	{
		return;
	}

	FinishTimeDifference = FMath::Abs(ParticipantFinishTimes[0] - ParticipantFinishTimes[1]);
	bWasPhotoFinish = FinishTimeDifference <= PhotoFinishThreshold;

	if (bWasPhotoFinish)
	{
		BroadcastDramaticMoment(EMGPinkSlipMoment::PhotoFinish, WinnerIndex);
		OnPhotoFinish.Broadcast(FinishTimeDifference, WinnerIndex);
	}
}

void UMGPinkSlipHandler::FetchVehicleInfo(int32 ParticipantIndex)
{
	if (ParticipantIndex < 0 || ParticipantIndex >= 2)
	{
		return;
	}

	FMGPinkSlipParticipant& Participant = Participants[ParticipantIndex];

	// Would fetch from garage/database subsystem
	// For now, use placeholder values
	Participant.VehicleName = NSLOCTEXT("MG", "UnknownVehicle", "Vehicle");
	Participant.VehiclePI = 500;
	Participant.EstimatedValue = 50000;

	// TODO: Integrate with MGGarageSubsystem to get actual vehicle data
	// if (UMGGarageSubsystem* Garage = GetGarageSubsystem())
	// {
	//     FMGVehicleData VehicleData;
	//     if (Garage->GetVehicleData(Participant.VehicleID, VehicleData))
	//     {
	//         Participant.VehicleName = FText::FromString(VehicleData.DisplayName);
	//         Participant.VehiclePI = FMath::RoundToInt(VehicleData.Stats.PerformanceIndex);
	//         Participant.EstimatedValue = FMath::RoundToInt64(VehicleData.Stats.EstimatedValue);
	//     }
	// }
}

bool UMGPinkSlipHandler::CheckREPRequirement(const FString& PlayerID) const
{
	// Would check player's REP tier against MinREPTier
	// For now, return true

	// TODO: Integrate with MGPlayerProgression
	// if (UMGPlayerProgression* Progression = GetProgressionSubsystem())
	// {
	//     return Progression->GetREPTier(PlayerID) >= MinREPTier;
	// }

	return true;
}

void UMGPinkSlipHandler::UpdateConfirmationState()
{
	const int32 RequiredConfirmations = bRequireTripleConfirmation ? 3 : 1;

	// Check if both have confirmed
	if (ChallengerConfirmations >= RequiredConfirmations &&
		DefenderConfirmations >= RequiredConfirmations)
	{
		// Both fully confirmed - point of no return
		BroadcastDramaticMoment(EMGPinkSlipMoment::PointOfNoReturn);
		SetState(EMGPinkSlipState::Verification);
	}
}
