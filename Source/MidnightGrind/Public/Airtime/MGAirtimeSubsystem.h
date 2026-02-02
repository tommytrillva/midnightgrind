// Copyright Epic Games, Inc. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGAirtimeSubsystem.h
 * =============================================================================
 *
 * PURPOSE:
 * This file defines the Airtime Subsystem, which manages jump-based gameplay
 * mechanics. While the Stunt Subsystem focuses on tricks and rotations, the
 * Airtime Subsystem specializes in ramp-based jumps, distance tracking, and
 * jump ratings.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. RAMPS:
 *    - Ramps are world objects that launch vehicles into the air
 *    - Each ramp has defined thresholds for Bronze/Silver/Gold/etc. ratings
 *    - Ramps can be secret (hidden, must be discovered) or regular
 *    - The system tracks personal bests and world records per ramp
 *
 * 2. AIRTIME vs STUNTS:
 *    - AIRTIME (this system): Focuses on the jump itself - how far, how high
 *    - STUNTS (MGStuntSubsystem): Focuses on what you DO while airborne
 *    - Both systems can work together (e.g., trick score + distance score)
 *
 * 3. TRICKS (in this context):
 *    - Tricks are pre-defined aerial maneuvers with input requirements
 *    - Different from stunt detection - these are intentional player actions
 *    - Can be chained for multipliers
 *
 * 4. LANDING QUALITY:
 *    - How well you land affects your score multiplier
 *    - Perfect landing = full points + bonus
 *    - Crash landing = lose most or all points
 *    - Also affects speed retention after landing
 *
 * 5. RATINGS:
 *    - Bronze/Silver/Gold/Platinum/Diamond/Legend tiers
 *    - Based on distance traveled (configurable per ramp)
 *    - Gives players clear progression goals for each jump
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Ramp Actors in World]
 *          |
 *          v
 *    [Vehicle enters ramp trigger]
 *          |
 *          v
 *    [MGAirtimeSubsystem] -- This file! Tracks jump metrics, calculates ratings
 *          |
 *          +---> [MGStuntSubsystem] -- For trick scoring during jump
 *          +---> [Score/Points System] -- Awards points based on rating
 *          +---> [Leaderboards] -- Records and world records
 *          +---> [UI System] -- Shows distance, rating, personal best
 *          +---> [Progression] -- Tracks discovered ramps, achievements
 *
 * DIFFERENCE FROM STUNT SUBSYSTEM:
 * - Stunt Subsystem: "What tricks did you do?"
 * - Airtime Subsystem: "How far/high did you jump from this ramp?"
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAirtimeSubsystem.generated.h"

/**
 * EMGJumpType - Categorizes different types of jump sources
 *
 * Each jump type may have different characteristics:
 * - Ramp: Standard launch ramp placed in the world
 * - Terrain: Natural terrain feature that causes a jump
 * - Bump: Small obstacle that briefly lifts the vehicle
 * - Kicker: Sharply angled ramp for high launches
 * - MegaRamp: Large ramp for massive distance/height
 * - HalfPipe: Curved surface for back-and-forth jumps
 * - Billboard: Destructible billboard that can be jumped through
 * - Rooftop: Building rooftop used as a launch point
 * - Shortcut: Jump that provides a shortcut through the course
 * - SecretJump: Hidden ramp that must be discovered
 */
UENUM(BlueprintType)
enum class EMGJumpType : uint8
{
	None				UMETA(DisplayName = "None"),
	Ramp				UMETA(DisplayName = "Ramp Jump"),
	Terrain				UMETA(DisplayName = "Terrain Jump"),
	Bump				UMETA(DisplayName = "Bump"),
	Kicker				UMETA(DisplayName = "Kicker"),
	MegaRamp			UMETA(DisplayName = "Mega Ramp"),
	HalfPipe			UMETA(DisplayName = "Half Pipe"),
	Billboard			UMETA(DisplayName = "Billboard"),
	Rooftop				UMETA(DisplayName = "Rooftop"),
	Shortcut			UMETA(DisplayName = "Shortcut Jump"),
	SecretJump			UMETA(DisplayName = "Secret Jump")
};

/**
 * EMGLandingQuality - How well the player landed after a jump
 *
 * Landing quality directly affects:
 * 1. Score multiplier (Perfect = 2x, Crash = 0x)
 * 2. Speed retention (Perfect keeps most speed, Crash loses speed)
 * 3. Visual/audio feedback
 *
 * Detection is based on:
 * - Vehicle angle relative to landing surface
 * - Impact velocity
 * - Whether all wheels touch down smoothly
 */
