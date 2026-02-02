// Copyright Midnight Grind. All Rights Reserved.

#include "PinkSlip/MGPinkSlipSubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Core/MGSaveSubsystem.h"
#include "Wager/MGWagerSubsystem.h"
#include "Engine/GameInstance.h"

void UMGPinkSlipSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Ensure dependent subsystems are initialized
	Collection.InitializeDependency(UMGGarageSubsystem::StaticClass());
	Collection.InitializeDependency(UMGSaveSubsystem::StaticClass());
	Collection.InitializeDependency(UMGWagerSubsystem::StaticClass());

	// Load saved pink slip data
	LoadPinkSlipData();

	// Clean up any expired trade locks
	CleanupExpiredTradeLocks();

	UE_LOG(LogTemp, Log, TEXT("MGPinkSlipSubsystem initialized - Pink slip races enabled"));
}

void UMGPinkSlipSubsystem::Deinitialize()
{
	// Save any pending data
	SavePinkSlipData();

	Super::Deinitialize();
}

bool UMGPinkSlipSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ==========================================
// ELIGIBILITY CHECKING
// ==========================================

EMGPinkSlipEligibility UMGPinkSlipSubsystem::CheckEligibility(const FGuid& VehicleID) const
{
	UMGGarageSubsystem* Garage = GetGarageSubsystem();
	UMGSaveSubsystem* Save = GetSaveSubsystem();

	if (!Garage || !Save)
	{
		return EMGPinkSlipEligibility::AccountRestricted;
	}

	// Check player level requirement
	const int32 PlayerLevel = Save->GetCurrentLevel();
	if (PlayerLevel < MinPlayerLevel)
	{
		return EMGPinkSlipEligibility::LevelTooLow;
	}

	// Check REP tier requirement
	const int32 PlayerREP = Save->GetCurrentREP();
	const int32 REPTier = GetREPTier(PlayerREP);
	if (REPTier < MinREPTier)
	{
		return EMGPinkSlipEligibility::InsufficientREP;
	}

	// Check cooldown
	if (IsOnCooldown())
	{
		return EMGPinkSlipEligibility::OnCooldown;
	}

	// Check vehicle-specific eligibility
	return CheckVehicleEligibility(VehicleID, 0);
}

EMGPinkSlipEligibility UMGPinkSlipSubsystem::CheckVehicleEligibility(const FGuid& VehicleID, int32 OpponentPI) const
{
	UMGGarageSubsystem* Garage = GetGarageSubsystem();
	if (!Garage)
	{
		return EMGPinkSlipEligibility::AccountRestricted;
	}

	// CRITICAL: Cannot wager your only vehicle
	// Per GDD Design Pillar: "Loss is permanent and meaningful"
	// Player must have at least 2 vehicles to participate
	const int32 VehicleCount = Garage->GetVehicleCount();
	if (VehicleCount <= 1)
	{
		return EMGPinkSlipEligibility::OnlyVehicle;
	}

	// Check if vehicle is trade-locked (recently won in pink slip)
	if (IsVehicleTradeLocked(VehicleID))
	{
		return EMGPinkSlipEligibility::VehicleTradeLocked;
	}

	// Check PI matching if opponent PI provided
	if (OpponentPI > 0)
	{
		const int32 VehiclePI = Garage->GetPerformanceIndex(VehicleID);
		const int32 PIDifference = FMath::Abs(VehiclePI - OpponentPI);

		if (PIDifference > MaxPIDifference)
		{
			return EMGPinkSlipEligibility::PIOutOfRange;
		}
	}

	// Check wager subsystem for too many active wagers
	UMGWagerSubsystem* Wager = GetWagerSubsystem();
	if (Wager && !Wager->CanCreateWager())
	{
		return EMGPinkSlipEligibility::TooManyActiveWagers;
	}

	return EMGPinkSlipEligibility::Eligible;
}

