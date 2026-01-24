// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDriftSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGDriftGrade : uint8
{
	None,
	D,
	C,
	B,
	A,
	S,
	SS,
	SSS
};

UENUM(BlueprintType)
enum class EMGDriftZoneType : uint8
{
	None,
	Standard,
	Hairpin,
	SwitchBack,
	Downhill,
	Touge,
	Highway,
	Parking
};

UENUM(BlueprintType)
enum class EMGDriftChainBonus : uint8
{
	None,
	Tandem,
	Counter,
	Manji,
	Feint,
	WallTap,
	DonutEntry,
	CloseCall,
	Overtake
};

USTRUCT(BlueprintType)
struct FMGActiveDrift
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftGrade CurrentGrade = EMGDriftGrade::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReverse = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinWallDistance = 9999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDriftChainBonus> ActiveBonuses;
};

USTRUCT(BlueprintType)
struct FMGDriftResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FinalMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftGrade Grade = EMGDriftGrade::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGDriftChainBonus> Bonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPerfect = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFailed = false;
};

USTRUCT(BlueprintType)
struct FMGDriftZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ZoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftZoneType ZoneType = EMGDriftZoneType::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldScore = 25000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumScore = 50000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ZoneCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoneRadius = 1000.0f;
};

USTRUCT(BlueprintType)
struct FMGDriftSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDrifts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestChain = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestSingleDrift = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDriftDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDriftTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDriftAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDriftSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectDrifts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FailedDrifts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDriftGrade, int32> GradeCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDriftChainBonus, int32> BonusCounts;
};

USTRUCT(BlueprintType)
struct FMGDriftLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime SetAt;
};

USTRUCT(BlueprintType)
struct FMGDriftConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDriftAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDriftSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainTimeWindow = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePointsPerSecond = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleMultiplierScale = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplierScale = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallProximityBonusDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WallProximityMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TandemBonusDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TandemMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SSSThreshold = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SSThreshold = 50000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SThreshold = 25000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AThreshold = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BThreshold = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CThreshold = 2500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DThreshold = 1000;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftStarted, const FMGActiveDrift&, Drift);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftEnded, const FMGDriftResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftGradeChanged, EMGDriftGrade, OldGrade, EMGDriftGrade, NewGrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftBonusTriggered, EMGDriftChainBonus, Bonus, int32, Points);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftChainExtended, int32, ChainCount, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDriftFailed, const FMGDriftResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftZoneEntered, const FMGDriftZone&, Zone, bool, bNewHighScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftScoreMilestone, int32, Score, FName, MilestoneName);

