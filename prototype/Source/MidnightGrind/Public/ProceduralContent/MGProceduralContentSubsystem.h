// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGProceduralContentSubsystem.h
 * @brief Procedural Content Generation System for Midnight Grind
 *
 * This subsystem is the central hub for all procedural content generation in
 * Midnight Grind. It orchestrates the creation of tracks, environments, challenges,
 * obstacles, shortcuts, weather effects, traffic patterns, and collectibles.
 *
 * The system uses a hierarchical seed architecture where a master seed generates
 * sub-seeds for each content type, ensuring reproducible generation while allowing
 * independent variation within each category.
 *
 * Key Features:
 * - Seed-based deterministic generation for shareable content
 * - Multi-layered track generation with segments and checkpoints
 * - Dynamic environment theming with Y2K aesthetic elements
 * - Procedural challenge and objective generation
 * - Traffic pattern simulation
 * - Collectible placement with rarity tiers
 *
 * Architecture Overview:
 * 1. Master Seed -> Sub-seeds for each content type
 * 2. Track Generation -> Segment sequence -> Environment overlay
 * 3. Challenge Generation -> Objectives based on track characteristics
 * 4. Collectible/Shortcut placement -> Based on track geometry
 *
 * Usage Example:
 * @code
 *   auto* PCG = GameInstance->GetSubsystem<UMGProceduralContentSubsystem>();
 *   FGenerationSettings Settings;
 *   Settings.PreferredTheme = EEnvironmentTheme::NeonAlley;
 *   Settings.TargetDifficulty = EGenerationDifficulty::Hard;
 *   FGenerationResult Result = PCG->GenerateTrack(Settings);
 * @endcode
 *
 * @see UMGRouteGeneratorSubsystem for low-level route geometry generation
 * @see FProceduralTrack for complete track data structure
 */

// MGProceduralContentSubsystem.h
// Procedural Content Generation System - Generates dynamic tracks, environments, and challenges
// Midnight Grind - Y2K Arcade Street Racing

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGProceduralContentSubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// Content Type Enumeration
// ============================================================================

/**
 * Types of procedurally generated content.
 * Used to categorize and track different content generation operations.
 */
UENUM(BlueprintType)
enum class EProceduralContentType : uint8
{
    Track,           /// Complete racing track with segments
    Environment,     /// Visual environment and atmosphere
    Challenge,       /// Race objectives and goals
    Obstacle,        /// Road hazards and barriers
    Shortcut,        /// Hidden alternate paths
    SecretArea,      /// Discoverable hidden locations
    Weather,         /// Dynamic weather conditions
    TrafficPattern,  /// AI traffic vehicle patterns
    EventSequence,   /// Scripted event sequences
    Collectible      /// Pickup items and bonuses
};

// ============================================================================
// Track Segment Type Enumeration
// ============================================================================

/**
 * Track segment geometry types.
 * Each segment type defines the road shape and driving characteristics
 * for a portion of the generated track.
 */
UENUM(BlueprintType)
enum class ETrackSegmentType : uint8
{
    Straight,         /// Long straight section for high speed
    GentleCurve,      /// Wide radius curve, easy to navigate
    SharpCurve,       /// Tight curve requiring braking
    Hairpin,          /// 180-degree switchback turn
    SweepingS,        /// Long S-curve sequence
    Chicane,          /// Quick direction changes
    Intersection,     /// Cross-traffic junction
    Tunnel,           /// Enclosed road section
    Bridge,           /// Elevated crossing
    Overpass,         /// Road passing over another
    Underpass,        /// Road passing under structure
    Jump,             /// Ramp launching vehicles airborne
    DriftZone,        /// Section optimized for drifting
    SpeedZone,        /// Speed trap/bonus section
    TechnicalSection  /// Complex multi-turn section
};

// ============================================================================
// Environment Theme Enumeration
// ============================================================================

/**
 * Visual environment themes for track generation.
 * Each theme influences lighting, props, colors, and atmosphere
 * to create distinct racing locations with Y2K aesthetic.
 */
UENUM(BlueprintType)
enum class EEnvironmentTheme : uint8
{
    UrbanDowntown,        /// Dense city center with skyscrapers
    IndustrialDistrict,   /// Warehouses, factories, shipping containers
    WaterfrontDocks,      /// Harbor area with cranes and boats
    SuburbanNeighborhood, /// Residential streets and strip malls
    HighwayOverpass,      /// Multi-level highway interchange
    ShoppingDistrict,     /// Retail area with storefronts
    ParkingStructure,     /// Multi-story parking garage
    AirportRunway,        /// Aircraft taxiways and terminals
    MountainPass,         /// Winding mountain roads
    CoastalHighway,       /// Oceanside cliffside roads
    NeonAlley,            /// Dark alleys lit by neon signs
    CyberpunkSlums,       /// Dystopian urban decay
    RetroArcade,          /// Classic arcade aesthetic
    Y2KMall              /// Late 90s shopping mall interior
};

