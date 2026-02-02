// Copyright Midnight Grind. All Rights Reserved.

#include "RouteGenerator/MGRouteGeneratorSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"

UMGRouteGeneratorSubsystem::UMGRouteGeneratorSubsystem()
    : bHasRoute(false)
    , bIsGenerating(false)
    , bCancelRequested(false)
    , GenerationProgress(0.0f)
{
}

void UMGRouteGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeStyleConfigs();
    InitializePresets();

    UE_LOG(LogTemp, Log, TEXT("RouteGenerator: Subsystem initialized"));
}

void UMGRouteGeneratorSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UMGRouteGeneratorSubsystem::InitializeStyleConfigs()
{
    // Street style
    FMGRouteStyleParams Street;
    Street.Style = EMGRouteStyle::Street;
    Street.MinRoadWidth = 12.0f;
    Street.MaxRoadWidth = 18.0f;
    Street.CurvePreference = 0.4f;
    Street.ElevationScale = 0.3f;
    Street.PreferredSegments = {
        EMGSegmentType::Straight,
        EMGSegmentType::GentleCurve,
        EMGSegmentType::SharpCurve,
        EMGSegmentType::Intersection
    };
    Street.PreferredSurfaces = { EMGSurfaceType::Asphalt };
    Street.PreferredScenic = { EMGScenicElement::NeonSigns, EMGScenicElement::Graffiti };
    Street.PossibleHazards = { EMGRouteHazard::Traffic, EMGRouteHazard::Pedestrians };
    StyleConfigs.Add(EMGRouteStyle::Street, Street);

    // Highway style
    FMGRouteStyleParams Highway;
    Highway.Style = EMGRouteStyle::Highway;
    Highway.MinRoadWidth = 20.0f;
    Highway.MaxRoadWidth = 30.0f;
    Highway.CurvePreference = 0.2f;
    Highway.ElevationScale = 0.5f;
    Highway.PreferredSegments = {
        EMGSegmentType::Straight,
        EMGSegmentType::GentleCurve,
        EMGSegmentType::Bridge
    };
    Highway.PreferredSurfaces = { EMGSurfaceType::Asphalt, EMGSurfaceType::Concrete };
    Highway.PreferredScenic = { EMGScenicElement::Skyline, EMGScenicElement::Billboard };
    Highway.PossibleHazards = { EMGRouteHazard::Traffic, EMGRouteHazard::Construction };
    StyleConfigs.Add(EMGRouteStyle::Highway, Highway);

    // Mountain style
    FMGRouteStyleParams Mountain;
    Mountain.Style = EMGRouteStyle::Mountain;
    Mountain.MinRoadWidth = 10.0f;
    Mountain.MaxRoadWidth = 15.0f;
    Mountain.CurvePreference = 0.7f;
    Mountain.ElevationScale = 2.0f;
    Mountain.PreferredSegments = {
        EMGSegmentType::SharpCurve,
        EMGSegmentType::Hairpin,
        EMGSegmentType::SShape,
        EMGSegmentType::Tunnel
    };
    Mountain.PreferredSurfaces = { EMGSurfaceType::Asphalt, EMGSurfaceType::Gravel };
    Mountain.PreferredScenic = { EMGScenicElement::Mountain, EMGScenicElement::Sunset };
    Mountain.PossibleHazards = { EMGRouteHazard::Debris, EMGRouteHazard::WetSurface, EMGRouteHazard::NarrowPath };
    StyleConfigs.Add(EMGRouteStyle::Mountain, Mountain);

    // Coastal style
    FMGRouteStyleParams Coastal;
    Coastal.Style = EMGRouteStyle::Coastal;
    Coastal.MinRoadWidth = 12.0f;
    Coastal.MaxRoadWidth = 18.0f;
    Coastal.CurvePreference = 0.5f;
    Coastal.ElevationScale = 0.4f;
    Coastal.PreferredSegments = {
        EMGSegmentType::GentleCurve,
        EMGSegmentType::SShape,
        EMGSegmentType::Bridge
    };
    Coastal.PreferredSurfaces = { EMGSurfaceType::Asphalt, EMGSurfaceType::Concrete };
    Coastal.PreferredScenic = { EMGScenicElement::Ocean, EMGScenicElement::Sunset };
    Coastal.PossibleHazards = { EMGRouteHazard::WetSurface, EMGRouteHazard::Sand };
    StyleConfigs.Add(EMGRouteStyle::Coastal, Coastal);

    // Downtown style
    FMGRouteStyleParams Downtown;
    Downtown.Style = EMGRouteStyle::Downtown;
    Downtown.MinRoadWidth = 14.0f;
    Downtown.MaxRoadWidth = 22.0f;
    Downtown.CurvePreference = 0.3f;
    Downtown.ElevationScale = 0.1f;
    Downtown.PreferredSegments = {
        EMGSegmentType::Straight,
        EMGSegmentType::SharpCurve,
        EMGSegmentType::Intersection,
        EMGSegmentType::Chicane
    };
    Downtown.PreferredSurfaces = { EMGSurfaceType::Asphalt, EMGSurfaceType::Cobblestone };
    Downtown.PreferredScenic = { EMGScenicElement::Skyline, EMGScenicElement::NeonSigns, EMGScenicElement::Landmarks };
    Downtown.PossibleHazards = { EMGRouteHazard::Traffic, EMGRouteHazard::Pedestrians, EMGRouteHazard::Construction };
    StyleConfigs.Add(EMGRouteStyle::Downtown, Downtown);

    // Industrial style
    FMGRouteStyleParams Industrial;
    Industrial.Style = EMGRouteStyle::Industrial;
    Industrial.MinRoadWidth = 15.0f;
    Industrial.MaxRoadWidth = 25.0f;
    Industrial.CurvePreference = 0.35f;
    Industrial.ElevationScale = 0.2f;
    Industrial.PreferredSegments = {
        EMGSegmentType::Straight,
        EMGSegmentType::GentleCurve,
        EMGSegmentType::Roundabout
    };
    Industrial.PreferredSurfaces = { EMGSurfaceType::Concrete, EMGSurfaceType::Asphalt, EMGSurfaceType::Metal };
    Industrial.PreferredScenic = { EMGScenicElement::None };
    Industrial.PossibleHazards = { EMGRouteHazard::RoadWork, EMGRouteHazard::Debris, EMGRouteHazard::OilSlick };
    StyleConfigs.Add(EMGRouteStyle::Industrial, Industrial);

    // Suburban style
    FMGRouteStyleParams Suburban;
    Suburban.Style = EMGRouteStyle::Suburban;
    Suburban.MinRoadWidth = 10.0f;
    Suburban.MaxRoadWidth = 16.0f;
    Suburban.CurvePreference = 0.45f;
    Suburban.ElevationScale = 0.3f;
    Suburban.PreferredSegments = {
        EMGSegmentType::Straight,
        EMGSegmentType::GentleCurve,
        EMGSegmentType::SShape,
        EMGSegmentType::Intersection
    };
    Suburban.PreferredSurfaces = { EMGSurfaceType::Asphalt };
    Suburban.PreferredScenic = { EMGScenicElement::None };
    Suburban.PossibleHazards = { EMGRouteHazard::Pedestrians, EMGRouteHazard::Traffic };
    StyleConfigs.Add(EMGRouteStyle::Suburban, Suburban);
}

