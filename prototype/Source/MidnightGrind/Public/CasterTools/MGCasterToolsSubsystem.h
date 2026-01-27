// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCasterToolsSubsystem.h
 * @brief Professional broadcast production tools for commentators and casters.
 *
 * This subsystem provides the complete toolkit needed for professional race commentary
 * and broadcast production. It handles camera control, overlay management, battle detection,
 * highlight tracking, and visual effects for creating engaging esports broadcasts.
 *
 * Key Features:
 * - Multiple camera modes (follow, helicopter, orbit, onboard, etc.)
 * - Real-time racer statistics and overlay data
 * - Automatic battle zone detection for exciting moments
 * - Highlight moment tracking with significance scoring
 * - Instant replay with auto-detection of key moments
 * - Telestrator drawing tools for analysis segments
 * - Hotkey system for quick production control
 *
 * Usage Example:
 * @code
 *     // Set up a broadcast camera
 *     UMGCasterToolsSubsystem* CasterTools = GetWorld()->GetSubsystem<UMGCasterToolsSubsystem>();
 *     CasterTools->SetCameraMode(EMGCasterCameraMode::BattleCam);
 *     CasterTools->SetOverlayPreset(EMGOverlayPreset::Broadcast);
 *     CasterTools->FocusOnPlayer(LeaderPlayerID);
 * @endcode
 *
 * @note This is a WorldSubsystem - it exists per-world and is recreated on level transitions.
 * @see UMGEsportsSubsystem for tournament and match management
 * @see UMGBroadcastSubsystem for output and streaming controls
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGCasterToolsSubsystem.generated.h"

//=============================================================================
// Enums - Camera and Display Configuration
//=============================================================================

/**
 * @brief Available camera modes for broadcast production.
 *
 * Each mode provides a different perspective suited for various race situations.
 * Casters typically switch between modes to keep broadcasts visually engaging.
 */
UENUM(BlueprintType)
enum class EMGCasterCameraMode : uint8
{
	FollowLeader,    ///< Automatically follows the race leader
	FollowPlayer,    ///< Follows a specific targeted player
	TrackOverview,   ///< Wide shot showing track layout and multiple racers
	HelicopterCam,   ///< Aerial view following the action from above
	OrbitCam,        ///< Circles around a target point or player
	FreeCam,         ///< Manual camera control for custom shots
	BattleCam,       ///< Auto-focuses on close racing battles
	OnboardCam,      ///< Driver's POV from inside the vehicle
	ReplayCam,       ///< Special camera used during replay playback
	PitLaneCam,      ///< Fixed camera covering pit lane activity
	StartFinishCam   ///< Fixed camera at the start/finish line
};

/**
 * @brief Preset configurations for broadcast overlay complexity.
 *
 * Presets provide quick access to common overlay configurations.
 * Use Custom to manually configure individual overlay elements.
 */
UENUM(BlueprintType)
enum class EMGOverlayPreset : uint8
{
	None,       ///< No overlay - clean video feed only
	Minimal,    ///< Position and lap count only
	Standard,   ///< Leaderboard, timing, basic stats
	Detailed,   ///< Full stats with sector times and gaps
	Broadcast,  ///< TV broadcast style with graphics
	Analysis,   ///< Post-race analysis with telemetry
	Custom      ///< Manually configured overlay elements
};

/**
 * @brief Types of highlight moments detected during a race.
 *
 * The system automatically detects and categorizes exciting moments.
 * Each type has a base significance value that affects auto-replay priority.
 */
UENUM(BlueprintType)
enum class EMGHighlightType : uint8
{
	Overtake,       ///< Position change between racers
	NearMiss,       ///< Close call without contact
	Crash,          ///< Vehicle collision or crash
	DriftCombo,     ///< Extended drift sequence
	NitroBoost,     ///< Nitro activation at critical moment
	FastestLap,     ///< New fastest lap time set
	LeadChange,     ///< First place position change
	PhotoFinish,    ///< Extremely close finish (under 0.1s)
	PerfectCorner,  ///< Optimal racing line through a corner
	BigJump,        ///< Large air time on jumps
	PoliceEscape,   ///< Evading police pursuit (game mode specific)
	Spinout         ///< Vehicle spin/loss of control
};

