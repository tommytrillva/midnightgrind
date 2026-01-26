// Copyright Midnight Grind. All Rights Reserved.

#include "Save/MGSaveManagerSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "License/MGLicenseSubsystem.h"
#include "VehicleClass/MGVehicleClassSubsystem.h"
#include "Stunt/MGStuntSubsystem.h"
#include "Shortcut/MGShortcutSubsystem.h"
#include "NearMiss/MGNearMissSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGSaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UserIndex = UMGSaveGame::GetDefaultUserIndex();
	CurrentSlotName = UMGSaveGame::GetDefaultSaveSlotName();

	// Create default save game object
	CurrentSaveGame = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));

	// Try to load existing save
	if (DoesSaveExist(CurrentSlotName))
	{
		LoadGame(CurrentSlotName);
	}

	// Start autosave timer
	if (bAutosaveEnabled)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AutosaveTimerHandle,
				this,
				&UMGSaveManagerSubsystem::OnAutosaveTimer,
				AutosaveInterval,
				true
			);
		}
	}
}

void UMGSaveManagerSubsystem::Deinitialize()
{
	// Final save before shutdown
	SaveGame(CurrentSlotName);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutosaveTimerHandle);
	}

	Super::Deinitialize();
}

bool UMGSaveManagerSubsystem::SaveGame(const FString& SlotName)
{
	if (bIsSaving)
	{
		return false;
	}

	FString SaveSlot = SlotName.IsEmpty() ? CurrentSlotName : SlotName;

	bIsSaving = true;

	// Gather data from all subsystems
	GatherSubsystemData();

	// Update timestamp
	CurrentSaveGame->SaveTimestamp = FDateTime::UtcNow();
	CurrentSaveGame->SaveSlotName = SaveSlot;

	// Save to slot
	bool bSuccess = UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlot, UserIndex);

	bIsSaving = false;

	OnSaveCompleted.Broadcast(bSuccess);

	return bSuccess;
}

void UMGSaveManagerSubsystem::SaveGameAsync(const FString& SlotName)
{
	if (bIsSaving)
	{
		return;
	}

	FString SaveSlot = SlotName.IsEmpty() ? CurrentSlotName : SlotName;

	bIsSaving = true;

	// Gather data from all subsystems
	GatherSubsystemData();

	// Update timestamp
	CurrentSaveGame->SaveTimestamp = FDateTime::UtcNow();
	CurrentSaveGame->SaveSlotName = SaveSlot;

	// Async save
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &UMGSaveManagerSubsystem::OnAsyncSaveComplete);

	UGameplayStatics::AsyncSaveGameToSlot(CurrentSaveGame, SaveSlot, UserIndex, SaveDelegate);
}

bool UMGSaveManagerSubsystem::QuickSave()
{
	return SaveGame(CurrentSlotName);
}

bool UMGSaveManagerSubsystem::LoadGame(const FString& SlotName)
{
	if (bIsLoading)
	{
		return false;
	}

	FString LoadSlot = SlotName.IsEmpty() ? CurrentSlotName : SlotName;

	if (!DoesSaveExist(LoadSlot))
	{
		OnLoadCompleted.Broadcast(false);
		return false;
	}

	bIsLoading = true;

	// Load from slot
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(LoadSlot, UserIndex);

	if (LoadedGame)
	{
		CurrentSaveGame = Cast<UMGSaveGame>(LoadedGame);

		if (CurrentSaveGame && ValidateSaveData(CurrentSaveGame))
		{
			CurrentSlotName = LoadSlot;

			// Distribute data to all subsystems
			DistributeSubsystemData();

			bIsLoading = false;
			OnLoadCompleted.Broadcast(true);
			return true;
		}
	}

	bIsLoading = false;
	OnLoadCompleted.Broadcast(false);
	return false;
}

void UMGSaveManagerSubsystem::LoadGameAsync(const FString& SlotName)
{
	if (bIsLoading)
	{
		return;
	}

	FString LoadSlot = SlotName.IsEmpty() ? CurrentSlotName : SlotName;

	if (!DoesSaveExist(LoadSlot))
	{
		OnLoadCompleted.Broadcast(false);
		return;
	}

	bIsLoading = true;

	// Async load
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(this, &UMGSaveManagerSubsystem::OnAsyncLoadComplete);

	UGameplayStatics::AsyncLoadGameFromSlot(LoadSlot, UserIndex, LoadDelegate);
}

bool UMGSaveManagerSubsystem::QuickLoad()
{
	return LoadGame(CurrentSlotName);
}

bool UMGSaveManagerSubsystem::DoesSaveExist(const FString& SlotName) const
{
	FString CheckSlot = SlotName.IsEmpty() ? CurrentSlotName : SlotName;
	return UGameplayStatics::DoesSaveGameExist(CheckSlot, UserIndex);
}

