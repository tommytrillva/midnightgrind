// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGSlipstreamSubsystem.h
 * Slipstream (Drafting) System for Midnight Grind Racing Game
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This subsystem implements the "slipstream" or "drafting" mechanic - a real-world
 * racing phenomenon where following closely behind another vehicle reduces air
 * resistance, allowing you to go faster with the same power output.
 *
 * In Midnight Grind, slipstreaming provides:
 * - Speed bonuses when driving behind other vehicles
 * - "Slingshot" ability to overtake after building up charge
 * - Nitro charging while drafting
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 *
 * 1. DRAFTING/SLIPSTREAM:
 *    When a vehicle moves through air, it creates a low-pressure "wake" behind it.
 *    A following vehicle in this wake experiences less air resistance (drag),
 *    allowing it to maintain higher speeds with less effort. This is called
 *    "drafting" in NASCAR or "slipstreaming" in Formula 1.
 *
 * 2. DRAFTING CONE:
 *    The slipstream effect only works within a cone-shaped area behind the lead
 *    vehicle. The cone has an angle (typically 30 degrees) and a maximum distance.
 *    Being directly behind the leader = maximum benefit. Being at an angle = less.
 *
 * 3. DRAFTING ZONES:
 *    - Outer: Far from leader, minimal benefit
 *    - Inner: Closer, good benefit
 *    - Optimal: "Sweet spot" distance for maximum benefit
 *    - TooClose: Dangerously close, risk of collision
 *
 * 4. SLINGSHOT MANEUVER:
 *    After drafting for a period, the follower builds up "charge". When full,
 *    they can execute a "slingshot" - pulling out of the slipstream with a
 *    temporary speed boost to overtake the leader. This is a classic racing
 *    tactic seen in real motorsport.
 *
 * 5. LINE OF SIGHT:
 *    The system can require clear line of sight between vehicles. If another
 *    car or obstacle blocks the path, the slipstream effect is interrupted.
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UWorldSubsystem, meaning one instance exists per game world/level
 * - Vehicles register themselves when spawned, unregister when destroyed
 * - The subsystem ticks every frame to update slipstream states
 * - Works with MGNitroBoostSubsystem to charge nitro while drafting
 * - Broadcasts events for UI feedback (visual effects, sounds, HUD indicators)
 *
 * USAGE EXAMPLE:
 * --------------
 * From a vehicle Blueprint or C++:
 *
 *   // Get the subsystem
 *   UMGSlipstreamSubsystem* Slipstream = GetWorld()->GetSubsystem<UMGSlipstreamSubsystem>();
 *
 *   // Register this vehicle
 *   Slipstream->RegisterVehicle(MyVehicle, MyVehicleData);
 *
 *   // Check if we're drafting
 *   if (Slipstream->IsInSlipstream(MyVehicle))
 *   {
 *       float SpeedBonus = Slipstream->GetCurrentSpeedBonus(MyVehicle);
 *       // Apply speed bonus to vehicle...
 *   }
 *
 *   // Use slingshot when ready
 *   if (Slipstream->IsSlingshotReady(MyVehicle))
 *   {
 *       Slipstream->ActivateSlingshot(MyVehicle);
 *   }
 *
 * @see UMGNitroBoostSubsystem - Nitro system that charges from drafting
 * @see UMGAerodynamicsSubsystem - Advanced aerodynamics calculations
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGSlipstreamSubsystem.generated.h"

// Forward declarations for soft object pointers
class UNiagaraSystem;
class USoundBase;

//=============================================================================
// ENUMERATIONS
// These define the discrete states and categories used by the slipstream system
//=============================================================================

/**
 * How strong the slipstream effect currently is.
 *
 * Strength increases the longer you stay in the slipstream and the closer
 * you are to the optimal drafting position. Higher strength = more benefit.
 *
 * Used for:
 * - Scaling speed bonuses
 * - Visual effect intensity
 * - Audio feedback volume
 * - UI indicator display
 */
UENUM(BlueprintType)
enum class EMGSlipstreamStrength : uint8
{
	None,      ///< Not in a slipstream at all
	Weak,      ///< Just entered or at edge of slipstream
	Moderate,  ///< Building up, decent benefit
	Strong,    ///< Well-positioned, good benefit
	Maximum    ///< Optimal position, maximum benefit, slingshot charges fastest
};

