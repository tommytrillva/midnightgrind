// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGReplayShareSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines the Replay Share Subsystem for Midnight Grind. It handles
 * clip editing, social media integration, video export, and community features.
 * This is the bridge between raw replay data and shareable content.
 *
 * KEY CONCEPTS FOR ENTRY-LEVEL DEVELOPERS:
 *
 * 1. CLIP vs. REPLAY:
 *    - REPLAY: Raw recorded gameplay data (from MGReplayBufferSubsystem).
 *    - CLIP: An edited, polished video created from replay data.
 *    - This subsystem converts replays into shareable clips with effects.
 *
 * 2. CLIP SEGMENTS (FMGClipSegment):
 *    - A clip can have multiple segments with different settings.
 *    - Each segment has: start/end time, playback speed, camera angle, effects.
 *    - Example: Segment 1 at normal speed, Segment 2 in slow motion.
 *
 * 3. CLIP EFFECTS (EMGClipEffect):
 *    - Visual effects applied to clips: SlowMotion, SpeedRamp, VHSFilter,
 *      NeonGlow, ChromaticAberration, FilmGrain, LetterBox, etc.
 *    - These add cinematic flair to raw gameplay footage.
 *
 * 4. SOCIAL PLATFORM INTEGRATION:
 *    - Connect accounts for Twitter, YouTube, TikTok, Discord, etc.
 *    - FMGSocialAccount stores connection state and tokens.
 *    - OAuth tokens expire, so IsTokenExpired() checks validity.
 *
 * 5. SHARE REQUESTS (FMGShareRequest):
 *    - When sharing a clip, a request is created and processed asynchronously.
 *    - Tracks status: Pending -> Processing -> Uploading -> Complete/Failed.
 *    - Progress callbacks let UI show upload progress bars.
 *
 * 6. COMMUNITY CLIPS (FMGCommunityClip):
 *    - Clips shared to the game's community hub.
 *    - Has social features: ViewCount, LikeCount, ShareCount, Comments.
 *    - bIsFeatured highlights clips selected by moderators.
 *
 * 7. EXPORT SETTINGS (FMGExportSettings):
 *    - Control output format (MP4, GIF, WebM), resolution (720p/1080p/4K).
 *    - Options for bitrate, framerate, audio inclusion, watermarks.
 *    - bAddIntroOutro can add game branding to the video.
 *
 * 8. WATERMARKS:
 *    - Game branding added to shared clips.
 *    - Default: "Midnight Grind" but customizable per clip.
 *    - Helps with game visibility when clips are shared.
 *
 * 9. DELEGATES (Events):
 *    - OnClipCreated: Fired when a new clip is created.
 *    - OnShareProgressUpdated: Fired during upload to update progress bar.
 *    - OnShareCompleted: Fired when share succeeds with result URL.
 *    - OnShareFailed: Fired on error with error message.
 *    - OnAccountConnected/Disconnected: For social account state changes.
 *
 * 10. HASHTAGS AND CAPTIONS:
 *     - Clips can include captions and hashtags for social posts.
 *     - These are platform-specific (Twitter character limits, etc.).
 *
 * WORKFLOW EXAMPLE:
 * 1. Player creates clip from replay: CreateClipFromReplay()
 * 2. Player edits clip: adds slow motion segment, VHS filter
 * 3. Player sets title, adds tags, chooses music
 * 4. Player exports: ExportClip() with 1080p settings
 * 5. Player shares: ShareClip() to Twitter with caption
 * 6. System uploads, fires OnShareCompleted with tweet URL
 * 7. Clip also uploaded to community: UploadToCommunity()
 *
 * ARCHITECTURE NOTE:
 * This subsystem works closely with MGReplayBufferSubsystem (provides replay
 * data) and MGSocialShareSubsystem (provides platform APIs). It acts as the
 * creative layer between raw recordings and social sharing.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGReplayShareSubsystem.generated.h"

// Forward declarations
class UMGReplayShareSubsystem;

