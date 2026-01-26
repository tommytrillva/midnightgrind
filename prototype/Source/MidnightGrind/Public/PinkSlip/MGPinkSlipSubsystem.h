// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSaveSubsystem.h"
#include "MGPinkSlipSubsystem.generated.h"

class UMGGarageSubsystem;
class UMGSaveSubsystem;
class UMGWagerSubsystem;
class UMGPinkSlipHandler;

/**
 * @brief Pink slip race eligibility status
 *
 * Defines all possible reasons why a player may or may not be eligible
 * to participate in a pink slip race.
 */
UENUM(BlueprintType)
enum class EMGPinkSlipEligibility : uint8
{
	/** Player meets all requirements */
	Eligible,
	/** Player has only one vehicle - cannot risk losing it */
	OnlyVehicle,
	/** Vehicle is trade-locked from recent pink slip win */
	VehicleTradeLocked,
	/** Player's REP tier is too low */
	InsufficientREP,
	/** Player is on cooldown from recent loss */
	OnCooldown,
	/** Vehicle PI is out of range for opponent */
	PIOutOfRange,
	/** Player level too low for pink slips */
	LevelTooLow,
	/** Player has too many active wagers */
	TooManyActiveWagers,
	/** Account has restrictions */
	AccountRestricted,
	/** Recent disconnects penalty active */
	DisconnectPenalty
};

/**
 * @brief Pink slip cooldown record
 *
 * Tracks cooldown state after a pink slip loss to prevent
 * rage-betting behavior.
 */
USTRUCT(BlueprintType)
struct FMGPinkSlipCooldown
{
	GENERATED_BODY()

	/** When the cooldown expires */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime CooldownExpires;

	/** Loss that triggered cooldown */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid LossTransferID;

	/** Vehicle that was lost */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FText LostVehicleName;

	/** Is cooldown currently active? */
	bool IsActive() const
	{
		return FDateTime::UtcNow() < CooldownExpires;
	}

	/** Get remaining cooldown time */
	FTimespan GetRemainingTime() const
	{
		return IsActive() ? (CooldownExpires - FDateTime::UtcNow()) : FTimespan::Zero();
	}
};

/**
 * @brief Vehicle trade lock record
 *
 * Prevents immediate resale of vehicles won in pink slip races
 * to maintain the "earned through racing" philosophy.
 */
USTRUCT(BlueprintType)
struct FMGVehicleTradeLock
{
	GENERATED_BODY()

	/** The vehicle under trade lock */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid VehicleID;

	/** When the trade lock expires */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime LockExpires;

	/** Transfer that caused this lock */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid TransferID;

	/** Is lock currently active? */
	bool IsActive() const
	{
		return FDateTime::UtcNow() < LockExpires;
	}
};

/**
 * @brief Complete pink slip transfer record for history
 *
 * Permanent record of a pink slip race outcome stored in save data.
 * This is the "title" that changed hands.
 */
USTRUCT(BlueprintType)
struct FMGPinkSlipTransferRecord
{
	GENERATED_BODY()

	/** Unique transfer ID */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid TransferID;

	/** Timestamp of transfer */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime Timestamp;

	/** Was local player the winner? */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bLocalPlayerWon = false;

	/** Opponent name */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FString OpponentName;

	/** Opponent ID (for AI: "AI_<name>") */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FString OpponentID;

	/** Vehicle that was transferred */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FGuid VehicleID;

	/** Vehicle name at time of transfer */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FText VehicleName;

	/** Vehicle value at time of transfer */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int64 VehicleValue = 0;

	/** Performance index of transferred vehicle */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 VehiclePI = 0;

	/** Track where the race took place */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName TrackID;

	/** Race type (sprint, circuit, etc.) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName RaceType;

	/** Was opponent AI? */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bWasAgainstAI = false;

	/** Number of witnesses (online races) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 WitnessCount = 0;

	/** Winning margin in seconds (negative = photo finish) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float WinningMargin = 0.0f;
};

/**
 * @brief Confirmation dialog request data
 *
 * Data passed to UI for the mandatory confirmation dialogs
 * before a pink slip race can begin.
 */
USTRUCT(BlueprintType)
struct FMGPinkSlipConfirmationData
{
	GENERATED_BODY()

	/** Which confirmation step (1-3 for triple confirm) */
	UPROPERTY(BlueprintReadOnly)
	int32 ConfirmationStep = 1;

	/** Total confirmations required */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalConfirmations = 3;

	/** Player's wagered vehicle name */
	UPROPERTY(BlueprintReadOnly)
	FText PlayerVehicleName;

