// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGClipSubsystem.h
 * @brief Video Clip Recording, Management, and Social Sharing System
 * @author Midnight Grind Team
 * @date 2024
 *
 * @section overview_sec Overview
 * This subsystem handles video recording, clip management, and sharing functionality.
 * Players can capture gameplay moments, save highlights, edit clips, and share to
 * social media platforms directly from the game.
 *
 * @section quickstart_sec Quick Start Example
 * @code
 * // Get the subsystem
 * UMGClipSubsystem* Clips = GetGameInstance()->GetSubsystem<UMGClipSubsystem>();
 *
 * // Save the last 30 seconds (replay buffer) when something cool happens
 * FGuid ClipId = Clips->SaveLastSeconds(30.0f, "Epic Drift Victory");
 *
 * // Or start/stop manual recording
 * Clips->StartRecording();
 * // ... gameplay ...
 * FGuid ClipId = Clips->StopRecording("My Race Highlights");
 *
 * // Mark as favorite
 * Clips->SetClipFavorite(ClipId, true);
 *
 * // Share to social media
 * Clips->ShareClip(ClipId, EMGSharePlatform::YouTube);
 *
 * // Listen for highlight detection
 * Clips->OnHighlightDetected.AddDynamic(this, &UMyClass::OnHighlightFound);
 * @endcode
 *
 * @section concepts_sec Key Concepts for Beginners
 *
 * @subsection clip_subsec What is a Clip?
 * A short video recording of gameplay (typically 10-120 seconds) used to capture
 * memorable moments like race wins, photo finishes, or epic drifts. Clips are
 * stored locally and can be shared to social media platforms.
 *
 * @subsection buffer_subsec Replay Buffer
 * The game continuously records the last X seconds in the background:
 * - When something cool happens, save that buffer as a clip
 * - This is how "Save last 30 seconds" features work in modern games
 * - Configurable via FMGClipSettings::BufferDuration
 *
 * @subsection highlight_subsec Automatic Highlight Detection
 * The system automatically detects exciting moments:
 * - Overtakes, near misses, victories
 * - Photo finishes, epic drifts, crashes
 * - Each highlight has a "score" indicating excitement level
 * - OnHighlightDetected fires when a moment is detected
 *
 * @subsection categories_subsec Clip Categories (EMGClipCategory)
 * | Category    | Description                           |
 * |-------------|---------------------------------------|
 * | General     | Uncategorized clips                   |
 * | Highlight   | Auto-detected exciting moments        |
 * | PhotoFinish | Close race finishes                   |
 * | NearMiss    | Close calls with traffic/obstacles    |
 * | Overtake    | Passing maneuvers                     |
 * | Drift       | Impressive drift sequences            |
 * | Crash       | Spectacular crashes                   |
 * | Victory     | Race wins                             |
 * | Custom      | User-defined category                 |
 *
 * @section workflow_sec Typical Workflow
 * @verbatim
 * 1. Game constantly buffers last 60 seconds
 * 2. Player gets a photo finish victory
 * 3. System detects this as a "highlight moment"
 * 4. OnHighlightDetected fires, UI shows "Save Clip?" prompt
 * 5. Player saves clip -> SaveLastSeconds(30.0f)
 * 6. Clip is processed, thumbnail generated, metadata saved
 * 7. Player can later share to YouTube/Twitter via ShareClip()
 * @endverbatim
 *
 * @section editing_sec Clip Editing
 * @code
 * // Trim a clip
 * FGuid TrimmedClipId = Clips->TrimClip(OriginalClipId, 5.0f, 25.0f);
 *
 * // Create an edited version with effects
 * FMGClipEditSettings EditSettings;
 * EditSettings.StartTime = 5.0f;
 * EditSettings.EndTime = 25.0f;
 * EditSettings.bSlowMotionAtEnd = true;
 * EditSettings.SlowMotionSpeed = 0.5f;
 * EditSettings.bAddWatermark = true;
 * FGuid EditedClipId = Clips->CreateEditedClip(OriginalClipId, EditSettings);
 * @endcode
 *
 * @section storage_sec Storage Management
 * Clips consume disk space and the system helps manage it:
 * - FMGClipSettings::MaxStoredClips limits total clip count
 * - FMGClipSettings::MaxStorageSizeMB limits total storage
 * - EnforceStorageLimits() automatically deletes old clips (respects favorites)
 * - GetRemainingStorage() returns available space
 *
 * @section events_subsec Delegates/Events
 * | Event               | Description                           |
 * |---------------------|---------------------------------------|
 * | OnRecordingStarted  | Manual recording began                |
 * | OnRecordingStopped  | Manual recording ended                |
 * | OnClipReady         | Clip fully processed and ready        |
 * | OnClipShared        | Clip uploaded to a platform           |
 * | OnClipDeleted       | Clip removed from storage             |
 * | OnHighlightDetected | Exciting moment auto-detected         |
 * | OnAutoClipSaved     | Automatic highlight clip saved        |
 * | OnClipUploadProgress| Upload percentage update              |
 *
 * @section related_sec Related Files
 * - MGClipSubsystem.cpp: Implementation
 * - MGPlatformIntegrationSubsystem.h: Platform capture features
 *
 * @see EMGClipQuality, EMGClipStatus, EMGClipCategory, EMGSharePlatform
 * @see FMGClipMetadata, FMGClipSettings, FMGClipEditSettings, FMGHighlightMoment
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Texture2D.h"
#include "MGClipSubsystem.generated.h"