/**
 * Which zone of the drafting area the follower vehicle is in.
 *
 * Think of the drafting area as concentric zones behind the lead vehicle:
 *
 *                    [LEAD VEHICLE]
 *                          |
 *         TooClose --------+-------- (danger zone, too close!)
 *                          |
 *          Optimal --------+-------- (sweet spot for max benefit)
 *                          |
 *           Inner  --------+-------- (good position)
 *                          |
 *           Outer  --------+-------- (minimal effect)
 *                          |
 *            None  --------+-------- (outside slipstream cone)
 *
 * The zone determines:
 * - How much speed bonus you receive
 * - How fast the slingshot charges
 * - Risk level (TooClose = collision danger)
 */
UENUM(BlueprintType)
enum class EMGDraftingZone : uint8
{
	None,      ///< Outside the slipstream cone entirely
	Outer,     ///< Far edge of the cone, minimal aerodynamic benefit
	Inner,     ///< Inside the cone, receiving good benefit
	Optimal,   ///< Perfect distance, maximum benefit and fastest charge
	TooClose   ///< Dangerously close, still get benefit but risk collision
};

//=============================================================================
// DATA STRUCTURES - CONFIGURATION
// These structs hold tunable parameters that designers can adjust
//=============================================================================

/**
 * Configuration parameters for the slipstream system.
 *
 * These values control how drafting feels and behaves. Game designers can
 * tweak these to balance the mechanic - making it easier/harder to draft,
 * more/less rewarding, etc.
 *
 * TUNING TIPS:
 * - Increase MaxDraftDistance for more forgiving drafting
 * - Increase DraftConeAngle to make it easier to stay in the slipstream
 * - Adjust BuildUpTime/FallOffTime to control how "sticky" the effect feels
 * - SlingshotBonus determines how powerful overtaking moves are
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamConfig
{
	GENERATED_BODY()

	/// Maximum distance (in Unreal units, cm) at which drafting has any effect.
	/// Beyond this distance, you're too far to benefit from the leader's wake.
	/// 3000 cm = 30 meters in real-world terms.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDraftDistance = 3000.0f;

	/// Minimum distance required for drafting (safety buffer).
	/// If closer than this, you're in the TooClose zone.
	/// 200 cm = 2 meters - very close in racing terms.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDraftDistance = 200.0f;

	/// Where the "Optimal" drafting zone begins (distance from leader).
	/// This is the start of the sweet spot for maximum benefit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalDistanceStart = 500.0f;

	/// Where the "Optimal" drafting zone ends.
	/// Between OptimalDistanceStart and OptimalDistanceEnd = best drafting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalDistanceEnd = 1500.0f;

	/// The angle (in degrees) of the drafting cone behind the lead vehicle.
	/// 30 degrees means you can be up to 15 degrees off-center on each side.
	/// Wider angle = easier to draft, narrower = requires precision.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DraftConeAngle = 30.0f;

	/// Maximum speed bonus when drafting (as a multiplier).
	/// 0.15 = 15% speed increase at optimal drafting position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedBonus = 0.15f;

	/// How much nitro charges per second while drafting.
	/// This connects the slipstream system to the nitro boost system.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroChargeRate = 5.0f;

	/// Time (in seconds) to reach full slipstream strength after entering.
	/// Creates a "warm-up" period - you don't get instant full benefit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuildUpTime = 1.5f;

	/// Time (in seconds) for slipstream effect to fade after leaving.
	/// Provides a brief "grace period" if you momentarily lose the draft.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallOffTime = 0.75f;

	/// Minimum speed (in game units/sec) the lead vehicle must be traveling.
	/// Drafting doesn't work if the leader is going too slow.
	/// Prevents exploits and keeps the mechanic realistic.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinLeadVehicleSpeed = 50.0f;

	/// If true, obstacles between vehicles break the slipstream.
	/// When false, you can draft "through" other cars (less realistic but simpler).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequireLineOfSight = true;

	/// Speed bonus from the slingshot maneuver (as a multiplier).
	/// 0.1 = 10% extra speed when you execute a slingshot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotBonus = 0.1f;

	/// How long (in seconds) the slingshot boost lasts.
	/// 2 seconds is enough time to complete an overtake.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotDuration = 2.0f;
};

//=============================================================================
// DATA STRUCTURES - RUNTIME STATE
// These structs hold the current state of the slipstream for each vehicle
//=============================================================================

/**
 * Current slipstream state for a single vehicle.
 *
 * This struct is updated every frame and contains everything you need to know
 * about a vehicle's current drafting situation. Use this to:
 * - Update UI elements (slipstream indicator, charge meter)
 * - Apply speed bonuses to the vehicle
 * - Trigger visual/audio effects
 * - Determine when slingshot is available
 *
 * Query this via: GetSlipstreamState(MyVehicle)
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamState
{
	GENERATED_BODY()

	/// True if this vehicle is currently in another vehicle's slipstream.
	/// This is the primary check - if false, all other values are inactive.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInSlipstream = false;

	/// How strong the slipstream effect currently is.
	/// Strength builds up over time while drafting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSlipstreamStrength Strength = EMGSlipstreamStrength::None;

	/// Which drafting zone the vehicle is currently in.
	/// Determines how much benefit is received and how fast charge builds.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDraftingZone Zone = EMGDraftingZone::None;

	/// Current speed bonus being applied (0.0 to MaxSpeedBonus from config).
	/// Multiply your vehicle's speed by (1 + CurrentBonus) for final speed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentBonus = 0.0f;

	/// Slingshot charge level (0.0 to 1.0).
	/// When this reaches 1.0, the slingshot is ready to use.
	/// Display this on a UI meter to show players their progress.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeLevel = 0.0f;

	/// Total time (seconds) spent in the slipstream during this session.
	/// Useful for statistics and achievements.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeInSlipstream = 0.0f;

	/// Current distance to the lead vehicle (in Unreal units/cm).
	/// Useful for UI feedback showing how close you are.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToLeader = 0.0f;

	/// Angle (in degrees) to the lead vehicle's center line.
	/// 0 = directly behind, higher = off to the side.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleToLeader = 0.0f;

	/// Reference to the vehicle being drafted.
	/// Can be null if not currently drafting anyone.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* LeadVehicle = nullptr;

	/// True if the slingshot charge is full and ready to activate.
	/// Show a "SLINGSHOT READY!" indicator when this is true.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlingshotReady = false;

	/// True if the slingshot is currently active (boosting).
	/// The vehicle is in the middle of a slingshot overtake.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlingshotActive = false;

	/// Time remaining (seconds) in the current slingshot boost.
	/// Counts down from SlingshotDuration to 0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotTimeRemaining = 0.0f;
};

/**
 * Data about a vehicle needed for slipstream calculations.
 *
 * Each registered vehicle must provide this data every frame so the
 * subsystem can calculate drafting relationships. The vehicle pawn
 * or controller is responsible for updating this information.
 *
 * This struct contains the physics data needed to determine:
 * - Where the vehicle is in the world
 * - Which direction it's facing (for the drafting cone)
 * - How fast it's going (leader must be moving for drafting)
 * - Vehicle dimensions (affects the size of the slipstream wake)
 */
