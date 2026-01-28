// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGReportSubsystem.h
 * Player Reporting and Moderation System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Report Subsystem, which handles all player-to-player
 * reporting, moderation actions, muting, blocking, and content filtering.
 * It's the community management backbone that helps keep the game safe and
 * enjoyable for all players.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. PLAYER REPORTING:
 *    Players can report others for bad behavior (cheating, harassment, etc.).
 *    Reports are submitted to the server for review by moderators or automated
 *    systems. Categories help route reports to appropriate reviewers.
 *
 * 2. REPORT CATEGORIES (EMGReportCategory):
 *    Different types of reportable offenses:
 *    - Cheating/Hacking: Using external tools for unfair advantage
 *    - Griefing: Intentionally ruining others' gameplay
 *    - Harassment: Targeting players with abuse
 *    - HateSpeech: Discrimination, slurs, etc.
 *    - Boosting: Artificially inflating stats/ranks
 *    - Exploiting: Abusing bugs for advantage
 *
 * 3. PUNISHMENT SYSTEM (EMGPunishmentType):
 *    Actions taken against rule-breakers:
 *    - Warning: First offense, just a notice
 *    - TempMute/PermMute: Can't use voice/text chat
 *    - TempBan/PermBan: Can't play the game
 *    - CompetitiveBan: Can't play ranked modes
 *    - ShadowBan: Can play but matched only with other banned players
 *
 * 4. MUTING vs BLOCKING:
 *    - Muting: You can't hear/see their messages, but can still match with them
 *    - Blocking: You can't hear/see them AND won't be matched with them
 *    Both are local (only affects you) and reversible.
 *
 * 5. CONTENT FILTERING:
 *    Automatic text filtering that:
 *    - Replaces profanity with asterisks (****)
 *    - Blocks slurs and hate speech
 *    - Prevents spam patterns
 *    - Can be customized per-player
 *
 * 6. REPORT ACCURACY SCORE:
 *    Tracks how often a player's reports result in action. Players who
 *    frequently submit false reports may have their reports deprioritized.
 *    This prevents abuse of the reporting system.
 *
 * 7. DAILY REPORT LIMITS:
 *    Players can only submit a limited number of reports per day to prevent
 *    spam and ensure each report is meaningful.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Player Actions]
 *          |
 *          v
 *   [MGReportSubsystem]
 *          |
 *          +-- SubmitReport() ---------> [Backend Moderation Queue]
 *          +-- MutePlayer() -----------> [Local Mute List]
 *          +-- BlockPlayer() ----------> [Local Block List + Matchmaking]
 *          +-- FilterText() -----------> [Content Filter]
 *          |
 *          v
 *   [Backend Server]
 *          |
 *          +-- ReceivePunishment() ----> [Show ban screen, restrict access]
 *          +-- ReceiveReportFeedback() -> [Notify reporter of outcome]
 *
 * RELATIONSHIP WITH OTHER SUBSYSTEMS:
 * ------------------------------------
 * - MGAntiCheatSubsystem: Automated cheating detection supplements player reports
 * - Matchmaking: Uses block list to prevent matching blocked players
 * - Chat System: Uses mute list and content filter for messages
 * - UI: Displays report dialogs, punishment notices, filter settings
 *
 * TYPICAL WORKFLOWS:
 * ------------------
 *
 * Reporting a Player:
 * 1. Player opens report menu on another player
 * 2. Selects category (e.g., "Cheating") and writes description
 * 3. SubmitReport() sends to server with match context
 * 4. Server queues for review (automated or manual)
 * 5. Reporter later receives feedback via OnReportFeedbackReceived
 *
 * Receiving a Punishment:
 * 1. Server determines player violated rules
 * 2. ReceivePunishment() called with punishment details
 * 3. OnPunishmentReceived fires, UI shows ban/mute notice
 * 4. Subsystem restricts access based on punishment type
 * 5. Timer checks for expiration, OnPunishmentExpired when done
 *
 * IMPORTANT STRUCTURES:
 * ---------------------
 * - FMGPlayerReport: Full details of a player report
 * - FMGPunishment: Details of a punishment (ban, mute, etc.)
 * - FMGMutedPlayer: Info about a muted player
 * - FMGBlockedPlayer: Info about a blocked player
 * - FMGContentFilterConfig: Settings for text filtering
 *
 * @see EMGReportCategory - Types of reportable offenses
 * @see EMGPunishmentType - Types of punishments that can be applied
 * @see FMGPlayerReport - Structure containing report details
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGReportSubsystem.generated.h"

