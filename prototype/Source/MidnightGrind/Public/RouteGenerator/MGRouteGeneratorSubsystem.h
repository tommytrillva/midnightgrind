// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRouteGeneratorSubsystem.generated.h"

// Forward declarations
class UMGRouteGeneratorSubsystem;

/**
 * Route generation style
 */
UENUM(BlueprintType)
enum class EMGRouteStyle : uint8
{
    Street          UMETA(DisplayName = "Street"),         // Urban street racing
    Highway         UMETA(DisplayName = "Highway"),        // High-speed highway runs
    Mountain        UMETA(DisplayName = "Mountain"),       // Winding mountain roads
    Industrial      UMETA(DisplayName = "Industrial"),     // Industrial zones
    Coastal         UMETA(DisplayName = "Coastal"),        // Beachfront routes
    Downtown        UMETA(DisplayName = "Downtown"),       // Dense city center
    Suburban        UMETA(DisplayName = "Suburban"),       // Residential areas
    Mixed           UMETA(DisplayName = "Mixed")           // Varied terrain
};

/**
 * Route segment type
 */
UENUM(BlueprintType)
enum class EMGSegmentType : uint8
{
    Straight        UMETA(DisplayName = "Straight"),
    GentleCurve     UMETA(DisplayName = "Gentle Curve"),
    SharpCurve      UMETA(DisplayName = "Sharp Curve"),
    Hairpin         UMETA(DisplayName = "Hairpin"),
    SShape          UMETA(DisplayName = "S-Curve"),
    Chicane         UMETA(DisplayName = "Chicane"),
    Intersection    UMETA(DisplayName = "Intersection"),
    Roundabout      UMETA(DisplayName = "Roundabout"),
    Tunnel          UMETA(DisplayName = "Tunnel"),
    Bridge          UMETA(DisplayName = "Bridge"),
    Jump            UMETA(DisplayName = "Jump"),
    Split           UMETA(DisplayName = "Split Path"),
    Merge           UMETA(DisplayName = "Merge"),
    Finish          UMETA(DisplayName = "Finish")
};

/**
 * Route complexity level
 */
UENUM(BlueprintType)
enum class EMGRouteComplexity : uint8
{
    Beginner        UMETA(DisplayName = "Beginner"),
    Intermediate    UMETA(DisplayName = "Intermediate"),
    Advanced        UMETA(DisplayName = "Advanced"),
    Expert          UMETA(DisplayName = "Expert"),
    Extreme         UMETA(DisplayName = "Extreme")
};

/**
 * Route hazard type
 */
UENUM(BlueprintType)
enum class EMGRouteHazard : uint8
{
    None            UMETA(DisplayName = "None"),
    Traffic         UMETA(DisplayName = "Traffic"),
    Pedestrians     UMETA(DisplayName = "Pedestrians"),
    Construction    UMETA(DisplayName = "Construction"),
    WetSurface      UMETA(DisplayName = "Wet Surface"),
    OilSlick        UMETA(DisplayName = "Oil Slick"),
    Debris          UMETA(DisplayName = "Debris"),
    NarrowPath      UMETA(DisplayName = "Narrow Path"),
    Oncoming        UMETA(DisplayName = "Oncoming Traffic"),
    RoadWork        UMETA(DisplayName = "Road Work")
};

/**
 * Surface type for segments
 */
UENUM(BlueprintType)
enum class EMGSurfaceType : uint8
{
    Asphalt         UMETA(DisplayName = "Asphalt"),
    Concrete        UMETA(DisplayName = "Concrete"),
    Cobblestone     UMETA(DisplayName = "Cobblestone"),
    Gravel          UMETA(DisplayName = "Gravel"),
    Dirt            UMETA(DisplayName = "Dirt"),
    Sand            UMETA(DisplayName = "Sand"),
    WetAsphalt      UMETA(DisplayName = "Wet Asphalt"),
    Ice             UMETA(DisplayName = "Ice"),
    Metal           UMETA(DisplayName = "Metal Grating")
};

/**
 * Scenic element type
 */
UENUM(BlueprintType)
enum class EMGScenicElement : uint8
{
    None            UMETA(DisplayName = "None"),
    Skyline         UMETA(DisplayName = "City Skyline"),
    Ocean           UMETA(DisplayName = "Ocean View"),
    Mountain        UMETA(DisplayName = "Mountain View"),
    NeonSigns       UMETA(DisplayName = "Neon Signs"),
    Graffiti        UMETA(DisplayName = "Street Art"),
    Landmarks       UMETA(DisplayName = "Landmarks"),
    Billboard       UMETA(DisplayName = "Billboards"),
    Sunset          UMETA(DisplayName = "Sunset View")
};

