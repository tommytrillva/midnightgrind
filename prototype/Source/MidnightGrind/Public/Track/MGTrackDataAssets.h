// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTrackDataAssets.h
 * @brief Track configuration data assets for Midnight Grind racing game.
 *
 * This file contains data asset definitions for configuring race tracks, including:
 * - Track identity and visual presentation (thumbnails, loading screens)
 * - Track specifications (length, difficulty, surface types)
 * - Timing targets and medal thresholds for lap times
 * - Track layout data (sectors, grid positions, shortcuts, hazards)
 * - Minimap configuration for UI display
 * - Audio settings (ambient sounds, music playlists)
 * - Unlock requirements for progression systems
 *
 * @section usage_track Usage
 * Create track data assets in the Unreal Editor:
 * Right-click in Content Browser > Miscellaneous > Data Asset > MGTrackDataAsset
 *
 * Each track in the game should have a corresponding UMGTrackDataAsset configured
 * with all necessary racing parameters. Use UMGTrackCollectionAsset to group tracks
 * for championships or themed series.
 *
 * @section related_track Related Systems
 * - Track loading: Uses MainLevel and StreamingLevels references
 * - Race timing: Uses Sectors and timing targets
 * - AI racing line: References AMGRacingLineActor
 * - Minimap display: Uses MinimapTexture and WorldBounds
 *
 * @see UMGTrackDataAsset
 * @see UMGTrackCollectionAsset
 * @see UMGTrackSurfaceAsset
 * @see UMGTrackWeatherPreset
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGTrackDataAssets.generated.h"

class UTexture2D;
class UMaterialInterface;
class USoundBase;
class AMGCheckpointActor;
class AMGRacingLineActor;
class AMGTrackBoundaryActor;

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * Track difficulty rating
 *
 * Defines the skill level required to race competitively on a track.
 * Affects AI behavior, recommended vehicle classes, and unlock requirements.
 */
UENUM(BlueprintType)
enum class EMGTrackDifficulty : uint8
{
	Easy,       ///< Beginner-friendly with wide roads and gentle curves
	Medium,     ///< Standard difficulty with moderate challenges
	Hard,       ///< Challenging track requiring good racing skills
	Expert,     ///< Very difficult with tight corners and hazards
	Legendary   ///< Maximum difficulty for elite racers only
};

/**
 * Track surface type
 *
 * Defines the primary road surface material affecting vehicle grip and handling.
 * Each surface type has associated physics properties defined in UMGTrackSurfaceAsset.
 */
UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt,    ///< Standard paved road - best grip
	Concrete,   ///< Concrete surface - slightly less grip than asphalt
	Gravel,     ///< Loose gravel - reduced grip, increased slide
	Dirt,       ///< Packed dirt - low grip, good for drifting
	Sand,       ///< Sandy surface - very low grip, slow
	Wet,        ///< Wet asphalt - reduced grip, spray effects
	Ice,        ///< Icy surface - minimal grip, very slippery
	Mixed       ///< Multiple surface types throughout track
};

/**
 * Track environment type
 *
 * Defines the visual theme and setting of the track.
 * Affects ambient audio, lighting presets, and available weather options.
 */
UENUM(BlueprintType)
enum class EMGTrackEnvironment : uint8
{
	City,       ///< Urban city streets with buildings and traffic lights
	Highway,    ///< Open highway with long straights
	Mountain,   ///< Winding mountain roads with elevation changes
	Coast,      ///< Coastal roads with ocean views
	Industrial, ///< Industrial zone with warehouses and factories
	Suburbs,    ///< Suburban neighborhoods and residential areas
	Downtown,   ///< Dense downtown area with skyscrapers
	Port        ///< Harbor/port area with shipping containers
};

// ============================================================================
// STRUCTURE DEFINITIONS
// ============================================================================

/**
 * Sector definition for track timing
 *
 * Tracks are divided into sectors (typically 3) for detailed timing analysis.
 * Each sector has its own medal time targets for competitive benchmarking.
 * Sector times help players identify areas for improvement.
 */
USTRUCT(BlueprintType)
struct FMGTrackSector
{
	GENERATED_BODY()

	/// Sector index within the track (0-based, typically 0-2 for 3 sectors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SectorIndex = 0;

	/// Human-readable sector name displayed in UI (e.g., "Sector 1", "Tunnel Section")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SectorName;

	/// Index of the checkpoint where this sector begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartCheckpointIndex = 0;

	/// Index of the checkpoint where this sector ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndCheckpointIndex = 0;

