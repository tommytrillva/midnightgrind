// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCollisionSubsystem.generated.h"

/**
 * Collision type
 */
UENUM(BlueprintType)
enum class EMGCollisionType : uint8
{
	None				UMETA(DisplayName = "None"),
	VehicleToVehicle	UMETA(DisplayName = "Vehicle to Vehicle"),
	VehicleToWall		UMETA(DisplayName = "Vehicle to Wall"),
	VehicleToBarrier	UMETA(DisplayName = "Vehicle to Barrier"),
	VehicleToTraffic	UMETA(DisplayName = "Vehicle to Traffic"),
	VehicleToObject		UMETA(DisplayName = "Vehicle to Object"),
	VehicleToRail		UMETA(DisplayName = "Vehicle to Rail"),
	Sideswipe			UMETA(DisplayName = "Sideswipe"),
	TBone				UMETA(DisplayName = "T-Bone"),
	HeadOn				UMETA(DisplayName = "Head On"),
	RearEnd				UMETA(DisplayName = "Rear End"),
	Rollover			UMETA(DisplayName = "Rollover"),
	Airborne			UMETA(DisplayName = "Airborne Landing")
};

/**
 * Collision severity
 */
UENUM(BlueprintType)
enum class EMGCollisionSeverity : uint8
{
	Glancing			UMETA(DisplayName = "Glancing"),
	Minor				UMETA(DisplayName = "Minor"),
	Moderate			UMETA(DisplayName = "Moderate"),
	Major				UMETA(DisplayName = "Major"),
	Severe				UMETA(DisplayName = "Severe"),
	Catastrophic		UMETA(DisplayName = "Catastrophic")
};

/**
 * Impact zone
 */
UENUM(BlueprintType)
enum class EMGImpactZone : uint8
{
	FrontCenter			UMETA(DisplayName = "Front Center"),
	FrontLeft			UMETA(DisplayName = "Front Left"),
	FrontRight			UMETA(DisplayName = "Front Right"),
	SideLeft			UMETA(DisplayName = "Side Left"),
	SideRight			UMETA(DisplayName = "Side Right"),
	RearCenter			UMETA(DisplayName = "Rear Center"),
	RearLeft			UMETA(DisplayName = "Rear Left"),
	RearRight			UMETA(DisplayName = "Rear Right"),
	Roof				UMETA(DisplayName = "Roof"),
	Undercarriage		UMETA(DisplayName = "Undercarriage")
};

/**
 * Vehicle damage state
 */
UENUM(BlueprintType)
enum class EMGDamageState : uint8
{
	Pristine			UMETA(DisplayName = "Pristine"),
	Scratched			UMETA(DisplayName = "Scratched"),
	Dented				UMETA(DisplayName = "Dented"),
	Damaged				UMETA(DisplayName = "Damaged"),
	HeavyDamage			UMETA(DisplayName = "Heavy Damage"),
	Critical			UMETA(DisplayName = "Critical"),
	Wrecked				UMETA(DisplayName = "Wrecked")
};

/**
 * Collision event data
 */
USTRUCT(BlueprintType)
struct FMGCollisionEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CollisionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OtherEntityId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionType Type = EMGCollisionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionSeverity Severity = EMGCollisionSeverity::Minor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGImpactZone ImpactZone = EMGImpactZone::FrontCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactNormal = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageDealt = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLoss = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator SpinImpulse = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCausedWreck = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasTakedown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsAwarded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Vehicle damage zone
 */
USTRUCT(BlueprintType)
struct FMGDamageZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGImpactZone Zone = EMGImpactZone::FrontCenter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageState State = EMGDamageState::Pristine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsArmored = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmorRating = 0.0f;
};

/**
 * Vehicle collision state
 */
USTRUCT(BlueprintType)
struct FMGVehicleCollisionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDamageState OverallState = EMGDamageState::Pristine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGImpactZone, FMGDamageZone> DamageZones;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuspensionHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransmissionHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<bool> TireStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWrecked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnFire = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDamageTaken = 0.0f;
};

/**
 * Collision physics config
 */
USTRUCT(BlueprintType)
struct FMGCollisionPhysicsConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MassInfluence = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedInfluence = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleInfluence = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestitutionCoefficient = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrictionCoefficient = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedForDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamagePerMPH = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpinImpulseMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BounceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLossPercentMin = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLossPercentMax = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InvincibilityAfterCollision = 0.5f;
};

/**
 * Crash effect definition
 */
USTRUCT(BlueprintType)
struct FMGCrashEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionSeverity MinSeverity = EMGCollisionSeverity::Minor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> ParticleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> SoundEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraShakeIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowMotionScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTriggerRumble = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RumbleIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RumbleDuration = 0.3f;
};

