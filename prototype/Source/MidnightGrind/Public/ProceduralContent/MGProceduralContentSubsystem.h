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
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EProceduralContentType : uint8
{
    Track,
    Environment,
    Challenge,
    Obstacle,
    Shortcut,
    SecretArea,
    Weather,
    TrafficPattern,
    EventSequence,
    Collectible
};

UENUM(BlueprintType)
enum class ETrackSegmentType : uint8
{
    Straight,
    GentleCurve,
    SharpCurve,
    Hairpin,
    SweepingS,
    Chicane,
    Intersection,
    Tunnel,
    Bridge,
    Overpass,
    Underpass,
    Jump,
    DriftZone,
    SpeedZone,
    TechnicalSection
};

UENUM(BlueprintType)
enum class EEnvironmentTheme : uint8
{
    UrbanDowntown,
    IndustrialDistrict,
    WaterfrontDocks,
    SuburbanNeighborhood,
    HighwayOverpass,
    ShoppingDistrict,
    ParkingStructure,
    AirportRunway,
    MountainPass,
    CoastalHighway,
    NeonAlley,
    CyberpunkSlums,
    RetroArcade,
    Y2KMall
};

UENUM(BlueprintType)
enum class EGenerationDifficulty : uint8
{
    VeryEasy,
    Easy,
    Medium,
    Hard,
    VeryHard,
    Extreme,
    Nightmare
};

UENUM(BlueprintType)
enum class EProceduralQuality : uint8
{
    Draft,
    Low,
    Medium,
    High,
    Ultra
};

UENUM(BlueprintType)
enum class EObstacleCategory : uint8
{
    Static,
    Dynamic,
    Destructible,
    Hazardous,
    Interactive,
    Decorative
};

UENUM(BlueprintType)
enum class EWeatherIntensity : uint8
{
    None,
    Light,
    Moderate,
    Heavy,
    Extreme
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Evening,
    Night,
    Midnight
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FProceduralSeed
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 MasterSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TrackSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EnvironmentSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ObstacleSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 WeatherSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TrafficSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SeedCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime GeneratedAt;
};

USTRUCT(BlueprintType)
struct FTrackSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SegmentId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETrackSegmentType SegmentType = ETrackSegmentType::Straight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EndPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator StartRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator EndRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Length = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BankAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ElevationChange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurveRadius = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedLimit = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DifficultyRating = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GripMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasBarriers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasStreetLights = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ControlPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> ConnectedSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> SurfaceProperties;
};

USTRUCT(BlueprintType)
struct FProceduralTrack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid TrackId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProceduralSeed Seed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentTheme Theme = EEnvironmentTheme::UrbanDowntown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGenerationDifficulty Difficulty = EGenerationDifficulty::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FTrackSegment> Segments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LapCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCircuit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAlternateRoutes = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AlternateRouteCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShortcutCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 JumpCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DriftZoneCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EstimatedLapTime = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DifficultyScore = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TrackCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TrackBoundsMin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TrackBoundsMax = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> CheckpointPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SpawnPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime GeneratedAt;
};

USTRUCT(BlueprintType)
struct FProceduralObstacle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ObstacleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObstacleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObstacleCategory Category = EObstacleCategory::Static;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CollisionRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageOnImpact = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedPenalty = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDestructible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 HealthPoints = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> ObstacleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> ObstacleMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> Properties;
};

USTRUCT(BlueprintType)
struct FProceduralShortcut
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ShortcutId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ShortcutName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EntryPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ExitPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSaved = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RiskLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinimumSpeedRequired = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresJump = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresDrift = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DiscoveryPoints = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> ObstacleIds;
};

USTRUCT(BlueprintType)
struct FProceduralEnvironment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid EnvironmentId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentTheme Theme = EEnvironmentTheme::UrbanDowntown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETimeOfDay TimeOfDay = ETimeOfDay::Night;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeatherIntensity WeatherIntensity = EWeatherIntensity::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VisibilityDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GripModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrafficDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PedestrianDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralObstacle> Obstacles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> LightPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FLinearColor> NeonColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> EnvironmentParams;
};

USTRUCT(BlueprintType)
struct FProceduralChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ChallengeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChallengeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChallengeDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChallengeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGenerationDifficulty Difficulty = EGenerationDifficulty::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimit = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardCredits = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardXP = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RewardItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RequiredTrackId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredVehicleClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> Modifiers;
};

USTRUCT(BlueprintType)
struct FTrafficPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid PatternId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PatternName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageSpeed = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeedVariation = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressivenessLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> VehicleTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> SpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DespawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> LaneDistribution;
};

USTRUCT(BlueprintType)
struct FProceduralCollectible
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid CollectibleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CollectibleType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Position = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointValue = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRare = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> CollectibleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor GlowColor = FLinearColor::Yellow;
};

