// Copyright Midnight Grind. All Rights Reserved.

#include "Wager/MGWagerSubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Core/MGSaveSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

void UMGWagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default configuration
	Config.MinCurrencyWager = 100;
	Config.MaxCurrencyWager = 1000000;
	Config.WagerExpirationHours = 24.0f;
	Config.HouseFeePercent = 0.0f;
	Config.bAllowPinkSlips = true;
	Config.MinLevelForPinkSlips = 20;
	Config.bAllowVehicleWagers = true;
	Config.MaxActiveWagers = 5;
	Config.bRequireStakeMatch = true;
	Config.StakeMatchTolerance = 0.2f;

	LoadWagerData();

	// Start expiration check timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ExpirationCheckHandle,
			this,
			&UMGWagerSubsystem::CheckExpiredWagers,
			60.0f,
			true
		);
	}
}

void UMGWagerSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationCheckHandle);
	}

	SaveWagerData();

	Super::Deinitialize();
}

bool UMGWagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FGuid UMGWagerSubsystem::ProposeWager(FName OpponentID, const FMGWagerStake& MyStake, const FMGWagerConditions& Conditions)
{
	if (!CanCreateWager())
	{
		return FGuid();
	}

	if (!ValidateStake(MyStake))
	{
		return FGuid();
	}

	if (!CanAffordStake(MyStake))
	{
		return FGuid();
	}

	FMGWager NewWager;
	NewWager.WagerID = FGuid::NewGuid();
	NewWager.State = EMGWagerState::Proposed;
	NewWager.Outcome = EMGWagerOutcome::Pending;
	NewWager.CreatedTime = FDateTime::UtcNow();
	NewWager.ExpiresTime = NewWager.CreatedTime + FTimespan::FromHours(Config.WagerExpirationHours);
	NewWager.Conditions = Conditions;

	// Set initiator
	NewWager.Initiator.PlayerID = LocalPlayerID;
	NewWager.Initiator.PlayerName = LocalPlayerName;
	NewWager.Initiator.Stake = MyStake;
	NewWager.Initiator.bAccepted = true;

	// Set opponent placeholder
	NewWager.Opponent.PlayerID = OpponentID;
	NewWager.Opponent.bAccepted = false;

	// Check if pink slip
	NewWager.bIsPinkSlip = (MyStake.StakeType == EMGWagerType::Vehicle);

	// Generate title
	if (NewWager.bIsPinkSlip)
	{
		NewWager.WagerTitle = FText::FromString(FString::Printf(TEXT("Pink Slip Race - %s"), *MyStake.VehicleName.ToString()));
	}
	else
	{
		NewWager.WagerTitle = FText::FromString(FString::Printf(TEXT("Wager Race - %lld Credits"), MyStake.CurrencyAmount));
	}

	// Lock initiator's stake
	LockStake(MyStake);
	NewWager.Initiator.bStakeLocked = true;

	AllWagers.Add(NewWager.WagerID, NewWager);
	SaveWagerData();

	OnWagerProposed.Broadcast(NewWager);

	return NewWager.WagerID;
}

FGuid UMGWagerSubsystem::ProposePinkSlipRace(FName OpponentID, FName MyVehicleID, const FMGWagerConditions& Conditions)
{
	if (!CanProposePinkSlip())
	{
		return FGuid();
	}

	if (!OwnsVehicle(MyVehicleID))
	{
		return FGuid();
	}

	FMGWagerStake Stake;
	Stake.StakeType = EMGWagerType::Vehicle;
	Stake.VehicleID = MyVehicleID;
	Stake.VehicleName = FText::FromString(MyVehicleID.ToString());

	return ProposeWager(OpponentID, Stake, Conditions);
}

FGuid UMGWagerSubsystem::ProposeCurrencyWager(FName OpponentID, int64 Amount, const FMGWagerConditions& Conditions)
{
	if (Amount < Config.MinCurrencyWager || Amount > Config.MaxCurrencyWager)
	{
		return FGuid();
	}

	FMGWagerStake Stake;
	Stake.StakeType = EMGWagerType::Currency;
	Stake.CurrencyAmount = Amount;
	Stake.EstimatedValue = Amount;

	return ProposeWager(OpponentID, Stake, Conditions);
}

