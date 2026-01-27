// MGProceduralContentSubsystem.cpp
// Procedural Content Generation System - Implementation
// Midnight Grind - Y2K Arcade Street Racing

#include "ProceduralContent/MGProceduralContentSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

void UMGProceduralContentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentMasterSeed = FDateTime::Now().ToUnixTimestamp();
    RandomStream.Initialize(static_cast<int32>(CurrentMasterSeed));
    CurrentQuality = EProceduralQuality::Medium;
    bIsGenerating = false;

    InitializeDefaultSettings();
    LoadSavedContent();

    UE_LOG(LogTemp, Log, TEXT("MGProceduralContentSubsystem initialized with seed: %lld"), CurrentMasterSeed);
}

void UMGProceduralContentSubsystem::Deinitialize()
{
    SaveContentToStorage();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AsyncGenerationTimer);
    }

    Super::Deinitialize();
}

void UMGProceduralContentSubsystem::InitializeDefaultSettings()
{
    DefaultSettings.Quality = EProceduralQuality::Medium;
    DefaultSettings.TargetDifficulty = EGenerationDifficulty::Medium;
    DefaultSettings.PreferredTheme = EEnvironmentTheme::UrbanDowntown;
    DefaultSettings.MinTrackLength = 2000.0f;
    DefaultSettings.MaxTrackLength = 8000.0f;
    DefaultSettings.MinSegments = 15;
    DefaultSettings.MaxSegments = 40;
    DefaultSettings.CurveFrequency = 0.4f;
    DefaultSettings.JumpFrequency = 0.1f;
    DefaultSettings.ShortcutFrequency = 0.15f;
    DefaultSettings.ObstacleDensity = 0.5f;
    DefaultSettings.CollectibleDensity = 0.3f;
    DefaultSettings.bAllowAlternateRoutes = true;
    DefaultSettings.bGenerateShortcuts = true;
    DefaultSettings.bGenerateSecretAreas = true;
    DefaultSettings.bGenerateTraffic = true;
    DefaultSettings.bDynamicWeather = false;
}

void UMGProceduralContentSubsystem::LoadSavedContent()
{
    // Initialize defaults first
    ContentStats.TotalTracksGenerated = 0;
    ContentStats.TotalEnvironmentsGenerated = 0;
    ContentStats.TotalChallengesGenerated = 0;
    ContentStats.TotalShortcutsDiscovered = 0;
    ContentStats.TotalSecretAreasFound = 0;
    ContentStats.AverageGenerationTime = 0.0f;
    ContentStats.TotalPlayTimeOnGenerated = 0.0f;
    ContentStats.FavoritedTracks = 0;
    ContentStats.SharedTracks = 0;

    ContentStats.ThemeUsageCounts.Add(TEXT("UrbanDowntown"), 0);
    ContentStats.ThemeUsageCounts.Add(TEXT("NeonAlley"), 0);
    ContentStats.ThemeUsageCounts.Add(TEXT("Y2KMall"), 0);
    ContentStats.ThemeUsageCounts.Add(TEXT("IndustrialDistrict"), 0);

    // Load from file
    FString SaveDir = FPaths::ProjectSavedDir() / TEXT("ProceduralContent");
    FString FilePath = SaveDir / TEXT("ProceduralContent.sav");

    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("No saved procedural content found, using defaults"));
        return;
    }

    FMemoryReader Archive(FileData, true);

    // Read version
    int32 Version = 0;
    Archive << Version;

    if (Version >= 1)
    {
        // Load master seed
        Archive << CurrentMasterSeed;

        // Load content stats
        Archive << ContentStats.TotalTracksGenerated;
        Archive << ContentStats.TotalEnvironmentsGenerated;
        Archive << ContentStats.TotalChallengesGenerated;
        Archive << ContentStats.TotalShortcutsDiscovered;
        Archive << ContentStats.TotalSecretAreasFound;
        Archive << ContentStats.AverageGenerationTime;
        Archive << ContentStats.TotalPlayTimeOnGenerated;
        Archive << ContentStats.FavoritedTracks;
        Archive << ContentStats.SharedTracks;

        // Load theme usage counts
        int32 ThemeCount = 0;
        Archive << ThemeCount;
        for (int32 i = 0; i < ThemeCount; i++)
        {
            FString ThemeName;
            int32 Count;
            Archive << ThemeName;
            Archive << Count;
            ContentStats.ThemeUsageCounts.FindOrAdd(ThemeName) = Count;
        }

        // Load saved tracks
        int32 SavedTrackCount = 0;
        Archive << SavedTrackCount;
        for (int32 i = 0; i < SavedTrackCount; i++)
        {
            FProceduralTrack Track;

            // Serialize track data
            Archive << Track.TrackId;
            Archive << Track.TrackName;
            Archive << Track.Seed.MasterSeed;
            Archive << Track.Seed.TrackSeed;
            Archive << Track.Seed.EnvironmentSeed;
            Archive << Track.Seed.SeedCode;
            Archive << Track.TotalLength;
            Archive << Track.EstimatedLapTime;
            Archive << Track.DifficultyScore;
            Archive << Track.JumpCount;
            Archive << Track.DriftZoneCount;
            Archive << Track.ShortcutCount;
            Archive << Track.bIsCircuit;

            int32 ThemeInt;
            Archive << ThemeInt;
            Track.Theme = static_cast<EEnvironmentTheme>(ThemeInt);

            int32 DiffInt;
            Archive << DiffInt;
            Track.Difficulty = static_cast<EGenerationDifficulty>(DiffInt);

            SavedTracks.Add(Track.TrackId, Track);
        }

        // Load favorite track IDs
        int32 FavoriteCount = 0;
        Archive << FavoriteCount;
        for (int32 i = 0; i < FavoriteCount; i++)
        {
            FGuid TrackId;
            Archive << TrackId;
            if (SavedTracks.Contains(TrackId))
            {
                FavoriteTracks.Add(TrackId, SavedTracks[TrackId]);
            }
        }

        // Load discovered shortcut IDs
        int32 ShortcutCount = 0;
        Archive << ShortcutCount;
        for (int32 i = 0; i < ShortcutCount; i++)
        {
            FGuid ShortcutId;
            Archive << ShortcutId;
            DiscoveredShortcutIds.Add(ShortcutId);
        }

        // Load track playtimes
        int32 PlaytimeCount = 0;
        Archive << PlaytimeCount;
        for (int32 i = 0; i < PlaytimeCount; i++)
        {
            FGuid TrackId;
            float Playtime;
            Archive << TrackId;
            Archive << Playtime;
            TrackPlaytimes.Add(TrackId, Playtime);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded procedural content: %d tracks, %d favorites"),
        SavedTracks.Num(), FavoriteTracks.Num());
}

