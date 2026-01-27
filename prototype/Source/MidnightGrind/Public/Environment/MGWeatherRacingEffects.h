// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Weather/MGWeatherSubsystem.h"
#include "MGWeatherRacingEffects.generated.h"

class UMGVehicleMovementComponent;
class AMGRacingAIController;
class UMGWeatherSubsystem;

// ============================================================================
// WEATHER RACE TYPE ENUMERATION
// ============================================================================

/**
 * @enum EMGWeatherRaceType
 * @brief Defines specialized race types with unique weather conditions
 *
 * Weather race types provide distinct environmental challenges that affect
 * vehicle handling, AI behavior, and race rewards. Each type combines
 * multiple weather effects for a unique racing experience.
 */
UENUM(BlueprintType)
enum class EMGWeatherRaceType : uint8
{
	/** Standard race with current weather conditions */
	Standard UMETA(DisplayName = "Standard"),

	/**
	 * Rain Race: Wet surface with active rain
	 * - Reduced tire grip (30-40% reduction)
	 * - Active puddles causing aquaplaning
	 * - Spray effects from vehicles ahead
	 * - Rain on windshield affecting visibility
	 */
	RainRace UMETA(DisplayName = "Rain Race"),

	/**
	 * Midnight Run: Deep night racing
	 * - Severely limited visibility beyond headlights
	 * - Headlight effectiveness is critical
	 * - AI perception significantly reduced
	 * - Enhanced neon atmosphere
	 */
	MidnightRun UMETA(DisplayName = "Midnight Run"),

	/**
	 * Fog Rally: Dense fog conditions
	 * - Visibility limited to 100-300 meters
	 * - AI perception range severely reduced
	 * - Headlights have limited penetration
	 * - Sound-based racing cues become important
	 */
	FogRally UMETA(DisplayName = "Fog Rally"),

	/**
	 * Storm Chase: Combined severe conditions
	 * - Heavy rain with wind gusts
	 * - Dynamic lightning illumination
	 * - Crosswind effects on vehicle stability
	 * - Maximum weather challenge
	 */
	StormChase UMETA(DisplayName = "Storm Chase"),

	/**
	 * Wind Sprint: High-speed wind challenge
	 * - Strong crosswinds affecting aerodynamics
	 * - Variable wind gusts requiring constant adjustment
	 * - Drafting effects amplified
	 * - Clear but challenging conditions
	 */
	WindSprint UMETA(DisplayName = "Wind Sprint")
};

// ============================================================================
// PUDDLE AND AQUAPLANING STRUCTS
// ============================================================================

/**
 * @struct FMGPuddleInstance
 * @brief Represents a single puddle on the track surface
 *
 * Puddles are spawned dynamically during rain conditions and persist
 * based on drainage calculations. They cause aquaplaning when vehicles
 * pass through at speed.
 */
USTRUCT(BlueprintType)
struct FMGPuddleInstance
{
	GENERATED_BODY()

	/** World location of puddle center */
	UPROPERTY(BlueprintReadOnly, Category = "Puddle")
	FVector Location = FVector::ZeroVector;

	/** Radius of the puddle in centimeters */
	UPROPERTY(BlueprintReadOnly, Category = "Puddle")
	float Radius = 200.0f;

	/** Water depth in centimeters (affects aquaplaning severity) */
	UPROPERTY(BlueprintReadOnly, Category = "Puddle")
	float Depth = 2.0f;

	/** Unique identifier for this puddle */
	UPROPERTY(BlueprintReadOnly, Category = "Puddle")
	int32 PuddleID = 0;

	/** Time this puddle has existed (for evaporation) */
	UPROPERTY(BlueprintReadOnly, Category = "Puddle")
	float Age = 0.0f;

