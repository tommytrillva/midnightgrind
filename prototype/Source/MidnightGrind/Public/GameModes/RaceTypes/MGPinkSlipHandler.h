// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGPinkSlipHandler.generated.h"

/**
 * Pink slip race state
 */
UENUM(BlueprintType)
enum class EMGPinkSlipState : uint8
{
	/** Waiting for both to confirm */
	WaitingConfirmation,
	/** Verifying requirements */
	Verification,
	/** Race active */
	Racing,
	/** Race complete, processing transfer */
	ProcessingTransfer,
	/** Transfer complete */
	Complete,
	/** Race voided (technical issues) */
	Voided
};

/**
 * Pink slip verification status
 */
UENUM(BlueprintType)
enum class EMGPinkSlipVerification : uint8
{
	/** Not yet verified */
	Pending,
	/** Verification passed */
	Passed,
	/** PI mismatch */
	PIOutOfRange,
	/** Player doesn't have required REP */
	InsufficientREP,
	/** Vehicle is player's only car */
	OnlyVehicle,
	/** Vehicle has trade lock */
	TradeLocked,
	/** Account not in good standing */
	AccountRestricted,
	/** Recent disconnects */
	DisconnectPenalty,
	/** Already in pink slip cooldown */
	OnCooldown
};

/**
 * Pink slip participant data
 */
USTRUCT(BlueprintType)
struct FMGPinkSlipParticipant
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Vehicle ID wagered */
	UPROPERTY(BlueprintReadOnly)
	FGuid VehicleID;

	/** Vehicle display name */
	UPROPERTY(BlueprintReadOnly)
	FText VehicleName;

	/** Vehicle PI */
	UPROPERTY(BlueprintReadOnly)
	int32 VehiclePI = 0;

	/** Estimated vehicle value */
	UPROPERTY(BlueprintReadOnly)
	int64 EstimatedValue = 0;

	/** Has confirmed wager */
	UPROPERTY(BlueprintReadOnly)
	bool bConfirmed = false;

	/** Verification status */
	UPROPERTY(BlueprintReadOnly)
	EMGPinkSlipVerification VerificationStatus = EMGPinkSlipVerification::Pending;

	/** Is AI opponent */
	UPROPERTY(BlueprintReadOnly)
	bool bIsAI = false;

	/** Is challenger (initiated race) */
	UPROPERTY(BlueprintReadOnly)
	bool bIsChallenger = false;
};

/**
 * Pink slip transfer record
 */
USTRUCT(BlueprintType)
struct FMGPinkSlipTransfer
{
	GENERATED_BODY()

	/** Transfer ID */
	UPROPERTY(BlueprintReadOnly)
	FGuid TransferID;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Winner player ID */
	UPROPERTY(BlueprintReadOnly)
	FString WinnerID;

	/** Loser player ID */
	UPROPERTY(BlueprintReadOnly)
	FString LoserID;

	/** Vehicle transferred */
	UPROPERTY(BlueprintReadOnly)
	FGuid VehicleID;

	/** Vehicle display name */
	UPROPERTY(BlueprintReadOnly)
	FText VehicleName;

	/** Vehicle value at time of transfer */
	UPROPERTY(BlueprintReadOnly)
	int64 VehicleValue = 0;

	/** Race type */
	UPROPERTY(BlueprintReadOnly)
	FName RaceType;

	/** Was AI opponent */
	UPROPERTY(BlueprintReadOnly)
	bool bWasAgainstAI = false;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPinkSlipStateChanged, EMGPinkSlipState, OldState, EMGPinkSlipState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPinkSlipVerified, int32, ParticipantIndex, EMGPinkSlipVerification, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkSlipConfirmed, int32, ParticipantIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPinkSlipTransferComplete, const FMGPinkSlipTransfer&, Transfer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPinkSlipDisconnect, int32, ParticipantIndex, bool, bIsLoss);

/**
 * Pink Slip Race Handler
 * Implements high-stakes title racing as per PRD Section 4.3
 *
 * Stakes: Both vehicles wagered
 * Requirements: REP tier + PI matching
 * Anti-Quit: Disconnect = loss
 * Outcome: Winner takes loser's car
 *
 * Features:
 * - Triple confirmation before start
 * - REP and PI verification
 * - Anti-disconnect protection
 * - Permanent vehicle transfer
 * - Race history recording
 * - Cooldown enforcement
 */