//=============================================================================
// Structs - Camera Configuration
//=============================================================================

/**
 * @brief Complete camera configuration for broadcast shots.
 *
 * Contains all parameters needed to position and behavior-configure
 * the broadcast camera, including follow settings and visual options.
 */
USTRUCT(BlueprintType)
struct FMGCasterCameraConfig
{
	GENERATED_BODY()

	/// Current camera behavior mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCasterCameraMode Mode = EMGCasterCameraMode::FollowLeader;

	/// Player ID to track (for FollowPlayer and OnboardCam modes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	/// Distance behind the target vehicle (centimeters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FollowDistance = 500.0f;

	/// Height above the target vehicle (centimeters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FollowHeight = 200.0f;

	/// Camera field of view in degrees (wider = more visible, more distortion)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;

	/// How quickly the camera transitions to new positions (higher = snappier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothingSpeed = 5.0f;

	/// Enable automatic camera switching based on race action
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoSwitch = true;

	/// Seconds between automatic camera switches
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoSwitchInterval = 8.0f;

	/// Give priority to switching to battle zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrioritizeBattles = true;

	/// Show motion blur lines at high speeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedLines = true;

	/// Camera shake intensity (0 = none, 1 = full)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShakeIntensity = 0.5f;
};

//=============================================================================
// Structs - Racer Data and Statistics
//=============================================================================

/**
 * @brief Real-time overlay data for a single racer.
 *
 * Contains all information needed to display a racer's current status
 * on the broadcast overlay, updated in real-time during the race.
 */
USTRUCT(BlueprintType)
struct FMGRacerOverlayData
{
	GENERATED_BODY()

	/// Unique player identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Name displayed on broadcast graphics
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/// Current race position (1 = leader)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	/// Positions gained/lost since race start (positive = gained)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PositionChange = 0;

	/// Time gap to the race leader (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToLeader = 0.0f;

	/// Time gap to the racer directly ahead (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToAhead = 0.0f;

	/// Current vehicle speed (km/h or mph based on settings)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	/// Maximum speed reached this race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeed = 0.0f;

	/// Current lap number
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	/// Time elapsed on current lap (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentLapTime = 0.0f;

	/// Personal best lap time this race (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	/// Previous lap completion time (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastLapTime = 0.0f;

	/// Nitro/boost remaining (0.0 = empty, 1.0 = full)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroRemaining = 0.0f;

	/// ID of the vehicle being driven
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Team color for graphics (if in team mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TeamColor = FLinearColor::White;

	/// Driver photo/avatar for on-screen graphics
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> DriverPhoto;

	/// Number of successful overtakes this race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OvertakesMade = 0;

	/// Number of times overtaken by others this race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OvertakesLost = 0;

	/// Currently in the pit lane
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInPit = false;

	/// Has retired/DNF from the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRetired = false;
};

/**
 * @brief Defines a zone where close racing is occurring.
 *
 * Battle zones are automatically detected areas where multiple racers
 * are competing closely. Used for camera focus and highlight detection.
 */
USTRUCT(BlueprintType)
struct FMGBattleZone
{
	GENERATED_BODY()

	/// Player IDs of racers involved in this battle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedPlayerIDs;

	/// World location at the center of the battle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CenterLocation = FVector::ZeroVector;

	/// Battle intensity score (0-1, based on proximity and action)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 0.0f;

	/// How long this battle has been ongoing (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	/// Whether this battle is for an actual race position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForPosition = false;

	/// The race position being contested (if bForPosition is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PositionFightingFor = 0;
};

//=============================================================================
// Structs - Highlights and Replay
//=============================================================================

/**
 * @brief Data for a detected highlight moment during the race.
 *
 * Highlights are significant moments automatically detected by the system
 * or manually marked by casters. Each has a significance score for prioritization.
 */
USTRUCT(BlueprintType)
struct FMGHighlightMoment
{
	GENERATED_BODY()