/**
 * Player collision stats
 */
USTRUCT(BlueprintType)
struct FMGCollisionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCollisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionType, int32> CollisionsByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionSeverity, int32> CollisionsBySeverity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownsDealt = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownsReceived = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WrecksTotal = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDamageDealt = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDamageReceived = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestImpactSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestImpactForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRacesWithoutCollision = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AggressivePointsEarned = 0;
};

/**
 * Takedown event
 */
USTRUCT(BlueprintType)
struct FMGTakedownEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TakedownId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AttackerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VictimId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCollisionType CollisionType = EMGCollisionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsAwarded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasRevenge = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasDuringDrift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Scoring config for collisions
 */
USTRUCT(BlueprintType)
struct FMGCollisionScoringConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePointsPerTakedown = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RevengeBonus = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftTakedownBonus = 300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirborneTakedownBonus = 400;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainMultiplierPerTakedown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxChainMultiplier = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainWindowSeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionType, int32> TypeBonusPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGCollisionSeverity, float> SeverityMultipliers;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollisionOccurred, const FString&, PlayerId, const FMGCollisionEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, const FString&, VehicleId, float, DamageAmount, EMGImpactZone, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageStateChanged, const FString&, VehicleId, EMGDamageState, OldState, EMGDamageState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleWrecked, const FString&, VehicleId, const FMGCollisionEvent&, FinalCollision);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownDealt, const FString&, AttackerId, const FMGTakedownEvent&, Takedown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownReceived, const FString&, VictimId, const FMGTakedownEvent&, Takedown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTakedownChain, const FString&, PlayerId, int32, ChainCount, float, ChainMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRevengeComplete, const FString&, AttackerId, const FString&, OriginalAttackerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVehicleRepaired, const FString&, VehicleId, float, RepairAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInvincibilityStart, const FString&, VehicleId, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvincibilityEnd, const FString&, VehicleId);

