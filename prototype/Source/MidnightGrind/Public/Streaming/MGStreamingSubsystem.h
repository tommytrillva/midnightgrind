// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGStreamingSubsystem.h
 * Asset Streaming Management System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem manages the loading and unloading of game assets (like track
 * sections, vehicles, textures, etc.) at runtime. Instead of loading everything
 * at once (which would use too much memory and cause long load times), this
 * system loads assets "on demand" as the player moves through the game world.
 *
 * Think of it like a conveyor belt: as the player drives forward, the system
 * loads upcoming track sections and unloads the ones left behind.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. ASSET STREAMING:
 *    - "Streaming" means loading/unloading content dynamically during gameplay
 *    - This prevents the game from needing all content in memory at once
 *    - Critical for large open-world or racing games with long tracks
 *
 * 2. STREAMING PRIORITY (EMGStreamingPriority):
 *    - Critical: Must be loaded NOW (player's car, immediate surroundings)
 *    - High: Should load soon (next track section player is approaching)
 *    - Normal: Standard priority (background elements)
 *    - Low: Can wait (distant scenery)
 *    - Preload: Speculative loading (might be needed based on prediction)
 *
 * 3. ASSET TYPES (EMGAssetType):
 *    - Track: Road geometry, barriers, checkpoints
 *    - Vehicle: Car meshes, physics assets
 *    - Environment: Buildings, trees, skybox elements
 *    - Audio: Sound effects, music
 *    - Texture: Visual surface details
 *    - Animation: Character/vehicle animations
 *
 * 4. SOFT OBJECT POINTERS (TSoftObjectPtr):
 *    - A "soft" reference that doesn't force the asset to load immediately
 *    - The asset path is stored, but the actual data loads on request
 *    - Essential for streaming systems to avoid loading everything upfront
 *
 * 5. CONCURRENT LOADS:
 *    - Multiple assets can load simultaneously (in parallel)
 *    - MaxConcurrentLoads limits how many to prevent disk/CPU overload
 *    - Default of 4 balances loading speed vs. system resources
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 *    [Player Position] --> [Streaming Subsystem] --> [Asset Requests]
 *           |                      |                       |
 *           v                      v                       v
 *    [Velocity/Speed]        [Priority Queue]        [Load/Unload]
 *           |                      |                       |
 *           v                      v                       v
 *    [Predict Future]        [Memory Budget]         [Game World]
 *
 * The subsystem connects to:
 * - MGMemoryBudgetSubsystem: To check if we have memory for new assets
 * - MGLODSubsystem: Works together to balance visual quality vs. memory
 * - MGPerformanceMonitorSubsystem: May reduce streaming if performance drops
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * 1. Preload upcoming track section:
 *    StreamingSubsystem->PreloadTrackSection(TrackID, NextSectionIndex);
 *
 * 2. Update player position (called every frame or tick):
 *    StreamingSubsystem->UpdatePlayerPosition(Location, Velocity);
 *
 * 3. Listen for load completion:
 *    StreamingSubsystem->OnAssetLoaded.AddDynamic(this, &MyClass::HandleLoaded);
 *
 * 4. Check if asset is ready:
 *    if (StreamingSubsystem->IsLoaded(AssetID)) { // Safe to use asset }
 *
 * IMPORTANT NOTES:
 * ----------------
 * - This is a UGameInstanceSubsystem, meaning ONE instance exists for the
 *   entire game session (persists across level transitions)
 * - Always check IsLoaded() before using streamed assets
 * - The system predicts required assets based on player velocity
 * - Memory budget compliance is critical - system may reject loads if over budget
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStreamingSubsystem.generated.h"

/**
 * EMGStreamingPriority - Determines the urgency of loading an asset.
 *
 * The streaming system processes requests in priority order, loading
 * Critical assets first before moving to lower priorities. This ensures
 * the player never sees missing content for important objects.
 *
 * For beginners: Think of this like a hospital triage system - the most
 * urgent cases (Critical) get treated first, while stable patients (Low)
 * can wait a bit longer.
 */
UENUM(BlueprintType)
enum class EMGStreamingPriority : uint8
{
	Critical,    // Must be loaded NOW - player vehicle, nearby track, collision geometry
	High,        // Should be loaded soon - upcoming track sections player is approaching
	Normal,      // Standard streaming - general gameplay assets at medium distance
	Low,         // Background loading - distant scenery, non-essential decorations
	Preload      // Speculative loading - might be needed based on player trajectory prediction
};

/**
 * EMGAssetType - Categories of assets that can be streamed.
 *
 * Different asset types have different memory footprints and loading
 * characteristics. For example, textures are typically larger but load
 * quickly, while audio may need to be fully loaded before playback.
 *
 * The streaming system may apply different strategies per type:
 * - Track: Must be fully loaded before player enters the section
 * - Vehicle: Should be loaded when opponents become visible
 * - Audio: May support partial/progressive loading
 */
UENUM(BlueprintType)
enum class EMGAssetType : uint8
{
	Track,       // Road geometry, racing surface, barriers, checkpoint triggers
	Vehicle,     // Car meshes, vehicle physics assets, wheel models
	Environment, // Buildings, trees, billboards, background scenery
	Audio,       // Sound effects, engine sounds, ambient audio, music tracks
	Texture,     // Visual surface details, decals, UI graphics
	Animation    // Character animations, vehicle suspension animations
};

/**
 * FMGStreamingRequest - Represents a single request to load an asset.
 *
 * When you want to load an asset, you create one of these structs and pass
 * it to RequestLoad(). The struct tracks all the information needed to
 * load the asset and monitor its progress.
 *
 * For beginners: This is like a "ticket" at a deli counter - it contains
 * all the info about what you want (the asset) and tracks your place in line.
 */
USTRUCT(BlueprintType)
struct FMGStreamingRequest
{
	GENERATED_BODY()

	// Unique identifier for this asset (e.g., "Track_Downtown_Section3")
	// Used to look up the asset later and check if it's loaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AssetID;

	// What kind of asset this is - affects how the system prioritizes and handles it
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAssetType AssetType = EMGAssetType::Track;

	// How urgently this asset needs to be loaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreamingPriority Priority = EMGStreamingPriority::Normal;

	// "Soft" pointer to the asset - stores the path without forcing immediate load
	// The actual asset data is loaded asynchronously when processing this request
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> AssetPath;

	// Tracks whether this asset has finished loading (true = ready to use)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLoaded = false;

	// Loading progress from 0.0 (not started) to 1.0 (complete)
	// Useful for showing loading bars or progress indicators
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadProgress = 0.0f;
};

/**
 * FMGStreamingStats - Current state and statistics of the streaming system.
 *
 * This struct provides a snapshot of what the streaming system is doing
 * right now. Useful for debugging, UI displays, and performance monitoring.
 *
 * Example usage: Display a loading indicator when ActiveLoads > 0, or
 * show a warning when MemoryUsedMB approaches MemoryBudgetMB.
 */
USTRUCT(BlueprintType)
struct FMGStreamingStats
{
	GENERATED_BODY()

	// Number of assets waiting in queue to be loaded (not yet started)
	UPROPERTY(BlueprintReadOnly)
	int32 PendingRequests = 0;

	// Number of assets currently being loaded right now (in progress)
	// Limited by MaxConcurrentLoads setting
	UPROPERTY(BlueprintReadOnly)
	int32 ActiveLoads = 0;

	// Total memory used by all currently loaded streamed assets (in megabytes)
	UPROPERTY(BlueprintReadOnly)
	int64 MemoryUsedMB = 0;

	// Maximum memory allowed for streamed assets (in megabytes)
	// When MemoryUsedMB approaches this, low-priority assets may be unloaded
	UPROPERTY(BlueprintReadOnly)
	int64 MemoryBudgetMB = 0;

	// Current disk/network bandwidth usage as a percentage (0.0 to 1.0)
	// High values may indicate the system is at maximum loading capacity
	UPROPERTY(BlueprintReadOnly)
	float BandwidthUsage = 0.0f;
};

/**
 * Event Delegates - Allow other systems to respond to streaming events.
 *
 * DELEGATES IN UNREAL ENGINE (for beginners):
 * A delegate is like a "callback" or "event listener". You can subscribe
 * a function to these delegates, and that function will be called when
 * the event occurs. This is the Observer pattern in action.
 *
 * Example:
 *   StreamingSubsystem->OnAssetLoaded.AddDynamic(this, &MyClass::WhenAssetLoads);
 *
 *   void MyClass::WhenAssetLoads(FName AssetID) {
 *       // Now safe to use the asset!
 *   }
 */

// Fired when an asset finishes loading and is ready to use
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAssetLoaded, FName, AssetID);

// Fired when an asset is removed from memory (no longer usable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAssetUnloaded, FName, AssetID);

// Fired periodically during loading to report progress (0.0 to 1.0)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLoadProgress, FName, AssetID, float, Progress);

/**
 * UMGStreamingSubsystem - The main streaming management class.
 *
 * SUBSYSTEM BASICS (for beginners):
 * ---------------------------------
 * UGameInstanceSubsystem is a special Unreal Engine class that:
 * 1. Automatically creates ONE instance when the game starts
 * 2. Persists for the entire game session (survives level changes)
 * 3. Can be accessed from anywhere via: GetGameInstance()->GetSubsystem<UMGStreamingSubsystem>()
 *
 * This makes it perfect for systems that need to work across the whole game,
 * like streaming which should continue loading assets even during transitions.
 *
 * MIDNIGHTGRIND_API MACRO:
 * This is a DLL export macro. It makes this class visible to other modules
 * that link against the MidnightGrind module. Required for plugin/module architecture.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Called automatically when the game instance is created
	// Sets up timers, initializes default settings, connects to other subsystems
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Called when the game instance is destroyed (game shutdown)
	// Cleans up pending requests, releases loaded assets
	virtual void Deinitialize() override;

	// =========================================================================
	// REQUEST MANAGEMENT
	// These functions let you request assets to be loaded or unloaded
	// =========================================================================
	// Add a new asset to the loading queue
	// The asset will be loaded based on its priority relative to other requests
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RequestLoad(const FMGStreamingRequest& Request);

	// Request that an asset be removed from memory
	// May not happen immediately if the asset is still referenced or in use
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RequestUnload(FName AssetID);

	// Change the priority of an asset that's already in the queue
	// Useful when player direction changes and different assets become more urgent
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetPriority(FName AssetID, EMGStreamingPriority Priority);

	// Check if an asset is fully loaded and ready to use
	// ALWAYS call this before trying to access a streamed asset!
	UFUNCTION(BlueprintPure, Category = "Streaming")
	bool IsLoaded(FName AssetID) const;

	// Get the loading progress of an asset (0.0 to 1.0)
	// Returns 0.0 if not found, 1.0 if fully loaded
	UFUNCTION(BlueprintPure, Category = "Streaming")
	float GetLoadProgress(FName AssetID) const;

	// =========================================================================
	// TRACK STREAMING
	// Specialized functions for loading race track sections
	// =========================================================================
	// Preload a specific section of a track before the player reaches it
	// Call this when you know which section the player is approaching
	// Example: PreloadTrackSection("Downtown", 5) loads the 5th section of Downtown track
	UFUNCTION(BlueprintCallable, Category = "Streaming|Track")
	void PreloadTrackSection(FName TrackID, int32 SectionIndex);

	// Update the system with the player's current position and velocity
	// Called frequently (every frame or tick) to enable predictive loading
	// The system uses velocity to predict where the player will be and preload accordingly
	UFUNCTION(BlueprintCallable, Category = "Streaming|Track")
	void UpdatePlayerPosition(FVector Position, FVector Velocity);

	// Set how far ahead (in Unreal units, usually centimeters) to stream track content
	// Larger radius = more memory used but smoother experience
	// Smaller radius = less memory but risk of pop-in
	UFUNCTION(BlueprintCallable, Category = "Streaming|Track")
	void SetTrackStreamingRadius(float Radius);

	// =========================================================================
	// VEHICLE STREAMING
	// Functions for loading vehicle assets (opponent cars, player liveries, etc.)
	// =========================================================================
	// Preload a vehicle's mesh, physics asset, and default materials
	// Call before spawning opponent vehicles so they appear instantly
	UFUNCTION(BlueprintCallable, Category = "Streaming|Vehicle")
	void PreloadVehicle(FName VehicleID);

	// Preload a specific paint job/livery for a vehicle
	// Liveries are typically texture assets that can be large
	// Example: PreloadVehicleLivery("Supra_MK4", "NeonGreen")
	UFUNCTION(BlueprintCallable, Category = "Streaming|Vehicle")
	void PreloadVehicleLivery(FName VehicleID, FName LiveryID);

	// =========================================================================
	// BANDWIDTH CONTROL
	// Manage how aggressively the system loads assets
	// =========================================================================
	// Set how many assets can load simultaneously
	// Higher values = faster loading but more CPU/disk usage
	// Lower values = slower loading but less resource contention
	// Default: 4. Consider lowering during intense gameplay, raising during menus.
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetMaxConcurrentLoads(int32 MaxLoads);

	// Enable or disable the entire streaming system
	// When disabled, no new loads start (existing loads may complete)
	// Useful during cutscenes or when you need all CPU for something else
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetStreamingEnabled(bool bEnabled);

	// Check if streaming is currently enabled
	UFUNCTION(BlueprintPure, Category = "Streaming")
	bool IsStreamingEnabled() const { return bStreamingEnabled; }

	// =========================================================================
	// STATISTICS
	// Get information about the current state of the streaming system
	// =========================================================================
	// Get current streaming statistics (queue size, memory usage, etc.)
	// Useful for debugging or showing loading status to players
	UFUNCTION(BlueprintPure, Category = "Streaming")
	FMGStreamingStats GetStats() const { return Stats; }

	// =========================================================================
	// EVENTS (Delegates)
	// Subscribe to these to be notified of streaming events
	// =========================================================================
	// Broadcast when an asset finishes loading - bind to this to know when assets are ready
	UPROPERTY(BlueprintAssignable, Category = "Streaming|Events")
	FMGOnAssetLoaded OnAssetLoaded;

	// Broadcast when an asset is unloaded from memory
	UPROPERTY(BlueprintAssignable, Category = "Streaming|Events")
	FMGOnAssetUnloaded OnAssetUnloaded;

	// Broadcast periodically during asset loading with progress updates
	UPROPERTY(BlueprintAssignable, Category = "Streaming|Events")
	FMGOnLoadProgress OnLoadProgress;

protected:
	// =========================================================================
	// INTERNAL METHODS (protected)
	// These are implementation details - you typically won't call these directly
	// =========================================================================
	void ProcessQueue();
	void UpdateStats();
	void PredictRequiredAssets(FVector Position, FVector Velocity);

private:
	// =========================================================================
	// INTERNAL DATA (private)
	// State data used by the subsystem - not directly accessible from outside
	// =========================================================================

	// Queue of assets waiting to be loaded (sorted by priority)
	UPROPERTY()
	TArray<FMGStreamingRequest> PendingRequests;

	// Assets currently in the process of loading
	UPROPERTY()
	TArray<FMGStreamingRequest> ActiveLoads;

	// Map of asset IDs to their loaded UObject pointers
	// Once an asset is here, it's safe to use
	UPROPERTY()
	TMap<FName, UObject*> LoadedAssets;

	// Current streaming statistics (updated periodically)
	FMGStreamingStats Stats;

	// Master switch for the streaming system
	bool bStreamingEnabled = true;

	// How many assets can load at once (parallel loading)
	int32 MaxConcurrentLoads = 4;

	// How far ahead to stream track content (in Unreal units, typically centimeters)
	float TrackStreamingRadius = 500.0f;

	// Cached player position for prediction calculations
	FVector LastPlayerPosition;

	// Timer handle for the periodic ProcessQueue calls
	FTimerHandle ProcessTimerHandle;
};