	/**
	 * @brief Calculate aquaplaning factor based on vehicle speed
	 * @param VehicleSpeedKPH Vehicle speed in kilometers per hour
	 * @return Aquaplaning factor (0 = no effect, 1 = full aquaplane)
	 */
	float CalculateAquaplaningFactor(float VehicleSpeedKPH) const
	{
		// Aquaplaning starts around 80 KPH and becomes severe at 140+ KPH
		constexpr float AquaplaneStartSpeed = 80.0f;
		constexpr float AquaplaneSevereSpeed = 140.0f;

		if (VehicleSpeedKPH < AquaplaneStartSpeed)
		{
			return 0.0f;
		}

		const float SpeedFactor = FMath::Clamp(
			(VehicleSpeedKPH - AquaplaneStartSpeed) / (AquaplaneSevereSpeed - AquaplaneStartSpeed),
			0.0f, 1.0f
		);

		// Depth increases aquaplaning severity
		const float DepthFactor = FMath::Clamp(Depth / 5.0f, 0.2f, 1.0f);

		return SpeedFactor * DepthFactor;
	}
};

/**
 * @struct FMGAquaplaningState
 * @brief Tracks aquaplaning state for a vehicle
 *
 * Monitors per-wheel aquaplaning conditions and provides
 * grip modifiers to the vehicle movement component.
 */
USTRUCT(BlueprintType)
struct FMGAquaplaningState
{
	GENERATED_BODY()

	/** Whether vehicle is currently aquaplaning */
	UPROPERTY(BlueprintReadOnly, Category = "Aquaplaning")
	bool bIsAquaplaning = false;

	/** Aquaplaning intensity (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Aquaplaning")
	float AquaplaningIntensity = 0.0f;

	/** Per-wheel aquaplaning factors (FL, FR, RL, RR) */
	UPROPERTY(BlueprintReadOnly, Category = "Aquaplaning")
	TArray<float> WheelAquaplaningFactors;

	/** Duration of current aquaplaning event */
	UPROPERTY(BlueprintReadOnly, Category = "Aquaplaning")
	float AquaplaningDuration = 0.0f;

	/** Current puddle being traversed (if any) */
	UPROPERTY(BlueprintReadOnly, Category = "Aquaplaning")
	int32 CurrentPuddleID = -1;

	FMGAquaplaningState()
	{
		WheelAquaplaningFactors.SetNum(4);
		for (int32 i = 0; i < 4; ++i)
		{
			WheelAquaplaningFactors[i] = 0.0f;
		}
	}

	/**
	 * @brief Get average grip reduction from aquaplaning
	 * @return Grip multiplier (1.0 = no reduction, 0.1 = severe)
	 */
	float GetGripMultiplier() const
	{
		if (!bIsAquaplaning)
		{
			return 1.0f;
		}

		// Severe grip loss during aquaplaning
		return FMath::Lerp(1.0f, 0.1f, AquaplaningIntensity);
	}
};

// ============================================================================
// NIGHT RACING STRUCTS
// ============================================================================

/**
 * @struct FMGNightRacingState
 * @brief Tracks night-specific racing conditions
 *
 * Night racing adds visibility challenges that depend heavily on
 * proper headlight usage. Vehicles with headlights off or damaged
 * suffer significant penalties.
 */
USTRUCT(BlueprintType)
struct FMGNightRacingState
{
	GENERATED_BODY()

	/** Current ambient light level (0 = pitch black, 1 = daylight) */
	UPROPERTY(BlueprintReadOnly, Category = "Night")
	float AmbientLight = 0.15f;

	/** Headlight effectiveness (0-1, affected by damage/dirt) */
	UPROPERTY(BlueprintReadOnly, Category = "Night")
	float HeadlightEffectiveness = 1.0f;

	/** Whether headlights are currently on */
	UPROPERTY(BlueprintReadOnly, Category = "Night")
	bool bHeadlightsOn = true;

	/** Visibility distance in meters */
	UPROPERTY(BlueprintReadOnly, Category = "Night")
	float VisibilityDistance = 150.0f;

