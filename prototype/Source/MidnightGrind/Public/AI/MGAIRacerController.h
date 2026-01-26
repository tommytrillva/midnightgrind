// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MGAIRacerController.generated.h"

class UMGAIDriverProfile;
class UMGTrackSubsystem;
class AMGRaceGameMode;

/**
 * AI driving state
 */
UENUM(BlueprintType)
enum class EMGAIDrivingState : uint8
{
	/** Waiting at start */
	Waiting,
	/** Normal racing */
	Racing,
	/** Attempting to overtake */
	Overtaking,
	/** Defending position */
	Defending,
	/** Recovering from collision/off-track */
	Recovering,
	/** Slowing for hazard */
	Caution,
	/** Catching up (rubber banding) */
	CatchingUp,
	/** Slowing down (rubber banding) */
	SlowingDown,
	/** Finished race */
	Finished
};

/**
 * AI perception data for nearby vehicles
 */
USTRUCT(BlueprintType)
struct FMGAIVehiclePerception
{
	GENERATED_BODY()

	/** The perceived vehicle */
	UPROPERTY(BlueprintReadOnly)
	AActor* Vehicle = nullptr;

	/** Relative position */
	UPROPERTY(BlueprintReadOnly)
	FVector RelativePosition = FVector::ZeroVector;

	/** Relative velocity */
	UPROPERTY(BlueprintReadOnly)
	FVector RelativeVelocity = FVector::ZeroVector;

	/** Distance to vehicle */
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;

	/** Angle to vehicle (-180 to 180) */
	UPROPERTY(BlueprintReadOnly)
	float Angle = 0.0f;

	/** Is ahead of us */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAhead = false;

	/** Is on left side */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOnLeft = false;

	/** Time to collision (if approaching) */
	UPROPERTY(BlueprintReadOnly)
	float TimeToCollision = FLT_MAX;

	/** Is player vehicle */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayer = false;
};

/**
 * Racing line point for AI navigation
 */
USTRUCT(BlueprintType)
struct FMGAIRacingLinePoint
{
	GENERATED_BODY()

	/** World position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** Optimal direction at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Direction = FVector::ForwardVector;

	/** Target speed at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetSpeed = 100.0f;

	/** Track width at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackWidth = 10.0f;

	/** Distance along track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongTrack = 0.0f;

	/** Is corner apex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsApex = false;

	/** Is braking zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBrakingZone = false;

	/** Is acceleration zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAccelerationZone = false;
};

/**
 * AI steering calculation result
 */
USTRUCT(BlueprintType)
struct FMGAISteeringOutput
{
	GENERATED_BODY()

	/** Steering input (-1 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float Steering = 0.0f;

	/** Throttle input (0 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float Throttle = 0.0f;

	/** Brake input (0 to 1) */
	UPROPERTY(BlueprintReadOnly)
	float Brake = 0.0f;

	/** Handbrake input */
	UPROPERTY(BlueprintReadOnly)
	bool bHandbrake = false;

	/** NOS input */
	UPROPERTY(BlueprintReadOnly)
	bool bNOS = false;

	/** Target point being steered toward */
	UPROPERTY(BlueprintReadOnly)
	FVector TargetPoint = FVector::ZeroVector;
};

/**
 * AI Racer Controller
 * Controls AI opponent vehicles during races
 *
 * Features:
 * - Racing line following
 * - Dynamic difficulty (rubber banding)
 * - Overtaking and defending
 * - Collision avoidance
 * - Personality-driven behavior
 */
UCLASS()
class MIDNIGHTGRIND_API AMGAIRacerController : public AAIController
{
	GENERATED_BODY()

public:
	AMGAIRacerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set driver profile */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDriverProfile(UMGAIDriverProfile* Profile);

	/** Get driver profile */
	UFUNCTION(BlueprintPure, Category = "AI|Config")
	UMGAIDriverProfile* GetDriverProfile() const { return DriverProfile; }

	/** Set difficulty multiplier (0.5 = easy, 1.0 = normal, 1.5 = hard) */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetDifficultyMultiplier(float Multiplier);

	/** Enable/disable rubber banding */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRubberBandingEnabled(bool bEnabled);

	/** Set racing line points */
	UFUNCTION(BlueprintCallable, Category = "AI|Config")
	void SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine);

	// ==========================================
	// STATE
	// ==========================================

	/** Get current driving state */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	EMGAIDrivingState GetDrivingState() const { return CurrentState; }

	/** Get current steering output */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	FMGAISteeringOutput GetSteeringOutput() const { return CurrentSteering; }

	/** Get current speed */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetCurrentSpeed() const;

	/** Get target speed */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetTargetSpeed() const { return CurrentTargetSpeed; }

	/** Get distance to racing line */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	float GetDistanceToRacingLine() const;

	/** Is currently overtaking */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	bool IsOvertaking() const { return CurrentState == EMGAIDrivingState::Overtaking; }

	/** Get perceived vehicles */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	TArray<FMGAIVehiclePerception> GetPerceivedVehicles() const { return PerceivedVehicles; }

	// ==========================================
	// CONTROL
	// ==========================================

	/** Start racing */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StartRacing();

	/** Stop racing (finish) */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void StopRacing();

	/** Force state change */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void ForceState(EMGAIDrivingState NewState);

	/** Notify of collision */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void NotifyCollision(AActor* OtherActor, const FVector& ImpactPoint, const FVector& ImpactNormal);

