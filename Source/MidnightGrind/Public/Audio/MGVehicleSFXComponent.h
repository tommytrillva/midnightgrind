// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGVehicleSFXComponent.h
 * @brief Vehicle Sound Effects Component (Non-Engine Audio)
 *
 * @section overview Overview
 * This component handles all vehicle sound effects except for engine sounds.
 * It manages tire sounds (rolling, skidding), collision impacts, scraping,
 * wind noise, suspension, and brakes. The component responds to vehicle
 * physics state and surface types to create immersive audio feedback.
 *
 * @section beginners Key Concepts for Beginners
 *
 * @subsection why_separate Why Separate from Engine Audio?
 * Vehicle audio is typically split into:
 * - Engine sounds: Complex RPM-based system (handled elsewhere)
 * - Everything else: This component
 *
 * Separation allows:
 * - Different audio designers to work on each
 * - Different update rates (engine needs high precision)
 * - Easier debugging and tuning
 * - Modular vehicle configuration
 *
 * @subsection surface_sounds Surface-Dependent Sounds
 * Real tires sound different on different surfaces. This component:
 * - Detects the surface type under each wheel
 * - Plays appropriate tire roll/skid sounds
 * - Adjusts pitch and volume based on surface properties
 *
 * @subsection surface_types Surface Types (EMGSurfaceType)
 * - Asphalt: Standard road (most common)
 * - Concrete: Harder, slightly different tone
 * - Gravel: Crunchy, louder spray sounds
 * - Dirt: Softer, earthier tone
 * - Grass: Very soft, muted
 * - Sand: Soft, whooshing spray
 * - Water: Splashing effects
 * - Metal: Loud, resonant (bridges, grates)
 * - Wood: Hollow, thumping (docks, bridges)
 *
 * @subsection tire_physics Tire Sound Physics
 * Tire sounds depend on:
 * - Speed: Faster = higher pitch, louder
 * - Slip ratio: How much the tire is sliding vs rolling
 * - Surface: Different textures = different sounds
 *
 * Slip ratio > threshold = skidding/screeching begins
 *
 * @subsection collision_sounds Collision System
 * Collisions are classified by force intensity:
 * - Light: Gentle tap (<50kN)
 * - Medium: Noticeable impact (50-200kN)
 * - Heavy: Hard crash (200-500kN)
 * - Extreme: Massive collision (>500kN)
 *
 * Each level has its own sound pool for variety.
 *
 * @section sound_types Sound Categories
 *
 * @subsection tire_sounds Tire Sounds
 * - Roll: Continuous sound while wheels turn on surface
 * - Skid: Screeching when tires lose traction
 * - Spray: Debris/water kicked up (gravel, water)
 *
 * @subsection impact_sounds Impact Sounds
 * - Collision: One-shot impacts of varying intensity
 * - Scrape: Loop while grinding against surfaces
 * - Glass: Breaking windows/lights
 * - Metal: Crunching bodywork
 *
 * @subsection ambient_sounds Ambient Sounds
 * - Wind: Increases with speed
 * - Suspension: Thumps on bumps/landings
 * - Brakes: Squeal when braking hard
 *
 * @section usage Usage Examples
 *
 * @subsection setup Component Setup
 * @code
 * // In your vehicle class header
 * UPROPERTY(VisibleAnywhere)
 * UMGVehicleSFXComponent* VehicleSFX;
 *
 * // In constructor
 * VehicleSFX = CreateDefaultSubobject<UMGVehicleSFXComponent>(TEXT("VehicleSFX"));
 *
 * // Configure surfaces in Blueprint or code
 * FMGSurfaceSoundConfig AsphaltConfig;
 * AsphaltConfig.SurfaceType = EMGSurfaceType::Asphalt;
 * AsphaltConfig.TireRollSound = LoadObject<USoundBase>(...);
 * AsphaltConfig.TireSkidSound = LoadObject<USoundBase>(...);
 * AsphaltConfig.SkidThreshold = 0.2f;
 * VehicleSFX->AddSurfaceConfig(AsphaltConfig);
 * @endcode
 *
 * @subsection per_frame Per-Frame Updates
 * @code
 * // In your vehicle's Tick
 * void AMyVehicle::Tick(float MGDeltaTime)
 * {
 *     Super::Tick(DeltaTime);
 *
 *     // Update SFX component with current state
 *     VehicleSFX->SetSpeed(GetVehicleMovement()->GetForwardSpeed());
 *
 *     // Tire slip from physics
 *     float FrontSlip = GetFrontWheelSlipRatio();
 *     float RearSlip = GetRearWheelSlipRatio();
 *     VehicleSFX->SetTireSlip(FrontSlip, RearSlip);
 *
 *     // Surface detection
 *     UPhysicalMaterial* WheelSurface = GetSurfaceUnderWheel(0);
 *     VehicleSFX->SetSurfaceFromPhysMat(WheelSurface);
 *
 *     // Brake input
 *     VehicleSFX->SetBrakeInput(BrakeValue);
 *
 *     // Airborne state
 *     VehicleSFX->SetAirborne(!AnyWheelTouchingGround());
 * }
 * @endcode
 *
 * @subsection collision_handling Collision Handling
 * @code
 * // In your vehicle's collision response
 * void AMyVehicle::NotifyHit(...)
 * {
 *     Super::NotifyHit(...);
 *
 *     // Get impact force from physics
 *     float Force = NormalImpulse.Size();
 *
 *     // Notify SFX component
 *     VehicleSFX->OnCollision(Force, HitLocation, HitNormal);
 * }
 *
 * // For continuous scraping
 * void AMyVehicle::OnScrapeBegin()
 * {
 *     VehicleSFX->StartScrape(1.0f);  // Full intensity
 * }
 *
 * void AMyVehicle::OnScrapeEnd()
 * {
 *     VehicleSFX->StopScrape();
 * }
 *
 * // Glass breaking
 * void AMyVehicle::OnWindowSmash(FVector Location)
 * {
 *     VehicleSFX->PlayGlassBreak(Location);
 * }
 * @endcode
 *
 * @subsection configuration Configuration
 * @code
 * // Set collision thresholds for your vehicle
 * VehicleSFX->LightCollisionThreshold = 30000.0f;   // Lighter vehicle
 * VehicleSFX->MediumCollisionThreshold = 100000.0f;
 * VehicleSFX->HeavyCollisionThreshold = 300000.0f;
 *
 * // Wind noise range
 * VehicleSFX->WindNoiseMinSpeed = 1500.0f;  // Start at 54 km/h
 * VehicleSFX->WindNoiseMaxSpeed = 6000.0f;  // Full at 216 km/h
 *
 * // Master volume control
 * VehicleSFX->SetMasterVolume(0.8f);
 *
 * // Disable for replays/ghosts
 * VehicleSFX->SetEnabled(false);
 * @endcode
 *
 * @subsection surface_mapping Physical Material Mapping
 * @code
 * // The component can auto-detect surface from physical materials
 * // Override PhysMatToSurfaceType() for custom mapping:
 *
 * EMGSurfaceType UMyVehicleSFX::PhysMatToSurfaceType(UPhysicalMaterial* PhysMat)
 * {
 *     if (!PhysMat) return EMGSurfaceType::Asphalt;
 *
 *     FName MatName = PhysMat->GetFName();
 *     if (MatName == TEXT("PM_Road")) return EMGSurfaceType::Asphalt;
 *     if (MatName == TEXT("PM_Gravel")) return EMGSurfaceType::Gravel;
 *     if (MatName == TEXT("PM_Grass")) return EMGSurfaceType::Grass;
 *     // etc.
 *
 *     return Super::PhysMatToSurfaceType(PhysMat);
 * }
 * @endcode
 *
 * @section config_structs Configuration Structures
 *
 * @subsection surface_config FMGSurfaceSoundConfig
 * Per-surface sound settings:
 * - TireRollSound: Loop while driving on surface
 * - TireSkidSound: Loop while skidding
 * - SurfaceSpraySound: Debris/water spray
 * - PitchMultiplier: Surface-specific pitch adjust
 * - VolumeMultiplier: Surface-specific volume adjust
 * - SkidThreshold: Slip ratio to start skidding
 *
 * @subsection collision_config FMGCollisionSoundConfig
 * Collision sound pools:
 * - LightImpacts: Array for variety
 * - MediumImpacts: Array for variety
 * - HeavyImpacts: Array for variety
 * - ExtremeImpacts: Array for variety
 * - ScrapeLoop: Continuous grinding
 * - GlassBreak: Window smash
 * - MetalCrunch: Bodywork damage
 *
 * @section audio_components Audio Components
 * The component manages several UAudioComponents internally:
 * - TireRollComponent: Surface roll loop
 * - TireSkidComponent: Skid/screech loop
 * - WindNoiseComponent: Speed-based wind
 * - ScrapeComponent: Wall grinding loop
 * - BrakeComponent: Brake squeal
 *
 * These are created at BeginPlay and cleaned up at EndPlay.
 *
 * @section state_queries State Queries
 * @code
 * // Check current state
 * EMGSurfaceType CurrentSurface = VehicleSFX->GetCurrentSurface();
 * bool bIsSkidding = VehicleSFX->IsSkidding();
 * bool bIsScraping = VehicleSFX->IsScraping();
 * @endcode
 *
 * @section performance Performance Notes
 * - Collision sounds have a cooldown (CollisionCooldown) to prevent spam
 * - Sounds only play when enabled and component is active
 * - Loops use UAudioComponent for efficiency
 * - One-shots use PlaySoundAtLocation for simplicity
 *
 * @see MGMusicManager.h For background music system
 * @see MG_VHCL_AudioComponent.h For engine audio (if separate)
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Core/MGSharedTypes.h"
#include "MGVehicleSFXComponent.generated.h"

