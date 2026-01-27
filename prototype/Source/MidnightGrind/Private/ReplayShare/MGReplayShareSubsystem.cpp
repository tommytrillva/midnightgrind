// MidnightGrind - Arcade Street Racing Game
// Replay Share Subsystem Implementation

#include "ReplayShare/MGReplayShareSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Guid.h"

UMGReplayShareSubsystem::UMGReplayShareSubsystem()
{
}

void UMGReplayShareSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Start processing timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGReplayShareSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            ProcessTimerHandle,
            [WeakThis]()
            {
                if (WeakThis.IsValid())
                {
                    WeakThis->ProcessExportQueue();
                    WeakThis->ProcessShareQueue();
                }
            },
            0.5f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("MGReplayShareSubsystem initialized"));
}

void UMGReplayShareSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ProcessTimerHandle);
    }

    Super::Deinitialize();
}

// ===== Clip Creation =====

FString UMGReplayShareSubsystem::CreateClipFromReplay(const FString& ReplayId, float StartTime, float EndTime)
{
    FMGClipData NewClip;
    NewClip.ClipId = GenerateUniqueId();
    NewClip.SourceReplayId = ReplayId;
    NewClip.CreatedAt = FDateTime::Now();
    NewClip.ClipType = EMGClipType::Highlight;

    // Create initial segment
    FMGClipSegment Segment;
    Segment.StartTime = StartTime;
    Segment.EndTime = EndTime;
    Segment.PlaybackSpeed = 1.0f;
    Segment.bIncludeAudio = true;
    NewClip.Segments.Add(Segment);

    NewClip.CalculateTotalDuration();

    Clips.Add(NewClip.ClipId, NewClip);
    OnClipCreated.Broadcast(NewClip);

    UE_LOG(LogTemp, Log, TEXT("Created clip %s from replay %s"), *NewClip.ClipId, *ReplayId);
    return NewClip.ClipId;
}

void UMGReplayShareSubsystem::UpdateClip(const FMGClipData& ClipData)
{
    if (Clips.Contains(ClipData.ClipId))
    {
        Clips[ClipData.ClipId] = ClipData;
        Clips[ClipData.ClipId].CalculateTotalDuration();
    }
}

void UMGReplayShareSubsystem::DeleteClip(const FString& ClipId)
{
    if (Clips.Remove(ClipId) > 0)
    {
        ExportProgress.Remove(ClipId);
        OnClipDeleted.Broadcast(ClipId);
        UE_LOG(LogTemp, Log, TEXT("Deleted clip: %s"), *ClipId);
    }
}

void UMGReplayShareSubsystem::AddSegmentToClip(const FString& ClipId, const FMGClipSegment& Segment)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip)
    {
        Clip->Segments.Add(Segment);
        Clip->CalculateTotalDuration();
    }
}

void UMGReplayShareSubsystem::RemoveSegmentFromClip(const FString& ClipId, int32 SegmentIndex)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip && Clip->Segments.IsValidIndex(SegmentIndex))
    {
        Clip->Segments.RemoveAt(SegmentIndex);
        Clip->CalculateTotalDuration();
    }
}

FMGClipData UMGReplayShareSubsystem::GetClip(const FString& ClipId) const
{
    const FMGClipData* Clip = Clips.Find(ClipId);
    return Clip ? *Clip : FMGClipData();
}

TArray<FMGClipData> UMGReplayShareSubsystem::GetAllClips() const
{
    TArray<FMGClipData> Result;
    Clips.GenerateValueArray(Result);
    return Result;
}

TArray<FMGClipData> UMGReplayShareSubsystem::GetClipsByType(EMGClipType ClipType) const
{
    TArray<FMGClipData> Result;
    for (const auto& Pair : Clips)
    {
        if (Pair.Value.ClipType == ClipType)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

// ===== Clip Editing =====

void UMGReplayShareSubsystem::SetClipTitle(const FString& ClipId, FText Title)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip)
    {
        Clip->Title = Title;
    }
}

void UMGReplayShareSubsystem::SetClipMusic(const FString& ClipId, FName MusicTrack, float Volume)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip)
    {
        Clip->MusicTrack = MusicTrack;
        Clip->MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
    }
}

void UMGReplayShareSubsystem::SetClipWatermark(const FString& ClipId, bool bEnabled, const FString& Text)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip)
    {
        Clip->bHasWatermark = bEnabled;
        Clip->WatermarkText = Text;
    }
}

void UMGReplayShareSubsystem::AddClipEffect(const FString& ClipId, int32 SegmentIndex, EMGClipEffect Effect)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip && Clip->Segments.IsValidIndex(SegmentIndex))
    {
        Clip->Segments[SegmentIndex].Effects.AddUnique(Effect);
    }
}

