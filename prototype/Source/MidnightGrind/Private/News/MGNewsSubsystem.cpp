// Copyright Midnight Grind. All Rights Reserved.

#include "News/MGNewsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGNewsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadReadStatus();

	// Set current patch info
	CurrentPatch.Version = TEXT("1.0.0");
	CurrentPatch.ReleaseDate = FDateTime::UtcNow();
	CurrentPatch.NewFeatures.Add(FText::FromString(TEXT("Launch day! Welcome to MIDNIGHT GRIND")));
	CurrentPatch.NewFeatures.Add(FText::FromString(TEXT("Online multiplayer racing")));
	CurrentPatch.NewFeatures.Add(FText::FromString(TEXT("Career mode with rivals system")));
	CurrentPatch.NewFeatures.Add(FText::FromString(TEXT("Crew system and tournaments")));

	// Refresh news periodically
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UMGNewsSubsystem::FetchNewsFromServer,
			300.0f, // Every 5 minutes
			true,
			5.0f // Initial delay
		);
	}
}

void UMGNewsSubsystem::Deinitialize()
{
	SaveReadStatus();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	Super::Deinitialize();
}

void UMGNewsSubsystem::RefreshNews()
{
	FetchNewsFromServer();
}

TArray<FMGNewsArticle> UMGNewsSubsystem::GetArticlesByCategory(EMGNewsCategory Category) const
{
	TArray<FMGNewsArticle> Result;
	for (const FMGNewsArticle& Article : Articles)
	{
		if (Article.Category == Category)
			Result.Add(Article);
	}
	return Result;
}

TArray<FMGNewsArticle> UMGNewsSubsystem::GetUnreadArticles() const
{
	TArray<FMGNewsArticle> Unread;
	for (const FMGNewsArticle& Article : Articles)
	{
		if (!Article.bIsRead)
			Unread.Add(Article);
	}
	return Unread;
}

int32 UMGNewsSubsystem::GetUnreadCount() const
{
	int32 Count = 0;
	for (const FMGNewsArticle& Article : Articles)
	{
		if (!Article.bIsRead)
			Count++;
	}
	return Count;
}

FMGNewsArticle UMGNewsSubsystem::GetArticle(const FString& ArticleID) const
{
	const FMGNewsArticle* Found = Articles.FindByPredicate(
		[&ArticleID](const FMGNewsArticle& A) { return A.ArticleID == ArticleID; });
	return Found ? *Found : FMGNewsArticle();
}

void UMGNewsSubsystem::MarkAsRead(const FString& ArticleID)
{
	for (FMGNewsArticle& Article : Articles)
	{
		if (Article.ArticleID == ArticleID)
		{
			Article.bIsRead = true;
			ReadArticleIDs.Add(ArticleID);
			break;
		}
	}
	SaveReadStatus();
}

void UMGNewsSubsystem::MarkAllAsRead()
{
	for (FMGNewsArticle& Article : Articles)
	{
		Article.bIsRead = true;
		ReadArticleIDs.Add(Article.ArticleID);
	}
	SaveReadStatus();
}

void UMGNewsSubsystem::AcknowledgeArticle(const FString& ArticleID)
{
	AcknowledgedArticleIDs.Add(ArticleID);
	MarkAsRead(ArticleID);
}

bool UMGNewsSubsystem::HasUnseenPatchNotes() const
{
	return LastSeenPatchVersion != CurrentPatch.Version;
}

void UMGNewsSubsystem::MarkPatchNotesAsSeen()
{
	LastSeenPatchVersion = CurrentPatch.Version;
	SaveReadStatus();
}

bool UMGNewsSubsystem::IsMaintenanceScheduled() const
{
	return ScheduledMaintenanceTime > FDateTime::UtcNow();
}

FDateTime UMGNewsSubsystem::GetNextMaintenanceTime() const
{
	return ScheduledMaintenanceTime;
}

FTimespan UMGNewsSubsystem::GetTimeUntilMaintenance() const
{
	if (!IsMaintenanceScheduled())
		return FTimespan::Zero();
	return ScheduledMaintenanceTime - FDateTime::UtcNow();
}

void UMGNewsSubsystem::FetchNewsFromServer()
{
	// Would fetch from backend API
	// For now, create sample articles

	TArray<FMGNewsArticle> NewArticles;

	FMGNewsArticle WelcomeArticle;
	WelcomeArticle.ArticleID = TEXT("welcome_001");
	WelcomeArticle.Title = FText::FromString(TEXT("Welcome to MIDNIGHT GRIND"));
	WelcomeArticle.Summary = FText::FromString(TEXT("The streets are calling. Are you ready to answer?"));
	WelcomeArticle.Category = EMGNewsCategory::Feature;
	WelcomeArticle.Priority = EMGNewsPriority::High;
	WelcomeArticle.PublishDate = FDateTime::UtcNow();
	NewArticles.Add(WelcomeArticle);

	FMGNewsArticle SeasonArticle;
	SeasonArticle.ArticleID = TEXT("season_001");
	SeasonArticle.Title = FText::FromString(TEXT("Season 1: Neon Nights"));
	SeasonArticle.Summary = FText::FromString(TEXT("The first season is here with exclusive rewards!"));
	SeasonArticle.Category = EMGNewsCategory::Season;
	SeasonArticle.Priority = EMGNewsPriority::Normal;
	SeasonArticle.PublishDate = FDateTime::UtcNow();
	SeasonArticle.ActionButtonText = FText::FromString(TEXT("View Season Pass"));
	NewArticles.Add(SeasonArticle);

	ProcessNewArticles(NewArticles);
	OnNewsRefreshed.Broadcast();
}

void UMGNewsSubsystem::ProcessNewArticles(const TArray<FMGNewsArticle>& NewArticles)
{
	for (const FMGNewsArticle& NewArticle : NewArticles)
	{
		// Check if already exists
		int32 ExistingIndex = Articles.IndexOfByPredicate(
			[&NewArticle](const FMGNewsArticle& A) { return A.ArticleID == NewArticle.ArticleID; });

		if (ExistingIndex == INDEX_NONE)
		{
			FMGNewsArticle ArticleToAdd = NewArticle;
			ArticleToAdd.bIsRead = ReadArticleIDs.Contains(NewArticle.ArticleID);

			Articles.Add(ArticleToAdd);
			OnNewArticleReceived.Broadcast(ArticleToAdd);

			if (ArticleToAdd.Priority == EMGNewsPriority::Critical)
			{
				OnCriticalNewsReceived.Broadcast(ArticleToAdd);
			}
		}
	}

	// Sort by date, newest first
	Articles.Sort([](const FMGNewsArticle& A, const FMGNewsArticle& B)
	{
		return A.PublishDate > B.PublishDate;
	});
}

void UMGNewsSubsystem::LoadReadStatus()
{
	// Would load from local storage
}

void UMGNewsSubsystem::SaveReadStatus()
{
	// Would save to local storage
}
