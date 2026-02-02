// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MG_AI_RaceManager.h
 * @brief AI Race Manager - Orchestrates AI opponents during races
 *
 * This subsystem is the central coordinator for all AI-controlled racers in
 * Midnight Grind. It handles the complete lifecycle of AI opponents from
 * spawning to race completion, including:
 *
 * - Spawning and despawning AI vehicles with configurable driver profiles
 * - Tracking race positions for all participants (AI and player)
 * - Managing rubber-banding (catch-up) mechanics for balanced gameplay
 * - Broadcasting events for race milestones (lap completion, finish, etc.)
 *
 * ARCHITECTURE OVERVIEW:
 * ----------------------
 * The manager is designed as an ActorComponent that attaches to your GameMode.
 * It maintains an array of FMGActiveAIOpponent structs, each representing one
 * AI racer with their vehicle, controller, and race progress data.
 *
 * TYPICAL USAGE FLOW:
 * 1. Attach UMGAIRaceManager to your racing GameMode
 * 2. Call SetTrackSpline() with the track's centerline spline
 * 3. Call SetRaceParameters() with track length and lap count
 * 4. Spawn AI opponents via SpawnAIOpponents() or GenerateAIField()
 * 5. Call InitializeForRace() to prepare AI for race start
 * 6. Call StartRacing() when the race begins
 * 7. Position updates happen automatically via TickComponent
 *
 * POSITION TRACKING:
 * The manager calculates positions using "total race distance" which combines:
 * - Current lap number * track length
 * - Current distance along track this lap
 * This allows accurate position sorting even when racers are on different laps.
 *
 * @see AMGRacingAIController - The AI controller that drives each opponent
 * @see FMGAIDriverConfig - Defines AI behavior characteristics
 * @see UMGDynamicDifficultySubsystem - Integrates with difficulty scaling
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/MGRacingAIController.h"
#include "MG_AI_RaceManager.generated.h"

class AMGVehiclePawn;
class AMGTrackSpline;
class AMGRacingAIController;
class UMGVehicleModelData;

// ============================================================================
// AI OPPONENT CONFIGURATION
// ============================================================================

/**
 * Configuration for spawning an AI opponent
 *
 * This struct defines everything needed to spawn a single AI racer:
 * the vehicle they drive, their driving personality, and where they
 * start on the grid. Used both for pre-designed opponents and
 * dynamically generated AI fields.
 *
 * EXAMPLE USAGE (Blueprint):
 * @code
 * FMGAIOpponentConfig Config;
 * Config.VehicleModel = MyCarData;
 * Config.DriverProfile.Aggression = 0.7f;  // Aggressive driver
 * Config.GridPosition = 2;  // Starting 3rd (0-indexed)
 * RaceManager->SpawnAIOpponent(Config, StartTransform);
 * @endcode
 */
USTRUCT(BlueprintType)
struct FMGAIOpponentConfig
{
	GENERATED_BODY()

	/**
	 * Vehicle model to use
	 * Soft reference allows async loading of vehicle assets
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	TSoftObjectPtr<UMGVehicleModelData> VehicleModel;

	/**
	 * Driver profile defining AI behavior
	 * Controls aggression, skill, consistency, and driving style
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	FMGAIDriverConfig DriverProfile;

	/**
	 * Starting grid position (0 = pole position)
	 * Higher numbers start further back on the grid
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	int32 GridPosition = 0;

	/**
	 * Custom vehicle pawn class override (optional)
	 * Leave null to use the default vehicle pawn class
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Opponent")
	TSubclassOf<AMGVehiclePawn> VehiclePawnClass;
};

// ============================================================================
// ACTIVE AI OPPONENT DATA
// ============================================================================

/**
 * Runtime data for an active AI opponent in the race
 *
 * This struct tracks all the live state for a spawned AI racer.
 * It's updated every tick with current position, lap count, and
 * race progress. The manager uses this data to calculate standings.
 *
 * NOTE: This is runtime data only - it's not saved between races.
 * For persistent opponent data, see the career mode systems.
 */
USTRUCT(BlueprintType)
struct FMGActiveAIOpponent
{
	GENERATED_BODY()

	/**
	 * Unique ID assigned by the manager
	 * Use this to reference specific opponents in API calls
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 OpponentId = -1;

	/** The spawned vehicle pawn actor */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMGVehiclePawn> VehiclePawn;

	/** The AI controller driving this vehicle */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMGRacingAIController> AIController;

	/** Driver profile being used (copy of spawn config) */
	UPROPERTY(BlueprintReadOnly)
	FMGAIDriverConfig DriverProfile;

	/**
	 * Current race position (1 = first place)
	 * Updated by the manager's position calculation logic
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPosition = 0;

	/** Current lap number (1 = first lap) */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 0;

	/**
	 * Distance along track this lap (in cm)
	 * Ranges from 0 to TrackLength, then resets on new lap
	 */
	UPROPERTY(BlueprintReadOnly)
	float TrackDistance = 0.0f;

