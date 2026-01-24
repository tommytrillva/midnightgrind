// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSpeedtrapSubsystem.generated.h"

/**
 * Speed trap type
 */
UENUM(BlueprintType)
enum class EMGSpeedtrapType : uint8
{
	Camera				UMETA(DisplayName = "Speed Camera"),
	Zone				UMETA(DisplayName = "Speed Zone"),
	Checkpoint			UMETA(DisplayName = "Speed Checkpoint"),
	TopSpeed			UMETA(DisplayName = "Top Speed"),
	Average				UMETA(DisplayName = "Average Speed"),
	Jump				UMETA(DisplayName = "Jump Distance"),
	Drift				UMETA(DisplayName = "Drift Zone"),
	NearMiss			UMETA(DisplayName = "Near Miss Zone"),
	Combo				UMETA(DisplayName = "Combo Zone")
};

/**
 * Speed trap rating
 */
UENUM(BlueprintType)
enum class EMGSpeedtrapRating : uint8
{
	None				UMETA(DisplayName = "None"),
	Bronze				UMETA(DisplayName = "Bronze"),
	Silver				UMETA(DisplayName = "Silver"),
	Gold				UMETA(DisplayName = "Gold"),
	Platinum			UMETA(DisplayName = "Platinum"),
	Diamond				UMETA(DisplayName = "Diamond"),
	Legend				UMETA(DisplayName = "Legend")
};

/**
 * Speed trap state
 */
UENUM(BlueprintType)
enum class EMGSpeedtrapState : uint8
{
	Inactive			UMETA(DisplayName = "Inactive"),
	Active				UMETA(DisplayName = "Active"),
	InProgress			UMETA(DisplayName = "In Progress"),
	Completed			UMETA(DisplayName = "Completed"),
	Failed				UMETA(DisplayName = "Failed")
};

/**
 * Speed trap definition
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapType Type = EMGSpeedtrapType::Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerWidth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerHeight = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoneLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RequiredDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DirectionTolerance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverThreshold = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldThreshold = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlatinumThreshold = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiamondThreshold = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LegendThreshold = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverPoints = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldPoints = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumPoints = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiamondPoints = 2500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LegendPoints = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTimeLimit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VisualAsset;
};

/**
 * Speed trap attempt
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapAttempt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttemptId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecordedValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExitSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeTaken = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating Rating = EMGSpeedtrapRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWorldRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaFromBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Active speed trap progress
 */
USTRUCT(BlueprintType)
struct FMGActiveSpeedtrap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapState State = EMGSpeedtrapState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeElapsed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SampleCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedSum = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating CurrentRating = EMGSpeedtrapRating::None;
};

/**
 * Speed trap record
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PersonalBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorldRecord = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FriendBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WorldRecordHolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FriendRecordHolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating BestRating = EMGSpeedtrapRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCompletions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime PersonalBestDate;
};

/**
 * Player speed trap stats
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FMGSpeedtrapRecord> Records;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalSpeedtrapsFound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalGoldRatings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPlatinumRatings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDiamondRatings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLegendRatings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestSpeedRecorded = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestJumpRecorded = 0.0f;
};

/**
 * Speed zone configuration
 */
USTRUCT(BlueprintType)
struct FMGSpeedZoneConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedMPH = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedDecayRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplierPerZone = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxComboMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearMissBonusPercent = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftBonusPercent = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OvertakeBonusPercent = 20.0f;
};

