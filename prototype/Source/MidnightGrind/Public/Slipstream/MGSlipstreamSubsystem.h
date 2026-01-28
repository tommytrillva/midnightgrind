// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSlipstreamSubsystem.h
 * Slipstream (Drafting) System for Midnight Grind Racing Game
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem implements the "slipstream" or "drafting" mechanic - a real-world
 * racing phenomenon where following closely behind another vehicle reduces air
 * resistance, allowing you to go faster with the same power output.
 *
 * In Midnight Grind, slipstreaming provides:
 * - Speed bonuses when driving behind other vehicles
 * - "Slingshot" ability to overtake after building up charge
 * - Nitro charging while drafting
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. DRAFTING/SLIPSTREAM:
 *    When a vehicle moves through air, it creates a low-pressure "wake" behind it.
 *    A following vehicle in this wake experiences less air resistance (drag),
 *    allowing it to maintain higher speeds with less effort. This is called
 *    "drafting" in NASCAR or "slipstreaming" in Formula 1.
 *
 * 2. DRAFTING CONE:
 *    The slipstream effect only works within a cone-shaped area behind the lead
 *    vehicle. The cone has an angle (typically 30 degrees) and a maximum distance.
 *    Being directly behind the leader = maximum benefit. Being at an angle = less.
 *
 * 3. DRAFTING ZONES:
 *    - Outer: Far from leader, minimal benefit
 *    - Inner: Closer, good benefit
 *    - Optimal: "Sweet spot" distance for maximum benefit
 *    - TooClose: Dangerously close, risk of collision
 *
 * 4. SLINGSHOT MANEUVER:
 *    After drafting for a period, the follower builds up "charge". When full,
 *    they can execute a "slingshot" - pulling out of the slipstream with a
 *    temporary speed boost to overtake the leader. This is a classic racing
 *    tactic seen in real motorsport.
 *
 * 5. LINE OF SIGHT:
 *    The system can require clear line of sight between vehicles. If another
 *    car or obstacle blocks the path, the slipstream effect is interrupted.
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UWorldSubsystem, meaning one instance exists per game world/level
 * - Vehicles register themselves when spawned, unregister when destroyed
 * - The subsystem ticks every frame to update slipstream states
 * - Works with MGNitroBoostSubsystem to charge nitro while drafting
 * - Broadcasts events for UI feedback (visual effects, sounds, HUD indicators)
 *
 * USAGE EXAMPLE:
 * --------------
 * From a vehicle Blueprint or C++:
 *
 *   // Get the subsystem
 *   UMGSlipstreamSubsystem* Slipstream = GetWorld()->GetSubsystem<UMGSlipstreamSubsystem>();
 *
 *   // Register this vehicle
 *   Slipstream->RegisterVehicle(MyVehicle, MyVehicleData);
 *
 *   // Check if we're drafting
 *   if (Slipstream->IsInSlipstream(MyVehicle))
 *   {
 *       float SpeedBonus = Slipstream->GetCurrentSpeedBonus(MyVehicle);
 *       // Apply speed bonus to vehicle...
 *   }
 *
 *   // Use slingshot when ready
 *   if (Slipstream->IsSlingshotReady(MyVehicle))
 *   {
 *       Slipstream->ActivateSlingshot(MyVehicle);
 *   }
 *
 * @see UMGNitroBoostSubsystem - Nitro system that charges from drafting
 * @see UMGAerodynamicsSubsystem - Advanced aerodynamics calculations
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGSlipstreamSubsystem.generated.h"

//=============================================================================
// ENUMERATIONS
// These define the discrete states and categories used by the slipstream system
//=============================================================================

/**
 * How strong the slipstream effect currently is.
 *
 * Strength increases the longer you stay in the slipstream and the closer
 * you are to the optimal drafting position. Higher strength = more benefit.
 *
 * Used for:
 * - Scaling speed bonuses
 * - Visual effect intensity
 * - Audio feedback volume
 * - UI indicator display
 */
