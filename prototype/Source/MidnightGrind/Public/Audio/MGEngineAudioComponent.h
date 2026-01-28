// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGEngineAudioComponent.h
 * @brief Realistic engine audio system with multi-layer sound mixing and RPM-based synthesis
 *
 * =============================================================================
 * OVERVIEW
 * =============================================================================
 *
 * The Engine Audio Component produces realistic vehicle engine sounds based on
 * RPM, throttle position, and load. It supports multi-layer sound mixing where
 * different audio samples crossfade based on engine state, creating seamless
 * transitions from idle to redline.
 *
 * =============================================================================
 * @section concepts KEY CONCEPTS FOR BEGINNERS
 * =============================================================================
 *
 * 1. MULTI-LAYER SOUND MIXING:
 *    - Real engine recordings are split into "layers" (Idle, Low, Mid, High, Redline).
 *    - Each layer plays simultaneously but at different volumes.
 *    - As RPM increases, lower layers fade out and higher layers fade in.
 *    - This creates smooth transitions without jarring pitch jumps.
 *
 * 2. RPM-BASED PITCH SHIFTING:
 *    - Each layer's pitch is adjusted based on current RPM.
 *    - RPMToPitchCurve defines how pitch scales with RPM.
 *    - Pitch multipliers keep sounds realistic across the RPM range.
 *
 * 3. THROTTLE RESPONSE:
 *    - bThrottleResponse: Whether a layer responds to throttle input.
 *    - On-throttle: Engine under load, fuller sound.
 *    - Off-throttle (decel): Engine braking sound, exhaust pops.
 *    - bOnThrottle is true when throttle exceeds ThrottleOnThreshold.
 *
 * 4. ENGINE SOUND LAYERS (EMGEngineSoundLayer):
 *    - Idle: Engine at rest, low RPM loop.
 *    - Low/Mid/High: Different RPM ranges of the powerband.
 *    - Redline: At or near rev limiter, aggressive sound.
 *    - Decel: Off-throttle engine braking sound.
 *    - ExhaustPops: Random backfires on deceleration.
 *    - Turbo/Supercharger: Forced induction sounds.
 *    - Transmission: Gear whine at high RPM.
 *
 * 5. ENGINE AUDIO PRESET (FMGEngineAudioPreset):
 *    - Complete configuration for an engine type.
 *    - Defines IdleRPM, RedlineRPM, LimiterRPM.
 *    - Contains all layers and their configurations.
 *    - Includes turbo/supercharger flags.
 *    - ExhaustPopProbability controls decel crackles.
 *
 * 6. NORMALIZED OUTPUT VALUES:
 *    - FMGEngineAudioParams provides 0-1 normalized values for audio middleware.
 *    - RPMNormalized: 0 = idle, 1 = redline.
 *    - LoadNormalized: 0 = coasting, 1 = full throttle under load.
 *    - BoostNormalized: 0 = no boost, 1 = max turbo pressure.
 *    - Useful for Wwise, FMOD, or MetaSounds integration.
 *
 * 7. ENGINE DAMAGE AUDIO:
 *    - SetEngineDamageLevel() affects sound quality.
 *    - Damaged engines misfire and knock.
 *    - IsMisfiring()/IsKnocking() indicate current damage effects.
 *    - OnEngineMisfire delegate fires for visual effects sync.
 *
 * 8. SPECIAL EFFECTS:
 *    - TriggerBackfire(): Exhaust pop sound effect.
 *    - TriggerBlowOffValve(): Turbo dump sound (psshh).
 *    - ShouldPlayBOV()/ShouldPlayBackfire(): Check triggers for external audio.
 *
 * 9. PROCEDURAL FALLBACK:
 *    - If bUseProceduralFallback is true and no assets are assigned,
 *      the system generates basic engine tones procedurally.
 *    - Not as realistic but useful for prototyping.
 *
 * =============================================================================
 * @section usage USAGE EXAMPLE
 * =============================================================================
 *
 * @code
 * // Get the component from your vehicle
 * UMGEngineAudioComponent* EngineAudio = Vehicle->FindComponentByClass<UMGEngineAudioComponent>();
 *
 * // Setup a preset (usually done in editor or data asset)
 * FMGEngineAudioPreset V8Preset;
 * V8Preset.PresetName = FName("V8_Muscle");
 * V8Preset.IdleRPM = 750.0f;
 * V8Preset.RedlineRPM = 6500.0f;
 * V8Preset.LimiterRPM = 6800.0f;
 * V8Preset.ExhaustPopProbability = 0.4f;
 * // Add layers...
 * EngineAudio->SetPreset(V8Preset);
 *
 * // Every tick from your vehicle physics:
 * EngineAudio->SetEngineState(CurrentRPM, ThrottleInput, EngineLoad, CurrentGear);
 *
 * // Or set individual values:
 * EngineAudio->SetRPM(CurrentRPM);
 * EngineAudio->SetThrottle(ThrottleInput);
 * EngineAudio->SetLoad(EngineLoad);
 * EngineAudio->SetGear(CurrentGear);
 *
 * // On gear change (for shift sounds):
 * EngineAudio->OnGearChange(OldGear, NewGear);
 *
 * // For turbo vehicles:
 * EngineAudio->SetBoost(TurboBoostPressure);
 * if (ThrottleClosed && BoostWasHigh)
 * {
 *     EngineAudio->TriggerBlowOffValve();
 * }
 *
 * // Get normalized params for external audio middleware:
 * FMGEngineAudioParams Params = EngineAudio->GetAudioParams();
 * // Send Params to Wwise/FMOD/MetaSounds...
 * @endcode
 *
 * =============================================================================
 * @section integration AUDIO MIDDLEWARE INTEGRATION
 * =============================================================================
 *
 * For professional audio integration with Wwise or FMOD:
 *
 * 1. Use GetAudioParams() to get all normalized values in one struct.
 * 2. Map these to RTPC (Real-Time Parameter Controls):
 *    - RPMNormalized -> "Engine_RPM"
 *    - LoadNormalized -> "Engine_Load"
 *    - BoostNormalized -> "Turbo_Boost"
 *    - ThrottleNormalized -> "Throttle_Position"
 * 3. Use ShouldPlayBOV()/ShouldPlayBackfire() to trigger one-shot events.
 * 4. Subscribe to OnRevLimiter and OnExhaustPop for event-driven sounds.
 *
 * @see UMGVehicleSFXComponent for non-engine vehicle sounds
 * @see FMGEngineAudioPreset for complete engine configuration
 * @see FMGEngineAudioParams for audio middleware output
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Curves/CurveFloat.h"
#include "MGEngineAudioComponent.generated.h"

