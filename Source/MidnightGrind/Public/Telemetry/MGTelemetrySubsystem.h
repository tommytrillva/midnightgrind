// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGTelemetrySubsystem.h
 * =============================================================================
 *
 * PURPOSE:
 * --------
 * This file defines the Telemetry Subsystem for Midnight Grind - a real-time
 * data recording system that captures detailed vehicle and race information
 * at high frequency. Think of it as the "black box recorder" for races.
 *
 * Telemetry is different from Analytics:
 * - Analytics: High-level events (race started, purchase made) - sent to servers
 * - Telemetry: Granular real-time data (speed every 50ms, tire temps) - used locally
 *
 * KEY CONCEPTS:
 * -------------
 *
 * 1. WORLD SUBSYSTEM:
 *    - This class inherits from UWorldSubsystem, meaning it exists per-world
 *      (per level). It's created when a level loads and destroyed when unloaded.
 *    - This is appropriate for telemetry because it's race-specific data that
 *      doesn't need to persist across levels.
 *    - The TelemetrySubsystem is only active during actual gameplay worlds.
 *
 * 2. TELEMETRY FRAMES:
 *    - The core unit of telemetry data - a snapshot of vehicle state at a moment.
 *    - Captured many times per second (configurable, default 20 FPS / 50ms).
 *    - Contains: speed, RPM, inputs, position, g-forces, tire data, etc.
 *
 * 3. LAPS AND SESSIONS:
 *    - Frames are grouped into Laps (all frames from crossing start line to next).
 *    - Laps are grouped into Sessions (all laps from session start to end).
 *    - This hierarchy allows analysis at multiple granularities.
 *
 * 4. DELTA TIMING:
 *    - Compares current performance against a reference lap (usually personal best).
 *    - "Delta" is the time difference at any given track position.
 *    - Negative delta = ahead of reference, Positive = behind.
 *    - Critical for competitive racing and ghost systems.
 *
 * 5. GHOST REPLAYS:
 *    - Telemetry data can be saved and used to create ghost vehicles.
 *    - Ghosts replay the exact inputs/positions from a previous lap.
 *    - Players can race against their own best lap or others' laps.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 * - The Telemetry Subsystem is accessed via the World:
 *     UMGTelemetrySubsystem* Telemetry = World->GetSubsystem<UMGTelemetrySubsystem>();
 *
 * - The Vehicle class calls RecordFrame() every tick during races.
 *
 * - The Race Manager calls StartSession(), StartLap(), CompleteLap(), EndSession()
 *   at appropriate race lifecycle moments.
 *
 * - The HUD reads current telemetry data for real-time displays.
 *
 * - The Ghost System uses saved sessions to replay vehicle movement.
 *
 * DATA FLOW:
 *   Vehicle -> RecordFrame() -> CurrentLap.Frames[] -> CompleteLap() ->
 *   CurrentSession.Laps[] -> EndSession() -> Save/Export
 *
 * USAGE EXAMPLES:
 * ---------------
 * // Start recording when race begins
 * TelemetrySubsystem->StartSession("DowntownCircuit", "Speedster");
 * TelemetrySubsystem->StartRecording();
 *
 * // Every frame during gameplay, record vehicle state
 * FMGTelemetryFrame Frame;
 * Frame.Speed = Vehicle->GetCurrentSpeed();
 * Frame.RPM = Vehicle->GetEngineRPM();
 * // ... fill other fields
 * TelemetrySubsystem->RecordFrame(Frame);
 *
 * // When lap completes
 * TelemetrySubsystem->CompleteLap(LapTime);
 *
 * // Display delta to best lap
 * float Delta = TelemetrySubsystem->GetCurrentDelta();
 * // Delta < 0 means player is faster than best lap
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGTelemetrySubsystem.generated.h"

/**
 * Telemetry Data Channels
 *
 * Defines the different types of data that can be recorded and displayed.
 * Used for filtering which data to show in overlays or export.
 */
