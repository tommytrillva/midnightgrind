// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MGRacingAIController.generated.h"

class AMGVehiclePawn;
class AMGTrackSpline;
class AMGCheckpoint;

/**
 * AI difficulty levels
 */
UENUM(BlueprintType)
enum class EMGAIDifficulty : uint8
{
	Rookie		UMETA(DisplayName = "Rookie"),
	Amateur		UMETA(DisplayName = "Amateur"),
	Pro			UMETA(DisplayName = "Pro"),
	Expert		UMETA(DisplayName = "Expert"),
	Legend		UMETA(DisplayName = "Legend")
};

/**
 * AI driving personality/style
 */
UENUM(BlueprintType)
enum class EMGAIPersonality : uint8
{
	Cautious	UMETA(DisplayName = "Cautious"),		// Brakes early, avoids contact
	Balanced	UMETA(DisplayName = "Balanced"),		// Standard racing behavior
	Aggressive	UMETA(DisplayName = "Aggressive"),		// Late braking, contact OK
	Dirty		UMETA(DisplayName = "Dirty"),			// Will block and ram
	Drifter		UMETA(DisplayName = "Drifter")			// Prefers sliding through corners
};

/**
 * AI driver profile - configures behavior characteristics
 */
USTRUCT(BlueprintType)
struct FMGAIDriverProfile
{
	GENERATED_BODY()

	/** Display name for this AI driver */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FText DriverName;

	/** Difficulty level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	EMGAIDifficulty Difficulty = EMGAIDifficulty::Amateur;

	/** Driving personality */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	EMGAIPersonality Personality = EMGAIPersonality::Balanced;

	/** Skill rating 0-100 (affects racing line accuracy) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0", ClampMax = "100"))
	float SkillRating = 50.0f;

	/** Reaction time in seconds (lower = better) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float ReactionTime = 0.2f;

	/** How much the AI wanders from ideal line (0 = perfect, 1 = sloppy) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0", ClampMax = "1"))
	float LineVariation = 0.3f;

	/** Braking skill - how close to optimal braking points (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0", ClampMax = "1"))
	float BrakingSkill = 0.7f;

	/** Throttle control skill (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0", ClampMax = "1"))
	float ThrottleControl = 0.7f;

	/** How aggressively AI defends position (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0", ClampMax = "1"))
	float DefensiveAggression = 0.5f;

	/** How aggressively AI attacks for overtakes (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0", ClampMax = "1"))
	float OvertakeAggression = 0.5f;

	/** Willingness to use nitrous (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0", ClampMax = "1"))
	float NitrousUsage = 0.5f;

	/** Target speed multiplier (0.8-1.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.8", ClampMax = "1.2"))
	float SpeedMultiplier = 1.0f;

	/** Create default profile for difficulty */
	static FMGAIDriverProfile CreateForDifficulty(EMGAIDifficulty Difficulty);
};

/**
 * Current AI driving state
 */
USTRUCT(BlueprintType)
struct FMGAIDrivingState
{
	GENERATED_BODY()

	/** Current distance along track spline */
	UPROPERTY(BlueprintReadOnly)
	float CurrentTrackDistance = 0.0f;

	/** Target distance we're driving toward */
	UPROPERTY(BlueprintReadOnly)
	float TargetTrackDistance = 0.0f;

	/** Current target world position */
	UPROPERTY(BlueprintReadOnly)
	FVector TargetPosition = FVector::ZeroVector;

	/** Target speed for current segment (cm/s) */
	UPROPERTY(BlueprintReadOnly)
	float TargetSpeed = 0.0f;

	/** Current throttle output (-1 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float ThrottleOutput = 0.0f;

	/** Current steering output (-1 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float SteeringOutput = 0.0f;

	/** Current brake output (0 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float BrakeOutput = 0.0f;

	/** Is handbrake engaged? */
	UPROPERTY(BlueprintReadOnly)
	bool bHandbrake = false;

	/** Is nitrous active? */
	UPROPERTY(BlueprintReadOnly)
	bool bNitrous = false;

	/** Current race position */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPosition = 0;

	/** Distance to vehicle ahead (cm, -1 if leading) */
	UPROPERTY(BlueprintReadOnly)
	float DistanceToVehicleAhead = -1.0f;

	/** Distance to vehicle behind (cm, -1 if last) */
	UPROPERTY(BlueprintReadOnly)
	float DistanceToVehicleBehind = -1.0f;

	/** Is currently attempting overtake? */
	UPROPERTY(BlueprintReadOnly)
	bool bAttemptingOvertake = false;

	/** Is currently defending position? */
	UPROPERTY(BlueprintReadOnly)
	bool bDefendingPosition = false;
};

/**
 * Rubber-banding configuration
 */
USTRUCT(BlueprintType)
struct FMGRubberBandingConfig
{
	GENERATED_BODY()

	/** Enable rubber-banding catch-up mechanics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	bool bEnabled = true;

	/** Distance behind leader to start catching up (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	float CatchUpStartDistance = 5000.0f; // 50 meters

	/** Maximum catch-up speed boost (multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	float MaxCatchUpBoost = 1.15f; // 15% faster

	/** Distance ahead to start slowing down (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	float SlowDownStartDistance = 5000.0f;

	/** Minimum slow-down factor (multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	float MinSlowDownFactor = 0.92f; // 8% slower

	/** How aggressively rubber-banding scales with distance (1 = linear) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	float ScalingExponent = 1.5f;

	/** Only apply to AI (not player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rubber Banding")
	bool bAIOnly = true;
};

/**
 * AI Controller for racing vehicles
 * Handles racing line following, overtaking, and race strategy
 */
UCLASS()
class MIDNIGHTGRIND_API AMGRacingAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMGRacingAIController();

