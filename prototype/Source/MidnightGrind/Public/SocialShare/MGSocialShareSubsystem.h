// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSocialShareSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGSharePlatform : uint8
{
	None,
	Twitter,
	Facebook,
	Instagram,
	TikTok,
	YouTube,
	Twitch,
	Discord,
	Reddit,
	Steam,
	PlayStation,
	Xbox,
	System
};

UENUM(BlueprintType)
enum class EMGShareContentType : uint8
{
	Screenshot,
	VideoClip,
	PhotoModeImage,
	ReplayHighlight,
	Achievement,
	Milestone,
	RaceResult,
	Livery,
	VehicleShowcase,
	LeaderboardRank,
	CrewInvite,
	Custom
};

UENUM(BlueprintType)
enum class EMGShareStatus : uint8
{
	Pending,
	Processing,
	Uploading,
	Posted,
	Failed,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGVideoQuality : uint8
{
	Low,       // 720p 30fps
	Medium,    // 1080p 30fps
	High,      // 1080p 60fps
	Ultra      // 4K 60fps
};

USTRUCT(BlueprintType)
struct FMGShareableContent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ContentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShareContentType Type = EMGShareContentType::Screenshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ThumbnailPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FileSizeBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Resolution = FIntPoint(1920, 1080);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Metadata;
};

USTRUCT(BlueprintType)
struct FMGShareRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGShareableContent Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGSharePlatform> TargetPlatforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeWatermark = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeGameTag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShareStatus Status = EMGShareStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UploadProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RequestTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGSharePlatform, FString> PostURLs;
};

USTRUCT(BlueprintType)
struct FMGShareSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoWatermark = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludePlayerName = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeVehicleInfo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeTrackInfo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVideoQuality DefaultVideoQuality = EMGVideoQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxClipDurationSeconds = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefaultHashtags = TEXT("#MidnightGrind #Racing");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGSharePlatform> LinkedPlatforms;
};

USTRUCT(BlueprintType)
struct FMGClipRecording
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RecordingID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRecording = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVideoQuality Quality = EMGVideoQuality::High;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeAudio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIncludeMic = false;
};

USTRUCT(BlueprintType)
struct FMGSocialStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalShares = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScreenshotsShared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ClipsShared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AchievementsShared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGSharePlatform, int32> SharesByPlatform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastShareTime;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenshotCaptured, const FMGShareableContent&, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipRecorded, const FMGShareableContent&, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShareRequestCreated, const FMGShareRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShareProgressUpdated, const FString&, RequestID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShareCompleted, const FString&, RequestID, const TArray<FString>&, PostURLs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShareFailed, const FString&, RequestID, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClipRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipRecordingStopped, const FMGShareableContent&, Content);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlatformLinked, EMGSharePlatform, Platform, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformUnlinked, EMGSharePlatform, Platform);

