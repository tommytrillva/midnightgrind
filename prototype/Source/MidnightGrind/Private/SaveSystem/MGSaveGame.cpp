// Copyright Midnight Grind. All Rights Reserved.

#include "SaveSystem/MGSaveGame.h"
#include "Progression/MGPlayerProgression.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Kismet/GameplayStatics.h"

const FString UMGSaveGameSubsystem::SaveSlotPrefix = TEXT("MG_Save_");
const FString UMGSaveGameSubsystem::DefaultSlotName = TEXT("QuickSave");

// ==========================================
// UMGSaveGame
// ==========================================

UMGSaveGame::UMGSaveGame()
{
	SaveVersion = GetCurrentSaveVersion();
	SaveTimestamp = FDateTime::Now();
}

FMGSaveSlotInfo UMGSaveGame::GetSlotInfo() const
{
	FMGSaveSlotInfo Info;
	Info.SlotName = SlotName;
	Info.PlayerName = PlayerName;
	Info.PlayerLevel = LevelProgression.CurrentLevel;
	Info.Credits = Credits;
	Info.VehicleCount = OwnedVehicles.Num();
	Info.TotalRaces = Statistics.TotalRaces;
	Info.PlayTime = Statistics.TotalPlayTime;
	Info.LastSaved = SaveTimestamp;
	Info.SaveVersion = SaveVersion;
	return Info;
}

// ==========================================
// UMGSaveGameSubsystem
// ==========================================

void UMGSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGSaveGameSubsystem initialized"));
}

void UMGSaveGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// SAVE OPERATIONS
// ==========================================

bool UMGSaveGameSubsystem::SaveGame(const FString& SlotName, int32 UserIndex)
{
	UMGSaveGame* SaveGameInstance = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create save game object"));
		OnSaveCompleted.Broadcast(false, SlotName);
		return false;
	}

	// Collect all game data
	CollectSaveData(SaveGameInstance);
	SaveGameInstance->SlotName = SlotName;
	SaveGameInstance->SaveTimestamp = FDateTime::Now();

	// Save to disk
	FString FullSlotName = GetFullSlotName(SlotName);
	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, FullSlotName, UserIndex);

	if (bSuccess)
	{
		CurrentSlotName = SlotName;
		bIsNewGame = false;
		UE_LOG(LogTemp, Log, TEXT("Game saved successfully to slot: %s"), *SlotName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save game to slot: %s"), *SlotName);
	}

	OnSaveCompleted.Broadcast(bSuccess, SlotName);
	return bSuccess;
}

bool UMGSaveGameSubsystem::LoadGame(const FString& SlotName, int32 UserIndex)
{
	FString FullSlotName = GetFullSlotName(SlotName);

	if (!UGameplayStatics::DoesSaveGameExist(FullSlotName, UserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Save slot does not exist: %s"), *SlotName);
		OnLoadCompleted.Broadcast(false, SlotName);
		return false;
	}

	UMGSaveGame* LoadedGame = Cast<UMGSaveGame>(UGameplayStatics::LoadGameFromSlot(FullSlotName, UserIndex));
	if (!LoadedGame)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load save game from slot: %s"), *SlotName);
		OnLoadCompleted.Broadcast(false, SlotName);
		return false;
	}

	// Version check
	if (LoadedGame->SaveVersion != UMGSaveGame::GetCurrentSaveVersion())
	{
		UE_LOG(LogTemp, Warning, TEXT("Save version mismatch: %s vs %s. Migration may be needed."),
			*LoadedGame->SaveVersion, *UMGSaveGame::GetCurrentSaveVersion());
		// TODO: Implement save migration if needed
	}

	// Apply loaded data to game systems
	ApplySaveData(LoadedGame);

	CurrentSlotName = SlotName;
	bIsNewGame = false;

	UE_LOG(LogTemp, Log, TEXT("Game loaded successfully from slot: %s"), *SlotName);
	OnLoadCompleted.Broadcast(true, SlotName);
	return true;
}

bool UMGSaveGameSubsystem::QuickSave()
{
	return SaveGame(DefaultSlotName);
}

bool UMGSaveGameSubsystem::QuickLoad()
{
	return LoadGame(DefaultSlotName);
}

bool UMGSaveGameSubsystem::DeleteSave(const FString& SlotName, int32 UserIndex)
{
	FString FullSlotName = GetFullSlotName(SlotName);

	if (!UGameplayStatics::DoesSaveGameExist(FullSlotName, UserIndex))
	{
		return false;
	}

	bool bSuccess = UGameplayStatics::DeleteGameInSlot(FullSlotName, UserIndex);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Deleted save slot: %s"), *SlotName);
	}

	return bSuccess;
}

bool UMGSaveGameSubsystem::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	FString FullSlotName = GetFullSlotName(SlotName);
	return UGameplayStatics::DoesSaveGameExist(FullSlotName, UserIndex);
}

// ==========================================
// SLOT MANAGEMENT
// ==========================================

TArray<FString> UMGSaveGameSubsystem::GetAllSaveSlots() const
{
	TArray<FString> Slots;

	// Check numbered slots
	for (int32 i = 1; i <= MaxSaveSlots; ++i)
	{
		FString SlotName = FString::Printf(TEXT("Slot%d"), i);
		if (DoesSaveExist(SlotName))
		{
			Slots.Add(SlotName);
		}
	}

	// Check default/quick save slot
	if (DoesSaveExist(DefaultSlotName))
	{
		Slots.Add(DefaultSlotName);
	}

	return Slots;
}