	// AAIController interface
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Set the track spline for navigation */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void SetTrackSpline(AMGTrackSpline* InTrackSpline);

	/** Set the AI driver profile */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void SetDriverProfile(const FMGAIDriverProfile& InProfile);

	/** Set rubber-banding configuration */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void SetRubberBandingConfig(const FMGRubberBandingConfig& InConfig);

	/** Initialize for race start */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void InitializeForRace();

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/** Start racing (enable AI control) */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void StartRacing();

	/** Stop racing (disable AI control) */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void StopRacing();

	/** Pause/resume AI */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void SetPaused(bool bPaused);

	/** Update race position info */
	UFUNCTION(BlueprintCallable, Category = "Racing AI")
	void UpdateRacePosition(int32 Position, float DistanceToAhead, float DistanceToBehind);

	// ==========================================
	// STATE & DATA
	// ==========================================

	/** Get current driving state */
	UFUNCTION(BlueprintPure, Category = "Racing AI")
	const FMGAIDrivingState& GetDrivingState() const { return DrivingState; }

	/** Get driver profile */
	UFUNCTION(BlueprintPure, Category = "Racing AI")
	const FMGAIDriverProfile& GetDriverProfile() const { return DriverProfile; }

	/** Get the controlled vehicle */
	UFUNCTION(BlueprintPure, Category = "Racing AI")
	AMGVehiclePawn* GetVehicle() const { return ControlledVehicle; }

	/** Is AI currently racing? */
	UFUNCTION(BlueprintPure, Category = "Racing AI")
	bool IsRacing() const { return bIsRacing && !bIsPaused; }

protected:
	// ==========================================
	// DRIVING LOGIC
	// ==========================================

	/** Main driving update */
	virtual void UpdateDriving(float DeltaTime);

	/** Calculate target point on track */
	virtual void UpdateTargetPoint();

	/** Calculate steering toward target */
	virtual float CalculateSteering();

	/** Calculate throttle/brake based on speed and upcoming track */
	virtual void CalculateSpeedControl(float& OutThrottle, float& OutBrake);

	/** Decide whether to use nitrous */
	virtual bool ShouldUseNitrous();

	/** Decide whether to use handbrake for drift */
	virtual bool ShouldUseDriftHandbrake();

	/** Apply rubber-banding speed adjustment */
	virtual float ApplyRubberBanding(float BaseSpeed);

	/** Check for and handle overtaking situations */
	virtual void UpdateOvertakeLogic();

	/** Check for and handle defensive driving */
	virtual void UpdateDefenseLogic();

	/** Add variation/mistakes based on skill */
	virtual void ApplySkillVariation(float DeltaTime);

	/** Get lookahead distance based on speed */
	float GetLookaheadDistance() const;

	/** Get optimal speed for track curvature */
	float GetOptimalSpeedForCurvature(float Curvature) const;

	// ==========================================
	// COMPONENTS & REFERENCES
	// ==========================================

	/** The vehicle being controlled */
	UPROPERTY()
	TObjectPtr<AMGVehiclePawn> ControlledVehicle;

	/** Track spline for navigation */
	UPROPERTY()
	TObjectPtr<AMGTrackSpline> TrackSpline;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** AI driver profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing AI")
	FMGAIDriverProfile DriverProfile;

	/** Rubber-banding configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing AI")
	FMGRubberBandingConfig RubberBandingConfig;

	/** How far ahead to look on track (base value, scales with speed) */
	UPROPERTY(EditAnywhere, Category = "Racing AI|Tuning")
	float BaseLookaheadDistance = 1500.0f; // 15 meters

	/** Speed factor for lookahead (cm/s to distance ratio) */
	UPROPERTY(EditAnywhere, Category = "Racing AI|Tuning")
	float LookaheadSpeedFactor = 0.5f;

	/** Maximum lookahead distance */
	UPROPERTY(EditAnywhere, Category = "Racing AI|Tuning")
	float MaxLookaheadDistance = 8000.0f; // 80 meters

	/** Steering sensitivity */
	UPROPERTY(EditAnywhere, Category = "Racing AI|Tuning")
	float SteeringSensitivity = 2.0f;

	/** Maximum speed (cm/s) on straights */
	UPROPERTY(EditAnywhere, Category = "Racing AI|Tuning")
	float MaxStraightSpeed = 8000.0f; // ~180 mph

	/** Minimum speed (cm/s) in tight corners */
	UPROPERTY(EditAnywhere, Category = "Racing AI|Tuning")
	float MinCornerSpeed = 1500.0f; // ~35 mph

	// ==========================================
	// RUNTIME STATE
	// ==========================================

	/** Current driving state */
	UPROPERTY(BlueprintReadOnly, Category = "Racing AI")
	FMGAIDrivingState DrivingState;

	/** Is actively racing? */
	UPROPERTY()
	bool bIsRacing = false;

	/** Is paused? */
	UPROPERTY()
	bool bIsPaused = false;

	/** Accumulated time for skill variation */
	float SkillVariationTimer = 0.0f;

	/** Current random steering offset (from mistakes) */
	float CurrentSteeringNoise = 0.0f;

	/** Current random throttle offset */
	float CurrentThrottleNoise = 0.0f;

	/** Time since last nitrous use */
	float TimeSinceNitrous = 0.0f;
};