// ============================================================================
// Generation Difficulty Enumeration
// ============================================================================

/**
 * Difficulty levels for procedural generation.
 * Affects track complexity, hazard frequency, time limits,
 * and overall challenge of generated content.
 */
UENUM(BlueprintType)
enum class EGenerationDifficulty : uint8
{
    VeryEasy,   /// Tutorial-level, very forgiving
    Easy,       /// Casual play, few hazards
    Medium,     /// Balanced challenge (default)
    Hard,       /// Significant challenge
    VeryHard,   /// Expert-level difficulty
    Extreme,    /// Near-impossible challenge
    Nightmare   /// Maximum difficulty for masochists
};

// ============================================================================
// Generation Quality Enumeration
// ============================================================================

/**
 * Quality settings for procedural generation.
 * Higher quality produces more detailed content but takes longer to generate.
 * Affects segment count, decoration density, and geometric complexity.
 */
UENUM(BlueprintType)
enum class EProceduralQuality : uint8
{
    Draft,  /// Quick generation, minimal detail
    Low,    /// Reduced detail for performance
    Medium, /// Balanced quality/performance
    High,   /// High detail, longer generation
    Ultra   /// Maximum detail, slowest generation
};

// ============================================================================
// Obstacle Category Enumeration
// ============================================================================

/**
 * Categories of track obstacles.
 * Determines obstacle behavior and interaction with vehicles.
 */
UENUM(BlueprintType)
enum class EObstacleCategory : uint8
{
    Static,        /// Immovable barriers and walls
    Dynamic,       /// Moving obstacles (traffic, gates)
    Destructible,  /// Breakable objects (crates, fences)
    Hazardous,     /// Damaging obstacles (fire, spikes)
    Interactive,   /// Triggers and switches
    Decorative     /// Visual-only, no collision
};

// ============================================================================
// Weather Intensity Enumeration
// ============================================================================

/**
 * Weather effect intensity levels.
 * Controls visual effects and handling impact of weather.
 */
UENUM(BlueprintType)
enum class EWeatherIntensity : uint8
{
    None,     /// Clear conditions
    Light,    /// Minimal weather effects
    Moderate, /// Noticeable weather impact
    Heavy,    /// Significant visibility/grip reduction
    Extreme   /// Severe conditions, major handling changes
};

// ============================================================================
// Time of Day Enumeration
// ============================================================================

/**
 * Time of day settings for environment lighting.
 * Affects sun position, ambient light, and atmosphere.
 */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,      /// Early morning, rising sun
    Morning,   /// Full daylight, low sun
    Noon,      /// Midday, overhead sun
    Afternoon, /// Late day, warm light
    Dusk,      /// Sunset, orange/pink sky
    Evening,   /// Post-sunset, fading light
    Night,     /// Dark with artificial lights
    Midnight   /// Darkest night, minimal ambient
};

// ============================================================================
// Procedural Seed Structure
// ============================================================================

/**
 * Hierarchical seed structure for procedural generation.
 * A master seed generates sub-seeds for each content type,
 * enabling reproducible generation while allowing independent variation.
 */
USTRUCT(BlueprintType)
struct FProceduralSeed
{
    GENERATED_BODY()

    /// Primary seed that derives all sub-seeds
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MasterSeed = 0;

    /// Seed for track layout generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TrackSeed = 0;

    /// Seed for environment decoration placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EnvironmentSeed = 0;

    /// Seed for obstacle placement
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ObstacleSeed = 0;

    /// Seed for weather variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeatherSeed = 0;

    /// Seed for traffic pattern generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TrafficSeed = 0;

    /// Human-readable seed code for sharing
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SeedCode;

    /// Timestamp when this seed was created
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime GeneratedAt;
};

// ============================================================================
// Track Segment Structure
// ============================================================================

/**
 * Individual track segment data.
 * Segments are the building blocks of procedural tracks,
 * each defining a continuous piece of road with consistent characteristics.
 */
USTRUCT(BlueprintType)
struct FTrackSegment
{
    GENERATED_BODY()

    /// Unique identifier for this segment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SegmentId;

    /// Geometric type of this segment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETrackSegmentType SegmentType = ETrackSegmentType::Straight;

    /// World position where segment begins
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartPosition = FVector::ZeroVector;

    /// World position where segment ends
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EndPosition = FVector::ZeroVector;

    /// Rotation at segment start
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator StartRotation = FRotator::ZeroRotator;

    /// Rotation at segment end
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator EndRotation = FRotator::ZeroRotator;

    /// Length of segment in world units
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Length = 100.0f;

    /// Road width in world units
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width = 12.0f;

