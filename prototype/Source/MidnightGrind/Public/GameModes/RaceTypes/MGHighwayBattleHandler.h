// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGHighwayBattleHandler.generated.h"

/**
 * Highway Battle state
 */
UENUM(BlueprintType)
enum class EMGHighwayBattleState : uint8
{
	/** Waiting for start */
	WaitingToStart,
	/** Racing, no one has advantage */
	Racing,
	/** Leader has gap, building advantage */
	BuildingGap,
	/** Decisive gap achieved, counting down */
	DecisiveGap,
	/** Race finished */
	Finished
};

/**
 * Highway Battle participant data
 */
USTRUCT(BlueprintType)
struct FMGHighwayBattleParticipant
{
	GENERATED_BODY()

	/** Vehicle actor */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Vehicle;

	/** Is currently in lead */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLeader = false;

	/** Current distance to opponent */
	UPROPERTY(BlueprintReadOnly)
	float DistanceToOpponent = 0.0f;

	/** Time with decisive gap */
	UPROPERTY(BlueprintReadOnly)
	float TimeWithDecisiveGap = 0.0f;

	/** Current speed (km/h) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentSpeed = 0.0f;

	/** Top speed achieved (km/h) */
	UPROPERTY(BlueprintReadOnly)
	float TopSpeedAchieved = 0.0f;

	/** Total distance traveled */
	UPROPERTY(BlueprintReadOnly)
	float DistanceTraveled = 0.0f;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeadChanged, AActor*, NewLeader, float, GapDistance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDecisiveGapProgress, AActor*, Leader, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGapAchieved, AActor*, Winner, float, FinalGap);

/**
 * Highway Battle Race Handler
 * Implements Wangan-style highway racing as per PRD Section 4.3
 *
 * Victory Condition: Create 200m+ gap for 5 seconds
 * Focus: Top speed, traffic weaving
 *
 * Features:
 * - 2 participants only
 * - Gap-based victory
 * - Real-time distance tracking
 * - Traffic hazards
 * - High-speed focus
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGHighwayBattleHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGHighwayBattleHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE
	// ==========================================

	virtual void InitializeRace(const FMGRaceConfiguration& Config) override;
	virtual void StartRace() override;
	virtual void UpdateRace(float DeltaTime) override;
	virtual void EndRace() override;
	virtual bool IsRaceComplete() const override;
	virtual TArray<FMGRaceResult> GetResults() const override;
	virtual FText GetRaceTypeName() const override;

	// ==========================================
	// HIGHWAY BATTLE SPECIFIC
	// ==========================================

	/**
	 * Get current battle state
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	EMGHighwayBattleState GetBattleState() const { return CurrentState; }

	/**
	 * Get participant data
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	FMGHighwayBattleParticipant GetParticipant(int32 Index) const;

	/**
	 * Get current leader
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	AActor* GetCurrentLeader() const;

	/**
	 * Get current gap distance
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	float GetCurrentGap() const { return CurrentGap; }

	/**
	 * Get decisive gap progress (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	float GetDecisiveGapProgress() const;

	/**
	 * Get required gap for victory
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	float GetRequiredGap() const { return RequiredGap; }

	/**
	 * Get required time with gap for victory
	 */
	UFUNCTION(BlueprintPure, Category = "Highway Battle")
	float GetRequiredGapTime() const { return RequiredGapTime; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Required gap distance for victory (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float RequiredGap = 20000.0f; // 200 meters

	/** Required time with decisive gap (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float RequiredGapTime = 5.0f;

	/** Minimum speed to count gap (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MinimumSpeedForGap = 100.0f;

	/** Maximum race duration (seconds, 0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxRaceDuration = 300.0f; // 5 minutes

	// ==========================================
	// EVENTS
	// ==========================================

	/** Lead changed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLeadChanged OnLeadChanged;

	/** Decisive gap progress update */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDecisiveGapProgress OnDecisiveGapProgress;

	/** Decisive gap achieved, race won */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGapAchieved OnGapAchieved;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update participant data */
	void UpdateParticipants(float DeltaTime);

	/** Update gap and lead */
	void UpdateGap();

	/** Update battle state */
	void UpdateBattleState(float DeltaTime);

	/** Calculate distance between participants */
	float CalculateGapDistance() const;

	/** Determine leader based on direction of travel */
	int32 DetermineLeader() const;

private:
	/** Current battle state */
	EMGHighwayBattleState CurrentState = EMGHighwayBattleState::WaitingToStart;

	/** Participant data (always 2) */
	FMGHighwayBattleParticipant Participants[2];

	/** Current gap distance */
	float CurrentGap = 0.0f;

	/** Current leader index (0 or 1) */
	int32 LeaderIndex = -1;

	/** Time in decisive gap */
	float TimeInDecisiveGap = 0.0f;

	/** Total race time */
	float TotalRaceTime = 0.0f;

	/** Is race complete */
	bool bRaceComplete = false;

	/** Winner index */
	int32 WinnerIndex = -1;

	/** Race start position (for distance tracking) */
	FVector RaceStartPosition = FVector::ZeroVector;

	/** Race direction (forward vector) */
	FVector RaceDirection = FVector::ForwardVector;
};
