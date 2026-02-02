// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGCrashReportingSubsystem.h
 * @brief Crash Reporting and Error Tracking Subsystem for Midnight Grind
 *
 * @section overview Overview
 * This file defines the Crash Reporting Subsystem, which provides comprehensive
 * error tracking, crash detection, and diagnostic data collection for Midnight Grind.
 * The subsystem helps developers identify and fix bugs by capturing detailed
 * information when errors or crashes occur during gameplay.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection game_instance_subsystem Game Instance Subsystem
 * This class inherits from UGameInstanceSubsystem, meaning:
 * - ONE instance exists for the entire game session (across all levels)
 * - It automatically initializes when the game starts
 * - It persists even when loading different maps/levels
 * - Access from anywhere: GetGameInstance()->GetSubsystem<UMGCrashReportingSubsystem>()
 *
 * @subsection severity Crash Severity Levels
 * - Info: General information messages (not errors)
 * - Warning: Potential issues that don't stop gameplay
 * - Error: Problems that may affect gameplay but don't crash
 * - Fatal: Crashes or unrecoverable errors
 *
 * @subsection breadcrumbs Breadcrumbs
 * Breadcrumbs are a trail of events leading up to a crash. Think of them like
 * "footprints" that show what the player was doing before an error:
 * - "Player started race on Downtown track"
 * - "Player enabled nitro boost"
 * - "Vehicle collision detected"
 * - "CRASH: Null pointer in physics system"
 * This helps developers understand the sequence of events that caused the issue.
 *
 * @subsection metadata Context and Metadata
 * Additional data attached to crash reports:
 * - User ID: Which player experienced the crash
 * - Session ID: Unique identifier for the play session
 * - Replay ID: Links to replay recording for reproducing the bug
 * - Tags: Key-value pairs for filtering (e.g., "track=downtown", "mode=online")
 * - Build Version: Which version of the game was running
 * - Platform: PC, Console, etc.
 *
 * @section usage Usage Examples
 *
 * @subsection setup Basic Setup
 * @code
 * // Get the subsystem
 * UMGCrashReportingSubsystem* CrashReporter =
 *     GetGameInstance()->GetSubsystem<UMGCrashReportingSubsystem>();
 *
 * // Set user context for all reports
 * CrashReporter->SetUserID(PlayerController->GetPlayerID());
 * CrashReporter->SetSessionID(FGuid::NewGuid().ToString());
 *
 * // Add tags for filtering crash reports later
 * CrashReporter->AddTag("game_mode", "career");
 * CrashReporter->AddTag("track", TrackName);
 * @endcode
 *
 * @subsection breadcrumb_usage Adding Breadcrumbs
 * @code
 * // Simple breadcrumb
 * CrashReporter->AddBreadcrumb("Race", "Player started race");
 *
 * // Breadcrumb with extra data
 * TMap<FString, FString> Data;
 * Data.Add("vehicle", VehicleID);
 * Data.Add("position", FString::FromInt(GridPosition));
 * CrashReporter->AddBreadcrumbWithData("Race", "Race countdown started", Data);
 * @endcode
 *
 * @subsection error_reporting Reporting Errors
 * @code
 * // Report a general error
 * CrashReporter->ReportCrash("Physics system returned invalid velocity", EMGCrashSeverity::Error);
 *
 * // Report a caught exception
 * CrashReporter->ReportException("NullReferenceException", "Vehicle pawn was null during respawn");
 *
 * // Report a specific error code
 * CrashReporter->ReportError("ERR_NETWORK_001", "Failed to connect to matchmaking server");
 * @endcode
 *
 * @subsection events Listening for Crash Events
 * @code
 * // In your class
 * CrashReporter->OnCrashReported.AddDynamic(this, &UMyClass::HandleCrashReported);
 *
 * void UMyClass::HandleCrashReported(const FMGCrashReport& Report)
 * {
 *     // Log to custom analytics
 *     UE_LOG(LogGame, Error, TEXT("Crash reported: %s"), *Report.Message);
 * }
 * @endcode
 *
 * @section architecture Architecture
 * @verbatim
 *   [Game Code]
 *        |
 *        v
 *   [UMGCrashReportingSubsystem]
 *        |
 *        +---> Breadcrumbs (circular buffer of recent events)
 *        +---> Tags/Metadata (context for filtering)
 *        +---> OnCrashReported delegate (notify listeners)
 *        |
 *        v
 *   [Crash Upload Service] --> [Backend Analytics]
 * @endverbatim
 *
 * @see EMGCrashSeverity - Severity levels for crash reports
 * @see FMGCrashReport - Complete crash report data structure
 * @see FMGBreadcrumb - Individual breadcrumb entry
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrashReportingSubsystem.generated.h"