	/** Moon illumination factor (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Night")
	float MoonIllumination = 0.3f;

	/** Street light coverage factor (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Night")
	float StreetLightCoverage = 0.5f;

	/**
	 * @brief Calculate effective visibility based on all factors
	 * @return Visibility distance in meters
	 */
	float CalculateEffectiveVisibility() const
	{
		float BaseVisibility = 50.0f; // Minimum visibility

		// Ambient contributes base visibility
		BaseVisibility += AmbientLight * 200.0f;

		// Moon adds some visibility
		BaseVisibility += MoonIllumination * 50.0f;

		// Street lights help in urban areas
		BaseVisibility += StreetLightCoverage * 100.0f;

		// Headlights are critical at night
		if (bHeadlightsOn)
		{
			BaseVisibility += HeadlightEffectiveness * 200.0f;
		}

		return FMath::Clamp(BaseVisibility, 30.0f, 500.0f);
	}
};

// ============================================================================
// FOG RACING STRUCTS
// ============================================================================

/**
 * @struct FMGFogRacingState
 * @brief Tracks fog-specific racing conditions
 *
 * Fog dramatically reduces visibility and makes AI perception
 * challenging. Sound cues become more important for situational awareness.
 */
USTRUCT(BlueprintType)
struct FMGFogRacingState
{
	GENERATED_BODY()

	/** Current fog density (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Fog")
	float FogDensity = 0.0f;

	/** Visibility distance in meters */
	UPROPERTY(BlueprintReadOnly, Category = "Fog")
	float VisibilityDistance = 5000.0f;

	/** Whether fog is thickening or clearing */
	UPROPERTY(BlueprintReadOnly, Category = "Fog")
	bool bFogIncreasing = false;

	/** Fog patches - some areas denser than others */
	UPROPERTY(BlueprintReadOnly, Category = "Fog")
	float LocalDensityMultiplier = 1.0f;

	/** AI perception range multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Fog")
	float AIPerceptionMultiplier = 1.0f;

	/**
	 * @brief Calculate AI perception modifier based on fog
	 * @return Perception range multiplier (0.1 to 1.0)
	 */
	float CalculateAIPerceptionModifier() const
	{
		// Dense fog severely limits AI perception
		const float DensityEffect = 1.0f - (FogDensity * 0.8f);
		return FMath::Clamp(DensityEffect * AIPerceptionMultiplier, 0.1f, 1.0f);
	}
};

// ============================================================================
// WIND EFFECTS STRUCTS
// ============================================================================

/**
 * @struct FMGWindState
 * @brief Tracks wind conditions affecting vehicle handling
 *
 * Wind creates crosswind forces that push vehicles laterally,
 * especially at high speeds where aerodynamic surfaces are more effective.
 */
USTRUCT(BlueprintType)
struct FMGWindState
{
	GENERATED_BODY()

	/** Wind speed in km/h */
	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	float WindSpeed = 0.0f;

	/** Wind direction (world space) */
	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	FVector WindDirection = FVector::ForwardVector;

	/** Current gust intensity (0-1, adds to base wind) */
	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	float GustIntensity = 0.0f;

	/** Time until next gust */
	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	float NextGustTime = 0.0f;

	/** Is currently in a gust event */
	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	bool bInGust = false;

	/** Gust duration remaining */
	UPROPERTY(BlueprintReadOnly, Category = "Wind")
	float GustDuration = 0.0f;

	/**
	 * @brief Calculate effective wind speed including gusts
	 * @return Total effective wind speed in km/h
	 */
	float GetEffectiveWindSpeed() const
	{
		float EffectiveSpeed = WindSpeed;
		if (bInGust)
		{
			EffectiveSpeed += WindSpeed * GustIntensity * 0.5f;
		}
		return EffectiveSpeed;
	}

