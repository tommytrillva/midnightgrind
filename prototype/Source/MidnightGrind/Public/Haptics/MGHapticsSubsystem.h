// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGHapticsSubsystem.h
 * @brief Controller Haptic Feedback System for immersive racing feedback
 *
 * @section Overview
 * This file defines the haptic feedback (vibration/rumble) system for Midnight Grind.
 * Haptic feedback creates physical sensations in the player's controller to enhance
 * immersion - you feel the engine rumble, collisions shake the controller, and
 * different road surfaces create distinct vibration patterns.
 *
 * @section WhatIsHaptic What is Haptic Feedback?
 * Haptic feedback refers to any technology that creates touch sensations. In gaming:
 *   - Basic rumble: Simple motor vibration (Xbox/PlayStation controllers)
 *   - HD rumble: More precise vibrations that can simulate textures
 *   - Adaptive triggers: Resistance in L2/R2 triggers (PS5 DualSense only)
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * @subsection GameInstanceSubsystem 1. Game Instance Subsystem
 * This class inherits from UGameInstanceSubsystem, meaning:
 *   - One instance exists for the entire game session
 *   - It persists across level loads (unlike World subsystems)
 *   - Access it via: GetGameInstance()->GetSubsystem<UMGHapticsSubsystem>()
 *
 * @subsection HapticPatterns 2. Haptic Patterns (FMGHapticPattern)
 * A pattern defines how vibration changes over time using arrays:
 *   - Amplitudes[]: How strong the vibration is (0.0 = off, 1.0 = max)
 *   - Frequencies[]: How fast the vibration oscillates (higher = buzzier)
 *   - Durations[]: How long each step lasts in seconds
 *
 * Example: A collision might be high amplitude briefly, then fade out.
 *
 * @subsection PrioritySystem 3. Priority System
 * Multiple haptic effects can play simultaneously, but controllers have limits.
 * Priority (0-10) determines which effects play when at capacity:
 *   - Collision = high priority (9) - always felt
 *   - Surface texture = low priority (2) - dropped if busy
 *
 * @subsection Channels 4. Channels (EMGHapticChannel)
 * Controllers have left and right motors. Effects can target:
 *   - Both: Full controller vibration
 *   - LeftOnly/RightOnly: Directional feedback (e.g., left-side collision)
 *
 * @subsection AdaptiveTriggers 5. Adaptive Triggers (PS5 DualSense)
 * The FMGTriggerFeedback struct controls resistance in the trigger buttons:
 *   - Resistance: How hard to push (simulates brake pressure)
 *   - StartPosition: Where resistance begins (0.0 = top, 1.0 = bottom)
 *   - Frequency/Strength: For vibration effects in the trigger
 *
 * Note: Falls back to regular haptics on non-DualSense controllers.
 *
 * @subsection SurfaceTypes 6. Surface Types (EMGSurfaceType)
 * Different driving surfaces create unique haptic signatures:
 *   - Asphalt: Smooth, low-frequency hum
 *   - Gravel: High-frequency, irregular rumble
 *   - Rumblestrip: Rapid pulsing (like real rumble strips)
 *
 * @section CodeExamples Code Examples
 *
 * @subsection GettingSubsystem Getting the Subsystem
 * @code
 * // From any UObject with access to GameInstance
 * UMGHapticsSubsystem* Haptics = GetGameInstance()->GetSubsystem<UMGHapticsSubsystem>();
 * @endcode
 *
 * @subsection OneShotEffects Playing One-Shot Effects (collision, checkpoint)
 * @code
 * // Simple haptic trigger
 * HapticsSubsystem->PlayHaptic(EMGHapticType::Collision, 0.8f);
 *
 * // With priority
 * FGuid HapticId = HapticsSubsystem->PlayHaptic(EMGHapticType::Collision, 0.8f, 9);
 *
 * // Stop it early if needed
 * HapticsSubsystem->StopHaptic(HapticId);
 * @endcode
 *
 * @subsection ContinuousFeedback Continuous Feedback (engine rumble)
 * @code
 * // Called every frame from vehicle code
 * void AVehicle::Tick(float DeltaTime)
 * {
 *     HapticsSubsystem->UpdateEngineRPM(CurrentRPM, MaxRPM);
 *     HapticsSubsystem->UpdateSpeed(GetVehicleSpeedKPH());
 * }
 * @endcode
 *
 * @subsection SurfaceChanges Surface Changes (detected by tire trace)
 * @code
 * void AVehicle::OnSurfaceChanged(EMGSurfaceType NewSurface)
 * {
 *     HapticsSubsystem->SetCurrentSurface(NewSurface);
 * }
 * @endcode
 *
 * @subsection TriggerFeedback Adaptive Triggers (brake pedal feel)
 * @code
 * // Set brake trigger resistance based on brake pressure
 * HapticsSubsystem->SetBrakeTrigger(BrakePressure, 0.1f);
 *
 * // Set throttle trigger with engine vibration
 * HapticsSubsystem->SetThrottleTrigger(0.2f, EngineFrequency);
 * @endcode
 *
 * @subsection EventListening Listening for Events
 * @code
 * // In your class setup
 * HapticsSubsystem->OnSurfaceChanged.AddDynamic(this, &AMyClass::HandleSurfaceChange);
 *
 * void AMyClass::HandleSurfaceChange(EMGSurfaceType NewSurface)
 * {
 *     // Update UI or audio based on surface
 * }
 * @endcode
 *
 * @section WheelIntegration Racing Wheel Integration
 * When a racing wheel is connected, haptic feedback can be routed to the wheel's
 * Force Feedback (FFB) motors via RouteToWheelFFB(). This provides more realistic
 * steering feel compared to controller vibration.
 *
 * @section Performance Performance Notes
 *   - MaxConcurrentHaptics limits simultaneous effects (default: 4)
 *   - Low battery detection reduces intensity to save power
 *   - UpdateActiveHaptics() runs on a timer, not every frame
 *
 * @see UMGRacingWheelSubsystem for wheel force feedback integration
 * @see EMGHapticType for all supported haptic event types
 *
 * @author Midnight Grind Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGHapticsSubsystem.generated.h"

