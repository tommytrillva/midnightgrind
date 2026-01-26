// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGRaceCountdownManager.generated.h"

/**
 * Countdown state
 */
UENUM(BlueprintType)
enum class EMGCountdownState : uint8
{
	/** Not active */
	Inactive,
	/** Pre-countdown delay */
	PreDelay,
	/** Counting down */
	Counting,
	/** Showing GO */
	Go,
	/** Complete */
	Complete
};

/**
 * Countdown visual style
 */
UENUM(BlueprintType)
enum class EMGCountdownStyle : uint8
{
	/** Classic 3-2-1-GO */
	Classic,
	/** Traffic lights (red-red-red-green) */
	TrafficLights,
	/** Christmas tree (drag racing) */
	ChristmasTree,
	/** Wangan style (minimal) */
	Wangan,
	/** Custom/Blueprint */
	Custom
};

/**
 * Countdown tick data
 */
USTRUCT(BlueprintType)
struct FMGCountdownTick
{
	GENERATED_BODY()

	/** Current value (3, 2, 1, 0=GO) */
	UPROPERTY(BlueprintReadOnly)
	int32 Value = 0;

	/** Time remaining in this tick */
	UPROPERTY(BlueprintReadOnly)
	float TimeRemaining = 0.0f;

	/** Progress through this tick (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Progress = 0.0f;

	/** Is this the GO tick */
	UPROPERTY(BlueprintReadOnly)
	bool bIsGo = false;

	/** Total countdown elapsed */
	UPROPERTY(BlueprintReadOnly)
	float TotalElapsed = 0.0f;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownGo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownCancelled);

/**
 * Race Countdown Manager
 * Handles race start countdown with events and visual styles
 *
 * Features:
 * - Multiple countdown styles
 * - Pre-delay before countdown
 * - Per-tick events for audio/visual sync
 * - Network synchronized
 * - Blueprint extensible
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceCountdownManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// COUNTDOWN CONTROL
	// ==========================================

	/**
	 * Start countdown
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void StartCountdown(int32 FromValue = 3, EMGCountdownStyle Style = EMGCountdownStyle::Classic);

	/**
	 * Start countdown with custom timing
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void StartCountdownCustom(int32 FromValue, float PreDelaySeconds, float TickDuration, float GoDuration);

	/**
	 * Cancel countdown
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void CancelCountdown();

	/**
	 * Pause countdown
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void PauseCountdown();

	/**
	 * Resume countdown
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void ResumeCountdown();

	/**
	 * Skip to GO
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void SkipToGo();

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * Get current state
	 */
	UFUNCTION(BlueprintPure, Category = "Countdown|State")
	EMGCountdownState GetState() const { return CurrentState; }

	/**
	 * Is countdown active
	 */
	UFUNCTION(BlueprintPure, Category = "Countdown|State")
	bool IsCountdownActive() const { return CurrentState != EMGCountdownState::Inactive && CurrentState != EMGCountdownState::Complete; }

	/**
	 * Is paused
	 */
	UFUNCTION(BlueprintPure, Category = "Countdown|State")
	bool IsPaused() const { return bIsPaused; }

	/**
	 * Get current countdown value
	 */
	UFUNCTION(BlueprintPure, Category = "Countdown|State")
	int32 GetCurrentValue() const { return CurrentValue; }

	/**
	 * Get countdown tick data
	 */
	UFUNCTION(BlueprintPure, Category = "Countdown|State")
	FMGCountdownTick GetTickData() const;

	/**
	 * Get countdown style
	 */
	UFUNCTION(BlueprintPure, Category = "Countdown|State")
	EMGCountdownStyle GetStyle() const { return CurrentStyle; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/**
	 * Set pre-delay duration
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown|Config")
	void SetPreDelay(float Seconds) { PreDelayDuration = Seconds; }

	/**
	 * Set tick duration
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown|Config")
	void SetTickDuration(float Seconds) { TickDuration = Seconds; }

	/**
	 * Set GO duration
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown|Config")
	void SetGoDuration(float Seconds) { GoDuration = Seconds; }

	/**
	 * Enable/disable sound
	 */
	UFUNCTION(BlueprintCallable, Category = "Countdown|Config")
	void SetSoundEnabled(bool bEnabled) { bSoundEnabled = bEnabled; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Countdown started */
	UPROPERTY(BlueprintAssignable, Category = "Countdown|Events")
	FOnCountdownStarted OnCountdownStarted;

	/** Countdown tick (3, 2, 1) */
	UPROPERTY(BlueprintAssignable, Category = "Countdown|Events")
	FOnCountdownTick OnCountdownTick;

	/** GO! */
	UPROPERTY(BlueprintAssignable, Category = "Countdown|Events")
	FOnCountdownGo OnCountdownGo;

	/** Countdown complete */
	UPROPERTY(BlueprintAssignable, Category = "Countdown|Events")
	FOnCountdownComplete OnCountdownComplete;

	/** Countdown cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Countdown|Events")
	FOnCountdownCancelled OnCountdownCancelled;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update countdown */
	void UpdateCountdown(float DeltaTime);

	/** Transition to next tick */
	void NextTick();

	/** Enter GO state */
	void EnterGoState();

	/** Complete countdown */
	void CompleteCountdown();

	/** Play tick sound */
	void PlayTickSound(int32 Value);

	/** Play GO sound */
	void PlayGoSound();

	/** Set state */
	void SetState(EMGCountdownState NewState);

	/** Tick function */
	void OnTick();

private:
	/** Current state */
	EMGCountdownState CurrentState = EMGCountdownState::Inactive;

	/** Current style */
	EMGCountdownStyle CurrentStyle = EMGCountdownStyle::Classic;

	/** Current countdown value */
	int32 CurrentValue = 0;

	/** Start value */
	int32 StartValue = 3;

	/** Current tick timer */
	float CurrentTickTimer = 0.0f;

	/** Total elapsed time */
	float TotalElapsedTime = 0.0f;

	/** Pre-delay duration */
	float PreDelayDuration = 0.5f;

	/** Tick duration (time per number) */
	float TickDuration = 1.0f;

	/** GO display duration */
	float GoDuration = 1.0f;

	/** Is paused */
	bool bIsPaused = false;

	/** Sound enabled */
	bool bSoundEnabled = true;

	/** Timer handle */
	FTimerHandle TickTimer;
};