/**
 * Crash Severity Levels
 *
 * Indicates how serious the error is. Choose the appropriate level to help
 * developers prioritize which bugs to fix first.
 *
 * Severity determines:
 * - Whether the crash is uploaded immediately vs batched
 * - Priority in the bug tracking system
 * - Whether the player is shown an error dialog
 */
UENUM(BlueprintType)
enum class EMGCrashSeverity : uint8
{
	Info,		// Not really an error - just notable information for debugging
	Warning,	// Something unexpected but gameplay continues normally
	Error,		// Something broke, gameplay may be degraded but continues
	Fatal		// Game cannot continue and will crash/close
};

/**
 * Crash Report - Complete Error Information Package
 *
 * Contains all information needed to debug and fix an error. When a crash
 * or error is reported, this structure is populated and uploaded to the
 * crash reporting backend.
 *
 * A good crash report should help developers answer:
 * - What went wrong? (Message, StackTrace)
 * - When did it happen? (Timestamp)
 * - What version/platform? (BuildVersion, Platform)
 * - What was the player doing? (Breadcrumbs via Metadata)
 * - How can we reproduce it? (ReplayID)
 */
USTRUCT(BlueprintType)
struct FMGCrashReport
{
	GENERATED_BODY()

	/** Unique identifier for this crash report (GUID format) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReportID;

	/** How severe the error is (Info, Warning, Error, Fatal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrashSeverity Severity = EMGCrashSeverity::Error;

	/** Human-readable description of what went wrong */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	/**
	 * Call stack showing the sequence of function calls that led to the error.
	 * Example:
	 *   UVehicle::UpdatePhysics()
	 *   UVehicle::Tick()
	 *   AActor::Tick()
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StackTrace;

	/** Game version (e.g., "1.0.0.1234") - critical for knowing if bug is already fixed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuildVersion;

	/** Platform identifier (e.g., "Windows", "PS5", "XboxSeriesX") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;

	/** When the crash occurred (UTC time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/** Additional context data and tags attached to this report */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Metadata;

	/** Link to replay recording if available (for reproducing the bug) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayID;

	/** Session identifier to correlate with analytics data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;
};

/**
 * Breadcrumb - Single Event in the Trail Leading to a Crash
 *
 * Breadcrumbs create a timeline of events before a crash, helping developers
 * understand what the player was doing. The most recent 100 breadcrumbs are
 * kept in a circular buffer.
 *
 * Good breadcrumb practices:
 * - Add breadcrumbs at key state changes (race start, vehicle change, menu open)
 * - Include relevant IDs in Data (track ID, vehicle ID)
 * - Use consistent category names for filtering
 * - Don't add too frequently (not every frame, but at meaningful moments)
 *
 * Categories should be consistent across the codebase:
 * - "Race" - Race lifecycle events
 * - "Vehicle" - Vehicle state changes
 * - "UI" - Menu and HUD interactions
 * - "Network" - Online/multiplayer events
 * - "Save" - Save/load operations
 */
USTRUCT(BlueprintType)
struct FMGBreadcrumb
{
	GENERATED_BODY()

