// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MGRaceModifiers.generated.h"

class AMGRaceGameMode;
class AMGVehiclePawn;
class AController;

/**
 * Modifier category for UI grouping
 */
UENUM(BlueprintType)
enum class EMGModifierCategory : uint8
{
	/** Affects vehicle physics/handling */
	Physics,
	/** Affects speed/acceleration */
	Speed,
	/** Affects race rules/scoring */
	Rules,
	/** Affects visuals/environment */
	Visual,
	/** Affects difficulty/AI */
	Difficulty,
	/** Fun/party modifiers */
	Party,
	/** Hardcore/challenge modifiers */
	Challenge
};

/**
 * Modifier effect severity
 */
UENUM(BlueprintType)
enum class EMGModifierSeverity : uint8
{
	/** Minor tweak */
	Minor,
	/** Noticeable change */
	Moderate,
	/** Major game changer */
	Major,
	/** Completely transforms gameplay */
	Extreme
};

/**
 * Base race modifier class
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGRaceModifier : public UObject
{
	GENERATED_BODY()

public:
	UMGRaceModifier();

	// ==========================================
	// IDENTIFICATION
	// ==========================================

	/** Unique modifier ID */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FName ModifierID;

	/** Display name */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText Description;

	/** Short description for UI */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	FText ShortDescription;

	/** Icon */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	UTexture2D* Icon;

	/** Category */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	EMGModifierCategory Category = EMGModifierCategory::Rules;

	/** Severity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier")
	EMGModifierSeverity Severity = EMGModifierSeverity::Moderate;

	// ==========================================
	// COMPATIBILITY
	// ==========================================

	/** Modifiers that cannot be used with this one */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	TArray<FName> IncompatibleModifiers;

	/** Required modifiers (must be active for this to work) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	TArray<FName> RequiredModifiers;

	/** Can be used in ranked races */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	bool bAllowedInRanked = false;

	/** Can be used in multiplayer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Compatibility")
	bool bAllowedInMultiplayer = true;

	// ==========================================
	// REWARDS
	// ==========================================

	/** XP multiplier when active */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Rewards")
	float XPMultiplier = 1.0f;

	/** Cash multiplier when active */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifier|Rewards")
	float CashMultiplier = 1.0f;

	// ==========================================
	// LIFECYCLE
	// ==========================================

	/** Called when modifier is activated */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnActivated(AMGRaceGameMode* GameMode);

	/** Called when modifier is deactivated */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnDeactivated(AMGRaceGameMode* GameMode);

	/** Called every tick while active */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnTick(float DeltaTime);

	/** Called when race starts */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnRaceStarted();

	/** Called when race ends */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnRaceEnded();

	/** Called when a vehicle spawns */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnVehicleSpawned(AMGVehiclePawn* Vehicle, AController* Controller);

	/** Called when a lap is completed */
	UFUNCTION(BlueprintNativeEvent, Category = "Modifier")
	void OnLapCompleted(AController* Controller, int32 LapNumber);

	/** Is this modifier compatible with another */
	UFUNCTION(BlueprintPure, Category = "Modifier")
	bool IsCompatibleWith(const UMGRaceModifier* Other) const;

protected:
	/** Cached game mode reference */
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> CachedGameMode;

	/** Is currently active */
	bool bIsActive = false;
};

// ==========================================
// CONCRETE MODIFIERS
// ==========================================

/**
 * No NOS modifier - disables nitrous
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_NoNOS : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_NoNOS();
	virtual void OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller) override;
};

/**
 * Unlimited NOS modifier - infinite nitrous
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_UnlimitedNOS : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_UnlimitedNOS();
	virtual void OnTick_Implementation(float DeltaTime) override;
};

/**
 * Ghost Mode - vehicles pass through each other
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_GhostMode : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_GhostMode();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller) override;
};

/**
 * Catch Up mode - rubber banding for trailing racers
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_CatchUp : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_CatchUp();
	virtual void OnTick_Implementation(float DeltaTime) override;

	/** Maximum speed boost for last place */
	UPROPERTY(EditDefaultsOnly, Category = "Catch Up")
	float MaxSpeedBoost = 1.15f;

	/** Maximum speed reduction for first place */
	UPROPERTY(EditDefaultsOnly, Category = "Catch Up")
	float MaxSpeedReduction = 0.95f;
};

/**
 * Slipstream Boost - increased drafting effect
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_SlipstreamBoost : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_SlipstreamBoost();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;

	/** Slipstream multiplier */
	UPROPERTY(EditDefaultsOnly, Category = "Slipstream")
	float SlipstreamMultiplier = 2.0f;
};

/**
 * One Hit Knockout - single collision eliminates
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_OneHitKO : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_OneHitKO();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;

	/** Handle collision event */
	UFUNCTION()
	void OnVehicleCollision(AMGVehiclePawn* Vehicle, AActor* OtherActor, float ImpactForce);

	/** Minimum impact force to trigger KO */
	UPROPERTY(EditDefaultsOnly, Category = "OneHitKO")
	float MinKOImpactForce = 50.0f;
};

