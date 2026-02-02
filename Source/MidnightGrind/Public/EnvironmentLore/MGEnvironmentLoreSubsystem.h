// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGEnvironmentLoreSubsystem.h
 * @brief Environmental Lore and Collectibles Subsystem for Midnight Grind
 *
 * This subsystem manages the game's narrative content, world-building lore,
 * collectible items, and environmental storytelling elements.
 *
 * ## Overview
 * The Environment Lore Subsystem creates an immersive world by:
 * - Managing collectible lore items scattered throughout the game world
 * - Tracking player discovery and reading progress
 * - Organizing lore into themed collections with rewards
 * - Supporting environmental storytelling through placed narratives
 * - Providing proximity-based discovery of nearby collectibles
 *
 * ## Architecture
 * This is a World Subsystem (not Game Instance), meaning:
 * - It exists per-world/level, not globally across the game
 * - ShouldCreateSubsystem() controls which worlds get this subsystem
 * - World-specific lore can be loaded per level
 *
 * It integrates with:
 * - Save/Load systems (persists discovery progress)
 * - UI/HUD systems (collectible notifications, lore viewer)
 * - Map/minimap systems (collectible markers)
 * - Audio systems (ambient audio clips for some lore)
 *
 * ## Key Concepts for Entry-Level Developers
 *
 * ### Lore Categories (EMGLoreCategory)
 * Types of narrative content organized by theme:
 * - **History**: Background on the city and racing scene's past
 * - **Character**: Backstories for NPCs and important figures
 * - **Location**: Descriptions and history of specific places
 * - **Vehicle**: Legends about famous cars and their owners
 * - **Culture**: Information about street racing culture
 * - **Mythology**: Urban legends and rumors in the racing world
 * - **Event**: Historical races and significant moments
 * - **Organization**: Info about crews, shops, and factions
 * - **Tutorial**: Gameplay tips disguised as in-world content
 * - **Secret**: Hidden lore for dedicated explorers
 *
 * ### Lore Rarity (EMGLoreRarity)
 * How rare/valuable a piece of lore is:
 * - Common -> Mythic (affects rewards and discovery difficulty)
 * - Rarer lore may require special conditions to find
 *
 * ### Collectible Types (EMGCollectibleType)
 * The physical form collectibles take in the world:
 * - **Document/Newspaper**: Written materials to read
 * - **Photograph**: Visual historical content
 * - **Recording**: Audio/video clips that play
 * - **Artifact**: Physical objects with significance
 * - **Graffiti/Poster**: Street art and flyers
 * - **Memorial**: Plaques and monuments
 * - **Landmark**: Viewable points of interest
 * - **Conversation**: Overheard NPC dialogue
 *
 * ### Lore Entry (FMGLoreEntry)
 * A single piece of lore content containing:
 * - Title and content text (the actual narrative)
 * - Category and rarity classification
 * - Optional image and audio assets
 * - Related lore IDs for connected narratives
 * - Chronological ordering for timeline views
 *
 * ### Collectible (FMGCollectible)
 * A world-placed item that unlocks lore:
 * - Has a world location where it can be found
 * - May require missions or conditions to appear
 * - Has a discovery radius (how close player must be)
 * - Can be hidden (not shown on map until found)
 * - Contains the lore entry it unlocks
 *
 * ### Collections (FMGLoreCollection)
 * Groups of related lore with completion rewards:
 * - Finding all items in a collection grants bonuses
 * - Rewards can be currency, items, or achievements
 * - Provides structure for completionist players
 *
 * ### Environmental Stories (FMGEnvironmentalStory)
 * Multi-part narratives told through the environment:
 * - Consist of multiple chapters (lore entries)
 * - Tied to specific locations in sequence
 * - Tell stories through exploration rather than cutscenes
 *
 * ## Data Flow
 * 1. Lore and collectibles registered via Register* functions
 * 2. Player explores world, UpdatePlayerLocation() called
 * 3. Proximity check finds nearby collectibles
 * 4. Player collects item -> DiscoverCollectible()
 * 5. OnCollectibleDiscovered event fires for UI notification
 * 6. Player reads lore -> MarkLoreAsRead()
 * 7. Collections checked for completion, rewards granted
 *
 * ## Usage Example
 * @code
 * // Get the lore subsystem for the current world
 * UMGEnvironmentLoreSubsystem* LoreSystem = GetWorld()->GetSubsystem<UMGEnvironmentLoreSubsystem>();
 *
 * // Update player position for proximity detection
 * LoreSystem->UpdatePlayerLocation(PlayerLocation);
 *
 * // Get nearby collectibles to show on minimap
 * TArray<FMGCollectible> Nearby = LoreSystem->GetNearbyCollectibles(500.0f);
 *
 * // When player interacts with a collectible
 * if (LoreSystem->DiscoverCollectible(CollectibleID))
 * {
 *     // Show collectible popup, play sound
 *     FMGLoreEntry Lore = LoreSystem->GetLoreEntry(LoreID);
 *     // Display lore content in UI
 * }
 *
 * // Check collection completion
 * if (LoreSystem->IsCollectionComplete(CollectionID))
 * {
 *     LoreSystem->ClaimCollectionReward(CollectionID);
 * }
 *
 * // Get progress stats for a menu screen
 * FMGLoreStats Stats = LoreSystem->GetLoreStats();
 * float OverallProgress = LoreSystem->GetOverallProgress(); // 0.0 - 1.0
 * @endcode
 *
 * ## Event System (Delegates)
 * Subscribe to react to lore activities:
 * - OnCollectibleDiscovered: Player found a new collectible
 * - OnLoreRead: Player opened and read a lore entry
 * - OnCollectionCompleted: All items in a collection found
 * - OnNearbyCollectible: Player entered proximity of a collectible
 * - OnLoreUnlocked: New lore became available
 * - OnEnvironmentalStoryProgress: Progress in a multi-part story
 *
 * ## Proximity Detection
 * The subsystem can automatically notify when players approach collectibles:
 * - Enable with SetProximityDetectionEnabled(true)
 * - Configurable check interval and notification radius
 * - Fires OnNearbyCollectible to show UI indicators
 *
 * @see EMGLoreCategory for content categories
 * @see FMGCollectible for collectible configuration
 * @see FMGLoreCollection for collection groupings
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGEnvironmentLoreSubsystem.generated.h"

