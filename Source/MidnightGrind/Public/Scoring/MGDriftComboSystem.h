// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGDriftComboSystem.h
 * @brief Drift scoring system with combo chains, multipliers, and style bonuses
 *
 * =============================================================================
 * OVERVIEW
 * =============================================================================
 *
 * The Drift Combo System tracks and scores player drifting with arcade-style
 * combo mechanics. Players earn points for maintaining drifts, chaining multiple
 * drifts together, and performing stylish maneuvers. The system is inspired by
 * classic arcade racers like Ridge Racer and Initial D.
 *
 * =============================================================================
 * @section concepts KEY CONCEPTS FOR BEGINNERS
 * =============================================================================
 *
 * 1. WHAT IS A DRIFT?
 *    - A drift is when the car slides sideways while moving forward.
 *    - Measured by "drift angle" - the difference between car direction and velocity.
 *    - MinDriftAngle (default 15 degrees) is required to start scoring.
 *    - MinDriftSpeed (default 40 km/h) prevents slow-speed cheese.
 *
 * 2. COMBO STATES (EMGDriftComboState):
 *    - Idle: Not drifting, no active combo.
 *    - Drifting: Currently in a drift, accumulating points.
 *    - Grace: Drift ended, brief window to start another drift.
 *    - Failed: Combo broken (collision, spin, etc).
 *
 * 3. COMBO CHAIN:
 *    - Each consecutive drift increases ComboCount.
 *    - ComboGracePeriod (default 2 seconds) is time allowed between drifts.
 *    - If you start a new drift within grace period, combo continues.
 *    - If grace period expires or you crash, combo is "dropped".
 *
 * 4. MULTIPLIER TIERS (FMGComboTier):
 *    - Higher combo counts unlock better multipliers.
 *    - Example tiers: 1x (1 drift), 1.5x (3 drifts), 2x (5 drifts), 3x (10 drifts).
 *    - Each tier has a name, color for UI, and multiplier value.
 *    - OnComboTierReached fires when entering a new tier.
 *
 * 5. DRIFT SCORING:
 *    - BasePointsPerSecond: Points earned each second of drifting.
 *    - AngleMultiplier: Bonus for steeper drift angles.
 *    - SpeedMultiplier: Bonus for drifting at higher speeds.
 *    - Final score = (Time * BasePoints) * (1 + Angle*AngleMult + Speed*SpeedMult) * ComboMultiplier
 *
 * 6. STYLE BONUSES (EMGDriftStyleBonus):
 *    - Extra points for skilled or risky moves:
 *    - Marathon: Long continuous drift (>5 seconds).
 *    - Extreme: High angle drift (>60 degrees).
 *    - NearMiss: Nearly hit something while drifting.
 *    - Overtake: Pass another car while drifting.
 *    - ChainLink: Chain multiple corners together.
 *    - Transition: Quick direction change (left-right-left).
 *    - HighSpeed: Drift at high speed (>120 km/h).
 *    - Perfect: Hit the racing line apex while drifting.
 *    - Checkpoint: Cross checkpoint while drifting.
 *
 * 7. DRIFT DATA (FMGDriftData):
 *    - Statistics for each individual drift:
 *    - Duration: How long the drift lasted.
 *    - MaxAngle/AverageAngle: Drift angle statistics.
 *    - MaxSpeed/AverageSpeed: Speed during drift.
 *    - Distance: How far traveled while drifting.
 *    - bWasLeftDrift: Direction for transition detection.
 *
 * 8. BANKING SCORES:
 *    - CurrentComboScore: Points accumulated but not yet "banked".
 *    - TotalBankedScore: Safely stored points.
 *    - Dropping a combo loses CurrentComboScore.
 *    - BankComboScore() manually banks the current score.
 *    - Scores are automatically banked when combo ends successfully.
 *
 * 9. EXTERNAL NOTIFICATIONS:
 *    - NotifyNearMiss(): Call when near miss system detects close call.
 *    - NotifyOvertake(): Call when player passes another vehicle.
 *    - NotifyCheckpointCrossed(): Call when crossing a checkpoint.
 *    - DropCombo(): Call on collision or spin to break the combo.
 *
 * =============================================================================
 * @section usage USAGE EXAMPLE
 * =============================================================================
 *
 * @code
 * // The component auto-updates from vehicle physics in TickComponent
 * // You mainly need to handle external events:
 *
 * // Get the component
 * UMGDriftComboSystem* DriftSystem = Vehicle->FindComponentByClass<UMGDriftComboSystem>();
 *
 * // Subscribe to events for UI updates
 * DriftSystem->OnDriftStarted.AddDynamic(this, &UMyHUD::ShowDriftStarted);
 * DriftSystem->OnDriftEnded.AddDynamic(this, &UMyHUD::ShowDriftScore);
 * DriftSystem->OnComboUpdated.AddDynamic(this, &UMyHUD::UpdateComboDisplay);
 * DriftSystem->OnComboDropped.AddDynamic(this, &UMyHUD::ShowComboDropped);
 * DriftSystem->OnStyleBonusEarned.AddDynamic(this, &UMyHUD::ShowStyleBonus);
 * DriftSystem->OnComboTierReached.AddDynamic(this, &UMyHUD::ShowTierUp);
 *
 * // Notify of external events
 * void AMyVehicle::OnNearMissDetected()
 * {
 *     DriftSystem->NotifyNearMiss();
 * }
 *
 * void AMyVehicle::OnOvertakeComplete()
 * {
 *     DriftSystem->NotifyOvertake();
 * }
 *
 * void AMyVehicle::OnCollision(float ImpactForce)
 * {
 *     if (ImpactForce > SpinThreshold)
 *     {
 *         DriftSystem->DropCombo(); // Crash breaks the combo
 *     }
 * }
 *
 * // Query current state for UI
 * void UMyHUD::UpdateDriftUI()
 * {
 *     if (DriftSystem->IsInCombo())
 *     {
 *         int32 Combo = DriftSystem->GetComboCount();
 *         float Multiplier = DriftSystem->GetCurrentMultiplier();
 *         float Score = DriftSystem->GetCurrentComboScore();
 *         float Grace = DriftSystem->GetGraceTimeRemaining();
 *         FMGComboTier Tier = DriftSystem->GetCurrentTier();
 *
 *         // Update UI with these values...
 *     }
 * }
 *
 * // At race end, get total drift score
 * float FinalDriftScore = DriftSystem->GetTotalScore();
 * @endcode
 *
 * =============================================================================
 * @section configuration CONFIGURATION IN EDITOR
 * =============================================================================
 *
 * Configure the component in the Blueprint editor or Details panel:
 *
 * 1. DRIFT THRESHOLDS:
 *    - MinDriftAngle: Lower = easier to start scoring (15-20 recommended).
 *    - MinDriftSpeed: Prevents exploits at low speed (30-50 km/h).
 *
 * 2. SCORING BALANCE:
 *    - BasePointsPerSecond: Base earning rate (100 = casual, 50 = hardcore).
 *    - AngleMultiplier: How much angle matters (0.02 = moderate).
 *    - SpeedMultiplier: How much speed matters (0.01 = moderate).
 *
 * 3. COMBO SETTINGS:
 *    - ComboGracePeriod: Time between drifts (2s = forgiving, 1s = tight).
 *    - ComboTiers: Array of tiers with thresholds and multipliers.
 *
 * 4. STYLE BONUS THRESHOLDS:
 *    - MarathonDriftThreshold: Seconds for marathon bonus (5s default).
 *    - ExtremeAngleThreshold: Angle for extreme bonus (60 degrees).
 *    - HighSpeedThreshold: Speed for high-speed bonus (120 km/h).
 *    - TransitionTimeWindow: Max time for transition bonus (0.5s).
 *
 * @see FMGComboTier for multiplier tier setup
 * @see FMGStyleBonusConfig for style bonus configuration
 * @see EMGDriftStyleBonus for available style bonus types
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGDriftComboSystem.generated.h"