	/** Category for grouping/filtering (e.g., "Race", "Vehicle", "UI") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Category;

	/** Human-readable description of what happened */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	/** When this event occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/** Additional key-value data for context (e.g., "TrackID" -> "Downtown") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Data;
};

// ============================================================================
// DELEGATE
// ============================================================================

/**
 * Delegate broadcast whenever a crash report is generated.
 * Bind to this to add custom crash handling (e.g., show error UI, log to custom service).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrashReported, const FMGCrashReport&, Report);

// ============================================================================
// MAIN CRASH REPORTING SUBSYSTEM CLASS
// ============================================================================

/**
 * UMGCrashReportingSubsystem
 *
 * Central system for error tracking and crash reporting. This subsystem helps
 * developers find and fix bugs by capturing detailed information when things
 * go wrong.
 *
 * WHEN TO USE THIS VS ANALYTICS:
 * - Use Analytics for expected events (race started, purchase made)
 * - Use CrashReporting for unexpected errors (null pointer, network failure)
 *
 * LIFECYCLE:
 * - Initialize(): Installs crash handlers, sets up platform-specific hooks
 * - Deinitialize(): Flushes any pending reports
 *
 * AUTOMATIC VS MANUAL REPORTING:
 * - Automatic (bAutoCapture = true): Catches unhandled exceptions, asserts
 * - Manual: Call ReportCrash(), ReportError(), ReportException() yourself
 *
 * BEST PRACTICES:
 * - Add breadcrumbs at key points in your code
 * - Set user/session context early (in game mode BeginPlay)
 * - Use consistent error codes for ReportError()
 * - Link replay IDs when recording replays
 *
 * ACCESSING THE SUBSYSTEM:
 *   UMGCrashReportingSubsystem* CrashReporting =
 *       GetGameInstance()->GetSubsystem<UMGCrashReportingSubsystem>();
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCrashReportingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Called when subsystem is created.
	 * Installs native crash handlers and initializes the reporting system.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Called when subsystem is destroyed.
	 * Flushes any pending crash reports.
	 */
	virtual void Deinitialize() override;

	// ==========================================
	// CRASH REPORTING - Report Errors
	// ==========================================
	// These functions create and upload crash reports.
	// Use the appropriate function based on the error type.

	/**
	 * Report a general crash or error with a severity level.
	 * Creates a crash report, attaches breadcrumbs, and uploads.
	 *
	 * @param Message - Description of what went wrong
	 * @param Severity - How serious the error is (default: Error)
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ReportCrash(const FString& Message, EMGCrashSeverity Severity = EMGCrashSeverity::Error);

	/**
	 * Report a caught exception.
	 * Use when you catch an exception and want to report it without crashing.
	 *
	 * @param ExceptionType - Type of exception (e.g., "NullReferenceException")
	 * @param Message - Details about the exception
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ReportException(const FString& ExceptionType, const FString& Message);

	/**
	 * Report a specific error code.
	 * Use for known error conditions with standardized codes.
	 * Error codes should be documented in a central location.
	 *
	 * @param ErrorCode - Standardized error code (e.g., "NET_001", "SAVE_002")
	 * @param Message - Human-readable description
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ReportError(const FString& ErrorCode, const FString& Message);

	// ==========================================
	// BREADCRUMBS - Event Trail for Debugging
	// ==========================================
	// Breadcrumbs create a trail of events leading up to a crash.
	// Add them at key points in your code to help with debugging.

	/**
	 * Add a simple breadcrumb (event marker).
	 * Call this at key points in gameplay to track what the player was doing.
	 *
	 * @param Category - Event category (e.g., "Race", "UI", "Vehicle")
	 * @param Message - What happened (e.g., "Started race", "Opened garage menu")
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void AddBreadcrumb(const FString& Category, const FString& Message);

	/**
	 * Add a breadcrumb with additional context data.
	 * Use when you need to attach extra information for debugging.
	 *
	 * @param Category - Event category
	 * @param Message - What happened
	 * @param Data - Key-value pairs with additional context
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void AddBreadcrumbWithData(const FString& Category, const FString& Message, const TMap<FString, FString>& Data);

	/**
	 * Clear all breadcrumbs.
	 * Use when starting a fresh context (e.g., returning to main menu).
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ClearBreadcrumbs();

	/**
	 * Get all current breadcrumbs.
	 * Useful for debugging or custom crash handling.
	 *
	 * @return Array of breadcrumbs (most recent at end)
	 */
	UFUNCTION(BlueprintPure, Category = "CrashReporting")
	TArray<FMGBreadcrumb> GetBreadcrumbs() const { return Breadcrumbs; }