void UMGRouteGeneratorSubsystem::InitializePresets()
{
    // Quick Sprint
    FMGRouteParams QuickSprint;
    QuickSprint.Style = EMGRouteStyle::Street;
    QuickSprint.Complexity = EMGRouteComplexity::Beginner;
    QuickSprint.TargetLength = 2000.0f;
    QuickSprint.MinSegments = 10;
    QuickSprint.MaxSegments = 25;
    QuickSprint.CurveFrequency = 0.3f;
    QuickSprint.bIsCircuit = false;
    RoutePresets.Add(TEXT("QuickSprint"), QuickSprint);

    // City Circuit
    FMGRouteParams CityCircuit;
    CityCircuit.Style = EMGRouteStyle::Downtown;
    CityCircuit.Complexity = EMGRouteComplexity::Intermediate;
    CityCircuit.TargetLength = 5000.0f;
    CityCircuit.MinSegments = 30;
    CityCircuit.MaxSegments = 60;
    CityCircuit.CurveFrequency = 0.4f;
    CityCircuit.bIsCircuit = true;
    CityCircuit.ShortcutChance = 0.2f;
    RoutePresets.Add(TEXT("CityCircuit"), CityCircuit);

    // Highway Run
    FMGRouteParams HighwayRun;
    HighwayRun.Style = EMGRouteStyle::Highway;
    HighwayRun.Complexity = EMGRouteComplexity::Intermediate;
    HighwayRun.TargetLength = 8000.0f;
    HighwayRun.StraightPreference = 0.6f;
    HighwayRun.CurveFrequency = 0.2f;
    HighwayRun.bIsCircuit = false;
    RoutePresets.Add(TEXT("HighwayRun"), HighwayRun);

    // Mountain Touge
    FMGRouteParams MountainTouge;
    MountainTouge.Style = EMGRouteStyle::Mountain;
    MountainTouge.Complexity = EMGRouteComplexity::Advanced;
    MountainTouge.TargetLength = 6000.0f;
    MountainTouge.CurveFrequency = 0.7f;
    MountainTouge.SharpCurveChance = 0.4f;
    MountainTouge.HairpinChance = 0.15f;
    MountainTouge.ElevationVariance = 100.0f;
    MountainTouge.MaxElevation = 300.0f;
    MountainTouge.bIsCircuit = false;
    RoutePresets.Add(TEXT("MountainTouge"), MountainTouge);

    // Coastal Cruise
    FMGRouteParams CoastalCruise;
    CoastalCruise.Style = EMGRouteStyle::Coastal;
    CoastalCruise.Complexity = EMGRouteComplexity::Intermediate;
    CoastalCruise.TargetLength = 5500.0f;
    CoastalCruise.CurveFrequency = 0.5f;
    CoastalCruise.ScenicDensity = 0.5f;
    CoastalCruise.bIsCircuit = false;
    RoutePresets.Add(TEXT("CoastalCruise"), CoastalCruise);

    // Expert Challenge
    FMGRouteParams ExpertChallenge;
    ExpertChallenge.Style = EMGRouteStyle::Mixed;
    ExpertChallenge.Complexity = EMGRouteComplexity::Expert;
    ExpertChallenge.TargetLength = 10000.0f;
    ExpertChallenge.MinSegments = 60;
    ExpertChallenge.MaxSegments = 120;
    ExpertChallenge.CurveFrequency = 0.5f;
    ExpertChallenge.SharpCurveChance = 0.35f;
    ExpertChallenge.HairpinChance = 0.1f;
    ExpertChallenge.HazardDensity = 0.4f;
    ExpertChallenge.ShortcutChance = 0.25f;
    ExpertChallenge.bAllowJumps = true;
    ExpertChallenge.bIsCircuit = true;
    RoutePresets.Add(TEXT("ExpertChallenge"), ExpertChallenge);
}

