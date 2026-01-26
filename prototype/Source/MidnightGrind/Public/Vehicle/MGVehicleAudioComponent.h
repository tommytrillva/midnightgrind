// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGVehicleAudioComponent.generated.h"

class UAudioComponent;
class USoundBase;
class USoundCue;
class AMGVehiclePawn;

/**
 * Engine sound layer configuration
 */
USTRUCT(BlueprintType)
struct FMGEngineSoundLayer
{
	GENERATED_BODY()

	/** Sound for this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* Sound = nullptr;

	/** RPM at which this layer fades in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInRPM = 0.0f;

	/** RPM at which this layer is at full volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FullVolumeRPM = 2000.0f;

	/** RPM at which this layer starts fading out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutRPM = 5000.0f;

	/** RPM at which this layer is fully faded out */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilentRPM = 7000.0f;

	/** Base volume for this layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BaseVolume = 1.0f;

	/** Pitch range (min at FadeInRPM, max at SilentRPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D PitchRange = FVector2D(0.8f, 1.5f);

	/** Apply load (throttle) modulation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bApplyLoadModulation = true;

	/** Load modulation strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LoadModulationStrength = 0.3f;
};

/**
 * Tire sound configuration per surface type
 */
USTRUCT(BlueprintType)
struct FMGTireSoundConfig
{
	GENERATED_BODY()

	/** Skid/screech sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* SkidSound = nullptr;

	/** Rolling sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* RollingSound = nullptr;

	/** Grip coefficient affects volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GripFactor = 1.0f;
};

/**
 * Backfire configuration
 */
USTRUCT(BlueprintType)
struct FMGBackfireConfig
{
	GENERATED_BODY()

	/** Backfire sounds (randomly selected) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USoundBase*> BackfireSounds;

	/** Minimum RPM for backfire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRPM = 4000.0f;

	/** Throttle lift threshold to trigger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleLiftThreshold = 0.3f;

	/** Minimum time between backfires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 0.3f;

	/** Probability per check (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Probability = 0.4f;
};

/**
 * Vehicle audio component - handles all vehicle sounds
 *
 * Features:
 * - Multi-layer engine sound with RPM crossfading
 * - Exhaust/backfire pops
 * - Tire sounds (skid, surface type)
 * - Transmission shift sounds
 * - Turbo/supercharger whine
 * - Wind noise
 * - NOS activation
 * - Collision/scrape sounds
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleAudioComponent();

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Engine sound layers (idle to high RPM) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	TArray<FMGEngineSoundLayer> EngineLayers;

	/** Single engine sound (used if no layers configured) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	USoundBase* SimpleEngineSound;

	/** Exhaust resonance sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	USoundBase* ExhaustSound;

	/** Backfire configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	FMGBackfireConfig BackfireConfig;

	/** Turbo whine sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	USoundBase* TurboSound;

	/** Turbo blow-off valve sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	USoundBase* BlowOffSound;

	/** Supercharger whine sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Engine")
	USoundBase* SuperchargerSound;

	// ==========================================
	// TRANSMISSION SOUNDS
	// ==========================================

	/** Gear shift up sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transmission")
	USoundBase* ShiftUpSound;

	/** Gear shift down sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transmission")
	USoundBase* ShiftDownSound;

	/** Clutch engagement sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transmission")
	USoundBase* ClutchSound;

	/** Gear grind (missed shift) sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transmission")
	USoundBase* GearGrindSound;

	// ==========================================
	// TIRE SOUNDS
	// ==========================================

	/** Default tire sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tires")
	FMGTireSoundConfig DefaultTireSounds;

	/** Tire sounds per physical material name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tires")
	TMap<FName, FMGTireSoundConfig> SurfaceTireSounds;

	/** Tire screech threshold (slip ratio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Tires")
	float SkidThreshold = 0.2f;

	// ==========================================
	// ENVIRONMENTAL
	// ==========================================

	/** Wind noise sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
	USoundBase* WindSound;

	/** Speed at which wind sound reaches full volume (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
	float WindFullSpeedKPH = 200.0f;

	// ==========================================
	// SPECIAL
	// ==========================================

	/** NOS activation sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Special")
	USoundBase* NOSActivateSound;

	/** NOS running loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Special")
	USoundBase* NOSLoopSound;

	/** NOS deactivation sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Special")
	USoundBase* NOSDeactivateSound;

	// ==========================================
	// COLLISION
	// ==========================================

	/** Light collision/bump sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Collision")
	USoundBase* LightImpactSound;

	/** Heavy collision/crash sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Collision")
	USoundBase* HeavyImpactSound;

	/** Scrape/grind against walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Collision")
	USoundBase* ScrapeSound;

	/** Impact velocity threshold for heavy impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Collision")
	float HeavyImpactThreshold = 30.0f;

	// ==========================================
	// CONTROL
	// ==========================================

	/** Initialize with owner vehicle */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void Initialize(AMGVehiclePawn* InVehicle);

