// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPowerupSubsystem.generated.h"

/**
 * Power-up type
 */
UENUM(BlueprintType)
enum class EMGPowerupType : uint8
{
	None				UMETA(DisplayName = "None"),
	SpeedBoost			UMETA(DisplayName = "Speed Boost"),
	Shield				UMETA(DisplayName = "Shield"),
	Nitro				UMETA(DisplayName = "Nitro"),
	SlowField			UMETA(DisplayName = "Slow Field"),
	EMPBlast			UMETA(DisplayName = "EMP Blast"),
	Missile				UMETA(DisplayName = "Missile"),
	OilSlick			UMETA(DisplayName = "Oil Slick"),
	SpikeStrip			UMETA(DisplayName = "Spike Strip"),
	Shockwave			UMETA(DisplayName = "Shockwave"),
	Repair				UMETA(DisplayName = "Repair"),
	Invisibility		UMETA(DisplayName = "Invisibility"),
	TimeWarp			UMETA(DisplayName = "Time Warp"),
	GhostMode			UMETA(DisplayName = "Ghost Mode"),
	Magnet				UMETA(DisplayName = "Magnet"),
	RocketBoost			UMETA(DisplayName = "Rocket Boost"),
	JammerDevice		UMETA(DisplayName = "Jammer Device"),
	TurboCharge			UMETA(DisplayName = "Turbo Charge")
};

/**
 * Power-up rarity
 */
UENUM(BlueprintType)
enum class EMGPowerupRarity : uint8
{
	Common				UMETA(DisplayName = "Common"),
	Uncommon			UMETA(DisplayName = "Uncommon"),
	Rare				UMETA(DisplayName = "Rare"),
	Epic				UMETA(DisplayName = "Epic"),
	Legendary			UMETA(DisplayName = "Legendary")
};

/**
 * Power-up state
 */
UENUM(BlueprintType)
enum class EMGPowerupState : uint8
{
	Inactive			UMETA(DisplayName = "Inactive"),
	Ready				UMETA(DisplayName = "Ready"),
	Active				UMETA(DisplayName = "Active"),
	Cooldown			UMETA(DisplayName = "Cooldown"),
	Depleted			UMETA(DisplayName = "Depleted")
};

/**
 * Power-up target type
 */
UENUM(BlueprintType)
enum class EMGPowerupTarget : uint8
{
	Self				UMETA(DisplayName = "Self"),
	SingleEnemy			UMETA(DisplayName = "Single Enemy"),
	AllEnemies			UMETA(DisplayName = "All Enemies"),
	AreaOfEffect		UMETA(DisplayName = "Area of Effect"),
	Forward				UMETA(DisplayName = "Forward"),
	Backward			UMETA(DisplayName = "Backward"),
	Homing				UMETA(DisplayName = "Homing"),
	Global				UMETA(DisplayName = "Global")
};

/**
 * Pickup spawn type
 */
UENUM(BlueprintType)
enum class EMGPickupSpawnType : uint8
{
	Fixed				UMETA(DisplayName = "Fixed"),
	Random				UMETA(DisplayName = "Random"),
	PositionBased		UMETA(DisplayName = "Position Based"),
	TimeBased			UMETA(DisplayName = "Time Based"),
	EventTriggered		UMETA(DisplayName = "Event Triggered")
};

/**
 * Power-up definition
 */
USTRUCT(BlueprintType)
struct FMGPowerupDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PowerupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType Type = EMGPowerupType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupRarity Rarity = EMGPowerupRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupTarget TargetType = EMGPowerupTarget::Self;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCharges = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMagnitude = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanStack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeBlocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> IconAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> SFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PowerupColor = FLinearColor::White;
};

/**
 * Active power-up instance
 */
USTRUCT(BlueprintType)
struct FMGActivePowerup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PowerupId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType Type = EMGPowerupType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupState State = EMGPowerupState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCharges = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStacks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourcePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ActivationTime;
};

/**
 * Pickup spawn point
 */
USTRUCT(BlueprintType)
struct FMGPickupSpawnPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpawnPointId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPickupSpawnType SpawnType = EMGPickupSpawnType::Fixed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPowerupType> AllowedPowerups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupRarity, float> RarityWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilRespawn = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType CurrentPowerup = EMGPowerupType::None;
};

/**
 * Power-up inventory slot
 */
USTRUCT(BlueprintType)
struct FMGPowerupSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGActivePowerup Powerup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlotCooldown = 0.0f;
};