/**
 * EMGSharePlatform - Social platforms for sharing
 */
UENUM(BlueprintType)
enum class EMGSharePlatform : uint8
{
    Twitter         UMETA(DisplayName = "Twitter/X"),
    YouTube         UMETA(DisplayName = "YouTube"),
    TikTok          UMETA(DisplayName = "TikTok"),
    Instagram       UMETA(DisplayName = "Instagram"),
    Facebook        UMETA(DisplayName = "Facebook"),
    Discord         UMETA(DisplayName = "Discord"),
    Reddit          UMETA(DisplayName = "Reddit"),
    Twitch          UMETA(DisplayName = "Twitch"),
    Steam           UMETA(DisplayName = "Steam"),
    PlayStation     UMETA(DisplayName = "PlayStation"),
    Xbox            UMETA(DisplayName = "Xbox"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * EMGClipType - Types of clips
 */
UENUM(BlueprintType)
enum class EMGClipType : uint8
{
    Race            UMETA(DisplayName = "Race"),
    Highlight       UMETA(DisplayName = "Highlight"),
    Drift           UMETA(DisplayName = "Drift"),
    Takedown        UMETA(DisplayName = "Takedown"),
    PhotoMode       UMETA(DisplayName = "Photo Mode"),
    Cinematic       UMETA(DisplayName = "Cinematic"),
    Tutorial        UMETA(DisplayName = "Tutorial"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * EMGExportFormat - Video export formats
 */
UENUM(BlueprintType)
enum class EMGExportFormat : uint8
{
    MP4_720p        UMETA(DisplayName = "MP4 720p"),
    MP4_1080p       UMETA(DisplayName = "MP4 1080p"),
    MP4_4K          UMETA(DisplayName = "MP4 4K"),
    GIF             UMETA(DisplayName = "GIF"),
    WebM            UMETA(DisplayName = "WebM")
};

/**
 * EMGShareStatus - Status of a share operation
 */
UENUM(BlueprintType)
enum class EMGShareStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    Processing      UMETA(DisplayName = "Processing"),
    Uploading       UMETA(DisplayName = "Uploading"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed"),
    Cancelled       UMETA(DisplayName = "Cancelled")
};

/**
 * EMGClipEffect - Visual effects for clips
 */
UENUM(BlueprintType)
enum class EMGClipEffect : uint8
{
    None            UMETA(DisplayName = "None"),
    SlowMotion      UMETA(DisplayName = "Slow Motion"),
    SpeedRamp       UMETA(DisplayName = "Speed Ramp"),
    Zoom            UMETA(DisplayName = "Zoom"),
    Shake           UMETA(DisplayName = "Shake"),
    VHSFilter       UMETA(DisplayName = "VHS Filter"),
    RetroFilter     UMETA(DisplayName = "Retro Filter"),
    NeonGlow        UMETA(DisplayName = "Neon Glow"),
    ChromaticAberration UMETA(DisplayName = "Chromatic Aberration"),
    FilmGrain       UMETA(DisplayName = "Film Grain"),
    LetterBox       UMETA(DisplayName = "Letter Box")
};

/**
 * FMGClipSegment - A segment within a clip
 */
USTRUCT(BlueprintType)
struct FMGClipSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlaybackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMGClipEffect> Effects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CameraAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIncludeAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AudioVolume;

    FMGClipSegment()
        : StartTime(0.0f)
        , EndTime(10.0f)
        , PlaybackSpeed(1.0f)
        , CameraAngle(NAME_None)
        , bIncludeAudio(true)
        , AudioVolume(1.0f)
    {}

    float GetDuration() const { return EndTime - StartTime; }
    float GetAdjustedDuration() const { return GetDuration() / PlaybackSpeed; }
};

/**
 * FMGClipData - Complete clip data
 */
USTRUCT(BlueprintType)
struct FMGClipData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClipId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGClipType ClipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGClipSegment> Segments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SourceReplayId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MusicVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWatermark;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WatermarkText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ThumbnailTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName VehicleUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName TrackUsed;

    FMGClipData()
        : ClipType(EMGClipType::Highlight)
        , TotalDuration(0.0f)
        , MusicTrack(NAME_None)
        , MusicVolume(0.5f)
        , bHasWatermark(true)
        , WatermarkText(TEXT("Midnight Grind"))
        , VehicleUsed(NAME_None)
        , TrackUsed(NAME_None)
    {}

    void CalculateTotalDuration()
    {
        TotalDuration = 0.0f;
        for (const FMGClipSegment& Segment : Segments)
        {
            TotalDuration += Segment.GetAdjustedDuration();
        }
    }
};

/**
 * FMGShareRequest - A share request to a platform
 */
USTRUCT(BlueprintType)
struct FMGShareRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClipId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGSharePlatform Platform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGExportFormat Format;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGShareStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Caption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Hashtags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime RequestedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CompletedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResultUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 FileSizeBytes;

    FMGShareRequest()
        : Platform(EMGSharePlatform::Twitter)
        , Format(EMGExportFormat::MP4_1080p)
        , Status(EMGShareStatus::Pending)
        , Progress(0.0f)
        , FileSizeBytes(0)
    {}
};

/**
 * FMGSocialAccount - Connected social account
 */
USTRUCT(BlueprintType)
struct FMGSocialAccount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGSharePlatform Platform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AccountId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Username;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AvatarUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsConnected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ConnectedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TokenExpiresAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShareCount;

