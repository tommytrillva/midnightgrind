// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGSpeedtrapSubsystem.h
 * @brief Speed trap system for measuring player performance at designated locations
 *
 * @section overview Overview
 * This file defines the Speedtrap Subsystem for Midnight Grind - a comprehensive
 * system that manages speed cameras, speed zones, jump challenges, and drift zones
 * scattered throughout the game world. Speed traps provide optional challenges that
 * reward skillful driving and encourage exploration.
 *
 * @section what_are_speedtraps What Are Speed Traps?
 * Speed traps are designated areas in the game world that measure and record
 * player performance. They come in various types:
 * - **Speed Cameras**: Single-point measurement of instantaneous speed
 * - **Speed Zones**: Sustained average speed through a section
 * - **Jump Distances**: Measure how far you fly through the air
 * - **Drift Zones**: Score based on drift angle and duration
 *
 * @section concepts Key Concepts for Beginners
 *
 * ### 1. Speed Trap Types (EMGSpeedtrapType)
 * Different challenges test different skills:
 * - Camera: Pass through at maximum speed (instant snapshot)
 * - Zone: Maintain high speed through an entire section
 * - Checkpoint: Hit multiple points in sequence
 * - TopSpeed: Achieve highest possible speed anywhere in zone
 * - Average: Maintain consistent speed (no braking!)
 * - Jump: Launch off a ramp and travel maximum distance
 * - Drift: Chain drifts through a designated area
 * - NearMiss: Pass close to traffic without hitting
 * - Combo: Combine multiple actions for bonus points
 *
 * ### 2. Rating System (EMGSpeedtrapRating)
 * Performance is rated on a tier system:
 * - None: Below minimum threshold
 * - Bronze: Entry level achievement
 * - Silver: Competent performance
 * - Gold: Skilled driving
 * - Platinum: Expert level
 * - Diamond: Exceptional mastery
 * - Legend: Near-perfect execution
 *
 * Each rating tier has:
 * - A threshold value (e.g., 150 mph for Gold)
 * - Point rewards that increase with tier
 *
 * ### 3. Speed Trap States (EMGSpeedtrapState)
 * Tracks the player's interaction with a speed trap:
 * - Inactive: Player not near the speed trap
 * - Active: Player is in detection range but hasn't entered
 * - InProgress: Player is currently in a speed zone
 * - Completed: Successfully finished the challenge
 * - Failed: Exited zone early, crashed, or slowed too much
 *
 * ### 4. Speed Trap Definition (FMGSpeedtrapDefinition)
 * The complete setup for a speed trap:
 * - SpeedtrapId: Unique identifier
 * - Type: What kind of challenge (camera, zone, jump, etc.)
 * - StartLocation/EndLocation: Physical boundaries
 * - TriggerWidth/TriggerHeight: Detection zone size
 * - Rating thresholds: Bronze through Legend requirements
 * - Point values: Rewards for each rating tier
 * - TimeLimit: Optional time constraint for zones
 *
 * ### 5. Speed Trap Attempt (FMGSpeedtrapAttempt)
 * Record of a single attempt:
 * - RecordedValue: The measured performance (speed, distance, etc.)
 * - MaxSpeed/AverageSpeed: Speed statistics
 * - EntrySpeed/ExitSpeed: Velocities at boundaries
 * - Rating: What tier was achieved
 * - PointsEarned: Reward for this attempt
 * - bIsPersonalBest/bIsWorldRecord: Achievement flags
 * - DeltaFromBest: How close to beating your record
 *
 * ### 6. Active Speed Trap Progress (FMGActiveSpeedtrap)
 * Real-time tracking during a zone:
 * - CurrentValue: Live measurement being accumulated
 * - MaxValue: Highest point during this attempt
 * - TimeRemaining/DistanceRemaining: Progress indicators
 * - CurrentRating: What tier you're trending toward
 *
 * ### 7. Records (FMGSpeedtrapRecord)
 * Persistent best performances:
 * - PersonalBest: Your highest recorded value
 * - WorldRecord: Global best (from all players)
 * - FriendBest: Best among your friends
 * - TotalAttempts/TotalCompletions: Usage statistics
 *
 * ### 8. Player Stats (FMGSpeedtrapPlayerStats)
 * Overall progress tracking:
 * - TotalSpeedtrapsFound: Discovery progress
 * - TotalGoldRatings (etc.): Count at each tier
 * - TotalPoints: Cumulative score
 * - HighestSpeedRecorded: All-time personal best speed
 *
 * @section workflow Typical Workflow
 * @code
 * // Get the speedtrap subsystem
 * UMGSpeedtrapSubsystem* Speedtrap = GetGameInstance()->GetSubsystem<UMGSpeedtrapSubsystem>();
 *
 * // Called every frame from vehicle to check for speed traps
 * Speedtrap->UpdateSpeedtrapDetection(PlayerId, Location, Velocity, DeltaTime);
 *
 * // When approaching a speed camera, system automatically:
 * // 1. Fires OnSpeedtrapEntered when player enters trigger zone
 * // 2. Records speed at the exact measurement point
 * // 3. Fires OnSpeedtrapRecorded with result and rating
 * // 4. Fires OnSpeedtrapNewPersonalBest if applicable
 *
 * // For speed zones, the flow is:
 * // 1. OnSpeedtrapEntered when entering the zone
 * // 2. OnSpeedtrapProgress fires periodically with live data
 * // 3. OnSpeedtrapExited when leaving (with Attempt data)
 *
 * // Check current progress during a zone
 * if (Speedtrap->HasActiveSpeedtrap(PlayerId))
 * {
 *     FMGActiveSpeedtrap Active = Speedtrap->GetActiveSpeedtrap(PlayerId);
 *     float CurrentSpeed = Active.CurrentValue;
 *     EMGSpeedtrapRating CurrentRating = Active.CurrentRating;
 *     // Update HUD with live feedback
 * }
 * @endcode
 *
 * @section discovery Discovery System
 * Speed traps can be hidden until discovered:
 * @code
 * // Check if a speed trap has been found
 * if (!Speedtrap->IsSpeedtrapDiscovered(SpeedtrapId))
 * {
 *     // Show "???" on map instead of details
 * }
 *
 * // Mark as discovered when player passes through first time
 * Speedtrap->DiscoverSpeedtrap(SpeedtrapId);
 * // Fires OnSpeedtrapDiscovered delegate
 *
 * // Track discovery progress
 * int32 Found = Speedtrap->GetTotalSpeedtrapsDiscovered();
 * float CompletionPercent = Speedtrap->GetCompletionPercentage();
 * @endcode
 *
 * @section leaderboards Leaderboards
 * Compare performance with other players:
 * @code
 * // Get global top 10
 * TArray<FMGSpeedtrapLeaderboardEntry> TopScores = Speedtrap->GetLeaderboard(SpeedtrapId, 10);
 *
 * // Get friends leaderboard
 * TArray<FString> FriendIds = { "friend1", "friend2", "friend3" };
 * TArray<FMGSpeedtrapLeaderboardEntry> FriendsScores = Speedtrap->GetFriendsLeaderboard(SpeedtrapId, FriendIds);
 *
 * // Check your rank
 * int32 MyRank = Speedtrap->GetPlayerRank(SpeedtrapId);
 * @endcode
 *
 * @section rating_calc Rating Calculation
 * @code
 * // Calculate what rating a value would achieve
 * EMGSpeedtrapRating Rating = Speedtrap->CalculateRating(SpeedtrapId, 185.0f); // Returns Gold
 *
 * // Get threshold for next tier
 * float NextThreshold = Speedtrap->GetNextRatingThreshold(SpeedtrapId, EMGSpeedtrapRating::Gold);
 * // Returns 200.0f (Platinum threshold)
 *
 * // Get points for a rating
 * int32 Points = Speedtrap->GetPointsForRating(SpeedtrapId, EMGSpeedtrapRating::Platinum);
 * @endcode
 *
 * @section speed_zones Speed Zone Configuration
 * @code
 * // Configure speed zone behavior
 * FMGSpeedZoneConfig Config;
 * Config.MinSpeedMPH = 50.0f;        // Minimum speed to stay in zone
 * Config.SpeedDecayRate = 10.0f;      // How fast speed requirement drops if you slow down
 * Config.ComboMultiplierPerZone = 0.1f; // Bonus for chaining zones
 * Config.NearMissBonusPercent = 10.0f;  // Extra points for near misses
 * Config.DriftBonusPercent = 15.0f;     // Extra points for drifting through
 * Speedtrap->SetSpeedZoneConfig(Config);
 * @endcode
 *
 * @section events Delegate Events
 * Subscribe to these for real-time updates:
 * - OnSpeedtrapEntered: Player entered a speed trap zone
 * - OnSpeedtrapExited: Player left a speed trap zone
 * - OnSpeedtrapRecorded: A speed camera recorded a value
 * - OnSpeedtrapNewPersonalBest: Player beat their previous best
 * - OnSpeedtrapNewWorldRecord: Player set a new global record
 * - OnSpeedtrapRatingAchieved: Player earned a rating tier
 * - OnSpeedtrapDiscovered: Player found a new speed trap
 * - OnSpeedtrapProgress: Live updates during a zone
 *
 * @section units Unit Conversion
 * The system uses cm/s internally but provides conversions:
 * @code
 * float SpeedMPH = Speedtrap->ConvertToMPH(VelocityCMPerSec);
 * float SpeedKPH = Speedtrap->ConvertToKPH(VelocityCMPerSec);
 * FText DisplayText = Speedtrap->FormatSpeed(SpeedMPH, bUseMetric);
 * @endcode
 *
 * @section persistence Data Persistence
 * Progress is saved automatically:
 * @code
 * Speedtrap->SaveSpeedtrapData();  // Save all progress
 * Speedtrap->LoadSpeedtrapData();  // Load on game start
 * @endcode
 *
 * @see UMGStatsTracker - Records speed trap attempts as part of overall stats
 * @see EMGSpeedtrapType - All challenge types
 * @see EMGSpeedtrapRating - All rating tiers
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSpeedtrapSubsystem.generated.h"

