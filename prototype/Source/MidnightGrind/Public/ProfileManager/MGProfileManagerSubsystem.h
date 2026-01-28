// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGProfileManagerSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Profile Manager Subsystem for Midnight Grind, which is
 * the central hub for all player data. If the game were a person, this subsystem
 * would be their memory - it remembers everything about the player: stats,
 * achievements, race history, friends, settings, and more.
 *
 * WHAT IS A PLAYER PROFILE?
 * -------------------------
 * A player profile is a collection of all data associated with a specific player:
 *
 *    +------------------------------------------+
 *    |            PLAYER PROFILE                |
 *    +------------------------------------------+
 *    | Identity: Name, Avatar, Status           |
 *    | Progression: Level, XP, Prestige         |
 *    | Statistics: Wins, Races, Best Times      |
 *    | History: Recent Races, Records           |
 *    | Achievements: Unlocked, Progress         |
 *    | Social: Friends, Blocked Players         |
 *    | Settings: Controls, Privacy              |
 *    | Currency: Cash, Premium Credits          |
 *    +------------------------------------------+
 *
 * This data persists between game sessions and syncs with backend servers
 * for online play.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. PROFILE VERSIONING (EMGProfileVersion):
 *    As the game updates, the profile structure may change. Version numbers
 *    help migrate old profiles to new formats without losing data.
 *
 *    Example: Version 1 had just basic stats. Version 2 added race history.
 *    When loading a Version 1 profile, the system upgrades it to Version 2
 *    by adding empty race history.
 *
 * 2. CAREER STATISTICS (FMGCareerStat):
 *    Tracks cumulative player accomplishments:
 *    - Total races completed
 *    - Total wins
 *    - Total distance driven
 *    - Longest drift
 *    - etc.
 *
 *    Each stat has:
 *    - Current value (cumulative)
 *    - Best value (personal record)
 *    - Higher/lower is better flag (for comparisons)
 *
 * 3. RACE HISTORY (FMGRaceHistoryEntry):
 *    A log of recent races with detailed information:
 *    - Which track and vehicle
 *    - Final position
 *    - Best lap time
 *    - XP and currency earned
 *
 *    This enables features like "replay recent race" or "show improvement over time."
 *
 * 4. VEHICLE USAGE STATS (FMGVehicleUsageStats):
 *    Per-vehicle statistics showing how the player performs with each car:
 *    - How many races with this vehicle
 *    - Win rate
 *    - Best top speed achieved
 *    - Total distance driven
 *
 *    Used for features like "most used vehicle" or "vehicle mastery."
 *
 * 5. TRACK RECORDS (FMGTrackRecord):
 *    Personal best times and performance on each track:
 *    - Best lap time
 *    - Best race time
 *    - Which vehicle set the record
 *
 *    Enables time trial features and personal improvement tracking.
 *
 * 6. ACHIEVEMENTS (FMGPlayerAchievement):
 *    Milestone accomplishments with progress tracking:
 *    - "Win 100 Races" - shows 45/100 progress
 *    - Rarity indicates how many players have earned it
 *    - Point values contribute to achievement score
 *
 * 7. CONTROL PRESETS (FMGControlPreset):
 *    Saved input configurations:
 *    - Steering sensitivity
 *    - Button mappings
 *    - Assist settings (traction control, etc.)
 *
 *    Players can have multiple presets for different controllers or play styles.
 *
 * 8. SOCIAL CONNECTIONS (FMGSocialConnection):
 *    Friend and rival data:
 *    - Online status
 *    - Race history together
 *    - Win/loss record against them
 *    - Block/mute flags
 *
 * 9. SEASONAL RANKINGS (FMGSeasonalRanking):
 *    Competitive rank data for each season:
 *    - Current rank and points
 *    - Peak rank achieved
 *    - Win rate in ranked mode
 *
 * ARCHITECTURE OVERVIEW:
 * ----------------------
 *
 *    +-------------+     +------------------+     +-------------+
 *    |    UI       | <-> | Profile Manager  | <-> |  Save Game  |
 *    | (displays)  |     | Subsystem        |     |  (local)    |
 *    +-------------+     +--------+---------+     +-------------+
 *                               |
 *                               v
 *                        +-------------+
 *                        |   Backend   |
 *                        |   Server    |
 *                        | (online sync)|
 *                        +-------------+
 *
 * PROFILE DATA FLOW:
 * ------------------
 *
 * Loading Profile:
 *    1. LoadProfile() called with player ID
 *    2. Local save file loaded (if exists)
 *    3. Profile version checked, migrated if needed
 *    4. OnProfileLoaded delegate fires
 *    5. UI updates to show player data
 *
 * During Gameplay:
 *    1. Player finishes a race
 *    2. AddRaceToHistory() called
 *    3. UpdateCareerStat() updates totals
 *    4. AddExperience() grants XP
 *    5. Profile marked "dirty" (needs saving)
 *    6. Autosave timer triggers SaveProfile()
 *
 * DELEGATE PATTERN:
 * -----------------
 * The subsystem broadcasts events when important things happen:
 *
 *    OnLevelUp: Player gained a level
 *       -> UI shows level up animation
 *       -> Unlocks are checked
 *
 *    OnAchievementUnlocked: Player earned an achievement
 *       -> Notification popup shown
 *       -> Steam/PlayStation trophy unlocked
 *
 *    OnCurrencyChanged: Money balance changed
 *       -> HUD updates currency display
 *
 * CODE EXAMPLE:
 * -------------
 *    // Get the subsystem
 *    UMGProfileManagerSubsystem* ProfileMgr =
 *        GetGameInstance()->GetSubsystem<UMGProfileManagerSubsystem>();
 *
 *    // Load a profile
 *    ProfileMgr->LoadProfile(TEXT("Player123"));
 *
 *    // Update stats after a race
 *    ProfileMgr->UpdateCareerStat(TEXT("TotalRaces"), 1, true);  // Add 1
 *    ProfileMgr->UpdateCareerStat(TEXT("TotalWins"), 1, true);   // Add 1
 *
 *    // Add XP and check for level up
 *    ProfileMgr->AddExperience(1500);  // OnLevelUp fires if threshold reached
 *
 *    // Record the race in history
 *    FMGRaceHistoryEntry Entry;
 *    Entry.TrackId = TEXT("Track_Downtown");
 *    Entry.Position = 1;
 *    Entry.ExperienceEarned = 1500;
 *    ProfileMgr->AddRaceToHistory(Entry);
 *
 *    // Check achievement progress
 *    ProfileMgr->UpdateAchievementProgress(TEXT("Win100Races"), 0.45f);
 *
 * PRIVACY AND SETTINGS:
 * ---------------------
 * FMGProfileSettings controls what other players can see:
 * - Public: Everyone sees everything
 * - FriendsOnly: Only friends see stats
 * - Private: Nobody sees your profile
 *
 * Individual toggles for:
 * - Online status visibility
 * - Statistics visibility
 * - Allow friend requests
 * - Allow race invites
 *
 * EXPORT/IMPORT:
 * --------------
 * Profiles can be exported to JSON for:
 * - Account transfers
 * - Debugging
 * - Backup purposes
 *
 * The checksum field validates that exported data hasn't been tampered with.
 *
 * RELATED FILES:
 * --------------
 * - MGProfileManagerSubsystem.cpp: Implementation of all functions
 * - MGSaveGameSubsystem: Handles actual file I/O
 * - MGBackendService: Syncs profiles with online servers
 * - MGAchievementSubsystem: Defines available achievements
 * - MGRankingSubsystem: Manages seasonal competitive ranks
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGProfileManagerSubsystem.generated.h"