	/** Player's wagered vehicle value */
	UPROPERTY(BlueprintReadOnly)
	int64 PlayerVehicleValue = 0;

	/** Opponent's wagered vehicle name */
	UPROPERTY(BlueprintReadOnly)
	FText OpponentVehicleName;

	/** Opponent's wagered vehicle value */
	UPROPERTY(BlueprintReadOnly)
	int64 OpponentVehicleValue = 0;

	/** Track name */
	UPROPERTY(BlueprintReadOnly)
	FText TrackName;

	/** Total value at stake */
	UPROPERTY(BlueprintReadOnly)
	int64 TotalValueAtStake = 0;

	/** Warning message based on step */
	UPROPERTY(BlueprintReadOnly)
	FText WarningMessage;

	/** Is this the final confirmation? */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFinalConfirmation = false;
};

/**
 * @brief Delegate declarations for UI binding
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkSlipEligibilityChecked, EMGPinkSlipEligibility, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkSlipConfirmationRequired, const FMGPinkSlipConfirmationData&, ConfirmData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkSlipTransferExecuted, const FMGPinkSlipTransferRecord&, TransferRecord);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPinkSlipVehicleLost, const FGuid&, VehicleID, const FText&, VehicleName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPinkSlipVehicleWon, const FGuid&, VehicleID, const FText&, VehicleName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkSlipCooldownStarted, const FMGPinkSlipCooldown&, Cooldown);

/**
 * @class UMGPinkSlipSubsystem
 * @brief Core subsystem managing pink slip race mechanics
 *
 * Pink slip races are the UNIQUE SELLING POINT of Midnight Grind.
 * Losing means losing your vehicle FOREVER. No retries, no refunds.
 *
 * This subsystem handles:
 * - Eligibility verification (vehicle count, REP, cooldowns)
 * - Mandatory triple-confirmation before racing
 * - Permanent vehicle ownership transfer on loss
 * - Trade locks on won vehicles
 * - Cooldowns after losses to prevent rage-betting
 * - Complete transfer history for persistence
 *
 * Per GDD Section 4.3:
 * - Stakes: Both vehicles wagered
 * - Requirements: REP tier + PI matching
 * - Anti-Quit: Disconnect = loss
 * - Outcome: Winner takes loser's car
 *
 * @see UMGPinkSlipHandler for race-time mechanics
 * @see UMGWagerSubsystem for wager management
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGPinkSlipSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// ELIGIBILITY CHECKING
	// ==========================================

	/**
	 * @brief Check if player can participate in pink slip races
	 *
	 * Validates all requirements:
	 * - Must own more than one vehicle
	 * - Must meet minimum REP tier
	 * - Must not be on cooldown
	 * - Must meet level requirements
	 *
	 * @param VehicleID The vehicle player wants to wager
	 * @return Eligibility status
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Eligibility")
	EMGPinkSlipEligibility CheckEligibility(const FGuid& VehicleID) const;

	/**
	 * @brief Check if specific vehicle can be wagered
	 *
	 * Additional checks beyond general eligibility:
	 * - Vehicle not trade-locked
	 * - Vehicle not player's only car
	 * - Vehicle within PI range for opponent
	 *
	 * @param VehicleID The vehicle to check
	 * @param OpponentPI Opponent's vehicle PI (0 = skip PI check)
	 * @return Eligibility status
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Eligibility")
	EMGPinkSlipEligibility CheckVehicleEligibility(const FGuid& VehicleID, int32 OpponentPI = 0) const;

	/**
	 * @brief Get human-readable reason for ineligibility
	 * @param Status The eligibility status to describe
	 * @return Localized message explaining the status
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Eligibility")
	FText GetEligibilityMessage(EMGPinkSlipEligibility Status) const;

	/**
	 * @brief Check if player is currently on cooldown
	 * @return true if cooldown is active
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Eligibility")
	bool IsOnCooldown() const;

	/**
	 * @brief Get current cooldown info
	 * @return Cooldown data (check IsActive() on result)
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Eligibility")
	FMGPinkSlipCooldown GetCurrentCooldown() const { return ActiveCooldown; }

	/**
	 * @brief Check if a vehicle is trade-locked
	 * @param VehicleID The vehicle to check
	 * @return true if vehicle cannot be traded/wagered
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Eligibility")
	bool IsVehicleTradeLocked(const FGuid& VehicleID) const;

	/**
	 * @brief Get trade lock info for a vehicle
	 * @param VehicleID The vehicle to check
	 * @param OutLock The lock data if found
	 * @return true if lock exists
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Eligibility")
	bool GetVehicleTradeLock(const FGuid& VehicleID, FMGVehicleTradeLock& OutLock) const;

	// ==========================================
	// CONFIRMATION SYSTEM
	// ==========================================

	/**
	 * @brief Request confirmation for pink slip wager
	 *
	 * Initiates the triple-confirmation process required before
	 * a pink slip race can begin. UI should bind to
	 * OnPinkSlipConfirmationRequired.
	 *
	 * @param PlayerVehicleID Player's wagered vehicle
	 * @param OpponentVehicleID Opponent's wagered vehicle
	 * @param TrackID Track where race will occur
	 * @return true if confirmation process started
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Confirmation")
	bool RequestConfirmation(const FGuid& PlayerVehicleID, const FGuid& OpponentVehicleID, FName TrackID);

	/**
	 * @brief Submit confirmation response
	 *
	 * Called by UI when player confirms or cancels.
	 * If confirmed and more confirmations needed, broadcasts
	 * next OnPinkSlipConfirmationRequired.
	 *
	 * @param bConfirmed true if player confirmed
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Confirmation")
	void SubmitConfirmation(bool bConfirmed);

	/**
	 * @brief Cancel ongoing confirmation process
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Confirmation")
	void CancelConfirmation();

	/**
	 * @brief Check if confirmation process is complete
	 * @return true if all confirmations received
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Confirmation")
	bool IsConfirmationComplete() const { return CurrentConfirmationStep > RequiredConfirmations; }

	/**
	 * @brief Get current confirmation step
	 * @return Current step (1-3) or 0 if not in confirmation
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Confirmation")
	int32 GetCurrentConfirmationStep() const { return CurrentConfirmationStep; }

	// ==========================================
	// TRANSFER EXECUTION
	// ==========================================

	/**
	 * @brief Execute permanent vehicle transfer after race
	 *
	 * THIS IS THE POINT OF NO RETURN.
	 *
	 * Called by UMGPinkSlipHandler when race completes.
	 * Permanently transfers ownership from loser to winner.
	 *
	 * @param WinnerID Player/AI who won
	 * @param LoserID Player/AI who lost
	 * @param VehicleID Vehicle being transferred (loser's)
	 * @param TrackID Where race occurred
	 * @param RaceType Type of race
	 * @param WinningMargin Time difference in seconds
	 * @param WitnessCount Number of spectators (online)
	 * @return Transfer record
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Transfer")
	FMGPinkSlipTransferRecord ExecuteTransfer(
		const FString& WinnerID,
		const FString& LoserID,
		const FGuid& VehicleID,
		FName TrackID,
		FName RaceType,
		float WinningMargin,
		int32 WitnessCount
	);

	/**
	 * @brief Handle player losing their vehicle
	 *
	 * Internal method that:
	 * - Removes vehicle from garage
	 * - Starts cooldown
	 * - Updates save data
	 * - Broadcasts loss event
	 *
	 * @param VehicleID Vehicle being lost
	 * @param TransferID The transfer record ID
	 */
	void ProcessPlayerLoss(const FGuid& VehicleID, const FGuid& TransferID);