// MOVED TO MGSharedTypes.h
// /**
//  * Surface type for tire sounds
//  */
// UENUM(BlueprintType)
// enum class EMGSurfaceType : uint8
// {
// 	Asphalt,
// 	Concrete,
// 	Gravel,
// 	Dirt,
// 	Grass,
// 	Sand,
// 	Water,
// 	Metal,
// 	Wood
// };

/**
 * Collision intensity classification.
 * 
 * Categorizes collision force into discrete levels for sound selection.
 * Thresholds are configured in UMGVehicleSFXComponent.
 */
UENUM(BlueprintType)
enum class EMGCollisionIntensity : uint8
{
	/** Light tap/scrape (<LightCollisionThreshold) */
	Light,
	/** Medium impact (Light to Medium threshold) */
	Medium,
	/** Heavy crash (Medium to Heavy threshold) */
	Heavy,
	/** Massive collision (>HeavyCollisionThreshold) */
	Extreme
};

/**
 * Per-surface tire sound configuration.
 * 
 * Defines how tires sound when rolling and skidding on a specific
 * surface type. Each surface can have unique sounds, volume, pitch,
 * and skid behavior.
 */
USTRUCT(BlueprintType)
struct FMGSurfaceSoundConfig
{
	GENERATED_BODY()

	/** Surface type this config applies to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	EMGSurfaceType SurfaceType = EMGSurfaceType::Asphalt;

	/** Tire roll loop sound (continuous while driving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	USoundBase* TireRollSound = nullptr;

	/** Tire skid/screech sound (when slipping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	USoundBase* TireSkidSound = nullptr;

	/** Surface spray sound (gravel/water debris kicked up) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface")
	USoundBase* SurfaceSpraySound = nullptr;

	/** Pitch multiplier for this surface (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PitchMultiplier = 1.0f;

	/** Volume multiplier for this surface (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	/** Slip ratio threshold to trigger skid sounds (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SkidThreshold = 0.2f;
};

/**
 * Collision and impact sound configuration.
 * 
 * Pools of sounds for different collision intensities, plus special
 * sounds for scraping, glass breaking, and metal crunching. Using
 * arrays allows random selection for variety.
 */
USTRUCT(BlueprintType)
struct FMGCollisionSoundConfig
{
	GENERATED_BODY()

