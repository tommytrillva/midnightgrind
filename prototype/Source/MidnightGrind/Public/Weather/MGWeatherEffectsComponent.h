// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MGWeatherSubsystem.h"
#include "MGWeatherEffectsComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UAudioComponent;
class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * Windshield effect data
 */
USTRUCT(BlueprintType)
struct FMGWindshieldEffectData
{
	GENERATED_BODY()

	/** Droplet count (rain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropletCount = 0.0f;

	/** Droplet size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropletSize = 1.0f;

	/** Streak amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreakAmount = 0.0f;

	/** Wiper position (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WiperPosition = 0.0f;

	/** Wetness amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WetnessAmount = 0.0f;

	/** Fog/condensation inside */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteriorFog = 0.0f;
};

/**
 * Screen space weather effect data
 */
USTRUCT(BlueprintType)
struct FMGScreenWeatherEffect
{
	GENERATED_BODY()

	/** Vignette intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VignetteIntensity = 0.0f;

	/** Blur amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlurAmount = 0.0f;

	/** Contrast adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ContrastMultiplier = 1.0f;

	/** Saturation adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SaturationMultiplier = 1.0f;

	/** Color tint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ColorTint = FLinearColor::White;

	/** Film grain intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FilmGrainIntensity = 0.0f;
};

/**
 * Weather Effects Component
 * Handles visual and audio effects for weather on vehicles
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGWeatherEffectsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGWeatherEffectsComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Rain particle system */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Rain")
	UNiagaraSystem* RainParticleSystem;

	/** Rain splash particle system */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Rain")
	UNiagaraSystem* RainSplashSystem;

	/** Snow particle system */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Snow")
	UNiagaraSystem* SnowParticleSystem;

	/** Dust particle system */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Dust")
	UNiagaraSystem* DustParticleSystem;

	/** Fog particle system (volumetric fog patches) */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Fog")
	UNiagaraSystem* FogPatchSystem;

	/** Lightning flash material */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Lightning")
	UMaterialInterface* LightningFlashMaterial;

	/** Windshield effect material */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Windshield")
	UMaterialInterface* WindshieldMaterial;

	// ==========================================
	// AUDIO
	// ==========================================

	/** Rain ambient sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* RainAmbientSound;

	/** Heavy rain ambient sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* HeavyRainAmbientSound;

	/** Thunder sounds */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TArray<USoundBase*> ThunderSounds;

	/** Wind ambient sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* WindAmbientSound;

	/** Snow ambient sound */
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundBase* SnowAmbientSound;

	// ==========================================
	// CONTROL
	// ==========================================

	/** Enable/disable weather effects */
	UFUNCTION(BlueprintCallable, Category = "Weather Effects")
	void SetWeatherEffectsEnabled(bool bEnabled);

	/** Are weather effects enabled */
	UFUNCTION(BlueprintPure, Category = "Weather Effects")
	bool AreWeatherEffectsEnabled() const { return bEffectsEnabled; }

	/** Force update effects */
	UFUNCTION(BlueprintCallable, Category = "Weather Effects")
	void ForceUpdateEffects();

	/** Set interior mode (affects windshield effects) */
	UFUNCTION(BlueprintCallable, Category = "Weather Effects")
	void SetInteriorMode(bool bIsInterior);

	/** Is in interior mode */
	UFUNCTION(BlueprintPure, Category = "Weather Effects")
	bool IsInteriorMode() const { return bInteriorMode; }

	// ==========================================
	// WINDSHIELD
	// ==========================================

	/** Get windshield effect data */
	UFUNCTION(BlueprintPure, Category = "Windshield")
	FMGWindshieldEffectData GetWindshieldEffects() const { return WindshieldEffects; }

	/** Activate wipers */
	UFUNCTION(BlueprintCallable, Category = "Windshield")
	void ActivateWipers();

	/** Set wiper mode (0=off, 1=intermittent, 2=normal, 3=fast) */
	UFUNCTION(BlueprintCallable, Category = "Windshield")
	void SetWiperMode(int32 Mode);

	/** Get wiper mode */
	UFUNCTION(BlueprintPure, Category = "Windshield")
	int32 GetWiperMode() const { return WiperMode; }

	// ==========================================
	// SCREEN EFFECTS
	// ==========================================

	/** Get screen weather effects */
	UFUNCTION(BlueprintPure, Category = "Screen Effects")
	FMGScreenWeatherEffect GetScreenEffects() const { return ScreenEffects; }

