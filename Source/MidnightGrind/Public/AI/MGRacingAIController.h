// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGRacingAIController.h
 * @brief Racing AI Controller - Core AI Opponent Brain for Midnight Grind
 *
 * @section overview Overview
 * This file defines the Racing AI Controller, which is the primary "brain" that
 * controls AI opponent vehicles during races. Each AI car in a race has its own
 * instance of this controller, which makes decisions about steering, throttle,
 * braking, and tactical maneuvers every frame.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection ai_controller What is an AI Controller?
 * In Unreal Engine, an AIController is what "possesses" (controls) a Pawn:
 * - PlayerController = Human player controls the pawn via input
 * - AIController = Computer controls the pawn via code/logic
 * When you see an AI car racing, this controller is making all the decisions.
 *
 * @subsection difficulty AI Difficulty System
 * Difficulty affects HOW WELL the AI drives, not physics advantages:
 * - Rookie: Slow reactions, makes mistakes, doesn't use full throttle
 * - Amateur: Below-average skill, occasional mistakes
 * - Professional: Average racer, balanced driving
 * - Expert: Skilled racer, aggressive overtakes
 * - Master: Near-perfect driving, minimal mistakes
 * - Legend: Perfect racing lines, maximum safe speeds
 *
 * @subsection personality AI Personalities
 * Each AI has a personality that affects their racing style:
 * - Balanced: Safe, consistent, predictable
 * - Aggressive: Takes risks, blocks, late braking
 * - Defensive: Protects position, avoids contact
 * - Showoff: Prioritizes style (drifts even when not optimal)
 * - Calculated: Optimal lines, efficient driving
 * - Wildcard: Unpredictable behavior
 *
 * @subsection state_machine AI State Machine
 * The AI switches between states based on the race situation:
 * - Waiting: Before race starts
 * - Racing: Normal driving behavior
 * - CatchingUp: Behind the pack, driving harder
 * - Defending: Protecting position from overtakers
 * - Overtaking: Attempting to pass another racer
 * - Recovering: Getting back on track after incident
 * - Finished: Race complete
 *
 * @subsection rubber_banding Rubber Banding (Catch-up Mechanics)
 * To keep races competitive, AI speed can be adjusted based on position:
 * - AI behind the player gets slight speed boost (catch-up)
 * - AI far ahead may slow down slightly
 * This is configurable and can be disabled for "pure" racing.
 *
 * @section usage Usage Examples
 *
 * @subsection basic_setup Basic Setup
 * @code
 * // Spawn an AI vehicle with a controller
 * AMGVehiclePawn* AIVehicle = GetWorld()->SpawnActor<AMGVehiclePawn>(VehicleClass, SpawnTransform);
 * AMGRacingAIController* AIController = GetWorld()->SpawnActor<AMGRacingAIController>();
 *
 * // Configure the AI
 * AIController->SetDifficulty(EMGAIDifficulty::Professional);
 * AIController->SetRacingLine(TrackRacingLineSpline);
 *
 * // Possess the vehicle
 * AIController->Possess(AIVehicle);
 * @endcode
 *
 * @subsection custom_profile Custom Driver Profile
 * @code
 * FMGAIDriverConfig Profile;
 * Profile.DriverName = FText::FromString("Max Velocity");
 * Profile.Difficulty = EMGAIDifficulty::Expert;
 * Profile.Personality = EMGAIPersonality::Aggressive;
 * Profile.SkillRating = 0.85f;
 * Profile.OvertakeAggression = 0.9f;
 * Profile.NOSAggression = 0.7f;
 *
 * AIController->SetDriverProfile(Profile);
 * @endcode
 *
 * @subsection race_control Race Control
 * @code
 * // Set race position info for rubber-banding
 * AIController->SetRacePosition(Position, TotalRacers);
 * AIController->SetDistanceToLeader(DistanceInCm);
 *
 * // Start racing
 * AIController->StartRacing();
 *
 * // Query AI state
 * EMGAIState CurrentState = AIController->GetAIState();
 * float Throttle = AIController->GetThrottleOutput();
 * bool WantsNOS = AIController->ShouldUseNOS();
 * @endcode
 *
 * @subsection events Listening to Events
 * @code
 * // Subscribe to state changes
 * AIController->OnAIStateChanged.AddDynamic(this, &UMyClass::HandleStateChanged);
 *
 * void UMyClass::HandleStateChanged(EMGAIState NewState)
 * {
 *     if (NewState == EMGAIState::Overtaking)
 *     {
 *         // Play overtake attempt sound
 *     }
 * }
 * @endcode
 *
 * @section architecture Architecture
 * @verbatim
 *   [AMGRacingAIController] - This class
 *          |
 *          +---> [FMGAIDriverConfig] - Personality & skill configuration
 *          |
 *          +---> [Racing Line Spline] - Path to follow
 *          |
 *          +---> [FMGAISteeringTarget] - Current navigation target
 *          |
 *          v
 *   [AMGVehiclePawn] - The controlled vehicle
 *          |
 *          +---> Receives throttle, brake, steering inputs
 *          +---> Receives NOS activation requests
 * @endverbatim
 *
 * @section override Blueprint Extension
 * This class is BlueprintType and Blueprintable, allowing:
 * - Creating Blueprint subclasses with custom behavior
 * - Overriding CalculateSteering, CalculateThrottle, etc. in Blueprints
 * - Adding visual debugging in editor
 *
 * @see FMGAIDriverConfig - Driver configuration struct
 * @see FMGRubberBandingConfig - Catch-up mechanics configuration
 * @see EMGAIDifficulty - Difficulty presets
 * @see EMGAIPersonality - Personality types
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "AIController.h"
#include "MGRacingAIController.generated.h"