void UMGProceduralContentSubsystem::SaveContentToStorage()
{
    FString SaveDir = FPaths::ProjectSavedDir() / TEXT("ProceduralContent");
    IFileManager::Get().MakeDirectory(*SaveDir, true);

    FString FilePath = SaveDir / TEXT("ProceduralContent.sav");

    FBufferArchive Archive;

    // Write version
    int32 Version = 1;
    Archive << Version;

    // Save master seed
    Archive << CurrentMasterSeed;

    // Save content stats
    Archive << ContentStats.TotalTracksGenerated;
    Archive << ContentStats.TotalEnvironmentsGenerated;
    Archive << ContentStats.TotalChallengesGenerated;
    Archive << ContentStats.TotalShortcutsDiscovered;
    Archive << ContentStats.TotalSecretAreasFound;
    Archive << ContentStats.AverageGenerationTime;
    Archive << ContentStats.TotalPlayTimeOnGenerated;
    Archive << ContentStats.FavoritedTracks;
    Archive << ContentStats.SharedTracks;

    // Save theme usage counts
    int32 ThemeCount = ContentStats.ThemeUsageCounts.Num();
    Archive << ThemeCount;
    for (const auto& Pair : ContentStats.ThemeUsageCounts)
    {
        FString ThemeName = Pair.Key;
        int32 Count = Pair.Value;
        Archive << ThemeName;
        Archive << Count;
    }

    // Save tracks
    int32 SavedTrackCount = SavedTracks.Num();
    Archive << SavedTrackCount;
    for (const auto& Pair : SavedTracks)
    {
        const FProceduralTrack& Track = Pair.Value;

        FGuid TrackId = Track.TrackId;
        FString TrackName = Track.TrackName;
        int64 MasterSeed = Track.Seed.MasterSeed;
        int32 TrackSeed = Track.Seed.TrackSeed;
        int32 EnvironmentSeed = Track.Seed.EnvironmentSeed;
        FString SeedCode = Track.Seed.SeedCode;
        float TotalLength = Track.TotalLength;
        float EstimatedLapTime = Track.EstimatedLapTime;
        float DifficultyScore = Track.DifficultyScore;
        int32 JumpCount = Track.JumpCount;
        int32 DriftZoneCount = Track.DriftZoneCount;
        int32 ShortcutCount = Track.ShortcutCount;
        bool bIsCircuit = Track.bIsCircuit;
        int32 ThemeInt = static_cast<int32>(Track.Theme);
        int32 DiffInt = static_cast<int32>(Track.Difficulty);

        Archive << TrackId;
        Archive << TrackName;
        Archive << MasterSeed;
        Archive << TrackSeed;
        Archive << EnvironmentSeed;
        Archive << SeedCode;
        Archive << TotalLength;
        Archive << EstimatedLapTime;
        Archive << DifficultyScore;
        Archive << JumpCount;
        Archive << DriftZoneCount;
        Archive << ShortcutCount;
        Archive << bIsCircuit;
        Archive << ThemeInt;
        Archive << DiffInt;
    }

    // Save favorite track IDs
    int32 FavoriteCount = FavoriteTracks.Num();
    Archive << FavoriteCount;
    for (const auto& Pair : FavoriteTracks)
    {
        FGuid TrackId = Pair.Key;
        Archive << TrackId;
    }

    // Save discovered shortcut IDs
    int32 ShortcutCount = DiscoveredShortcutIds.Num();
    Archive << ShortcutCount;
    for (const FGuid& ShortcutId : DiscoveredShortcutIds)
    {
        FGuid Id = ShortcutId;
        Archive << Id;
    }

    // Save track playtimes
    int32 PlaytimeCount = TrackPlaytimes.Num();
    Archive << PlaytimeCount;
    for (const auto& Pair : TrackPlaytimes)
    {
        FGuid TrackId = Pair.Key;
        float Playtime = Pair.Value;
        Archive << TrackId;
        Archive << Playtime;
    }

    // Write to file
    if (FFileHelper::SaveArrayToFile(Archive, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Saved procedural content: %d tracks, %d favorites to %s"),
            SavedTracks.Num(), FavoriteTracks.Num(), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save procedural content to %s"), *FilePath);
    }
}

// ============================================================================
// Seed Management
// ============================================================================

FProceduralSeed UMGProceduralContentSubsystem::GenerateRandomSeed()
{
    FProceduralSeed NewSeed;
    NewSeed.MasterSeed = FDateTime::Now().ToUnixTimestamp() ^ RandomStream.RandRange(0, INT32_MAX);
    NewSeed.TrackSeed = RandomStream.RandRange(0, INT32_MAX);
    NewSeed.EnvironmentSeed = RandomStream.RandRange(0, INT32_MAX);
    NewSeed.ObstacleSeed = RandomStream.RandRange(0, INT32_MAX);
    NewSeed.WeatherSeed = RandomStream.RandRange(0, INT32_MAX);
    NewSeed.TrafficSeed = RandomStream.RandRange(0, INT32_MAX);
    NewSeed.GeneratedAt = FDateTime::Now();

    // Generate shareable seed code
    NewSeed.SeedCode = FString::Printf(TEXT("MG-%08X-%08X-%04X"),
        static_cast<uint32>(NewSeed.MasterSeed & 0xFFFFFFFF),
        static_cast<uint32>(NewSeed.TrackSeed),
        static_cast<uint16>(NewSeed.EnvironmentSeed & 0xFFFF));

    return NewSeed;
}

FProceduralSeed UMGProceduralContentSubsystem::CreateSeedFromCode(const FString& SeedCode)
{
    FProceduralSeed ParsedSeed;
    ParsedSeed.SeedCode = SeedCode;
    ParsedSeed.GeneratedAt = FDateTime::Now();

    // Parse seed code format: MG-XXXXXXXX-XXXXXXXX-XXXX
    TArray<FString> Parts;
    SeedCode.ParseIntoArray(Parts, TEXT("-"));

    if (Parts.Num() >= 4)
    {
        ParsedSeed.MasterSeed = FCString::Strtoi64(*Parts[1], nullptr, 16);
        ParsedSeed.TrackSeed = FCString::Strtoi(*Parts[2], nullptr, 16);
        ParsedSeed.EnvironmentSeed = FCString::Strtoi(*Parts[3], nullptr, 16);

        // Derive other seeds from master
        FRandomStream TempStream(static_cast<int32>(ParsedSeed.MasterSeed));
        ParsedSeed.ObstacleSeed = TempStream.RandRange(0, INT32_MAX);
        ParsedSeed.WeatherSeed = TempStream.RandRange(0, INT32_MAX);
        ParsedSeed.TrafficSeed = TempStream.RandRange(0, INT32_MAX);
    }

    return ParsedSeed;
}

FString UMGProceduralContentSubsystem::GetSeedCode(const FProceduralSeed& Seed) const
{
    return Seed.SeedCode;
}

void UMGProceduralContentSubsystem::SetMasterSeed(int64 NewSeed)
{
    CurrentMasterSeed = NewSeed;
    RandomStream.Initialize(static_cast<int32>(NewSeed));
}

// ============================================================================
// Track Generation
// ============================================================================

FGenerationResult UMGProceduralContentSubsystem::GenerateTrack(const FGenerationSettings& Settings)
{
    FProceduralSeed NewSeed = GenerateRandomSeed();
    return GenerateTrackFromSeed(NewSeed, Settings);
}

FGenerationResult UMGProceduralContentSubsystem::GenerateTrackFromSeed(const FProceduralSeed& Seed, const FGenerationSettings& Settings)
{
    FGenerationResult Result;
    Result.bSuccess = false;

    if (bIsGenerating)
    {
        Result.ErrorMessage = TEXT("Generation already in progress");
        OnGenerationFailed.Broadcast(Result.ErrorMessage);
        return Result;
    }

    bIsGenerating = true;
    float StartTime = FPlatformTime::Seconds();

    OnGenerationProgress.Broadcast(0.0f, TEXT("Initializing generation"));

    // Initialize random stream with track seed
    FRandomStream TrackStream(Seed.TrackSeed);

    // Create track
    FProceduralTrack NewTrack;
    NewTrack.TrackId = FGuid::NewGuid();
    NewTrack.Seed = Seed;
    NewTrack.Theme = Settings.PreferredTheme;
    NewTrack.Difficulty = Settings.TargetDifficulty;
    NewTrack.GeneratedAt = FDateTime::Now();

    // Generate track name based on theme
    TArray<FString> ThemeNames;
    switch (Settings.PreferredTheme)
    {
        case EEnvironmentTheme::UrbanDowntown:
            ThemeNames = { TEXT("Downtown"), TEXT("City Center"), TEXT("Metro"), TEXT("Uptown") };
            break;
        case EEnvironmentTheme::NeonAlley:
            ThemeNames = { TEXT("Neon"), TEXT("Electric"), TEXT("Glow"), TEXT("Cyber") };
            break;
        case EEnvironmentTheme::Y2KMall:
            ThemeNames = { TEXT("Millennium"), TEXT("Y2K"), TEXT("Retro"), TEXT("2000s") };
            break;
        case EEnvironmentTheme::IndustrialDistrict:
            ThemeNames = { TEXT("Industrial"), TEXT("Factory"), TEXT("Warehouse"), TEXT("Steel") };
            break;
        default:
            ThemeNames = { TEXT("Street"), TEXT("Circuit"), TEXT("Track"), TEXT("Route") };
    }

    TArray<FString> TrackSuffixes = { TEXT("Sprint"), TEXT("Circuit"), TEXT("Run"), TEXT("Loop"), TEXT("Chase"), TEXT("Drift") };

    int32 NameIndex = TrackStream.RandRange(0, ThemeNames.Num() - 1);
    int32 SuffixIndex = TrackStream.RandRange(0, TrackSuffixes.Num() - 1);
    NewTrack.TrackName = FString::Printf(TEXT("%s %s"), *ThemeNames[NameIndex], *TrackSuffixes[SuffixIndex]);

    OnGenerationProgress.Broadcast(0.1f, TEXT("Generating segments"));

    // Calculate number of segments
    int32 SegmentCount = TrackStream.RandRange(Settings.MinSegments, Settings.MaxSegments);

    // Generate segments
    FVector CurrentPosition = FVector::ZeroVector;
    FRotator CurrentRotation = FRotator::ZeroRotator;
    ETrackSegmentType LastSegmentType = ETrackSegmentType::Straight;

    for (int32 i = 0; i < SegmentCount; i++)
    {
        OnGenerationProgress.Broadcast(0.1f + (0.5f * (float(i) / float(SegmentCount))),
            FString::Printf(TEXT("Generating segment %d/%d"), i + 1, SegmentCount));

        // Select segment type based on difficulty and last segment
        ETrackSegmentType SegmentType = SelectNextSegmentType(LastSegmentType, Settings.TargetDifficulty);

        // Generate segment
        FTrackSegment NewSegment = GenerateSegment(SegmentType, CurrentPosition, CurrentRotation);

        // Check for collisions with existing segments
        if (!CheckSegmentCollision(NewSegment, NewTrack.Segments))
        {
            NewTrack.Segments.Add(NewSegment);
            CurrentPosition = NewSegment.EndPosition;
            CurrentRotation = NewSegment.EndRotation;
            LastSegmentType = SegmentType;

            // Update track stats
            NewTrack.TotalLength += NewSegment.Length;
            if (SegmentType == ETrackSegmentType::Jump)
            {
                NewTrack.JumpCount++;
            }
            else if (SegmentType == ETrackSegmentType::DriftZone)
            {
                NewTrack.DriftZoneCount++;
            }
        }
    }

    // Close the circuit if needed
    if (Settings.bAllowAlternateRoutes || NewTrack.Segments.Num() > 5)
    {
        NewTrack.bIsCircuit = true;

        // Add closing segment to connect back to start
        FVector ClosingDirection = -CurrentPosition;
        ClosingDirection.Normalize();

        FTrackSegment ClosingSegment;
        ClosingSegment.SegmentId = FGuid::NewGuid();
        ClosingSegment.SegmentType = ETrackSegmentType::GentleCurve;
        ClosingSegment.StartPosition = CurrentPosition;
        ClosingSegment.EndPosition = FVector::ZeroVector;
        ClosingSegment.StartRotation = CurrentRotation;
        ClosingSegment.EndRotation = FRotator::ZeroRotator;
        ClosingSegment.Length = CurrentPosition.Size();
        ClosingSegment.Width = 12.0f;

        NewTrack.Segments.Add(ClosingSegment);
        NewTrack.TotalLength += ClosingSegment.Length;
    }

    OnGenerationProgress.Broadcast(0.6f, TEXT("Generating checkpoints"));
    GenerateCheckpoints(NewTrack);

    OnGenerationProgress.Broadcast(0.65f, TEXT("Generating spawn positions"));
    GenerateSpawnPositions(NewTrack);

    OnGenerationProgress.Broadcast(0.7f, TEXT("Generating shortcuts"));
    // Generate shortcuts if enabled
    if (Settings.bGenerateShortcuts)
    {
        int32 ShortcutCount = TrackStream.RandRange(1, 4);
        Result.GeneratedShortcuts = GenerateShortcuts(NewTrack, ShortcutCount);
        NewTrack.ShortcutCount = Result.GeneratedShortcuts.Num();
    }

    OnGenerationProgress.Broadcast(0.75f, TEXT("Generating environment"));
    // Generate environment
    Result.GeneratedEnvironment = GenerateEnvironment(Settings.PreferredTheme, NewTrack);

    OnGenerationProgress.Broadcast(0.8f, TEXT("Generating collectibles"));
    // Generate collectibles
    Result.GeneratedCollectibles = GenerateCollectibles(NewTrack, Settings.CollectibleDensity);

    OnGenerationProgress.Broadcast(0.85f, TEXT("Generating traffic patterns"));
    // Generate traffic if enabled
    if (Settings.bGenerateTraffic)
    {
        Result.GeneratedTrafficPattern = GenerateTrafficPattern(NewTrack, 0.5f);
    }

    OnGenerationProgress.Broadcast(0.9f, TEXT("Generating challenges"));
    // Generate challenges
    Result.GeneratedChallenges = GenerateChallengeSet(NewTrack, 3);

    // Calculate difficulty and estimate lap time
    NewTrack.DifficultyScore = CalculateTrackDifficulty(NewTrack);
    NewTrack.EstimatedLapTime = EstimateLapTime(NewTrack);

    // Calculate bounds
    FVector MinBounds = FVector(FLT_MAX);
    FVector MaxBounds = FVector(-FLT_MAX);
    FVector CenterSum = FVector::ZeroVector;

    for (const FTrackSegment& Segment : NewTrack.Segments)
    {
        MinBounds = MinBounds.ComponentMin(Segment.StartPosition);
        MaxBounds = MaxBounds.ComponentMax(Segment.StartPosition);
        CenterSum += Segment.StartPosition;
    }

    NewTrack.TrackBoundsMin = MinBounds;
    NewTrack.TrackBoundsMax = MaxBounds;
    NewTrack.TrackCenter = NewTrack.Segments.Num() > 0 ? CenterSum / NewTrack.Segments.Num() : FVector::ZeroVector;

    OnGenerationProgress.Broadcast(0.95f, TEXT("Finalizing track"));

    // Finalize result
    Result.GeneratedTrack = NewTrack;
    Result.bSuccess = true;
    Result.GenerationTime = FPlatformTime::Seconds() - StartTime;
    Result.TotalObjectsGenerated = NewTrack.Segments.Num() +
                                   Result.GeneratedShortcuts.Num() +
                                   Result.GeneratedCollectibles.Num() +
                                   Result.GeneratedEnvironment.Obstacles.Num();

    // Update stats
    ContentStats.TotalTracksGenerated++;
    ContentStats.TotalEnvironmentsGenerated++;
    ContentStats.TotalChallengesGenerated += Result.GeneratedChallenges.Num();

    float TotalTime = ContentStats.AverageGenerationTime * (ContentStats.TotalTracksGenerated - 1);
    ContentStats.AverageGenerationTime = (TotalTime + Result.GenerationTime) / ContentStats.TotalTracksGenerated;

    // Update theme usage
    FString ThemeName;
    switch (Settings.PreferredTheme)
    {
        case EEnvironmentTheme::UrbanDowntown: ThemeName = TEXT("UrbanDowntown"); break;
        case EEnvironmentTheme::NeonAlley: ThemeName = TEXT("NeonAlley"); break;
        case EEnvironmentTheme::Y2KMall: ThemeName = TEXT("Y2KMall"); break;
        case EEnvironmentTheme::IndustrialDistrict: ThemeName = TEXT("IndustrialDistrict"); break;
        default: ThemeName = TEXT("Other"); break;
    }
    if (ContentStats.ThemeUsageCounts.Contains(ThemeName))
    {
        ContentStats.ThemeUsageCounts[ThemeName]++;
    }

    bIsGenerating = false;

    OnGenerationProgress.Broadcast(1.0f, TEXT("Generation complete"));
    OnTrackGenerated.Broadcast(NewTrack);
    OnEnvironmentGenerated.Broadcast(Result.GeneratedEnvironment);
    OnGenerationComplete.Broadcast(Result);

    UE_LOG(LogTemp, Log, TEXT("Generated track '%s' with %d segments in %.3f seconds"),
        *NewTrack.TrackName, NewTrack.Segments.Num(), Result.GenerationTime);

    return Result;
}

void UMGProceduralContentSubsystem::GenerateTrackAsync(const FGenerationSettings& Settings)
{
    if (bIsGenerating)
    {
        OnGenerationFailed.Broadcast(TEXT("Generation already in progress"));
        return;
    }

    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGProceduralContentSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimerForNextTick([WeakThis, Settings]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->GenerateTrack(Settings);
            }
        });
    }
}