/**
 * Speed trap type
 */
UENUM(BlueprintType)
enum class EMGSpeedtrapType : uint8
{
	Camera				UMETA(DisplayName = "Speed Camera"),
	Zone				UMETA(DisplayName = "Speed Zone"),
	Checkpoint			UMETA(DisplayName = "Speed Checkpoint"),
	TopSpeed			UMETA(DisplayName = "Top Speed"),
	Average				UMETA(DisplayName = "Average Speed"),
	Jump				UMETA(DisplayName = "Jump Distance"),
	Drift				UMETA(DisplayName = "Drift Zone"),
	NearMiss			UMETA(DisplayName = "Near Miss Zone"),
	Combo				UMETA(DisplayName = "Combo Zone")
};

/**
 * Speed trap rating
 */
UENUM(BlueprintType)
enum class EMGSpeedtrapRating : uint8
{
	None				UMETA(DisplayName = "None"),
	Bronze				UMETA(DisplayName = "Bronze"),
	Silver				UMETA(DisplayName = "Silver"),
	Gold				UMETA(DisplayName = "Gold"),
	Platinum			UMETA(DisplayName = "Platinum"),
	Diamond				UMETA(DisplayName = "Diamond"),
	Legend				UMETA(DisplayName = "Legend")
};

/**
 * Speed trap state
 */
