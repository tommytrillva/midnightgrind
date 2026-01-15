// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/MGRacingAIController.h"
#include "MGAIRaceManager.generated.h"

class AMGVehiclePawn;
class AMGTrackSpline;
class AMGRacingAIController;
class UMGVehicleModelData;

/**
 * Configuration for spawning an AI opponent
 */
USTRUCT(BlueprintType)
struct FMGAIOpponentConfig
{
	GENERATED_BODY()

	/** Vehicle model to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	TSoftObjectPtr<UMGVehicleModelData> VehicleModel;

	/** Driver profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	FMGAIDriverProfile DriverProfile;

	/** Starting grid position (0 = pole) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	int32 GridPosition = 0;

	/** Custom vehicle pawn class override (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	TSubclassOf<AMGVehiclePawn> VehiclePawnClass;
};

/**
 * Data for an active AI opponent in the race
 */
USTRUCT(BlueprintType)
struct FMGActiveAIOpponent
{
	GENERATED_BODY()

	/** Unique ID */
	UPROPERTY(BlueprintReadOnly)
	int32 OpponentId = -1;

	/** The spawned vehicle pawn */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMGVehiclePawn> VehiclePawn;

	/** The AI controller */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMGRacingAIController> AIController;

	/** Driver profile being used */
	UPROPERTY(BlueprintReadOnly)
	FMGAIDriverProfile DriverProfile;

	/** Current race position */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPosition = 0;

	/** Current lap */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 0;

	/** Distance along track */
	UPROPERTY(BlueprintReadOnly)
	float TrackDistance = 0.0f;

	/** Total race distance (track length * laps + current distance) */
	UPROPERTY(BlueprintReadOnly)
	float TotalRaceDistance = 0.0f;

	/** Has finished race */
	UPROPERTY(BlueprintReadOnly)
	bool bFinished = false;

	/** Finish time */
	UPROPERTY(BlueprintReadOnly)
	float FinishTime = 0.0f;

	bool IsValid() const { return VehiclePawn != nullptr && AIController != nullptr; }
};

/**
 * Configuration for the AI race manager
 */
USTRUCT(BlueprintType)
struct FMGAIRaceManagerConfig
{
	GENERATED_BODY()

	/** Global rubber-banding settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Manager")
	FMGRubberBandingConfig RubberBandingConfig;

	/** Default AI controller class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Manager")
	TSubclassOf<AMGRacingAIController> AIControllerClass;

	/** Update frequency for position calculations (Hz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Manager")
	float PositionUpdateRate = 10.0f;
};

/** Delegate for AI opponent events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIOpponentFinished, int32, OpponentId, float, FinishTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIPositionChanged, int32, OpponentId, int32, OldPosition, int32, NewPosition);

/**
 * Manages AI opponents in a race
 * Attach to GameMode to spawn and control AI racers
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGAIRaceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGAIRaceManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Set the track spline for AI navigation */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetTrackSpline(AMGTrackSpline* InTrackSpline);

	/** Set the race configuration */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetConfiguration(const FMGAIRaceManagerConfig& InConfig);

	/** Set the track length and lap count for position tracking */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetRaceParameters(float InTrackLength, int32 InTotalLaps);

	// ==========================================
	// AI SPAWNING
	// ==========================================

	/** Spawn a single AI opponent */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	int32 SpawnAIOpponent(const FMGAIOpponentConfig& Config, const FTransform& SpawnTransform);

	/** Spawn multiple AI opponents from configuration array */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<int32> SpawnAIOpponents(const TArray<FMGAIOpponentConfig>& Configs, const TArray<FTransform>& SpawnTransforms);

	/** Generate AI opponents for difficulty level */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<int32> GenerateAIField(int32 OpponentCount, EMGAIDifficulty BaseDifficulty, const TArray<FTransform>& SpawnTransforms);

	/** Remove an AI opponent */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void RemoveAIOpponent(int32 OpponentId);

	/** Remove all AI opponents */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void RemoveAllAIOpponents();

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/** Initialize all AI for race start */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void InitializeForRace();

	/** Start all AI racing */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void StartRacing();

	/** Stop all AI racing */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void StopRacing();

	/** Pause/resume all AI */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetAllPaused(bool bPaused);

	/** Notify AI that a lap was completed */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void OnAILapCompleted(int32 OpponentId);

	/** Notify AI that they finished the race */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void OnAIFinished(int32 OpponentId, float FinishTime);

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get all active AI opponents */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<FMGActiveAIOpponent> GetAllOpponents() const;

	/** Get a specific opponent by ID */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	bool GetOpponent(int32 OpponentId, FMGActiveAIOpponent& OutOpponent) const;

	/** Get opponent count */
	UFUNCTION(BlueprintPure, Category = "AI Race Manager")
	int32 GetOpponentCount() const { return ActiveOpponents.Num(); }

	/** Get opponents sorted by position */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<FMGActiveAIOpponent> GetOpponentsByPosition() const;

	/** Include player vehicle in position calculations */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetPlayerVehicle(AMGVehiclePawn* PlayerVehicle, int32 PlayerLap, float PlayerTrackDistance);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when an AI finishes the race */
	UPROPERTY(BlueprintAssignable, Category = "AI Race Manager|Events")
	FOnAIOpponentFinished OnAIOpponentFinished;

	/** Called when an AI's position changes */
	UPROPERTY(BlueprintAssignable, Category = "AI Race Manager|Events")
	FOnAIPositionChanged OnAIPositionChanged;

protected:
	/** Update positions for all opponents */
	void UpdatePositions();

	/** Update an opponent's track distance */
	void UpdateOpponentTrackDistance(FMGActiveAIOpponent& Opponent);

	/** Calculate total race distance */
	float CalculateTotalRaceDistance(int32 Lap, float TrackDistance) const;

	/** Find available opponent ID */
	int32 GetNextOpponentId();

	// ==========================================
	// DATA
	// ==========================================

	/** Track spline for navigation */
	UPROPERTY()
	TObjectPtr<AMGTrackSpline> TrackSpline;

	/** Manager configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FMGAIRaceManagerConfig Configuration;

	/** Active AI opponents */
	UPROPERTY()
	TArray<FMGActiveAIOpponent> ActiveOpponents;

	/** Track length in cm */
	UPROPERTY()
	float TrackLength = 0.0f;

	/** Total laps in race */
	UPROPERTY()
	int32 TotalLaps = 3;

	/** Is race active? */
	UPROPERTY()
	bool bRaceActive = false;

	/** Time since last position update */
	float PositionUpdateTimer = 0.0f;

	/** Next opponent ID to assign */
	int32 NextOpponentId = 1;

	/** Player vehicle for position tracking */
	UPROPERTY()
	TObjectPtr<AMGVehiclePawn> PlayerVehicle;

	/** Player's current lap */
	int32 PlayerLap = 0;

	/** Player's track distance */
	float PlayerTrackDistance = 0.0f;
};
