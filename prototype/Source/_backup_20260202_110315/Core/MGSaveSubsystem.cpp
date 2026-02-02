// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize with default data
	CurrentSaveData = GetDefaultSaveData();
	CurrentSlotIndex = -1;
	bHasUnsavedChanges = false;

	// Start auto-save timer if enabled
	if (bAutoSaveEnabled)
	{
		if (UWorld* World = GetWorld())
		{
			TWeakObjectPtr<UMGSaveSubsystem> WeakThis(this);
			World->GetTimerManager().SetTimer(
				AutoSaveTimerHandle,
				[WeakThis]() { if (WeakThis.IsValid()) WeakThis->OnAutoSaveTimerTick(); },
				AutoSaveIntervalMinutes * 60.0f,
				true
			);
		}
	}
}

void UMGSaveSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	Super::Deinitialize();
}

// ==========================================
// SAVE OPERATIONS
// ==========================================

bool UMGSaveSubsystem::SaveGame(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	OnSaveStarted.Broadcast(SlotIndex);

	// Gather current state
	GatherCurrentGameState(CurrentSaveData);
	CurrentSaveData.SaveTime = FDateTime::Now();
	CurrentSaveData.SaveVersion = CurrentSaveVersion;

	// Create save game object
	UMGSaveGameObject* SaveGameObject = NewObject<UMGSaveGameObject>();
	SaveGameObject->SaveData = CurrentSaveData;

	// Save to disk
	FString SlotName = GetSaveSlotName(SlotIndex);
	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);

	if (bSuccess)
	{
		CurrentSlotIndex = SlotIndex;
		bHasUnsavedChanges = false;
	}

	OnSaveCompleted.Broadcast(SlotIndex, bSuccess);

	return bSuccess;
}

void UMGSaveSubsystem::SaveGameAsync(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		OnSaveCompleted.Broadcast(SlotIndex, false);
		return;
	}

	OnSaveStarted.Broadcast(SlotIndex);

	// Gather current state
	GatherCurrentGameState(CurrentSaveData);
	CurrentSaveData.SaveTime = FDateTime::Now();
	CurrentSaveData.SaveVersion = CurrentSaveVersion;

	// Create save game object
	UMGSaveGameObject* SaveGameObject = NewObject<UMGSaveGameObject>();
	SaveGameObject->SaveData = CurrentSaveData;

	// Async save
	FString SlotName = GetSaveSlotName(SlotIndex);
	TWeakObjectPtr<UMGSaveSubsystem> WeakThis(this);
	UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject, SlotName, 0,
		FAsyncSaveGameToSlotDelegate::CreateLambda([WeakThis, SlotIndex](const FString& SlotName, const int32 UserIndex, bool bSuccess)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			if (bSuccess)
			{
				WeakThis->CurrentSlotIndex = SlotIndex;
				WeakThis->bHasUnsavedChanges = false;
			}
			WeakThis->OnSaveCompleted.Broadcast(SlotIndex, bSuccess);
		})
	);
}

bool UMGSaveSubsystem::QuickSave()
{
	return SaveGame(QuickSaveSlotIndex);
}

void UMGSaveSubsystem::TriggerAutoSave()
{
	if (!bAutoSaveEnabled)
	{
		return;
	}

	OnAutoSave.Broadcast(AutoSaveSlotIndex);
	SaveGameAsync(AutoSaveSlotIndex);
}

// ==========================================
// LOAD OPERATIONS
// ==========================================

bool UMGSaveSubsystem::LoadGame(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	OnLoadStarted.Broadcast(SlotIndex);

	FString SlotName = GetSaveSlotName(SlotIndex);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnLoadCompleted.Broadcast(SlotIndex, false);
		return false;
	}

	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UMGSaveGameObject* SaveGameObject = Cast<UMGSaveGameObject>(LoadedGame);

	if (!SaveGameObject)
	{
		OnLoadCompleted.Broadcast(SlotIndex, false);
		return false;
	}

	CurrentSaveData = SaveGameObject->SaveData;
	CurrentSlotIndex = SlotIndex;
	bHasUnsavedChanges = false;

	// Apply loaded state to game
	ApplyLoadedGameState(CurrentSaveData);

	OnLoadCompleted.Broadcast(SlotIndex, true);

	return true;
}