	/// Type of highlight event
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHighlightType Type = EMGHighlightType::Overtake;

	/// Primary player involved (e.g., the overtaker)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	/// Secondary player involved (e.g., the overtaken player)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	/// World location where the highlight occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/// Race time when the highlight occurred (seconds from race start)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceTime = 0.0f;

	/// Importance score (0-1, higher = more significant)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Significance = 0.0f;

	/// Associated replay clip ID (if replay was captured)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayID;

	/// Start time within the replay clip (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReplayStartTime = 0.0f;

	/// Recommended replay duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReplayDuration = 5.0f;

	/// Whether auto-replay was triggered for this highlight
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoReplayTriggered = false;
};

/**
 * @brief Information about a track sector for timing displays.
 *
 * Tracks are divided into sectors (typically 3) for detailed timing analysis.
 * Each sector tracks the current fastest time holder.
 */
USTRUCT(BlueprintType)
struct FMGTrackSector
{
	GENERATED_BODY()

	/// Sector number (0, 1, 2 for a 3-sector track)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SectorIndex = 0;

	/// Display name for the sector (e.g., "Sector 1", "Tunnel Section")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SectorName;

	/// Track distance where this sector begins
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistance = 0.0f;

	/// Track distance where this sector ends
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndDistance = 0.0f;

	/// Player ID holding the fastest sector time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FastestPlayerID;

	/// Fastest time through this sector (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestTime = 0.0f;
};

/**
 * @brief Aggregate race statistics for the broadcast overlay.
 *
 * Summary statistics shown on broadcast graphics, providing viewers
 * with an overview of race progress and key metrics.
 */
USTRUCT(BlueprintType)
struct FMGRaceStatsSummary
{
	GENERATED_BODY()

	/// Total overtakes that have occurred in the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalOvertakes = 0;

	/// Number of times the lead has changed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LeadChanges = 0;

	/// Player ID of the current race leader
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentLeaderID;

	/// Time gap between P1 and P2 (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeaderGapToSecond = 0.0f;

	/// Player ID holding the overall fastest lap
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FastestLapHolderID;

	/// Fastest lap time set in the race (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestLapTime = 0.0f;

	/// Lap number when the fastest lap was set
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FastestLapLapNumber = 0;

	/// Total time elapsed since race start (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceElapsedTime = 0.0f;

	/// Current lap for the leader
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	/// Total laps in the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLaps = 0;

	/// Average speed across all racers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	/// Highest speed recorded in the race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedReached = 0.0f;

	/// Player who achieved the top speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TopSpeedPlayerID;
};

/**
 * @brief Hotkey binding for caster controls.
 *
 * Maps keyboard/controller inputs to caster actions for quick access
 * during live broadcasts.
 */
USTRUCT(BlueprintType)
struct FMGCasterHotkey
{
	GENERATED_BODY()

	/// Input key bound to this action
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey Key;

	/// Internal action name (e.g., "FocusLeader", "TriggerReplay")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionName;

	/// Human-readable description for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};

//=============================================================================
// Delegates - Event Notifications
//=============================================================================

/// Broadcast when the camera mode changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeChanged, EMGCasterCameraMode, NewMode);

/// Broadcast when camera focus switches to a different player
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusedPlayerChanged, const FString&, PlayerID);

/// Broadcast when a highlight moment is detected
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighlightDetected, const FMGHighlightMoment&, Highlight);

/// Broadcast when a new battle zone is detected
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleZoneDetected, const FMGBattleZone&, Battle);

/// Broadcast when the race leader changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeadChanged, const FString&, NewLeaderID, const FString&, OldLeaderID);

/// Broadcast when a new fastest lap is set
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFastestLapSet, const FMGRacerOverlayData&, RacerData);

/// Broadcast when instant replay playback begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInstantReplayStarted);

/// Broadcast when instant replay playback ends
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInstantReplayEnded);