UENUM(BlueprintType)
enum class EMGSlipstreamStrength : uint8
{
	None,      ///< Not in a slipstream at all
	Weak,      ///< Just entered or at edge of slipstream
	Moderate,  ///< Building up, decent benefit
	Strong,    ///< Well-positioned, good benefit
	Maximum    ///< Optimal position, maximum benefit, slingshot charges fastest
};

/**
 * Which zone of the drafting area the follower vehicle is in.
 *
 * Think of the drafting area as concentric zones behind the lead vehicle:
 *
 *                    [LEAD VEHICLE]
 *                          |
 *         TooClose --------+-------- (danger zone, too close!)
 *                          |
 *          Optimal --------+-------- (sweet spot for max benefit)
 *                          |
 *           Inner  --------+-------- (good position)
 *                          |
 *           Outer  --------+-------- (minimal effect)
 *                          |
 *            None  --------+-------- (outside slipstream cone)
 *
 * The zone determines:
 * - How much speed bonus you receive
 * - How fast the slingshot charges
 * - Risk level (TooClose = collision danger)
 */
UENUM(BlueprintType)
enum class EMGDraftingZone : uint8
{
	None,      ///< Outside the slipstream cone entirely
	Outer,     ///< Far edge of the cone, minimal aerodynamic benefit
	Inner,     ///< Inside the cone, receiving good benefit
	Optimal,   ///< Perfect distance, maximum benefit and fastest charge
	TooClose   ///< Dangerously close, still get benefit but risk collision
};

USTRUCT(BlueprintType)
struct FMGSlipstreamConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDraftDistance = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDraftDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalDistanceStart = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalDistanceEnd = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DraftConeAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedBonus = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroChargeRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuildUpTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallOffTime = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinLeadVehicleSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotBonus = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotDuration = 2.0f;
};

USTRUCT(BlueprintType)
struct FMGSlipstreamState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInSlipstream = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSlipstreamStrength Strength = EMGSlipstreamStrength::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDraftingZone Zone = EMGDraftingZone::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeInSlipstream = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToLeader = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleToLeader = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* LeadVehicle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlingshotReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlingshotActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotTimeRemaining = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGVehicleSlipstreamData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Vehicle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ForwardVector = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VehicleLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VehicleWidth = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragCoefficient = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGSlipstreamVisual
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSlipstreamEffect = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WeakColor = FLinearColor(0.5f, 0.5f, 1.0f, 0.3f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ModerateColor = FLinearColor(0.3f, 0.7f, 1.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor StrongColor = FLinearColor(0.0f, 0.8f, 1.0f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor MaximumColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> SlipstreamEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SlipstreamSound;
};

USTRUCT(BlueprintType)
struct FMGSlipstreamStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeInSlipstream = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceInSlipstream = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlingshotsPerformed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SuccessfulOvertakes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestSlipstreamDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroChargedFromDrafting = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesAsLeader = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeAsLeader = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlipstreamEntered, AActor*, LeadVehicle, EMGDraftingZone, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlipstreamExited);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlipstreamStrengthChanged, EMGSlipstreamStrength, NewStrength);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlingshotReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlingshotActivated, float, BonusSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlingshotEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDraftingNitroCharged, float, Amount);