    FMGSocialAccount()
        : Platform(EMGSharePlatform::Twitter)
        , bIsConnected(false)
        , ShareCount(0)
    {}

    bool IsTokenExpired() const
    {
        return FDateTime::Now() > TokenExpiresAt;
    }
};

/**
 * FMGCommunityClip - A clip shared to the community
 */
USTRUCT(BlueprintType)
struct FMGCommunityClip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClipId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CreatorId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CreatorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FMGClipData ClipData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VideoUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ThumbnailUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ViewCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LikeCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShareCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CommentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime UploadedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFeatured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLikedByMe;

    FMGCommunityClip()
        : ViewCount(0)
        , LikeCount(0)
        , ShareCount(0)
        , CommentCount(0)
        , bIsFeatured(false)
        , bIsLikedByMe(false)
    {}
};

/**
 * FMGExportSettings - Settings for video export
 */
USTRUCT(BlueprintType)
struct FMGExportSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGExportFormat Format;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Bitrate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIncludeGameAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIncludeMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIncludeVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAddWatermark;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAddIntroOutro;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OutputPath;

    FMGExportSettings()
        : Format(EMGExportFormat::MP4_1080p)
        , Bitrate(8000)
        , FrameRate(60)
        , bIncludeGameAudio(true)
        , bIncludeMusic(true)
        , bIncludeVoiceChat(false)
        , bAddWatermark(true)
        , bAddIntroOutro(false)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnClipCreated, const FMGClipData&, Clip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnClipDeleted, const FString&, ClipId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnShareProgressUpdated, const FString&, RequestId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnShareCompleted, const FString&, RequestId, const FString&, ResultUrl);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnShareFailed, const FString&, RequestId, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAccountConnected, EMGSharePlatform, Platform, const FMGSocialAccount&, Account);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAccountDisconnected, EMGSharePlatform, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnExportProgressUpdated, const FString&, ClipId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnExportCompleted, const FString&, ClipId, const FString&, FilePath);

/**
 * UMGReplayShareSubsystem
 *
 * Manages clip creation and social sharing for Midnight Grind.
 * Features include:
 * - Clip editing from replays
 * - Social media integration
 * - Video export
 * - Community clip browsing
 * - Account connection management
 */
