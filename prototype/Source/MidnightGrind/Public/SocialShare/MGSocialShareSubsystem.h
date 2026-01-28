// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSocialShareSubsystem.h
 * @brief Social Sharing System - Screenshot capture, video recording, and social media integration
 * @author Midnight Grind Team
 * @version 1.0
 *
 * @section overview Overview
 * ============================================================================
 * MGSocialShareSubsystem.h
 * Midnight Grind - Social Sharing and Media Capture System
 * ============================================================================
 *
 * This subsystem handles screenshot capture, video clip recording, and sharing
 * content to social media platforms. This is the primary interface for players
 * to share their gaming moments with the world.
 *
 * @section features Key Features
 * - Screenshot capture (with/without UI, photo mode)
 * - Video clip recording (instant replay, live recording)
 * - Social media integration (Twitter, YouTube, Discord, etc.)
 * - Quick share options (clipboard, system share dialog)
 * - Content library management
 * - Custom watermarks
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection content 1. Shareable Content (FMGShareableContent)
 * Any content that can be shared: screenshots, video clips, achievements.
 * - Each piece of content has a unique ID, file path, thumbnail, metadata.
 * - Stored in a "content library" for later access.
 *
 * @subsection contenttypes 2. Content Types (EMGShareContentType)
 * - Screenshot: Single image capture of gameplay.
 * - VideoClip: Short recorded video (up to MaxClipDurationSeconds).
 * - PhotoModeImage: High-quality image from photo mode with filters.
 * - ReplayHighlight: Clip from replay system.
 * - Achievement/Milestone: Auto-generated content for accomplishments.
 * - RaceResult: Shareable race finish card.
 * - Livery/VehicleShowcase: Custom car designs.
 *
 * @subsection linking 3. Platform Linking
 * - Players connect their social accounts (Twitter, YouTube, etc.).
 * - LinkedPlatforms stores which accounts are connected.
 * - LinkPlatform() initiates OAuth flow for authentication.
 * - Platform-specific APIs handle actual posting.
 *
 * @subsection requests 4. Share Requests (FMGShareRequest)
 * - When sharing, a request is created and processed asynchronously.
 * - Can target multiple platforms simultaneously.
 * - Tracks upload progress and completion status.
 * - PostURLs map stores the resulting URL for each platform.
 *
 * @subsection recording 5. Video Recording
 * - StartClipRecording() begins capturing gameplay video.
 * - Recording has configurable quality (720p to 4K) and duration limits.
 * - SaveLastNSeconds() captures the immediate past (instant replay style).
 * - Includes options for game audio, voice chat, and microphone.
 *
 * @subsection screenshots 6. Screenshot Capture
 * - CaptureScreenshot() takes an immediate screenshot.
 * - CaptureScreenshotWithUI() option to include or exclude UI elements.
 * - CapturePhotoModeImage() for high-quality artistic shots with filters.
 *
 * @subsection quickshare 7. Quick Share
 * - QuickShareToClipboard(): Copy content for pasting elsewhere.
 * - QuickShareToSystemDialog(): Opens OS-native share sheet (mobile-style).
 * - Faster than full social media posting.
 *
 * @subsection watermarks 8. Watermarks
 * - Automatic game branding on shared content.
 * - CustomWatermark allows personalized overlay.
 * - Configurable via ShareSettings.
 *
 * @subsection quality 9. Video Quality (EMGVideoQuality)
 * - Low: 720p 30fps - Small files, fast upload.
 * - Medium: 1080p 30fps - Balanced quality and size.
 * - High: 1080p 60fps - Smooth, good quality.
 * - Ultra: 4K 60fps - Maximum quality, large files.
 *
 * @section usage Usage Examples
 *
 * @code
 * // Get the subsystem
 * UMGSocialShareSubsystem* Social = GetGameInstance()->GetSubsystem<UMGSocialShareSubsystem>();
 *
 * // === SCREENSHOTS ===
 * // Take a screenshot
 * Social->CaptureScreenshot();
 *
 * // Take a screenshot without UI elements
 * Social->CaptureScreenshotWithUI(false);
 *
 * // Take a photo mode image with a filter
 * Social->CapturePhotoModeImage("Cinematic");
 *
 * // Get recent screenshots for gallery
 * TArray<FMGShareableContent> Screenshots = Social->GetRecentScreenshots(10);
 *
 * // === VIDEO RECORDING ===
 * // Start recording a clip (60 seconds max, high quality)
 * Social->StartClipRecording(60.0f, EMGVideoQuality::High);
 *
 * // Stop recording when done
 * Social->StopClipRecording();
 *
 * // Or save the last 30 seconds of gameplay (instant replay)
 * Social->SaveLastNSeconds(30.0f);
 *
 * // Check if currently recording
 * if (Social->IsRecording())
 * {
 *     FMGClipRecording CurrentRec = Social->GetCurrentRecording();
 *     float Duration = CurrentRec.CurrentDuration;
 * }
 *
 * // === SHARING ===
 * // Share content to multiple platforms
 * TArray<EMGSharePlatform> Platforms;
 * Platforms.Add(EMGSharePlatform::Twitter);
 * Platforms.Add(EMGSharePlatform::Discord);
 *
 * FMGShareableContent MyContent = Screenshots[0];
 * FString RequestId = Social->ShareContent(MyContent, Platforms, "Just won an epic race!");
 *
 * // Quick share to clipboard
 * Social->QuickShareToClipboard(MyContent);
 *
 * // Quick share using system dialog (iOS/Android style)
 * Social->QuickShareToSystemDialog(MyContent);
 *
 * // === PLATFORM LINKING ===
 * // Link social media accounts
 * Social->LinkPlatform(EMGSharePlatform::Twitter);
 *
 * // Check which platforms are linked
 * TArray<EMGSharePlatform> Linked = Social->GetLinkedPlatforms();
 * bool bTwitterLinked = Social->IsPlatformLinked(EMGSharePlatform::Twitter);
 *
 * // === CONTENT MANAGEMENT ===
 * // Get all saved content
 * TArray<FMGShareableContent> AllContent = Social->GetAllContent();
 *
 * // Get content by type
 * TArray<FMGShareableContent> Clips = Social->GetContentByType(EMGShareContentType::VideoClip);
 *
 * // Check storage usage
 * int64 StorageUsed = Social->GetTotalStorageUsed();
 *
 * // Clean up old content (older than 30 days)
 * Social->CleanupOldContent(30);
 *
 * // === EVENT LISTENERS ===
 * Social->OnScreenshotCaptured.AddDynamic(this, &UMyClass::HandleScreenshot);
 * Social->OnClipRecorded.AddDynamic(this, &UMyClass::HandleClipReady);
 * Social->OnShareCompleted.AddDynamic(this, &UMyClass::HandleShareSuccess);
 * Social->OnShareFailed.AddDynamic(this, &UMyClass::HandleShareError);
 * @endcode
 *
 * @section workflow Typical Workflow Example
 * 1. Player wins race, sees "Share Victory?" prompt
 * 2. Player taps screenshot button -> CaptureScreenshot()
 * 3. OnScreenshotCaptured fires, UI shows preview
 * 4. Player adds message, selects Twitter + Discord
 * 5. ShareContent() called -> FMGShareRequest created
 * 6. OnShareProgressUpdated fires as upload proceeds
 * 7. OnShareCompleted fires with tweet URL and Discord link
 * 8. UI shows "Shared successfully!" with links
 *
 * @section storage Storage Management
 * - Content is stored locally in ContentLibrary.
 * - GetTotalStorageUsed() returns current disk usage.
 * - CleanupOldContent() removes content older than N days.
 * - Important for managing device storage on consoles/mobile.
 *
 * @section delegates Available Delegates
 * - OnScreenshotCaptured: Screenshot ready for use/preview.
 * - OnClipRecorded: Video recording finished processing.
 * - OnShareRequestCreated: New share request created.
 * - OnShareProgressUpdated: Upload progress changed (for progress bars).
 * - OnShareCompleted: Successfully posted with URLs.
 * - OnShareFailed: Error occurred during sharing.
 * - OnClipRecordingStarted: Video recording began.
 * - OnClipRecordingStopped: Recording stopped with content.
 * - OnPlatformLinked/Unlinked: Account connection state changed.
 *
 * @see UMGPhotoModeSubsystem For advanced photo mode features
 * @see UMGReplaySubsystem For replay recording and playback
 * ============================================================================
 */