bool UMGWagerSubsystem::CanCreateWager() const
{
	int32 ActiveCount = 0;
	for (const auto& Pair : AllWagers)
	{
		if (Pair.Value.Initiator.PlayerID == LocalPlayerID || Pair.Value.Opponent.PlayerID == LocalPlayerID)
		{
			if (Pair.Value.State == EMGWagerState::Proposed ||
				Pair.Value.State == EMGWagerState::Accepted ||
				Pair.Value.State == EMGWagerState::Active)
			{
				ActiveCount++;
			}
		}
	}

	return ActiveCount < Config.MaxActiveWagers;
}

bool UMGWagerSubsystem::CanProposePinkSlip() const
{
	if (!Config.bAllowPinkSlips)
	{
		return false;
	}

	if (LocalPlayerLevel < Config.MinLevelForPinkSlips)
	{
		return false;
	}

	return CanCreateWager();
}

bool UMGWagerSubsystem::AcceptWager(FGuid WagerID, const FMGWagerStake& MyStake)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return false;
	}

	if (Wager->State != EMGWagerState::Proposed)
	{
		return false;
	}

	if (Wager->Opponent.PlayerID != LocalPlayerID)
	{
		return false;
	}

	if (!ValidateStake(MyStake))
	{
		return false;
	}

	if (!CanAffordStake(MyStake))
	{
		return false;
	}

	// Check stake matching if required
	if (Config.bRequireStakeMatch && !DoStakesMatch(Wager->Initiator.Stake, MyStake))
	{
		return false;
	}

	// Lock opponent's stake
	LockStake(MyStake);

	Wager->Opponent.Stake = MyStake;
	Wager->Opponent.bAccepted = true;
	Wager->Opponent.bStakeLocked = true;
	Wager->Opponent.PlayerName = LocalPlayerName;
	Wager->State = EMGWagerState::Accepted;

	SaveWagerData();

	OnWagerAccepted.Broadcast(*Wager);

	return true;
}

bool UMGWagerSubsystem::DeclineWager(FGuid WagerID)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return false;
	}

	if (Wager->State != EMGWagerState::Proposed)
	{
		return false;
	}

	// Unlock initiator's stake
	UnlockStake(Wager->Initiator.Stake);

	Wager->State = EMGWagerState::Cancelled;
	SaveWagerData();

	OnWagerDeclined.Broadcast(WagerID);

	return true;
}

bool UMGWagerSubsystem::CounterOffer(FGuid WagerID, const FMGWagerStake& CounterStake)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return false;
	}

	if (Wager->State != EMGWagerState::Proposed)
	{
		return false;
	}

	if (!ValidateStake(CounterStake))
	{
		return false;
	}

	// Swap initiator and opponent, propose new wager
	FName OriginalInitiator = Wager->Opponent.PlayerID;
	FMGWagerConditions Conditions = Wager->Conditions;

	// Cancel original wager
	DeclineWager(WagerID);

	// Create counter wager
	return ProposeWager(OriginalInitiator, CounterStake, Conditions).IsValid();
}

bool UMGWagerSubsystem::CancelWager(FGuid WagerID)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return false;
	}

	// Can only cancel if we're the initiator and it hasn't been accepted
	if (Wager->Initiator.PlayerID != LocalPlayerID)
	{
		return false;
	}

	if (Wager->State != EMGWagerState::Proposed)
	{
		return false;
	}

	// Unlock stake
	UnlockStake(Wager->Initiator.Stake);

	Wager->State = EMGWagerState::Cancelled;
	SaveWagerData();

	OnWagerCancelled.Broadcast(WagerID);

	return true;
}

TArray<FMGWager> UMGWagerSubsystem::GetPendingWagers() const
{
	TArray<FMGWager> Result;
	for (const auto& Pair : AllWagers)
	{
		if (Pair.Value.State == EMGWagerState::Proposed || Pair.Value.State == EMGWagerState::Accepted)
		{
			if (Pair.Value.Initiator.PlayerID == LocalPlayerID || Pair.Value.Opponent.PlayerID == LocalPlayerID)
			{
				Result.Add(Pair.Value);
			}
		}
	}
	return Result;
}