UENUM(BlueprintType)
enum class EMGSpeedtrapState : uint8
{
	Inactive			UMETA(DisplayName = "Inactive"),
	Active				UMETA(DisplayName = "Active"),
	InProgress			UMETA(DisplayName = "In Progress"),
	Completed			UMETA(DisplayName = "Completed"),
	Failed				UMETA(DisplayName = "Failed")
};

/**
 * Speed trap definition
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapType Type = EMGSpeedtrapType::Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerWidth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerHeight = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoneLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RequiredDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DirectionTolerance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverThreshold = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldThreshold = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlatinumThreshold = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiamondThreshold = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LegendThreshold = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverPoints = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldPoints = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumPoints = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiamondPoints = 2500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LegendPoints = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTimeLimit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VisualAsset;
};

/**
 * Speed trap attempt
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapAttempt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttemptId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecordedValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExitSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeTaken = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating Rating = EMGSpeedtrapRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWorldRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaFromBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Active speed trap progress
 */
USTRUCT(BlueprintType)
struct FMGActiveSpeedtrap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapState State = EMGSpeedtrapState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeElapsed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SampleCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedSum = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating CurrentRating = EMGSpeedtrapRating::None;
};

/**
 * Speed trap record
 *
 * Stores the player's performance history for a specific speed trap,
 * including personal bests, world records, and attempt statistics.
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapRecord
{
	GENERATED_BODY()

	/// Speedtrap this record is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString SpeedtrapId;

	/// Player's personal best value (speed, distance, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float PersonalBest = 0.0f;

	/// Current world record value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float WorldRecord = 0.0f;

	/// Best value among friends
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float FriendBest = 0.0f;

	/// Player name who holds the world record
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	FString WorldRecordHolder;

	/// Player name who holds the friend best
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	FString FriendRecordHolder;

	/// Best rating achieved on this speedtrap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ratings")
	EMGSpeedtrapRating BestRating = EMGSpeedtrapRating::None;

	/// Total number of attempts on this speedtrap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalAttempts = 0;

	/// Number of successful completions (achieved at least Bronze)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalCompletions = 0;

	/// Number of attempts that achieved a rating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 SuccessfulAttempts = 0;

	/// Total points earned from this speedtrap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 TotalPointsEarned = 0;

	/// When the personal best was achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	FDateTime PersonalBestDate;

	/// History of recent attempt values (for graphs/progress tracking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "History")
	TArray<float> AttemptHistory;
};

/**
 * Player speed trap stats
 *
 * Tracks overall player progress across all speed traps including:
 * - Discovery progress
 * - Rating tier counts
 * - Total points earned
 * - Personal records
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapPlayerStats
{
	GENERATED_BODY()

	/// Unique player identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString PlayerId;

	/// Per-speedtrap records keyed by SpeedtrapId
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	TMap<FString, FMGSpeedtrapRecord> Records;

	/// Number of unique speed traps discovered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 TotalSpeedtrapsFound = 0;

	/// Total number of successful completions across all speedtraps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 TotalSpeedtrapCompletions = 0;

	/// Number of Gold ratings achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ratings")
	int32 TotalGoldRatings = 0;

	/// Number of Platinum ratings achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ratings")
	int32 TotalPlatinumRatings = 0;

	/// Number of Diamond ratings achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ratings")
	int32 TotalDiamondRatings = 0;

	/// Number of Legend ratings achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ratings")
	int32 TotalLegendRatings = 0;

	/// Total points earned across all speedtraps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Points")
	int32 TotalPoints = 0;

	/// Cumulative points earned (for save/load)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Points")
	int32 TotalPointsEarned = 0;

	/// Highest speed ever recorded (mph)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float HighestSpeedRecorded = 0.0f;

	/// Longest jump distance ever recorded
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float LongestJumpRecorded = 0.0f;

	/// Total distance traveled at high speed (for tracking engagement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float TotalDistanceAtSpeed = 0.0f;

	/// Count of ratings achieved per tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ratings")
	TMap<EMGSpeedtrapRating, int32> RatingCounts;

	/// List of discovered speedtrap IDs (for save/load)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FString> DiscoveredSpeedtraps;
};

/**
 * Speed zone configuration
 */