UENUM(BlueprintType)
enum class EMGLandingQuality : uint8
{
	Perfect				UMETA(DisplayName = "Perfect"),
	Great				UMETA(DisplayName = "Great"),
	Good				UMETA(DisplayName = "Good"),
	Rough				UMETA(DisplayName = "Rough"),
	Bad					UMETA(DisplayName = "Bad"),
	Crash				UMETA(DisplayName = "Crash")
};

/**
 * EMGAirtimeTrick - Types of tricks that can be performed during a jump
 *
 * These are intentional player-triggered tricks, not auto-detected stunts.
 * Players input specific controls to execute these maneuvers.
 *
 * ROTATION TRICKS:
 * - Barrel: Roll around the forward axis
 * - Flip: Rotate end-over-end
 * - Spin: Rotate around the vertical axis
 * - Corkscrew: Diagonal rotation combining roll and flip
 * - FlatSpin: Horizontal spinning while level
 *
 * POSITION TRICKS:
 * - Invert: Turn the vehicle upside down
 * - NoseGrab: Tilt nose down (like a skateboard nose grab)
 * - TailGrab: Tilt tail down
 *
 * Tricks can be chained for multiplier bonuses.
 */
UENUM(BlueprintType)
enum class EMGAirtimeTrick : uint8
{
	None				UMETA(DisplayName = "None"),
	Barrel				UMETA(DisplayName = "Barrel Roll"),
	Flip				UMETA(DisplayName = "Flip"),
	Spin				UMETA(DisplayName = "Spin"),
	Corkscrew			UMETA(DisplayName = "Corkscrew"),
	FlatSpin			UMETA(DisplayName = "Flat Spin"),
	Invert				UMETA(DisplayName = "Invert"),
	NoseGrab			UMETA(DisplayName = "Nose Grab"),
	TailGrab			UMETA(DisplayName = "Tail Grab")
};

/**
 * EMGJumpRating - Achievement tier for a jump's distance
 *
 * Each ramp defines distance thresholds for these ratings.
 * Example for a specific ramp:
 * - Bronze: 30 meters
 * - Silver: 50 meters
 * - Gold: 75 meters
 * - Platinum: 100 meters
 * - Diamond: 150 meters
 * - Legend: 200 meters
 *
 * Ratings provide:
 * - Visual feedback (different colors/effects per tier)
 * - Point rewards (higher tier = more points)
 * - Progression tracking (collect all Gold ratings, etc.)
 */
UENUM(BlueprintType)
enum class EMGJumpRating : uint8
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
 * FMGActiveJump - Real-time tracking data for a jump in progress
 *
 * This struct holds all the data being actively measured while the
 * vehicle is airborne after launching from a ramp. It's updated every
 * frame until the vehicle lands.
 *
 * KEY METRICS TRACKED:
 * - AirtimeDuration: How long in the air (seconds)
 * - MaxHeight: Peak height reached above launch point
 * - HorizontalDistance: How far traveled from launch point
 * - LaunchSpeed/Angle: Initial conditions for trajectory
 * - Rotation tracking: For trick detection
 * - Score accumulation: Real-time score calculation
 *
 * RELATIONSHIP TO OTHER STRUCTS:
 * - FMGActiveJump: Current, changing data (this struct)
 * - FMGJumpResult: Finalized data after landing (created from this)
 */
USTRUCT(BlueprintType)
struct FMGActiveJump
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString JumpId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpType Type = EMGJumpType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirtimeDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CurrentRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator TotalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGAirtimeTrick> ActiveTricks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TricksCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearMissWhileAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NearMissCount = 0;
};

/**
 * FMGJumpResult - Finalized data for a completed jump
 *
 * After the vehicle lands, the FMGActiveJump is converted into this
 * permanent record. This is what gets:
 * - Displayed in the UI as the final result
 * - Compared against personal bests
 * - Submitted to leaderboards
 * - Stored in player statistics
 *
 * KEY DATA:
 * - All metrics from the jump (time, height, distance)
 * - Landing quality assessment
 * - Rating achieved (Bronze through Legend)
 * - Score breakdown (base + tricks + landing bonus)
 * - Tricks performed during the jump
 * - Record flags (personal best, world record)
 */
