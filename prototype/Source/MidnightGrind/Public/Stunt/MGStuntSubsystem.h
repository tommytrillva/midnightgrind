// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * =============================================================================
 * MGStuntSubsystem.h
 * =============================================================================
 *
 * PURPOSE:
 * This file defines the Stunt Subsystem, which is responsible for tracking and
 * scoring aerial stunts and tricks that players perform while driving vehicles
 * in the game. Think of it as the system that watches when your car goes airborne
 * and rewards you for doing cool things like flips, barrel rolls, and big jumps.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. SUBSYSTEM:
 *    - A Subsystem in Unreal Engine is a singleton-like object that exists for
 *      the lifetime of its outer object (in this case, the GameInstance).
 *    - GameInstanceSubsystem means this system persists across level loads and
 *      is available throughout the entire game session.
 *    - You can access it from anywhere using: GetGameInstance()->GetSubsystem<UMGStuntSubsystem>()
 *
 * 2. STUNTS vs TRICKS:
 *    - "Stunts" refer to the overall aerial maneuver (the jump itself)
 *    - "Tricks" are specific actions performed during a stunt (barrel rolls, flips)
 *    - This system tracks both and combines them for scoring
 *
 * 3. COMBO SYSTEM:
 *    - When players perform multiple stunts in quick succession, they build a "combo"
 *    - Combos multiply the points earned and must be "banked" (saved) before timing out
 *    - If you crash or take too long between stunts, the combo is lost
 *
 * 4. STUNT ZONES:
 *    - Special areas on the map designed for stunts (like ramps, rooftops, canyons)
 *    - These zones give bonus multipliers for performing stunts there
 *    - Can have target scores for players to beat
 *
 * 5. LANDING SYSTEM:
 *    - How you land affects your final score
 *    - Perfect landings give bonuses, crash landings can zero out your points
 *    - The system measures the angle of your vehicle relative to the ground
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Vehicle/Player]
 *          |
 *          v
 *    [Physics Detection] -- Detects when vehicle leaves ground
 *          |
 *          v
 *    [MGStuntSubsystem] -- This file! Tracks air state, rotations, calculates scores
 *          |
 *          +---> [Score/Points System] -- Awards points to player
 *          +---> [Boost System] -- Rewards boost meter for stunts
 *          +---> [UI System] -- Shows stunt names, scores, combos
 *          +---> [Progression System] -- Tracks statistics, achievements
 *
 * RELATED SYSTEMS:
 * - MGAirtimeSubsystem: Handles ramp-based jumps and jump ratings
 * - MGSpeedtrapSubsystem: Handles speed-based challenges
 * - Both can interact with this system (e.g., stunt bonuses during speed zones)
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStuntSubsystem.generated.h"

/**
 * EMGStuntType - Enumeration of all possible stunt/trick types
 *
 * This enum categorizes every type of stunt the player can perform.
 * Each type has different point values and detection requirements.
 *
 * ROTATION-BASED STUNTS:
 * - BarrelRoll: Spinning around the vehicle's forward axis (like a log rolling)
 * - Flip: Rotating end-over-end (front flip or back flip)
 * - FlatSpin: Spinning horizontally while airborne (like a spinning top)
 * - Corkscrew: Combination of roll and flip (diagonal rotation)
 *
 * HEIGHT/TIME-BASED STUNTS:
 * - Jump: Basic jump (minimum air time threshold)
 * - BigAir: Longer air time or higher jump
 * - MassiveAir: Even longer/higher than BigAir
 * - Hangtime: Extended time in the air without rotation
 *
 * CONTEXT-BASED STUNTS:
 * - NearMissAir: Passing close to obstacles while airborne
 * - OncomingAir: Passing oncoming traffic while airborne
 * - DriftJump: Launching into the air from a drift
 * - TwoWheels: Driving on two wheels (not technically airborne)
 *
 * LOCATION-BASED STUNTS:
 * - TrainHop: Jumping over or off a train
 * - BridgeJump: Jumping from a bridge
 * - RoofJump: Jumping from rooftops
 * - CanyonJump: Jumping across a canyon
 * - Signature: Special stunts unique to specific locations
 *
 * LANDING TYPES:
 * - PerfectLanding: Landed cleanly with correct orientation
 * - CrashLanding: Landed poorly, may affect score negatively
 */
UENUM(BlueprintType)
enum class EMGStuntType : uint8
{
	Jump			UMETA(DisplayName = "Jump"),
	BigAir			UMETA(DisplayName = "Big Air"),
	MassiveAir		UMETA(DisplayName = "Massive Air"),
	BarrelRoll		UMETA(DisplayName = "Barrel Roll"),
	Corkscrew		UMETA(DisplayName = "Corkscrew"),
	Flip			UMETA(DisplayName = "Flip"),
	FlatSpin		UMETA(DisplayName = "Flat Spin"),
	TwoWheels		UMETA(DisplayName = "Two Wheels"),
	NearMissAir		UMETA(DisplayName = "Near Miss Air"),
	DriftJump		UMETA(DisplayName = "Drift Jump"),
	OncomingAir		UMETA(DisplayName = "Oncoming Air"),
	Hangtime		UMETA(DisplayName = "Hangtime"),
	PerfectLanding	UMETA(DisplayName = "Perfect Landing"),
	CrashLanding	UMETA(DisplayName = "Crash Landing"),
	TrainHop		UMETA(DisplayName = "Train Hop"),
	BridgeJump		UMETA(DisplayName = "Bridge Jump"),
	RoofJump		UMETA(DisplayName = "Rooftop Jump"),
	CanyonJump		UMETA(DisplayName = "Canyon Jump"),
	Signature		UMETA(DisplayName = "Signature Stunt")
};

