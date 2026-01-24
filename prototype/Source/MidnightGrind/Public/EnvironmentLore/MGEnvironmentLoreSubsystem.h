// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGEnvironmentLoreSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGLoreCategory : uint8
{
	History,         // City/racing scene history
	Character,       // Character backstories
	Location,        // Location descriptions
	Vehicle,         // Vehicle legends
	Culture,         // Street racing culture
	Mythology,       // Urban legends
	Event,           // Historical events
	Organization,    // Crews, shops, etc.
	Tutorial,        // Gameplay tips
	Secret           // Hidden lore
};

UENUM(BlueprintType)
enum class EMGLoreRarity : uint8
{
	Common,
	Uncommon,
	Rare,
	Epic,
	Legendary,
	Mythic
};

UENUM(BlueprintType)
enum class EMGCollectibleType : uint8
{
	Document,        // Papers, notes, letters
	Photograph,      // Old photos
	Newspaper,       // News clippings
	Recording,       // Audio/video recordings
	Artifact,        // Physical objects
	Graffiti,        // Street art
	Memorial,        // Plaques, monuments
	Poster,          // Posters, flyers
	Landmark,        // Viewable landmarks
	Conversation     // Overheard dialogue
};

USTRUCT(BlueprintType)
struct FMGLoreEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LoreID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Content;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLoreCategory Category = EMGLoreCategory::History;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLoreRarity Rarity = EMGLoreRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> AudioClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RelatedLoreIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RelatedCharacters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RelatedLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChronologicalOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TimelineDate;
};

USTRUCT(BlueprintType)
struct FMGCollectible
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollectibleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLoreEntry LoreEntry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollectibleType Type = EMGCollectibleType::Document;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredMission = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresSpecialCondition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpecialConditionHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiscoveryRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> GlowMaterial;
};

USTRUCT(BlueprintType)
struct FMGCollectibleProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollectibleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDiscovered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DiscoveredTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstReadTime;
};

USTRUCT(BlueprintType)
struct FMGLoreCollection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollectionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CollectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLoreCategory Category = EMGLoreCategory::History;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> LoreIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardItem = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardCurrency = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> CollectionIcon;
};

USTRUCT(BlueprintType)
struct FMGEnvironmentalStory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StoryID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText StoryTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLoreEntry> Chapters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> StoryLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsComplete = false;
};

USTRUCT(BlueprintType)
struct FMGLoreStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollectibles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiscoveredCollectibles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReadCollectibles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGLoreCategory, int32> ByCategory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGLoreRarity, int32> ByRarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollectionsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollections = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectibleDiscovered, const FMGCollectible&, Collectible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoreRead, const FMGLoreEntry&, Lore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectionCompleted, const FMGLoreCollection&, Collection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNearbyCollectible, const FMGCollectible&, Collectible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoreUnlocked, FName, LoreID, const FMGLoreEntry&, Lore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentalStoryProgress, const FMGEnvironmentalStory&, Story);

