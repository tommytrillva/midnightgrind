// Copyright Midnight Grind. All Rights Reserved.

#include "Stats/MGStatsTracker.h"

void UMGStatsTracker::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize first play date if not set
	if (PlayerStats.Time.FirstPlayDate.GetTicks() == 0)
	{
		PlayerStats.Time.FirstPlayDate = FDateTime::UtcNow();
	}

	RecordSessionStart();
}

void UMGStatsTracker::Deinitialize()
{
	RecordSessionEnd();
	Super::Deinitialize();
}

FMGVehicleRacingStats UMGStatsTracker::GetVehicleStats(FName VehicleID) const
{
	if (const FMGVehicleRacingStats* Stats = VehicleStatsMap.Find(VehicleID))
	{
		return *Stats;
	}
	FMGVehicleRacingStats Empty;
	Empty.VehicleID = VehicleID;
	return Empty;
}

FMGTrackStats UMGStatsTracker::GetTrackStats(FName TrackID) const
{
	if (const FMGTrackStats* Stats = TrackStatsMap.Find(TrackID))
	{
		return *Stats;
	}
	FMGTrackStats Empty;
	Empty.TrackID = TrackID;
	return Empty;
}

void UMGStatsTracker::RecordRaceResult(FName TrackID, FName VehicleID, int32 Position, int32 TotalRacers,
	float RaceTime, float BestLap, float DriftScore, bool bPinkSlip)
{
	PlayerStats.Race.TotalRaces++;
	PlayerStats.Race.TotalRaceTime += RaceTime;

	bool bWon = (Position == 1);
	bool bPodium = (Position <= 3);
	bool bComebackWin = bWon && (TotalRacers > 4); // Simplified check

	if (bWon)
	{
		PlayerStats.Race.Wins++;
		PlayerStats.Race.CurrentWinStreak++;
		PlayerStats.Race.BestWinStreak = FMath::Max(PlayerStats.Race.BestWinStreak, PlayerStats.Race.CurrentWinStreak);

		if (bComebackWin)
		{
			PlayerStats.Race.ComebackWins++;
		}
	}
	else
	{
		PlayerStats.Race.CurrentWinStreak = 0;
	}

	if (bPodium)
	{
		PlayerStats.Race.Podiums++;
	}

	if (bPinkSlip)
	{
		if (bWon)
		{
			PlayerStats.Race.PinkSlipsWon++;
		}
		else
		{
			PlayerStats.Race.PinkSlipsLost++;
		}
	}

	// Best lap tracking
	if (BestLap > 0.0f)
	{
		if (PlayerStats.Race.BestLapTime <= 0.0f || BestLap < PlayerStats.Race.BestLapTime)
		{
			PlayerStats.Race.BestLapTime = BestLap;
			PlayerStats.Race.BestLapTrack = TrackID;
		}
	}

	// Update vehicle stats
	UpdateVehicleStats(VehicleID, [&](FMGVehicleRacingStats& Stats)
	{
		Stats.RacesEntered++;
		if (bWon) Stats.Wins++;
		Stats.TotalDriftScore += DriftScore;
		if (BestLap > 0.0f && (Stats.BestLapTime <= 0.0f || BestLap < Stats.BestLapTime))
		{
			Stats.BestLapTime = BestLap;
			Stats.BestLapTrack = TrackID;
		}
	});

	// Update track stats
	UpdateTrackStats(TrackID, [&](FMGTrackStats& Stats)
	{
		Stats.TimesRaced++;
		if (bWon) Stats.Wins++;
		Stats.TotalDriftScore += DriftScore;
		if (BestLap > 0.0f && (Stats.BestLapTime <= 0.0f || BestLap < Stats.BestLapTime))
		{
			Stats.BestLapTime = BestLap;
			Stats.BestVehicle = VehicleID;
		}
		if (RaceTime > 0.0f && (Stats.BestRaceTime <= 0.0f || RaceTime < Stats.BestRaceTime))
		{
			Stats.BestRaceTime = RaceTime;
		}
	});

	OnStatUpdated.Broadcast(FName(TEXT("TotalRaces")), static_cast<float>(PlayerStats.Race.TotalRaces));
	CheckMilestones();
}

void UMGStatsTracker::RecordDrift(float Score, float Duration, int32 ComboCount)
{
	PlayerStats.Driving.TotalDriftScore += Score;
	PlayerStats.Driving.DriftCount++;
	PlayerStats.Driving.LongestDrift = FMath::Max(PlayerStats.Driving.LongestDrift, Duration);
	PlayerStats.Driving.BestDriftCombo = FMath::Max(PlayerStats.Driving.BestDriftCombo, ComboCount);

	if (!CurrentVehicle.IsNone())
	{
		UpdateVehicleStats(CurrentVehicle, [&](FMGVehicleRacingStats& Stats)
		{
			Stats.TotalDriftScore += Score;
		});
	}

	OnStatUpdated.Broadcast(FName(TEXT("TotalDriftScore")), PlayerStats.Driving.TotalDriftScore);
}