UENUM(BlueprintType)
enum class EMGTelemetryChannel : uint8
{
	Speed,			// Current velocity in km/h or mph
	RPM,			// Engine revolutions per minute
	Gear,			// Current gear (1-6, 0=neutral, -1=reverse)
	Throttle,		// Accelerator input (0.0 to 1.0)
	Brake,			// Brake input (0.0 to 1.0)
	Steering,		// Steering input (-1.0 left to 1.0 right)
	Nitro,			// Nitro/boost amount remaining
	LapTime,		// Current lap elapsed time
	SectorTime,		// Current sector elapsed time
	Position,		// Race position (1st, 2nd, etc.)
	DeltaTime,		// Time difference vs reference lap
	TireTemp,		// Tire temperatures (affects grip)
	TireWear,		// Tire wear percentage (simulated wear)
	FuelLevel,		// Remaining fuel (if fuel system enabled)
	EngineTemp,		// Engine temperature (affects performance)
	Suspension,		// Suspension travel/compression
	GForce,			// G-forces experienced by driver
	Altitude,		// Height above track (for jumps/hills)
	TrackPosition	// Percentage of track completed (0-100%)
};

/**
 * Telemetry Overlay Display Styles
 *
 * Predefined visual styles for the telemetry HUD overlay.
 * Players can choose based on preference or use case.
 */
UENUM(BlueprintType)
enum class EMGTelemetryOverlayStyle : uint8
{
	Minimal,		// Just speed and position - least screen clutter
	Standard,		// Speed, RPM, gear, lap times - good default
	Detailed,		// Adds tire info, g-forces, delta - for serious players
	Professional,	// Full data suite - for esports/competitive play
	Streamer,		// Optimized layout for streaming/recording
	Custom			// User-configured set of elements
};

/**
 * Telemetry Frame - Single Snapshot of Vehicle State
 *
 * This is the core data structure for telemetry. Each frame captures the complete
 * state of the vehicle at a single moment in time. During a race, frames are
 * captured many times per second (typically 20-60 FPS).
 *
 * The vehicle class is responsible for populating this structure and calling
 * RecordFrame() on the telemetry subsystem.
 *
 * Arrays (TireTemperatures, TireWear, etc.) are indexed by wheel:
 *   [0] = Front Left, [1] = Front Right, [2] = Rear Left, [3] = Rear Right
 */
USTRUCT(BlueprintType)
struct FMGTelemetryFrame
{
	GENERATED_BODY()

	// === TIMING ===

	/** Time since lap start in seconds - used for frame ordering and interpolation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	// === SPEED AND ENGINE ===

	/** Current speed in km/h (primary speed value) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	/** Current speed in mph (for regions using imperial) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMPH = 0.0f;

	/** Engine RPM - revolutions per minute (typically 0-10000) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPM = 0.0f;

	/** Current gear: -1=reverse, 0=neutral, 1-6=forward gears */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gear = 0;

	// === PLAYER INPUTS ===

	/** Throttle pedal input: 0.0 (released) to 1.0 (fully pressed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleInput = 0.0f;

	/** Brake pedal input: 0.0 (released) to 1.0 (fully pressed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakeInput = 0.0f;

	/** Steering wheel input: -1.0 (full left) to 1.0 (full right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringInput = 0.0f;

	// === NITRO/BOOST ===

	/** Remaining nitro: 0.0 (empty) to 1.0 (full tank) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroAmount = 0.0f;

	/** True if nitro boost is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNitroActive = false;

	// === POSITION AND MOVEMENT ===

	/** World-space position of the vehicle (X, Y, Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** World-space rotation of the vehicle (Pitch, Yaw, Roll) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	/** Velocity vector - direction and magnitude of movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	/** Acceleration vector - rate of velocity change */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Acceleration = FVector::ZeroVector;

	// === G-FORCES ===
	// G-force = acceleration / gravity. 1G = normal gravity.
	// Racing cars can experience 2-5G in corners and braking.

	/** G-force vector (X=lateral, Y=longitudinal, Z=vertical) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GForce = FVector::ZeroVector;

	/** Lateral (sideways) G-force - high in corners */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralG = 0.0f;