FMGGeneratedRoute UMGRouteGeneratorSubsystem::GenerateRoute(const FMGRouteParams& Params)
{
    bIsGenerating = true;
    bCancelRequested = false;
    GenerationProgress = 0.0f;

    FMGGeneratedRoute Route;
    Route.RouteId = FGuid::NewGuid();
    Route.Style = Params.Style;
    Route.Complexity = Params.Complexity;
    Route.bIsCircuit = Params.bIsCircuit;
    Route.GenerationSeed = Params.RandomSeed != 0 ? Params.RandomSeed : FMath::Rand();
    Route.GenerationDate = FDateTime::Now();

    // Initialize random stream
    RandomStream.Initialize(Route.GenerationSeed);

    OnGenerationProgress.Broadcast(0.0f, TEXT("Starting generation..."));

    // Determine number of segments
    int32 TargetSegments = FMath::Clamp(
        FMath::RoundToInt(Params.TargetLength / 100.0f),
        Params.MinSegments,
        Params.MaxSegments
    );

    float CurrentLength = 0.0f;
    FMGRouteSegment PreviousSegment;
    PreviousSegment.EndPoint = FVector::ZeroVector;
    PreviousSegment.Type = EMGSegmentType::Straight;

    // Generate segments
    for (int32 i = 0; i < TargetSegments && !bCancelRequested; ++i)
    {
        if (CurrentLength >= Params.MaxLength)
        {
            break;
        }

        FMGRouteSegment Segment = GenerateSegment(Params, PreviousSegment, i);
        Route.Segments.Add(Segment);

        CurrentLength += Segment.Length;
        PreviousSegment = Segment;

        GenerationProgress = (float)(i + 1) / (float)TargetSegments * 0.6f;
        OnSegmentGenerated.Broadcast(i, Segment);

        if (i % 10 == 0)
        {
            OnGenerationProgress.Broadcast(GenerationProgress,
                FString::Printf(TEXT("Generating segment %d/%d"), i + 1, TargetSegments));
        }
    }

    if (bCancelRequested)
    {
        bIsGenerating = false;
        OnGenerationFailed.Broadcast(TEXT("Generation cancelled"));
        return FMGGeneratedRoute();
    }

    OnGenerationProgress.Broadcast(0.7f, TEXT("Generating checkpoints..."));

    // Handle circuit closure
    if (Params.bIsCircuit && Route.Segments.Num() > 0)
    {
        FMGRouteSegment& LastSegment = Route.Segments.Last();
        FMGRouteSegment& FirstSegment = Route.Segments[0];

        // Create closing segment
        FMGRouteSegment ClosingSegment;
        ClosingSegment.SegmentIndex = Route.Segments.Num();
        ClosingSegment.Type = EMGSegmentType::GentleCurve;
        ClosingSegment.StartPoint = LastSegment.EndPoint;
        ClosingSegment.EndPoint = FirstSegment.StartPoint;
        ClosingSegment.Length = FVector::Dist(ClosingSegment.StartPoint, ClosingSegment.EndPoint);
        ApplyStyleToSegment(ClosingSegment, Params.Style);
        Route.Segments.Add(ClosingSegment);
    }

    // Generate checkpoints
    GenerateCheckpoints(Route);
    GenerationProgress = 0.8f;

    OnGenerationProgress.Broadcast(0.85f, TEXT("Generating spawn points..."));

    // Generate spawn points
    GenerateSpawnPoints(Route);

    OnGenerationProgress.Broadcast(0.9f, TEXT("Generating shortcuts..."));

    // Generate shortcuts
    GenerateShortcuts(Route, Params);

    OnGenerationProgress.Broadcast(0.95f, TEXT("Calculating metrics..."));

    // Calculate route metrics
    CalculateRouteMetrics(Route);

    // Generate route name
    Route.RouteName = FString::Printf(TEXT("%s Route %d"),
        *UEnum::GetDisplayValueAsText(Params.Style).ToString(),
        Route.GenerationSeed % 10000);

    GenerationProgress = 1.0f;
    bIsGenerating = false;

    CurrentRoute = Route;
    bHasRoute = true;
    CachedRacingLine.Empty();

    OnGenerationProgress.Broadcast(1.0f, TEXT("Generation complete"));
    OnRouteGenerated.Broadcast(Route);

    UE_LOG(LogTemp, Log, TEXT("RouteGenerator: Generated route '%s' with %d segments, %.0fm length"),
           *Route.RouteName, Route.Segments.Num(), Route.TotalLength);

    return Route;
}

void UMGRouteGeneratorSubsystem::GenerateRouteAsync(const FMGRouteParams& Params)
{
    // In a real implementation, this would use async tasks
    // For now, just call synchronous version
    GenerateRoute(Params);
}

void UMGRouteGeneratorSubsystem::CancelGeneration()
{
    bCancelRequested = true;
}

bool UMGRouteGeneratorSubsystem::IsGenerating() const
{
    return bIsGenerating;
}

float UMGRouteGeneratorSubsystem::GetGenerationProgress() const
{
    return GenerationProgress;
}