UCLASS()
class MIDNIGHTGRIND_API UMGDriftSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Core Drift Functions
	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void UpdateDriftState(float DeltaTime, float SlipAngle, float Speed, const FVector& Position, const FVector& Velocity);

	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void StartDrift();

	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void EndDrift(bool bFailed = false);

	UFUNCTION(BlueprintCallable, Category = "Drift|Core")
	void ResetSession();

	UFUNCTION(BlueprintPure, Category = "Drift|Core")
	bool IsDrifting() const { return ActiveDrift.bIsDrifting; }

	UFUNCTION(BlueprintPure, Category = "Drift|Core")
	FMGActiveDrift GetActiveDrift() const { return ActiveDrift; }

	UFUNCTION(BlueprintPure, Category = "Drift|Core")
	float GetTimeSinceLastDrift() const;

	// Scoring
	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	int32 GetCurrentScore() const { return ActiveDrift.CurrentPoints; }

	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	int32 GetSessionScore() const { return SessionStats.TotalPoints; }

	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	float GetCurrentMultiplier() const { return ActiveDrift.Multiplier; }

	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	EMGDriftGrade GetCurrentGrade() const { return ActiveDrift.CurrentGrade; }

	UFUNCTION(BlueprintPure, Category = "Drift|Scoring")
	EMGDriftGrade CalculateGradeFromPoints(int32 Points) const;

	// Bonuses
	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void TriggerBonus(EMGDriftChainBonus Bonus);

	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void SetWallProximity(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void SetTandemPartner(bool bHasTandemPartner, float PartnerDistance);

	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void RegisterOvertake();

	UFUNCTION(BlueprintCallable, Category = "Drift|Bonus")
	void RegisterCloseCall(float Distance);

	UFUNCTION(BlueprintPure, Category = "Drift|Bonus")
	int32 GetBonusPoints(EMGDriftChainBonus Bonus) const;

	// Chain Management
	UFUNCTION(BlueprintPure, Category = "Drift|Chain")
	int32 GetChainCount() const { return ActiveDrift.ChainCount; }

	UFUNCTION(BlueprintPure, Category = "Drift|Chain")
	bool IsChainActive() const;

	UFUNCTION(BlueprintCallable, Category = "Drift|Chain")
	void ExtendChain();

	UFUNCTION(BlueprintCallable, Category = "Drift|Chain")
	void BreakChain();

	// Zones
	UFUNCTION(BlueprintCallable, Category = "Drift|Zones")
	void EnterDriftZone(const FMGDriftZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "Drift|Zones")
	void ExitDriftZone();

	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	bool IsInDriftZone() const { return bInDriftZone; }

	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	FMGDriftZone GetCurrentDriftZone() const { return CurrentZone; }

	UFUNCTION(BlueprintCallable, Category = "Drift|Zones")
	void RegisterDriftZone(const FMGDriftZone& Zone);

	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	TArray<FMGDriftZone> GetAllDriftZones() const;

	UFUNCTION(BlueprintPure, Category = "Drift|Zones")
	int32 GetZoneHighScore(FName ZoneID) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Drift|Stats")
	FMGDriftSessionStats GetSessionStats() const { return SessionStats; }

	UFUNCTION(BlueprintPure, Category = "Drift|Stats")
	FMGDriftSessionStats GetCareerStats() const { return CareerStats; }

	UFUNCTION(BlueprintCallable, Category = "Drift|Stats")
	void MergeSessionToCareer();

	// Leaderboard
	UFUNCTION(BlueprintPure, Category = "Drift|Leaderboard")
	TArray<FMGDriftLeaderboardEntry> GetZoneLeaderboard(FName ZoneID, int32 MaxEntries = 50) const;

	UFUNCTION(BlueprintCallable, Category = "Drift|Leaderboard")
	void SubmitZoneScore(FName ZoneID, int32 Score, FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Drift|Leaderboard")
	int32 GetZoneLeaderboardPosition(FName ZoneID) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Drift|Config")
	void SetDriftConfig(const FMGDriftConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Drift|Config")
	FMGDriftConfig GetDriftConfig() const { return Config; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftStarted OnDriftStarted;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftEnded OnDriftEnded;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftGradeChanged OnDriftGradeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftBonusTriggered OnDriftBonusTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftChainExtended OnDriftChainExtended;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftFailed OnDriftFailed;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftZoneEntered OnDriftZoneEntered;

	UPROPERTY(BlueprintAssignable, Category = "Drift|Events")
	FOnDriftScoreMilestone OnDriftScoreMilestone;

protected:
	void CalculatePoints(float DeltaTime);
	void UpdateGrade();
	void UpdateMultiplier();
	void CheckMilestones(int32 OldScore, int32 NewScore);
	FMGDriftResult BuildDriftResult(bool bFailed);
	void SaveDriftData();
	void LoadDriftData();

	UPROPERTY()
	FMGActiveDrift ActiveDrift;

	UPROPERTY()
	FMGDriftSessionStats SessionStats;

	UPROPERTY()
	FMGDriftSessionStats CareerStats;

	UPROPERTY()
	FMGDriftConfig Config;

	UPROPERTY()
	FMGDriftZone CurrentZone;

	UPROPERTY()
	TArray<FMGDriftZone> RegisteredZones;

	UPROPERTY()
	TMap<FName, int32> ZoneHighScores;

	UPROPERTY()
	TMap<FName, TArray<FMGDriftLeaderboardEntry>> ZoneLeaderboards;

	UPROPERTY()
	bool bInDriftZone = false;

	UPROPERTY()
	float LastDriftEndTime = 0.0f;

	UPROPERTY()
	float MaxAngleThisDrift = 0.0f;

	UPROPERTY()
	float MaxSpeedThisDrift = 0.0f;

	UPROPERTY()
	bool bHasTandemPartner = false;

	UPROPERTY()
	float TandemPartnerDistance = 0.0f;

	UPROPERTY()
	int32 LastMilestoneReached = 0;
};
