// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Race Starter - Bridge between Garage/UI and Race Flow

#include "Race/MGRaceStarter.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGRaceStarter, Log, All);

// ==========================================
// RACE STARTER COMPONENT
// ==========================================

UMGRaceStarter::UMGRaceStarter()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Default quick race settings
	QuickRaceSettings.PreferredRaceType = FName("Circuit");
	QuickRaceSettings.DefaultLaps = 3;
	QuickRaceSettings.DefaultAICount = 5;
	QuickRaceSettings.DefaultDifficulty = 0.5f;
	QuickRaceSettings.bRandomizeTrack = true;
	QuickRaceSettings.bMidnightOnly = true;

	// Some default favorite tracks
	QuickRaceSettings.FavoriteTracks.Add(FName("Track_Downtown"));
	QuickRaceSettings.FavoriteTracks.Add(FName("Track_Highway"));
	QuickRaceSettings.FavoriteTracks.Add(FName("Track_Mountain"));
}

void UMGRaceStarter::BeginPlay()
{
	Super::BeginPlay();
	CacheSubsystems();
}

void UMGRaceStarter::CacheSubsystems()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>();
		RaceFlowSubsystem = GI->GetSubsystem<UMGRaceFlowSubsystem>();
	}
}

// ==========================================
// QUICK START FUNCTIONS
// ==========================================

bool UMGRaceStarter::StartQuickRace()
{
	CacheSubsystems();

	if (!CanStartRace())
	{
		ReportResult(false, TEXT("Cannot start race - another race may be in progress"));
		return false;
	}

	// Get selected vehicle
	FName VehicleID = GetSelectedVehicleID();
	if (VehicleID.IsNone())
	{
		ReportResult(false, TEXT("No vehicle selected in garage"));
		return false;
	}

	// Select track
	FName TrackID = SelectRandomTrack();
	if (TrackID.IsNone())
	{
		ReportResult(false, TEXT("No tracks available"));
		return false;
	}

	// Build setup
	FMGRaceSetupRequest Setup;
	Setup.TrackID = TrackID;
	Setup.PlayerVehicleID = VehicleID;
	ApplyQuickRaceSettings(Setup);
	FillDefaultValues(Setup);

	// Start race
	if (RaceFlowSubsystem.IsValid())
	{
		bool bResult = RaceFlowSubsystem->StartRace(Setup);
		ReportResult(bResult, bResult ? TEXT("Quick race started") : TEXT("Failed to start quick race"));
		return bResult;
	}

	ReportResult(false, TEXT("Race flow subsystem not available"));
	return false;
}

bool UMGRaceStarter::StartRaceOnTrack(FName TrackID)
{
	CacheSubsystems();

	if (!CanStartRace())
	{
		ReportResult(false, TEXT("Cannot start race"));
		return false;
	}

	FName VehicleID = GetSelectedVehicleID();
	if (VehicleID.IsNone())
	{
		ReportResult(false, TEXT("No vehicle selected"));
		return false;
	}

	FMGRaceSetupRequest Setup;
	Setup.TrackID = TrackID;
	Setup.PlayerVehicleID = VehicleID;
	ApplyQuickRaceSettings(Setup);
	FillDefaultValues(Setup);

	if (RaceFlowSubsystem.IsValid())
	{
		bool bResult = RaceFlowSubsystem->StartRace(Setup);
		ReportResult(bResult, bResult ? TEXT("Race started") : TEXT("Failed to start race"));
		return bResult;
	}

	ReportResult(false, TEXT("Race flow subsystem not available"));
	return false;
}

bool UMGRaceStarter::StartTestRace()
{
	CacheSubsystems();

	if (!CanStartRace())
	{
		ReportResult(false, TEXT("Cannot start race"));
		return false;
	}

	FMGRaceSetupRequest Setup = UMGRaceFlowSubsystem::GetTestRaceSetup();

	// Try to use garage vehicle if available
	FName VehicleID = GetSelectedVehicleID();
	if (!VehicleID.IsNone())
	{
		Setup.PlayerVehicleID = VehicleID;
	}

	if (RaceFlowSubsystem.IsValid())
	{
		bool bResult = RaceFlowSubsystem->StartRace(Setup);
		ReportResult(bResult, bResult ? TEXT("Test race started") : TEXT("Failed to start test race"));
		return bResult;
	}

	ReportResult(false, TEXT("Race flow subsystem not available"));
	return false;
}

bool UMGRaceStarter::StartCareerRace(FName EventID)
{
	CacheSubsystems();

	// MVP: Career races use standard race setup
	// TODO: Load event configuration from data asset

	FMGRaceSetupRequest Setup;
	Setup.RaceType = FName("Circuit");
	Setup.TrackID = FName("Track_Downtown");
	Setup.PlayerVehicleID = GetSelectedVehicleID();
	Setup.LapCount = 3;
	Setup.AICount = 7;
	Setup.AIDifficulty = 0.6f;
	Setup.BaseCashReward = 7500;
	Setup.BaseRepReward = 150;
	FillDefaultValues(Setup);

	if (RaceFlowSubsystem.IsValid())
	{
		bool bResult = RaceFlowSubsystem->StartRace(Setup);
		ReportResult(bResult, bResult ? TEXT("Career race started") : TEXT("Failed to start career race"));
		return bResult;
	}

	return false;
}

