// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGAIDecisionMaking.h
 * @brief Pure C++ AI decision-making logic for racing opponents
 * 
 * @namespace MidnightGrind::Racing::AI
 * This namespace contains pure calculation logic for AI racing decisions.
 * Extracted from AMGRacingAIController to enable:
 * - Unit testing without UObject overhead
 * - Reuse across different AI controller types
 * - Easier behavior tweaking and balancing
 * 
 * The AMGRacingAIController class calls these functions and handles UObject lifecycle.
 * 
 * Created: Phase 2 - Hybrid Namespacing Refactor
 */

#include "CoreMinimal.h"
#include "AI/MGRacingAIController.h" // For enums and config structs

/**
 * @namespace MidnightGrind::Racing::AI
 * @brief AI racing decision-making utilities
 * 
 * Pure C++ functions for AI steering, throttle, overtaking, and tactical decisions.
 * Called by AMGRacingAIController and other AI systems.
 */
namespace MidnightGrind::Racing::AI
{
	/**
	 * @struct FSteeringDecision
	 * @brief Result of steering calculation with metadata
	 * 
	 * Contains not just the steering angle but also contextual information
	 * about why the decision was made, useful for debugging and visualization.
	 */
	struct MIDNIGHTGRIND_API FSteeringDecision
	{
		/** Target steering angle in degrees (-45 to +45) */
		float TargetAngle = 0.0f;

		/** Urgency of the steering input (0 = casual, 1 = emergency) */
		float Urgency = 0.5f;

		/** Is this steering to avoid collision? */
		bool bAvoidingCollision = false;

		/** Is this part of an overtake maneuver? */
		bool bOvertaking = false;

		/** Distance to the target point (cm) */
		float DistanceToTarget = 0.0f;

		/** Recommended speed for this turn */
		float RecommendedSpeed = 0.0f;
	};

	/**
	 * @struct FOvertakeDecision
	 * @brief Result of overtake opportunity evaluation
	 */
	struct MIDNIGHTGRIND_API FOvertakeDecision
	{
		/** Should attempt overtake */
		bool bShouldOvertake = false;

		/** Preferred side for overtake (left = negative, right = positive) */
		float PreferredSide = 0.0f;

		/** Confidence in overtake success (0-1) */
		float Confidence = 0.0f;

		/** Reason for decision (for debugging) */
		FString Reason;
	};

	/**
	 * @struct FNOSDecision
	 * @brief Result of NOS usage evaluation
	 */
	struct MIDNIGHTGRIND_API FNOSDecision
	{
		/** Should activate NOS */
		bool bShouldActivate = false;

		/** Expected gain from NOS use (seconds saved) */
		float ExpectedGain = 0.0f;

		/** Risk assessment (0 = safe, 1 = risky) */
		float Risk = 0.0f;

		/** Reason for decision */
		FString Reason;
	};

