// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGAntiCheatSubsystem.h
 * @brief Anti-Cheat Subsystem for Midnight Grind
 *
 * This subsystem provides comprehensive cheat detection and prevention for the game.
 * It operates as a client-side first line of defense, working in conjunction with
 * server-side validation (see MGServerAuthSubsystem) to maintain fair play.
 *
 * Key Responsibilities:
 * - Real-time validation of player movement, speed, and race results
 * - Detection of common cheating methods (speed hacks, teleportation, memory manipulation)
 * - File and memory integrity verification to detect tampering
 * - Player trust scoring system to flag suspicious accounts
 * - Player report management for community-driven moderation
 * - Automatic penalty application for confirmed violations
 *
 * Architecture Overview:
 * The subsystem uses a layered approach to cheat detection:
 * 1. Real-time validation - Checks game state changes as they happen
 * 2. Statistical analysis - Detects anomalies over time
 * 3. Integrity checks - Periodic verification of game files and memory
 * 4. Trust system - Long-term reputation tracking per player
 *
 * Integration Notes:
 * - This subsystem automatically initializes with the game instance
 * - Validation functions should be called from gameplay code at appropriate points
 * - Violations are automatically reported to the backend server
 * - Works alongside MGServerAuthSubsystem for server-authoritative validation
 *
 * @see UMGServerAuthSubsystem for server-side validation
 * @see FMGViolationRecord for violation data structure
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAntiCheatSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Types of cheating violations that can be detected
 *
 * Each violation type corresponds to a specific cheating method. The anti-cheat
 * system uses these to categorize detected issues and apply appropriate penalties.
 */
UENUM(BlueprintType)
enum class EMGViolationType : uint8
{
	SpeedHack			UMETA(DisplayName = "Speed Hack"),			///< Player moving faster than physically possible
	TeleportHack		UMETA(DisplayName = "Teleport Hack"),		///< Instant position changes bypassing normal movement
	WallHack			UMETA(DisplayName = "Wall Hack"),			///< Passing through collision geometry
	TimerManipulation	UMETA(DisplayName = "Timer Manipulation"),	///< Altering game time or race timers
	ResourceHack		UMETA(DisplayName = "Resource Hack"),		///< Illegitimate currency or XP gains
	MemoryManipulation	UMETA(DisplayName = "Memory Manipulation"),	///< Direct memory editing detected
	PacketManipulation	UMETA(DisplayName = "Packet Manipulation"),	///< Altered network packets
	ImpossibleStats		UMETA(DisplayName = "Impossible Stats"),	///< Stats that exceed game limits
	AnomalousInput		UMETA(DisplayName = "Anomalous Input"),		///< Inhuman input patterns (bots, macros)
	ModifiedFiles		UMETA(DisplayName = "Modified Files"),		///< Game files altered from expected state
	Exploit				UMETA(DisplayName = "Exploit"),				///< Abusing game bugs for unfair advantage
	Botting				UMETA(DisplayName = "Botting"),				///< Automated gameplay detected
	RubberBanding		UMETA(DisplayName = "Rubber Banding"),		///< Suspicious position corrections
	Unknown				UMETA(DisplayName = "Unknown")				///< Unclassified suspicious behavior
};

/**
 * @brief Severity levels for detected violations
 *
 * Severity determines the response to a violation. Lower severities may just
 * log data for analysis, while higher severities trigger immediate action.
 * The penalty system uses severity to determine ban duration and trust impact.
 */
UENUM(BlueprintType)
enum class EMGViolationSeverity : uint8
{
	Info		UMETA(DisplayName = "Info"),		///< Logged for analysis, no action taken
	Warning		UMETA(DisplayName = "Warning"),		///< Suspicious but not conclusive
	Minor		UMETA(DisplayName = "Minor"),		///< Confirmed minor violation, small penalty
	Major		UMETA(DisplayName = "Major"),		///< Serious violation, significant penalty
	Critical	UMETA(DisplayName = "Critical")		///< Severe violation, immediate ban considered
};

