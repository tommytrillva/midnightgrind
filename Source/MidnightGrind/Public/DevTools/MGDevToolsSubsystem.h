// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGDevToolsSubsystem.h - Developer Tools and Debug Utilities
 * =============================================================================
 *
 * PURPOSE:
 * This is the main developer tools subsystem that provides cheats, performance
 * monitoring, debug visualizations, and time manipulation. Unlike MGDevCommands
 * which focuses on console commands, this subsystem provides a more complete
 * developer toolkit with metrics, profiling, and configurable access control.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. ACCESS CONTROL (EMGDevToolAccess):
 *    - Disabled: Cheats completely off (for shipping builds)
 *    - DevBuildOnly: Only works in Editor and Development builds
 *    - AllBuilds: Works everywhere (DANGEROUS for production!)
 *    - This prevents players from cheating in the released game.
 *
 * 2. PERFORMANCE METRICS (FMGPerformanceMetrics):
 *    - FPS: Frames Per Second - how smoothly the game runs
 *    - FrameTimeMS: Milliseconds per frame (16.67ms = 60 FPS)
 *    - GameThreadMS: Time spent on game logic (AI, physics, etc.)
 *    - RenderThreadMS: Time spent preparing graphics commands
 *    - GPUTimeMS: Time the graphics card spends drawing
 *    - DrawCalls: Number of separate draw commands (fewer = better)
 *    - If any of these are too high, the game will lag.
 *
 * 3. DEBUG VISUALIZATION (FMGDebugVisualization):
 *    - Visual overlays that help debug specific systems
 *    - ShowCollision: See collision boxes/spheres
 *    - ShowAIDebug: See what AI is "thinking"
 *    - ShowRacingLine: See the optimal path around the track
 *    - ShowCheckpoints: See checkpoint triggers
 *    - These are only visible in development builds.
 *
 * 4. TIME CONTROL:
 *    - SetTimeScale(): Slow motion (0.5) or fast forward (2.0)
 *    - PauseGame(): Completely stop time
 *    - StepFrame(): Advance exactly one frame while paused
 *    - Essential for debugging physics and animations frame-by-frame.
 *
 * 5. DELEGATES (DECLARE_DYNAMIC_MULTICAST_DELEGATE):
 *    - OnCheatExecuted: Fires when any cheat is used (for logging/telemetry)
 *    - OnDevConsoleToggled: Fires when the dev console opens/closes
 *    - "Dynamic" means it works with Blueprints
 *    - "Multicast" means multiple listeners can subscribe
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Developer Input]
 *           |
 *           v
 *    [UMGDevToolsSubsystem]
 *           |
 *           +---> [Cheat Commands] -- Modify game state directly
 *           +---> [Performance Monitor] -- Collect and display metrics
 *           +---> [Debug Visualization] -- Toggle visual overlays
 *           +---> [Time Control] -- Manipulate game time
 *           |
 *           v
 *    [Various Game Subsystems] -- Currency, Vehicles, Races, etc.
 *
 * SECURITY CONSIDERATIONS:
 * - This subsystem can completely break game balance (infinite money, etc.)
 * - ALWAYS set AccessLevel to Disabled for shipping builds
 * - Consider logging cheat usage for QA tracking
 * - Never expose these functions in player-facing UI
 *
 * COMMON USE CASES:
 * - GiveGrindCash/GiveNeonCredits: Test purchases without grinding
 * - UnlockAllVehicles/Tracks: Access all content for testing
 * - SetTimeScale: Debug physics in slow motion
 * - StartProfiling: Find performance bottlenecks
 * - ToggleFPSDisplay: Monitor performance during play
 *
 * USAGE EXAMPLE (C++):
 *   UMGDevToolsSubsystem* DevTools = GetGameInstance()->GetSubsystem<UMGDevToolsSubsystem>();
 *   if (DevTools->IsDevBuild())
 *   {
 *       DevTools->GiveGrindCash(1000000);
 *       DevTools->SetTimeScale(0.5f);  // Half speed for debugging
 *   }
 *
 * USAGE EXAMPLE (Blueprint):
 *   1. Get Game Instance
 *   2. Get Subsystem (UMGDevToolsSubsystem)
 *   3. Check IsDevBuild()
 *   4. Call desired cheat function
 *
 * @see UMGDevCommands for console command-based cheats
 * @see UMGEconomySubsystem for legitimate currency operations
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Analytics/MGAnalyticsSubsystem.h"
#include "MGDevToolsSubsystem.generated.h"

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Access level for developer tools
 *
 * Controls when dev tools are available based on build configuration.
 * This should be set appropriately to prevent cheating in release builds.
 */