	/** Longitudinal (forward/back) G-force - high during acceleration/braking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalG = 0.0f;

	// === TIRE DATA ===
	// Arrays indexed: [0]=FL, [1]=FR, [2]=RL, [3]=RR

	/** Tire temperatures in Celsius - affects grip (optimal ~80-100C) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> TireTemperatures;

	/** Tire wear: 1.0 (new) to 0.0 (worn out) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> TireWear;

	/** Suspension compression: 0.0 (extended) to 1.0 (compressed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SuspensionTravel;

	/** Wheel slip ratio: 0.0 (no slip) to 1.0+ (spinning/locking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> WheelSlip;

	// === ENGINE AND FUEL ===

	/** Engine temperature in Celsius - overheating reduces performance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineTemperature = 0.0f;

	/** Oil temperature in Celsius - simulation detail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OilTemperature = 0.0f;

	/** Remaining fuel: 1.0 (full) to 0.0 (empty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelLevel = 0.0f;

	// === RACE PROGRESS ===

	/** Percentage of lap completed: 0.0 (start line) to 1.0 (finish line) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackPercentage = 0.0f;

	/** Current lap number (1-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	/** Current track sector (usually 1-3 per lap) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSector = 0;

	/** Current race position (1 = first place) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacePosition = 0;

	// === DRIFT STATE ===

	/** True if the vehicle is currently in a drift */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDrifting = false;

	/** Angle between velocity and heading in degrees (0 = straight, 90 = sideways) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftAngle = 0.0f;
};

/**
 * Lap Telemetry - Complete Data for One Lap
 *
 * Aggregates all telemetry frames and statistics for a single lap.
 * Created when crossing the start/finish line and finalized when crossing again.
 *
 * This structure is used for:
 * - Personal best comparisons
 * - Ghost replay creation
 * - Lap time leaderboards
 * - Driving analysis and coaching
 */
USTRUCT(BlueprintType)
struct FMGLapTelemetry
{
	GENERATED_BODY()

	// === LAP IDENTIFICATION ===

	/** Which lap this is (1-based: first lap = 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	/** Total time to complete this lap in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	/** Time for each sector (tracks typically have 3 sectors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorTimes;

	// === SPEED STATISTICS ===

	/** Highest speed reached during this lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	/** Average speed over the entire lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	// === DRIVING STYLE METRICS ===

	/** Highest gear used during the lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopGear = 0;

	/** Number of gear changes - efficiency metric */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearShifts = 0;

	/** Amount of nitro consumed (0.0 to 1.0 scale) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroUsed = 0.0f;

	/** Total distance traveled while braking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDistance = 0.0f;

	// === DRIFT STATISTICS ===

	/** Cumulative drift angle in degrees (sum of all drift angles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDriftAngle = 0.0f;

	/** Number of separate drift events */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftCount = 0;

	// === G-FORCE PEAKS ===

	/** Highest lateral G-force experienced (cornering intensity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLateralG = 0.0f;

	/** Highest longitudinal G-force (hardest braking/acceleration) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLongitudinalG = 0.0f;

	// === RAW FRAME DATA ===

	/** All recorded frames for this lap - used for ghost replay and analysis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTelemetryFrame> Frames;

	// === STATUS ===

	/** True if this is the player's best lap on this track/vehicle combo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;
};

/**
 * Telemetry Session - All Data from a Race/Practice Session
 *
 * The top-level container for telemetry data. A session represents a complete
 * race or practice run, containing all laps and aggregate statistics.
 *
 * Sessions can be:
 * - Saved to disk for later analysis
 * - Exported to CSV/JSON for external tools
 * - Used to create ghost replays
 * - Shared with other players
 */
USTRUCT(BlueprintType)
struct FMGTelemetrySession
{
	GENERATED_BODY()

	// === SESSION IDENTIFICATION ===

	/** Unique identifier for this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SessionID;

	/** When the session started */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	/** When the session ended */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	// === CONTEXT ===

	/** Which track was raced */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Which vehicle was used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	// === LAP DATA ===

