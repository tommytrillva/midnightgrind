// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "MGVehicleVFXComponent.generated.h"

class UMGVFXSubsystem;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 * Tire VFX state for each wheel
 */
USTRUCT(BlueprintType)
struct FMGTireVFXState
{
	GENERATED_BODY()

	/** Is tire currently smoking */
	UPROPERTY(BlueprintReadOnly)
	bool bIsSmoking = false;

	/** Is tire currently drifting */
	UPROPERTY(BlueprintReadOnly)
	bool bIsDrifting = false;

	/** Current slip amount (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float SlipAmount = 0.0f;

	/** Surface type tire is on */
	UPROPERTY(BlueprintReadOnly)
	FName SurfaceType = NAME_None;

	/** Tire temperature (affects smoke intensity) */
	UPROPERTY(BlueprintReadOnly)
	float TireTemperature = 0.0f;

	/** Active smoke component */
	UPROPERTY()
	UNiagaraComponent* SmokeComponent = nullptr;

	/** Active skidmark component */
	UPROPERTY()
	UNiagaraComponent* SkidmarkComponent = nullptr;
};

/**
 * Exhaust VFX configuration
 */
USTRUCT(BlueprintType)
struct FMGExhaustConfig
{
	GENERATED_BODY()

	/** Socket name for exhaust position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SocketName = NAME_None;

	/** Exhaust offset from socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Offset = FVector::ZeroVector;

	/** Is this exhaust active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

/**
 * Vehicle damage state for VFX
 */
USTRUCT(BlueprintType)
struct FMGVehicleDamageVFXState
{
	GENERATED_BODY()

	/** Overall damage (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float OverallDamage = 0.0f;

	/** Front damage */
	UPROPERTY(BlueprintReadOnly)
	float FrontDamage = 0.0f;

	/** Rear damage */
	UPROPERTY(BlueprintReadOnly)
	float RearDamage = 0.0f;

	/** Left side damage */
	UPROPERTY(BlueprintReadOnly)
	float LeftDamage = 0.0f;

	/** Right side damage */
	UPROPERTY(BlueprintReadOnly)
	float RightDamage = 0.0f;

	/** Is engine smoking */
	UPROPERTY(BlueprintReadOnly)
	bool bEngineSmoking = false;

	/** Is on fire */
	UPROPERTY(BlueprintReadOnly)
	bool bOnFire = false;
};

/**
 * Vehicle VFX Component
 * Handles all vehicle-specific visual effects
 *
 * Features:
 * - Dynamic tire smoke with heat simulation
 * - Drift trails with color customization
 * - Exhaust flames, backfires, and NOS effects
 * - Collision sparks and damage visualization
 * - Environmental interaction (puddles, dust, debris)
 * - Speed-based wind and heat effects
 */
UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleVFXComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// TIRE VFX
	// ==========================================

	/** Update tire state for a wheel */
	UFUNCTION(BlueprintCallable, Category = "VFX|Tires")
	void UpdateTireState(int32 WheelIndex, float SlipRatio, float SlipAngle, bool bOnGround, FName SurfaceType);

	/** Start tire burnout effect */
	UFUNCTION(BlueprintCallable, Category = "VFX|Tires")
	void StartBurnout(int32 WheelIndex);

	/** Stop tire burnout */
	UFUNCTION(BlueprintCallable, Category = "VFX|Tires")
	void StopBurnout(int32 WheelIndex);

	/** Set drift trail color (crew color) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Tires")
	void SetDriftTrailColor(FLinearColor Color);

	/** Get tire VFX state */
	UFUNCTION(BlueprintPure, Category = "VFX|Tires")
	FMGTireVFXState GetTireState(int32 WheelIndex) const;

	// ==========================================
	// EXHAUST VFX
	// ==========================================

	/** Set exhaust configurations */
	UFUNCTION(BlueprintCallable, Category = "VFX|Exhaust")
	void SetExhaustConfigs(const TArray<FMGExhaustConfig>& Configs);