	/**
	 * Total race distance for position calculations
	 * Formula: (CurrentLap * TrackLength) + TrackDistance
	 * This allows comparing positions across different laps
	 */
	UPROPERTY(BlueprintReadOnly)
	float TotalRaceDistance = 0.0f;

	/** Whether this opponent has crossed the finish line */
	UPROPERTY(BlueprintReadOnly)
	bool bFinished = false;

	/** Time when opponent finished (0 if not finished) */
	UPROPERTY(BlueprintReadOnly)
	float FinishTime = 0.0f;

	/**
	 * Validates that the opponent has valid references
	 * @return true if both VehiclePawn and AIController are valid
	 */
	bool IsValid() const { return VehiclePawn != nullptr && AIController != nullptr; }
};

// ============================================================================
// MANAGER CONFIGURATION
// ============================================================================

/**
 * Configuration for the AI race manager
 *
 * Global settings that affect all AI opponents in a race.
 * Set these before spawning opponents for consistent behavior.
 */
USTRUCT(BlueprintType)
struct FMGAIRaceManagerConfig
{
	GENERATED_BODY()

	/**
	 * Global rubber-banding settings
	 * Rubber-banding adjusts AI speed based on position relative to player
	 * to keep races competitive and exciting
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Manager")
	FMGRubberBandingConfig RubberBandingConfig;

	/**
	 * Default AI controller class for spawned opponents
	 * Can be overridden per-opponent if needed
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Manager")
	TSubclassOf<AMGRacingAIController> AIControllerClass;

	/**
	 * How often to recalculate positions (in Hz)
	 * Higher values = more responsive but more CPU usage
	 * Recommended: 10 Hz for arcade racing
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race Manager")
	float PositionUpdateRate = 10.0f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Fired when an AI opponent crosses the finish line
/// @param OpponentId The unique ID of the finishing opponent
/// @param FinishTime The race time when they finished
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIOpponentFinished, int32, OpponentId, float, FinishTime);

/// Fired when an AI opponent's race position changes
/// @param OpponentId The unique ID of the affected opponent
/// @param OldPosition Their previous position
/// @param NewPosition Their new position
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIPositionChanged, int32, OpponentId, int32, OldPosition, int32, NewPosition);

// ============================================================================
// MAIN RACE MANAGER CLASS
// ============================================================================

/**
 * Manages AI opponents in a race
 *
 * Attach this component to your GameMode to spawn and control AI racers.
 * Handles spawning, position tracking, race events, and cleanup.
 *
 * @see FMGAIOpponentConfig for spawn configuration
 * @see FMGActiveAIOpponent for runtime opponent data
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGAIRaceManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGAIRaceManager();

	virtual void TickComponent(float MGDeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/**
	 * Set the track spline for AI navigation
	 * Must be called before spawning AI opponents
	 * @param InTrackSpline The track's centerline spline actor
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetTrackSpline(AMGTrackSpline* InTrackSpline);

	/**
	 * Set the race configuration
	 * @param InConfig Configuration including rubber-banding and update rate
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetConfiguration(const FMGAIRaceManagerConfig& InConfig);

	/**
	 * Set the track length and lap count for position tracking
	 * Required for accurate position calculations
	 * @param InTrackLength Total track length in centimeters
	 * @param InTotalLaps Number of laps in the race
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetRaceParameters(float InTrackLength, int32 InTotalLaps);

	// ==========================================
	// AI SPAWNING
	// ==========================================

	/**
	 * Spawn a single AI opponent
	 * @param Config The opponent's configuration (vehicle, profile, etc.)
	 * @param SpawnTransform Where to spawn the vehicle
	 * @return Unique opponent ID, or -1 if spawn failed
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	int32 SpawnAIOpponent(const FMGAIOpponentConfig& Config, const FTransform& SpawnTransform);

	/**
	 * Spawn multiple AI opponents from configuration array
	 * Configs and SpawnTransforms arrays must have matching lengths
	 * @param Configs Array of opponent configurations
	 * @param SpawnTransforms Array of spawn locations (one per config)
	 * @return Array of assigned opponent IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<int32> SpawnAIOpponents(const TArray<FMGAIOpponentConfig>& Configs, const TArray<FTransform>& SpawnTransforms);

	/**
	 * Auto-generate AI opponents for a difficulty level
	 * Creates a field of AI racers with varied skill levels based on the
	 * base difficulty. Useful for quick race setup.
	 * @param OpponentCount Number of AI opponents to generate
	 * @param BaseDifficulty The target difficulty level
	 * @param SpawnTransforms Grid positions for each opponent
	 * @return Array of assigned opponent IDs
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<int32> GenerateAIField(int32 OpponentCount, EMGAIDifficulty BaseDifficulty, const TArray<FTransform>& SpawnTransforms);

	/**
	 * Remove an AI opponent from the race
	 * Despawns the vehicle and removes from tracking
	 * @param OpponentId The ID of the opponent to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void RemoveAIOpponent(int32 OpponentId);

	/**
	 * Remove all AI opponents
	 * Call this during race cleanup or when resetting
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void RemoveAllAIOpponents();

	// ==========================================
	// RACE CONTROL
	// ==========================================

	/**
	 * Initialize all AI for race start
	 * Puts AI in ready state, waiting for StartRacing() call
	 * Call this after spawning all opponents
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void InitializeForRace();

	/**
	 * Start all AI racing
	 * Call this when the race countdown reaches zero
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void StartRacing();

	/**
	 * Stop all AI racing
	 * AI will stop driving but vehicles remain spawned
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void StopRacing();

	/**
	 * Pause or resume all AI
	 * Use for pause menus or cutscenes
	 * @param bPaused True to pause, false to resume
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetAllPaused(bool bPaused);

	/**
	 * Notify manager that an AI completed a lap
	 * Usually called by checkpoint system
	 * @param OpponentId The opponent who completed the lap
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void OnAILapCompleted(int32 OpponentId);

	/**
	 * Notify manager that an AI finished the race
	 * @param OpponentId The opponent who finished
	 * @param FinishTime Their total race time
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void OnAIFinished(int32 OpponentId, float FinishTime);

	// ==========================================
	// QUERIES
	// ==========================================

	/**
	 * Get all active AI opponents
	 * @return Array of all opponent data structs
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<FMGActiveAIOpponent> GetAllOpponents() const;

	/**
	 * Get a specific opponent by ID
	 * @param OpponentId The opponent to look up
	 * @param OutOpponent [out] The opponent data if found
	 * @return True if opponent was found
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	bool GetOpponent(int32 OpponentId, FMGActiveAIOpponent& OutOpponent) const;

	/**
	 * Get the number of active opponents
	 * @return Count of spawned AI opponents
	 */
	UFUNCTION(BlueprintPure, Category = "AI Race Manager")
	int32 GetOpponentCount() const { return ActiveOpponents.Num(); }

