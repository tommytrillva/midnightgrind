// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNewsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGNewsCategory : uint8
{
	PatchNotes,
	Event,
	Community,
	Esports,
	Maintenance,
	Feature,
	Season
};

UENUM(BlueprintType)
enum class EMGNewsPriority : uint8
{
	Low,
	Normal,
	High,
	Critical
};

USTRUCT(BlueprintType)
struct FMGNewsArticle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ArticleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FullContent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNewsCategory Category = EMGNewsCategory::PatchNotes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNewsPriority Priority = EMGNewsPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime PublishDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiryDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ImageURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ActionButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresAcknowledgement = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Tags;
};

USTRUCT(BlueprintType)
struct FMGPatchNote
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Version;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReleaseDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> NewFeatures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Improvements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> BugFixes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> BalanceChanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> KnownIssues;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNewArticleReceived, const FMGNewsArticle&, Article);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCriticalNewsReceived, const FMGNewsArticle&, Article);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnNewsRefreshed);

UCLASS()
class MIDNIGHTGRIND_API UMGNewsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// News Retrieval
	UFUNCTION(BlueprintCallable, Category = "News")
	void RefreshNews();

	UFUNCTION(BlueprintPure, Category = "News")
	TArray<FMGNewsArticle> GetAllArticles() const { return Articles; }

	UFUNCTION(BlueprintPure, Category = "News")
	TArray<FMGNewsArticle> GetArticlesByCategory(EMGNewsCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "News")
	TArray<FMGNewsArticle> GetUnreadArticles() const;

	UFUNCTION(BlueprintPure, Category = "News")
	int32 GetUnreadCount() const;

	UFUNCTION(BlueprintPure, Category = "News")
	FMGNewsArticle GetArticle(const FString& ArticleID) const;

	// Article Interaction
	UFUNCTION(BlueprintCallable, Category = "News")
	void MarkAsRead(const FString& ArticleID);

	UFUNCTION(BlueprintCallable, Category = "News")
	void MarkAllAsRead();

	UFUNCTION(BlueprintCallable, Category = "News")
	void AcknowledgeArticle(const FString& ArticleID);

	// Patch Notes
	UFUNCTION(BlueprintPure, Category = "News|Patch")
	FMGPatchNote GetCurrentPatchNotes() const { return CurrentPatch; }

	UFUNCTION(BlueprintPure, Category = "News|Patch")
	TArray<FMGPatchNote> GetPatchHistory() const { return PatchHistory; }

	UFUNCTION(BlueprintPure, Category = "News|Patch")
	bool HasUnseenPatchNotes() const;

	UFUNCTION(BlueprintCallable, Category = "News|Patch")
	void MarkPatchNotesAsSeen();

	// Maintenance
	UFUNCTION(BlueprintPure, Category = "News|Maintenance")
	bool IsMaintenanceScheduled() const;

	UFUNCTION(BlueprintPure, Category = "News|Maintenance")
	FDateTime GetNextMaintenanceTime() const;

	UFUNCTION(BlueprintPure, Category = "News|Maintenance")
	FTimespan GetTimeUntilMaintenance() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "News|Events")
	FMGOnNewArticleReceived OnNewArticleReceived;

	UPROPERTY(BlueprintAssignable, Category = "News|Events")
	FMGOnCriticalNewsReceived OnCriticalNewsReceived;

	UPROPERTY(BlueprintAssignable, Category = "News|Events")
	FMGOnNewsRefreshed OnNewsRefreshed;

protected:
	void FetchNewsFromServer();
	void ProcessNewArticles(const TArray<FMGNewsArticle>& NewArticles);
	void LoadReadStatus();
	void SaveReadStatus();

private:
	UPROPERTY()
	TArray<FMGNewsArticle> Articles;

	UPROPERTY()
	FMGPatchNote CurrentPatch;

	UPROPERTY()
	TArray<FMGPatchNote> PatchHistory;

	TSet<FString> ReadArticleIDs;
	TSet<FString> AcknowledgedArticleIDs;
	FString LastSeenPatchVersion;
	FDateTime ScheduledMaintenanceTime;
	FTimerHandle RefreshTimerHandle;
};