class AMGVehiclePawn;
class AMGCheckpoint;
class USplineComponent;

/**
 * AI difficulty preset
 */
UENUM(BlueprintType)
enum class EMGAIDifficulty : uint8
{
	/** Beginner - slow, makes mistakes */
	Rookie,
	/** Easy - below average */
	Amateur,
	/** Normal - average racer */
	Professional,
	/** Hard - skilled racer */
	Expert,
	/** Very Hard - near-perfect */
	Master,
	/** Impossible - perfect lines, max speed */
	Legend
};

/**
 * AI personality type
 */
UENUM(BlueprintType)
enum class EMGAIPersonality : uint8
{
	/** Balanced driving */
	Balanced,
	/** Aggressive - takes risks, blocks */
	Aggressive,
	/** Defensive - safe lines, avoids contact */
	Defensive,
	/** Drifter - prioritizes style */
	Showoff,
	/** Calculated - optimal racing line */
	Calculated,
	/** Unpredictable - varies behavior */
	Wildcard
};

/**
 * AI state
 */
UENUM(BlueprintType)
enum class EMGAIState : uint8
{
	/** Waiting for race start */
	Waiting,
	/** Normal racing */
	Racing,
	/** Catching up to pack */
	CatchingUp,
	/** Defending position */
	Defending,
	/** Attempting overtake */
	Overtaking,
	/** Recovering from incident */
	Recovering,
	/** Finished race */
	Finished
};

/**
 * AI driver config - defines unique characteristics for this controller
 * NOTE: Renamed from FMGAIDriverProfile to avoid conflict with UMGAIDriverProfile in MG_AI_DriverProfile.h
 */
USTRUCT(BlueprintType)
struct FMGAIDriverConfig
{
	GENERATED_BODY()

	/** Driver name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FText DriverName;

	/** Difficulty level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	EMGAIDifficulty Difficulty = EMGAIDifficulty::Professional;

	/** Personality type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	EMGAIPersonality Personality = EMGAIPersonality::Balanced;

	/** Skill rating (0-1) affects all driving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SkillRating = 0.7f;

	/** Cornering ability (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CorneringSkill = 0.7f;

	/** Braking ability (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrakingSkill = 0.7f;

	/** Overtaking aggression (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OvertakeAggression = 0.5f;

	/** Defensive ability (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefensiveSkill = 0.5f;

	/** Consistency (0-1) - higher = fewer mistakes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Consistency = 0.8f;

	/** Risk tolerance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiskTolerance = 0.5f;

	/** Reaction time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ReactionTime = 0.3f;

	/** Top speed limiter (0-1, 1 = full speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float TopSpeedFactor = 0.95f;

	/** Preferred racing line offset (-1 to 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float PreferredLineOffset = 0.0f;

	/** Catchup enabled (rubber banding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bUseCatchup = true;

	/** NOS usage strategy (0 = conservative, 1 = aggressive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NOSAggression = 0.5f;

	// ========================================
	// WEATHER AWARENESS
	// ========================================

	/**
	 * @brief Weather adaptation skill (0-1)
	 * Higher values mean AI adjusts better to weather conditions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeatherAdaptation = 0.7f;

	/**
	 * @brief Night vision capability (0-1)
	 * Affects how well AI performs in low-light conditions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NightDrivingSkill = 0.7f;

	/**
	 * @brief Wet weather driving skill (0-1)
	 * Affects performance in rain and on wet surfaces
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WetWeatherSkill = 0.7f;

	/**
	 * @brief Perception range multiplier applied by weather system
	 * Set by UMGWeatherRacingSubsystem based on conditions
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Weather")
	float WeatherPerceptionMultiplier = 1.0f;

	/** Generate from difficulty preset */
	void GenerateFromDifficulty(EMGAIDifficulty InDifficulty);
};

/**
 * Rubber-banding configuration
 * Controls how AI adjusts performance based on race position
 */