FText UMGPinkSlipSubsystem::GetEligibilityMessage(EMGPinkSlipEligibility Status) const
{
	switch (Status)
	{
	case EMGPinkSlipEligibility::Eligible:
		return NSLOCTEXT("PinkSlip", "Eligible", "You are eligible for pink slip racing.");

	case EMGPinkSlipEligibility::OnlyVehicle:
		return NSLOCTEXT("PinkSlip", "OnlyVehicle",
			"You cannot wager your only vehicle. Purchase or win another car first.");

	case EMGPinkSlipEligibility::VehicleTradeLocked:
		return NSLOCTEXT("PinkSlip", "TradeLocked",
			"This vehicle is trade-locked from a recent pink slip win. Wait for the lock to expire.");

	case EMGPinkSlipEligibility::InsufficientREP:
		return FText::Format(
			NSLOCTEXT("PinkSlip", "InsufficientREP",
				"Pink slip racing requires REP Tier {0} or higher. Keep racing to build your reputation."),
			FText::AsNumber(MinREPTier));

	case EMGPinkSlipEligibility::OnCooldown:
		{
			FTimespan Remaining = ActiveCooldown.GetRemainingTime();
			int32 HoursRemaining = FMath::CeilToInt(Remaining.GetTotalHours());
			return FText::Format(
				NSLOCTEXT("PinkSlip", "OnCooldown",
					"You are on cooldown after your last loss. {0} hours remaining."),
				FText::AsNumber(HoursRemaining));
		}

	case EMGPinkSlipEligibility::PIOutOfRange:
		return FText::Format(
			NSLOCTEXT("PinkSlip", "PIOutOfRange",
				"Vehicle performance must be within {0} PI of your opponent's vehicle."),
			FText::AsNumber(MaxPIDifference));

	case EMGPinkSlipEligibility::LevelTooLow:
		return FText::Format(
			NSLOCTEXT("PinkSlip", "LevelTooLow",
				"Pink slip racing requires Level {0} or higher."),
			FText::AsNumber(MinPlayerLevel));

	case EMGPinkSlipEligibility::TooManyActiveWagers:
		return NSLOCTEXT("PinkSlip", "TooManyWagers",
			"You have too many active wagers. Complete or cancel some first.");

	case EMGPinkSlipEligibility::AccountRestricted:
		return NSLOCTEXT("PinkSlip", "Restricted",
			"Your account is currently restricted from pink slip racing.");

	case EMGPinkSlipEligibility::DisconnectPenalty:
		return NSLOCTEXT("PinkSlip", "DisconnectPenalty",
			"Recent disconnects have temporarily restricted your pink slip access.");

	default:
		return NSLOCTEXT("PinkSlip", "Unknown", "Unable to determine eligibility.");
	}
}

bool UMGPinkSlipSubsystem::IsOnCooldown() const
{
	return ActiveCooldown.IsActive();
}

bool UMGPinkSlipSubsystem::IsVehicleTradeLocked(const FGuid& VehicleID) const
{
	for (const FMGVehicleTradeLock& Lock : TradeLocks)
	{
		if (Lock.VehicleID == VehicleID && Lock.IsActive())
		{
			return true;
		}
	}
	return false;
}

bool UMGPinkSlipSubsystem::GetVehicleTradeLock(const FGuid& VehicleID, FMGVehicleTradeLock& OutLock) const
{
	for (const FMGVehicleTradeLock& Lock : TradeLocks)
	{
		if (Lock.VehicleID == VehicleID)
		{
			OutLock = Lock;
			return true;
		}
	}
	return false;
}

// ==========================================
// CONFIRMATION SYSTEM
// ==========================================

bool UMGPinkSlipSubsystem::RequestConfirmation(const FGuid& PlayerVehicleID, const FGuid& OpponentVehicleID, FName TrackID)
{
	// Verify eligibility first
	EMGPinkSlipEligibility Eligibility = CheckEligibility(PlayerVehicleID);
	if (Eligibility != EMGPinkSlipEligibility::Eligible)
	{
		OnEligibilityChecked.Broadcast(Eligibility);
		return false;
	}

	// Store pending confirmation data
	PendingPlayerVehicleID = PlayerVehicleID;
	PendingOpponentVehicleID = OpponentVehicleID;
	PendingTrackID = TrackID;
	CurrentConfirmationStep = 1;

	// Build and broadcast first confirmation
	PendingConfirmation = BuildConfirmationData(1);
	OnConfirmationRequired.Broadcast(PendingConfirmation);

	return true;
}

void UMGPinkSlipSubsystem::SubmitConfirmation(bool bConfirmed)
{
	if (CurrentConfirmationStep == 0)
	{
		// No confirmation in progress
		return;
	}

	if (!bConfirmed)
	{
		// Player cancelled
		CancelConfirmation();
		return;
	}

	// Move to next step
	CurrentConfirmationStep++;

	if (CurrentConfirmationStep <= RequiredConfirmations)
	{
		// More confirmations needed
		PendingConfirmation = BuildConfirmationData(CurrentConfirmationStep);
		OnConfirmationRequired.Broadcast(PendingConfirmation);
	}
	// If CurrentConfirmationStep > RequiredConfirmations, IsConfirmationComplete() returns true
	// The calling code should check this and proceed with the race
}

