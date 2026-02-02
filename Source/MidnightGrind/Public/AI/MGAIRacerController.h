// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGAIRacerController.h
 * @brief Advanced AI Racer Controller with Tactical Decision-Making
 *
 * @section overview Overview
 * This file defines the Advanced AI Racer Controller, an enhanced AI system that
 * provides sophisticated racing behavior with personality-driven tactics, skill-based
 * catch-up mechanics (NOT rubber-banding), and fair competition that follows the
 * same physics rules as the player.
 *
 * This controller is designed around GDD Pillar 5: "Unified Challenge" - AI opponents
 * use the exact same physics as players. They compete through smarter decisions,
 * not physics advantages.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection driving_states Driving States
 * The AI operates in different behavioral states based on the race situation:
 * - Waiting: At start line, waiting for green light
 * - Racing: Normal driving, following optimal racing line
 * - Overtaking: Actively attempting to pass another vehicle
 * - Defending: Protecting position without dirty driving
 * - Recovering: Getting back on track after collision/off-track
 * - Caution: Slowing for hazard (traffic, incident)
 * - PushingHard: Skill-based catch-up (taking more risks)
 * - ManagingLead: Conservative driving when ahead
 * - Drafting: Using slipstream aerodynamic advantage
 * - Finished: Race complete
 *
 * @subsection skill_vs_rubber Skill-Based Catch-Up vs Rubber Banding
 * Traditional "rubber banding" gives AI speed boosts - this feels unfair!
 * Skill-based catch-up means AI DRIVES DIFFERENTLY based on position:
 * - When behind: Takes more risks (later braking, tighter lines)
 * - When ahead: Drives conservatively (manages lead, avoids risks)
 * The AI never gets physics advantages - just smarter/riskier decisions.
 *
 * @subsection perception AI Perception
 * The AI "sees" nearby vehicles and tracks:
 * - Distance and angle to each vehicle
 * - Whether they're ahead or behind on track
 * - Speed differences for closing rates
 * - Slipstream opportunities
 * - Time to collision calculations
 *
 * @subsection racing_line Racing Line Following
 * Each point on the racing line contains:
 * - World position and direction
 * - Target speed (based on corner tightness)
 * - Track width for avoidance calculations
 * - Zone flags (apex, braking, acceleration)
 * - Curvature for steering calculations
 * - Grip level (affected by surface/weather)
 *
 * @subsection pid_steering PID Steering
 * The AI uses PID (Proportional-Integral-Derivative) control for smooth steering:
 * - Proportional: How far off the line? Steer proportionally.
 * - Integral: Been off-line for a while? Increase correction.
 * - Derivative: Rapidly getting worse? Apply quick correction.
 * This creates smooth, human-like steering instead of jerky robot movements.
 *
 * @subsection tactics Tactical Decision Making
 * The AI makes strategic choices about:
 * - WHEN to overtake (waiting for good opportunity vs. forcing)
 * - HOW to overtake (late braking, better exit, around outside)
 * - WHEN to defend (covering racing line, protecting inside)
 * - Risk level (based on personality, race position, lap count)
 *
 * @section usage Usage Examples
 *
 * @subsection controller_setup Controller Setup
 * @code
 * // Get or spawn the controller
 * AMGAIRacerController* AIController = Cast<AMGAIRacerController>(Vehicle->GetController());
 *
 * // Configure driver profile
 * AIController->SetDriverProfile(DriverProfileAsset);
 *
 * // Set difficulty (affects decision quality, not physics)
 * AIController->SetDifficultyMultiplier(1.0f);  // Normal
 *
 * // Enable skill-based catch-up (NOT rubber banding!)
 * AIController->SetSkillBasedCatchUpEnabled(true);
 *
 * // Provide the racing line
 * AIController->SetRacingLine(RacingLinePoints);
 * @endcode
 *
 * @subsection querying_state Querying AI State
 * @code
 * // What is the AI doing?
 * EMGAIDrivingState State = AIController->GetDrivingState();
 *
 * // Get steering outputs
 * FMGAISteeringOutput Output = AIController->GetSteeringOutput();
 * float Steering = Output.Steering;    // -1 to 1
 * float Throttle = Output.Throttle;    // 0 to 1
 * float Brake = Output.Brake;          // 0 to 1
 * bool WantsNOS = Output.bNOS;
 *
 * // Get nearby vehicles the AI sees
 * TArray<FMGAIVehiclePerception> NearbyVehicles = AIController->GetPerceivedVehicles();
 *
 * // Get tactical information
 * FMGAITacticalData Tactics = AIController->GetTacticalData();
 * bool InSlipstream = Tactics.bInSlipstream;
 * EMGOvertakeStrategy Strategy = Tactics.OvertakeStrategy;
 * @endcode
 *
 * @subsection race_updates Race Position Updates
 * @code
 * // Update race position for tactical decisions
 * AIController->UpdateRacePosition(
 *     CurrentPosition,    // 1 = first place
 *     TotalRacers,        // Total racers in race
 *     GapToLeader,        // Seconds behind leader
 *     GapToAhead          // Seconds behind vehicle ahead
 * );
 * @endcode
 *
 * @subsection events AI Events
 * @code
 * // Subscribe to state changes
 * AIController->OnDrivingStateChanged.AddDynamic(this, &UMyClass::OnAIStateChanged);
 *
 * // Subscribe to successful overtakes
 * AIController->OnOvertakeComplete.AddDynamic(this, &UMyClass::OnAIOvertook);
 *
 * void UMyClass::OnAIOvertook(AActor* OvertakenVehicle, EMGOvertakeStrategy Strategy)
 * {
 *     // Play overtake celebration, update positions, etc.
 * }
 * @endcode
 *
 * @section architecture Architecture
 * @verbatim
 *   [AMGAIRacerController] - This class
 *          |
 *          +---> [UMGAIDriverProfile] - Personality & skill data asset
 *          |
 *          +---> [FMGAIRacingLinePoint[]] - Path to follow
 *          |
 *          +---> [FMGAIVehiclePerception[]] - Nearby vehicle awareness
 *          |
 *          +---> [FMGAITacticalData] - Current tactical decisions
 *          |
 *          +---> [State Machine] - Racing/Overtaking/Defending/etc.
 *          |
 *          v
 *   [Vehicle Pawn] - Receives steering outputs
 * @endverbatim
 *
 * @section design Design Philosophy
 * Per GDD Pillar 5 (Unified Challenge):
 * - AI uses the SAME physics as players
 * - No speed boosts or grip advantages
 * - Difficulty through decision quality, not cheats
 * - Fair competition that rewards player skill
 *
 * @see UMGAIDriverProfile - Driver personality data asset
 * @see FMGAIRacingLinePoint - Racing line data structure
 * @see FMGAISteeringOutput - Steering/throttle/brake outputs
 * @see FMGAITacticalData - Tactical decision information
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "AIController.h"
#include "MGAIRacerController.generated.h"

