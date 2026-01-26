// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDestructionSubsystem.generated.h"

/**
 * Destructible type
 */
UENUM(BlueprintType)
enum class EMGDestructibleType : uint8
{
	None				UMETA(DisplayName = "None"),
	Prop				UMETA(DisplayName = "Prop"),
	StreetFurniture		UMETA(DisplayName = "Street Furniture"),
	Fence				UMETA(DisplayName = "Fence"),
	Barrier				UMETA(DisplayName = "Barrier"),
	Sign				UMETA(DisplayName = "Sign"),
	Billboard			UMETA(DisplayName = "Billboard"),
	Vehicle				UMETA(DisplayName = "Vehicle"),
	Crate				UMETA(DisplayName = "Crate"),
	Container			UMETA(DisplayName = "Container"),
	Vegetation			UMETA(DisplayName = "Vegetation"),
	Building			UMETA(DisplayName = "Building Part"),
	UtilityPole			UMETA(DisplayName = "Utility Pole"),
	Hydrant				UMETA(DisplayName = "Fire Hydrant"),
	TrafficLight		UMETA(DisplayName = "Traffic Light"),
	BusStop				UMETA(DisplayName = "Bus Stop"),
	Kiosk				UMETA(DisplayName = "Kiosk"),
	Scaffolding			UMETA(DisplayName = "Scaffolding")
};

/**
 * Destruction category for scoring
 */
UENUM(BlueprintType)
enum class EMGDestructionCategory : uint8
{
	Minor				UMETA(DisplayName = "Minor"),
	Standard			UMETA(DisplayName = "Standard"),
	Major				UMETA(DisplayName = "Major"),
	Spectacular			UMETA(DisplayName = "Spectacular"),
	Legendary			UMETA(DisplayName = "Legendary")
};

/**
 * Destruction effect type
 */
UENUM(BlueprintType)
enum class EMGDestructionEffect : uint8
{
	None				UMETA(DisplayName = "None"),
	Shatter				UMETA(DisplayName = "Shatter"),
	Explode				UMETA(DisplayName = "Explode"),
	Crumble				UMETA(DisplayName = "Crumble"),
	Collapse			UMETA(DisplayName = "Collapse"),
	Deform				UMETA(DisplayName = "Deform"),
	Burn				UMETA(DisplayName = "Burn"),
	Splinter			UMETA(DisplayName = "Splinter"),
	Spray				UMETA(DisplayName = "Water Spray")
};

/**
 * Destructible definition
 */
USTRUCT(BlueprintType)
struct FMGDestructibleDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestructibleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructibleType Type = EMGDestructibleType::Prop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructionCategory Category = EMGDestructionCategory::Minor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructionEffect DestructionEffect = EMGDestructionEffect::Shatter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mass = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinImpactSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowdownFactor = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageToVehicle = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChainReact = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainReactRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ChainReactTriggers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlocksRespawn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> DestroyedMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> DestructionVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> DestructionSFX;
};

/**
 * Destructible instance
 */
USTRUCT(BlueprintType)
struct FMGDestructibleInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestructibleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRespawning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestroyedByPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime DestructionTime;
};

/**
 * Destruction event
 */
USTRUCT(BlueprintType)
struct FMGDestructionEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DestructibleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructibleType Type = EMGDestructibleType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDestructionCategory Category = EMGDestructionCategory::Minor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ImpactSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasChainReaction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Destruction combo
 */
USTRUCT(BlueprintType)
struct FMGDestructionCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGDestructionEvent> ComboEvents;
};

/**
 * Player destruction stats
 */
USTRUCT(BlueprintType)
struct FMGDestructionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDestructibleType, int32> TypeCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDestructionCategory, int32> CategoryCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDestroyed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LongestChainReaction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalPropertyDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectacularDestructions = 0;
};

/**
 * Destruction zone
 */
USTRUCT(BlueprintType)
struct FMGDestructionZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ZoneId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ZoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> DestructibleInstances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDestructibles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DestroyedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionBonus = 0;
};

