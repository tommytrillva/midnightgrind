// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGGhostSubsystem.h
 * @brief Ghost Recording and Playback System for Racing Time Trials
 *
 * @section overview Overview
 * This file defines the Ghost Subsystem, which records and plays back "ghost" vehicles
 * that represent previous race performances. Ghosts are transparent representations of
 * a vehicle's path through a race, allowing players to compete against their own best
 * times, world records, friends, or rivals.
 *
 * @section beginners Key Concepts for Beginners
 *
 * @subsection what_is_ghost What is a Racing Ghost?
 * A "ghost" in racing games is a semi-transparent replay of a previous race run.
 * Think of it like a recording of someone driving - you can see where they went,
 * how fast they were going, and compete directly against that recording.
 *
 * Common uses:
 * - Racing against your personal best time
 * - Competing with world record holders
 * - Practicing by following faster players' lines
 * - Social competition with friends/rivals
 *
 * @subsection recording How Recording Works
 * During a race, the system captures "frames" of vehicle data at regular intervals
 * (default: ~30fps). Each frame stores:
 * - Position and rotation (where the car is)
 * - Velocity and speed (how fast it's moving)
 * - Control inputs (throttle, brake, steering)
 * - Vehicle state (gear, RPM, nitro, drifting)
 * - Track progress (lap, sector, distance along track)
 *
 * @subsection playback How Playback Works
 * When playing back a ghost, the system:
 * 1. Spawns a semi-transparent vehicle actor
 * 2. Interpolates between recorded frames to get smooth movement
 * 3. Updates position/rotation each tick based on current playback time
 * 4. Calculates time deltas (how far ahead/behind the player is)
 *
 * @subsection ghost_types Ghost Types (EMGGhostType)
 * - Personal: Your own previous runs
 * - Rival: From players you're competing against
 * - WorldRecord: The fastest time globally
 * - Friend: From your friends list
 * - Developer: Official reference ghosts
 * - Challenge: For specific challenge events
 * - Tutorial: Teaching optimal racing lines
 * - AI: Computer-generated reference ghosts
 *
 * @subsection visibility Visibility Modes (EMGGhostVisibility)
 * - Full: Solid vehicle (can obstruct view)
 * - Transparent: Semi-see-through (most common)
 * - Outline: Just the vehicle silhouette
 * - Trail: Shows path as a line/ribbon
 * - Markers: Periodic position markers only
 * - Hidden: Ghost is tracked but not visible
 *
 * @section data_flow Data Flow
 * @code
 *   [Start Recording]
 *         |
 *         v
 *   [Capture Frames] --> [FMGGhostFrame array in FMGGhostData]
 *         |
 *   [Stop Recording]
 *         |
 *         v
 *   [Save/Upload Ghost]
 *         |
 *   ======|======= (later) =======
 *         |
 *         v
 *   [Load/Download Ghost]
 *         |
 *         v
 *   [Start Playback] --> [FMGGhostInstance tracks current state]
 *         |
 *         v
 *   [Interpolate frames based on time]
 *         |
 *         v
 *   [Update ghost vehicle position]
 * @endcode
 *
 * @section usage Usage Examples
 *
 * @subsection recording_example Recording a Ghost
 * @code
 * // Get the subsystem
 * UMGGhostSubsystem* GhostSystem = GetGameInstance()->GetSubsystem<UMGGhostSubsystem>();
 *
 * // Start recording at race start
 * FGuid RecordingID = GhostSystem->StartRecording(
 *     TEXT("Track_DowntownLoop"),
 *     TEXT("Vehicle_Nissan_GTR"),
 *     TEXT("Player_001")
 * );
 *
 * // Each frame during the race, record vehicle state
 * FMGGhostFrame Frame;
 * Frame.Timestamp = RaceTime;
 * Frame.Position = Vehicle->GetActorLocation();
 * Frame.Rotation = Vehicle->GetActorRotation();
 * Frame.Speed = Vehicle->GetCurrentSpeed();
 * Frame.Throttle = Vehicle->GetThrottleInput();
 * // ... fill other fields
 * GhostSystem->RecordFrame(RecordingID, Frame);
 *
 * // Mark lap/sector completions for split times
 * GhostSystem->MarkLapComplete(RecordingID, LapTime);
 *
 * // Stop recording at race end
 * GhostSystem->StopRecording(RecordingID);
 * // Ghost is automatically saved if it's a new personal best
 * @endcode
 *
 * @subsection playback_example Playing Back a Ghost
 * @code
 * // Load your personal best ghost
 * FMGGhostData PersonalBest = GhostSystem->GetPersonalBest(TEXT("Track_DowntownLoop"));
 *
 * // Start playback - returns an instance ID for this playback session
 * FGuid PlaybackID = GhostSystem->StartPlayback(PersonalBest);
 *
 * // Customize appearance
 * GhostSystem->SetGhostVisibility(PlaybackID, EMGGhostVisibility::Transparent);
 * GhostSystem->SetGhostColor(PlaybackID, FLinearColor::Blue);
 * GhostSystem->SetGhostOpacity(PlaybackID, 0.5f);
 *
 * // Get current interpolated state for rendering
 * FMGGhostFrame CurrentFrame = GhostSystem->GetCurrentFrame(PlaybackID);
 * GhostVehicleActor->SetActorLocation(CurrentFrame.Position);
 * GhostVehicleActor->SetActorRotation(CurrentFrame.Rotation);
 * @endcode
 *
 * @subsection comparison_example Comparing with Ghosts
 * @code
 * // Start comparing player vs personal best
 * GhostSystem->StartComparison(PlayerGhostID, PersonalBestGhostID);
 *
 * // Get current time difference
 * float TimeDelta = GhostSystem->GetTimeDelta(PlayerGhostID, PersonalBestGhostID);
 * // Negative = player is ahead, Positive = player is behind
 *
 * // Get comparison status
 * EMGGhostComparison Status = GhostSystem->GetComparisonStatus();
 * if (Status == EMGGhostComparison::Ahead)
 * {
 *     ShowGreenTimeDelta(-TimeDelta);  // Player is faster
 * }
 * else if (Status == EMGGhostComparison::Behind)
 * {
 *     ShowRedTimeDelta(TimeDelta);  // Player is slower
 * }
 * @endcode
 *
 * @subsection quick_race Quick Race Functions
 * @code
 * // Convenient shortcuts for common use cases
 * GhostSystem->RacePersonalBest(TEXT("Track_DowntownLoop"));  // Race your best
 * GhostSystem->RaceWorldRecord(TEXT("Track_DowntownLoop"));   // Race the WR
 * GhostSystem->RaceRival(TEXT("Track_DowntownLoop"), TEXT("Rival_Speedy")); // Race a rival
 * @endcode
 *
 * @section settings Ghost Settings
 * The FMGGhostSettings struct allows customization of ghost behavior:
 * - bShowGhosts: Master toggle for ghost visibility
 * - MaxGhostsOnTrack: Performance limit (default: 3)
 * - DefaultVisibility: How new ghosts appear
 * - PersonalBestColor/WorldRecordColor/RivalColor: Visual differentiation
 * - RecordingInterval: Frame capture rate (default: 33ms = ~30fps)
 * - bCompressGhostData: Reduces storage size
 *
 * @section compression Data Compression
 * Ghost data can become large (thousands of frames). The system supports:
 * - Removing redundant frames (no significant change)
 * - Quantizing position/rotation to reduce precision
 * - Run-length encoding for repeated values
 * CompressedSize in FMGGhostData tracks the compressed byte count.
 *
 * @section online Online Features
 * - UploadGhost(): Share ghosts to leaderboards
 * - DownloadGhost(): Get ghosts from other players
 * - FetchLeaderboard(): Get top times and ghost availability
 * - DownloadRivalGhost(): Get a specific rank's ghost
 *
 * @section events Events/Delegates
 * Subscribe to these for UI updates:
 * - OnGhostRecordingStarted/Completed: Recording lifecycle
 * - OnGhostPlaybackStarted/Completed: Playback lifecycle
 * - OnGhostComparison: Real-time time delta updates
 * - OnNewPersonalBest: Celebrate improvements!
 * - OnGhostDownloaded/Uploaded: Online operations complete
 * - OnLeaderboardFetched: Leaderboard data ready
 *
 * @see FMGGhostFrame For individual frame data structure
 * @see FMGGhostData For complete ghost recording structure
 * @see FMGGhostInstance For playback state tracking
 * @see FMGGhostSettings For configuration options
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGGhostSubsystem.generated.h"

// MOVED TO MGSharedTypes.h
// UENUM(BlueprintType)
// enum class EMGGhostType : uint8
// {
// 	Personal,
// 	Rival,
// 	WorldRecord,
// 	Friend,
// 	Developer,
// 	Challenge,
// 	Tutorial,
// 	AI
// };

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

// MOVED TO MGSharedTypes.h
// UENUM(BlueprintType)
// enum class EMGGhostState : uint8
// {
// 	Idle,
// 	Recording,
// 	Playing,
// 	Paused,
// 	Finished,
// 	OutOfSync
// };

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

//=============================================================================
// Wrapper Structs for TMap Value Types
//=============================================================================

/**
 * @brief Wrapper for TArray<FMGGhostLeaderboardEntry> to support UPROPERTY in TMap.
 */
USTRUCT(BlueprintType)
struct FMGGhostLeaderboardEntryArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGGhostLeaderboardEntry> Entries;
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
	void UpdatePlayback(float MGDeltaTime);
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
	TMap<FName, FMGGhostLeaderboardEntryArray> Leaderboards;

	UPROPERTY()
	TMap<FGuid, FMGGhostData> GhostCache;

	/** Downloaded ghosts from online services (temporary cache) */
	UPROPERTY()
	TMap<FGuid, FMGGhostData> DownloadedGhosts;

	UPROPERTY()
	TArray<FGuid> GhostIndex;

	UPROPERTY()
	FMGGhostComparator CurrentComparison;

	UPROPERTY()
	bool bComparing = false;

	FTimerHandle GhostTickHandle;
};