/**
 * Route checkpoint data
 */
USTRUCT(BlueprintType)
struct FMGRouteCheckpoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    int32 CheckpointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float DistanceFromStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float SuggestedSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    bool bIsSector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    bool bIsFinishLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float TimeExtension;

    FMGRouteCheckpoint()
        : CheckpointIndex(0)
        , Location(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , Width(20.0f)
        , DistanceFromStart(0.0f)
        , SuggestedSpeed(100.0f)
        , bIsSector(false)
        , bIsFinishLine(false)
        , TimeExtension(30.0f)
    {}
};

/**
 * Route segment data
 */
USTRUCT(BlueprintType)
struct FMGRouteSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    int32 SegmentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    EMGSegmentType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    FVector ControlPoint1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    FVector ControlPoint2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float Length;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float CurveRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float CurveAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float ElevationChange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float Banking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    EMGSurfaceType Surface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    TArray<EMGRouteHazard> Hazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    EMGScenicElement ScenicElement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float SuggestedSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    bool bHasShortcut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    bool bIsSecret;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
    float DriftPotential;

    FMGRouteSegment()
        : SegmentIndex(0)
        , Type(EMGSegmentType::Straight)
        , StartPoint(FVector::ZeroVector)
        , EndPoint(FVector::ZeroVector)
        , ControlPoint1(FVector::ZeroVector)
        , ControlPoint2(FVector::ZeroVector)
        , Length(100.0f)
        , Width(15.0f)
        , CurveRadius(0.0f)
        , CurveAngle(0.0f)
        , Elevation(0.0f)
        , ElevationChange(0.0f)
        , Banking(0.0f)
        , Surface(EMGSurfaceType::Asphalt)
        , ScenicElement(EMGScenicElement::None)
        , SuggestedSpeed(150.0f)
        , MaxSpeed(300.0f)
        , bHasShortcut(false)
        , bIsSecret(false)
        , DriftPotential(0.5f)
    {}
};

/**
 * Route spawn point for vehicles/objects
 */
USTRUCT(BlueprintType)
struct FMGRouteSpawnPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 GridPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float DistanceFromStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bIsStartingGrid;

    FMGRouteSpawnPoint()
        : Location(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , GridPosition(0)
        , DistanceFromStart(0.0f)
        , bIsStartingGrid(false)
    {}
};

/**
 * Shortcut path definition
 */
USTRUCT(BlueprintType)
struct FMGShortcut
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    FGuid ShortcutId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    int32 EntrySegmentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    int32 ExitSegmentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    float TimeSaved;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    float RiskLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    EMGSurfaceType Surface;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    bool bRequiresJump;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    bool bRequiresDestruction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shortcut")
    bool bIsHidden;

    FMGShortcut()
        : EntrySegmentIndex(0)
        , ExitSegmentIndex(0)
        , TimeSaved(2.0f)
        , RiskLevel(0.5f)
        , Surface(EMGSurfaceType::Asphalt)
        , bRequiresJump(false)
        , bRequiresDestruction(false)
        , bIsHidden(false)
    {}
};

/**
 * Route generation parameters
 */
USTRUCT(BlueprintType)
struct FMGRouteParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    EMGRouteStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    EMGRouteComplexity Complexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float TargetLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float MinLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float MaxLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    int32 MinSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    int32 MaxSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float CurveFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float SharpCurveChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float HairpinChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float StraightPreference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float ElevationVariance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float MaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float ShortcutChance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float HazardDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    float ScenicDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    bool bIsCircuit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    bool bAllowJumps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    bool bAllowSplitPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Params")
    int32 RandomSeed;

    FMGRouteParams()
        : Style(EMGRouteStyle::Street)
        , Complexity(EMGRouteComplexity::Intermediate)
        , TargetLength(5000.0f)
        , MinLength(3000.0f)
        , MaxLength(10000.0f)
        , MinSegments(20)
        , MaxSegments(100)
        , CurveFrequency(0.4f)
        , SharpCurveChance(0.2f)
        , HairpinChance(0.05f)
        , StraightPreference(0.3f)
        , ElevationVariance(50.0f)
        , MaxElevation(200.0f)
        , ShortcutChance(0.15f)
        , HazardDensity(0.2f)
        , ScenicDensity(0.3f)
        , bIsCircuit(false)
        , bAllowJumps(true)
        , bAllowSplitPaths(false)
        , RandomSeed(0)
    {}
};

/**
 * Complete generated route
 */
