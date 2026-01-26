// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGPinkSlipHandler.h"
#include "GameModes/MGRaceConfiguration.h"
#include "PinkSlip/MGPinkSlipSubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Reputation/MGReputationSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

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

	// AI opponents always pass verification
	if (Participant.bIsAI)
	{
		Participant.VerificationStatus = EMGPinkSlipVerification::Passed;
		OnVerified.Broadcast(ParticipantIndex, Participant.VerificationStatus);
		return Participant.VerificationStatus;
	}

	// Check PI difference first (applies to both players)
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

	// Use PinkSlipSubsystem for comprehensive eligibility checking
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (GameInstance)
	{
		UMGPinkSlipSubsystem* PinkSlipSubsystem = GameInstance->GetSubsystem<UMGPinkSlipSubsystem>();
		if (PinkSlipSubsystem)
		{
			// Get opponent PI for matching check
			int32 OpponentPI = Participants[OtherIndex].VehiclePI;

			// Perform full eligibility check
			EMGPinkSlipEligibility Eligibility = PinkSlipSubsystem->CheckVehicleEligibility(
				Participant.VehicleID,
				OpponentPI
			);

			// Map eligibility to verification status
			switch (Eligibility)
			{
			case EMGPinkSlipEligibility::Eligible:
				Participant.VerificationStatus = EMGPinkSlipVerification::Passed;
				break;

			case EMGPinkSlipEligibility::OnlyVehicle:
				Participant.VerificationStatus = EMGPinkSlipVerification::OnlyVehicle;
				break;

			case EMGPinkSlipEligibility::VehicleTradeLocked:
				Participant.VerificationStatus = EMGPinkSlipVerification::TradeLocked;
				break;

			case EMGPinkSlipEligibility::InsufficientREP:
				Participant.VerificationStatus = EMGPinkSlipVerification::InsufficientREP;
				break;

			case EMGPinkSlipEligibility::OnCooldown:
				Participant.VerificationStatus = EMGPinkSlipVerification::OnCooldown;
				break;

			case EMGPinkSlipEligibility::PIOutOfRange:
				Participant.VerificationStatus = EMGPinkSlipVerification::PIOutOfRange;
				break;

			case EMGPinkSlipEligibility::DisconnectPenalty:
				Participant.VerificationStatus = EMGPinkSlipVerification::DisconnectPenalty;
				break;

			case EMGPinkSlipEligibility::AccountRestricted:
			default:
				Participant.VerificationStatus = EMGPinkSlipVerification::AccountRestricted;
				break;
			}

			OnVerified.Broadcast(ParticipantIndex, Participant.VerificationStatus);
			return Participant.VerificationStatus;
		}
	}

	// Fallback: If no subsystem available, pass verification
	// This allows offline/test scenarios to work
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

	// Check for photo finish before recording
	CheckPhotoFinish();

	int32 LoserIndex = 1 - WinnerIndex;

	// Build transfer record for handler
	TransferRecord.TransferID = FGuid::NewGuid();
	TransferRecord.Timestamp = FDateTime::UtcNow();
	TransferRecord.WinnerID = Participants[WinnerIndex].PlayerID;
	TransferRecord.LoserID = Participants[LoserIndex].PlayerID;
	TransferRecord.VehicleID = Participants[LoserIndex].VehicleID;
	TransferRecord.VehicleName = Participants[LoserIndex].VehicleName;
	TransferRecord.VehicleValue = Participants[LoserIndex].EstimatedValue;
	TransferRecord.RaceType = InnerRaceHandler ? FName(*InnerRaceHandler->GetRaceTypeName().ToString()) : FName(TEXT("Unknown"));
	TransferRecord.bWasAgainstAI = Participants[LoserIndex].bIsAI;

	// CRITICAL: Execute permanent vehicle transfer via PinkSlipSubsystem
	// This is the point of no return - the title changes hands FOREVER
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (GameInstance)
	{
		UMGPinkSlipSubsystem* PinkSlipSubsystem = GameInstance->GetSubsystem<UMGPinkSlipSubsystem>();
		if (PinkSlipSubsystem)
		{
			// Execute the PERMANENT transfer
			FMGPinkSlipTransferRecord FullRecord = PinkSlipSubsystem->ExecuteTransfer(
				Participants[WinnerIndex].PlayerID,
				Participants[LoserIndex].PlayerID,
				Participants[LoserIndex].VehicleID,
				NAME_None, // Track ID - would come from race configuration
				TransferRecord.RaceType,
				FinishTimeDifference,
				Witnesses.Num()
			);

			UE_LOG(LogTemp, Warning, TEXT("PINK SLIP COMPLETE: %s won %s's vehicle (%s)"),
				*Participants[WinnerIndex].PlayerID,
				*Participants[LoserIndex].PlayerID,
				*TransferRecord.VehicleName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CRITICAL: PinkSlipSubsystem not found - transfer NOT executed!"));
		}
	}

	// Broadcast dramatic moment - keys change hands
	BroadcastDramaticMoment(EMGPinkSlipMoment::KeysChange, WinnerIndex);

	// Walk of shame for loser
	BroadcastDramaticMoment(EMGPinkSlipMoment::WalkOfShame, LoserIndex);

	// Record transfer for witnesses
	RecordTransfer();

	SetState(EMGPinkSlipState::Complete);
	bRaceComplete = true;

	// Start rematch window
	RematchWindowRemaining = RematchWindowSeconds;

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

	// For AI opponents, use preset values
	if (Participant.bIsAI)
	{
		Participant.VehicleName = NSLOCTEXT("MG", "AIVehicle", "AI Vehicle");
		Participant.VehiclePI = 500;
		Participant.EstimatedValue = 50000;
		return;
	}

	// Fetch from garage subsystem for player vehicles
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (!GameInstance)
	{
		return;
	}

	UMGGarageSubsystem* Garage = GameInstance->GetSubsystem<UMGGarageSubsystem>();
	if (!Garage)
	{
		return;
	}

	FMGOwnedVehicle VehicleData;
	if (Garage->GetVehicle(Participant.VehicleID, VehicleData))
	{
		Participant.VehicleName = FText::FromString(VehicleData.CustomName);
		Participant.VehiclePI = VehicleData.PerformanceIndex;
		Participant.EstimatedValue = Garage->CalculateSellValue(Participant.VehicleID);

		UE_LOG(LogTemp, Log, TEXT("Pink slip participant %d: %s (PI: %d, Value: %lld)"),
			ParticipantIndex,
			*VehicleData.CustomName,
			Participant.VehiclePI,
			Participant.EstimatedValue);
	}
	else
	{
		// Fallback to defaults if vehicle not found
		Participant.VehicleName = NSLOCTEXT("MG", "UnknownVehicle", "Unknown Vehicle");
		Participant.VehiclePI = 500;
		Participant.EstimatedValue = 50000;

		UE_LOG(LogTemp, Warning, TEXT("Pink slip: Could not find vehicle %s in garage"),
			*Participant.VehicleID.ToString());
	}
}

bool UMGPinkSlipHandler::CheckREPRequirement(const FString& PlayerID) const
{
	// Get reputation subsystem and check tier
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UMGReputationSubsystem* ReputationSubsystem = GI->GetSubsystem<UMGReputationSubsystem>())
		{
			// Get player's overall reputation tier
			EMGReputationTier PlayerTier = ReputationSubsystem->GetTier(EMGReputationCategory::Overall);

			// Convert tier enum to numeric for comparison
			const int32 PlayerTierValue = static_cast<int32>(PlayerTier);

			// MinREPTier: 0=Unknown, 1=Rookie, 2=Regular, 3=Respected, 4=Elite, 5=Legend
			return PlayerTierValue >= MinREPTier;
		}
	}

	// If we can't access the reputation system, default to allowing (fail open)
	UE_LOG(LogTemp, Warning, TEXT("PinkSlipHandler: Could not access ReputationSubsystem, allowing player"));
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