void UMGSaveSubsystem::LoadGameAsync(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		OnLoadCompleted.Broadcast(SlotIndex, false);
		return;
	}

	FString SlotName = GetSaveSlotName(SlotIndex);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnLoadCompleted.Broadcast(SlotIndex, false);
		return;
	}

	OnLoadStarted.Broadcast(SlotIndex);

	TWeakObjectPtr<UMGSaveSubsystem> WeakThis(this);
	UGameplayStatics::AsyncLoadGameFromSlot(SlotName, 0,
		FAsyncLoadGameFromSlotDelegate::CreateLambda([WeakThis, SlotIndex](const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGame)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}
			UMGSaveGameObject* SaveGameObject = Cast<UMGSaveGameObject>(LoadedGame);

			if (!SaveGameObject)
			{
				WeakThis->OnLoadCompleted.Broadcast(SlotIndex, false);
				return;
			}

			WeakThis->CurrentSaveData = SaveGameObject->SaveData;
			WeakThis->CurrentSlotIndex = SlotIndex;
			WeakThis->bHasUnsavedChanges = false;

			WeakThis->ApplyLoadedGameState(WeakThis->CurrentSaveData);

			WeakThis->OnLoadCompleted.Broadcast(SlotIndex, true);
		})
	);
}

bool UMGSaveSubsystem::QuickLoad()
{
	return LoadGame(QuickSaveSlotIndex);
}

// ==========================================
// SLOT MANAGEMENT
// ==========================================

TArray<FMGSaveSlotInfo> UMGSaveSubsystem::GetAllSaveSlots() const
{
	TArray<FMGSaveSlotInfo> Slots;

	for (int32 i = 0; i < MaxSaveSlots; ++i)
	{
		Slots.Add(GetSaveSlotInfo(i));
	}

	return Slots;
}

FMGSaveSlotInfo UMGSaveSubsystem::GetSaveSlotInfo(int32 SlotIndex) const
{
	FMGSaveSlotInfo Info;
	Info.SlotIndex = SlotIndex;

	FString SlotName = GetSaveSlotName(SlotIndex);

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		Info.bIsValid = false;
		return Info;
	}

	// Load just to get metadata
	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
	UMGSaveGameObject* SaveGameObject = Cast<UMGSaveGameObject>(LoadedGame);

	if (!SaveGameObject)
	{
		Info.bIsValid = false;
		return Info;
	}

	const FMGSaveGameData& Data = SaveGameObject->SaveData;

	Info.bIsValid = true;
	Info.SlotName = SlotName;
	Info.PlayerName = Data.Profile.PlayerName;
	Info.PlayerLevel = Data.Profile.Level;
	Info.TotalCash = Data.Profile.Cash;
	Info.TotalREP = Data.Profile.REP;
	Info.TotalVehicles = Data.OwnedVehicles.Num();
	Info.TotalPlaytime = Data.Profile.TotalPlaytime;
	Info.LastSaveTime = Data.SaveTime;
	Info.SaveVersion = Data.SaveVersion;

	// Get current vehicle name
	for (const FMGSaveVehicleData& Vehicle : Data.OwnedVehicles)
	{
		if (Vehicle.VehicleInstanceID == Data.CurrentVehicleID)
		{
			Info.CurrentVehicleName = Vehicle.CustomName.IsEmpty() ? Vehicle.VehicleDefinitionID.ToString() : Vehicle.CustomName;
			break;
		}
	}

	return Info;
}

bool UMGSaveSubsystem::IsSaveSlotValid(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	FString SlotName = GetSaveSlotName(SlotIndex);
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

bool UMGSaveSubsystem::DeleteSaveSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	FString SlotName = GetSaveSlotName(SlotIndex);
	bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, 0);

	OnSaveSlotDeleted.Broadcast(SlotIndex, bSuccess);

	return bSuccess;
}

bool UMGSaveSubsystem::CopySaveSlot(int32 SourceSlot, int32 DestSlot)
{
	if (SourceSlot < 0 || SourceSlot >= MaxSaveSlots ||
		DestSlot < 0 || DestSlot >= MaxSaveSlots)
	{
		return false;
	}

	FString SourceName = GetSaveSlotName(SourceSlot);

	if (!UGameplayStatics::DoesSaveGameExist(SourceName, 0))
	{
		return false;
	}

	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(SourceName, 0);
	UMGSaveGameObject* SaveGameObject = Cast<UMGSaveGameObject>(LoadedGame);

	if (!SaveGameObject)
	{
		return false;
	}

	FString DestName = GetSaveSlotName(DestSlot);
	return UGameplayStatics::SaveGameToSlot(SaveGameObject, DestName, 0);
}

// ==========================================
// PROFILE DATA
// ==========================================