class UMGAIDriverProfile;
class UMGTrackSubsystem;
class AMGRaceGameMode;

/**
 * @brief AI driving state enumeration
 *
 * Defines the current behavioral state of the AI racer. States determine
 * how the AI calculates steering, throttle, and tactical decisions.
 *
 * Design Note: Per GDD Pillar 5 (Unified Challenge), AI uses the same physics
 * as players. States affect decision-making, not physics advantages.
 */
UENUM(BlueprintType)
enum class EMGAIDrivingState : uint8
{
	/** Waiting at start line for race signal */
	Waiting,
	/** Normal racing - following optimal racing line */
	Racing,
	/** Attempting to overtake - looking for passing opportunity */
	Overtaking,
	/** Defending position - blocking without dirty driving */
	Defending,
	/** Recovering from collision/off-track incident */
	Recovering,
	/** Slowing for hazard ahead (traffic, incident) */
	Caution,
	/** Skill-based catch-up - pushing harder within physics limits */
	PushingHard,
	/** Managing lead - maintaining pace without unnecessary risk */
	ManagingLead,
	/** Slipstream drafting - using aerodynamic advantage */
	Drafting,
	/** Finished race */
	Finished
};

/**
 * @brief Overtake strategy types
 *
 * Defines how the AI approaches passing opportunities based on
 * driver personality and track conditions.
 */
