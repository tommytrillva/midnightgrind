// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file MGTakedownSubsystem.h
 * @brief Arcade-Style Aggressive Driving and Takedown Combat System
 *
 * @section overview Overview
 * This subsystem implements the aggressive driving mechanics inspired by games like
 * Burnout. It handles vehicle-to-vehicle combat through takedowns, tracks aggression
 * levels, manages revenge mechanics, and provides the iconic crash camera effects.
 *
 * @section key_concepts Key Concepts for Beginners
 *
 * 1. WHAT IS A TAKEDOWN?
 *    A takedown occurs when you force another vehicle to crash through aggressive
 *    contact. Different techniques yield different takedown types:
 *    - Ram: Hit from behind at high speed
 *    - Sideswipe: Scrape along the side to push them off course
 *    - PIT Maneuver: Tap their rear quarter-panel to spin them out
 *    - Shunt: Push them into obstacles
 *    - Slam: Force them into walls
 *    - Traffic Check: Push them into oncoming traffic
 *    - Air Strike: Land on them from a jump
 *
 * 2. TAKEDOWN FLOW:
 *    @code
 *    Collision Detected -> ProcessCollision() -> IsValidTakedown()?
 *                                                    |
 *                              No: Just a bump       | Yes: Real takedown
 *                                   |                v
 *                                   v          RegisterTakedown()
 *                              No points              |
 *                                              CalculatePoints()
 *                                                     |
 *                                              UpdateStreak()
 *                                                     |
 *                                              StartCrashCamera()
 *                                                     |
 *                                              OnTakedownOccurred fires
 *    @endcode
 *
 * 3. COLLISION PROCESSING:
 *    When two vehicles collide, the physics system sends collision data to
 *    ProcessCollision(). The system analyzes:
 *    - Impact force (MinTakedownImpactForce threshold = 5000 Newtons)
 *    - Impact angle (determines takedown type)
 *    - Relative velocities (who was the aggressor)
 *    - Attacker vs Victim determination
 *
 * 4. AGGRESSION SYSTEM:
 *    The aggression meter tracks how aggressively you're driving:
 *    - Builds from collisions, near-misses, and takedowns
 *    - Decays over time (DecayRate per second)
 *    - Higher levels = higher score multipliers
 *    - Max level triggers "Rampage" mode with bonus effects
 *
 *    Levels: None -> Mild -> Moderate -> Aggressive -> Violent -> Rampage
 *
 * 5. STREAK SYSTEM:
 *    Chain takedowns together for multiplied rewards:
 *    - Each takedown resets the streak timer (StreakWindow = 10 seconds)
 *    - Consecutive takedowns within the window build your streak
 *    - Higher streaks = exponentially better rewards
 *    - OnStreakUpdated fires each time you extend your streak
 *    - OnStreakEnded fires when the timer expires
 *
 * 6. REVENGE SYSTEM:
 *    When an opponent takes you down, they become a "Revenge Target":
 *    - TrackRevengeTarget() marks them
 *    - Taking them down awards bonus "Revenge" points
 *    - RevengeMultiplier increases reward (default 1.5x)
 *    - Satisfying gameplay loop: get wrecked, seek revenge, profit
 *
 * 7. CRASH CAMERA:
 *    The dramatic slow-motion camera when you score a takedown:
 *    - Multiple modes: QuickSlowMo, CinematicChase, ImpactZoom, etc.
 *    - Aftertouch: Control your wreck to cause more damage
 *    - ApplyAftertouch() lets player steer their crashed vehicle
 *
 * 8. POINTS AND REWARDS:
 *    FMGTakedownPoints defines scoring for each takedown type:
 *    - BasePoints: Fixed amount for the takedown type
 *    - SpeedMultiplier: Bonus based on impact speed
 *    - ForceMultiplier: Bonus based on collision force
 *    - Special bonuses: Airborne, Revenge, Traffic, Wall
 *    - BoostReward: Refills your boost meter
 *
 * @section usage_examples Code Examples
 *
 * @code
 * // Get the takedown subsystem
 * UMGTakedownSubsystem* Takedown = GetGameInstance()->GetSubsystem<UMGTakedownSubsystem>();
 *
 * // Start a takedown session
 * Takedown->StartSession();
 *
 * // When a collision happens (usually from physics callback)
 * FMGTakedownCollision CollisionData;
 * CollisionData.AttackerId = TEXT("Player1");
 * CollisionData.VictimId = TEXT("AI_Racer_3");
 * CollisionData.ImpactPoint = HitResult.ImpactPoint;
 * CollisionData.ImpactNormal = HitResult.ImpactNormal;
 * CollisionData.AttackerVelocity = MyVehicle->GetVelocity();
 * CollisionData.VictimVelocity = OtherVehicle->GetVelocity();
 *
 * // Process the collision
 * if (Takedown->ProcessCollision(CollisionData))
 * {
 *     // A valid takedown was registered
 *     // Events will fire automatically
 * }
 *
 * // Check current aggression state
 * EMGAggressionLevel Level = Takedown->GetAggressionLevel();
 * float Percent = Takedown->GetAggressionPercent();
 *
 * // Check for active revenge targets
 * TArray<FMGRevengeTarget> Targets = Takedown->GetActiveRevengeTargets();
 * for (const FMGRevengeTarget& Target : Targets)
 * {
 *     ShowRevengeIndicator(Target.TargetId, Target.RevengeMultiplier);
 * }
 *
 * // Listen for takedown events
 * Takedown->OnTakedownOccurred.AddDynamic(this, &MyClass::HandleTakedown);
 * Takedown->OnRampageActivated.AddDynamic(this, &MyClass::HandleRampage);
 * Takedown->OnRevengeTakedown.AddDynamic(this, &MyClass::HandleRevenge);
 *
 * // Get session statistics
 * FMGTakedownSessionStats Stats = Takedown->GetSessionStats();
 * UE_LOG(LogGame, Log, TEXT("Total Takedowns: %d, Best Streak: %d"),
 *        Stats.TotalTakedowns, Stats.BestStreak);
 *
 * // End session and save
 * Takedown->EndSession();
 * Takedown->SaveTakedownData();
 * @endcode
 *
 * @section events_to_listen Events to Listen For
 * - OnTakedownOccurred: A takedown happened (shows points, triggers UI)
 * - OnPlayerWrecked: Local player got taken down (show respawn UI)
 * - OnStreakUpdated: Streak count changed (update HUD)
 * - OnStreakEnded: Streak timer expired (hide streak UI)
 * - OnAggressionLevelChanged: Aggression tier changed (update meter color)
 * - OnRampageActivated: Maximum aggression reached (special effects)
 * - OnRevengeAvailable: Someone who wrecked you is nearby
 * - OnRevengeTakedown: You got revenge (bonus celebration)
 * - OnCrashCameraStarted/Ended: Crash camera lifecycle
 * - OnAftertouchApplied: Player is controlling their wreck
 *
 * @see UMGVehicleSubsystem - Vehicle physics and damage
 * @see UMGBoostSubsystem - Boost meter rewards from takedowns
 * @see UMGScoreSubsystem - Score integration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTakedownSubsystem.generated.h"

