// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGHighwayBattleHandler.h
 * @brief Highway Battle Handler - Wangan-style high-speed gap racing.
 *
 * Highway Battles are inspired by Japanese highway racing culture, particularly
 * the "Wangan" (bayshore) racing scene. Two drivers compete at extreme speeds
 * on long highway stretches, with victory determined by creating and maintaining
 * a decisive gap rather than crossing a finish line.
 *
 * @section overview_highway Overview
 * Unlike traditional circuit racing, Highway Battles have no fixed distance or
 * finish line. The race continues until one driver creates a gap of 200+ meters
 * and holds it for 5 seconds. This creates intense, sustained high-speed racing
 * where maintaining top speed and navigating traffic are paramount.
 *
 * @section concepts_highway Key Concepts for Beginners
 *
 * 1. **Gap-Based Victory**: Instead of racing to a finish line, you win by
 *    "losing" your opponent - creating enough distance that they can't catch up.
 *    The RequiredGap (200m) and RequiredGapTime (5s) define victory conditions.
 *
 * 2. **Battle States**: The race progresses through states:
 *    - WaitingToStart: Pre-race positioning
 *    - Racing: Normal racing, no significant gap
 *    - BuildingGap: One driver is pulling ahead
 *    - DecisiveGap: Gap threshold reached, countdown to victory
 *    - Finished: Winner determined
 *
 * 3. **Minimum Speed Requirement**: The leader must maintain MinimumSpeedForGap
 *    (100 km/h) for the gap to count. You can't win by stopping while your
 *    opponent crashes - you must actually outrun them.
 *
 * 4. **Top Speed Focus**: Unlike technical tracks, highways reward raw speed.
 *    Vehicle tuning should prioritize top speed and high-speed stability over
 *    acceleration or cornering.
 *
 * @section win_highway Win Condition
 * Create a gap of RequiredGap (default 200 meters) and maintain it for
 * RequiredGapTime (default 5 seconds) while traveling above MinimumSpeedForGap.
 *
 * Alternative endings:
 * - Opponent crashes: Automatic victory
 * - Time limit reached (MaxRaceDuration): Closest gap wins
 * - Both crash: Race voided
 *
 * @section traffic_highway Traffic System
 * Highway battles typically include AI traffic that both racers must navigate.
 * Traffic adds unpredictability and tests drivers' ability to find gaps at
 * high speed. Collision with traffic usually results in significant speed loss,
 * making it critical to maintain clear sight lines.
 *
 * @section ui_highway UI Elements
 * - Current speed (km/h)
 * - Gap distance to opponent (positive = you're ahead)
 * - Decisive gap progress bar (fills during 5s countdown)
 * - Top speed achieved
 * - Total distance traveled
 *
 * @section example_highway Example Configuration
 *
 * @code
 * // Create and configure a Highway Battle:
 * UMGHighwayBattleHandler* Handler = NewObject<UMGHighwayBattleHandler>();
 *
 * // Customize victory conditions
 * Handler->RequiredGap = 30000.0f;       // 300 meters for harder races
 * Handler->RequiredGapTime = 7.0f;       // Hold for 7 seconds
 * Handler->MinimumSpeedForGap = 150.0f;  // Must be going 150+ km/h
 * Handler->MaxRaceDuration = 600.0f;     // 10 minute time limit
 *
 * // Initialize and start
 * Handler->InitializeRace(Config);
 * Handler->StartRace();
 *
 * // Subscribe to events
 * Handler->OnLeadChanged.AddDynamic(this, &UMyClass::OnLeadSwitch);
 * Handler->OnDecisiveGapProgress.AddDynamic(this, &UMyClass::UpdateGapUI);
 * Handler->OnGapAchieved.AddDynamic(this, &UMyClass::ShowVictory);
 * @endcode
 *
 * @section history_highway Cultural Background
 * Highway battles were popularized by the manga/anime "Wangan Midnight" which
 * depicted illegal racing on Tokyo's Shuto Expressway. The format emphasizes
 * the thrill of sustained high-speed driving and the psychological battle of
 * trying to shake off a pursuer at 300+ km/h.
 *
 * @see UMGRaceTypeHandler Base class for all race type handlers
 * @see UMGTougeHandler For technical mountain pass racing
 * @see PRD Section 4.3 For design requirements
 */

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