void UMGStatsTracker::RecordOvertake()
{
	PlayerStats.Driving.Overtakes++;
	OnStatUpdated.Broadcast(FName(TEXT("Overtakes")), static_cast<float>(PlayerStats.Driving.Overtakes));
}

void UMGStatsTracker::RecordNearMiss()
{
	PlayerStats.Driving.NearMisses++;
	OnStatUpdated.Broadcast(FName(TEXT("NearMisses")), static_cast<float>(PlayerStats.Driving.NearMisses));
}

void UMGStatsTracker::RecordCollision(bool bWall, bool bTraffic, float Damage)
{
	PlayerStats.Driving.Collisions++;
	PlayerStats.Driving.TotalDamage += Damage;

	if (bWall)
	{
		PlayerStats.Driving.WallHits++;
	}
	if (bTraffic)
	{
		PlayerStats.Driving.TrafficHits++;
	}

	if (!CurrentVehicle.IsNone())
	{
		UpdateVehicleStats(CurrentVehicle, [&](FMGVehicleRacingStats& Stats)
		{
			Stats.TotalDamage += static_cast<int32>(Damage);
		});
	}
}

void UMGStatsTracker::RecordDistance(float Distance)
{
	PlayerStats.Driving.TotalDistance += Distance;

	if (!CurrentVehicle.IsNone())
	{
		UpdateVehicleStats(CurrentVehicle, [&](FMGVehicleRacingStats& Stats)
		{
			Stats.DistanceDriven += Distance;
		});
	}

	CheckMilestones();
}

void UMGStatsTracker::RecordTopSpeed(float Speed)
{
	if (Speed > PlayerStats.Driving.TopSpeed)
	{
		PlayerStats.Driving.TopSpeed = Speed;
		OnStatUpdated.Broadcast(FName(TEXT("TopSpeed")), Speed);
	}

	if (!CurrentVehicle.IsNone())
	{
		UpdateVehicleStats(CurrentVehicle, [&](FMGVehicleRacingStats& Stats)
		{
			Stats.TopSpeed = FMath::Max(Stats.TopSpeed, Speed);
		});
	}
}

void UMGStatsTracker::RecordNOSUsage(float Amount)
{
	PlayerStats.Driving.TotalNOSUsed += Amount;
}

void UMGStatsTracker::RecordCreditsEarned(int64 Amount, FName Source)
{
	PlayerStats.Economy.TotalCreditsEarned += Amount;

	if (Source == FName(TEXT("Race")))
	{
		PlayerStats.Economy.CreditsFromRaces += Amount;
	}
	else if (Source == FName(TEXT("Challenge")))
	{
		PlayerStats.Economy.CreditsFromChallenges += Amount;
	}
	else if (Source == FName(TEXT("Sale")))
	{
		PlayerStats.Economy.CreditsFromSales += Amount;
	}

	OnStatUpdated.Broadcast(FName(TEXT("TotalCreditsEarned")), static_cast<float>(PlayerStats.Economy.TotalCreditsEarned));
}

void UMGStatsTracker::RecordCreditsSpent(int64 Amount, FName Category)
{
	PlayerStats.Economy.TotalCreditsSpent += Amount;

	if (Category == FName(TEXT("Vehicle")))
	{
		PlayerStats.Economy.SpentOnVehicles += Amount;
		PlayerStats.Economy.VehiclesPurchased++;
	}
	else if (Category == FName(TEXT("Part")))
	{
		PlayerStats.Economy.SpentOnParts += Amount;
		PlayerStats.Economy.PartsPurchased++;
	}
	else if (Category == FName(TEXT("Customization")))
	{
		PlayerStats.Economy.SpentOnCustomization += Amount;
	}
	else if (Category == FName(TEXT("Repair")))
	{
		PlayerStats.Economy.SpentOnRepairs += Amount;
	}
}

void UMGStatsTracker::RecordSessionStart()
{
	SessionStartTime = FDateTime::UtcNow();
	CurrentSessionTime = 0.0f;
	PlayerStats.Time.SessionCount++;

	// Check if new day
	FDateTime LastPlay = PlayerStats.Time.LastPlayDate;
	FDateTime Today = FDateTime::UtcNow();
	if (LastPlay.GetDay() != Today.GetDay() || LastPlay.GetMonth() != Today.GetMonth() || LastPlay.GetYear() != Today.GetYear())
	{
		PlayerStats.Time.DaysPlayed++;
	}
}

void UMGStatsTracker::RecordSessionEnd()
{
	PlayerStats.Time.LastPlayDate = FDateTime::UtcNow();
	PlayerStats.Time.LongestSession = FMath::Max(PlayerStats.Time.LongestSession, CurrentSessionTime);
}