UCLASS()
class MIDNIGHTGRIND_API UMGEnvironmentLoreSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Lore|Registration")
	void RegisterLoreEntry(const FMGLoreEntry& Entry);

	UFUNCTION(BlueprintCallable, Category = "Lore|Registration")
	void RegisterCollectible(const FMGCollectible& Collectible);

	UFUNCTION(BlueprintCallable, Category = "Lore|Registration")
	void RegisterCollection(const FMGLoreCollection& Collection);

	UFUNCTION(BlueprintCallable, Category = "Lore|Registration")
	void RegisterEnvironmentalStory(const FMGEnvironmentalStory& Story);

	// Discovery
	UFUNCTION(BlueprintCallable, Category = "Lore|Discovery")
	bool DiscoverCollectible(FName CollectibleID);

	UFUNCTION(BlueprintCallable, Category = "Lore|Discovery")
	void DiscoverCollectibleAtLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Lore|Discovery")
	void MarkLoreAsRead(FName LoreID);

	UFUNCTION(BlueprintPure, Category = "Lore|Discovery")
	bool IsCollectibleDiscovered(FName CollectibleID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Discovery")
	bool IsLoreRead(FName LoreID) const;

	// Queries
	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	FMGLoreEntry GetLoreEntry(FName LoreID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	FMGCollectible GetCollectible(FName CollectibleID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGLoreEntry> GetLoreByCategory(EMGLoreCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGLoreEntry> GetDiscoveredLore() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGLoreEntry> GetUnreadLore() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGCollectible> GetCollectiblesInArea(FName LocationID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGCollectible> GetUndiscoveredCollectibles() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGLoreEntry> GetRelatedLore(FName LoreID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGLoreEntry> GetCharacterLore(FName CharacterID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Query")
	TArray<FMGLoreEntry> GetLocationLore(FName LocationID) const;

	// Collections
	UFUNCTION(BlueprintPure, Category = "Lore|Collections")
	TArray<FMGLoreCollection> GetAllCollections() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Collections")
	FMGLoreCollection GetCollection(FName CollectionID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Collections")
	float GetCollectionProgress(FName CollectionID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Collections")
	bool IsCollectionComplete(FName CollectionID) const;

	UFUNCTION(BlueprintCallable, Category = "Lore|Collections")
	bool ClaimCollectionReward(FName CollectionID);

	// Environmental Stories
	UFUNCTION(BlueprintPure, Category = "Lore|Stories")
	TArray<FMGEnvironmentalStory> GetEnvironmentalStories() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Stories")
	FMGEnvironmentalStory GetStory(FName StoryID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Stories")
	int32 GetStoryProgress(FName StoryID) const;

	// Proximity Detection
	UFUNCTION(BlueprintCallable, Category = "Lore|Proximity")
	void UpdatePlayerLocation(FVector Location);

	UFUNCTION(BlueprintPure, Category = "Lore|Proximity")
	TArray<FMGCollectible> GetNearbyCollectibles(float Radius = 500.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Lore|Proximity")
	void SetProximityDetectionEnabled(bool bEnabled);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Lore|Stats")
	FMGLoreStats GetLoreStats() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Stats")
	float GetOverallProgress() const;

	UFUNCTION(BlueprintPure, Category = "Lore|Stats")
	float GetCategoryProgress(EMGLoreCategory Category) const;

	// Timeline
	UFUNCTION(BlueprintPure, Category = "Lore|Timeline")
	TArray<FMGLoreEntry> GetChronologicalLore() const;

	// Search
	UFUNCTION(BlueprintPure, Category = "Lore|Search")
	TArray<FMGLoreEntry> SearchLore(const FString& SearchTerm) const;

	// Hints
	UFUNCTION(BlueprintPure, Category = "Lore|Hints")
	FText GetHintForUndiscoveredCollectible(FName CollectibleID) const;

	UFUNCTION(BlueprintPure, Category = "Lore|Hints")
	FMGCollectible GetClosestUndiscoveredCollectible() const;

	// Save/Load
	UFUNCTION(BlueprintPure, Category = "Lore|Save")
	TArray<FMGCollectibleProgress> GetAllProgress() const { return CollectibleProgress; }

	UFUNCTION(BlueprintCallable, Category = "Lore|Save")
	void LoadProgress(const TArray<FMGCollectibleProgress>& Progress);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Lore|Events")
	FOnCollectibleDiscovered OnCollectibleDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Lore|Events")
	FOnLoreRead OnLoreRead;

	UPROPERTY(BlueprintAssignable, Category = "Lore|Events")
	FOnCollectionCompleted OnCollectionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Lore|Events")
	FOnNearbyCollectible OnNearbyCollectible;

	UPROPERTY(BlueprintAssignable, Category = "Lore|Events")
	FOnLoreUnlocked OnLoreUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Lore|Events")
	FOnEnvironmentalStoryProgress OnEnvironmentalStoryProgress;

protected:
	void OnLoreTick();
	void CheckProximityCollectibles();
	void CheckCollectionCompletion();
	void UpdateStats();
	void InitializeDefaultLore();

	UPROPERTY()
	TMap<FName, FMGLoreEntry> LoreEntries;

	UPROPERTY()
	TMap<FName, FMGCollectible> Collectibles;

	UPROPERTY()
	TMap<FName, FMGLoreCollection> Collections;

	UPROPERTY()
	TMap<FName, FMGEnvironmentalStory> Stories;

	UPROPERTY()
	TArray<FMGCollectibleProgress> CollectibleProgress;

	UPROPERTY()
	TSet<FName> ClaimedCollectionRewards;

	UPROPERTY()
	FVector CurrentPlayerLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bProximityDetectionEnabled = true;

	UPROPERTY()
	float ProximityCheckInterval = 1.0f;

	UPROPERTY()
	float NearbyNotificationRadius = 100.0f;

	UPROPERTY()
	TSet<FName> NotifiedCollectibles;

	FTimerHandle LoreTickHandle;
};
