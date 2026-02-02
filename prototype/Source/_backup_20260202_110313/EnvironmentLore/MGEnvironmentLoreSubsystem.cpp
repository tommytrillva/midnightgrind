// Copyright Midnight Grind. All Rights Reserved.

#include "EnvironmentLore/MGEnvironmentLoreSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGEnvironmentLoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultLore();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LoreTickHandle,
			this,
			&UMGEnvironmentLoreSubsystem::OnLoreTick,
			ProximityCheckInterval,
			true
		);
	}
}

void UMGEnvironmentLoreSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LoreTickHandle);
	}
	Super::Deinitialize();
}

bool UMGEnvironmentLoreSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// Registration
void UMGEnvironmentLoreSubsystem::RegisterLoreEntry(const FMGLoreEntry& Entry)
{
	LoreEntries.Add(Entry.LoreID, Entry);
}

void UMGEnvironmentLoreSubsystem::RegisterCollectible(const FMGCollectible& Collectible)
{
	Collectibles.Add(Collectible.CollectibleID, Collectible);

	// Also register the lore entry if not already present
	if (!LoreEntries.Contains(Collectible.LoreEntry.LoreID))
	{
		RegisterLoreEntry(Collectible.LoreEntry);
	}
}

void UMGEnvironmentLoreSubsystem::RegisterCollection(const FMGLoreCollection& Collection)
{
	Collections.Add(Collection.CollectionID, Collection);
}

void UMGEnvironmentLoreSubsystem::RegisterEnvironmentalStory(const FMGEnvironmentalStory& Story)
{
	Stories.Add(Story.StoryID, Story);
}

// Discovery
bool UMGEnvironmentLoreSubsystem::DiscoverCollectible(FName CollectibleID)
{
	const FMGCollectible* Collectible = Collectibles.Find(CollectibleID);
	if (!Collectible)
	{
		return false;
	}

	FMGCollectibleProgress* Progress = CollectibleProgress.FindByPredicate([CollectibleID](const FMGCollectibleProgress& P)
	{
		return P.CollectibleID == CollectibleID;
	});

	if (Progress && Progress->bDiscovered)
	{
		return false; // Already discovered
	}

	if (!Progress)
	{
		FMGCollectibleProgress NewProgress;
		NewProgress.CollectibleID = CollectibleID;
		NewProgress.bDiscovered = true;
		NewProgress.DiscoveredTime = FDateTime::Now();
		CollectibleProgress.Add(NewProgress);
	}
	else
	{
		Progress->bDiscovered = true;
		Progress->DiscoveredTime = FDateTime::Now();
	}

	OnCollectibleDiscovered.Broadcast(*Collectible);
	OnLoreUnlocked.Broadcast(Collectible->LoreEntry.LoreID, Collectible->LoreEntry);

	// Check if this completes a collection
	CheckCollectionCompletion();

	return true;
}

void UMGEnvironmentLoreSubsystem::DiscoverCollectibleAtLocation(FVector Location)
{
	for (const auto& Pair : Collectibles)
	{
		const FMGCollectible& Collectible = Pair.Value;
		float Distance = FVector::Dist(Location, Collectible.WorldLocation);

		if (Distance <= Collectible.DiscoveryRadius)
		{
			DiscoverCollectible(Collectible.CollectibleID);
		}
	}
}

void UMGEnvironmentLoreSubsystem::MarkLoreAsRead(FName LoreID)
{
	// Find collectible that contains this lore
	for (auto& Progress : CollectibleProgress)
	{
		const FMGCollectible* Collectible = Collectibles.Find(Progress.CollectibleID);
		if (Collectible && Collectible->LoreEntry.LoreID == LoreID)
		{
			if (!Progress.bRead)
			{
				Progress.bRead = true;
				Progress.FirstReadTime = FDateTime::Now();

				const FMGLoreEntry* Entry = LoreEntries.Find(LoreID);
				if (Entry)
				{
					OnLoreRead.Broadcast(*Entry);
				}
			}
			return;
		}
	}
}

bool UMGEnvironmentLoreSubsystem::IsCollectibleDiscovered(FName CollectibleID) const
{
	const FMGCollectibleProgress* Progress = CollectibleProgress.FindByPredicate([CollectibleID](const FMGCollectibleProgress& P)
	{
		return P.CollectibleID == CollectibleID;
	});

	return Progress && Progress->bDiscovered;
}

bool UMGEnvironmentLoreSubsystem::IsLoreRead(FName LoreID) const
{
	for (const auto& Progress : CollectibleProgress)
	{
		const FMGCollectible* Collectible = Collectibles.Find(Progress.CollectibleID);
		if (Collectible && Collectible->LoreEntry.LoreID == LoreID)
		{
			return Progress.bRead;
		}
	}
	return false;
}

