// Copyright Midnight Grind. All Rights Reserved.

#include "Clip/MGClipSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/FileManager.h"

void UMGClipSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default settings
	Settings.DefaultQuality = EMGClipQuality::High;
	Settings.DefaultClipLength = 30.0f;
	Settings.MaxClipLength = 120.0f;
	Settings.BufferDuration = 60.0f;
	Settings.bAutoCapture = true;
	Settings.bCaptureVictories = true;
	Settings.bCaptureHighlights = true;
	Settings.bCaptureCloseFinishes = true;
	Settings.MaxStoredClips = 50;
	Settings.MaxStorageSizeMB = 5000;

	// Set storage path
	ClipStoragePath = FPaths::ProjectSavedDir() / TEXT("Clips");
	IFileManager::Get().MakeDirectory(*ClipStoragePath, true);

	LoadClipData();

	// Start clip tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ClipTickHandle,
			this,
			&UMGClipSubsystem::OnClipTick,
			1.0f,
			true
		);
	}
}

void UMGClipSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ClipTickHandle);
	}

	if (bIsRecording)
	{
		StopRecording();
	}

	SaveClipData();
	Super::Deinitialize();
}

bool UMGClipSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Recording
// ============================================================================

void UMGClipSubsystem::StartRecording()
{
	if (bIsRecording)
	{
		return;
	}

	bIsRecording = true;

	if (const UWorld* World = GetWorld())
	{
		RecordingStartTime = World->GetTimeSeconds();
	}

	// Would initialize actual video recording here
	OnRecordingStarted.Broadcast();
}

FGuid UMGClipSubsystem::StopRecording(const FString& Title)
{
	if (!bIsRecording)
	{
		return FGuid();
	}

	bIsRecording = false;
	float Duration = GetRecordingDuration();

	// Create clip metadata
	FMGClipMetadata NewClip;
	NewClip.ClipID = FGuid::NewGuid();
	NewClip.Title = Title.IsEmpty() ? FString::Printf(TEXT("Clip_%s"), *FDateTime::Now().ToString()) : Title;
	NewClip.Status = EMGClipStatus::Processing;
	NewClip.Quality = Settings.DefaultQuality;
	NewClip.Duration = Duration;
	NewClip.RecordedAt = FDateTime::UtcNow();
	NewClip.FilePath = GenerateClipPath(NewClip.ClipID);
	NewClip.ThumbnailPath = NewClip.FilePath.Replace(TEXT(".mp4"), TEXT("_thumb.jpg"));

	// Would finalize actual video recording here
	NewClip.Status = EMGClipStatus::Ready;

	SavedClips.Add(NewClip);
	Stats.TotalClipsRecorded++;
	Stats.TotalRecordedDuration += Duration;

	EnforceStorageLimits();
	UpdateStats();

	OnRecordingStopped.Broadcast(NewClip.ClipID);
	OnClipReady.Broadcast(NewClip);

	SaveClipData();

	return NewClip.ClipID;
}

FGuid UMGClipSubsystem::SaveLastSeconds(float Seconds, const FString& Title)
{
	float ClampedSeconds = FMath::Clamp(Seconds, 5.0f, Settings.BufferDuration);

	// Create clip from buffer
	FMGClipMetadata NewClip;
	NewClip.ClipID = FGuid::NewGuid();
	NewClip.Title = Title.IsEmpty() ? FString::Printf(TEXT("Instant_Replay_%s"), *FDateTime::Now().ToString()) : Title;
	NewClip.Status = EMGClipStatus::Processing;
	NewClip.Quality = Settings.DefaultQuality;
	NewClip.Duration = ClampedSeconds;
	NewClip.RecordedAt = FDateTime::UtcNow();
	NewClip.FilePath = GenerateClipPath(NewClip.ClipID);
	NewClip.ThumbnailPath = NewClip.FilePath.Replace(TEXT(".mp4"), TEXT("_thumb.jpg"));

	// Would extract from recording buffer here
	NewClip.Status = EMGClipStatus::Ready;

	SavedClips.Add(NewClip);
	Stats.TotalClipsRecorded++;
	Stats.TotalRecordedDuration += ClampedSeconds;

	EnforceStorageLimits();
	UpdateStats();

	OnClipReady.Broadcast(NewClip);
	SaveClipData();

	return NewClip.ClipID;
}