	/** All completed laps in this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLapTelemetry> Laps;

	/** Quick reference to the fastest lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapTelemetry BestLap;

	// === AGGREGATE STATISTICS ===

	/** Total distance driven in this session (all laps combined) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.0f;

	/** Total time spent in this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	/** Number of completed laps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLaps = 0;
};

/**
 * Telemetry Comparison - Delta Timing Between Laps
 *
 * Used for real-time comparison between the current lap and a reference lap
 * (usually personal best or a ghost). The "delta" shows how far ahead or
 * behind the player is at any given point on the track.
 *
 * How delta works:
 * - At any track position, compare current time to reference time at same position
 * - Negative delta = player is FASTER (ahead of reference)
 * - Positive delta = player is SLOWER (behind reference)
 *
 * Example: Delta of -0.5 means "you will finish 0.5 seconds faster at this pace"
 */
USTRUCT(BlueprintType)
struct FMGTelemetryComparison
{
	GENERATED_BODY()

	/** The lap being compared against (personal best, ghost, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapTelemetry ReferenceLap;

	/** The current lap in progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapTelemetry CurrentLap;

	/** Current time difference in seconds (negative = faster, positive = slower) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MGDeltaTime = 0.0f;

	/** Delta values at regular distance intervals for graphing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> DeltaAtDistance;

	/** True if player is currently ahead of reference pace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAhead = false;
};

/**
 * Telemetry Overlay Configuration
 *
 * Settings for the on-screen telemetry display (HUD overlay).
 * Players can customize what information they want to see during races.
 *
 * The overlay is drawn by the HUD class using data from this config
 * and current telemetry frame data.
 */
USTRUCT(BlueprintType)
struct FMGTelemetryOverlayConfig
{
	GENERATED_BODY()

	// === STYLE PRESET ===

	/** Preset style - sets sensible defaults for all toggles below */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTelemetryOverlayStyle Style = EMGTelemetryOverlayStyle::Standard;

	// === ELEMENT TOGGLES ===

	/** Show speedometer (digital or analog display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeed = true;

	/** Show RPM gauge (tachometer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRPM = true;

	/** Show current gear indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGear = true;

	/** Show throttle/brake/steering input bars */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowInputs = true;

	/** Show G-force meter/ball */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGForce = true;

	/** Show tire temperature and wear indicators */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTireInfo = false;

	/** Show delta time vs reference lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDelta = true;

	/** Show track minimap with positions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMinimap = true;

	/** Show lap counter and lap times */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLapInfo = true;

	// === APPEARANCE SETTINGS ===

	/** Overlay transparency: 0.0 (invisible) to 1.0 (solid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlayOpacity = 0.8f;

	/** Screen position offset for the overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D OverlayPosition = FVector2D(0.0f, 0.0f);

	/** Size multiplier for the overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlayScale = 1.0f;
};

// ============================================================================
// DELEGATES
// ============================================================================
// Delegates allow other systems to react to telemetry events.
// The HUD binds to these to update displays, the ghost system uses
// OnLapCompleted to save ghost data, etc.

/** Broadcast every time a telemetry frame is recorded (high frequency!) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTelemetryFrameRecorded, const FMGTelemetryFrame&, Frame);

/** Broadcast when a lap is completed (crossing finish line) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompleted, const FMGLapTelemetry&, LapData);

/** Broadcast when a sector is completed (passing sector markers) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectorCompleted, int32, Sector, float, SectorTime);

/** Broadcast when a new personal best lap time is set */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPersonalBest, const FMGLapTelemetry&, BestLap);

/** Broadcast whenever delta time changes significantly (for HUD updates) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeltaUpdated, float, DeltaTime);

// ============================================================================
// MAIN TELEMETRY SUBSYSTEM CLASS
// ============================================================================

/**
 * UMGTelemetrySubsystem
 *
 * Manages real-time vehicle telemetry data during races. Unlike the Analytics
 * subsystem which tracks high-level events, Telemetry captures granular
 * moment-to-moment vehicle state.
 *
 * SUBSYSTEM LIFECYCLE:
 * - This is a World Subsystem, created when a level loads
 * - ShouldCreateSubsystem() filters to only create during gameplay levels
 * - Destroyed when the level unloads
 *
 * TYPICAL USAGE FLOW:
 * 1. Race Manager calls StartSession(TrackID, VehicleID)
 * 2. Race Manager calls StartRecording() when countdown finishes
 * 3. Vehicle calls RecordFrame() every tick during race
 * 4. Race Manager calls StartLap() when crossing start line
 * 5. Race Manager calls CompleteLap(time) when completing a lap
 * 6. Race Manager calls StopRecording() and EndSession() when race ends
 *
 * ACCESSING THE SUBSYSTEM:
 *   // From any Actor with a World context
 *   UMGTelemetrySubsystem* Telemetry = GetWorld()->GetSubsystem<UMGTelemetrySubsystem>();
 *
 * PERFORMANCE CONSIDERATIONS:
 * - RecordFrame() is called very frequently (20-60 times per second)
 * - Frames are stored in memory during the race
 * - Memory usage: ~200 bytes per frame, ~4000 frames per minute at 60 FPS
 * - Sessions should be saved/cleared after races to free memory
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTelemetrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Called when subsystem is created - sets up initial state */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when subsystem is destroyed - cleanup */
	virtual void Deinitialize() override;

