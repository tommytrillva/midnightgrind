// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGPinkSlipTypes.h
 * @brief Common types and utilities for the Pink Slip race system
 *
 * Pink slip races are the UNIQUE SELLING POINT of Midnight Grind.
 * This file contains shared enumerations and utility functions used
 * across the pink slip subsystem.
 *
 * CRITICAL DESIGN CONSTRAINTS (Per GDD Section 4.3 and Design Pillar 3):
 *
 * 1. NO RETRIES: Once a pink slip race ends, the result is FINAL.
 *    There is no "restart race" or "try again" option.
 *
 * 2. PERMANENT TRANSFER: Vehicle ownership changes are irreversible.
 *    The save system commits immediately after transfer.
 *
 * 3. NO EXPLOITATION: Disconnecting, quitting, or technical issues
 *    result in automatic loss for the responsible party.
 *
 * 4. TRIPLE CONFIRMATION: Players must confirm 3 times before racing.
 *    This is not optional and cannot be bypassed.
 *
 * 5. ELIGIBILITY GATES: Players must meet REP tier, level, and vehicle
 *    count requirements. These cannot be bypassed.
 */

#include "CoreMinimal.h"
#include "MGPinkSlipTypes.generated.h"

/**
 * @brief Result of a pink slip race
 *
 * Used to communicate final race outcome. Once set, cannot be changed.
 */
UENUM(BlueprintType)
enum class EMGPinkSlipResult : uint8
{
	/** Race not yet complete */
	Pending,

	/** Participant 0 (challenger) won */
	ChallengerWon,

	/** Participant 1 (defender) won */
	DefenderWon,

	/** Race voided due to technical issues - both keep cars */
	Voided,

	/** Challenger disconnected - defender wins */
	ChallengerDisconnected,

	/** Defender disconnected - challenger wins */
	DefenderDisconnected
};

/**
 * @brief Reasons a pink slip race may be voided
 *
 * Voiding is EXTREMELY RARE and only for true technical failures.
 * Any player-caused issue results in a loss, not a void.
 */
UENUM(BlueprintType)
enum class EMGPinkSlipVoidReason : uint8
{
	/** Not voided */
	None,

	/** Server crash during race */
	ServerCrash,

	/** Both players disconnected simultaneously */
	MutualDisconnect,

	/** Admin intervention (cheating detected) */
	AdminIntervention,

	/** Critical game bug affecting outcome */
	CriticalBug
};