void UMGSaveSubsystem::SetPlayerName(const FString& Name)
{
	CurrentSaveData.Profile.PlayerName = Name;
	MarkDirty();
}

FString UMGSaveSubsystem::GetPlayerName() const
{
	return CurrentSaveData.Profile.PlayerName;
}

void UMGSaveSubsystem::AddCash(int64 Amount)
{
	CurrentSaveData.Profile.Cash += Amount;
	MarkDirty();
}

bool UMGSaveSubsystem::SpendCash(int64 Amount)
{
	if (CurrentSaveData.Profile.Cash < Amount)
	{
		return false;
	}

	CurrentSaveData.Profile.Cash -= Amount;
	MarkDirty();
	return true;
}

int64 UMGSaveSubsystem::GetCurrentCash() const
{
	return CurrentSaveData.Profile.Cash;
}

void UMGSaveSubsystem::AddREP(int32 Amount)
{
	CurrentSaveData.Profile.REP += Amount;
	MarkDirty();
}

int32 UMGSaveSubsystem::GetCurrentREP() const
{
	return CurrentSaveData.Profile.REP;
}

void UMGSaveSubsystem::AddXP(int32 Amount)
{
	CurrentSaveData.Profile.TotalXP += Amount;

	// Simple leveling formula
	int32 NewLevel = 1 + (CurrentSaveData.Profile.TotalXP / 1000);
	if (NewLevel > CurrentSaveData.Profile.Level)
	{
		CurrentSaveData.Profile.Level = NewLevel;
	}

	MarkDirty();
}

int32 UMGSaveSubsystem::GetCurrentLevel() const
{
	return CurrentSaveData.Profile.Level;
}

void UMGSaveSubsystem::AddPlaytime(float Hours)
{
	CurrentSaveData.Profile.TotalPlaytime += Hours;
	MarkDirty();
}

void UMGSaveSubsystem::MarkDirty()
{
	bHasUnsavedChanges = true;
}

// ==========================================
// VEHICLE DATA
// ==========================================

FGuid UMGSaveSubsystem::AddOwnedVehicle(const FMGSaveVehicleData& VehicleData)
{
	FMGSaveVehicleData NewVehicle = VehicleData;

	if (!NewVehicle.VehicleInstanceID.IsValid())
	{
		NewVehicle.VehicleInstanceID = FGuid::NewGuid();
	}

	NewVehicle.PurchaseDate = FDateTime::Now();

	CurrentSaveData.OwnedVehicles.Add(NewVehicle);
	MarkDirty();

	return NewVehicle.VehicleInstanceID;
}

bool UMGSaveSubsystem::RemoveOwnedVehicle(const FGuid& VehicleInstanceID)
{
	for (int32 i = CurrentSaveData.OwnedVehicles.Num() - 1; i >= 0; --i)
	{
		if (CurrentSaveData.OwnedVehicles[i].VehicleInstanceID == VehicleInstanceID)
		{
			CurrentSaveData.OwnedVehicles.RemoveAt(i);
			MarkDirty();
			return true;
		}
	}
	return false;
}

TArray<FMGSaveVehicleData> UMGSaveSubsystem::GetOwnedVehicles() const
{
	return CurrentSaveData.OwnedVehicles;
}

FMGSaveVehicleData UMGSaveSubsystem::GetVehicleData(const FGuid& VehicleInstanceID) const
{
	for (const FMGSaveVehicleData& Vehicle : CurrentSaveData.OwnedVehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleInstanceID)
		{
			return Vehicle;
		}
	}
	return FMGSaveVehicleData();
}

bool UMGSaveSubsystem::UpdateVehicleData(const FMGSaveVehicleData& VehicleData)
{
	for (FMGSaveVehicleData& Vehicle : CurrentSaveData.OwnedVehicles)
	{
		if (Vehicle.VehicleInstanceID == VehicleData.VehicleInstanceID)
		{
			Vehicle = VehicleData;
			MarkDirty();
			return true;
		}
	}
	return false;
}

void UMGSaveSubsystem::SetCurrentVehicle(const FGuid& VehicleInstanceID)
{
	CurrentSaveData.CurrentVehicleID = VehicleInstanceID;
	MarkDirty();
}

FGuid UMGSaveSubsystem::GetCurrentVehicleID() const
{
	return CurrentSaveData.CurrentVehicleID;
}

FMGSaveVehicleData UMGSaveSubsystem::GetCurrentVehicle() const
{
	return GetVehicleData(CurrentSaveData.CurrentVehicleID);
}

// ==========================================
// PROGRESSION DATA
// ==========================================