	/** Trigger exhaust backfire */
	UFUNCTION(BlueprintCallable, Category = "VFX|Exhaust")
	void TriggerBackfire();

	/** Activate NOS effects */
	UFUNCTION(BlueprintCallable, Category = "VFX|Exhaust")
	void ActivateNOS();

	/** Deactivate NOS effects */
	UFUNCTION(BlueprintCallable, Category = "VFX|Exhaust")
	void DeactivateNOS();

	/** Set exhaust intensity based on throttle/RPM */
	UFUNCTION(BlueprintCallable, Category = "VFX|Exhaust")
	void SetExhaustIntensity(float ThrottlePosition, float RPMNormalized);

	// ==========================================
	// DAMAGE VFX
	// ==========================================

	/** Set damage state */
	UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
	void SetDamageState(const FMGVehicleDamageVFXState& DamageState);

	/** Trigger collision impact VFX */
	UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
	void TriggerCollisionImpact(FVector ImpactPoint, FVector ImpactNormal, float ImpactForce);

	/** Start scrape sparks */
	UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
	void StartScrapeSparks(FVector ContactPoint, FVector Direction);

	/** Stop scrape sparks */
	UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
	void StopScrapeSparks();

	/** Spawn debris on impact */
	UFUNCTION(BlueprintCallable, Category = "VFX|Damage")
	void SpawnDebris(FVector Location, FVector Direction, int32 DebrisCount = 5);

	// ==========================================
	// ENVIRONMENT INTERACTION
	// ==========================================

	/** Trigger puddle splash */
	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void TriggerPuddleSplash(FVector Location, float Speed);

	/** Trigger dust cloud */
	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void TriggerDustCloud(FVector Location, float Intensity);

	/** Trigger leaf/debris scatter */
	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void TriggerDebrisScatter(FVector Location, FVector Direction);

	/** Set whether driving through rain */
	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void SetInRain(bool bInRain);

	// ==========================================
	// SPEED EFFECTS
	// ==========================================

	/** Update speed-based effects */
	UFUNCTION(BlueprintCallable, Category = "VFX|Speed")
	void UpdateSpeedEffects(float SpeedKPH, float SpeedNormalized);

	/** Enable/disable speed lines */
	UFUNCTION(BlueprintCallable, Category = "VFX|Speed")
	void SetSpeedLinesEnabled(bool bEnabled);

	/** Enable/disable heat distortion */
	UFUNCTION(BlueprintCallable, Category = "VFX|Speed")
	void SetHeatDistortionEnabled(bool bEnabled);

protected:
	// ==========================================
	// NIAGARA SYSTEMS
	// ==========================================

	/** Tire smoke system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Tires")
	UNiagaraSystem* TireSmokeSystem = nullptr;

	/** Tire skidmark system (ribbon trail) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Tires")
	UNiagaraSystem* SkidmarkSystem = nullptr;

	/** Drift trail system (colored ribbon) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Tires")
	UNiagaraSystem* DriftTrailSystem = nullptr;

	/** Burnout smoke system (thick smoke) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Tires")
	UNiagaraSystem* BurnoutSmokeSystem = nullptr;

	/** Exhaust flame system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Exhaust")
	UNiagaraSystem* ExhaustFlameSystem = nullptr;

	/** Exhaust backfire system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Exhaust")
	UNiagaraSystem* BackfireSystem = nullptr;

	/** NOS flame system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Exhaust")
	UNiagaraSystem* NOSFlameSystem = nullptr;

	/** NOS trail system (blue flame ribbon) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Exhaust")
	UNiagaraSystem* NOSTrailSystem = nullptr;

	/** Collision sparks system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Damage")
	UNiagaraSystem* CollisionSparksSystem = nullptr;

	/** Metal scrape sparks system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Damage")
	UNiagaraSystem* ScrapeSparksSystem = nullptr;

	/** Debris particles system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Damage")
	UNiagaraSystem* DebrisSystem = nullptr;

	/** Engine smoke system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Damage")
	UNiagaraSystem* EngineSmokeSystem = nullptr;

	/** Engine fire system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Damage")
	UNiagaraSystem* EngineFireSystem = nullptr;

	/** Puddle splash system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Environment")
	UNiagaraSystem* PuddleSplashSystem = nullptr;

	/** Dust cloud system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Environment")
	UNiagaraSystem* DustCloudSystem = nullptr;

	/** Debris scatter system (leaves, paper, etc.) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Environment")
	UNiagaraSystem* DebrisScatterSystem = nullptr;

	/** Rain interaction system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Environment")
	UNiagaraSystem* RainInteractionSystem = nullptr;

	/** Speed lines system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Speed")
	UNiagaraSystem* SpeedLinesSystem = nullptr;

	/** Heat distortion system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Speed")
	UNiagaraSystem* HeatDistortionSystem = nullptr;

	/** Wind particles system */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Systems|Speed")
	UNiagaraSystem* WindParticlesSystem = nullptr;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Wheel socket names (FL, FR, RL, RR) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	TArray<FName> WheelSocketNames;