/**
 * @brief Player trust levels for reputation tracking
 *
 * Trust levels are calculated from cumulative player behavior over time.
 * Higher trust players may receive reduced validation overhead, while
 * lower trust players face increased scrutiny. Trust can be rebuilt
 * through consistent fair play.
 */
UENUM(BlueprintType)
enum class EMGTrustLevel : uint8
{
	Trusted			UMETA(DisplayName = "Trusted"),		///< Long-term clean record, reduced checks
	Normal			UMETA(DisplayName = "Normal"),		///< Default starting level for all players
	Suspicious		UMETA(DisplayName = "Suspicious"),	///< Minor violations detected, increased monitoring
	Flagged			UMETA(DisplayName = "Flagged"),		///< Multiple violations, under review
	Banned			UMETA(DisplayName = "Banned")		///< Access revoked due to cheating
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief Complete record of a detected cheating violation
 *
 * This structure captures all relevant information about a detected violation,
 * including evidence that can be used for review. Records are stored locally
 * and uploaded to the server for centralized tracking and analysis.
 */
USTRUCT(BlueprintType)
struct FMGViolationRecord
{
	GENERATED_BODY()

	/// Unique identifier for this specific violation instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FString ViolationID;

	/// The player who committed the violation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FString PlayerID;

	/// Classification of the cheating method used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	EMGViolationType Type = EMGViolationType::Unknown;

	/// How serious the violation is (affects penalty)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	EMGViolationSeverity Severity = EMGViolationSeverity::Info;

	/// Human-readable explanation of what was detected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FString Description;

	/// Contextual information (Race ID, session ID, track name, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FString Context;

	/// Key-value pairs of evidence data (e.g., "speed" -> "500", "expected_max" -> "350")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	TMap<FString, FString> Evidence;

	/// When the violation was detected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	FDateTime Timestamp;

	/// Whether this violation has been sent to the backend server
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	bool bReportedToServer = false;

	/// Whether a penalty has been applied for this violation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Violation")
	bool bActionTaken = false;
};

/**
 * @brief Player-submitted report of suspected cheating
 *
 * Allows players to report suspicious behavior they observe in other players.
 * Reports are stored and can be used alongside automated detection to identify
 * cheaters. Multiple reports against the same player increases review priority.
 */
USTRUCT(BlueprintType)
struct FMGPlayerReport
{
	GENERATED_BODY()

	/// Unique identifier for this report
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString ReportID;

	/// Player who submitted the report
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString ReporterID;

	/// Player being reported for cheating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString ReportedPlayerID;

	/// Category of the report (e.g., "Speed Hack", "Unfair Advantage")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString Reason;

	/// Additional details provided by the reporter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString Description;

	/// The race or match where the suspicious behavior was observed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FString MatchID;

	/// When the report was submitted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	FDateTime Timestamp;

	/// Whether a moderator has reviewed this report
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	bool bReviewed = false;

	/// Whether action was taken based on this report
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Report")
	bool bActionTaken = false;
};

/**
 * @brief Configuration thresholds for validation checks
 *
 * These values define the boundaries of acceptable gameplay. Values exceeding
 * these thresholds trigger violation detection. Thresholds should be tuned
 * based on the fastest vehicles and most extreme legitimate gameplay scenarios.
 *
 * @note These are default values; actual limits may vary per vehicle and track
 */
USTRUCT(BlueprintType)
struct FMGValidationThresholds
{
	GENERATED_BODY()

	/// Maximum allowed vehicle speed in km/h (fastest vehicle at max upgrades)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float MaxSpeed = 500.0f;

	/// Maximum allowed acceleration in m/s^2 (prevents instant speed gains)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float MaxAcceleration = 50.0f;

	/// Maximum distance a player can move in one frame in meters (teleport detection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float MaxTeleportDistance = 100.0f;

	/// Shortest possible lap time in seconds (per-track overrides recommended)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float MinLapTime = 20.0f;

	/// Maximum drift angle in degrees before physics violation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float MaxDriftAngle = 90.0f;

