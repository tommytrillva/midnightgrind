// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "MGVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UMaterialParameterCollection;

/**
 * VFX quality level
 */
UENUM(BlueprintType)
enum class EMGVFXQuality : uint8
{
	/** Minimal particles, no screen effects */
	Low,
	/** Reduced particle counts */
	Medium,
	/** Full quality */
	High,
	/** Maximum quality with extra details */
	Ultra
};

/**
 * VFX event types for triggering
 */
UENUM(BlueprintType)
enum class EMGVFXEvent : uint8
{
	/** Tire starts smoking */
	TireSmokeStart,
	/** Tire stops smoking */
	TireSmokeEnd,
	/** Drift initiated */
	DriftStart,
	/** Drift ended */
	DriftEnd,
	/** Exhaust backfire */
	ExhaustBackfire,
	/** NOS activated */
	NOSActivate,
	/** NOS deactivated */
	NOSDeactivate,
	/** Collision impact */
	CollisionImpact,
	/** Scrape started */
	ScrapeStart,
	/** Scrape ended */
	ScrapeEnd,
	/** Gear shift */
	GearShift,
	/** Reached top speed */
	TopSpeed,
	/** Near miss with another vehicle */
	NearMiss,
	/** Crossed finish line */
	FinishLine,
	/** New lap */
	NewLap,
	/** Final lap started */
	FinalLap,
	/** Race position changed */
	PositionChange,
	/** Personal best pace */
	OnPBPace
};

/**
 * Pooled VFX instance
 */
USTRUCT()
struct FMGPooledVFX
{
	GENERATED_BODY()

	UPROPERTY()
	UNiagaraComponent* Component = nullptr;

	UPROPERTY()
	float LastUsedTime = 0.0f;

	UPROPERTY()
	bool bInUse = false;
};

/**
 * VFX spawn request
 */
USTRUCT(BlueprintType)
struct FMGVFXSpawnRequest
{
	GENERATED_BODY()

	/** Niagara system to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* System = nullptr;

	/** World location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** World rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	/** Attach to actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* AttachToActor = nullptr;

	/** Attach socket name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachSocketName = NAME_None;

	/** Auto destroy when complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDestroy = true;

	/** Use pooling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsePooling = true;

	/** Priority (higher = more likely to spawn at low quality) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

/**
 * Global VFX parameters for synchronization
 */
USTRUCT(BlueprintType)
struct FMGGlobalVFXParams
{
	GENERATED_BODY()