// ============================================================================
// LORE CATEGORY ENUMERATION
// ============================================================================

/**
 * @brief Categories for organizing narrative content
 *
 * Lore is organized by theme to help players browse the codex
 * and to support filtered views in the UI.
 */
UENUM(BlueprintType)
enum class EMGLoreCategory : uint8
{
	History,         ///< City and racing scene background
	Character,       ///< NPC backstories and biographies
	Location,        ///< Place descriptions and history
	Vehicle,         ///< Car legends and famous vehicles
	Culture,         ///< Street racing culture and traditions
	Mythology,       ///< Urban legends and rumors
	Event,           ///< Historical races and moments
	Organization,    ///< Crews, shops, and factions
	Tutorial,        ///< Gameplay tips as in-world content
	Secret           ///< Hidden lore for dedicated explorers
};

// ============================================================================
// LORE RARITY ENUMERATION
// ============================================================================

/**
 * @brief Rarity tiers for lore and collectibles
 *
 * Higher rarity lore is harder to find and may require
 * special conditions. Affects rewards and achievement tracking.
 */
UENUM(BlueprintType)
enum class EMGLoreRarity : uint8
{
	Common,      ///< Easily found, basic lore
	Uncommon,    ///< Slightly harder to find
	Rare,        ///< Requires exploration
	Epic,        ///< Hidden in obscure locations
	Legendary,   ///< Very difficult to discover
	Mythic       ///< Requires special conditions
};

// ============================================================================
// COLLECTIBLE TYPE ENUMERATION
// ============================================================================

/**
 * @brief Physical form that collectibles take in the game world
 *
 * Different types provide varied discovery experiences and
 * may trigger different UI presentations (text, image, audio).
 */
