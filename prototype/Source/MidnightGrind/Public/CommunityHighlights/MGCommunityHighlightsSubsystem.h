// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCommunityHighlightsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGHighlightType : uint8
{
	FeaturedLivery,
	FeaturedTrack,
	TopRacer,
	ClipOfTheDay,
	PhotoOfTheDay,
	CrewSpotlight,
	RisingTalent,
	LegendStatus
};

USTRUCT(BlueprintType)
struct FMGCommunityHighlight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HighlightID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHighlightType Type = EMGHighlightType::FeaturedLivery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CreatorPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CreatorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContentID; // Livery ID, Track ID, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ThumbnailURL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MediaURL; // Video or full image

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FeaturedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LikeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DownloadCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLikedByPlayer = false;
};

USTRUCT(BlueprintType)
struct FMGCreatorProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCreations = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDownloads = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLikes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FeatureCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVerifiedCreator = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHighlightsFetched, const TArray<FMGCommunityHighlight>&, Highlights);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerFeatured, const FMGCommunityHighlight&, Highlight);

UCLASS()
class MIDNIGHTGRIND_API UMGCommunityHighlightsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Highlights
	UFUNCTION(BlueprintCallable, Category = "Community")
	void RefreshHighlights();

	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FMGCommunityHighlight> GetCurrentHighlights() const { return CurrentHighlights; }

	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FMGCommunityHighlight> GetHighlightsByType(EMGHighlightType Type) const;

	UFUNCTION(BlueprintPure, Category = "Community")
	FMGCommunityHighlight GetHighlightOfTheDay() const;

	// Interaction
	UFUNCTION(BlueprintCallable, Category = "Community")
	void LikeHighlight(const FString& HighlightID);

	UFUNCTION(BlueprintCallable, Category = "Community")
	void UnlikeHighlight(const FString& HighlightID);

	UFUNCTION(BlueprintCallable, Category = "Community")
	void DownloadContent(const FString& HighlightID);

	// Submissions
	UFUNCTION(BlueprintCallable, Category = "Community")
	void SubmitForFeature(FName ContentID, EMGHighlightType Type);

	UFUNCTION(BlueprintPure, Category = "Community")
	bool HasPendingSubmission() const;

	// Creator Profiles
	UFUNCTION(BlueprintPure, Category = "Community")
	FMGCreatorProfile GetCreatorProfile(const FString& PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "Community")
	void FollowCreator(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Community")
	TArray<FString> GetFollowedCreators() const { return FollowedCreators; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Community|Events")
	FMGOnHighlightsFetched OnHighlightsFetched;

	UPROPERTY(BlueprintAssignable, Category = "Community|Events")
	FMGOnPlayerFeatured OnPlayerFeatured;

protected:
	void FetchHighlightsFromServer();
	void CheckIfPlayerFeatured();

private:
	UPROPERTY()
	TArray<FMGCommunityHighlight> CurrentHighlights;

	TArray<FString> FollowedCreators;
	TSet<FString> LikedHighlights;
	FString LocalPlayerID;
};
