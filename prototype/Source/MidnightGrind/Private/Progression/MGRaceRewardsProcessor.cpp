// Copyright Midnight Grind. All Rights Reserved.

#include "Progression/MGRaceRewardsProcessor.h"
#include "Progression/MGPlayerProgression.h"
#include "GameModes/MGRaceGameMode.h"
#include "Race/MGRaceHistorySubsystem.h"
#include "Career/MGCareerSubsystem.h"
#include "Social/MGLeaderboardSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMGRaceRewardsProcessor::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get player progression
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			PlayerProgression = GI->GetSubsystem<UMGPlayerProgression>();
		}
	}

	// Try to bind to race game mode
	BindToRaceGameMode();
}

void UMGRaceRewardsProcessor::Deinitialize()
{
	// Unbind from race game mode delegate
	if (RaceGameMode.IsValid())
	{
		RaceGameMode->OnRaceResultsReady.RemoveDynamic(this, &UMGRaceRewardsProcessor::OnRaceResultsReady);
	}

	Super::Deinitialize();
}

bool UMGRaceRewardsProcessor::ShouldCreateSubsystem(UObject* Outer) const
{
	// Always create - we need this for reward processing
	return true;
}

// ==========================================
// PERFORMANCE TRACKING
// ==========================================

void UMGRaceRewardsProcessor::BeginRaceTracking(int32 StartingPosition)
{
	bIsTracking = true;
	CurrentPerformance = FMGRacePerformanceData();
	CurrentPerformance.StartingPosition = StartingPosition;

	// Store pre-reward state for comparison
	if (PlayerProgression.IsValid())
	{
		PreRewardLevel = PlayerProgression->GetCurrentLevel();
		PreRewardTier = PlayerProgression->GetCrewReputationTier(CurrentRaceCrew);
	}
}

void UMGRaceRewardsProcessor::RecordOvertake()
{
	if (bIsTracking)
	{
		CurrentPerformance.Overtakes++;
	}
}

void UMGRaceRewardsProcessor::RecordOvertaken()
{
	if (bIsTracking)
	{
		CurrentPerformance.TimesOvertaken++;
	}
}

void UMGRaceRewardsProcessor::RecordDriftScore(float Score)
{
	if (bIsTracking)
	{
		CurrentPerformance.TotalDriftScore += Score;
	}
}

void UMGRaceRewardsProcessor::RecordNearMiss()
{
	if (bIsTracking)
	{
		CurrentPerformance.NearMisses++;
	}
}

void UMGRaceRewardsProcessor::RecordCollision()
{
	if (bIsTracking)
	{
		CurrentPerformance.Collisions++;
	}
}

void UMGRaceRewardsProcessor::RecordMaxSpeed(float SpeedMPH)
{
	if (bIsTracking && SpeedMPH > CurrentPerformance.MaxSpeedMPH)
	{
		CurrentPerformance.MaxSpeedMPH = SpeedMPH;
	}
}

// ==========================================
// REWARD PROCESSING
// ==========================================

FMGRaceRewards UMGRaceRewardsProcessor::CalculateRewards(const FMGFinalRaceResult& Result, EMGCrew RaceCrew, bool bIsRanked)
{
	FMGRaceRewards Rewards;

	// Get total racers from game mode
	int32 TotalRacers = 8;
	if (RaceGameMode.IsValid())
	{
		TotalRacers = RaceGameMode->GetRacerCount();
	}

	// Calculate XP
	Rewards.XPBreakdown = CalculateXPBreakdown(Result, bIsRanked, TotalRacers);

	// Credits from race result
	Rewards.CreditsEarned = Result.CashEarned;

	// Reputation
	Rewards.ReputationEarned = Result.ReputationEarned;
	Rewards.ReputationCrew = RaceCrew;

	// Check for records
	if (RaceGameMode.IsValid())
	{
		FName TrackID = RaceGameMode->GetRaceConfig().TrackName;
		Rewards.bNewPersonalBest = CheckPersonalBest(TrackID, Result.TotalTime);
		Rewards.bNewTrackRecord = CheckTrackRecord(TrackID, Result.TotalTime);
	}

	// Store performance data
	CurrentPerformance.FinalPosition = Result.Position;
	CurrentPerformance.bFinished = !Result.bDNF;

	LastRewards = Rewards;
	OnRaceRewardsCalculated.Broadcast(Rewards);

	return Rewards;
}