	/**
	 * Determines if this subsystem should be created for a given world.
	 * Returns true only for gameplay worlds (not editor previews, etc.)
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// RECORDING CONTROL
	// ==========================================
	// Start/stop telemetry recording. Recording should be active
	// during actual gameplay but paused during menus/cutscenes.

	/**
	 * Begin recording telemetry frames.
	 * Call when the race actually starts (after countdown).
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void StartRecording();

	/**
	 * Stop recording and finalize current data.
	 * Call when the race ends.
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void StopRecording();

	/**
	 * Temporarily pause recording (e.g., during pause menu).
	 * Use ResumeRecording() to continue.
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void PauseRecording();

	/** Resume recording after a pause */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void ResumeRecording();

	/** Check if currently recording telemetry data */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Recording")
	bool IsRecording() const { return bIsRecording; }

	/**
	 * Set how many frames per second to record.
	 * Higher = more precision but more memory. Default is 20 FPS.
	 *
	 * @param FramesPerSecond - Recording frequency (10-60 recommended)
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void SetRecordingRate(float FramesPerSecond);

	// ==========================================
	// CURRENT FRAME DATA
	// ==========================================
	// Functions for recording and retrieving individual frames.

	/**
	 * Record a telemetry frame. Call this from the vehicle every tick.
	 * The frame will be added to the current lap's frame array.
	 *
	 * @param Frame - Fully populated telemetry frame structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Data")
	void RecordFrame(const FMGTelemetryFrame& Frame);

	/** Get the most recently recorded frame */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Data")
	FMGTelemetryFrame GetCurrentFrame() const { return CurrentFrame; }

	/**
	 * Retrieve the frame closest to a given timestamp.
	 * Uses interpolation if exact timestamp not available.
	 *
	 * @param Timestamp - Time in seconds since lap start
	 * @return Frame data at or near that timestamp
	 */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Data")
	FMGTelemetryFrame GetFrameAtTime(float Timestamp) const;

	// ==========================================
	// LAP MANAGEMENT
	// ==========================================
	// Track lap boundaries and sector times. The Race Manager should call
	// these functions when the vehicle crosses timing lines.

	/**
	 * Begin a new lap. Call when crossing start/finish line.
	 * Resets the current lap frame buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Lap")
	void StartLap();

	/**
	 * Complete the current lap. Call when crossing finish line.
	 * Finalizes lap statistics and adds to session.
	 * Automatically checks for personal best.
	 *
	 * @param LapTime - Official lap time from timing system
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Lap")
	void CompleteLap(float LapTime);

	/**
	 * Record completion of a track sector.
	 * Sectors divide the track for intermediate timing.
	 *
	 * @param Sector - Sector number (typically 1, 2, or 3)
	 * @param SectorTime - Time for this sector
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Lap")
	void CompleteSector(int32 Sector, float SectorTime);

	/** Get telemetry for the lap currently in progress */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Lap")
	FMGLapTelemetry GetCurrentLapTelemetry() const { return CurrentLap; }

	/** Get telemetry for the best lap in this session */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Lap")
	FMGLapTelemetry GetBestLapTelemetry() const { return BestLap; }