USTRUCT(BlueprintType)
struct FMGJumpResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpType Type = EMGJumpType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirtimeDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandingSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLandingQuality LandingQuality = EMGLandingQuality::Good;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpRating Rating = EMGJumpRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrickScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LandingBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGAirtimeTrick> TricksPerformed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrickCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWorldRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * FMGRampDefinition - Complete definition of a ramp in the game world
 *
 * This struct describes everything about a ramp: where it is, what type,
 * how it behaves, and what scores are needed for each rating tier.
 *
 * RAMP PROPERTIES:
 * - Location/Rotation: World position and orientation
 * - LaunchAngle: Angle of the ramp surface (affects trajectory)
 * - SpeedBoostPercent: Optional speed boost when hitting the ramp
 * - Trigger dimensions: Size of the detection area
 *
 * RATING THRESHOLDS:
 * Each ramp defines custom distance requirements for ratings.
 * A short ramp might have Bronze at 30m, while a mega ramp might
 * have Bronze at 100m.
 *
 * GAMEPLAY FLAGS:
 * - bAllowTricks: Can players perform tricks off this ramp?
 * - bIsSecret: Is this a hidden ramp that must be discovered?
 * - TrackId: Which race track this ramp belongs to (if any)
 *
 * LEVEL DESIGN USAGE:
 * Designers create these definitions for each ramp in their levels.
 * They can tune thresholds to match the ramp's intended difficulty.
 */
USTRUCT(BlueprintType)
struct FMGRampDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpType Type = EMGJumpType::Ramp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBoostPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerWidth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinLaunchSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeDistanceMeters = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverDistanceMeters = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldDistanceMeters = 75;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumDistanceMeters = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiamondDistanceMeters = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LegendDistanceMeters = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowTricks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSecret = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RampAsset;
};

/**
 * FMGTrickDefinition - Definition of a performable trick
 *
 * Each trick type has specific requirements and rewards defined here.
 *
 * TRICK REQUIREMENTS:
 * - MinAirtimeRequired: Minimum air time to attempt this trick
 * - RotationRequired: How much rotation completes the trick (usually 360)
 * - RotationAxis: Which axis the rotation is around
 * - ExecutionTime: How long the trick takes to perform
 *
 * TRICK REWARDS:
 * - BasePoints: Points awarded for completing the trick
 * - ChainMultiplier: Bonus for chaining this trick with others
 *
 * TRICK CHAINING:
 * When bCanChain is true, players can combo multiple tricks.
 * Each subsequent trick in a chain gets the ChainMultiplier applied
 * to its base points.
 *
 * Example: Barrel Roll (100pts) -> Flip (100pts) -> Spin (100pts)
 * Chain: 100 + (100 * 1.2) + (100 * 1.4) = 360 points
 */
USTRUCT(BlueprintType)
struct FMGTrickDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAirtimeTrick Type = EMGAirtimeTrick::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinAirtimeRequired = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationRequired = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationAxis = FRotator(0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExecutionTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainMultiplier = 1.2f;
};

/**
 * FMGAirtimePlayerStats - Cumulative airtime statistics for a player
 *
 * This struct aggregates all jump-related stats for a player's career.
 * Unlike session stats, these persist across play sessions.
 *
 * GENERAL STATS:
 * - TotalJumps: How many jumps completed
 * - TotalAirtime: Cumulative seconds spent airborne
 * - TotalPoints: Total points earned from jumps
 *
 * RECORD STATS:
 * - LongestAirtime: Best single jump duration
 * - HighestJump: Maximum height achieved
 * - LongestDistance: Farthest jump distance
 * - HighestSingleJumpScore: Best score from one jump
 *
 * BREAKDOWN STATS:
 * - TrickCounts: How many of each trick type performed
 * - RatingCounts: How many of each rating achieved
 * - RampBestDistances: Personal best for each ramp
 *
 * DISCOVERY:
 * - SecretRampsFound: How many hidden ramps discovered
 */
USTRUCT(BlueprintType)
struct FMGAirtimePlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalJumps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalAirtime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestAirtime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestJump = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTricks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrashLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestSingleJumpScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGAirtimeTrick, int32> TrickCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGJumpRating, int32> RatingCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> RampBestDistances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SecretRampsFound = 0;
};

/**
 * FMGRampRecord - Player's record data for a specific ramp
 *
 * Each ramp has its own record tracking. This allows for:
 * - Personal best distance per ramp
 * - Comparison to world records
 * - Progress tracking toward ratings
 *
 * DATA TRACKED:
 * - PersonalBestDistance: Player's best distance on this ramp
 * - WorldRecordDistance: Global best (from leaderboards)
 * - PersonalBestScore: Player's highest score on this ramp
 * - BestRating: Highest rating achieved
 * - TotalAttempts: How many times player has used this ramp
 * - SuccessfulLandings: How many attempts ended without crashing
 */
