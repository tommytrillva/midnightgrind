// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGServerAuthSubsystem.h
 * Server-Authoritative Validation System for Midnight Grind
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the Server Authentication Subsystem, which is the game's
 * primary defense against cheating by ensuring all important game actions are
 * validated by the server before being accepted. Think of it as a "trust but
 * verify" system - the client (player's game) can play normally, but the server
 * double-checks everything important.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. SERVER-AUTHORITATIVE ARCHITECTURE:
 *    In multiplayer games, we can't trust the player's computer (client) because
 *    hackers can modify it. Instead, the server is the "source of truth" - it
 *    makes final decisions about what's valid. This subsystem sends data to
 *    the server for validation.
 *
 * 2. GAME INSTANCE SUBSYSTEM:
 *    This class inherits from UGameInstanceSubsystem, meaning it:
 *    - Lives for the entire game session (not just one level)
 *    - Is automatically created when the game starts
 *    - Can be accessed from anywhere using: GameInstance->GetSubsystem<UMGServerAuthSubsystem>()
 *
 * 3. REQUEST/RESPONSE PATTERN:
 *    The subsystem sends "requests" to the server and receives "responses":
 *    - FMGServerRequest: Data we send to the server (race results, purchases, etc.)
 *    - FMGServerResponse: The server's answer (valid, invalid, suspicious, etc.)
 *
 * 4. RATE LIMITING:
 *    To prevent abuse (like spamming the server), we track how many requests
 *    a player can make in a time period. This prevents denial-of-service attacks.
 *
 * 5. CRYPTOGRAPHIC SIGNING:
 *    Requests are "signed" with a secret key, creating a digital signature.
 *    This proves the request came from our game and wasn't tampered with.
 *    Like signing a letter to prove you wrote it.
 *
 * HOW IT FITS IN THE ARCHITECTURE:
 * --------------------------------
 *
 *   [Player's Game (Client)]
 *          |
 *          v
 *   [MGServerAuthSubsystem] -- Signs & sends requests -->  [Backend Server]
 *          |                                                      |
 *          |  <-- Receives validation responses ------------------+
 *          v
 *   [MGAntiCheatSubsystem] -- Works together for cheat detection
 *
 * The ServerAuth subsystem works closely with:
 * - MGAntiCheatSubsystem: Client-side detection, ServerAuth is server-side validation
 * - Backend Server: The actual server that validates requests (not in this codebase)
 * - Race System: Submits race results for validation
 * - Economy System: Validates all currency transactions
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. Player finishes a race
 * 2. RaceManager calls SubmitRaceResult() with the results
 * 3. ServerAuth signs the data and sends it to the backend
 * 4. Server validates (checks if times are possible, rewards are correct, etc.)
 * 5. Server responds with Valid/Invalid/Suspicious
 * 6. Game applies rewards only if Valid, or flags account if Suspicious
 *
 * SECURITY FEATURES:
 * ------------------
 * - Request signing: Prevents request forgery
 * - Timestamp validation: Prevents replay attacks (reusing old valid requests)
 * - Sequence numbers: Ensures requests are processed in order
 * - Rate limiting: Prevents server abuse
 * - Session tokens: Ensures requests come from authenticated players
 * - Checksums: Detect data corruption or tampering
 *
 * @see UMGAntiCheatSubsystem - Client-side cheat detection (works with this system)
 * @see FMGServerRequest - The request structure sent to the server
 * @see FMGServerResponse - The response structure received from the server
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "MGServerAuthSubsystem.generated.h"

// =============================================================================
// ENUMERATIONS
// =============================================================================

/**
 * Types of requests that can be sent to the server for validation.
 *
 * Each type has different validation rules on the server side. For example,
 * RaceResult submissions are checked against track records and physics limits,
 * while CurrencyTransaction validates against the player's balance.
 */

UENUM(BlueprintType)
enum class EMGServerRequestType : uint8
{
	RaceResult,				///< Race completion data (times, positions, rewards)
	CurrencyTransaction,	///< Any change to player's in-game currency
	VehiclePurchase,		///< Buying a new vehicle from the shop
	UpgradePurchase,		///< Buying upgrades/parts for vehicles
	ReplaySubmission,		///< Ghost/replay data for verification or sharing
	LeaderboardSubmission,	///< High score/best time submission
	ProgressionUpdate,		///< Level ups, unlocks, career progress
	AchievementUnlock,		///< Trophy/achievement completion
	Custom					///< Catch-all for other validated actions
};

/**
 * Results returned by the server after validating a request.
 *
 * These tell us whether the server accepted our request and why it might
 * have been rejected. Always check this before applying any results locally.
 */
UENUM(BlueprintType)
enum class EMGValidationResult : uint8
{
	Valid,				///< Request accepted - safe to apply results
	Invalid_Signature,	///< Signature mismatch - possible tampering or wrong key
	Invalid_Timestamp,	///< Request too old or in future - possible replay attack
	Invalid_Data,		///< Data fails validation (impossible stats, etc.)
	Invalid_Session,	///< Session expired or invalid - need to re-authenticate
	RateLimited,		///< Too many requests - wait before retrying
	ServerError,		///< Server-side error - retry later
	Suspicious,			///< Data looks suspicious - flagged for review
	Banned				///< Player is banned - reject and show ban notice
};

// =============================================================================
// DATA STRUCTURES
// =============================================================================

/**
 * A request sent to the backend server for validation.
 *
 * This structure contains everything needed for the server to validate
 * a game action. The signature proves authenticity, the timestamp prevents
 * replay attacks, and the sequence number ensures proper ordering.
 *
 * SECURITY FIELDS EXPLAINED:
 * - Signature: HMAC hash of the payload using a secret key. If anyone modifies
 *   the payload, the signature won't match and the server rejects it.
 * - Timestamp: When the request was created. Requests too old are rejected to
 *   prevent "replay attacks" (re-sending captured valid requests).
 * - SequenceNumber: Must be greater than the last accepted number. Prevents
 *   duplicate submissions and ensures order.
 * - SessionToken: Proves the player is authenticated. Like a concert ticket.
 */
USTRUCT(BlueprintType)
struct FMGServerRequest
{
	GENERATED_BODY()

	/// Unique ID for tracking this specific request through the system
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequestID;

	/// What kind of action this request represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGServerRequestType Type = EMGServerRequestType::Custom;

	/// The actual data being validated (JSON format typically)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Payload;

	/// Cryptographic signature to prove this request is authentic
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Signature;

	/// When this request was created (for replay attack prevention)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/// Sequential number to ensure request ordering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SequenceNumber = 0;

	/// Token proving the player is authenticated
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionToken;
};

/**
 * Response received from the backend server after validation.
 *
 * Contains the server's decision about whether to accept the request,
 * along with any additional data the server wants to send back.
 * The server's sequence number and timestamp can be used to verify
 * response authenticity and synchronize client time.
 */
USTRUCT(BlueprintType)
struct FMGServerResponse
{
	GENERATED_BODY()

	/// Matches the RequestID of the request this responds to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequestID;

	/// Whether the server accepted or rejected the request
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGValidationResult Result = EMGValidationResult::Valid;

	/// Human-readable message (useful for debugging/logging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	/// Any data the server sends back (e.g., corrected values, new state)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResponseData;

	/// Server's sequence number for response verification
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ServerSequenceNumber = 0;

	/// Server's current time (for time sync)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ServerTimestamp;
};

/**
 * Complete race result data submitted for server validation.
 *
 * This is one of the most important structures in the anti-cheat system.
 * It contains not just the final results, but also telemetry data (position
 * samples, speed samples) that the server can analyze to detect cheating.
 *
 * WHY SO MUCH DATA?
 * The server can reconstruct and verify the race by checking:
 * - Are lap times consistent with position/speed samples?
 * - Do speeds ever exceed vehicle capabilities?
 * - Does the path follow the track layout?
 * - Does the replay hash match the submitted telemetry?
 *
 * PI (Performance Index): A single number representing total vehicle
 * performance. Used to ensure players used legal vehicle configurations.
 */
USTRUCT(BlueprintType)
struct FMGRaceResultSubmission
{
	GENERATED_BODY()

	/// Unique identifier for this specific race instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RaceID;

	/// The multiplayer/online session this race occurred in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	/// Which track was raced (e.g., "Tokyo_Circuit")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/// Game mode (e.g., "Sprint", "Circuit", "Drift")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	/// Finishing position (1 = first place, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalPosition = 0;

	/// Total time from start to finish
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceTimeSeconds = 0.0f;

	/// Fastest single lap time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTimeSeconds = 0.0f;

	/// Time for each individual lap (for multi-lap races)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> LapTimes;

	/// Which vehicle was used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	/// Performance Index of the vehicle configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclePI = 0;

	/// Sampled world positions during the race (for path verification)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> PositionSamples;

	/// Sampled speeds during the race (for physics verification)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SpeedSamples;

	/// Hash of the replay file (to verify replay matches results)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayHash;

	/// Integrity checksum of all the above data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Checksum = 0;
};

/**
 * Economy transaction data submitted for server validation.
 *
 * Every currency change (earning, spending, etc.) is validated by the server.
 * This prevents players from hacking infinite money or duplicating items.
 * The server maintains the authoritative balance and rejects invalid transactions.
 *
 * DOUBLE-ENTRY BOOKKEEPING:
 * We send both BalanceBefore and BalanceAfter so the server can verify:
 * - The client's understanding of current balance is correct
 * - The math adds up (BalanceAfter = BalanceBefore + Amount)
 * - The transaction makes sense (can't spend more than you have)
 */
USTRUCT(BlueprintType)
struct FMGTransactionSubmission
{
	GENERATED_BODY()

	/// Unique identifier for this transaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionID;

	/// Type of transaction: "Earn", "Spend", "Refund", etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionType;

	/// Which currency: "Cash", "PremiumCredits", "XP", etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrencyType;

	/// Amount being transacted (positive for earn, negative for spend)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Amount = 0;

	/// If purchasing something, what item (empty for earnings)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemID;

	/// Where the currency came from/went to (e.g., "RaceReward", "ShopPurchase")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Source;

	/// Player's balance before this transaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BalanceBefore = 0;

	/// Player's balance after this transaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BalanceAfter = 0;

	/// Integrity checksum of this transaction data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Checksum = 0;
};

/**
 * Information about rate limiting for a specific request type.
 *
 * Rate limiting prevents abuse by restricting how many requests can be
 * made in a time period. Different request types may have different limits
 * (e.g., race results might be limited to 60/hour, while shop purchases
 * might be limited to 10/minute).
 *
 * LEAKY BUCKET ALGORITHM:
 * The system uses a "leaky bucket" style rate limit:
 * - You start with a full bucket of tokens (RequestLimit)
 * - Each request uses one token (decrements RequestsRemaining)
 * - Tokens refill over time until the bucket is full again
 * - When empty, requests are rejected until tokens refill
 */
USTRUCT(BlueprintType)
struct FMGRateLimitInfo
{
	GENERATED_BODY()

	/// Which type of request this limit applies to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGServerRequestType RequestType = EMGServerRequestType::Custom;

	/// How many more requests can be made before hitting the limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequestsRemaining = 100;

	/// Maximum requests allowed in the time window
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequestLimit = 100;

	/// When the limit resets (requests remaining goes back to limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ResetTime;

	/// If rate limited, how many seconds until you can retry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RetryAfterSeconds = 0.0f;
};

// =============================================================================
// DELEGATES (Events)
// =============================================================================
// These are Unreal's event system. Other parts of the code can "subscribe"
// to these delegates and get notified when something happens. For example,
// the UI might subscribe to OnRateLimited to show a "please wait" message.

/// Fires when any server validation completes (success or failure)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnServerValidationComplete, const FString&, RequestID, EMGValidationResult, Result);

/// Fires specifically when a race result validation completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRaceResultValidated, const FString&, RaceID, bool, bValid);

/// Fires specifically when a transaction validation completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTransactionValidated, const FString&, TransactionID, bool, bValid);

/// Fires when we hit a rate limit (useful for showing "slow down" UI)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRateLimited, EMGServerRequestType, RequestType);

/// Fires when the server flags something as suspicious (may lead to investigation)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSuspiciousActivity, const FString&, Reason);

// =============================================================================
// SERVER AUTH SUBSYSTEM CLASS
// =============================================================================

/**
 * Core subsystem for server-authoritative game validation.
 *
 * This subsystem handles all communication with the backend server for
 * validating game actions. It ensures that race results, purchases, and
 * other important actions are verified by a trusted server before being
 * accepted.
 *
 * USAGE EXAMPLE:
 * @code
 * // Get the subsystem
 * UMGServerAuthSubsystem* ServerAuth = GameInstance->GetSubsystem<UMGServerAuthSubsystem>();
 *
 * // After a race ends, submit results for validation
 * FMGRaceResultSubmission Result;
 * Result.RaceID = CurrentRaceID;
 * Result.TrackID = "Tokyo_Circuit";
 * Result.RaceTimeSeconds = 125.5f;
 * // ... fill in other fields ...
 *
 * ServerAuth->SubmitRaceResult(Result);
 *
 * // Listen for the result
 * ServerAuth->OnRaceResultValidated.AddDynamic(this, &AMyClass::HandleRaceValidated);
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGServerAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Called automatically when the game instance is created
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called automatically when the game is shutting down
	virtual void Deinitialize() override;

	// ==========================================
	// SESSION MANAGEMENT
	// ==========================================
	// A "session" represents an authenticated connection to the server.
	// You must have a valid session before submitting any requests.

	/**
	 * Starts a new session with the backend server.
	 * @param AuthToken Token received from the authentication system (login)
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Session")
	void InitializeSession(const FString& AuthToken);

	/** Ends the current session (call on logout or disconnect) */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Session")
	void EndSession();

	/** Returns true if we have an active, valid session */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|Session")
	bool HasValidSession() const { return !CurrentSessionToken.IsEmpty(); }

	/** Gets the current session token (for debugging/logging) */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|Session")
	FString GetSessionToken() const { return CurrentSessionToken; }

	// ==========================================
	// RACE RESULT SUBMISSION
	// ==========================================
	// These functions handle submitting race results to the server.
	// Call StartRaceTracking when a race begins, record data during the race,
	// then call SubmitRaceResult when the race ends.

	/**
	 * Submits completed race results for server validation.
	 * Call this when a race finishes. Rewards are only granted if server validates.
	 * @param Result Complete race result data including telemetry
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void SubmitRaceResult(const FMGRaceResultSubmission& Result);

	/**
	 * Starts tracking data for a new race. Call when race countdown begins.
	 * @param RaceID Unique identifier for this race
	 * @param TrackID Which track is being raced
	 * @param GameModeID What game mode (Sprint, Circuit, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void StartRaceTracking(const FString& RaceID, FName TrackID, FName GameModeID);

	/**
	 * Records a checkpoint crossing during the race.
	 * @param CheckpointIndex Which checkpoint (0, 1, 2, etc.)
	 * @param Time Time elapsed when crossing this checkpoint
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void RecordRaceCheckpoint(int32 CheckpointIndex, float Time);

	/**
	 * Records a position/speed sample during the race. Call periodically.
	 * @param Position Current world position of the vehicle
	 * @param Speed Current speed in km/h
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void RecordPositionSample(FVector Position, float Speed);

	// ==========================================
	// TRANSACTION VALIDATION
	// ==========================================
	// All economy operations must be validated by the server.
	// This prevents infinite money exploits and purchase fraud.

	/**
	 * Submits a transaction for server validation.
	 * @param Transaction Complete transaction details
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Transaction")
	void SubmitTransaction(const FMGTransactionSubmission& Transaction);

	/**
	 * Validates a purchase before completing it.
	 * Call before deducting currency to ensure the purchase is legitimate.
	 * @param ItemID What is being purchased
	 * @param Price How much it costs
	 * @param CurrencyType Which currency is being spent
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Transaction")
	void ValidatePurchase(const FString& ItemID, int64 Price, const FString& CurrencyType);

	/**
	 * Validates currency being granted to the player.
	 * Call when giving rewards, race winnings, etc.
	 * @param Source Where the currency is coming from (e.g., "RaceReward")
	 * @param Amount How much is being granted
	 * @param CurrencyType Which currency is being granted
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Transaction")
	void ValidateCurrencyGrant(const FString& Source, int64 Amount, const FString& CurrencyType);

	// ==========================================
	// REPLAY VERIFICATION
	// ==========================================
	// Replays can be submitted for verification to prove a time/score is legitimate.
	// This is especially important for leaderboard entries.

	/**
	 * Submits a replay file for server-side verification.
	 * The server can simulate the replay to verify the results are achievable.
	 * @param RaceID Which race this replay is from
	 * @param ReplayData Raw replay file data
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Replay")
	void SubmitReplayForVerification(const FString& RaceID, const TArray<uint8>& ReplayData);

	/**
	 * Calculates a hash of replay data for integrity checking.
	 * @param ReplayData Raw replay file data
	 * @return Hash string that uniquely identifies this replay
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Replay")
	FString CalculateReplayHash(const TArray<uint8>& ReplayData) const;

	// ==========================================
	// LEADERBOARD SUBMISSION
	// ==========================================

	/**
	 * Submits a score/time to a leaderboard with proof.
	 * @param LeaderboardID Which leaderboard to submit to
	 * @param Score The score or time (in appropriate units for the leaderboard)
	 * @param ReplayHash Hash of the replay that proves this score
	 */
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Leaderboard")
	void SubmitLeaderboardEntry(FName LeaderboardID, int64 Score, const FString& ReplayHash);

	// ==========================================
	// RATE LIMITING
	// ==========================================
	// Check these before making requests to avoid getting rate limited.

	/**
	 * Checks if we can make a request of the given type without being rate limited.
	 * @param Type The type of request to check
	 * @return True if the request can be made, false if rate limited
	 */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|RateLimit")
	bool CanMakeRequest(EMGServerRequestType Type) const;

	/**
	 * Gets current rate limit status for a request type.
	 * @param Type The type of request to check
	 * @return Current rate limit information
	 */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|RateLimit")
	FMGRateLimitInfo GetRateLimitInfo(EMGServerRequestType Type) const;

	/**
	 * Gets seconds until rate limit resets for a request type.
	 * @param Type The type of request to check
	 * @return Seconds until limit resets (0 if not limited)
	 */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|RateLimit")
	float GetTimeUntilReset(EMGServerRequestType Type) const;

	// ==========================================
	// VALIDATION HELPERS
	// ==========================================
	// Low-level functions for creating and validating secure requests.

	/**
	 * Calculates a checksum for data integrity verification.
	 * @param Data String to calculate checksum for
	 * @return Integer checksum value
	 */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|Validation")
	int32 CalculateChecksum(const FString& Data) const;

	/**
	 * Creates a cryptographic signature for a request payload.
	 * @param Payload The data to sign
	 * @return Signature string to include with the request
	 */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|Validation")
	FString SignRequest(const FString& Payload) const;

	/**
	 * Verifies that a server response is authentic and untampered.
	 * @param Response The response to validate
	 * @return True if response is valid and authentic
	 */
	UFUNCTION(BlueprintPure, Category = "ServerAuth|Validation")
	bool ValidateServerResponse(const FMGServerResponse& Response) const;

	// ==========================================
	// EVENTS (Delegates)
	// ==========================================
	// Subscribe to these to react when server operations complete.
	// Example: OnRaceResultValidated.AddDynamic(this, &MyClass::HandleValidation);

	/// Fires when any server validation request completes
	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnServerValidationComplete OnServerValidationComplete;

	/// Fires when a race result validation completes
	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnRaceResultValidated OnRaceResultValidated;

	/// Fires when a transaction validation completes
	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnTransactionValidated OnTransactionValidated;

	/// Fires when a request is rate limited
	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnRateLimited OnRateLimited;

	/// Fires when server detects suspicious activity
	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnSuspiciousActivity OnSuspiciousActivity;

protected:
	// ==========================================
	// INTERNAL FUNCTIONS
	// ==========================================
	// These are implementation details - you don't call these directly.

	/** Sends a request to the backend server via HTTP */
	void SendRequest(const FMGServerRequest& Request);

	/** Callback when HTTP response is received */
	void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/** Updates rate limit tracking after a request */
	void UpdateRateLimits(EMGServerRequestType Type);

	/** Processes queued requests that were waiting for rate limits */
	void ProcessPendingRequests();

	/** Generates a unique ID for a new request */
	FString GenerateRequestID() const;

	/** Checks if a timestamp is within acceptable range */
	bool ValidateTimestamp(const FDateTime& Timestamp) const;

	/** Analyzes request patterns for suspicious behavior */
	void DetectSuspiciousPatterns();

private:
	// ==========================================
	// INTERNAL DATA
	// ==========================================

	/** Current authentication session token */
	FString CurrentSessionToken;

	/** Secret key used for signing requests (do NOT expose this) */
	FString ServerSecretKey;

	/** URL of the backend validation server */
	FString BackendURL;

	/** Queue of requests waiting to be sent (used when rate limited) */
	TArray<FMGServerRequest> PendingRequests;

	/** Current rate limit status for each request type */
	TMap<EMGServerRequestType, FMGRateLimitInfo> RateLimits;

	/** Timestamps of recent requests (for pattern detection) */
	TMap<FString, FDateTime> RecentRequests;

	/** Timer for processing queued requests */
	FTimerHandle ProcessQueueHandle;

	/** Next sequence number to use for requests */
	int32 CurrentSequenceNumber = 0;

	/** Maximum allowed time difference between client and server (seconds) */
	float MaxTimestampDrift = 300.0f; // 5 minutes

	/** Maximum requests that can be queued while rate limited */
	int32 MaxPendingRequests = 50;

	// ==========================================
	// RACE TRACKING DATA
	// ==========================================
	// These are populated during a race via the tracking functions.

	/** ID of the race currently being tracked */
	FString CurrentRaceID;

	/** Track being raced */
	FName CurrentTrackID;

	/** Game mode being played */
	FName CurrentGameModeID;

	/** Times at each checkpoint crossing */
	TArray<float> CheckpointTimes;

	/** Sampled positions during the race */
	TArray<FVector> PositionHistory;

	/** Sampled speeds during the race */
	TArray<float> SpeedHistory;
};