void UMGSaveSubsystem::CompleteRace(FName TrackID, bool bWon)
{
	CurrentSaveData.Progression.TotalRacesCompleted++;

	if (bWon)
	{
		CurrentSaveData.Progression.TotalRacesWon++;

		int32& TrackWins = CurrentSaveData.Progression.RaceWinsByTrack.FindOrAdd(TrackID);
		TrackWins++;
	}

	MarkDirty();
}

void UMGSaveSubsystem::CompleteMission(FName MissionID)
{
	if (!CurrentSaveData.Progression.CompletedMissions.Contains(MissionID))
	{
		CurrentSaveData.Progression.CompletedMissions.Add(MissionID);
		MarkDirty();
	}
}

bool UMGSaveSubsystem::IsMissionComplete(FName MissionID) const
{
	return CurrentSaveData.Progression.CompletedMissions.Contains(MissionID);
}

void UMGSaveSubsystem::UnlockArea(FName AreaID)
{
	if (!CurrentSaveData.Progression.UnlockedAreas.Contains(AreaID))
	{
		CurrentSaveData.Progression.UnlockedAreas.Add(AreaID);
		MarkDirty();
	}
}

bool UMGSaveSubsystem::IsAreaUnlocked(FName AreaID) const
{
	return CurrentSaveData.Progression.UnlockedAreas.Contains(AreaID);
}

void UMGSaveSubsystem::UnlockAchievement(FName AchievementID)
{
	if (!CurrentSaveData.Progression.UnlockedAchievements.Contains(AchievementID))
	{
		CurrentSaveData.Progression.UnlockedAchievements.Add(AchievementID);
		MarkDirty();
	}
}

bool UMGSaveSubsystem::IsAchievementUnlocked(FName AchievementID) const
{
	return CurrentSaveData.Progression.UnlockedAchievements.Contains(AchievementID);
}

void UMGSaveSubsystem::UpdateAchievementProgress(FName AchievementID, int32 Progress)
{
	CurrentSaveData.Progression.AchievementProgress.Add(AchievementID, Progress);
	MarkDirty();
}

int32 UMGSaveSubsystem::GetAchievementProgress(FName AchievementID) const
{
	const int32* Progress = CurrentSaveData.Progression.AchievementProgress.Find(AchievementID);
	return Progress ? *Progress : 0;
}

void UMGSaveSubsystem::SetPersonalBest(const FMGTrackRecord& Record)
{
	// Find and update existing, or add new
	for (FMGTrackRecord& Existing : CurrentSaveData.Progression.PersonalBests)
	{
		if (Existing.TrackID == Record.TrackID && Existing.RaceType == Record.RaceType)
		{
			if (Record.BestLapTime < Existing.BestLapTime || Existing.BestLapTime <= 0.0f)
			{
				Existing = Record;
				MarkDirty();
			}
			return;
		}
	}

	CurrentSaveData.Progression.PersonalBests.Add(Record);
	MarkDirty();
}

FMGTrackRecord UMGSaveSubsystem::GetPersonalBest(FName TrackID, EMGRaceType RaceType) const
{
	for (const FMGTrackRecord& Record : CurrentSaveData.Progression.PersonalBests)
	{
		if (Record.TrackID == TrackID && Record.RaceType == RaceType)
		{
			return Record;
		}
	}
	return FMGTrackRecord();
}

// ==========================================
// INVENTORY
// ==========================================

void UMGSaveSubsystem::AddPartToInventory(FName PartID, int32 Quantity)
{
	int32& CurrentQuantity = CurrentSaveData.PartsInventory.FindOrAdd(PartID);
	CurrentQuantity += Quantity;
	MarkDirty();
}

bool UMGSaveSubsystem::RemovePartFromInventory(FName PartID, int32 Quantity)
{
	int32* CurrentQuantity = CurrentSaveData.PartsInventory.Find(PartID);

	if (!CurrentQuantity || *CurrentQuantity < Quantity)
	{
		return false;
	}

	*CurrentQuantity -= Quantity;

	if (*CurrentQuantity <= 0)
	{
		CurrentSaveData.PartsInventory.Remove(PartID);
	}

	MarkDirty();
	return true;
}

int32 UMGSaveSubsystem::GetPartQuantity(FName PartID) const
{
	const int32* Quantity = CurrentSaveData.PartsInventory.Find(PartID);
	return Quantity ? *Quantity : 0;
}

TMap<FName, int32> UMGSaveSubsystem::GetAllInventoryParts() const
{
	return CurrentSaveData.PartsInventory;
}

// ==========================================
// SETTINGS
// ==========================================

