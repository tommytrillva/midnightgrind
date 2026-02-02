// Copyright Midnight Grind. All Rights Reserved.

#include "CommunityHighlights/MGCommunityHighlightsSubsystem.h"

void UMGCommunityHighlightsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RefreshHighlights();
}

void UMGCommunityHighlightsSubsystem::RefreshHighlights()
{
	FetchHighlightsFromServer();
}

TArray<FMGCommunityHighlight> UMGCommunityHighlightsSubsystem::GetHighlightsByType(EMGHighlightType Type) const
{
	TArray<FMGCommunityHighlight> Result;
	for (const FMGCommunityHighlight& Highlight : CurrentHighlights)
	{
		if (Highlight.Type == Type)
			Result.Add(Highlight);
	}
	return Result;
}

FMGCommunityHighlight UMGCommunityHighlightsSubsystem::GetHighlightOfTheDay() const
{
	// Return highest priority highlight
	for (const FMGCommunityHighlight& Highlight : CurrentHighlights)
	{
		if (Highlight.Type == EMGHighlightType::ClipOfTheDay || Highlight.Type == EMGHighlightType::PhotoOfTheDay)
			return Highlight;
	}
	return CurrentHighlights.Num() > 0 ? CurrentHighlights[0] : FMGCommunityHighlight();
}

void UMGCommunityHighlightsSubsystem::LikeHighlight(const FString& HighlightID)
{
	for (FMGCommunityHighlight& Highlight : CurrentHighlights)
	{
		if (Highlight.HighlightID == HighlightID && !Highlight.bIsLikedByPlayer)
		{
			Highlight.bIsLikedByPlayer = true;
			Highlight.LikeCount++;
			LikedHighlights.Add(HighlightID);
			// Would send to server
			break;
		}
	}
}

void UMGCommunityHighlightsSubsystem::UnlikeHighlight(const FString& HighlightID)
{
	for (FMGCommunityHighlight& Highlight : CurrentHighlights)
	{
		if (Highlight.HighlightID == HighlightID && Highlight.bIsLikedByPlayer)
		{
			Highlight.bIsLikedByPlayer = false;
			Highlight.LikeCount = FMath::Max(0, Highlight.LikeCount - 1);
			LikedHighlights.Remove(HighlightID);
			break;
		}
	}
}

void UMGCommunityHighlightsSubsystem::DownloadContent(const FString& HighlightID)
{
	for (FMGCommunityHighlight& Highlight : CurrentHighlights)
	{
		if (Highlight.HighlightID == HighlightID)
		{
			Highlight.DownloadCount++;
			// Would trigger actual download
			break;
		}
	}
}

void UMGCommunityHighlightsSubsystem::SubmitForFeature(FName ContentID, EMGHighlightType Type)
{
	// Would submit to server for review
}

bool UMGCommunityHighlightsSubsystem::HasPendingSubmission() const
{
	// Would check server
	return false;
}

FMGCreatorProfile UMGCommunityHighlightsSubsystem::GetCreatorProfile(const FString& PlayerID) const
{
	// Would fetch from server
	FMGCreatorProfile Profile;
	Profile.PlayerID = PlayerID;
	return Profile;
}

void UMGCommunityHighlightsSubsystem::FollowCreator(const FString& PlayerID)
{
	if (!FollowedCreators.Contains(PlayerID))
	{
		FollowedCreators.Add(PlayerID);
	}
}

void UMGCommunityHighlightsSubsystem::FetchHighlightsFromServer()
{
	// Would fetch from backend
	CurrentHighlights.Empty();

	// Sample highlights
	FMGCommunityHighlight LiveryHighlight;
	LiveryHighlight.HighlightID = TEXT("hl_livery_001");
	LiveryHighlight.Type = EMGHighlightType::FeaturedLivery;
	LiveryHighlight.Title = FText::FromString(TEXT("Midnight Aurora"));
	LiveryHighlight.Description = FText::FromString(TEXT("Stunning aurora-inspired livery with PS1 vibes"));
	LiveryHighlight.CreatorName = FText::FromString(TEXT("NeonDreamer"));
	LiveryHighlight.FeaturedDate = FDateTime::UtcNow();
	LiveryHighlight.LikeCount = 1247;
	LiveryHighlight.DownloadCount = 532;
	CurrentHighlights.Add(LiveryHighlight);

	FMGCommunityHighlight ClipHighlight;
	ClipHighlight.HighlightID = TEXT("hl_clip_001");
	ClipHighlight.Type = EMGHighlightType::ClipOfTheDay;
	ClipHighlight.Title = FText::FromString(TEXT("Insane Drift Finish!"));
	ClipHighlight.Description = FText::FromString(TEXT("Photo finish with a perfect drift"));
	ClipHighlight.CreatorName = FText::FromString(TEXT("DriftKing99"));
	ClipHighlight.FeaturedDate = FDateTime::UtcNow();
	ClipHighlight.LikeCount = 3891;
	CurrentHighlights.Add(ClipHighlight);

	FMGCommunityHighlight RacerHighlight;
	RacerHighlight.HighlightID = TEXT("hl_racer_001");
	RacerHighlight.Type = EMGHighlightType::TopRacer;
	RacerHighlight.Title = FText::FromString(TEXT("This Week's Champion"));
	RacerHighlight.CreatorName = FText::FromString(TEXT("MidnightLegend"));
	RacerHighlight.FeaturedDate = FDateTime::UtcNow();
	CurrentHighlights.Add(RacerHighlight);

	OnHighlightsFetched.Broadcast(CurrentHighlights);
	CheckIfPlayerFeatured();
}

void UMGCommunityHighlightsSubsystem::CheckIfPlayerFeatured()
{
	for (const FMGCommunityHighlight& Highlight : CurrentHighlights)
	{
		if (Highlight.CreatorPlayerID == LocalPlayerID)
		{
			OnPlayerFeatured.Broadcast(Highlight);
			break;
		}
	}
}