USTRUCT(BlueprintType)
struct FMGSpeedZoneConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedMPH = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedDecayRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplierPerZone = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxComboMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearMissBonusPercent = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftBonusPercent = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OvertakeBonusPercent = 20.0f;
};

/**
 * Leaderboard entry
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpeedtrapId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecordValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSpeedtrapRating Rating = EMGSpeedtrapRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordDate;
};

//=============================================================================
// Wrapper Structs for TMap Value Types
//=============================================================================

/**
 * @brief Wrapper for TArray<FMGSpeedtrapLeaderboardEntry> to support UPROPERTY in TMap.
 */
USTRUCT(BlueprintType)
struct FMGSpeedtrapLeaderboardEntryArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGSpeedtrapLeaderboardEntry> Entries;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapEntered, const FString&, SpeedtrapId, const FString&, PlayerId, float, EntrySpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapExited, const FString&, SpeedtrapId, const FMGSpeedtrapAttempt&, Attempt, bool, bCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapRecorded, const FString&, SpeedtrapId, float, RecordedValue, EMGSpeedtrapRating, Rating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeedtrapNewPersonalBest, const FString&, SpeedtrapId, float, OldBest, float, NewBest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapNewWorldRecord, const FString&, SpeedtrapId, float, RecordValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapRatingAchieved, EMGSpeedtrapRating, Rating, int32, TotalAtRating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapDiscovered, const FString&, SpeedtrapId, int32, TotalDiscovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedtrapProgress, const FString&, SpeedtrapId, float, Progress);