void UMGSaveSubsystem::SetGameSetting(const FString& Key, const FString& Value)
{
	CurrentSaveData.GameSettings.Add(Key, Value);
	MarkDirty();
}

FString UMGSaveSubsystem::GetGameSetting(const FString& Key, const FString& DefaultValue) const
{
	const FString* Value = CurrentSaveData.GameSettings.Find(Key);
	return Value ? *Value : DefaultValue;
}

void UMGSaveSubsystem::SetMetricUnits(bool bMetric)
{
	CurrentSaveData.Profile.bMetricUnits = bMetric;
	MarkDirty();
}

bool UMGSaveSubsystem::GetMetricUnits() const
{
	return CurrentSaveData.Profile.bMetricUnits;
}

void UMGSaveSubsystem::SetManualTransmission(bool bManual)
{
	CurrentSaveData.Profile.bManualTransmission = bManual;
	MarkDirty();
}

bool UMGSaveSubsystem::GetManualTransmission() const
{
	return CurrentSaveData.Profile.bManualTransmission;
}

// ==========================================
// AUTO-SAVE CONFIGURATION
// ==========================================

void UMGSaveSubsystem::SetAutoSaveEnabled(bool bEnabled)
{
	bAutoSaveEnabled = bEnabled;

	if (UWorld* World = GetWorld())
	{
		if (bEnabled)
		{
			TWeakObjectPtr<UMGSaveSubsystem> WeakThis(this);
			World->GetTimerManager().SetTimer(
				AutoSaveTimerHandle,
				[WeakThis]() { if (WeakThis.IsValid()) WeakThis->OnAutoSaveTimerTick(); },
				AutoSaveIntervalMinutes * 60.0f,
				true
			);
		}
		else
		{
			World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		}
	}
}

void UMGSaveSubsystem::SetAutoSaveInterval(float IntervalMinutes)
{
	AutoSaveIntervalMinutes = FMath::Max(1.0f, IntervalMinutes);

	// Restart timer with new interval
	if (bAutoSaveEnabled)
	{
		SetAutoSaveEnabled(true);
	}
}

// ==========================================
// NEW GAME
// ==========================================

void UMGSaveSubsystem::CreateNewGame(int32 SlotIndex, const FString& PlayerName)
{
	CurrentSaveData = GetDefaultSaveData();
	CurrentSaveData.Profile.PlayerName = PlayerName;
	CurrentSlotIndex = SlotIndex;
	bHasUnsavedChanges = true;

	// Save immediately
	SaveGame(SlotIndex);
}

FMGSaveGameData UMGSaveSubsystem::GetDefaultSaveData() const
{
	FMGSaveGameData Data;

	Data.SaveVersion = CurrentSaveVersion;
	Data.SaveTime = FDateTime::Now();
	Data.GameVersion = TEXT("1.0.0");

	// Default profile
	Data.Profile.PlayerName = TEXT("Racer");
	Data.Profile.Level = 1;
	Data.Profile.TotalXP = 0;
	Data.Profile.Cash = 10000; // Starting cash
	Data.Profile.REP = 0;
	Data.Profile.TotalPlaytime = 0.0f;
	Data.Profile.bMetricUnits = false;
	Data.Profile.bManualTransmission = false;
	Data.Profile.DifficultyLevel = 1; // Medium

	// Default unlocked areas
	Data.Progression.UnlockedAreas.Add(FName("Downtown"));
	Data.Progression.CurrentChapter = 1;

	return Data;
}

// ==========================================
// INTERNAL FUNCTIONS
// ==========================================

FString UMGSaveSubsystem::GetSaveSlotName(int32 SlotIndex) const
{
	return FString::Printf(TEXT("MidnightGrind_Save_%02d"), SlotIndex);
}

void UMGSaveSubsystem::GatherCurrentGameState(FMGSaveGameData& OutData)
{
	// This would gather state from various subsystems
	// For now, CurrentSaveData is kept up to date via the setter functions

	// Update save timestamp
	OutData.SaveTime = FDateTime::Now();
}

void UMGSaveSubsystem::ApplyLoadedGameState(const FMGSaveGameData& Data)
{
	// This would apply loaded state to various subsystems
	// For now, other subsystems read from CurrentSaveData via this subsystem

	// Could broadcast events to notify other systems
	// OnGameLoaded.Broadcast();
}

void UMGSaveSubsystem::OnAutoSaveTimerTick()
{
	if (bHasUnsavedChanges && bAutoSaveEnabled)
	{
		TriggerAutoSave();
	}
}