void UMGRaceRewardsProcessor::GrantRewards(const FMGRaceRewards& Rewards)
{
	if (!PlayerProgression.IsValid())
	{
		return;
	}

	FMGRaceRewards MutableRewards = Rewards;

	// Store pre-grant state
	int32 LevelBefore = PlayerProgression->GetCurrentLevel();
	EMGReputationTier TierBefore = PlayerProgression->GetCrewReputationTier(Rewards.ReputationCrew);

	// Grant XP
	if (Rewards.XPBreakdown.TotalXP > 0)
	{
		PlayerProgression->AddXP(Rewards.XPBreakdown.TotalXP);
	}

	// Grant reputation
	if (Rewards.ReputationEarned > 0 && Rewards.ReputationCrew != EMGCrew::None)
	{
		PlayerProgression->AddCrewReputation(Rewards.ReputationCrew, Rewards.ReputationEarned);
	}

	// Record race statistics
	if (RaceGameMode.IsValid())
	{
		FMGRaceConfig Config = RaceGameMode->GetRaceConfig();

		// Determine race type ID based on mode
		FName RaceTypeID = TEXT("Circuit"); // Default

		PlayerProgression->RecordRaceResult(
			CurrentPerformance.FinalPosition,
			RaceGameMode->GetRacerCount(),
			Rewards.ReputationCrew,
			RaceTypeID
		);
	}

	// Record additional stats
	if (CurrentPerformance.DistanceKm > 0.0f)
	{
		PlayerProgression->AddDistanceDriven(CurrentPerformance.DistanceKm);
	}
	if (CurrentPerformance.MaxSpeedMPH > 0.0f)
	{
		PlayerProgression->RecordTopSpeed(CurrentPerformance.MaxSpeedMPH);
	}
	if (CurrentPerformance.TotalDriftScore > 0.0f)
	{
		PlayerProgression->RecordDriftScore(CurrentPerformance.TotalDriftScore);
	}

	// Check for level up
	int32 LevelAfter = PlayerProgression->GetCurrentLevel();
	if (LevelAfter > LevelBefore)
	{
		MutableRewards.bLeveledUp = true;
		MutableRewards.NewLevel = LevelAfter;
	}

	// Check for reputation tier change
	EMGReputationTier TierAfter = PlayerProgression->GetCrewReputationTier(Rewards.ReputationCrew);
	if (TierAfter != TierBefore)
	{
		MutableRewards.bReputationTierChanged = true;
		MutableRewards.NewReputationTier = TierAfter;
	}

	// Check for new unlocks
	TArray<FMGUnlock> NewUnlocks = PlayerProgression->CheckAndGrantNewUnlocks();
	MutableRewards.NewUnlocks = NewUnlocks;

	// Broadcast individual unlocks for animation
	for (int32 i = 0; i < NewUnlocks.Num(); i++)
	{
		OnNewUnlockFromRace.Broadcast(NewUnlocks[i], i);
	}

	// Record to race history subsystem
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			// Record to race history
			if (UMGRaceHistorySubsystem* RaceHistory = GI->GetSubsystem<UMGRaceHistorySubsystem>())
			{
				FMGRaceResult HistoryResult;
				HistoryResult.RaceId = FGuid::NewGuid();

				if (RaceGameMode.IsValid())
				{
					FMGRaceConfig Config = RaceGameMode->GetRaceConfig();
					HistoryResult.TrackId = Config.TrackName.ToString();
					HistoryResult.TrackName = FText::FromName(Config.TrackName);
					HistoryResult.RaceType = FName(*UEnum::GetValueAsString(Config.RaceType));
					HistoryResult.TotalRacers = RaceGameMode->GetRacerCount();
				}

				HistoryResult.Position = CurrentPerformance.FinalPosition;
				HistoryResult.bDNF = !CurrentPerformance.bFinished;
				HistoryResult.bWasCleanRace = CurrentPerformance.IsCleanRace();
				HistoryResult.DistanceM = CurrentPerformance.DistanceKm * 1000.0f;
				HistoryResult.TopSpeedKPH = CurrentPerformance.MaxSpeedMPH * 1.60934f;
				HistoryResult.CashEarned = Rewards.CreditsEarned;
				HistoryResult.ReputationEarned = Rewards.ReputationEarned;
				HistoryResult.XPEarned = static_cast<int32>(Rewards.XPBreakdown.TotalXP);
				HistoryResult.Timestamp = FDateTime::Now();

				RaceHistory->RecordRaceResult(HistoryResult);
			}

			// Notify career subsystem
			if (UMGCareerSubsystem* Career = GI->GetSubsystem<UMGCareerSubsystem>())
			{
				TArray<FString> DefeatedRivals; // Would be populated from race results
				Career->OnRaceCompleted(
					CurrentPerformance.FinalPosition,
					RaceGameMode.IsValid() ? RaceGameMode->GetRacerCount() : 1,
					CurrentPerformance.IsCleanRace(),
					DefeatedRivals
				);
			}
		}
	}

	// Update last rewards with post-grant state
	LastRewards = MutableRewards;

	// Stop tracking
	bIsTracking = false;

	OnRaceRewardsGranted.Broadcast(MutableRewards);
}