	/// Total length of this sector in meters (used for speed calculations)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Length = 0.0f;

	/// Target time in seconds to achieve gold medal in this sector
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldTime = 30.0f;

	/// Target time in seconds to achieve silver medal in this sector
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverTime = 35.0f;

	/// Target time in seconds to achieve bronze medal in this sector
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeTime = 40.0f;
};

/**
 * Track shortcut definition
 *
 * Defines an alternate route through the track that can save time.
 * Shortcuts add strategic depth to racing - higher risk routes offer
 * better times but may be harder to execute cleanly.
 */
USTRUCT(BlueprintType)
struct FMGTrackShortcut
{
	GENERATED_BODY()

	/// Unique identifier for this shortcut (e.g., "SC_AlleyJump")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ShortcutID;

	/// Whether this shortcut is available from race start (false = must be unlocked)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabledByDefault = true;

	/// World position where the shortcut entrance is located
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EntryPosition = FVector::ZeroVector;

	/// World position where the shortcut rejoins the main track
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ExitPosition = FVector::ZeroVector;

	/// Estimated time saved in seconds when shortcut is executed perfectly
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeSaved = 2.0f;

	/// Skill level required to use shortcut effectively (0.0 = easy, 1.0 = very hard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifficultyRating = 0.5f;

	/// Risk level - higher values mean greater chance of crash or time loss
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RiskLevel = 0.5f;
};

/**
 * Hazard zone definition
 *
 * Defines dangerous areas on the track that affect vehicle performance.
 * Examples include oil slicks, debris, water puddles, or construction zones.
 * Hazards add variety and challenge to track layouts.
 */
USTRUCT(BlueprintType)
struct FMGTrackHazard
{
	GENERATED_BODY()

	/// Type of hazard (e.g., "OilSlick", "WaterPuddle", "Debris")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HazardType;

	/// World position at the center of the hazard zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/// Radius in centimeters defining the hazard's area of effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	/// Multiplier for speed reduction when in hazard (0.0-1.0, lower = more reduction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedReduction = 0.2f;

	/// Multiplier for grip reduction when in hazard (0.0-1.0, lower = more slippery)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GripReduction = 0.3f;

	/// Distance in centimeters at which to display hazard warning to player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WarningDistance = 200.0f;
};

/**
 * Starting grid position
 *
 * Defines the spawn location and orientation for a single starting position
 * on the race grid. Position 0 is pole position (first place start).
 */
USTRUCT(BlueprintType)
struct FMGGridPosition
{
	GENERATED_BODY()

	/// Grid position index (0 = pole position, 1 = second place start, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridIndex = 0;

	/// World coordinates for vehicle spawn location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/// Initial rotation for the spawned vehicle (should face track direction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Track leaderboard record
 *
 * Stores information about a recorded time on this track.
 * Used for both local records and server-synced leaderboard entries.
 */
USTRUCT(BlueprintType)
struct FMGTrackRecord
{
	GENERATED_BODY()

	/// Player display name or unique identifier who set this record
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	/// ID of the vehicle used to set this record
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Best single lap time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	/// Total race completion time in seconds (all laps combined)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	/// Timestamp when this record was achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DateSet;