FMGRouteSegment UMGRouteGeneratorSubsystem::GenerateSegment(const FMGRouteParams& Params,
                                                            const FMGRouteSegment& PreviousSegment,
                                                            int32 Index)
{
    FMGRouteSegment Segment;
    Segment.SegmentIndex = Index;

    // Choose segment type
    Segment.Type = ChooseNextSegmentType(Params, PreviousSegment.Type);

    // Set start point from previous segment
    Segment.StartPoint = PreviousSegment.EndPoint;

    // Get style parameters
    FMGRouteStyleParams StyleParams = GetStyleParams(Params.Style);

    // Calculate segment properties based on type
    float BaseLength = RandomStream.FRandRange(80.0f, 200.0f);
    float CurveAngle = 0.0f;
    float CurveRadius = 0.0f;

    switch (Segment.Type)
    {
        case EMGSegmentType::Straight:
            BaseLength = RandomStream.FRandRange(150.0f, 400.0f);
            break;

        case EMGSegmentType::GentleCurve:
            CurveAngle = RandomStream.FRandRange(15.0f, 45.0f);
            CurveRadius = RandomStream.FRandRange(100.0f, 200.0f);
            if (RandomStream.FRand() > 0.5f) CurveAngle = -CurveAngle;
            break;

        case EMGSegmentType::SharpCurve:
            CurveAngle = RandomStream.FRandRange(60.0f, 120.0f);
            CurveRadius = RandomStream.FRandRange(40.0f, 80.0f);
            if (RandomStream.FRand() > 0.5f) CurveAngle = -CurveAngle;
            break;

        case EMGSegmentType::Hairpin:
            CurveAngle = RandomStream.FRandRange(150.0f, 180.0f);
            CurveRadius = RandomStream.FRandRange(20.0f, 40.0f);
            if (RandomStream.FRand() > 0.5f) CurveAngle = -CurveAngle;
            break;

        case EMGSegmentType::SShape:
            CurveAngle = RandomStream.FRandRange(30.0f, 60.0f);
            CurveRadius = RandomStream.FRandRange(60.0f, 100.0f);
            BaseLength = RandomStream.FRandRange(200.0f, 350.0f);
            break;

        case EMGSegmentType::Chicane:
            CurveAngle = RandomStream.FRandRange(20.0f, 40.0f);
            CurveRadius = RandomStream.FRandRange(30.0f, 50.0f);
            BaseLength = RandomStream.FRandRange(100.0f, 200.0f);
            break;

        case EMGSegmentType::Jump:
            BaseLength = RandomStream.FRandRange(50.0f, 100.0f);
            break;

        case EMGSegmentType::Tunnel:
        case EMGSegmentType::Bridge:
            BaseLength = RandomStream.FRandRange(100.0f, 300.0f);
            break;

        default:
            break;
    }

    Segment.Length = BaseLength;
    Segment.CurveAngle = CurveAngle;
    Segment.CurveRadius = CurveRadius;

    // Calculate width
    Segment.Width = RandomStream.FRandRange(StyleParams.MinRoadWidth, StyleParams.MaxRoadWidth);

    // Calculate elevation
    float ElevationDelta = RandomStream.FRandRange(-Params.ElevationVariance, Params.ElevationVariance) *
                           StyleParams.ElevationScale;
    Segment.Elevation = FMath::Clamp(
        PreviousSegment.Elevation + ElevationDelta,
        -Params.MaxElevation,
        Params.MaxElevation
    );
    Segment.ElevationChange = Segment.Elevation - PreviousSegment.Elevation;

    // Calculate banking for curves
    if (FMath::Abs(CurveAngle) > 30.0f)
    {
        Segment.Banking = FMath::Sign(CurveAngle) * FMath::Clamp(FMath::Abs(CurveAngle) / 180.0f * 15.0f, 0.0f, 15.0f);
    }

    // Calculate end point
    Segment.EndPoint = CalculateSegmentEndPoint(Segment);

    // Calculate control points for bezier curve
    FVector Direction = (Segment.EndPoint - Segment.StartPoint).GetSafeNormal();
    float ControlDistance = Segment.Length / 3.0f;

    Segment.ControlPoint1 = Segment.StartPoint + Direction * ControlDistance;
    Segment.ControlPoint2 = Segment.EndPoint - Direction * ControlDistance;

    // Apply style-specific properties
    ApplyStyleToSegment(Segment, Params.Style);

    // Calculate speeds
    float BaseSpeed = 200.0f;

    switch (Segment.Type)
    {
        case EMGSegmentType::Straight:
            Segment.MaxSpeed = BaseSpeed * 1.5f;
            Segment.SuggestedSpeed = BaseSpeed * 1.3f;
            break;
        case EMGSegmentType::GentleCurve:
            Segment.MaxSpeed = BaseSpeed * 1.2f;
            Segment.SuggestedSpeed = BaseSpeed * 1.0f;
            break;
        case EMGSegmentType::SharpCurve:
            Segment.MaxSpeed = BaseSpeed * 0.9f;
            Segment.SuggestedSpeed = BaseSpeed * 0.7f;
            break;
        case EMGSegmentType::Hairpin:
            Segment.MaxSpeed = BaseSpeed * 0.6f;
            Segment.SuggestedSpeed = BaseSpeed * 0.4f;
            break;
        case EMGSegmentType::Chicane:
            Segment.MaxSpeed = BaseSpeed * 0.8f;
            Segment.SuggestedSpeed = BaseSpeed * 0.6f;
            break;
        default:
            Segment.MaxSpeed = BaseSpeed;
            Segment.SuggestedSpeed = BaseSpeed * 0.8f;
            break;
    }

    // Drift potential
    Segment.DriftPotential = FMath::Clamp(FMath::Abs(CurveAngle) / 180.0f, 0.0f, 1.0f);

    // Shortcut potential
    if (RandomStream.FRand() < Params.ShortcutChance &&
        (Segment.Type == EMGSegmentType::SShape || Segment.Type == EMGSegmentType::GentleCurve))
    {
        Segment.bHasShortcut = true;
    }

    // Hazards
    if (RandomStream.FRand() < Params.HazardDensity && StyleParams.PossibleHazards.Num() > 0)
    {
        int32 HazardIndex = RandomStream.RandRange(0, StyleParams.PossibleHazards.Num() - 1);
        Segment.Hazards.Add(StyleParams.PossibleHazards[HazardIndex]);
    }

    // Scenic elements
    if (RandomStream.FRand() < Params.ScenicDensity && StyleParams.PreferredScenic.Num() > 0)
    {
        int32 ScenicIndex = RandomStream.RandRange(0, StyleParams.PreferredScenic.Num() - 1);
        Segment.ScenicElement = StyleParams.PreferredScenic[ScenicIndex];
    }

    return Segment;
}

EMGSegmentType UMGRouteGeneratorSubsystem::ChooseNextSegmentType(const FMGRouteParams& Params,
                                                                  EMGSegmentType Previous)
{
    // Get style params
    FMGRouteStyleParams StyleParams = GetStyleParams(Params.Style);

    // Don't follow hairpin with another hairpin
    if (Previous == EMGSegmentType::Hairpin)
    {
        return EMGSegmentType::Straight;
    }

    // Random selection based on preferences
    float Roll = RandomStream.FRand();

    if (Roll < Params.StraightPreference)
    {
        return EMGSegmentType::Straight;
    }

    Roll = RandomStream.FRand();

    if (Roll < Params.HairpinChance)
    {
        return EMGSegmentType::Hairpin;
    }
    else if (Roll < Params.HairpinChance + Params.SharpCurveChance)
    {
        return EMGSegmentType::SharpCurve;
    }
    else if (Roll < Params.CurveFrequency)
    {
        // Choose from preferred segments
        if (StyleParams.PreferredSegments.Num() > 0)
        {
            int32 Index = RandomStream.RandRange(0, StyleParams.PreferredSegments.Num() - 1);
            return StyleParams.PreferredSegments[Index];
        }
        return EMGSegmentType::GentleCurve;
    }

    // S-curve or chicane occasionally
    if (RandomStream.FRand() < 0.15f)
    {
        return RandomStream.FRand() < 0.5f ? EMGSegmentType::SShape : EMGSegmentType::Chicane;
    }

    // Jump if allowed
    if (Params.bAllowJumps && RandomStream.FRand() < 0.05f)
    {
        return EMGSegmentType::Jump;
    }

    return EMGSegmentType::GentleCurve;
}

FVector UMGRouteGeneratorSubsystem::CalculateSegmentEndPoint(const FMGRouteSegment& Segment)
{
    // Calculate end point based on segment type and properties
    FVector Direction = FVector::ForwardVector;

    if (Segment.SegmentIndex > 0)
    {
        // Use direction from previous segment
        Direction = (Segment.StartPoint - Segment.ControlPoint2).GetSafeNormal();
        if (Direction.IsNearlyZero())
        {
            Direction = FVector::ForwardVector;
        }
    }

    // Apply curve rotation
    FRotator Rotation = FRotator(0.0f, Segment.CurveAngle, 0.0f);
    Direction = Rotation.RotateVector(Direction);

    // Calculate end point
    FVector EndPoint = Segment.StartPoint + Direction * Segment.Length;

    // Apply elevation
    EndPoint.Z = Segment.Elevation;

    return EndPoint;
}

