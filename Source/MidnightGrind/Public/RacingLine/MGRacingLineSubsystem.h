// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGRacingLineSubsystem.h
 * @brief Racing Line Subsystem - Runtime management of racing lines and driver assists
 *
 * This subsystem provides real-time racing line services for both AI opponents and
 * player assistance features. It manages loaded racing lines, tracks vehicle
 * positions against the optimal line, and provides driver assistance data.
 *
 * KEY RESPONSIBILITIES:
 * ---------------------
 * 1. RACING LINE MANAGEMENT
 *    - Load/unload racing lines for different tracks and vehicle classes
 *    - Support multiple line types (optimal, safe, aggressive, wet weather)
 *    - Store and retrieve custom/recorded racing lines
 *
 * 2. VEHICLE TRACKING
 *    - Track multiple vehicles against the racing line
 *    - Calculate deviation from optimal path and speed
 *    - Generate performance scores and feedback
 *
 * 3. DRIVER ASSISTANCE
 *    - Provide speed advisories and gear suggestions
 *    - Detect approaching corners and braking zones
 *    - Generate visual racing line data for HUD display
 *
 * 4. LINE RECORDING
 *    - Record player racing lines for analysis
 *    - Compare recorded lines against optimal
 *    - Allow saving custom lines for later use
 *
 * RACING LINE TYPES EXPLAINED:
 * ----------------------------
 * - Optimal: Fastest theoretical line, uses full track width
 * - Safe: More conservative, larger safety margins
 * - Aggressive: Pushes limits, late braking, track limit usage
 * - DriftLine: Optimized for drift scoring, not pure speed
 * - FuelSaving: Minimizes braking/acceleration for endurance
 * - WetWeather: Avoids standing water, gentler inputs
 *
 * TYPICAL USAGE FLOW:
 * 1. Load racing line for current track: LoadRacingLine(TrackID, LineType)
 * 2. Register vehicles to track: RegisterVehicle(VehicleID)
 * 3. Update vehicle state each frame: UpdateVehiclePosition(...)
 * 4. Query deviation and recommendations: GetVehicleDeviation(...)
 * 5. Unload when leaving track: UnloadRacingLine()
 *
 * @see UMGRacingLineGenerator - Creates racing lines from track data
 * @see AMGRacingAIController - AI uses this subsystem for path following
 * @see UMGHUDWidget - Uses visualization data for racing line display
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRacingLineSubsystem.generated.h"

// ============================================================================
// RACING LINE TYPE ENUMERATIONS
// ============================================================================

/**
 * Racing line style/type classification
 *
 * Different driving situations call for different racing lines.
 * The subsystem can store and switch between multiple line types
 * for the same track.
 */
UENUM(BlueprintType)
enum class EMGRacingLineType : uint8
{
	/// Mathematically optimal line for fastest lap time
	Optimal,

	/// Conservative line with extra safety margins
	Safe,

	/// Aggressive line that pushes track limits
	Aggressive,

	/// Line optimized for drift angle and scoring
	DriftLine,

	/// Eco-friendly line minimizing energy use
	FuelSaving,

	/// Adapted line for wet/slippery conditions
	WetWeather,

	/// User-defined or recorded custom line
	Custom
};

/**
 * Track segment classification for the current racing line point
 *
 * Tells you what kind of section the vehicle is currently in,
 * useful for adjusting AI behavior or UI display.
 */
UENUM(BlueprintType)
enum class EMGLineSegmentType : uint8
{
	/// Minimal curvature, full throttle section
	Straight,

	/// Curved section requiring steering and possibly braking
	Corner,

	/// Very tight turn (often 180+ degrees)
	Hairpin,

	/// Rapid direction changes (S-curves)
	Chicane,

	/// Section where braking is required
	Braking,

	/// Section where full throttle is possible after a slow section
	Acceleration,

	/// Section designed for initiating/maintaining drifts
	DriftZone,

	/// Section optimal for drafting/slipstreaming
	Slipstream
};

/**
 * Visual display mode for the racing line assist
 *
 * Controls how the racing line is rendered for the player.
 */
UENUM(BlueprintType)
enum class EMGLineVisualMode : uint8
{
	/// Racing line hidden
	Off,

	/// Basic line with color coding (green/red/yellow)
	Simple,

	/// Line with speed numbers and brake markers
	Detailed,

