// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGTypes.h"
#include "MGSinglePlayerRaceMode.generated.h"

class AMGAIRacerController;
class AMGVehiclePawn;
class UMGRacingLineGenerator;
class UMGPlayerProgression;

/**
 * Streamlined Single-Player Race Mode
 * 
 * Focused on FAST LOADING and FUN RACING:
 * - Only essential subsystems loaded
 * - AI opponents with varied personalities
 * - Instant race starts
 * - Basic progression (unlock cars, upgrade parts)
 * 
 * Design Philosophy:
 * - Get player racing in under 3 seconds
 * - Make AI feel alive and competitive
 * - Keep progression simple but satisfying
 * - Polish over perfection
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGSinglePlayerRaceMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMGSinglePlayerRaceMode();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	//~ End AActor Interface

	//~ Begin AGameModeBase Interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	//~ End AGameModeBase Interface

	// ============================================
	// RACE MANAGEMENT
	// ============================================

	/** Start countdown for race */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void StartRaceCountdown();

	/** Begin race (called after countdown) */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void StartRace();

	/** Finish race for a vehicle */
	UFUNCTION(BlueprintCallable, Category = "Race")
	void FinishRace(AMGVehiclePawn* Vehicle, float RaceTime);

	/** Check if race is active */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsRaceActive() const { return bRaceActive; }

	/** Get countdown time remaining */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetCountdownRemaining() const { return CountdownRemaining; }

	/** Get race time elapsed */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetRaceTimeElapsed() const { return RaceTimeElapsed; }

	// ============================================
	// AI OPPONENTS
	// ============================================

	/** Spawn AI opponents for the race */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SpawnAIOpponents(int32 NumOpponents = 5);

	/** Get all AI racers */
	UFUNCTION(BlueprintPure, Category = "AI")
	TArray<AMGAIRacerController*> GetAIRacers() const { return AIRacers; }

	/** Get current race standings */
	UFUNCTION(BlueprintPure, Category = "AI")
	TArray<AMGVehiclePawn*> GetRaceStandings() const;

	// ============================================
	// PROGRESSION
	// ============================================

	/** Award rewards for race finish */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AwardRaceRewards(int32 Position, float RaceTime);

	/** Get player progression */
	UFUNCTION(BlueprintPure, Category = "Progression")
	UMGPlayerProgression* GetPlayerProgression() const { return PlayerProgression; }

	// ============================================
	// EVENTS
	// ============================================

	/** Event fired when countdown starts */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceCountdownStart OnRaceCountdownStart;

	/** Event fired when race begins */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceStart OnRaceStart;

	/** Event fired when race ends */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceFinish OnRaceFinish;

	/** Event fired when player finishes */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnPlayerFinish OnPlayerFinish;

protected:
	// ============================================
	// RACE STATE
	// ============================================

	/** Is race currently active? */
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	bool bRaceActive;

	/** Is countdown active? */
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	bool bCountdownActive;

	/** Countdown time remaining */
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	float CountdownRemaining;

	/** Race time elapsed since start */
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	float RaceTimeElapsed;

	/** Number of laps for the race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 NumLaps;

	// ============================================
	// AI CONFIGURATION
	// ============================================

	/** AI vehicle classes to spawn (randomized) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<TSubclassOf<AMGVehiclePawn>> AIVehicleClasses;

	/** AI driver personalities (varied difficulty/style) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<FString> AIPersonalities;

	/** Active AI racers */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TArray<AMGAIRacerController*> AIRacers;

	/** Active AI vehicles */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TArray<AMGVehiclePawn*> AIVehicles;

	// ============================================
	// PROGRESSION
	// ============================================

	/** Player progression component */
	UPROPERTY(BlueprintReadOnly, Category = "Progression")
	UMGPlayerProgression* PlayerProgression;

	/** Cash reward for 1st place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 FirstPlaceCash;

	/** Cash reward for 2nd place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 SecondPlaceCash;

	/** Cash reward for 3rd place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 ThirdPlaceCash;

	/** Base cash for completing race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
	int32 CompletionCash;

	// ============================================
	// INTERNAL HELPERS
	// ============================================

	/** Spawn single AI racer */
	AMGAIRacerController* SpawnAIRacer(int32 Index);

	/** Update race standings */
	void UpdateRaceStandings(float InDeltaTime);

	/** Calculate track progress for a vehicle */
	float CalculateTrackProgress(AMGVehiclePawn* Vehicle) const;
};

// ============================================
// EVENT DELEGATES
// ============================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceCountdownStart, float, CountdownDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceFinish, AMGVehiclePawn*, Winner, float, WinTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerFinish, int32, Position, float, RaceTime);
