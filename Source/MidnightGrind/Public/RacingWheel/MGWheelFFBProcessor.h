// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGWheelFFBProcessor.h
 * =============================================================================
 *
 * PURPOSE:
 * This class is the "brain" of the force feedback system. It takes vehicle
 * physics data and calculates what forces should be sent to the steering wheel
 * to create a realistic, informative driving feel.
 *
 * WHY FFB MATTERS:
 * Force feedback isn't just for immersion - it's a critical skill-building tool.
 * A well-tuned FFB system teaches players to drive better by communicating:
 * - When tires are at the grip limit (before they actually slide)
 * - How to catch and control slides/drifts
 * - How the car's weight shifts during braking and cornering
 * - The difference between losing front grip (understeer) vs rear grip (oversteer)
 *
 * THE SCIENCE BEHIND IT:
 * Real car steering "feel" comes from several physics phenomena:
 *
 * 1. SELF-ALIGNING TORQUE (SAT):
 *    Tires naturally want to align with their direction of travel.
 *    This creates the "centering" force you feel in a real car.
 *    Key insight: This force DECREASES as tires start to slide!
 *    That's why experienced drivers can "feel" the grip limit.
 *
 * 2. PNEUMATIC TRAIL:
 *    The contact patch of a tire is not a single point - it's an area.
 *    Forces act through the center of this area, which shifts as slip
 *    angle increases. This creates the aligning torque mentioned above.
 *
 * 3. SLIP ANGLE:
 *    The angle between where a tire points and where it's actually going.
 *    - Small slip angle = grip (tire biting into road)
 *    - Large slip angle = sliding (tire scrubbing sideways)
 *    - Peak grip is typically around 6-10 degrees of slip
 *
 * HOW THIS CLASS WORKS:
 *
 *   [Vehicle Physics] --> FMGFFBInputData --> [This Processor] --> FFB Commands --> [Wheel]
 *
 * Each frame:
 * 1. Vehicle code fills an FMGFFBInputData struct with current physics state
 * 2. ProcessVehicleData() calculates target forces for each feedback type
 * 3. Tick() smooths these forces to prevent jarring changes
 * 4. ApplyEffects() sends commands to the wheel via the subsystem
 *
 * FORCE COMPONENTS CALCULATED:
 * - Self-centering: Speed-based spring that reduces with grip loss
 * - Aligning torque: The "SAT" feedback that communicates grip level
 * - Grip feedback: Wheel "lightness" that indicates front tire saturation
 * - Drift feedback: Counter-steer force during oversteer/drifting
 * - Weight transfer: Feel the car load/unload during maneuvers
 * - G-force: Resistance proportional to lateral/longitudinal acceleration
 * - Surface effects: Vibrations from kerbs, gravel, etc.
 * - Engine: RPM-linked vibration for immersion
 *
 * TUNING PHILOSOPHY:
 * The constants in this file are tuned for an "arcade-sim" balance:
 * - Realistic enough to teach proper driving technique
 * - Exaggerated enough to be clear and readable
 * - Forgiving enough to be fun, not frustrating
 *
 * If you're making a hardcore sim, increase the subtlety.
 * If you're making a pure arcade game, increase the intensity.
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MGRacingWheelTypes.h"
#include "MGWheelFFBProcessor.generated.h"

// Forward declaration - the processor needs to send effects through the subsystem
class UMGRacingWheelSubsystem;