/**
 * EMGStuntQuality - Quality rating for how well a stunt was performed
 *
 * This enum represents a tiered rating system for stunts, similar to
 * letter grades or star ratings in other games. Better quality = more points.
 *
 * Quality is calculated based on multiple factors:
 * - Air time (longer = better)
 * - Maximum height reached
 * - Distance traveled
 * - Number of rotations completed
 * - Landing quality
 * - Bonus conditions (near misses, oncoming traffic, etc.)
 *
 * The quality affects:
 * - Base point multiplier
 * - Visual feedback (different UI effects per tier)
 * - Voice announcer callouts
 * - Achievement tracking
 */
UENUM(BlueprintType)
enum class EMGStuntQuality : uint8
{
	Basic			UMETA(DisplayName = "Basic"),
	Good			UMETA(DisplayName = "Good"),
	Great			UMETA(DisplayName = "Great"),
	Awesome			UMETA(DisplayName = "Awesome"),
	Incredible		UMETA(DisplayName = "Incredible"),
	Legendary		UMETA(DisplayName = "Legendary")
};

/**
 * EMGRotationDirection - Direction of vehicle rotation during stunts
 *
 * Used to track which way the vehicle is spinning during aerial maneuvers.
 * This is important for:
 * - Detecting specific trick types (left barrel roll vs right barrel roll)
 * - Bonus points for reversing rotation direction mid-air
 * - Animation and visual effect selection
 *
 * "Both" indicates the player has rotated in multiple directions during
 * a single stunt, which may trigger special bonus points.
 */
UENUM(BlueprintType)
enum class EMGRotationDirection : uint8
{
	None			UMETA(DisplayName = "None"),
	Clockwise		UMETA(DisplayName = "Clockwise"),
	CounterClockwise UMETA(DisplayName = "Counter-Clockwise"),
	Both			UMETA(DisplayName = "Both Directions")
};

/**
 * EMGLandingState - The state/quality of how the vehicle landed
 *
 * Landing is a critical moment in the stunt system - it determines whether
 * you "bank" your points or lose them. This enum tracks the landing outcome.
 *
 * LANDING DETECTION:
 * The system compares your vehicle's orientation when landing versus what
 * would be "correct" for the landing surface:
 * - Perfect: Vehicle is nearly aligned with the ground (< PerfectLandingAngle)
 * - Good: Slightly off but recoverable
 * - Hard: Significant angle difference, may affect vehicle handling
 * - Crash: Too angled, vehicle may flip or take damage
 * - Rollover: Vehicle has flipped completely over
 *
 * POINT EFFECTS:
 * - Perfect: Bonus multiplier on all stunt points
 * - Good: Full points, no bonus
 * - Hard: Reduced points
 * - Crash/Rollover: Points may be zeroed or heavily penalized
 *
 * "Pending" means the vehicle is still airborne and hasn't landed yet.
 */
UENUM(BlueprintType)
enum class EMGLandingState : uint8
{
	Pending			UMETA(DisplayName = "In Air"),
	Perfect			UMETA(DisplayName = "Perfect Landing"),
	Good			UMETA(DisplayName = "Good Landing"),
	Hard			UMETA(DisplayName = "Hard Landing"),
	Crash			UMETA(DisplayName = "Crash Landing"),
	Rollover		UMETA(DisplayName = "Rollover")
};

/**
 * FMGStuntEvent - Complete data for a single stunt occurrence
 *
 * This struct captures everything about one stunt from takeoff to landing.
 * Think of it as a "receipt" for a stunt that records all the details.
 *
 * WHEN IS THIS CREATED?
 * A new FMGStuntEvent is created when:
 * 1. Vehicle leaves the ground (launch detected)
 * 2. System starts tracking air time, rotations, height, etc.
 * 3. Vehicle lands (landing detected)
 * 4. All data is finalized and points are calculated
 *
 * KEY DATA TRACKED:
 * - Physics: Air time, height, distance, launch/landing speeds
 * - Rotations: How many times rotated on each axis (X=roll, Y=pitch, Z=yaw)
 * - Scoring: Base points, bonus points, total points, boost reward
 * - Context: Was drifting? Had near misses? Oncoming traffic?
 * - Location: Exact world coordinates of launch and landing
 *
 * USAGE:
 * - Displayed in UI when stunt completes
 * - Stored in combo system for combo tracking
 * - Saved to statistics for progression
 * - Can be replayed or shared
 */
USTRUCT(BlueprintType)
struct FMGStuntEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStuntType StuntType = EMGStuntType::Jump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStuntQuality Quality = EMGStuntQuality::Basic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLandingState Landing = EMGLandingState::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandingSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RotationsX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RotationsY = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RotationsZ = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostReward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LandingLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHadNearMiss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHadOncoming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BonusTags;
};