    /// Banking angle for curves (degrees, positive = banked inward)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BankAngle = 0.0f;

    /// Height change from start to end
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ElevationChange = 0.0f;

    /// Curve radius for non-straight segments
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurveRadius = 0.0f;

    /// Recommended maximum speed (km/h)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedLimit = 200.0f;

    /// Difficulty contribution (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DifficultyRating = 1.0f;

    /// Surface grip multiplier (1.0 = normal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GripMultiplier = 1.0f;

    /// Whether barriers line this segment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasBarriers = true;

    /// Whether street lights are present
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasStreetLights = true;

    /// Bezier curve control points for complex geometry
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ControlPoints;

    /// IDs of segments this connects to
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> ConnectedSegments;

    /// Custom surface properties (friction, roughness, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> SurfaceProperties;
};

// ============================================================================
// Procedural Track Structure
// ============================================================================

/**
 * Complete procedurally generated track.
 * Contains all segments, metadata, and computed statistics
 * for a fully playable racing track.
 */
USTRUCT(BlueprintType)
struct FProceduralTrack
{
    GENERATED_BODY()

    /// Unique identifier for this track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid TrackId;

    /// Display name for the track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TrackName;

    /// Seed used to generate this track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProceduralSeed Seed;

    /// Visual theme of the track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentTheme Theme = EEnvironmentTheme::UrbanDowntown;

    /// Overall difficulty level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGenerationDifficulty Difficulty = EGenerationDifficulty::Medium;

    /// All segments making up the track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTrackSegment> Segments;

    /// Total track length in world units
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalLength = 0.0f;

    /// Number of laps for circuit races
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LapCount = 3;

    /// Whether track is a closed loop
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCircuit = true;

    /// Whether multiple route choices exist
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAlternateRoutes = false;

    /// Number of alternate route branches
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AlternateRouteCount = 0;

    /// Number of shortcut paths
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShortcutCount = 0;

    /// Number of jump ramps
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 JumpCount = 0;

    /// Number of designated drift zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DriftZoneCount = 0;

    /// Expected lap time in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedLapTime = 120.0f;

    /// Computed difficulty score (0-100)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DifficultyScore = 50.0f;

    /// Center point of track bounding box
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TrackCenter = FVector::ZeroVector;

    /// Minimum corner of bounding box
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TrackBoundsMin = FVector::ZeroVector;

    /// Maximum corner of bounding box
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TrackBoundsMax = FVector::ZeroVector;

    /// Checkpoint gate positions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> CheckpointPositions;

    /// Starting grid spawn positions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SpawnPositions;

    /// When this track was generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime GeneratedAt;
};

// ============================================================================
// Procedural Obstacle Structure
// ============================================================================

/**
 * Procedurally placed obstacle data.
 * Defines hazards, barriers, and interactive objects on the track.
 */
USTRUCT(BlueprintType)
struct FProceduralObstacle
{
    GENERATED_BODY()

    /// Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ObstacleId;

    /// Type name for mesh/behavior lookup
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObstacleType;

    /// Behavioral category
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObstacleCategory Category = EObstacleCategory::Static;

    /// World position
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    /// World rotation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    /// Scale multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale = FVector::OneVector;

    /// Collision detection radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CollisionRadius = 50.0f;

    /// Damage dealt on collision
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageOnImpact = 10.0f;

    /// Speed reduction on collision (multiplier)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedPenalty = 0.5f;

    /// Whether obstacle can be destroyed
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDestructible = false;

    /// Health for destructible obstacles
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HealthPoints = 100;

    /// Visual mesh asset reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> ObstacleMesh;

    /// Material override
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> ObstacleMaterial;

    /// Custom gameplay properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> Properties;
};

// ============================================================================
// Procedural Shortcut Structure
// ============================================================================

/**
 * Shortcut path data.
 * Defines hidden alternate routes that save time but carry risk.
 */
USTRUCT(BlueprintType)
struct FProceduralShortcut
{
    GENERATED_BODY()

    /// Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ShortcutId;

    /// Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ShortcutName;

    /// Where shortcut begins
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EntryPoint = FVector::ZeroVector;

    /// Where shortcut rejoins main track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ExitPoint = FVector::ZeroVector;

    /// Estimated time savings (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSaved = 2.0f;

    /// Danger level (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RiskLevel = 0.5f;

    /// Minimum speed to enter successfully
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinimumSpeedRequired = 100.0f;

    /// Whether a jump is required
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresJump = false;

    /// Whether drifting is required
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresDrift = false;

    /// Whether shortcut is visually hidden
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHidden = false;

    /// Points awarded for first discovery
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DiscoveryPoints = 100;

    /// Waypoints along shortcut path
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;

    /// Obstacles within shortcut path
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> ObstacleIds;
};