/**
 * Advanced FFB Processor
 *
 * Calculates physics-accurate force feedback to communicate vehicle state
 * through the steering wheel. See file header for detailed documentation.
 *
 * USAGE:
 * This class is created and owned by UMGRacingWheelSubsystem.
 * You don't interact with it directly - use the subsystem's high-level
 * FFB functions instead (UpdateFFBFromVehicle, etc.).
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

	// --- Lifecycle ---

	/**
	 * Initialize the processor with a reference to the wheel subsystem.
	 * Called automatically by the subsystem during its initialization.
	 * @param InWheelSubsystem The subsystem that will receive our FFB commands
	 */
	void Initialize(UMGRacingWheelSubsystem* InWheelSubsystem);

	/**
	 * Update smoothing and apply effects. Called every frame by the subsystem.
	 * This smooths the force values calculated by ProcessVehicleData to prevent
	 * jarring force changes between frames.
	 * @param DeltaTime Time since last tick (seconds)
	 */
	void Tick(float MGDeltaTime);

	/**
	 * Calculate FFB forces from vehicle physics data.
	 * This is the main calculation function - it reads the vehicle state
	 * and computes target force values for each feedback component.
	 * Call this every frame from your vehicle's Tick function.
	 * @param VehicleData Current vehicle physics state
	 * @param Profile User's FFB settings (strength multipliers, etc.)
	 */
	void ProcessVehicleData(const FMGFFBInputData& VehicleData, const FMGWheelProfile& Profile);

	// --- Getters for Debugging/UI ---

	/**
	 * Get the combined output force being sent to the wheel.
	 * Useful for debugging and FFB clipping indicators.
	 * @return Total force magnitude (typically -1 to 1, can exceed during clipping)
	 */
	float GetTotalOutputForce() const { return TotalOutputForce; }

	/**
	 * Get individual force component values for debugging display.
	 * Shows how much each effect type is contributing to the total.
	 * @param OutSelfCentering Self-centering spring force
	 * @param OutAligningTorque Tire SAT (grip feel) force
	 * @param OutUndersteer Understeer feedback (wheel lightness)
	 * @param OutOversteer Oversteer/drift counter-steer force
	 * @param OutSurface Surface vibration intensity
	 * @param OutEngine Engine vibration intensity
	 */
	void GetEffectContributions(float& OutSelfCentering, float& OutAligningTorque,
								float& OutUndersteer, float& OutOversteer,
								float& OutSurface, float& OutEngine) const;

	/**
	 * Reset all forces to zero. Called when wheel disconnects or FFB is disabled.
	 */
	void Reset();