class AMGVehiclePawn;
class UMGVehicleMovementComponent;

/**
 * Drift combo state
 */
UENUM(BlueprintType)
enum class EMGDriftComboState : uint8
{
	/** Not drifting */
	Idle,
	/** Currently building a drift */
	Drifting,
	/** Drift ended, in grace period */
	Grace,
	/** Combo failed (collision, spin, etc) */
	Failed
};

/**
 * Drift style bonus types
 */
UENUM(BlueprintType)
enum class EMGDriftStyleBonus : uint8
{
	None,
	/** Long continuous drift */
	Marathon,
	/** High angle drift */
	Extreme,
	/** Near miss while drifting */
	NearMiss,
	/** Overtake while drifting */
	Overtake,
	/** Chain multiple corners */
	ChainLink,
	/** Drift in opposite direction quickly */
	Transition,
	/** Drift at high speed */
	HighSpeed,
	/** Perfect drift line (apex) */
	Perfect,
	/** Drift through checkpoint */
	Checkpoint
};

/**
 * Individual drift data
 */
USTRUCT(BlueprintType)
struct FMGDriftData
{
	GENERATED_BODY()

	/** Duration of drift in seconds */
	UPROPERTY(BlueprintReadOnly)
	float Duration = 0.0f;

	/** Maximum angle reached */
	UPROPERTY(BlueprintReadOnly)
	float MaxAngle = 0.0f;

