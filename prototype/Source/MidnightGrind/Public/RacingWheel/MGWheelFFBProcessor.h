// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MGRacingWheelTypes.h"
#include "MGWheelFFBProcessor.generated.h"

class UMGRacingWheelSubsystem;

/**
 * Advanced FFB Processor
 *
 * Creates an intuitive, skill-based driving feel through physics-accurate force feedback.
 * The goal is to make the wheel communicate the car's state so players can feel:
 * - When they're at the grip limit
 * - How to initiate and maintain drifts
 * - Weight transfer through corners
 * - The difference between understeer and oversteer
 *
 * Key Design Principles:
 * 1. Self-centering reduces as grip is lost (critical for feeling the limit)
 * 2. The wheel "goes light" during understeer (front tires sliding)
 * 3. Counter-steer forces during oversteer help players catch slides
 * 4. Slip angle feedback follows real pneumatic trail physics
 * 5. Surface textures provide constant road information
 */
UCLASS()
class MIDNIGHTGRIND_API UMGWheelFFBProcessor : public UObject
{
	GENERATED_BODY()

public:
	UMGWheelFFBProcessor();

	void Initialize(UMGRacingWheelSubsystem* InWheelSubsystem);
	void Tick(float DeltaTime);
	void ProcessVehicleData(const FMGFFBInputData& VehicleData, const FMGWheelProfile& Profile);
	float GetTotalOutputForce() const { return TotalOutputForce; }
	void GetEffectContributions(float& OutSelfCentering, float& OutAligningTorque,
								float& OutUndersteer, float& OutOversteer,
								float& OutSurface, float& OutEngine) const;
	void Reset();

protected:
	// === Core Force Calculations ===

	/**
	 * Calculate self-centering force based on speed and grip
	 * - Increases with speed (faster = more centering force)
	 * - Decreases as tires lose grip (critical for feeling the limit!)
	 * - Goes to zero when airborne
	 */
	float CalculateSelfCenteringForce(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate aligning torque (Self-Aligning Torque / SAT)
	 * This is the force that makes real steering feel "heavy" or "light"
	 * - Based on tire slip angle and pneumatic trail
	 * - Peaks at optimal slip, then drops off as tires slide
	 * - This is what tells you when you're at the grip limit
	 */
	float CalculateAligningTorque(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate tire grip feedback
	 * - Communicates front tire grip level through wheel weight
	 * - Wheel goes progressively lighter as front grip is lost
	 */
	float CalculateGripFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate drift/oversteer feedback
	 * - Provides counter-steer force to help catch slides
	 * - Force direction opposes the slide
	 * - Magnitude helps players learn the correct counter-steer amount
	 */
	float CalculateDriftFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate weight transfer effects
	 * - Feel the car loading outer tires in corners
	 * - Feel braking weight shift forward
	 * - Feel acceleration weight shift rearward
	 */
	float CalculateWeightTransferFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate G-force feedback
	 * - Lateral Gs create resistance in turn direction
	 * - Longitudinal Gs affect wheel weight
	 */
	float CalculateGForceFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	// === Effect Updates ===
	void UpdateKerbEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);
	void UpdateSurfaceEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);
	void UpdateEngineEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);
	void UpdateCollisionEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	// === Output ===
	void ApplyEffects(const FMGWheelProfile& Profile);

	// === Utilities ===
	float SmoothForce(float Current, float Target, float SmoothTime, float DeltaTime);
	float ApplyDeadzone(float Value, float Deadzone);
	float NormalizeSlipAngle(float SlipAngleDeg);
	float CalculateTireGripFromSlip(float SlipAngle);