	/**
	 * @class FSteeringCalculator
	 * @brief Static utility class for AI steering calculations
	 * 
	 * Calculates optimal steering angles to reach racing line targets,
	 * with collision avoidance and personality-based adjustments.
	 */
	class MIDNIGHTGRIND_API FSteeringCalculator
	{
	public:
		/**
		 * @brief Calculate optimal steering toward racing line target
		 * 
		 * Uses predictive steering model:
		 * 1. Calculate angle to target
		 * 2. Adjust for current speed (higher speed = gentler steering)
		 * 3. Apply personality modifiers (aggressive = sharper, defensive = smoother)
		 * 4. Add skill-based variation/wobble
		 * 
		 * @param CurrentPosition AI vehicle's current world position
		 * @param CurrentVelocity AI vehicle's current velocity vector
		 * @param TargetPosition Racing line target point
		 * @param CurrentSpeed Current speed in km/h
		 * @param Profile AI driver profile (skill, personality, etc.)
		 * @return Steering decision with angle and metadata
		 */
		static FSteeringDecision CalculateOptimalSteering(
			const FVector& CurrentPosition,
			const FVector& CurrentVelocity,
			const FVector& TargetPosition,
			float CurrentSpeed,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Calculate collision avoidance steering adjustment
		 * 
		 * Detects obstacles ahead and calculates steering correction to avoid them.
		 * Uses predictive path projection to determine if collision is imminent.
		 * 
		 * @param CurrentPosition AI vehicle's current position
		 * @param CurrentVelocity AI vehicle's current velocity
		 * @param ObstaclePosition Position of obstacle to avoid
		 * @param ObstacleVelocity Velocity of obstacle
		 * @param Profile AI driver profile
		 * @return Steering adjustment in degrees (negative = steer left, positive = steer right)
		 */
		static float CalculateCollisionAvoidance(
			const FVector& CurrentPosition,
			const FVector& CurrentVelocity,
			const FVector& ObstaclePosition,
			const FVector& ObstacleVelocity,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Smooth steering output to prevent jerky movements
		 * 
		 * Applies exponential smoothing to steering inputs based on AI skill.
		 * Lower skill = more smoothing needed.
		 * 
		 * @param CurrentSteering Current steering angle
		 * @param TargetSteering Desired steering angle
		 * @param DeltaTime Time since last update
		 * @param SmoothingSpeed How fast to reach target (higher = faster)
		 * @return Smoothed steering angle
		 */
		static float SmoothSteeringTransition(
			float CurrentSteering,
			float TargetSteering,
			float DeltaTime,
			float SmoothingSpeed);

	private:
		/** Calculate angle to target accounting for velocity */
		static float CalculateSteeringAngleToTarget(
			const FVector& CurrentPosition,
			const FVector& CurrentVelocity,
			const FVector& TargetPosition);

		/** Apply personality-based steering modifiers */
		static float ApplyPersonalityModifier(float BaseAngle, const FMGAIDriverConfig& Profile);

		/** Add skill-based steering variation (mistakes) */
		static float ApplySkillVariation(float BaseAngle, const FMGAIDriverConfig& Profile);
	};

	/**
	 * @class FThrottleCalculator
	 * @brief Static utility class for AI throttle/brake calculations
	 * 
	 * Determines optimal throttle and brake inputs to reach target speeds
	 * while respecting racing line recommendations.
	 */
	class MIDNIGHTGRIND_API FThrottleCalculator
	{
	public:
		/**
		 * @brief Calculate optimal throttle input to reach target speed
		 * 
		 * Considers:
		 * - Current speed vs target speed
		 * - Acceleration characteristics of vehicle
		 * - Distance to corner/braking zone
		 * - AI skill and aggression
		 * 
		 * @param CurrentSpeed Current speed in km/h
		 * @param TargetSpeed Desired speed in km/h
		 * @param DistanceToTarget Distance to target point in cm
		 * @param Profile AI driver profile
		 * @return Throttle output (0.0 to 1.0)
		 */
		static float CalculateThrottle(
			float CurrentSpeed,
			float TargetSpeed,
			float DistanceToTarget,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Calculate optimal brake input
		 * 
		 * Uses simple physics model:
		 * - Calculate required deceleration to reach target speed
		 * - Convert to brake pressure based on AI braking skill
		 * - Apply late braking for aggressive personalities
		 * 
		 * @param CurrentSpeed Current speed in km/h
		 * @param TargetSpeed Desired speed in km/h
		 * @param DistanceToCorner Distance to braking point in cm
		 * @param Profile AI driver profile
		 * @return Brake output (0.0 to 1.0)
		 */
		static float CalculateBrake(
			float CurrentSpeed,
			float TargetSpeed,
			float DistanceToCorner,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Calculate braking point distance for a corner
		 * 
		 * Determines how far before the corner the AI should start braking
		 * based on speed difference and AI skill.
		 * 
		 * Better drivers brake later (closer to corner).
		 * 
		 * @param CurrentSpeed Current speed in km/h
		 * @param CornerSpeed Target corner speed in km/h
		 * @param BrakingSkill AI braking skill (0-1)
		 * @return Distance to braking point in cm
		 */
		static float CalculateBrakingPoint(
			float CurrentSpeed,
			float CornerSpeed,
			float BrakingSkill);

	private:
		/** Maximum deceleration in m/s² */
		static constexpr float MAX_DECELERATION = 9.8f;

		/** Braking safety margin multiplier */
		static constexpr float BRAKING_SAFETY_MARGIN = 1.2f;
	};

	/**
	 * @class FOvertakeDecisionMaker
	 * @brief Static utility class for overtaking decisions
	 * 
	 * Evaluates overtake opportunities and selects optimal passing strategies.
	 */
	class MIDNIGHTGRIND_API FOvertakeDecisionMaker
	{
	public:
		/**
		 * @brief Evaluate if AI should attempt overtake on vehicle ahead
		 * 
		 * Considers:
		 * - Speed advantage
		 * - Distance to vehicle ahead
		 * - Available space (inside/outside line)
		 * - Upcoming corners (don't overtake before tight corners)
		 * - AI aggression and risk tolerance
		 * - Current position in race
		 * 
		 * @param VehicleAheadPosition Position of vehicle to overtake
		 * @param VehicleAheadVelocity Velocity of vehicle ahead
		 * @param AIPosition AI vehicle position
		 * @param AIVelocity AI vehicle velocity
		 * @param DistanceToCorner Distance to next corner (cm)
		 * @param Profile AI driver profile
		 * @return Overtake decision with confidence and strategy
		 */
		static FOvertakeDecision EvaluateOvertakeOpportunity(
			const FVector& VehicleAheadPosition,
			const FVector& VehicleAheadVelocity,
			const FVector& AIPosition,
			const FVector& AIVelocity,
			float DistanceToCorner,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Select optimal side for overtake (inside/outside line)
		 * 
		 * Inside line = shorter distance but requires later braking
		 * Outside line = longer but can carry more speed
		 * 
		 * @param CornerDirection Direction of upcoming corner (left/right)
		 * @param SpeedAdvantage How much faster AI is than target
		 * @param Profile AI driver profile
		 * @return Preferred side multiplier (-1 = inside, +1 = outside)
		 */
		static float SelectOvertakeSide(
			float CornerDirection,
			float SpeedAdvantage,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Calculate overtake confidence score
		 * 
		 * Higher confidence = more likely to commit to overtake.
		 * Based on speed advantage, available space, and AI personality.
		 * 
		 * @param SpeedAdvantage Speed difference (km/h)
		 * @param Distance Distance to vehicle ahead (cm)
		 * @param AvailableSpace Width of available passing space (cm)
		 * @param Profile AI driver profile
		 * @return Confidence score (0-1)
		 */
		static float CalculateOvertakeConfidence(
			float SpeedAdvantage,
			float Distance,
			float AvailableSpace,
			const FMGAIDriverConfig& Profile);

	private:
		/** Minimum speed advantage to consider overtaking (km/h) */
		static constexpr float MIN_SPEED_ADVANTAGE = 5.0f;

		/** Maximum distance to attempt overtake (cm) */
		static constexpr float MAX_OVERTAKE_DISTANCE = 5000.0f;
	};

	/**
	 * @class FNOSStrategyCalculator
	 * @brief Static utility class for NOS activation decisions
	 * 
	 * Determines when AI should use nitrous oxide boost for maximum advantage.
	 */
	class MIDNIGHTGRIND_API FNOSStrategyCalculator
	{
	public:
		/**
		 * @brief Evaluate if AI should activate NOS now
		 * 
		 * Optimal NOS usage scenarios:
		 * - Overtaking on straightaway
		 * - Defending position from close pursuer
		 * - Final sprint to finish line
		 * - Catching up to pack (if rubber-banding enabled)
		 * 
		 * Avoid NOS usage:
		 * - In corners (wasted on wheel spin)
		 * - When already at top speed
		 * - When low on NOS and far from finish
		 * 
		 * @param CurrentSpeed Current speed in km/h
		 * @param MaxSpeed Vehicle's top speed in km/h
		 * @param NOSRemaining Percentage of NOS remaining (0-1)
		 * @param DistanceToFinish Distance to race finish (cm)
		 * @param IsOvertaking Currently attempting overtake
		 * @param Profile AI driver profile
		 * @return NOS decision with reasoning
		 */
		static FNOSDecision EvaluateNOSActivation(
			float CurrentSpeed,
			float MaxSpeed,
			float NOSRemaining,
			float DistanceToFinish,
			bool IsOvertaking,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Calculate optimal NOS duration for current situation
		 * 
		 * Short burst vs long burn depends on:
		 * - Remaining NOS quantity
		 * - Distance to next corner
		 * - Current race position
		 * 
		 * @param NOSRemaining Percentage of NOS remaining
		 * @param DistanceToCorner Distance to next corner
		 * @param Profile AI driver profile
		 * @return Recommended NOS duration in seconds
		 */
		static float CalculateOptimalNOSDuration(
			float NOSRemaining,
			float DistanceToCorner,
			const FMGAIDriverConfig& Profile);

	private:
		/** Minimum NOS reserve to keep for finish sprint */
		static constexpr float MIN_NOS_RESERVE = 0.15f;

		/** Minimum speed percentage to consider NOS (avoid wasting on low speed) */
		static constexpr float MIN_SPEED_PERCENTAGE = 0.6f;
	};

	/**
	 * @class FDefensiveManeuverCalculator
	 * @brief Static utility class for defensive racing tactics
	 * 
	 * Calculates blocking moves and defensive lines to protect position.
	 */
	class MIDNIGHTGRIND_API FDefensiveManeuverCalculator
	{
	public:
		/**
		 * @brief Calculate defensive line adjustment to block overtake attempt
		 * 
		 * Moves to inside line on corner entry to block overtaking space.
		 * Legal defensive moves only (one move rule respected).
		 * 
		 * @param CurrentPosition AI vehicle position
		 * @param ThreatPosition Position of overtaking threat
		 * @param RacingLineCenter Center of racing line
		 * @param Profile AI driver profile
		 * @return Defensive line offset from racing line
		 */
		static float CalculateBlockingPosition(
			const FVector& CurrentPosition,
			const FVector& ThreatPosition,
			const FVector& RacingLineCenter,
			const FMGAIDriverConfig& Profile);

		/**
		 * @brief Determine if AI should defend position aggressively
		 * 
		 * More aggressive defense when:
		 * - High race position (protecting podium)
		 * - Close to finish line
		 * - AI has defensive personality
		 * 
		 * @param CurrentPosition Race position (1 = leading)
		 * @param DistanceToFinish Distance to finish in cm
		 * @param Profile AI driver profile
		 * @return True if should defend aggressively
		 */
		static bool ShouldDefendAggressively(
			int32 CurrentPosition,
			float DistanceToFinish,
			const FMGAIDriverConfig& Profile);
	};

	/**
	 * @class FMistakeSimulator
	 * @brief Static utility class for simulating AI mistakes based on skill
	 * 
	 * Adds realistic imperfection to AI driving.
	 */
	class MIDNIGHTGRIND_API FMistakeSimulator
	{
	public:
		/**
		 * @brief Determine if AI should make a mistake this frame
		 * 
		 * Mistake probability based on:
		 * - AI consistency rating (lower = more mistakes)
		 * - Pressure situations (overtaking, being pursued)
		 * - Corner difficulty
		 * - Fatigue (longer races = more mistakes)
		 * 
		 * @param Profile AI driver profile
		 * @param IsUnderPressure AI is in high-pressure situation
		 * @param CornerDifficulty Corner difficulty rating (0-1)
		 * @return True if AI should make a mistake
		 */
		static bool ShouldMakeMistake(
			const FMGAIDriverConfig& Profile,
			bool IsUnderPressure,
			float CornerDifficulty);

		/**
		 * @brief Generate random mistake type
		 * 
		 * Types:
		 * - Late braking (brake too late, overshoot corner)
		 * - Early braking (brake too early, lose momentum)
		 * - Wide turn (exit corner too wide)
		 * - Understeer (fail to turn in properly)
		 * - Wheel lock (brake too hard)
		 * 
		 * @return Mistake type enum
		 */
		static EMGAIMistakeType GenerateMistakeType();

		/**
		 * @brief Calculate severity of mistake
		 * 
		 * Lower skill = more severe mistakes.
		 * 
		 * @param Profile AI driver profile
		 * @return Severity multiplier (1.0 = normal, 2.0 = double effect)
		 */
		static float CalculateMistakeSeverity(const FMGAIDriverConfig& Profile);
	};

	/**
	 * @class FRubberBandingCalculator
	 * @brief Static utility class for rubber-banding (catch-up) calculations
	 * 
	 * Calculates performance adjustments to keep races competitive.
	 */
	class MIDNIGHTGRIND_API FRubberBandingCalculator
	{
	public:
		/**
		 * @brief Calculate catch-up boost multiplier based on race position
		 * 
		 * AI behind the pack get slight speed boost.
		 * AI far ahead get slight slowdown.
		 * Player is never artificially slowed (respects skill).
		 * 
		 * @param CurrentPosition AI's race position (1 = first)
		 * @param TotalRacers Total number of racers
		 * @param DistanceToLeader Distance behind race leader (cm, negative if ahead)
		 * @param Config Rubber-banding configuration
		 * @return Throttle adjustment multiplier (-0.2 to +0.3)
		 */
		static float CalculateCatchUpAdjustment(
			int32 CurrentPosition,
			int32 TotalRacers,
			float DistanceToLeader,
			const FMGRubberBandingConfig& Config);

		/**
		 * @brief Smooth rubber-banding transitions to avoid sudden changes
		 * 
		 * Gradually applies catch-up boost instead of instant changes.
		 * 
		 * @param CurrentAdjustment Current adjustment value
		 * @param TargetAdjustment Desired adjustment value
		 * @param DeltaTime Time since last update
		 * @return Smoothed adjustment value
		 */
		static float SmoothRubberBandingTransition(
			float CurrentAdjustment,
			float TargetAdjustment,
			float DeltaTime);

	private:
		/** Rubber-banding transition speed */
		static constexpr float TRANSITION_SPEED = 2.0f;
	};

} // namespace MidnightGrind::Racing::AI

/**
 * @brief AI mistake types for FMistakeSimulator
 */
UENUM(BlueprintType)
enum class EMGAIMistakeType : uint8
{
	LateBraking     UMETA(DisplayName = "Late Braking"),
	EarlyBraking    UMETA(DisplayName = "Early Braking"),
	WideTurn        UMETA(DisplayName = "Wide Turn"),
	Understeer      UMETA(DisplayName = "Understeer"),
	WheelLock       UMETA(DisplayName = "Wheel Lock"),
	Oversteer       UMETA(DisplayName = "Oversteer")
};