	/** Race intensity (0-1, affects particle counts, colors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceIntensity = 0.0f;

	/** Player speed normalized (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerSpeedNorm = 0.0f;

	/** Time of day (0-1, affects lighting/colors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeOfDay = 0.0f;

	/** Weather intensity (0-1, rain/fog) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeatherIntensity = 0.0f;

	/** Is player in first place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayerInFirst = false;

	/** Is final lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFinalLap = false;

	/** Crew primary color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CrewColor = FLinearColor::White;
};

/**
 * VFX Subsystem
 * Central management for all visual effects
 *
 * Features:
 * - Niagara system pooling for performance
 * - Quality-based particle scaling
 * - Global parameter synchronization
 * - Event-driven VFX triggering
 * - Material parameter collection updates
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	// ==========================================
	// VFX SPAWNING
	// ==========================================

	/** Spawn a VFX at location */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	UNiagaraComponent* SpawnVFX(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

	/** Spawn a VFX attached to actor */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	UNiagaraComponent* SpawnVFXAttached(UNiagaraSystem* System, AActor* AttachTo, FName SocketName = NAME_None);

	/** Spawn VFX from request struct */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	UNiagaraComponent* SpawnVFXFromRequest(const FMGVFXSpawnRequest& Request);

	/** Return a pooled VFX to the pool */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void ReturnToPool(UNiagaraComponent* Component);

	// ==========================================
	// VFX EVENTS
	// ==========================================

	/** Trigger a VFX event */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void TriggerVFXEvent(EMGVFXEvent Event, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* Context = nullptr);

	/** Register a system for an event type */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void RegisterEventVFX(EMGVFXEvent Event, UNiagaraSystem* System, int32 Priority = 0);

	// ==========================================
	// GLOBAL PARAMETERS
	// ==========================================

	/** Set global VFX parameters */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetGlobalParams(const FMGGlobalVFXParams& Params);

	/** Get current global params */
	UFUNCTION(BlueprintPure, Category = "VFX")
	FMGGlobalVFXParams GetGlobalParams() const { return GlobalParams; }

	/** Set race intensity */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetRaceIntensity(float Intensity);

	/** Set player speed (normalized 0-1) */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetPlayerSpeed(float SpeedNorm);

	/** Set crew color for effects */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetCrewColor(FLinearColor Color);

	// ==========================================
	// QUALITY SETTINGS
	// ==========================================

	/** Set VFX quality level */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetQuality(EMGVFXQuality Quality);

	/** Get current quality */
	UFUNCTION(BlueprintPure, Category = "VFX")
	EMGVFXQuality GetQuality() const { return CurrentQuality; }

	/** Get particle count multiplier for current quality */
	UFUNCTION(BlueprintPure, Category = "VFX")
	float GetParticleCountMultiplier() const;

	/** Should spawn this priority at current quality? */
	UFUNCTION(BlueprintPure, Category = "VFX")
	bool ShouldSpawnAtQuality(int32 Priority) const;

	// ==========================================
	// SCREEN EFFECTS
	// ==========================================

	/** Trigger screen shake */
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void TriggerScreenShake(float Intensity, float Duration, bool bFalloff = true);

	/** Set chromatic aberration intensity */
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void SetChromaticAberration(float Intensity);

	/** Set radial blur (speed lines) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void SetRadialBlur(float Intensity, FVector2D Center = FVector2D(0.5f, 0.5f));

	/** Set vignette intensity */
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void SetVignette(float Intensity);

	/** Flash screen color */
	UFUNCTION(BlueprintCallable, Category = "VFX|Screen")
	void FlashScreen(FLinearColor Color, float Duration, float Intensity = 1.0f);

	// ==========================================
	// EVENTS
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVFXEventTriggered, EMGVFXEvent, Event, FVector, Location, AActor*, Context);

	UPROPERTY(BlueprintAssignable, Category = "VFX|Events")
	FOnVFXEventTriggered OnVFXEventTriggered;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Maximum pooled instances per system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	int32 MaxPooledPerSystem = 10;

	/** Pool cleanup interval */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	float PoolCleanupInterval = 30.0f;

	/** Maximum active VFX at once (quality-dependent) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	int32 MaxActiveVFX = 100;

	/** Material parameter collection for global params */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	UMaterialParameterCollection* GlobalParamCollection = nullptr;

	// ==========================================
	// STATE
	// ==========================================

	/** Current quality level */
	EMGVFXQuality CurrentQuality = EMGVFXQuality::High;

	/** Global VFX parameters */
	FMGGlobalVFXParams GlobalParams;

	/** Pooled VFX by system */
	UPROPERTY()
	TMap<UNiagaraSystem*, TArray<FMGPooledVFX>> VFXPool;

	/** Event to system mappings */
	UPROPERTY()
	TMap<EMGVFXEvent, UNiagaraSystem*> EventVFXMap;

	/** Event priorities */
	TMap<EMGVFXEvent, int32> EventPriorities;

	/** Active VFX count */
	int32 ActiveVFXCount = 0;

	/** Time since last pool cleanup */
	float TimeSinceCleanup = 0.0f;

	/** Screen shake state */
	float CurrentShakeIntensity = 0.0f;
	float ShakeDuration = 0.0f;
	float ShakeTimer = 0.0f;
	bool bShakeFalloff = true;

	/** Screen flash state */
	FLinearColor FlashColor = FLinearColor::Black;
	float FlashDuration = 0.0f;
	float FlashTimer = 0.0f;
	float FlashIntensity = 0.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Get or create pooled component */
	UNiagaraComponent* GetPooledComponent(UNiagaraSystem* System);

	/** Create new pooled component */
	UNiagaraComponent* CreatePooledComponent(UNiagaraSystem* System, TArray<FMGPooledVFX>& Pool);

	/** Cleanup unused pool entries */
	void CleanupPool();

	/** Update material parameter collection */
	void UpdateMaterialParams();

	/** Update screen effects */
	void UpdateScreenEffects(float DeltaTime);

	/** Get max active VFX for quality */
	int32 GetMaxActiveForQuality() const;
};