FTrackSegment UMGProceduralContentSubsystem::GenerateSegment(ETrackSegmentType SegmentType, const FVector& StartPos, const FRotator& StartRot)
{
    switch (SegmentType)
    {
        case ETrackSegmentType::Straight:
            return CreateStraightSegment(StartPos, StartRot, RandomStream.FRandRange(100.0f, 400.0f));

        case ETrackSegmentType::GentleCurve:
            return CreateCurveSegment(StartPos, StartRot, 200.0f, RandomStream.FRandRange(15.0f, 45.0f));

        case ETrackSegmentType::SharpCurve:
            return CreateCurveSegment(StartPos, StartRot, 100.0f, RandomStream.FRandRange(45.0f, 90.0f));

        case ETrackSegmentType::Hairpin:
            return CreateCurveSegment(StartPos, StartRot, 50.0f, RandomStream.FRandRange(135.0f, 180.0f));

        case ETrackSegmentType::Jump:
            return CreateJumpSegment(StartPos, StartRot, RandomStream.FRandRange(50.0f, 150.0f), RandomStream.FRandRange(5.0f, 20.0f));

        default:
            return CreateStraightSegment(StartPos, StartRot, 200.0f);
    }
}

TArray<FTrackSegment> UMGProceduralContentSubsystem::GenerateSegmentSequence(int32 SegmentCount, EGenerationDifficulty Difficulty)
{
    TArray<FTrackSegment> Segments;

    FVector CurrentPos = FVector::ZeroVector;
    FRotator CurrentRot = FRotator::ZeroRotator;
    ETrackSegmentType LastType = ETrackSegmentType::Straight;

    for (int32 i = 0; i < SegmentCount; i++)
    {
        ETrackSegmentType NextType = SelectNextSegmentType(LastType, Difficulty);
        FTrackSegment Segment = GenerateSegment(NextType, CurrentPos, CurrentRot);
        Segments.Add(Segment);

        CurrentPos = Segment.EndPosition;
        CurrentRot = Segment.EndRotation;
        LastType = NextType;
    }

    return Segments;
}

bool UMGProceduralContentSubsystem::ValidateTrack(const FProceduralTrack& Track)
{
    // Check minimum segment count
    if (Track.Segments.Num() < 5)
    {
        return false;
    }

    // Check minimum track length
    if (Track.TotalLength < 1000.0f)
    {
        return false;
    }

    // Check for valid connections
    for (int32 i = 0; i < Track.Segments.Num() - 1; i++)
    {
        float Distance = FVector::Distance(Track.Segments[i].EndPosition, Track.Segments[i + 1].StartPosition);
        if (Distance > 10.0f) // Allow small tolerance
        {
            return false;
        }
    }

    return true;
}

FProceduralTrack UMGProceduralContentSubsystem::OptimizeTrack(const FProceduralTrack& Track)
{
    FProceduralTrack OptimizedTrack = Track;

    // Remove redundant segments
    TArray<FTrackSegment> OptimizedSegments;
    for (int32 i = 0; i < OptimizedTrack.Segments.Num(); i++)
    {
        const FTrackSegment& Segment = OptimizedTrack.Segments[i];

        // Skip very short segments
        if (Segment.Length < 10.0f && i > 0 && i < OptimizedTrack.Segments.Num() - 1)
        {
            continue;
        }

        OptimizedSegments.Add(Segment);
    }

    OptimizedTrack.Segments = OptimizedSegments;

    // Recalculate total length
    OptimizedTrack.TotalLength = 0.0f;
    for (const FTrackSegment& Segment : OptimizedTrack.Segments)
    {
        OptimizedTrack.TotalLength += Segment.Length;
    }

    return OptimizedTrack;
}

// ============================================================================
// Environment Generation
// ============================================================================

