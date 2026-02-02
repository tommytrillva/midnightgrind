// Copyright Midnight Grind. All Rights Reserved.

#include "Race/MGRaceHistorySubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UMGRaceHistorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadHistory();
}

void UMGRaceHistorySubsystem::Deinitialize()
{
	SaveHistory();
	Super::Deinitialize();
}

// ============================================================================
// RECORDING RESULTS
// ============================================================================

void UMGRaceHistorySubsystem::RecordRaceResult(const FMGRaceResult& Result)
{
	// Add to history (newest first)
	RaceHistory.Insert(Result, 0);

	// Trim history if too large
	while (RaceHistory.Num() > MaxHistoryEntries)
	{
		RaceHistory.Pop();
	}

	// Update all statistics
	UpdateLifetimeStats(Result);
	UpdateTrackStats(Result);
	UpdateVehicleStats(Result);
	UpdateStreaks(Result);
	CheckPersonalBests(Result);

	// Broadcast event
	OnRaceResultRecorded.Broadcast(Result);

	// Auto-save after each race
	SaveHistory();
}

FMGRaceResult UMGRaceHistorySubsystem::CreateAndRecordResult(
	const FString& TrackId,
	const FText& TrackName,
	FName RaceType,
	int32 Position,
	int32 TotalRacers,
	float RaceTime,
	float BestLapTime,
	FName VehicleId,
	const FText& VehicleName,
	bool bWasCleanRace)
{
	FMGRaceResult Result;
	Result.RaceId = FGuid::NewGuid();
	Result.TrackId = TrackId;
	Result.TrackName = TrackName;
	Result.RaceType = RaceType;
	Result.Position = Position;
	Result.TotalRacers = TotalRacers;
	Result.RaceTime = RaceTime;
	Result.BestLapTime = BestLapTime;
	Result.VehicleId = VehicleId;
	Result.VehicleName = VehicleName;
	Result.bWasCleanRace = bWasCleanRace;
	Result.Timestamp = FDateTime::Now();

	RecordRaceResult(Result);
	return Result;
}

// ============================================================================
// QUERYING HISTORY
// ============================================================================

TArray<FMGRaceResult> UMGRaceHistorySubsystem::GetRecentResults(int32 Count) const
{
	TArray<FMGRaceResult> Results;
	int32 NumToCopy = FMath::Min(Count, RaceHistory.Num());

	for (int32 i = 0; i < NumToCopy; ++i)
	{
		Results.Add(RaceHistory[i]);
	}

	return Results;
}

TArray<FMGRaceResult> UMGRaceHistorySubsystem::GetResultsForTrack(const FString& TrackId) const
{
	TArray<FMGRaceResult> Results;

	for (const FMGRaceResult& Result : RaceHistory)
	{
		if (Result.TrackId == TrackId)
		{
			Results.Add(Result);
		}
	}

	return Results;
}

TArray<FMGRaceResult> UMGRaceHistorySubsystem::GetResultsForVehicle(FName VehicleId) const
{
	TArray<FMGRaceResult> Results;

	for (const FMGRaceResult& Result : RaceHistory)
	{
		if (Result.VehicleId == VehicleId)
		{
			Results.Add(Result);
		}
	}

	return Results;
}

TArray<FMGRaceResult> UMGRaceHistorySubsystem::GetWins() const
{
	TArray<FMGRaceResult> Wins;

	for (const FMGRaceResult& Result : RaceHistory)
	{
		if (Result.IsWin())
		{
			Wins.Add(Result);
		}
	}

	return Wins;
}

bool UMGRaceHistorySubsystem::GetResultById(const FGuid& RaceId, FMGRaceResult& OutResult) const
{
	for (const FMGRaceResult& Result : RaceHistory)
	{
		if (Result.RaceId == RaceId)
		{
			OutResult = Result;
			return true;
		}
	}

	return false;
}

// ============================================================================
// STATISTICS
// ============================================================================