void UMGPinkSlipSubsystem::CancelConfirmation()
{
	CurrentConfirmationStep = 0;
	PendingPlayerVehicleID.Invalidate();
	PendingOpponentVehicleID.Invalidate();
	PendingTrackID = NAME_None;
	PendingConfirmation = FMGPinkSlipConfirmationData();
}

FMGPinkSlipConfirmationData UMGPinkSlipSubsystem::BuildConfirmationData(int32 Step) const
{
	FMGPinkSlipConfirmationData Data;
	Data.ConfirmationStep = Step;
	Data.TotalConfirmations = RequiredConfirmations;
	Data.bIsFinalConfirmation = (Step == RequiredConfirmations);

	UMGGarageSubsystem* Garage = GetGarageSubsystem();
	if (Garage)
	{
		FMGOwnedVehicle PlayerVehicle;
		if (Garage->GetVehicle(PendingPlayerVehicleID, PlayerVehicle))
		{
			Data.PlayerVehicleName = FText::FromString(PlayerVehicle.CustomName);
			Data.PlayerVehicleValue = Garage->CalculateSellValue(PendingPlayerVehicleID);
		}

		// For opponent vehicle, this would come from wager data
		// For now, use placeholder or fetch from wager subsystem
		Data.OpponentVehicleName = NSLOCTEXT("PinkSlip", "OpponentVehicle", "Opponent's Vehicle");
		Data.OpponentVehicleValue = Data.PlayerVehicleValue; // Assume similar value
	}

	Data.TotalValueAtStake = Data.PlayerVehicleValue + Data.OpponentVehicleValue;
	Data.TrackName = FText::FromName(PendingTrackID);
	Data.WarningMessage = GetConfirmationWarning(Step);

	return Data;
}

FText UMGPinkSlipSubsystem::GetConfirmationWarning(int32 Step) const
{
	switch (Step)
	{
	case 1:
		return NSLOCTEXT("PinkSlip", "Confirm1",
			"WARNING: Pink slip races are PERMANENT. If you lose, you will lose your vehicle FOREVER. "
			"There are NO retries, NO refunds, NO exceptions. Are you sure you want to continue?");

	case 2:
		return NSLOCTEXT("PinkSlip", "Confirm2",
			"FINAL WARNING: You are about to wager your vehicle in a pink slip race. "
			"The winner takes BOTH vehicles. The loser walks away with NOTHING. "
			"This is your last chance to back out safely.");

	case 3:
		return NSLOCTEXT("PinkSlip", "Confirm3",
			"POINT OF NO RETURN: By confirming, you agree that the outcome of this race is FINAL. "
			"Disconnecting will result in automatic loss. Your keys are now on the table. "
			"May the best driver win.");

	default:
		return FText::GetEmpty();
	}
}

// ==========================================
// TRANSFER EXECUTION
// ==========================================