	/// 3D ribbon showing ideal path through corners
	ThreeD,

	/// Augmented reality style with floating markers
	AR,

	/// Shows predicted position based on current speed
	Predictive
};

/**
 * Braking intensity indicator
 *
 * Provides a discrete indication of how hard to brake,
 * useful for HUD displays and audio cues.
 */
UENUM(BlueprintType)
enum class EMGBrakingIndicator : uint8
{
	/// No braking needed - full throttle
	None,

	/// Light braking or lift-off
	Light,

	/// Moderate braking
	Medium,

	/// Heavy braking required
	Heavy,

	/// Maximum braking (emergency or very tight corner)
	MaxBraking
};

/**
 * Corner phase for detailed corner guidance
 *
 * Breaks down corner navigation into distinct phases,
 * each requiring different driver inputs.
 */
UENUM(BlueprintType)
enum class EMGCornerPhase : uint8
{
	/// Approaching the corner, still at high speed
	Approach,

	/// In the braking zone, decelerating
	BrakingZone,

	/// Initiating the turn, steering input begins
	TurnIn,

	/// At the apex, clipping the inside of the corner
	Apex,

	/// Exiting the corner, unwinding steering
	Exit,

	/// Past the corner, accelerating to next section
	Acceleration
};

// ============================================================================
// RACING LINE DATA STRUCTURES
// ============================================================================

/**
 * Single point on a racing line with full driving data
 *
 * This is the primary data structure for racing line information.
 * Contains position, speed targets, and driving recommendations.
 *
 * AI USAGE: Controllers query these points to determine target
 * position, speed, and steering inputs.
 *
 * PLAYER ASSIST: Used to show the racing line, provide speed
 * warnings, and calculate deviation scores.
 */
USTRUCT(BlueprintType)
struct FMGRacingLinePoint
{
	GENERATED_BODY()

	// --- Position Data ---

	/** World space position of this racing line point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldPosition = FVector::ZeroVector;

	/** Forward direction at this point (normalized) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Direction = FVector::ForwardVector;

	/** Distance from start/finish line in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongTrack = 0.0f;

	// --- Speed Targets ---

	/** Target speed at this point (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalSpeed = 0.0f;

	/** Minimum safe speed (lower = off track or spin risk) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeed = 0.0f;

	/** Maximum achievable speed (vehicle limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	// --- Track Geometry ---

	/** Track width at this point (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackWidth = 10.0f;

	/** Lateral offset from centerline (+ = right, - = left) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralOffset = 0.0f;

	/** Curvature at this point (1/radius, 0 = straight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Curvature = 0.0f;

	/** Track gradient (+ = uphill, - = downhill, degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Grade = 0.0f;

	/** Track camber/banking (+ = banked into corner, degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Camber = 0.0f;

	// --- Segment Classification ---

	/** What type of section this point is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLineSegmentType SegmentType = EMGLineSegmentType::Straight;

	/** Braking intensity required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBrakingIndicator BrakingLevel = EMGBrakingIndicator::None;

	// --- Control Recommendations ---

	/** Recommended throttle position (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottlePercent = 1.0f;

	/** Recommended brake position (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakePercent = 0.0f;

	/** Suggested gear (0 = auto/no suggestion) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearSuggestion = 0;

	// --- Key Point Markers ---

	/** True if this is a corner apex point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bApexPoint = false;

	/** True if this is where braking should begin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBrakingPoint = false;

	/** True if this is the turn-in point for a corner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTurnInPoint = false;
};

/**
 * Corner data for UI and telemetry
 *
 * Provides comprehensive information about a single corner,
 * used for corner info displays and AI strategy.
 */
USTRUCT(BlueprintType)
struct FMGCornerData
{
	GENERATED_BODY()

	/** Sequential corner number on this track (1, 2, 3...) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CornerNumber = 0;

	/** Named corner (e.g., "Deadman's Curve", "Neon Hairpin") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CornerName;

	/** Distance along track where corner zone begins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntryDistance = 0.0f;

	/** Distance along track where corner zone ends */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExitDistance = 0.0f;

	/** Distance to the apex point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ApexDistance = 0.0f;

	/** World position of the apex */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ApexPosition = FVector::ZeroVector;