UENUM(BlueprintType)
enum class EMGCollectibleType : uint8
{
	Document,        ///< Written papers, notes, and letters
	Photograph,      ///< Historical images to view
	Newspaper,       ///< News clippings and articles
	Recording,       ///< Audio or video content
	Artifact,        ///< Physical objects with significance
	Graffiti,        ///< Street art and murals
	Memorial,        ///< Plaques, monuments, and dedications
	Poster,          ///< Promotional flyers and posters
	Landmark,        ///< Viewable points of interest
	Conversation     ///< Overheard NPC dialogue
};

// ============================================================================
// LORE ENTRY STRUCTURE
// ============================================================================

/**
 * @brief A single piece of narrative content
 *
 * Lore entries contain the actual story content players discover.
 * They can include text, images, and audio for rich storytelling.
 * Linked entries create a web of interconnected narratives.
 */
USTRUCT(BlueprintType)
struct FMGLoreEntry
{
	GENERATED_BODY()

	/// Unique identifier for this lore piece
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LoreID;

	/// Display title shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/// Full text content of the lore
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Content;

	/// Brief summary for list views
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortDescription;

	/// Category for organization and filtering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLoreCategory Category = EMGLoreCategory::History;

	/// Rarity affecting discovery difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLoreRarity Rarity = EMGLoreRarity::Common;

	/// Optional image to display with the lore
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Image;

	/// Optional audio clip to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> AudioClip;

	/// IDs of related lore entries for cross-referencing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RelatedLoreIDs;

	/// Character IDs mentioned in this lore
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RelatedCharacters;

	/// Location IDs mentioned in this lore
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RelatedLocations;

	/// Order in timeline view (lower = earlier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChronologicalOrder = 0;

	/// Human-readable date for timeline display
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TimelineDate;
};

// ============================================================================
// COLLECTIBLE STRUCTURE
// ============================================================================

/**
 * @brief A world-placed item that unlocks lore when discovered
 *
 * Collectibles are the physical manifestation of lore in the game world.
 * Players find them by exploring, and discovery unlocks the associated
 * lore entry for reading in the codex.
 */
USTRUCT(BlueprintType)
struct FMGCollectible
{
	GENERATED_BODY()

	/// Unique identifier for this collectible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollectibleID;

	/// Lore content unlocked by this collectible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLoreEntry LoreEntry;

	/// Physical type affecting presentation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollectibleType Type = EMGCollectibleType::Document;

	/// World position where collectible can be found
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	/// Named location area (for filtering by area)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID = NAME_None;

	/// Mission that must be completed before this appears
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredMission = NAME_None;

	/// If true, special gameplay is needed to access
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresSpecialCondition = false;

	/// Hint text for how to unlock if condition required
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpecialConditionHint;

	/// How close player must be to discover (meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiscoveryRadius = 200.0f;

	/// If true, not shown on map until discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHidden = false;

	/// Visual mesh to render in world
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/// Glow material for highlighting
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> GlowMaterial;
};

// ============================================================================
// COLLECTIBLE PROGRESS STRUCTURE
// ============================================================================

/**
 * @brief Tracks player progress on a single collectible
 *
 * Saved to player profile to persist discovery state across sessions.
 * Tracks both discovery (found) and read (opened in codex) states.
 */
USTRUCT(BlueprintType)
struct FMGCollectibleProgress
{
	GENERATED_BODY()

	/// Reference to the collectible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollectibleID;

	/// True if player has found this collectible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDiscovered = false;

	/// True if player has opened and read the lore
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRead = false;

	/// When the collectible was discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DiscoveredTime;

	/// When the player first read the lore content
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstReadTime;
};

// ============================================================================
// LORE COLLECTION STRUCTURE
// ============================================================================

/**
 * @brief A themed group of lore entries with completion rewards
 *
 * Collections encourage completionist gameplay by grouping related
 * lore and providing rewards for finding all entries in a set.
 */
USTRUCT(BlueprintType)
struct FMGLoreCollection
{
	GENERATED_BODY()

	/// Unique identifier for this collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CollectionID;

