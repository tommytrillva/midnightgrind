// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGRaceConfiguration.h
 * @brief Race Configuration Data Assets for Midnight Grind racing game.
 *
 * This file contains data asset classes and supporting structures that define
 * the complete configuration for race events. Race configurations are the
 * "recipe" for a race - they specify everything from track selection and
 * opponent difficulty to rewards and unlock requirements.
 *
 * @section overview_config Overview
 * The race configuration system provides:
 * - UMGRaceConfiguration: Primary data asset for complete race setup
 * - UMGQuickRacePreset: Simplified preset for instant/casual play
 * - FMGAIOpponentConfig: Per-opponent AI behavior settings
 * - FMGRaceRewardConfig: Reward distribution configuration
 *
 * @section concepts_config Key Concepts for Beginners
 *
 * 1. **Data Assets**: In Unreal Engine, Data Assets are Blueprint-editable
 *    configuration files that live in your Content folder. Unlike regular
 *    Blueprints, they don't spawn into the world - they just hold data.
 *    Designers can create many race configurations in the Editor without code.
 *
 * 2. **Primary Data Asset**: UMGRaceConfiguration extends UPrimaryDataAsset,
 *    which allows the Asset Manager to load/manage these assets efficiently.
 *    Each configuration has a unique RaceID for lookup.
 *
 * 3. **Soft Object Pointers**: TSoftObjectPtr references (like VehicleModel,
 *    RaceLevel) are "lazy" references that don't load the asset until needed.
 *    This prevents loading every car model when loading a single race config.
 *
 * 4. **USTRUCT**: The FMG* structs bundle related settings together and are
 *    editable in Blueprint property panels. They're simpler than UObjects
 *    but can still be exposed to designers.
 *
 * @section architecture_config Architecture
 *
 *    [Content Browser]
 *           |
 *    [UMGRaceConfiguration Data Assets]  <-- Designer creates these
 *           |
 *    [CreateRaceConfig()]  <-- Converts to runtime FMGRaceConfig
 *           |
 *    [AMGRaceGameMode]  <-- Receives config and runs the race
 *           |
 *    [UMGRaceFlowManager]  <-- Handles rewards using config data
 *
 * @section usage_config Usage Example
 *
 * @code
 * // In Blueprint or C++, load and use a race configuration:
 *
 * // 1. Get the race configuration from the asset manager
 * UMGRaceConfiguration* Config = Cast<UMGRaceConfiguration>(
 *     UAssetManager::GetPrimaryAssetObject(FPrimaryAssetId("RaceConfig", "Race_Downtown_Sprint")));
 *
 * // 2. Check if player can access this race
 * if (Config->CanPlayerAccess(PlayerLevel, PlayerRep, CompletedRaces))
 * {
 *     // 3. Convert to runtime config and start race
 *     FMGRaceConfig RuntimeConfig = Config->CreateRaceConfig();
 *     GameMode->SetRaceConfig(RuntimeConfig);
 *     GameMode->StartCountdown();
 * }
 *
 * // Or for quick races:
 * UMGQuickRacePreset* Preset = LoadObject<UMGQuickRacePreset>(...);
 * // Apply preset values directly to the game mode
 * @endcode
 *
 * @section creating_config Creating Race Configurations
 *
 * 1. In Content Browser, right-click > Miscellaneous > Data Asset
 * 2. Select MGRaceConfiguration as the class
 * 3. Name it descriptively (e.g., "RC_Downtown_Circuit_Hard")
 * 4. Double-click to open and configure all settings
 * 5. Save - it's now available for use in menus and career mode
 *
 * @see AMGRaceGameMode For the game mode that uses these configurations
 * @see UMGRaceFlowManager For reward processing
 * @see FMGRaceConfig For the runtime configuration struct
 */

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameModes/MGRaceGameMode.h"
#include "AI/MG_AI_RaceManager.h"
#include "MGRaceConfiguration.generated.h"

class UMGVehicleModelData;

// FMGAIOpponentConfig - defined in AI/MG_AI_RaceManager.h

/**
 * Race Reward Configuration
 */
USTRUCT(BlueprintType)
struct FMGRaceRewardConfig
{
	GENERATED_BODY()