	/** World position where braking should begin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BrakingPoint = FVector::ZeroVector;

	/** World position for turn-in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TurnInPoint = FVector::ZeroVector;

	/** Total angle of the corner in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CornerAngle = 0.0f;

	/** Target speed when entering braking zone (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalEntrySpeed = 0.0f;

	/** Target speed at the apex (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalApexSpeed = 0.0f;

	/** Target speed at corner exit (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalExitSpeed = 0.0f;

	/** True if corner turns left */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeftHander = true;

	/** True if this is a hairpin (very tight corner) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHairpin = false;

	/** Recommended gear through the corner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecommendedGear = 3;

	/** Corner difficulty rating (1.0 = easy, 5.0 = very difficult) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Difficulty = 1.0f;
};

/**
 * Complete racing line data for a track
 *
 * Contains all points and metadata for one racing line.
 * Multiple FMGRacingLine structs can exist for different
 * line types on the same track.
 */
USTRUCT(BlueprintType)
struct FMGRacingLine
{
	GENERATED_BODY()

	/** Track identifier this line is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Type of racing line (optimal, safe, drift, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRacingLineType LineType = EMGRacingLineType::Optimal;

	/** Array of all points comprising this racing line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRacingLinePoint> Points;

	/** Corner data for this track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCornerData> Corners;

	/** Total track/line length in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.0f;

	/** Estimated lap time following this line perfectly (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedLapTime = 0.0f;

	/** Vehicle class this line was generated for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClass;

	/** When this line was created/last updated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedDate;

	/** Has this line been validated/tested */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bValidated = false;
};

// ============================================================================
// DRIVER ASSIST CONFIGURATION
// ============================================================================

/**
 * Driver assist display and behavior settings
 *
 * Controls what racing line assistance is shown to the player
 * and how it behaves. Can be adjusted per-player or per-difficulty.
 */
USTRUCT(BlueprintType)
struct FMGDriverAssistSettings
{
	GENERATED_BODY()

	// --- Feature Toggles ---

	/** Master toggle for racing line visibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRacingLine = true;

	/** How the racing line is rendered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLineVisualMode VisualMode = EMGLineVisualMode::Simple;

	/** Show brake point markers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowBrakingPoints = true;

	/** Show apex markers on corners */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowApexMarkers = true;

	/** Show speed recommendations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedAdvisor = true;

	/** Show gear suggestions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGearSuggestion = true;

	/** Show corner names/numbers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCornerNames = false;

	/** Show predictive racing line based on current speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPredictiveLine = false;

	// --- Visual Settings ---

	/** Racing line transparency (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineOpacity = 0.8f;

	/** Racing line width in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineWidth = 0.5f;

	/** Color for acceleration zones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AccelerateColor = FLinearColor::Green;

	/** Color for braking zones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BrakeColor = FLinearColor::Red;

	/** Color for coasting/neutral zones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CoastColor = FLinearColor::Yellow;

	// --- Distance Settings ---

	/** How far ahead to show the racing line (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookAheadDistance = 100.0f;

	/** Distance over which line fades out (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeDistance = 50.0f;
};

// ============================================================================
// PERFORMANCE TRACKING STRUCTURES
// ============================================================================

/**
 * Instantaneous deviation from the racing line
 *
 * Updated each frame for tracked vehicles, showing how well
 * they're following the optimal line.
 */
USTRUCT(BlueprintType)
struct FMGLineDeviation
{
	GENERATED_BODY()

	/** Lateral distance from optimal line (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralDeviation = 0.0f;

	/** Difference from optimal speed (m/s, + = too fast) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedDeviation = 0.0f;

	/** Difference from optimal throttle (+ = more throttle than needed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleDeviation = 0.0f;

	/** Difference from optimal brake (+ = more brake than needed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakeDeviation = 0.0f;

	/** True if vehicle is within acceptable distance of line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnLine = true;

	/** True if vehicle is going faster than optimal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTooFast = false;

	/** True if vehicle is going slower than optimal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTooSlow = false;

	/** Overall deviation score (100 = perfect, 0 = way off) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeviationScore = 100.0f;
};

/**
 * Accumulated performance data for line following
 *
 * Tracks how well a vehicle has followed the racing line
 * over time, useful for scoring and telemetry.
 *
 * This struct accumulates data throughout a lap/race and provides
 * detailed feedback on driver performance relative to the optimal line.
 *
 * @see UMGRacingLineSubsystem::GetVehiclePerformance
 */
USTRUCT(BlueprintType)
struct FMGLinePerformance
{
	GENERATED_BODY()