/**
 * Player power-up inventory
 */
USTRUCT(BlueprintType)
struct FMGPowerupInventory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPowerupSlot> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSlots = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGActivePowerup> ActiveEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasShield = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldTimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPowerupsCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPowerupsUsed = 0;
};

/**
 * Projectile instance
 */
USTRUCT(BlueprintType)
struct FMGPowerupProjectile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProjectileId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType PowerupType = EMGPowerupType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourcePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifetimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHoming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HomingStrength = 0.0f;
};

/**
 * Dropped hazard
 */
USTRUCT(BlueprintType)
struct FMGDroppedHazard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HazardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPowerupType SourcePowerup = EMGPowerupType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourcePlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifetimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMagnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsOwner = false;
};

/**
 * Power-up usage stats
 */
USTRUCT(BlueprintType)
struct FMGPowerupStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> PowerupsCollected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> PowerupsUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> HitsDealt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> HitsReceived;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPowerupType, int32> HitsBlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalProjectilesLaunched = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalProjectilesHit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileAccuracy = 0.0f;
};

/**
 * Power-up balance config
 */
USTRUCT(BlueprintType)
struct FMGPowerupBalanceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRubberBanding = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeaderPowerupNerf = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastPlacePowerupBuff = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<int32, float> PositionRarityBoost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPowerupType> LeaderRestrictedPowerups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPowerupType> LastPlaceGuaranteedPowerups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalCooldownMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalDurationMultiplier = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPowerupCollected, const FString&, PlayerId, EMGPowerupType, PowerupType, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPowerupActivated, const FString&, PlayerId, const FMGActivePowerup&, Powerup, const FString&, TargetId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerupExpired, const FString&, PlayerId, EMGPowerupType, PowerupType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPowerupHit, const FString&, SourceId, const FString&, TargetId, EMGPowerupType, PowerupType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPowerupBlocked, const FString&, TargetId, EMGPowerupType, PowerupType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShieldActivated, const FString&, PlayerId, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldDepleted, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPickupSpawned, const FString&, SpawnPointId, EMGPowerupType, PowerupType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileLaunched, const FString&, ProjectileId, const FMGPowerupProjectile&, Projectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardDropped, const FMGDroppedHazard&, Hazard);