bool UMGSaveManagerSubsystem::DeleteSave(const FString& SlotName)
{
	if (SlotName.IsEmpty())
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

TArray<FString> UMGSaveManagerSubsystem::GetAllSaveSlots() const
{
	TArray<FString> Slots;

	// Check for common slot names
	TArray<FString> PossibleSlots = {
		TEXT("MidnightGrindSave"),
		TEXT("MidnightGrindSave_1"),
		TEXT("MidnightGrindSave_2"),
		TEXT("MidnightGrindSave_3"),
		TEXT("MidnightGrindSave_Autosave")
	};

	for (const FString& Slot : PossibleSlots)
	{
		if (UGameplayStatics::DoesSaveGameExist(Slot, UserIndex))
		{
			Slots.Add(Slot);
		}
	}

	return Slots;
}

void UMGSaveManagerSubsystem::CreateNewGame(const FString& SlotName)
{
	FString NewSlot = SlotName.IsEmpty() ? CurrentSlotName : SlotName;

	// Create fresh save game
	CurrentSaveGame = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));
	CurrentSaveGame->SaveSlotName = NewSlot;
	CurrentSlotName = NewSlot;

	// Initial save
	SaveGame(NewSlot);
}

void UMGSaveManagerSubsystem::SetAutosaveEnabled(bool bEnabled)
{
	bAutosaveEnabled = bEnabled;

	if (UWorld* World = GetWorld())
	{
		if (bEnabled)
		{
			World->GetTimerManager().SetTimer(
				AutosaveTimerHandle,
				this,
				&UMGSaveManagerSubsystem::OnAutosaveTimer,
				AutosaveInterval,
				true
			);
		}
		else
		{
			World->GetTimerManager().ClearTimer(AutosaveTimerHandle);
		}
	}
}

void UMGSaveManagerSubsystem::SetAutosaveInterval(float Seconds)
{
	AutosaveInterval = FMath::Max(60.0f, Seconds); // Minimum 1 minute

	// Reset timer with new interval
	if (bAutosaveEnabled)
	{
		SetAutosaveEnabled(true);
	}
}

void UMGSaveManagerSubsystem::TriggerAutosave()
{
	SaveGame(TEXT("MidnightGrindSave_Autosave"));
}

void UMGSaveManagerSubsystem::GatherSubsystemData()
{
	if (!CurrentSaveGame)
	{
		return;
	}

	// Update play time
	CurrentSaveGame->TotalPlayTime += GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.0f;

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	// Gather Economy data
	if (UMGEconomySubsystem* Economy = GI->GetSubsystem<UMGEconomySubsystem>())
	{
		CurrentSaveGame->PlayerCash = Economy->GetCredits();
	}

	// Gather Garage data - store vehicle IDs
	if (UMGGarageSubsystem* Garage = GI->GetSubsystem<UMGGarageSubsystem>())
	{
		CurrentSaveGame->UnlockedVehicles.Empty();
		TArray<FMGOwnedVehicle> Vehicles = Garage->GetAllVehicles();
		for (const FMGOwnedVehicle& Vehicle : Vehicles)
		{
			// Store vehicle model asset name as the identifier
			if (!Vehicle.VehicleModelData.IsNull())
			{
				CurrentSaveGame->UnlockedVehicles.Add(FName(*Vehicle.VehicleModelData.GetAssetName()));
			}
		}
	}

	// Gather License data
	if (UMGLicenseSubsystem* License = GI->GetSubsystem<UMGLicenseSubsystem>())
	{
		CurrentSaveGame->LicenseData.CurrentLicenseLevel = static_cast<int32>(License->GetHighestLicenseTier());
		CurrentSaveGame->LicenseData.TotalLicenseTests = License->GetTotalTestsCompleted();
		CurrentSaveGame->LicenseData.PerfectLicenseTests = License->GetTotalGoldMedals();
	}

	// Gather Stunt data
	if (UMGStuntSubsystem* Stunt = GI->GetSubsystem<UMGStuntSubsystem>())
	{
		FMGStuntSessionStats StuntStats = Stunt->GetSessionStats();
		CurrentSaveGame->StuntData.TotalStunts = StuntStats.TotalStunts;
		CurrentSaveGame->StuntData.TotalStuntScore = StuntStats.TotalPoints;
		CurrentSaveGame->StuntData.StuntComboMax = StuntStats.BestCombo;
		CurrentSaveGame->StuntData.LongestJump = StuntStats.LongestJump;
		CurrentSaveGame->StuntData.HighestAirTime = StuntStats.HighestAir;
	}

	// Gather Shortcut data
	if (UMGShortcutSubsystem* Shortcut = GI->GetSubsystem<UMGShortcutSubsystem>())
	{
		FMGShortcutSessionStats ShortcutStats = Shortcut->GetSessionStats();
		CurrentSaveGame->ShortcutData.TotalShortcutsUsed = ShortcutStats.TotalShortcutsUsed;
		CurrentSaveGame->ShortcutData.TotalTimeSaved = Shortcut->GetTotalTimeSaved();
		CurrentSaveGame->ShortcutData.SecretShortcutsFound = ShortcutStats.SecretShortcutsFound;
		// Convert discovered shortcuts to FName array
		TArray<FMGShortcutDefinition> DiscoveredShortcuts = Shortcut->GetDiscoveredShortcuts();
		CurrentSaveGame->ShortcutData.DiscoveredShortcuts.Empty();
		for (const FMGShortcutDefinition& ShortcutDef : DiscoveredShortcuts)
		{
			CurrentSaveGame->ShortcutData.DiscoveredShortcuts.Add(FName(*ShortcutDef.ShortcutId));
		}
	}

	// Gather NearMiss data
	if (UMGNearMissSubsystem* NearMiss = GI->GetSubsystem<UMGNearMissSubsystem>())
	{
		FMGStyleSessionStats NearMissStats = NearMiss->GetSessionStats();
		CurrentSaveGame->NearMissData.TotalNearMisses = NearMissStats.TotalNearMisses;
		CurrentSaveGame->NearMissData.TotalNearMissScore = NearMissStats.TotalStylePoints;
		CurrentSaveGame->NearMissData.NearMissChainMax = NearMissStats.BestCombo;
		CurrentSaveGame->NearMissData.ClosestNearMissDistance = NearMissStats.ClosestDistance;
	}

	UE_LOG(LogTemp, Log, TEXT("SaveManager: Gathered subsystem data - Cash: %lld, Vehicles: %d, Stunts: %d, NearMisses: %d"),
		CurrentSaveGame->PlayerCash, CurrentSaveGame->UnlockedVehicles.Num(),
		CurrentSaveGame->StuntData.TotalStunts, CurrentSaveGame->NearMissData.TotalNearMisses);
}

