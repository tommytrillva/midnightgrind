// Copyright Midnight Grind. All Rights Reserved.

#include "Save/MGSaveManagerSubsystem.h"
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

	// Subsystem data is gathered through the subsystems' SaveXXXData() methods
	// which write directly to the CurrentSaveGame object
	// This is done via the individual subsystem interfaces
}

void UMGSaveManagerSubsystem::DistributeSubsystemData()
{
	if (!CurrentSaveGame)
	{
		return;
	}

	// Subsystem data is distributed through the subsystems' LoadXXXData() methods
	// which read from the CurrentSaveGame object
	// This is done via the individual subsystem interfaces
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