/// Broadcast when the overlay preset changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverlayPresetChanged, EMGOverlayPreset, NewPreset);

//=============================================================================
// UMGCasterToolsSubsystem - Main Subsystem Class
//=============================================================================

/**
 * @brief Professional broadcast production toolkit for race commentary.
 *
 * This WorldSubsystem provides comprehensive tools for casters and broadcasters
 * to produce engaging race coverage. Features include:
 *
 * - Multi-mode camera system with smooth transitions
 * - Real-time racer statistics and timing data
 * - Automatic battle zone detection
 * - Highlight moment tracking with auto-replay
 * - Telestrator drawing tools for analysis
 * - Configurable hotkey system
 *
 * The subsystem automatically tracks race events and provides data suitable
 * for professional broadcast graphics and overlays.
 *
 * @note Access via: GetWorld()->GetSubsystem<UMGCasterToolsSubsystem>()
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCasterToolsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Lifecycle
	//-------------------------------------------------------------------------

	/** Initialize the caster tools subsystem. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Clean up when the subsystem is destroyed. */
	virtual void Deinitialize() override;

	/** Determine if this subsystem should be created for the given world. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//-------------------------------------------------------------------------
	// Camera Control
	//-------------------------------------------------------------------------

	/**
	 * @brief Set the broadcast camera mode.
	 * @param Mode The camera behavior mode to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void SetCameraMode(EMGCasterCameraMode Mode);

	/**
	 * @brief Apply a complete camera configuration.
	 * @param Config Full camera settings including mode, position, and effects.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void SetCameraConfig(const FMGCasterCameraConfig& Config);

	/** @return The current camera mode. */
	UFUNCTION(BlueprintPure, Category = "Caster|Camera")
	EMGCasterCameraMode GetCurrentCameraMode() const { return CurrentCameraConfig.Mode; }

	/** @return The complete current camera configuration. */
	UFUNCTION(BlueprintPure, Category = "Caster|Camera")
	FMGCasterCameraConfig GetCameraConfig() const { return CurrentCameraConfig; }

	/**
	 * @brief Focus the camera on a specific player.
	 * @param PlayerID The player to track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void FocusOnPlayer(const FString& PlayerID);

	/**
	 * @brief Focus the camera on a battle zone.
	 * @param Battle The battle zone to center the camera on.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void FocusOnBattle(const FMGBattleZone& Battle);

	/** @brief Cycle camera focus to the next player in position order. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void CycleToNextPlayer();

	/** @brief Cycle camera focus to the previous player in position order. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void CycleToPreviousPlayer();

	/** @brief Toggle automatic camera switching on/off. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void ToggleAutoCameraSwitch();

	/** @return The player ID currently being focused by the camera. */
	UFUNCTION(BlueprintPure, Category = "Caster|Camera")
	FString GetFocusedPlayerID() const { return FocusedPlayerID; }

	//-------------------------------------------------------------------------
	// Overlay Management
	//-------------------------------------------------------------------------

	/**
	 * @brief Apply an overlay preset configuration.
	 * @param Preset The overlay preset to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetOverlayPreset(EMGOverlayPreset Preset);

	/** @return The currently active overlay preset. */
	UFUNCTION(BlueprintPure, Category = "Caster|Overlay")
	EMGOverlayPreset GetCurrentOverlayPreset() const { return CurrentOverlayPreset; }

	/**
	 * @brief Toggle the leaderboard display.
	 * @param bShow True to show, false to hide.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowLeaderboard(bool bShow);

	/**
	 * @brief Toggle the timing tower display.
	 * @param bShow True to show, false to hide.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowTimingTower(bool bShow);

	/**
	 * @brief Toggle the minimap display.
	 * @param bShow True to show, false to hide.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowMinimap(bool bShow);

	/**
	 * @brief Toggle driver information cards.
	 * @param bShow True to show, false to hide.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowDriverCards(bool bShow);

	/**
	 * @brief Toggle battle zone indicators on the overlay.
	 * @param bShow True to show, false to hide.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowBattleIndicators(bool bShow);

	/**
	 * @brief Temporarily highlight a player on the overlay.
	 * @param PlayerID Player to highlight.
	 * @param Duration How long to show the highlight (seconds).
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void HighlightPlayer(const FString& PlayerID, float Duration = 3.0f);

	/**
	 * @brief Show a side-by-side comparison overlay for two players.
	 * @param PlayerA First player ID.
	 * @param PlayerB Second player ID.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void ShowComparisonOverlay(const FString& PlayerA, const FString& PlayerB);

	/** @brief Hide the comparison overlay. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void HideComparisonOverlay();

	//-------------------------------------------------------------------------
	// Racer Data Access
	//-------------------------------------------------------------------------

	/** @return Array of overlay data for all racers. */
	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	TArray<FMGRacerOverlayData> GetAllRacerData() const { return RacerData; }

	/**
	 * @brief Get overlay data for a specific racer.
	 * @param PlayerID The player to get data for.
	 * @return Racer overlay data (empty struct if not found).
	 */
	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	FMGRacerOverlayData GetRacerData(const FString& PlayerID) const;

	/** @return Aggregate race statistics summary. */
	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	FMGRaceStatsSummary GetRaceStats() const { return RaceStats; }

	/** @return Array of track sector timing data. */
	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	TArray<FMGTrackSector> GetSectorData() const { return SectorData; }

	//-------------------------------------------------------------------------
	// Battle Detection
	//-------------------------------------------------------------------------

	/** @return Array of currently active battle zones. */
	UFUNCTION(BlueprintPure, Category = "Caster|Battle")
	TArray<FMGBattleZone> GetActiveBattles() const { return ActiveBattles; }

	/** @return The most intense (highest scoring) current battle. */
	UFUNCTION(BlueprintPure, Category = "Caster|Battle")
	FMGBattleZone GetMostIntenseBattle() const;

	/**
	 * @brief Set the gap threshold for battle detection.
	 * @param GapThreshold Maximum gap (seconds) to consider racers "in battle".
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Battle")
	void SetBattleDetectionThreshold(float GapThreshold);

	//-------------------------------------------------------------------------
	// Highlights and Replay
	//-------------------------------------------------------------------------

	/** @return Array of all highlight moments detected this session. */
	UFUNCTION(BlueprintPure, Category = "Caster|Highlights")
	TArray<FMGHighlightMoment> GetHighlights() const { return Highlights; }

	/**
	 * @brief Get highlights filtered by type.
	 * @param Type The highlight type to filter for.
	 * @return Array of matching highlights.
	 */
	UFUNCTION(BlueprintPure, Category = "Caster|Highlights")
	TArray<FMGHighlightMoment> GetHighlightsByType(EMGHighlightType Type) const;

	/**
	 * @brief Trigger instant replay of a specific highlight.
	 * @param Highlight The highlight moment to replay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void TriggerInstantReplay(const FMGHighlightMoment& Highlight);

	/**
	 * @brief Trigger instant replay of the last N seconds.
	 * @param Seconds How far back to replay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void TriggerInstantReplayOfLast(float Seconds);

	/** @brief Stop the currently playing instant replay. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void StopInstantReplay();

	/** @return True if an instant replay is currently playing. */
	UFUNCTION(BlueprintPure, Category = "Caster|Highlights")
	bool IsPlayingInstantReplay() const { return bPlayingInstantReplay; }

	/**
	 * @brief Enable or disable automatic replay triggering.
	 * @param bEnabled True to enable auto-replay.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void SetAutoReplayEnabled(bool bEnabled);

	/**
	 * @brief Set the minimum significance required for auto-replay.
	 * @param Significance Threshold (0-1, higher = only major moments).
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void SetAutoReplayMinSignificance(float Significance);

	/**
	 * @brief Manually bookmark the current moment for later review.
	 * @param Description Text description of why this moment was bookmarked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void BookmarkMoment(const FString& Description);

	//-------------------------------------------------------------------------
	// Graphics Effects
	//-------------------------------------------------------------------------

	/**
	 * @brief Apply slow motion effect.
	 * @param TimeScale Time multiplier (0.5 = half speed).
	 * @param Duration How long to maintain slow motion (seconds).
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void SetSlowMotion(float TimeScale, float Duration);

	/** @brief Reset time scale to normal (1.0). */
	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void ResetTimeScale();

	/**
	 * @brief Toggle dramatic visual filter for key moments.
	 * @param bEnable True to enable the filter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void ApplyDramaticFilter(bool bEnable);

	/**
	 * @brief Configure depth of field effect.
	 * @param bEnable True to enable DOF.
	 * @param FocalDistance Distance to focus point (0 = auto-focus on target).
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void SetDepthOfField(bool bEnable, float FocalDistance = 0.0f);

	//-------------------------------------------------------------------------
	// Telestrator / Drawing Tools
	//-------------------------------------------------------------------------

	/** @brief Enter drawing mode for on-screen annotations. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void StartDrawing();

	/** @brief Exit drawing mode. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void StopDrawing();

	/** @brief Clear all on-screen drawings. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void ClearDrawings();

	/**
	 * @brief Set the color for telestrator drawings.
	 * @param Color The drawing color.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void SetDrawingColor(FLinearColor Color);

	/**
	 * @brief Set the line thickness for telestrator drawings.
	 * @param Thickness Line width in pixels.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void SetDrawingThickness(float Thickness);

	/** @return True if currently in drawing mode. */
	UFUNCTION(BlueprintPure, Category = "Caster|Telestrator")
	bool IsDrawingMode() const { return bDrawingMode; }

	//-------------------------------------------------------------------------
	// Hotkeys
	//-------------------------------------------------------------------------

	/** @return Array of all configured hotkey bindings. */
	UFUNCTION(BlueprintPure, Category = "Caster|Hotkeys")
	TArray<FMGCasterHotkey> GetHotkeyBindings() const { return HotkeyBindings; }

	/**
	 * @brief Bind a key to a caster action.
	 * @param Key The input key to bind.
	 * @param ActionName The action to perform when pressed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Caster|Hotkeys")
	void SetHotkeyBinding(const FKey& Key, const FString& ActionName);

	//-------------------------------------------------------------------------
	// Recording
	//-------------------------------------------------------------------------

	/** @brief Start recording the broadcast output. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Recording")
	void StartBroadcastRecording();

	/** @brief Stop recording the broadcast output. */
	UFUNCTION(BlueprintCallable, Category = "Caster|Recording")
	void StopBroadcastRecording();

	/** @return True if currently recording the broadcast. */
	UFUNCTION(BlueprintPure, Category = "Caster|Recording")
	bool IsRecordingBroadcast() const { return bRecordingBroadcast; }

	//-------------------------------------------------------------------------
	// Events - Bindable Delegates
	//-------------------------------------------------------------------------

	/// Fired when the camera mode changes
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnCameraModeChanged OnCameraModeChanged;

	/// Fired when camera focus switches to a different player
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnFocusedPlayerChanged OnFocusedPlayerChanged;

	/// Fired when a highlight moment is detected
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnHighlightDetected OnHighlightDetected;

	/// Fired when a battle zone is detected
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnBattleZoneDetected OnBattleZoneDetected;

	/// Fired when the race leader changes
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnLeadChanged OnLeadChanged;

	/// Fired when a new fastest lap is achieved
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnFastestLapSet OnFastestLapSet;

	/// Fired when instant replay playback begins
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnInstantReplayStarted OnInstantReplayStarted;

	/// Fired when instant replay playback ends
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnInstantReplayEnded OnInstantReplayEnded;

	/// Fired when the overlay preset changes
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnOverlayPresetChanged OnOverlayPresetChanged;