	/**
	 * @brief Calculate crosswind force for a vehicle
	 * @param VehicleForward Vehicle's forward direction
	 * @param VehicleSpeedKPH Vehicle speed in km/h
	 * @param VehicleFrontalArea Frontal area in square meters
	 * @param DragCoefficient Vehicle's drag coefficient
	 * @return Lateral force in Newtons
	 */
	float CalculateCrosswindForce(
		const FVector& VehicleForward,
		float VehicleSpeedKPH,
		float VehicleFrontalArea,
		float DragCoefficient) const
	{
		// Calculate relative wind angle
		const FVector WindDir = WindDirection.GetSafeNormal();
		const FVector VehicleRight = FVector::CrossProduct(FVector::UpVector, VehicleForward).GetSafeNormal();

		// Crosswind component
		const float CrosswindComponent = FMath::Abs(FVector::DotProduct(WindDir, VehicleRight));

		// Wind force increases with square of speed
		const float EffectiveWindSpeedMS = GetEffectiveWindSpeed() / 3.6f; // Convert to m/s
		const float VehicleSpeedMS = VehicleSpeedKPH / 3.6f;

		// Relative wind velocity
		const float RelativeWindSpeed = EffectiveWindSpeedMS + (VehicleSpeedMS * 0.1f);

		// Force = 0.5 * rho * v^2 * A * Cd
		constexpr float AirDensity = 1.225f; // kg/m^3 at sea level
		const float Force = 0.5f * AirDensity * FMath::Square(RelativeWindSpeed) *
		                    VehicleFrontalArea * DragCoefficient * CrosswindComponent;

		return Force;
	}
};

// ============================================================================
// MAIN WEATHER RACING EFFECTS STRUCT
// ============================================================================