/**
 * Elimination - last place eliminated each lap
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_Elimination : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_Elimination();
	virtual void OnLapCompleted_Implementation(AController* Controller, int32 LapNumber) override;
	virtual void OnTick_Implementation(float DeltaTime) override;

private:
	/** Track eliminated racers */
	TSet<AController*> EliminatedRacers;

	/** Timer for elimination check */
	float EliminationCheckTimer = 0.0f;

	/** Check interval */
	float EliminationCheckInterval = 1.0f;
};

/**
 * Mirror Mode - track is mirrored
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_MirrorMode : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_MirrorMode();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;
};

/**
 * Time Attack - checkpoints add time
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_TimeAttack : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_TimeAttack();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	/** Starting time in seconds */
	UPROPERTY(EditDefaultsOnly, Category = "Time Attack")
	float StartingTime = 60.0f;

	/** Time added per checkpoint */
	UPROPERTY(EditDefaultsOnly, Category = "Time Attack")
	float TimePerCheckpoint = 10.0f;

private:
	/** Current remaining time per racer */
	TMap<AController*, float> RacerTimes;
};

/**
 * Night Vision - dark track with limited visibility
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_NightVision : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_NightVision();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;

	/** Visibility distance in meters */
	UPROPERTY(EditDefaultsOnly, Category = "Night Vision")
	float VisibilityDistance = 50.0f;
};

/**
 * Traffic Mode - adds traffic vehicles to track
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_Traffic : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_Traffic();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnDeactivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	/** Traffic density (vehicles per km of track) */
	UPROPERTY(EditDefaultsOnly, Category = "Traffic")
	float TrafficDensity = 5.0f;

	/** Traffic vehicle classes */
	UPROPERTY(EditDefaultsOnly, Category = "Traffic")
	TArray<TSubclassOf<AActor>> TrafficVehicleClasses;

private:
	/** Spawned traffic vehicles */
	UPROPERTY()
	TArray<AActor*> SpawnedTraffic;
};

/**
 * Drift Only - can only gain position while drifting
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_DriftOnly : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_DriftOnly();
	virtual void OnTick_Implementation(float DeltaTime) override;

	/** Minimum drift angle to count as drifting */
	UPROPERTY(EditDefaultsOnly, Category = "Drift Only")
	float MinDriftAngle = 15.0f;
};

/**
 * Random Events - periodic random events during race
 */
UCLASS()
class MIDNIGHTGRIND_API UMGModifier_RandomEvents : public UMGRaceModifier
{
	GENERATED_BODY()

public:
	UMGModifier_RandomEvents();
	virtual void OnActivated_Implementation(AMGRaceGameMode* GameMode) override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	/** Minimum time between events */
	UPROPERTY(EditDefaultsOnly, Category = "Random Events")
	float MinEventInterval = 15.0f;

	/** Maximum time between events */
	UPROPERTY(EditDefaultsOnly, Category = "Random Events")
	float MaxEventInterval = 45.0f;

private:
	float EventTimer = 0.0f;
	float NextEventTime = 0.0f;

	void TriggerRandomEvent();
};

// ==========================================
// MODIFIER MANAGER
// ==========================================

/**
 * Race Modifier Manager
 * Handles activation and management of race modifiers
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRaceModifierManager : public UObject
{
	GENERATED_BODY()

public:
	UMGRaceModifierManager();

	/** Initialize with game mode */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	void Initialize(AMGRaceGameMode* GameMode);

	/** Activate a modifier by ID */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	bool ActivateModifier(FName ModifierID);

	/** Deactivate a modifier by ID */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	bool DeactivateModifier(FName ModifierID);

	/** Deactivate all modifiers */
	UFUNCTION(BlueprintCallable, Category = "Modifier Manager")
	void DeactivateAllModifiers();

	/** Check if modifier is active */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	bool IsModifierActive(FName ModifierID) const;

	/** Get all active modifiers */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	TArray<UMGRaceModifier*> GetActiveModifiers() const;

	/** Get all available modifiers */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	TArray<TSubclassOf<UMGRaceModifier>> GetAvailableModifiers() const;

	/** Get total XP multiplier from active modifiers */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	float GetTotalXPMultiplier() const;

	/** Get total cash multiplier from active modifiers */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	float GetTotalCashMultiplier() const;

	/** Can activate modifier (compatibility check) */
	UFUNCTION(BlueprintPure, Category = "Modifier Manager")
	bool CanActivateModifier(FName ModifierID) const;

	/** Tick all active modifiers */
	void TickModifiers(float DeltaTime);

	/** Notify race started */
	void NotifyRaceStarted();

	/** Notify race ended */
	void NotifyRaceEnded();

	/** Notify vehicle spawned */
	void NotifyVehicleSpawned(AMGVehiclePawn* Vehicle, AController* Controller);

	/** Notify lap completed */
	void NotifyLapCompleted(AController* Controller, int32 LapNumber);

private:
	/** Game mode reference */
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> GameModeRef;

	/** Active modifiers */
	UPROPERTY()
	TMap<FName, UMGRaceModifier*> ActiveModifiers;

	/** Available modifier classes */
	UPROPERTY()
	TArray<TSubclassOf<UMGRaceModifier>> RegisteredModifiers;

	/** Create modifier instance */
	UMGRaceModifier* CreateModifier(TSubclassOf<UMGRaceModifier> ModifierClass);
};