/**
 * Type of takedown maneuver
 */
UENUM(BlueprintType)
enum class EMGTakedownType : uint8
{
	Ram				UMETA(DisplayName = "Ram"),
	Sideswipe		UMETA(DisplayName = "Sideswipe"),
	PIT				UMETA(DisplayName = "PIT Maneuver"),
	Shunt			UMETA(DisplayName = "Shunt"),
	Slam			UMETA(DisplayName = "Slam"),
	Grind			UMETA(DisplayName = "Grind"),
	TrafficCheck	UMETA(DisplayName = "Traffic Check"),
	WallGrind		UMETA(DisplayName = "Wall Grind"),
	AirStrike		UMETA(DisplayName = "Air Strike"),
	Aftertouch		UMETA(DisplayName = "Aftertouch"),
	Revenge			UMETA(DisplayName = "Revenge Takedown"),
	Psyche			UMETA(DisplayName = "Psyche-Out"),
	Signature		UMETA(DisplayName = "Signature Takedown")
};

/**
 * Takedown target type
 */
UENUM(BlueprintType)
enum class EMGTakedownTarget : uint8
{
	Opponent		UMETA(DisplayName = "Opponent"),
	Police			UMETA(DisplayName = "Police"),
	Traffic			UMETA(DisplayName = "Traffic"),
	Rival			UMETA(DisplayName = "Rival"),
	Boss			UMETA(DisplayName = "Boss"),
	Self			UMETA(DisplayName = "Self (Crashed)")
};