float UMGClipSubsystem::GetRecordingDuration() const
{
	if (!bIsRecording)
	{
		return 0.0f;
	}

	if (const UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds() - RecordingStartTime;
	}

	return 0.0f;
}

float UMGClipSubsystem::GetBufferDuration() const
{
	return Settings.BufferDuration;
}

// ============================================================================
// Clip Management
// ============================================================================

TArray<FMGClipMetadata> UMGClipSubsystem::GetClipsByCategory(EMGClipCategory Category) const
{
	TArray<FMGClipMetadata> FilteredClips;

	for (const FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.Category == Category)
		{
			FilteredClips.Add(Clip);
		}
	}

	return FilteredClips;
}

TArray<FMGClipMetadata> UMGClipSubsystem::GetFavoriteClips() const
{
	TArray<FMGClipMetadata> Favorites;

	for (const FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.bIsFavorite)
		{
			Favorites.Add(Clip);
		}
	}

	return Favorites;
}

FMGClipMetadata UMGClipSubsystem::GetClip(FGuid ClipID) const
{
	for (const FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			return Clip;
		}
	}

	return FMGClipMetadata();
}

bool UMGClipSubsystem::DeleteClip(FGuid ClipID)
{
	for (int32 i = 0; i < SavedClips.Num(); ++i)
	{
		if (SavedClips[i].ClipID == ClipID)
		{
			// Delete files
			IFileManager::Get().Delete(*SavedClips[i].FilePath);
			IFileManager::Get().Delete(*SavedClips[i].ThumbnailPath);

			SavedClips.RemoveAt(i);

			OnClipDeleted.Broadcast(ClipID);
			UpdateStats();
			SaveClipData();

			return true;
		}
	}

	return false;
}

void UMGClipSubsystem::SetClipFavorite(FGuid ClipID, bool bFavorite)
{
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.bIsFavorite = bFavorite;
			SaveClipData();
			return;
		}
	}
}

void UMGClipSubsystem::RenameClip(FGuid ClipID, const FString& NewTitle)
{
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Title = NewTitle;
			SaveClipData();
			return;
		}
	}
}

void UMGClipSubsystem::SetClipCategory(FGuid ClipID, EMGClipCategory Category)
{
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Category = Category;
			SaveClipData();
			return;
		}
	}
}

void UMGClipSubsystem::AddClipTag(FGuid ClipID, const FString& Tag)
{
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			if (!Clip.Tags.Contains(Tag))
			{
				Clip.Tags.Add(Tag);
				SaveClipData();
			}
			return;
		}
	}
}

void UMGClipSubsystem::RemoveClipTag(FGuid ClipID, const FString& Tag)
{
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Tags.Remove(Tag);
			SaveClipData();
			return;
		}
	}
}

// ============================================================================
// Editing
// ============================================================================

FGuid UMGClipSubsystem::TrimClip(FGuid ClipID, float StartTime, float EndTime)
{
	FMGClipMetadata SourceClip = GetClip(ClipID);
	if (!SourceClip.ClipID.IsValid())
	{
		return FGuid();
	}

	FMGClipEditSettings EditSettings;
	EditSettings.StartTime = StartTime;
	EditSettings.EndTime = EndTime;

	return CreateEditedClip(ClipID, EditSettings);
}

FGuid UMGClipSubsystem::CreateEditedClip(FGuid SourceClipID, const FMGClipEditSettings& EditSettings)
{
	FMGClipMetadata SourceClip = GetClip(SourceClipID);
	if (!SourceClip.ClipID.IsValid())
	{
		return FGuid();
	}

	FMGClipMetadata NewClip;
	NewClip.ClipID = FGuid::NewGuid();
	NewClip.Title = SourceClip.Title + TEXT("_Edited");
	NewClip.Status = EMGClipStatus::Processing;
	NewClip.Quality = SourceClip.Quality;
	NewClip.Duration = EditSettings.EndTime - EditSettings.StartTime;
	NewClip.RecordedAt = FDateTime::UtcNow();
	NewClip.FilePath = GenerateClipPath(NewClip.ClipID);
	NewClip.ThumbnailPath = NewClip.FilePath.Replace(TEXT(".mp4"), TEXT("_thumb.jpg"));
	NewClip.Category = SourceClip.Category;
	NewClip.TrackID = SourceClip.TrackID;
	NewClip.GameModeID = SourceClip.GameModeID;
	NewClip.VehicleID = SourceClip.VehicleID;

	// Would process the video here using EditSettings
	NewClip.Status = EMGClipStatus::Ready;

	SavedClips.Add(NewClip);
	Stats.TotalClipsRecorded++;

	EnforceStorageLimits();
	UpdateStats();

	OnClipReady.Broadcast(NewClip);
	SaveClipData();

	return NewClip.ClipID;
}