/**
 * FMGActiveAirState - Real-time tracking of vehicle state while airborne
 *
 * This struct holds the CURRENT state of an ongoing stunt. Unlike FMGStuntEvent
 * which is a finalized record, this is actively updated every frame while
 * the vehicle is in the air.
 *
 * REAL-TIME TRACKING:
 * Every tick (frame) while airborne, the system updates:
 * - CurrentAirTime: How long we've been airborne (in seconds)
 * - CurrentHeight: Current height above launch point (in Unreal units)
 * - MaxHeight: Highest point reached so far
 * - AccumulatedPitch/Roll/Yaw: Total rotation in each axis
 *
 * LAUNCH DATA:
 * When the vehicle first leaves the ground, we snapshot:
 * - LaunchPosition: World location where we left the ground
 * - LaunchVelocity: Speed and direction at launch
 * - LaunchRotation: Vehicle orientation at launch
 * - LaunchTime: Timestamp for the launch
 *
 * WHY TRACK THIS SEPARATELY FROM FMGStuntEvent?
 * - FMGActiveAirState is the "working memory" - constantly changing
 * - FMGStuntEvent is the "final report" - created once, never changes
 * - This separation allows the UI to show real-time stunt info
 *   while also having a clean record once the stunt completes
 */
USTRUCT(BlueprintType)
struct FMGActiveAirState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentAirTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator LaunchRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CurrentRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccumulatedPitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccumulatedRoll = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccumulatedYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NearMissCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OncomingCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LaunchTime;
};

/**
 * FMGStuntPointConfig - Scoring configuration for a specific stunt type
 *
 * This struct defines how points are calculated for each type of stunt.
 * Designers can tweak these values to balance the game without changing code.
 *
 * SCORING FORMULA (simplified):
 * Total = BasePoints
 *       + (AirTime * AirTimeMultiplier)
 *       + (MaxHeight * HeightMultiplier)
 *       + (Distance * DistanceMultiplier)
 *       + (TotalRotation * RotationMultiplier)
 *       + (LaunchSpeed * SpeedMultiplier)
 *
 * EXAMPLE:
 * For a BigAir stunt with these settings:
 *   BasePoints = 100
 *   AirTimeMultiplier = 10.0
 *
 * If player gets 3 seconds of air time:
 *   Points from air time = 3.0 * 10.0 = 30 bonus points
 *
 * BOOST REWARD:
 * BoostReward is how much boost meter the player gets for this stunt.
 * This creates a gameplay loop: Do stunts -> Get boost -> Go faster ->
 * Hit bigger ramps -> Do bigger stunts
 */
USTRUCT(BlueprintType)
struct FMGStuntPointConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStuntType StuntType = EMGStuntType::Jump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirTimeMultiplier = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeightMultiplier = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationMultiplier = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostReward = 5.0f;
};

/**
 * FMGStuntThresholds - Threshold values for detecting and classifying stunts
 *
 * This struct defines the minimum requirements for various stunt types.
 * These are the "gates" that determine what counts as what.
 *
 * AIR TIME THRESHOLDS:
 * - MinAirTimeForStunt: Below this, it's not even counted as a jump
 * - BigAirTime: Time threshold to upgrade from "Jump" to "Big Air"
 * - MassiveAirTime: Time threshold for "Massive Air"
 *
 * HEIGHT THRESHOLDS:
 * - MinHeightForStunt: Minimum height to qualify as a stunt
 * - BigAirHeight: Height threshold for Big Air rating
 * - MassiveAirHeight: Height threshold for Massive Air rating
 *
 * ROTATION THRESHOLDS:
 * - BarrelRollDegrees: How many degrees of roll = one barrel roll (usually 360)
 * - FlipDegrees: How many degrees of pitch = one flip
 * - FlatSpinDegrees: How many degrees of yaw = one flat spin
 *
 * LANDING ANGLE THRESHOLDS:
 * These define the angle tolerance (in degrees) for landing quality:
 * - PerfectLandingAngle: Max angle deviation for perfect landing
 * - GoodLandingAngle: Max angle for good landing
 * - HardLandingAngle: Max angle for hard landing (above this = crash)
 *
 * TUNING TIP:
 * Lowering thresholds makes stunts easier to achieve.
 * Raising thresholds makes the game more challenging.
 */
USTRUCT(BlueprintType)
struct FMGStuntThresholds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinAirTimeForStunt = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BigAirTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MassiveAirTime = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHeightForStunt = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BigAirHeight = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MassiveAirHeight = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BarrelRollDegrees = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlipDegrees = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlatSpinDegrees = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectLandingAngle = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodLandingAngle = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HardLandingAngle = 45.0f;
};

/**
 * FMGStuntCombo - Tracks an active stunt combo chain
 *
 * Combos are a core gameplay mechanic that rewards players for performing
 * multiple stunts in quick succession without crashing.
 *
 * HOW COMBOS WORK:
 * 1. Player performs first stunt -> Combo starts (ComboCount = 1)
 * 2. Timer starts counting down (TimeRemaining)
 * 3. Player performs another stunt before timer runs out -> ComboCount++
 * 4. Timer resets, multiplier increases
 * 5. Repeat until player banks the combo or timer runs out
 *
 * BANKING VS LOSING:
 * - "Banking" a combo: Landing safely saves your accumulated points
 * - "Losing" a combo: Crashing or letting timer expire loses pending points
 *
 * MULTIPLIER CALCULATION:
 * The multiplier increases based on:
 * - Number of stunts in combo
 * - Variety of stunt types (UniqueStuntTypes)
 * - Quality of individual stunts
 *
 * EXAMPLE COMBO:
 * Stunt 1: Barrel Roll (100 pts) -> Multiplier 1.0x = 100 pts pending
 * Stunt 2: Big Air (200 pts) -> Multiplier 1.5x = 300 pts pending, total 400
 * Stunt 3: Flip (150 pts) -> Multiplier 2.0x = 300 pts pending, total 700
 * Bank Combo -> Player receives 700 points
 *
 * ComboEvents array stores all the individual stunts in the current combo
 * for display, replay, and final calculation.
 */