USTRUCT(BlueprintType)
struct FGenerationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EProceduralQuality Quality = EProceduralQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGenerationDifficulty TargetDifficulty = EGenerationDifficulty::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentTheme PreferredTheme = EEnvironmentTheme::UrbanDowntown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinTrackLength = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTrackLength = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinSegments = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSegments = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurveFrequency = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float JumpFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShortcutFrequency = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ObstacleDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CollectibleDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowAlternateRoutes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateShortcuts = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateSecretAreas = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bGenerateTraffic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDynamicWeather = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> CustomParameters;
};

USTRUCT(BlueprintType)
struct FGenerationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProceduralTrack GeneratedTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProceduralEnvironment GeneratedEnvironment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralChallenge> GeneratedChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralShortcut> GeneratedShortcuts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FProceduralCollectible> GeneratedCollectibles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTrafficPattern GeneratedTrafficPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GenerationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalObjectsGenerated = 0;
};

USTRUCT(BlueprintType)
struct FProceduralContentStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalTracksGenerated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalEnvironmentsGenerated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalChallengesGenerated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalShortcutsDiscovered = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSecretAreasFound = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageGenerationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPlayTimeOnGenerated = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FavoritedTracks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SharedTracks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> ThemeUsageCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> DifficultyDistribution;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackGenerated, const FProceduralTrack&, Track);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentGenerated, const FProceduralEnvironment&, Environment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeGenerated, const FProceduralChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGenerationProgress, float, Progress, const FString&, CurrentStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationComplete, const FGenerationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGenerationFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShortcutDiscovered, const FProceduralShortcut&, Shortcut);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSeedShared, const FString&, SeedCode, const FString&, TrackName);