UCLASS()
class MIDNIGHTGRIND_API UMGReplayShareSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGReplayShareSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Clip Creation =====

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Clips")
    FString CreateClipFromReplay(const FString& ReplayId, float StartTime, float EndTime);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Clips")
    void UpdateClip(const FMGClipData& ClipData);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Clips")
    void DeleteClip(const FString& ClipId);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Clips")
    void AddSegmentToClip(const FString& ClipId, const FMGClipSegment& Segment);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Clips")
    void RemoveSegmentFromClip(const FString& ClipId, int32 SegmentIndex);

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Clips")
    FMGClipData GetClip(const FString& ClipId) const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Clips")
    TArray<FMGClipData> GetAllClips() const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Clips")
    TArray<FMGClipData> GetClipsByType(EMGClipType ClipType) const;

    // ===== Clip Editing =====

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void SetClipTitle(const FString& ClipId, FText Title);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void SetClipMusic(const FString& ClipId, FName MusicTrack, float Volume);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void SetClipWatermark(const FString& ClipId, bool bEnabled, const FString& Text);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void AddClipEffect(const FString& ClipId, int32 SegmentIndex, EMGClipEffect Effect);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void SetSegmentSpeed(const FString& ClipId, int32 SegmentIndex, float Speed);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void SetSegmentCamera(const FString& ClipId, int32 SegmentIndex, FName CameraAngle);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Edit")
    void AddClipTag(const FString& ClipId, const FString& Tag);

    // ===== Export =====

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Export")
    void ExportClip(const FString& ClipId, const FMGExportSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Export")
    void CancelExport(const FString& ClipId);

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Export")
    bool IsExporting(const FString& ClipId) const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Export")
    float GetExportProgress(const FString& ClipId) const;

    // ===== Sharing =====

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Share")
    FString ShareClip(const FString& ClipId, EMGSharePlatform Platform, FText Caption, const TArray<FString>& Hashtags);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Share")
    void CancelShare(const FString& RequestId);

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Share")
    FMGShareRequest GetShareRequest(const FString& RequestId) const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Share")
    TArray<FMGShareRequest> GetPendingShares() const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Share")
    TArray<FMGShareRequest> GetShareHistory() const;

    // ===== Social Accounts =====

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Accounts")
    void ConnectAccount(EMGSharePlatform Platform);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Accounts")
    void DisconnectAccount(EMGSharePlatform Platform);

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Accounts")
    bool IsAccountConnected(EMGSharePlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Accounts")
    FMGSocialAccount GetAccount(EMGSharePlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Accounts")
    TArray<FMGSocialAccount> GetConnectedAccounts() const;

    // ===== Community =====

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Community")
    void UploadToCommunity(const FString& ClipId);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Community")
    void FetchCommunityClips(int32 Count, int32 Offset);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Community")
    void FetchFeaturedClips();

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Community")
    TArray<FMGCommunityClip> GetCommunityClips() const;

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Community")
    void LikeCommunityClip(const FString& ClipId);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Community")
    void ReportCommunityClip(const FString& ClipId, const FString& Reason);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnClipCreated OnClipCreated;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnClipDeleted OnClipDeleted;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnShareProgressUpdated OnShareProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnShareCompleted OnShareCompleted;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnShareFailed OnShareFailed;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnAccountConnected OnAccountConnected;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnAccountDisconnected OnAccountDisconnected;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnExportProgressUpdated OnExportProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "ReplayShare|Events")
    FMGOnExportCompleted OnExportCompleted;

protected:
    FString GenerateUniqueId();
    void ProcessExportQueue();
    void ProcessShareQueue();

private:
    UPROPERTY()
    TMap<FString, FMGClipData> Clips;

    UPROPERTY()
    TMap<EMGSharePlatform, FMGSocialAccount> ConnectedAccounts;

    UPROPERTY()
    TArray<FMGShareRequest> ShareRequests;

    UPROPERTY()
    TArray<FMGCommunityClip> CommunityClips;

    UPROPERTY()
    TMap<FString, float> ExportProgress;

    FTimerHandle ProcessTimerHandle;
};