USTRUCT(BlueprintType)
struct FMGStuntCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboWindow = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGStuntEvent> ComboEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UniqueStuntTypes = 0;
};

/**
 * FMGTwoWheelState - Tracks two-wheel driving stunt state
 *
 * Two-wheel driving is a special ground-based stunt where the vehicle
 * tilts to one side and drives on only two wheels. While not technically
 * "airborne," it's managed by the stunt system due to its similar nature.
 *
 * HOW IT WORKS:
 * 1. Vehicle tilts past a threshold angle (e.g., 30+ degrees)
 * 2. System detects only two wheels touching ground
 * 3. TwoWheelState becomes Active
 * 4. Points accumulate based on duration and distance
 * 5. When vehicle returns to four wheels, stunt ends
 *
 * TRACKED DATA:
 * - bActive: Whether currently on two wheels
 * - Duration: How long in seconds (for UI display and scoring)
 * - Distance: How far traveled on two wheels (in Unreal units)
 * - bIsLeftSide: Left wheels or right wheels touching
 * - TiltAngle: Current tilt angle (larger = more impressive/points)
 * - AccumulatedPoints: Running point total for this attempt
 *
 * GAMEPLAY CONSIDERATIONS:
 * - Harder to control but rewards skillful players
 * - Can be combined with other stunts (e.g., two-wheel driving off a ramp)
 * - Risky: If you tilt too far, you might roll over
 */
USTRUCT(BlueprintType)
struct FMGTwoWheelState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLeftSide = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TiltAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AccumulatedPoints = 0;
};

/**
 * FMGStuntSessionStats - Statistics for a single play session
 *
 * This struct aggregates all stunt performance data for the current
 * gaming session. It's reset when a new session starts and can be
 * saved/displayed at session end.
 *
 * WHAT IS A SESSION?
 * A "session" typically means from when the player starts playing
 * to when they quit or finish a race/event. Sessions can be:
 * - A single race
 * - A free roam period
 * - A stunt challenge mode
 *
 * TRACKED STATISTICS:
 * - TotalStunts: Raw count of all stunts performed
 * - TotalPoints: Sum of all points earned from stunts
 * - BestCombo: Highest combo count achieved
 * - BestSingleStunt: Most points from a single stunt
 * - TotalAirTime: Cumulative seconds spent airborne
 * - LongestJump: Farthest horizontal distance in one jump
 * - HighestJump: Maximum height reached in one jump
 * - MostRotation: Most rotation in a single stunt (degrees)
 * - Counters for specific stunt types (barrel rolls, flips, etc.)
 * - Landing statistics (perfect vs crash)
 * - Breakdown by stunt type (StuntsByType map)
 *
 * USAGE:
 * - End-of-race summaries
 * - Achievement tracking
 * - Leaderboard submissions
 * - Personal record comparisons
 */
USTRUCT(BlueprintType)
struct FMGStuntSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalStunts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestSingleStunt = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalAirTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestJump = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestJump = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MostRotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBarrelRolls = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalFlips = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrashLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGStuntType, int32> StuntsByType;
};

/**
 * FMGStuntZone - Definition of a designated stunt area in the world
 *
 * Stunt Zones are special locations designed specifically for performing
 * stunts. They provide bonus multipliers and have target scores to beat.
 *
 * ZONE TYPES:
 * - Ramp areas: Multiple ramps clustered together
 * - Rooftop zones: Building tops accessible by ramps
 * - Canyon zones: Natural terrain features for jumps
 * - Signature zones: Unique, hand-crafted stunt opportunities
 *
 * HOW ZONES WORK:
 * 1. Player enters the zone radius
 * 2. Zone becomes "active" for that player
 * 3. Any stunts performed get the PointMultiplier applied
 * 4. UI may show zone name and target score
 * 5. If player beats target score, special rewards are given
 *
 * ZONE PROPERTIES:
 * - Location/Radius: Defines the circular area of the zone
 * - PreferredStunt: What type of stunt this zone is designed for
 * - PointMultiplier: Bonus multiplier for stunts in this zone
 * - TargetScore: Score to beat for completion/achievement
 * - BestScore: Player's personal best in this zone
 * - bIsSignature: True for unique, special zones
 *
 * GAMEPLAY USAGE:
 * - Discovery: Players can find new zones while exploring
 * - Competition: Beat friends' scores in specific zones
 * - Progression: Complete all zones for 100% completion
 */