	/// Maximum in-game currency earnable from a single race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	int32 MaxCurrencyPerRace = 100000;

	/// Maximum XP earnable from a single race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	int32 MaxXPPerRace = 50000;

	/// Maximum ping variance in ms before flagging connection manipulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float MaxPingVariance = 500.0f;

	/// Maximum client-server time difference in seconds before flagging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float TimeDesyncThreshold = 2.0f;
};

/**
 * @brief Results from a system integrity check
 *
 * Contains the outcome of various integrity checks including file verification,
 * memory scanning, and environment detection. Used to identify tampered game
 * installations or suspicious runtime environments.
 */
USTRUCT(BlueprintType)
struct FMGIntegrityCheckResult
{
	GENERATED_BODY()

	/// True if all integrity checks passed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	bool bPassed = true;

	/// List of specific checks that failed (for logging/debugging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	TArray<FString> FailedChecks;

	/// File paths that have been modified from expected checksums
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	TArray<FString> ModifiedFiles;

	/// True if a debugger is attached to the game process
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	bool bDebuggerDetected = false;

	/// True if game is running inside a virtual machine
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	bool bVirtualMachineDetected = false;

	/// True if critical memory regions have been altered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	bool bMemoryModified = false;

	/// When this integrity check was performed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integrity")
	FDateTime CheckTime;
};

// ============================================================================
// DELEGATES
// ============================================================================

/// Broadcast when any violation is detected, regardless of severity
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViolationDetected, const FMGViolationRecord&, Violation);

/// Broadcast when a player's trust level changes (up or down)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrustLevelChanged, EMGTrustLevel, NewLevel);

/// Broadcast when a player is banned (includes the ban reason)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBanned, const FString&, Reason);