/**
 * Leaderboard entry
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecordValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating Rating = EMGSpeedtrapRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordDate;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapEntered, const FString&, SpeedtrapId, const FString&, PlayerId, float, EntrySpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapExited, const FString&, SpeedtrapId, const FMGSpeedtrapAttempt&, Attempt, bool, bCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapRecorded, const FString&, SpeedtrapId, float, RecordedValue, EMGSpeedtrapRating, Rating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapNewPersonalBest, const FString&, SpeedtrapId, float, OldBest, float, NewBest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapNewWorldRecord, const FString&, SpeedtrapId, float, RecordValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapRatingAchieved, EMGSpeedtrapRating, Rating, int32, TotalAtRating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapDiscovered, const FString&, SpeedtrapId, int32, TotalDiscovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapProgress, const FString&, SpeedtrapId, float, Progress);

/**
 * Speed Trap Subsystem
 * Manages speed cameras, speed zones, and speed challenges
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSpeedtrapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapEntered OnSpeedtrapEntered;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapExited OnSpeedtrapExited;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapRecorded OnSpeedtrapRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapNewPersonalBest OnSpeedtrapNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapNewWorldRecord OnSpeedtrapNewWorldRecord;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapRatingAchieved OnSpeedtrapRatingAchieved;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapDiscovered OnSpeedtrapDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapProgress OnSpeedtrapProgress;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Registration")
	void RegisterSpeedtrap(const FMGSpeedtrapDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Registration")
	void UnregisterSpeedtrap(const FString& SpeedtrapId);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	FMGSpeedtrapDefinition GetSpeedtrap(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	TArray<FMGSpeedtrapDefinition> GetAllSpeedtraps() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	TArray<FMGSpeedtrapDefinition> GetSpeedtrapsInArea(FVector Center, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	TArray<FMGSpeedtrapDefinition> GetSpeedtrapsForTrack(const FString& TrackId) const;

	// Detection
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Detection")
	void UpdateSpeedtrapDetection(const FString& PlayerId, FVector Location, FVector Velocity, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Detection")
	bool TryEnterSpeedtrap(const FString& PlayerId, const FString& SpeedtrapId, float Speed, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Detection")
	void ExitSpeedtrap(const FString& PlayerId, const FString& SpeedtrapId, float ExitSpeed);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Detection")
	bool IsInSpeedtrapTrigger(FVector Location, const FMGSpeedtrapDefinition& Speedtrap) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Detection")
	bool IsInSpeedtrapZone(FVector Location, const FMGSpeedtrapDefinition& Speedtrap) const;

	// Active State
	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	FMGActiveSpeedtrap GetActiveSpeedtrap(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	bool HasActiveSpeedtrap(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	float GetCurrentSpeed(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	EMGSpeedtrapRating GetCurrentRating(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|State")
	void CancelActiveSpeedtrap(const FString& PlayerId);

	// Recording
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Recording")
	FMGSpeedtrapAttempt RecordCameraSpeed(const FString& PlayerId, const FString& SpeedtrapId, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Recording")
	FMGSpeedtrapAttempt RecordZoneCompletion(const FString& PlayerId, const FString& SpeedtrapId);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Recording")
	FMGSpeedtrapAttempt RecordJumpDistance(const FString& PlayerId, const FString& SpeedtrapId, float Distance);

	// Rating Calculation
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	EMGSpeedtrapRating CalculateRating(const FString& SpeedtrapId, float Value) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	int32 GetPointsForRating(const FString& SpeedtrapId, EMGSpeedtrapRating Rating) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	float GetThresholdForRating(const FString& SpeedtrapId, EMGSpeedtrapRating Rating) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	float GetNextRatingThreshold(const FString& SpeedtrapId, EMGSpeedtrapRating CurrentRating) const;

	// Records
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Records")
	FMGSpeedtrapRecord GetSpeedtrapRecord(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Records")
	float GetPersonalBest(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Records")
	float GetWorldRecord(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Records")
	void SetWorldRecord(const FString& SpeedtrapId, float Value, const FString& PlayerName);

	// Leaderboards
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Leaderboard")
	TArray<FMGSpeedtrapLeaderboardEntry> GetLeaderboard(const FString& SpeedtrapId, int32 MaxEntries = 10) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Leaderboard")
	TArray<FMGSpeedtrapLeaderboardEntry> GetFriendsLeaderboard(const FString& SpeedtrapId, const TArray<FString>& FriendIds) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Leaderboard")
	int32 GetPlayerRank(const FString& SpeedtrapId) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	FMGSpeedtrapPlayerStats GetPlayerStats() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	int32 GetTotalSpeedtrapsDiscovered() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	int32 GetTotalRatingsAtLevel(EMGSpeedtrapRating Rating) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	float GetCompletionPercentage() const;

	// Discovery
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Discovery")
	void DiscoverSpeedtrap(const FString& SpeedtrapId);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Discovery")
	bool IsSpeedtrapDiscovered(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Discovery")
	TArray<FString> GetDiscoveredSpeedtraps() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Discovery")
	TArray<FString> GetUndiscoveredSpeedtraps() const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Config")
	void SetSpeedZoneConfig(const FMGSpeedZoneConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Config")
	FMGSpeedZoneConfig GetSpeedZoneConfig() const;

	// Unit Conversion
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Utility")
	float ConvertToMPH(float CMPerSecond) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Utility")
	float ConvertToKPH(float CMPerSecond) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Utility")
	FText FormatSpeed(float SpeedMPH, bool bUseMetric = false) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Persistence")
	void SaveSpeedtrapData();

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Persistence")
	void LoadSpeedtrapData();

protected:
	void UpdateActiveZone(const FString& PlayerId, float Speed, float DeltaTime);
	FMGSpeedtrapAttempt FinalizeAttempt(const FString& PlayerId, const FString& SpeedtrapId, float RecordedValue);
	void UpdateRecords(const FMGSpeedtrapAttempt& Attempt);
	void CheckForRatingAchievement(EMGSpeedtrapRating Rating);
	FString GenerateAttemptId() const;

private:
	UPROPERTY()
	TMap<FString, FMGSpeedtrapDefinition> RegisteredSpeedtraps;

	UPROPERTY()
	TMap<FString, FMGActiveSpeedtrap> ActiveSpeedtraps;

	UPROPERTY()
	FMGSpeedtrapPlayerStats PlayerStats;

	UPROPERTY()
	TArray<FString> DiscoveredSpeedtrapIds;

	UPROPERTY()
	TMap<FString, TArray<FMGSpeedtrapLeaderboardEntry>> Leaderboards;

	UPROPERTY()
	FMGSpeedZoneConfig SpeedZoneConfig;

	UPROPERTY()
	int32 AttemptCounter = 0;

	FTimerHandle SpeedtrapTickTimer;
};