UCLASS()
class MIDNIGHTGRIND_API UMGSlipstreamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Registration")
	void RegisterVehicle(AActor* Vehicle, const FMGVehicleSlipstreamData& Data);

	UFUNCTION(BlueprintCallable, Category = "Slipstream|Registration")
	void UnregisterVehicle(AActor* Vehicle);

	UFUNCTION(BlueprintCallable, Category = "Slipstream|Registration")
	void UpdateVehicleData(AActor* Vehicle, const FMGVehicleSlipstreamData& Data);

	UFUNCTION(BlueprintPure, Category = "Slipstream|Registration")
	bool IsVehicleRegistered(AActor* Vehicle) const;

	// Slipstream State
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	FMGSlipstreamState GetSlipstreamState(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	bool IsInSlipstream(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	float GetCurrentSpeedBonus(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	EMGSlipstreamStrength GetSlipstreamStrength(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	EMGDraftingZone GetDraftingZone(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	AActor* GetLeadVehicle(AActor* Vehicle) const;

	// Slingshot
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Slingshot")
	bool ActivateSlingshot(AActor* Vehicle);

	UFUNCTION(BlueprintPure, Category = "Slipstream|Slingshot")
	bool IsSlingshotReady(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|Slingshot")
	bool IsSlingshotActive(AActor* Vehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|Slingshot")
	float GetSlingshotChargePercent(AActor* Vehicle) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Config")
	void SetConfig(const FMGSlipstreamConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Slipstream|Config")
	FMGSlipstreamConfig GetConfig() const { return Config; }

	UFUNCTION(BlueprintCallable, Category = "Slipstream|Config")
	void SetVisualConfig(const FMGSlipstreamVisual& NewVisual);

	UFUNCTION(BlueprintPure, Category = "Slipstream|Config")
	FMGSlipstreamVisual GetVisualConfig() const { return VisualConfig; }

	// Statistics
	UFUNCTION(BlueprintPure, Category = "Slipstream|Stats")
	FMGSlipstreamStats GetStats(AActor* Vehicle) const;

	UFUNCTION(BlueprintCallable, Category = "Slipstream|Stats")
	void ResetStats(AActor* Vehicle);

	UFUNCTION(BlueprintCallable, Category = "Slipstream|Stats")
	void ResetAllStats();

	// Queries
	UFUNCTION(BlueprintPure, Category = "Slipstream|Query")
	TArray<AActor*> GetVehiclesInSlipstream(AActor* LeadVehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|Query")
	int32 GetDraftingTrainLength(AActor* LeadVehicle) const;

	UFUNCTION(BlueprintPure, Category = "Slipstream|Query")
	AActor* FindBestDraftTarget(AActor* Vehicle) const;

	// Debug
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Debug")
	void SetDebugDrawEnabled(bool bEnabled);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlipstreamEntered OnSlipstreamEntered;

	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlipstreamExited OnSlipstreamExited;

	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlipstreamStrengthChanged OnSlipstreamStrengthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlingshotReady OnSlingshotReady;

	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlingshotActivated OnSlingshotActivated;

	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlingshotEnded OnSlingshotEnded;

	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnDraftingNitroCharged OnDraftingNitroCharged;

protected:
	void OnSlipstreamTick();
	void UpdateVehicleSlipstream(AActor* Vehicle);
	void ProcessSlipstreamState(AActor* Vehicle, FMGSlipstreamState& State);
	void UpdateSlingshot(AActor* Vehicle, FMGSlipstreamState& State);
	AActor* FindLeadVehicle(AActor* Vehicle) const;
	bool IsInDraftingCone(AActor* Follower, AActor* Leader) const;
	bool HasLineOfSight(AActor* Follower, AActor* Leader) const;
	float CalculateSlipstreamBonus(float Distance, EMGDraftingZone Zone) const;
	EMGDraftingZone DetermineZone(float Distance) const;
	EMGSlipstreamStrength DetermineStrength(float ChargeLevel) const;
	void DrawDebugSlipstream(AActor* Vehicle, const FMGSlipstreamState& State);

	UPROPERTY()
	FMGSlipstreamConfig Config;

	UPROPERTY()
	FMGSlipstreamVisual VisualConfig;

	UPROPERTY()
	TMap<AActor*, FMGVehicleSlipstreamData> RegisteredVehicles;

	UPROPERTY()
	TMap<AActor*, FMGSlipstreamState> VehicleStates;

	UPROPERTY()
	TMap<AActor*, FMGSlipstreamStats> VehicleStats;

	UPROPERTY()
	bool bDebugDraw = false;

	FTimerHandle SlipstreamTickHandle;
};