FMGRaceRewards UMGRaceRewardsProcessor::ProcessRaceEnd(const FMGFinalRaceResult& Result, EMGCrew RaceCrew, bool bIsRanked)
{
	FMGRaceRewards Rewards = CalculateRewards(Result, RaceCrew, bIsRanked);
	GrantRewards(Rewards);
	return Rewards;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGRaceRewardsProcessor::BindToRaceGameMode()
{
	// Find race game mode
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GM = World->GetAuthGameMode())
		{
			if (AMGRaceGameMode* RaceGM = Cast<AMGRaceGameMode>(GM))
			{
				RaceGameMode = RaceGM;
				RaceGM->OnRaceResultsReady.AddDynamic(this, &UMGRaceRewardsProcessor::OnRaceResultsReady);
			}
		}
	}
}

void UMGRaceRewardsProcessor::OnRaceResultsReady(const TArray<FMGFinalRaceResult>& Results)
{
	// Find player result
	for (const FMGFinalRaceResult& Result : Results)
	{
		if (Result.bIsPlayer)
		{
			// Process rewards for player
			ProcessRaceEnd(Result, CurrentRaceCrew, bCurrentRaceRanked);
			break;
		}
	}
}

FMGXPBreakdown UMGRaceRewardsProcessor::CalculateXPBreakdown(const FMGFinalRaceResult& Result, bool bIsRanked, int32 TotalRacers)
{
	FMGXPBreakdown XP;

	// Base XP for finishing
	if (!Result.bDNF)
	{
		XP.BaseXP = BaseFinishXP;

		// Position XP (higher positions get more)
		int32 PositionBonus = FMath::Max(0, TotalRacers - Result.Position + 1);
		XP.PositionXP = XPPerPosition * PositionBonus;

		// Best lap bonus - check if this racer had the best lap
		// This would require comparing across all results
		// For now, give bonus if position is 1st-3rd and they have a lap time
		if (Result.Position <= 3 && Result.BestLap > 0.0f)
		{
			XP.BestLapXP = BestLapXP;
		}
	}
	else
	{
		// DNF gets minimal base XP
		XP.BaseXP = BaseFinishXP / 4;
	}

	// Clean race bonus
	if (CurrentPerformance.IsCleanRace())
	{
		XP.CleanRaceXP = CleanRaceXP;
	}

	// Overtake XP
	XP.OvertakeXP = XPPerOvertake * CurrentPerformance.Overtakes;

	// Drift XP (per 1000 points)
	int32 DriftThousands = FMath::FloorToInt(CurrentPerformance.TotalDriftScore / 1000.0f);
	XP.DriftXP = XPPerThousandDrift * DriftThousands;

	// Near miss XP
	XP.NearMissXP = XPPerNearMiss * CurrentPerformance.NearMisses;

	// Ranked bonus (applied as multiplier to base + position)
	if (bIsRanked)
	{
		int64 BaseAndPosition = XP.BaseXP + XP.PositionXP;
		XP.RankedBonusXP = static_cast<int64>(BaseAndPosition * (RankedXPMultiplier - 1.0f));
	}

	XP.CalculateTotal();

	return XP;
}

bool UMGRaceRewardsProcessor::CheckPersonalBest(FName TrackID, float Time) const
{
	if (Time <= 0.0f)
	{
		return false;
	}

	// Get leaderboard subsystem to check personal best
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UMGLeaderboardSubsystem* LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>())
			{
				const FMGTrackRecord TrackRecord = LeaderboardSubsystem->GetTrackRecords(TrackID);

				// Convert time to milliseconds for comparison (scores stored as ms)
				const int64 TimeMs = static_cast<int64>(Time * 1000.0f);
				const int64 PersonalBestMs = TrackRecord.PersonalBest.Score;

				// New personal best if no previous record (Score == 0) or faster time
				return (PersonalBestMs == 0 || TimeMs < PersonalBestMs);
			}
		}
	}

	return false;
}

bool UMGRaceRewardsProcessor::CheckTrackRecord(FName TrackID, float Time) const
{
	if (Time <= 0.0f)
	{
		return false;
	}

	// Get leaderboard subsystem to check track record
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UMGLeaderboardSubsystem* LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>())
			{
				const FMGTrackRecord TrackRecord = LeaderboardSubsystem->GetTrackRecords(TrackID);

				// Convert time to milliseconds for comparison (scores stored as ms)
				const int64 TimeMs = static_cast<int64>(Time * 1000.0f);
				const int64 WorldRecordMs = TrackRecord.WorldRecord.Score;

				// New track record if no existing record (Score == 0) or faster time
				return (WorldRecordMs == 0 || TimeMs < WorldRecordMs);
			}
		}
	}

	return false;
}