/**
 * @struct FMGWeatherRacingEffects
 * @brief Complete weather racing state combining all environmental effects
 *
 * This struct aggregates all weather-related racing effects into a single
 * queryable state. It is updated each frame by the MGWeatherRacingSubsystem
 * and provides modifiers for vehicle physics, AI behavior, and visual effects.
 *
 * @note Use GetEffectiveGripMultiplier() to get the combined grip modifier
 *       from all weather effects for vehicle physics calculations.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGWeatherRacingEffects
{
	GENERATED_BODY()

	// ========================================
	// RACE TYPE
	// ========================================

	/** Active weather race type */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects")
	EMGWeatherRaceType ActiveRaceType = EMGWeatherRaceType::Standard;

	// ========================================
	// COMPONENT STATES
	// ========================================

	/** Current aquaplaning state */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Rain")
	FMGAquaplaningState AquaplaningState;

	/** Current night racing state */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Night")
	FMGNightRacingState NightState;

	/** Current fog state */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Fog")
	FMGFogRacingState FogState;

	/** Current wind state */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Wind")
	FMGWindState WindState;

	// ========================================
	// AGGREGATE MODIFIERS
	// ========================================

	/** Combined grip multiplier from all weather effects (0.1 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Modifiers")
	float EffectiveGripMultiplier = 1.0f;

	/** AI perception range multiplier (0.1 - 1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Modifiers")
	float AIPerceptionMultiplier = 1.0f;

	/** Visibility distance in meters */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Modifiers")
	float EffectiveVisibilityDistance = 10000.0f;

	/** Top speed modifier from aerodynamic effects */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Modifiers")
	float TopSpeedModifier = 1.0f;

	// ========================================
	// REWARD MODIFIERS
	// ========================================

	/** REP (Reputation) multiplier for completing race */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Rewards")
	float REPMultiplier = 1.0f;

	/** Cash multiplier for completing race */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Rewards")
	float CashMultiplier = 1.0f;

	/** XP multiplier for completing race */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Rewards")
	float XPMultiplier = 1.0f;

	// ========================================
	// CONDITION FLAGS
	// ========================================

	/** Is rain actively falling */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Conditions")
	bool bIsRaining = false;

	/** Is it night time */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Conditions")
	bool bIsNight = false;

	/** Is fog present */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Conditions")
	bool bIsFoggy = false;

	/** Is wind significant */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Conditions")
	bool bIsWindy = false;

	/** Is road surface wet */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Conditions")
	bool bIsWetSurface = false;

	/** Are puddles present on track */
	UPROPERTY(BlueprintReadOnly, Category = "WeatherEffects|Conditions")
	bool bHasPuddles = false;

	// ========================================
	// UTILITY METHODS
	// ========================================

	/**
	 * @brief Get effective grip multiplier combining all effects
	 * @return Combined grip multiplier (0.1 to 1.0)
	 */
	float GetEffectiveGripMultiplier() const
	{
		float Grip = 1.0f;

		// Wet surface reduces grip
		if (bIsWetSurface)
		{
			Grip *= 0.7f;
		}

		// Aquaplaning severely reduces grip
		Grip *= AquaplaningState.GetGripMultiplier();

		return FMath::Clamp(Grip * EffectiveGripMultiplier, 0.1f, 1.0f);
	}

	/**
	 * @brief Get difficulty rating for current conditions (1-5)
	 * @return Difficulty rating where 1 is easy, 5 is extreme
	 */
	int32 GetDifficultyRating() const
	{
		int32 Rating = 1;

		if (bIsWetSurface) Rating++;
		if (bIsNight) Rating++;
		if (bIsFoggy && FogState.FogDensity > 0.5f) Rating++;
		if (bIsWindy && WindState.WindSpeed > 40.0f) Rating++;
		if (AquaplaningState.bIsAquaplaning) Rating++;

		return FMath::Clamp(Rating, 1, 5);
	}

	/**
	 * @brief Get display name for current weather race type
	 * @return Localized display name
	 */
	FText GetRaceTypeDisplayName() const
	{
		switch (ActiveRaceType)
		{
		case EMGWeatherRaceType::RainRace:
			return NSLOCTEXT("WeatherRace", "RainRace", "Rain Race");
		case EMGWeatherRaceType::MidnightRun:
			return NSLOCTEXT("WeatherRace", "MidnightRun", "Midnight Run");
		case EMGWeatherRaceType::FogRally:
			return NSLOCTEXT("WeatherRace", "FogRally", "Fog Rally");
		case EMGWeatherRaceType::StormChase:
			return NSLOCTEXT("WeatherRace", "StormChase", "Storm Chase");
		case EMGWeatherRaceType::WindSprint:
			return NSLOCTEXT("WeatherRace", "WindSprint", "Wind Sprint");
		default:
			return NSLOCTEXT("WeatherRace", "Standard", "Standard");
		}
	}
};

// ============================================================================
// WEATHER RACE CONFIGURATION
// ============================================================================

/**
 * @struct FMGWeatherRaceConfig
 * @brief Configuration for a weather-themed race event
 *
 * Defines all parameters for setting up a weather race including
 * base conditions, effect intensities, and reward multipliers.
 */
USTRUCT(BlueprintType)
struct FMGWeatherRaceConfig
{
	GENERATED_BODY()

	/** Weather race type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGWeatherRaceType RaceType = EMGWeatherRaceType::Standard;

	/** Display name for this configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FText DisplayName;

	/** Description of conditions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (MultiLine = true))
	FText Description;

	// ========================================
	// RAIN SETTINGS
	// ========================================

	/** Rain intensity (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Rain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RainIntensity = 0.0f;

	/** Puddle density on track (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Rain", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PuddleDensity = 0.0f;

	/** Aquaplaning severity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Rain", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AquaplaningSeverity = 1.0f;

	// ========================================
	// NIGHT SETTINGS
	// ========================================

	/** Force night time (override time of day) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Night")
	bool bForceNight = false;

	/** Ambient light level override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Night", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bForceNight"))
	float AmbientLightOverride = 0.1f;

	/** Headlight importance multiplier (affects AI difficulty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Night", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float HeadlightImportance = 1.0f;

	// ========================================
	// FOG SETTINGS
	// ========================================

	/** Fog density (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Fog", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDensity = 0.0f;

	/** Visibility distance in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Fog", meta = (ClampMin = "50.0", ClampMax = "10000.0"))
	float VisibilityDistance = 10000.0f;

	/** AI perception reduction in fog */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Fog", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AIFogPerceptionModifier = 1.0f;