TArray<FMGWager> UMGWagerSubsystem::GetActiveWagers() const
{
	TArray<FMGWager> Result;
	for (const auto& Pair : AllWagers)
	{
		if (Pair.Value.State == EMGWagerState::Active)
		{
			if (Pair.Value.Initiator.PlayerID == LocalPlayerID || Pair.Value.Opponent.PlayerID == LocalPlayerID)
			{
				Result.Add(Pair.Value);
			}
		}
	}
	return Result;
}

TArray<FMGWager> UMGWagerSubsystem::GetIncomingWagers() const
{
	TArray<FMGWager> Result;
	for (const auto& Pair : AllWagers)
	{
		if (Pair.Value.State == EMGWagerState::Proposed && Pair.Value.Opponent.PlayerID == LocalPlayerID)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGWager> UMGWagerSubsystem::GetOutgoingWagers() const
{
	TArray<FMGWager> Result;
	for (const auto& Pair : AllWagers)
	{
		if (Pair.Value.State == EMGWagerState::Proposed && Pair.Value.Initiator.PlayerID == LocalPlayerID)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

FMGWager UMGWagerSubsystem::GetWager(FGuid WagerID) const
{
	const FMGWager* Wager = AllWagers.Find(WagerID);
	return Wager ? *Wager : FMGWager();
}

bool UMGWagerSubsystem::HasActiveWager() const
{
	return CurrentRaceWagerID.IsValid();
}

FMGWager UMGWagerSubsystem::GetCurrentRaceWager() const
{
	return GetWager(CurrentRaceWagerID);
}

void UMGWagerSubsystem::StartWagerRace(FGuid WagerID)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return;
	}

	if (Wager->State != EMGWagerState::Accepted)
	{
		return;
	}

	Wager->State = EMGWagerState::Active;
	Wager->RaceSessionID = FName(*FGuid::NewGuid().ToString());
	CurrentRaceWagerID = WagerID;

	SaveWagerData();

	OnWagerStarted.Broadcast(*Wager);
}

void UMGWagerSubsystem::ReportRaceResult(FGuid WagerID, FName WinnerID, int32 InitiatorPosition, int32 OpponentPosition)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return;
	}

	if (Wager->State != EMGWagerState::Active)
	{
		return;
	}

	Wager->Initiator.FinalPosition = InitiatorPosition;
	Wager->Opponent.FinalPosition = OpponentPosition;
	Wager->WinnerID = WinnerID;

	ProcessWagerCompletion(*Wager, WinnerID);
}

void UMGWagerSubsystem::DisputeResult(FGuid WagerID, const FString& Reason)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return;
	}

	if (Wager->State != EMGWagerState::Active && Wager->State != EMGWagerState::Completed)
	{
		return;
	}

	Wager->State = EMGWagerState::Disputed;
	SaveWagerData();
}

TArray<FMGWagerHistory> UMGWagerSubsystem::GetWagerHistory(int32 MaxEntries) const
{
	TArray<FMGWagerHistory> Result;
	int32 Count = FMath::Min(MaxEntries, WagerHistory.Num());
	for (int32 i = 0; i < Count; i++)
	{
		Result.Add(WagerHistory[i]);
	}
	return Result;
}

int32 UMGWagerSubsystem::GetTotalWagersWon() const
{
	return TotalWon;
}

int32 UMGWagerSubsystem::GetTotalWagersLost() const
{
	return TotalLost;
}

int64 UMGWagerSubsystem::GetTotalCurrencyWon() const
{
	return CurrencyWon;
}

int64 UMGWagerSubsystem::GetTotalCurrencyLost() const
{
	return CurrencyLost;
}

bool UMGWagerSubsystem::ValidateStake(const FMGWagerStake& Stake) const
{
	switch (Stake.StakeType)
	{
	case EMGWagerType::Currency:
		return Stake.CurrencyAmount >= Config.MinCurrencyWager &&
			   Stake.CurrencyAmount <= Config.MaxCurrencyWager;

	case EMGWagerType::Vehicle:
		return Config.bAllowVehicleWagers && !Stake.VehicleID.IsNone();

	case EMGWagerType::Part:
		return !Stake.PartID.IsNone();

	case EMGWagerType::Cosmetic:
		return !Stake.CosmeticID.IsNone();

	case EMGWagerType::Experience:
		return Stake.ExperienceAmount > 0;

	case EMGWagerType::Mixed:
		return true;

	default:
		return false;
	}
}