// ============================================================================
// Procedural Environment Structure
// ============================================================================

/**
 * Environment configuration data.
 * Defines lighting, weather, atmosphere, and decorations for a track.
 */
USTRUCT(BlueprintType)
struct FProceduralEnvironment
{
    GENERATED_BODY()

    /// Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid EnvironmentId;

    /// Visual theme
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentTheme Theme = EEnvironmentTheme::UrbanDowntown;

    /// Lighting time setting
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Night;

    /// Weather severity
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeatherIntensity WeatherIntensity = EWeatherIntensity::None;

    /// Weather type name (Rain, Snow, Fog, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WeatherType;

    /// Ambient light color
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientLightColor = FLinearColor::White;

    /// Ambient light brightness
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLightIntensity = 1.0f;

    /// Atmospheric fog color
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor::Gray;

    /// Fog density (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.01f;

    /// Maximum view distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VisibilityDistance = 10000.0f;

    /// Surface grip modifier from conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GripModifier = 1.0f;

    /// Traffic vehicle density (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrafficDensity = 0.5f;

    /// Pedestrian NPC density (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PedestrianDensity = 0.3f;

    /// Placed obstacles in environment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralObstacle> Obstacles;

    /// Street light positions
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> LightPositions;

    /// Y2K neon color palette
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLinearColor> NeonColors;

    /// Custom environment parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> EnvironmentParams;
};

// ============================================================================
// Procedural Challenge Structure
// ============================================================================

/**
 * Generated challenge/objective data.
 * Defines goals, requirements, and rewards for procedural challenges.
 */
USTRUCT(BlueprintType)
struct FProceduralChallenge
{
    GENERATED_BODY()

    /// Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ChallengeId;

    /// Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChallengeName;

    /// Detailed description
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChallengeDescription;

    /// Challenge type name (TimeAttack, Drift, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChallengeType;

    /// Difficulty level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGenerationDifficulty Difficulty = EGenerationDifficulty::Medium;

    /// Target value to achieve (score, speed, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue = 0.0f;

    /// Time limit in seconds (0 = no limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit = 120.0f;

    /// Currency reward on completion
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardCredits = 500;

    /// Experience points reward
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardXP = 100;

    /// Item rewards (part IDs, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RewardItems;

    /// Track this challenge uses
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RequiredTrackId;

    /// Allowed vehicle classes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredVehicleClasses;

    /// Individual objective targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> Objectives;

    /// Gameplay modifiers (NoNitro, ReverseTrack, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> Modifiers;
};

// ============================================================================
// Traffic Pattern Structure
// ============================================================================

/**
 * Traffic simulation pattern data.
 * Defines AI traffic vehicle spawning and behavior.
 */
USTRUCT(BlueprintType)
struct FTrafficPattern
{
    GENERATED_BODY()

    /// Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid PatternId;

    /// Pattern name for identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PatternName;

    /// Traffic density (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density = 0.5f;

    /// Base speed of traffic (km/h)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageSpeed = 60.0f;

    /// Speed variance (+/- km/h)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedVariation = 20.0f;

    /// How reactive/dangerous traffic is (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressivenessLevel = 0.3f;

    /// Vehicle type names to spawn
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> VehicleTypes;

    /// Spawn point locations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SpawnPoints;

    /// Despawn trigger locations
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DespawnPoints;

    /// Distribution across lanes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> LaneDistribution;
};

// ============================================================================
// Procedural Collectible Structure
// ============================================================================

/**
 * Collectible item placement data.
 * Defines pickup items placed along the track.
 */
USTRUCT(BlueprintType)
struct FProceduralCollectible
{
    GENERATED_BODY()

    /// Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid CollectibleId;

    /// Type name (NitroBoost, Coin, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CollectibleType;

    /// World position
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    /// Point value when collected
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointValue = 10;

    /// Whether this is a rare/special collectible
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRare = false;

    /// Whether collectible is visually hidden
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden = false;

    /// Time until respawn after collection (0 = no respawn)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnTime = 30.0f;

    /// Visual mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> CollectibleMesh;

    /// Glow effect color
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor GlowColor = FLinearColor::Yellow;
};

// ============================================================================
// Generation Settings Structure
// ============================================================================

/**
 * Configuration settings for content generation.
 * Customize these parameters to control generated content characteristics.
 */
USTRUCT(BlueprintType)
struct FGenerationSettings
{
    GENERATED_BODY()

    /// Output quality level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EProceduralQuality Quality = EProceduralQuality::Medium;

    /// Target difficulty
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGenerationDifficulty TargetDifficulty = EGenerationDifficulty::Medium;

    /// Desired visual theme
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentTheme PreferredTheme = EEnvironmentTheme::UrbanDowntown;

    /// Minimum track length (world units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinTrackLength = 2000.0f;

