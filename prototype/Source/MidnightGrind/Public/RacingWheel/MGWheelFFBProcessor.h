// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MGRacingWheelTypes.h"
#include "MGWheelFFBProcessor.generated.h"

class UMGRacingWheelSubsystem;

/**
 * FFB Processor
 *
 * Translates vehicle physics state into force feedback commands.
 * Calculates appropriate FFB effects based on:
 * - Self-centering force (speed-dependent spring)
 * - Aligning torque / road feel (tire slip angle)
 * - Understeer/oversteer feedback
 * - Collision impacts
 * - Kerb/rumble strip effects
 * - Engine vibration at redline
 */
UCLASS()
class MIDNIGHTGRIND_API UMGWheelFFBProcessor : public UObject
{
	GENERATED_BODY()

public:
	UMGWheelFFBProcessor();

	/**
	 * Initialize the processor with a wheel subsystem reference
	 */
	void Initialize(UMGRacingWheelSubsystem* InWheelSubsystem);

	/**
	 * Tick the processor (called from wheel subsystem)
	 */
	void Tick(float DeltaTime);

	/**
	 * Process vehicle physics data and update FFB
	 */
	void ProcessVehicleData(const FMGFFBInputData& VehicleData, const FMGWheelProfile& Profile);

	/**
	 * Get the total output force magnitude (for clipping detection)
	 */
	float GetTotalOutputForce() const { return TotalOutputForce; }

	/**
	 * Get individual effect contributions for debugging
	 */
	void GetEffectContributions(float& OutSelfCentering, float& OutAligningTorque,
								float& OutUndersteer, float& OutOversteer,
								float& OutSurface, float& OutEngine) const;

	/**
	 * Reset all accumulated forces
	 */
	void Reset();

protected:
	/** Calculate self-centering spring force */
	float CalculateSelfCenteringForce(float Speed, float SteeringAngle, const FMGWheelProfile& Profile);

	/** Calculate aligning torque from tire slip */
	float CalculateAligningTorque(float SlipAngle, float TireLoad, const FMGWheelProfile& Profile);

	/** Calculate understeer feedback */
	float CalculateUndersteerFeedback(float FrontSlip, float RearSlip, const FMGWheelProfile& Profile);

	/** Calculate oversteer feedback */
	float CalculateOversteerFeedback(float FrontSlip, float RearSlip, float YawRate, const FMGWheelProfile& Profile);

	/** Calculate kerb/rumble strip effect */
	void UpdateKerbEffect(bool bOnKerb, float Speed, const FMGWheelProfile& Profile);

	/** Calculate surface texture effect */
	void UpdateSurfaceEffect(FName SurfaceType, float Speed, const FMGWheelProfile& Profile);

	/** Calculate engine vibration effect */
	void UpdateEngineEffect(float RPMPercent, const FMGWheelProfile& Profile);

	/** Apply effects to the wheel */
	void ApplyEffects(const FMGWheelProfile& Profile);

	/** Smooth force transitions */
	float SmoothForce(float CurrentForce, float TargetForce, float SmoothTime, float DeltaTime);

	/** Apply force curve/gamma */
	float ApplyForceCurve(float Force, float Gamma = 1.0f);

private:
	/** Wheel subsystem reference */
	UPROPERTY()
	TWeakObjectPtr<UMGRacingWheelSubsystem> WheelSubsystem;

	/** Last processed vehicle data */
	FMGFFBInputData LastVehicleData;

	/** Current force contributions */
	float CurrentSelfCenteringForce = 0.0f;
	float CurrentAligningTorque = 0.0f;
	float CurrentUndersteerForce = 0.0f;
	float CurrentOversteerForce = 0.0f;
	float CurrentSurfaceForce = 0.0f;
	float CurrentEngineVibration = 0.0f;
	float CurrentDamperForce = 0.0f;

	/** Target forces for smoothing */
	float TargetSelfCenteringForce = 0.0f;
	float TargetAligningTorque = 0.0f;
	float TargetUndersteerForce = 0.0f;
	float TargetOversteerForce = 0.0f;

	/** Total output force */
	float TotalOutputForce = 0.0f;

	/** Active effect IDs */
	FGuid ConstantForceEffectID;
	FGuid SpringEffectID;
	FGuid DamperEffectID;
	FGuid SurfaceEffectID;
	FGuid EngineEffectID;
	FGuid KerbEffectID;

	/** Timing */
	float TimeSinceLastUpdate = 0.0f;

	/** State tracking */
	bool bOnKerb = false;
	FName CurrentSurface = NAME_None;
	bool bIsAirborne = false;

	/** Smoothing parameters */
	const float SelfCenteringSmoothTime = 0.05f;
	const float AligningTorqueSmoothTime = 0.03f;
	const float SlipFeedbackSmoothTime = 0.08f;

	/** Force calculation constants */
	const float MaxSelfCenteringSpeed = 200.0f; // km/h at which self-centering is maximum
	const float MinSelfCenteringSpeed = 5.0f;   // Below this, no self-centering
	const float MaxSlipAngleForFeedback = 12.0f; // Degrees
	const float UndersteerThreshold = 0.7f;     // Slip ratio threshold
	const float OversteerThreshold = 0.6f;      // Rear slip angle threshold
};