USTRUCT(BlueprintType)
struct FMGGeneratedRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    FGuid RouteId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    FString RouteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    EMGRouteStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    EMGRouteComplexity Complexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    TArray<FMGRouteSegment> Segments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    TArray<FMGRouteCheckpoint> Checkpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    TArray<FMGRouteSpawnPoint> SpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    TArray<FMGShortcut> Shortcuts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float TotalLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float AverageWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float MaxElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float MinElevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float TotalElevationGain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    int32 TotalCurves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    int32 SharpCurves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    int32 Hairpins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float EstimatedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    float DifficultyRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    bool bIsCircuit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    int32 GenerationSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    FDateTime GenerationDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    FVector BoundsMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Route")
    FVector BoundsMax;

    FMGGeneratedRoute()
        : Style(EMGRouteStyle::Street)
        , Complexity(EMGRouteComplexity::Intermediate)
        , TotalLength(0.0f)
        , AverageWidth(15.0f)
        , MaxElevation(0.0f)
        , MinElevation(0.0f)
        , TotalElevationGain(0.0f)
        , TotalCurves(0)
        , SharpCurves(0)
        , Hairpins(0)
        , EstimatedTime(0.0f)
        , DifficultyRating(0.5f)
        , bIsCircuit(false)
        , GenerationSeed(0)
        , GenerationDate(FDateTime::Now())
        , BoundsMin(FVector::ZeroVector)
        , BoundsMax(FVector::ZeroVector)
    {}
};

/**
 * Spline point for racing line
 */
USTRUCT(BlueprintType)
struct FMGRacingLinePoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    FVector Tangent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    bool bIsBrakingZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    bool bIsDriftZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
    bool bIsNitroZone;

    FMGRacingLinePoint()
        : Location(FVector::ZeroVector)
        , Tangent(FVector::ForwardVector)
        , Width(15.0f)
        , Speed(150.0f)
        , Distance(0.0f)
        , bIsBrakingZone(false)
        , bIsDriftZone(false)
        , bIsNitroZone(false)
    {}
};

/**
 * Style parameters for route generation
 */
USTRUCT(BlueprintType)
struct FMGRouteStyleParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    EMGRouteStyle Style;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    float MinRoadWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    float MaxRoadWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    float CurvePreference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    float ElevationScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    TArray<EMGSegmentType> PreferredSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    TArray<EMGSurfaceType> PreferredSurfaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    TArray<EMGScenicElement> PreferredScenic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
    TArray<EMGRouteHazard> PossibleHazards;

    FMGRouteStyleParams()
        : Style(EMGRouteStyle::Street)
        , MinRoadWidth(12.0f)
        , MaxRoadWidth(20.0f)
        , CurvePreference(0.5f)
        , ElevationScale(1.0f)
    {}
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRouteGenerated, const FMGGeneratedRoute&, Route);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnGenerationProgress, float, Progress, const FString&, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnGenerationFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnSegmentGenerated, int32, SegmentIndex, const FMGRouteSegment&, Segment);

