// Copyright Midnight Grind. All Rights Reserved.

#pragma once

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
 * AI driver profile - defines unique characteristics
 */
USTRUCT(BlueprintType)
struct FMGAIDriverProfile
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

	/** Generate from difficulty preset */
	void GenerateFromDifficulty(EMGAIDifficulty InDifficulty);
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
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set driver profile */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverProfile(const FMGAIDriverProfile& Profile);

	/** Get driver profile */
	UFUNCTION(BlueprintPure, Category = "AI|Config")
	const FMGAIDriverProfile& GetDriverProfile() const { return DriverProfile; }

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
	void UpdateState(float DeltaTime);

	/** Update navigation */
	void UpdateNavigation(float DeltaTime);

	/** Update vehicle inputs */
	void UpdateVehicleInputs(float DeltaTime);

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
	FMGAIDriverProfile DriverProfile;

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
};