/**
 * OVERVIEW:
 * This file defines the Social Share Subsystem for Midnight Grind. It handles
 * screenshot capture, video clip recording, and sharing content to social media
 * platforms. This is the primary interface for players to share their gaming
 * moments with the world.
 *
 * KEY CONCEPTS FOR ENTRY-LEVEL DEVELOPERS:
 *
 * 1. SHAREABLE CONTENT (FMGShareableContent):
 *    - Any content that can be shared: screenshots, video clips, achievements.
 *    - Each piece of content has a unique ID, file path, thumbnail, metadata.
 *    - Stored in a "content library" for later access.
 *
 * 2. CONTENT TYPES (EMGShareContentType):
 *    - Screenshot: Single image capture of gameplay.
 *    - VideoClip: Short recorded video (up to MaxClipDurationSeconds).
 *    - PhotoModeImage: High-quality image from photo mode with filters.
 *    - ReplayHighlight: Clip from replay system.
 *    - Achievement/Milestone: Auto-generated content for accomplishments.
 *    - RaceResult: Shareable race finish card.
 *    - Livery/VehicleShowcase: Custom car designs.
 *
 * 3. PLATFORM LINKING:
 *    - Players connect their social accounts (Twitter, YouTube, etc.).
 *    - LinkedPlatforms stores which accounts are connected.
 *    - LinkPlatform() initiates OAuth flow for authentication.
 *    - Platform-specific APIs handle actual posting.
 *
 * 4. SHARE REQUESTS (FMGShareRequest):
 *    - When sharing, a request is created and processed asynchronously.
 *    - Can target multiple platforms simultaneously.
 *    - Tracks upload progress and completion status.
 *    - PostURLs map stores the resulting URL for each platform.
 *
 * 5. VIDEO RECORDING:
 *    - StartClipRecording() begins capturing gameplay video.
 *    - Recording has configurable quality (720p to 4K) and duration limits.
 *    - SaveLastNSeconds() captures the immediate past (instant replay style).
 *    - Includes options for game audio, voice chat, and microphone.
 *
 * 6. SCREENSHOT CAPTURE:
 *    - CaptureScreenshot() takes an immediate screenshot.
 *    - CaptureScreenshotWithUI() option to include or exclude UI elements.
 *    - CapturePhotoModeImage() for high-quality artistic shots with filters.
 *
 * 7. QUICK SHARE:
 *    - QuickShareToClipboard(): Copy content for pasting elsewhere.
 *    - QuickShareToSystemDialog(): Opens OS-native share sheet (mobile-style).
 *    - Faster than full social media posting.
 *
 * 8. WATERMARKS:
 *    - Automatic game branding on shared content.
 *    - CustomWatermark allows personalized overlay.
 *    - Configurable via ShareSettings.
 *
 * 9. SOCIAL STATS (FMGSocialStats):
 *    - Tracks sharing activity: total shares, by platform, by content type.
 *    - Useful for analytics and player engagement features.
 *
 * 10. VIDEO QUALITY (EMGVideoQuality):
 *     - Low: 720p 30fps - Small files, fast upload.
 *     - Medium: 1080p 30fps - Balanced quality and size.
 *     - High: 1080p 60fps - Smooth, good quality.
 *     - Ultra: 4K 60fps - Maximum quality, large files.
 *
 * DELEGATES (Events):
 * - OnScreenshotCaptured: Screenshot ready for use/preview.
 * - OnClipRecorded: Video recording finished processing.
 * - OnShareProgressUpdated: Upload progress changed (for progress bars).
 * - OnShareCompleted: Successfully posted with URLs.
 * - OnShareFailed: Error occurred during sharing.
 * - OnPlatformLinked/Unlinked: Account connection state changed.
 *
 * STORAGE MANAGEMENT:
 * - Content is stored locally in ContentLibrary.
 * - GetTotalStorageUsed() returns current disk usage.
 * - CleanupOldContent() removes content older than N days.
 * - Important for managing device storage on consoles/mobile.
 *
 * WORKFLOW EXAMPLE:
 * 1. Player wins race, sees "Share Victory?" prompt
 * 2. Player taps screenshot button -> CaptureScreenshot()
 * 3. OnScreenshotCaptured fires, UI shows preview
 * 4. Player adds message, selects Twitter + Discord
 * 5. ShareContent() called -> FMGShareRequest created
 * 6. OnShareProgressUpdated fires as upload proceeds
 * 7. OnShareCompleted fires with tweet URL and Discord link
 * 8. UI shows "Shared successfully!" with links
 *
 * =============================================================================
 */

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