// =============================================================================
// ENUMERATIONS
// =============================================================================

/**
 * Categories of reportable player behavior.
 *
 * These categories help moderators quickly understand and route reports.
 * Each category may have different handling procedures and severity levels.
 * Choose the most specific category that applies when submitting a report.
 */

UENUM(BlueprintType)
enum class EMGReportCategory : uint8
{
	None,					///< No category selected (invalid report)
	Cheating,				///< Using cheats (speed hacks, aimbots, etc.)
	Hacking,				///< Exploiting security vulnerabilities
	Griefing,				///< Intentionally ruining others' experience
	Harassment,				///< Targeted abuse of specific players
	HateSpeech,				///< Discrimination, slurs, hate speech
	InappropriateName,		///< Offensive player name or clan tag
	InappropriateContent,	///< Offensive liveries, decals, or images
	Boosting,				///< Artificially inflating rank/stats with friends
	AFKAbuse,				///< Going AFK to farm rewards without playing
	Exploiting,				///< Abusing game bugs for advantage
	RealMoneyTrading,		///< Selling in-game items for real currency
	Impersonation,			///< Pretending to be devs, mods, or other players
	Spam,					///< Excessive repetitive messages or invites
	Other					///< Doesn't fit other categories
};

/**
 * Current status of a submitted report.
 *
 * Reports go through a lifecycle from submission to resolution.
 * The status tells the reporter what happened to their report.
 */
UENUM(BlueprintType)
enum class EMGReportStatus : uint8
{
	Pending,				///< Report received, waiting in queue
	UnderReview,			///< Moderator is currently reviewing
	ActionTaken,			///< Violation confirmed, punishment applied
	Dismissed,				///< Report reviewed but no violation found
	Duplicate,				///< Same player already reported for this
	InsufficientEvidence	///< Cannot verify without more evidence
};

/**
 * Severity level of a report or violation.
 *
 * Higher severity reports are prioritized for review and may result
 * in more severe punishments. Severity is sometimes auto-assigned
 * based on category (e.g., hate speech is usually High/Critical).
 */
UENUM(BlueprintType)
enum class EMGReportSeverity : uint8
{
	Low,		///< Minor issue, low priority (e.g., mild spam)
	Medium,		///< Standard issue, normal priority (e.g., griefing)
	High,		///< Serious issue, high priority (e.g., harassment)
	Critical	///< Urgent issue, immediate attention (e.g., hate speech, threats)
};

/**
 * Types of punishments that can be applied to rule-breakers.
 *
 * Punishments escalate with repeated offenses. First offense might be
 * a warning, while repeat offenders get bans. Permanent punishments
 * are reserved for severe violations or chronic offenders.
 */
UENUM(BlueprintType)
enum class EMGPunishmentType : uint8
{
	None,				///< No punishment (report dismissed)
	Warning,			///< Official warning, no restrictions
	TempMute,			///< Temporarily cannot use voice/text chat
	PermMute,			///< Permanently cannot use voice/text chat
	TempBan,			///< Temporarily cannot play the game
	PermBan,			///< Permanently banned from the game
	CompetitiveBan,		///< Cannot play ranked/competitive modes
	ResetProgress,		///< Player's progression is reset (for boosting)
	RemoveContent,		///< Inappropriate content removed (liveries, etc.)
	ShadowBan			///< Can play but only matched with other shadowbanned players
};