	/** Get telemetry for all completed laps in this session */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Lap")
	TArray<FMGLapTelemetry> GetAllLapsTelemetry() const;

	// ==========================================
	// SESSION MANAGEMENT
	// ==========================================
	// Sessions group all laps from a single race/practice run.

	/**
	 * Start a new telemetry session.
	 * Call before the race begins (during loading or pre-race screen).
	 *
	 * @param TrackID - Identifier for the track being raced
	 * @param VehicleID - Identifier for the vehicle being used
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	void StartSession(FName TrackID, FName VehicleID);

	/**
	 * End the current session.
	 * Finalizes all data and prepares for saving/export.
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	void EndSession();

	/** Get the current session data */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Session")
	FMGTelemetrySession GetCurrentSession() const { return CurrentSession; }

	/**
	 * Save the current session to a file.
	 * Can be loaded later for ghost replay or analysis.
	 *
	 * @param Filename - Path to save the session (e.g., "Track_Vehicle_Date.telem")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	void SaveSession(const FString& Filename);

	/**
	 * Load a previously saved session from file.
	 *
	 * @param Filename - Path to the session file
	 * @return True if loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	bool LoadSession(const FString& Filename);

	// ==========================================
	// COMPARISON / DELTA TIMING
	// ==========================================
	// Compare current performance against a reference lap.
	// Essential for the "delta bar" HUD element showing +/- time.

	/**
	 * Set a specific lap as the reference for comparison.
	 *
	 * @param Lap - The lap telemetry to compare against
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Comparison")
	void SetReferenceLap(const FMGLapTelemetry& Lap);

	/**
	 * Use the current session's best lap as reference.
	 * Common choice - player races against their own best.
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Comparison")
	void SetReferenceLapFromBest();

	/**
	 * Load a ghost's lap data as reference.
	 * Used when racing against downloaded ghosts or friends' times.
	 *
	 * @param GhostID - Identifier for the ghost data to load
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Comparison")
	void SetReferenceLapFromGhost(FName GhostID);

	/**
	 * Get the current time delta at player's current position.
	 * Negative = faster than reference, Positive = slower.
	 *
	 * @return Delta time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Comparison")
	float GetCurrentDelta() const;

	/**
	 * Get the delta at a specific track distance.
	 * Useful for graphing delta over the lap.
	 *
	 * @param Distance - Track distance in Unreal units
	 * @return Delta time at that distance
	 */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Comparison")
	float GetDeltaAtDistance(float Distance) const;

	/** Get the full comparison data structure */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Comparison")
	FMGTelemetryComparison GetComparison() const { return Comparison; }

	// ==========================================
	// OVERLAY CONFIGURATION
	// ==========================================
	// Settings for the on-screen telemetry HUD. The HUD class reads
	// this configuration to determine what to display.

	/**
	 * Apply a complete overlay configuration.
	 *
	 * @param Config - Full configuration structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Overlay")
	void SetOverlayConfig(const FMGTelemetryOverlayConfig& Config);

	/** Get the current overlay configuration */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Overlay")
	FMGTelemetryOverlayConfig GetOverlayConfig() const { return OverlayConfig; }

	/**
	 * Show or hide the entire telemetry overlay.
	 *
	 * @param bVisible - True to show, false to hide
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Overlay")
	void SetOverlayVisible(bool bVisible);

	/** Check if the overlay is currently visible */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Overlay")
	bool IsOverlayVisible() const { return bOverlayVisible; }

	/**
	 * Change the overlay style preset.
	 * Automatically updates which elements are shown.
	 *
	 * @param Style - Preset to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Overlay")
	void SetOverlayStyle(EMGTelemetryOverlayStyle Style);

	// ==========================================
	// DATA EXPORT
	// ==========================================
	// Export telemetry data for external analysis tools.

	/**
	 * Export session data to CSV format.
	 * Can be opened in Excel or imported into analysis tools.
	 *
	 * @param Filename - Output file path (e.g., "MyLap.csv")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Export")
	void ExportToCSV(const FString& Filename);

	/**
	 * Export session data to JSON format.
	 * More structured than CSV, preserves hierarchy.
	 *
	 * @param Filename - Output file path (e.g., "MyLap.json")
	 */
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Export")
	void ExportToJSON(const FString& Filename);

