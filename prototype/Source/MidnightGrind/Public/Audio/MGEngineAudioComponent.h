// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Curves/CurveFloat.h"
#include "MGEngineAudioComponent.generated.h"

/**
 * Engine sound layer type
 */
UENUM(BlueprintType)
enum class EMGEngineSoundLayer : uint8
{
	/** Idle loop */
	Idle,
	/** Low RPM range */
	Low,
	/** Mid RPM range */
	Mid,
	/** High RPM range */
	High,
	/** Redline/limiter */
	Redline,
	/** Off-throttle deceleration */
	Decel,
	/** Exhaust pops/crackles */
	ExhaustPops,
	/** Turbo whistle/spool */
	Turbo,
	/** Supercharger whine */
	Supercharger,
	/** Transmission whine */
	Transmission
};

/**
 * Engine sound layer configuration
 */
USTRUCT(BlueprintType)
struct FMGEngineSoundLayer
{
	GENERATED_BODY()

	/** Layer type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	EMGEngineSoundLayer LayerType = EMGEngineSoundLayer::Idle;

	/** Sound asset (leave null for procedural) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	USoundBase* Sound = nullptr;

	/** Base volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BaseVolume = 1.0f;

	/** Base pitch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float BasePitch = 1.0f;

	/** RPM range where this layer is active (min) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float RPMMin = 0.0f;

	/** RPM range where this layer is active (max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float RPMMax = 10000.0f;

	/** Fade in range (RPM units below RPMMin) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float FadeInRange = 500.0f;

	/** Fade out range (RPM units above RPMMax) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float FadeOutRange = 500.0f;

	/** Does this layer respond to throttle? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	bool bThrottleResponse = true;

	/** Is this an off-throttle layer? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	bool bOffThrottleOnly = false;
};

/**
 * Engine audio preset for different engine types
 */
USTRUCT(BlueprintType)
struct FMGEngineAudioPreset
{
	GENERATED_BODY()

	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FName PresetName;

	/** Idle RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float IdleRPM = 800.0f;

	/** Redline RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float RedlineRPM = 7000.0f;

	/** Rev limiter RPM */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float LimiterRPM = 7200.0f;

	/** RPM to pitch curve (normalized 0-1 RPM to pitch multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	UCurveFloat* RPMToPitchCurve = nullptr;

	/** RPM to volume curve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	UCurveFloat* RPMToVolumeCurve = nullptr;

	/** Load to volume curve (engine load 0-1 to volume) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	UCurveFloat* LoadToVolumeCurve = nullptr;

	/** Sound layers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	TArray<FMGEngineSoundLayer> Layers;

	/** Has forced induction (turbo/supercharger) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	bool bHasTurbo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	bool bHasSupercharger = false;

	/** Exhaust pop probability on decel (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExhaustPopProbability = 0.3f;

	/** Minimum time between exhaust pops */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	float ExhaustPopCooldown = 0.15f;
};

/**
 * Engine Audio Component
 * Produces realistic engine sounds based on RPM, throttle, and load
 *
 * Features:
 * - Multi-layer sound mixing
 * - RPM-based pitch shifting
 * - Throttle on/off response
 * - Turbo/supercharger sounds
 * - Exhaust pops on deceleration
 * - Procedural synthesis fallback when no assets
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGEngineAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGEngineAudioComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// ENGINE STATE INPUT
	// ==========================================

	/** Set current RPM */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetRPM(float NewRPM);

	/** Set throttle position (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetThrottle(float NewThrottle);

	/** Set engine load (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetLoad(float NewLoad);

	/** Set current gear */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetGear(int32 NewGear);

	/** Notify of gear change (for shift sounds) */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void OnGearChange(int32 FromGear, int32 ToGear);

	/** Set all engine state at once */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetEngineState(float RPM, float Throttle, float Load, int32 Gear);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set engine audio preset */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetPreset(const FMGEngineAudioPreset& NewPreset);

	/** Get current preset */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	FMGEngineAudioPreset GetPreset() const { return Preset; }

	/** Set master volume */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetMasterVolume(float Volume);

	/** Enable/disable engine audio */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetEnabled(bool bEnabled);

	/** Is engine audio enabled? */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	bool IsEnabled() const { return bAudioEnabled; }

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Get current RPM */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	float GetCurrentRPM() const { return CurrentRPM; }

	/** Get current throttle */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	float GetCurrentThrottle() const { return CurrentThrottle; }

	/** Is at redline? */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	bool IsAtRedline() const;

	/** Is on throttle? */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	bool IsOnThrottle() const { return CurrentThrottle > ThrottleOnThreshold; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when hitting rev limiter */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevLimiter);

	UPROPERTY(BlueprintAssignable, Category = "Engine Audio|Events")
	FOnRevLimiter OnRevLimiter;

	/** Called on exhaust pop */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExhaustPop);

	UPROPERTY(BlueprintAssignable, Category = "Engine Audio|Events")
	FOnExhaustPop OnExhaustPop;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Engine audio preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio|Config")
	FMGEngineAudioPreset Preset;

	/** Master volume multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio|Config", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MasterVolume = 1.0f;

	/** Throttle threshold to consider "on throttle" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio|Config")
	float ThrottleOnThreshold = 0.1f;

	/** RPM smoothing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio|Config")
	float RPMSmoothingSpeed = 8.0f;

	/** Use procedural synthesis when no sound assets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio|Config")
	bool bUseProceduralFallback = true;

	// ==========================================
	// STATE
	// ==========================================

	/** Current RPM (smoothed) */
	float CurrentRPM = 0.0f;

	/** Target RPM */
	float TargetRPM = 0.0f;

	/** Current throttle */
	float CurrentThrottle = 0.0f;

	/** Current load */
	float CurrentLoad = 0.0f;

	/** Current gear */
	int32 CurrentGear = 1;

	/** Is audio enabled? */
	bool bAudioEnabled = true;

	/** Time since last exhaust pop */
	float TimeSinceLastPop = 0.0f;

	/** Was on throttle last frame? */
	bool bWasOnThrottle = false;

	/** Was at limiter last frame? */
	bool bWasAtLimiter = false;

	// ==========================================
	// AUDIO COMPONENTS
	// ==========================================

	/** Audio components for each layer */
	UPROPERTY()
	TMap<EMGEngineSoundLayer, UAudioComponent*> LayerComponents;

	/** Procedural audio component (fallback) */
	UPROPERTY()
	UAudioComponent* ProceduralComponent = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize audio components */
	void InitializeAudioComponents();

	/** Cleanup audio components */
	void CleanupAudioComponents();

	/** Update all layer volumes and pitches */
	void UpdateLayers(float DeltaTime);

	/** Calculate volume for a layer */
	float CalculateLayerVolume(const FMGEngineSoundLayer& Layer) const;

	/** Calculate pitch for current RPM */
	float CalculatePitch() const;

	/** Check and trigger exhaust pops */
	void UpdateExhaustPops(float DeltaTime);

	/** Create procedural engine sound */
	void UpdateProceduralSound();

	/** Get normalized RPM (0-1 from idle to redline) */
	float GetNormalizedRPM() const;
};