USTRUCT(BlueprintType)
struct FMGRampRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PersonalBestDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorldRecordDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WorldRecordHolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PersonalBestScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpRating BestRating = EMGJumpRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SuccessfulLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime PersonalBestDate;
};

/**
 * FMGAirtimeScoringConfig - Global scoring configuration for airtime
 *
 * This struct defines how points are calculated for all jumps.
 * Designers can tune these values to balance the scoring system.
 *
 * BASE SCORING:
 * Points = (Airtime * PointsPerSecondAirtime)
 *        + (Height * PointsPerMeterHeight)
 *        + (Distance * PointsPerMeterDistance)
 *
 * LANDING MULTIPLIERS:
 * The total is then multiplied based on landing quality:
 * - Perfect: 2.0x
 * - Great: 1.5x
 * - Good: 1.0x
 * - Rough: 0.5x
 * - Bad: 0.25x
 * - Crash: 0.0x (lose all points!)
 *
 * TRICK CHAIN BONUSES:
 * Each trick in a chain adds to the multiplier:
 * - First trick: 1.0x
 * - Second trick: 1.25x (1.0 + 0.25)
 * - Third trick: 1.5x
 * - Maximum: 3.0x (MaxTrickChainMultiplier)
 *
 * SPECIAL BONUSES:
 * - NearMissWhileAirborneBonus: Extra points for close calls in air
 * - SpeedBonusMultiplier: Bonus for high-speed launches
 */
USTRUCT(BlueprintType)
struct FMGAirtimeScoringConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsPerSecondAirtime = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsPerMeterHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsPerMeterDistance = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectLandingMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatLandingMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodLandingMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoughLandingMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BadLandingMultiplier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrashLandingMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrickChainMultiplierPerTrick = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTrickChainMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearMissWhileAirborneBonus = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusMultiplier = 1.25f;
};

/**
 * FMGLandingConfig - Configuration for landing quality detection
 *
 * This struct defines the tolerances and thresholds for determining
 * how well a player landed after a jump.
 *
 * ANGLE TOLERANCES:
 * These define how far off from "perfect" the vehicle can be:
 * - PerfectAngleTolerance: 5 degrees = perfect landing
 * - GreatAngleTolerance: 15 degrees = great landing
 * - GoodAngleTolerance: 30 degrees = good landing
 * - RoughAngleTolerance: 45 degrees = rough landing
 * - CrashAngleThreshold: 60+ degrees = crash
 *
 * PHYSICS SETTINGS:
 * - MinGroundCheckDistance: How far to raycast for ground detection
 * - LandingImpactThreshold: Impact velocity that affects landing
 *
 * SPEED RETENTION:
 * After landing, the vehicle keeps a percentage of its speed:
 * - PerfectSpeedRetention: 95% (keep almost all speed)
 * - CrashSpeedLoss: 50% (lose half speed on crash)
 *
 * TUNING:
 * Wider tolerances = easier landings, more forgiving gameplay
 * Tighter tolerances = harder landings, more skill required
 */
USTRUCT(BlueprintType)
struct FMGLandingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectAngleTolerance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatAngleTolerance = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodAngleTolerance = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoughAngleTolerance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrashAngleThreshold = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinGroundCheckDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandingImpactThreshold = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectSpeedRetention = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrashSpeedLoss = 0.5f;
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================
// These delegates allow other systems to respond to airtime events.
// Subscribe to these in Blueprint or C++ to receive notifications.
//
// NOTE: All delegates include PlayerId to support multiplayer - even in
// single-player, the player has an ID for consistency.
// =============================================================================

// Fired when a player launches off a ramp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnJumpStarted, const FString&, PlayerId, EMGJumpType, Type, float, LaunchSpeed);

// Fired when a jump is complete (player has landed)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJumpEnded, const FString&, PlayerId, const FMGJumpResult&, Result);

// Fired every frame while airborne (for real-time UI updates)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAirtimeUpdate, const FString&, PlayerId, float, CurrentAirtime, float, CurrentHeight);

// Fired when a trick is successfully completed mid-air
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrickCompleted, const FString&, PlayerId, EMGAirtimeTrick, Trick, int32, PointsEarned);

// Fired when a trick chain is extended (multiple tricks in one jump)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrickChain, const FString&, PlayerId, int32, ChainCount, float, ChainMultiplier);