FProceduralEnvironment UMGProceduralContentSubsystem::GenerateEnvironment(EEnvironmentTheme Theme, const FProceduralTrack& ForTrack)
{
    FProceduralEnvironment Environment;
    Environment.EnvironmentId = FGuid::NewGuid();
    Environment.Theme = Theme;

    // Random time of day (favor night for Y2K aesthetic)
    float TimeRoll = RandomStream.FRand();
    if (TimeRoll < 0.5f)
    {
        Environment.TimeOfDay = ETimeOfDay::Night;
    }
    else if (TimeRoll < 0.7f)
    {
        Environment.TimeOfDay = ETimeOfDay::Midnight;
    }
    else if (TimeRoll < 0.85f)
    {
        Environment.TimeOfDay = ETimeOfDay::Evening;
    }
    else
    {
        Environment.TimeOfDay = ETimeOfDay::Dusk;
    }

    // Set weather
    float WeatherRoll = RandomStream.FRand();
    if (WeatherRoll < 0.6f)
    {
        Environment.WeatherType = TEXT("Clear");
        Environment.WeatherIntensity = EWeatherIntensity::None;
        Environment.GripModifier = 1.0f;
    }
    else if (WeatherRoll < 0.8f)
    {
        Environment.WeatherType = TEXT("Rain");
        Environment.WeatherIntensity = EWeatherIntensity::Light;
        Environment.GripModifier = 0.85f;
    }
    else if (WeatherRoll < 0.95f)
    {
        Environment.WeatherType = TEXT("Rain");
        Environment.WeatherIntensity = EWeatherIntensity::Moderate;
        Environment.GripModifier = 0.7f;
    }
    else
    {
        Environment.WeatherType = TEXT("Storm");
        Environment.WeatherIntensity = EWeatherIntensity::Heavy;
        Environment.GripModifier = 0.55f;
    }

    // Theme-based lighting
    switch (Theme)
    {
        case EEnvironmentTheme::NeonAlley:
            Environment.AmbientLightColor = FLinearColor(0.1f, 0.05f, 0.2f);
            Environment.AmbientLightIntensity = 0.3f;
            Environment.FogColor = FLinearColor(0.2f, 0.0f, 0.3f);
            Environment.FogDensity = 0.02f;
            break;

        case EEnvironmentTheme::Y2KMall:
            Environment.AmbientLightColor = FLinearColor(0.8f, 0.7f, 1.0f);
            Environment.AmbientLightIntensity = 0.6f;
            Environment.FogColor = FLinearColor(0.9f, 0.85f, 1.0f);
            Environment.FogDensity = 0.005f;
            break;

        case EEnvironmentTheme::IndustrialDistrict:
            Environment.AmbientLightColor = FLinearColor(0.4f, 0.35f, 0.3f);
            Environment.AmbientLightIntensity = 0.4f;
            Environment.FogColor = FLinearColor(0.5f, 0.45f, 0.4f);
            Environment.FogDensity = 0.015f;
            break;

        case EEnvironmentTheme::CyberpunkSlums:
            Environment.AmbientLightColor = FLinearColor(0.0f, 0.15f, 0.2f);
            Environment.AmbientLightIntensity = 0.25f;
            Environment.FogColor = FLinearColor(0.0f, 0.1f, 0.15f);
            Environment.FogDensity = 0.025f;
            break;

        default: // UrbanDowntown
            Environment.AmbientLightColor = FLinearColor(0.2f, 0.18f, 0.25f);
            Environment.AmbientLightIntensity = 0.4f;
            Environment.FogColor = FLinearColor(0.3f, 0.25f, 0.35f);
            Environment.FogDensity = 0.01f;
    }

    // Generate lights
    Environment.LightPositions = GenerateLightPositions(ForTrack, Theme);

    // Generate Y2K neon palette
    Environment.NeonColors = GenerateNeonPalette(Theme, 6);

    // Generate obstacles
    Environment.Obstacles = GenerateObstacles(ForTrack, 0.5f);

    // Traffic and pedestrian density based on theme
    switch (Theme)
    {
        case EEnvironmentTheme::UrbanDowntown:
            Environment.TrafficDensity = 0.7f;
            Environment.PedestrianDensity = 0.5f;
            break;
        case EEnvironmentTheme::NeonAlley:
            Environment.TrafficDensity = 0.3f;
            Environment.PedestrianDensity = 0.6f;
            break;
        case EEnvironmentTheme::IndustrialDistrict:
            Environment.TrafficDensity = 0.4f;
            Environment.PedestrianDensity = 0.1f;
            break;
        default:
            Environment.TrafficDensity = 0.5f;
            Environment.PedestrianDensity = 0.3f;
    }

    return Environment;
}

TArray<FProceduralObstacle> UMGProceduralContentSubsystem::GenerateObstacles(const FProceduralTrack& Track, float Density)
{
    TArray<FProceduralObstacle> Obstacles;

    TArray<FString> ObstacleTypes = {
        TEXT("TrafficCone"),
        TEXT("Barrier"),
        TEXT("Dumpster"),
        TEXT("Crate"),
        TEXT("Mailbox"),
        TEXT("FireHydrant"),
        TEXT("TrashCan"),
        TEXT("Bench"),
        TEXT("StreetSign"),
        TEXT("ParkingMeter")
    };

    int32 ObstacleCount = FMath::RoundToInt(Track.Segments.Num() * Density * 3);

    for (int32 i = 0; i < ObstacleCount; i++)
    {
        if (Track.Segments.Num() == 0) break;

        FProceduralObstacle Obstacle;
        Obstacle.ObstacleId = FGuid::NewGuid();
        Obstacle.ObstacleType = ObstacleTypes[RandomStream.RandRange(0, ObstacleTypes.Num() - 1)];

        // Determine category
        if (Obstacle.ObstacleType == TEXT("TrafficCone") || Obstacle.ObstacleType == TEXT("Crate"))
        {
            Obstacle.Category = EObstacleCategory::Destructible;
            Obstacle.bDestructible = true;
            Obstacle.HealthPoints = 50;
        }
        else if (Obstacle.ObstacleType == TEXT("Barrier") || Obstacle.ObstacleType == TEXT("Dumpster"))
        {
            Obstacle.Category = EObstacleCategory::Static;
            Obstacle.bDestructible = false;
        }
        else
        {
            Obstacle.Category = EObstacleCategory::Decorative;
            Obstacle.bDestructible = false;
        }

        // Position near track edges
        int32 SegmentIndex = RandomStream.RandRange(0, Track.Segments.Num() - 1);
        const FTrackSegment& Segment = Track.Segments[SegmentIndex];

        float AlongTrack = RandomStream.FRand();
        FVector BasePosition = FMath::Lerp(Segment.StartPosition, Segment.EndPosition, AlongTrack);

        // Offset to side of track
        FVector TrackDirection = (Segment.EndPosition - Segment.StartPosition).GetSafeNormal();
        FVector SideOffset = FVector::CrossProduct(TrackDirection, FVector::UpVector);
        float SideDistance = (Segment.Width * 0.5f) + RandomStream.FRandRange(2.0f, 8.0f);

        if (RandomStream.FRand() > 0.5f)
        {
            SideOffset *= -1.0f;
        }

        Obstacle.Position = BasePosition + SideOffset * SideDistance;
        Obstacle.Rotation = FRotator(0.0f, RandomStream.FRandRange(0.0f, 360.0f), 0.0f);
        Obstacle.Scale = FVector::OneVector * RandomStream.FRandRange(0.8f, 1.2f);
        Obstacle.CollisionRadius = 30.0f + RandomStream.FRandRange(0.0f, 40.0f);
        Obstacle.DamageOnImpact = RandomStream.FRandRange(5.0f, 25.0f);
        Obstacle.SpeedPenalty = RandomStream.FRandRange(0.3f, 0.7f);

        Obstacles.Add(Obstacle);
    }

    return Obstacles;
}

TArray<FVector> UMGProceduralContentSubsystem::GenerateLightPositions(const FProceduralTrack& Track, EEnvironmentTheme Theme)
{
    TArray<FVector> LightPositions;

    float LightSpacing = 50.0f; // Distance between lights

    switch (Theme)
    {
        case EEnvironmentTheme::NeonAlley:
            LightSpacing = 30.0f;
            break;
        case EEnvironmentTheme::IndustrialDistrict:
            LightSpacing = 80.0f;
            break;
        default:
            LightSpacing = 50.0f;
    }

    for (const FTrackSegment& Segment : Track.Segments)
    {
        int32 LightsOnSegment = FMath::Max(1, FMath::RoundToInt(Segment.Length / LightSpacing));

        for (int32 i = 0; i < LightsOnSegment; i++)
        {
            float T = float(i) / float(LightsOnSegment);
            FVector LightPos = FMath::Lerp(Segment.StartPosition, Segment.EndPosition, T);

            // Offset to sides
            FVector Direction = (Segment.EndPosition - Segment.StartPosition).GetSafeNormal();
            FVector Side = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

            // Add lights on both sides
            LightPositions.Add(LightPos + Side * (Segment.Width * 0.5f + 3.0f) + FVector(0, 0, 500.0f));
            LightPositions.Add(LightPos - Side * (Segment.Width * 0.5f + 3.0f) + FVector(0, 0, 500.0f));
        }
    }

    return LightPositions;
}

TArray<FLinearColor> UMGProceduralContentSubsystem::GenerateNeonPalette(EEnvironmentTheme Theme, int32 ColorCount)
{
    TArray<FLinearColor> Palette;

    for (int32 i = 0; i < ColorCount; i++)
    {
        Palette.Add(GenerateY2KNeonColor());
    }

    // Add theme-specific colors
    switch (Theme)
    {
        case EEnvironmentTheme::NeonAlley:
            Palette.Add(FLinearColor(1.0f, 0.0f, 0.5f)); // Hot pink
            Palette.Add(FLinearColor(0.0f, 1.0f, 1.0f)); // Cyan
            break;
        case EEnvironmentTheme::Y2KMall:
            Palette.Add(FLinearColor(0.8f, 0.0f, 1.0f)); // Purple
            Palette.Add(FLinearColor(1.0f, 0.6f, 0.0f)); // Orange
            break;
        case EEnvironmentTheme::CyberpunkSlums:
            Palette.Add(FLinearColor(0.0f, 0.8f, 0.2f)); // Green
            Palette.Add(FLinearColor(1.0f, 0.0f, 0.0f)); // Red
            break;
        default:
            Palette.Add(FLinearColor(0.0f, 0.5f, 1.0f)); // Blue
            Palette.Add(FLinearColor(1.0f, 1.0f, 0.0f)); // Yellow
    }

    return Palette;
}