UCLASS(Blueprintable, BlueprintType)
class MIDNIGHTGRIND_API UMGPinkSlipHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGPinkSlipHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE
	// ==========================================

	virtual void InitializeRace(const FMGRaceConfiguration& Config) override;
	virtual void StartRace() override;
	virtual void UpdateRace(float DeltaTime) override;
	virtual void EndRace() override;
	virtual bool IsRaceComplete() const override;
	virtual TArray<FMGRaceResult> GetResults() const override;
	virtual FText GetRaceTypeName() const override;

	// ==========================================
	// PINK SLIP SETUP
	// ==========================================

	/**
	 * Set challenger (initiator)
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Setup")
	void SetChallenger(const FString& PlayerID, FGuid VehicleID);

	/**
	 * Set defender (challenged)
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Setup")
	void SetDefender(const FString& PlayerID, FGuid VehicleID);

	/**
	 * Confirm wager for participant
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Setup")
	void ConfirmWager(int32 ParticipantIndex);

	/**
	 * Verify participant requirements
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Setup")
	EMGPinkSlipVerification VerifyParticipant(int32 ParticipantIndex);

	/**
	 * Verify both participants
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Setup")
	bool VerifyBoth();

	/**
	 * Set inner race handler (actual race type)
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Setup")
	void SetInnerRaceHandler(UMGRaceTypeHandler* Handler);

	// ==========================================
	// PINK SLIP STATE
	// ==========================================

	/**
	 * Get current state
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|State")
	EMGPinkSlipState GetPinkSlipState() const { return CurrentState; }

	/**
	 * Get participant data
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|State")
	FMGPinkSlipParticipant GetParticipant(int32 Index) const;

	/**
	 * Are both confirmed
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|State")
	bool AreBothConfirmed() const;

	/**
	 * Are both verified
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|State")
	bool AreBothVerified() const;

	/**
	 * Get winner index (-1 if not yet determined)
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|State")
	int32 GetWinnerIndex() const { return WinnerIndex; }

	// ==========================================
	// DISCONNECT HANDLING
	// ==========================================

	/**
	 * Report participant disconnect
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Disconnect")
	void ReportDisconnect(int32 ParticipantIndex);

	/**
	 * Report participant reconnect
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Disconnect")
	void ReportReconnect(int32 ParticipantIndex);

	/**
	 * Get disconnect grace period remaining
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Disconnect")
	float GetDisconnectGracePeriodRemaining() const { return DisconnectGraceRemaining; }

	// ==========================================
	// TRANSFER
	// ==========================================

	/**
	 * Get transfer record (after completion)
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip|Transfer")
	FMGPinkSlipTransfer GetTransferRecord() const { return TransferRecord; }

	/**
	 * Void the race (technical issues)
	 */
	UFUNCTION(BlueprintCallable, Category = "Pink Slip|Transfer")
	void VoidRace(FText Reason);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Maximum PI difference allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 MaxPIDifference = 50;

	/** Disconnect grace period (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float DisconnectGracePeriod = 30.0f;

	/** Cooldown after loss (hours) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 LossCooldownHours = 24;

	/** Trade lock duration for won vehicles (days) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	int32 WonVehicleTradeLockDays = 7;

	// ==========================================
	// EVENTS
	// ==========================================

	/** State changed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPinkSlipStateChanged OnStateChanged;

	/** Participant verified */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPinkSlipVerified OnVerified;

	/** Participant confirmed wager */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPinkSlipConfirmed OnConfirmed;

	/** Transfer complete */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPinkSlipTransferComplete OnTransferComplete;

	/** Disconnect detected */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPinkSlipDisconnect OnDisconnect;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Set state */
	void SetState(EMGPinkSlipState NewState);

	/** Process vehicle transfer */
	void ProcessTransfer();

	/** Apply cooldown to loser */
	void ApplyCooldown(const FString& PlayerID);

	/** Apply trade lock to won vehicle */
	void ApplyTradeLock(FGuid VehicleID);

	/** Update disconnect handling */
	void UpdateDisconnect(float DeltaTime);

	/** Record transfer history */
	void RecordTransfer();

private:
	/** Current state */
	EMGPinkSlipState CurrentState = EMGPinkSlipState::WaitingConfirmation;

	/** Participants (always 2) */
	FMGPinkSlipParticipant Participants[2];

	/** Inner race handler (actual race type being run) */
	UPROPERTY()
	TObjectPtr<UMGRaceTypeHandler> InnerRaceHandler;

	/** Winner index */
	int32 WinnerIndex = -1;

	/** Transfer record */
	FMGPinkSlipTransfer TransferRecord;

	/** Is race complete */
	bool bRaceComplete = false;

	/** Disconnected participant index (-1 if none) */
	int32 DisconnectedParticipant = -1;

	/** Disconnect grace period remaining */
	float DisconnectGraceRemaining = 0.0f;

	/** Void reason */
	FText VoidReason;
};