	/** Exhaust configurations */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	TArray<FMGExhaustConfig> ExhaustConfigs;

	/** Engine socket name */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	FName EngineSocketName = FName("Engine");

	/** Speed threshold for speed effects (KPH) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	float SpeedEffectsThreshold = 120.0f;

	/** Slip threshold for tire smoke */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	float TireSmokeSlipThreshold = 0.3f;

	/** Drift trail minimum slip angle (degrees) */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	float DriftTrailMinAngle = 15.0f;

	/** Tire temperature rise rate */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	float TireHeatRate = 2.0f;

	/** Tire temperature cool rate */
	UPROPERTY(EditDefaultsOnly, Category = "VFX|Config")
	float TireCoolRate = 0.5f;

	// ==========================================
	// STATE
	// ==========================================

	/** Per-wheel VFX state */
	UPROPERTY()
	TArray<FMGTireVFXState> TireStates;

	/** Current damage state */
	FMGVehicleDamageVFXState CurrentDamageState;

	/** Drift trail color */
	FLinearColor DriftColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // Default blue

	/** Is NOS active */
	bool bNOSActive = false;

	/** Current speed (cached) */
	float CurrentSpeedKPH = 0.0f;
	float CurrentSpeedNorm = 0.0f;

	/** Is in rain */
	bool bIsInRain = false;

	/** Speed effects enabled */
	bool bSpeedLinesEnabled = true;
	bool bHeatDistortionEnabled = true;

	// ==========================================
	// ACTIVE COMPONENTS
	// ==========================================

	UPROPERTY()
	TArray<UNiagaraComponent*> ExhaustFlameComps;

	UPROPERTY()
	TArray<UNiagaraComponent*> NOSFlameComps;

	UPROPERTY()
	TArray<UNiagaraComponent*> NOSTrailComps;

	UPROPERTY()
	UNiagaraComponent* EngineSmokeComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* EngineFireComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* ScrapeSparksComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* SpeedLinesComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* HeatDistortionComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* WindParticlesComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* RainInteractionComp = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Get VFX subsystem */
	UMGVFXSubsystem* GetVFXSubsystem() const;

	/** Initialize tire VFX state */
	void InitializeTireStates();

	/** Update tire smoke for a wheel */
	void UpdateTireSmoke(int32 WheelIndex, float DeltaTime);

	/** Update exhaust effects */
	void UpdateExhaustEffects(float DeltaTime);

	/** Update damage effects */
	void UpdateDamageEffects(float DeltaTime);

	/** Spawn attached Niagara component */
	UNiagaraComponent* SpawnAttachedNiagara(UNiagaraSystem* System, FName SocketName, FVector Offset = FVector::ZeroVector);

	/** Get wheel world transform */
	bool GetWheelTransform(int32 WheelIndex, FVector& OutLocation, FRotator& OutRotation) const;
};