USTRUCT(BlueprintType)
struct FMGVehicleSlipstreamData
{
	GENERATED_BODY()

	/// Reference to the vehicle actor.
	/// Must be a valid pointer to participate in the slipstream system.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Vehicle = nullptr;

	/// Current world position of the vehicle (center point).
	/// Update this every frame from the vehicle's location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/// Current velocity vector (direction and magnitude).
	/// Used to calculate relative speeds and approach angles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/// Current speed (magnitude of velocity) in units per second.
	/// Convenience value - could be calculated from Velocity.Size().
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	/// Unit vector pointing in the direction the vehicle is facing.
	/// Critical for determining the drafting cone direction.
	/// The slipstream extends BEHIND this direction.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ForwardVector = FVector::ForwardVector;

	/// Length of the vehicle in Unreal units (cm).
	/// Longer vehicles create larger/longer slipstream wakes.
	/// 400 cm = 4 meters, typical for a sports car.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VehicleLength = 400.0f;

	/// Width of the vehicle in Unreal units (cm).
	/// Wider vehicles create wider slipstream cones.
	/// 180 cm = 1.8 meters, typical sports car width.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VehicleWidth = 180.0f;

	/// Drag coefficient affecting slipstream effectiveness.
	/// Higher drag = more turbulent wake = more benefit for followers.
	/// 1.0 = standard, higher values for boxier vehicles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragCoefficient = 1.0f;
};

//=============================================================================
// DATA STRUCTURES - VISUAL/AUDIO CONFIGURATION
// Settings for slipstream visual effects and audio feedback
//=============================================================================