void UMGRouteGeneratorSubsystem::GenerateCheckpoints(FMGGeneratedRoute& Route)
{
    if (Route.Segments.Num() == 0)
    {
        return;
    }

    float TotalDistance = 0.0f;
    float CheckpointInterval = Route.TotalLength / 10.0f; // Roughly 10 checkpoints
    CheckpointInterval = FMath::Clamp(CheckpointInterval, 200.0f, 1000.0f);

    float NextCheckpointDistance = CheckpointInterval;
    int32 CheckpointIndex = 0;

    for (int32 i = 0; i < Route.Segments.Num(); ++i)
    {
        const FMGRouteSegment& Segment = Route.Segments[i];

        while (TotalDistance + Segment.Length >= NextCheckpointDistance &&
               NextCheckpointDistance < Route.TotalLength)
        {
            float T = (NextCheckpointDistance - TotalDistance) / Segment.Length;
            T = FMath::Clamp(T, 0.0f, 1.0f);

            FMGRouteCheckpoint Checkpoint;
            Checkpoint.CheckpointIndex = CheckpointIndex;
            Checkpoint.Location = BezierPoint(Segment.StartPoint, Segment.ControlPoint1,
                                              Segment.ControlPoint2, Segment.EndPoint, T);
            Checkpoint.Rotation = CalculateSegmentRotation(Segment, T);
            Checkpoint.Width = Segment.Width;
            Checkpoint.DistanceFromStart = NextCheckpointDistance;
            Checkpoint.SuggestedSpeed = Segment.SuggestedSpeed;
            Checkpoint.bIsSector = (CheckpointIndex % 3 == 0);
            Checkpoint.TimeExtension = 30.0f;

            Route.Checkpoints.Add(Checkpoint);

            CheckpointIndex++;
            NextCheckpointDistance += CheckpointInterval;
        }

        TotalDistance += Segment.Length;
    }

    // Add finish line checkpoint
    if (Route.Segments.Num() > 0)
    {
        FMGRouteCheckpoint Finish;
        Finish.CheckpointIndex = CheckpointIndex;

        if (Route.bIsCircuit)
        {
            Finish.Location = Route.Segments[0].StartPoint;
        }
        else
        {
            Finish.Location = Route.Segments.Last().EndPoint;
        }

        Finish.DistanceFromStart = Route.TotalLength;
        Finish.bIsFinishLine = true;
        Finish.Width = Route.Segments[0].Width;

        Route.Checkpoints.Add(Finish);
    }
}

void UMGRouteGeneratorSubsystem::GenerateSpawnPoints(FMGGeneratedRoute& Route, int32 MaxSpawns)
{
    if (Route.Segments.Num() == 0)
    {
        return;
    }

    const FMGRouteSegment& FirstSegment = Route.Segments[0];

    // Calculate spawn grid
    float LaneWidth = FirstSegment.Width / 3.0f;
    float RowSpacing = 10.0f;

    FVector ForwardDir = (FirstSegment.EndPoint - FirstSegment.StartPoint).GetSafeNormal();
    FVector RightDir = FVector::CrossProduct(ForwardDir, FVector::UpVector).GetSafeNormal();

    for (int32 i = 0; i < MaxSpawns && i < 12; ++i)
    {
        int32 Row = i / 2;
        int32 Lane = i % 2;

        FMGRouteSpawnPoint Spawn;
        Spawn.GridPosition = i + 1;
        Spawn.bIsStartingGrid = true;

        // Stagger positions
        FVector Offset = -ForwardDir * (Row * RowSpacing);
        Offset += RightDir * ((Lane == 0 ? -1.0f : 1.0f) * LaneWidth);

        Spawn.Location = FirstSegment.StartPoint + Offset;
        Spawn.Location.Z = FirstSegment.Elevation;
        Spawn.Rotation = ForwardDir.Rotation();
        Spawn.DistanceFromStart = -Row * RowSpacing;

        Route.SpawnPoints.Add(Spawn);
    }
}

void UMGRouteGeneratorSubsystem::GenerateShortcuts(FMGGeneratedRoute& Route, const FMGRouteParams& Params)
{
    for (int32 i = 0; i < Route.Segments.Num(); ++i)
    {
        if (Route.Segments[i].bHasShortcut)
        {
            // Find a suitable exit point
            int32 ExitIndex = FMath::Min(i + RandomStream.RandRange(2, 5), Route.Segments.Num() - 1);

            if (ExitIndex <= i)
            {
                continue;
            }

            FMGShortcut Shortcut;
            Shortcut.ShortcutId = FGuid::NewGuid();
            Shortcut.Name = FString::Printf(TEXT("Shortcut %d"), Route.Shortcuts.Num() + 1);
            Shortcut.EntrySegmentIndex = i;
            Shortcut.ExitSegmentIndex = ExitIndex;

            // Calculate path
            FVector EntryPoint = Route.Segments[i].StartPoint +
                                 (Route.Segments[i].EndPoint - Route.Segments[i].StartPoint) * 0.5f;
            FVector ExitPoint = Route.Segments[ExitIndex].StartPoint +
                                (Route.Segments[ExitIndex].EndPoint - Route.Segments[ExitIndex].StartPoint) * 0.5f;

            Shortcut.PathPoints.Add(EntryPoint);
            Shortcut.PathPoints.Add((EntryPoint + ExitPoint) * 0.5f + FVector(0, 0, 5));
            Shortcut.PathPoints.Add(ExitPoint);

            // Calculate time saved (rough estimate)
            float MainRouteLength = 0.0f;
            for (int32 j = i; j < ExitIndex; ++j)
            {
                MainRouteLength += Route.Segments[j].Length;
            }
            float ShortcutLength = FVector::Dist(EntryPoint, ExitPoint);
            Shortcut.TimeSaved = (MainRouteLength - ShortcutLength) / 50.0f; // Assume 50 units/sec

            Shortcut.RiskLevel = RandomStream.FRandRange(0.3f, 0.8f);
            Shortcut.Surface = EMGSurfaceType::Gravel;
            Shortcut.bRequiresJump = RandomStream.FRand() < 0.3f;
            Shortcut.bIsHidden = RandomStream.FRand() < 0.2f;

            Route.Shortcuts.Add(Shortcut);
        }
    }
}