// Fired when the player lands (with quality assessment)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLanding, const FString&, PlayerId, EMGLandingQuality, Quality, int32, LandingBonus);

// Fired when a rating is achieved for a jump
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnJumpRating, const FString&, PlayerId, EMGJumpRating, Rating, const FString&, RampId);

// Fired when player beats their personal best on a ramp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewPersonalBest, const FString&, PlayerId, const FString&, RampId, float, NewDistance);

// Fired when a hidden ramp is discovered for the first time
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSecretRampFound, const FString&, PlayerId, const FString&, RampId);

// Fired when player reaches a new maximum height during a jump
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHeightReached, const FString&, PlayerId, float, Height);

// Fired when player has a near-miss with an obstacle while in the air
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearMissWhileAirborne, const FString&, PlayerId, float, BonusMultiplier);

/**
 * UMGAirtimeSubsystem - Manages ramp-based jumps and airtime mechanics
 *
 * This subsystem handles all aspects of ramp jumps:
 * - Ramp registration and detection
 * - Jump tracking (distance, height, airtime)
 * - Trick performance and chaining
 * - Landing quality assessment
 * - Rating calculation and records
 * - Statistics and progression
 *
 * DIFFERENCE FROM STUNT SUBSYSTEM:
 * - Stunt Subsystem: General aerial tricks, rotation detection, combos
 * - Airtime Subsystem: Ramp-specific jumps, distance ratings, records
 *
 * They complement each other: A player can get a "Gold" rating from
 * this system AND a "Barrel Roll" bonus from the Stunt system on
 * the same jump.
 *
 * HOW TO ACCESS:
 *   UMGAirtimeSubsystem* AirtimeSystem = GetGameInstance()->GetSubsystem<UMGAirtimeSubsystem>();
 *
 * TYPICAL FLOW:
 * 1. Level loads, ramps call RegisterRamp() to add themselves
 * 2. Player drives into ramp trigger
 * 3. CheckRampLaunch() detects launch and calls StartJump()
 * 4. Every frame: UpdateJump() tracks position, tricks
 * 5. Player lands: EndJump() calculates results and awards points
 * 6. Results compared to records, events broadcast
 *
 * MULTIPLAYER SUPPORT:
 * All functions take a PlayerId parameter, allowing the system to
 * track multiple players' jumps simultaneously.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAirtimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnJumpStarted OnJumpStarted;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnJumpEnded OnJumpEnded;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnAirtimeUpdate OnAirtimeUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnTrickCompleted OnTrickCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnTrickChain OnTrickChain;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnLanding OnLanding;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnJumpRating OnJumpRating;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnSecretRampFound OnSecretRampFound;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnMaxHeightReached OnMaxHeightReached;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnNearMissWhileAirborne OnNearMissWhileAirborne;

	// =============================================================================
	// RAMP REGISTRATION
	// =============================================================================
	// These functions manage the collection of ramps known to the system.
	// Ramp actors should call RegisterRamp when they're spawned/loaded.
	// =============================================================================

	/**
	 * RegisterRamp - Add a ramp to the system
	 * Called by ramp actors in BeginPlay to make themselves trackable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Ramp")
	void RegisterRamp(const FMGRampDefinition& Ramp);

	/**
	 * UnregisterRamp - Remove a ramp from the system
	 * Called when a ramp actor is destroyed (e.g., level unload).
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Ramp")
	void UnregisterRamp(const FString& RampId);

	/** GetRamp - Get a specific ramp's definition by ID */
	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	FMGRampDefinition GetRamp(const FString& RampId) const;

	/** GetAllRamps - Get all registered ramps */
	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	TArray<FMGRampDefinition> GetAllRamps() const;

	/**
	 * GetRampsInArea - Find ramps within a radius of a point
	 * Useful for minimap display or nearby ramp indicators.
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	TArray<FMGRampDefinition> GetRampsInArea(FVector Center, float Radius) const;

	/**
	 * GetRampsForTrack - Get all ramps belonging to a specific race track
	 * Used for track-specific challenges and statistics.
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	TArray<FMGRampDefinition> GetRampsForTrack(const FString& TrackId) const;

	// =============================================================================
	// JUMP DETECTION AND TRACKING
	// =============================================================================
	// Core functions for detecting, tracking, and completing jumps.
	// The vehicle physics system should call these at appropriate times.
	// =============================================================================

	/**
	 * CheckRampLaunch - Check if player just launched from any ramp
	 *
	 * @param PlayerId - Which player to check
	 * @param Location - Current player position
	 * @param Velocity - Current player velocity
	 * @return True if a ramp launch was detected (StartJump was called)
	 *
	 * This checks all ramps and starts a jump if the player is in a
	 * ramp trigger with sufficient velocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	bool CheckRampLaunch(const FString& PlayerId, FVector Location, FVector Velocity);

	/**
	 * StartJump - Manually start tracking a jump (if not using CheckRampLaunch)
	 *
	 * @param PlayerId - Which player is jumping
	 * @param RampId - Which ramp they launched from
	 * @param LaunchPosition - World position at launch
	 * @param LaunchVelocity - Velocity at launch
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	void StartJump(const FString& PlayerId, const FString& RampId, FVector LaunchPosition, FVector LaunchVelocity);

	/**
	 * UpdateJump - Call every frame while player is potentially airborne
	 *
	 * @param PlayerId - Which player
	 * @param Position - Current world position
	 * @param Velocity - Current velocity
	 * @param Rotation - Current rotation (for trick detection)
	 * @param bIsGrounded - Physics ground check result
	 * @param DeltaTime - Frame time
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	void UpdateJump(const FString& PlayerId, FVector Position, FVector Velocity, FRotator Rotation, bool bIsGrounded, float MGDeltaTime);

	/**
	 * EndJump - Finalize a jump when the player lands
	 *
	 * @param PlayerId - Which player
	 * @param LandingPosition - Where they landed
	 * @param LandingVelocity - Velocity at landing (for quality calculation)
	 * @param LandingAngle - Angle of vehicle relative to ground
	 * @return Complete jump result with scores and ratings
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	FMGJumpResult EndJump(const FString& PlayerId, FVector LandingPosition, FVector LandingVelocity, float LandingAngle);

	/** IsAirborne - Check if a player is currently tracked as airborne */
	UFUNCTION(BlueprintPure, Category = "Airtime|Detection")
	bool IsAirborne(const FString& PlayerId) const;

	/** GetActiveJump - Get current jump data for a player (for UI) */
	UFUNCTION(BlueprintPure, Category = "Airtime|Detection")
	FMGActiveJump GetActiveJump(const FString& PlayerId) const;

	// =============================================================================
	// TRICKS
	// =============================================================================
	// Functions for managing and performing tricks during jumps.
	// =============================================================================

	/**
	 * RegisterTrick - Add a trick definition to the system
	 * Called during initialization to set up available tricks.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Tricks")
	void RegisterTrick(const FMGTrickDefinition& Trick);

	/** GetTrickDefinition - Get the definition for a trick type */
	UFUNCTION(BlueprintPure, Category = "Airtime|Tricks")
	FMGTrickDefinition GetTrickDefinition(EMGAirtimeTrick Type) const;

	/**
	 * PerformTrick - Attempt to perform a trick
	 *
	 * @param PlayerId - Which player
	 * @param Trick - Which trick to attempt
	 * @return True if trick was successfully initiated
	 *
	 * Call this when player inputs a trick command. The trick will
	 * be added to the active jump if conditions are met.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Tricks")
	bool PerformTrick(const FString& PlayerId, EMGAirtimeTrick Trick);

	/**
	 * DetectTricks - Auto-detect tricks from rotation
	 *
	 * @param PlayerId - Which player
	 * @param DeltaRotation - Rotation change this frame
	 * @param DeltaTime - Frame time
	 *
	 * This can automatically detect and award tricks based on
	 * vehicle rotation, rather than requiring explicit input.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Tricks")
	void DetectTricks(const FString& PlayerId, FRotator DeltaRotation, float MGDeltaTime);

	/** CanPerformTrick - Check if a trick can be performed right now */
	UFUNCTION(BlueprintPure, Category = "Airtime|Tricks")
	bool CanPerformTrick(const FString& PlayerId, EMGAirtimeTrick Trick) const;

	/** GetAvailableTricks - Get list of tricks currently possible */
	UFUNCTION(BlueprintPure, Category = "Airtime|Tricks")
	TArray<EMGAirtimeTrick> GetAvailableTricks(const FString& PlayerId) const;

	// =============================================================================
	// LANDING
	// =============================================================================
	// Functions for evaluating landing quality and effects.
	// =============================================================================

	/**
	 * CalculateLandingQuality - Determine how well the player landed
	 *
	 * @param Velocity - Velocity at moment of landing
	 * @param SurfaceNormal - Normal of the landing surface
	 * @param VehicleRotation - Vehicle orientation at landing
	 * @return Quality rating from Perfect to Crash
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Landing")
	EMGLandingQuality CalculateLandingQuality(FVector Velocity, FVector SurfaceNormal, FRotator VehicleRotation) const;

	/** GetLandingBonus - Get bonus points for landing quality */
	UFUNCTION(BlueprintPure, Category = "Airtime|Landing")
	int32 GetLandingBonus(EMGLandingQuality Quality) const;

	/**
	 * GetLandingSpeedRetention - How much speed to keep after landing
	 * Perfect landings keep most speed, crashes lose significant speed.
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Landing")
	float GetLandingSpeedRetention(EMGLandingQuality Quality) const;

	// =============================================================================
	// SCORING
	// =============================================================================
	// Functions for calculating scores and ratings.
	// =============================================================================

	/**
	 * CalculateJumpScore - Calculate total score for a jump
	 *
	 * @param Jump - The active jump data
	 * @param Landing - Landing quality
	 * @return Total points for the jump
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Scoring")
	int32 CalculateJumpScore(const FMGActiveJump& Jump, EMGLandingQuality Landing) const;

	/**
	 * CalculateRating - Determine rating based on distance for a specific ramp
	 *
	 * @param RampId - Which ramp (thresholds vary per ramp)
	 * @param Distance - Distance achieved
	 * @return Rating tier achieved
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Scoring")
	EMGJumpRating CalculateRating(const FString& RampId, float Distance) const;

	/**
	 * CalculateTrickScore - Calculate points for a trick with chain bonus
	 *
	 * @param Trick - Which trick
	 * @param ChainCount - Position in trick chain (1 = first trick, etc.)
	 * @return Points for this trick
	 */
	UFUNCTION(BlueprintPure, Category = "Airtime|Scoring")
	int32 CalculateTrickScore(EMGAirtimeTrick Trick, int32 ChainCount) const;

	// =============================================================================
	// RECORDS
	// =============================================================================
	// Functions for accessing and managing personal bests and world records.
	// =============================================================================

	/** GetRampRecord - Get complete record data for a ramp */
	UFUNCTION(BlueprintPure, Category = "Airtime|Records")
	FMGRampRecord GetRampRecord(const FString& RampId) const;

	/** GetPersonalBestDistance - Get player's best distance on a ramp */
	UFUNCTION(BlueprintPure, Category = "Airtime|Records")
	float GetPersonalBestDistance(const FString& RampId) const;

	/** GetWorldRecord - Get the global best distance on a ramp */
	UFUNCTION(BlueprintPure, Category = "Airtime|Records")
	float GetWorldRecord(const FString& RampId) const;

	/**
	 * SetWorldRecord - Set a new world record (from leaderboard sync)
	 * Typically called when receiving data from online services.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Records")
	void SetWorldRecord(const FString& RampId, float Distance, const FString& PlayerName);

	// =============================================================================
	// BONUSES
	// =============================================================================
	// Functions for applying bonus multipliers during jumps.
	// =============================================================================

	/**
	 * RegisterNearMissWhileAirborne - Call when player has near-miss while in air
	 * Adds bonus to the current jump score.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Bonus")
	void RegisterNearMissWhileAirborne(const FString& PlayerId);

	/**
	 * ApplySpeedBonus - Apply bonus for high-speed launches
	 * Called automatically based on launch velocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Bonus")
	void ApplySpeedBonus(const FString& PlayerId, float SpeedMPH);

	// =============================================================================
	// STATS
	// =============================================================================
	// Functions for accessing player statistics.
	// =============================================================================

	/** GetPlayerStats - Get cumulative stats for a player */
	UFUNCTION(BlueprintPure, Category = "Airtime|Stats")
	FMGAirtimePlayerStats GetPlayerStats(const FString& PlayerId) const;

	/** ResetPlayerStats - Clear all stats for a player (use carefully!) */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// =============================================================================
	// DISCOVERY
	// =============================================================================
	// Functions for tracking discovered secret ramps.
	// =============================================================================

	/**
	 * DiscoverSecretRamp - Mark a secret ramp as discovered
	 * Called when player first uses a secret ramp.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Discovery")
	void DiscoverSecretRamp(const FString& PlayerId, const FString& RampId);

	/** IsRampDiscovered - Check if a secret ramp has been found */
	UFUNCTION(BlueprintPure, Category = "Airtime|Discovery")
	bool IsRampDiscovered(const FString& RampId) const;

	/** GetDiscoveredRamps - Get list of all discovered ramp IDs */
	UFUNCTION(BlueprintPure, Category = "Airtime|Discovery")
	TArray<FString> GetDiscoveredRamps() const;

	// =============================================================================
	// CONFIGURATION
	// =============================================================================
	// Functions for getting and setting system configuration.
	// =============================================================================

	/** SetScoringConfig - Set the scoring configuration */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Config")
	void SetScoringConfig(const FMGAirtimeScoringConfig& Config);

	/** GetScoringConfig - Get current scoring configuration */
	UFUNCTION(BlueprintPure, Category = "Airtime|Config")
	FMGAirtimeScoringConfig GetScoringConfig() const;

	/** SetLandingConfig - Set the landing detection configuration */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Config")
	void SetLandingConfig(const FMGLandingConfig& Config);

	/** GetLandingConfig - Get current landing configuration */
	UFUNCTION(BlueprintPure, Category = "Airtime|Config")
	FMGLandingConfig GetLandingConfig() const;

	// =============================================================================
	// UPDATE
	// =============================================================================

	/**
	 * UpdateAirtimeSystem - Main tick function for the subsystem
	 * Called automatically or manually to update all active jumps.
	 */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Update")
	void UpdateAirtimeSystem(float MGDeltaTime);

	// =============================================================================
	// PERSISTENCE
	// =============================================================================

	/** SaveAirtimeData - Save all airtime data to disk */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Persistence")
	void SaveAirtimeData();

	/** LoadAirtimeData - Load saved airtime data from disk */
	UFUNCTION(BlueprintCallable, Category = "Airtime|Persistence")
	void LoadAirtimeData();