	/** Vehicle being tracked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identification")
	FName VehicleID;

	// --- Deviation Metrics ---

	/** Average lateral deviation across the lap (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deviation")
	float AverageDeviation = 0.0f;

	/** Deviation by sector (array index = sector number) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Deviation")
	TArray<float> SectorDeviations;

	// --- Corner Performance ---

	/** Best corner score achieved (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	float BestCornerScore = 0.0f;

	/** Worst corner score (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	float WorstCornerScore = 0.0f;

	/** Which corner had the worst performance (1-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	int32 WorstCornerNumber = 0;

	/** Score for each corner (array index = corner number - 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	TArray<float> CornerScores;

	/** Total number of corners taken during this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	int32 TotalCornersTaken = 0;

	/** Number of corners where apex was hit perfectly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	int32 PerfectApexes = 0;

	/** Percentage of apexes hit correctly (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Corners")
	float ApexHitPercentage = 0.0f;

	// --- Braking Performance ---

	/** Braking efficiency score (0-100, 100 = optimal brake points) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Braking")
	float BrakingEfficiency = 0.0f;

	// --- Overall Scores ---

	/** Overall line-following score for the lap (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overall")
	float OverallLineScore = 0.0f;

	/** Consistency score - how stable the performance is (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overall")
	float ConsistencyScore = 0.0f;

	/** Estimated time lost compared to optimal (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overall")
	float TimeToOptimal = 0.0f;
};

/**
 * Braking zone data for driver feedback
 *
 * Provides detailed information about upcoming or current
 * braking zones for HUD display and audio cues.
 */
USTRUCT(BlueprintType)
struct FMGBrakingZone
{
	GENERATED_BODY()

	/** Distance where braking zone begins (cm from start) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistance = 0.0f;

	/** Distance where braking zone ends */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndDistance = 0.0f;

	/** World position of braking zone start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPosition = FVector::ZeroVector;

	/** World position of braking zone end */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndPosition = FVector::ZeroVector;

	/** Speed when entering braking zone (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	/** Target speed at end of braking zone (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExitSpeed = 0.0f;

	/** Recommended brake force (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalBrakeForce = 0.0f;

	/** Which corner this braking zone is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AssociatedCorner = 0;

	/** True if braking zone is on a downhill section */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDownhill = false;

	/** Total length of braking zone (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDistance = 0.0f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Fired when a racing line is loaded for a track
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRacingLineLoaded, FName, TrackID, EMGRacingLineType, LineType);

/// Fired when a vehicle's deviation is updated (per frame)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLineDeviationUpdated, FName, VehicleID, const FMGLineDeviation&, Deviation);

/// Fired when a tracked vehicle is approaching a corner
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCornerApproaching, FName, VehicleID, const FMGCornerData&, Corner);

/// Fired when a tracked vehicle enters a braking zone
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrakingZoneEntered, FName, VehicleID, const FMGBrakingZone&, Zone);

/// Fired when vehicle speed significantly differs from optimal
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedWarning, FName, VehicleID, float, SpeedDifference);

/// Fired when driver assist settings are changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistSettingsChanged, const FMGDriverAssistSettings&, NewSettings);

//=============================================================================
// Wrapper Structs for TMap Value Types
//=============================================================================

/**
 * @brief Wrapper for TArray<FMGRacingLine> to support UPROPERTY in TMap.
 */
USTRUCT(BlueprintType)
struct FMGRacingLineArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRacingLine> Lines;
};

// ============================================================================
// RACING LINE SUBSYSTEM CLASS
// ============================================================================

/**
 * Racing Line Subsystem
 *
 * Game instance subsystem that manages racing lines and provides
 * driver assistance features. Persists across level loads.
 *
 * PRIMARY FUNCTIONS:
 * - Load and manage racing lines for tracks
 * - Track vehicle positions against the optimal line
 * - Provide deviation feedback and recommendations
 * - Generate visual data for HUD racing line display
 * - Record and save player racing lines
 *
 * @see FMGRacingLine - Complete racing line data
 * @see FMGRacingLinePoint - Individual line point
 * @see FMGDriverAssistSettings - Assist configuration
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRacingLineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// SUBSYSTEM LIFECYCLE
	// ==========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// RACING LINE LOADING
	// ==========================================

	/**
	 * Load a racing line for a track
	 * @param TrackID The track to load the line for
	 * @param LineType Which type of line to load (default: Optimal)
	 * @return True if line was loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Load")
	bool LoadRacingLine(FName TrackID, EMGRacingLineType LineType = EMGRacingLineType::Optimal);

	/**
	 * Load a custom racing line
	 * @param TrackID Track identifier
	 * @param CustomLine The custom line data to load
	 * @return True if loaded successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Load")
	bool LoadCustomLine(FName TrackID, const FMGRacingLine& CustomLine);

	/** Unload the current racing line */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Load")
	void UnloadRacingLine();