void UMGRouteGeneratorSubsystem::CalculateRouteMetrics(FMGGeneratedRoute& Route)
{
    Route.TotalLength = 0.0f;
    Route.TotalCurves = 0;
    Route.SharpCurves = 0;
    Route.Hairpins = 0;
    Route.MaxElevation = -FLT_MAX;
    Route.MinElevation = FLT_MAX;
    Route.TotalElevationGain = 0.0f;
    Route.BoundsMin = FVector(FLT_MAX);
    Route.BoundsMax = FVector(-FLT_MAX);

    float TotalWidth = 0.0f;

    for (const FMGRouteSegment& Segment : Route.Segments)
    {
        Route.TotalLength += Segment.Length;
        TotalWidth += Segment.Width;

        // Track elevation
        Route.MaxElevation = FMath::Max(Route.MaxElevation, Segment.Elevation);
        Route.MinElevation = FMath::Min(Route.MinElevation, Segment.Elevation);

        if (Segment.ElevationChange > 0)
        {
            Route.TotalElevationGain += Segment.ElevationChange;
        }

        // Count curves
        switch (Segment.Type)
        {
            case EMGSegmentType::GentleCurve:
            case EMGSegmentType::SShape:
            case EMGSegmentType::Chicane:
                Route.TotalCurves++;
                break;
            case EMGSegmentType::SharpCurve:
                Route.TotalCurves++;
                Route.SharpCurves++;
                break;
            case EMGSegmentType::Hairpin:
                Route.TotalCurves++;
                Route.SharpCurves++;
                Route.Hairpins++;
                break;
            default:
                break;
        }

        // Update bounds
        Route.BoundsMin.X = FMath::Min(Route.BoundsMin.X, FMath::Min(Segment.StartPoint.X, Segment.EndPoint.X));
        Route.BoundsMin.Y = FMath::Min(Route.BoundsMin.Y, FMath::Min(Segment.StartPoint.Y, Segment.EndPoint.Y));
        Route.BoundsMin.Z = FMath::Min(Route.BoundsMin.Z, FMath::Min(Segment.StartPoint.Z, Segment.EndPoint.Z));
        Route.BoundsMax.X = FMath::Max(Route.BoundsMax.X, FMath::Max(Segment.StartPoint.X, Segment.EndPoint.X));
        Route.BoundsMax.Y = FMath::Max(Route.BoundsMax.Y, FMath::Max(Segment.StartPoint.Y, Segment.EndPoint.Y));
        Route.BoundsMax.Z = FMath::Max(Route.BoundsMax.Z, FMath::Max(Segment.StartPoint.Z, Segment.EndPoint.Z));
    }

    if (Route.Segments.Num() > 0)
    {
        Route.AverageWidth = TotalWidth / Route.Segments.Num();
    }

    // Estimate race time (assuming average speed of 150 km/h)
    Route.EstimatedTime = Route.TotalLength / 41.67f; // 150 km/h = 41.67 m/s

    // Calculate difficulty rating
    float DifficultyScore = 0.0f;
    DifficultyScore += (float)Route.SharpCurves / 10.0f;
    DifficultyScore += (float)Route.Hairpins * 0.3f;
    DifficultyScore += Route.TotalElevationGain / 500.0f;
    DifficultyScore += (15.0f - Route.AverageWidth) / 10.0f;

    Route.DifficultyRating = FMath::Clamp(DifficultyScore, 0.0f, 1.0f);
}

void UMGRouteGeneratorSubsystem::ApplyStyleToSegment(FMGRouteSegment& Segment, EMGRouteStyle Style)
{
    if (FMGRouteStyleParams* StyleParams = StyleConfigs.Find(Style))
    {
        // Apply surface
        if (StyleParams->PreferredSurfaces.Num() > 0)
        {
            int32 SurfaceIndex = RandomStream.RandRange(0, StyleParams->PreferredSurfaces.Num() - 1);
            Segment.Surface = StyleParams->PreferredSurfaces[SurfaceIndex];
        }
    }
}

bool UMGRouteGeneratorSubsystem::SaveRoute(const FMGGeneratedRoute& Route, const FString& SlotName)
{
    // In a real implementation, serialize to file
    UE_LOG(LogTemp, Log, TEXT("RouteGenerator: Saved route '%s' to slot '%s'"),
           *Route.RouteName, *SlotName);
    return true;
}

FMGGeneratedRoute UMGRouteGeneratorSubsystem::LoadRoute(const FString& SlotName)
{
    // In a real implementation, deserialize from file
    return FMGGeneratedRoute();
}

bool UMGRouteGeneratorSubsystem::DeleteRoute(const FString& SlotName)
{
    return true;
}

TArray<FString> UMGRouteGeneratorSubsystem::GetSavedRouteNames() const
{
    return TArray<FString>();
}

FMGGeneratedRoute UMGRouteGeneratorSubsystem::GetCurrentRoute() const
{
    return CurrentRoute;
}

void UMGRouteGeneratorSubsystem::SetCurrentRoute(const FMGGeneratedRoute& Route)
{
    CurrentRoute = Route;
    bHasRoute = true;
    CachedRacingLine.Empty();
}

bool UMGRouteGeneratorSubsystem::HasCurrentRoute() const
{
    return bHasRoute;
}

FMGRouteSegment UMGRouteGeneratorSubsystem::GetSegmentAtDistance(float Distance) const
{
    int32 Index = GetSegmentIndexAtDistance(Distance);
    if (CurrentRoute.Segments.IsValidIndex(Index))
    {
        return CurrentRoute.Segments[Index];
    }
    return FMGRouteSegment();
}

int32 UMGRouteGeneratorSubsystem::GetSegmentIndexAtDistance(float Distance) const
{
    float Accumulated = 0.0f;

    for (int32 i = 0; i < CurrentRoute.Segments.Num(); ++i)
    {
        Accumulated += CurrentRoute.Segments[i].Length;
        if (Accumulated >= Distance)
        {
            return i;
        }
    }

    return CurrentRoute.Segments.Num() - 1;
}