FMGTrackStats UMGRaceHistorySubsystem::GetTrackStats(const FString& TrackId) const
{
	if (const FMGTrackStats* Stats = TrackStatsMap.Find(TrackId))
	{
		return *Stats;
	}

	return FMGTrackStats();
}

FMGVehicleRaceStats UMGRaceHistorySubsystem::GetVehicleStats(FName VehicleId) const
{
	if (const FMGVehicleRaceStats* Stats = VehicleStatsMap.Find(VehicleId))
	{
		return *Stats;
	}

	return FMGVehicleRaceStats();
}

TArray<FMGTrackStats> UMGRaceHistorySubsystem::GetMostRacedTracks(int32 Count) const
{
	TArray<FMGTrackStats> AllStats;
	TrackStatsMap.GenerateValueArray(AllStats);

	// Sort by total races
	AllStats.Sort([](const FMGTrackStats& A, const FMGTrackStats& B)
	{
		return A.TotalRaces > B.TotalRaces;
	});

	// Return top N
	if (AllStats.Num() > Count)
	{
		AllStats.SetNum(Count);
	}

	return AllStats;
}

TArray<FMGVehicleRaceStats> UMGRaceHistorySubsystem::GetMostSuccessfulVehicles(int32 Count) const
{
	TArray<FMGVehicleRaceStats> AllStats;
	VehicleStatsMap.GenerateValueArray(AllStats);

	// Sort by win rate (with minimum races threshold)
	AllStats.Sort([](const FMGVehicleRaceStats& A, const FMGVehicleRaceStats& B)
	{
		// Require at least 3 races to be considered
		bool bAQualified = A.TotalRaces >= 3;
		bool bBQualified = B.TotalRaces >= 3;

		if (bAQualified != bBQualified)
		{
			return bAQualified;
		}

		return A.WinRate > B.WinRate;
	});

	if (AllStats.Num() > Count)
	{
		AllStats.SetNum(Count);
	}

	return AllStats;
}

// ============================================================================
// PERSONAL BESTS
// ============================================================================

float UMGRaceHistorySubsystem::GetPersonalBestTime(const FString& TrackId) const
{
	if (const float* BestTime = PersonalBestTimes.Find(TrackId))
	{
		return *BestTime;
	}

	return 0.0f;
}

float UMGRaceHistorySubsystem::GetPersonalBestLap(const FString& TrackId) const
{
	if (const float* BestLap = PersonalBestLaps.Find(TrackId))
	{
		return *BestLap;
	}

	return 0.0f;
}

bool UMGRaceHistorySubsystem::IsNewPersonalBest(const FString& TrackId, float Time) const
{
	float CurrentBest = GetPersonalBestTime(TrackId);
	return CurrentBest <= 0.0f || Time < CurrentBest;
}

// ============================================================================
// INTERNAL UPDATES
// ============================================================================

void UMGRaceHistorySubsystem::UpdateLifetimeStats(const FMGRaceResult& Result)
{
	LifetimeStats.TotalRaces++;

	if (Result.IsWin())
	{
		LifetimeStats.TotalWins++;
	}

	if (Result.IsPodium())
	{
		LifetimeStats.TotalPodiums++;
	}

	if (Result.bDNF)
	{
		LifetimeStats.TotalDNFs++;
	}

	if (Result.bWasCleanRace)
	{
		LifetimeStats.CleanRaces++;
	}

	LifetimeStats.TotalDistanceKM += Result.DistanceM / 1000.0;
	LifetimeStats.TotalRaceTimeHours += Result.RaceTime / 3600.0;

	if (Result.TopSpeedKPH > LifetimeStats.HighestTopSpeedKPH)
	{
		LifetimeStats.HighestTopSpeedKPH = Result.TopSpeedKPH;
	}

	LifetimeStats.TotalCashEarned += Result.CashEarned;
	LifetimeStats.TotalReputationEarned += Result.ReputationEarned;
	LifetimeStats.TotalXPEarned += Result.XPEarned;

	if (Result.bWasOnlineRace)
	{
		LifetimeStats.OnlineRaces++;
		if (Result.IsWin())
		{
			LifetimeStats.OnlineWins++;
		}
	}
}