/**
 * @brief Utility class with static helper functions
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPinkSlipHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Check if a result indicates a winner
	 * @param Result The race result
	 * @return true if there is a definitive winner
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	static bool HasWinner(EMGPinkSlipResult Result)
	{
		return Result == EMGPinkSlipResult::ChallengerWon ||
			   Result == EMGPinkSlipResult::DefenderWon ||
			   Result == EMGPinkSlipResult::ChallengerDisconnected ||
			   Result == EMGPinkSlipResult::DefenderDisconnected;
	}

	/**
	 * @brief Check if result was due to disconnect
	 * @param Result The race result
	 * @return true if someone disconnected
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	static bool WasDisconnect(EMGPinkSlipResult Result)
	{
		return Result == EMGPinkSlipResult::ChallengerDisconnected ||
			   Result == EMGPinkSlipResult::DefenderDisconnected;
	}

	/**
	 * @brief Get winner index from result
	 * @param Result The race result
	 * @return 0 for challenger, 1 for defender, -1 if no winner
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	static int32 GetWinnerIndex(EMGPinkSlipResult Result)
	{
		switch (Result)
		{
		case EMGPinkSlipResult::ChallengerWon:
		case EMGPinkSlipResult::DefenderDisconnected:
			return 0;
		case EMGPinkSlipResult::DefenderWon:
		case EMGPinkSlipResult::ChallengerDisconnected:
			return 1;
		default:
			return -1;
		}
	}

	/**
	 * @brief Get loser index from result
	 * @param Result The race result
	 * @return 0 for challenger, 1 for defender, -1 if no loser
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	static int32 GetLoserIndex(EMGPinkSlipResult Result)
	{
		const int32 Winner = GetWinnerIndex(Result);
		return Winner >= 0 ? (1 - Winner) : -1;
	}

	/**
	 * @brief Check if result allows retry
	 *
	 * ALWAYS RETURNS FALSE. Pink slip races NEVER allow retries.
	 * This function exists for code clarity and to document the intent.
	 *
	 * @param Result The race result (unused - always returns false)
	 * @return Always false - no retries in pink slip
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip", meta = (DisplayName = "Allows Retry (Always False)"))
	static bool AllowsRetry(EMGPinkSlipResult Result)
	{
		// CRITICAL: Pink slip races NEVER allow retries
		// This is a core design pillar - loss is permanent and meaningful
		return false;
	}

	/**
	 * @brief Get human-readable result message
	 * @param Result The race result
	 * @param bForWinner true if message is for winner, false for loser
	 * @return Localized message
	 */
	UFUNCTION(BlueprintPure, Category = "Pink Slip")
	static FText GetResultMessage(EMGPinkSlipResult Result, bool bForWinner)
	{
		if (Result == EMGPinkSlipResult::Voided)
		{
			return NSLOCTEXT("PinkSlip", "Voided", "Race voided - both vehicles returned");
		}

		if (Result == EMGPinkSlipResult::Pending)
		{
			return NSLOCTEXT("PinkSlip", "Pending", "Race in progress...");
		}

		if (bForWinner)
		{
			if (WasDisconnect(Result))
			{
				return NSLOCTEXT("PinkSlip", "WinByDisconnect",
					"Your opponent disconnected. Victory is yours, and so are the keys.");
			}
			return NSLOCTEXT("PinkSlip", "Victory",
				"VICTORY. The keys are yours. Drive it like you earned it.");
		}
		else
		{
			if (WasDisconnect(Result))
			{
				return NSLOCTEXT("PinkSlip", "LossByDisconnect",
					"Connection lost. The race is forfeit. Hand over the keys.");
			}
			return NSLOCTEXT("PinkSlip", "Defeat",
				"DEFEAT. Your keys now belong to someone else. No take-backs.");
		}
	}
};

/**
 * @brief Configuration constants for pink slip system
 *
 * These values are the default configuration. Subsystem properties
 * can override these, but these represent the designed baseline.
 */
namespace MGPinkSlipConstants
{
	/** Minimum vehicles required to participate (must have backup) */
	constexpr int32 MinVehiclesToParticipate = 2;

	/** Minimum REP tier for pink slip racing (per GDD: Tier 3 = RESPECTED) */
	constexpr int32 MinREPTier = 3;

	/** Minimum player level for pink slips */
	constexpr int32 MinPlayerLevel = 20;

	/** Maximum PI difference between vehicles */
	constexpr int32 MaxPIDifference = 50;

	/** Cooldown after loss in hours */
	constexpr int32 CooldownHours = 24;

	/** Trade lock for won vehicles in days */
	constexpr int32 TradeLockDays = 7;

	/** Number of confirmations required */
	constexpr int32 RequiredConfirmations = 3;

	/** Disconnect grace period in seconds */
	constexpr float DisconnectGracePeriod = 30.0f;

	/** Photo finish threshold in seconds */
	constexpr float PhotoFinishThreshold = 0.5f;

	/** Rematch offer window in seconds */
	constexpr float RematchWindowSeconds = 120.0f;

	/** Maximum spectators/witnesses */
	constexpr int32 MaxWitnesses = 50;

	/** REP tier thresholds (per GDD Section 4.2) */
	constexpr int32 REPTierThresholds[] = { 0, 1000, 5000, 15000, 35000, 75000 };
}