	/**
	 * @brief Handle player winning a vehicle
	 *
	 * Internal method that:
	 * - Adds vehicle to garage
	 * - Applies trade lock
	 * - Updates save data
	 * - Broadcasts win event
	 *
	 * @param VehicleData The vehicle data to add
	 * @param TransferID The transfer record ID
	 */
	void ProcessPlayerWin(const FMGSaveVehicleData& VehicleData, const FGuid& TransferID);

	// ==========================================
	// HISTORY & STATISTICS
	// ==========================================

	/**
	 * @brief Get pink slip transfer history
	 * @param MaxEntries Maximum entries to return (0 = all)
	 * @return Array of transfer records, newest first
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|History")
	TArray<FMGPinkSlipTransferRecord> GetTransferHistory(int32 MaxEntries = 50) const;

	/**
	 * @brief Get total vehicles won in pink slips
	 * @return Count of vehicles won
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|History")
	int32 GetTotalVehiclesWon() const;

	/**
	 * @brief Get total vehicles lost in pink slips
	 * @return Count of vehicles lost
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|History")
	int32 GetTotalVehiclesLost() const;

	/**
	 * @brief Get total value of vehicles won
	 * @return Sum of all won vehicle values
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|History")
	int64 GetTotalValueWon() const;

	/**
	 * @brief Get total value of vehicles lost
	 * @return Sum of all lost vehicle values
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|History")
	int64 GetTotalValueLost() const;

	/**
	 * @brief Get pink slip win/loss ratio
	 * @return Win rate as decimal (0.0-1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|History")
	float GetPinkSlipWinRate() const;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Cooldown duration after loss in hours */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 CooldownHours = 24;