void UMGRaceHistorySubsystem::UpdateTrackStats(const FMGRaceResult& Result)
{
	FMGTrackStats& Stats = TrackStatsMap.FindOrAdd(Result.TrackId);
	Stats.TrackId = Result.TrackId;
	Stats.TotalRaces++;

	if (Result.IsWin())
	{
		Stats.Wins++;
	}

	if (Result.IsPodium())
	{
		Stats.Podiums++;
	}

	// Update best times
	if (Result.RaceTime > 0.0f && !Result.bDNF)
	{
		if (Stats.BestTime <= 0.0f || Result.RaceTime < Stats.BestTime)
		{
			Stats.BestTime = Result.RaceTime;
		}
	}

	if (Result.BestLapTime > 0.0f)
	{
		if (Stats.BestLapTime <= 0.0f || Result.BestLapTime < Stats.BestLapTime)
		{
			Stats.BestLapTime = Result.BestLapTime;
		}
	}

	// Recalculate average position
	float TotalPosition = Stats.AveragePosition * (Stats.TotalRaces - 1) + Result.Position;
	Stats.AveragePosition = TotalPosition / Stats.TotalRaces;

	Stats.LastRaced = Result.Timestamp;
}

void UMGRaceHistorySubsystem::UpdateVehicleStats(const FMGRaceResult& Result)
{
	FMGVehicleRaceStats& Stats = VehicleStatsMap.FindOrAdd(Result.VehicleId);
	Stats.VehicleId = Result.VehicleId;
	Stats.TotalRaces++;

	if (Result.IsWin())
	{
		Stats.Wins++;
	}

	if (Result.IsPodium())
	{
		Stats.Podiums++;
	}

	Stats.TotalDistanceKM += Result.DistanceM / 1000.0f;
	Stats.WinRate = Stats.TotalRaces > 0 ? (float)Stats.Wins / Stats.TotalRaces : 0.0f;

	if (Result.TopSpeedKPH > Stats.TopSpeedRecordKPH)
	{
		Stats.TopSpeedRecordKPH = Result.TopSpeedKPH;
	}
}

void UMGRaceHistorySubsystem::UpdateStreaks(const FMGRaceResult& Result)
{
	// Win streak
	if (Result.IsWin())
	{
		LifetimeStats.CurrentWinStreak++;
		if (LifetimeStats.CurrentWinStreak > LifetimeStats.BestWinStreak)
		{
			LifetimeStats.BestWinStreak = LifetimeStats.CurrentWinStreak;
			OnWinStreakUpdated.Broadcast(LifetimeStats.CurrentWinStreak);
		}
	}
	else
	{
		LifetimeStats.CurrentWinStreak = 0;
	}

	// Podium streak
	if (Result.IsPodium())
	{
		LifetimeStats.CurrentPodiumStreak++;
		if (LifetimeStats.CurrentPodiumStreak > LifetimeStats.BestPodiumStreak)
		{
			LifetimeStats.BestPodiumStreak = LifetimeStats.CurrentPodiumStreak;
		}
	}
	else
	{
		LifetimeStats.CurrentPodiumStreak = 0;
	}
}

void UMGRaceHistorySubsystem::CheckPersonalBests(const FMGRaceResult& Result)
{
	if (Result.bDNF || Result.RaceTime <= 0.0f)
	{
		return;
	}

	// Check race time
	float* CurrentBest = PersonalBestTimes.Find(Result.TrackId);
	if (!CurrentBest || Result.RaceTime < *CurrentBest)
	{
		float OldBest = CurrentBest ? *CurrentBest : 0.0f;
		PersonalBestTimes.Add(Result.TrackId, Result.RaceTime);
		OnNewPersonalBest.Broadcast(Result.TrackId, Result.RaceTime);
	}

	// Check lap time
	if (Result.BestLapTime > 0.0f)
	{
		float* CurrentBestLap = PersonalBestLaps.Find(Result.TrackId);
		if (!CurrentBestLap || Result.BestLapTime < *CurrentBestLap)
		{
			PersonalBestLaps.Add(Result.TrackId, Result.BestLapTime);
		}
	}
}