// Queries
FMGLoreEntry UMGEnvironmentLoreSubsystem::GetLoreEntry(FName LoreID) const
{
	const FMGLoreEntry* Entry = LoreEntries.Find(LoreID);
	return Entry ? *Entry : FMGLoreEntry();
}

FMGCollectible UMGEnvironmentLoreSubsystem::GetCollectible(FName CollectibleID) const
{
	const FMGCollectible* Collectible = Collectibles.Find(CollectibleID);
	return Collectible ? *Collectible : FMGCollectible();
}

TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetLoreByCategory(EMGLoreCategory Category) const
{
	TArray<FMGLoreEntry> Result;

	for (const auto& Pair : LoreEntries)
	{
		if (Pair.Value.Category == Category)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetDiscoveredLore() const
{
	TArray<FMGLoreEntry> Result;

	for (const auto& Progress : CollectibleProgress)
	{
		if (Progress.bDiscovered)
		{
			const FMGCollectible* Collectible = Collectibles.Find(Progress.CollectibleID);
			if (Collectible)
			{
				Result.Add(Collectible->LoreEntry);
			}
		}
	}

	return Result;
}

TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetUnreadLore() const
{
	TArray<FMGLoreEntry> Result;

	for (const auto& Progress : CollectibleProgress)
	{
		if (Progress.bDiscovered && !Progress.bRead)
		{
			const FMGCollectible* Collectible = Collectibles.Find(Progress.CollectibleID);
			if (Collectible)
			{
				Result.Add(Collectible->LoreEntry);
			}
		}
	}

	return Result;
}

TArray<FMGCollectible> UMGEnvironmentLoreSubsystem::GetCollectiblesInArea(FName LocationID) const
{
	TArray<FMGCollectible> Result;

	for (const auto& Pair : Collectibles)
	{
		if (Pair.Value.LocationID == LocationID)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGCollectible> UMGEnvironmentLoreSubsystem::GetUndiscoveredCollectibles() const
{
	TArray<FMGCollectible> Result;

	for (const auto& Pair : Collectibles)
	{
		if (!IsCollectibleDiscovered(Pair.Key))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetRelatedLore(FName LoreID) const
{
	TArray<FMGLoreEntry> Result;

	const FMGLoreEntry* Entry = LoreEntries.Find(LoreID);
	if (!Entry)
	{
		return Result;
	}

	for (FName RelatedID : Entry->RelatedLoreIDs)
	{
		const FMGLoreEntry* Related = LoreEntries.Find(RelatedID);
		if (Related)
		{
			Result.Add(*Related);
		}
	}

	return Result;
}

TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetCharacterLore(FName CharacterID) const
{
	TArray<FMGLoreEntry> Result;

	for (const auto& Pair : LoreEntries)
	{
		if (Pair.Value.RelatedCharacters.Contains(CharacterID))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetLocationLore(FName LocationID) const
{
	TArray<FMGLoreEntry> Result;

	for (const auto& Pair : LoreEntries)
	{
		if (Pair.Value.RelatedLocations.Contains(LocationID))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

// Collections
TArray<FMGLoreCollection> UMGEnvironmentLoreSubsystem::GetAllCollections() const
{
	TArray<FMGLoreCollection> Result;
	Collections.GenerateValueArray(Result);
	return Result;
}

FMGLoreCollection UMGEnvironmentLoreSubsystem::GetCollection(FName CollectionID) const
{
	const FMGLoreCollection* Collection = Collections.Find(CollectionID);
	return Collection ? *Collection : FMGLoreCollection();
}

float UMGEnvironmentLoreSubsystem::GetCollectionProgress(FName CollectionID) const
{
	const FMGLoreCollection* Collection = Collections.Find(CollectionID);
	if (!Collection || Collection->LoreIDs.Num() == 0)
	{
		return 0.0f;
	}

	int32 Discovered = 0;
	for (FName LoreID : Collection->LoreIDs)
	{
		for (const auto& Pair : Collectibles)
		{
			if (Pair.Value.LoreEntry.LoreID == LoreID && IsCollectibleDiscovered(Pair.Key))
			{
				Discovered++;
				break;
			}
		}
	}

	return static_cast<float>(Discovered) / Collection->LoreIDs.Num();
}

bool UMGEnvironmentLoreSubsystem::IsCollectionComplete(FName CollectionID) const
{
	return GetCollectionProgress(CollectionID) >= 1.0f;
}

bool UMGEnvironmentLoreSubsystem::ClaimCollectionReward(FName CollectionID)
{
	if (!IsCollectionComplete(CollectionID))
	{
		return false;
	}

	if (ClaimedCollectionRewards.Contains(CollectionID))
	{
		return false;
	}

	const FMGLoreCollection* Collection = Collections.Find(CollectionID);
	if (!Collection)
	{
		return false;
	}

	ClaimedCollectionRewards.Add(CollectionID);

	// Would grant rewards through economy subsystem
	return true;
}

// Environmental Stories
TArray<FMGEnvironmentalStory> UMGEnvironmentLoreSubsystem::GetEnvironmentalStories() const
{
	TArray<FMGEnvironmentalStory> Result;
	Stories.GenerateValueArray(Result);
	return Result;
}

FMGEnvironmentalStory UMGEnvironmentLoreSubsystem::GetStory(FName StoryID) const
{
	const FMGEnvironmentalStory* Story = Stories.Find(StoryID);
	return Story ? *Story : FMGEnvironmentalStory();
}

int32 UMGEnvironmentLoreSubsystem::GetStoryProgress(FName StoryID) const
{
	const FMGEnvironmentalStory* Story = Stories.Find(StoryID);
	if (!Story)
	{
		return 0;
	}

	int32 Progress = 0;
	for (const FMGLoreEntry& Chapter : Story->Chapters)
	{
		for (const auto& Pair : Collectibles)
		{
			if (Pair.Value.LoreEntry.LoreID == Chapter.LoreID && IsCollectibleDiscovered(Pair.Key))
			{
				Progress++;
				break;
			}
		}
	}

	return Progress;
}

// Proximity Detection
void UMGEnvironmentLoreSubsystem::UpdatePlayerLocation(FVector Location)
{
	CurrentPlayerLocation = Location;
}

TArray<FMGCollectible> UMGEnvironmentLoreSubsystem::GetNearbyCollectibles(float Radius) const
{
	TArray<FMGCollectible> Result;

	for (const auto& Pair : Collectibles)
	{
		if (!IsCollectibleDiscovered(Pair.Key))
		{
			float Distance = FVector::Dist(CurrentPlayerLocation, Pair.Value.WorldLocation);
			if (Distance <= Radius)
			{
				Result.Add(Pair.Value);
			}
		}
	}

	return Result;
}

void UMGEnvironmentLoreSubsystem::SetProximityDetectionEnabled(bool bEnabled)
{
	bProximityDetectionEnabled = bEnabled;
}

// Stats
FMGLoreStats UMGEnvironmentLoreSubsystem::GetLoreStats() const
{
	FMGLoreStats Stats;

	Stats.TotalCollectibles = Collectibles.Num();
	Stats.TotalCollections = Collections.Num();

	for (const auto& Progress : CollectibleProgress)
	{
		if (Progress.bDiscovered)
		{
			Stats.DiscoveredCollectibles++;
		}
		if (Progress.bRead)
		{
			Stats.ReadCollectibles++;
		}
	}

	for (const auto& Pair : Collections)
	{
		if (IsCollectionComplete(Pair.Key))
		{
			Stats.CollectionsCompleted++;
		}
	}

	// Count by category and rarity
	for (const auto& Pair : Collectibles)
	{
		EMGLoreCategory Cat = Pair.Value.LoreEntry.Category;
		EMGLoreRarity Rar = Pair.Value.LoreEntry.Rarity;

		if (IsCollectibleDiscovered(Pair.Key))
		{
			Stats.ByCategory.FindOrAdd(Cat)++;
			Stats.ByRarity.FindOrAdd(Rar)++;
		}
	}

	return Stats;
}

float UMGEnvironmentLoreSubsystem::GetOverallProgress() const
{
	if (Collectibles.Num() == 0)
	{
		return 0.0f;
	}

	int32 Discovered = 0;
	for (const auto& Pair : Collectibles)
	{
		if (IsCollectibleDiscovered(Pair.Key))
		{
			Discovered++;
		}
	}

	return static_cast<float>(Discovered) / Collectibles.Num();
}

float UMGEnvironmentLoreSubsystem::GetCategoryProgress(EMGLoreCategory Category) const
{
	int32 Total = 0;
	int32 Discovered = 0;

	for (const auto& Pair : Collectibles)
	{
		if (Pair.Value.LoreEntry.Category == Category)
		{
			Total++;
			if (IsCollectibleDiscovered(Pair.Key))
			{
				Discovered++;
			}
		}
	}

	if (Total == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(Discovered) / Total;
}

// Timeline
TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::GetChronologicalLore() const
{
	TArray<FMGLoreEntry> Result = GetDiscoveredLore();

	Result.Sort([](const FMGLoreEntry& A, const FMGLoreEntry& B)
	{
		return A.ChronologicalOrder < B.ChronologicalOrder;
	});

	return Result;
}

// Search
TArray<FMGLoreEntry> UMGEnvironmentLoreSubsystem::SearchLore(const FString& SearchTerm) const
{
	TArray<FMGLoreEntry> Result;
	FString LowerSearch = SearchTerm.ToLower();

	for (const auto& Pair : LoreEntries)
	{
		FString TitleLower = Pair.Value.Title.ToString().ToLower();
		FString ContentLower = Pair.Value.Content.ToString().ToLower();

		if (TitleLower.Contains(LowerSearch) || ContentLower.Contains(LowerSearch))
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

// Hints
FText UMGEnvironmentLoreSubsystem::GetHintForUndiscoveredCollectible(FName CollectibleID) const
{
	const FMGCollectible* Collectible = Collectibles.Find(CollectibleID);
	if (!Collectible)
	{
		return FText::GetEmpty();
	}

	if (Collectible->bRequiresSpecialCondition)
	{
		return Collectible->SpecialConditionHint;
	}

	// Return generic location hint
	return FText::Format(
		FText::FromString(TEXT("Search near {0}")),
		FText::FromName(Collectible->LocationID)
	);
}

FMGCollectible UMGEnvironmentLoreSubsystem::GetClosestUndiscoveredCollectible() const
{
	FMGCollectible Closest;
	float ClosestDist = FLT_MAX;

	for (const auto& Pair : Collectibles)
	{
		if (!IsCollectibleDiscovered(Pair.Key) && !Pair.Value.bIsHidden)
		{
			float Dist = FVector::Dist(CurrentPlayerLocation, Pair.Value.WorldLocation);
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				Closest = Pair.Value;
			}
		}
	}

	return Closest;
}

// Save/Load
void UMGEnvironmentLoreSubsystem::LoadProgress(const TArray<FMGCollectibleProgress>& Progress)
{
	CollectibleProgress = Progress;
	CheckCollectionCompletion();
}

// Internal
void UMGEnvironmentLoreSubsystem::OnLoreTick()
{
	if (bProximityDetectionEnabled)
	{
		CheckProximityCollectibles();
	}
}

void UMGEnvironmentLoreSubsystem::CheckProximityCollectibles()
{
	TArray<FMGCollectible> Nearby = GetNearbyCollectibles(NearbyNotificationRadius);

	for (const FMGCollectible& Collectible : Nearby)
	{
		if (!NotifiedCollectibles.Contains(Collectible.CollectibleID))
		{
			NotifiedCollectibles.Add(Collectible.CollectibleID);
			OnNearbyCollectible.Broadcast(Collectible);
		}
	}
}

void UMGEnvironmentLoreSubsystem::CheckCollectionCompletion()
{
	for (const auto& Pair : Collections)
	{
		if (IsCollectionComplete(Pair.Key) && !ClaimedCollectionRewards.Contains(Pair.Key))
		{
			OnCollectionCompleted.Broadcast(Pair.Value);
		}
	}
}

void UMGEnvironmentLoreSubsystem::UpdateStats()
{
	// Stats are calculated on demand via GetLoreStats()
}

void UMGEnvironmentLoreSubsystem::InitializeDefaultLore()
{
	// Create default history collection
	FMGLoreCollection HistoryCollection;
	HistoryCollection.CollectionID = FName("COLLECTION_CITY_HISTORY");
	HistoryCollection.CollectionName = FText::FromString(TEXT("City Origins"));
	HistoryCollection.Description = FText::FromString(TEXT("Discover the history of the city's street racing scene"));
	HistoryCollection.Category = EMGLoreCategory::History;
	HistoryCollection.RewardCurrency = 10000;
	RegisterCollection(HistoryCollection);

	// Create legends collection
	FMGLoreCollection LegendsCollection;
	LegendsCollection.CollectionID = FName("COLLECTION_RACING_LEGENDS");
	LegendsCollection.CollectionName = FText::FromString(TEXT("Racing Legends"));
	LegendsCollection.Description = FText::FromString(TEXT("Learn about the legendary racers who built the scene"));
	LegendsCollection.Category = EMGLoreCategory::Character;
	LegendsCollection.RewardItem = FName("LIVERY_LEGEND");
	RegisterCollection(LegendsCollection);

	// Create urban myths collection
	FMGLoreCollection MythsCollection;
	MythsCollection.CollectionID = FName("COLLECTION_URBAN_MYTHS");
	MythsCollection.CollectionName = FText::FromString(TEXT("Urban Myths"));
	MythsCollection.Description = FText::FromString(TEXT("Uncover the urban legends of midnight street racing"));
	MythsCollection.Category = EMGLoreCategory::Mythology;
	MythsCollection.RewardItem = FName("DECAL_MYTHIC");
	RegisterCollection(MythsCollection);
}
