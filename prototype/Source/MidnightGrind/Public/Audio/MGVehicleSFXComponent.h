// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "MGVehicleSFXComponent.generated.h"

/**
 * Surface type for tire sounds
 */
UENUM(BlueprintType)
enum class EMGSurfaceType : uint8
{
	Asphalt,
	Concrete,
	Gravel,
	Dirt,
	Grass,
	Sand,
	Water,
	Metal,
	Wood
};

/**
 * Collision intensity level
 */
UENUM(BlueprintType)
enum class EMGCollisionIntensity : uint8
{
	/** Light tap/scrape */
	Light,
	/** Medium impact */
	Medium,
	/** Heavy crash */
	Heavy,
	/** Massive collision */
	Extreme
};

/**
 * Per-surface sound configuration
 */
USTRUCT(BlueprintType)
struct FMGSurfaceSoundConfig
{
	GENERATED_BODY()

	/** Surface type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	EMGSurfaceType SurfaceType = EMGSurfaceType::Asphalt;

	/** Tire roll loop sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	USoundBase* TireRollSound = nullptr;

	/** Tire skid/screech sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	USoundBase* TireSkidSound = nullptr;

	/** Surface spray sound (gravel, water) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	USoundBase* SurfaceSpraySound = nullptr;

	/** Pitch multiplier for this surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	float PitchMultiplier = 1.0f;

	/** Volume multiplier for this surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	float VolumeMultiplier = 1.0f;

	/** Skid volume threshold (slip ratio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	float SkidThreshold = 0.2f;
};

/**
 * Collision sound configuration
 */
USTRUCT(BlueprintType)
struct FMGCollisionSoundConfig
{
	GENERATED_BODY()

	/** Light impact sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> LightImpacts;

	/** Medium impact sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> MediumImpacts;

	/** Heavy impact sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> HeavyImpacts;

	/** Extreme impact sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> ExtremeImpacts;

	/** Scrape loop sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USoundBase* ScrapeLoop = nullptr;

	/** Glass break sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USoundBase* GlassBreak = nullptr;

	/** Metal crunch sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USoundBase* MetalCrunch = nullptr;
};

/**
 * Vehicle SFX Component
 * Handles all non-engine vehicle sounds
 *
 * Features:
 * - Surface-dependent tire sounds
 * - Tire skid/screech based on slip
 * - Collision impacts with intensity scaling
 * - Scrape sounds for continuous contact
 * - Wind noise based on speed
 * - Suspension sounds
 * - Brake sounds
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGVehicleSFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGVehicleSFXComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// VEHICLE STATE INPUT
	// ==========================================

	/** Set current speed (cm/s) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetSpeed(float Speed);

	/** Set tire slip ratio (0-1+) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetTireSlip(float FrontSlip, float RearSlip);

	/** Set current surface type */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetSurfaceType(EMGSurfaceType Surface);

	/** Set surface from physical material */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetSurfaceFromPhysMat(UPhysicalMaterial* PhysMat);

	/** Set brake input (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetBrakeInput(float Brake);

	/** Set if vehicle is airborne */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetAirborne(bool bInAir);

	// ==========================================
	// COLLISION EVENTS
	// ==========================================

	/** Notify of collision impact */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void OnCollision(float ImpactForce, FVector ImpactLocation, FVector ImpactNormal);

	/** Start scrape sound (continuous contact) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void StartScrape(float Intensity);

	/** Stop scrape sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void StopScrape();

	/** Play glass break sound */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void PlayGlassBreak(FVector Location);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Add surface sound configuration */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void AddSurfaceConfig(const FMGSurfaceSoundConfig& Config);

	/** Set collision sounds */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetCollisionConfig(const FMGCollisionSoundConfig& Config);

	/** Set master volume */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetMasterVolume(float Volume);

	/** Enable/disable SFX */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void SetEnabled(bool bEnabled);

	// ==========================================
	// STATE
	// ==========================================

	/** Get current surface type */
	UFUNCTION(BlueprintPure, Category = "Vehicle SFX")
	EMGSurfaceType GetCurrentSurface() const { return CurrentSurface; }

	/** Is currently skidding? */
	UFUNCTION(BlueprintPure, Category = "Vehicle SFX")
	bool IsSkidding() const { return bIsSkidding; }

	/** Is scraping? */
	UFUNCTION(BlueprintPure, Category = "Vehicle SFX")
	bool IsScraping() const { return bIsScraping; }

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Surface sound configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	TMap<EMGSurfaceType, FMGSurfaceSoundConfig> SurfaceConfigs;

	/** Collision sound configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	FMGCollisionSoundConfig CollisionConfig;

	/** Wind noise sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	USoundBase* WindNoiseSound = nullptr;

	/** Suspension thump sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	USoundBase* SuspensionSound = nullptr;

	/** Brake squeal sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	USoundBase* BrakeSquealSound = nullptr;

	/** Master volume */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MasterVolume = 1.0f;

	/** Collision force thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	float LightCollisionThreshold = 50000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	float MediumCollisionThreshold = 200000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	float HeavyCollisionThreshold = 500000.0f;

	/** Wind noise starts at this speed (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	float WindNoiseMinSpeed = 2000.0f;

	/** Wind noise full volume at this speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	float WindNoiseMaxSpeed = 8000.0f;

	/** Minimum time between collision sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config")
	float CollisionCooldown = 0.1f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current speed */
	float CurrentSpeed = 0.0f;

	/** Current tire slip */
	float CurrentFrontSlip = 0.0f;
	float CurrentRearSlip = 0.0f;

	/** Current surface */
	EMGSurfaceType CurrentSurface = EMGSurfaceType::Asphalt;

	/** Current brake input */
	float CurrentBrake = 0.0f;

	/** Is airborne */
	bool bIsAirborne = false;

	/** Is skidding */
	bool bIsSkidding = false;

	/** Is scraping */
	bool bIsScraping = false;

	/** Is enabled */
	bool bIsEnabled = true;

	/** Time since last collision sound */
	float TimeSinceLastCollision = 0.0f;

	// ==========================================
	// AUDIO COMPONENTS
	// ==========================================

	UPROPERTY()
	UAudioComponent* TireRollComponent = nullptr;

	UPROPERTY()
	UAudioComponent* TireSkidComponent = nullptr;

	UPROPERTY()
	UAudioComponent* WindNoiseComponent = nullptr;

	UPROPERTY()
	UAudioComponent* ScrapeComponent = nullptr;

	UPROPERTY()
	UAudioComponent* BrakeComponent = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	void InitializeAudioComponents();
	void CleanupAudioComponents();
	void UpdateTireSounds(float DeltaTime);
	void UpdateWindNoise(float DeltaTime);
	void UpdateBrakeSounds(float DeltaTime);

	EMGCollisionIntensity GetCollisionIntensity(float Force) const;
	USoundBase* GetRandomCollisionSound(EMGCollisionIntensity Intensity) const;
	const FMGSurfaceSoundConfig* GetCurrentSurfaceConfig() const;

	EMGSurfaceType PhysMatToSurfaceType(UPhysicalMaterial* PhysMat) const;
};