USTRUCT(BlueprintType)
struct FMGRubberBandingConfig
{
	GENERATED_BODY()

	/** Enable catch-up boost for AI behind the leader */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up")
	bool bEnableCatchUp = true;

	/** Enable slow-down for AI far ahead */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up")
	bool bEnableSlowDown = true;

	/** Maximum throttle boost when catching up (0-0.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float MaxCatchUpBoost = 0.15f;

	/** Maximum throttle reduction when far ahead (0-0.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up", meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float MaxSlowDownPenalty = 0.1f;

	/** Distance threshold (cm) to start applying catch-up (e.g., 5000 = 50m behind) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up", meta = (ClampMin = "1000.0"))
	float CatchUpDistanceThreshold = 5000.0f;

	/** Distance threshold (cm) to start applying slow-down (e.g., 10000 = 100m ahead) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up", meta = (ClampMin = "1000.0"))
	float SlowDownDistanceThreshold = 10000.0f;

	/** Maximum distance for full catch-up boost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up", meta = (ClampMin = "5000.0"))
	float MaxCatchUpDistance = 30000.0f;

	/** Scale catch-up by difficulty (0 = same for all, 1 = harder difficulty = less help) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Catch-up", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DifficultyScaling = 0.5f;
};

/**
 * Steering target info
 */
USTRUCT(BlueprintType)
struct FMGAISteeringTarget
{
	GENERATED_BODY()

	/** World location to steer toward */
	UPROPERTY(BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;

	/** Desired speed at this point */
	UPROPERTY(BlueprintReadOnly)
	float TargetSpeed = 0.0f;

	/** Distance to target */
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;

	/** Is this a braking zone */
	UPROPERTY(BlueprintReadOnly)
	bool bBrakingZone = false;

	/** Suggested gear */
	UPROPERTY(BlueprintReadOnly)
	int32 SuggestedGear = 0;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIStateChanged, EMGAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIOvertakeAttempt, AActor*, TargetVehicle, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAIMistake);

/**
 * Racing AI Controller
 * Controls AI opponent vehicles with configurable behavior
 *
 * Features:
 * - Difficulty presets from Rookie to Legend
 * - Personality-based behavior variation
 * - Racing line following with spline support
 * - Overtaking and defensive maneuvers
 * - Rubber banding (catchup) system
 * - Mistake simulation based on skill
 * - NOS usage strategy
 * - Collision avoidance
 * - Blueprint-extensible
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API AMGRacingAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMGRacingAIController();

	virtual void BeginPlay() override;
	virtual void Tick(float MGDeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set driver profile */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverProfile(const FMGAIDriverConfig& Profile);

	/** Get driver profile */
	UFUNCTION(BlueprintPure, Category = "AI|Config")
	const FMGAIDriverConfig& GetDriverProfile() const { return DriverProfile; }

	/** Set difficulty (generates profile) */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDifficulty(EMGAIDifficulty Difficulty);

	/** Set racing line spline */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRacingLine(USplineComponent* Spline);

	/** Enable/disable AI control */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetAIEnabled(bool bEnabled);

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/** Start racing */
	UFUNCTION(BlueprintCallable, Category = "AI|Race")
	void StartRacing();

	/** Stop racing */
	UFUNCTION(BlueprintCallable, Category = "AI|Race")
	void StopRacing();

	/** Set current checkpoint target */
	UFUNCTION(BlueprintCallable, Category = "AI|Race")
	void SetTargetCheckpoint(AMGCheckpoint* Checkpoint);

	/** Set race position info */
	UFUNCTION(BlueprintCallable, Category = "AI|Race")
	void SetRacePosition(int32 Position, int32 TotalRacers);

	/** Set distance to leader for rubber-banding calculations */
	UFUNCTION(BlueprintCallable, Category = "AI|Race")
	void SetDistanceToLeader(float DistanceCm);

	/** Set rubber-banding configuration */
	UFUNCTION(BlueprintCallable, Category = "AI|Race")
	void SetRubberBandingConfig(const FMGRubberBandingConfig& Config);

	/** Get current rubber-banding adjustment (-1 to 1, negative = slowing, positive = boost) */
	UFUNCTION(BlueprintPure, Category = "AI|Race")
	float GetRubberBandingAdjustment() const { return CurrentRubberBandingAdjustment; }

	/** Get current state */
	UFUNCTION(BlueprintPure, Category = "AI|Race")
	EMGAIState GetAIState() const { return CurrentState; }

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get current steering target */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	FMGAISteeringTarget GetCurrentSteeringTarget() const { return CurrentTarget; }

	/** Get throttle output (0-1) */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	float GetThrottleOutput() const { return ThrottleOutput; }

	/** Get brake output (0-1) */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	float GetBrakeOutput() const { return BrakeOutput; }