// Forward declarations
class UMGProfileManagerSubsystem;

/**
 * Profile data version for migration support
 */
UENUM(BlueprintType)
enum class EMGProfileVersion : uint8
{
    Initial         UMETA(DisplayName = "Initial"),
    AddedStats      UMETA(DisplayName = "Added Stats"),
    AddedPresets    UMETA(DisplayName = "Added Presets"),
    AddedHistory    UMETA(DisplayName = "Added History"),
    AddedSocial     UMETA(DisplayName = "Added Social"),
    Current         UMETA(DisplayName = "Current")
};

/**
 * Player status for online presence
 */
UENUM(BlueprintType)
enum class EMGPlayerStatus : uint8
{
    Offline         UMETA(DisplayName = "Offline"),
    Online          UMETA(DisplayName = "Online"),
    InRace          UMETA(DisplayName = "In Race"),
    InGarage        UMETA(DisplayName = "In Garage"),
    InMenu          UMETA(DisplayName = "In Menu"),
    Away            UMETA(DisplayName = "Away"),
    DoNotDisturb    UMETA(DisplayName = "Do Not Disturb"),
    Invisible       UMETA(DisplayName = "Invisible")
};

/**
 * Profile privacy settings
 */
UENUM(BlueprintType)
enum class EMGProfilePrivacy : uint8
{
    Public          UMETA(DisplayName = "Public"),
    FriendsOnly     UMETA(DisplayName = "Friends Only"),
    Private         UMETA(DisplayName = "Private"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * Achievement rarity tier
 */
UENUM(BlueprintType)
enum class EMGAchievementRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Epic            UMETA(DisplayName = "Epic"),
    Legendary       UMETA(DisplayName = "Legendary"),
    Mythic          UMETA(DisplayName = "Mythic")
};

/**
 * Racing discipline specialization
 */
UENUM(BlueprintType)
enum class EMGRacingDiscipline : uint8
{
    StreetRacing    UMETA(DisplayName = "Street Racing"),
    Drifting        UMETA(DisplayName = "Drifting"),
    Drag            UMETA(DisplayName = "Drag Racing"),
    TimeAttack      UMETA(DisplayName = "Time Attack"),
    Pursuit         UMETA(DisplayName = "Pursuit"),
    Stunt           UMETA(DisplayName = "Stunt"),
    Rally           UMETA(DisplayName = "Rally"),
    AllRounder      UMETA(DisplayName = "All Rounder")
};

/**
 * Player reputation level
 */
UENUM(BlueprintType)
enum class EMGReputationLevel : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Amateur         UMETA(DisplayName = "Amateur"),
    Rookie          UMETA(DisplayName = "Rookie"),
    Regular         UMETA(DisplayName = "Regular"),
    Veteran         UMETA(DisplayName = "Veteran"),
    Elite           UMETA(DisplayName = "Elite"),
    Legend          UMETA(DisplayName = "Legend"),
    Icon            UMETA(DisplayName = "Icon")
};