void UMGReplayShareSubsystem::SetSegmentSpeed(const FString& ClipId, int32 SegmentIndex, float Speed)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip && Clip->Segments.IsValidIndex(SegmentIndex))
    {
        Clip->Segments[SegmentIndex].PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 4.0f);
        Clip->CalculateTotalDuration();
    }
}

void UMGReplayShareSubsystem::SetSegmentCamera(const FString& ClipId, int32 SegmentIndex, FName CameraAngle)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip && Clip->Segments.IsValidIndex(SegmentIndex))
    {
        Clip->Segments[SegmentIndex].CameraAngle = CameraAngle;
    }
}

void UMGReplayShareSubsystem::AddClipTag(const FString& ClipId, const FString& Tag)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (Clip)
    {
        Clip->Tags.AddUnique(Tag);
    }
}

// ===== Export =====

void UMGReplayShareSubsystem::ExportClip(const FString& ClipId, const FMGExportSettings& Settings)
{
    if (!Clips.Contains(ClipId))
    {
        return;
    }

    ExportProgress.Add(ClipId, 0.0f);
    UE_LOG(LogTemp, Log, TEXT("Starting export for clip: %s"), *ClipId);
}

void UMGReplayShareSubsystem::CancelExport(const FString& ClipId)
{
    ExportProgress.Remove(ClipId);
    UE_LOG(LogTemp, Log, TEXT("Cancelled export for clip: %s"), *ClipId);
}

bool UMGReplayShareSubsystem::IsExporting(const FString& ClipId) const
{
    return ExportProgress.Contains(ClipId);
}

float UMGReplayShareSubsystem::GetExportProgress(const FString& ClipId) const
{
    const float* Progress = ExportProgress.Find(ClipId);
    return Progress ? *Progress : 0.0f;
}

// ===== Sharing =====

FString UMGReplayShareSubsystem::ShareClip(const FString& ClipId, EMGSharePlatform Platform, FText Caption, const TArray<FString>& Hashtags)
{
    if (!Clips.Contains(ClipId))
    {
        return FString();
    }

    if (!IsAccountConnected(Platform))
    {
        return FString();
    }

    FMGShareRequest Request;
    Request.RequestId = GenerateUniqueId();
    Request.ClipId = ClipId;
    Request.Platform = Platform;
    Request.Status = EMGShareStatus::Pending;
    Request.Caption = Caption;
    Request.Hashtags = Hashtags;
    Request.RequestedAt = FDateTime::Now();

    ShareRequests.Add(Request);

    UE_LOG(LogTemp, Log, TEXT("Share request created: %s for clip %s"), *Request.RequestId, *ClipId);
    return Request.RequestId;
}

void UMGReplayShareSubsystem::CancelShare(const FString& RequestId)
{
    for (FMGShareRequest& Request : ShareRequests)
    {
        if (Request.RequestId == RequestId && Request.Status == EMGShareStatus::Pending)
        {
            Request.Status = EMGShareStatus::Cancelled;
            break;
        }
    }
}

FMGShareRequest UMGReplayShareSubsystem::GetShareRequest(const FString& RequestId) const
{
    for (const FMGShareRequest& Request : ShareRequests)
    {
        if (Request.RequestId == RequestId)
        {
            return Request;
        }
    }
    return FMGShareRequest();
}

TArray<FMGShareRequest> UMGReplayShareSubsystem::GetPendingShares() const
{
    TArray<FMGShareRequest> Result;
    for (const FMGShareRequest& Request : ShareRequests)
    {
        if (Request.Status == EMGShareStatus::Pending ||
            Request.Status == EMGShareStatus::Processing ||
            Request.Status == EMGShareStatus::Uploading)
        {
            Result.Add(Request);
        }
    }
    return Result;
}

TArray<FMGShareRequest> UMGReplayShareSubsystem::GetShareHistory() const
{
    return ShareRequests;
}

// ===== Social Accounts =====

void UMGReplayShareSubsystem::ConnectAccount(EMGSharePlatform Platform)
{
    // In a real implementation, initiate OAuth flow
    FMGSocialAccount Account;
    Account.Platform = Platform;
    Account.bIsConnected = true;
    Account.ConnectedAt = FDateTime::Now();
    Account.TokenExpiresAt = FDateTime::Now() + FTimespan::FromDays(30);
    Account.Username = TEXT("MidnightRacer");
    Account.AccountId = GenerateUniqueId();

    ConnectedAccounts.Add(Platform, Account);
    OnAccountConnected.Broadcast(Platform, Account);

    UE_LOG(LogTemp, Log, TEXT("Connected account for platform: %d"), static_cast<int32>(Platform));
}

void UMGReplayShareSubsystem::DisconnectAccount(EMGSharePlatform Platform)
{
    if (ConnectedAccounts.Remove(Platform) > 0)
    {
        OnAccountDisconnected.Broadcast(Platform);
        UE_LOG(LogTemp, Log, TEXT("Disconnected account for platform: %d"), static_cast<int32>(Platform));
    }
}