	/** Credits for 1st place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FirstPlaceCredits = 5000;

	/** Credits for 2nd place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SecondPlaceCredits = 3000;

	/** Credits for 3rd place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ThirdPlaceCredits = 2000;

	/** Credits for finishing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParticipationCredits = 500;

	/** XP for first place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FirstPlaceXP = 1000;

	/** XP for finishing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParticipationXP = 100;

	/** Multiplier for perfect start (no collision on first lap) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CleanRaceMultiplier = 1.25f;
};

/**
 * Race Configuration Data Asset
 * Defines all settings for a race event
 *
 * Use this to quickly set up races with specific:
 * - Track and race type
 * - Number of laps/distance
 * - AI opponents with customizable difficulty
 * - Rewards and unlocks
 * - Weather and time of day
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRaceConfiguration : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UMGRaceConfiguration();

	// ==========================================
	// IDENTIFICATION
	// ==========================================

	/** Unique ID for this race configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName RaceID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	// ==========================================
	// RACE SETTINGS
	// ==========================================

	/** Track to race on (empty = use current level) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race")
	FName TrackID;

	/** Level to load (if different from current) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race")
	TSoftObjectPtr<UWorld> RaceLevel;

	/** Race type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	/** Number of laps (for circuit races) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race", meta = (EditCondition = "RaceType == EMGRaceType::Circuit", ClampMin = "1", ClampMax = "99"))
	int32 NumberOfLaps = 3;

	/** Race distance in meters (for sprint/drag) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race", meta = (EditCondition = "RaceType != EMGRaceType::Circuit"))
	float RaceDistanceMeters = 1000.0f;

	/** Time limit in seconds (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race")
	float TimeLimitSeconds = 0.0f;

	/** Allow player to respawn after crash */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race")
	bool bAllowRespawn = true;

	/** Respawn penalty time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Race")
	float RespawnPenaltySeconds = 3.0f;

	// ==========================================
	// AI OPPONENTS
	// ==========================================

	/** Number of AI opponents (0 = time trial) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0", ClampMax = "11"))
	int32 NumberOfOpponents = 5;

	/** Specific AI opponent configurations (if empty, generates random) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TArray<FMGAIOpponentConfig> OpponentConfigs;

	/** Base difficulty for generated opponents (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseDifficulty = 0.5f;

	/** Enable rubber banding (AI catches up / slows down) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	bool bEnableRubberBanding = true;

	/** Rubber banding strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RubberBandingStrength = 0.5f;

	// ==========================================
	// ENVIRONMENT
	// ==========================================

	/** Time of day (0-24 hours) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDay = 20.0f; // 8 PM default for that midnight grind vibe

	/** Weather type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	FName WeatherPreset = TEXT("Clear");

	/** Enable dynamic weather changes during race */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	bool bDynamicWeather = false;

	/**
	 * @brief Weather race type for special weather events
	 * Standard = use WeatherPreset, others enable weather-specific bonuses
	 * @see EMGWeatherRaceType
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	FName WeatherRaceType = TEXT("Standard");

	/**
	 * @brief Enable advanced weather racing effects
	 * Activates puddles, aquaplaning, wind effects, and visibility systems
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Environment")
	bool bEnableWeatherRacingEffects = false;

	// ==========================================
	// RESTRICTIONS
	// ==========================================

	/** Minimum performance index allowed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
	float MinPerformanceIndex = 0.0f;

	/** Maximum performance index allowed (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
	float MaxPerformanceIndex = 0.0f;

	/** Required performance class (None = any) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
	EMGPerformanceClass RequiredClass = EMGPerformanceClass::D;

	/** Enforce class restriction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
	bool bEnforceClassRestriction = false;

	/** Allowed drivetrain types (empty = all allowed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Restrictions")
	TArray<EMGDrivetrainType> AllowedDrivetrains;

	// ==========================================
	// REWARDS
	// ==========================================

	/** Race rewards */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	FMGRaceRewardConfig Rewards;

	/** First-time completion bonus credits */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rewards")
	int32 FirstCompletionBonus = 2500;

	// ==========================================
	// UNLOCK REQUIREMENTS
	// ==========================================

	/** Required player level to access */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	int32 RequiredLevel = 1;

	/** Required reputation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	int32 RequiredReputation = 0;

	/** Required previous race completions (RaceID list) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unlock")
	TArray<FName> RequiredCompletedRaces;

	// ==========================================
	// FUNCTIONS
	// ==========================================

	/** Get primary asset ID */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Create a race config struct from this data asset */
	UFUNCTION(BlueprintPure, Category = "Race")
	FMGRaceConfig CreateRaceConfig() const;

	/** Check if player meets requirements */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool CanPlayerAccess(int32 PlayerLevel, int32 PlayerReputation, const TArray<FName>& CompletedRaces) const;

	/** Get display-friendly time of day string */
	UFUNCTION(BlueprintPure, Category = "Race")
	FString GetTimeOfDayString() const;
};

/**
 * Quick Race Preset - Simplified race configuration for instant play
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGQuickRacePreset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	FText PresetName;

	/** Number of laps */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	int32 Laps = 3;

	/** Number of opponents */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	int32 Opponents = 5;

	/** Difficulty (Easy/Medium/Hard) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	float Difficulty = 0.5f;

	/** Race type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preset")
	EMGRaceType RaceType = EMGRaceType::Circuit;
};