// ==========================================
// CUSTOM RACE SETUP
// ==========================================

void UMGRaceStarter::BeginCustomRace()
{
	bConfiguringRace = true;
	PendingSetup = FMGRaceSetupRequest();

	// Start with selected vehicle
	PendingSetup.PlayerVehicleID = GetSelectedVehicleID();
}

void UMGRaceStarter::SetTrack(FName TrackID)
{
	PendingSetup.TrackID = TrackID;
}

void UMGRaceStarter::SetRaceType(FName RaceType)
{
	PendingSetup.RaceType = RaceType;
}

void UMGRaceStarter::SetLapCount(int32 Laps)
{
	PendingSetup.LapCount = FMath::Clamp(Laps, 1, 99);
}

void UMGRaceStarter::SetAI(int32 Count, float Difficulty)
{
	PendingSetup.AICount = FMath::Clamp(Count, 0, 15);
	PendingSetup.AIDifficulty = FMath::Clamp(Difficulty, 0.0f, 1.0f);
}

void UMGRaceStarter::SetTimeOfDay(float Time)
{
	PendingSetup.TimeOfDay = FMath::Clamp(Time, 0.0f, 1.0f);
}

void UMGRaceStarter::SetWeather(float Weather)
{
	PendingSetup.Weather = FMath::Clamp(Weather, 0.0f, 1.0f);
}

void UMGRaceStarter::SetPinkSlip(bool bEnabled, FName OpponentVehicleID)
{
	PendingSetup.bIsPinkSlip = bEnabled;
	PendingSetup.PinkSlipVehicleID = OpponentVehicleID;
}

void UMGRaceStarter::SetVehicleOverride(FName VehicleID)
{
	PendingSetup.PlayerVehicleID = VehicleID;
}

bool UMGRaceStarter::CommitRace()
{
	if (!bConfiguringRace)
	{
		ReportResult(false, TEXT("No custom race configured"));
		return false;
	}

	CacheSubsystems();

	if (!CanStartRace())
	{
		ReportResult(false, TEXT("Cannot start race"));
		return false;
	}

	FillDefaultValues(PendingSetup);

	bool bResult = false;
	if (RaceFlowSubsystem.IsValid())
	{
		bResult = RaceFlowSubsystem->StartRace(PendingSetup);
	}

	bConfiguringRace = false;

	ReportResult(bResult, bResult ? TEXT("Custom race started") : TEXT("Failed to start custom race"));
	return bResult;
}

void UMGRaceStarter::CancelCustomRace()
{
	bConfiguringRace = false;
	PendingSetup = FMGRaceSetupRequest();
}

// ==========================================
// STATUS
// ==========================================

bool UMGRaceStarter::CanStartRace() const
{
	if (!RaceFlowSubsystem.IsValid())
	{
		const_cast<UMGRaceStarter*>(this)->CacheSubsystems();
	}

	if (RaceFlowSubsystem.IsValid())
	{
		return RaceFlowSubsystem->CanStartRace();
	}

	return false;
}

FName UMGRaceStarter::GetSelectedVehicleID() const
{
	if (!GarageSubsystem.IsValid())
	{
		const_cast<UMGRaceStarter*>(this)->CacheSubsystems();
	}

	if (GarageSubsystem.IsValid())
	{
		FMGOwnedVehicle Vehicle;
		if (GarageSubsystem->GetSelectedVehicle(Vehicle))
		{
			// Convert GUID to FName for race setup
			// In full implementation, use the vehicle model ID
			return FName(*Vehicle.VehicleId.ToString());
		}
	}

	return NAME_None;
}

TArray<FName> UMGRaceStarter::GetAvailableTracks() const
{
	if (!RaceFlowSubsystem.IsValid())
	{
		const_cast<UMGRaceStarter*>(this)->CacheSubsystems();
	}

	if (RaceFlowSubsystem.IsValid())
	{
		return RaceFlowSubsystem->GetAvailableTracks();
	}

	return TArray<FName>();
}

// ==========================================
// HELPERS
// ==========================================

FName UMGRaceStarter::SelectRandomTrack() const
{
	if (QuickRaceSettings.bRandomizeTrack)
	{
		// Prefer favorites if available
		if (QuickRaceSettings.FavoriteTracks.Num() > 0)
		{
			int32 Index = FMath::RandRange(0, QuickRaceSettings.FavoriteTracks.Num() - 1);
			return QuickRaceSettings.FavoriteTracks[Index];
		}

		// Otherwise pick from all tracks
		TArray<FName> AllTracks = GetAvailableTracks();
		if (AllTracks.Num() > 0)
		{
			int32 Index = FMath::RandRange(0, AllTracks.Num() - 1);
			return AllTracks[Index];
		}
	}
	else if (QuickRaceSettings.FavoriteTracks.Num() > 0)
	{
		// Return first favorite
		return QuickRaceSettings.FavoriteTracks[0];
	}

	// Fallback
	return FName("Track_Downtown");
}