USTRUCT(BlueprintType)
struct FMGStuntZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ZoneId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ZoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStuntType PreferredStunt = EMGStuntType::Jump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSignature = false;
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================
//
// Delegates are Unreal Engine's implementation of the Observer pattern.
// They allow this subsystem to "broadcast" events that other parts of the
// game can listen to and respond to.
//
// WHAT ARE DELEGATES?
// Think of delegates like a radio station:
// - The stunt system "broadcasts" when something happens
// - Other systems "tune in" to receive the message
// - Multiple systems can listen to the same broadcast
//
// HOW TO USE DELEGATES:
// In Blueprint: Use "Bind Event" nodes on the subsystem
// In C++: Call subsystem->OnStuntCompleted.AddDynamic(this, &MyClass::HandleStunt)
//
// DYNAMIC_MULTICAST means:
// - DYNAMIC: Can be used in Blueprints
// - MULTICAST: Multiple listeners can subscribe
// =============================================================================

// Fired when a vehicle launches into the air (stunt begins)
// Use this to: Start UI animations, play launch sound effects
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStuntStarted, EMGStuntType, StuntType, FVector, LaunchLocation);

// Fired when a stunt is successfully completed (landed)
// Use this to: Show score popup, update combo UI, trigger celebrations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStuntCompleted, const FMGStuntEvent&, Event, int32, TotalPoints);

// Fired when a stunt fails (crash, timeout, etc.)
// Use this to: Show failure message, play crash sound, reset combo display
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStuntFailed, const FString&, Reason);

// Fired when player completes a full rotation (360 degrees) while airborne
// Use this to: Show "Barrel Roll!" text, play rotation sound, update rotation counter
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRotationMilestone, EMGStuntType, RotationType, int32, Rotations, int32, Points);

// Fired when combo count or multiplier changes
// Use this to: Update combo counter UI, animate multiplier display
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboUpdated, int32, ComboCount, float, Multiplier);

// Fired when a combo is successfully banked (cashed in)
// Use this to: Show points awarded, clear combo UI, trigger reward effects
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboBanked, int32, FinalCombo, int32, TotalPoints);

// Fired when vehicle starts driving on two wheels
// Use this to: Start two-wheel UI, play tilt sound effect
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTwoWheelStarted, bool, bLeftSide, float, TiltAngle);

// Fired when two-wheel driving ends (back to four wheels)
// Use this to: Show points earned, stop two-wheel UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTwoWheelEnded, float, Duration, float, Distance, int32, Points);

// Fired when vehicle lands after being airborne
// Use this to: Show landing quality, apply landing effects, trigger camera shake
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLanding, EMGLandingState, State, int32, BonusPoints);

/**
 * UMGStuntSubsystem - The main stunt tracking and scoring system
 *
 * This is the core subsystem that manages all stunt-related functionality.
 * It's a GameInstanceSubsystem, meaning there's exactly one instance that
 * persists for the entire game session across all levels.
 *
 * RESPONSIBILITIES:
 * 1. DETECTION: Determine when the vehicle is airborne and what it's doing
 * 2. TRACKING: Monitor rotation, height, distance, and time while in air
 * 3. SCORING: Calculate points based on stunt performance
 * 4. COMBOS: Manage combo chains and multipliers
 * 5. ZONES: Handle stunt zone bonuses and tracking
 * 6. STATS: Accumulate session and career statistics
 * 7. EVENTS: Broadcast delegates for UI and other systems to respond
 *
 * HOW TO ACCESS THIS SUBSYSTEM:
 *
 * From C++:
 *   UMGStuntSubsystem* StuntSystem = GetGameInstance()->GetSubsystem<UMGStuntSubsystem>();
 *   if (StuntSystem) { StuntSystem->NotifyLaunch(...); }
 *
 * From Blueprint:
 *   Get Game Instance -> Get Subsystem (MGStuntSubsystem class) -> Call functions
 *
 * TYPICAL USAGE FLOW:
 * 1. Vehicle physics detects wheels leaving ground
 * 2. Vehicle calls NotifyLaunch() with launch position and velocity
 * 3. Every frame while airborne, vehicle calls UpdateAirState()
 * 4. When vehicle lands, it calls NotifyLanding()
 * 5. Subsystem calculates score, updates combos, broadcasts events
 * 6. UI receives events and displays results to player
 *
 * FUNCTION CATEGORIES:
 * - Air State Management: Track when vehicle is airborne
 * - Two-Wheel Driving: Handle two-wheel stunt state
 * - Stunt Detection: Identify what types of stunts were performed
 * - Point Calculation: Score stunts based on configuration
 * - Combo Management: Handle combo chains and banking
 * - Stunt Zones: Manage designated stunt areas
 * - Session Management: Track per-session statistics
 * - Utility: Helper functions for display names, colors, etc.
 * - Persistence: Save/load stunt data
 */