/**
 * Power-up Subsystem
 * Manages item pickups, power-ups, combat items, and offensive/defensive mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPowerupSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupCollected OnPowerupCollected;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupActivated OnPowerupActivated;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupExpired OnPowerupExpired;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupHit OnPowerupHit;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPowerupBlocked OnPowerupBlocked;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnShieldActivated OnShieldActivated;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnShieldDepleted OnShieldDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnPickupSpawned OnPickupSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnProjectileLaunched OnProjectileLaunched;

	UPROPERTY(BlueprintAssignable, Category = "Powerup|Events")
	FOnHazardDropped OnHazardDropped;

	// Power-up Definitions
	UFUNCTION(BlueprintCallable, Category = "Powerup|Definition")
	void RegisterPowerupDefinition(const FMGPowerupDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "Powerup|Definition")
	FMGPowerupDefinition GetPowerupDefinition(EMGPowerupType Type) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Definition")
	TArray<FMGPowerupDefinition> GetAllPowerupDefinitions() const;

	// Spawn Point Management
	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void RegisterSpawnPoint(const FMGPickupSpawnPoint& SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void UnregisterSpawnPoint(const FString& SpawnPointId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void ActivateAllSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void DeactivateAllSpawnPoints();

	UFUNCTION(BlueprintCallable, Category = "Powerup|SpawnPoints")
	void RespawnPickup(const FString& SpawnPointId);

	UFUNCTION(BlueprintPure, Category = "Powerup|SpawnPoints")
	TArray<FMGPickupSpawnPoint> GetActiveSpawnPoints() const;

	// Collection
	UFUNCTION(BlueprintCallable, Category = "Powerup|Collection")
	bool TryCollectPickup(const FString& PlayerId, const FString& SpawnPointId, int32 RacePosition);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Collection")
	bool GrantPowerup(const FString& PlayerId, EMGPowerupType Type, int32 SlotIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Collection")
	EMGPowerupType RollPowerup(int32 RacePosition, int32 TotalRacers, const TArray<EMGPowerupType>& AllowedTypes);

	// Inventory Management
	UFUNCTION(BlueprintCallable, Category = "Powerup|Inventory")
	void InitializePlayerInventory(const FString& PlayerId, int32 MaxSlots = 2);

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	FMGPowerupInventory GetPlayerInventory(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	bool HasPowerup(const FString& PlayerId, EMGPowerupType Type) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	FMGPowerupSlot GetSlot(const FString& PlayerId, int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Inventory")
	int32 GetEmptySlot(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Inventory")
	void SwapSlots(const FString& PlayerId, int32 SlotA, int32 SlotB);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Inventory")
	void DiscardSlot(const FString& PlayerId, int32 SlotIndex);

	// Activation
	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	bool UsePowerup(const FString& PlayerId, int32 SlotIndex, const FString& TargetId = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	bool ActivatePowerupEffect(const FString& PlayerId, EMGPowerupType Type, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	void DeactivatePowerup(const FString& PlayerId, const FString& InstanceId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Activation")
	void DeactivateAllPowerups(const FString& PlayerId);

	// Shield
	UFUNCTION(BlueprintCallable, Category = "Powerup|Shield")
	void ActivateShield(const FString& PlayerId, float Duration);

	UFUNCTION(BlueprintPure, Category = "Powerup|Shield")
	bool HasActiveShield(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Shield")
	float GetShieldTimeRemaining(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Shield")
	bool TryBlockAttack(const FString& TargetId, EMGPowerupType AttackType);

	// Projectiles
	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	FString LaunchProjectile(const FMGPowerupProjectile& Projectile);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	void UpdateProjectiles(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Powerup|Projectiles")
	TArray<FMGPowerupProjectile> GetActiveProjectiles() const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	void DestroyProjectile(const FString& ProjectileId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Projectiles")
	bool CheckProjectileHit(const FString& ProjectileId, const FString& TargetId, FVector TargetLocation);

	// Hazards
	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	FString DropHazard(const FMGDroppedHazard& Hazard);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	void UpdateHazards(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Powerup|Hazards")
	TArray<FMGDroppedHazard> GetActiveHazards() const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	void RemoveHazard(const FString& HazardId);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Hazards")
	bool CheckHazardCollision(const FString& PlayerId, FVector PlayerLocation);

	// Effects Query
	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	bool HasActiveEffect(const FString& PlayerId, EMGPowerupType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	float GetEffectTimeRemaining(const FString& PlayerId, EMGPowerupType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	float GetEffectMultiplier(const FString& PlayerId, EMGPowerupType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Effects")
	TArray<FMGActivePowerup> GetAllActiveEffects(const FString& PlayerId) const;

	// Balance
	UFUNCTION(BlueprintCallable, Category = "Powerup|Balance")
	void SetBalanceConfig(const FMGPowerupBalanceConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Powerup|Balance")
	FMGPowerupBalanceConfig GetBalanceConfig() const;

	UFUNCTION(BlueprintPure, Category = "Powerup|Balance")
	float GetPositionMultiplier(int32 Position, int32 TotalRacers) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Powerup|Stats")
	FMGPowerupStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Powerup|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Update
	UFUNCTION(BlueprintCallable, Category = "Powerup|Update")
	void UpdatePowerups(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Powerup|Update")
	void UpdateSpawnPoints(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Powerup|Persistence")
	void SavePowerupData();

	UFUNCTION(BlueprintCallable, Category = "Powerup|Persistence")
	void LoadPowerupData();

protected:
	void TickPowerups(float DeltaTime);
	void ProcessPowerupExpiration(const FString& PlayerId, FMGActivePowerup& Powerup);
	void ApplyPowerupEffect(const FString& PlayerId, const FMGPowerupDefinition& Definition);
	void RemovePowerupEffect(const FString& PlayerId, EMGPowerupType Type);
	FString GenerateInstanceId() const;

private:
	UPROPERTY()
	TMap<EMGPowerupType, FMGPowerupDefinition> PowerupDefinitions;

	UPROPERTY()
	TMap<FString, FMGPickupSpawnPoint> SpawnPoints;

	UPROPERTY()
	TMap<FString, FMGPowerupInventory> PlayerInventories;

	UPROPERTY()
	TMap<FString, FMGPowerupStats> PlayerStats;

	UPROPERTY()
	TArray<FMGPowerupProjectile> ActiveProjectiles;

	UPROPERTY()
	TArray<FMGDroppedHazard> ActiveHazards;

	UPROPERTY()
	FMGPowerupBalanceConfig BalanceConfig;

	UPROPERTY()
	int32 InstanceCounter = 0;

	FTimerHandle PowerupTickTimer;
};