	/**
	 * Get current frame data as a formatted string.
	 * Useful for debug displays.
	 *
	 * @return Human-readable telemetry string
	 */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Export")
	FString GetTelemetryAsString() const;

	// ==========================================
	// ANALYSIS
	// ==========================================
	// Query computed statistics from recorded data.

	/** Get average speed over the current/last lap */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetAverageSpeed() const;

	/** Get top speed reached in current/last lap */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetMaxSpeed() const;

	/** Get peak G-force (lateral or longitudinal) from current/last lap */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetMaxGForce() const;

	/**
	 * Calculate braking efficiency metric.
	 * Higher = better threshold braking without locking wheels.
	 *
	 * @return Efficiency percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetBrakingEfficiency() const;

	/**
	 * Get the driving line (position at each frame).
	 * Can be used to render the optimal racing line.
	 *
	 * @return Array of positions forming the driving path
	 */
	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	TArray<FVector> GetDrivingLine() const;

	// ==========================================
	// DELEGATE EVENTS
	// ==========================================
	// Bind to these to react to telemetry events.

	/**
	 * Broadcast every time a frame is recorded.
	 * WARNING: Very high frequency! Use sparingly.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnTelemetryFrameRecorded OnTelemetryFrameRecorded;

	/** Broadcast when a lap is completed */
	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnLapCompleted OnLapCompleted;

	/** Broadcast when a sector is completed */
	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnSectorCompleted OnSectorCompleted;

	/** Broadcast when a new personal best is set */
	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnPersonalBest OnPersonalBest;

	/** Broadcast when delta time changes (for HUD updates) */
	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnDeltaUpdated OnDeltaUpdated;

protected:
	// ==========================================
	// INTERNAL FUNCTIONS
	// ==========================================

	/** Called by timer at recording interval */
	void OnTelemetryTick();

	/** Process the current frame data (update statistics, etc.) */
	void ProcessCurrentFrame();

	/** Recalculate delta time against reference lap */
	void UpdateComparison();

	/** Update running statistics for current lap (max speed, etc.) */
	void UpdateLapStatistics();

	/**
	 * Blend between two frames for smooth interpolation.
	 * Used when retrieving frames at arbitrary timestamps.
	 *
	 * @param A - First frame
	 * @param B - Second frame
	 * @param Alpha - Blend factor (0.0 = A, 1.0 = B)
	 * @return Interpolated frame
	 */
	FMGTelemetryFrame InterpolateFrames(const FMGTelemetryFrame& A, const FMGTelemetryFrame& B, float Alpha) const;

	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Most recently recorded frame */
	UPROPERTY()
	FMGTelemetryFrame CurrentFrame;

	/** Lap currently in progress */
	UPROPERTY()
	FMGLapTelemetry CurrentLap;

	/** Best lap in current session */
	UPROPERTY()
	FMGLapTelemetry BestLap;

	/** Lap used for delta comparison (could be best, ghost, etc.) */
	UPROPERTY()
	FMGLapTelemetry ReferenceLap;

	/** Current telemetry session containing all laps */
	UPROPERTY()
	FMGTelemetrySession CurrentSession;

	/** Comparison data between current and reference lap */
	UPROPERTY()
	FMGTelemetryComparison Comparison;

	/** HUD overlay display settings */
	UPROPERTY()
	FMGTelemetryOverlayConfig OverlayConfig;

	// === STATE FLAGS ===

	/** True when actively recording frames */
	UPROPERTY()
	bool bIsRecording = false;

	/** True when recording is temporarily paused */
	UPROPERTY()
	bool bIsPaused = false;

	/** True if telemetry overlay should be displayed */
	UPROPERTY()
	bool bOverlayVisible = true;

	/** Time between frame recordings in seconds (0.05 = 20 FPS) */
	UPROPERTY()
	float RecordingInterval = 0.05f;

	/** Cumulative distance traveled in current lap */
	UPROPERTY()
	float TotalDistance = 0.0f;

	/** Timer handle for the recording tick */
	FTimerHandle TelemetryTickHandle;
};