UCLASS()
class MIDNIGHTGRIND_API UMGStuntSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Initialize is called when the subsystem is created (game starts)
	// Deinitialize is called when it's destroyed (game ends)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnStuntStarted OnStuntStarted;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnStuntCompleted OnStuntCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnStuntFailed OnStuntFailed;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnRotationMilestone OnRotationMilestone;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnComboUpdated OnComboUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnComboBanked OnComboBanked;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnTwoWheelStarted OnTwoWheelStarted;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnTwoWheelEnded OnTwoWheelEnded;

	UPROPERTY(BlueprintAssignable, Category = "Stunt|Events")
	FOnLanding OnLanding;

	// =============================================================================
	// AIR STATE MANAGEMENT
	// =============================================================================
	// These functions track the vehicle's state while airborne.
	// The vehicle physics system should call these at appropriate times.
	// =============================================================================

	/**
	 * NotifyLaunch - Call this when the vehicle leaves the ground
	 *
	 * @param Position - World location where the vehicle left the ground
	 * @param Velocity - Current velocity vector at launch
	 * @param Rotation - Vehicle rotation at launch
	 * @param bWasDrifting - True if vehicle was drifting when it launched (for drift jump bonus)
	 *
	 * This starts tracking a new potential stunt. Call this the frame the vehicle
	 * is detected as airborne (all wheels off ground).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyLaunch(FVector Position, FVector Velocity, FRotator Rotation, bool bWasDrifting);

	/**
	 * UpdateAirState - Call every frame while the vehicle is airborne
	 *
	 * @param CurrentPosition - Current world position of the vehicle
	 * @param CurrentRotation - Current rotation of the vehicle
	 * @param DeltaTime - Time since last frame (for air time accumulation)
	 *
	 * This updates all tracking: air time, height, rotation accumulation, etc.
	 * Also checks for rotation milestones (e.g., completing a barrel roll).
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void UpdateAirState(FVector CurrentPosition, FRotator CurrentRotation, float DeltaTime);

	/**
	 * NotifyLanding - Call when the vehicle touches ground again
	 *
	 * @param Position - World location where the vehicle landed
	 * @param Velocity - Velocity at moment of landing
	 * @param Rotation - Vehicle rotation at landing
	 *
	 * This finalizes the stunt, calculates scores, updates combos,
	 * and broadcasts the OnStuntCompleted or OnStuntFailed event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyLanding(FVector Position, FVector Velocity, FRotator Rotation);

	/**
	 * NotifyNearMissWhileAirborne - Call when vehicle passes close to obstacle while in air
	 * This adds bonus points and records the near miss for the current stunt.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyNearMissWhileAirborne();

	/**
	 * NotifyOncomingWhileAirborne - Call when vehicle passes oncoming traffic while in air
	 * Similar to near miss but specifically for oncoming vehicles.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyOncomingWhileAirborne();

	/** Returns true if the vehicle is currently tracked as airborne */
	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	bool IsAirborne() const;

	/** Gets the current active air state data (read-only snapshot) */
	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	FMGActiveAirState GetActiveAirState() const;

	/** Gets current air time in seconds (for UI display) */
	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	float GetCurrentAirTime() const;

	/** Gets current height above launch point in Unreal units */
	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	float GetCurrentHeight() const;

	// =============================================================================
	// TWO-WHEEL DRIVING
	// =============================================================================
	// Functions for tracking when the vehicle drives on only two wheels.
	// This is a skill-based stunt that earns points over time.
	// =============================================================================

	/**
	 * StartTwoWheelDriving - Called when vehicle begins driving on two wheels
	 *
	 * @param bLeftSide - True if left wheels are on ground, false for right wheels
	 * @param TiltAngle - Current tilt angle in degrees
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|TwoWheel")
	void StartTwoWheelDriving(bool bLeftSide, float TiltAngle);

	/**
	 * UpdateTwoWheelDriving - Called every frame while on two wheels
	 *
	 * @param Distance - Distance traveled since last update
	 * @param TiltAngle - Current tilt angle (for dynamic scoring)
	 * @param DeltaTime - Time since last frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|TwoWheel")
	void UpdateTwoWheelDriving(float Distance, float TiltAngle, float DeltaTime);

	/**
	 * EndTwoWheelDriving - Called when vehicle returns to four wheels
	 * Finalizes the two-wheel stunt and awards points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|TwoWheel")
	void EndTwoWheelDriving();

	/** Returns true if currently driving on two wheels */
	UFUNCTION(BlueprintPure, Category = "Stunt|TwoWheel")
	bool IsTwoWheelDriving() const;

	/** Gets current two-wheel state data */
	UFUNCTION(BlueprintPure, Category = "Stunt|TwoWheel")
	FMGTwoWheelState GetTwoWheelState() const;

	// =============================================================================
	// STUNT DETECTION
	// =============================================================================
	// Functions that analyze the current air state to determine what stunts
	// were performed and how well they were executed.
	// =============================================================================

	/**
	 * DetectStuntsFromAirState - Analyzes current air state and returns all detected stunt types
	 *
	 * @return Array of all stunt types that were performed during this air time
	 *
	 * Example: A player does a flip while getting big air might return:
	 * [EMGStuntType::BigAir, EMGStuntType::Flip]
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt")
	TArray<EMGStuntType> DetectStuntsFromAirState() const;

	/**
	 * CalculateStuntQuality - Determines the quality rating for a completed stunt
	 *
	 * @param Event - The completed stunt event to evaluate
	 * @return Quality rating from Basic to Legendary
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt")
	EMGStuntQuality CalculateStuntQuality(const FMGStuntEvent& Event) const;

	/**
	 * CalculateLandingState - Determines how well the player landed
	 *
	 * @param LaunchRotation - Vehicle rotation when it left the ground
	 * @param LandingRotation - Vehicle rotation when it touched down
	 * @param LandingVelocity - Velocity at landing (for impact calculation)
	 * @return Landing quality from Perfect to Rollover
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt")
	EMGLandingState CalculateLandingState(FRotator LaunchRotation, FRotator LandingRotation, FVector LandingVelocity) const;

	// =============================================================================
	// POINT CALCULATION
	// =============================================================================
	// Functions that calculate points and rewards for stunts.
	// These use the FMGStuntPointConfig settings to determine values.
	// =============================================================================

	/**
	 * CalculateStuntPoints - Calculate total points for a stunt event
	 *
	 * @param Event - The stunt event to score
	 * @return Total points earned (base + bonuses)
	 *
	 * Uses the scoring formula defined in FMGStuntPointConfig.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Points")
	int32 CalculateStuntPoints(const FMGStuntEvent& Event) const;

	/**
	 * CalculateLandingBonus - Calculate bonus/penalty for landing quality
	 *
	 * @param Landing - How well the player landed
	 * @param BasePoints - Points before landing modifier
	 * @return Bonus points (positive) or penalty (negative)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Points")
	int32 CalculateLandingBonus(EMGLandingState Landing, int32 BasePoints) const;

	/**
	 * CalculateBoostReward - Calculate how much boost to award for a stunt
	 *
	 * @param Event - The completed stunt event
	 * @return Amount of boost to add to player's boost meter
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Points")
	float CalculateBoostReward(const FMGStuntEvent& Event) const;

	// =============================================================================
	// CONFIGURATION
	// =============================================================================
	// Functions to get and set the scoring and detection parameters.
	// Designers can use these to tune the stunt system without code changes.
	// =============================================================================

	/**
	 * SetStuntPointConfig - Set scoring configuration for a specific stunt type
	 * Use this to customize how many points each stunt type is worth.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Config")
	void SetStuntPointConfig(EMGStuntType StuntType, const FMGStuntPointConfig& Config);

	/**
	 * GetStuntPointConfig - Get current scoring configuration for a stunt type
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Config")
	FMGStuntPointConfig GetStuntPointConfig(EMGStuntType StuntType) const;

	/**
	 * SetThresholds - Set all detection thresholds at once
	 * Use this to adjust how stunts are detected and classified.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Config")
	void SetThresholds(const FMGStuntThresholds& Thresholds);

	/**
	 * GetThresholds - Get current detection thresholds
	 */
	UFUNCTION(BlueprintPure, Category = "Stunt|Config")
	FMGStuntThresholds GetThresholds() const;

	// =============================================================================
	// COMBO MANAGEMENT
	// =============================================================================
	// Functions to manage stunt combo chains. Combos multiply points but
	// must be "banked" before the timer runs out or you crash.
	// =============================================================================

	/**
	 * ExtendCombo - Add a stunt to the current combo chain
	 *
	 * @param Event - The stunt event to add to the combo
	 *
	 * This increases combo count, updates multiplier, and resets the combo timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Combo")
	void ExtendCombo(const FMGStuntEvent& Event);

	/**
	 * BankCombo - Cash in the current combo and award final points
	 *
	 * Call this when the player has safely completed their combo chain.
	 * Points are calculated with the full multiplier and added to score.
	 * Broadcasts OnComboBanked event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Combo")
	void BankCombo();

	/**
	 * LoseCombo - Forfeit the current combo (crash, timeout, etc.)
	 *
	 * Call this when the combo should be lost without awarding points.
	 * Resets combo state. Does NOT broadcast OnComboBanked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Combo")
	void LoseCombo();

	/**
	 * GetCurrentCombo - Get the current combo state for UI display
	 */
	UFUNCTION(BlueprintPure, Category = "Stunt|Combo")
	FMGStuntCombo GetCurrentCombo() const;

	/**
	 * IsComboActive - Check if there's an active combo chain
	 */
	UFUNCTION(BlueprintPure, Category = "Stunt|Combo")
	bool IsComboActive() const;

	// =============================================================================
	// STUNT ZONES
	// =============================================================================
	// Functions to manage designated stunt areas in the game world.
	// Stunt zones provide bonus multipliers and have target scores.
	// =============================================================================

	/**
	 * RegisterStuntZone - Add a new stunt zone to the system
	 * Called by level designers or during level load to set up zones.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	void RegisterStuntZone(const FMGStuntZone& Zone);

	/**
	 * GetStuntZone - Get zone data by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	FMGStuntZone GetStuntZone(const FString& ZoneId) const;

	/**
	 * GetNearestStuntZone - Find the closest stunt zone to a location
	 * Useful for UI indicators pointing to nearby stunt opportunities.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	FMGStuntZone GetNearestStuntZone(FVector Location) const;

	/**
	 * IsInStuntZone - Check if a location is inside any stunt zone
	 *
	 * @param Location - World position to check
	 * @param OutZoneId - Filled with zone ID if inside a zone
	 * @return True if inside a zone
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	bool IsInStuntZone(FVector Location, FString& OutZoneId) const;

	/**
	 * UpdateStuntZoneBestScore - Update the best score for a zone
	 * Called when player beats their previous best in a zone.
	 */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	void UpdateStuntZoneBestScore(const FString& ZoneId, int32 NewScore);

	// =============================================================================
	// SESSION MANAGEMENT
	// =============================================================================
	// Functions to manage stunt tracking sessions (e.g., a race or free roam)
	// =============================================================================

	/** StartSession - Begin a new stunt tracking session. Resets session stats. */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Session")
	void StartSession();

	/** EndSession - End the current session. Stats are finalized. */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Session")
	void EndSession();

	/** IsSessionActive - Check if a session is currently in progress */
	UFUNCTION(BlueprintPure, Category = "Stunt|Session")
	bool IsSessionActive() const;

	/** GetSessionStats - Get statistics for the current session */
	UFUNCTION(BlueprintPure, Category = "Stunt|Session")
	FMGStuntSessionStats GetSessionStats() const;

	// =============================================================================
	// STATISTICS
	// =============================================================================
	// Quick access to common statistics for UI and progression
	// =============================================================================

	/** GetTotalStuntPoints - Total points earned from stunts this session */
	UFUNCTION(BlueprintPure, Category = "Stunt|Stats")
	int32 GetTotalStuntPoints() const;

	/** GetTotalStunts - Total number of stunts performed this session */
	UFUNCTION(BlueprintPure, Category = "Stunt|Stats")
	int32 GetTotalStunts() const;

	/**
	 * GetRecentStunts - Get the most recent stunt events
	 * @param Count - How many recent stunts to return
	 * Useful for showing a stunt history in the UI.
	 */
	UFUNCTION(BlueprintPure, Category = "Stunt|Stats")
	TArray<FMGStuntEvent> GetRecentStunts(int32 Count) const;

	// =============================================================================
	// UTILITY FUNCTIONS
	// =============================================================================
	// Helper functions for UI display and formatting
	// =============================================================================

	/** GetStuntDisplayName - Get localized display name for a stunt type */
	UFUNCTION(BlueprintPure, Category = "Stunt|Utility")
	FText GetStuntDisplayName(EMGStuntType StuntType) const;

	/** GetQualityDisplayName - Get localized display name for a quality rating */
	UFUNCTION(BlueprintPure, Category = "Stunt|Utility")
	FText GetQualityDisplayName(EMGStuntQuality Quality) const;

	/** GetQualityColor - Get the UI color associated with a quality rating */
	UFUNCTION(BlueprintPure, Category = "Stunt|Utility")
	FLinearColor GetQualityColor(EMGStuntQuality Quality) const;

	// =============================================================================
	// PERSISTENCE (SAVE/LOAD)
	// =============================================================================
	// Functions to save and load stunt data (zone best scores, career stats, etc.)
	// =============================================================================

	/** SaveStuntData - Persist stunt data to disk */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Persistence")
	void SaveStuntData();

	/** LoadStuntData - Load previously saved stunt data */
	UFUNCTION(BlueprintCallable, Category = "Stunt|Persistence")
	void LoadStuntData();