bool UMGWagerSubsystem::DoStakesMatch(const FMGWagerStake& Stake1, const FMGWagerStake& Stake2) const
{
	// Same type requirement
	if (Stake1.StakeType != Stake2.StakeType)
	{
		// Allow currency to match with estimated value of items
		if (Stake1.StakeType == EMGWagerType::Currency || Stake2.StakeType == EMGWagerType::Currency)
		{
			int64 Value1 = Stake1.StakeType == EMGWagerType::Currency ? Stake1.CurrencyAmount : Stake1.EstimatedValue;
			int64 Value2 = Stake2.StakeType == EMGWagerType::Currency ? Stake2.CurrencyAmount : Stake2.EstimatedValue;

			if (Value1 == 0 || Value2 == 0)
			{
				return false;
			}

			float Ratio = (float)FMath::Min(Value1, Value2) / (float)FMath::Max(Value1, Value2);
			return Ratio >= (1.0f - Config.StakeMatchTolerance);
		}
		return false;
	}

	// Same type - check values
	switch (Stake1.StakeType)
	{
	case EMGWagerType::Currency:
		{
			int64 Diff = FMath::Abs(Stake1.CurrencyAmount - Stake2.CurrencyAmount);
			float Tolerance = Config.StakeMatchTolerance * FMath::Max(Stake1.CurrencyAmount, Stake2.CurrencyAmount);
			return (float)Diff <= Tolerance;
		}

	case EMGWagerType::Vehicle:
		{
			// For vehicles, compare estimated values
			if (Stake1.EstimatedValue == 0 || Stake2.EstimatedValue == 0)
			{
				return true; // Allow if no values set (trust players)
			}
			float Ratio = (float)FMath::Min(Stake1.EstimatedValue, Stake2.EstimatedValue) /
						  (float)FMath::Max(Stake1.EstimatedValue, Stake2.EstimatedValue);
			return Ratio >= (1.0f - Config.StakeMatchTolerance);
		}

	default:
		return true;
	}
}

bool UMGWagerSubsystem::CanAffordStake(const FMGWagerStake& Stake) const
{
	// Get save subsystem to check currency
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	UMGSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UMGSaveSubsystem>();
	if (!SaveSubsystem)
	{
		return false;
	}

	switch (Stake.StakeType)
	{
	case EMGWagerType::Currency:
		return SaveSubsystem->GetCurrentCash() >= Stake.CurrencyAmount;

	case EMGWagerType::Vehicle:
		// Vehicle ownership is checked separately
		return true;

	case EMGWagerType::Part:
		return SaveSubsystem->GetPartQuantity(Stake.PartID) > 0;

	default:
		return true;
	}
}

bool UMGWagerSubsystem::OwnsVehicle(FName VehicleID) const
{
	// Get garage subsystem to check vehicle ownership
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return false;
	}

	UMGGarageSubsystem* Garage = GameInstance->GetSubsystem<UMGGarageSubsystem>();
	if (!Garage)
	{
		return false;
	}

	// Convert FName to FGuid for garage lookup
	// In full implementation, VehicleID would be the GUID string
	FGuid VehicleGuid;
	if (FGuid::Parse(VehicleID.ToString(), VehicleGuid))
	{
		FMGOwnedVehicle Vehicle;
		return Garage->GetVehicle(VehicleGuid, Vehicle);
	}

	return false;
}

void UMGWagerSubsystem::SetConfig(const FMGWagerConfig& NewConfig)
{
	Config = NewConfig;
	SaveWagerData();
}

void UMGWagerSubsystem::SetLocalPlayer(FName PlayerID, const FString& PlayerName, int32 PlayerLevel)
{
	LocalPlayerID = PlayerID;
	LocalPlayerName = PlayerName;
	LocalPlayerLevel = PlayerLevel;
}

void UMGWagerSubsystem::ReceiveWagerProposal(const FMGWager& Wager)
{
	if (AllWagers.Contains(Wager.WagerID))
	{
		return;
	}

	AllWagers.Add(Wager.WagerID, Wager);
	SaveWagerData();

	OnWagerProposed.Broadcast(Wager);
}

void UMGWagerSubsystem::ReceiveWagerAcceptance(FGuid WagerID)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return;
	}

	Wager->State = EMGWagerState::Accepted;
	Wager->Opponent.bAccepted = true;
	SaveWagerData();

	OnWagerAccepted.Broadcast(*Wager);
}