/**
 * Vehicle stat category for favorites
 */
UENUM(BlueprintType)
enum class EMGVehicleStat : uint8
{
    Speed           UMETA(DisplayName = "Speed"),
    Acceleration    UMETA(DisplayName = "Acceleration"),
    Handling        UMETA(DisplayName = "Handling"),
    Drift           UMETA(DisplayName = "Drift"),
    Nitro           UMETA(DisplayName = "Nitro"),
    Durability      UMETA(DisplayName = "Durability")
};

/**
 * Single career stat entry
 */
USTRUCT(BlueprintType)
struct FMGCareerStat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
    FString StatId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
    int64 Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
    int64 BestValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
    FDateTime LastUpdated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
    bool bHigherIsBetter;

    FMGCareerStat()
        : Value(0)
        , BestValue(0)
        , LastUpdated(FDateTime::Now())
        , bHigherIsBetter(true)
    {}
};

/**
 * Race history entry
 */
USTRUCT(BlueprintType)
struct FMGRaceHistoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FGuid RaceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString TrackId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString VehicleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FString VehicleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    FDateTime RaceDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 TotalRacers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    float RaceTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    float BestLapTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 DriftsPerformed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 NitroBoosts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 NearMisses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 Takedowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 ExperienceEarned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    int32 CurrencyEarned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    bool bWasOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
    bool bWasRanked;

    FMGRaceHistoryEntry()
        : RaceDate(FDateTime::Now())
        , Position(0)
        , TotalRacers(0)
        , RaceTime(0.0f)
        , BestLapTime(0.0f)
        , DriftsPerformed(0)
        , NitroBoosts(0)
        , NearMisses(0)
        , Takedowns(0)
        , ExperienceEarned(0)
        , CurrencyEarned(0)
        , bWasOnline(false)
        , bWasRanked(false)
    {}
};