/**
 * Speed Trap Subsystem
 * Manages speed cameras, speed zones, and speed challenges
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSpeedtrapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapEntered OnSpeedtrapEntered;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapExited OnSpeedtrapExited;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapRecorded OnSpeedtrapRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapNewPersonalBest OnSpeedtrapNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapNewWorldRecord OnSpeedtrapNewWorldRecord;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapRatingAchieved OnSpeedtrapRatingAchieved;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapDiscovered OnSpeedtrapDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Speedtrap|Events")
	FOnSpeedtrapProgress OnSpeedtrapProgress;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Registration")
	void RegisterSpeedtrap(const FMGSpeedtrapDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Registration")
	void UnregisterSpeedtrap(const FString& SpeedtrapId);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	FMGSpeedtrapDefinition GetSpeedtrap(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	TArray<FMGSpeedtrapDefinition> GetAllSpeedtraps() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	TArray<FMGSpeedtrapDefinition> GetSpeedtrapsInArea(FVector Center, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Registration")
	TArray<FMGSpeedtrapDefinition> GetSpeedtrapsForTrack(const FString& TrackId) const;

	// Detection
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Detection")
	void UpdateSpeedtrapDetection(const FString& PlayerId, FVector Location, FVector Velocity, float MGDeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Detection")
	bool TryEnterSpeedtrap(const FString& PlayerId, const FString& SpeedtrapId, float Speed, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Detection")
	void ExitSpeedtrap(const FString& PlayerId, const FString& SpeedtrapId, float ExitSpeed);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Detection")
	bool IsInSpeedtrapTrigger(FVector Location, const FMGSpeedtrapDefinition& Speedtrap) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Detection")
	bool IsInSpeedtrapZone(FVector Location, const FMGSpeedtrapDefinition& Speedtrap) const;

	// Active State
	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	FMGActiveSpeedtrap GetActiveSpeedtrap(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	bool HasActiveSpeedtrap(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	float GetCurrentSpeed(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|State")
	EMGSpeedtrapRating GetCurrentRating(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|State")
	void CancelActiveSpeedtrap(const FString& PlayerId);

	// Recording
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Recording")
	FMGSpeedtrapAttempt RecordCameraSpeed(const FString& PlayerId, const FString& SpeedtrapId, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Recording")
	FMGSpeedtrapAttempt RecordZoneCompletion(const FString& PlayerId, const FString& SpeedtrapId);

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Recording")
	FMGSpeedtrapAttempt RecordJumpDistance(const FString& PlayerId, const FString& SpeedtrapId, float Distance);

	// Rating Calculation
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	EMGSpeedtrapRating CalculateRating(const FString& SpeedtrapId, float Value) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	int32 GetPointsForRating(const FString& SpeedtrapId, EMGSpeedtrapRating Rating) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	float GetThresholdForRating(const FString& SpeedtrapId, EMGSpeedtrapRating Rating) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Rating")
	float GetNextRatingThreshold(const FString& SpeedtrapId, EMGSpeedtrapRating CurrentRating) const;

	// Records
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Records")
	FMGSpeedtrapRecord GetSpeedtrapRecord(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Records")
	float GetPersonalBest(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Records")
	float GetWorldRecord(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Records")
	void SetWorldRecord(const FString& SpeedtrapId, float Value, const FString& PlayerName);

	// Leaderboards
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Leaderboard")
	TArray<FMGSpeedtrapLeaderboardEntry> GetLeaderboard(const FString& SpeedtrapId, int32 MaxEntries = 10) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Leaderboard")
	TArray<FMGSpeedtrapLeaderboardEntry> GetFriendsLeaderboard(const FString& SpeedtrapId, const TArray<FString>& FriendIds) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Leaderboard")
	int32 GetPlayerRank(const FString& SpeedtrapId) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	FMGSpeedtrapPlayerStats GetPlayerStats() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	int32 GetTotalSpeedtrapsDiscovered() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	int32 GetTotalRatingsAtLevel(EMGSpeedtrapRating Rating) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Stats")
	float GetCompletionPercentage() const;

	// Discovery
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Discovery")
	void DiscoverSpeedtrap(const FString& SpeedtrapId);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Discovery")
	bool IsSpeedtrapDiscovered(const FString& SpeedtrapId) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Discovery")
	TArray<FString> GetDiscoveredSpeedtraps() const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Discovery")
	TArray<FString> GetUndiscoveredSpeedtraps() const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Config")
	void SetSpeedZoneConfig(const FMGSpeedZoneConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Config")
	FMGSpeedZoneConfig GetSpeedZoneConfig() const;

	// Unit Conversion
	UFUNCTION(BlueprintPure, Category = "Speedtrap|Utility")
	float ConvertToMPH(float CMPerSecond) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Utility")
	float ConvertToKPH(float CMPerSecond) const;

	UFUNCTION(BlueprintPure, Category = "Speedtrap|Utility")
	FText FormatSpeed(float SpeedMPH, bool bUseMetric = false) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Persistence")
	void SaveSpeedtrapData();

	UFUNCTION(BlueprintCallable, Category = "Speedtrap|Persistence")
	void LoadSpeedtrapData();

protected:
	void UpdateActiveZone(const FString& PlayerId, float Speed, float MGDeltaTime);
	FMGSpeedtrapAttempt FinalizeAttempt(const FString& PlayerId, const FString& SpeedtrapId, float RecordedValue);
	void UpdateRecords(const FMGSpeedtrapAttempt& Attempt);
	void CheckForRatingAchievement(EMGSpeedtrapRating Rating);
	FString GenerateAttemptId() const;

private:
	UPROPERTY()
	TMap<FString, FMGSpeedtrapDefinition> RegisteredSpeedtraps;

	UPROPERTY()
	TMap<FString, FMGActiveSpeedtrap> ActiveSpeedtraps;

	UPROPERTY()
	FMGSpeedtrapPlayerStats PlayerStats;

	UPROPERTY()
	TArray<FString> DiscoveredSpeedtrapIds;

	UPROPERTY()
	TMap<FString, FMGSpeedtrapLeaderboardEntryArray> Leaderboards;

	UPROPERTY()
	FMGSpeedZoneConfig SpeedZoneConfig;

	UPROPERTY()
	int32 AttemptCounter = 0;

	FTimerHandle SpeedtrapTickTimer;
};