void UMGProceduralContentSubsystem::ApplyWeatherToEnvironment(FProceduralEnvironment& Environment, const FString& WeatherType, EWeatherIntensity Intensity)
{
    Environment.WeatherType = WeatherType;
    Environment.WeatherIntensity = Intensity;

    if (WeatherType == TEXT("Rain"))
    {
        switch (Intensity)
        {
            case EWeatherIntensity::Light:
                Environment.GripModifier = 0.9f;
                Environment.VisibilityDistance = 8000.0f;
                break;
            case EWeatherIntensity::Moderate:
                Environment.GripModifier = 0.75f;
                Environment.VisibilityDistance = 5000.0f;
                Environment.FogDensity *= 1.5f;
                break;
            case EWeatherIntensity::Heavy:
                Environment.GripModifier = 0.6f;
                Environment.VisibilityDistance = 3000.0f;
                Environment.FogDensity *= 2.0f;
                break;
            case EWeatherIntensity::Extreme:
                Environment.GripModifier = 0.45f;
                Environment.VisibilityDistance = 1500.0f;
                Environment.FogDensity *= 3.0f;
                break;
            default:
                break;
        }
    }
    else if (WeatherType == TEXT("Fog"))
    {
        Environment.GripModifier = 0.95f;
        Environment.FogDensity *= (2.0f + static_cast<float>(Intensity));
        Environment.VisibilityDistance = 10000.0f / (1.0f + static_cast<float>(Intensity));
    }
}

// ============================================================================
// Challenge Generation
// ============================================================================

FProceduralChallenge UMGProceduralContentSubsystem::GenerateChallenge(const FProceduralTrack& Track, EGenerationDifficulty Difficulty)
{
    FProceduralChallenge Challenge;
    Challenge.ChallengeId = FGuid::NewGuid();
    Challenge.Difficulty = Difficulty;
    Challenge.RequiredTrackId = Track.TrackId;

    TArray<FString> ChallengeTypes = {
        TEXT("TimeAttack"),
        TEXT("DriftScore"),
        TEXT("PerfectLap"),
        TEXT("Overtake"),
        TEXT("CollectAll"),
        TEXT("NoCollision"),
        TEXT("TopSpeed"),
        TEXT("AirTime")
    };

    int32 TypeIndex = RandomStream.RandRange(0, ChallengeTypes.Num() - 1);
    Challenge.ChallengeType = ChallengeTypes[TypeIndex];

    // Set difficulty multiplier
    float DiffMultiplier = 1.0f;
    switch (Difficulty)
    {
        case EGenerationDifficulty::VeryEasy: DiffMultiplier = 0.5f; break;
        case EGenerationDifficulty::Easy: DiffMultiplier = 0.75f; break;
        case EGenerationDifficulty::Medium: DiffMultiplier = 1.0f; break;
        case EGenerationDifficulty::Hard: DiffMultiplier = 1.5f; break;
        case EGenerationDifficulty::VeryHard: DiffMultiplier = 2.0f; break;
        case EGenerationDifficulty::Extreme: DiffMultiplier = 3.0f; break;
        case EGenerationDifficulty::Nightmare: DiffMultiplier = 5.0f; break;
    }

    // Configure based on type
    if (Challenge.ChallengeType == TEXT("TimeAttack"))
    {
        Challenge.ChallengeName = FString::Printf(TEXT("%s Time Attack"), *Track.TrackName);
        Challenge.ChallengeDescription = TEXT("Complete the track within the time limit");
        Challenge.TargetValue = Track.EstimatedLapTime * 0.9f / DiffMultiplier;
        Challenge.TimeLimit = Track.EstimatedLapTime * 1.5f / DiffMultiplier;
        Challenge.Objectives.Add(TEXT("FinishTime"), Challenge.TargetValue);
    }
    else if (Challenge.ChallengeType == TEXT("DriftScore"))
    {
        Challenge.ChallengeName = FString::Printf(TEXT("%s Drift Master"), *Track.TrackName);
        Challenge.ChallengeDescription = TEXT("Achieve the target drift score");
        Challenge.TargetValue = 50000.0f * DiffMultiplier;
        Challenge.TimeLimit = Track.EstimatedLapTime * 2.0f;
        Challenge.Objectives.Add(TEXT("DriftScore"), Challenge.TargetValue);
    }
    else if (Challenge.ChallengeType == TEXT("TopSpeed"))
    {
        Challenge.ChallengeName = FString::Printf(TEXT("%s Speed Demon"), *Track.TrackName);
        Challenge.ChallengeDescription = TEXT("Reach the target top speed");
        Challenge.TargetValue = 200.0f + (50.0f * DiffMultiplier);
        Challenge.TimeLimit = Track.EstimatedLapTime * 3.0f;
        Challenge.Objectives.Add(TEXT("TopSpeed"), Challenge.TargetValue);
    }
    else
    {
        Challenge.ChallengeName = FString::Printf(TEXT("%s Challenge"), *Track.TrackName);
        Challenge.ChallengeDescription = TEXT("Complete the challenge objectives");
        Challenge.TargetValue = 100.0f * DiffMultiplier;
        Challenge.TimeLimit = Track.EstimatedLapTime * 2.0f;
    }

    // Calculate rewards
    Challenge.RewardCredits = FMath::RoundToInt(500.0f * DiffMultiplier);
    Challenge.RewardXP = FMath::RoundToInt(100.0f * DiffMultiplier);

    // Add bonus reward items for harder difficulties
    if (Difficulty >= EGenerationDifficulty::Hard)
    {
        Challenge.RewardItems.Add(TEXT("RarePart_Random"));
    }
    if (Difficulty >= EGenerationDifficulty::Extreme)
    {
        Challenge.RewardItems.Add(TEXT("ExclusiveDecal_Procedural"));
    }

    ContentStats.TotalChallengesGenerated++;
    OnChallengeGenerated.Broadcast(Challenge);

    return Challenge;
}

TArray<FProceduralChallenge> UMGProceduralContentSubsystem::GenerateChallengeSet(const FProceduralTrack& Track, int32 ChallengeCount)
{
    TArray<FProceduralChallenge> Challenges;

    TArray<EGenerationDifficulty> Difficulties = {
        EGenerationDifficulty::Easy,
        EGenerationDifficulty::Medium,
        EGenerationDifficulty::Hard
    };

    for (int32 i = 0; i < ChallengeCount; i++)
    {
        EGenerationDifficulty Diff = Difficulties[FMath::Min(i, Difficulties.Num() - 1)];
        Challenges.Add(GenerateChallenge(Track, Diff));
    }

    return Challenges;
}

FProceduralChallenge UMGProceduralContentSubsystem::GenerateDailyChallenge()
{
    // Use date as seed for consistent daily challenge
    FDateTime Today = FDateTime::Today();
    int32 DailySeed = Today.GetYear() * 10000 + Today.GetMonth() * 100 + Today.GetDay();

    FRandomStream DailyStream(DailySeed);

    FGenerationSettings DailySettings = DefaultSettings;
    DailySettings.TargetDifficulty = EGenerationDifficulty::Medium;

    FGenerationResult Result = GenerateTrack(DailySettings);

    if (Result.bSuccess)
    {
        FProceduralChallenge DailyChallenge = GenerateChallenge(Result.GeneratedTrack, EGenerationDifficulty::Medium);
        DailyChallenge.ChallengeName = FString::Printf(TEXT("Daily Challenge - %s"), *Today.ToString(TEXT("%Y-%m-%d")));
        DailyChallenge.RewardCredits *= 2; // Bonus for daily
        DailyChallenge.RewardXP *= 2;
        return DailyChallenge;
    }

    return FProceduralChallenge();
}

TArray<FProceduralChallenge> UMGProceduralContentSubsystem::GenerateWeeklyChallenges()
{
    TArray<FProceduralChallenge> WeeklyChallenges;

    // Generate 7 challenges, one for each day
    for (int32 i = 0; i < 7; i++)
    {
        EGenerationDifficulty Diff = static_cast<EGenerationDifficulty>(FMath::Min(i / 2, static_cast<int32>(EGenerationDifficulty::VeryHard)));

        FGenerationSettings Settings = DefaultSettings;
        Settings.TargetDifficulty = Diff;

        FGenerationResult Result = GenerateTrack(Settings);
        if (Result.bSuccess)
        {
            FProceduralChallenge Challenge = GenerateChallenge(Result.GeneratedTrack, Diff);
            Challenge.ChallengeName = FString::Printf(TEXT("Weekly Challenge Day %d"), i + 1);
            WeeklyChallenges.Add(Challenge);
        }
    }

    return WeeklyChallenges;
}

// ============================================================================
// Shortcuts and Secrets
// ============================================================================