protected:
	// ==========================================
	// CORE FORCE CALCULATIONS
	// ==========================================
	// Each function calculates one component of the total FFB force.
	// They all take the same inputs (vehicle data + profile) and return
	// a force value, typically in the -1 to 1 range.

	/**
	 * Calculate self-centering force based on speed and grip
	 *
	 * This simulates the tendency of real steering to return to center.
	 * In real cars, this comes from caster angle and tire characteristics.
	 *
	 * BEHAVIOR:
	 * - Zero at very low speeds (allows easy maneuvering when parked)
	 * - Increases with speed up to a maximum
	 * - DECREASES when tires lose grip (this is the "lightening" effect
	 *   that tells you the tires are about to/starting to slide)
	 * - Goes to zero when airborne (no road contact = no centering)
	 *
	 * WHY IT MATTERS:
	 * This reduction of centering force as grip is lost is THE key
	 * feedback that lets skilled drivers feel the limit. It's subtle
	 * but incredibly important for high-skill driving.
	 */
	float CalculateSelfCenteringForce(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate aligning torque (Self-Aligning Torque / SAT)
	 *
	 * This is the force that makes real steering feel "heavy" or "light."
	 * It's based on the physics of the tire's contact patch (pneumatic trail).
	 *
	 * PHYSICS BACKGROUND:
	 * When a tire generates lateral force (cornering), that force acts
	 * through a point behind the steering axis. This creates a torque
	 * that tries to align the tire with its direction of travel.
	 *
	 * THE KEY INSIGHT:
	 * SAT peaks at moderate slip angles (around optimal grip), then
	 * DECREASES as slip angle increases beyond the grip limit.
	 * This is the "steering goes light" sensation that racing drivers
	 * use to know when they've exceeded the tire's capabilities.
	 *
	 * IMPLEMENTATION:
	 * - Increases with slip angle up to optimal grip point (~8 degrees)
	 * - Decreases past the optimal point as tires transition to sliding
	 * - Falls to a low baseline value when fully sliding
	 */
	float CalculateAligningTorque(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate tire grip feedback
	 *
	 * Communicates front tire grip level through wheel "weight."
	 * This is closely related to SAT but provides additional feedback
	 * specifically about the front tires' grip state.
	 *
	 * BEHAVIOR:
	 * - Full grip = normal wheel weight
	 * - Partial grip loss = wheel gets progressively lighter
	 * - Full slide = very light wheel (understeer feedback)
	 *
	 * This helps players understand why the car isn't turning:
	 * "The wheel is light" = front tires are sliding = reduce speed/steering
	 */
	float CalculateGripFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate drift/oversteer feedback
	 *
	 * When the rear of the car slides out (oversteer), this provides
	 * a force that helps the player counter-steer.
	 *
	 * DRIFT PHYSICS:
	 * In oversteer, the rear tires have more slip than the fronts.
	 * The car rotates (yaws) more than the driver intended.
	 * To catch it, you need to steer INTO the slide (counter-steer).
	 *
	 * FFB IMPLEMENTATION:
	 * - Detects oversteer/drift condition from rear slip angles
	 * - Creates a force pushing the wheel toward the counter-steer direction
	 * - Force magnitude indicates how much counter-steer is appropriate
	 * - Very quick response time (critical for catching slides!)
	 *
	 * This helps players learn drift technique - the wheel "tells" them
	 * where to point it, and with practice they develop muscle memory.
	 */
	float CalculateDriftFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate weight transfer effects
	 *
	 * Simulates how weight shifts around the car during dynamic maneuvers.
	 *
	 * WEIGHT TRANSFER PHYSICS:
	 * - Braking: Weight shifts forward (front tires grip more)
	 * - Acceleration: Weight shifts rearward (rear tires grip more)
	 * - Cornering: Weight shifts to outside tires (they grip more)
	 *
	 * FFB IMPLEMENTATION:
	 * - Feels like the wheel "loads up" (gets heavier) in the turn direction
	 * - Braking makes the wheel heavier (more front grip = more centering)
	 * - Acceleration can lighten the wheel (less front grip)
	 *
	 * This helps players understand car balance and trail braking technique.
	 */
	float CalculateWeightTransferFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/**
	 * Calculate G-force feedback
	 *
	 * Direct mapping of vehicle accelerations to steering resistance.
	 *
	 * - Lateral Gs: Create resistance in the turn direction
	 *   (cornering hard makes the wheel fight back)
	 * - Longitudinal Gs: Affect overall wheel weight
	 *   (braking = heavier, accelerating = lighter)
	 *
	 * This is a simpler, more arcade-style feedback compared to the
	 * physics-based calculations above. It provides good "seat of pants"
	 * feel even if the physics data is simplified.
	 */
	float CalculateGForceFeedback(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	// ==========================================
	// EFFECT UPDATES
	// ==========================================
	// These functions handle specific effect types that use the wheel's
	// periodic/vibration capabilities (as opposed to constant forces).

	/** Update kerb/rumble strip vibration effect */
	void UpdateKerbEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/** Update road surface texture effect (gravel, dirt, etc.) */
	void UpdateSurfaceEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/** Update engine RPM vibration effect */
	void UpdateEngineEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	/** Update collision impact effect (one-shot, then fades) */
	void UpdateCollisionEffect(const FMGFFBInputData& Data, const FMGWheelProfile& Profile);

	// ==========================================
	// OUTPUT
	// ==========================================

	/**
	 * Combine all calculated forces and send to the wheel.
	 * This sums up all force components, applies profile scaling,
	 * checks for clipping, and sends the final commands.
	 */
	void ApplyEffects(const FMGWheelProfile& Profile);

	// ==========================================
	// UTILITIES
	// ==========================================
	// Helper functions for common calculations.

	/**
	 * Smoothly interpolate from current to target value.
	 * Uses exponential smoothing for natural-feeling transitions.
	 * @param Current Current value
	 * @param Target Target value
	 * @param SmoothTime Time constant (lower = faster response)
	 * @param DeltaTime Frame time
	 * @return New smoothed value
	 */
	float SmoothForce(float Current, float Target, float SmoothTime, float MGDeltaTime);

	/**
	 * Apply a deadzone to a value.
	 * Values within the deadzone become zero; values outside are scaled
	 * so the usable range is still 0-1.
	 */
	float ApplyDeadzone(float Value, float Deadzone);

	/**
	 * Normalize a slip angle to a 0-1 range for force calculations.
	 */
	float NormalizeSlipAngle(float SlipAngleDeg);

	/**
	 * Calculate tire grip level from slip angle.
	 * Returns 1.0 at optimal slip, decreases toward 0 as slip increases.
	 * This models the "slip curve" of a tire.
	 */
	float CalculateTireGripFromSlip(float SlipAngle);

private:
	// ==========================================
	// REFERENCES
	// ==========================================

	/**
	 * Weak pointer to the wheel subsystem.
	 * We use a weak pointer because:
	 * 1. The subsystem owns us (so we shouldn't prevent its destruction)
	 * 2. We need to send FFB commands through it
	 * TWeakObjectPtr automatically becomes null if the subsystem is destroyed.
	 */
	UPROPERTY()
	TWeakObjectPtr<UMGRacingWheelSubsystem> WheelSubsystem;

	// ==========================================
	// PREVIOUS FRAME DATA
	// ==========================================
	// Used to calculate derivatives (rate of change) for some effects.

	/** Vehicle data from the previous frame */
	FMGFFBInputData LastVehicleData;

	/** Previous frame's delta time (used when current delta is invalid) */
	float LastDeltaTime = 0.016f;  // Default to ~60fps

	// ==========================================
	// FORCE COMPONENTS
	// ==========================================
	// Each force type has a "Current" (smoothed) and "Target" (instant) value.
	// The smoothing prevents jarring force changes between frames.

	// --- Current Values (after smoothing - sent to wheel) ---
	float CurrentSelfCentering = 0.0f;
	float CurrentAligningTorque = 0.0f;
	float CurrentGripFeedback = 0.0f;
	float CurrentDriftFeedback = 0.0f;
	float CurrentWeightTransfer = 0.0f;
	float CurrentGForce = 0.0f;
	float CurrentSurfaceRumble = 0.0f;
	float CurrentEngineVibration = 0.0f;

	// --- Target Values (calculated from physics - before smoothing) ---
	float TargetSelfCentering = 0.0f;
	float TargetAligningTorque = 0.0f;
	float TargetGripFeedback = 0.0f;
	float TargetDriftFeedback = 0.0f;
	float TargetWeightTransfer = 0.0f;
	float TargetGForce = 0.0f;

	// ==========================================
	// OUTPUT VALUES
	// ==========================================
	// The combined force values that are actually sent to the wheel.

	/** Total combined force output (for clipping detection) */
	float TotalOutputForce = 0.0f;

	/** Sum of all constant (non-spring) forces */
	float TotalConstantForce = 0.0f;

	/** Sum of all spring-type forces */
	float TotalSpringForce = 0.0f;

	// ==========================================
	// ACTIVE EFFECT IDs
	// ==========================================
	// We track GUIDs of active effects so we can update them each frame
	// instead of constantly creating new effects (which is expensive).

	FGuid ConstantForceEffectID;  // Main steering force
	FGuid SpringEffectID;         // Self-centering spring
	FGuid DamperEffectID;         // Movement resistance
	FGuid SurfaceEffectID;        // Road texture vibration
	FGuid EngineEffectID;         // Engine rumble
	FGuid KerbEffectID;           // Rumble strip effect
	FGuid CollisionEffectID;      // Impact effect (fades out)

	// ==========================================
	// STATE TRACKING
	// ==========================================
	// Track state changes for edge detection and effect management.

	float TimeSinceLastUpdate = 0.0f;  // Watchdog timer
	bool bWasOnKerb = false;           // Edge detection for kerb entry
	FName CurrentSurfaceType = NAME_None;  // Track surface changes
	bool bIsAirborne = false;          // Track airborne state
	bool bWasDrifting = false;         // Edge detection for drift entry
	float DriftEntryTime = 0.0f;       // When current drift started
	float CurrentDriftDuration = 0.0f; // How long we've been drifting
	float PeakDriftAngle = 0.0f;       // Maximum angle achieved in current drift

	// ==========================================
	// SMOOTHING TIME CONSTANTS
	// ==========================================
	// These control how quickly forces change between frames.
	// Lower values = faster response (can feel twitchy)
	// Higher values = smoother (can feel laggy)
	//
	// The values are tuned per-effect based on what feels right:
	// - Grip information (SAT, drift) needs to be FAST for safety
	// - Background effects (weight transfer) can be slower for comfort

	static constexpr float SelfCenteringSmoothTime = 0.04f;    // Quick - centering is important
	static constexpr float AligningTorqueSmoothTime = 0.025f;  // Very quick - this tells you grip!
	static constexpr float GripFeedbackSmoothTime = 0.06f;     // Slightly slower for stability
	static constexpr float DriftFeedbackSmoothTime = 0.02f;    // Nearly instant! Catching slides is critical
	static constexpr float WeightTransferSmoothTime = 0.08f;   // Slower - weight transfer is gradual in reality
	static constexpr float GForceSmoothTime = 0.05f;           // Medium speed

	// ==========================================
	// PHYSICS TUNING CONSTANTS
	// ==========================================
	// These values define the "feel" of the simulation.
	// Adjust these to change the balance between arcade and simulation.
	// All values are tuned for an arcade-sim hybrid feel.

	// --- Self-Centering Parameters ---
	// Controls the spring that returns the wheel to center

	/** Speed below which there's no centering force (km/h) - allows parking maneuvers */
	static constexpr float MinSpeedForCentering = 5.0f;

	/** Speed at which centering force reaches maximum (km/h) */
	static constexpr float MaxCenteringSpeed = 180.0f;

	/** Base spring strength at max speed (before grip reduction) */
	static constexpr float BaseCenteringStrength = 0.4f;

	// --- Aligning Torque Parameters ---
	// Controls the "magic" feel of the tires - this is what makes steering feel alive!

	/** Slip angle at which grip is at its peak (degrees) - tires grip best here */
	static constexpr float OptimalSlipAngle = 8.0f;

	/** Slip angle beyond which tires are fully sliding (degrees) */
	static constexpr float MaxSlipAngle = 25.0f;

	/** Aligning torque force at optimal slip (when grip is at peak) */
	static constexpr float PeakAligningTorque = 0.6f;

	/** Aligning torque when fully sliding (the "goes light" effect) */
	static constexpr float SlidingAligningTorque = 0.15f;

	// --- Drift Feedback Parameters ---
	// Controls the counter-steer assistance during oversteer/drifting

	/** Angle at which we consider the car to be "drifting" (degrees) */
	static constexpr float DriftAngleThreshold = 10.0f;

	/** Maximum drift angle for scaling feedback (degrees) */
	static constexpr float MaxDriftAngleForFeedback = 60.0f;

	/** Base counter-steer force at drift threshold */
	static constexpr float DriftCounterForceBase = 0.5f;

	/** Maximum counter-steer force at large drift angles */
	static constexpr float DriftCounterForceMax = 0.8f;

	// --- Weight Transfer Parameters ---
	// Controls how G-forces affect steering feel

	/** Lateral G at which force is at maximum */
	static constexpr float MaxLateralGForFeedback = 1.5f;

	/** Longitudinal G at which force is at maximum */
	static constexpr float MaxLongitudinalGForFeedback = 1.2f;

	// --- Grip Simulation Parameters ---
	// Controls when the grip feedback starts reducing

	/** Slip angle at which grip starts to fade (degrees) */
	static constexpr float GripLossStartAngle = 6.0f;

	/** Slip angle at which grip is considered fully lost (degrees) */
	static constexpr float GripLossFullAngle = 20.0f;
};