	/// True if this lap was completed without wall hits or penalties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCleanLap = false;
};

// ============================================================================
// DATA ASSET CLASSES
// ============================================================================

/**
 * Track Data Asset
 *
 * Master configuration asset for a race track. Contains all data needed to
 * load, configure, and race on a track including identity info, specifications,
 * timing targets, layout data, and unlock requirements.
 *
 * @section creating_track Creating a Track Asset
 * 1. Right-click in Content Browser
 * 2. Select Miscellaneous > Data Asset
 * 3. Choose MGTrackDataAsset
 * 4. Configure all required properties
 *
 * @section required_track Required Configuration
 * - TrackID: Unique identifier (must be unique across all tracks)
 * - MainLevel: Reference to the actual track level
 * - GridPositions: At least one starting position
 * - CheckpointCount: Must match checkpoints in the level
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTIFICATION
	// ==========================================
	// Basic track identity information for display and lookup.

	/// Unique identifier for this track (e.g., "TRK_Downtown_01"). Must be unique across all tracks.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	FName TrackID;

	/// Localized display name shown in menus (e.g., "Downtown Sprint")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	FText TrackName;

	/// Extended description of the track for selection screens
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity", meta = (MultiLine = true))
	FText Description;

	/// Fictional location name (e.g., "Neo Tokyo", "Miami Beach")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	FText LocationName;

	/// Small preview image for track selection menus (recommended: 256x256)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	UTexture2D* Thumbnail;

	/// Large image displayed during track loading (recommended: 1920x1080)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	UTexture2D* LoadingImage;

	// ==========================================
	// SPECIFICATIONS
	// ==========================================
	// Core track parameters defining its racing characteristics.

	/// Total track length in meters (full lap distance)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	float TrackLength = 5000.0f;

	/// Default number of laps for standard races on this track
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	int32 DefaultLapCount = 3;

	/// Maximum number of vehicles that can race simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	int32 MaxRacers = 8;

	/// Overall difficulty rating affecting AI and recommendations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	EMGTrackDifficulty Difficulty = EMGTrackDifficulty::Medium;

	/// Main surface type (determines default physics properties)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	EMGTrackSurface PrimarySurface = EMGTrackSurface::Asphalt;

	/// Visual environment theme for the track
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	EMGTrackEnvironment Environment = EMGTrackEnvironment::City;

	/// True for closed-loop circuits, false for point-to-point races
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	bool bIsCircuit = true;

	/// Number of checkpoints on the track (must match level actors)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	int32 CheckpointCount = 10;

	// ==========================================
	// TIMING TARGETS
	// ==========================================
	// Medal thresholds and timing benchmarks for competitive play.

	/// Expected average lap time in seconds for a skilled player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float ParLapTime = 90.0f;

	/// Lap time threshold for gold medal (fastest)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float GoldLapTime = 80.0f;

	/// Lap time threshold for silver medal
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float SilverLapTime = 85.0f;

	/// Lap time threshold for bronze medal
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float BronzeLapTime = 95.0f;

	/// Current track record (synced from leaderboard server)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	FMGTrackRecord TrackRecord;

	// ==========================================
	// SECTORS
	// ==========================================
	// Track segment definitions for detailed timing analysis.

	/// Array of sector definitions (typically 3 sectors per track)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Sectors")
	TArray<FMGTrackSector> Sectors;

	// ==========================================
	// LAYOUT
	// ==========================================
	// Track layout elements: start positions, shortcuts, and hazards.

	/// Starting grid positions for all racers (index 0 = pole position)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Layout")
	TArray<FMGGridPosition> GridPositions;

	/// Available shortcuts/alternate routes on this track
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Layout")
	TArray<FMGTrackShortcut> Shortcuts;

	/// Hazard zones that affect vehicle handling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Layout")
	TArray<FMGTrackHazard> Hazards;

	// ==========================================
	// MINIMAP
	// ==========================================
	// Configuration for the in-race minimap display.

	/// Top-down texture of the track for minimap rendering
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Minimap")
	UTexture2D* MinimapTexture;

	/// World-space bounds for mapping positions to minimap UV coordinates
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Minimap")
	FBox2D WorldBounds;

	/// Rotation offset in degrees to align minimap with track orientation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Minimap")
	float MinimapRotation = 0.0f;

	// ==========================================
	// AUDIO
	// ==========================================
	// Track-specific audio configuration.

	/// Looping ambient sound for the track environment
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Audio")
	USoundBase* AmbientSound;

	/// ID of the music playlist to use during races on this track
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Audio")
	FName MusicPlaylistID;

	// ==========================================
	// LEVEL REFERENCES
	// ==========================================
	// References to the actual game levels/maps.

	/// Primary level asset containing the track geometry and actors
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Level")
	TSoftObjectPtr<UWorld> MainLevel;

	/// Additional sub-levels for level streaming (optimization)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Level")
	TArray<TSoftObjectPtr<UWorld>> StreamingLevels;

	// ==========================================
	// UNLOCK
	// ==========================================
	// Player progression requirements to access this track.

	/// If true, track is available immediately without unlocking
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	bool bUnlockedByDefault = true;

	/// Minimum player level required to unlock (if not unlocked by default)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	int32 RequiredLevel = 1;

	/// Minimum reputation points required to unlock
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	int32 RequiredReputation = 0;

	/// In-game currency cost to purchase/unlock the track
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	int32 UnlockPrice = 0;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/**
	 * Gets the grid position data for a specific starting position.
	 * @param Index The grid position index (0 = pole position)
	 * @return Grid position data including world location and rotation
	 */
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGGridPosition GetGridPosition(int32 Index) const;

	/**
	 * Gets sector data by index.
	 * @param Index The sector index (typically 0-2)
	 * @return Sector data including timing targets
	 */
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGTrackSector GetSector(int32 Index) const;