/**
 * Reasons why a player might be muted.
 *
 * Distinguishes between player-initiated mutes (I don't want to hear them)
 * and system-imposed mutes (they violated rules).
 */
UENUM(BlueprintType)
enum class EMGMuteReason : uint8
{
	None,			///< Not muted
	Manual,			///< Player manually muted them (personal choice)
	Reported,		///< Muted as result of report investigation
	AutoDetected,	///< Automatically muted by detection system
	ContentFilter	///< Muted because their messages trigger filters
};

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * Complete information about a player report.
 *
 * Contains everything needed to investigate a report: who reported whom,
 * what the complaint is, evidence, and context. This data is sent to
 * the moderation backend for review.
 *
 * EVIDENCE COLLECTION:
 * Good reports include evidence to help moderators make decisions:
 * - EvidenceURLs: Screenshots or video clips
 * - ReplayTimestamp: Specific moment in the replay to review
 * - ChatLogs: Relevant chat messages
 * - Metadata: Any other context (vehicle used, race position, etc.)
 */
USTRUCT(BlueprintType)
struct FMGPlayerReport
{
	GENERATED_BODY()

	/// Unique identifier for this report (used for tracking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReportID;

	/// ID of the player who submitted the report
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReporterPlayerID;

	/// Display name of the reporter (for moderator reference)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReporterPlayerName;

	/// ID of the player being reported
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReportedPlayerID;

	/// Display name of the reported player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReportedPlayerName;

	/// What type of offense is being reported
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportCategory Category = EMGReportCategory::None;

	/// Detailed description from the reporter
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// How serious the report is (affects priority)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportSeverity Severity = EMGReportSeverity::Medium;

	/// Current status in the review process
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportStatus Status = EMGReportStatus::Pending;

	/// When the report was submitted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReportedAt;

	/// When a moderator reviewed the report (if reviewed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReviewedAt;

	/// The match/race where the incident occurred
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MatchID;

	/// URLs to screenshots, videos, or other evidence
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EvidenceURLs;

	/// Timestamp in the replay to review (e.g., "02:34")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayTimestamp;

	/// Relevant chat messages captured around the incident
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ChatLogs;

	/// Additional context (key-value pairs for flexibility)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> Metadata;
};

/**
 * A punishment applied to a player for rule violations.
 *
 * Punishments are received from the server after moderation review.
 * The client displays appropriate notices and enforces restrictions.
 * Players can appeal punishments if they believe they were unjust.
 *
 * PUNISHMENT LIFECYCLE:
 * 1. Report is submitted and reviewed
 * 2. Moderator issues punishment
 * 3. Server sends FMGPunishment to client
 * 4. Client shows notice and enforces restrictions
 * 5. When ExpiresAt passes, punishment ends (unless permanent)
 * 6. Player can appeal, which flags bAppealed for re-review
 */
USTRUCT(BlueprintType)
struct FMGPunishment
{
	GENERATED_BODY()

	/// Unique identifier for this punishment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PunishmentID;

	/// ID of the player receiving the punishment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	/// What type of punishment was applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPunishmentType Type = EMGPunishmentType::None;

	/// What category of violation led to this punishment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReportCategory RelatedCategory = EMGReportCategory::None;

	/// Human-readable explanation shown to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Reason;

	/// When the punishment was applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime IssuedAt;

	/// When the punishment expires (ignored if bIsPermanent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	/// If true, this punishment never expires
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPermanent = false;

	/// If true, punishment is currently in effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	/// If true, player has submitted an appeal
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAppealed = false;

	/// The report that led to this punishment (for tracking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RelatedReportID;
};

/**
 * Information about a player that has been muted.
 *
 * Muting is a LOCAL action - it only affects what YOU see/hear.
 * The muted player doesn't know they've been muted. This is useful
 * for avoiding toxic players without confrontation.
 *
 * MUTE OPTIONS:
 * - Voice: Can't hear their voice chat
 * - Text: Can't see their text messages
 * - Emotes: Can't see their quick-chat emotes/reactions
 */