void UMGWagerSubsystem::ReceiveWagerDecline(FGuid WagerID)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return;
	}

	// Unlock our stake if we were the initiator
	if (Wager->Initiator.PlayerID == LocalPlayerID)
	{
		UnlockStake(Wager->Initiator.Stake);
	}

	Wager->State = EMGWagerState::Cancelled;
	SaveWagerData();

	OnWagerDeclined.Broadcast(WagerID);
}

void UMGWagerSubsystem::ReceiveWagerResult(FGuid WagerID, FName WinnerID)
{
	FMGWager* Wager = AllWagers.Find(WagerID);
	if (!Wager)
	{
		return;
	}

	if (Wager->State != EMGWagerState::Active)
	{
		return;
	}

	Wager->WinnerID = WinnerID;
	ProcessWagerCompletion(*Wager, WinnerID);
}

void UMGWagerSubsystem::CheckExpiredWagers()
{
	FDateTime Now = FDateTime::UtcNow();
	TArray<FGuid> ExpiredWagers;

	for (auto& Pair : AllWagers)
	{
		if (Pair.Value.State == EMGWagerState::Proposed)
		{
			if (Now > Pair.Value.ExpiresTime)
			{
				ExpiredWagers.Add(Pair.Key);
			}
		}
	}

	for (const FGuid& WagerID : ExpiredWagers)
	{
		FMGWager* Wager = AllWagers.Find(WagerID);
		if (Wager)
		{
			// Unlock initiator's stake
			if (Wager->Initiator.bStakeLocked)
			{
				UnlockStake(Wager->Initiator.Stake);
			}

			Wager->State = EMGWagerState::Expired;
		}
	}

	if (ExpiredWagers.Num() > 0)
	{
		SaveWagerData();
	}
}

void UMGWagerSubsystem::ProcessWagerCompletion(FMGWager& Wager, FName WinnerID)
{
	Wager.State = EMGWagerState::Completed;
	Wager.CompletedTime = FDateTime::UtcNow();
	Wager.WinnerID = WinnerID;

	// Determine outcome
	if (WinnerID.IsNone())
	{
		Wager.Outcome = EMGWagerOutcome::Draw;
		// Return stakes to both players
		UnlockStake(Wager.Initiator.Stake);
		UnlockStake(Wager.Opponent.Stake);
	}
	else if (WinnerID == Wager.Initiator.PlayerID)
	{
		Wager.Outcome = EMGWagerOutcome::WonByInitiator;
		TransferStake(Wager.Opponent.Stake, Wager.Opponent.PlayerID, Wager.Initiator.PlayerID);
		UnlockStake(Wager.Initiator.Stake);
	}
	else if (WinnerID == Wager.Opponent.PlayerID)
	{
		Wager.Outcome = EMGWagerOutcome::WonByOpponent;
		TransferStake(Wager.Initiator.Stake, Wager.Initiator.PlayerID, Wager.Opponent.PlayerID);
		UnlockStake(Wager.Opponent.Stake);
	}

	// Update local player stats
	bool bLocalWon = (WinnerID == LocalPlayerID);
	if (Wager.Initiator.PlayerID == LocalPlayerID || Wager.Opponent.PlayerID == LocalPlayerID)
	{
		AddToHistory(Wager, bLocalWon);

		if (bLocalWon)
		{
			TotalWon++;
			const FMGWagerStake& WonStake = (LocalPlayerID == Wager.Initiator.PlayerID) ?
				Wager.Opponent.Stake : Wager.Initiator.Stake;
			if (WonStake.StakeType == EMGWagerType::Currency)
			{
				CurrencyWon += WonStake.CurrencyAmount;
			}
		}
		else if (!WinnerID.IsNone())
		{
			TotalLost++;
			const FMGWagerStake& LostStake = (LocalPlayerID == Wager.Initiator.PlayerID) ?
				Wager.Initiator.Stake : Wager.Opponent.Stake;
			if (LostStake.StakeType == EMGWagerType::Currency)
			{
				CurrencyLost += LostStake.CurrencyAmount;
			}
		}
	}

	// Clear current race wager if this was it
	if (CurrentRaceWagerID == Wager.WagerID)
	{
		CurrentRaceWagerID.Invalidate();
	}

	SaveWagerData();

	OnWagerCompleted.Broadcast(Wager, bLocalWon);
}