	/**
	 * Get opponents sorted by race position
	 * First element is the leader, last is in last place
	 * @return Sorted array of opponents
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	TArray<FMGActiveAIOpponent> GetOpponentsByPosition() const;

	/**
	 * Include player vehicle in position calculations
	 * Call this each frame to keep player position updated
	 * @param PlayerVehicle The player's vehicle pawn
	 * @param PlayerLap Player's current lap
	 * @param PlayerTrackDistance Player's distance along track
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Race Manager")
	void SetPlayerVehicle(AMGVehiclePawn* PlayerVehicle, int32 PlayerLap, float PlayerTrackDistance);

	// ==========================================
	// EVENTS
	// ==========================================

	/**
	 * Called when an AI finishes the race
	 * Bind to this to show finish notifications
	 */
	UPROPERTY(BlueprintAssignable, Category = "AI Race Manager|Events")
	FOnAIOpponentFinished OnAIOpponentFinished;

	/**
	 * Called when an AI's position changes
	 * Useful for position change UI feedback
	 */
	UPROPERTY(BlueprintAssignable, Category = "AI Race Manager|Events")
	FOnAIPositionChanged OnAIPositionChanged;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Recalculate and update positions for all racers */
	void UpdatePositions();

	/** Update a single opponent's track distance based on their location */
	void UpdateOpponentTrackDistance(FMGActiveAIOpponent& Opponent);

	/**
	 * Calculate total race distance for position sorting
	 * @param Lap Current lap number
	 * @param TrackDistance Distance along track this lap
	 * @return Combined distance value for comparison
	 */
	float CalculateTotalRaceDistance(int32 Lap, float TrackDistance) const;

	/** Get the next available unique opponent ID */
	int32 GetNextOpponentId();

	// ==========================================
	// DATA
	// ==========================================

	/** Track spline used for AI navigation and distance calculations */
	UPROPERTY()
	TObjectPtr<AMGTrackSpline> TrackSpline;

	/** Manager configuration (rubber-banding, update rate, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
	FMGAIRaceManagerConfig Configuration;

	/** Array of all active AI opponents */
	UPROPERTY()
	TArray<FMGActiveAIOpponent> ActiveOpponents;

	/** Total track length in centimeters */
	UPROPERTY()
	float TrackLength = 0.0f;

	/** Number of laps in the race */
	UPROPERTY()
	int32 TotalLaps = 3;

	/** Whether the race is currently active */
	UPROPERTY()
	bool bRaceActive = false;

	/** Timer for rate-limiting position updates */
	float PositionUpdateTimer = 0.0f;

	/** Counter for generating unique opponent IDs */
	int32 NextOpponentId = 1;

	/** Reference to player vehicle for position tracking */
	UPROPERTY()
	TObjectPtr<AMGVehiclePawn> PlayerVehicle;

	/** Player's current lap number */
	int32 PlayerLap = 0;

	/** Player's current distance along track */
	float PlayerTrackDistance = 0.0f;
};