    /// Maximum track length (world units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTrackLength = 10000.0f;

    /// Minimum segment count
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinSegments = 10;

    /// Maximum segment count
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSegments = 50;

    /// How often curves appear (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurveFrequency = 0.4f;

    /// How often jumps appear (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpFrequency = 0.1f;

    /// How often shortcuts appear (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShortcutFrequency = 0.15f;

    /// Obstacle placement density (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ObstacleDensity = 0.5f;

    /// Collectible placement density (0.0-1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CollectibleDensity = 0.3f;

    /// Allow branching paths
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowAlternateRoutes = true;

    /// Generate shortcut paths
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateShortcuts = true;

    /// Generate hidden areas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateSecretAreas = true;

    /// Include AI traffic
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateTraffic = true;

    /// Enable weather changes
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDynamicWeather = false;

    /// Additional custom parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> CustomParameters;
};

// ============================================================================
// Generation Result Structure
// ============================================================================

/**
 * Results from a generation operation.
 * Contains all generated content and metadata about the generation process.
 */
USTRUCT(BlueprintType)
struct FGenerationResult
{
    GENERATED_BODY()

    /// Whether generation succeeded
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess = false;

    /// Error message if generation failed
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ErrorMessage;

    /// The generated track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProceduralTrack GeneratedTrack;

    /// The generated environment
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProceduralEnvironment GeneratedEnvironment;

    /// Generated challenges for this track
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralChallenge> GeneratedChallenges;

    /// Generated shortcuts
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralShortcut> GeneratedShortcuts;

    /// Placed collectibles
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralCollectible> GeneratedCollectibles;

    /// Traffic configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTrafficPattern GeneratedTrafficPattern;

    /// Time taken to generate (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GenerationTime = 0.0f;

    /// Total objects created
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalObjectsGenerated = 0;
};

// ============================================================================
// Procedural Content Statistics Structure
// ============================================================================

/**
 * Statistics tracking for procedural content.
 * Used for analytics and player progression tracking.
 */
USTRUCT(BlueprintType)
struct FProceduralContentStats
{
    GENERATED_BODY()

    /// Total tracks generated by player
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalTracksGenerated = 0;

    /// Total environments generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalEnvironmentsGenerated = 0;

    /// Total challenges generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalChallengesGenerated = 0;

    /// Total shortcuts player has found
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalShortcutsDiscovered = 0;

    /// Total secret areas found
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSecretAreasFound = 0;

    /// Average time per generation (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageGenerationTime = 0.0f;

    /// Total playtime on generated tracks (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPlayTimeOnGenerated = 0.0f;

    /// Number of tracks marked as favorite
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FavoritedTracks = 0;

    /// Number of tracks shared with others
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SharedTracks = 0;

    /// Usage count per theme
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> ThemeUsageCounts;

    /// Distribution of difficulty levels used
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> DifficultyDistribution;
};

// ============================================================================
// Delegate Declarations
// ============================================================================

/// Broadcast when a track finishes generating
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackGenerated, const FProceduralTrack&, Track);

/// Broadcast when environment generation completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentGenerated, const FProceduralEnvironment&, Environment);

/// Broadcast when a challenge is generated
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeGenerated, const FProceduralChallenge&, Challenge);

/// Broadcast during generation with progress updates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGenerationProgress, float, Progress, const FString&, CurrentStep);

/// Broadcast when all generation completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationComplete, const FGenerationResult&, Result);

/// Broadcast when generation fails
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationFailed, const FString&, ErrorMessage);

/// Broadcast when player discovers a shortcut
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShortcutDiscovered, const FProceduralShortcut&, Shortcut);

/// Broadcast when a seed code is shared
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeedShared, const FString&, SeedCode, const FString&, TrackName);

// ============================================================================
// Procedural Content Subsystem Class
// ============================================================================