UCLASS()
class MIDNIGHTGRIND_API UMGSocialShareSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Screenshot
	UFUNCTION(BlueprintCallable, Category = "SocialShare|Screenshot")
	void CaptureScreenshot();

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Screenshot")
	void CaptureScreenshotWithUI(bool bIncludeUI);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Screenshot")
	void CapturePhotoModeImage(const FString& FilterName = TEXT(""));

	UFUNCTION(BlueprintPure, Category = "SocialShare|Screenshot")
	TArray<FMGShareableContent> GetRecentScreenshots(int32 Count = 10) const;

	// Video Recording
	UFUNCTION(BlueprintCallable, Category = "SocialShare|Recording")
	bool StartClipRecording(float MaxDurationSeconds = 60.0f, EMGVideoQuality Quality = EMGVideoQuality::High);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Recording")
	void StopClipRecording();

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Recording")
	void CancelClipRecording();

	UFUNCTION(BlueprintPure, Category = "SocialShare|Recording")
	bool IsRecording() const { return CurrentRecording.bIsRecording; }

	UFUNCTION(BlueprintPure, Category = "SocialShare|Recording")
	FMGClipRecording GetCurrentRecording() const { return CurrentRecording; }

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Recording")
	void SaveReplayAsClip(float StartTime, float EndTime);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Recording")
	void SaveLastNSeconds(float Seconds);

	UFUNCTION(BlueprintPure, Category = "SocialShare|Recording")
	TArray<FMGShareableContent> GetRecentClips(int32 Count = 10) const;

	// Sharing
	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	FString ShareContent(const FMGShareableContent& Content, const TArray<EMGSharePlatform>& Platforms, const FString& Message = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	FString ShareScreenshot(const FString& ContentID, const TArray<EMGSharePlatform>& Platforms);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	FString ShareClip(const FString& ContentID, const TArray<EMGSharePlatform>& Platforms);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	FString ShareAchievement(FName AchievementID, const TArray<EMGSharePlatform>& Platforms);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	FString ShareRaceResult(const FString& RaceID, const TArray<EMGSharePlatform>& Platforms);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	FString ShareLivery(const FString& LiveryID, const TArray<EMGSharePlatform>& Platforms);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Share")
	void CancelShare(const FString& RequestID);

	UFUNCTION(BlueprintPure, Category = "SocialShare|Share")
	FMGShareRequest GetShareRequest(const FString& RequestID) const;

	UFUNCTION(BlueprintPure, Category = "SocialShare|Share")
	TArray<FMGShareRequest> GetActiveShareRequests() const;

	// Quick Share
	UFUNCTION(BlueprintCallable, Category = "SocialShare|QuickShare")
	void QuickShareToClipboard(const FMGShareableContent& Content);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|QuickShare")
	void QuickShareToSystemDialog(const FMGShareableContent& Content);

	// Platform Integration
	UFUNCTION(BlueprintCallable, Category = "SocialShare|Platforms")
	void LinkPlatform(EMGSharePlatform Platform);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Platforms")
	void UnlinkPlatform(EMGSharePlatform Platform);

	UFUNCTION(BlueprintPure, Category = "SocialShare|Platforms")
	bool IsPlatformLinked(EMGSharePlatform Platform) const;

	UFUNCTION(BlueprintPure, Category = "SocialShare|Platforms")
	TArray<EMGSharePlatform> GetLinkedPlatforms() const;

	UFUNCTION(BlueprintPure, Category = "SocialShare|Platforms")
	TArray<EMGSharePlatform> GetAvailablePlatforms() const;

	// Settings
	UFUNCTION(BlueprintPure, Category = "SocialShare|Settings")
	FMGShareSettings GetShareSettings() const { return Settings; }

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Settings")
	void UpdateShareSettings(const FMGShareSettings& NewSettings);

	// Content Management
	UFUNCTION(BlueprintPure, Category = "SocialShare|Content")
	TArray<FMGShareableContent> GetAllContent() const { return ContentLibrary; }

	UFUNCTION(BlueprintPure, Category = "SocialShare|Content")
	TArray<FMGShareableContent> GetContentByType(EMGShareContentType Type) const;

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Content")
	bool DeleteContent(const FString& ContentID);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Content")
	void CleanupOldContent(int32 MaxAgeDays = 30);

	UFUNCTION(BlueprintPure, Category = "SocialShare|Content")
	int64 GetTotalStorageUsed() const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "SocialShare|Stats")
	FMGSocialStats GetSocialStats() const { return Stats; }

	// Watermark
	UFUNCTION(BlueprintCallable, Category = "SocialShare|Watermark")
	void SetCustomWatermark(UTexture2D* Watermark);

	UFUNCTION(BlueprintCallable, Category = "SocialShare|Watermark")
	void ClearCustomWatermark();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnScreenshotCaptured OnScreenshotCaptured;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnClipRecorded OnClipRecorded;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnShareRequestCreated OnShareRequestCreated;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnShareProgressUpdated OnShareProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnShareCompleted OnShareCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnShareFailed OnShareFailed;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnClipRecordingStarted OnClipRecordingStarted;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnClipRecordingStopped OnClipRecordingStopped;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnPlatformLinked OnPlatformLinked;

	UPROPERTY(BlueprintAssignable, Category = "SocialShare|Events")
	FOnPlatformUnlinked OnPlatformUnlinked;

protected:
	void ProcessShareRequest(FMGShareRequest& Request);
	void SimulateUpload(const FString& RequestID);
	FMGShareableContent CreateContentRecord(EMGShareContentType Type, const FString& FilePath);
	FString GenerateContentID() const;
	FString GenerateRequestID() const;
	void OnRecordingTick();
	FString GetPlatformName(EMGSharePlatform Platform) const;

	UPROPERTY()
	TArray<FMGShareableContent> ContentLibrary;

	UPROPERTY()
	TArray<FMGShareRequest> ActiveRequests;

	UPROPERTY()
	FMGShareSettings Settings;

	UPROPERTY()
	FMGClipRecording CurrentRecording;

	UPROPERTY()
	FMGSocialStats Stats;

	UPROPERTY()
	TSet<EMGSharePlatform> LinkedPlatformsSet;

	UPROPERTY()
	UTexture2D* CustomWatermark = nullptr;

	FTimerHandle RecordingTimerHandle;
};