/**
 * Engine audio parameters for middleware integration.
 * All values are normalized 0-1 for easy use in audio graphs.
 */
USTRUCT(BlueprintType)
struct FMGEngineAudioParams
{
	GENERATED_BODY()

	/** Normalized RPM (0 = idle, 1 = redline) */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	float RPMNormalized = 0.0f;

	/** Normalized engine load (0 = coasting, 1 = full load) */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	float LoadNormalized = 0.0f;

	/** Throttle position (0 = closed, 1 = WOT) */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	float ThrottleNormalized = 0.0f;

	/** Normalized boost level (0 = no boost, 1 = max boost) */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	float BoostNormalized = 0.0f;

	/** Current gear as float for smooth transitions */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	float GearNormalized = 0.0f;

	/** Is engine on throttle (load > threshold) */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	bool bOnThrottle = false;

	/** Is at rev limiter */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	bool bAtLimiter = false;

	/** Is in deceleration (off throttle, high RPM) */
	UPROPERTY(BlueprintReadOnly, Category = "Engine Audio")
	bool bDecel = false;
};

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
	// AUDIO PARAMETER OUTPUT
	// These functions provide normalized values for audio system integration
	// ==========================================

	/**
	 * Get normalized RPM value (0-1 from idle to redline).
	 * Use for pitch/volume control in audio middleware.
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Output")
	float GetNormalizedRPM() const;

	/**
	 * Get normalized engine load (0-1).
	 * Higher load = fuller engine sound.
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Output")
	float GetNormalizedLoad() const { return FMath::Clamp(CurrentLoad, 0.0f, 1.0f); }

	/**
	 * Get normalized boost level (0-1 from no boost to max boost).
	 * Use for turbo whistle/blow-off valve sounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Output")
	float GetNormalizedBoost() const { return FMath::Clamp(CurrentBoost, 0.0f, 1.0f); }

	/**
	 * Set current boost level (0-1 normalized).
	 */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void SetBoost(float NewBoost);

	/**
	 * Get all audio parameters in a single struct for efficient integration.
	 * @return Struct containing RPM, load, throttle, boost, gear as normalized 0-1 values
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Output")
	FMGEngineAudioParams GetAudioParams() const;

	/**
	 * Trigger exhaust backfire effect.
	 * Call on aggressive downshifts, anti-lag pops, etc.
	 */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void TriggerBackfire();

	/**
	 * Trigger blow-off valve sound (turbo dump).
	 * Call when throttle closes at high boost.
	 */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio")
	void TriggerBlowOffValve();

	/**
	 * Should audio system play BOV sound this frame?
	 * Returns true momentarily when conditions are met.
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Output")
	bool ShouldPlayBOV() const { return bBOVTriggered; }

	/**
	 * Should audio system play backfire sound this frame?
	 */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Output")
	bool ShouldPlayBackfire() const { return bBackfireTriggered; }

	// ==========================================
	// ENGINE DAMAGE AUDIO
	// ==========================================

	/**
	 * Set engine damage level for audio effects.
	 * @param DamageLevel 0-1 where 0 = healthy, 1 = severe damage
	 */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio|Damage")
	void SetEngineDamageLevel(float DamageLevel);

	/** Get current engine damage level */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Damage")
	float GetEngineDamageLevel() const { return EngineDamageLevel; }

	/** Is engine misfiring due to damage */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Damage")
	bool IsMisfiring() const { return bIsMisfiring; }

	/** Is engine knocking due to damage */
	UFUNCTION(BlueprintPure, Category = "Engine Audio|Damage")
	bool IsKnocking() const { return bIsKnocking; }

	/** Trigger a random misfire sound */
	UFUNCTION(BlueprintCallable, Category = "Engine Audio|Damage")
	void TriggerMisfire();

	/** Called when engine misfire occurs */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEngineMisfire);

	UPROPERTY(BlueprintAssignable, Category = "Engine Audio|Events")
	FOnEngineMisfire OnEngineMisfire;

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

	/** Current boost level (normalized 0-1) */
	float CurrentBoost = 0.0f;

	/** Previous throttle for BOV detection */
	float PreviousThrottle = 0.0f;

	/** BOV triggered this frame */
	bool bBOVTriggered = false;

	/** Backfire triggered this frame */
	bool bBackfireTriggered = false;

	/** Maximum gear count for normalization */
	int32 MaxGears = 6;

	// Damage audio state
	/** Engine damage level (0-1) */
	float EngineDamageLevel = 0.0f;

	/** Is engine misfiring */
	bool bIsMisfiring = false;

	/** Is engine knocking */
	bool bIsKnocking = false;

	/** Time since last misfire */
	float TimeSinceLastMisfire = 0.0f;

	/** Misfire interval based on damage */
	float MisfireInterval = 1.0f;

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