/**
 * Chain reaction
 */
USTRUCT(BlueprintType)
struct FMGChainReaction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChainId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InitiatorPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AffectedInstances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChainLength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;
};

/**
 * Destruction scoring config
 */
USTRUCT(BlueprintType)
struct FMGDestructionScoringConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboWindowSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplierPerHit = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxComboMultiplier = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainReactionMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectacularThreshold = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectacularBonus = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGDestructionCategory, int32> CategoryBasePoints;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructibleDestroyed, const FString&, PlayerId, const FMGDestructionEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDestructionComboUpdated, const FString&, PlayerId, int32, ComboCount, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionComboEnded, const FString&, PlayerId, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChainReactionStarted, const FString&, PlayerId, const FString&, ChainId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChainReactionExtended, const FString&, ChainId, int32, ChainLength, int32, NewPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChainReactionEnded, const FString&, ChainId, int32, FinalLength, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionZoneProgress, const FString&, ZoneId, float, CompletionPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionZoneCompleted, const FString&, ZoneId, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpectacularDestruction, const FString&, PlayerId, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructibleRespawned, const FString&, InstanceId, FVector, Location);

/**
 * Destruction Subsystem
 * Manages environmental destruction, scoring, and chain reactions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDestructionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructibleDestroyed OnDestructibleDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionComboUpdated OnDestructionComboUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionComboEnded OnDestructionComboEnded;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnChainReactionStarted OnChainReactionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnChainReactionExtended OnChainReactionExtended;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnChainReactionEnded OnChainReactionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionZoneProgress OnDestructionZoneProgress;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructionZoneCompleted OnDestructionZoneCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnSpectacularDestruction OnSpectacularDestruction;

	UPROPERTY(BlueprintAssignable, Category = "Destruction|Events")
	FOnDestructibleRespawned OnDestructibleRespawned;

	// Definition Registration
	UFUNCTION(BlueprintCallable, Category = "Destruction|Definition")
	void RegisterDestructibleDefinition(const FMGDestructibleDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "Destruction|Definition")
	FMGDestructibleDefinition GetDestructibleDefinition(const FString& DestructibleId) const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Definition")
	TArray<FMGDestructibleDefinition> GetAllDefinitions() const;

	// Instance Management
	UFUNCTION(BlueprintCallable, Category = "Destruction|Instance")
	FString SpawnDestructible(const FString& DestructibleId, FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Instance")
	void RemoveDestructible(const FString& InstanceId);

	UFUNCTION(BlueprintPure, Category = "Destruction|Instance")
	FMGDestructibleInstance GetDestructibleInstance(const FString& InstanceId) const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Instance")
	TArray<FMGDestructibleInstance> GetDestructiblesInRadius(FVector Center, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Instance")
	TArray<FMGDestructibleInstance> GetDestroyedInstances() const;

	// Destruction
	UFUNCTION(BlueprintCallable, Category = "Destruction|Destruction")
	FMGDestructionEvent DestroyDestructible(const FString& InstanceId, const FString& PlayerId, FVector ImpactVelocity);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Destruction")
	bool DamageDestructible(const FString& InstanceId, float Damage, const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Destruction")
	bool TryDestroyOnImpact(const FString& InstanceId, const FString& PlayerId, FVector ImpactVelocity, float ImpactForce);

	UFUNCTION(BlueprintPure, Category = "Destruction|Destruction")
	bool CanBeDestroyed(const FString& InstanceId, float ImpactSpeed) const;

	// Combo System
	UFUNCTION(BlueprintPure, Category = "Destruction|Combo")
	FMGDestructionCombo GetCurrentCombo(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Combo")
	bool HasActiveCombo(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Destruction|Combo")
	void ExtendCombo(const FString& PlayerId, const FMGDestructionEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Combo")
	void EndCombo(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Combo")
	void ResetCombo(const FString& PlayerId);

	// Chain Reactions
	UFUNCTION(BlueprintCallable, Category = "Destruction|Chain")
	FString StartChainReaction(const FString& PlayerId, const FString& InitialInstanceId);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Chain")
	void ProcessChainReaction(const FString& ChainId);

	UFUNCTION(BlueprintPure, Category = "Destruction|Chain")
	FMGChainReaction GetChainReaction(const FString& ChainId) const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Chain")
	TArray<FString> GetChainableInstances(FVector Origin, float Radius, const TArray<FString>& ExcludeIds) const;

	// Zones
	UFUNCTION(BlueprintCallable, Category = "Destruction|Zone")
	void RegisterDestructionZone(const FMGDestructionZone& Zone);

	UFUNCTION(BlueprintPure, Category = "Destruction|Zone")
	FMGDestructionZone GetDestructionZone(const FString& ZoneId) const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Zone")
	TArray<FMGDestructionZone> GetAllZones() const;

	UFUNCTION(BlueprintCallable, Category = "Destruction|Zone")
	void UpdateZoneProgress(const FString& InstanceId);

	UFUNCTION(BlueprintPure, Category = "Destruction|Zone")
	float GetZoneCompletionPercent(const FString& ZoneId) const;

	// Scoring
	UFUNCTION(BlueprintPure, Category = "Destruction|Scoring")
	int32 CalculateDestructionPoints(const FString& DestructibleId, float ImpactSpeed, float ComboMultiplier) const;

	UFUNCTION(BlueprintCallable, Category = "Destruction|Scoring")
	void SetScoringConfig(const FMGDestructionScoringConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Destruction|Scoring")
	FMGDestructionScoringConfig GetScoringConfig() const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Destruction|Stats")
	FMGDestructionStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Destruction|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Destruction|Stats")
	int32 GetTotalDestroyedCount() const;

	UFUNCTION(BlueprintPure, Category = "Destruction|Stats")
	float GetTotalPropertyDamage() const;

	// Respawn
	UFUNCTION(BlueprintCallable, Category = "Destruction|Respawn")
	void RespawnDestructible(const FString& InstanceId);

	UFUNCTION(BlueprintCallable, Category = "Destruction|Respawn")
	void RespawnAll();

	UFUNCTION(BlueprintCallable, Category = "Destruction|Respawn")
	void RespawnInRadius(FVector Center, float Radius);

	// Update
	UFUNCTION(BlueprintCallable, Category = "Destruction|Update")
	void UpdateDestruction(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Destruction|Persistence")
	void SaveDestructionData();

	UFUNCTION(BlueprintCallable, Category = "Destruction|Persistence")
	void LoadDestructionData();

protected:
	void TickDestruction(float DeltaTime);
	void UpdateCombos(float DeltaTime);
	void UpdateRespawns(float DeltaTime);
	void UpdateChainReactions(float DeltaTime);
	void ProcessChainReactionStep(const FString& ChainId);
	void CheckSpectacularDestruction(const FString& PlayerId, const FMGDestructionEvent& Event);
	FString GenerateInstanceId() const;
	FString GenerateEventId() const;
	FString GenerateChainId() const;

private:
	UPROPERTY()
	TMap<FString, FMGDestructibleDefinition> Definitions;

	UPROPERTY()
	TMap<FString, FMGDestructibleInstance> Instances;

	UPROPERTY()
	TMap<FString, FMGDestructionCombo> ActiveCombos;

	UPROPERTY()
	TMap<FString, FMGChainReaction> ActiveChainReactions;

	UPROPERTY()
	TMap<FString, FMGDestructionZone> Zones;

	UPROPERTY()
	TMap<FString, FMGDestructionStats> PlayerStats;

	UPROPERTY()
	FMGDestructionScoringConfig ScoringConfig;

	UPROPERTY()
	int32 InstanceCounter = 0;

	UPROPERTY()
	int32 EventCounter = 0;

	UPROPERTY()
	int32 ChainCounter = 0;

	UPROPERTY()
	float TotalPropertyDamage = 0.0f;

	FTimerHandle DestructionTickTimer;
};