protected:
	// =============================================================================
	// PROTECTED HELPER FUNCTIONS
	// =============================================================================
	// Internal functions used by the public API.
	// =============================================================================

	/** TickAirtime - Internal tick called by timer */
	void TickAirtime(float MGDeltaTime);

	/** UpdateActiveJumps - Update all currently active jumps */
	void UpdateActiveJumps(float MGDeltaTime);

	/** UpdateJumpMetrics - Update metrics for a single active jump */
	void UpdateJumpMetrics(FMGActiveJump& Jump, FVector Position, float MGDeltaTime);

	/** FinalizeJump - Create jump result and clean up active jump */
	FMGJumpResult FinalizeJump(const FString& PlayerId, EMGLandingQuality Landing);

	/** UpdateRecords - Check and update personal/world records */
	void UpdateRecords(const FString& PlayerId, const FMGJumpResult& Result);

	/** UpdatePlayerStats - Add jump result to cumulative stats */
	void UpdatePlayerStats(const FString& PlayerId, const FMGJumpResult& Result);

	/** IsInRampTrigger - Check if position is inside a ramp's trigger volume */
	bool IsInRampTrigger(FVector Position, const FMGRampDefinition& Ramp) const;

	/** GenerateJumpId - Create unique ID for a new jump */
	FString GenerateJumpId() const;

	/** GenerateResultId - Create unique ID for a jump result */
	FString GenerateResultId() const;