protected:
	//-------------------------------------------------------------------------
	// Internal Update Methods
	//-------------------------------------------------------------------------

	/** Main tick function for caster tools updates. */
	void OnCasterTick();

	/** Update racer data from the race system. */
	void UpdateRacerData();

	/** Scan for and update battle zones. */
	void DetectBattles();

	/** Scan for and register highlight moments. */
	void DetectHighlights();

	/** Process automatic camera switching logic. */
	void ProcessAutoCamera();

	/** Register a new highlight moment. */
	void RegisterHighlight(const FMGHighlightMoment& Highlight);

	/** Set up default hotkey bindings. */
	void InitializeHotkeys();

	//-------------------------------------------------------------------------
	// Camera State
	//-------------------------------------------------------------------------

	/// Current camera configuration
	UPROPERTY()
	FMGCasterCameraConfig CurrentCameraConfig;

	/// Player ID currently focused by the camera
	UPROPERTY()
	FString FocusedPlayerID;

	//-------------------------------------------------------------------------
	// Overlay State
	//-------------------------------------------------------------------------

	/// Current overlay preset
	UPROPERTY()
	EMGOverlayPreset CurrentOverlayPreset = EMGOverlayPreset::Standard;

	//-------------------------------------------------------------------------
	// Race Data
	//-------------------------------------------------------------------------

	/// Real-time data for all racers
	UPROPERTY()
	TArray<FMGRacerOverlayData> RacerData;

	/// Aggregate race statistics
	UPROPERTY()
	FMGRaceStatsSummary RaceStats;

	/// Track sector timing data
	UPROPERTY()
	TArray<FMGTrackSector> SectorData;

	/// Currently active battle zones
	UPROPERTY()
	TArray<FMGBattleZone> ActiveBattles;

	/// All detected highlight moments
	UPROPERTY()
	TArray<FMGHighlightMoment> Highlights;

	/// Configured hotkey bindings
	UPROPERTY()
	TArray<FMGCasterHotkey> HotkeyBindings;

	//-------------------------------------------------------------------------
	// Overlay Element Visibility
	//-------------------------------------------------------------------------

	/// Show leaderboard on overlay
	UPROPERTY()
	bool bShowLeaderboard = true;

	/// Show timing tower on overlay
	UPROPERTY()
	bool bShowTimingTower = true;

	/// Show minimap on overlay
	UPROPERTY()
	bool bShowMinimap = true;

	/// Show driver information cards
	UPROPERTY()
	bool bShowDriverCards = false;

	/// Show battle zone indicators
	UPROPERTY()
	bool bShowBattleIndicators = true;

	//-------------------------------------------------------------------------
	// Replay State
	//-------------------------------------------------------------------------

	/// Currently playing an instant replay
	UPROPERTY()
	bool bPlayingInstantReplay = false;

	/// Auto-replay feature enabled
	UPROPERTY()
	bool bAutoReplayEnabled = true;

	/// Minimum significance for auto-replay trigger
	UPROPERTY()
	float AutoReplayMinSignificance = 0.7f;

	//-------------------------------------------------------------------------
	// Telestrator State
	//-------------------------------------------------------------------------

	/// Currently in drawing mode
	UPROPERTY()
	bool bDrawingMode = false;

	/// Current drawing color
	UPROPERTY()
	FLinearColor DrawingColor = FLinearColor::Yellow;

	/// Current drawing line thickness
	UPROPERTY()
	float DrawingThickness = 3.0f;

	//-------------------------------------------------------------------------
	// Recording State
	//-------------------------------------------------------------------------

	/// Currently recording broadcast
	UPROPERTY()
	bool bRecordingBroadcast = false;

	//-------------------------------------------------------------------------
	// Detection Thresholds
	//-------------------------------------------------------------------------

	/// Gap threshold for battle detection (seconds)
	UPROPERTY()
	float BattleGapThreshold = 0.5f;

	/// Timer for auto camera switching
	UPROPERTY()
	float AutoSwitchTimer = 0.0f;

	/// Previous leader for lead change detection
	UPROPERTY()
	FString PreviousLeaderID;

	/// Timer handle for periodic updates
	FTimerHandle CasterTickHandle;
};