/**
 * Procedural Content Generation Subsystem
 *
 * Central hub for all procedural content generation in Midnight Grind.
 * Handles track, environment, challenge, and collectible generation with
 * seed-based determinism for reproducible and shareable content.
 *
 * The subsystem maintains generation quality settings, tracks player statistics,
 * and manages the storage of favorited and shared tracks.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGProceduralContentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    // ========================================================================
    // Seed Management
    // ========================================================================

    /**
     * Generates a new random seed structure.
     * @return A complete seed with master and derived sub-seeds
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Seed")
    FProceduralSeed GenerateRandomSeed();

    /**
     * Creates a seed structure from a share code.
     * @param SeedCode Human-readable seed code string
     * @return Decoded seed structure
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Seed")
    FProceduralSeed CreateSeedFromCode(const FString& SeedCode);

    /**
     * Converts a seed to a shareable code string.
     * @param Seed The seed to encode
     * @return Human-readable share code
     */
    UFUNCTION(BlueprintPure, Category = "Procedural|Seed")
    FString GetSeedCode(const FProceduralSeed& Seed) const;

    /**
     * Sets the master seed for subsequent generation.
     * @param NewSeed The new master seed value
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Seed")
    void SetMasterSeed(int64 NewSeed);

    /** Returns the current master seed value. */
    UFUNCTION(BlueprintPure, Category = "Procedural|Seed")
    int64 GetCurrentMasterSeed() const { return CurrentMasterSeed; }

    // ========================================================================
    // Track Generation
    // ========================================================================

    /**
     * Generates a complete track synchronously.
     * @param Settings Configuration for generation
     * @return Generation result with track data
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FGenerationResult GenerateTrack(const FGenerationSettings& Settings);

    /**
     * Generates a track from a specific seed.
     * @param Seed The seed to use for generation
     * @param Settings Additional generation settings
     * @return Generation result with track data
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FGenerationResult GenerateTrackFromSeed(const FProceduralSeed& Seed, const FGenerationSettings& Settings);

    /**
     * Begins asynchronous track generation.
     * Listen to OnGenerationComplete for results.
     * @param Settings Configuration for generation
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    void GenerateTrackAsync(const FGenerationSettings& Settings);

    /**
     * Generates a single track segment.
     * @param SegmentType Type of segment to create
     * @param StartPos Starting position
     * @param StartRot Starting rotation
     * @return The generated segment data
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FTrackSegment GenerateSegment(ETrackSegmentType SegmentType, const FVector& StartPos, const FRotator& StartRot);

    /**
     * Generates a sequence of connected segments.
     * @param SegmentCount Number of segments to generate
     * @param Difficulty Difficulty level to target
     * @return Array of connected segments
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    TArray<FTrackSegment> GenerateSegmentSequence(int32 SegmentCount, EGenerationDifficulty Difficulty);

    /**
     * Validates a track for playability.
     * @param Track The track to validate
     * @return True if track is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    bool ValidateTrack(const FProceduralTrack& Track);

    /**
     * Optimizes track geometry and flow.
     * @param Track The track to optimize
     * @return Optimized track copy
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FProceduralTrack OptimizeTrack(const FProceduralTrack& Track);

    // ========================================================================
    // Environment Generation
    // ========================================================================

    /**
     * Generates environment data for a track.
     * @param Theme Visual theme to use
     * @param ForTrack Track to generate environment for
     * @return Generated environment configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    FProceduralEnvironment GenerateEnvironment(EEnvironmentTheme Theme, const FProceduralTrack& ForTrack);

    /**
     * Generates obstacles along a track.
     * @param Track The track to populate
     * @param Density Obstacle density (0.0-1.0)
     * @return Array of placed obstacles
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    TArray<FProceduralObstacle> GenerateObstacles(const FProceduralTrack& Track, float Density);

    /**
     * Generates street light positions.
     * @param Track The track to light
     * @param Theme Theme for light placement style
     * @return Array of light positions
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    TArray<FVector> GenerateLightPositions(const FProceduralTrack& Track, EEnvironmentTheme Theme);

    /**
     * Generates a neon color palette for the theme.
     * @param Theme Theme to generate colors for
     * @param ColorCount Number of colors to generate
     * @return Array of Y2K-style neon colors
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    TArray<FLinearColor> GenerateNeonPalette(EEnvironmentTheme Theme, int32 ColorCount);

    /**
     * Applies weather effects to an environment.
     * @param Environment Environment to modify
     * @param WeatherType Type of weather (Rain, Snow, etc.)
     * @param Intensity Weather severity
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    void ApplyWeatherToEnvironment(FProceduralEnvironment& Environment, const FString& WeatherType, EWeatherIntensity Intensity);

    // ========================================================================
    // Challenge Generation
    // ========================================================================

    /**
     * Generates a challenge for a track.
     * @param Track Track to create challenge for
     * @param Difficulty Target difficulty
     * @return Generated challenge data
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    FProceduralChallenge GenerateChallenge(const FProceduralTrack& Track, EGenerationDifficulty Difficulty);

    /**
     * Generates multiple challenges for a track.
     * @param Track Track to create challenges for
     * @param ChallengeCount Number to generate
     * @return Array of generated challenges
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    TArray<FProceduralChallenge> GenerateChallengeSet(const FProceduralTrack& Track, int32 ChallengeCount);

    /**
     * Generates the daily rotating challenge.
     * @return Today's challenge
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    FProceduralChallenge GenerateDailyChallenge();

    /**
     * Generates the weekly challenge set.
     * @return This week's challenges
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    TArray<FProceduralChallenge> GenerateWeeklyChallenges();

    // ========================================================================
    // Shortcuts and Secrets
    // ========================================================================

    /**
     * Generates shortcuts for a track.
     * @param Track Track to add shortcuts to
     * @param MaxShortcuts Maximum number to generate
     * @return Array of generated shortcuts
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Shortcuts")
    TArray<FProceduralShortcut> GenerateShortcuts(const FProceduralTrack& Track, int32 MaxShortcuts);

    /**
     * Validates a shortcut is usable.
     * @param Shortcut Shortcut to validate
     * @param Track Track the shortcut belongs to
     * @return True if shortcut is valid
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Shortcuts")
    bool IsValidShortcut(const FProceduralShortcut& Shortcut, const FProceduralTrack& Track);

    /**
     * Marks a shortcut as discovered by the player.
     * @param ShortcutId ID of discovered shortcut
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Shortcuts")
    void DiscoverShortcut(const FGuid& ShortcutId);

    /** Returns all shortcuts the player has discovered. */
    UFUNCTION(BlueprintPure, Category = "Procedural|Shortcuts")
    TArray<FProceduralShortcut> GetDiscoveredShortcuts() const;

    // ========================================================================
    // Traffic Generation
    // ========================================================================

    /**
     * Generates a traffic pattern for a track.
     * @param Track Track to generate traffic for
     * @param Density Traffic density (0.0-1.0)
     * @return Traffic pattern configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Traffic")
    FTrafficPattern GenerateTrafficPattern(const FProceduralTrack& Track, float Density);

    /**
     * Gets spawn points from a traffic pattern.
     * @param Pattern The traffic pattern
     * @return Array of spawn positions
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Traffic")
    TArray<FVector> GetTrafficSpawnPoints(const FTrafficPattern& Pattern) const;

    /**
     * Updates a traffic pattern over time.
     * @param Pattern Pattern to update
     * @param DeltaTime Time since last update
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Traffic")
    void UpdateTrafficPattern(FTrafficPattern& Pattern, float DeltaTime);

    // ========================================================================
    // Collectibles
    // ========================================================================

    /**
     * Generates collectibles along a track.
     * @param Track Track to populate
     * @param Density Collectible density (0.0-1.0)
     * @return Array of placed collectibles
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Collectibles")
    TArray<FProceduralCollectible> GenerateCollectibles(const FProceduralTrack& Track, float Density);

    /**
     * Generates hidden collectibles.
     * @param Track Track to hide collectibles in
     * @param Count Number to generate
     * @return Array of hidden collectibles
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Collectibles")
    TArray<FProceduralCollectible> GenerateHiddenCollectibles(const FProceduralTrack& Track, int32 Count);

    /**
     * Marks a collectible as collected.
     * @param CollectibleId ID of collected item
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Collectibles")
    void CollectItem(const FGuid& CollectibleId);

    // ========================================================================
    // Content Storage
    // ========================================================================

    /**
     * Saves a generated track to storage.
     * @param Track Track to save
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    void SaveGeneratedTrack(const FProceduralTrack& Track);

    /**
     * Loads a track by its seed code.
     * @param SeedCode Seed code to load
     * @return Loaded track (empty if not found)
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    FProceduralTrack LoadTrackBySeed(const FString& SeedCode);

    /** Returns all saved tracks. */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    TArray<FProceduralTrack> GetSavedTracks() const;

    /**
     * Deletes a saved track.
     * @param TrackId ID of track to delete
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    void DeleteSavedTrack(const FGuid& TrackId);

    /**
     * Marks a track as favorite.
     * @param TrackId ID of track to favorite
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    void FavoriteTrack(const FGuid& TrackId);

    /** Returns all favorited tracks. */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    TArray<FProceduralTrack> GetFavoriteTracks() const;

    // ========================================================================
    // Sharing
    // ========================================================================

    /**
     * Generates a share code for a track.
     * @param TrackId Track to share
     * @return Shareable code string
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Sharing")
    FString ShareTrack(const FGuid& TrackId);

    /**
     * Imports a track from a share code.
     * @param ShareCode Code to import
     * @return Imported track
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Sharing")
    FProceduralTrack ImportSharedTrack(const FString& ShareCode);

    /**
     * Validates a share code format.
     * @param ShareCode Code to validate
     * @return True if code is valid format
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Sharing")
    bool ValidateShareCode(const FString& ShareCode);

    // ========================================================================
    // Statistics
    // ========================================================================

    /** Returns generation statistics. */
    UFUNCTION(BlueprintPure, Category = "Procedural|Stats")
    FProceduralContentStats GetContentStats() const { return ContentStats; }

    /**
     * Records playtime on a generated track.
     * @param TrackId Track being played
     * @param PlaytimeSeconds Time played in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Stats")
    void RecordTrackPlaytime(const FGuid& TrackId, float PlaytimeSeconds);

    /**
     * Increments generation counter for a content type.
     * @param ContentType Type of content generated
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Stats")
    void IncrementGenerationCount(EProceduralContentType ContentType);

    // ========================================================================
    // Quality Settings
    // ========================================================================

    /**
     * Sets the generation quality level.
     * @param Quality New quality setting
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Settings")
    void SetGenerationQuality(EProceduralQuality Quality);

    /** Returns current generation quality. */
    UFUNCTION(BlueprintPure, Category = "Procedural|Settings")
    EProceduralQuality GetGenerationQuality() const { return CurrentQuality; }

    /**
     * Sets default generation settings.
     * @param Settings New default settings
     */
    UFUNCTION(BlueprintCallable, Category = "Procedural|Settings")
    void SetDefaultSettings(const FGenerationSettings& Settings);

    /** Returns current default settings. */
    UFUNCTION(BlueprintPure, Category = "Procedural|Settings")
    FGenerationSettings GetDefaultSettings() const { return DefaultSettings; }

    // ========================================================================
    // Events
    // ========================================================================

    /// Broadcast when a track finishes generating
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnTrackGenerated OnTrackGenerated;

    /// Broadcast when environment generation completes
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnEnvironmentGenerated OnEnvironmentGenerated;

    /// Broadcast when a challenge is generated
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnChallengeGenerated OnChallengeGenerated;

    /// Broadcast during async generation with progress
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnGenerationProgress OnGenerationProgress;

    /// Broadcast when all generation completes
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnGenerationComplete OnGenerationComplete;

    /// Broadcast when generation fails
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnGenerationFailed OnGenerationFailed;

    /// Broadcast when player discovers a shortcut
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnShortcutDiscovered OnShortcutDiscovered;

    /// Broadcast when a track is shared
    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnSeedShared OnSeedShared;