	// ==========================================
	// CONTEXT - Identifying Information
	// ==========================================
	// Set context to identify who/what is crashing.
	// This information is attached to all crash reports.

	/**
	 * Set the current user/player ID.
	 * Helps identify affected players and detect patterns.
	 *
	 * @param UserID - Player's unique identifier (from account system)
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetUserID(const FString& UserID);

	/**
	 * Set the current session ID.
	 * Links crash reports to analytics session data.
	 *
	 * @param SessionID - Session identifier (from AnalyticsSubsystem)
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetSessionID(const FString& SessionID);

	/**
	 * Add a tag for filtering crash reports.
	 * Tags help group and filter crashes in the dashboard.
	 *
	 * @param Key - Tag name (e.g., "game_mode", "track", "vehicle")
	 * @param Value - Tag value (e.g., "career", "downtown", "speedster")
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void AddTag(const FString& Key, const FString& Value);

	/**
	 * Set the current replay recording ID.
	 * If a crash occurs, developers can watch the replay to reproduce it.
	 *
	 * @param ReplayID - Identifier for the current replay recording
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetCurrentReplayID(const FString& ReplayID);

	// ==========================================
	// CONFIGURATION - Settings
	// ==========================================
	// Control crash reporting behavior.

	/**
	 * Enable or disable crash reporting.
	 * Players should be able to opt out for privacy.
	 *
	 * @param bEnabled - True to enable, false to disable all reporting
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetEnabled(bool bEnabled);

	/**
	 * Check if crash reporting is currently enabled.
	 *
	 * @return True if reporting is active
	 */
	UFUNCTION(BlueprintPure, Category = "CrashReporting")
	bool IsEnabled() const { return bIsEnabled; }

	/**
	 * Enable or disable automatic crash capture.
	 * When enabled, unhandled exceptions are automatically reported.
	 * When disabled, only manual ReportCrash() calls create reports.
	 *
	 * @param bAuto - True to auto-capture, false for manual only
	 */
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetAutoCapture(bool bAuto);

	// ==========================================
	// EVENTS - Delegates
	// ==========================================

	/**
	 * Broadcast when a crash report is created.
	 * Bind to this to show error UI or log to custom services.
	 */
	UPROPERTY(BlueprintAssignable, Category = "CrashReporting|Events")
	FMGOnCrashReported OnCrashReported;

protected:
	// ==========================================
	// INTERNAL FUNCTIONS
	// ==========================================

	/**
	 * Install platform-specific crash handlers.
	 * Called during Initialize() to hook into OS crash reporting.
	 */
	void InstallCrashHandler();

	/**
	 * Upload a crash report to the backend server.
	 * Handles network failures gracefully with retry logic.
	 */
	void UploadReport(const FMGCrashReport& Report);

	/**
	 * Generate a stack trace from the current call stack.
	 * Platform-specific implementation.
	 *
	 * @return Formatted stack trace string
	 */
	FString GenerateStackTrace();

	/**
	 * Callback when Unreal Engine's crash handler is triggered.
	 * Creates and uploads a crash report with maximum available info.
	 */
	void OnEngineCrash();

private:
	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Circular buffer of recent breadcrumbs (limited to MaxBreadcrumbs) */
	UPROPERTY()
	TArray<FMGBreadcrumb> Breadcrumbs;

	/** Key-value tags for filtering/grouping crash reports */
	TMap<FString, FString> Tags;

	/** Current player's unique identifier */
	FString CurrentUserID;

	/** Current play session identifier */
	FString CurrentSessionID;

	/** Current replay recording identifier (if any) */
	FString CurrentReplayID;

	/** Game build version string */
	FString BuildVersion;

	/** Master enable/disable switch */
	bool bIsEnabled = true;

	/** Whether to automatically capture unhandled crashes */
	bool bAutoCapture = true;

	/** Maximum number of breadcrumbs to keep (oldest are dropped) */
	int32 MaxBreadcrumbs = 100;
};