private:
	UPROPERTY()
	TWeakObjectPtr<UMGRacingWheelSubsystem> WheelSubsystem;

	// Previous frame data for derivatives
	FMGFFBInputData LastVehicleData;
	float LastDeltaTime = 0.016f;

	// === Current Force Components ===
	float CurrentSelfCentering = 0.0f;
	float CurrentAligningTorque = 0.0f;
	float CurrentGripFeedback = 0.0f;
	float CurrentDriftFeedback = 0.0f;
	float CurrentWeightTransfer = 0.0f;
	float CurrentGForce = 0.0f;
	float CurrentSurfaceRumble = 0.0f;
	float CurrentEngineVibration = 0.0f;

	// === Target Forces (for smoothing) ===
	float TargetSelfCentering = 0.0f;
	float TargetAligningTorque = 0.0f;
	float TargetGripFeedback = 0.0f;
	float TargetDriftFeedback = 0.0f;
	float TargetWeightTransfer = 0.0f;
	float TargetGForce = 0.0f;

	// === Output ===
	float TotalOutputForce = 0.0f;
	float TotalConstantForce = 0.0f;
	float TotalSpringForce = 0.0f;

	// === Active Effect IDs ===
	FGuid ConstantForceEffectID;
	FGuid SpringEffectID;
	FGuid DamperEffectID;
	FGuid SurfaceEffectID;
	FGuid EngineEffectID;
	FGuid KerbEffectID;
	FGuid CollisionEffectID;

	// === State Tracking ===
	float TimeSinceLastUpdate = 0.0f;
	bool bWasOnKerb = false;
	FName CurrentSurfaceType = NAME_None;
	bool bIsAirborne = false;
	bool bWasDrifting = false;
	float DriftEntryTime = 0.0f;
	float CurrentDriftDuration = 0.0f;
	float PeakDriftAngle = 0.0f;

	// === Smoothing Times (tuned for responsiveness vs smoothness) ===
	// Faster = more responsive but can feel twitchy
	// Slower = smoother but can feel laggy
	static constexpr float SelfCenteringSmoothTime = 0.04f;    // Quick response for centering
	static constexpr float AligningTorqueSmoothTime = 0.025f;  // Very quick - this is your grip info
	static constexpr float GripFeedbackSmoothTime = 0.06f;     // Slightly slower for stability
	static constexpr float DriftFeedbackSmoothTime = 0.02f;    // Instant! Critical for catching slides
	static constexpr float WeightTransferSmoothTime = 0.08f;   // Slower - weight transfer is gradual
	static constexpr float GForceSmoothTime = 0.05f;           // Medium speed

	// === Physics Constants ===
	// These define how the forces feel - tuned for arcade-sim balance

	// Self-centering
	static constexpr float MinSpeedForCentering = 5.0f;        // km/h - no centering when nearly stopped
	static constexpr float MaxCenteringSpeed = 180.0f;         // km/h - centering maxes out here
	static constexpr float BaseCenteringStrength = 0.4f;       // Base spring strength at max speed

	// Aligning torque (the magic that makes steering feel alive)
	static constexpr float OptimalSlipAngle = 8.0f;            // Degrees - peak grip slip angle
	static constexpr float MaxSlipAngle = 25.0f;               // Degrees - beyond this, full slide
	static constexpr float PeakAligningTorque = 0.6f;          // Force at optimal slip
	static constexpr float SlidingAligningTorque = 0.15f;      // Force when fully sliding (goes light!)

	// Drift feedback
	static constexpr float DriftAngleThreshold = 10.0f;        // Degrees to consider "drifting"
	static constexpr float MaxDriftAngleForFeedback = 60.0f;   // Degrees - scale force up to here
	static constexpr float DriftCounterForceBase = 0.5f;       // Base counter-steer force
	static constexpr float DriftCounterForceMax = 0.8f;        // Max counter-steer force

	// Weight transfer
	static constexpr float MaxLateralGForFeedback = 1.5f;      // G - scale forces up to this
	static constexpr float MaxLongitudinalGForFeedback = 1.2f; // G

	// Grip simulation
	static constexpr float GripLossStartAngle = 6.0f;          // Degrees - grip starts to fade
	static constexpr float GripLossFullAngle = 20.0f;          // Degrees - grip fully lost
};