USTRUCT(BlueprintType)
struct FMGMutedPlayer
{
	GENERATED_BODY()

	/// ID of the muted player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	/// Display name (for the mute list UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	/// Why they were muted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMuteReason Reason = EMGMuteReason::Manual;

	/// When the mute was applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime MutedAt;

	/// If true, their voice chat is silenced
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuteVoice = true;

	/// If true, their text messages are hidden
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuteText = true;

	/// If true, their emotes/quick-chats are hidden
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideEmotes = false;
};

/**
 * Information about a player that has been blocked.
 *
 * Blocking is STRONGER than muting. Blocked players:
 * - Cannot communicate with you (implicit mute)
 * - Won't be matched with you in games (if bPreventMatching)
 * - Their shared content (liveries, etc.) is hidden from you
 *
 * Use blocking for players you never want to encounter again.
 */
USTRUCT(BlueprintType)
struct FMGBlockedPlayer
{
	GENERATED_BODY()

	/// ID of the blocked player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	/// Display name (for the block list UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	/// When the block was applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BlockedAt;

	/// If true, matchmaking will avoid putting you in the same game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPreventMatching = true;

	/// If true, their shared content (liveries, etc.) is hidden
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideContent = true;
};

/**
 * Feedback sent to players about reports they submitted.
 *
 * Players want to know their reports matter. This structure delivers
 * feedback after moderation review. For privacy, we don't disclose
 * exactly what punishment was applied, just whether action was taken.
 *
 * Example feedback messages:
 * - "Action was taken against the reported player. Thank you."
 * - "We reviewed your report but couldn't confirm a violation."
 * - "Thank you for your report. The player has been warned."
 */
USTRUCT(BlueprintType)
struct FMGReportFeedback
{
	GENERATED_BODY()

	/// Which report this feedback is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReportID;

	/// Whether moderation took action (punishment applied)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActionTaken = false;

	/// Message explaining the outcome
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FeedbackMessage;

	/// When this feedback was received
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReceivedAt;
};

/**
 * Statistics about a player's reporting history.
 *
 * Tracks both reports submitted (as a reporter) and received (as reported).
 * The accuracy score affects report priority - players who frequently
 * submit valid reports are prioritized, while false reporters are deprioritized.
 *
 * REPORT ACCURACY:
 * ReportAccuracyScore ranges from 0.0 to 1.0
 * - 1.0 = All reports resulted in action (perfect)
 * - 0.5 = Half of reports resulted in action (normal)
 * - 0.0 = No reports resulted in action (may be false reporting)
 */
USTRUCT(BlueprintType)
struct FMGReportingStats
{
	GENERATED_BODY()

	/// How many reports this player has submitted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReportsSubmitted = 0;

	/// How many of their reports led to punishment
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReportsResultingInAction = 0;

	/// How many times this player has been reported by others
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReportsReceived = 0;

	/// How many punishments this player has received
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PunishmentsReceived = 0;

	/// Ratio of accurate reports (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReportAccuracyScore = 1.0f;

	/// How many players this player has blocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayersBlocked = 0;

	/// How many players this player has muted
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayersMuted = 0;
};

/**
 * Configuration for the automatic text content filter.
 *
 * The content filter automatically censors inappropriate text in chat.
 * Players can customize their filter settings based on preference.
 * Some filtering (like slurs) may be mandatory and not configurable.
 *
 * FILTER LEVELS:
 * - 0: Off (only mandatory filtering)
 * - 1: Light (obvious profanity only)
 * - 2: Standard (most profanity, default)
 * - 3: Strict (aggressive filtering, may have false positives)
 */
USTRUCT(BlueprintType)
struct FMGContentFilterConfig
{
	GENERATED_BODY()

	/// Filter common profanity/swear words
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterProfanity = true;

	/// Filter hate speech and slurs (usually mandatory, can't disable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterSlurs = true;

	/// Filter spam patterns (repeated characters, caps spam)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterSpam = true;