/**
 * Route Generator Subsystem
 *
 * Procedurally generates racing routes with varied terrain, curves,
 * hazards, and scenic elements. Creates checkpoints, spawn points,
 * shortcuts, and racing lines for dynamic track generation.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRouteGeneratorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGRouteGeneratorSubsystem();

    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    // Route generation
    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    FMGGeneratedRoute GenerateRoute(const FMGRouteParams& Params);

    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    void GenerateRouteAsync(const FMGRouteParams& Params);

    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    void CancelGeneration();

    UFUNCTION(BlueprintPure, Category = "RouteGenerator")
    bool IsGenerating() const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator")
    float GetGenerationProgress() const;

    // Route management
    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    bool SaveRoute(const FMGGeneratedRoute& Route, const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    FMGGeneratedRoute LoadRoute(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    bool DeleteRoute(const FString& SlotName);

    UFUNCTION(BlueprintPure, Category = "RouteGenerator")
    TArray<FString> GetSavedRouteNames() const;

    // Route queries
    UFUNCTION(BlueprintPure, Category = "RouteGenerator")
    FMGGeneratedRoute GetCurrentRoute() const;

    UFUNCTION(BlueprintCallable, Category = "RouteGenerator")
    void SetCurrentRoute(const FMGGeneratedRoute& Route);

    UFUNCTION(BlueprintPure, Category = "RouteGenerator")
    bool HasCurrentRoute() const;

    // Segment queries
    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Segments")
    FMGRouteSegment GetSegmentAtDistance(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Segments")
    int32 GetSegmentIndexAtDistance(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Segments")
    FVector GetPointOnRoute(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Segments")
    FRotator GetRotationOnRoute(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Segments")
    float GetWidthAtDistance(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Segments")
    EMGSurfaceType GetSurfaceAtDistance(float Distance) const;

    // Checkpoint queries
    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Checkpoints")
    FMGRouteCheckpoint GetCheckpoint(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Checkpoints")
    int32 GetCheckpointCount() const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Checkpoints")
    FMGRouteCheckpoint GetNearestCheckpoint(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Checkpoints")
    float GetDistanceToNextCheckpoint(float CurrentDistance) const;

    // Spawn points
    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Spawns")
    TArray<FMGRouteSpawnPoint> GetStartingGrid(int32 MaxPositions = 12) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Spawns")
    FMGRouteSpawnPoint GetSpawnPoint(int32 GridPosition) const;

    // Shortcuts
    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Shortcuts")
    TArray<FMGShortcut> GetShortcuts() const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Shortcuts")
    FMGShortcut GetNearestShortcut(const FVector& Location, float MaxDistance = 100.0f) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Shortcuts")
    bool IsOnShortcut(const FVector& Location, FGuid& OutShortcutId) const;

    // Racing line
    UFUNCTION(BlueprintCallable, Category = "RouteGenerator|RacingLine")
    TArray<FMGRacingLinePoint> GenerateRacingLine(int32 Resolution = 100);

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|RacingLine")
    FMGRacingLinePoint GetRacingLinePoint(float Distance) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|RacingLine")
    TArray<FMGRacingLinePoint> GetRacingLine() const;

    // Style configuration
    UFUNCTION(BlueprintCallable, Category = "RouteGenerator|Style")
    void SetStyleParams(EMGRouteStyle Style, const FMGRouteStyleParams& Params);

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Style")
    FMGRouteStyleParams GetStyleParams(EMGRouteStyle Style) const;

    // Validation
    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Validation")
    bool ValidateRoute(const FMGGeneratedRoute& Route, FString& OutError) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Validation")
    bool IsLocationOnRoute(const FVector& Location, float Tolerance = 50.0f) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Validation")
    float GetDistanceAlongRoute(const FVector& Location) const;

    // Presets
    UFUNCTION(BlueprintCallable, Category = "RouteGenerator|Presets")
    FMGRouteParams GetPresetParams(const FString& PresetName) const;

    UFUNCTION(BlueprintPure, Category = "RouteGenerator|Presets")
    TArray<FString> GetAvailablePresets() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "RouteGenerator|Events")
    FMGOnRouteGenerated OnRouteGenerated;

    UPROPERTY(BlueprintAssignable, Category = "RouteGenerator|Events")
    FMGOnGenerationProgress OnGenerationProgress;

    UPROPERTY(BlueprintAssignable, Category = "RouteGenerator|Events")
    FMGOnGenerationFailed OnGenerationFailed;

    UPROPERTY(BlueprintAssignable, Category = "RouteGenerator|Events")
    FMGOnSegmentGenerated OnSegmentGenerated;

protected:
    // Current route
    UPROPERTY()
    FMGGeneratedRoute CurrentRoute;

    UPROPERTY()
    bool bHasRoute;

    // Generation state
    UPROPERTY()
    bool bIsGenerating;

    UPROPERTY()
    bool bCancelRequested;

    UPROPERTY()
    float GenerationProgress;

    // Racing line cache
    UPROPERTY()
    TArray<FMGRacingLinePoint> CachedRacingLine;

    // Style configurations
    UPROPERTY()
    TMap<EMGRouteStyle, FMGRouteStyleParams> StyleConfigs;

    // Route presets
    UPROPERTY()
    TMap<FString, FMGRouteParams> RoutePresets;

    // Random stream for deterministic generation
    FRandomStream RandomStream;

    // Generation helpers
    void InitializeStyleConfigs();
    void InitializePresets();

    FMGRouteSegment GenerateSegment(const FMGRouteParams& Params, const FMGRouteSegment& PreviousSegment, int32 Index);
    EMGSegmentType ChooseNextSegmentType(const FMGRouteParams& Params, EMGSegmentType Previous);
    FVector CalculateSegmentEndPoint(const FMGRouteSegment& Segment);
    void GenerateCheckpoints(FMGGeneratedRoute& Route);
    void GenerateSpawnPoints(FMGGeneratedRoute& Route, int32 MaxSpawns = 12);
    void GenerateShortcuts(FMGGeneratedRoute& Route, const FMGRouteParams& Params);
    void CalculateRouteMetrics(FMGGeneratedRoute& Route);
    void ApplyStyleToSegment(FMGRouteSegment& Segment, EMGRouteStyle Style);

    FVector BezierPoint(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3, float T) const;
    float CalculateCurveLength(const FMGRouteSegment& Segment) const;
    FRotator CalculateSegmentRotation(const FMGRouteSegment& Segment, float T) const;
};