private:
	// =============================================================================
	// PRIVATE MEMBER VARIABLES
	// =============================================================================

	// All registered ramps, keyed by RampId
	UPROPERTY()
	TMap<FString, FMGRampDefinition> Ramps;

	// Currently active jumps, keyed by PlayerId (one active jump per player)
	UPROPERTY()
	TMap<FString, FMGActiveJump> ActiveJumps;

	// Trick definitions, keyed by trick type
	UPROPERTY()
	TMap<EMGAirtimeTrick, FMGTrickDefinition> TrickDefinitions;

	// Player records for each ramp
	UPROPERTY()
	TMap<FString, FMGRampRecord> RampRecords;

	// Cumulative stats per player
	UPROPERTY()
	TMap<FString, FMGAirtimePlayerStats> PlayerStats;

	// List of discovered secret ramp IDs
	UPROPERTY()
	TArray<FString> DiscoveredRamps;

	// Current scoring configuration
	UPROPERTY()
	FMGAirtimeScoringConfig ScoringConfig;

	// Current landing detection configuration
	UPROPERTY()
	FMGLandingConfig LandingConfig;

	// Counter for generating unique jump IDs
	UPROPERTY()
	int32 JumpCounter = 0;

	// Counter for generating unique result IDs
	UPROPERTY()
	int32 ResultCounter = 0;

	// Timer handle for periodic airtime updates
	FTimerHandle AirtimeTickTimer;
};