	/** Check if a racing line is currently loaded */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Load")
	bool IsLineLoaded() const { return bLineLoaded; }

	/** Get the currently loaded racing line */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Load")
	FMGRacingLine GetCurrentLine() const { return CurrentLine; }

	/**
	 * Get available line types for a track
	 * @param TrackID Track to query
	 * @return Array of available line types
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Load")
	TArray<EMGRacingLineType> GetAvailableLineTypes(FName TrackID) const;

	// ==========================================
	// LINE QUERY - POSITION & SPEED DATA
	// ==========================================

	/**
	 * Get racing line point at a specific track distance
	 * @param Distance Distance along track in cm
	 * @return Interpolated point data
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FMGRacingLinePoint GetPointAtDistance(float Distance) const;

	/**
	 * Get the nearest racing line point to a world position
	 * @param WorldPosition Position to find nearest point to
	 * @return Nearest point data
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FMGRacingLinePoint GetNearestPoint(const FVector& WorldPosition) const;

	/**
	 * Get distance along the line for a world position
	 * @param WorldPosition Position to query
	 * @return Distance in cm from start/finish
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	float GetDistanceAlongLine(const FVector& WorldPosition) const;

	/** Get world position of the racing line at a distance */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FVector GetLinePositionAtDistance(float Distance) const;

	/** Get direction vector of the racing line at a distance */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FVector GetLineDirectionAtDistance(float Distance) const;

	/** Get optimal speed at a distance along the line */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	float GetOptimalSpeedAtDistance(float Distance) const;

	/**
	 * Get all racing line points within a distance range
	 * @param StartDistance Start of range (cm)
	 * @param EndDistance End of range (cm)
	 * @return Array of points in range
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	TArray<FMGRacingLinePoint> GetPointsInRange(float StartDistance, float EndDistance) const;

	// ==========================================
	// CORNER INFORMATION
	// ==========================================

	/** Get all corners on the current track */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	TArray<FMGCornerData> GetAllCorners() const;

	/**
	 * Get data for a specific corner
	 * @param CornerNumber 1-based corner number
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	FMGCornerData GetCorner(int32 CornerNumber) const;

	/**
	 * Get the next corner ahead of current position
	 * @param CurrentDistance Current track distance
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	FMGCornerData GetNextCorner(float CurrentDistance) const;

	/**
	 * Get distance to the next corner
	 * @param CurrentDistance Current track distance
	 * @return Distance in cm to next corner entry
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	float GetDistanceToNextCorner(float CurrentDistance) const;

	/**
	 * Get the current corner phase for a position
	 * @param CurrentDistance Track distance
	 * @return Current phase (Approach, Braking, TurnIn, Apex, Exit, Acceleration)
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	EMGCornerPhase GetCornerPhase(float CurrentDistance) const;

	/**
	 * Check if currently in a corner zone
	 * @param CurrentDistance Track distance
	 * @return True if between corner entry and exit
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	bool IsInCorner(float CurrentDistance) const;

	// ==========================================
	// BRAKING ZONE INFORMATION
	// ==========================================

	/** Get all braking zones on the current track */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	TArray<FMGBrakingZone> GetAllBrakingZones() const;