	/// Filter URLs/links (prevents phishing)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFilterLinks = false;

	/// Overall filter aggressiveness (0=off, 1=light, 2=standard, 3=strict)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FilterLevel = 2;

	/// Custom words this player wants blocked (in addition to defaults)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CustomBlockedWords;

	/// Words to allow that might otherwise be filtered (false positives)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedWords;
};

// =============================================================================
// DELEGATES (Events)
// =============================================================================
// Subscribe to these to react when report-related events occur.
// Example: OnPunishmentReceived.AddDynamic(this, &MyClass::ShowBanScreen);

/// Fires when a report is successfully submitted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportSubmitted, FGuid, ReportID);

/// Fires when feedback is received about a previously submitted report
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReportFeedbackReceived, const FMGReportFeedback&, Feedback);

/// Fires when this player receives a punishment (show ban/mute notice)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPunishmentReceived, const FMGPunishment&, Punishment);

/// Fires when a temporary punishment expires (can play again)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPunishmentExpired, const FMGPunishment&, Punishment);

/// Fires when a player is muted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerMuted, const FMGMutedPlayer&, MutedPlayer);

/// Fires when a player is unmuted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUnmuted, FName, PlayerID);

/// Fires when a player is blocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerBlocked, const FMGBlockedPlayer&, BlockedPlayer);

/// Fires when a player is unblocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerUnblocked, FName, PlayerID);

// =============================================================================
// REPORT SUBSYSTEM CLASS
// =============================================================================