FMGPinkSlipTransferRecord UMGPinkSlipSubsystem::ExecuteTransfer(
	const FString& WinnerID,
	const FString& LoserID,
	const FGuid& VehicleID,
	FName TrackID,
	FName RaceType,
	float WinningMargin,
	int32 WitnessCount)
{
	FMGPinkSlipTransferRecord Record;
	Record.TransferID = FGuid::NewGuid();
	Record.Timestamp = FDateTime::UtcNow();
	Record.TrackID = TrackID;
	Record.RaceType = RaceType;
	Record.WinningMargin = WinningMargin;
	Record.WitnessCount = WitnessCount;
	Record.VehicleID = VehicleID;
	Record.bWasAgainstAI = LoserID.StartsWith(TEXT("AI_")) || WinnerID.StartsWith(TEXT("AI_"));

	UMGGarageSubsystem* Garage = GetGarageSubsystem();
	UMGSaveSubsystem* Save = GetSaveSubsystem();

	// Get local player ID to determine win/loss
	FString LocalPlayerID = TEXT("LocalPlayer"); // Would come from player state in full implementation

	// Determine if local player won or lost
	const bool bLocalWon = (WinnerID == LocalPlayerID);
	const bool bLocalLost = (LoserID == LocalPlayerID);

	Record.bLocalPlayerWon = bLocalWon;
	Record.OpponentID = bLocalWon ? LoserID : WinnerID;
	Record.OpponentName = Record.OpponentID; // Would be display name in full implementation

	// Get vehicle info before transfer
	if (Garage)
	{
		FMGOwnedVehicle Vehicle;
		if (Garage->GetVehicle(VehicleID, Vehicle))
		{
			Record.VehicleName = FText::FromString(Vehicle.CustomName);
			Record.VehicleValue = Garage->CalculateSellValue(VehicleID);
			Record.VehiclePI = Vehicle.PerformanceIndex;
		}
	}

	// Execute the transfer
	if (bLocalLost)
	{
		// PLAYER LOST - Remove their vehicle permanently
		ProcessPlayerLoss(VehicleID, Record.TransferID);
		TotalLosses++;
		TotalValueLost += Record.VehicleValue;

		UE_LOG(LogTemp, Warning, TEXT("PINK SLIP LOSS: Player lost vehicle %s (Value: %lld)"),
			*Record.VehicleName.ToString(), Record.VehicleValue);
	}
	else if (bLocalWon)
	{
		// PLAYER WON - Add opponent's vehicle to garage
		// Create vehicle data from opponent (would come from network in full implementation)
		FMGSaveVehicleData WonVehicleData;
		WonVehicleData.VehicleInstanceID = VehicleID;
		WonVehicleData.CustomName = Record.VehicleName.ToString();
		WonVehicleData.PurchasePrice = Record.VehicleValue;
		WonVehicleData.TotalInvested = Record.VehicleValue;
		WonVehicleData.PurchaseDate = FDateTime::Now();

		ProcessPlayerWin(WonVehicleData, Record.TransferID);
		TotalWins++;
		TotalValueWon += Record.VehicleValue;

		UE_LOG(LogTemp, Log, TEXT("PINK SLIP WIN: Player won vehicle %s (Value: %lld)"),
			*Record.VehicleName.ToString(), Record.VehicleValue);
	}

	// Add to history
	AddToHistory(Record);

	// Save all changes
	SavePinkSlipData();

	// Broadcast transfer complete
	OnTransferExecuted.Broadcast(Record);

	return Record;
}

void UMGPinkSlipSubsystem::ProcessPlayerLoss(const FGuid& VehicleID, const FGuid& TransferID)
{
	UMGGarageSubsystem* Garage = GetGarageSubsystem();
	UMGSaveSubsystem* Save = GetSaveSubsystem();

	if (!Garage || !Save)
	{
		return;
	}

	// Get vehicle name for cooldown record
	FMGOwnedVehicle Vehicle;
	FText VehicleName = NSLOCTEXT("PinkSlip", "UnknownVehicle", "Unknown Vehicle");
	if (Garage->GetVehicle(VehicleID, Vehicle))
	{
		VehicleName = FText::FromString(Vehicle.CustomName);
	}

	// PERMANENTLY remove vehicle from garage
	// THIS IS IRREVERSIBLE
	FMGGarageResult Result = Garage->RemoveVehicle(VehicleID);
	if (Result.bSuccess)
	{
		// Also remove from save data
		Save->RemoveOwnedVehicle(VehicleID);

		// Broadcast loss event
		OnVehicleLost.Broadcast(VehicleID, VehicleName);

		// Start cooldown to prevent rage betting
		StartCooldown(TransferID, VehicleName);

		// Force save to ensure loss is permanent
		Save->QuickSave();
	}
}

void UMGPinkSlipSubsystem::ProcessPlayerWin(const FMGSaveVehicleData& VehicleData, const FGuid& TransferID)
{
	UMGGarageSubsystem* Garage = GetGarageSubsystem();
	UMGSaveSubsystem* Save = GetSaveSubsystem();

	if (!Garage || !Save)
	{
		return;
	}

	// Add won vehicle to save system first
	FGuid NewVehicleID = Save->AddOwnedVehicle(VehicleData);

	// Add to garage subsystem
	FGuid GarageVehicleID;
	Garage->AddVehicleByID(FName(*VehicleData.CustomName), GarageVehicleID);

	// Apply trade lock - cannot immediately wager won vehicle
	AddTradeLock(NewVehicleID, TransferID);

	// Broadcast win event
	OnVehicleWon.Broadcast(NewVehicleID, FText::FromString(VehicleData.CustomName));

	// Save to ensure win is recorded
	Save->QuickSave();
}

// ==========================================
// HISTORY & STATISTICS
// ==========================================