TArray<FProceduralShortcut> UMGProceduralContentSubsystem::GenerateShortcuts(const FProceduralTrack& Track, int32 MaxShortcuts)
{
    TArray<FProceduralShortcut> Shortcuts;

    if (Track.Segments.Num() < 10)
    {
        return Shortcuts; // Not enough segments for shortcuts
    }

    TArray<FString> ShortcutNames = {
        TEXT("Alley Cut"),
        TEXT("Parking Garage"),
        TEXT("Back Street"),
        TEXT("Loading Dock"),
        TEXT("Underground Pass"),
        TEXT("Rooftop Jump"),
        TEXT("Mall Bypass")
    };

    for (int32 i = 0; i < MaxShortcuts; i++)
    {
        // Find two segments that are close but not adjacent
        int32 StartSegIndex = RandomStream.RandRange(0, Track.Segments.Num() / 2);
        int32 EndSegIndex = RandomStream.RandRange(Track.Segments.Num() / 2 + 3, Track.Segments.Num() - 1);

        if (EndSegIndex - StartSegIndex < 5) continue; // Too close

        const FTrackSegment& StartSeg = Track.Segments[StartSegIndex];
        const FTrackSegment& EndSeg = Track.Segments[EndSegIndex];

        FProceduralShortcut Shortcut;
        Shortcut.ShortcutId = FGuid::NewGuid();
        Shortcut.ShortcutName = ShortcutNames[RandomStream.RandRange(0, ShortcutNames.Num() - 1)];
        Shortcut.EntryPoint = StartSeg.EndPosition;
        Shortcut.ExitPoint = EndSeg.StartPosition;

        // Calculate time saved based on skipped segments
        float SkippedLength = 0.0f;
        for (int32 j = StartSegIndex + 1; j < EndSegIndex; j++)
        {
            SkippedLength += Track.Segments[j].Length;
        }

        float ShortcutLength = FVector::Distance(Shortcut.EntryPoint, Shortcut.ExitPoint);
        float AverageSpeed = 150.0f; // km/h estimate
        Shortcut.TimeSaved = (SkippedLength - ShortcutLength) / AverageSpeed;

        Shortcut.RiskLevel = RandomStream.FRandRange(0.3f, 0.9f);
        Shortcut.MinimumSpeedRequired = 80.0f + (Shortcut.RiskLevel * 100.0f);
        Shortcut.bRequiresJump = RandomStream.FRand() > 0.7f;
        Shortcut.bRequiresDrift = RandomStream.FRand() > 0.8f;
        Shortcut.bHidden = RandomStream.FRand() > 0.6f;
        Shortcut.DiscoveryPoints = Shortcut.bHidden ? 500 : 100;

        // Generate path points
        int32 PathPointCount = RandomStream.RandRange(3, 6);
        for (int32 p = 0; p < PathPointCount; p++)
        {
            float T = float(p) / float(PathPointCount - 1);
            FVector PathPoint = FMath::Lerp(Shortcut.EntryPoint, Shortcut.ExitPoint, T);

            // Add some variation
            PathPoint.X += RandomStream.FRandRange(-20.0f, 20.0f);
            PathPoint.Y += RandomStream.FRandRange(-20.0f, 20.0f);

            Shortcut.PathPoints.Add(PathPoint);
        }

        if (IsValidShortcut(Shortcut, Track))
        {
            Shortcuts.Add(Shortcut);
        }
    }

    return Shortcuts;
}

bool UMGProceduralContentSubsystem::IsValidShortcut(const FProceduralShortcut& Shortcut, const FProceduralTrack& Track)
{
    // Check that shortcut actually saves time
    if (Shortcut.TimeSaved <= 0.0f)
    {
        return false;
    }

    // Check shortcut isn't too long
    float ShortcutLength = FVector::Distance(Shortcut.EntryPoint, Shortcut.ExitPoint);
    if (ShortcutLength > Track.TotalLength * 0.3f)
    {
        return false;
    }

    // Check path doesn't intersect too many segments
    // (simplified check)
    return true;
}

void UMGProceduralContentSubsystem::DiscoverShortcut(const FGuid& ShortcutId)
{
    if (!DiscoveredShortcutIds.Contains(ShortcutId))
    {
        DiscoveredShortcutIds.Add(ShortcutId);
        ContentStats.TotalShortcutsDiscovered++;

        // Find and broadcast the shortcut
        for (const auto& TrackPair : SavedTracks)
        {
            // Shortcut discovery notification would be handled here
        }
    }
}

TArray<FProceduralShortcut> UMGProceduralContentSubsystem::GetDiscoveredShortcuts() const
{
    TArray<FProceduralShortcut> Discovered;
    // Return discovered shortcuts from saved tracks
    return Discovered;
}

// ============================================================================
// Traffic Generation
// ============================================================================

FTrafficPattern UMGProceduralContentSubsystem::GenerateTrafficPattern(const FProceduralTrack& Track, float Density)
{
    FTrafficPattern Pattern;
    Pattern.PatternId = FGuid::NewGuid();
    Pattern.PatternName = TEXT("Generated Traffic");
    Pattern.Density = Density;
    Pattern.AverageSpeed = 50.0f + RandomStream.FRandRange(-10.0f, 10.0f);
    Pattern.SpeedVariation = 15.0f;
    Pattern.AggressivenessLevel = RandomStream.FRandRange(0.1f, 0.5f);

    // Vehicle types
    Pattern.VehicleTypes = {
        TEXT("Sedan"),
        TEXT("SUV"),
        TEXT("Truck"),
        TEXT("Van"),
        TEXT("SportsCar"),
        TEXT("Taxi")
    };

    // Generate spawn points along track
    for (const FTrackSegment& Segment : Track.Segments)
    {
        if (RandomStream.FRand() < Density * 0.5f)
        {
            FVector SpawnPoint = (Segment.StartPosition + Segment.EndPosition) * 0.5f;
            // Offset to opposite lane
            FVector Direction = (Segment.EndPosition - Segment.StartPosition).GetSafeNormal();
            FVector LaneOffset = FVector::CrossProduct(Direction, FVector::UpVector) * (Segment.Width * 0.3f);

            Pattern.SpawnPoints.Add(SpawnPoint + LaneOffset);
            Pattern.DespawnPoints.Add(SpawnPoint - LaneOffset * 2.0f);
        }
    }

    // Lane distribution
    Pattern.LaneDistribution.Add(TEXT("Left"), 0.3f);
    Pattern.LaneDistribution.Add(TEXT("Center"), 0.4f);
    Pattern.LaneDistribution.Add(TEXT("Right"), 0.3f);

    return Pattern;
}

TArray<FVector> UMGProceduralContentSubsystem::GetTrafficSpawnPoints(const FTrafficPattern& Pattern) const
{
    return Pattern.SpawnPoints;
}

void UMGProceduralContentSubsystem::UpdateTrafficPattern(FTrafficPattern& Pattern, float DeltaTime)
{
    // Dynamic traffic pattern updates would go here
    // Adjust density based on race state, time, etc.
}

// ============================================================================
// Collectibles
// ============================================================================

TArray<FProceduralCollectible> UMGProceduralContentSubsystem::GenerateCollectibles(const FProceduralTrack& Track, float Density)
{
    TArray<FProceduralCollectible> Collectibles;

    int32 CollectibleCount = FMath::RoundToInt(Track.Segments.Num() * Density * 5);

    TArray<FString> CollectibleTypes = {
        TEXT("NitroBoost"),
        TEXT("ScoreMultiplier"),
        TEXT("Cash"),
        TEXT("RepPoints"),
        TEXT("Mystery")
    };

    for (int32 i = 0; i < CollectibleCount; i++)
    {
        if (Track.Segments.Num() == 0) break;

        FProceduralCollectible Collectible;
        Collectible.CollectibleId = FGuid::NewGuid();
        Collectible.CollectibleType = CollectibleTypes[RandomStream.RandRange(0, CollectibleTypes.Num() - 1)];

        // Position on track
        int32 SegmentIndex = RandomStream.RandRange(0, Track.Segments.Num() - 1);
        const FTrackSegment& Segment = Track.Segments[SegmentIndex];

        float AlongTrack = RandomStream.FRand();
        Collectible.Position = FMath::Lerp(Segment.StartPosition, Segment.EndPosition, AlongTrack);
        Collectible.Position.Z += 50.0f; // Float above track

        // Rarity
        Collectible.bIsRare = RandomStream.FRand() > 0.9f;
        Collectible.bIsHidden = RandomStream.FRand() > 0.85f;

        // Points based on type and rarity
        if (Collectible.CollectibleType == TEXT("NitroBoost"))
        {
            Collectible.PointValue = 0; // Functional, not points
            Collectible.GlowColor = FLinearColor(0.0f, 0.5f, 1.0f);
        }
        else if (Collectible.CollectibleType == TEXT("ScoreMultiplier"))
        {
            Collectible.PointValue = 0;
            Collectible.GlowColor = FLinearColor(1.0f, 0.8f, 0.0f);
        }
        else if (Collectible.CollectibleType == TEXT("Cash"))
        {
            Collectible.PointValue = Collectible.bIsRare ? 500 : 100;
            Collectible.GlowColor = FLinearColor(0.0f, 1.0f, 0.3f);
        }
        else if (Collectible.CollectibleType == TEXT("RepPoints"))
        {
            Collectible.PointValue = Collectible.bIsRare ? 200 : 50;
            Collectible.GlowColor = FLinearColor(1.0f, 0.0f, 0.5f);
        }
        else
        {
            Collectible.PointValue = RandomStream.RandRange(50, 300);
            Collectible.GlowColor = FLinearColor(0.8f, 0.0f, 1.0f);
        }

        Collectible.RespawnTime = 30.0f;

        Collectibles.Add(Collectible);
    }

    return Collectibles;
}