class UMGRacingWheelSubsystem;

UENUM(BlueprintType)
enum class EMGHapticType : uint8
{
	None,
	EngineIdle,
	EngineRevving,
	Acceleration,
	Braking,
	Collision,
	SurfaceChange,
	Drift,
	Boost,
	LandingImpact,
	GearShift,
	RedlineWarning,
	Damage,
	NitroActivate,
	NitroSustain,
	CheckpointPass,
	LapComplete,
	RaceFinish,
	CountdownTick,
	CountdownGo
};

UENUM(BlueprintType)
enum class EMGHapticChannel : uint8
{
	Both,
	LeftOnly,
	RightOnly
};

UENUM(BlueprintType)
enum class EMGSurfaceType : uint8
{
	Asphalt,
	Concrete,
	Gravel,
	Dirt,
	Grass,
	Sand,
	Wet,
	Ice,
	Metal,
	Rumblestrip
};

UENUM(BlueprintType)
enum class EMGTriggerEffect : uint8
{
	None,
	Resistance,
	Vibration,
	Section,
	Bow,
	Galloping,
	Machine
};

USTRUCT(BlueprintType)
struct FMGHapticPattern
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PatternID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHapticType HapticType = EMGHapticType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Amplitudes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Frequencies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Durations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHapticChannel Channel = EMGHapticChannel::Both;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoopInterval = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGTriggerFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTriggerEffect LeftEffect = EMGTriggerEffect::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTriggerEffect RightEffect = EMGTriggerEffect::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeftResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RightResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeftStartPosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RightStartPosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeftFrequency = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RightFrequency = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeftStrength = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RightStrength = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGActiveHaptic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid HapticID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGHapticPattern Pattern;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStep = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StepTimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IntensityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPaused = false;
};