void UMGStatsTracker::UpdatePlayTime(float DeltaSeconds, FName Activity)
{
	PlayerStats.Time.TotalPlayTime += DeltaSeconds;
	CurrentSessionTime += DeltaSeconds;
	CurrentActivity = Activity;

	if (Activity == FName(TEXT("Race")))
	{
		PlayerStats.Time.TimeInRaces += DeltaSeconds;
	}
	else if (Activity == FName(TEXT("Garage")))
	{
		PlayerStats.Time.TimeInGarage += DeltaSeconds;
	}
	else
	{
		PlayerStats.Time.TimeInMenus += DeltaSeconds;
	}
}

void UMGStatsTracker::CheckMilestones()
{
	auto CheckMilestone = [this](FName MilestoneID, bool Condition)
	{
		if (Condition && !ReachedMilestones.Contains(MilestoneID))
		{
			ReachedMilestones.Add(MilestoneID);
			OnMilestoneReached.Broadcast(MilestoneID);
		}
	};

	// Race milestones
	CheckMilestone(FName(TEXT("First_Race")), PlayerStats.Race.TotalRaces >= 1);
	CheckMilestone(FName(TEXT("10_Races")), PlayerStats.Race.TotalRaces >= 10);
	CheckMilestone(FName(TEXT("50_Races")), PlayerStats.Race.TotalRaces >= 50);
	CheckMilestone(FName(TEXT("100_Races")), PlayerStats.Race.TotalRaces >= 100);
	CheckMilestone(FName(TEXT("First_Win")), PlayerStats.Race.Wins >= 1);
	CheckMilestone(FName(TEXT("10_Wins")), PlayerStats.Race.Wins >= 10);
	CheckMilestone(FName(TEXT("50_Wins")), PlayerStats.Race.Wins >= 50);
	CheckMilestone(FName(TEXT("5_WinStreak")), PlayerStats.Race.BestWinStreak >= 5);
	CheckMilestone(FName(TEXT("10_WinStreak")), PlayerStats.Race.BestWinStreak >= 10);

	// Driving milestones
	CheckMilestone(FName(TEXT("100_Miles")), PlayerStats.Driving.GetDistanceInMiles() >= 100.0f);
	CheckMilestone(FName(TEXT("1000_Miles")), PlayerStats.Driving.GetDistanceInMiles() >= 1000.0f);
	CheckMilestone(FName(TEXT("Drift_10k")), PlayerStats.Driving.TotalDriftScore >= 10000.0f);
	CheckMilestone(FName(TEXT("Drift_100k")), PlayerStats.Driving.TotalDriftScore >= 100000.0f);
	CheckMilestone(FName(TEXT("Speed_200")), PlayerStats.Driving.TopSpeed >= 200.0f);
	CheckMilestone(FName(TEXT("Speed_250")), PlayerStats.Driving.TopSpeed >= 250.0f);
	CheckMilestone(FName(TEXT("100_Overtakes")), PlayerStats.Driving.Overtakes >= 100);

	// Economy milestones
	CheckMilestone(FName(TEXT("Earned_100k")), PlayerStats.Economy.TotalCreditsEarned >= 100000);
	CheckMilestone(FName(TEXT("Earned_1M")), PlayerStats.Economy.TotalCreditsEarned >= 1000000);
	CheckMilestone(FName(TEXT("Earned_10M")), PlayerStats.Economy.TotalCreditsEarned >= 10000000);

	// Time milestones
	CheckMilestone(FName(TEXT("Play_1_Hour")), PlayerStats.Time.GetPlayTimeHours() >= 1.0f);
	CheckMilestone(FName(TEXT("Play_10_Hours")), PlayerStats.Time.GetPlayTimeHours() >= 10.0f);
	CheckMilestone(FName(TEXT("Play_100_Hours")), PlayerStats.Time.GetPlayTimeHours() >= 100.0f);
}

void UMGStatsTracker::UpdateVehicleStats(FName VehicleID, const TFunction<void(FMGVehicleRacingStats&)>& UpdateFunc)
{
	if (VehicleID.IsNone())
	{
		return;
	}

	FMGVehicleRacingStats& Stats = VehicleStatsMap.FindOrAdd(VehicleID);
	Stats.VehicleID = VehicleID;
	UpdateFunc(Stats);
}

void UMGStatsTracker::UpdateTrackStats(FName TrackID, const TFunction<void(FMGTrackStats&)>& UpdateFunc)
{
	if (TrackID.IsNone())
	{
		return;
	}

	FMGTrackStats& Stats = TrackStatsMap.FindOrAdd(TrackID);
	Stats.TrackID = TrackID;
	UpdateFunc(Stats);
}