TArray<FProceduralCollectible> UMGProceduralContentSubsystem::GenerateHiddenCollectibles(const FProceduralTrack& Track, int32 Count)
{
    TArray<FProceduralCollectible> HiddenCollectibles;

    for (int32 i = 0; i < Count; i++)
    {
        if (Track.Segments.Num() == 0) break;

        FProceduralCollectible Collectible;
        Collectible.CollectibleId = FGuid::NewGuid();
        Collectible.CollectibleType = TEXT("SecretCollectible");
        Collectible.bIsRare = true;
        Collectible.bIsHidden = true;
        Collectible.PointValue = 1000;
        Collectible.GlowColor = FLinearColor(1.0f, 0.84f, 0.0f); // Gold

        // Position off the main track
        int32 SegmentIndex = RandomStream.RandRange(0, Track.Segments.Num() - 1);
        const FTrackSegment& Segment = Track.Segments[SegmentIndex];

        FVector Direction = (Segment.EndPosition - Segment.StartPosition).GetSafeNormal();
        FVector Side = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

        Collectible.Position = Segment.StartPosition + Side * (Segment.Width + 50.0f);
        Collectible.Position.Z += 30.0f;

        Collectible.RespawnTime = 0.0f; // One-time collection

        HiddenCollectibles.Add(Collectible);
    }

    return HiddenCollectibles;
}

void UMGProceduralContentSubsystem::CollectItem(const FGuid& CollectibleId)
{
    // Handle collectible collection
    UE_LOG(LogTemp, Log, TEXT("Collected item: %s"), *CollectibleId.ToString());
}

// ============================================================================
// Content Storage
// ============================================================================

void UMGProceduralContentSubsystem::SaveGeneratedTrack(const FProceduralTrack& Track)
{
    SavedTracks.Add(Track.TrackId, Track);
    UE_LOG(LogTemp, Log, TEXT("Saved track: %s"), *Track.TrackName);
}

FProceduralTrack UMGProceduralContentSubsystem::LoadTrackBySeed(const FString& SeedCode)
{
    // Search saved tracks for matching seed
    for (const auto& Pair : SavedTracks)
    {
        if (Pair.Value.Seed.SeedCode == SeedCode)
        {
            return Pair.Value;
        }
    }

    // Not found in saved - regenerate from seed
    FProceduralSeed Seed = CreateSeedFromCode(SeedCode);
    FGenerationResult Result = GenerateTrackFromSeed(Seed, DefaultSettings);

    return Result.GeneratedTrack;
}

TArray<FProceduralTrack> UMGProceduralContentSubsystem::GetSavedTracks() const
{
    TArray<FProceduralTrack> Tracks;
    SavedTracks.GenerateValueArray(Tracks);
    return Tracks;
}

void UMGProceduralContentSubsystem::DeleteSavedTrack(const FGuid& TrackId)
{
    SavedTracks.Remove(TrackId);
    FavoriteTracks.Remove(TrackId);
    TrackPlaytimes.Remove(TrackId);
}

void UMGProceduralContentSubsystem::FavoriteTrack(const FGuid& TrackId)
{
    if (SavedTracks.Contains(TrackId))
    {
        FavoriteTracks.Add(TrackId, SavedTracks[TrackId]);
        ContentStats.FavoritedTracks++;
    }
}

TArray<FProceduralTrack> UMGProceduralContentSubsystem::GetFavoriteTracks() const
{
    TArray<FProceduralTrack> Tracks;
    FavoriteTracks.GenerateValueArray(Tracks);
    return Tracks;
}

// ============================================================================
// Sharing
// ============================================================================

FString UMGProceduralContentSubsystem::ShareTrack(const FGuid& TrackId)
{
    if (SavedTracks.Contains(TrackId))
    {
        const FProceduralTrack& Track = SavedTracks[TrackId];
        ContentStats.SharedTracks++;
        OnSeedShared.Broadcast(Track.Seed.SeedCode, Track.TrackName);
        return Track.Seed.SeedCode;
    }
    return TEXT("");
}

FProceduralTrack UMGProceduralContentSubsystem::ImportSharedTrack(const FString& ShareCode)
{
    if (ValidateShareCode(ShareCode))
    {
        FProceduralTrack Track = LoadTrackBySeed(ShareCode);
        SaveGeneratedTrack(Track);
        return Track;
    }
    return FProceduralTrack();
}

bool UMGProceduralContentSubsystem::ValidateShareCode(const FString& ShareCode)
{
    // Validate format: MG-XXXXXXXX-XXXXXXXX-XXXX
    if (!ShareCode.StartsWith(TEXT("MG-")))
    {
        return false;
    }

    TArray<FString> Parts;
    ShareCode.ParseIntoArray(Parts, TEXT("-"));

    return Parts.Num() >= 4;
}

// ============================================================================
// Statistics
// ============================================================================

void UMGProceduralContentSubsystem::RecordTrackPlaytime(const FGuid& TrackId, float PlaytimeSeconds)
{
    if (TrackPlaytimes.Contains(TrackId))
    {
        TrackPlaytimes[TrackId] += PlaytimeSeconds;
    }
    else
    {
        TrackPlaytimes.Add(TrackId, PlaytimeSeconds);
    }

    ContentStats.TotalPlayTimeOnGenerated += PlaytimeSeconds;
}

void UMGProceduralContentSubsystem::IncrementGenerationCount(EProceduralContentType ContentType)
{
    switch (ContentType)
    {
        case EProceduralContentType::Track:
            ContentStats.TotalTracksGenerated++;
            break;
        case EProceduralContentType::Environment:
            ContentStats.TotalEnvironmentsGenerated++;
            break;
        case EProceduralContentType::Challenge:
            ContentStats.TotalChallengesGenerated++;
            break;
        case EProceduralContentType::Shortcut:
            ContentStats.TotalShortcutsDiscovered++;
            break;
        case EProceduralContentType::SecretArea:
            ContentStats.TotalSecretAreasFound++;
            break;
        default:
            break;
    }
}

// ============================================================================
// Quality Settings
// ============================================================================

void UMGProceduralContentSubsystem::SetGenerationQuality(EProceduralQuality Quality)
{
    CurrentQuality = Quality;

    // Adjust default settings based on quality
    switch (Quality)
    {
        case EProceduralQuality::Draft:
            DefaultSettings.MaxSegments = 20;
            DefaultSettings.ObstacleDensity = 0.2f;
            DefaultSettings.CollectibleDensity = 0.1f;
            break;
        case EProceduralQuality::Low:
            DefaultSettings.MaxSegments = 30;
            DefaultSettings.ObstacleDensity = 0.3f;
            DefaultSettings.CollectibleDensity = 0.2f;
            break;
        case EProceduralQuality::Medium:
            DefaultSettings.MaxSegments = 40;
            DefaultSettings.ObstacleDensity = 0.5f;
            DefaultSettings.CollectibleDensity = 0.3f;
            break;
        case EProceduralQuality::High:
            DefaultSettings.MaxSegments = 60;
            DefaultSettings.ObstacleDensity = 0.7f;
            DefaultSettings.CollectibleDensity = 0.5f;
            break;
        case EProceduralQuality::Ultra:
            DefaultSettings.MaxSegments = 80;
            DefaultSettings.ObstacleDensity = 1.0f;
            DefaultSettings.CollectibleDensity = 0.7f;
            break;
    }
}

void UMGProceduralContentSubsystem::SetDefaultSettings(const FGenerationSettings& Settings)
{
    DefaultSettings = Settings;
}

// ============================================================================
// Internal Helpers
// ============================================================================

FTrackSegment UMGProceduralContentSubsystem::CreateStraightSegment(const FVector& Start, const FRotator& Rotation, float Length)
{
    FTrackSegment Segment;
    Segment.SegmentId = FGuid::NewGuid();
    Segment.SegmentType = ETrackSegmentType::Straight;
    Segment.StartPosition = Start;
    Segment.StartRotation = Rotation;
    Segment.Length = Length;
    Segment.Width = 12.0f;
    Segment.BankAngle = 0.0f;
    Segment.CurveRadius = 0.0f;
    Segment.SpeedLimit = 250.0f;
    Segment.DifficultyRating = 1.0f;
    Segment.GripMultiplier = 1.0f;
    Segment.bHasBarriers = true;
    Segment.bHasStreetLights = true;

    FVector Direction = Rotation.Vector();
    Segment.EndPosition = Start + Direction * Length;
    Segment.EndRotation = Rotation;

    return Segment;
}

FTrackSegment UMGProceduralContentSubsystem::CreateCurveSegment(const FVector& Start, const FRotator& Rotation, float Radius, float Angle)
{
    FTrackSegment Segment;
    Segment.SegmentId = FGuid::NewGuid();

    if (FMath::Abs(Angle) > 90.0f)
    {
        Segment.SegmentType = ETrackSegmentType::Hairpin;
    }
    else if (FMath::Abs(Angle) > 45.0f)
    {
        Segment.SegmentType = ETrackSegmentType::SharpCurve;
    }
    else
    {
        Segment.SegmentType = ETrackSegmentType::GentleCurve;
    }

    Segment.StartPosition = Start;
    Segment.StartRotation = Rotation;
    Segment.CurveRadius = Radius;
    Segment.Width = 12.0f;

    // Calculate arc length
    float ArcLength = (FMath::Abs(Angle) / 360.0f) * 2.0f * PI * Radius;
    Segment.Length = ArcLength;

    // Bank angle based on curve severity
    Segment.BankAngle = FMath::Clamp(Angle * 0.15f, -15.0f, 15.0f);

    // Speed limit based on radius
    Segment.SpeedLimit = FMath::Clamp(Radius * 1.2f, 60.0f, 200.0f);

    // Difficulty based on curve tightness
    Segment.DifficultyRating = (Radius > KINDA_SMALL_NUMBER) ? FMath::Clamp(180.0f / Radius, 1.0f, 5.0f) : 5.0f;

    Segment.GripMultiplier = 1.0f;
    Segment.bHasBarriers = true;
    Segment.bHasStreetLights = true;

    // Calculate end position and rotation
    float TurnDirection = (RandomStream.FRand() > 0.5f) ? 1.0f : -1.0f;
    FRotator EndRot = Rotation;
    EndRot.Yaw += Angle * TurnDirection;
    Segment.EndRotation = EndRot;

    // Approximate end position
    FVector Direction = Rotation.Vector();
    FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal() * TurnDirection;

    float ChordLength = 2.0f * Radius * FMath::Sin(FMath::DegreesToRadians(FMath::Abs(Angle) * 0.5f));
    FVector ChordDirection = (Direction + EndRot.Vector()).GetSafeNormal();
    Segment.EndPosition = Start + ChordDirection * ChordLength;

    return Segment;
}