/**
 * Vehicle usage statistics
 */
USTRUCT(BlueprintType)
struct FMGVehicleUsageStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    FString VehicleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    FString VehicleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    int32 RacesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    int32 Wins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    int32 Podiums;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float TotalDistanceDriven;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float TotalTimeDriven;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float BestTopSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    float LongestDrift;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    int32 TotalDrifts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    int32 TotalTakedowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    int32 TotalNitroBoosts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    FDateTime LastUsed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
    bool bIsFavorite;

    FMGVehicleUsageStats()
        : RacesCompleted(0)
        , Wins(0)
        , Podiums(0)
        , TotalDistanceDriven(0.0f)
        , TotalTimeDriven(0.0f)
        , BestTopSpeed(0.0f)
        , LongestDrift(0.0f)
        , TotalDrifts(0)
        , TotalTakedowns(0)
        , TotalNitroBoosts(0)
        , LastUsed(FDateTime::Now())
        , bIsFavorite(false)
    {}
};

/**
 * Track performance record
 */
USTRUCT(BlueprintType)
struct FMGTrackRecord
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    FString TrackId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    FString TrackName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    int32 TimesPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    int32 BestPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    float BestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    float BestLapTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    FString BestVehicleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    int32 Wins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    int32 Podiums;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    float AveragePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    FDateTime PersonalRecordDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
    bool bIsFavorite;

    FMGTrackRecord()
        : TimesPlayed(0)
        , BestPosition(99)
        , BestTime(999999.0f)
        , BestLapTime(999999.0f)
        , Wins(0)
        , Podiums(0)
        , AveragePosition(0.0f)
        , PersonalRecordDate(FDateTime::Now())
        , bIsFavorite(false)
    {}
};

/**
 * Player achievement data
 */
USTRUCT(BlueprintType)
struct FMGPlayerAchievement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    FString AchievementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    EMGAchievementRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    int32 PointsValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    float Progress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    bool bUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    FDateTime UnlockDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    FString IconPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
    bool bHidden;

    FMGPlayerAchievement()
        : Rarity(EMGAchievementRarity::Common)
        , PointsValue(10)
        , Progress(0.0f)
        , TargetValue(1.0f)
        , bUnlocked(false)
        , UnlockDate(FDateTime::MinValue())
        , bHidden(false)
    {}
};

/**
 * Control preset configuration
 */
USTRUCT(BlueprintType)
struct FMGControlPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    FString PresetId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float SteeringSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float SteeringDeadzone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float ThrottleSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    float BrakeSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bInvertSteering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bAssistBraking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bAssistSteering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bTractionControl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bStabilityControl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bAutoTransmission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    TMap<FString, FKey> KeyBindings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
    bool bIsDefault;

    FMGControlPreset()
        : SteeringSensitivity(1.0f)
        , SteeringDeadzone(0.1f)
        , ThrottleSensitivity(1.0f)
        , BrakeSensitivity(1.0f)
        , bInvertSteering(false)
        , bAssistBraking(true)
        , bAssistSteering(false)
        , bTractionControl(false)
        , bStabilityControl(false)
        , bAutoTransmission(true)
        , bIsDefault(false)
    {}
};

/**
 * Social connection/friend data
 */
USTRUCT(BlueprintType)
struct FMGSocialConnection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    EMGPlayerStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    EMGReputationLevel Reputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FString CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FDateTime FriendSince;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FDateTime LastOnline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 RacesTogether;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 WinsAgainst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 LossesAgainst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FString AvatarPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsFavorite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsMuted;

    FMGSocialConnection()
        : Status(EMGPlayerStatus::Offline)
        , Level(1)
        , Reputation(EMGReputationLevel::Unknown)
        , FriendSince(FDateTime::Now())
        , LastOnline(FDateTime::Now())
        , RacesTogether(0)
        , WinsAgainst(0)
        , LossesAgainst(0)
        , bIsFavorite(false)
        , bIsBlocked(false)
        , bIsMuted(false)
    {}
};