/**
 * Crash camera mode
 */
UENUM(BlueprintType)
enum class EMGCrashCameraMode : uint8
{
	None			UMETA(DisplayName = "None"),
	QuickSlowMo		UMETA(DisplayName = "Quick Slow-Mo"),
	CinematicChase	UMETA(DisplayName = "Cinematic Chase"),
	ImpactZoom		UMETA(DisplayName = "Impact Zoom"),
	DebrisFollow	UMETA(DisplayName = "Debris Follow"),
	WreckageOrbit	UMETA(DisplayName = "Wreckage Orbit"),
	DriverView		UMETA(DisplayName = "Driver View"),
	Aftertouch		UMETA(DisplayName = "Aftertouch Control")
};

/**
 * Aggression level
 */
UENUM(BlueprintType)
enum class EMGAggressionLevel : uint8
{
	None			UMETA(DisplayName = "None"),
	Mild			UMETA(DisplayName = "Mild"),
	Moderate		UMETA(DisplayName = "Moderate"),
	Aggressive		UMETA(DisplayName = "Aggressive"),
	Violent			UMETA(DisplayName = "Violent"),
	Rampage			UMETA(DisplayName = "Rampage")
};

/**
 * Takedown result
 */
UENUM(BlueprintType)
enum class EMGTakedownResult : uint8
{
	Success			UMETA(DisplayName = "Successful Takedown"),
	Fail			UMETA(DisplayName = "Failed"),
	Counter			UMETA(DisplayName = "Counter Takedown"),
	Trade			UMETA(DisplayName = "Trade (Both Wrecked)"),
	Evade			UMETA(DisplayName = "Target Evaded"),
	Survived		UMETA(DisplayName = "Target Survived")
};

/**
 * Individual takedown event
 */
USTRUCT(BlueprintType)
struct FMGTakedownEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTakedownType TakedownType = EMGTakedownType::Ram;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTakedownTarget TargetType = EMGTakedownTarget::Opponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTakedownResult Result = EMGTakedownResult::Success;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VictimId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedAtImpact = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostReward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRevenge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvolvedTraffic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInvolvedWall = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BonusTags;
};

/**
 * Takedown point values
 */
USTRUCT(BlueprintType)
struct FMGTakedownPoints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTakedownType TakedownType = EMGTakedownType::Ram;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RevengeBonus = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirborneBonus = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrafficBonus = 75;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WallBonus = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostReward = 10.0f;
};

/**
 * Crash camera configuration
 */
USTRUCT(BlueprintType)
struct FMGCrashCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrashCameraMode Mode = EMGCrashCameraMode::QuickSlowMo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionScale = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraDistance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrbitSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAftertouch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AftertouchForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFollowDebris = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShakeIntensity = 0.5f;
};

/**
 * Collision data for takedown detection
 */
USTRUCT(BlueprintType)
struct FMGTakedownCollision
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CollisionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VictimId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector AttackerVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VictimVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackerMass = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VictimMass = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAttackerAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVictimAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearWall = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearTraffic = false;
};

/**
 * Player aggression state
 */
USTRUCT(BlueprintType)
struct FMGAggressionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAggressionLevel Level = EMGAggressionLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionMeter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxAggression = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecentTakedowns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecentCollisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecayRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TakedownBonus = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionBonus = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampageTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RampageDuration = 10.0f;
};