/**
 * Visual and audio settings for slipstream feedback.
 *
 * Racing games need strong visual/audio feedback to communicate the
 * slipstream state to players. This struct configures:
 * - Colors for different slipstream strengths
 * - Particle effects showing the air wake
 * - Sound effects for the "whoosh" of drafting
 *
 * The colors progress from cool blue (weak) to intense cyan (maximum)
 * to communicate increasing benefit through color intensity.
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamVisual
{
	GENERATED_BODY()

	/// Whether to display visual slipstream effects at all.
	/// Set false for performance or if using custom effects.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSlipstreamEffect = true;

	/// Global multiplier for effect intensity.
	/// 0.5 = subtle effects, 1.0 = normal, 2.0 = very visible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectIntensity = 1.0f;

	/// Color for weak slipstream (just entered, edge of cone).
	/// Pale blue, semi-transparent.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WeakColor = FLinearColor(0.5f, 0.5f, 1.0f, 0.3f);

	/// Color for moderate slipstream (building up).
	/// Lighter blue, more visible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ModerateColor = FLinearColor(0.3f, 0.7f, 1.0f, 0.5f);

	/// Color for strong slipstream (well-positioned).
	/// Cyan, clearly visible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor StrongColor = FLinearColor(0.0f, 0.8f, 1.0f, 0.7f);

	/// Color for maximum slipstream (optimal zone, slingshot charging).
	/// Bright cyan, fully opaque - signals "you're nailing it!"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor MaximumColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);

	/// Niagara particle system for the visual slipstream effect.
	/// TSoftObjectPtr means it loads on demand, not at startup.
	/// Should be a trail/wake effect following the leader.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> SlipstreamEffect;

	/// Sound effect that plays while in the slipstream.
	/// Typically a wind/rushing air sound that intensifies with strength.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SlipstreamSound;
};

//=============================================================================
// DATA STRUCTURES - STATISTICS
// Tracking data for achievements, leaderboards, and player progression
//=============================================================================

/**
 * Statistics tracking for slipstream usage.
 *
 * These stats are accumulated during gameplay and can be used for:
 * - Achievements ("Draft for 60 seconds in one race")
 * - Leaderboards ("Most successful slingshot overtakes")
 * - Player progression and skill assessment
 * - Post-race summary screens
 *
 * Stats are tracked per-vehicle and can be reset between races or sessions.
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamStats
{
	GENERATED_BODY()

	/// Cumulative time (seconds) spent in any slipstream.
	/// Good for "draft master" type achievements.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeInSlipstream = 0.0f;

	/// Total distance traveled (in game units) while drafting.
	/// Useful for distance-based challenges.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceInSlipstream = 0.0f;

	/// Number of times the slingshot ability was used.
	/// Indicates aggressive, overtake-focused driving style.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlingshotsPerformed = 0;

	/// Number of overtakes completed immediately after a slingshot.
	/// Higher success rate = skilled at timing slingshots.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SuccessfulOvertakes = 0;

	/// Longest continuous time (seconds) in a single slipstream.
	/// Requires skill to maintain optimal position for extended periods.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestSlipstreamDuration = 0.0f;

	/// Total nitro charged from drafting (in nitro units).
	/// Shows how much the player relies on drafting for nitro vs. other methods.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroChargedFromDrafting = 0.0f;

	/// Number of times this vehicle has been the leader in a drafting chain.
	/// Being drafted by others doesn't benefit you directly but shows race position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesAsLeader = 0;

	/// Total time (seconds) with other vehicles drafting behind this one.
	/// Indicates time spent in the lead/front of the pack.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeAsLeader = 0.0f;
};

//=============================================================================
// DELEGATES (Event Callbacks)
// These allow other systems to react to slipstream events
//=============================================================================

/**
 * ABOUT DELEGATES:
 * Delegates are Unreal's event/callback system. They allow the slipstream
 * subsystem to notify other parts of the game when something happens,
 * without needing direct references to those systems.
 *
 * Example usage in Blueprint:
 *   1. Get the Slipstream Subsystem
 *   2. Bind an event to "OnSlipstreamEntered"
 *   3. Your event fires automatically when player enters a slipstream
 *
 * Example usage in C++:
 *   SlipstreamSubsystem->OnSlipstreamEntered.AddDynamic(this, &UMyClass::HandleSlipstreamEntered);
 */