UENUM(BlueprintType)
enum class EMGOvertakeStrategy : uint8
{
	/** Wait for clear opportunity on straight */
	Patient,
	/** Dive inside at braking zone */
	LateBraking,
	/** Use superior corner exit speed to pass on straight */
	BetterExit,
	/** Take outside line through corner */
	AroundOutside,
	/** Use slipstream on straight */
	SlipstreamPass,
	/** Force error through sustained pressure */
	Pressure
};

/**
 * @brief Defense strategy types
 *
 * Defines how the AI defends position without resorting to dirty tactics.
 */
UENUM(BlueprintType)
enum class EMGDefenseStrategy : uint8
{
	/** Cover the racing line, force opponent to go around */
	CoverLine,
	/** Late defensive move to inside line */
	CoverInside,
	/** Use superior corner speed to maintain gap */
	PaceDefense,
	/** Take defensive line through entire corner sequence */
	DefensiveLine
};

/**
 * @brief Skill-based catch-up mode
 *
 * Instead of rubber-banding (which violates the Unified Challenge pillar),
 * AI uses skill-based behaviors to catch up or manage their position.
 */
UENUM(BlueprintType)
enum class EMGAICatchUpBehavior : uint8
{
	/** No catch-up active - racing at natural pace */
	None,
	/** Taking more risks in corners (later braking, earlier throttle) */
	RiskTaking,
	/** Using slipstream more aggressively */
	DraftingFocus,
	/** Pushing harder on straights (closer to rev limiter) */
	MaxEffort,
	/** Managing tires/engine by not overdriving */
	Conservation
};

/**
 * @brief AI perception data for nearby vehicles
 *
 * Contains spatial and temporal data about a perceived vehicle,
 * used for tactical decision-making.
 */
USTRUCT(BlueprintType)
struct FMGAIVehiclePerception
{
	GENERATED_BODY()

	/** The perceived vehicle actor */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	AActor* Vehicle = nullptr;

	/** Relative position in local space */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	FVector RelativePosition = FVector::ZeroVector;

	/** Relative velocity */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	FVector RelativeVelocity = FVector::ZeroVector;

	/** Distance to vehicle center */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	float Distance = 0.0f;

	/** Angle to vehicle (-180 to 180 degrees) */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	float Angle = 0.0f;

	/** Is the vehicle ahead on track */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	bool bIsAhead = false;

	/** Is the vehicle on our left side */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	bool bIsOnLeft = false;

	/** Time to collision if approaching (FLT_MAX if not converging) */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	float TimeToCollision = FLT_MAX;

	/** Is this the player's vehicle */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	bool bIsPlayer = false;

	/** Speed difference (positive = we're faster) */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	float SpeedDifference = 0.0f;

	/** Track position difference (positive = they're ahead in race) */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	float TrackPositionDelta = 0.0f;

	/** Is in slipstream range */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	bool bInSlipstreamRange = false;

	/** Estimated skill level based on observed driving */
	UPROPERTY(BlueprintReadOnly, Category = "Perception")
	float EstimatedSkill = 0.5f;
};

/**
 * @brief Racing line point for AI navigation
 *
 * Represents a single point on the optimal racing line with
 * associated speed and zone data.
 */
USTRUCT(BlueprintType)
struct FMGAIRacingLinePoint
{
	GENERATED_BODY()