	/** Average angle */
	UPROPERTY(BlueprintReadOnly)
	float AverageAngle = 0.0f;

	/** Average speed during drift */
	UPROPERTY(BlueprintReadOnly)
	float AverageSpeed = 0.0f;

	/** Max speed during drift */
	UPROPERTY(BlueprintReadOnly)
	float MaxSpeed = 0.0f;

	/** Distance covered while drifting */
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;

	/** Base score for this drift */
	UPROPERTY(BlueprintReadOnly)
	float BaseScore = 0.0f;

	/** Was this drift direction left or right */
	UPROPERTY(BlueprintReadOnly)
	bool bWasLeftDrift = false;
};

/**
 * Combo multiplier tier
 */
USTRUCT(BlueprintType)
struct FMGComboTier
{
	GENERATED_BODY()

	/** Tier name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TierName;

	/** Minimum combo count to reach this tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinComboCount = 0;

	/** Multiplier for this tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/** Color for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TierColor = FLinearColor::White;
};

/**
 * Style bonus config
 */
USTRUCT(BlueprintType)
struct FMGStyleBonusConfig
{
	GENERATED_BODY()

	/** Bonus type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDriftStyleBonus BonusType = EMGDriftStyleBonus::None;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Points awarded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusPoints = 100;

	/** Multiplier added */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MultiplierBonus = 0.1f;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftStarted, float, InitialAngle, bool, bIsLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDriftEnded, const FMGDriftData&, DriftData, float, ScoreEarned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnComboUpdated, int32, ComboCount, float, Multiplier, float, TotalScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboDropped, float, FinalScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStyleBonusEarned, EMGDriftStyleBonus, BonusType, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboTierReached, const FMGComboTier&, Tier, int32, ComboCount);

/**
 * Drift Combo System Component
 * Tracks and scores drifting with combo mechanics
 *
 * Features:
 * - Combo chain tracking
 * - Multiplier tiers
 * - Style bonuses
 * - Grace period between drifts
 * - Near miss integration
 * - Score calculation
 */
UCLASS(ClassGroup=(Racing), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGDriftComboSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGDriftComboSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Minimum drift angle to start scoring */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Drift")
	float MinDriftAngle = 15.0f;

	/** Minimum speed to start scoring (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Drift")
	float MinDriftSpeed = 40.0f;

	/** Grace period between drifts to maintain combo (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Combo")
	float ComboGracePeriod = 2.0f;

	/** Base points per second of drifting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Scoring")
	float BasePointsPerSecond = 100.0f;

	/** Points multiplier per degree of angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Scoring")
	float AngleMultiplier = 0.02f;

	/** Points multiplier for speed (per 10 km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Scoring")
	float SpeedMultiplier = 0.01f;

	/** Combo tiers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Combo")
	TArray<FMGComboTier> ComboTiers;

	/** Style bonus configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Style")
	TArray<FMGStyleBonusConfig> StyleBonuses;

	/** Duration threshold for Marathon bonus (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Style")
	float MarathonDriftThreshold = 5.0f;

	/** Angle threshold for Extreme bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Style")
	float ExtremeAngleThreshold = 60.0f;

	/** Speed threshold for HighSpeed bonus (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Style")
	float HighSpeedThreshold = 120.0f;

	/** Maximum time between drifts for Transition bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Style")
	float TransitionTimeWindow = 0.5f;

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Get current combo state */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	EMGDriftComboState GetComboState() const { return CurrentState; }

	/** Get current combo count */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	int32 GetComboCount() const { return ComboCount; }

	/** Get current multiplier */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	float GetCurrentMultiplier() const { return CurrentMultiplier; }

	/** Get current combo score (not yet banked) */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	float GetCurrentComboScore() const { return CurrentComboScore; }

	/** Get total banked score */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	float GetTotalScore() const { return TotalBankedScore; }

	/** Get current drift data (if drifting) */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	FMGDriftData GetCurrentDriftData() const { return CurrentDrift; }

	/** Get grace timer remaining */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	float GetGraceTimeRemaining() const { return GraceTimer; }

	/** Is currently drifting */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	bool IsDrifting() const { return CurrentState == EMGDriftComboState::Drifting; }

	/** Is in combo (drifting or grace period) */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	bool IsInCombo() const { return CurrentState == EMGDriftComboState::Drifting || CurrentState == EMGDriftComboState::Grace; }

	/** Get current combo tier */
	UFUNCTION(BlueprintPure, Category = "Drift Combo")
	FMGComboTier GetCurrentTier() const;

	// ==========================================
	// ACTIONS
	// ==========================================

	/** Manually award style bonus */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	void AwardStyleBonus(EMGDriftStyleBonus BonusType);

	/** Notify of near miss (for bonus) */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	void NotifyNearMiss();

	/** Notify of overtake while drifting */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	void NotifyOvertake();

	/** Notify of checkpoint crossed while drifting */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	void NotifyCheckpointCrossed();

	/** Force drop combo (collision, spin out, etc) */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	void DropCombo();

	/** Reset all scoring */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	void ResetScore();

	/** Bank current combo score */
	UFUNCTION(BlueprintCallable, Category = "Drift Combo")
	float BankComboScore();

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Drift Combo|Events")
	FOnDriftStarted OnDriftStarted;

	UPROPERTY(BlueprintAssignable, Category = "Drift Combo|Events")
	FOnDriftEnded OnDriftEnded;

	UPROPERTY(BlueprintAssignable, Category = "Drift Combo|Events")
	FOnComboUpdated OnComboUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Drift Combo|Events")
	FOnComboDropped OnComboDropped;

	UPROPERTY(BlueprintAssignable, Category = "Drift Combo|Events")
	FOnStyleBonusEarned OnStyleBonusEarned;

	UPROPERTY(BlueprintAssignable, Category = "Drift Combo|Events")
	FOnComboTierReached OnComboTierReached;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Start a new drift */
	void StartDrift(float InitialAngle, bool bIsLeft);

	/** End current drift */
	void EndDrift();

	/** Update drift scoring */
	void UpdateDriftScoring(float MGDeltaTime, float CurrentAngle, float CurrentSpeed);

	/** Update combo state */
	void UpdateComboState(float MGDeltaTime);

	/** Calculate score for a drift */
	float CalculateDriftScore(const FMGDriftData& DriftData) const;

	/** Calculate multiplier for current combo */
	float CalculateMultiplier() const;

	/** Check and award style bonuses */
	void CheckStyleBonuses(const FMGDriftData& DriftData);

	/** Get style bonus config */
	const FMGStyleBonusConfig* GetStyleBonusConfig(EMGDriftStyleBonus BonusType) const;

	/** Check for tier advancement */
	void CheckTierAdvancement();

private:
	// ==========================================
	// STATE
	// ==========================================

	/** Current combo state */
	EMGDriftComboState CurrentState = EMGDriftComboState::Idle;

	/** Current drift being tracked */
	FMGDriftData CurrentDrift;

	/** Drift samples for averaging */
	TArray<float> AngleSamples;
	TArray<float> SpeedSamples;

	/** Combo count */
	int32 ComboCount = 0;

	/** Current multiplier */
	float CurrentMultiplier = 1.0f;

	/** Current combo score (not yet banked) */
	float CurrentComboScore = 0.0f;

	/** Total banked score */
	float TotalBankedScore = 0.0f;

	/** Grace period timer */
	float GraceTimer = 0.0f;

	/** Time since last drift ended (for transition detection) */
	float TimeSinceLastDrift = 0.0f;

	/** Last drift direction (for transition detection) */
	bool bLastDriftWasLeft = false;

	/** Current combo tier index */
	int32 CurrentTierIndex = 0;

	/** Style bonuses earned this combo */
	TSet<EMGDriftStyleBonus> EarnedBonusesThisCombo;

	// ==========================================
	// REFERENCES
	// ==========================================

	/** Cached vehicle pawn */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> VehiclePawn;

	/** Cached movement component */
	UPROPERTY()
	TWeakObjectPtr<UMGVehicleMovementComponent> MovementComponent;
};