FVector UMGRouteGeneratorSubsystem::GetPointOnRoute(float Distance) const
{
    if (!bHasRoute || CurrentRoute.Segments.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    float Accumulated = 0.0f;

    for (const FMGRouteSegment& Segment : CurrentRoute.Segments)
    {
        if (Accumulated + Segment.Length >= Distance)
        {
            float T = (Distance - Accumulated) / Segment.Length;
            return BezierPoint(Segment.StartPoint, Segment.ControlPoint1,
                              Segment.ControlPoint2, Segment.EndPoint, T);
        }
        Accumulated += Segment.Length;
    }

    return CurrentRoute.Segments.Last().EndPoint;
}

FRotator UMGRouteGeneratorSubsystem::GetRotationOnRoute(float Distance) const
{
    if (!bHasRoute || CurrentRoute.Segments.Num() == 0)
    {
        return FRotator::ZeroRotator;
    }

    float Accumulated = 0.0f;

    for (const FMGRouteSegment& Segment : CurrentRoute.Segments)
    {
        if (Accumulated + Segment.Length >= Distance)
        {
            float T = (Distance - Accumulated) / Segment.Length;
            return CalculateSegmentRotation(Segment, T);
        }
        Accumulated += Segment.Length;
    }

    const FMGRouteSegment& LastSegment = CurrentRoute.Segments.Last();
    return (LastSegment.EndPoint - LastSegment.StartPoint).GetSafeNormal().Rotation();
}

float UMGRouteGeneratorSubsystem::GetWidthAtDistance(float Distance) const
{
    FMGRouteSegment Segment = GetSegmentAtDistance(Distance);
    return Segment.Width;
}

EMGSurfaceType UMGRouteGeneratorSubsystem::GetSurfaceAtDistance(float Distance) const
{
    FMGRouteSegment Segment = GetSegmentAtDistance(Distance);
    return Segment.Surface;
}

FMGRouteCheckpoint UMGRouteGeneratorSubsystem::GetCheckpoint(int32 Index) const
{
    if (CurrentRoute.Checkpoints.IsValidIndex(Index))
    {
        return CurrentRoute.Checkpoints[Index];
    }
    return FMGRouteCheckpoint();
}

int32 UMGRouteGeneratorSubsystem::GetCheckpointCount() const
{
    return CurrentRoute.Checkpoints.Num();
}

FMGRouteCheckpoint UMGRouteGeneratorSubsystem::GetNearestCheckpoint(const FVector& Location) const
{
    FMGRouteCheckpoint Nearest;
    float MinDist = FLT_MAX;

    for (const FMGRouteCheckpoint& Checkpoint : CurrentRoute.Checkpoints)
    {
        float Dist = FVector::DistSquared(Location, Checkpoint.Location);
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Nearest = Checkpoint;
        }
    }

    return Nearest;
}

float UMGRouteGeneratorSubsystem::GetDistanceToNextCheckpoint(float CurrentDistance) const
{
    for (const FMGRouteCheckpoint& Checkpoint : CurrentRoute.Checkpoints)
    {
        if (Checkpoint.DistanceFromStart > CurrentDistance)
        {
            return Checkpoint.DistanceFromStart - CurrentDistance;
        }
    }

    return 0.0f;
}

TArray<FMGRouteSpawnPoint> UMGRouteGeneratorSubsystem::GetStartingGrid(int32 MaxPositions) const
{
    TArray<FMGRouteSpawnPoint> Grid;

    for (const FMGRouteSpawnPoint& Spawn : CurrentRoute.SpawnPoints)
    {
        if (Spawn.bIsStartingGrid && Grid.Num() < MaxPositions)
        {
            Grid.Add(Spawn);
        }
    }

    return Grid;
}

FMGRouteSpawnPoint UMGRouteGeneratorSubsystem::GetSpawnPoint(int32 GridPosition) const
{
    for (const FMGRouteSpawnPoint& Spawn : CurrentRoute.SpawnPoints)
    {
        if (Spawn.GridPosition == GridPosition)
        {
            return Spawn;
        }
    }
    return FMGRouteSpawnPoint();
}

TArray<FMGShortcut> UMGRouteGeneratorSubsystem::GetShortcuts() const
{
    return CurrentRoute.Shortcuts;
}

FMGShortcut UMGRouteGeneratorSubsystem::GetNearestShortcut(const FVector& Location, float MaxDistance) const
{
    FMGShortcut Nearest;
    float MinDist = MaxDistance * MaxDistance;

    for (const FMGShortcut& Shortcut : CurrentRoute.Shortcuts)
    {
        if (Shortcut.PathPoints.Num() > 0)
        {
            float Dist = FVector::DistSquared(Location, Shortcut.PathPoints[0]);
            if (Dist < MinDist)
            {
                MinDist = Dist;
                Nearest = Shortcut;
            }
        }
    }

    return Nearest;
}

bool UMGRouteGeneratorSubsystem::IsOnShortcut(const FVector& Location, FGuid& OutShortcutId) const
{
    for (const FMGShortcut& Shortcut : CurrentRoute.Shortcuts)
    {
        for (const FVector& Point : Shortcut.PathPoints)
        {
            if (FVector::Dist(Location, Point) < 50.0f)
            {
                OutShortcutId = Shortcut.ShortcutId;
                return true;
            }
        }
    }
    return false;
}

TArray<FMGRacingLinePoint> UMGRouteGeneratorSubsystem::GenerateRacingLine(int32 Resolution)
{
    CachedRacingLine.Empty();

    if (!bHasRoute || CurrentRoute.Segments.Num() == 0)
    {
        return CachedRacingLine;
    }

    float StepDistance = CurrentRoute.TotalLength / Resolution;
    float CurrentDistance = 0.0f;

    for (int32 i = 0; i <= Resolution; ++i)
    {
        FMGRacingLinePoint Point;
        Point.Distance = CurrentDistance;
        Point.Location = GetPointOnRoute(CurrentDistance);

        // Calculate tangent
        FVector NextPoint = GetPointOnRoute(CurrentDistance + 1.0f);
        Point.Tangent = (NextPoint - Point.Location).GetSafeNormal();

        Point.Width = GetWidthAtDistance(CurrentDistance);

        // Get segment for speed info
        FMGRouteSegment Segment = GetSegmentAtDistance(CurrentDistance);
        Point.Speed = Segment.SuggestedSpeed;

        // Determine zones
        Point.bIsDriftZone = Segment.DriftPotential > 0.5f;
        Point.bIsBrakingZone = (Segment.Type == EMGSegmentType::SharpCurve ||
                                Segment.Type == EMGSegmentType::Hairpin);
        Point.bIsNitroZone = (Segment.Type == EMGSegmentType::Straight &&
                              Segment.Length > 200.0f);

        CachedRacingLine.Add(Point);
        CurrentDistance += StepDistance;
    }

    return CachedRacingLine;
}

