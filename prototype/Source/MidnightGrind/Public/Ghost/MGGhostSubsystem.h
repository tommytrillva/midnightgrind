// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGGhostSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGGhostType : uint8
{
	Personal,
	Rival,
	WorldRecord,
	Friend,
	Developer,
	Challenge,
	Tutorial,
	AI
};

UENUM(BlueprintType)
enum class EMGGhostVisibility : uint8
{
	Full,
	Transparent,
	Outline,
	Trail,
	Markers,
	Hidden
};

UENUM(BlueprintType)
enum class EMGGhostState : uint8
{
	Idle,
	Recording,
	Playing,
	Paused,
	Finished,
	OutOfSync
};

UENUM(BlueprintType)
enum class EMGGhostComparison : uint8
{
	Ahead,
	Behind,
	Even,
	Unknown
};

USTRUCT(BlueprintType)
struct FMGGhostFrame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Throttle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Brake = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Steering = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gear = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNitroActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WheelFL = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WheelFR = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WheelRL = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WheelRR = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongTrack = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Sector = 0;
};

USTRUCT(BlueprintType)
struct FMGGhostData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid GhostID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGhostType GhostType = EMGGhostType::Personal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGGhostFrame> Frames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> LapTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GameVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bValidated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWorldRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompressedSize = 0;
};

USTRUCT(BlueprintType)
struct FMGGhostInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGhostData GhostData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGhostState State = EMGGhostState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentFrameIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CurrentRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGhostVisibility Visibility = EMGGhostVisibility::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GhostColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Opacity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaybackSpeed = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGGhostComparator
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PlayerGhostID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RivalGhostID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeDifference = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceDifference = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGhostComparison Status = EMGGhostComparison::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorDifferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PredictedFinalDifference = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGGhostSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGhosts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGhostVisibility DefaultVisibility = EMGGhostVisibility::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxGhostsOnTrack = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowPersonalBest = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowWorldRecord = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRivalGhosts = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTimeDelta = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDistanceDelta = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GhostOpacity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PersonalBestColor = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WorldRecordColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RivalColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecordingInterval = 0.033f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoSavePersonalBest = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompressGhostData = true;
};

USTRUCT(BlueprintType)
struct FMGGhostLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid GhostID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDownloaded = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostRecordingStarted, FGuid, GhostID, FName, TrackID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostRecordingCompleted, FGuid, GhostID, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostPlaybackStarted, FGuid, InstanceID, const FMGGhostData&, GhostData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGhostPlaybackCompleted, FGuid, InstanceID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostComparison, const FMGGhostComparator&, Comparison, EMGGhostComparison, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewPersonalBest, FName, TrackID, float, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostDownloaded, FGuid, GhostID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGhostUploaded, FGuid, GhostID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLeaderboardFetched, FName, TrackID, const TArray<FMGGhostLeaderboardEntry>&, Entries, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRivalGhostLoaded, FName, TrackID, FName, RivalID, bool, bSuccess);