bool UMGClipSubsystem::ExportClip(FGuid ClipID, const FString& OutputPath, EMGClipQuality Quality)
{
	FMGClipMetadata Clip = GetClip(ClipID);
	if (!Clip.ClipID.IsValid())
	{
		return false;
	}

	// Would export the clip with specified quality
	// For now, just copy the file
	return IFileManager::Get().Copy(*OutputPath, *Clip.FilePath) == COPY_OK;
}

// ============================================================================
// Sharing
// ============================================================================

bool UMGClipSubsystem::ShareClip(FGuid ClipID, EMGSharePlatform Platform)
{
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Status = EMGClipStatus::Uploading;
			UploadProgress.Add(ClipID, 0.0f);

			// Would initiate upload here
			// Simulating completion
			Clip.Status = EMGClipStatus::Uploaded;
			Clip.bIsShared = true;
			Clip.ShareURL = FString::Printf(TEXT("https://midnightgrind.gg/clips/%s"), *ClipID.ToString());

			Stats.TotalClipsShared++;

			UploadProgress.Remove(ClipID);

			OnClipShared.Broadcast(ClipID, Platform);
			SaveClipData();

			return true;
		}
	}

	return false;
}

FString UMGClipSubsystem::GetShareURL(FGuid ClipID) const
{
	FMGClipMetadata Clip = GetClip(ClipID);
	return Clip.ShareURL;
}

void UMGClipSubsystem::CopyClipToClipboard(FGuid ClipID)
{
	FMGClipMetadata Clip = GetClip(ClipID);
	if (Clip.ClipID.IsValid() && !Clip.ShareURL.IsEmpty())
	{
		FPlatformApplicationMisc::ClipboardCopy(*Clip.ShareURL);
	}
}

bool UMGClipSubsystem::IsClipUploading(FGuid ClipID) const
{
	return UploadProgress.Contains(ClipID);
}

float UMGClipSubsystem::GetUploadProgress(FGuid ClipID) const
{
	if (const float* Progress = UploadProgress.Find(ClipID))
	{
		return *Progress;
	}
	return 0.0f;
}

// ============================================================================
// Highlights
// ============================================================================

void UMGClipSubsystem::RegisterHighlightMoment(const FMGHighlightMoment& Moment)
{
	RecentHighlights.Add(Moment);

	// Limit recent highlights
	const int32 MaxRecentHighlights = 20;
	while (RecentHighlights.Num() > MaxRecentHighlights)
	{
		RecentHighlights.RemoveAt(0);
	}

	OnHighlightDetected.Broadcast(Moment);

	// Auto-capture if enabled
	if (Settings.bCaptureHighlights)
	{
		SaveHighlight(Moment);
	}
}

FGuid UMGClipSubsystem::SaveHighlight(const FMGHighlightMoment& Highlight)
{
	FGuid ClipID = SaveLastSeconds(10.0f, Highlight.Description.ToString());

	// Update clip metadata
	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Category = Highlight.Category;
			Clip.bIsHighlight = true;
			Clip.FeaturedPlayers = Highlight.InvolvedPlayers;
			break;
		}
	}

	return ClipID;
}

void UMGClipSubsystem::ClearHighlights()
{
	RecentHighlights.Empty();
}

// ============================================================================
// Auto-capture
// ============================================================================

void UMGClipSubsystem::TriggerVictoryCapture()
{
	if (!Settings.bCaptureVictories)
	{
		return;
	}

	FMGHighlightMoment Moment;
	Moment.Category = EMGClipCategory::Victory;
	Moment.Description = FText::FromString(TEXT("Victory!"));

	if (const UWorld* World = GetWorld())
	{
		Moment.Timestamp = World->GetTimeSeconds();
	}

	FGuid ClipID = SaveLastSeconds(15.0f, TEXT("Victory"));

	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Category = EMGClipCategory::Victory;
			OnAutoClipSaved.Broadcast(Clip);
			break;
		}
	}
}