FMGRacingLinePoint UMGRouteGeneratorSubsystem::GetRacingLinePoint(float Distance) const
{
    if (CachedRacingLine.Num() == 0)
    {
        return FMGRacingLinePoint();
    }

    // Find closest point
    for (int32 i = 0; i < CachedRacingLine.Num() - 1; ++i)
    {
        if (CachedRacingLine[i + 1].Distance >= Distance)
        {
            // Interpolate between points
            float T = (Distance - CachedRacingLine[i].Distance) /
                      (CachedRacingLine[i + 1].Distance - CachedRacingLine[i].Distance);

            FMGRacingLinePoint Result;
            Result.Distance = Distance;
            Result.Location = FMath::Lerp(CachedRacingLine[i].Location,
                                          CachedRacingLine[i + 1].Location, T);
            Result.Tangent = FMath::Lerp(CachedRacingLine[i].Tangent,
                                         CachedRacingLine[i + 1].Tangent, T).GetSafeNormal();
            Result.Width = FMath::Lerp(CachedRacingLine[i].Width,
                                       CachedRacingLine[i + 1].Width, T);
            Result.Speed = FMath::Lerp(CachedRacingLine[i].Speed,
                                       CachedRacingLine[i + 1].Speed, T);
            Result.bIsBrakingZone = CachedRacingLine[i].bIsBrakingZone;
            Result.bIsDriftZone = CachedRacingLine[i].bIsDriftZone;
            Result.bIsNitroZone = CachedRacingLine[i].bIsNitroZone;

            return Result;
        }
    }

    return CachedRacingLine.Last();
}

TArray<FMGRacingLinePoint> UMGRouteGeneratorSubsystem::GetRacingLine() const
{
    return CachedRacingLine;
}

void UMGRouteGeneratorSubsystem::SetStyleParams(EMGRouteStyle Style, const FMGRouteStyleParams& Params)
{
    StyleConfigs.Add(Style, Params);
}

FMGRouteStyleParams UMGRouteGeneratorSubsystem::GetStyleParams(EMGRouteStyle Style) const
{
    if (const FMGRouteStyleParams* Params = StyleConfigs.Find(Style))
    {
        return *Params;
    }
    return FMGRouteStyleParams();
}

bool UMGRouteGeneratorSubsystem::ValidateRoute(const FMGGeneratedRoute& Route, FString& OutError) const
{
    if (Route.Segments.Num() == 0)
    {
        OutError = TEXT("Route has no segments");
        return false;
    }

    if (Route.TotalLength < 100.0f)
    {
        OutError = TEXT("Route is too short");
        return false;
    }

    if (Route.Checkpoints.Num() < 2)
    {
        OutError = TEXT("Route needs at least 2 checkpoints");
        return false;
    }

    return true;
}

bool UMGRouteGeneratorSubsystem::IsLocationOnRoute(const FVector& Location, float Tolerance) const
{
    if (!bHasRoute)
    {
        return false;
    }

    for (const FMGRouteSegment& Segment : CurrentRoute.Segments)
    {
        // Simple distance check to segment
        FVector ClosestPoint = FMath::ClosestPointOnSegment(Location,
                                                            Segment.StartPoint,
                                                            Segment.EndPoint);
        if (FVector::Dist(Location, ClosestPoint) < Tolerance + Segment.Width / 2.0f)
        {
            return true;
        }
    }

    return false;
}

float UMGRouteGeneratorSubsystem::GetDistanceAlongRoute(const FVector& Location) const
{
    if (!bHasRoute || CurrentRoute.Segments.Num() == 0)
    {
        return 0.0f;
    }

    float Accumulated = 0.0f;
    float MinDist = FLT_MAX;
    float BestDistance = 0.0f;

    for (const FMGRouteSegment& Segment : CurrentRoute.Segments)
    {
        // Sample points along segment
        for (float T = 0.0f; T <= 1.0f; T += 0.1f)
        {
            FVector Point = BezierPoint(Segment.StartPoint, Segment.ControlPoint1,
                                        Segment.ControlPoint2, Segment.EndPoint, T);
            float Dist = FVector::DistSquared(Location, Point);
            if (Dist < MinDist)
            {
                MinDist = Dist;
                BestDistance = Accumulated + Segment.Length * T;
            }
        }
        Accumulated += Segment.Length;
    }

    return BestDistance;
}

FMGRouteParams UMGRouteGeneratorSubsystem::GetPresetParams(const FString& PresetName) const
{
    if (const FMGRouteParams* Params = RoutePresets.Find(PresetName))
    {
        return *Params;
    }
    return FMGRouteParams();
}

TArray<FString> UMGRouteGeneratorSubsystem::GetAvailablePresets() const
{
    TArray<FString> Names;
    RoutePresets.GetKeys(Names);
    return Names;
}

FVector UMGRouteGeneratorSubsystem::BezierPoint(const FVector& P0, const FVector& P1,
                                                 const FVector& P2, const FVector& P3, float T) const
{
    float OneMinusT = 1.0f - T;
    float OneMinusTSq = OneMinusT * OneMinusT;
    float TSq = T * T;

    return P0 * (OneMinusTSq * OneMinusT) +
           P1 * (3.0f * OneMinusTSq * T) +
           P2 * (3.0f * OneMinusT * TSq) +
           P3 * (TSq * T);
}

float UMGRouteGeneratorSubsystem::CalculateCurveLength(const FMGRouteSegment& Segment) const
{
    // Approximate bezier length
    float Length = 0.0f;
    FVector PrevPoint = Segment.StartPoint;

    for (float T = 0.1f; T <= 1.0f; T += 0.1f)
    {
        FVector Point = BezierPoint(Segment.StartPoint, Segment.ControlPoint1,
                                    Segment.ControlPoint2, Segment.EndPoint, T);
        Length += FVector::Dist(PrevPoint, Point);
        PrevPoint = Point;
    }

    return Length;
}

FRotator UMGRouteGeneratorSubsystem::CalculateSegmentRotation(const FMGRouteSegment& Segment, float T) const
{
    // Calculate tangent at point
    FVector Point1 = BezierPoint(Segment.StartPoint, Segment.ControlPoint1,
                                 Segment.ControlPoint2, Segment.EndPoint, FMath::Max(0.0f, T - 0.01f));
    FVector Point2 = BezierPoint(Segment.StartPoint, Segment.ControlPoint1,
                                 Segment.ControlPoint2, Segment.EndPoint, FMath::Min(1.0f, T + 0.01f));

    FVector Tangent = (Point2 - Point1).GetSafeNormal();

    FRotator Rotation = Tangent.Rotation();
    Rotation.Roll = Segment.Banking * FMath::Sin(T * PI); // Bank in middle of curve

    return Rotation;
}