void UMGSaveManagerSubsystem::DistributeSubsystemData()
{
	if (!CurrentSaveGame)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	// Distribute Economy data
	if (UMGEconomySubsystem* Economy = GI->GetSubsystem<UMGEconomySubsystem>())
	{
		Economy->SetCredits(CurrentSaveGame->PlayerCash);
	}

	// Distribute Garage data - restore vehicles
	if (UMGGarageSubsystem* Garage = GI->GetSubsystem<UMGGarageSubsystem>())
	{
		// Clear existing vehicles and add saved ones
		// Note: For MVP, we add vehicles by ID - full implementation would use complete vehicle data
		for (const FName& VehicleID : CurrentSaveGame->UnlockedVehicles)
		{
			FGuid NewVehicleId;
			Garage->AddVehicleByID(VehicleID, NewVehicleId);
		}
	}

	// Note: Subsystem stats are restored when sessions are started
	// The save data acts as persistent storage accessed via GetSaveDataMutable()
	// Individual subsystems can query save data when initializing their sessions

	UE_LOG(LogTemp, Log, TEXT("SaveManager: Distributed subsystem data - Cash: %lld, Vehicles: %d, Stunts: %d, NearMisses: %d"),
		CurrentSaveGame->PlayerCash, CurrentSaveGame->UnlockedVehicles.Num(),
		CurrentSaveGame->StuntData.TotalStunts, CurrentSaveGame->NearMissData.TotalNearMisses);
}

bool UMGSaveManagerSubsystem::ValidateSaveData(const UMGSaveGame* SaveData) const
{
	if (!SaveData)
	{
		return false;
	}

	// Check version compatibility
	if (SaveData->SaveVersion < 1)
	{
		return false;
	}

	// Basic sanity checks
	if (SaveData->PlayerLevel < 1 || SaveData->PlayerLevel > 100)
	{
		return false;
	}

	if (SaveData->PlayerCash < 0)
	{
		return false;
	}

	return true;
}

void UMGSaveManagerSubsystem::OnAsyncSaveComplete(const FString& SlotName, int32 InUserIndex, bool bSuccess)
{
	bIsSaving = false;
	OnSaveCompleted.Broadcast(bSuccess);
}

void UMGSaveManagerSubsystem::OnAsyncLoadComplete(const FString& SlotName, int32 InUserIndex, USaveGame* LoadedGame)
{
	if (LoadedGame)
	{
		CurrentSaveGame = Cast<UMGSaveGame>(LoadedGame);

		if (CurrentSaveGame && ValidateSaveData(CurrentSaveGame))
		{
			CurrentSlotName = SlotName;
			DistributeSubsystemData();
			bIsLoading = false;
			OnLoadCompleted.Broadcast(true);
			return;
		}
	}

	bIsLoading = false;
	OnLoadCompleted.Broadcast(false);
}

void UMGSaveManagerSubsystem::OnAutosaveTimer()
{
	TriggerAutosave();
}