	/**
	 * Determines which medal a lap time earns.
	 * @param LapTime The lap time in seconds to evaluate
	 * @return Medal name: "Gold", "Silver", "Bronze", or "None"
	 */
	UFUNCTION(BlueprintPure, Category = "Track")
	FName GetMedalForLapTime(float LapTime) const;

	/**
	 * Converts a world position to minimap texture UV coordinates.
	 * @param WorldPosition 3D world position to convert
	 * @return 2D UV coordinates for minimap display (0-1 range)
	 */
	UFUNCTION(BlueprintPure, Category = "Track")
	FVector2D WorldToMinimapUV(FVector WorldPosition) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

/**
 * Track Collection Data Asset
 *
 * Groups multiple tracks together for championships, themed series,
 * or location-based collections. Used for career mode progression
 * and tournament organization.
 *
 * @section championship_track Championship Mode
 * When bIsChampionship is true, configure PointsPerPosition to define
 * the points awarded for each finishing position. Points accumulate
 * across all tracks in the collection to determine the champion.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackCollectionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/// Unique identifier for this collection (e.g., "CHAMP_Street_Series")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FName CollectionID;

	/// Display name for the collection (e.g., "Street Racing Championship")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText CollectionName;

	/// Description of the collection theme or challenge
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText Description;

	/// Ordered list of tracks in this collection (soft references for async loading)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	TArray<TSoftObjectPtr<UMGTrackDataAsset>> Tracks;

	/// Preview image for collection selection UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	UTexture2D* Thumbnail;

	/// True if this collection uses championship scoring across races
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	bool bIsChampionship = false;

	/// Points awarded for each finishing position (index 0 = 1st place points)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	TArray<int32> PointsPerPosition;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

/**
 * Track Surface Data Asset
 *
 * Defines physics properties for different road surface types.
 * Each surface type affects grip, drag, tire wear, and visual effects.
 * Create one asset per surface type used in your tracks.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackSurfaceAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/// The surface type this asset configures
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	EMGTrackSurface SurfaceType = EMGTrackSurface::Asphalt;

	/// Tire grip multiplier (1.0 = normal, <1.0 = less grip, >1.0 = more grip)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float GripMultiplier = 1.0f;

	/// Additional drag coefficient applied on this surface (0 = no extra drag)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float DragCoefficient = 0.0f;

	/// Rolling resistance factor affecting deceleration (higher = slower)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float RollingResistance = 0.01f;

	/// Tire wear rate multiplier (1.0 = normal, >1.0 = faster wear)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float TireWearMultiplier = 1.0f;

	/// Visual particle intensity for dust/spray effects (0 = none)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float ParticleIntensity = 0.0f;

	/// Tire sound pitch modifier for surface-specific audio (1.0 = normal)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float SoundPitchMultiplier = 1.0f;

	/// Physics material asset defining collision properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	UPhysicalMaterial* PhysicalMaterial;
};

/**
 * Track Weather Preset
 *
 * Defines weather and time-of-day conditions for track variations.
 * Weather affects visibility, grip, and visual atmosphere.
 * Apply different presets to create day/night or weather variants
 * of existing tracks.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackWeatherPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	/// Unique identifier for this weather preset (e.g., "WEATHER_RainyNight")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FName PresetID;

	/// Localized name shown in track selection (e.g., "Rainy Night")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FText DisplayName;

	/// Time of day in 24-hour format (0.0-24.0, e.g., 14.0 = 2:00 PM)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float TimeOfDay = 14.0f;

	/// Grip reduction from wet conditions (1.0 = dry, <1.0 = reduced grip)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float GripMultiplier = 1.0f;

	/// Maximum visible distance in centimeters (fog/rain limitation)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float VisibilityDistance = 10000.0f;

	/// Rain particle intensity (0.0 = clear, 1.0 = heavy rain)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float RainIntensity = 0.0f;

	/// Atmospheric fog density (0.0 = clear, higher = thicker fog)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float FogDensity = 0.0f;

	/// Sun/moon brightness multiplier (0.0 = dark, 1.0 = full brightness)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float SunIntensity = 1.0f;

	/// Color tint applied to ambient lighting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FLinearColor AmbientColor = FLinearColor::White;

	/// True if this is a night-time preset (affects lighting system)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	bool bIsNight = false;

	/// True to enable vehicle headlights automatically
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	bool bEnableHeadlights = false;
};