void UMGRaceStarter::ApplyQuickRaceSettings(FMGRaceSetupRequest& Setup)
{
	Setup.RaceType = QuickRaceSettings.PreferredRaceType;
	Setup.LapCount = QuickRaceSettings.DefaultLaps;
	Setup.AICount = QuickRaceSettings.DefaultAICount;
	Setup.AIDifficulty = QuickRaceSettings.DefaultDifficulty;

	if (QuickRaceSettings.bMidnightOnly)
	{
		Setup.TimeOfDay = 0.0f;
	}
	else
	{
		Setup.TimeOfDay = FMath::FRandRange(0.0f, 1.0f);
	}
}

void UMGRaceStarter::FillDefaultValues(FMGRaceSetupRequest& Setup)
{
	// Fill any missing values with sensible defaults
	if (Setup.RaceType.IsNone())
	{
		Setup.RaceType = FName("Circuit");
	}

	if (Setup.LapCount <= 0)
	{
		Setup.LapCount = 3;
	}

	if (Setup.BaseCashReward <= 0)
	{
		Setup.BaseCashReward = 5000;
	}

	if (Setup.BaseRepReward <= 0)
	{
		Setup.BaseRepReward = 100;
	}
}

void UMGRaceStarter::ReportResult(bool bSuccess, const FString& Message)
{
	if (bSuccess)
	{
		UE_LOG(LogMGRaceStarter, Log, TEXT("%s"), *Message);
	}
	else
	{
		UE_LOG(LogMGRaceStarter, Warning, TEXT("%s"), *Message);
	}

	OnRaceStartResult.Broadcast(bSuccess, Message);
}

// ==========================================
// BLUEPRINT FUNCTION LIBRARY
// ==========================================

bool UMGRaceStarterLibrary::StartQuickRace(UObject* WorldContextObject, FName TrackID)
{
	UMGRaceFlowSubsystem* RaceFlow = GetRaceFlowSubsystem(WorldContextObject);
	if (!RaceFlow)
	{
		return false;
	}

	FName VehicleID = GetSelectedVehicleID(WorldContextObject);
	if (VehicleID.IsNone())
	{
		// Use test vehicle
		VehicleID = FName("Vehicle_240SX");
	}

	if (TrackID.IsNone())
	{
		TrackID = FName("Track_Downtown");
	}

	return RaceFlow->StartQuickRace(TrackID, VehicleID);
}

bool UMGRaceStarterLibrary::StartRace(UObject* WorldContextObject, const FMGRaceSetupRequest& Setup)
{
	UMGRaceFlowSubsystem* RaceFlow = GetRaceFlowSubsystem(WorldContextObject);
	if (!RaceFlow)
	{
		return false;
	}

	return RaceFlow->StartRace(Setup);
}

bool UMGRaceStarterLibrary::StartTestRace(UObject* WorldContextObject)
{
	UMGRaceFlowSubsystem* RaceFlow = GetRaceFlowSubsystem(WorldContextObject);
	if (!RaceFlow)
	{
		return false;
	}

	FMGRaceSetupRequest Setup = UMGRaceFlowSubsystem::GetTestRaceSetup();

	// Try to use garage vehicle
	FName VehicleID = GetSelectedVehicleID(WorldContextObject);
	if (!VehicleID.IsNone())
	{
		Setup.PlayerVehicleID = VehicleID;
	}

	return RaceFlow->StartRace(Setup);
}

UMGRaceFlowSubsystem* UMGRaceStarterLibrary::GetRaceFlowSubsystem(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		return nullptr;
	}

	return GI->GetSubsystem<UMGRaceFlowSubsystem>();
}

bool UMGRaceStarterLibrary::CanStartRace(UObject* WorldContextObject)
{
	UMGRaceFlowSubsystem* RaceFlow = GetRaceFlowSubsystem(WorldContextObject);
	return RaceFlow && RaceFlow->CanStartRace();
}

FMGRaceFlowResult UMGRaceStarterLibrary::GetLastRaceResult(UObject* WorldContextObject)
{
	UMGRaceFlowSubsystem* RaceFlow = GetRaceFlowSubsystem(WorldContextObject);
	if (RaceFlow)
	{
		return RaceFlow->GetLastResult();
	}
	return FMGRaceFlowResult();
}

FName UMGRaceStarterLibrary::GetSelectedVehicleID(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return NAME_None;
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GI)
	{
		return NAME_None;
	}

	UMGGarageSubsystem* Garage = GI->GetSubsystem<UMGGarageSubsystem>();
	if (!Garage)
	{
		return NAME_None;
	}

	FMGOwnedVehicle Vehicle;
	if (Garage->GetSelectedVehicle(Vehicle))
	{
		return FName(*Vehicle.VehicleId.ToString());
	}

	return NAME_None;
}