	/** World position of this racing line point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	FVector Position = FVector::ZeroVector;

	/** Optimal direction at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	FVector Direction = FVector::ForwardVector;

	/** Target speed at this point (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float TargetSpeed = 100.0f;

	/** Track width at this point (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float TrackWidth = 10.0f;

	/** Distance along track from start (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float DistanceAlongTrack = 0.0f;

	/** Is this a corner apex point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	bool bIsApex = false;

	/** Is this in a braking zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	bool bIsBrakingZone = false;

	/** Is this in an acceleration zone (post-apex) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	bool bIsAccelerationZone = false;

	/** Curvature at this point (1/radius, higher = tighter) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float Curvature = 0.0f;

	/** Grip level (0-1, affected by surface type) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float GripLevel = 1.0f;

	/** Optimal gear at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	int32 OptimalGear = 3;

	/** Is this a good overtaking zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	bool bIsOvertakingZone = false;

	/** Camber angle (affects grip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing Line")
	float CamberAngle = 0.0f;
};

/**
 * @brief AI steering calculation result
 *
 * Contains all vehicle control outputs calculated by the AI
 * steering system for a single frame.
 */
USTRUCT(BlueprintType)
struct FMGAISteeringOutput
{
	GENERATED_BODY()

	/** Steering input (-1 to 1, left to right) */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	float Steering = 0.0f;

	/** Throttle input (0 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	float Throttle = 0.0f;

	/** Brake input (0 to 1) */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	float Brake = 0.0f;

	/** Handbrake engaged */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	bool bHandbrake = false;

	/** NOS activation requested */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	bool bNOS = false;

	/** Target point being steered toward */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	FVector TargetPoint = FVector::ZeroVector;

	/** Clutch input (0 to 1, for smooth shifting) */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	float Clutch = 0.0f;

	/** Desired gear (0 = neutral, -1 = reverse) */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	int32 DesiredGear = 1;

	/** Confidence in current inputs (0-1, lower = more uncertain) */
	UPROPERTY(BlueprintReadOnly, Category = "Steering")
	float Confidence = 1.0f;
};

/**
 * @brief Tactical decision data
 *
 * Contains information about the AI's current tactical situation
 * and planned maneuvers.
 */
USTRUCT(BlueprintType)
struct FMGAITacticalData
{
	GENERATED_BODY()

	/** Current overtake strategy being executed */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	EMGOvertakeStrategy OvertakeStrategy = EMGOvertakeStrategy::Patient;

	/** Current defense strategy being executed */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	EMGDefenseStrategy DefenseStrategy = EMGDefenseStrategy::CoverLine;

	/** Current catch-up mode */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	EMGAICatchUpBehavior CatchUpMode = EMGAICatchUpBehavior::None;

	/** Target vehicle for overtake/defense */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	AActor* TacticalTarget = nullptr;

	/** Distance to next overtaking zone */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	float DistanceToOvertakeZone = 0.0f;

	/** Time spent following current target */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	float TimeFollowing = 0.0f;

	/** Current risk level being taken (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	float CurrentRiskLevel = 0.5f;

	/** Accumulated tire wear simulation */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	float SimulatedTireWear = 0.0f;

	/** Is currently in slipstream of another vehicle */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	bool bInSlipstream = false;

	/** Slipstream speed bonus currently being applied */
	UPROPERTY(BlueprintReadOnly, Category = "Tactical")
	float SlipstreamBonus = 0.0f;
};

/** Delegate for AI state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIDrivingStateChanged, EMGAIDrivingState, OldState, EMGAIDrivingState, NewState);

/** Delegate for successful overtake */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIOvertakeComplete, AActor*, OvertakenVehicle, EMGOvertakeStrategy, Strategy);

/** Delegate for being overtaken */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIWasOvertaken, AActor*, OvertakingVehicle);