/**
 * Comprehensive player reporting and moderation system.
 *
 * This subsystem handles all aspects of community moderation:
 * - Submitting reports against other players
 * - Muting and blocking players locally
 * - Receiving and enforcing punishments
 * - Content filtering for chat
 * - Tracking reporting statistics
 *
 * USAGE EXAMPLES:
 * @code
 * // Get the subsystem
 * UMGReportSubsystem* Report = GameInstance->GetSubsystem<UMGReportSubsystem>();
 *
 * // Report a player for cheating
 * FGuid ReportID = Report->SubmitReport(
 *     TEXT("PlayerID123"),
 *     EMGReportCategory::Cheating,
 *     NSLOCTEXT("", "", "Player was going twice the max speed")
 * );
 *
 * // Mute an annoying player
 * Report->MutePlayer(TEXT("PlayerID456"), true, true); // voice and text
 *
 * // Block a toxic player
 * Report->BlockPlayer(TEXT("PlayerID789"), true); // prevent matching
 *
 * // Filter user input before displaying
 * FString SafeMessage = Report->FilterText(UserInput);
 *
 * // Check if current player is banned
 * if (Report->IsBanned())
 * {
 *     // Show ban screen with reason and time remaining
 *     float TimeLeft = Report->GetBanTimeRemaining();
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGReportSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Called automatically when the game instance is created
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called automatically when the game is shutting down
	virtual void Deinitialize() override;

	/// Determines if this subsystem should be created (always true for this subsystem)
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// REPORT SUBMISSION
	// ==========================================
	// Functions for submitting reports against other players.

	/**
	 * Submits a basic report against another player.
	 * @param ReportedPlayerID ID of the player to report
	 * @param Category Type of offense being reported
	 * @param Description Detailed explanation of what happened
	 * @param MatchID Optional: the match where it occurred
	 * @return Unique ID for tracking this report
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Submit")
	FGuid SubmitReport(FName ReportedPlayerID, EMGReportCategory Category, const FText& Description, FName MatchID = NAME_None);

	/**
	 * Submits a report with attached evidence (screenshots, video links).
	 * @param ReportedPlayerID ID of the player to report
	 * @param Category Type of offense being reported
	 * @param Description Detailed explanation of what happened
	 * @param EvidenceURLs URLs to screenshots or video evidence
	 * @param MatchID Optional: the match where it occurred
	 * @return Unique ID for tracking this report
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Submit")
	FGuid SubmitReportWithEvidence(FName ReportedPlayerID, EMGReportCategory Category, const FText& Description, const TArray<FString>& EvidenceURLs, FName MatchID = NAME_None);

	/**
	 * Checks if the current player can report a specific player.
	 * May return false if: already reported today, on cooldown, or blocked.
	 * @param PlayerID The player to potentially report
	 * @return True if report can be submitted
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	bool CanReportPlayer(FName PlayerID) const;

	/**
	 * Gets how many reports the player can still submit today.
	 * @return Number of remaining reports (0 = at limit)
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	int32 GetRemainingReportsToday() const;

	/**
	 * Gets all reports this player has submitted.
	 * @return Array of submitted reports
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	TArray<FMGPlayerReport> GetSubmittedReports() const;

	/**
	 * Gets a specific report by ID.
	 * @param ReportID The report to retrieve
	 * @return The report data (check ReportID.IsValid() for success)
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Submit")
	FMGPlayerReport GetReport(FGuid ReportID) const;

	// ==========================================
	// REPORT CATEGORIES
	// ==========================================
	// Helper functions for the report UI.

	/**
	 * Gets all available report categories for the UI.
	 * @return Array of category enum values
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Categories")
	TArray<EMGReportCategory> GetAvailableCategories() const;

	/**
	 * Gets the localized display name for a category.
	 * @param Category The category to get the name for
	 * @return Localized name (e.g., "Cheating", "Harassment")
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Categories")
	FText GetCategoryDisplayName(EMGReportCategory Category) const;

	/**
	 * Gets a description explaining what the category covers.
	 * @param Category The category to describe
	 * @return Localized description for the report UI
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Categories")
	FText GetCategoryDescription(EMGReportCategory Category) const;

	// ==========================================
	// MUTING
	// ==========================================
	// Muting silences a player's communications but still allows matching.
	// Use this for players who are annoying but not worth blocking.

	/**
	 * Mutes a player (hides their communications from you).
	 * @param PlayerID The player to mute
	 * @param bMuteVoice If true, silence their voice chat
	 * @param bMuteText If true, hide their text messages
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void MutePlayer(FName PlayerID, bool bMuteVoice = true, bool bMuteText = true);

	/**
	 * Unmutes a player (restores their communications).
	 * @param PlayerID The player to unmute
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void UnmutePlayer(FName PlayerID);

	/**
	 * Checks if a player is currently muted.
	 * @param PlayerID The player to check
	 * @return True if muted
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Mute")
	bool IsPlayerMuted(FName PlayerID) const;

	/**
	 * Gets detailed mute info for a muted player.
	 * @param PlayerID The player to look up
	 * @return Mute info (check PlayerID.IsValid() for success)
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Mute")
	FMGMutedPlayer GetMutedPlayerInfo(FName PlayerID) const;

	/**
	 * Gets all currently muted players.
	 * @return Array of muted player info
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Mute")
	TArray<FMGMutedPlayer> GetMutedPlayers() const;

	/**
	 * Mutes all players (except friends/party members typically).
	 * @param bMuteVoice If true, silence all voice chat
	 * @param bMuteText If true, hide all text messages
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void MuteAll(bool bMuteVoice = true, bool bMuteText = true);

	/**
	 * Removes the "mute all" setting (individual mutes remain).
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Mute")
	void UnmuteAll();

	// ==========================================
	// BLOCKING
	// ==========================================
	// Blocking is stronger than muting - blocked players won't be
	// matched with you (if enabled) and all their content is hidden.

	/**
	 * Blocks a player (prevents communication and optionally matching).
	 * @param PlayerID The player to block
	 * @param bPreventMatching If true, matchmaking will avoid this player
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Block")
	void BlockPlayer(FName PlayerID, bool bPreventMatching = true);

	/**
	 * Unblocks a player (restores normal interaction).
	 * @param PlayerID The player to unblock
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Block")
	void UnblockPlayer(FName PlayerID);

	/**
	 * Checks if a player is currently blocked.
	 * @param PlayerID The player to check
	 * @return True if blocked
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Block")
	bool IsPlayerBlocked(FName PlayerID) const;

	/**
	 * Gets all currently blocked players.
	 * @return Array of blocked player info
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Block")
	TArray<FMGBlockedPlayer> GetBlockedPlayers() const;

	/**
	 * Checks if matchmaking should avoid a specific player.
	 * @param PlayerID The player to check
	 * @return True if matchmaking should avoid them
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Block")
	bool ShouldPreventMatching(FName PlayerID) const;

	// ==========================================
	// PUNISHMENTS
	// ==========================================
	// Punishments are received from the server when this player
	// violates rules. Use these to check status and display notices.

	/**
	 * Checks if the current player has any active punishment.
	 * @return True if any punishment is currently in effect
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	bool HasActivePunishment() const;

	/**
	 * Gets all punishments currently in effect.
	 * @return Array of active punishments
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	TArray<FMGPunishment> GetActivePunishments() const;

	/**
	 * Gets a specific punishment by ID.
	 * @param PunishmentID The punishment to retrieve
	 * @return The punishment data
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	FMGPunishment GetPunishment(FGuid PunishmentID) const;

	/**
	 * Checks if the current player is banned (cannot play).
	 * @return True if player has an active ban
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	bool IsBanned() const;

	/**
	 * Checks if the current player is muted (cannot communicate).
	 * @return True if player has an active mute punishment
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	bool IsMuted() const;

	/**
	 * Gets when the current ban expires.
	 * @return Expiration time (check IsBanned() first)
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	FDateTime GetBanExpirationTime() const;

	/**
	 * Gets seconds remaining on the current ban.
	 * @return Seconds until ban expires (0 if not banned or permanent)
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Punishment")
	float GetBanTimeRemaining() const;

	/**
	 * Submits an appeal for a punishment.
	 * @param PunishmentID Which punishment to appeal
	 * @param AppealReason Why the player believes the punishment is unjust
	 * @return True if appeal was submitted successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Punishment")
	bool AppealPunishment(FGuid PunishmentID, const FText& AppealReason);

	// ==========================================
	// CONTENT FILTERING
	// ==========================================
	// Automatic filtering of inappropriate text content.
	// Use these to sanitize user input before display.

	/**
	 * Filters inappropriate content from text.
	 * Replaces filtered words with asterisks (e.g., "f***").
	 * @param InputText The text to filter
	 * @return Filtered text safe for display
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	FString FilterText(const FString& InputText) const;

	/**
	 * Checks if text contains content that would be filtered.
	 * Useful for validation before submission.
	 * @param Text The text to check
	 * @return True if text contains filtered content
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Filter")
	bool ContainsFilteredContent(const FString& Text) const;

	/**
	 * Updates the content filter configuration.
	 * @param Config New filter settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	void SetContentFilterConfig(const FMGContentFilterConfig& Config);

	/**
	 * Gets current content filter configuration.
	 * @return Current filter settings
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Filter")
	FMGContentFilterConfig GetContentFilterConfig() const { return FilterConfig; }

	/**
	 * Adds a custom word to the block list.
	 * @param Word Word to block
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	void AddCustomBlockedWord(const FString& Word);

	/**
	 * Removes a word from the custom block list.
	 * @param Word Word to remove from blocking
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Filter")
	void RemoveCustomBlockedWord(const FString& Word);

	// ==========================================
	// FEEDBACK
	// ==========================================
	// Feedback about submitted reports.

	/**
	 * Called when feedback is received from the server about a report.
	 * @param Feedback The feedback data
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Feedback")
	void ReceiveReportFeedback(const FMGReportFeedback& Feedback);

	/**
	 * Gets recent feedback about submitted reports.
	 * @param MaxEntries Maximum number of entries to return
	 * @return Array of feedback, newest first
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Feedback")
	TArray<FMGReportFeedback> GetRecentFeedback(int32 MaxEntries = 10) const;

	// ==========================================
	// STATS
	// ==========================================

	/**
	 * Gets reporting statistics for the current player.
	 * @return Statistics about reports submitted and received
	 */
	UFUNCTION(BlueprintPure, Category = "Report|Stats")
	FMGReportingStats GetReportingStats() const { return Stats; }

	// ==========================================
	// NETWORK (Called by Server)
	// ==========================================

	/**
	 * Called when a punishment is received from the server.
	 * Triggers OnPunishmentReceived delegate and enforces restrictions.
	 * @param Punishment The punishment details from the server
	 */
	UFUNCTION(BlueprintCallable, Category = "Report|Network")
	void ReceivePunishment(const FMGPunishment& Punishment);

	// ==========================================
	// EVENTS (Delegates)
	// ==========================================
	// Subscribe to these to react to report system events.

	/// Fires when a report is successfully submitted
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnReportSubmitted OnReportSubmitted;

	/// Fires when feedback about a report is received
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnReportFeedbackReceived OnReportFeedbackReceived;

	/// Fires when a punishment is received (show ban/mute screen)
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPunishmentReceived OnPunishmentReceived;

	/// Fires when a temporary punishment expires
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPunishmentExpired OnPunishmentExpired;

	/// Fires when a player is muted
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerMuted OnPlayerMuted;

	/// Fires when a player is unmuted
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerUnmuted OnPlayerUnmuted;

	/// Fires when a player is blocked
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerBlocked OnPlayerBlocked;

	/// Fires when a player is unblocked
	UPROPERTY(BlueprintAssignable, Category = "Report|Events")
	FOnPlayerUnblocked OnPlayerUnblocked;