bool UMGSaveGameSubsystem::GetSaveSlotInfo(const FString& SlotName, FMGSaveSlotInfo& OutInfo, int32 UserIndex) const
{
	FString FullSlotName = GetFullSlotName(SlotName);

	if (!UGameplayStatics::DoesSaveGameExist(FullSlotName, UserIndex))
	{
		return false;
	}

	UMGSaveGame* LoadedGame = Cast<UMGSaveGame>(UGameplayStatics::LoadGameFromSlot(FullSlotName, UserIndex));
	if (!LoadedGame)
	{
		return false;
	}

	OutInfo = LoadedGame->GetSlotInfo();
	return true;
}

TArray<FMGSaveSlotInfo> UMGSaveGameSubsystem::GetAllSaveSlotInfo() const
{
	TArray<FMGSaveSlotInfo> AllInfo;

	TArray<FString> Slots = GetAllSaveSlots();
	for (const FString& SlotName : Slots)
	{
		FMGSaveSlotInfo Info;
		if (GetSaveSlotInfo(SlotName, Info))
		{
			AllInfo.Add(Info);
		}
	}

	return AllInfo;
}

// ==========================================
// AUTO-SAVE
// ==========================================

void UMGSaveGameSubsystem::SetAutoSaveEnabled(bool bEnabled)
{
	bAutoSaveEnabled = bEnabled;
}

void UMGSaveGameSubsystem::TriggerAutoSave()
{
	if (!bAutoSaveEnabled || bIsNewGame)
	{
		return;
	}

	// Use current slot or default
	FString SlotToUse = CurrentSlotName.IsEmpty() ? DefaultSlotName : CurrentSlotName;
	SaveGame(SlotToUse);
}

// ==========================================
// NEW GAME
// ==========================================

void UMGSaveGameSubsystem::StartNewGame(const FString& PlayerName)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// Reset progression
	UMGPlayerProgression* Progression = GameInstance->GetSubsystem<UMGPlayerProgression>();
	if (Progression)
	{
		Progression->SetPlayerName(PlayerName);
		// Progression will reinitialize to defaults
	}

	// Reset economy
	UMGEconomySubsystem* Economy = GameInstance->GetSubsystem<UMGEconomySubsystem>();
	if (Economy)
	{
		Economy->SetCredits(10000); // Starting credits
	}

	// Clear garage (note: subsystem would need a Reset function)
	// UMGGarageSubsystem* Garage = GameInstance->GetSubsystem<UMGGarageSubsystem>();

	CurrentSlotName.Empty();
	bIsNewGame = true;

	UE_LOG(LogTemp, Log, TEXT("Started new game for player: %s"), *PlayerName);
}

// ==========================================
// INTERNAL
// ==========================================

void UMGSaveGameSubsystem::CollectSaveData(UMGSaveGame* SaveGame)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !SaveGame)
	{
		return;
	}

	// Collect progression data
	UMGPlayerProgression* Progression = GameInstance->GetSubsystem<UMGPlayerProgression>();
	if (Progression)
	{
		SaveGame->PlayerName = Progression->GetPlayerName();
		SaveGame->LevelProgression = Progression->GetLevelProgression();
		SaveGame->Statistics = Progression->GetRaceStatistics();
		SaveGame->Unlocks = Progression->GetAllUnlocks();

		// Copy crew reputations
		TArray<FMGCrewReputation> Reps = Progression->GetAllCrewReputations();
		for (const FMGCrewReputation& Rep : Reps)
		{
			SaveGame->CrewReputations.Add(Rep.Crew, Rep);
		}
	}

	// Collect economy data
	UMGEconomySubsystem* Economy = GameInstance->GetSubsystem<UMGEconomySubsystem>();
	if (Economy)
	{
		SaveGame->Credits = Economy->GetCredits();
		SaveGame->TotalEarned = Economy->GetTotalEarned();
		SaveGame->TotalSpent = Economy->GetTotalSpent();
		SaveGame->TransactionHistory = Economy->GetTransactionHistory();
	}

	// Collect garage data
	UMGGarageSubsystem* Garage = GameInstance->GetSubsystem<UMGGarageSubsystem>();
	if (Garage)
	{
		SaveGame->OwnedVehicles = Garage->GetAllVehicles();

		FMGOwnedVehicle SelectedVehicle;
		if (Garage->GetSelectedVehicle(SelectedVehicle))
		{
			SaveGame->SelectedVehicleId = SelectedVehicle.VehicleId;
		}
	}
}

void UMGSaveGameSubsystem::ApplySaveData(const UMGSaveGame* SaveGame)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance || !SaveGame)
	{
		return;
	}

	// Note: The subsystems would need "LoadFromSave" functions to properly restore state
	// For now, this is a placeholder showing the pattern

	// Apply progression data
	UMGPlayerProgression* Progression = GameInstance->GetSubsystem<UMGPlayerProgression>();
	if (Progression)
	{
		Progression->SetPlayerName(SaveGame->PlayerName);
		// Would need: Progression->LoadFromSave(SaveGame->LevelProgression, SaveGame->Statistics, ...);
	}

	// Apply economy data
	UMGEconomySubsystem* Economy = GameInstance->GetSubsystem<UMGEconomySubsystem>();
	if (Economy)
	{
		Economy->SetCredits(SaveGame->Credits);
		// Would need: Economy->LoadFromSave(SaveGame->TotalEarned, SaveGame->TotalSpent, ...);
	}

	// Apply garage data
	UMGGarageSubsystem* Garage = GameInstance->GetSubsystem<UMGGarageSubsystem>();
	if (Garage)
	{
		// Would need: Garage->LoadFromSave(SaveGame->OwnedVehicles, SaveGame->SelectedVehicleId);
	}

	UE_LOG(LogTemp, Log, TEXT("Applied save data for player: %s"), *SaveGame->PlayerName);
}

FString UMGSaveGameSubsystem::GetFullSlotName(const FString& SlotName) const
{
	return SaveSlotPrefix + SlotName;
}