	// ========================================
	// WIND SETTINGS
	// ========================================

	/** Base wind speed in km/h */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Wind", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float WindSpeed = 0.0f;

	/** Wind direction angle (0-360 degrees, 0 = North) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Wind", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float WindDirectionAngle = 0.0f;

	/** Enable random wind gusts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Wind")
	bool bEnableGusts = false;

	/** Maximum gust intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Wind", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableGusts"))
	float MaxGustIntensity = 0.5f;

	// ========================================
	// REWARD SETTINGS
	// ========================================

	/** REP bonus multiplier for this race type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Rewards", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float REPMultiplier = 1.0f;

	/** Cash bonus multiplier for this race type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Rewards", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float CashMultiplier = 1.0f;

	/** XP bonus multiplier for this race type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Rewards", meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float XPMultiplier = 1.0f;

	/**
	 * @brief Get wind direction as a vector
	 * @return Normalized wind direction vector
	 */
	FVector GetWindDirectionVector() const
	{
		const float Radians = FMath::DegreesToRadians(WindDirectionAngle);
		return FVector(FMath::Cos(Radians), FMath::Sin(Radians), 0.0f);
	}
};

// ============================================================================
// DELEGATES
// ============================================================================

/** Fired when a vehicle enters a puddle */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPuddleEntered, AActor*, Vehicle, const FMGPuddleInstance&, Puddle);

/** Fired when a vehicle starts aquaplaning */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAquaplaningStarted, AActor*, Vehicle, float, Intensity);

/** Fired when aquaplaning ends */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAquaplaningEnded, AActor*, Vehicle);

/** Fired when weather race type changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherRaceTypeChanged, EMGWeatherRaceType, OldType, EMGWeatherRaceType, NewType);

/** Fired when a wind gust occurs */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWindGust, float, Intensity, FVector, Direction);

/** Fired when visibility changes significantly */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVisibilityChanged, float, OldVisibility, float, NewVisibility);

// ============================================================================
// WEATHER RACING SUBSYSTEM
// ============================================================================

/**
 * @class UMGWeatherRacingSubsystem
 * @brief World subsystem managing weather effects on racing
 *
 * This subsystem coordinates weather effects with vehicle physics and AI behavior.
 * It provides:
 * - Puddle spawning and management during rain
 * - Aquaplaning detection and force application
 * - Night racing visibility calculations
 * - Fog-based AI perception adjustments
 * - Wind force calculations for vehicles
 * - Reward multiplier calculations for weather races
 *
 * @note This subsystem requires UMGWeatherSubsystem to be active.
 *
 * @see UMGWeatherSubsystem for base weather state
 * @see UMGVehicleMovementComponent for physics integration
 */