UCLASS()
class MIDNIGHTGRIND_API UMGGhostSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Recording
	UFUNCTION(BlueprintCallable, Category = "Ghost|Record")
	FGuid StartRecording(FName TrackID, FName VehicleID, FName PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Record")
	void StopRecording(FGuid GhostID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Record")
	void CancelRecording(FGuid GhostID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Record")
	void RecordFrame(FGuid GhostID, const FMGGhostFrame& Frame);

	UFUNCTION(BlueprintPure, Category = "Ghost|Record")
	bool IsRecording() const;

	UFUNCTION(BlueprintPure, Category = "Ghost|Record")
	FGuid GetActiveRecordingID() const { return ActiveRecordingID; }

	UFUNCTION(BlueprintCallable, Category = "Ghost|Record")
	void MarkLapComplete(FGuid GhostID, float LapTime);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Record")
	void MarkSectorComplete(FGuid GhostID, int32 Sector, float SectorTime);

	// Playback
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	FGuid StartPlayback(const FMGGhostData& GhostData);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void StopPlayback(FGuid InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void PausePlayback(FGuid InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void ResumePlayback(FGuid InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void SetPlaybackTime(FGuid InstanceID, float Time);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void SetPlaybackSpeed(FGuid InstanceID, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void SetLooping(FGuid InstanceID, bool bLoop);

	UFUNCTION(BlueprintPure, Category = "Ghost|Playback")
	FMGGhostInstance GetGhostInstance(FGuid InstanceID) const;

	UFUNCTION(BlueprintPure, Category = "Ghost|Playback")
	TArray<FMGGhostInstance> GetActiveGhosts() const;

	UFUNCTION(BlueprintPure, Category = "Ghost|Playback")
	FMGGhostFrame GetCurrentFrame(FGuid InstanceID) const;

	// Ghost Management
	UFUNCTION(BlueprintCallable, Category = "Ghost|Manage")
	bool SaveGhost(const FMGGhostData& GhostData);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Manage")
	bool LoadGhost(FGuid GhostID, FMGGhostData& OutGhostData);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Manage")
	bool DeleteGhost(FGuid GhostID);

	UFUNCTION(BlueprintPure, Category = "Ghost|Manage")
	TArray<FMGGhostData> GetSavedGhosts(FName TrackID) const;

	UFUNCTION(BlueprintPure, Category = "Ghost|Manage")
	FMGGhostData GetPersonalBest(FName TrackID) const;

	UFUNCTION(BlueprintCallable, Category = "Ghost|Manage")
	void SetPersonalBest(FName TrackID, const FMGGhostData& GhostData);

	UFUNCTION(BlueprintPure, Category = "Ghost|Manage")
	bool HasPersonalBest(FName TrackID) const;

	// Comparison
	UFUNCTION(BlueprintCallable, Category = "Ghost|Compare")
	void StartComparison(FGuid PlayerGhostID, FGuid RivalGhostID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Compare")
	void StopComparison();

	UFUNCTION(BlueprintPure, Category = "Ghost|Compare")
	FMGGhostComparator GetCurrentComparison() const { return CurrentComparison; }

	UFUNCTION(BlueprintPure, Category = "Ghost|Compare")
	float GetTimeDelta(FGuid PlayerInstance, FGuid RivalInstance) const;

	UFUNCTION(BlueprintPure, Category = "Ghost|Compare")
	float GetDistanceDelta(FGuid PlayerInstance, FGuid RivalInstance) const;

	UFUNCTION(BlueprintPure, Category = "Ghost|Compare")
	EMGGhostComparison GetComparisonStatus() const;

	// Visualization
	UFUNCTION(BlueprintCallable, Category = "Ghost|Visual")
	void SetGhostVisibility(FGuid InstanceID, EMGGhostVisibility Visibility);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Visual")
	void SetGhostColor(FGuid InstanceID, FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Visual")
	void SetGhostOpacity(FGuid InstanceID, float Opacity);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Visual")
	void HideAllGhosts();

	UFUNCTION(BlueprintCallable, Category = "Ghost|Visual")
	void ShowAllGhosts();

	// Online
	UFUNCTION(BlueprintCallable, Category = "Ghost|Online")
	void DownloadGhost(FGuid GhostID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Online")
	void UploadGhost(const FMGGhostData& GhostData);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Online")
	void FetchLeaderboard(FName TrackID, int32 StartRank, int32 Count);

	UFUNCTION(BlueprintPure, Category = "Ghost|Online")
	TArray<FMGGhostLeaderboardEntry> GetLeaderboard(FName TrackID) const;

	UFUNCTION(BlueprintCallable, Category = "Ghost|Online")
	void DownloadRivalGhost(FName TrackID, int32 Rank);

	UFUNCTION(BlueprintPure, Category = "Ghost|Online")
	FMGGhostData GetWorldRecord(FName TrackID) const;

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Ghost|Settings")
	void SetGhostSettings(const FMGGhostSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Ghost|Settings")
	FMGGhostSettings GetGhostSettings() const { return Settings; }

	// Quick Actions
	UFUNCTION(BlueprintCallable, Category = "Ghost|Quick")
	void RacePersonalBest(FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Quick")
	void RaceWorldRecord(FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Quick")
	void RaceRival(FName TrackID, FName RivalID);

	UFUNCTION(BlueprintCallable, Category = "Ghost|Quick")
	void ClearActiveGhosts();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostRecordingStarted OnGhostRecordingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostRecordingCompleted OnGhostRecordingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostPlaybackStarted OnGhostPlaybackStarted;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostPlaybackCompleted OnGhostPlaybackCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostComparison OnGhostComparison;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostDownloaded OnGhostDownloaded;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnGhostUploaded OnGhostUploaded;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnLeaderboardFetched OnLeaderboardFetched;

	UPROPERTY(BlueprintAssignable, Category = "Ghost|Events")
	FOnRivalGhostLoaded OnRivalGhostLoaded;

protected:
	void OnGhostTick();
	void UpdatePlayback(float DeltaTime);
	void UpdateComparison();
	FMGGhostFrame InterpolateFrame(const FMGGhostData& GhostData, float Time) const;
	int32 FindFrameIndex(const FMGGhostData& GhostData, float Time) const;
	void CompressGhostData(FMGGhostData& GhostData);
	void DecompressGhostData(FMGGhostData& GhostData);
	void SaveGhostToFile(const FMGGhostData& GhostData);
	bool LoadGhostFromFile(FGuid GhostID, FMGGhostData& OutData);
	void SaveGhostIndex();
	void LoadGhostIndex();

	UPROPERTY()
	FMGGhostSettings Settings;

	UPROPERTY()
	TMap<FGuid, FMGGhostData> ActiveRecordings;

	UPROPERTY()
	FGuid ActiveRecordingID;

	UPROPERTY()
	TMap<FGuid, FMGGhostInstance> ActivePlaybacks;

	UPROPERTY()
	TMap<FName, FMGGhostData> PersonalBests;

	UPROPERTY()
	TMap<FName, FMGGhostData> WorldRecords;

	UPROPERTY()
	TMap<FName, TArray<FMGGhostLeaderboardEntry>> Leaderboards;

	UPROPERTY()
	TMap<FGuid, FMGGhostData> GhostCache;

	UPROPERTY()
	TArray<FGuid> GhostIndex;

	UPROPERTY()
	FMGGhostComparator CurrentComparison;

	UPROPERTY()
	bool bComparing = false;

	FTimerHandle GhostTickHandle;
};
