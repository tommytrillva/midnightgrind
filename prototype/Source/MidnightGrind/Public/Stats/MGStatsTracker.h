// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStatsTracker.generated.h"

USTRUCT(BlueprintType)
struct FMGRaceStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 TotalRaces = 0;
	UPROPERTY(BlueprintReadOnly) int32 Wins = 0;
	UPROPERTY(BlueprintReadOnly) int32 Podiums = 0;
	UPROPERTY(BlueprintReadOnly) int32 DNFs = 0;
	UPROPERTY(BlueprintReadOnly) int32 PinkSlipsWon = 0;
	UPROPERTY(BlueprintReadOnly) int32 PinkSlipsLost = 0;
	UPROPERTY(BlueprintReadOnly) int32 CurrentWinStreak = 0;
	UPROPERTY(BlueprintReadOnly) int32 BestWinStreak = 0;
	UPROPERTY(BlueprintReadOnly) float TotalRaceTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) float BestLapTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) FName BestLapTrack;
	UPROPERTY(BlueprintReadOnly) int32 LapsCompleted = 0;
	UPROPERTY(BlueprintReadOnly) int32 CheckpointsMissed = 0;
	UPROPERTY(BlueprintReadOnly) int32 PerfectLaps = 0;
	UPROPERTY(BlueprintReadOnly) int32 PhotoFinishes = 0;
	UPROPERTY(BlueprintReadOnly) int32 ComebackWins = 0;

	float GetWinRate() const { return TotalRaces > 0 ? static_cast<float>(Wins) / TotalRaces : 0.0f; }
	float GetPodiumRate() const { return TotalRaces > 0 ? static_cast<float>(Podiums) / TotalRaces : 0.0f; }
};

USTRUCT(BlueprintType)
struct FMGDrivingStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) float TotalDistance = 0.0f;
	UPROPERTY(BlueprintReadOnly) float TopSpeed = 0.0f;
	UPROPERTY(BlueprintReadOnly) float TotalDriftScore = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 DriftCount = 0;
	UPROPERTY(BlueprintReadOnly) float LongestDrift = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 BestDriftCombo = 0;
	UPROPERTY(BlueprintReadOnly) float TotalAirTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 NearMisses = 0;
	UPROPERTY(BlueprintReadOnly) int32 Overtakes = 0;
	UPROPERTY(BlueprintReadOnly) int32 TimesOvertaken = 0;
	UPROPERTY(BlueprintReadOnly) float TotalNOSUsed = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 PerfectStarts = 0;
	UPROPERTY(BlueprintReadOnly) int32 Collisions = 0;
	UPROPERTY(BlueprintReadOnly) int32 WallHits = 0;
	UPROPERTY(BlueprintReadOnly) int32 TrafficHits = 0;
	UPROPERTY(BlueprintReadOnly) float TotalDamage = 0.0f;

	float GetDistanceInMiles() const { return TotalDistance / 160934.0f; }
	float GetDistanceInKm() const { return TotalDistance / 100000.0f; }
};

USTRUCT(BlueprintType)
struct FMGEconomyStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int64 TotalCreditsEarned = 0;
	UPROPERTY(BlueprintReadOnly) int64 TotalCreditsSpent = 0;
	UPROPERTY(BlueprintReadOnly) int64 CreditsFromRaces = 0;
	UPROPERTY(BlueprintReadOnly) int64 CreditsFromChallenges = 0;
	UPROPERTY(BlueprintReadOnly) int64 CreditsFromSales = 0;
	UPROPERTY(BlueprintReadOnly) int64 SpentOnVehicles = 0;
	UPROPERTY(BlueprintReadOnly) int64 SpentOnParts = 0;
	UPROPERTY(BlueprintReadOnly) int64 SpentOnCustomization = 0;
	UPROPERTY(BlueprintReadOnly) int64 SpentOnRepairs = 0;
	UPROPERTY(BlueprintReadOnly) int32 VehiclesPurchased = 0;
	UPROPERTY(BlueprintReadOnly) int32 VehiclesSold = 0;
	UPROPERTY(BlueprintReadOnly) int32 PartsPurchased = 0;
};

USTRUCT(BlueprintType)
struct FMGSocialStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 RivalsCreated = 0;
	UPROPERTY(BlueprintReadOnly) int32 NemesesDefeated = 0;
	UPROPERTY(BlueprintReadOnly) int32 RivalTauntsReceived = 0;
	UPROPERTY(BlueprintReadOnly) int32 MultiplayerRaces = 0;
	UPROPERTY(BlueprintReadOnly) int32 MultiplayerWins = 0;
	UPROPERTY(BlueprintReadOnly) int32 FriendRaces = 0;
	UPROPERTY(BlueprintReadOnly) int32 InvitesSent = 0;
	UPROPERTY(BlueprintReadOnly) int32 InvitesAccepted = 0;
};