	/** Get steering output (-1 to 1) */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	float GetSteeringOutput() const { return SteeringOutput; }

	/** Should use NOS */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	bool ShouldUseNOS() const { return bWantsNOS; }

	/** Get vehicle being controlled */
	UFUNCTION(BlueprintPure, Category = "AI|Query")
	AMGVehiclePawn* GetControlledVehicle() const { return ControlledVehicle; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** AI state changed */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIStateChanged OnAIStateChanged;

	/** AI attempted overtake */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIOvertakeAttempt OnOvertakeAttempt;

	/** AI made a mistake */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIMistake OnMistake;

protected:
	// ==========================================
	// BEHAVIOR METHODS (Override in Blueprint)
	// ==========================================

	/** Calculate steering toward target - override for custom behavior */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Behavior")
	float CalculateSteering(const FVector& TargetLocation);

	/** Calculate throttle - override for custom behavior */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Behavior")
	float CalculateThrottle(float TargetSpeed, float CurrentSpeed);

	/** Calculate brake - override for custom behavior */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Behavior")
	float CalculateBrake(float TargetSpeed, float CurrentSpeed, float DistanceToCorner);

	/** Decide if should attempt overtake */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Behavior")
	bool ShouldAttemptOvertake(AActor* VehicleAhead, float Distance);

	/** Decide if should use NOS */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Behavior")
	bool ShouldActivateNOS();

	/** Called when making a mistake */
	UFUNCTION(BlueprintNativeEvent, Category = "AI|Behavior")
	void OnMakeMistake();

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update state machine */
	void UpdateState(float MGDeltaTime);

	/** Update navigation */
	void UpdateNavigation(float MGDeltaTime);

	/** Update vehicle inputs */
	void UpdateVehicleInputs(float MGDeltaTime);

	/** Apply inputs to vehicle */
	void ApplyInputsToVehicle();

	/** Find racing line target */
	FVector GetRacingLineTarget(float LookaheadDistance) const;

	/** Check for vehicles ahead */
	AActor* DetectVehicleAhead(float& OutDistance) const;

	/** Calculate catchup boost */
	float CalculateCatchupBoost() const;

	/** Should make random mistake */
	bool ShouldMakeMistake() const;

	/** Set new state */
	void SetState(EMGAIState NewState);

	/** Get speed for current section */
	float GetTargetSpeedForSection() const;

private:
	/** Driver profile */
	UPROPERTY(EditAnywhere, Category = "AI")
	FMGAIDriverConfig DriverProfile;

	/** Racing line spline */
	UPROPERTY()
	TWeakObjectPtr<USplineComponent> RacingLineSpline;

	/** Target checkpoint */
	UPROPERTY()
	TWeakObjectPtr<AMGCheckpoint> TargetCheckpoint;

	/** Controlled vehicle */
	UPROPERTY()
	TObjectPtr<AMGVehiclePawn> ControlledVehicle;

	/** Current state */
	EMGAIState CurrentState = EMGAIState::Waiting;

	/** Current steering target */
	FMGAISteeringTarget CurrentTarget;

	/** Output values */
	float ThrottleOutput = 0.0f;
	float BrakeOutput = 0.0f;
	float SteeringOutput = 0.0f;
	bool bWantsNOS = false;

	/** Race info */
	int32 CurrentPosition = 0;
	int32 TotalRacers = 0;

	/** Progress along racing line (0-1) */
	float RacingLineProgress = 0.0f;

	/** Is AI enabled */
	bool bAIEnabled = true;

	/** Time in current state */
	float StateTime = 0.0f;

	/** Mistake cooldown */
	float MistakeCooldown = 0.0f;

	/** Overtake attempt cooldown */
	float OvertakeCooldown = 0.0f;

	/** Lookahead distance for racing line */
	UPROPERTY(EditAnywhere, Category = "AI|Navigation")
	float LookaheadDistance = 2000.0f;

	/** Detection range for vehicles ahead */
	UPROPERTY(EditAnywhere, Category = "AI|Navigation")
	float VehicleDetectionRange = 5000.0f;

	/** Steering smoothing */
	UPROPERTY(EditAnywhere, Category = "AI|Control")
	float SteeringSmoothSpeed = 5.0f;

	/** Previous steering for smoothing */
	float PreviousSteering = 0.0f;

	// ==========================================
	// RUBBER-BANDING
	// ==========================================

	/** Rubber-banding configuration */
	UPROPERTY(EditAnywhere, Category = "AI|RubberBanding")
	FMGRubberBandingConfig RubberBandingConfig;

	/** Current distance to leader (cm, negative = ahead of leader) */
	float DistanceToLeader = 0.0f;

	/** Current rubber-banding adjustment being applied */
	float CurrentRubberBandingAdjustment = 0.0f;
};