	/** Set post process component to control */
	UFUNCTION(BlueprintCallable, Category = "Screen Effects")
	void SetPostProcessComponent(UPostProcessComponent* PostProcess);

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Effects enabled */
	bool bEffectsEnabled = true;

	/** Interior mode */
	bool bInteriorMode = false;

	/** Current windshield effects */
	FMGWindshieldEffectData WindshieldEffects;

	/** Current screen effects */
	FMGScreenWeatherEffect ScreenEffects;

	/** Wiper mode */
	int32 WiperMode = 0;

	/** Wiper timer */
	float WiperTimer = 0.0f;

	/** Wiper interval based on mode */
	float WiperInterval = 3.0f;

	// ==========================================
	// CACHED REFERENCES
	// ==========================================

	UPROPERTY()
	UMGWeatherSubsystem* WeatherSubsystem;

	UPROPERTY()
	UNiagaraComponent* RainParticleComponent;

	UPROPERTY()
	UNiagaraComponent* SplashParticleComponent;

	UPROPERTY()
	UNiagaraComponent* SnowParticleComponent;

	UPROPERTY()
	UNiagaraComponent* DustParticleComponent;

	UPROPERTY()
	UAudioComponent* AmbientAudioComponent;

	UPROPERTY()
	UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* WindshieldMID;

	/** Last weather type */
	EMGWeatherType LastWeatherType;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize particle components */
	void InitializeParticleComponents();

	/** Initialize audio components */
	void InitializeAudioComponents();

	/** Update rain effects */
	void UpdateRainEffects(float Intensity);

	/** Update snow effects */
	void UpdateSnowEffects(float Intensity);

	/** Update fog effects */
	void UpdateFogEffects(float Density);

	/** Update dust effects */
	void UpdateDustEffects(float Intensity);

	/** Update windshield effects */
	void UpdateWindshieldEffects(float DeltaTime);

	/** Update screen effects */
	void UpdateScreenEffects();

	/** Update audio */
	void UpdateAudio();

	/** Update wipers */
	void UpdateWipers(float DeltaTime);

	/** Handle weather changed */
	UFUNCTION()
	void OnWeatherChanged(const FMGWeatherState& NewWeather);

	/** Handle lightning strike */
	UFUNCTION()
	void OnLightningStrike();

	/** Play thunder sound */
	void PlayThunderSound(float Delay);

	/** Apply post process settings */
	void ApplyPostProcessSettings();
};

/**
 * Weather Effect Actor
 * Spawnable actor for world-space weather effects
 */
UCLASS()
class MIDNIGHTGRIND_API AMGWeatherEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AMGWeatherEffectActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Get weather effects component */
	UFUNCTION(BlueprintPure, Category = "Weather")
	UMGWeatherEffectsComponent* GetWeatherEffectsComponent() const { return WeatherEffectsComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UMGWeatherEffectsComponent* WeatherEffectsComponent;

	/** Follow target actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	AActor* FollowTarget;

	/** Height offset above target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float HeightOffset = 500.0f;

	/** Update follow position */
	void UpdateFollowPosition();
};

/**
 * Road Surface Effect Component
 * Handles road surface wetness and spray effects
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGRoadSurfaceEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGRoadSurfaceEffectComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Tire spray particle system */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* TireSpraySystem;

	/** Puddle splash particle system */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* PuddleSplashSystem;

	/** Spray intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float SprayIntensityMultiplier = 1.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Get current spray intensity */
	UFUNCTION(BlueprintPure, Category = "Road Surface")
	float GetSprayIntensity() const { return CurrentSprayIntensity; }

	/** Is vehicle in puddle */
	UFUNCTION(BlueprintPure, Category = "Road Surface")
	bool IsInPuddle() const { return bIsInPuddle; }

	/** Set vehicle speed for spray calculation */
	UFUNCTION(BlueprintCallable, Category = "Road Surface")
	void SetVehicleSpeed(float Speed);

protected:
	UPROPERTY()
	UMGWeatherSubsystem* WeatherSubsystem;

	UPROPERTY()
	TArray<UNiagaraComponent*> TireSprayComponents;

	/** Current spray intensity */
	float CurrentSprayIntensity = 0.0f;

	/** Vehicle speed */
	float VehicleSpeed = 0.0f;

	/** Is in puddle */
	bool bIsInPuddle = false;

	/** Update spray effects */
	void UpdateSprayEffects();

	/** Handle road condition changed */
	UFUNCTION()
	void OnRoadConditionChanged(EMGRoadCondition NewCondition);
};
