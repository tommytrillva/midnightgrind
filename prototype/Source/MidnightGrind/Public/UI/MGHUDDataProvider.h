// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGHUDDataProvider.generated.h"

class AMGVehiclePawn;

/**
 * Speed display mode
 */
UENUM(BlueprintType)
enum class EMGSpeedDisplayMode : uint8
{
	/** Kilometers per hour */
	KPH,
	/** Miles per hour */
	MPH
};

/**
 * Race HUD data for Blueprint binding
 */
USTRUCT(BlueprintType)
struct FMGRaceHUDData
{
	GENERATED_BODY()

	// ==========================================
	// VEHICLE DATA
	// ==========================================

	/** Current speed in display units */
	UPROPERTY(BlueprintReadOnly)
	float Speed = 0.0f;

	/** Speed display mode */
	UPROPERTY(BlueprintReadOnly)
	EMGSpeedDisplayMode SpeedMode = EMGSpeedDisplayMode::KPH;

	/** Current gear (0 = neutral, -1 = reverse) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentGear = 0;

	/** Maximum gear */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxGear = 6;

	/** Engine RPM */
	UPROPERTY(BlueprintReadOnly)
	float EngineRPM = 0.0f;

	/** Max engine RPM */
	UPROPERTY(BlueprintReadOnly)
	float MaxRPM = 8000.0f;

	/** RPM normalized (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float RPMNormalized = 0.0f;

	/** Is in redline zone */
	UPROPERTY(BlueprintReadOnly)
	bool bInRedline = false;

	/** NOS amount (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float NOSAmount = 1.0f;

	/** Is NOS active */
	UPROPERTY(BlueprintReadOnly)
	bool bNOSActive = false;

	// ==========================================
	// RACE DATA
	// ==========================================

	/** Current position */
	UPROPERTY(BlueprintReadOnly)
	int32 Position = 1;

	/** Total racers */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalRacers = 1;

	/** Current lap */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 1;

	/** Total laps */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalLaps = 3;

	/** Current lap time (seconds) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentLapTime = 0.0f;

	/** Best lap time (seconds) */
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	/** Total race time (seconds) */
	UPROPERTY(BlueprintReadOnly)
	float TotalRaceTime = 0.0f;

	/** Distance to next checkpoint (meters) */
	UPROPERTY(BlueprintReadOnly)
	float DistanceToCheckpoint = 0.0f;

	/** Gap to leader (seconds, 0 if leader) */
	UPROPERTY(BlueprintReadOnly)
	float GapToLeader = 0.0f;

	/** Gap to next (seconds, positive = behind) */
	UPROPERTY(BlueprintReadOnly)
	float GapToNext = 0.0f;

	/** Progress through race (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float RaceProgress = 0.0f;

	// ==========================================
	// SCORING DATA
	// ==========================================

	/** Current drift score */
	UPROPERTY(BlueprintReadOnly)
	int32 DriftScore = 0;

	/** Drift combo multiplier */
	UPROPERTY(BlueprintReadOnly)
	float DriftMultiplier = 1.0f;

	/** Drift combo count */
	UPROPERTY(BlueprintReadOnly)
	int32 DriftCombo = 0;

	/** Is currently drifting */
	UPROPERTY(BlueprintReadOnly)
	bool bIsDrifting = false;

	/** Current drift angle (degrees) */
	UPROPERTY(BlueprintReadOnly)
	float DriftAngle = 0.0f;

	/** Total score */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalScore = 0;

	// ==========================================
	// VEHICLE STATE
	// ==========================================

	/** Damage percentage (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float DamagePercent = 0.0f;

	/** Tire wear percentage (0-1, 0 = new) */
	UPROPERTY(BlueprintReadOnly)
	float TireWear = 0.0f;

	/** Engine temperature (0-1, normalized) */
	UPROPERTY(BlueprintReadOnly)
	float EngineTemp = 0.5f;

	/** Is overheating */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOverheating = false;

	// ==========================================
	// POLICE DATA
	// ==========================================

	/** Current heat level (0-4) */
	UPROPERTY(BlueprintReadOnly)
	int32 HeatLevel = 0;

	/** Is in pursuit */
	UPROPERTY(BlueprintReadOnly)
	bool bInPursuit = false;

	/** Cooldown progress (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float CooldownProgress = 0.0f;

	// ==========================================
	// COUNTDOWN DATA
	// ==========================================

	/** Is countdown active */
	UPROPERTY(BlueprintReadOnly)
	bool bCountdownActive = false;

	/** Countdown value (3, 2, 1, GO) */
	UPROPERTY(BlueprintReadOnly)
	int32 CountdownValue = 0;

	/** Is showing GO */
	UPROPERTY(BlueprintReadOnly)
	bool bShowGo = false;
};

/**
 * Minimap data for Blueprint binding
 */
USTRUCT(BlueprintType)
struct FMGMinimapData
{
	GENERATED_BODY()

	/** Player position */
	UPROPERTY(BlueprintReadOnly)
	FVector2D PlayerPosition = FVector2D::ZeroVector;

	/** Player rotation (degrees) */
	UPROPERTY(BlueprintReadOnly)
	float PlayerRotation = 0.0f;

