// Copyright Midnight Grind. All Rights Reserved.

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

/**
 * Track difficulty rating
 */
UENUM(BlueprintType)
enum class EMGTrackDifficulty : uint8
{
	Easy,
	Medium,
	Hard,
	Expert,
	Legendary
};

/**
 * Track surface type
 */
UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt,
	Concrete,
	Gravel,
	Dirt,
	Sand,
	Wet,
	Ice,
	Mixed
};

/**
 * Track environment type
 */
UENUM(BlueprintType)
enum class EMGTrackEnvironment : uint8
{
	City,
	Highway,
	Mountain,
	Coast,
	Industrial,
	Suburbs,
	Downtown,
	Port
};

/**
 * Sector definition for track timing
 */
USTRUCT(BlueprintType)
struct FMGTrackSector
{
	GENERATED_BODY()

	/** Sector index (0-2 typically) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SectorIndex = 0;

	/** Sector display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SectorName;

	/** Start checkpoint index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartCheckpointIndex = 0;

	/** End checkpoint index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndCheckpointIndex = 0;

	/** Sector length in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Length = 0.0f;

	/** Target time for gold medal (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldTime = 30.0f;

	/** Target time for silver medal (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverTime = 35.0f;

	/** Target time for bronze medal (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeTime = 40.0f;
};

/**
 * Track shortcut definition
 */
USTRUCT(BlueprintType)
struct FMGTrackShortcut
{
	GENERATED_BODY()

	/** Shortcut identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ShortcutID;

	/** Is shortcut enabled by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabledByDefault = true;

	/** Entry position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EntryPosition = FVector::ZeroVector;

	/** Exit position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ExitPosition = FVector::ZeroVector;

	/** Approximate time saved (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeSaved = 2.0f;

	/** Skill level required (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifficultyRating = 0.5f;

	/** Risk level (higher = more dangerous) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RiskLevel = 0.5f;
};

/**
 * Hazard zone definition
 */
USTRUCT(BlueprintType)
struct FMGTrackHazard
{
	GENERATED_BODY()

	/** Hazard type name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HazardType;

	/** Center position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** Hazard radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	/** Speed reduction in zone (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedReduction = 0.2f;

	/** Grip reduction in zone (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GripReduction = 0.3f;

	/** Warning distance before hazard */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WarningDistance = 200.0f;
};

/**
 * Starting grid position
 */
USTRUCT(BlueprintType)
struct FMGGridPosition
{
	GENERATED_BODY()

	/** Grid position index (0 = pole) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridIndex = 0;

	/** World position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** Starting rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * Track leaderboard record
 */
USTRUCT(BlueprintType)
struct FMGTrackRecord
{
	GENERATED_BODY()

	/** Player name/ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	/** Vehicle used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/** Lap time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	/** Total race time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	/** Date achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DateSet;

	/** Was this a clean lap (no penalties) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCleanLap = false;
};

/**
 * Track Data Asset
 * Master configuration for a race track
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTIFICATION
	// ==========================================

	/** Unique track identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	FName TrackID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	FText TrackName;

	/** Track description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity", meta = (MultiLine = true))
	FText Description;

	/** Track location/city name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	FText LocationName;

	/** Track thumbnail */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	UTexture2D* Thumbnail;

	/** Loading screen image */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Identity")
	UTexture2D* LoadingImage;

	// ==========================================
	// SPECIFICATIONS
	// ==========================================

	/** Track length in meters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	float TrackLength = 5000.0f;

	/** Number of laps for standard race */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	int32 DefaultLapCount = 3;

	/** Maximum number of racers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	int32 MaxRacers = 8;

	/** Difficulty rating */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	EMGTrackDifficulty Difficulty = EMGTrackDifficulty::Medium;

	/** Primary surface type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	EMGTrackSurface PrimarySurface = EMGTrackSurface::Asphalt;

	/** Environment type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	EMGTrackEnvironment Environment = EMGTrackEnvironment::City;

	/** Is this a circuit (closed loop) or point-to-point */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	bool bIsCircuit = true;

	/** Number of checkpoints */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Specs")
	int32 CheckpointCount = 10;

	// ==========================================
	// TIMING TARGETS
	// ==========================================