USTRUCT(BlueprintType)
struct FMGSurfaceFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSurfaceType SurfaceType = EMGSurfaceType::Asphalt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseFrequency = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseAmplitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsSteering = false;
};

USTRUCT(BlueprintType)
struct FMGHapticsConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineVibrationIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAdaptiveTriggers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBrakeTriggerFeedback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bThrottleTriggerFeedback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxConcurrentHaptics = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReduceOnLowBattery = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowBatteryThreshold = 0.2f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHapticStarted, FGuid, HapticID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHapticStopped, FGuid, HapticID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerFeedbackChanged, const FMGTriggerFeedback&, Feedback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurfaceChanged, EMGSurfaceType, NewSurface);

UCLASS()
class MIDNIGHTGRIND_API UMGHapticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Basic Haptics
	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	FGuid PlayHaptic(EMGHapticType Type, float Intensity = 1.0f, int32 Priority = 0);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	FGuid PlayHapticPattern(const FMGHapticPattern& Pattern, float IntensityMultiplier = 1.0f, int32 Priority = 0);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	FGuid PlayHapticPulse(float Intensity, float Duration, EMGHapticChannel Channel = EMGHapticChannel::Both);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	void StopHaptic(FGuid HapticID);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	void StopAllHaptics();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	void PauseHaptic(FGuid HapticID);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Play")
	void ResumeHaptic(FGuid HapticID);

	// Engine Feedback
	UFUNCTION(BlueprintCallable, Category = "Haptics|Engine")
	void UpdateEngineRPM(float RPM, float MaxRPM);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Engine")
	void OnGearShift(bool bUpshift);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Engine")
	void OnRedline(bool bInRedline);

	// Vehicle State
	UFUNCTION(BlueprintCallable, Category = "Haptics|Vehicle")
	void UpdateSpeed(float SpeedKPH);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Vehicle")
	void OnCollision(float ImpactForce, FVector ImpactDirection);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Vehicle")
	void OnLanding(float ImpactForce);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Vehicle")
	void OnDriftStart();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Vehicle")
	void OnDriftEnd();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Vehicle")
	void UpdateDriftAngle(float Angle);

	// Boost/Nitro
	UFUNCTION(BlueprintCallable, Category = "Haptics|Boost")
	void OnBoostActivate();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Boost")
	void OnBoostDeactivate();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Boost")
	void UpdateBoostIntensity(float Intensity);

	// Surface Feedback
	UFUNCTION(BlueprintCallable, Category = "Haptics|Surface")
	void SetCurrentSurface(EMGSurfaceType Surface);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Surface")
	void UpdateSurfaceFeedback(float SpeedMultiplier);

	UFUNCTION(BlueprintPure, Category = "Haptics|Surface")
	EMGSurfaceType GetCurrentSurface() const { return CurrentSurface; }

	// Adaptive Triggers (PS5 DualSense)
	UFUNCTION(BlueprintCallable, Category = "Haptics|Triggers")
	void SetTriggerFeedback(const FMGTriggerFeedback& Feedback);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Triggers")
	void SetBrakeTrigger(float Resistance, float StartPosition = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Triggers")
	void SetThrottleTrigger(float Resistance, float Frequency = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Triggers")
	void ClearTriggerFeedback();

	UFUNCTION(BlueprintPure, Category = "Haptics|Triggers")
	FMGTriggerFeedback GetCurrentTriggerFeedback() const { return CurrentTriggerFeedback; }

	// Race Events
	UFUNCTION(BlueprintCallable, Category = "Haptics|Race")
	void OnCheckpointPassed();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Race")
	void OnLapCompleted();

	UFUNCTION(BlueprintCallable, Category = "Haptics|Race")
	void OnRaceFinished(int32 Position);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Race")
	void OnCountdownTick(int32 TickNumber);

	UFUNCTION(BlueprintCallable, Category = "Haptics|Race")
	void OnCountdownGo();

	// Patterns
	UFUNCTION(BlueprintCallable, Category = "Haptics|Patterns")
	void RegisterPattern(const FMGHapticPattern& Pattern);

	UFUNCTION(BlueprintPure, Category = "Haptics|Patterns")
	FMGHapticPattern GetPattern(FName PatternID) const;

	UFUNCTION(BlueprintPure, Category = "Haptics|Patterns")
	TArray<FMGHapticPattern> GetAllPatterns() const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Haptics|Config")
	void SetConfig(const FMGHapticsConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Haptics|Config")
	FMGHapticsConfig GetConfig() const { return Config; }

	UFUNCTION(BlueprintCallable, Category = "Haptics|Config")
	void SetEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Haptics|Config")
	bool IsEnabled() const { return Config.bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Haptics|Config")
	void SetGlobalIntensity(float Intensity);

	// State
	UFUNCTION(BlueprintPure, Category = "Haptics|State")
	TArray<FMGActiveHaptic> GetActiveHaptics() const;

	UFUNCTION(BlueprintPure, Category = "Haptics|State")
	bool IsHapticActive(FGuid HapticID) const;

	UFUNCTION(BlueprintPure, Category = "Haptics|State")
	bool HasAdaptiveTriggerSupport() const;

	UFUNCTION(BlueprintPure, Category = "Haptics|State")
	float GetControllerBatteryLevel() const;

	// Racing Wheel Integration
	UFUNCTION(BlueprintPure, Category = "Haptics|Wheel")
	bool IsRacingWheelConnected() const;

	/**
	 * Route haptic feedback to racing wheel FFB when connected
	 * Call this when you want haptic feedback to also trigger wheel FFB
	 */
	UFUNCTION(BlueprintCallable, Category = "Haptics|Wheel")
	void RouteToWheelFFB(EMGHapticType Type, float Intensity);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Haptics|Events")
	FOnHapticStarted OnHapticStarted;

	UPROPERTY(BlueprintAssignable, Category = "Haptics|Events")
	FOnHapticStopped OnHapticStopped;

	UPROPERTY(BlueprintAssignable, Category = "Haptics|Events")
	FOnTriggerFeedbackChanged OnTriggerFeedbackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Haptics|Events")
	FOnSurfaceChanged OnSurfaceChanged;

protected:
	void OnHapticsTick();
	void UpdateActiveHaptics(float DeltaTime);
	void ApplyHapticOutput();
	void InitializeDefaultPatterns();
	FMGHapticPattern GetDefaultPattern(EMGHapticType Type) const;
	void ApplyIntensityModifiers(float& LeftIntensity, float& RightIntensity);
	void UpdateContinuousFeedback();
	int32 GetNextPrioritySlot(int32 Priority);

	UPROPERTY()
	TArray<FMGActiveHaptic> ActiveHaptics;

	UPROPERTY()
	TMap<FName, FMGHapticPattern> RegisteredPatterns;

	UPROPERTY()
	TMap<EMGSurfaceType, FMGSurfaceFeedback> SurfaceFeedbackMap;

	UPROPERTY()
	FMGHapticsConfig Config;

	UPROPERTY()
	FMGTriggerFeedback CurrentTriggerFeedback;

	UPROPERTY()
	EMGSurfaceType CurrentSurface = EMGSurfaceType::Asphalt;

	UPROPERTY()
	float CurrentEngineRPMPercent = 0.0f;

	UPROPERTY()
	float CurrentSpeed = 0.0f;

	UPROPERTY()
	bool bIsDrifting = false;

	UPROPERTY()
	bool bIsBoosting = false;

	UPROPERTY()
	float CurrentBoostIntensity = 0.0f;

	UPROPERTY()
	float CurrentDriftAngle = 0.0f;

	UPROPERTY()
	float ControllerBatteryLevel = 1.0f;

	FTimerHandle HapticsTickHandle;

	/** Racing wheel subsystem reference */
	UPROPERTY()
	TWeakObjectPtr<UMGRacingWheelSubsystem> RacingWheelSubsystem;

	/** Cache racing wheel subsystem */
	void CacheRacingWheelSubsystem();
};