TArray<FMGPinkSlipTransferRecord> UMGPinkSlipSubsystem::GetTransferHistory(int32 MaxEntries) const
{
	if (MaxEntries <= 0 || MaxEntries >= TransferHistory.Num())
	{
		return TransferHistory;
	}

	TArray<FMGPinkSlipTransferRecord> Result;
	for (int32 i = 0; i < MaxEntries && i < TransferHistory.Num(); ++i)
	{
		Result.Add(TransferHistory[i]);
	}
	return Result;
}

int32 UMGPinkSlipSubsystem::GetTotalVehiclesWon() const
{
	return TotalWins;
}

int32 UMGPinkSlipSubsystem::GetTotalVehiclesLost() const
{
	return TotalLosses;
}

int64 UMGPinkSlipSubsystem::GetTotalValueWon() const
{
	return TotalValueWon;
}

int64 UMGPinkSlipSubsystem::GetTotalValueLost() const
{
	return TotalValueLost;
}

float UMGPinkSlipSubsystem::GetPinkSlipWinRate() const
{
	const int32 TotalRaces = TotalWins + TotalLosses;
	if (TotalRaces == 0)
	{
		return 0.0f;
	}
	return static_cast<float>(TotalWins) / static_cast<float>(TotalRaces);
}

// ==========================================
// HELPER METHODS
// ==========================================

UMGGarageSubsystem* UMGPinkSlipSubsystem::GetGarageSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMGGarageSubsystem>() : nullptr;
}

UMGSaveSubsystem* UMGPinkSlipSubsystem::GetSaveSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMGSaveSubsystem>() : nullptr;
}

UMGWagerSubsystem* UMGPinkSlipSubsystem::GetWagerSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMGWagerSubsystem>() : nullptr;
}

int32 UMGPinkSlipSubsystem::GetREPTier(int32 TotalREP) const
{
	// REP tier thresholds from GDD Section 4.2
	// Tier 0: 0-999 (UNKNOWN)
	// Tier 1: 1,000-4,999 (NEWCOMER)
	// Tier 2: 5,000-14,999 (KNOWN)
	// Tier 3: 15,000-34,999 (RESPECTED)
	// Tier 4: 35,000-74,999 (FEARED)
	// Tier 5: 75,000+ (LEGENDARY)

	if (TotalREP >= 75000) return 5;
	if (TotalREP >= 35000) return 4;
	if (TotalREP >= 15000) return 3;
	if (TotalREP >= 5000) return 2;
	if (TotalREP >= 1000) return 1;
	return 0;
}

void UMGPinkSlipSubsystem::CleanupExpiredTradeLocks()
{
	TradeLocks.RemoveAll([](const FMGVehicleTradeLock& Lock)
	{
		return !Lock.IsActive();
	});
}

void UMGPinkSlipSubsystem::AddTradeLock(const FGuid& VehicleID, const FGuid& TransferID)
{
	FMGVehicleTradeLock Lock;
	Lock.VehicleID = VehicleID;
	Lock.TransferID = TransferID;
	Lock.LockExpires = FDateTime::UtcNow() + FTimespan::FromDays(TradeLockDays);

	TradeLocks.Add(Lock);
}

void UMGPinkSlipSubsystem::StartCooldown(const FGuid& TransferID, const FText& VehicleName)
{
	ActiveCooldown.LossTransferID = TransferID;
	ActiveCooldown.LostVehicleName = VehicleName;
	ActiveCooldown.CooldownExpires = FDateTime::UtcNow() + FTimespan::FromHours(CooldownHours);

	OnCooldownStarted.Broadcast(ActiveCooldown);

	UE_LOG(LogTemp, Log, TEXT("Pink slip cooldown started: %d hours after losing %s"),
		CooldownHours, *VehicleName.ToString());
}

void UMGPinkSlipSubsystem::AddToHistory(const FMGPinkSlipTransferRecord& Record)
{
	// Insert at front (newest first)
	TransferHistory.Insert(Record, 0);

	// Limit history size
	const int32 MaxHistorySize = 100;
	if (TransferHistory.Num() > MaxHistorySize)
	{
		TransferHistory.SetNum(MaxHistorySize);
	}
}

void UMGPinkSlipSubsystem::SavePinkSlipData()
{
	// In full implementation, this would serialize to save system
	// For now, subsystem properties marked with SaveGame will be
	// automatically saved when the save subsystem saves game state

	UMGSaveSubsystem* Save = GetSaveSubsystem();
	if (Save)
	{
		Save->MarkDirty();
	}
}

void UMGPinkSlipSubsystem::LoadPinkSlipData()
{
	// In full implementation, this would deserialize from save system
	// SaveGame properties are automatically loaded
}