void UMGClipSubsystem::TriggerCloseFinishCapture()
{
	if (!Settings.bCaptureCloseFinishes)
	{
		return;
	}

	FGuid ClipID = SaveLastSeconds(10.0f, TEXT("Photo Finish"));

	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Category = EMGClipCategory::PhotoFinish;
			OnAutoClipSaved.Broadcast(Clip);
			break;
		}
	}
}

void UMGClipSubsystem::TriggerHighlightCapture(EMGClipCategory Category, const FText& Description)
{
	FGuid ClipID = SaveLastSeconds(10.0f, Description.ToString());

	for (FMGClipMetadata& Clip : SavedClips)
	{
		if (Clip.ClipID == ClipID)
		{
			Clip.Category = Category;
			Clip.Description = Description;
			Clip.bIsHighlight = true;
			OnAutoClipSaved.Broadcast(Clip);
			break;
		}
	}
}

// ============================================================================
// Settings
// ============================================================================

void UMGClipSubsystem::SetClipSettings(const FMGClipSettings& NewSettings)
{
	Settings = NewSettings;
	EnforceStorageLimits();
	SaveClipData();
}

// ============================================================================
// Stats
// ============================================================================

int64 UMGClipSubsystem::GetTotalStorageUsed() const
{
	return Stats.TotalStorageUsed;
}

int64 UMGClipSubsystem::GetRemainingStorage() const
{
	int64 MaxBytes = Settings.MaxStorageSizeMB * 1024 * 1024;
	return FMath::Max(0LL, MaxBytes - Stats.TotalStorageUsed);
}

// ============================================================================
// Thumbnail
// ============================================================================

void UMGClipSubsystem::GenerateThumbnail(FGuid ClipID, float AtTime)
{
	// Would extract frame from video at specified time and save as thumbnail
}

TSoftObjectPtr<UTexture2D> UMGClipSubsystem::GetClipThumbnail(FGuid ClipID) const
{
	// Would load thumbnail texture
	return TSoftObjectPtr<UTexture2D>();
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGClipSubsystem::OnClipTick()
{
	ProcessRecordingBuffer();
}

void UMGClipSubsystem::ProcessRecordingBuffer()
{
	// Would manage rolling recording buffer here
}

void UMGClipSubsystem::EnforceStorageLimits()
{
	// Sort by date (oldest first)
	SavedClips.Sort([](const FMGClipMetadata& A, const FMGClipMetadata& B)
	{
		return A.RecordedAt < B.RecordedAt;
	});

	// Remove old clips if over count limit
	while (SavedClips.Num() > Settings.MaxStoredClips)
	{
		// Don't delete favorites
		for (int32 i = 0; i < SavedClips.Num(); ++i)
		{
			if (!SavedClips[i].bIsFavorite)
			{
				DeleteClip(SavedClips[i].ClipID);
				break;
			}
		}
	}

	// Remove old clips if over storage limit
	int64 MaxBytes = Settings.MaxStorageSizeMB * 1024 * 1024;
	while (Stats.TotalStorageUsed > MaxBytes && SavedClips.Num() > 0)
	{
		// Find oldest non-favorite clip
		for (int32 i = 0; i < SavedClips.Num(); ++i)
		{
			if (!SavedClips[i].bIsFavorite)
			{
				DeleteClip(SavedClips[i].ClipID);
				break;
			}
		}
	}
}

FString UMGClipSubsystem::GenerateClipPath(FGuid ClipID) const
{
	return ClipStoragePath / FString::Printf(TEXT("%s.mp4"), *ClipID.ToString());
}

void UMGClipSubsystem::UpdateStats()
{
	Stats.TotalStorageUsed = 0;
	Stats.TotalViews = 0;
	Stats.TotalLikes = 0;

	for (const FMGClipMetadata& Clip : SavedClips)
	{
		Stats.TotalStorageUsed += Clip.FileSizeBytes;
		Stats.TotalViews += Clip.ViewCount;
		Stats.TotalLikes += Clip.LikeCount;
	}
}

void UMGClipSubsystem::SaveClipData()
{
	// Persist clip metadata
	// Implementation would use USaveGame or cloud save
}

void UMGClipSubsystem::LoadClipData()
{
	// Load persisted clip metadata
	// Implementation would use USaveGame or cloud save
}