UCLASS()
class MIDNIGHTGRIND_API UMGWeatherRacingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End USubsystem Interface

	/** Tick called by world */
	void Tick(float DeltaTime);
	virtual TStatId GetStatId() const;

	// ========================================
	// WEATHER RACE TYPES
	// ========================================

	/**
	 * @brief Set active weather race type
	 * @param RaceType The weather race type to activate
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|Type")
	void SetWeatherRaceType(EMGWeatherRaceType RaceType);

	/**
	 * @brief Apply a weather race configuration
	 * @param Config The configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|Type")
	void ApplyWeatherRaceConfig(const FMGWeatherRaceConfig& Config);

	/**
	 * @brief Get default configuration for a race type
	 * @param RaceType The race type to get config for
	 * @return Default configuration for the race type
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Type")
	FMGWeatherRaceConfig GetDefaultConfigForType(EMGWeatherRaceType RaceType) const;

	/**
	 * @brief Get current weather racing effects state
	 * @return Current weather racing effects
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing")
	FMGWeatherRacingEffects GetCurrentEffects() const { return CurrentEffects; }

	/**
	 * @brief Get active weather race type
	 * @return Current weather race type
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing")
	EMGWeatherRaceType GetActiveRaceType() const { return CurrentEffects.ActiveRaceType; }

	// ========================================
	// PUDDLES AND AQUAPLANING
	// ========================================

	/**
	 * @brief Get all active puddles
	 * @return Array of active puddle instances
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Puddles")
	TArray<FMGPuddleInstance> GetActivePuddles() const { return ActivePuddles; }

	/**
	 * @brief Spawn a puddle at location
	 * @param Location World location for puddle center
	 * @param Radius Puddle radius in centimeters
	 * @param Depth Water depth in centimeters
	 * @return The spawned puddle instance
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|Puddles")
	FMGPuddleInstance SpawnPuddle(const FVector& Location, float Radius = 200.0f, float Depth = 2.0f);

	/**
	 * @brief Clear all puddles
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|Puddles")
	void ClearAllPuddles();

	/**
	 * @brief Check if a point is in a puddle
	 * @param Location World location to check
	 * @param OutPuddle Output puddle if found
	 * @return True if location is in a puddle
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Puddles")
	bool IsPointInPuddle(const FVector& Location, FMGPuddleInstance& OutPuddle) const;

	/**
	 * @brief Calculate aquaplaning state for a vehicle
	 * @param VehicleLocation Vehicle world location
	 * @param VehicleSpeedKPH Vehicle speed in km/h
	 * @param WheelLocations Locations of each wheel (FL, FR, RL, RR)
	 * @return Calculated aquaplaning state
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|Aquaplaning")
	FMGAquaplaningState CalculateAquaplaning(
		const FVector& VehicleLocation,
		float VehicleSpeedKPH,
		const TArray<FVector>& WheelLocations);

	// ========================================
	// VISIBILITY AND PERCEPTION
	// ========================================

	/**
	 * @brief Get effective visibility distance at a location
	 * @param Location World location to check
	 * @return Visibility distance in meters
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Visibility")
	float GetEffectiveVisibility(const FVector& Location) const;

	/**
	 * @brief Get AI perception multiplier for current conditions
	 * @return Perception range multiplier (0.1 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|AI")
	float GetAIPerceptionMultiplier() const { return CurrentEffects.AIPerceptionMultiplier; }

	/**
	 * @brief Update AI controller with weather effects
	 * @param AIController The AI controller to update
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|AI")
	void UpdateAIForWeather(AMGRacingAIController* AIController);

	// ========================================
	// WIND EFFECTS
	// ========================================

	/**
	 * @brief Get current wind state
	 * @return Current wind state
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Wind")
	FMGWindState GetWindState() const { return CurrentEffects.WindState; }

	/**
	 * @brief Calculate wind force for a vehicle
	 * @param VehicleForward Vehicle forward direction
	 * @param VehicleSpeedKPH Vehicle speed in km/h
	 * @param FrontalArea Vehicle frontal area in m^2
	 * @param DragCoefficient Vehicle drag coefficient
	 * @return Wind force vector in Newtons
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Wind")
	FVector CalculateWindForce(
		const FVector& VehicleForward,
		float VehicleSpeedKPH,
		float FrontalArea,
		float DragCoefficient) const;

	/**
	 * @brief Trigger a wind gust
	 * @param Intensity Gust intensity (0-1)
	 * @param Duration Gust duration in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRacing|Wind")
	void TriggerWindGust(float Intensity, float Duration);

	// ========================================
	// REWARDS
	// ========================================

	/**
	 * @brief Get reward multipliers for current weather conditions
	 * @param OutREPMultiplier Output REP multiplier
	 * @param OutCashMultiplier Output cash multiplier
	 * @param OutXPMultiplier Output XP multiplier
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Rewards")
	void GetRewardMultipliers(float& OutREPMultiplier, float& OutCashMultiplier, float& OutXPMultiplier) const;

	/**
	 * @brief Calculate bonus rewards text for UI
	 * @return Formatted text describing weather bonuses
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRacing|Rewards")
	FText GetWeatherBonusDescription() const;

	// ========================================
	// EVENTS
	// ========================================

	/** Fired when a vehicle enters a puddle */
	UPROPERTY(BlueprintAssignable, Category = "WeatherRacing|Events")
	FOnPuddleEntered OnPuddleEntered;

	/** Fired when a vehicle starts aquaplaning */
	UPROPERTY(BlueprintAssignable, Category = "WeatherRacing|Events")
	FOnAquaplaningStarted OnAquaplaningStarted;

	/** Fired when aquaplaning ends */
	UPROPERTY(BlueprintAssignable, Category = "WeatherRacing|Events")
	FOnAquaplaningEnded OnAquaplaningEnded;

	/** Fired when weather race type changes */
	UPROPERTY(BlueprintAssignable, Category = "WeatherRacing|Events")
	FOnWeatherRaceTypeChanged OnWeatherRaceTypeChanged;

	/** Fired when a wind gust occurs */
	UPROPERTY(BlueprintAssignable, Category = "WeatherRacing|Events")
	FOnWindGust OnWindGust;