// ============================================================================
// PERSISTENCE
// ============================================================================

void UMGRaceHistorySubsystem::SaveHistory()
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("RaceHistory.json");

	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	// Save version
	RootObject->SetNumberField(TEXT("version"), 1);

	// Save lifetime stats
	TSharedPtr<FJsonObject> StatsObject = MakeShareable(new FJsonObject);
	StatsObject->SetNumberField(TEXT("totalRaces"), LifetimeStats.TotalRaces);
	StatsObject->SetNumberField(TEXT("totalWins"), LifetimeStats.TotalWins);
	StatsObject->SetNumberField(TEXT("totalPodiums"), LifetimeStats.TotalPodiums);
	StatsObject->SetNumberField(TEXT("totalDNFs"), LifetimeStats.TotalDNFs);
	StatsObject->SetNumberField(TEXT("cleanRaces"), LifetimeStats.CleanRaces);
	StatsObject->SetNumberField(TEXT("currentWinStreak"), LifetimeStats.CurrentWinStreak);
	StatsObject->SetNumberField(TEXT("bestWinStreak"), LifetimeStats.BestWinStreak);
	StatsObject->SetNumberField(TEXT("currentPodiumStreak"), LifetimeStats.CurrentPodiumStreak);
	StatsObject->SetNumberField(TEXT("bestPodiumStreak"), LifetimeStats.BestPodiumStreak);
	StatsObject->SetNumberField(TEXT("totalDistanceKM"), LifetimeStats.TotalDistanceKM);
	StatsObject->SetNumberField(TEXT("totalRaceTimeHours"), LifetimeStats.TotalRaceTimeHours);
	StatsObject->SetNumberField(TEXT("highestTopSpeedKPH"), LifetimeStats.HighestTopSpeedKPH);
	StatsObject->SetNumberField(TEXT("totalCashEarned"), LifetimeStats.TotalCashEarned);
	StatsObject->SetNumberField(TEXT("totalReputationEarned"), LifetimeStats.TotalReputationEarned);
	StatsObject->SetNumberField(TEXT("totalXPEarned"), LifetimeStats.TotalXPEarned);
	StatsObject->SetNumberField(TEXT("onlineRaces"), LifetimeStats.OnlineRaces);
	StatsObject->SetNumberField(TEXT("onlineWins"), LifetimeStats.OnlineWins);
	RootObject->SetObjectField(TEXT("lifetimeStats"), StatsObject);

	// Save personal bests
	TSharedPtr<FJsonObject> BestsObject = MakeShareable(new FJsonObject);
	for (const auto& Pair : PersonalBestTimes)
	{
		BestsObject->SetNumberField(Pair.Key, Pair.Value);
	}
	RootObject->SetObjectField(TEXT("personalBestTimes"), BestsObject);

	TSharedPtr<FJsonObject> BestLapsObject = MakeShareable(new FJsonObject);
	for (const auto& Pair : PersonalBestLaps)
	{
		BestLapsObject->SetNumberField(Pair.Key, Pair.Value);
	}
	RootObject->SetObjectField(TEXT("personalBestLaps"), BestLapsObject);

	// Save recent race history (just the last 100 for file size)
	TArray<TSharedPtr<FJsonValue>> HistoryArray;
	int32 HistoryToSave = FMath::Min(100, RaceHistory.Num());
	for (int32 i = 0; i < HistoryToSave; ++i)
	{
		const FMGRaceResult& Result = RaceHistory[i];
		TSharedPtr<FJsonObject> RaceObj = MakeShareable(new FJsonObject);

		RaceObj->SetStringField(TEXT("raceId"), Result.RaceId.ToString());
		RaceObj->SetStringField(TEXT("trackId"), Result.TrackId);
		RaceObj->SetStringField(TEXT("trackName"), Result.TrackName.ToString());
		RaceObj->SetStringField(TEXT("raceType"), Result.RaceType.ToString());
		RaceObj->SetNumberField(TEXT("position"), Result.Position);
		RaceObj->SetNumberField(TEXT("totalRacers"), Result.TotalRacers);
		RaceObj->SetNumberField(TEXT("raceTime"), Result.RaceTime);
		RaceObj->SetNumberField(TEXT("bestLapTime"), Result.BestLapTime);
		RaceObj->SetStringField(TEXT("vehicleId"), Result.VehicleId.ToString());
		RaceObj->SetStringField(TEXT("vehicleName"), Result.VehicleName.ToString());
		RaceObj->SetBoolField(TEXT("wasCleanRace"), Result.bWasCleanRace);
		RaceObj->SetBoolField(TEXT("dnf"), Result.bDNF);
		RaceObj->SetStringField(TEXT("timestamp"), Result.Timestamp.ToString());
		RaceObj->SetNumberField(TEXT("cashEarned"), Result.CashEarned);
		RaceObj->SetNumberField(TEXT("reputationEarned"), Result.ReputationEarned);
		RaceObj->SetBoolField(TEXT("wasOnlineRace"), Result.bWasOnlineRace);

		HistoryArray.Add(MakeShareable(new FJsonValueObject(RaceObj)));
	}
	RootObject->SetArrayField(TEXT("raceHistory"), HistoryArray);

	// Serialize and save
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		FFileHelper::SaveStringToFile(JsonString, *SavePath);
	}
}