protected:
	// ==========================================
	// INTERNAL FUNCTIONS
	// ==========================================

	/** Called periodically to process report-related tasks */
	void OnReportTick();

	/** Checks if any temporary punishments have expired */
	void CheckPunishmentExpiration();

	/** Checks if a word should be filtered */
	bool IsWordFiltered(const FString& Word) const;

	/** Saves report data to local storage (mute/block lists) */
	void SaveReportData();

	/** Loads report data from local storage */
	void LoadReportData();

	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Reports this player has submitted */
	UPROPERTY()
	TArray<FMGPlayerReport> SubmittedReports;

	/** Punishments currently in effect */
	UPROPERTY()
	TArray<FMGPunishment> ActivePunishments;

	/** Historical record of past punishments */
	UPROPERTY()
	TArray<FMGPunishment> PunishmentHistory;

	/** Players this player has muted (maps PlayerID -> MuteInfo) */
	UPROPERTY()
	TMap<FName, FMGMutedPlayer> MutedPlayers;

	/** Players this player has blocked (maps PlayerID -> BlockInfo) */
	UPROPERTY()
	TMap<FName, FMGBlockedPlayer> BlockedPlayers;

	/** Feedback received about submitted reports */
	UPROPERTY()
	TArray<FMGReportFeedback> ReportFeedback;

	/** Reporting statistics for this player */
	UPROPERTY()
	FMGReportingStats Stats;

	/** Current content filter settings */
	UPROPERTY()
	FMGContentFilterConfig FilterConfig;

	/** Base list of filtered words (loaded from config) */
	UPROPERTY()
	TArray<FString> BaseFilteredWords;

	/** Tracks how many times we've reported each player (for rate limiting) */
	UPROPERTY()
	TMap<FName, int32> ReportsPerPlayer;

	/** Maximum reports allowed per day */
	UPROPERTY()
	int32 DailyReportLimit = 10;

	/** Reports submitted so far today */
	UPROPERTY()
	int32 ReportsSubmittedToday = 0;

	/** When the daily report counter was last reset */
	UPROPERTY()
	FDateTime LastReportResetDate;

	/** Global flag: mute all voice chat */
	UPROPERTY()
	bool bMuteAllVoice = false;

	/** Global flag: mute all text chat */
	UPROPERTY()
	bool bMuteAllText = false;

	/** Timer handle for periodic report system tasks */
	FTimerHandle ReportTickHandle;
};