/**
 * Collision Subsystem
 * Manages vehicle collisions, damage, takedowns, and crash effects
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCollisionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnCollisionOccurred OnCollisionOccurred;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnDamageStateChanged OnDamageStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnVehicleWrecked OnVehicleWrecked;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnTakedownDealt OnTakedownDealt;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnTakedownReceived OnTakedownReceived;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnTakedownChain OnTakedownChain;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnRevengeComplete OnRevengeComplete;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnVehicleRepaired OnVehicleRepaired;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnInvincibilityStart OnInvincibilityStart;

	UPROPERTY(BlueprintAssignable, Category = "Collision|Events")
	FOnInvincibilityEnd OnInvincibilityEnd;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "Collision|Vehicle")
	void RegisterVehicle(const FString& VehicleId, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Collision|Vehicle")
	void UnregisterVehicle(const FString& VehicleId);

	UFUNCTION(BlueprintPure, Category = "Collision|Vehicle")
	FMGVehicleCollisionState GetVehicleState(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Vehicle")
	bool IsVehicleRegistered(const FString& VehicleId) const;

	// Collision Processing
	UFUNCTION(BlueprintCallable, Category = "Collision|Processing")
	FMGCollisionEvent ProcessCollision(const FString& PlayerId, const FString& OtherEntityId, FVector ImpactLocation, FVector ImpactNormal, FVector ImpactVelocity, EMGCollisionType Type);

	UFUNCTION(BlueprintCallable, Category = "Collision|Processing")
	void ProcessVehicleToVehicle(const FString& VehicleA, const FString& VehicleB, FVector ImpactLocation, FVector RelativeVelocity);

	UFUNCTION(BlueprintCallable, Category = "Collision|Processing")
	void ProcessVehicleToStatic(const FString& VehicleId, FVector ImpactLocation, FVector ImpactNormal, FVector Velocity);

	// Collision Detection
	UFUNCTION(BlueprintPure, Category = "Collision|Detection")
	EMGCollisionType DetectCollisionType(FVector ImpactNormal, FVector VehicleForward, FVector RelativeVelocity) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Detection")
	EMGCollisionSeverity CalculateSeverity(float ImpactSpeed, float ImpactForce) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Detection")
	EMGImpactZone DetermineImpactZone(FVector LocalImpactPoint) const;

	// Damage System
	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	float ApplyDamage(const FString& VehicleId, float DamageAmount, EMGImpactZone Zone);

	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	void RepairVehicle(const FString& VehicleId, float RepairAmount);

	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	void RepairZone(const FString& VehicleId, EMGImpactZone Zone, float RepairAmount);

	UFUNCTION(BlueprintCallable, Category = "Collision|Damage")
	void FullRepair(const FString& VehicleId);

	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	float GetTotalHealth(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	float GetZoneHealth(const FString& VehicleId, EMGImpactZone Zone) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	EMGDamageState GetDamageState(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Damage")
	bool IsWrecked(const FString& VehicleId) const;

	// Takedowns
	UFUNCTION(BlueprintCallable, Category = "Collision|Takedown")
	FMGTakedownEvent RegisterTakedown(const FString& AttackerId, const FString& VictimId, const FMGCollisionEvent& Collision);

	UFUNCTION(BlueprintPure, Category = "Collision|Takedown")
	int32 GetTakedownChainCount(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Takedown")
	float GetTakedownChainMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Takedown")
	bool IsRevengeTarget(const FString& PlayerId, const FString& TargetId) const;

	UFUNCTION(BlueprintCallable, Category = "Collision|Takedown")
	void ClearTakedownChain(const FString& PlayerId);

	// Invincibility
	UFUNCTION(BlueprintCallable, Category = "Collision|Invincibility")
	void GrantInvincibility(const FString& VehicleId, float Duration);

	UFUNCTION(BlueprintPure, Category = "Collision|Invincibility")
	bool IsInvincible(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Invincibility")
	float GetRemainingInvincibility(const FString& VehicleId) const;

	// Physics Response
	UFUNCTION(BlueprintPure, Category = "Collision|Physics")
	FVector CalculateBounceVelocity(FVector InVelocity, FVector ImpactNormal, float Restitution) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Physics")
	FRotator CalculateSpinImpulse(const FMGCollisionEvent& Collision) const;

	UFUNCTION(BlueprintPure, Category = "Collision|Physics")
	float CalculateSpeedLoss(const FMGCollisionEvent& Collision) const;

	// Effects
	UFUNCTION(BlueprintCallable, Category = "Collision|Effects")
	void RegisterCrashEffect(const FMGCrashEffect& Effect);

	UFUNCTION(BlueprintPure, Category = "Collision|Effects")
	FMGCrashEffect GetCrashEffect(EMGCollisionSeverity Severity) const;

	UFUNCTION(BlueprintCallable, Category = "Collision|Effects")
	void TriggerCrashEffects(const FMGCollisionEvent& Collision);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Collision|Stats")
	FMGCollisionStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Collision|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Collision|Stats")
	TArray<FMGCollisionEvent> GetRecentCollisions(const FString& PlayerId, int32 MaxCount) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Collision|Config")
	void SetPhysicsConfig(const FMGCollisionPhysicsConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Collision|Config")
	FMGCollisionPhysicsConfig GetPhysicsConfig() const;

	UFUNCTION(BlueprintCallable, Category = "Collision|Config")
	void SetScoringConfig(const FMGCollisionScoringConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Collision|Config")
	FMGCollisionScoringConfig GetScoringConfig() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Collision|Update")
	void UpdateCollisionSystem(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Collision|Persistence")
	void SaveCollisionData();

	UFUNCTION(BlueprintCallable, Category = "Collision|Persistence")
	void LoadCollisionData();

protected:
	void TickCollision(float DeltaTime);
	void UpdateInvincibility(float DeltaTime);
	void UpdateTakedownChains(float DeltaTime);
	void UpdateFireDamage(float DeltaTime);
	EMGDamageState CalculateDamageState(float HealthPercent) const;
	void CheckWreckCondition(const FString& VehicleId, const FMGCollisionEvent& Collision);
	void UpdateVehicleState(const FString& VehicleId);
	void UpdatePlayerStats(const FString& PlayerId, const FMGCollisionEvent& Collision);
	FString GenerateCollisionId() const;
	FString GenerateTakedownId() const;

private:
	UPROPERTY()
	TMap<FString, FMGVehicleCollisionState> VehicleStates;

	UPROPERTY()
	TMap<FString, float> InvincibilityTimers;

	UPROPERTY()
	TMap<FString, int32> TakedownChains;

	UPROPERTY()
	TMap<FString, float> TakedownChainTimers;

	UPROPERTY()
	TMap<FString, FString> RevengeTargets;

	UPROPERTY()
	TMap<FString, FMGCollisionStats> PlayerStats;

	UPROPERTY()
	TMap<FString, TArray<FMGCollisionEvent>> RecentCollisions;

	UPROPERTY()
	TArray<FMGCrashEffect> CrashEffects;

	UPROPERTY()
	FMGCollisionPhysicsConfig PhysicsConfig;

	UPROPERTY()
	FMGCollisionScoringConfig ScoringConfig;

	UPROPERTY()
	int32 CollisionCounter = 0;

	UPROPERTY()
	int32 TakedownCounter = 0;

	FTimerHandle CollisionTickTimer;
};