/**
 * Takedown streak data
 */
USTRUCT(BlueprintType)
struct FMGTakedownStreak
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreakTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreakWindow = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTakedownEvent> StreakEvents;
};

/**
 * Revenge tracking data
 */
USTRUCT(BlueprintType)
struct FMGRevengeTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesWreckedBy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesWrecked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastWreckedByTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActiveRevenge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RevengeMultiplier = 1.5f;
};

/**
 * Session takedown statistics
 */
USTRUCT(BlueprintType)
struct FMGTakedownSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTakedowns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTimesWrecked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGTakedownType, int32> TakedownsByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> TakedownsByVictim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RevengeTakedowns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirborneTakedowns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrafficTakedowns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WallTakedowns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalBoostEarned = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestImpactForce = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownOccurred, const FMGTakedownEvent&, Event, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerWrecked, const FString&, AttackerId, const FVector&, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStreakUpdated, int32, StreakCount, float, TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreakEnded, int32, FinalStreak);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAggressionLevelChanged, EMGAggressionLevel, OldLevel, EMGAggressionLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRampageActivated, float, Duration, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRevengeAvailable, const FString&, TargetId, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRevengeTakedown, const FString&, TargetId, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrashCameraStarted, EMGCrashCameraMode, Mode, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrashCameraEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAftertouchApplied, const FString&, VehicleId, FVector, Direction, float, Force);