USTRUCT(BlueprintType)
struct FMGProgressionStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 CurrentLevel = 1;
	UPROPERTY(BlueprintReadOnly) int32 TotalXPEarned = 0;
	UPROPERTY(BlueprintReadOnly) int32 TotalReputation = 0;
	UPROPERTY(BlueprintReadOnly) int32 ChallengesCompleted = 0;
	UPROPERTY(BlueprintReadOnly) int32 AchievementsUnlocked = 0;
	UPROPERTY(BlueprintReadOnly) int32 VehiclesUnlocked = 0;
	UPROPERTY(BlueprintReadOnly) int32 TracksUnlocked = 0;
	UPROPERTY(BlueprintReadOnly) int32 CareerEventsCompleted = 0;
	UPROPERTY(BlueprintReadOnly) float CareerProgress = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGTimeStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) float TotalPlayTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) float TimeInRaces = 0.0f;
	UPROPERTY(BlueprintReadOnly) float TimeInGarage = 0.0f;
	UPROPERTY(BlueprintReadOnly) float TimeInMenus = 0.0f;
	UPROPERTY(BlueprintReadOnly) float LongestSession = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 SessionCount = 0;
	UPROPERTY(BlueprintReadOnly) FDateTime FirstPlayDate;
	UPROPERTY(BlueprintReadOnly) FDateTime LastPlayDate;
	UPROPERTY(BlueprintReadOnly) int32 DaysPlayed = 0;

	float GetPlayTimeHours() const { return TotalPlayTime / 3600.0f; }
};

USTRUCT(BlueprintType)
struct FMGVehicleStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName VehicleID;
	UPROPERTY(BlueprintReadOnly) int32 RacesEntered = 0;
	UPROPERTY(BlueprintReadOnly) int32 Wins = 0;
	UPROPERTY(BlueprintReadOnly) float DistanceDriven = 0.0f;
	UPROPERTY(BlueprintReadOnly) float BestLapTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) FName BestLapTrack;
	UPROPERTY(BlueprintReadOnly) float TotalDriftScore = 0.0f;
	UPROPERTY(BlueprintReadOnly) float TopSpeed = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 TotalDamage = 0;
	UPROPERTY(BlueprintReadOnly) int64 RepairCosts = 0;
};

USTRUCT(BlueprintType)
struct FMGTrackStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName TrackID;
	UPROPERTY(BlueprintReadOnly) int32 TimesRaced = 0;
	UPROPERTY(BlueprintReadOnly) int32 Wins = 0;
	UPROPERTY(BlueprintReadOnly) float BestLapTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) float BestRaceTime = 0.0f;
	UPROPERTY(BlueprintReadOnly) FName BestVehicle;
	UPROPERTY(BlueprintReadOnly) float TotalDriftScore = 0.0f;
	UPROPERTY(BlueprintReadOnly) int32 LeaderboardPosition = 0;
};

USTRUCT(BlueprintType)
struct FMGPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FMGRaceStats Race;
	UPROPERTY(BlueprintReadOnly) FMGDrivingStats Driving;
	UPROPERTY(BlueprintReadOnly) FMGEconomyStats Economy;
	UPROPERTY(BlueprintReadOnly) FMGSocialStats Social;
	UPROPERTY(BlueprintReadOnly) FMGProgressionStats Progression;
	UPROPERTY(BlueprintReadOnly) FMGTimeStats Time;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUpdated, FName, StatName, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneReached, FName, MilestoneID);

UCLASS()
class MIDNIGHTGRIND_API UMGStatsTracker : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Stats")
	const FMGPlayerStats& GetPlayerStats() const { return PlayerStats; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	FMGVehicleStats GetVehicleStats(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	FMGTrackStats GetTrackStats(FName TrackID) const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordRaceResult(FName TrackID, FName VehicleID, int32 Position, int32 TotalRacers,
		float RaceTime, float BestLap, float DriftScore, bool bPinkSlip);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordDrift(float Score, float Duration, int32 ComboCount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordOvertake();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordNearMiss();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordCollision(bool bWall, bool bTraffic, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordTopSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordNOSUsage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordCreditsEarned(int64 Amount, FName Source);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordCreditsSpent(int64 Amount, FName Category);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordSessionStart();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordSessionEnd();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpdatePlayTime(float DeltaSeconds, FName Activity);

	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatUpdated OnStatUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnMilestoneReached OnMilestoneReached;

protected:
	void CheckMilestones();
	void UpdateVehicleStats(FName VehicleID, const TFunction<void(FMGVehicleStats&)>& UpdateFunc);
	void UpdateTrackStats(FName TrackID, const TFunction<void(FMGTrackStats&)>& UpdateFunc);

private:
	FMGPlayerStats PlayerStats;
	TMap<FName, FMGVehicleStats> VehicleStatsMap;
	TMap<FName, FMGTrackStats> TrackStatsMap;
	TSet<FName> ReachedMilestones;
	float CurrentSessionTime = 0.0f;
	FDateTime SessionStartTime;
	FName CurrentActivity;
	FName CurrentVehicle;
};