// ============================================================================
// ANTI-CHEAT SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Core anti-cheat subsystem for detecting and preventing cheating
 *
 * This GameInstanceSubsystem provides comprehensive cheat detection through
 * real-time validation, statistical analysis, and integrity checking. It
 * maintains player trust scores and handles both automated detection and
 * player-submitted reports.
 *
 * Usage Example:
 * @code
 * // Get the subsystem from the game instance
 * UMGAntiCheatSubsystem* AntiCheat = GameInstance->GetSubsystem<UMGAntiCheatSubsystem>();
 *
 * // Validate a player's speed during gameplay
 * if (!AntiCheat->ValidateSpeed(PlayerID, CurrentSpeed, VehicleID))
 * {
 *     // Violation was detected and recorded automatically
 * }
 *
 * // Check player trust before allowing ranked matches
 * EMGTrustLevel Trust = AntiCheat->GetPlayerTrustLevel(PlayerID);
 * if (Trust == EMGTrustLevel::Banned)
 * {
 *     // Reject player from match
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAntiCheatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Initializes the anti-cheat system, sets up periodic checks, and loads checksums
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Cleans up timers and pending reports before shutdown
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================
	// Subscribe to these delegates to react to anti-cheat events in your game code

	/// Fires when any violation is detected (use for logging, UI alerts)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnViolationDetected OnViolationDetected;

	/// Fires when a player's trust level changes (use to update matchmaking eligibility)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrustLevelChanged OnTrustLevelChanged;

	/// Fires when a player is banned (use to show ban screen, disconnect player)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerBanned OnPlayerBanned;

	// ==========================================
	// REAL-TIME VALIDATION
	// ==========================================
	// These functions should be called during gameplay to validate state changes.
	// They return false and record a violation if cheating is detected.

	/**
	 * @brief Validates player position change for teleport detection
	 * @param PlayerID Unique identifier of the player
	 * @param Position Current world position
	 * @param PreviousPosition Position from last frame
	 * @param DeltaTime Time elapsed since last check
	 * @return True if movement is valid, false if teleportation detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidatePosition(const FString& PlayerID, FVector Position, FVector PreviousPosition, float DeltaTime);

	/**
	 * @brief Validates that vehicle speed is within acceptable limits
	 * @param PlayerID Unique identifier of the player
	 * @param CurrentSpeed Current speed in km/h
	 * @param VehicleID The vehicle being driven (for per-vehicle limits)
	 * @return True if speed is valid, false if speed hack detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateSpeed(const FString& PlayerID, float CurrentSpeed, FName VehicleID);

	/**
	 * @brief Validates that a lap time is physically possible
	 * @param PlayerID Unique identifier of the player
	 * @param LapTime The recorded lap time in seconds
	 * @param TrackID The track being raced (for per-track minimums)
	 * @return True if lap time is valid, false if impossibly fast
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateLapTime(const FString& PlayerID, float LapTime, FName TrackID);

	/**
	 * @brief Validates complete race results including rewards
	 * @param PlayerID Unique identifier of the player
	 * @param Position Final race position (1st, 2nd, etc.)
	 * @param RaceTime Total race time in seconds
	 * @param CashEarned In-game currency earned
	 * @param XPEarned Experience points earned
	 * @return True if results are valid, false if manipulation detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateRaceResult(const FString& PlayerID, int32 Position, float RaceTime, int32 CashEarned, int32 XPEarned);

	/**
	 * @brief Validates currency transactions for resource hacking
	 * @param PlayerID Unique identifier of the player
	 * @param Amount Currency amount being added
	 * @param Source Where the currency came from (race, purchase, etc.)
	 * @return True if transaction is valid, false if hack detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateCurrencyTransaction(const FString& PlayerID, int32 Amount, const FString& Source);

	/**
	 * @brief Validates vehicle stats against data-driven limits
	 * @param VehicleID The vehicle to validate
	 * @param Speed Maximum speed stat
	 * @param Acceleration Acceleration stat
	 * @param Handling Handling stat
	 * @return True if stats are within bounds, false if modified
	 */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateVehicleStats(FName VehicleID, float Speed, float Acceleration, float Handling);

	// ==========================================
	// INTEGRITY CHECKS
	// ==========================================
	// These functions verify game file and memory integrity to detect tampering.
	// They are called periodically but can also be triggered manually.

	/**
	 * @brief Runs all integrity checks and returns comprehensive results
	 * @return Results structure with details on all checks performed
	 */
	UFUNCTION(BlueprintCallable, Category = "Integrity")
	FMGIntegrityCheckResult RunIntegrityCheck();

	/**
	 * @brief Verifies game files match expected checksums
	 * @return True if all critical files are unmodified
	 */
	UFUNCTION(BlueprintCallable, Category = "Integrity")
	bool CheckFileIntegrity();

	/**
	 * @brief Scans for memory modifications in critical game regions
	 * @return True if memory appears unmodified
	 */
	UFUNCTION(BlueprintCallable, Category = "Integrity")
	bool CheckMemoryIntegrity();

	/**
	 * @brief Detects if a debugger or code injector is attached
	 * @return True if debugging tools are detected (potential cheating)
	 */
	UFUNCTION(BlueprintCallable, Category = "Integrity")
	bool CheckForDebugger();

	/**
	 * @brief Detects if running inside a virtual machine
	 * @return True if VM detected (may indicate cheat development)
	 */
	UFUNCTION(BlueprintCallable, Category = "Integrity")
	bool CheckForVirtualMachine();

	/**
	 * @brief Verifies client time is synchronized with server
	 * @return True if time difference is within acceptable threshold
	 */
	UFUNCTION(BlueprintCallable, Category = "Integrity")
	bool VerifyTimeSync();

	// ==========================================
	// ANOMALY DETECTION
	// ==========================================
	// Statistical and behavioral analysis for detecting subtle cheating patterns
	// that may not trigger immediate threshold violations.

	/**
	 * @brief Manually report suspicious behavior detected by game systems
	 * @param PlayerID Player exhibiting the anomaly
	 * @param Type Classification of the anomaly
	 * @param Description Human-readable explanation
	 * @param Evidence Key-value data supporting the detection
	 */
	UFUNCTION(BlueprintCallable, Category = "Anomaly")
	void ReportAnomaly(const FString& PlayerID, EMGViolationType Type, const FString& Description,
		const TMap<FString, FString>& Evidence);

	/**
	 * @brief Checks if a stat value is statistically anomalous for this player
	 * @param PlayerID Player to check
	 * @param StatType Type of stat (e.g., "win_rate", "avg_speed")
	 * @param Value Current stat value
	 * @return True if the value is a statistical outlier
	 */
	UFUNCTION(BlueprintCallable, Category = "Anomaly")
	bool CheckStatisticalAnomaly(const FString& PlayerID, const FString& StatType, float Value);

	/**
	 * @brief Analyzes input patterns for inhuman behavior (bots, macros)
	 * @param InputHistory Recent input samples (stick positions, timings)
	 * @return True if input appears automated or inhuman
	 */
	UFUNCTION(BlueprintCallable, Category = "Anomaly")
	bool DetectInputAnomaly(const TArray<FVector2D>& InputHistory);

	// ==========================================
	// PLAYER REPORTS
	// ==========================================
	// Community-driven reporting system allowing players to flag suspicious behavior.
	// Reports are rate-limited to prevent abuse.

	/**
	 * @brief Submit a player report for suspected cheating
	 * @param ReportedPlayerID The player being reported
	 * @param Reason Category of the report (e.g., "Speed Hack")
	 * @param Description Additional details from the reporter
	 * @return True if report was submitted, false if on cooldown or invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "Reports")
	bool ReportPlayer(const FString& ReportedPlayerID, const FString& Reason, const FString& Description);

	/**
	 * @brief Gets all reports filed against a specific player
	 * @param PlayerID The player to look up
	 * @return Array of reports against this player
	 */
	UFUNCTION(BlueprintPure, Category = "Reports")
	TArray<FMGPlayerReport> GetPlayerReports(const FString& PlayerID) const;

	/**
	 * @brief Gets the total number of reports against a player
	 * @param PlayerID The player to look up
	 * @return Number of reports filed
	 */
	UFUNCTION(BlueprintPure, Category = "Reports")
	int32 GetReportCount(const FString& PlayerID) const;

	/**
	 * @brief Checks if the local player can submit a report (cooldown check)
	 * @param PlayerID The player to potentially report
	 * @return True if a report can be submitted, false if on cooldown
	 */
	UFUNCTION(BlueprintPure, Category = "Reports")
	bool CanReportPlayer(const FString& PlayerID) const;

	// ==========================================
	// TRUST SYSTEM
	// ==========================================

	/** Get player trust level */
	UFUNCTION(BlueprintPure, Category = "Trust")
	EMGTrustLevel GetPlayerTrustLevel(const FString& PlayerID) const;

	/** Get local player trust level */
	UFUNCTION(BlueprintPure, Category = "Trust")
	EMGTrustLevel GetLocalTrustLevel() const { return LocalTrustLevel; }

	/** Increase trust score */
	UFUNCTION(BlueprintCallable, Category = "Trust")
	void IncreaseTrustScore(const FString& PlayerID, int32 Amount);

	/** Decrease trust score */
	UFUNCTION(BlueprintCallable, Category = "Trust")
	void DecreaseTrustScore(const FString& PlayerID, int32 Amount);

	/** Get trust score */
	UFUNCTION(BlueprintPure, Category = "Trust")
	int32 GetTrustScore(const FString& PlayerID) const;

	// ==========================================
	// VIOLATION MANAGEMENT
	// ==========================================

	/** Get violation history */
	UFUNCTION(BlueprintPure, Category = "Violations")
	TArray<FMGViolationRecord> GetViolationHistory(const FString& PlayerID) const;

	/** Get violation count */
	UFUNCTION(BlueprintPure, Category = "Violations")
	int32 GetViolationCount(const FString& PlayerID, EMGViolationType Type = EMGViolationType::Unknown) const;

	/** Clear old violations */
	UFUNCTION(BlueprintCallable, Category = "Violations")
	void ClearOldViolations(int32 DaysOld = 30);

	// ==========================================
	// BAN MANAGEMENT
	// ==========================================

	/** Check if player is banned */
	UFUNCTION(BlueprintPure, Category = "Ban")
	bool IsPlayerBanned(const FString& PlayerID) const;

	/** Get ban reason */
	UFUNCTION(BlueprintPure, Category = "Ban")
	FString GetBanReason(const FString& PlayerID) const;

	/** Get ban expiry */
	UFUNCTION(BlueprintCallable, Category = "Ban")
	FDateTime GetBanExpiry(const FString& PlayerID) const;

	/** Is local player banned */
	UFUNCTION(BlueprintPure, Category = "Ban")
	bool IsLocalPlayerBanned() const { return bIsLocalBanned; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set validation thresholds */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void SetValidationThresholds(const FMGValidationThresholds& Thresholds);

	/** Get validation thresholds */
	UFUNCTION(BlueprintPure, Category = "Config")
	FMGValidationThresholds GetValidationThresholds() const { return ValidationThresholds; }

	/** Enable/disable anti-cheat */
	UFUNCTION(BlueprintCallable, Category = "Config")
	void SetAntiCheatEnabled(bool bEnabled);

	/** Is anti-cheat enabled */
	UFUNCTION(BlueprintPure, Category = "Config")
	bool IsAntiCheatEnabled() const { return bAntiCheatEnabled; }

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Anti-cheat enabled */
	UPROPERTY()
	bool bAntiCheatEnabled = true;

	/** Validation thresholds */
	UPROPERTY()
	FMGValidationThresholds ValidationThresholds;

	/** Violation records */
	UPROPERTY()
	TArray<FMGViolationRecord> ViolationRecords;

	/** Player reports */
	UPROPERTY()
	TArray<FMGPlayerReport> PlayerReports;

	/** Trust scores */
	UPROPERTY()
	TMap<FString, int32> TrustScores;

	/** Ban list */
	UPROPERTY()
	TMap<FString, FDateTime> BanList;

	/** Ban reasons */
	UPROPERTY()
	TMap<FString, FString> BanReasons;

	/** Local player trust level */
	UPROPERTY()
	EMGTrustLevel LocalTrustLevel = EMGTrustLevel::Normal;

	/** Local player banned */
	UPROPERTY()
	bool bIsLocalBanned = false;

	/** Local player ID */
	UPROPERTY()
	FString LocalPlayerID;

	/** Last report times (for cooldown) */
	UPROPERTY()
	TMap<FString, FDateTime> LastReportTimes;

	/** Report cooldown seconds */
	UPROPERTY()
	float ReportCooldownSeconds = 300.0f;

	/** Last positions for teleport detection */
	UPROPERTY()
	TMap<FString, FVector> LastPlayerPositions;

	/** File checksums */
	UPROPERTY()
	TMap<FString, FString> ExpectedFileChecksums;

	/** Server time offset */
	UPROPERTY()
	float ServerTimeOffset = 0.0f;

	/** Timer for periodic checks */
	FTimerHandle IntegrityCheckTimerHandle;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize expected checksums */
	void InitializeChecksums();

	/** Record violation */
	void RecordViolation(const FString& PlayerID, EMGViolationType Type, EMGViolationSeverity Severity,
		const FString& Description, const TMap<FString, FString>& Evidence);

	/** Update trust level */
	void UpdateTrustLevel(const FString& PlayerID);

	/** Get trust level from score */
	EMGTrustLevel GetTrustLevelFromScore(int32 Score) const;

	/** Apply automatic penalty */
	void ApplyAutomaticPenalty(const FString& PlayerID, const FMGViolationRecord& Violation);

	/** Report to server */
	void ReportToServer(const FMGViolationRecord& Violation);

	/** Periodic integrity check */
	void PeriodicIntegrityCheck();

	/** Calculate file checksum */
	FString CalculateFileChecksum(const FString& FilePath) const;
};