/// Fired when a vehicle enters another vehicle's slipstream.
/// Use this to start visual effects, play "entering draft" sound, show HUD indicator.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlipstreamEntered, AActor*, LeadVehicle, EMGDraftingZone, Zone);

/// Fired when a vehicle exits a slipstream.
/// Use this to stop visual effects, play "exiting draft" sound, hide HUD indicator.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlipstreamExited);

/// Fired when slipstream strength changes (e.g., Weak -> Moderate -> Strong).
/// Use this to adjust visual effect intensity, change indicator color.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlipstreamStrengthChanged, EMGSlipstreamStrength, NewStrength);

/// Fired when slingshot ability becomes available.
/// Use this to show "SLINGSHOT READY!" prompt, play activation sound.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlingshotReady);

/// Fired when player activates slingshot, provides the speed bonus amount.
/// Use this to apply camera effects, play boost sound, show speed lines.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlingshotActivated, float, BonusSpeed);

/// Fired when slingshot boost ends.
/// Use this to return to normal visual state, play wind-down sound.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlingshotEnded);

/// Fired when nitro is charged from drafting, provides the charge amount.
/// Use this to update nitro UI, play small charging sound.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDraftingNitroCharged, float, Amount);

//=============================================================================
// MAIN SUBSYSTEM CLASS
//=============================================================================