void UMGWagerSubsystem::TransferStake(const FMGWagerStake& Stake, FName FromPlayer, FName ToPlayer)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	bool bReceived = (ToPlayer == LocalPlayerID);
	bool bLost = (FromPlayer == LocalPlayerID);

	switch (Stake.StakeType)
	{
	case EMGWagerType::Currency:
		{
			UMGSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UMGSaveSubsystem>();
			if (SaveSubsystem)
			{
				if (bReceived)
				{
					// Won currency
					SaveSubsystem->AddCash(Stake.CurrencyAmount);
					UE_LOG(LogTemp, Log, TEXT("Wager won: +%lld credits"), Stake.CurrencyAmount);
				}
				else if (bLost)
				{
					// Lost currency (should already be locked)
					SaveSubsystem->SpendCash(Stake.CurrencyAmount);
					UE_LOG(LogTemp, Log, TEXT("Wager lost: -%lld credits"), Stake.CurrencyAmount);
				}
			}
		}
		break;

	case EMGWagerType::Vehicle:
		{
			// Vehicle transfers are handled by MGPinkSlipSubsystem for permanent pink slip races
			// This path is for non-pink-slip vehicle wagers if they exist
			UE_LOG(LogTemp, Warning,
				TEXT("Vehicle stake transfer via WagerSubsystem - should use PinkSlipSubsystem for permanent transfers"));

			// For pink slip races, the MGPinkSlipSubsystem handles the actual transfer
			// This just broadcasts the event
		}
		break;

	case EMGWagerType::Part:
		{
			UMGSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UMGSaveSubsystem>();
			if (SaveSubsystem)
			{
				if (bReceived)
				{
					SaveSubsystem->AddPartToInventory(Stake.PartID, 1);
				}
				else if (bLost)
				{
					SaveSubsystem->RemovePartFromInventory(Stake.PartID, 1);
				}
			}
		}
		break;

	default:
		break;
	}

	OnStakeTransferred.Broadcast(Stake, bReceived);
}

void UMGWagerSubsystem::LockStake(const FMGWagerStake& Stake)
{
	// This would integrate with economy/inventory systems to prevent
	// the staked items from being sold/traded while wager is active
}

void UMGWagerSubsystem::UnlockStake(const FMGWagerStake& Stake)
{
	// This would integrate with economy/inventory systems to release
	// the staked items back to normal availability
}

void UMGWagerSubsystem::AddToHistory(const FMGWager& Wager, bool bWon)
{
	FMGWagerHistory Entry;
	Entry.WagerID = Wager.WagerID;
	Entry.CompletedTime = Wager.CompletedTime;
	Entry.TrackID = Wager.Conditions.TrackID;
	Entry.bWasPinkSlip = Wager.bIsPinkSlip;

	if (Wager.Initiator.PlayerID == LocalPlayerID)
	{
		Entry.OpponentID = Wager.Opponent.PlayerID;
		Entry.OpponentName = Wager.Opponent.PlayerName;
		Entry.StakeWon = bWon ? Wager.Opponent.Stake : FMGWagerStake();
		Entry.StakeLost = bWon ? FMGWagerStake() : Wager.Initiator.Stake;
	}
	else
	{
		Entry.OpponentID = Wager.Initiator.PlayerID;
		Entry.OpponentName = Wager.Initiator.PlayerName;
		Entry.StakeWon = bWon ? Wager.Initiator.Stake : FMGWagerStake();
		Entry.StakeLost = bWon ? FMGWagerStake() : Wager.Opponent.Stake;
	}

	Entry.Outcome = bWon ? EMGWagerOutcome::WonByInitiator : EMGWagerOutcome::WonByOpponent;

	// Add to front of history
	WagerHistory.Insert(Entry, 0);

	// Limit history size
	const int32 MaxHistorySize = 100;
	if (WagerHistory.Num() > MaxHistorySize)
	{
		WagerHistory.SetNum(MaxHistorySize);
	}
}

void UMGWagerSubsystem::SaveWagerData()
{
	// This would integrate with save game system
	// Serialize AllWagers, WagerHistory, stats to persistent storage
}

void UMGWagerSubsystem::LoadWagerData()
{
	// This would integrate with save game system
	// Deserialize AllWagers, WagerHistory, stats from persistent storage
}