protected:
    // ========================================================================
    // Internal Segment Generation Helpers
    // ========================================================================

    /** Creates a straight segment at the given position. */
    FTrackSegment CreateStraightSegment(const FVector& Start, const FRotator& Rotation, float Length);

    /** Creates a curved segment with specified radius and angle. */
    FTrackSegment CreateCurveSegment(const FVector& Start, const FRotator& Rotation, float Radius, float Angle);

    /** Creates a jump ramp segment. */
    FTrackSegment CreateJumpSegment(const FVector& Start, const FRotator& Rotation, float Length, float Height);

    /** Calculates overall track difficulty score. */
    float CalculateTrackDifficulty(const FProceduralTrack& Track);

    /** Estimates lap time based on track geometry. */
    float EstimateLapTime(const FProceduralTrack& Track);

    /** Checks if a new segment collides with existing track. */
    bool CheckSegmentCollision(const FTrackSegment& NewSegment, const TArray<FTrackSegment>& ExistingSegments);

    /** Generates checkpoint positions along track. */
    void GenerateCheckpoints(FProceduralTrack& Track);

    /** Generates starting grid positions. */
    void GenerateSpawnPositions(FProceduralTrack& Track);

    /** Selects next segment type based on current state and difficulty. */
    ETrackSegmentType SelectNextSegmentType(ETrackSegmentType CurrentType, EGenerationDifficulty Difficulty);

    /** Generates a Y2K-appropriate neon color. */
    FLinearColor GenerateY2KNeonColor();