protected:
	// ========================================
	// CONFIGURATION
	// ========================================

	/** Maximum number of active puddles */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Puddles")
	int32 MaxPuddles = 50;

	/** Puddle evaporation rate (depth cm per second) */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Puddles")
	float PuddleEvaporationRate = 0.01f;

	/** Rain puddle spawn rate (puddles per second at max rain) */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Puddles")
	float PuddleSpawnRate = 0.5f;

	/** Minimum wind speed to affect vehicles (km/h) */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Wind")
	float MinEffectiveWindSpeed = 20.0f;

	/** Average time between gusts (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Wind")
	float AverageGustInterval = 15.0f;

	/** Gust duration range (min, max) in seconds */
	UPROPERTY(EditDefaultsOnly, Category = "Config|Wind")
	FVector2D GustDurationRange = FVector2D(2.0f, 5.0f);

private:
	// ========================================
	// INTERNAL STATE
	// ========================================

	/** Current weather racing effects */
	UPROPERTY()
	FMGWeatherRacingEffects CurrentEffects;

	/** Active puddles on track */
	UPROPERTY()
	TArray<FMGPuddleInstance> ActivePuddles;

	/** Next puddle ID */
	int32 NextPuddleID = 0;

	/** Time since last puddle spawn */
	float PuddleSpawnAccumulator = 0.0f;

	/** Time since last gust */
	float GustTimer = 0.0f;

	/** Reference to weather subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGWeatherSubsystem> WeatherSubsystem;

	/** Vehicles currently aquaplaning */
	UPROPERTY()
	TMap<AActor*, FMGAquaplaningState> VehicleAquaplaningStates;

	// ========================================
	// INTERNAL METHODS
	// ========================================

	/** Update effects from weather subsystem */
	void UpdateFromWeatherSubsystem();

	/** Update puddle simulation */
	void UpdatePuddles(float DeltaTime);

	/** Update wind gusts */
	void UpdateWindGusts(float DeltaTime);

	/** Calculate aggregate modifiers */
	void UpdateAggregateModifiers();

	/** Spawn puddles based on rain */
	void SpawnRainPuddles(float DeltaTime);

	/** Setup default race type configurations */
	void SetupDefaultConfigurations();

	/** Apply configuration to current state */
	void ApplyConfiguration(const FMGWeatherRaceConfig& Config);

	/** Default configurations for each race type */
	TMap<EMGWeatherRaceType, FMGWeatherRaceConfig> DefaultConfigurations;
};