// ============================================================================
// Main Subsystem
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGProceduralContentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // Seed Management
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Seed")
    FProceduralSeed GenerateRandomSeed();

    UFUNCTION(BlueprintCallable, Category = "Procedural|Seed")
    FProceduralSeed CreateSeedFromCode(const FString& SeedCode);

    UFUNCTION(BlueprintPure, Category = "Procedural|Seed")
    FString GetSeedCode(const FProceduralSeed& Seed) const;

    UFUNCTION(BlueprintCallable, Category = "Procedural|Seed")
    void SetMasterSeed(int64 NewSeed);

    UFUNCTION(BlueprintPure, Category = "Procedural|Seed")
    int64 GetCurrentMasterSeed() const { return CurrentMasterSeed; }

    // ========================================================================
    // Track Generation
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FGenerationResult GenerateTrack(const FGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FGenerationResult GenerateTrackFromSeed(const FProceduralSeed& Seed, const FGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    void GenerateTrackAsync(const FGenerationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FTrackSegment GenerateSegment(ETrackSegmentType SegmentType, const FVector& StartPos, const FRotator& StartRot);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    TArray<FTrackSegment> GenerateSegmentSequence(int32 SegmentCount, EGenerationDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    bool ValidateTrack(const FProceduralTrack& Track);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Track")
    FProceduralTrack OptimizeTrack(const FProceduralTrack& Track);

    // ========================================================================
    // Environment Generation
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    FProceduralEnvironment GenerateEnvironment(EEnvironmentTheme Theme, const FProceduralTrack& ForTrack);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    TArray<FProceduralObstacle> GenerateObstacles(const FProceduralTrack& Track, float Density);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    TArray<FVector> GenerateLightPositions(const FProceduralTrack& Track, EEnvironmentTheme Theme);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    TArray<FLinearColor> GenerateNeonPalette(EEnvironmentTheme Theme, int32 ColorCount);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Environment")
    void ApplyWeatherToEnvironment(FProceduralEnvironment& Environment, const FString& WeatherType, EWeatherIntensity Intensity);

    // ========================================================================
    // Challenge Generation
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    FProceduralChallenge GenerateChallenge(const FProceduralTrack& Track, EGenerationDifficulty Difficulty);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    TArray<FProceduralChallenge> GenerateChallengeSet(const FProceduralTrack& Track, int32 ChallengeCount);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    FProceduralChallenge GenerateDailyChallenge();

    UFUNCTION(BlueprintCallable, Category = "Procedural|Challenge")
    TArray<FProceduralChallenge> GenerateWeeklyChallenges();

    // ========================================================================
    // Shortcuts and Secrets
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Shortcuts")
    TArray<FProceduralShortcut> GenerateShortcuts(const FProceduralTrack& Track, int32 MaxShortcuts);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Shortcuts")
    bool IsValidShortcut(const FProceduralShortcut& Shortcut, const FProceduralTrack& Track);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Shortcuts")
    void DiscoverShortcut(const FGuid& ShortcutId);

    UFUNCTION(BlueprintPure, Category = "Procedural|Shortcuts")
    TArray<FProceduralShortcut> GetDiscoveredShortcuts() const;

    // ========================================================================
    // Traffic Generation
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Traffic")
    FTrafficPattern GenerateTrafficPattern(const FProceduralTrack& Track, float Density);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Traffic")
    TArray<FVector> GetTrafficSpawnPoints(const FTrafficPattern& Pattern) const;

    UFUNCTION(BlueprintCallable, Category = "Procedural|Traffic")
    void UpdateTrafficPattern(FTrafficPattern& Pattern, float DeltaTime);

    // ========================================================================
    // Collectibles
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Collectibles")
    TArray<FProceduralCollectible> GenerateCollectibles(const FProceduralTrack& Track, float Density);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Collectibles")
    TArray<FProceduralCollectible> GenerateHiddenCollectibles(const FProceduralTrack& Track, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Collectibles")
    void CollectItem(const FGuid& CollectibleId);

    // ========================================================================
    // Content Storage
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    void SaveGeneratedTrack(const FProceduralTrack& Track);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    FProceduralTrack LoadTrackBySeed(const FString& SeedCode);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    TArray<FProceduralTrack> GetSavedTracks() const;

    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    void DeleteSavedTrack(const FGuid& TrackId);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    void FavoriteTrack(const FGuid& TrackId);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Storage")
    TArray<FProceduralTrack> GetFavoriteTracks() const;

    // ========================================================================
    // Sharing
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Sharing")
    FString ShareTrack(const FGuid& TrackId);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Sharing")
    FProceduralTrack ImportSharedTrack(const FString& ShareCode);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Sharing")
    bool ValidateShareCode(const FString& ShareCode);

    // ========================================================================
    // Statistics
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Procedural|Stats")
    FProceduralContentStats GetContentStats() const { return ContentStats; }

    UFUNCTION(BlueprintCallable, Category = "Procedural|Stats")
    void RecordTrackPlaytime(const FGuid& TrackId, float PlaytimeSeconds);

    UFUNCTION(BlueprintCallable, Category = "Procedural|Stats")
    void IncrementGenerationCount(EProceduralContentType ContentType);

    // ========================================================================
    // Quality Settings
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Procedural|Settings")
    void SetGenerationQuality(EProceduralQuality Quality);

    UFUNCTION(BlueprintPure, Category = "Procedural|Settings")
    EProceduralQuality GetGenerationQuality() const { return CurrentQuality; }

    UFUNCTION(BlueprintCallable, Category = "Procedural|Settings")
    void SetDefaultSettings(const FGenerationSettings& Settings);

    UFUNCTION(BlueprintPure, Category = "Procedural|Settings")
    FGenerationSettings GetDefaultSettings() const { return DefaultSettings; }

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnTrackGenerated OnTrackGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnEnvironmentGenerated OnEnvironmentGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnChallengeGenerated OnChallengeGenerated;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnGenerationProgress OnGenerationProgress;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnGenerationComplete OnGenerationComplete;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnGenerationFailed OnGenerationFailed;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnShortcutDiscovered OnShortcutDiscovered;

    UPROPERTY(BlueprintAssignable, Category = "Procedural|Events")
    FOnSeedShared OnSeedShared;

protected:
    // Internal generation helpers
    FTrackSegment CreateStraightSegment(const FVector& Start, const FRotator& Rotation, float Length);
    FTrackSegment CreateCurveSegment(const FVector& Start, const FRotator& Rotation, float Radius, float Angle);
    FTrackSegment CreateJumpSegment(const FVector& Start, const FRotator& Rotation, float Length, float Height);

    float CalculateTrackDifficulty(const FProceduralTrack& Track);
    float EstimateLapTime(const FProceduralTrack& Track);
    bool CheckSegmentCollision(const FTrackSegment& NewSegment, const TArray<FTrackSegment>& ExistingSegments);

    void GenerateCheckpoints(FProceduralTrack& Track);
    void GenerateSpawnPositions(FProceduralTrack& Track);

    ETrackSegmentType SelectNextSegmentType(ETrackSegmentType CurrentType, EGenerationDifficulty Difficulty);
    FLinearColor GenerateY2KNeonColor();

private:
    UPROPERTY()
    int64 CurrentMasterSeed;

    UPROPERTY()
    EProceduralQuality CurrentQuality;

    UPROPERTY()
    FGenerationSettings DefaultSettings;

    UPROPERTY()
    FProceduralContentStats ContentStats;

    UPROPERTY()
    TMap<FGuid, FProceduralTrack> SavedTracks;

    UPROPERTY()
    TMap<FGuid, FProceduralTrack> FavoriteTracks;

    UPROPERTY()
    TArray<FGuid> DiscoveredShortcutIds;

    UPROPERTY()
    TMap<FGuid, float> TrackPlaytimes;

    UPROPERTY()
    FRandomStream RandomStream;

    UPROPERTY()
    bool bIsGenerating;

    FTimerHandle AsyncGenerationTimer;

    void InitializeDefaultSettings();
    void LoadSavedContent();
    void SaveContentToStorage();
};
