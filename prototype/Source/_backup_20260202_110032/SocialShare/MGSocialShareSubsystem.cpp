// Copyright Midnight Grind. All Rights Reserved.

#include "SocialShare/MGSocialShareSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/FileHelper.h"
#include "ImageUtils.h"

void UMGSocialShareSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default settings
	Settings.bAutoWatermark = true;
	Settings.bIncludePlayerName = true;
	Settings.DefaultVideoQuality = EMGVideoQuality::High;
	Settings.MaxClipDurationSeconds = 60;
	Settings.DefaultHashtags = TEXT("#MidnightGrind #Racing #StreetRacing");
}

void UMGSocialShareSubsystem::Deinitialize()
{
	if (CurrentRecording.bIsRecording)
	{
		CancelClipRecording();
	}

	Super::Deinitialize();
}

// Screenshot
void UMGSocialShareSubsystem::CaptureScreenshot()
{
	CaptureScreenshotWithUI(false);
}

void UMGSocialShareSubsystem::CaptureScreenshotWithUI(bool bIncludeUI)
{
	FString FileName = FString::Printf(TEXT("Screenshot_%s.png"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Screenshots") / FileName;

	// Would trigger actual screenshot capture
	// FScreenshotRequest::RequestScreenshot(FilePath, bIncludeUI, false);

	FMGShareableContent Content = CreateContentRecord(EMGShareContentType::Screenshot, FilePath);
	Content.Resolution = FIntPoint(1920, 1080);
	ContentLibrary.Add(Content);

	Stats.ScreenshotsShared++;
	OnScreenshotCaptured.Broadcast(Content);
}

void UMGSocialShareSubsystem::CapturePhotoModeImage(const FString& FilterName)
{
	FString FileName = FString::Printf(TEXT("PhotoMode_%s.png"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("PhotoMode") / FileName;

	FMGShareableContent Content = CreateContentRecord(EMGShareContentType::PhotoModeImage, FilePath);
	Content.Resolution = FIntPoint(3840, 2160); // 4K for photo mode
	if (!FilterName.IsEmpty())
	{
		Content.Metadata.Add(FName("Filter"), FilterName);
	}

	ContentLibrary.Add(Content);
	OnScreenshotCaptured.Broadcast(Content);
}

TArray<FMGShareableContent> UMGSocialShareSubsystem::GetRecentScreenshots(int32 Count) const
{
	TArray<FMGShareableContent> Result;

	for (const FMGShareableContent& Content : ContentLibrary)
	{
		if (Content.Type == EMGShareContentType::Screenshot ||
			Content.Type == EMGShareContentType::PhotoModeImage)
		{
			Result.Add(Content);
			if (Result.Num() >= Count)
			{
				break;
			}
		}
	}

	return Result;
}

// Video Recording
bool UMGSocialShareSubsystem::StartClipRecording(float MaxDurationSeconds, EMGVideoQuality Quality)
{
	if (CurrentRecording.bIsRecording)
	{
		return false;
	}

	CurrentRecording.RecordingID = GenerateContentID();
	CurrentRecording.bIsRecording = true;
	CurrentRecording.CurrentDuration = 0.0f;
	CurrentRecording.MaxDuration = FMath::Min(MaxDurationSeconds, static_cast<float>(Settings.MaxClipDurationSeconds));
	CurrentRecording.Quality = Quality;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RecordingTimerHandle,
			this,
			&UMGSocialShareSubsystem::OnRecordingTick,
			0.1f,
			true
		);
	}

	OnClipRecordingStarted.Broadcast();
	return true;
}

void UMGSocialShareSubsystem::StopClipRecording()
{
	if (!CurrentRecording.bIsRecording)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecordingTimerHandle);
	}

	CurrentRecording.bIsRecording = false;

	FString FileName = FString::Printf(TEXT("Clip_%s.mp4"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Clips") / FileName;

	FMGShareableContent Content = CreateContentRecord(EMGShareContentType::VideoClip, FilePath);
	Content.DurationSeconds = CurrentRecording.CurrentDuration;

	switch (CurrentRecording.Quality)
	{
	case EMGVideoQuality::Low:
		Content.Resolution = FIntPoint(1280, 720);
		break;
	case EMGVideoQuality::Medium:
		Content.Resolution = FIntPoint(1920, 1080);
		break;
	case EMGVideoQuality::High:
		Content.Resolution = FIntPoint(1920, 1080);
		break;
	case EMGVideoQuality::Ultra:
		Content.Resolution = FIntPoint(3840, 2160);
		break;
	}

	ContentLibrary.Add(Content);

	OnClipRecordingStopped.Broadcast(Content);
	OnClipRecorded.Broadcast(Content);

	CurrentRecording = FMGClipRecording();
}

void UMGSocialShareSubsystem::CancelClipRecording()
{
	if (!CurrentRecording.bIsRecording)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecordingTimerHandle);
	}

	CurrentRecording = FMGClipRecording();
}