/**
 * Core subsystem managing all slipstream/drafting mechanics.
 *
 * This is a UWorldSubsystem, meaning:
 * - One instance exists per game world (level)
 * - Automatically created when the world is created
 * - Automatically destroyed when the world is destroyed
 * - Access it from anywhere via: GetWorld()->GetSubsystem<UMGSlipstreamSubsystem>()
 *
 * RESPONSIBILITIES:
 * - Track all vehicles and their positions
 * - Calculate which vehicles are in slipstream of others
 * - Update slipstream states every frame
 * - Manage slingshot charge and activation
 * - Track statistics for each vehicle
 * - Broadcast events for UI and effects systems
 *
 * LIFECYCLE:
 * 1. Initialize() - Called when subsystem starts, sets up tick timer
 * 2. OnSlipstreamTick() - Called every frame, updates all vehicle states
 * 3. Deinitialize() - Called when subsystem shuts down, cleans up
 *
 * INTEGRATION POINTS:
 * - Vehicle Pawns: Call RegisterVehicle/UpdateVehicleData each frame
 * - UI System: Bind to delegates and query state for HUD updates
 * - VFX System: Bind to delegates to trigger particle effects
 * - Audio System: Bind to delegates to play sound effects
 * - Nitro System: Receives OnDraftingNitroCharged to add nitro
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSlipstreamSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//=========================================================================
	// LIFECYCLE FUNCTIONS
	// Called automatically by Unreal's subsystem framework
	//=========================================================================

	/// Called when the subsystem is created. Sets up initial state and tick timer.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when the subsystem is destroyed. Cleans up timers and references.
	virtual void Deinitialize() override;

	/// Determines if this subsystem should be created for the given world.
	/// Returns true for game worlds, false for editor preview worlds.
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//=========================================================================
	// VEHICLE REGISTRATION
	// Vehicles must register to participate in the slipstream system
	//=========================================================================

	/**
	 * Registers a vehicle with the slipstream system.
	 *
	 * Call this when a vehicle spawns (e.g., in BeginPlay).
	 * The vehicle must be registered before it can draft or be drafted.
	 *
	 * @param Vehicle The vehicle actor to register
	 * @param Data Initial vehicle data (position, velocity, dimensions)
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Registration")
	void RegisterVehicle(AActor* Vehicle, const FMGVehicleSlipstreamData& Data);

	/**
	 * Unregisters a vehicle from the slipstream system.
	 *
	 * Call this when a vehicle is destroyed or leaves the race.
	 * Cleans up any slipstream relationships involving this vehicle.
	 *
	 * @param Vehicle The vehicle actor to unregister
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Registration")
	void UnregisterVehicle(AActor* Vehicle);

	/**
	 * Updates a vehicle's data (position, velocity, etc.).
	 *
	 * Call this EVERY FRAME from your vehicle's Tick function.
	 * The slipstream calculations depend on up-to-date position data.
	 *
	 * @param Vehicle The vehicle to update
	 * @param Data Current frame's vehicle data
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Registration")
	void UpdateVehicleData(AActor* Vehicle, const FMGVehicleSlipstreamData& Data);

	/**
	 * Checks if a vehicle is currently registered.
	 *
	 * @param Vehicle The vehicle to check
	 * @return True if the vehicle is registered and participating
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Registration")
	bool IsVehicleRegistered(AActor* Vehicle) const;

	//=========================================================================
	// SLIPSTREAM STATE QUERIES
	// Functions to check the current drafting situation for a vehicle
	//=========================================================================

	/**
	 * Gets the complete slipstream state for a vehicle.
	 *
	 * This is the main query function - returns everything you need to know
	 * about a vehicle's current drafting situation in one struct.
	 *
	 * @param Vehicle The vehicle to query
	 * @return Complete state including zone, strength, charge, etc.
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	FMGSlipstreamState GetSlipstreamState(AActor* Vehicle) const;

	/**
	 * Quick check if a vehicle is currently in any slipstream.
	 *
	 * Use this for simple conditionals. For more detail, use GetSlipstreamState().
	 *
	 * @param Vehicle The vehicle to check
	 * @return True if drafting behind another vehicle
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	bool IsInSlipstream(AActor* Vehicle) const;

	/**
	 * Gets the current speed bonus being received from drafting.
	 *
	 * Returns 0.0 if not drafting. Apply this to your vehicle's speed:
	 * FinalSpeed = BaseSpeed * (1.0 + GetCurrentSpeedBonus())
	 *
	 * @param Vehicle The vehicle to query
	 * @return Speed bonus as a multiplier (0.0 to Config.MaxSpeedBonus)
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	float GetCurrentSpeedBonus(AActor* Vehicle) const;

	/**
	 * Gets the current slipstream strength level.
	 *
	 * Useful for driving visual/audio feedback intensity.
	 *
	 * @param Vehicle The vehicle to query
	 * @return Current strength (None, Weak, Moderate, Strong, Maximum)
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	EMGSlipstreamStrength GetSlipstreamStrength(AActor* Vehicle) const;

	/**
	 * Gets which drafting zone the vehicle is currently in.
	 *
	 * @param Vehicle The vehicle to query
	 * @return Current zone (None, Outer, Inner, Optimal, TooClose)
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	EMGDraftingZone GetDraftingZone(AActor* Vehicle) const;

	/**
	 * Gets the vehicle being drafted (the one in front).
	 *
	 * @param Vehicle The following vehicle
	 * @return Pointer to the lead vehicle, or nullptr if not drafting
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|State")
	AActor* GetLeadVehicle(AActor* Vehicle) const;

	//=========================================================================
	// SLINGSHOT ABILITY
	// The "slingshot" is a boost earned by sustained drafting
	//=========================================================================

	/**
	 * Activates the slingshot ability for a vehicle.
	 *
	 * The slingshot must be fully charged (IsSlingshotReady() == true).
	 * When activated, the vehicle gets a temporary speed boost to overtake
	 * the vehicle they were drafting.
	 *
	 * Typically bound to a player input (e.g., same button as boost).
	 *
	 * @param Vehicle The vehicle attempting to slingshot
	 * @return True if slingshot was successfully activated
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Slingshot")
	bool ActivateSlingshot(AActor* Vehicle);

	/**
	 * Checks if the slingshot is fully charged and ready to use.
	 *
	 * Show a "SLINGSHOT READY!" prompt when this returns true.
	 *
	 * @param Vehicle The vehicle to check
	 * @return True if slingshot can be activated
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Slingshot")
	bool IsSlingshotReady(AActor* Vehicle) const;

	/**
	 * Checks if the vehicle is currently in the middle of a slingshot boost.
	 *
	 * While active, the vehicle receives the slingshot speed bonus.
	 *
	 * @param Vehicle The vehicle to check
	 * @return True if slingshot boost is currently active
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Slingshot")
	bool IsSlingshotActive(AActor* Vehicle) const;

	/**
	 * Gets the slingshot charge level as a percentage (0-100).
	 *
	 * Use this for UI charge meters. 100% = ready to use.
	 * Charge builds while in slipstream, drains when not.
	 *
	 * @param Vehicle The vehicle to query
	 * @return Charge percentage (0.0 to 1.0, multiply by 100 for display)
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Slingshot")
	float GetSlingshotChargePercent(AActor* Vehicle) const;

	//=========================================================================
	// CONFIGURATION
	// Functions to get/set the slipstream system parameters
	//=========================================================================

	/**
	 * Updates the slipstream configuration.
	 *
	 * Use this to change drafting behavior mid-game (e.g., for different
	 * race modes with easier/harder drafting).
	 *
	 * @param NewConfig New configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Config")
	void SetConfig(const FMGSlipstreamConfig& NewConfig);

	/**
	 * Gets the current slipstream configuration.
	 *
	 * @return Current config struct
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Config")
	FMGSlipstreamConfig GetConfig() const { return Config; }

	/**
	 * Updates the visual/audio configuration.
	 *
	 * @param NewVisual New visual settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Config")
	void SetVisualConfig(const FMGSlipstreamVisual& NewVisual);

	/**
	 * Gets the current visual/audio configuration.
	 *
	 * @return Current visual config struct
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Config")
	FMGSlipstreamVisual GetVisualConfig() const { return VisualConfig; }

	//=========================================================================
	// STATISTICS
	// Functions to query and manage slipstream statistics
	//=========================================================================

	/**
	 * Gets accumulated statistics for a vehicle.
	 *
	 * @param Vehicle The vehicle to query
	 * @return Stats struct with all accumulated data
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Stats")
	FMGSlipstreamStats GetStats(AActor* Vehicle) const;

	/**
	 * Resets statistics for a single vehicle.
	 *
	 * Call at the start of a new race or session.
	 *
	 * @param Vehicle The vehicle whose stats to reset
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Stats")
	void ResetStats(AActor* Vehicle);

	/**
	 * Resets statistics for all registered vehicles.
	 *
	 * Call at the start of a new race or session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Stats")
	void ResetAllStats();

	//=========================================================================
	// ADVANCED QUERIES
	// Functions for AI and complex gameplay scenarios
	//=========================================================================

	/**
	 * Gets all vehicles currently drafting behind a specific leader.
	 *
	 * Useful for AI to understand pack dynamics, or for displaying
	 * a "drafting chain" visualization.
	 *
	 * @param LeadVehicle The vehicle being drafted
	 * @return Array of all vehicles in this vehicle's slipstream
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Query")
	TArray<AActor*> GetVehiclesInSlipstream(AActor* LeadVehicle) const;

	/**
	 * Counts how many vehicles are in a connected drafting chain.
	 *
	 * Example: A drafts B, B drafts C, C drafts D = train length of 4.
	 * Useful for scoring bonuses for maintaining pack formations.
	 *
	 * @param LeadVehicle The front vehicle of the potential train
	 * @return Number of connected drafting vehicles
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Query")
	int32 GetDraftingTrainLength(AActor* LeadVehicle) const;

	/**
	 * Finds the best vehicle to draft for a given vehicle.
	 *
	 * Considers distance, angle, and speed to find the optimal target.
	 * Useful for AI-controlled vehicles to make drafting decisions.
	 *
	 * @param Vehicle The vehicle looking for a draft target
	 * @return Best draft target, or nullptr if none suitable
	 */
	UFUNCTION(BlueprintPure, Category = "Slipstream|Query")
	AActor* FindBestDraftTarget(AActor* Vehicle) const;

	//=========================================================================
	// DEBUG
	// Development and testing tools
	//=========================================================================

	/**
	 * Enables/disables debug visualization.
	 *
	 * When enabled, draws drafting cones and slipstream states in the viewport.
	 * Only works in development builds, disabled in shipping builds.
	 *
	 * @param bEnabled True to show debug visualizations
	 */
	UFUNCTION(BlueprintCallable, Category = "Slipstream|Debug")
	void SetDebugDrawEnabled(bool bEnabled);

	//=========================================================================
	// DELEGATES (Bindable Events)
	// Bind to these to react to slipstream events
	//=========================================================================

	/// Broadcast when a vehicle enters another's slipstream.
	/// Parameters: Lead vehicle actor, which zone was entered.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlipstreamEntered OnSlipstreamEntered;

	/// Broadcast when a vehicle exits a slipstream.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlipstreamExited OnSlipstreamExited;

	/// Broadcast when slipstream strength level changes.
	/// Parameter: New strength level.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlipstreamStrengthChanged OnSlipstreamStrengthChanged;

	/// Broadcast when slingshot becomes fully charged and ready.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlingshotReady OnSlingshotReady;

	/// Broadcast when slingshot is activated.
	/// Parameter: Bonus speed being applied.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlingshotActivated OnSlingshotActivated;

	/// Broadcast when slingshot boost duration ends.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnSlingshotEnded OnSlingshotEnded;

	/// Broadcast when nitro is charged from drafting.
	/// Parameter: Amount of nitro charged.
	UPROPERTY(BlueprintAssignable, Category = "Slipstream|Events")
	FOnDraftingNitroCharged OnDraftingNitroCharged;