/**
 * @brief AI Racer Controller
 *
 * Controls AI opponent vehicles during races with personality-driven
 * behavior, tactical decision-making, and skill-based catch-up mechanics.
 *
 * Design Philosophy (per GDD):
 * - Uses same physics as players (Pillar 5: Unified Challenge)
 * - No rubber-banding that violates physics
 * - Skill-based difficulty through decision quality
 * - Personality affects tactics, not physics cheats
 *
 * Key Features:
 * - Racing line following with PID steering
 * - Dynamic overtaking based on personality and opportunity
 * - Fair defensive driving (no dirty blocking)
 * - Skill-based catch-up (risk-taking, not speed boosts)
 * - Collision avoidance
 * - Slipstream awareness and utilization
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API AMGAIRacerController : public AAIController
{
	GENERATED_BODY()

public:
	AMGAIRacerController();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float MGDeltaTime) override;
	//~ End AActor Interface

	//~ Begin AController Interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	//~ End AController Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * @brief Set the driver profile for this AI
	 * @param Profile The driver profile data asset
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverProfile(UMGAIDriverProfile* Profile);

	/**
	 * @brief Get the current driver profile
	 * @return The active driver profile, or nullptr if none set
	 */
	UFUNCTION(BlueprintPure, Category = "AI|Config")
	UMGAIDriverProfile* GetDriverProfile() const { return DriverProfile; }

	/**
	 * @brief Set difficulty multiplier
	 *
	 * Affects AI skill parameters without violating physics.
	 * Lower values make AI make more mistakes, higher values
	 * make AI drive more optimally.
	 *
	 * @param Multiplier Difficulty scale (0.5 = easy, 1.0 = normal, 1.5 = hard)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDifficultyMultiplier(float Multiplier);

	/**
	 * @brief Enable or disable skill-based catch-up system
	 *
	 * When enabled, AI will take calculated risks to catch up
	 * or drive conservatively when leading. This does NOT
	 * provide any physics advantages.
	 *
	 * @param bEnabled Whether to enable the system
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetSkillBasedCatchUpEnabled(bool bEnabled);

	/**
	 * @brief Set the racing line points for this AI to follow
	 * @param RacingLine Array of racing line points
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine);

	/**
	 * @brief Set how aggressively AI will pursue overtakes
	 * @param Aggression 0-1 scale, affects risk tolerance
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetOvertakeAggression(float Aggression);

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * @brief Get the current driving state
	 * @return Current AI driving state
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	EMGAIDrivingState GetDrivingState() const { return CurrentState; }

	/**
	 * @brief Get the current steering output values
	 * @return Steering output struct
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	FMGAISteeringOutput GetSteeringOutput() const { return CurrentSteering; }

	/**
	 * @brief Get the vehicle's current speed in cm/s
	 * @return Current speed
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetCurrentSpeed() const;

	/**
	 * @brief Get the current target speed
	 * @return Target speed the AI is aiming for
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetTargetSpeed() const { return CurrentTargetSpeed; }

	/**
	 * @brief Get distance to the optimal racing line
	 * @return Lateral distance from racing line
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetDistanceToRacingLine() const;

	/**
	 * @brief Check if currently attempting an overtake
	 * @return True if in overtaking state
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	bool IsOvertaking() const { return CurrentState == EMGAIDrivingState::Overtaking; }

	/**
	 * @brief Get all currently perceived vehicles
	 * @return Array of vehicle perception data
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	TArray<FMGAIVehiclePerception> GetPerceivedVehicles() const { return PerceivedVehicles; }

	/**
	 * @brief Get current tactical data
	 * @return Tactical decision information
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	FMGAITacticalData GetTacticalData() const { return TacticalData; }

	/**
	 * @brief Get current progress along racing line (0-1)
	 * @return Normalized progress
	 */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetRacingLineProgress() const { return RacingLineProgress; }

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/**
	 * @brief Start racing when the race begins
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StartRacing();

	/**
	 * @brief Stop racing (e.g., when finished)
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StopRacing();

	/**
	 * @brief Force a specific driving state
	 * @param NewState The state to force
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void ForceState(EMGAIDrivingState NewState);

	/**
	 * @brief Notify the AI of a collision
	 * @param OtherActor The actor collided with
	 * @param ImpactPoint World location of impact
	 * @param ImpactNormal Normal vector of impact
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void NotifyCollision(AActor* OtherActor, const FVector& ImpactPoint, const FVector& ImpactNormal);

	/**
	 * @brief Notify the AI that it went off track
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void NotifyOffTrack();

	/**
	 * @brief Update race position information for tactical decisions
	 * @param Position Current race position (1 = first)
	 * @param TotalRacers Total number of racers
	 * @param GapToLeader Time gap to leader in seconds
	 * @param GapToAhead Time gap to vehicle ahead
	 */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void UpdateRacePosition(int32 Position, int32 TotalRacers, float GapToLeader, float GapToAhead);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when driving state changes */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIDrivingStateChanged OnDrivingStateChanged;

	/** Fired when AI successfully completes an overtake */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIOvertakeComplete OnOvertakeComplete;

	/** Fired when AI is overtaken by another vehicle */
	UPROPERTY(BlueprintAssignable, Category = "AI|Events")
	FOnAIWasOvertaken OnWasOvertaken;