UENUM(BlueprintType)
enum class EMGDevToolAccess : uint8
{
	/** Dev tools completely disabled */
	Disabled,
	/** Dev tools only available in development/editor builds */
	DevBuildOnly,
	/** Dev tools available in all builds (use with caution) */
	AllBuilds
};

// ============================================================================
// Data Structures
// ============================================================================

/**
 * @brief Definition of a cheat command
 *
 * Describes a single cheat command including its trigger string,
 * parameters, and access requirements.
 */
USTRUCT(BlueprintType)
struct FMGCheatCommand
{
	GENERATED_BODY()

	/** Unique identifier for this cheat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CommandID;

	/** Console command string to trigger this cheat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Command;

	/** Human-readable description of what this cheat does */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Parameter names accepted by this command */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Parameters;

	/** Whether this cheat requires a development build */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDevBuild = true;
};

// FMGPerformanceMetrics - defined in Analytics/MGAnalyticsSubsystem.h

/**
 * @brief Debug visualization toggle settings
 *
 * Controls which debug overlays and visualizations are currently active.
 */
USTRUCT(BlueprintType)
struct FMGDebugVisualization
{
	GENERATED_BODY()

	/** Display FPS counter on screen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowFPS = false;

	/** Display network statistics overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowNetStats = false;

	/** Render collision volumes and shapes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCollision = false;

	/** Show AI decision-making debug info */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowAIDebug = false;

	/** Display the optimal racing line on track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRacingLine = false;

	/** Highlight checkpoint triggers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCheckpoints = false;

	/** Show vehicle spawn point locations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpawnPoints = false;

	/** Render scene in wireframe mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWireframeMode = false;
};

// ============================================================================
// Delegate Declarations
// ============================================================================

/** Broadcast when a cheat command is executed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCheatExecuted, FName, CommandID, const TArray<FString>&, Parameters);

/** Broadcast when the dev console is toggled open/closed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnDevConsoleToggled);

// ============================================================================
// Subsystem Class
// ============================================================================

/**
 * @brief Developer Tools Subsystem
 *
 * Provides development and debugging functionality including cheat commands,
 * performance monitoring, debug visualizations, and time manipulation.
 *
 * This subsystem should only be accessible in development builds to prevent
 * cheating in production releases. Access level can be configured via
 * SetAccessLevel().
 *
 * Persists across level transitions as a GameInstance subsystem.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGDevToolsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @brief Called when the subsystem is created. Registers built-in cheats and starts metrics collection.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ========================================================================
	// Console Commands
	// ========================================================================

	/**
	 * @brief Execute a console command string
	 * @param Command The command to execute (e.g., "give_cash 10000")
	 * @return True if the command was recognized and executed
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools")
	bool ExecuteCommand(const FString& Command);

	/**
	 * @brief Register a new cheat command
	 * @param Cheat The cheat command definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools")
	void RegisterCheat(const FMGCheatCommand& Cheat);

	/**
	 * @brief Get all registered cheat commands
	 * @return Array of available cheat definitions
	 */
	UFUNCTION(BlueprintPure, Category = "DevTools")
	TArray<FMGCheatCommand> GetAvailableCheats() const;

	/**
	 * @brief Check if this is a development build
	 * @return True if running in editor or development configuration
	 */
	UFUNCTION(BlueprintPure, Category = "DevTools")
	bool IsDevBuild() const;

	// ========================================================================
	// Quick Cheats - Currency
	// ========================================================================

	/**
	 * @brief Add Grind Cash (primary currency) to the player
	 * @param Amount The amount of Grind Cash to add
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void GiveGrindCash(int64 Amount);

	/**
	 * @brief Add Neon Credits (premium currency) to the player
	 * @param Amount The amount of Neon Credits to add
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void GiveNeonCredits(int64 Amount);

	// ========================================================================
	// Quick Cheats - Unlocks
	// ========================================================================

	/**
	 * @brief Unlock all vehicles in the player's garage
	 * Does not affect purchase/ownership records
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void UnlockAllVehicles();

	/**
	 * @brief Unlock all tracks/races
	 * Bypasses progression requirements
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void UnlockAllTracks();

	// ========================================================================
	// Quick Cheats - Race Manipulation
	// ========================================================================

	/**
	 * @brief Set the player's speed multiplier
	 * @param Multiplier Speed multiplier (1.0 = normal, 2.0 = double speed)
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void SetPlayerSpeed(float Multiplier);

	/**
	 * @brief Set the AI racers' speed multiplier
	 * @param Multiplier Speed multiplier (1.0 = normal, 0.5 = half speed)
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void SetAISpeed(float Multiplier);

	/**
	 * @brief Teleport the player to a specific checkpoint
	 * @param CheckpointIndex The 0-based index of the checkpoint
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void TeleportToCheckpoint(int32 CheckpointIndex);

	/**
	 * @brief Instantly win the current race
	 * Awards standard victory rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void WinRace();

	/**
	 * @brief Skip to a specific position in the race
	 * @param Position The race position to skip to (1 = first place)
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Cheats")
	void SkipToPosition(int32 Position);

	// ========================================================================
	// Performance Monitoring
	// ========================================================================

	/**
	 * @brief Get current performance metrics
	 * @return Snapshot of current performance data
	 */
	UFUNCTION(BlueprintPure, Category = "DevTools|Performance")
	FMGPerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