	/**
	 * Get the next braking zone ahead
	 * @param CurrentDistance Current track distance
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	FMGBrakingZone GetNextBrakingZone(float CurrentDistance) const;

	/**
	 * Get distance to the next braking zone
	 * @param CurrentDistance Current track distance
	 * @return Distance in cm to braking zone start
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	float GetDistanceToNextBrakingZone(float CurrentDistance) const;

	/**
	 * Check if currently in a braking zone
	 * @param CurrentDistance Track distance
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	bool IsInBrakingZone(float CurrentDistance) const;

	/**
	 * Get braking intensity for current position
	 * @param CurrentDistance Track distance
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	EMGBrakingIndicator GetBrakingIndicator(float CurrentDistance) const;

	// ==========================================
	// VEHICLE TRACKING
	// ==========================================

	/**
	 * Register a vehicle for tracking
	 * @param VehicleID Unique identifier for the vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Track")
	void RegisterVehicle(FName VehicleID);

	/**
	 * Unregister a vehicle from tracking
	 * @param VehicleID Vehicle to stop tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Track")
	void UnregisterVehicle(FName VehicleID);

	/**
	 * Update a tracked vehicle's state (call each frame)
	 * @param VehicleID The vehicle to update
	 * @param Position Current world position
	 * @param CurrentSpeed Current speed (m/s)
	 * @param Throttle Current throttle input (0-1)
	 * @param Brake Current brake input (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Track")
	void UpdateVehiclePosition(FName VehicleID, const FVector& Position, float CurrentSpeed, float Throttle, float Brake);

	/**
	 * Get current deviation for a tracked vehicle
	 * @param VehicleID Vehicle to query
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Track")
	FMGLineDeviation GetVehicleDeviation(FName VehicleID) const;

	/**
	 * Get accumulated performance data for a vehicle
	 * @param VehicleID Vehicle to query
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Track")
	FMGLinePerformance GetVehiclePerformance(FName VehicleID) const;

	// ==========================================
	// SPEED ADVISORY
	// ==========================================

	/** Get recommended speed for a tracked vehicle's current position */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetRecommendedSpeed(FName VehicleID) const;

	/** Get difference between current and optimal speed (+ = too fast) */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetSpeedDifference(FName VehicleID) const;

	/** Get recommended gear for current position */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	int32 GetRecommendedGear(FName VehicleID) const;

	/** Get recommended throttle input (0-1) */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetRecommendedThrottle(FName VehicleID) const;

	/** Get recommended brake input (0-1) */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetRecommendedBrake(FName VehicleID) const;

	// ==========================================
	// ASSIST SETTINGS
	// ==========================================

	/**
	 * Apply new driver assist settings
	 * @param Settings The new settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetAssistSettings(const FMGDriverAssistSettings& Settings);

	/** Get current driver assist settings */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Settings")
	FMGDriverAssistSettings GetAssistSettings() const { return AssistSettings; }

	/**
	 * Toggle racing line visibility
	 * @param bVisible Whether to show the racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetLineVisibility(bool bVisible);

	/**
	 * Set the visual display mode
	 * @param Mode How to render the racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetVisualMode(EMGLineVisualMode Mode);

	/**
	 * Set how far ahead to show the racing line
	 * @param Distance Look-ahead distance in meters
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetLookAheadDistance(float Distance);

	// ==========================================
	// LINE GENERATION & RECORDING
	// ==========================================

	/**
	 * Generate a racing line from spline points
	 * @param SplinePoints Track centerline points
	 * @param TrackID Identifier for the track
	 * @return Generated racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	FMGRacingLine GenerateLineFromSpline(const TArray<FVector>& SplinePoints, FName TrackID);

	/**
	 * Generate an optimal racing line for a vehicle class
	 * @param TrackID Track to generate for
	 * @param VehicleClass Vehicle class for performance parameters
	 * @return Generated optimal line
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	FMGRacingLine GenerateOptimalLine(FName TrackID, FName VehicleClass);

	/**
	 * Start recording the player's racing line
	 * @param VehicleID Vehicle to record
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	void RecordPlayerLine(FName VehicleID);

	/** Stop recording and finalize the line */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	void StopRecordingLine();

	/** Check if currently recording a line */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Generate")
	bool IsRecording() const { return bRecording; }

	/** Get the currently recorded line (if recording) */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	FMGRacingLine GetRecordedLine() const;

	/**
	 * Save a racing line to storage
	 * @param Line The line data to save
	 * @param TrackID Track identifier
	 * @param LineName Custom name for the line
	 */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	void SaveLine(const FMGRacingLine& Line, FName TrackID, FName LineName);

	// ==========================================
	// VISUALIZATION DATA FOR HUD
	// ==========================================