protected:
	// ==========================================
	// CONFIGURATION PROPERTIES
	// ==========================================

	/** Driver profile asset containing personality and skill data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UMGAIDriverProfile> DriverProfile;

	/** Difficulty multiplier affecting skill parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float DifficultyMultiplier = 1.0f;

	/** Enable skill-based catch-up (NOT rubber banding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bSkillBasedCatchUpEnabled = true;

	/** Look ahead distance for steering calculations (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Steering", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float SteeringLookAhead = 15.0f;

	/** Look ahead distance for speed/braking calculations (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Steering", meta = (ClampMin = "10.0", ClampMax = "100.0"))
	float SpeedLookAhead = 30.0f;

	/** Perception radius for detecting other vehicles (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Perception", meta = (ClampMin = "20.0", ClampMax = "200.0"))
	float PerceptionRadius = 50.0f;

	/** Minimum time gap to maintain when following (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Racing", meta = (ClampMin = "0.3", ClampMax = "3.0"))
	float MinFollowingGap = 1.0f;

	/** Threshold for deciding to attempt an overtake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Racing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OvertakeThreshold = 0.7f;

	/** Maximum time to wait before abandoning an overtake attempt (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Racing", meta = (ClampMin = "2.0", ClampMax = "15.0"))
	float MaxOvertakeTime = 8.0f;

	/** PID steering proportional gain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PID", meta = (ClampMin = "0.5", ClampMax = "5.0"))
	float SteeringPGain = 2.0f;

	/** PID steering integral gain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PID", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SteeringIGain = 0.1f;

	/** PID steering derivative gain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|PID", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SteeringDGain = 0.5f;

	/** Slipstream detection angle (degrees from directly behind) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Slipstream", meta = (ClampMin = "5.0", ClampMax = "30.0"))
	float SlipstreamAngle = 15.0f;

	/** Maximum slipstream range (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Slipstream", meta = (ClampMin = "5.0", ClampMax = "50.0"))
	float SlipstreamRange = 25.0f;

	// ==========================================
	// RACING LINE DATA
	// ==========================================

	/** Cached racing line points */
	UPROPERTY()
	TArray<FMGAIRacingLinePoint> RacingLinePoints;

	/** Current index on racing line */
	int32 CurrentRacingLineIndex = 0;

	/** Progress along racing line (0-1 for one lap) */
	float RacingLineProgress = 0.0f;

	/** Total racing line length */
	float TotalRacingLineLength = 0.0f;

	// ==========================================
	// STATE DATA
	// ==========================================

	/** Current driving state */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EMGAIDrivingState CurrentState = EMGAIDrivingState::Waiting;

	/** Current steering output */
	FMGAISteeringOutput CurrentSteering;

	/** Current target speed (cm/s) */
	float CurrentTargetSpeed = 0.0f;

	/** Perceived vehicles array */
	TArray<FMGAIVehiclePerception> PerceivedVehicles;

	/** Tactical decision data */
	FMGAITacticalData TacticalData;

	/** Time spent in current state */
	float TimeInState = 0.0f;

	/** Recovery timer countdown */
	float RecoveryTimer = 0.0f;

	/** Overtake attempt timer */
	float OvertakeTimer = 0.0f;

	/** Side being used for current overtake (true = left) */
	bool bOvertakeOnLeft = false;

	/** Last steering error for PID derivative */
	float LastSteeringError = 0.0f;

	/** Accumulated steering error for PID integral */
	float SteeringErrorIntegral = 0.0f;

	/** Current race position */
	int32 CurrentRacePosition = 0;

	/** Last known race position (for mood/learning delta tracking) */
	int32 LastKnownPosition = 0;

	/** Last known damage percentage (0-1, for mood delta tracking) */
	float LastKnownDamage = 0.0f;

	/** Total racers in current race */
	int32 TotalRacersInRace = 0;

	/** Gap to race leader in seconds */
	float GapToLeader = 0.0f;

	/** Gap to vehicle immediately ahead */
	float GapToVehicleAhead = 0.0f;

	// ==========================================
	// REFERENCES
	// ==========================================

	/** Track subsystem reference */
	UPROPERTY()
	TObjectPtr<UMGTrackSubsystem> TrackSubsystem;

	/** Race game mode reference */
	UPROPERTY()
	TObjectPtr<AMGRaceGameMode> RaceGameMode;

	/** Controlled vehicle pawn */
	UPROPERTY()
	TObjectPtr<APawn> VehiclePawn;

	// ==========================================
	// CORE UPDATE METHODS
	// ==========================================

	/** Update perception of nearby vehicles */
	virtual void UpdatePerception();

	/** Update the state machine */
	virtual void UpdateStateMachine(float MGDeltaTime);

	/** Calculate steering for current state */
	virtual void CalculateSteering(float MGDeltaTime);

	/** Apply calculated steering to vehicle */
	virtual void ApplySteering();

	/** Update racing line progress tracking */
	virtual void UpdateRacingLineProgress();

	/** Update tactical decision-making */
	virtual void UpdateTactics(float MGDeltaTime);

	/** Update mood and learning systems (adaptive AI behavior) */
	virtual void UpdateMoodAndLearning(float MGDeltaTime);

	// ==========================================
	// STATE HANDLERS
	// ==========================================

	/** Handle waiting state logic */
	virtual void HandleWaitingState(float MGDeltaTime);

	/** Handle normal racing state */
	virtual void HandleRacingState(float MGDeltaTime);

	/** Handle overtaking state */
	virtual void HandleOvertakingState(float MGDeltaTime);

	/** Handle position defense state */
	virtual void HandleDefendingState(float MGDeltaTime);

	/** Handle collision/off-track recovery */
	virtual void HandleRecoveringState(float MGDeltaTime);

	/** Handle pushing hard (skill-based catch-up) */
	virtual void HandlePushingHardState(float MGDeltaTime);

	/** Handle managing lead (conservative driving when ahead) */
	virtual void HandleManagingLeadState(float MGDeltaTime);

	/** Handle drafting state */
	virtual void HandleDraftingState(float MGDeltaTime);

	// ==========================================
	// STEERING CALCULATIONS
	// ==========================================

	/** Calculate steering to follow the racing line */
	virtual FMGAISteeringOutput CalculateRacingLineSteering();

	/** Calculate steering for an overtake maneuver */
	virtual FMGAISteeringOutput CalculateOvertakeSteering();

	/** Calculate steering for defensive driving */
	virtual FMGAISteeringOutput CalculateDefenseSteering();

	/** Calculate steering during recovery */
	virtual FMGAISteeringOutput CalculateRecoverySteering();

	/** Calculate steering for drafting behind another vehicle */
	virtual FMGAISteeringOutput CalculateDraftingSteering();

	/** Calculate avoidance steering offset */
	virtual FVector CalculateAvoidanceOffset();

	/** Calculate target speed for current position */
	virtual float CalculateTargetSpeed();

	/** Calculate skill-based speed adjustment (NOT rubber banding) */
	virtual float CalculateSkillBasedAdjustment();

	// ==========================================
	// TACTICAL DECISIONS
	// ==========================================

	/** Evaluate whether to attempt an overtake */
	virtual bool ShouldAttemptOvertake() const;

	/** Evaluate whether to defend position */
	virtual bool ShouldDefendPosition() const;

	/** Choose best overtake strategy for current situation */
	virtual EMGOvertakeStrategy ChooseOvertakeStrategy(const FMGAIVehiclePerception& Target) const;

	/** Choose best defense strategy for current situation */
	virtual EMGDefenseStrategy ChooseDefenseStrategy(const FMGAIVehiclePerception& Attacker) const;

	/** Determine appropriate catch-up mode based on race position */
	virtual EMGAICatchUpBehavior DetermineCatchUpMode() const;

	/** Check if we should start drafting */
	virtual bool ShouldStartDrafting() const;

	// ==========================================
	// UTILITY METHODS
	// ==========================================

	/** Get a point on the racing line at specified distance ahead */
	FMGAIRacingLinePoint GetRacingLinePointAhead(float Distance) const;

	/** Find the closest racing line point to a world position */
	int32 FindClosestRacingLinePoint(const FVector& Position) const;

	/** Get the player's vehicle */
	APawn* GetPlayerVehicle() const;

	/** Get perception data for vehicle directly ahead */
	FMGAIVehiclePerception GetVehicleAhead() const;

	/** Get perception data for vehicle directly behind */
	FMGAIVehiclePerception GetVehicleBehind() const;

	/** Check if a path is clear for overtaking */
	bool IsOvertakePathClear(bool bOnLeft) const;

	/** Apply driver profile modifiers to steering output */
	void ApplyProfileModifiers(FMGAISteeringOutput& Output);

	/** Set new driving state with event broadcast */
	void SetState(EMGAIDrivingState NewState);

	/** Add personality-based steering noise */
	float AddSteeringNoise(float BaseValue);

	/** Calculate slipstream bonus for given position */
	float CalculateSlipstreamBonus(const FVector& Position, const FVector& Velocity) const;

	/** Check if position is in slipstream of another vehicle */
	bool IsInSlipstream(AActor* LeadVehicle) const;

	/** Get appropriate risk level based on personality and situation */
	float GetSituationalRiskLevel() const;

	/** Calculate braking distance for given speed delta */
	float CalculateBrakingDistance(float CurrentSpeed, float TargetSpeed) const;

	// ==========================================
	// AGGRESSION RESPONSE SYSTEM
	// ==========================================

	/**
	 * Handle contact response based on AI personality
	 * @param Response The determined response type
	 * @param Offender The actor that caused contact
	 * @param Severity Contact severity (0-1)
	 */
	void HandleContactResponse(EMGContactResponse Response, AActor* Offender, float Severity);

	/**
	 * Apply aggression-based behavior modifiers to steering
	 * @param Output Steering output to modify
	 */
	void ApplyAggressionModifiers(FMGAISteeringOutput& Output);

	/**
	 * Check if we should attempt a dirty move against target
	 * @param Target The target vehicle
	 * @return True if dirty tactics are appropriate
	 */
	bool ShouldAttemptDirtyMove(AActor* Target) const;

	/**
	 * Calculate personality-based brake point adjustment
	 * @return Adjustment factor (negative = earlier, positive = later)
	 */
	float GetPersonalityBrakeAdjustment() const;
};