	/** Other racer positions */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector2D> RacerPositions;

	/** Checkpoint positions */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector2D> CheckpointPositions;

	/** Current checkpoint index */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentCheckpointIndex = 0;

	/** Police positions (if in pursuit) */
	UPROPERTY(BlueprintReadOnly)
	TArray<FVector2D> PolicePositions;

	/** Map scale */
	UPROPERTY(BlueprintReadOnly)
	float MapScale = 1.0f;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHUDDataUpdated, const FMGRaceHUDData&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinimapDataUpdated, const FMGMinimapData&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPositionChanged, int32, OldPosition, int32, NewPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapCompleted, int32, LapNumber, float, LapTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBestLap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinalLap);

/**
 * HUD Data Provider
 * Central data provider for all HUD elements with Blueprint binding
 *
 * Features:
 * - Aggregates data from multiple subsystems
 * - Provides clean Blueprint-bindable structs
 * - Event-driven updates for performance
 * - Formatted display values
 * - Caching and throttling
 */
UCLASS()
class MIDNIGHTGRIND_API UMGHUDDataProvider : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// DATA ACCESS
	// ==========================================

	/**
	 * Get current race HUD data
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Data")
	FMGRaceHUDData GetRaceHUDData() const { return CachedHUDData; }

	/**
	 * Get current minimap data
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Data")
	FMGMinimapData GetMinimapData() const { return CachedMinimapData; }

	/**
	 * Get formatted speed string
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Formatting")
	FText GetFormattedSpeed() const;

	/**
	 * Get formatted lap time
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Formatting")
	FText GetFormattedLapTime(float TimeSeconds) const;

	/**
	 * Get formatted position (1st, 2nd, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Formatting")
	FText GetFormattedPosition(int32 Position) const;

	/**
	 * Get formatted gap
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Formatting")
	FText GetFormattedGap(float GapSeconds) const;

	/**
	 * Get formatted gear
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Formatting")
	FText GetFormattedGear(int32 Gear) const;

	// ==========================================
	// VEHICLE BINDING
	// ==========================================

	/**
	 * Set player vehicle to track
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Vehicle")
	void SetPlayerVehicle(AMGVehiclePawn* Vehicle);

	/**
	 * Clear player vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Vehicle")
	void ClearPlayerVehicle();

	// ==========================================
	// SETTINGS
	// ==========================================

	/**
	 * Set speed display mode
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Settings")
	void SetSpeedDisplayMode(EMGSpeedDisplayMode Mode);

	/**
	 * Get speed display mode
	 */
	UFUNCTION(BlueprintPure, Category = "HUD|Settings")
	EMGSpeedDisplayMode GetSpeedDisplayMode() const { return SpeedDisplayMode; }

	/**
	 * Set HUD update rate
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Settings")
	void SetUpdateRate(float UpdatesPerSecond);

	// ==========================================
	// COUNTDOWN
	// ==========================================

	/**
	 * Start countdown display
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Countdown")
	void StartCountdown(int32 StartValue = 3);

	/**
	 * Show GO
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Countdown")
	void ShowGo();

	/**
	 * End countdown display
	 */
	UFUNCTION(BlueprintCallable, Category = "HUD|Countdown")
	void EndCountdown();

	// ==========================================
	// EVENTS
	// ==========================================

	/** HUD data updated */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnHUDDataUpdated OnHUDDataUpdated;

	/** Minimap data updated */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnMinimapDataUpdated OnMinimapDataUpdated;

	/** Race position changed */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnPositionChanged OnPositionChanged;

	/** Lap completed */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnLapCompleted OnLapCompleted;

	/** Best lap achieved */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnBestLap OnBestLap;

	/** Final lap started */
	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnFinalLap OnFinalLap;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update HUD data */
	void UpdateHUDData();

	/** Update minimap data */
	void UpdateMinimapData();

	/** Gather vehicle data */
	void GatherVehicleData(FMGRaceHUDData& Data);

	/** Gather race data */
	void GatherRaceData(FMGRaceHUDData& Data);

	/** Gather scoring data */
	void GatherScoringData(FMGRaceHUDData& Data);

	/** Gather police data */
	void GatherPoliceData(FMGRaceHUDData& Data);

	/** Convert speed units */
	float ConvertSpeed(float SpeedCmPerSec) const;

	/** Tick function */
	void OnTick();

private:
	/** Cached HUD data */
	FMGRaceHUDData CachedHUDData;

	/** Cached minimap data */
	FMGMinimapData CachedMinimapData;

	/** Player vehicle */
	UPROPERTY()
	TWeakObjectPtr<AMGVehiclePawn> PlayerVehicle;

	/** Speed display mode */
	EMGSpeedDisplayMode SpeedDisplayMode = EMGSpeedDisplayMode::KPH;

	/** Update interval */
	float UpdateInterval = 1.0f / 30.0f; // 30 Hz default

	/** Timer handle */
	FTimerHandle TickTimer;

	/** Previous position for change detection */
	int32 PreviousPosition = 0;

	/** Previous lap for change detection */
	int32 PreviousLap = 0;
};