void UMGRaceHistorySubsystem::LoadHistory()
{
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("RaceHistory.json");

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *SavePath))
	{
		// No save file, start fresh
		return;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return;
	}

	// Load lifetime stats
	const TSharedPtr<FJsonObject>* StatsObject;
	if (RootObject->TryGetObjectField(TEXT("lifetimeStats"), StatsObject))
	{
		(*StatsObject)->TryGetNumberField(TEXT("totalRaces"), LifetimeStats.TotalRaces);
		(*StatsObject)->TryGetNumberField(TEXT("totalWins"), LifetimeStats.TotalWins);
		(*StatsObject)->TryGetNumberField(TEXT("totalPodiums"), LifetimeStats.TotalPodiums);
		(*StatsObject)->TryGetNumberField(TEXT("totalDNFs"), LifetimeStats.TotalDNFs);
		(*StatsObject)->TryGetNumberField(TEXT("cleanRaces"), LifetimeStats.CleanRaces);
		(*StatsObject)->TryGetNumberField(TEXT("currentWinStreak"), LifetimeStats.CurrentWinStreak);
		(*StatsObject)->TryGetNumberField(TEXT("bestWinStreak"), LifetimeStats.BestWinStreak);
		(*StatsObject)->TryGetNumberField(TEXT("currentPodiumStreak"), LifetimeStats.CurrentPodiumStreak);
		(*StatsObject)->TryGetNumberField(TEXT("bestPodiumStreak"), LifetimeStats.BestPodiumStreak);
		(*StatsObject)->TryGetNumberField(TEXT("highestTopSpeedKPH"), LifetimeStats.HighestTopSpeedKPH);

		int64 Cash, Rep;
		int32 XP;
		(*StatsObject)->TryGetNumberField(TEXT("totalCashEarned"), Cash);
		(*StatsObject)->TryGetNumberField(TEXT("totalReputationEarned"), Rep);
		(*StatsObject)->TryGetNumberField(TEXT("totalXPEarned"), XP);
		LifetimeStats.TotalCashEarned = Cash;
		LifetimeStats.TotalReputationEarned = Rep;
		LifetimeStats.TotalXPEarned = XP;

		(*StatsObject)->TryGetNumberField(TEXT("onlineRaces"), LifetimeStats.OnlineRaces);
		(*StatsObject)->TryGetNumberField(TEXT("onlineWins"), LifetimeStats.OnlineWins);
	}

	// Load personal bests
	const TSharedPtr<FJsonObject>* BestsObject;
	if (RootObject->TryGetObjectField(TEXT("personalBestTimes"), BestsObject))
	{
		for (const auto& Pair : (*BestsObject)->Values)
		{
			double Time;
			if (Pair.Value->TryGetNumber(Time))
			{
				PersonalBestTimes.Add(Pair.Key, static_cast<float>(Time));
			}
		}
	}

	const TSharedPtr<FJsonObject>* BestLapsObject;
	if (RootObject->TryGetObjectField(TEXT("personalBestLaps"), BestLapsObject))
	{
		for (const auto& Pair : (*BestLapsObject)->Values)
		{
			double Time;
			if (Pair.Value->TryGetNumber(Time))
			{
				PersonalBestLaps.Add(Pair.Key, static_cast<float>(Time));
			}
		}
	}

	// Load race history
	const TArray<TSharedPtr<FJsonValue>>* HistoryArray;
	if (RootObject->TryGetArrayField(TEXT("raceHistory"), HistoryArray))
	{
		for (const TSharedPtr<FJsonValue>& Value : *HistoryArray)
		{
			const TSharedPtr<FJsonObject>* RaceObj;
			if (Value->TryGetObject(RaceObj))
			{
				FMGRaceResult Result;

				FString RaceIdStr;
				if ((*RaceObj)->TryGetStringField(TEXT("raceId"), RaceIdStr))
				{
					FGuid::Parse(RaceIdStr, Result.RaceId);
				}

				(*RaceObj)->TryGetStringField(TEXT("trackId"), Result.TrackId);

				FString TrackNameStr;
				if ((*RaceObj)->TryGetStringField(TEXT("trackName"), TrackNameStr))
				{
					Result.TrackName = FText::FromString(TrackNameStr);
				}

				FString RaceTypeStr;
				if ((*RaceObj)->TryGetStringField(TEXT("raceType"), RaceTypeStr))
				{
					Result.RaceType = FName(*RaceTypeStr);
				}

				(*RaceObj)->TryGetNumberField(TEXT("position"), Result.Position);
				(*RaceObj)->TryGetNumberField(TEXT("totalRacers"), Result.TotalRacers);
				(*RaceObj)->TryGetNumberField(TEXT("raceTime"), Result.RaceTime);
				(*RaceObj)->TryGetNumberField(TEXT("bestLapTime"), Result.BestLapTime);

				FString VehicleIdStr;
				if ((*RaceObj)->TryGetStringField(TEXT("vehicleId"), VehicleIdStr))
				{
					Result.VehicleId = FName(*VehicleIdStr);
				}

				FString VehicleNameStr;
				if ((*RaceObj)->TryGetStringField(TEXT("vehicleName"), VehicleNameStr))
				{
					Result.VehicleName = FText::FromString(VehicleNameStr);
				}

				(*RaceObj)->TryGetBoolField(TEXT("wasCleanRace"), Result.bWasCleanRace);
				(*RaceObj)->TryGetBoolField(TEXT("dnf"), Result.bDNF);
				(*RaceObj)->TryGetBoolField(TEXT("wasOnlineRace"), Result.bWasOnlineRace);

				FString TimestampStr;
				if ((*RaceObj)->TryGetStringField(TEXT("timestamp"), TimestampStr))
				{
					FDateTime::Parse(TimestampStr, Result.Timestamp);
				}

				int64 Cash, Rep;
				(*RaceObj)->TryGetNumberField(TEXT("cashEarned"), Cash);
				(*RaceObj)->TryGetNumberField(TEXT("reputationEarned"), Rep);
				Result.CashEarned = Cash;
				Result.ReputationEarned = Rep;

				RaceHistory.Add(Result);
			}
		}
	}

	// Rebuild track and vehicle stats from history
	for (const FMGRaceResult& Result : RaceHistory)
	{
		UpdateTrackStats(Result);
		UpdateVehicleStats(Result);
	}
}

void UMGRaceHistorySubsystem::ClearHistory()
{
	RaceHistory.Empty();
	LifetimeStats = FMGLifetimeStats();
	TrackStatsMap.Empty();
	VehicleStatsMap.Empty();
	PersonalBestTimes.Empty();
	PersonalBestLaps.Empty();

	SaveHistory();
}