/**
 * Player profile settings
 */
USTRUCT(BlueprintType)
struct FMGProfileSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    EMGProfilePrivacy Privacy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bShowOnlineStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowFriendRequests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowCrewInvites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowRaceInvites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bShowInLeaderboards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAllowSpectators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bShareReplays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bShowStatistics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bReceiveNotifications;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoJoinVoiceChat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FString StatusMessage;

    FMGProfileSettings()
        : Privacy(EMGProfilePrivacy::FriendsOnly)
        , bShowOnlineStatus(true)
        , bAllowFriendRequests(true)
        , bAllowCrewInvites(true)
        , bAllowRaceInvites(true)
        , bShowInLeaderboards(true)
        , bAllowSpectators(true)
        , bShareReplays(false)
        , bShowStatistics(true)
        , bReceiveNotifications(true)
        , bAutoJoinVoiceChat(false)
    {}
};

/**
 * Seasonal ranking data
 */
USTRUCT(BlueprintType)
struct FMGSeasonalRanking
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    FString SeasonId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    FString SeasonName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    int32 Rank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    int32 RankPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    int32 PeakRank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    int32 PeakRankPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    int32 RankedRaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    int32 RankedWins;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    float WinRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    FDateTime SeasonStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    FDateTime SeasonEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranking")
    bool bIsActive;

    FMGSeasonalRanking()
        : Rank(0)
        , RankPoints(0)
        , PeakRank(0)
        , PeakRankPoints(0)
        , RankedRaces(0)
        , RankedWins(0)
        , WinRate(0.0f)
        , SeasonStart(FDateTime::Now())
        , SeasonEnd(FDateTime::Now())
        , bIsActive(false)
    {}
};

/**
 * Complete player profile data
 */
USTRUCT(BlueprintType)
struct FMGPlayerProfile
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString AvatarPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString BannerPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString TitleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString BadgeId;

    // Status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    EMGPlayerStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    EMGReputationLevel Reputation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    EMGRacingDiscipline MainDiscipline;

    // Progression
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int64 TotalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int32 PrestigeLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int32 AchievementPoints;

    // Currency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int64 SoftCurrency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int64 PremiumCurrency;

    // Statistics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TMap<FString, FMGCareerStat> CareerStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FMGRaceHistoryEntry> RaceHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TMap<FString, FMGVehicleUsageStats> VehicleStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TMap<FString, FMGTrackRecord> TrackRecords;

    // Achievements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TMap<FString, FMGPlayerAchievement> Achievements;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FMGProfileSettings Settings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FMGControlPreset> ControlPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int32 ActivePresetIndex;

    // Social
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FMGSocialConnection> Friends;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FString> BlockedPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FString CrewId;

    // Ranking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    TArray<FMGSeasonalRanking> SeasonalRankings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    int32 GlobalRank;

    // Metadata
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    EMGProfileVersion Version;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FDateTime CreatedDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    FDateTime LastLoginDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
    float TotalPlaytime;

    FMGPlayerProfile()
        : Status(EMGPlayerStatus::Offline)
        , Reputation(EMGReputationLevel::Unknown)
        , MainDiscipline(EMGRacingDiscipline::AllRounder)
        , Level(1)
        , TotalExperience(0)
        , PrestigeLevel(0)
        , AchievementPoints(0)
        , SoftCurrency(0)
        , PremiumCurrency(0)
        , ActivePresetIndex(0)
        , GlobalRank(0)
        , Version(EMGProfileVersion::Current)
        , CreatedDate(FDateTime::Now())
        , LastLoginDate(FDateTime::Now())
        , TotalPlaytime(0.0f)
    {}
};

/**
 * Profile export format
 */