	/** Light impact sound pool (random selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> LightImpacts;

	/** Medium impact sound pool (random selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> MediumImpacts;

	/** Heavy impact sound pool (random selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> HeavyImpacts;

	/** Extreme impact sound pool (random selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TArray<USoundBase*> ExtremeImpacts;

	/** Continuous scraping/grinding loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USoundBase* ScrapeLoop = nullptr;

	/** Glass/window breaking sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USoundBase* GlassBreak = nullptr;

	/** Metal deformation/crunch sound */
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
	virtual void TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	/** Light collision force threshold (N) - below this plays light impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config", meta = (ClampMin = "0.0"))
	float LightCollisionThreshold = 50000.0f;

	/** Medium collision force threshold (N) - above light, below this plays medium impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config", meta = (ClampMin = "0.0"))
	float MediumCollisionThreshold = 200000.0f;

	/** Heavy collision force threshold (N) - above this plays heavy/extreme impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX|Config", meta = (ClampMin = "0.0"))
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
	void UpdateTireSounds(float MGDeltaTime);
	void UpdateWindNoise(float MGDeltaTime);
	void UpdateBrakeSounds(float MGDeltaTime);

	EMGCollisionIntensity GetCollisionIntensity(float Force) const;
	USoundBase* GetRandomCollisionSound(EMGCollisionIntensity Intensity) const;
	const FMGSurfaceSoundConfig* GetCurrentSurfaceConfig() const;

	EMGSurfaceType PhysMatToSurfaceType(UPhysicalMaterial* PhysMat) const;
};