	/**
	 * @brief Start a named profiling session
	 * Data will be saved when StopProfiling is called
	 * @param ProfileName Name for this profiling session
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Performance")
	void StartProfiling(const FString& ProfileName);

	/**
	 * @brief Stop the current profiling session and save data
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Performance")
	void StopProfiling();

	/**
	 * @brief Capture the current frame for detailed analysis
	 * Saves GPU capture if supported by the platform
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Performance")
	void CaptureFrame();

	// ========================================================================
	// Debug Visualization
	// ========================================================================

	/**
	 * @brief Set all debug visualization settings at once
	 * @param Settings The visualization settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Debug")
	void SetVisualization(const FMGDebugVisualization& Settings);

	/**
	 * @brief Get current debug visualization settings
	 * @return Current visualization toggle states
	 */
	UFUNCTION(BlueprintPure, Category = "DevTools|Debug")
	FMGDebugVisualization GetVisualization() const { return DebugVis; }

	/**
	 * @brief Toggle the FPS display on/off
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Debug")
	void ToggleFPSDisplay();

	/**
	 * @brief Toggle the network stats display on/off
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Debug")
	void ToggleNetStats();

	// ========================================================================
	// Time Control
	// ========================================================================

	/**
	 * @brief Set the game time scale
	 * Affects physics, animations, and game logic
	 * @param Scale Time multiplier (1.0 = normal, 0.5 = half speed, 2.0 = double)
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Time")
	void SetTimeScale(float Scale);

	/**
	 * @brief Pause the game completely
	 * Equivalent to SetTimeScale(0.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Time")
	void PauseGame();

	/**
	 * @brief Advance the game by exactly one frame while paused
	 * Useful for frame-by-frame debugging
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools|Time")
	void StepFrame();

	// ========================================================================
	// Access Control
	// ========================================================================

	/**
	 * @brief Set the access level for dev tools
	 * @param Access The access level to set
	 */
	UFUNCTION(BlueprintCallable, Category = "DevTools")
	void SetAccessLevel(EMGDevToolAccess Access);

	/**
	 * @brief Get the current access level
	 * @return The current access level setting
	 */
	UFUNCTION(BlueprintPure, Category = "DevTools")
	EMGDevToolAccess GetAccessLevel() const { return AccessLevel; }

	// ========================================================================
	// Events
	// ========================================================================

	/** Broadcast when any cheat command is executed */
	UPROPERTY(BlueprintAssignable, Category = "DevTools|Events")
	FMGOnCheatExecuted OnCheatExecuted;

	/** Broadcast when the developer console is opened or closed */
	UPROPERTY(BlueprintAssignable, Category = "DevTools|Events")
	FMGOnDevConsoleToggled OnDevConsoleToggled;

protected:
	/// @brief Register all built-in cheat commands
	void RegisterBuiltInCheats();

	/// @brief Update performance metrics (called on timer)
	void UpdateMetrics();

	/// @brief Check if a cheat can be executed given current access level
	/// @param Cheat The cheat to check
	/// @return True if the cheat is allowed to execute
	bool CanExecuteCheat(const FMGCheatCommand& Cheat) const;

private:
	/** All registered cheat commands */
	UPROPERTY()
	TArray<FMGCheatCommand> RegisteredCheats;

	/** Current performance metrics snapshot */
	FMGPerformanceMetrics CurrentMetrics;

	/** Current debug visualization settings */
	FMGDebugVisualization DebugVis;

	/** Current dev tools access level */
	EMGDevToolAccess AccessLevel = EMGDevToolAccess::DevBuildOnly;

	/** Whether a profiling session is active */
	bool bIsProfiling = false;

	/** Timer for periodic metrics updates */
	FTimerHandle MetricsTimerHandle;
};