	/// Display name for the collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CollectionName;

	/// Description of what the collection contains
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Category for organization
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLoreCategory Category = EMGLoreCategory::History;

	/// IDs of lore entries in this collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> LoreIDs;

	/// Item reward for completing the collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardItem = NAME_None;

	/// Currency reward for completing the collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardCurrency = 0;

	/// Icon displayed in collection UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> CollectionIcon;
};

// ============================================================================
// ENVIRONMENTAL STORY STRUCTURE
// ============================================================================

/**
 * @brief A multi-part narrative told through the environment
 *
 * Environmental stories are discovered sequentially by visiting
 * locations in order. Each chapter builds on the previous,
 * telling a story through exploration rather than cutscenes.
 */
USTRUCT(BlueprintType)
struct FMGEnvironmentalStory
{
	GENERATED_BODY()

	/// Unique identifier for this story
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StoryID;

	/// Display title of the story
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText StoryTitle;

	/// Ordered chapters that make up the story
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLoreEntry> Chapters;

	/// World locations for each chapter in order
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> StoryLocations;

	/// True when all chapters have been discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsComplete = false;
};

// ============================================================================
// LORE STATS STRUCTURE
// ============================================================================

/**
 * @brief Aggregate statistics for lore collection progress
 *
 * Provides snapshot data for UI progress displays and
 * achievement tracking. Calculated on demand from progress data.
 */
USTRUCT(BlueprintType)
struct FMGLoreStats
{
	GENERATED_BODY()

	/// Total number of collectibles in the game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollectibles = 0;

	/// Number of collectibles discovered by player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiscoveredCollectibles = 0;

	/// Number of lore entries player has read
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReadCollectibles = 0;

	/// Discovered count by category
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGLoreCategory, int32> ByCategory;

	/// Discovered count by rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGLoreRarity, int32> ByRarity;

	/// Number of collections fully completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollectionsCompleted = 0;

	/// Total number of collections in the game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollections = 0;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/**
 * Delegates allow other systems to react to lore events.
 * Bind to these in Blueprints or C++ to update UI, play sounds, etc.
 */

/// Fired when player discovers a new collectible
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectibleDiscovered, const FMGCollectible&, Collectible);

/// Fired when player opens and reads a lore entry
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoreRead, const FMGLoreEntry&, Lore);

/// Fired when player completes a collection (found all entries)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectionCompleted, const FMGLoreCollection&, Collection);

/// Fired when player enters proximity of an undiscovered collectible
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNearbyCollectible, const FMGCollectible&, Collectible);

/// Fired when lore becomes available (e.g., mission unlock)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoreUnlocked, FName, LoreID, const FMGLoreEntry&, Lore);

/// Fired when player progresses in an environmental story
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentalStoryProgress, const FMGEnvironmentalStory&, Story);

// ============================================================================
// ENVIRONMENT LORE SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core subsystem managing narrative content and collectibles
 *
 * World Subsystem that manages all lore, collectibles, and environmental
 * storytelling for the current world/level.
 *
 * ## Key Differences from Game Instance Subsystem
 * This is a World Subsystem, meaning:
 * - Created per-world (not persistent across levels)
 * - Can have different lore sets per level
 * - ShouldCreateSubsystem() controls which worlds get this
 *
 * ## Responsibilities
 * - Register lore entries, collectibles, and collections
 * - Track player discovery and reading progress
 * - Detect proximity to collectibles
 * - Manage collection completion and rewards
 * - Support environmental story progression
 * - Provide search and filtering for codex UI
 * - Persist progress to save data
 *
 * ## Proximity Detection
 * When enabled, the subsystem periodically checks if the player
 * is near undiscovered collectibles and fires OnNearbyCollectible
 * to show UI indicators.
 *
 * ## Update Flow
 * OnLoreTick() is called periodically to:
 * 1. Check for nearby collectibles
 * 2. Update story progression
 * 3. Check collection completion
 */
UCLASS()
class MIDNIGHTGRIND_API UMGEnvironmentLoreSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/// Controls which worlds get this subsystem
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