protected:
	// =============================================================================
	// PROTECTED HELPER FUNCTIONS
	// =============================================================================
	// These are internal functions used by the public API.
	// They're protected so derived classes could override them if needed.
	// =============================================================================

	/**
	 * FinalizeStunt - Create a complete FMGStuntEvent from the current air state
	 * Called internally when NotifyLanding is received.
	 */
	FMGStuntEvent FinalizeStunt();

	/**
	 * CheckRotationMilestones - Check if any rotation thresholds were crossed
	 * Broadcasts OnRotationMilestone when a full rotation is completed.
	 */
	void CheckRotationMilestones();

	/**
	 * TickCombo - Update combo timer every frame
	 * Called by the combo tick timer. Loses combo if timer expires.
	 */
	void TickCombo(float DeltaTime);

	/**
	 * InitializeDefaultConfigs - Set up default scoring configs
	 * Called during Initialize to populate PointConfigs with defaults.
	 */
	void InitializeDefaultConfigs();

	/**
	 * CountFullRotations - Convert degrees to number of complete rotations
	 * @param Degrees - Total degrees rotated
	 * @return Number of complete 360-degree rotations
	 */
	int32 CountFullRotations(float Degrees) const;

private:
	// =============================================================================
	// PRIVATE MEMBER VARIABLES
	// =============================================================================
	// These store the subsystem's state. UPROPERTY() makes them visible to
	// Unreal's reflection system (needed for GC and serialization).
	// =============================================================================

	// Current state while vehicle is airborne
	UPROPERTY()
	FMGActiveAirState ActiveAirState;

	// Current two-wheel driving state
	UPROPERTY()
	FMGTwoWheelState TwoWheelState;

	// Current active combo chain
	UPROPERTY()
	FMGStuntCombo CurrentCombo;

	// Detection thresholds for classifying stunts
	UPROPERTY()
	FMGStuntThresholds StuntThresholds;

	// Statistics for the current session
	UPROPERTY()
	FMGStuntSessionStats SessionStats;

	// Scoring configurations for each stunt type
	UPROPERTY()
	TMap<EMGStuntType, FMGStuntPointConfig> PointConfigs;

	// All registered stunt zones (by ZoneId)
	UPROPERTY()
	TMap<FString, FMGStuntZone> StuntZones;

	// Circular buffer of recent stunts for history/replay
	UPROPERTY()
	TArray<FMGStuntEvent> RecentStunts;

	// Whether a stunt session is currently active
	UPROPERTY()
	bool bSessionActive = false;

	// Tracking for rotation milestone detection (to avoid duplicate broadcasts)
	UPROPERTY()
	int32 LastReportedRolls = 0;

	UPROPERTY()
	int32 LastReportedFlips = 0;

	UPROPERTY()
	int32 LastReportedSpins = 0;

	// Timer handle for combo countdown tick
	FTimerHandle ComboTickTimer;

	// Maximum number of recent stunts to keep in memory
	static constexpr int32 MaxRecentStunts = 50;
};