	/** Par lap time (seconds) - average expected time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float ParLapTime = 90.0f;

	/** Gold medal lap time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float GoldLapTime = 80.0f;

	/** Silver medal lap time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float SilverLapTime = 85.0f;

	/** Bronze medal lap time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	float BronzeLapTime = 95.0f;

	/** Track record (server-synced) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Times")
	FMGTrackRecord TrackRecord;

	// ==========================================
	// SECTORS
	// ==========================================

	/** Sector definitions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Sectors")
	TArray<FMGTrackSector> Sectors;

	// ==========================================
	// LAYOUT
	// ==========================================

	/** Starting grid positions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Layout")
	TArray<FMGGridPosition> GridPositions;

	/** Shortcut definitions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Layout")
	TArray<FMGTrackShortcut> Shortcuts;

	/** Hazard zones */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Layout")
	TArray<FMGTrackHazard> Hazards;

	// ==========================================
	// MINIMAP
	// ==========================================

	/** Minimap texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Minimap")
	UTexture2D* MinimapTexture;

	/** World bounds for minimap mapping */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Minimap")
	FBox2D WorldBounds;

	/** Minimap rotation offset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Minimap")
	float MinimapRotation = 0.0f;

	// ==========================================
	// AUDIO
	// ==========================================

	/** Ambient sound for track */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Audio")
	USoundBase* AmbientSound;

	/** Music playlist ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Audio")
	FName MusicPlaylistID;

	// ==========================================
	// LEVEL REFERENCES
	// ==========================================

	/** Main level to load */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Level")
	TSoftObjectPtr<UWorld> MainLevel;

	/** Streaming sub-levels */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Level")
	TArray<TSoftObjectPtr<UWorld>> StreamingLevels;

	// ==========================================
	// UNLOCK
	// ==========================================

	/** Is track available by default */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	bool bUnlockedByDefault = true;

	/** Required player level to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	int32 RequiredLevel = 1;

	/** Required reputation to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	int32 RequiredReputation = 0;

	/** Price to unlock (if purchasable) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Track|Unlock")
	int32 UnlockPrice = 0;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/** Get grid position for index */
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGGridPosition GetGridPosition(int32 Index) const;

	/** Get sector by index */
	UFUNCTION(BlueprintPure, Category = "Track")
	FMGTrackSector GetSector(int32 Index) const;

	/** Get medal for lap time */
	UFUNCTION(BlueprintPure, Category = "Track")
	FName GetMedalForLapTime(float LapTime) const;

	/** Convert world position to minimap UV */
	UFUNCTION(BlueprintPure, Category = "Track")
	FVector2D WorldToMinimapUV(FVector WorldPosition) const;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

/**
 * Track Collection Data Asset
 * Groups tracks for a championship or location
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackCollectionAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Collection identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FName CollectionID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText CollectionName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	FText Description;

	/** Tracks in collection */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	TArray<TSoftObjectPtr<UMGTrackDataAsset>> Tracks;

	/** Collection thumbnail */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	UTexture2D* Thumbnail;

	/** Is this a championship series */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	bool bIsChampionship = false;

	/** Points awarded per position (if championship) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collection")
	TArray<int32> PointsPerPosition;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

/**
 * Track Surface Data Asset
 * Physics properties for different surfaces
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackSurfaceAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Surface type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	EMGTrackSurface SurfaceType = EMGTrackSurface::Asphalt;

	/** Grip multiplier (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float GripMultiplier = 1.0f;

	/** Drag coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float DragCoefficient = 0.0f;

	/** Rolling resistance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float RollingResistance = 0.01f;

	/** Tire wear multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float TireWearMultiplier = 1.0f;

	/** Dust/spray particle intensity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float ParticleIntensity = 0.0f;

	/** Sound attenuation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	float SoundPitchMultiplier = 1.0f;

	/** Physical material for this surface */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Surface")
	UPhysicalMaterial* PhysicalMaterial;
};

/**
 * Track Weather Preset
 * Weather conditions for track variations
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGTrackWeatherPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Weather preset name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FName PresetID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FText DisplayName;

	/** Time of day (0-24) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float TimeOfDay = 14.0f;

	/** Grip multiplier for wet conditions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float GripMultiplier = 1.0f;

	/** Visibility distance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float VisibilityDistance = 10000.0f;

	/** Rain intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float RainIntensity = 0.0f;

	/** Fog density */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float FogDensity = 0.0f;

	/** Sun intensity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	float SunIntensity = 1.0f;

	/** Ambient color tint */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	FLinearColor AmbientColor = FLinearColor::White;

	/** Is this night racing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	bool bIsNight = false;

	/** Enable headlights */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
	bool bEnableHeadlights = false;
};