// Forward declarations
class UTexture2D;

UENUM(BlueprintType)
enum class EMGClipQuality : uint8
{
	Low,
	Medium,
	High,
	Ultra
};

UENUM(BlueprintType)
enum class EMGClipStatus : uint8
{
	Recording,
	Processing,
	Ready,
	Uploading,
	Uploaded,
	Failed,
	Deleted
};

UENUM(BlueprintType)
enum class EMGClipCategory : uint8
{
	General,
	Highlight,
	PhotoFinish,
	NearMiss,
	Overtake,
	Drift,
	Crash,
	Victory,
	Custom
};

UENUM(BlueprintType)
enum class EMGSharePlatform : uint8
{
	Internal,
	YouTube,
	Twitter,
	TikTok,
	Discord,
	Clipboard
};

USTRUCT(BlueprintType)
struct FMGClipMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ClipID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGClipCategory Category = EMGClipCategory::General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGClipStatus Status = EMGClipStatus::Ready;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGClipQuality Quality = EMGClipQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 FileSizeBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ThumbnailPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FeaturedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHighlight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFavorite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShared = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShareURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ViewCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LikeCount = 0;
};

USTRUCT(BlueprintType)
struct FMGClipSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGClipQuality DefaultQuality = EMGClipQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultClipLength = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxClipLength = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BufferDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoCapture = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCaptureVictories = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCaptureHighlights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCaptureCloseFinishes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeVoiceChat = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludePlayerNames = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStoredClips = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MaxStorageSizeMB = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ThumbnailWidth = 320;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ThumbnailHeight = 180;
};

USTRUCT(BlueprintType)
struct FMGClipEditSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaybackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlowMotionAtEnd = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAddMusicOverlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MusicTrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MusicVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameAudioVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAddWatermark = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WatermarkText;
};

USTRUCT(BlueprintType)
struct FMGHighlightMoment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGClipCategory Category = EMGClipCategory::Highlight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> InvolvedPlayers;
};

USTRUCT(BlueprintType)
struct FMGClipStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalClipsRecorded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalClipsShared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalViews = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLikes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalStorageUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRecordedDuration = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecordingStopped, FGuid, ClipID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipReady, const FMGClipMetadata&, Clip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClipShared, FGuid, ClipID, EMGSharePlatform, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipDeleted, FGuid, ClipID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighlightDetected, const FMGHighlightMoment&, Highlight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutoClipSaved, const FMGClipMetadata&, Clip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClipUploadProgress, FGuid, ClipID, float, Progress);