private:
    // ========================================================================
    // Internal State
    // ========================================================================

    /// Current master seed for generation
    UPROPERTY()
    int64 CurrentMasterSeed;

    /// Current generation quality setting
    UPROPERTY()
    EProceduralQuality CurrentQuality;

    /// Default generation settings
    UPROPERTY()
    FGenerationSettings DefaultSettings;

    /// Lifetime statistics
    UPROPERTY()
    FProceduralContentStats ContentStats;

    /// All saved tracks by ID
    UPROPERTY()
    TMap<FGuid, FProceduralTrack> SavedTracks;

    /// Favorite tracks by ID
    UPROPERTY()
    TMap<FGuid, FProceduralTrack> FavoriteTracks;

    /// IDs of shortcuts player has discovered
    UPROPERTY()
    TArray<FGuid> DiscoveredShortcutIds;

    /// Playtime per track
    UPROPERTY()
    TMap<FGuid, float> TrackPlaytimes;

    /// Random number generator
    UPROPERTY()
    FRandomStream RandomStream;

    /// True if async generation is in progress
    UPROPERTY()
    bool bIsGenerating;

    /// Timer handle for async generation
    FTimerHandle AsyncGenerationTimer;

    // ========================================================================
    // Internal Initialization
    // ========================================================================

    /** Sets up default generation settings. */
    void InitializeDefaultSettings();

    /** Loads previously saved content from storage. */
    void LoadSavedContent();

    /** Persists current content to storage. */
    void SaveContentToStorage();
};
