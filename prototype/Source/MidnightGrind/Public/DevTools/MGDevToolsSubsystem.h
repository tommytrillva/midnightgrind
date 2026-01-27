// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGDevToolsSubsystem.h
 * @brief Developer Tools and Debug Utilities Subsystem
 *
 * This subsystem provides development and debugging functionality for
 * Midnight Grind. It includes cheat commands, performance monitoring,
 * debug visualizations, and time manipulation tools.
 *
 * Key Features:
 * - Console command system with cheat registration
 * - Quick cheats for currency, unlocks, and race manipulation
 * - Real-time performance metrics monitoring
 * - Debug visualization overlays (collision, AI, racing lines)
 * - Time control (pause, slow-mo, frame stepping)
 * - Configurable access levels for different build types
 *
 * @warning This subsystem should only be enabled in development builds.
 *          Production builds should have AccessLevel set to Disabled.
 *
 * Usage Example:
 * @code
 * UMGDevToolsSubsystem* DevTools = GameInstance->GetSubsystem<UMGDevToolsSubsystem>();
 * if (DevTools->IsDevBuild())
 * {
 *     DevTools->GiveGrindCash(1000000);
 *     DevTools->SetTimeScale(0.5f);
 * }
 * @endcode
 *
 * @see UMGEconomySubsystem for legitimate currency operations
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
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

/**
 * @brief Real-time performance metrics snapshot
 *
 * Contains a snapshot of current performance data for monitoring
 * and debugging purposes.
 */
USTRUCT(BlueprintType)
struct FMGPerformanceMetrics
{
	GENERATED_BODY()

	/** Current frames per second */
	UPROPERTY(BlueprintReadOnly)
	float FPS = 0.0f;

	/** Time for the last frame in milliseconds */
	UPROPERTY(BlueprintReadOnly)
	float FrameTimeMS = 0.0f;

	/** Time spent on game thread in milliseconds */
	UPROPERTY(BlueprintReadOnly)
	float GameThreadMS = 0.0f;

	/** Time spent on render thread in milliseconds */
	UPROPERTY(BlueprintReadOnly)
	float RenderThreadMS = 0.0f;

	/** Time spent on GPU in milliseconds */
	UPROPERTY(BlueprintReadOnly)
	float GPUTimeMS = 0.0f;

	/** Number of draw calls this frame */
	UPROPERTY(BlueprintReadOnly)
	int32 DrawCalls = 0;

	/** Total triangles rendered this frame */
	UPROPERTY(BlueprintReadOnly)
	int64 TrianglesRendered = 0;

	/** Current memory usage in megabytes */
	UPROPERTY(BlueprintReadOnly)
	int64 MemoryUsedMB = 0;

	/** Network round-trip latency in milliseconds */
	UPROPERTY(BlueprintReadOnly)
	float NetworkLatencyMS = 0.0f;
};

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