UCLASS()
class MIDNIGHTGRIND_API UMGClipSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Recording
	UFUNCTION(BlueprintCallable, Category = "Clip|Recording")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category = "Clip|Recording")
	FGuid StopRecording(const FString& Title = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Clip|Recording")
	FGuid SaveLastSeconds(float Seconds, const FString& Title = TEXT(""));

	UFUNCTION(BlueprintPure, Category = "Clip|Recording")
	bool IsRecording() const { return bIsRecording; }

	UFUNCTION(BlueprintPure, Category = "Clip|Recording")
	float GetRecordingDuration() const;

	UFUNCTION(BlueprintPure, Category = "Clip|Recording")
	float GetBufferDuration() const;

	// Clip Management
	UFUNCTION(BlueprintPure, Category = "Clip|Management")
	TArray<FMGClipMetadata> GetAllClips() const { return SavedClips; }

	UFUNCTION(BlueprintPure, Category = "Clip|Management")
	TArray<FMGClipMetadata> GetClipsByCategory(EMGClipCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Clip|Management")
	TArray<FMGClipMetadata> GetFavoriteClips() const;

	UFUNCTION(BlueprintPure, Category = "Clip|Management")
	FMGClipMetadata GetClip(FGuid ClipID) const;

	UFUNCTION(BlueprintCallable, Category = "Clip|Management")
	bool DeleteClip(FGuid ClipID);

	UFUNCTION(BlueprintCallable, Category = "Clip|Management")
	void SetClipFavorite(FGuid ClipID, bool bFavorite);

	UFUNCTION(BlueprintCallable, Category = "Clip|Management")
	void RenameClip(FGuid ClipID, const FString& NewTitle);

	UFUNCTION(BlueprintCallable, Category = "Clip|Management")
	void SetClipCategory(FGuid ClipID, EMGClipCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Clip|Management")
	void AddClipTag(FGuid ClipID, const FString& Tag);

	UFUNCTION(BlueprintCallable, Category = "Clip|Management")
	void RemoveClipTag(FGuid ClipID, const FString& Tag);

	// Editing
	UFUNCTION(BlueprintCallable, Category = "Clip|Editing")
	FGuid TrimClip(FGuid ClipID, float StartTime, float EndTime);

	UFUNCTION(BlueprintCallable, Category = "Clip|Editing")
	FGuid CreateEditedClip(FGuid SourceClipID, const FMGClipEditSettings& EditSettings);

	UFUNCTION(BlueprintCallable, Category = "Clip|Editing")
	bool ExportClip(FGuid ClipID, const FString& OutputPath, EMGClipQuality Quality);

	// Sharing
	UFUNCTION(BlueprintCallable, Category = "Clip|Sharing")
	bool ShareClip(FGuid ClipID, EMGSharePlatform Platform);

	UFUNCTION(BlueprintCallable, Category = "Clip|Sharing")
	FString GetShareURL(FGuid ClipID) const;

	UFUNCTION(BlueprintCallable, Category = "Clip|Sharing")
	void CopyClipToClipboard(FGuid ClipID);

	UFUNCTION(BlueprintPure, Category = "Clip|Sharing")
	bool IsClipUploading(FGuid ClipID) const;

	UFUNCTION(BlueprintPure, Category = "Clip|Sharing")
	float GetUploadProgress(FGuid ClipID) const;

	// Highlights
	UFUNCTION(BlueprintCallable, Category = "Clip|Highlights")
	void RegisterHighlightMoment(const FMGHighlightMoment& Moment);

	UFUNCTION(BlueprintPure, Category = "Clip|Highlights")
	TArray<FMGHighlightMoment> GetRecentHighlights() const { return RecentHighlights; }

	UFUNCTION(BlueprintCallable, Category = "Clip|Highlights")
	FGuid SaveHighlight(const FMGHighlightMoment& Highlight);

	UFUNCTION(BlueprintCallable, Category = "Clip|Highlights")
	void ClearHighlights();

	// Auto-capture
	UFUNCTION(BlueprintCallable, Category = "Clip|Auto")
	void TriggerVictoryCapture();

	UFUNCTION(BlueprintCallable, Category = "Clip|Auto")
	void TriggerCloseFinishCapture();

	UFUNCTION(BlueprintCallable, Category = "Clip|Auto")
	void TriggerHighlightCapture(EMGClipCategory Category, const FText& Description);

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Clip|Settings")
	void SetClipSettings(const FMGClipSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Clip|Settings")
	FMGClipSettings GetClipSettings() const { return Settings; }

	// Stats
	UFUNCTION(BlueprintPure, Category = "Clip|Stats")
	FMGClipStats GetClipStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Clip|Stats")
	int64 GetTotalStorageUsed() const;

	UFUNCTION(BlueprintPure, Category = "Clip|Stats")
	int64 GetRemainingStorage() const;

	// Thumbnail
	UFUNCTION(BlueprintCallable, Category = "Clip|Thumbnail")
	void GenerateThumbnail(FGuid ClipID, float AtTime = 0.0f);

	UFUNCTION(BlueprintPure, Category = "Clip|Thumbnail")
	TSoftObjectPtr<UTexture2D> GetClipThumbnail(FGuid ClipID) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnRecordingStarted OnRecordingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnRecordingStopped OnRecordingStopped;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnClipReady OnClipReady;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnClipShared OnClipShared;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnClipDeleted OnClipDeleted;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnHighlightDetected OnHighlightDetected;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnAutoClipSaved OnAutoClipSaved;

	UPROPERTY(BlueprintAssignable, Category = "Clip|Events")
	FOnClipUploadProgress OnClipUploadProgress;

protected:
	void OnClipTick();
	void ProcessRecordingBuffer();
	void EnforceStorageLimits();
	FString GenerateClipPath(FGuid ClipID) const;
	void UpdateStats();
	void SaveClipData();
	void LoadClipData();

	UPROPERTY()
	TArray<FMGClipMetadata> SavedClips;

	UPROPERTY()
	TArray<FMGHighlightMoment> RecentHighlights;

	UPROPERTY()
	TMap<FGuid, float> UploadProgress;

	UPROPERTY()
	FMGClipSettings Settings;

	UPROPERTY()
	FMGClipStats Stats;

	UPROPERTY()
	bool bIsRecording = false;

	UPROPERTY()
	float RecordingStartTime = 0.0f;

	UPROPERTY()
	FString ClipStoragePath;

	FTimerHandle ClipTickHandle;
};