	/**
	 * Get visible line points for rendering
	 * @param ViewerPosition Current camera/vehicle position
	 * @param LookAhead How far ahead to get points (meters)
	 * @return World positions for line rendering
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	TArray<FVector> GetVisibleLinePoints(const FVector& ViewerPosition, float LookAhead) const;

	/**
	 * Get color for racing line at a distance (based on speed/braking)
	 * @param Distance Track distance
	 * @return Color for that section of line
	 */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	FLinearColor GetLineColorAtDistance(float Distance) const;

	/** Get world positions of all braking markers */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	TArray<FVector> GetBrakingMarkerPositions() const;

	/** Get world positions of all apex markers */
	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	TArray<FVector> GetApexMarkerPositions() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when a racing line is loaded */
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnRacingLineLoaded OnRacingLineLoaded;

	/** Fired when vehicle deviation is updated */
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnLineDeviationUpdated OnLineDeviationUpdated;

	/** Fired when a corner is approaching */
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnCornerApproaching OnCornerApproaching;

	/** Fired when entering a braking zone */
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnBrakingZoneEntered OnBrakingZoneEntered;

	/** Fired when speed significantly differs from optimal */
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnSpeedWarning OnSpeedWarning;

	/** Fired when assist settings change */
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnAssistSettingsChanged OnAssistSettingsChanged;

protected:
	// ==========================================
	// INTERNAL UPDATE METHODS
	// ==========================================

	/** Main tick update for line tracking */
	void OnLineTick();

	/** Update deviation calculations for all tracked vehicles */
	void UpdateVehicleDeviations();

	/** Check and broadcast corner approach events */
	void CheckCornerApproach(FName VehicleID, float Distance);

	/** Check and broadcast braking zone events */
	void CheckBrakingZone(FName VehicleID, float Distance);

	/** Calculate braking zones from racing line data */
	void CalculateBrakingZones();

	/** Identify corners from racing line data */
	void IdentifyCorners();

	/** Interpolate a point between two racing line points */
	FMGRacingLinePoint InterpolatePoint(float Distance) const;

	/** Find array index for nearest point to a distance */
	int32 FindNearestPointIndex(float Distance) const;

	/** Save line data to disk */
	void SaveLineData();

	/** Load line data from disk */
	void LoadLineData();

	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Currently loaded racing line */
	UPROPERTY()
	FMGRacingLine CurrentLine;

	/** Whether a line is currently loaded */
	UPROPERTY()
	bool bLineLoaded = false;

	/** All stored racing lines indexed by track ID */
	UPROPERTY()
	TMap<FName, FMGRacingLineArray> TrackLines;

	/** Calculated braking zones for current line */
	UPROPERTY()
	TArray<FMGBrakingZone> BrakingZones;

	/** Current driver assist settings */
	UPROPERTY()
	FMGDriverAssistSettings AssistSettings;

	// --- Vehicle Tracking Data ---

	/** Current positions of tracked vehicles */
	UPROPERTY()
	TMap<FName, FVector> VehiclePositions;

	/** Current speeds of tracked vehicles */
	UPROPERTY()
	TMap<FName, float> VehicleSpeeds;

	/** Track distances for tracked vehicles */
	UPROPERTY()
	TMap<FName, float> VehicleDistances;

	/** Current deviation data per vehicle */
	UPROPERTY()
	TMap<FName, FMGLineDeviation> VehicleDeviations;

	/** Accumulated performance data per vehicle */
	UPROPERTY()
	TMap<FName, FMGLinePerformance> VehiclePerformances;

	// --- Recording State ---

	/** Whether currently recording a player line */
	UPROPERTY()
	bool bRecording = false;

	/** Vehicle being recorded */
	UPROPERTY()
	FName RecordingVehicle;

	/** Points recorded so far */
	UPROPERTY()
	TArray<FMGRacingLinePoint> RecordedPoints;

	// --- Event Cooldown Tracking ---

	/** Last corner warned about per vehicle (prevents spam) */
	UPROPERTY()
	TMap<FName, int32> VehicleLastCornerWarning;

	/** Last braking zone warned about per vehicle */
	UPROPERTY()
	TMap<FName, int32> VehicleLastBrakingWarning;

	/** Timer handle for periodic updates */
	FTimerHandle LineTickHandle;
};