	/** Trade lock duration for won vehicles in days */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 TradeLockDays = 7;

	/** Maximum PI difference for matchmaking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxPIDifference = 50;

	/** Minimum REP tier required for pink slips (per GDD: tier 3+) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MinREPTier = 3;

	/** Minimum player level for pink slips */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MinPlayerLevel = 20;

	/** Number of confirmations required (triple confirm = 3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 RequiredConfirmations = 3;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when eligibility is checked */
	UPROPERTY(BlueprintAssignable, Category = "Pink Slip|Events")
	FOnPinkSlipEligibilityChecked OnEligibilityChecked;

	/** Fired when confirmation dialog needed */
	UPROPERTY(BlueprintAssignable, Category = "Pink Slip|Events")
	FOnPinkSlipConfirmationRequired OnConfirmationRequired;

	/** Fired when transfer is executed */
	UPROPERTY(BlueprintAssignable, Category = "Pink Slip|Events")
	FOnPinkSlipTransferExecuted OnTransferExecuted;

	/** Fired when player loses a vehicle */
	UPROPERTY(BlueprintAssignable, Category = "Pink Slip|Events")
	FOnPinkSlipVehicleLost OnVehicleLost;

	/** Fired when player wins a vehicle */
	UPROPERTY(BlueprintAssignable, Category = "Pink Slip|Events")
	FOnPinkSlipVehicleWon OnVehicleWon;

	/** Fired when cooldown starts */
	UPROPERTY(BlueprintAssignable, Category = "Pink Slip|Events")
	FOnPinkSlipCooldownStarted OnCooldownStarted;

protected:
	// ==========================================
	// INTERNAL STATE
	// ==========================================

	/** Active cooldown (if any) */
	UPROPERTY(SaveGame)
	FMGPinkSlipCooldown ActiveCooldown;

	/** Active trade locks */
	UPROPERTY(SaveGame)
	TArray<FMGVehicleTradeLock> TradeLocks;

	/** Transfer history */
	UPROPERTY(SaveGame)
	TArray<FMGPinkSlipTransferRecord> TransferHistory;

	/** Statistics */
	UPROPERTY(SaveGame)
	int32 TotalWins = 0;

	UPROPERTY(SaveGame)
	int32 TotalLosses = 0;

	UPROPERTY(SaveGame)
	int64 TotalValueWon = 0;

	UPROPERTY(SaveGame)
	int64 TotalValueLost = 0;

	// ==========================================
	// CONFIRMATION STATE
	// ==========================================

	/** Current confirmation step (0 = not in confirmation) */
	int32 CurrentConfirmationStep = 0;

	/** Pending confirmation data */
	FMGPinkSlipConfirmationData PendingConfirmation;

	/** Player vehicle ID for pending confirmation */
	FGuid PendingPlayerVehicleID;

	/** Opponent vehicle ID for pending confirmation */
	FGuid PendingOpponentVehicleID;

	/** Track ID for pending confirmation */
	FName PendingTrackID;

	// ==========================================
	// HELPER METHODS
	// ==========================================

	/** Get garage subsystem */
	UMGGarageSubsystem* GetGarageSubsystem() const;

	/** Get save subsystem */
	UMGSaveSubsystem* GetSaveSubsystem() const;

	/** Get wager subsystem */
	UMGWagerSubsystem* GetWagerSubsystem() const;

	/** Calculate REP tier from total REP */
	int32 GetREPTier(int32 TotalREP) const;

	/** Clean up expired trade locks */
	void CleanupExpiredTradeLocks();

	/** Add trade lock for vehicle */
	void AddTradeLock(const FGuid& VehicleID, const FGuid& TransferID);

	/** Start cooldown for player */
	void StartCooldown(const FGuid& TransferID, const FText& VehicleName);

	/** Build confirmation data for current step */
	FMGPinkSlipConfirmationData BuildConfirmationData(int32 Step) const;

	/** Get warning message for confirmation step */
	FText GetConfirmationWarning(int32 Step) const;

	/** Save pink slip data to save system */
	void SavePinkSlipData();

	/** Load pink slip data from save system */
	void LoadPinkSlipData();

	/** Add record to history */
	void AddToHistory(const FMGPinkSlipTransferRecord& Record);
};