FTrackSegment UMGProceduralContentSubsystem::CreateJumpSegment(const FVector& Start, const FRotator& Rotation, float Length, float Height)
{
    FTrackSegment Segment;
    Segment.SegmentId = FGuid::NewGuid();
    Segment.SegmentType = ETrackSegmentType::Jump;
    Segment.StartPosition = Start;
    Segment.StartRotation = Rotation;
    Segment.Length = Length;
    Segment.Width = 14.0f; // Slightly wider for safety
    Segment.ElevationChange = Height;
    Segment.BankAngle = 0.0f;
    Segment.CurveRadius = 0.0f;
    Segment.SpeedLimit = 300.0f; // Need speed for jumps
    Segment.DifficultyRating = 3.0f + (Height / 10.0f);
    Segment.GripMultiplier = 1.2f; // Good grip on ramp
    Segment.bHasBarriers = false; // Open for jump
    Segment.bHasStreetLights = true;

    FVector Direction = Rotation.Vector();
    Segment.EndPosition = Start + Direction * Length;
    Segment.EndPosition.Z += Height * 0.1f; // Slight elevation at landing
    Segment.EndRotation = Rotation;

    // Control points for the ramp shape
    Segment.ControlPoints.Add(Start);
    Segment.ControlPoints.Add(Start + Direction * (Length * 0.4f) + FVector(0, 0, Height));
    Segment.ControlPoints.Add(Start + Direction * (Length * 0.6f) + FVector(0, 0, Height * 0.5f));
    Segment.ControlPoints.Add(Segment.EndPosition);

    return Segment;
}

float UMGProceduralContentSubsystem::CalculateTrackDifficulty(const FProceduralTrack& Track)
{
    float TotalDifficulty = 0.0f;

    for (const FTrackSegment& Segment : Track.Segments)
    {
        TotalDifficulty += Segment.DifficultyRating;
    }

    // Factor in jumps and drift zones
    TotalDifficulty += Track.JumpCount * 5.0f;
    TotalDifficulty += Track.DriftZoneCount * 3.0f;

    // Normalize to 0-100 scale
    return FMath::Clamp(TotalDifficulty / Track.Segments.Num() * 20.0f, 0.0f, 100.0f);
}

float UMGProceduralContentSubsystem::EstimateLapTime(const FProceduralTrack& Track)
{
    float EstimatedTime = 0.0f;
    float BaseSpeed = 150.0f; // km/h average

    for (const FTrackSegment& Segment : Track.Segments)
    {
        float SegmentSpeed = FMath::Min(BaseSpeed, Segment.SpeedLimit);
        float SegmentTime = Segment.Length / (SegmentSpeed * 0.277778f); // Convert km/h to m/s
        EstimatedTime += SegmentTime;
    }

    // Add time for corners (slowdown)
    int32 CurveCount = 0;
    for (const FTrackSegment& Segment : Track.Segments)
    {
        if (Segment.SegmentType != ETrackSegmentType::Straight)
        {
            CurveCount++;
        }
    }
    EstimatedTime += CurveCount * 1.5f; // Average 1.5 seconds lost per curve

    return EstimatedTime;
}

bool UMGProceduralContentSubsystem::CheckSegmentCollision(const FTrackSegment& NewSegment, const TArray<FTrackSegment>& ExistingSegments)
{
    // Simple collision check - see if new segment overlaps existing ones
    for (const FTrackSegment& Existing : ExistingSegments)
    {
        // Skip recent segments (allows connecting)
        if (ExistingSegments.Num() > 0 && &Existing == &ExistingSegments.Last())
        {
            continue;
        }

        float Distance = FVector::Distance(NewSegment.StartPosition, Existing.StartPosition);
        if (Distance < (NewSegment.Width + Existing.Width) && Distance > 1.0f)
        {
            // Check if they're actually overlapping, not just close
            float MinSeparation = (NewSegment.Width + Existing.Width) * 0.5f;
            if (Distance < MinSeparation)
            {
                return true; // Collision detected
            }
        }
    }

    return false; // No collision
}

void UMGProceduralContentSubsystem::GenerateCheckpoints(FProceduralTrack& Track)
{
    Track.CheckpointPositions.Empty();

    // Place checkpoint every few segments
    int32 CheckpointInterval = FMath::Max(3, Track.Segments.Num() / 10);

    for (int32 i = 0; i < Track.Segments.Num(); i += CheckpointInterval)
    {
        if (i < Track.Segments.Num())
        {
            Track.CheckpointPositions.Add(Track.Segments[i].StartPosition);
        }
    }

    // Always add finish line position
    if (Track.Segments.Num() > 0)
    {
        Track.CheckpointPositions.Add(Track.Segments[0].StartPosition);
    }
}

void UMGProceduralContentSubsystem::GenerateSpawnPositions(FProceduralTrack& Track)
{
    Track.SpawnPositions.Empty();

    if (Track.Segments.Num() == 0) return;

    const FTrackSegment& StartSegment = Track.Segments[0];
    FVector StartPos = StartSegment.StartPosition;
    FVector Direction = StartSegment.StartRotation.Vector();
    FVector Side = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

    // Generate 8 spawn positions (2 rows of 4)
    for (int32 Row = 0; Row < 2; Row++)
    {
        for (int32 Col = 0; Col < 4; Col++)
        {
            FVector SpawnPos = StartPos;
            SpawnPos -= Direction * (Row * 8.0f + 5.0f); // Rows behind start line
            SpawnPos += Side * ((Col - 1.5f) * 3.5f); // Spread across width
            Track.SpawnPositions.Add(SpawnPos);
        }
    }
}

ETrackSegmentType UMGProceduralContentSubsystem::SelectNextSegmentType(ETrackSegmentType CurrentType, EGenerationDifficulty Difficulty)
{
    // Weights for different segment types based on difficulty
    TMap<ETrackSegmentType, float> Weights;

    // Base weights
    Weights.Add(ETrackSegmentType::Straight, 3.0f);
    Weights.Add(ETrackSegmentType::GentleCurve, 2.0f);
    Weights.Add(ETrackSegmentType::SharpCurve, 1.0f);
    Weights.Add(ETrackSegmentType::Hairpin, 0.3f);
    Weights.Add(ETrackSegmentType::Jump, 0.2f);
    Weights.Add(ETrackSegmentType::DriftZone, 0.5f);

    // Adjust for difficulty
    float DifficultyMod = static_cast<float>(Difficulty) / 3.0f;
    Weights[ETrackSegmentType::SharpCurve] *= (1.0f + DifficultyMod);
    Weights[ETrackSegmentType::Hairpin] *= (1.0f + DifficultyMod * 2.0f);
    Weights[ETrackSegmentType::Jump] *= (1.0f + DifficultyMod);

    // Reduce consecutive same types
    if (Weights.Contains(CurrentType))
    {
        Weights[CurrentType] *= 0.5f;
    }

    // Don't follow hairpin with another hairpin
    if (CurrentType == ETrackSegmentType::Hairpin)
    {
        Weights[ETrackSegmentType::Hairpin] = 0.0f;
        Weights[ETrackSegmentType::Straight] *= 2.0f;
    }

    // Calculate total weight
    float TotalWeight = 0.0f;
    for (const auto& Pair : Weights)
    {
        TotalWeight += Pair.Value;
    }

    // Random selection
    float Roll = RandomStream.FRandRange(0.0f, TotalWeight);
    float Cumulative = 0.0f;

    for (const auto& Pair : Weights)
    {
        Cumulative += Pair.Value;
        if (Roll <= Cumulative)
        {
            return Pair.Key;
        }
    }

    return ETrackSegmentType::Straight;
}

FLinearColor UMGProceduralContentSubsystem::GenerateY2KNeonColor()
{
    // Y2K aesthetic neon colors
    TArray<FLinearColor> Y2KColors = {
        FLinearColor(1.0f, 0.0f, 0.5f),   // Hot pink
        FLinearColor(0.0f, 1.0f, 1.0f),   // Cyan
        FLinearColor(0.5f, 0.0f, 1.0f),   // Purple
        FLinearColor(1.0f, 0.5f, 0.0f),   // Orange
        FLinearColor(0.0f, 1.0f, 0.5f),   // Mint green
        FLinearColor(1.0f, 1.0f, 0.0f),   // Yellow
        FLinearColor(1.0f, 0.0f, 1.0f),   // Magenta
        FLinearColor(0.0f, 0.5f, 1.0f),   // Electric blue
    };

    int32 Index = RandomStream.RandRange(0, Y2KColors.Num() - 1);
    FLinearColor BaseColor = Y2KColors[Index];

    // Add slight variation
    BaseColor.R += RandomStream.FRandRange(-0.1f, 0.1f);
    BaseColor.G += RandomStream.FRandRange(-0.1f, 0.1f);
    BaseColor.B += RandomStream.FRandRange(-0.1f, 0.1f);

    return BaseColor.GetClamped();
}
