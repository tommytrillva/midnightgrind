// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "Environment/MGWeatherRacingEffects.h"
#include "MGWeatherRaceHandler.generated.h"

class UMGWeatherRacingSubsystem;
class UMGWeatherSubsystem;

/**
 * @class UMGWeatherRaceHandler
 * @brief Race type handler for weather-themed race events
 *
 * This handler manages weather-specific race types including:
 * - Rain Race: Wet conditions with puddles and aquaplaning
 * - Midnight Run: Night racing with limited visibility
 * - Fog Rally: Dense fog with reduced perception
 * - Storm Chase: Combined severe weather challenge
 * - Wind Sprint: High-speed wind effects
 *
 * Weather races provide bonus rewards for completing difficult
 * environmental challenges. The handler coordinates between the
 * weather subsystem, racing effects subsystem, and the base
 * race game mode.
 *
 * @see UMGWeatherRacingSubsystem for weather effect management
 * @see EMGWeatherRaceType for available weather race types
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API UMGWeatherRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGWeatherRaceHandler();

	//~ Begin UMGRaceTypeHandler Interface
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;
	virtual void Activate() override;
	virtual void Deactivate() override;
	virtual void Reset() override;
	virtual void OnRaceStarted() override;
	virtual void OnRaceTick(float MGDeltaTime) override;
	virtual void OnRaceEnded() override;
	virtual EMGRaceType GetRaceType() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	virtual int32 CalculateXPForPosition(int32 Position, int32 TotalRacers) const override;
	virtual int32 CalculateReputationEarned(int32 Position, bool bWon) const override;
	//~ End UMGRaceTypeHandler Interface

	// ========================================
	// WEATHER RACE CONFIGURATION
	// ========================================

	/**
	 * @brief Set the weather race type for this event
	 * @param RaceType The weather race type to use
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRace")
	void SetWeatherRaceType(EMGWeatherRaceType RaceType);

	/**
	 * @brief Get the current weather race type
	 * @return Active weather race type
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace")
	EMGWeatherRaceType GetWeatherRaceType() const { return ActiveWeatherRaceType; }

	/**
	 * @brief Apply a custom weather race configuration
	 * @param Config The configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "WeatherRace")
	void ApplyCustomConfig(const FMGWeatherRaceConfig& Config);

	/**
	 * @brief Get the current weather racing effects
	 * @return Current weather effects state
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace")
	FMGWeatherRacingEffects GetCurrentWeatherEffects() const;

	// ========================================
	// WEATHER BONUSES
	// ========================================

	/**
	 * @brief Get the current REP bonus multiplier
	 * @return REP multiplier (1.0 = no bonus)
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Bonuses")
	float GetREPBonusMultiplier() const;

	/**
	 * @brief Get the current cash bonus multiplier
	 * @return Cash multiplier (1.0 = no bonus)
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Bonuses")
	float GetCashBonusMultiplier() const;

	/**
	 * @brief Get the current XP bonus multiplier
	 * @return XP multiplier (1.0 = no bonus)
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Bonuses")
	float GetXPBonusMultiplier() const;

	/**
	 * @brief Get formatted bonus description for UI
	 * @return Text describing all active bonuses
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Bonuses")
	FText GetBonusDescription() const;

	// ========================================
	// CHALLENGE TRACKING
	// ========================================

	/**
	 * @brief Get number of puddles hit during race
	 * @return Puddle hit count
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Stats")
	int32 GetPuddlesHit() const { return PuddlesHit; }

	/**
	 * @brief Get total aquaplaning time in seconds
	 * @return Time spent aquaplaning
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Stats")
	float GetAquaplaningTime() const { return TotalAquaplaningTime; }

	/**
	 * @brief Get maximum wind gust survived without crashing
	 * @return Maximum gust intensity survived (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Stats")
	float GetMaxGustSurvived() const { return MaxGustSurvived; }

	/**
	 * @brief Check if race was completed without aquaplaning
	 * @return True if clean (no aquaplaning)
	 */
	UFUNCTION(BlueprintPure, Category = "WeatherRace|Stats")
	bool WasCleanRun() const { return TotalAquaplaningTime < 0.5f; }

protected:
	// ========================================
	// CONFIGURATION
	// ========================================

	/** Active weather race type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	EMGWeatherRaceType ActiveWeatherRaceType = EMGWeatherRaceType::Standard;

	/** Custom configuration (if applied) */
	UPROPERTY()
	FMGWeatherRaceConfig CustomConfig;

	/** Whether custom config is being used */
	UPROPERTY()
	bool bUsingCustomConfig = false;

	/** Base race type this weather race wraps (defaults to Circuit) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	EMGRaceType BaseRaceType = EMGRaceType::Circuit;

	// ========================================
	// RUNTIME STATE
	// ========================================

	/** Reference to weather racing subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGWeatherRacingSubsystem> WeatherRacingSubsystem;

	/** Reference to base weather subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGWeatherSubsystem> WeatherSubsystem;

	/** Number of puddles hit by player */
	UPROPERTY()
	int32 PuddlesHit = 0;

	/** Total time spent aquaplaning */
	UPROPERTY()
	float TotalAquaplaningTime = 0.0f;

	/** Maximum gust intensity survived */
	UPROPERTY()
	float MaxGustSurvived = 0.0f;

	/** Was player aquaplaning last frame */
	UPROPERTY()
	bool bWasAquaplaning = false;

	// ========================================
	// INTERNAL METHODS
	// ========================================

	/** Initialize subsystem references */
	void InitializeSubsystems();

	/** Update weather effects each tick */
	void UpdateWeatherEffects(float MGDeltaTime);

	/** Track aquaplaning statistics */
	void TrackAquaplaning(float MGDeltaTime);

	/** Track wind gust statistics */
	void TrackWindGusts();

	/** Update AI controllers for weather */
	void UpdateAIForWeather();

	/** Called when puddle is hit */
	UFUNCTION()
	void OnPuddleHit(AActor* Vehicle, const FMGPuddleInstance& Puddle);

	/** Called when wind gust occurs */
	UFUNCTION()
	void OnWindGustOccurred(float Intensity, FVector Direction);
};