bool UMGReplayShareSubsystem::IsAccountConnected(EMGSharePlatform Platform) const
{
    const FMGSocialAccount* Account = ConnectedAccounts.Find(Platform);
    return Account && Account->bIsConnected && !Account->IsTokenExpired();
}

FMGSocialAccount UMGReplayShareSubsystem::GetAccount(EMGSharePlatform Platform) const
{
    const FMGSocialAccount* Account = ConnectedAccounts.Find(Platform);
    return Account ? *Account : FMGSocialAccount();
}

TArray<FMGSocialAccount> UMGReplayShareSubsystem::GetConnectedAccounts() const
{
    TArray<FMGSocialAccount> Result;
    ConnectedAccounts.GenerateValueArray(Result);
    return Result;
}

// ===== Community =====

void UMGReplayShareSubsystem::UploadToCommunity(const FString& ClipId)
{
    FMGClipData* Clip = Clips.Find(ClipId);
    if (!Clip)
    {
        return;
    }

    // In a real implementation, upload to community server
    FMGCommunityClip CommunityClip;
    CommunityClip.ClipId = ClipId;
    CommunityClip.ClipData = *Clip;
    CommunityClip.UploadedAt = FDateTime::Now();
    CommunityClip.CreatorId = TEXT("LocalPlayer");
    CommunityClip.CreatorName = TEXT("Local Player");

    CommunityClips.Add(CommunityClip);

    UE_LOG(LogTemp, Log, TEXT("Uploaded clip to community: %s"), *ClipId);
}

void UMGReplayShareSubsystem::FetchCommunityClips(int32 Count, int32 Offset)
{
    // In a real implementation, fetch from community server
    UE_LOG(LogTemp, Log, TEXT("Fetching %d community clips at offset %d"), Count, Offset);
}

void UMGReplayShareSubsystem::FetchFeaturedClips()
{
    // In a real implementation, fetch featured clips
    UE_LOG(LogTemp, Log, TEXT("Fetching featured clips"));
}

TArray<FMGCommunityClip> UMGReplayShareSubsystem::GetCommunityClips() const
{
    return CommunityClips;
}

void UMGReplayShareSubsystem::LikeCommunityClip(const FString& ClipId)
{
    for (FMGCommunityClip& Clip : CommunityClips)
    {
        if (Clip.ClipId == ClipId)
        {
            if (!Clip.bIsLikedByMe)
            {
                Clip.bIsLikedByMe = true;
                Clip.LikeCount++;
            }
            break;
        }
    }
}

void UMGReplayShareSubsystem::ReportCommunityClip(const FString& ClipId, const FString& Reason)
{
    // In a real implementation, send report to server
    UE_LOG(LogTemp, Log, TEXT("Reported clip %s: %s"), *ClipId, *Reason);
}

// ===== Internal Helpers =====

FString UMGReplayShareSubsystem::GenerateUniqueId()
{
    return FGuid::NewGuid().ToString();
}

void UMGReplayShareSubsystem::ProcessExportQueue()
{
    TArray<FString> CompletedExports;

    for (auto& Pair : ExportProgress)
    {
        // Simulate export progress
        Pair.Value += 0.05f;
        OnExportProgressUpdated.Broadcast(Pair.Key, Pair.Value);

        if (Pair.Value >= 1.0f)
        {
            CompletedExports.Add(Pair.Key);
            FString OutputPath = FString::Printf(TEXT("/Saved/Clips/%s.mp4"), *Pair.Key);
            OnExportCompleted.Broadcast(Pair.Key, OutputPath);
        }
    }

    for (const FString& ClipId : CompletedExports)
    {
        ExportProgress.Remove(ClipId);
    }
}

void UMGReplayShareSubsystem::ProcessShareQueue()
{
    for (FMGShareRequest& Request : ShareRequests)
    {
        if (Request.Status == EMGShareStatus::Pending)
        {
            Request.Status = EMGShareStatus::Processing;
        }
        else if (Request.Status == EMGShareStatus::Processing)
        {
            Request.Progress += 0.1f;
            OnShareProgressUpdated.Broadcast(Request.RequestId, Request.Progress);

            if (Request.Progress >= 0.5f)
            {
                Request.Status = EMGShareStatus::Uploading;
            }
        }
        else if (Request.Status == EMGShareStatus::Uploading)
        {
            Request.Progress += 0.1f;
            OnShareProgressUpdated.Broadcast(Request.RequestId, Request.Progress);

            if (Request.Progress >= 1.0f)
            {
                Request.Status = EMGShareStatus::Complete;
                Request.CompletedAt = FDateTime::Now();
                Request.ResultUrl = FString::Printf(TEXT("https://example.com/share/%s"), *Request.RequestId);
                OnShareCompleted.Broadcast(Request.RequestId, Request.ResultUrl);
            }
        }
    }
}