protected:
	//=========================================================================
	// INTERNAL FUNCTIONS
	// These implement the actual slipstream logic. Called every frame.
	//=========================================================================

	/// Main tick function - called every frame by timer.
	/// Iterates through all vehicles and updates their slipstream states.
	void OnSlipstreamTick();

	/// Updates slipstream state for a single vehicle.
	/// Finds potential leaders, checks if in drafting cone, calculates bonus.
	void UpdateVehicleSlipstream(AActor* Vehicle);

	/// Processes and finalizes the slipstream state for a vehicle.
	/// Handles state transitions, strength changes, and nitro charging.
	void ProcessSlipstreamState(AActor* Vehicle, FMGSlipstreamState& State);

	/// Updates slingshot charge and handles activation/deactivation.
	/// Called for vehicles that are currently in a slipstream.
	void UpdateSlingshot(AActor* Vehicle, FMGSlipstreamState& State);

	/// Finds the best vehicle to draft for the given follower.
	/// Returns the closest suitable leader, or nullptr if none found.
	AActor* FindLeadVehicle(AActor* Vehicle) const;

	/// Checks if the follower is within the leader's drafting cone.
	/// Uses angle and distance calculations.
	bool IsInDraftingCone(AActor* Follower, AActor* Leader) const;

	/// Performs a line trace to check for obstacles between vehicles.
	/// Returns true if there's clear line of sight.
	bool HasLineOfSight(AActor* Follower, AActor* Leader) const;

	/// Calculates the speed bonus based on distance and zone.
	/// Optimal zone = maximum bonus, outer zone = minimum bonus.
	float CalculateSlipstreamBonus(float Distance, EMGDraftingZone Zone) const;

	/// Determines which drafting zone a distance corresponds to.
	/// Maps distance to None/Outer/Inner/Optimal/TooClose.
	EMGDraftingZone DetermineZone(float Distance) const;

	/// Converts charge level (0-1) to a strength enum.
	/// Higher charge = higher strength = more visual feedback.
	EMGSlipstreamStrength DetermineStrength(float ChargeLevel) const;

	/// Draws debug visualization for slipstream state.
	/// Shows cones, zones, and current state in the viewport.
	void DrawDebugSlipstream(AActor* Vehicle, const FMGSlipstreamState& State);

	//=========================================================================
	// INTERNAL STATE
	// These store the subsystem's runtime data
	//=========================================================================

	/// Current slipstream configuration (distances, angles, bonuses).
	UPROPERTY()
	FMGSlipstreamConfig Config;

	/// Visual/audio configuration (colors, effects, sounds).
	UPROPERTY()
	FMGSlipstreamVisual VisualConfig;

	/// Map of all registered vehicles to their current data.
	/// Key = Vehicle Actor, Value = Position/velocity/size data.
	UPROPERTY()
	TMap<AActor*, FMGVehicleSlipstreamData> RegisteredVehicles;

	/// Map of all vehicles to their current slipstream state.
	/// Key = Vehicle Actor, Value = Current drafting state.
	UPROPERTY()
	TMap<AActor*, FMGSlipstreamState> VehicleStates;

	/// Map of all vehicles to their accumulated statistics.
	/// Key = Vehicle Actor, Value = Stats for this session.
	UPROPERTY()
	TMap<AActor*, FMGSlipstreamStats> VehicleStats;

	/// Whether debug visualization is currently enabled.
	UPROPERTY()
	bool bDebugDraw = false;

	/// Timer handle for the main tick function.
	/// Set up in Initialize(), cleared in Deinitialize().
	FTimerHandle SlipstreamTickHandle;

	/// Fixed tick rate for slipstream updates (seconds per tick).
	/// Using fixed timestep for deterministic physics calculations.
	static constexpr float SlipstreamTickInterval = 0.016f;

	/// Accumulated time for internal calculations.
	float AccumulatedTime = 0.0f;
};