void UMGSocialShareSubsystem::SaveReplayAsClip(float StartTime, float EndTime)
{
	FString FileName = FString::Printf(TEXT("Replay_%s.mp4"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Clips") / FileName;

	FMGShareableContent Content = CreateContentRecord(EMGShareContentType::ReplayHighlight, FilePath);
	Content.DurationSeconds = EndTime - StartTime;
	Content.Resolution = FIntPoint(1920, 1080);

	ContentLibrary.Add(Content);
	OnClipRecorded.Broadcast(Content);
}

void UMGSocialShareSubsystem::SaveLastNSeconds(float Seconds)
{
	// Would extract from replay buffer
	SaveReplayAsClip(0.0f, Seconds);
}

TArray<FMGShareableContent> UMGSocialShareSubsystem::GetRecentClips(int32 Count) const
{
	TArray<FMGShareableContent> Result;

	for (const FMGShareableContent& Content : ContentLibrary)
	{
		if (Content.Type == EMGShareContentType::VideoClip ||
			Content.Type == EMGShareContentType::ReplayHighlight)
		{
			Result.Add(Content);
			if (Result.Num() >= Count)
			{
				break;
			}
		}
	}

	return Result;
}

// Sharing
FString UMGSocialShareSubsystem::ShareContent(const FMGShareableContent& Content, const TArray<EMGSharePlatform>& Platforms, const FString& Message)
{
	FMGShareRequest Request;
	Request.RequestID = GenerateRequestID();
	Request.Content = Content;
	Request.TargetPlatforms = Platforms;
	Request.CustomMessage = Message.IsEmpty() ? Settings.DefaultHashtags : Message + TEXT(" ") + Settings.DefaultHashtags;
	Request.bIncludeWatermark = Settings.bAutoWatermark;
	Request.bIncludeGameTag = true;
	Request.Status = EMGShareStatus::Pending;
	Request.RequestTime = FDateTime::Now();

	ActiveRequests.Add(Request);
	OnShareRequestCreated.Broadcast(Request);

	ProcessShareRequest(Request);

	return Request.RequestID;
}

FString UMGSocialShareSubsystem::ShareScreenshot(const FString& ContentID, const TArray<EMGSharePlatform>& Platforms)
{
	const FMGShareableContent* Content = ContentLibrary.FindByPredicate([&ContentID](const FMGShareableContent& C)
	{
		return C.ContentID == ContentID;
	});

	if (Content)
	{
		return ShareContent(*Content, Platforms, TEXT(""));
	}

	return TEXT("");
}

FString UMGSocialShareSubsystem::ShareClip(const FString& ContentID, const TArray<EMGSharePlatform>& Platforms)
{
	return ShareScreenshot(ContentID, Platforms);
}

FString UMGSocialShareSubsystem::ShareAchievement(FName AchievementID, const TArray<EMGSharePlatform>& Platforms)
{
	FMGShareableContent Content;
	Content.ContentID = GenerateContentID();
	Content.Type = EMGShareContentType::Achievement;
	Content.Title = FText::FromString(FString::Printf(TEXT("Achievement Unlocked: %s"), *AchievementID.ToString()));
	Content.CreatedTime = FDateTime::Now();
	Content.Metadata.Add(FName("AchievementID"), AchievementID.ToString());

	return ShareContent(Content, Platforms, TEXT("Just unlocked an achievement!"));
}

FString UMGSocialShareSubsystem::ShareRaceResult(const FString& RaceID, const TArray<EMGSharePlatform>& Platforms)
{
	FMGShareableContent Content;
	Content.ContentID = GenerateContentID();
	Content.Type = EMGShareContentType::RaceResult;
	Content.Title = FText::FromString(TEXT("Race Result"));
	Content.CreatedTime = FDateTime::Now();
	Content.Metadata.Add(FName("RaceID"), RaceID);

	return ShareContent(Content, Platforms, TEXT("Check out my race result!"));
}

FString UMGSocialShareSubsystem::ShareLivery(const FString& LiveryID, const TArray<EMGSharePlatform>& Platforms)
{
	FMGShareableContent Content;
	Content.ContentID = GenerateContentID();
	Content.Type = EMGShareContentType::Livery;
	Content.Title = FText::FromString(TEXT("Custom Livery"));
	Content.CreatedTime = FDateTime::Now();
	Content.Metadata.Add(FName("LiveryID"), LiveryID);

	return ShareContent(Content, Platforms, TEXT("Check out my custom livery!"));
}

void UMGSocialShareSubsystem::CancelShare(const FString& RequestID)
{
	int32 Index = ActiveRequests.IndexOfByPredicate([&RequestID](const FMGShareRequest& R)
	{
		return R.RequestID == RequestID;
	});

	if (Index != INDEX_NONE)
	{
		ActiveRequests[Index].Status = EMGShareStatus::Cancelled;
		ActiveRequests.RemoveAt(Index);
	}
}

FMGShareRequest UMGSocialShareSubsystem::GetShareRequest(const FString& RequestID) const
{
	const FMGShareRequest* Request = ActiveRequests.FindByPredicate([&RequestID](const FMGShareRequest& R)
	{
		return R.RequestID == RequestID;
	});

	return Request ? *Request : FMGShareRequest();
}

TArray<FMGShareRequest> UMGSocialShareSubsystem::GetActiveShareRequests() const
{
	TArray<FMGShareRequest> Result;

	for (const FMGShareRequest& Request : ActiveRequests)
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

// Quick Share
void UMGSocialShareSubsystem::QuickShareToClipboard(const FMGShareableContent& Content)
{
	FString ShareText = FString::Printf(
		TEXT("%s - %s %s"),
		*Content.Title.ToString(),
		*Content.Description.ToString(),
		*Settings.DefaultHashtags
	);

	FPlatformApplicationMisc::ClipboardCopy(*ShareText);
}

void UMGSocialShareSubsystem::QuickShareToSystemDialog(const FMGShareableContent& Content)
{
	// Would trigger platform share sheet
	TArray<EMGSharePlatform> Platforms = { EMGSharePlatform::System };
	ShareContent(Content, Platforms, TEXT(""));
}

// Platform Integration
void UMGSocialShareSubsystem::LinkPlatform(EMGSharePlatform Platform)
{
	// Would trigger OAuth flow
	LinkedPlatformsSet.Add(Platform);
	Settings.LinkedPlatforms.AddUnique(Platform);
	OnPlatformLinked.Broadcast(Platform, true);
}

void UMGSocialShareSubsystem::UnlinkPlatform(EMGSharePlatform Platform)
{
	LinkedPlatformsSet.Remove(Platform);
	Settings.LinkedPlatforms.Remove(Platform);
	OnPlatformUnlinked.Broadcast(Platform);
}

bool UMGSocialShareSubsystem::IsPlatformLinked(EMGSharePlatform Platform) const
{
	return LinkedPlatformsSet.Contains(Platform);
}

TArray<EMGSharePlatform> UMGSocialShareSubsystem::GetLinkedPlatforms() const
{
	return Settings.LinkedPlatforms;
}

TArray<EMGSharePlatform> UMGSocialShareSubsystem::GetAvailablePlatforms() const
{
	TArray<EMGSharePlatform> Result;
	Result.Add(EMGSharePlatform::Twitter);
	Result.Add(EMGSharePlatform::Facebook);
	Result.Add(EMGSharePlatform::Instagram);
	Result.Add(EMGSharePlatform::TikTok);
	Result.Add(EMGSharePlatform::YouTube);
	Result.Add(EMGSharePlatform::Discord);
	Result.Add(EMGSharePlatform::Reddit);
	Result.Add(EMGSharePlatform::System);

#if PLATFORM_WINDOWS
	Result.Add(EMGSharePlatform::Steam);
#elif PLATFORM_PS5
	Result.Add(EMGSharePlatform::PlayStation);
#elif PLATFORM_XSX
	Result.Add(EMGSharePlatform::Xbox);
#endif

	return Result;
}

// Settings
void UMGSocialShareSubsystem::UpdateShareSettings(const FMGShareSettings& NewSettings)
{
	Settings = NewSettings;
}

// Content Management
TArray<FMGShareableContent> UMGSocialShareSubsystem::GetContentByType(EMGShareContentType Type) const
{
	TArray<FMGShareableContent> Result;

	for (const FMGShareableContent& Content : ContentLibrary)
	{
		if (Content.Type == Type)
		{
			Result.Add(Content);
		}
	}

	return Result;
}

bool UMGSocialShareSubsystem::DeleteContent(const FString& ContentID)
{
	int32 Index = ContentLibrary.IndexOfByPredicate([&ContentID](const FMGShareableContent& C)
	{
		return C.ContentID == ContentID;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	// Delete file
	const FMGShareableContent& Content = ContentLibrary[Index];
	IFileManager::Get().Delete(*Content.FilePath);
	if (!Content.ThumbnailPath.IsEmpty())
	{
		IFileManager::Get().Delete(*Content.ThumbnailPath);
	}

	ContentLibrary.RemoveAt(Index);
	return true;
}

void UMGSocialShareSubsystem::CleanupOldContent(int32 MaxAgeDays)
{
	FDateTime Cutoff = FDateTime::Now() - FTimespan::FromDays(MaxAgeDays);

	for (int32 i = ContentLibrary.Num() - 1; i >= 0; i--)
	{
		if (ContentLibrary[i].CreatedTime < Cutoff)
		{
			DeleteContent(ContentLibrary[i].ContentID);
		}
	}
}

int64 UMGSocialShareSubsystem::GetTotalStorageUsed() const
{
	int64 Total = 0;
	for (const FMGShareableContent& Content : ContentLibrary)
	{
		Total += Content.FileSizeBytes;
	}
	return Total;
}

// Watermark
void UMGSocialShareSubsystem::SetCustomWatermark(UTexture2D* Watermark)
{
	CustomWatermark = Watermark;
}

void UMGSocialShareSubsystem::ClearCustomWatermark()
{
	CustomWatermark = nullptr;
}

// Internal
void UMGSocialShareSubsystem::ProcessShareRequest(FMGShareRequest& Request)
{
	Request.Status = EMGShareStatus::Processing;

	// Find request in active list and update
	FMGShareRequest* ActiveRequest = ActiveRequests.FindByPredicate([&Request](const FMGShareRequest& R)
	{
		return R.RequestID == Request.RequestID;
	});

	if (ActiveRequest)
	{
		*ActiveRequest = Request;
	}

	// Simulate upload process
	SimulateUpload(Request.RequestID);
}

void UMGSocialShareSubsystem::SimulateUpload(const FString& RequestID)
{
	FMGShareRequest* Request = ActiveRequests.FindByPredicate([&RequestID](const FMGShareRequest& R)
	{
		return R.RequestID == RequestID;
	});

	if (!Request)
	{
		return;
	}

	Request->Status = EMGShareStatus::Uploading;

	// Simulate progress updates
	if (UWorld* World = GetWorld())
	{
		FTimerHandle ProgressHandle;
		TWeakObjectPtr<UMGSocialShareSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			ProgressHandle,
			[WeakThis, RequestID]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}
				FMGShareRequest* R = WeakThis->ActiveRequests.FindByPredicate([&RequestID](const FMGShareRequest& Req)
				{
					return Req.RequestID == RequestID;
				});

				if (R && R->Status == EMGShareStatus::Uploading)
				{
					R->UploadProgress = FMath::Min(1.0f, R->UploadProgress + 0.25f);
					WeakThis->OnShareProgressUpdated.Broadcast(RequestID, R->UploadProgress);

					if (R->UploadProgress >= 1.0f)
					{
						R->Status = EMGShareStatus::Posted;
						R->CompletedTime = FDateTime::Now();

						TArray<FString> URLs;
						for (EMGSharePlatform Platform : R->TargetPlatforms)
						{
							FString URL = FString::Printf(TEXT("https://%s.com/post/%s"),
								*WeakThis->GetPlatformName(Platform).ToLower(),
								*R->RequestID.Left(8));
							R->PostURLs.Add(Platform, URL);
							URLs.Add(URL);
						}

						WeakThis->Stats.TotalShares++;
						WeakThis->Stats.LastShareTime = FDateTime::Now();

						WeakThis->OnShareCompleted.Broadcast(RequestID, URLs);
					}
				}
			},
			0.5f,
			true,
			1.0f
		);
	}
}

FMGShareableContent UMGSocialShareSubsystem::CreateContentRecord(EMGShareContentType Type, const FString& FilePath)
{
	FMGShareableContent Content;
	Content.ContentID = GenerateContentID();
	Content.Type = Type;
	Content.FilePath = FilePath;
	Content.CreatedTime = FDateTime::Now();

	// Generate thumbnail path
	FString ThumbnailName = FPaths::GetBaseFilename(FilePath) + TEXT("_thumb.jpg");
	Content.ThumbnailPath = FPaths::GetPath(FilePath) / ThumbnailName;

	return Content;
}

FString UMGSocialShareSubsystem::GenerateContentID() const
{
	return FGuid::NewGuid().ToString().Left(12);
}

FString UMGSocialShareSubsystem::GenerateRequestID() const
{
	return FGuid::NewGuid().ToString();
}

void UMGSocialShareSubsystem::OnRecordingTick()
{
	if (CurrentRecording.bIsRecording)
	{
		CurrentRecording.CurrentDuration += 0.1f;

		if (CurrentRecording.CurrentDuration >= CurrentRecording.MaxDuration)
		{
			StopClipRecording();
		}
	}
}

FString UMGSocialShareSubsystem::GetPlatformName(EMGSharePlatform Platform) const
{
	switch (Platform)
	{
	case EMGSharePlatform::Twitter: return TEXT("Twitter");
	case EMGSharePlatform::Facebook: return TEXT("Facebook");
	case EMGSharePlatform::Instagram: return TEXT("Instagram");
	case EMGSharePlatform::TikTok: return TEXT("TikTok");
	case EMGSharePlatform::YouTube: return TEXT("YouTube");
	case EMGSharePlatform::Twitch: return TEXT("Twitch");
	case EMGSharePlatform::Discord: return TEXT("Discord");
	case EMGSharePlatform::Reddit: return TEXT("Reddit");
	case EMGSharePlatform::Steam: return TEXT("Steam");
	case EMGSharePlatform::PlayStation: return TEXT("PlayStation");
	case EMGSharePlatform::Xbox: return TEXT("Xbox");
	case EMGSharePlatform::System: return TEXT("System");
	default: return TEXT("Unknown");
	}
}