	/** Notify off track */
	UFUNCTION(BlueprintCallable, Category = "AI|Control")
	void NotifyOffTrack();

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Driver profile asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UMGAIDriverProfile* DriverProfile;

	/** Difficulty multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float DifficultyMultiplier = 1.0f;

	/** Enable rubber banding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRubberBandingEnabled = true;

	/** Rubber band strength (how much to adjust speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float RubberBandStrength = 0.2f;

	/** Look ahead distance for steering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SteeringLookAhead = 15.0f;

	/** Look ahead distance for speed (braking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SpeedLookAhead = 30.0f;

	/** Perception radius for other vehicles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float PerceptionRadius = 50.0f;

	/** Minimum time gap when following */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MinFollowingGap = 1.0f;

	/** Overtake decision threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float OvertakeThreshold = 0.7f;

	// ==========================================
	// RACING LINE
	// ==========================================

	/** Racing line points */
	UPROPERTY()
	TArray<FMGAIRacingLinePoint> RacingLinePoints;

	/** Current racing line index */
	int32 CurrentRacingLineIndex = 0;

	/** Progress along racing line (0-1) */
	float RacingLineProgress = 0.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current driving state */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EMGAIDrivingState CurrentState = EMGAIDrivingState::Waiting;

	/** Current steering output */
	FMGAISteeringOutput CurrentSteering;

	/** Current target speed */
	float CurrentTargetSpeed = 0.0f;

	/** Perceived vehicles */
	TArray<FMGAIVehiclePerception> PerceivedVehicles;

	/** Time in current state */
	float TimeInState = 0.0f;

	/** Recovery timer */
	float RecoveryTimer = 0.0f;

	/** Overtake timer */
	float OvertakeTimer = 0.0f;

	/** Overtake side (true = left, false = right) */
	bool bOvertakeOnLeft = false;

	/** Last steering error (for PID) */
	float LastSteeringError = 0.0f;

	/** Steering error integral */
	float SteeringErrorIntegral = 0.0f;

	// ==========================================
	// REFERENCES
	// ==========================================

	/** Track subsystem */
	UPROPERTY()
	UMGTrackSubsystem* TrackSubsystem;

	/** Race game mode */
	UPROPERTY()
	AMGRaceGameMode* RaceGameMode;

	/** Controlled vehicle pawn */
	UPROPERTY()
	APawn* VehiclePawn;

	// ==========================================
	// CORE LOGIC
	// ==========================================

	/** Update perception of nearby vehicles */
	void UpdatePerception();

	/** Update driving state machine */
	void UpdateStateMachine(float DeltaTime);

	/** Calculate steering for current state */
	void CalculateSteering(float DeltaTime);

	/** Apply steering to vehicle */
	void ApplySteering();

	/** Update racing line progress */
	void UpdateRacingLineProgress();

	// ==========================================
	// STATE HANDLERS
	// ==========================================

	/** Handle waiting state */
	void HandleWaitingState(float DeltaTime);

	/** Handle racing state */
	void HandleRacingState(float DeltaTime);

	/** Handle overtaking state */
	void HandleOvertakingState(float DeltaTime);

	/** Handle defending state */
	void HandleDefendingState(float DeltaTime);

	/** Handle recovering state */
	void HandleRecoveringState(float DeltaTime);

	/** Handle catching up state */
	void HandleCatchingUpState(float DeltaTime);

	/** Handle slowing down state */
	void HandleSlowingDownState(float DeltaTime);

	// ==========================================
	// STEERING CALCULATIONS
	// ==========================================

	/** Calculate steering to follow racing line */
	FMGAISteeringOutput CalculateRacingLineSteering();

	/** Calculate steering for overtake */
	FMGAISteeringOutput CalculateOvertakeSteering();

	/** Calculate steering for defense */
	FMGAISteeringOutput CalculateDefenseSteering();

	/** Calculate steering for recovery */
	FMGAISteeringOutput CalculateRecoverySteering();

	/** Calculate avoidance steering adjustment */
	FVector CalculateAvoidanceOffset();

	/** Calculate target speed for current position */
	float CalculateTargetSpeed();

	/** Calculate rubber band speed adjustment */
	float CalculateRubberBandAdjustment();

	// ==========================================
	// UTILITIES
	// ==========================================

	/** Get point on racing line at distance ahead */
	FMGAIRacingLinePoint GetRacingLinePointAhead(float Distance) const;

	/** Get closest racing line point */
	int32 FindClosestRacingLinePoint(const FVector& Position) const;

	/** Should attempt overtake */
	bool ShouldAttemptOvertake() const;

	/** Should defend position */
	bool ShouldDefendPosition() const;

	/** Get player vehicle */
	APawn* GetPlayerVehicle() const;

	/** Get vehicle ahead */
	FMGAIVehiclePerception GetVehicleAhead() const;

	/** Get vehicle behind */
	FMGAIVehiclePerception GetVehicleBehind() const;

	/** Check if path is clear for overtake */
	bool IsOvertakePathClear(bool bOnLeft) const;

	/** Apply driver profile modifiers */
	void ApplyProfileModifiers(FMGAISteeringOutput& Output);

	/** Set new state */
	void SetState(EMGAIDrivingState NewState);

	/** Add noise to steering (for personality) */
	float AddSteeringNoise(float BaseValue);
};