/**
 * Takedown Subsystem
 * Manages arcade-style aggressive driving, takedowns, and crash mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTakedownSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnTakedownOccurred OnTakedownOccurred;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnPlayerWrecked OnPlayerWrecked;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnStreakUpdated OnStreakUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnStreakEnded OnStreakEnded;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnAggressionLevelChanged OnAggressionLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnRampageActivated OnRampageActivated;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnRevengeAvailable OnRevengeAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnRevengeTakedown OnRevengeTakedown;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnCrashCameraStarted OnCrashCameraStarted;

	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnCrashCameraEnded OnCrashCameraEnded;

	/** Called when aftertouch force is applied during crash camera */
	UPROPERTY(BlueprintAssignable, Category = "Takedown|Events")
	FOnAftertouchApplied OnAftertouchApplied;

	// Collision Processing
	UFUNCTION(BlueprintCallable, Category = "Takedown|Collision")
	bool ProcessCollision(const FMGTakedownCollision& Collision);

	UFUNCTION(BlueprintCallable, Category = "Takedown|Collision")
	EMGTakedownType DetermineCollisionType(const FMGTakedownCollision& Collision) const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Collision")
	bool IsValidTakedown(const FMGTakedownCollision& Collision) const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Collision")
	float CalculateImpactForce(const FMGTakedownCollision& Collision) const;

	// Takedown Registration
	UFUNCTION(BlueprintCallable, Category = "Takedown")
	FMGTakedownEvent RegisterTakedown(EMGTakedownType Type, const FMGTakedownCollision& Collision);

	UFUNCTION(BlueprintCallable, Category = "Takedown")
	void RegisterPlayerWreck(const FString& AttackerId, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Takedown")
	int32 CalculateTakedownPoints(const FMGTakedownEvent& Event) const;

	UFUNCTION(BlueprintCallable, Category = "Takedown")
	float CalculateBoostReward(const FMGTakedownEvent& Event) const;

	// Point Configuration
	UFUNCTION(BlueprintCallable, Category = "Takedown|Config")
	void SetTakedownPoints(EMGTakedownType Type, const FMGTakedownPoints& Points);

	UFUNCTION(BlueprintCallable, Category = "Takedown|Config")
	FMGTakedownPoints GetTakedownPoints(EMGTakedownType Type) const;

	// Streak Management
	UFUNCTION(BlueprintPure, Category = "Takedown|Streak")
	FMGTakedownStreak GetCurrentStreak() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Streak")
	int32 GetCurrentStreakCount() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Streak")
	float GetStreakTimeRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Streak")
	void ExtendStreak(const FMGTakedownEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Takedown|Streak")
	void EndStreak();

	// Aggression System
	UFUNCTION(BlueprintPure, Category = "Takedown|Aggression")
	FMGAggressionState GetAggressionState() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Aggression")
	EMGAggressionLevel GetAggressionLevel() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Aggression")
	float GetAggressionPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Aggression")
	void AddAggression(float Amount);

	UFUNCTION(BlueprintPure, Category = "Takedown|Aggression")
	bool IsRampageActive() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Aggression")
	float GetRampageTimeRemaining() const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Aggression")
	float GetAggressionMultiplier() const;

	// Revenge System
	UFUNCTION(BlueprintCallable, Category = "Takedown|Revenge")
	void TrackRevengeTarget(const FString& TargetId);

	UFUNCTION(BlueprintPure, Category = "Takedown|Revenge")
	bool HasRevengeTarget(const FString& TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Revenge")
	FMGRevengeTarget GetRevengeTarget(const FString& TargetId) const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Revenge")
	TArray<FMGRevengeTarget> GetActiveRevengeTargets() const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Revenge")
	void ClearRevengeTarget(const FString& TargetId);

	// Crash Camera
	UFUNCTION(BlueprintCallable, Category = "Takedown|Camera")
	void StartCrashCamera(const FMGTakedownEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Takedown|Camera")
	void EndCrashCamera();

	UFUNCTION(BlueprintPure, Category = "Takedown|Camera")
	bool IsCrashCameraActive() const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Camera")
	void SetCrashCameraConfig(const FMGCrashCameraConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Takedown|Camera")
	FMGCrashCameraConfig GetCrashCameraConfig() const;

	UFUNCTION(BlueprintCallable, Category = "Takedown|Camera")
	void ApplyAftertouch(FVector Direction);

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Takedown|Session")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Takedown|Session")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "Takedown|Session")
	bool IsSessionActive() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Session")
	FMGTakedownSessionStats GetSessionStats() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "Takedown|Stats")
	int32 GetTotalTakedowns() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Stats")
	int32 GetTotalTimesWrecked() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Stats")
	int32 GetBestStreak() const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Stats")
	TArray<FMGTakedownEvent> GetRecentTakedowns(int32 Count) const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "Takedown|Utility")
	FText GetTakedownDisplayName(EMGTakedownType Type) const;

	UFUNCTION(BlueprintPure, Category = "Takedown|Utility")
	FLinearColor GetAggressionColor() const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Takedown|Persistence")
	void SaveTakedownData();

	UFUNCTION(BlueprintCallable, Category = "Takedown|Persistence")
	void LoadTakedownData();

protected:
	void TickAggression(float DeltaTime);
	void TickStreak(float DeltaTime);
	void UpdateAggressionLevel();
	void ActivateRampage();
	void DeactivateRampage();
	void InitializeDefaultPointValues();
	EMGCrashCameraMode SelectCrashCameraMode(const FMGTakedownEvent& Event) const;

private:
	UPROPERTY()
	TMap<EMGTakedownType, FMGTakedownPoints> TakedownPointValues;

	UPROPERTY()
	FMGTakedownStreak CurrentStreak;

	UPROPERTY()
	FMGAggressionState AggressionState;

	UPROPERTY()
	FMGTakedownSessionStats SessionStats;

	UPROPERTY()
	TMap<FString, FMGRevengeTarget> RevengeTargets;

	UPROPERTY()
	TArray<FMGTakedownEvent> RecentTakedowns;

	UPROPERTY()
	FMGCrashCameraConfig CrashCameraConfig;

	UPROPERTY()
	bool bSessionActive = false;

	UPROPERTY()
	bool bCrashCameraActive = false;

	UPROPERTY()
	FMGTakedownEvent CurrentCrashEvent;

	UPROPERTY()
	FString PlayerId;

	FTimerHandle AggressionTickTimer;
	FTimerHandle StreakTickTimer;
	FTimerHandle CrashCameraTimer;

	static constexpr int32 MaxRecentTakedowns = 50;
	static constexpr float MinTakedownImpactForce = 5000.0f;
};