USTRUCT(BlueprintType)
struct FMGProfileExport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
    FMGPlayerProfile Profile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
    FString ExportVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
    FDateTime ExportDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
    FString Checksum;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
    bool bIsEncrypted;

    FMGProfileExport()
        : ExportDate(FDateTime::Now())
        , bIsEncrypted(false)
    {}
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnProfileLoaded, const FMGPlayerProfile&, Profile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnProfileSaved, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLevelUp, int32, NewLevel, int32, ExperienceGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAchievementUnlocked, const FString&, AchievementId, const FMGPlayerAchievement&, Achievement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAchievementProgress, const FString&, AchievementId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnStatUpdated, const FString&, StatId, int64, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnReputationChanged, EMGReputationLevel, OldLevel, EMGReputationLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnFriendStatusChanged, const FString&, PlayerId, EMGPlayerStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRaceHistoryAdded, const FMGRaceHistoryEntry&, Entry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCurrencyChanged, int64, SoftCurrency, int64, PremiumCurrency);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnProfileMigrated, EMGProfileVersion, FromVersion, EMGProfileVersion, ToVersion);

/**
 * Profile Manager Subsystem
 *
 * Manages player profiles including statistics, achievements, settings,
 * race history, social connections, and progression data. Handles profile
 * saving, loading, migration, and export.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGProfileManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGProfileManagerSubsystem();

    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    // Profile management
    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool LoadProfile(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool SaveProfile();

    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool CreateNewProfile(const FString& PlayerId, const FString& DisplayName);

    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool DeleteProfile(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Profile")
    FMGPlayerProfile GetCurrentProfile() const;

    UFUNCTION(BlueprintPure, Category = "Profile")
    bool HasActiveProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Profile")
    bool MigrateProfile(EMGProfileVersion TargetVersion);

    // Identity
    UFUNCTION(BlueprintCallable, Category = "Profile|Identity")
    bool SetDisplayName(const FString& NewName);

    UFUNCTION(BlueprintCallable, Category = "Profile|Identity")
    bool SetAvatar(const FString& AvatarPath);

    UFUNCTION(BlueprintCallable, Category = "Profile|Identity")
    bool SetBanner(const FString& BannerPath);

    UFUNCTION(BlueprintCallable, Category = "Profile|Identity")
    bool SetTitle(const FString& TitleId);

    UFUNCTION(BlueprintCallable, Category = "Profile|Identity")
    bool SetBadge(const FString& BadgeId);

    UFUNCTION(BlueprintCallable, Category = "Profile|Identity")
    bool SetStatus(EMGPlayerStatus NewStatus);

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Profile|Stats")
    bool UpdateCareerStat(const FString& StatId, int64 Value, bool bIsDelta = true);

    UFUNCTION(BlueprintPure, Category = "Profile|Stats")
    int64 GetCareerStatValue(const FString& StatId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Stats")
    int64 GetCareerStatBest(const FString& StatId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Stats")
    TArray<FMGCareerStat> GetAllCareerStats() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Stats")
    void RegisterCareerStat(const FString& StatId, const FText& DisplayName, bool bHigherIsBetter = true);

    // Race history
    UFUNCTION(BlueprintCallable, Category = "Profile|History")
    void AddRaceToHistory(const FMGRaceHistoryEntry& Entry);

    UFUNCTION(BlueprintPure, Category = "Profile|History")
    TArray<FMGRaceHistoryEntry> GetRaceHistory(int32 MaxEntries = 50) const;

    UFUNCTION(BlueprintPure, Category = "Profile|History")
    TArray<FMGRaceHistoryEntry> GetRaceHistoryForTrack(const FString& TrackId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|History")
    TArray<FMGRaceHistoryEntry> GetRaceHistoryForVehicle(const FString& VehicleId) const;

    UFUNCTION(BlueprintCallable, Category = "Profile|History")
    void ClearRaceHistory();

    // Vehicle stats
    UFUNCTION(BlueprintCallable, Category = "Profile|Vehicle")
    void UpdateVehicleStats(const FMGVehicleUsageStats& Stats);

    UFUNCTION(BlueprintPure, Category = "Profile|Vehicle")
    FMGVehicleUsageStats GetVehicleStats(const FString& VehicleId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Vehicle")
    TArray<FMGVehicleUsageStats> GetAllVehicleStats() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Vehicle")
    TArray<FMGVehicleUsageStats> GetFavoriteVehicles() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Vehicle")
    FMGVehicleUsageStats GetMostUsedVehicle() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Vehicle")
    void SetVehicleFavorite(const FString& VehicleId, bool bFavorite);

    // Track records
    UFUNCTION(BlueprintCallable, Category = "Profile|Track")
    void UpdateTrackRecord(const FMGTrackRecord& Record);

    UFUNCTION(BlueprintPure, Category = "Profile|Track")
    FMGTrackRecord GetTrackRecord(const FString& TrackId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Track")
    TArray<FMGTrackRecord> GetAllTrackRecords() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Track")
    TArray<FMGTrackRecord> GetFavoriteTracks() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Track")
    void SetTrackFavorite(const FString& TrackId, bool bFavorite);

    // Achievements
    UFUNCTION(BlueprintCallable, Category = "Profile|Achievements")
    void RegisterAchievement(const FMGPlayerAchievement& Achievement);

    UFUNCTION(BlueprintCallable, Category = "Profile|Achievements")
    bool UpdateAchievementProgress(const FString& AchievementId, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Profile|Achievements")
    bool UnlockAchievement(const FString& AchievementId);

    UFUNCTION(BlueprintPure, Category = "Profile|Achievements")
    FMGPlayerAchievement GetAchievement(const FString& AchievementId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Achievements")
    TArray<FMGPlayerAchievement> GetAllAchievements() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Achievements")
    TArray<FMGPlayerAchievement> GetUnlockedAchievements() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Achievements")
    TArray<FMGPlayerAchievement> GetLockedAchievements() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Achievements")
    int32 GetTotalAchievementPoints() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Achievements")
    float GetAchievementCompletionPercent() const;

    // Progression
    UFUNCTION(BlueprintCallable, Category = "Profile|Progression")
    bool AddExperience(int64 Amount);

    UFUNCTION(BlueprintPure, Category = "Profile|Progression")
    int64 GetExperienceForLevel(int32 Level) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Progression")
    int64 GetExperienceToNextLevel() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Progression")
    float GetLevelProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Progression")
    bool Prestige();

    UFUNCTION(BlueprintPure, Category = "Profile|Progression")
    bool CanPrestige() const;

    // Currency
    UFUNCTION(BlueprintCallable, Category = "Profile|Currency")
    bool AddSoftCurrency(int64 Amount);

    UFUNCTION(BlueprintCallable, Category = "Profile|Currency")
    bool SpendSoftCurrency(int64 Amount);

    UFUNCTION(BlueprintCallable, Category = "Profile|Currency")
    bool AddPremiumCurrency(int64 Amount);

    UFUNCTION(BlueprintCallable, Category = "Profile|Currency")
    bool SpendPremiumCurrency(int64 Amount);

    UFUNCTION(BlueprintPure, Category = "Profile|Currency")
    int64 GetSoftCurrency() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Currency")
    int64 GetPremiumCurrency() const;

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Profile|Settings")
    void UpdateProfileSettings(const FMGProfileSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Profile|Settings")
    FMGProfileSettings GetProfileSettings() const;

    // Control presets
    UFUNCTION(BlueprintCallable, Category = "Profile|Controls")
    bool AddControlPreset(const FMGControlPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Profile|Controls")
    bool UpdateControlPreset(int32 Index, const FMGControlPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Profile|Controls")
    bool RemoveControlPreset(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Profile|Controls")
    bool SetActivePreset(int32 Index);

    UFUNCTION(BlueprintPure, Category = "Profile|Controls")
    FMGControlPreset GetActiveControlPreset() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Controls")
    TArray<FMGControlPreset> GetAllControlPresets() const;

    // Social
    UFUNCTION(BlueprintCallable, Category = "Profile|Social")
    bool AddFriend(const FMGSocialConnection& Friend);

    UFUNCTION(BlueprintCallable, Category = "Profile|Social")
    bool RemoveFriend(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Profile|Social")
    bool BlockPlayer(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Profile|Social")
    bool UnblockPlayer(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "Profile|Social")
    bool SetFriendFavorite(const FString& PlayerId, bool bFavorite);

    UFUNCTION(BlueprintCallable, Category = "Profile|Social")
    void UpdateFriendStatus(const FString& PlayerId, EMGPlayerStatus NewStatus);

    UFUNCTION(BlueprintPure, Category = "Profile|Social")
    TArray<FMGSocialConnection> GetFriends() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Social")
    TArray<FMGSocialConnection> GetOnlineFriends() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Social")
    TArray<FMGSocialConnection> GetFavoriteFriends() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Social")
    bool IsFriend(const FString& PlayerId) const;

    UFUNCTION(BlueprintPure, Category = "Profile|Social")
    bool IsBlocked(const FString& PlayerId) const;

    // Rankings
    UFUNCTION(BlueprintCallable, Category = "Profile|Ranking")
    void UpdateSeasonalRanking(const FMGSeasonalRanking& Ranking);

    UFUNCTION(BlueprintPure, Category = "Profile|Ranking")
    FMGSeasonalRanking GetCurrentSeasonRanking() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Ranking")
    TArray<FMGSeasonalRanking> GetAllSeasonalRankings() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Ranking")
    void SetGlobalRank(int32 Rank);

    // Reputation
    UFUNCTION(BlueprintCallable, Category = "Profile|Reputation")
    void UpdateReputation(EMGReputationLevel NewLevel);

    UFUNCTION(BlueprintPure, Category = "Profile|Reputation")
    EMGReputationLevel GetReputation() const;

    // Export/Import
    UFUNCTION(BlueprintCallable, Category = "Profile|Export")
    FMGProfileExport ExportProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Export")
    bool ImportProfile(const FMGProfileExport& ExportData);

    UFUNCTION(BlueprintCallable, Category = "Profile|Export")
    FString ExportProfileToJson() const;

    UFUNCTION(BlueprintCallable, Category = "Profile|Export")
    bool ImportProfileFromJson(const FString& JsonString);

    // Playtime
    UFUNCTION(BlueprintCallable, Category = "Profile|Playtime")
    void UpdatePlaytime(float DeltaSeconds);

    UFUNCTION(BlueprintPure, Category = "Profile|Playtime")
    float GetTotalPlaytime() const;

    UFUNCTION(BlueprintPure, Category = "Profile|Playtime")
    FString GetFormattedPlaytime() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnProfileLoaded OnProfileLoaded;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnProfileSaved OnProfileSaved;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnLevelUp OnLevelUp;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnAchievementUnlocked OnAchievementUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnAchievementProgress OnAchievementProgress;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnStatUpdated OnStatUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnReputationChanged OnReputationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnFriendStatusChanged OnFriendStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnRaceHistoryAdded OnRaceHistoryAdded;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnCurrencyChanged OnCurrencyChanged;

    UPROPERTY(BlueprintAssignable, Category = "Profile|Events")
    FMGOnProfileMigrated OnProfileMigrated;

protected:
    // Current player profile
    UPROPERTY()
    FMGPlayerProfile CurrentProfile;

    // Profile state
    UPROPERTY()
    bool bHasLoadedProfile;

    UPROPERTY()
    bool bIsDirty;

    // Configuration
    UPROPERTY()
    int32 MaxRaceHistoryEntries;

    UPROPERTY()
    int32 MaxControlPresets;

    UPROPERTY()
    int32 MaxLevel;

    UPROPERTY()
    int32 PrestigeMaxLevel;

    // Autosave
    UPROPERTY()
    float AutosaveInterval;

    UPROPERTY()
    FTimerHandle AutosaveTimerHandle;

    // Helper functions
    void InitializeDefaultProfile();
    void InitializeDefaultStats();
    void InitializeDefaultAchievements();
    void InitializeDefaultControlPreset();

    void PerformAutoSave();
    bool ValidateProfile(const FMGPlayerProfile& Profile) const;
    FString GenerateChecksum(const FMGPlayerProfile& Profile) const;

    int32 CalculateLevelFromExperience(int64 Experience) const;
    void CheckLevelUp(int64 OldExperience, int64 NewExperience);

    void MarkDirty();
};