	/** Start all audio systems */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StartAudio();

	/** Stop all audio systems */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAudio();

	/** Pause audio (e.g., for pause menu) */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PauseAudio(bool bPause);

	/** Set master volume */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when gear changes */
	UFUNCTION(BlueprintCallable, Category = "Audio|Events")
	void OnGearChanged(int32 OldGear, int32 NewGear);

	/** Called when NOS state changes */
	UFUNCTION(BlueprintCallable, Category = "Audio|Events")
	void OnNOSStateChanged(bool bActive);

	/** Called when vehicle collides */
	UFUNCTION(BlueprintCallable, Category = "Audio|Events")
	void OnCollision(float ImpactVelocity, bool bIsScrape);

	/** Play a one-shot backfire */
	UFUNCTION(BlueprintCallable, Category = "Audio|Events")
	void PlayBackfire();

	/** Play blow-off valve */
	UFUNCTION(BlueprintCallable, Category = "Audio|Events")
	void PlayBlowOff();

protected:
	// ==========================================
	// UPDATE
	// ==========================================

	/** Update engine sounds based on current state */
	void UpdateEngineSounds(float DeltaTime);

	/** Update tire sounds */
	void UpdateTireSounds(float DeltaTime);

	/** Update wind sound */
	void UpdateWindSound(float DeltaTime);

	/** Update turbo/supercharger sounds */
	void UpdateForcedInductionSounds(float DeltaTime);

	/** Check for and play backfires */
	void UpdateBackfires(float DeltaTime);

	// ==========================================
	// HELPERS
	// ==========================================

	/** Get volume for an engine layer at given RPM */
	float GetLayerVolume(const FMGEngineSoundLayer& Layer, float RPM, float Load) const;

	/** Get pitch for an engine layer at given RPM */
	float GetLayerPitch(const FMGEngineSoundLayer& Layer, float RPM) const;

	/** Create or get audio component */
	UAudioComponent* GetOrCreateAudioComponent(FName Name, USoundBase* Sound);

	/** Play one-shot sound at location */
	void PlayOneShotAtLocation(USoundBase* Sound, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

	/** Play one-shot 2D */
	void PlayOneShot2D(USoundBase* Sound, float Volume = 1.0f, float Pitch = 1.0f);

private:
	// ==========================================
	// REFERENCES
	// ==========================================

	/** Owner vehicle */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> OwnerVehicle;

	// ==========================================
	// AUDIO COMPONENTS
	// ==========================================

	/** Engine layer audio components */
	UPROPERTY()
	TArray<UAudioComponent*> EngineLayerComponents;

	/** Simple engine component */
	UPROPERTY()
	UAudioComponent* SimpleEngineComponent;

	/** Exhaust component */
	UPROPERTY()
	UAudioComponent* ExhaustComponent;

	/** Turbo component */
	UPROPERTY()
	UAudioComponent* TurboComponent;

	/** Supercharger component */
	UPROPERTY()
	UAudioComponent* SuperchargerComponent;

	/** Tire skid components (one per wheel) */
	UPROPERTY()
	TArray<UAudioComponent*> TireSkidComponents;

	/** Wind component */
	UPROPERTY()
	UAudioComponent* WindComponent;

	/** NOS loop component */
	UPROPERTY()
	UAudioComponent* NOSComponent;

	/** Scrape component */
	UPROPERTY()
	UAudioComponent* ScrapeComponent;

	// ==========================================
	// STATE
	// ==========================================

	/** Current master volume */
	float MasterVolume = 1.0f;

	/** Is audio active */
	bool bAudioActive = false;

	/** Previous throttle for backfire detection */
	float PreviousThrottle = 0.0f;

	/** Backfire cooldown timer */
	float BackfireCooldown = 0.0f;

	/** Previous gear for shift detection */
	int32 PreviousGear = 1;

	/** Is NOS currently active */
	bool bNOSActive = false;

	/** Current scrape state */
	bool bIsScraping = false;

	/** Time since last scrape sound */
	float ScrapeSoundTimer = 0.0f;

	/** Smoothed RPM for audio */
	float SmoothedRPM = 0.0f;

	/** RPM smoothing factor */
	float RPMSmoothingFactor = 10.0f;
};
