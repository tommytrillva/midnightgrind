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
 * Stunt quality rating
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
 * Rotation direction
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
 * Landing state
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
 * Individual stunt event data
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
 * Active air state tracking
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
 * Stunt point configuration
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
 * Stunt threshold definitions
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
 * Stunt combo data
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
 * Two-wheel driving state
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
 * Session stunt statistics
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
 * Stunt zone definition
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

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStuntStarted, EMGStuntType, StuntType, FVector, LaunchLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStuntCompleted, const FMGStuntEvent&, Event, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStuntFailed, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRotationMilestone, EMGStuntType, RotationType, int32, Rotations, int32, Points);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboUpdated, int32, ComboCount, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboBanked, int32, FinalCombo, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTwoWheelStarted, bool, bLeftSide, float, TiltAngle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTwoWheelEnded, float, Duration, float, Distance, int32, Points);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLanding, EMGLandingState, State, int32, BonusPoints);

/**
 * Stunt Subsystem
 * Manages aerial tricks, jumps, rotations, and stunt scoring
 */
UCLASS()
class MIDNIGHTGRIND_API UMGStuntSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
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

	// Air State Management
	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyLaunch(FVector Position, FVector Velocity, FRotator Rotation, bool bWasDrifting);

	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void UpdateAirState(FVector CurrentPosition, FRotator CurrentRotation, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyLanding(FVector Position, FVector Velocity, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyNearMissWhileAirborne();

	UFUNCTION(BlueprintCallable, Category = "Stunt|Air")
	void NotifyOncomingWhileAirborne();

	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	bool IsAirborne() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	FMGActiveAirState GetActiveAirState() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	float GetCurrentAirTime() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Air")
	float GetCurrentHeight() const;

	// Two-Wheel Driving
	UFUNCTION(BlueprintCallable, Category = "Stunt|TwoWheel")
	void StartTwoWheelDriving(bool bLeftSide, float TiltAngle);

	UFUNCTION(BlueprintCallable, Category = "Stunt|TwoWheel")
	void UpdateTwoWheelDriving(float Distance, float TiltAngle, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Stunt|TwoWheel")
	void EndTwoWheelDriving();

	UFUNCTION(BlueprintPure, Category = "Stunt|TwoWheel")
	bool IsTwoWheelDriving() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|TwoWheel")
	FMGTwoWheelState GetTwoWheelState() const;

	// Stunt Detection
	UFUNCTION(BlueprintCallable, Category = "Stunt")
	TArray<EMGStuntType> DetectStuntsFromAirState() const;

	UFUNCTION(BlueprintCallable, Category = "Stunt")
	EMGStuntQuality CalculateStuntQuality(const FMGStuntEvent& Event) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt")
	EMGLandingState CalculateLandingState(FRotator LaunchRotation, FRotator LandingRotation, FVector LandingVelocity) const;

	// Point Calculation
	UFUNCTION(BlueprintCallable, Category = "Stunt|Points")
	int32 CalculateStuntPoints(const FMGStuntEvent& Event) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt|Points")
	int32 CalculateLandingBonus(EMGLandingState Landing, int32 BasePoints) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt|Points")
	float CalculateBoostReward(const FMGStuntEvent& Event) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Stunt|Config")
	void SetStuntPointConfig(EMGStuntType StuntType, const FMGStuntPointConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Stunt|Config")
	FMGStuntPointConfig GetStuntPointConfig(EMGStuntType StuntType) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt|Config")
	void SetThresholds(const FMGStuntThresholds& Thresholds);

	UFUNCTION(BlueprintPure, Category = "Stunt|Config")
	FMGStuntThresholds GetThresholds() const;

	// Combo Management
	UFUNCTION(BlueprintCallable, Category = "Stunt|Combo")
	void ExtendCombo(const FMGStuntEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Stunt|Combo")
	void BankCombo();

	UFUNCTION(BlueprintCallable, Category = "Stunt|Combo")
	void LoseCombo();

	UFUNCTION(BlueprintPure, Category = "Stunt|Combo")
	FMGStuntCombo GetCurrentCombo() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Combo")
	bool IsComboActive() const;

	// Stunt Zones
	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	void RegisterStuntZone(const FMGStuntZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	FMGStuntZone GetStuntZone(const FString& ZoneId) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	FMGStuntZone GetNearestStuntZone(FVector Location) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	bool IsInStuntZone(FVector Location, FString& OutZoneId) const;

	UFUNCTION(BlueprintCallable, Category = "Stunt|Zones")
	void UpdateStuntZoneBestScore(const FString& ZoneId, int32 NewScore);

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Stunt|Session")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Stunt|Session")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "Stunt|Session")
	bool IsSessionActive() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Session")
	FMGStuntSessionStats GetSessionStats() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "Stunt|Stats")
	int32 GetTotalStuntPoints() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Stats")
	int32 GetTotalStunts() const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Stats")
	TArray<FMGStuntEvent> GetRecentStunts(int32 Count) const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "Stunt|Utility")
	FText GetStuntDisplayName(EMGStuntType StuntType) const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Utility")
	FText GetQualityDisplayName(EMGStuntQuality Quality) const;

	UFUNCTION(BlueprintPure, Category = "Stunt|Utility")
	FLinearColor GetQualityColor(EMGStuntQuality Quality) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Stunt|Persistence")
	void SaveStuntData();

	UFUNCTION(BlueprintCallable, Category = "Stunt|Persistence")
	void LoadStuntData();

protected:
	FMGStuntEvent FinalizeStunt();
	void CheckRotationMilestones();
	void TickCombo(float DeltaTime);
	void InitializeDefaultConfigs();
	int32 CountFullRotations(float Degrees) const;

private:
	UPROPERTY()
	FMGActiveAirState ActiveAirState;

	UPROPERTY()
	FMGTwoWheelState TwoWheelState;

	UPROPERTY()
	FMGStuntCombo CurrentCombo;

	UPROPERTY()
	FMGStuntThresholds StuntThresholds;

	UPROPERTY()
	FMGStuntSessionStats SessionStats;

	UPROPERTY()
	TMap<EMGStuntType, FMGStuntPointConfig> PointConfigs;

	UPROPERTY()
	TMap<FString, FMGStuntZone> StuntZones;

	UPROPERTY()
	TArray<FMGStuntEvent> RecentStunts;

	UPROPERTY()
	bool bSessionActive = false;

	UPROPERTY()
	int32 LastReportedRolls = 0;

	UPROPERTY()
	int32 LastReportedFlips = 0;

	UPROPERTY()
	int32 LastReportedSpins = 0;

	FTimerHandle ComboTickTimer;

	static constexpr int32 MaxRecentStunts = 50;
};
