// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * @file MGReplayShareSubsystem.h
 * @brief Replay Share Subsystem - Clip creation and social media integration
 *
 * This subsystem handles the creation, editing, and sharing of gameplay clips.
 * It bridges the replay system with social media platforms and community features.
 *
 * CORE FEATURES:
 * ==============
 *
 * 1. CLIP CREATION
 *    - Create clips from replay timestamps
 *    - Multi-segment clip editing (combine multiple moments)
 *    - Support for various clip types (race, drift, takedown, etc.)
 *
 * 2. CLIP EDITING
 *    - Playback speed control (slow motion, speed ramp)
 *    - Visual effects (VHS filter, neon glow, film grain)
 *    - Music track selection and mixing
 *    - Camera angle switching per segment
 *    - Watermark customization
 *
 * 3. VIDEO EXPORT
 *    - Multiple format support (MP4, GIF, WebM)
 *    - Resolution presets (720p, 1080p, 4K)
 *    - Bitrate and quality control
 *    - Progress tracking and cancellation
 *
 * 4. SOCIAL SHARING
 *    - Platform integration (Twitter, YouTube, TikTok, Discord)
 *    - OAuth account connection
 *    - Caption and hashtag support
 *    - Share progress tracking
 *
 * 5. COMMUNITY HUB
 *    - Upload clips to community server
 *    - Browse and discover clips
 *    - Like, comment, and report functionality
 *    - Featured clips system
 *
 * WORKFLOW EXAMPLE:
 * -----------------
 * 1. Player finishes an exciting race
 * 2. Opens replay viewer, finds a highlight moment
 * 3. Creates clip: CreateClipFromReplay(ReplayID, 45.0f, 55.0f)
 * 4. Adds slow-mo effect: AddClipEffect(ClipID, 0, SlowMotion)
 * 5. Sets music: SetClipMusic(ClipID, "RetroWave_01", 0.5f)
 * 6. Exports: ExportClip(ClipID, MP4_1080p)
 * 7. Shares: ShareClip(ClipID, Twitter, "Epic drift! #MidnightGrind")
 *
 * SECURITY NOTES:
 * - OAuth tokens are stored securely and expire
 * - IsTokenExpired() should be checked before sharing
 * - Community clips are moderated (ReportCommunityClip)
 *
 * @see UMGReplaySubsystem - Source of replay data
 * @see UMGReplayBufferSubsystem - Continuous recording buffer
 * @see UMGSocialShareSubsystem - Lower-level social API
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Core/MGSharedTypes.h"
#include "SocialShare/MGSocialShareSubsystem.h"
#include "MGReplayShareSubsystem.generated.h"

// EMGSharePlatform - REMOVED (duplicate)
// Canonical definition in: Clip/MGClipSubsystem.h

/**
 * Clip type classification
 *
 * Categorizes clips for filtering and organization.
 * Also affects auto-tagging and community discovery.
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

// EMGShareStatus - MOVED to Core/MGSharedTypes.h

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
 * Clip Segment
 *
 * A single continuous portion of a clip with its own settings.
 * Multiple segments can be combined to create complex edits.
 *
 * EXAMPLE: Create a clip with slow-mo crash followed by normal speed replay
 * - Segment 1: StartTime=10, EndTime=12, Speed=0.25 (slow motion crash)
 * - Segment 2: StartTime=12, EndTime=20, Speed=1.0 (normal replay)
 *
 * GetAdjustedDuration() returns the actual playback time after speed adjustment.
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
 * Complete Clip Data
 *
 * Contains all information needed to render and share a clip.
 * This is the primary data structure for clip management.
 *
 * Clips are created from replays using CreateClipFromReplay(),
 * then edited with the various Set* functions, and finally
 * exported or shared.
 *
 * @see UMGReplayShareSubsystem::CreateClipFromReplay
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

// FMGShareRequest - REMOVED (duplicate)
// Canonical definition in: SocialShare/MGSocialShareSubsystem.h

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
 * Replay Share Subsystem
 *
 * Game Instance subsystem for clip creation and social media sharing.
 * Persists across level loads and manages the entire clip lifecycle.
 *
 * RESPONSIBILITIES:
 * - Create and edit clips from replay data
 * - Export clips to video files
 * - Share to social media platforms
 * - Manage community clip browsing
 * - Handle OAuth account connections
 *
 * This is a GameInstanceSubsystem, meaning there's one instance
 * that persists for the entire game session. Access it via:
 * @code
 * UMGReplayShareSubsystem* ShareSystem = GameInstance->GetSubsystem<UMGReplayShareSubsystem>();
 * @endcode
 *
 * All operations are asynchronous where appropriate, with progress
 * reported via Blueprint-assignable delegates.
 *
 * @see UMGReplaySubsystem - Provides source replay data
 * @see FMGClipData - Primary clip data structure
 */
UCLASS()
class MIDNIGHTGRIND_API UMGReplayShareSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGReplayShareSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ==========================================
    // CLIP CREATION
    // ==========================================

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

    // ==========================================
    // CLIP EDITING
    // ==========================================

    /** Set the display title for a clip */
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

    // ==========================================
    // VIDEO EXPORT
    // ==========================================

    /** Begin exporting a clip to video file (async, check OnExportCompleted) */
    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Export")
    void ExportClip(const FString& ClipId, const FMGExportSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "ReplayShare|Export")
    void CancelExport(const FString& ClipId);

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Export")
    bool IsExporting(const FString& ClipId) const;

    UFUNCTION(BlueprintPure, Category = "ReplayShare|Export")
    float GetExportProgress(const FString& ClipId) const;

    // ==========================================
    // SOCIAL SHARING
    // ==========================================

    /**
     * Share a clip to a social platform
     * @param ClipId The clip to share
     * @param Platform Target social platform
     * @param Caption Text to accompany the post
     * @param Hashtags Tags to include (platform-specific formatting applied)
     * @return Request ID for tracking progress (empty if failed)
     */
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

    // ==========================================
    // SOCIAL ACCOUNT MANAGEMENT
    // ==========================================

    /** Initiate OAuth connection flow for a social platform */
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

    // ==========================================
    // COMMUNITY HUB
    // ==========================================

    /** Upload a clip to the community server for discovery */
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

    // ==========================================
    // EVENTS (Blueprint Assignable Delegates)
    // ==========================================

    /** Fired when a new clip is created */
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
