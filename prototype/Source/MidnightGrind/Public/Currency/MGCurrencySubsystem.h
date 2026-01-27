// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCurrencySubsystem.h
 * @brief Fair and player-friendly currency management system for Midnight Grind.
 *
 * This subsystem implements a generous economy designed to respect players:
 * - **Grind Cash**: Primary currency earned abundantly through ALL gameplay activities
 * - **Neon Credits**: Premium currency that is BOTH earnable through gameplay AND purchasable
 * - **Crew Tokens**: Earned exclusively through cooperative crew activities
 * - **Season Points**: Earned through battle pass and seasonal progression
 * - **Legacy Marks**: Earned through story milestones and achievements
 *
 * @section philosophy Design Philosophy
 * This system is built on the principle that players should feel rewarded, not
 * punished. Key design decisions:
 * - NO gameplay advantages for purchase - premium currency is cosmetics only
 * - Premium currency (Neon Credits) CAN be earned through normal gameplay
 * - Generous daily/weekly bonuses that reward consistent play without punishing absence
 * - Multipliers are EARNED through gameplay, never purchased
 * - Race rewards scale with difficulty and player performance
 *
 * @section usage Basic Usage Example
 * @code
 * UMGCurrencySubsystem* Currency = GetGameInstance()->GetSubsystem<UMGCurrencySubsystem>();
 *
 * // Award race earnings
 * FMGRaceEarnings Earnings = Currency->CalculateRaceEarnings(1, 8, TrackID, true, false, true);
 * Currency->AwardRaceEarnings(Earnings);
 *
 * // Check daily bonus
 * if (Currency->CanClaimDailyBonus())
 * {
 *     Currency->ClaimDailyBonus();
 * }
 * @endcode
 *
 * @see UMGShopSubsystem For spending currency on items
 * @see UMGReputationSubsystem For reputation-based progression
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCurrencySubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Types of currency in the Midnight Grind economy.
 *
 * Each currency serves a distinct purpose and is earned through
 * different activities, encouraging varied gameplay.
 */
UENUM(BlueprintType)
enum class EMGCurrencyType : uint8
{
	/** Primary earnable currency - abundant and used for most purchases */
	GrindCash,
	/** Premium currency - earnable through gameplay AND purchasable with real money */
	NeonCredits,
	/** Earned exclusively through crew activities and cooperative play */
	CrewTokens,
	/** Earned through season pass progression and seasonal events */
	SeasonPoints,
	/** Earned through story/career milestones - commemorates achievements */
	LegacyMarks
};

/**
 * @brief Sources from which currency can be earned.
 *
 * Tracking the source allows for analytics, fraud detection,
 * and displaying earning breakdowns to players.
 */
UENUM(BlueprintType)
enum class EMGEarnSource : uint8
{
	/** Base reward for completing a race */
	RaceFinish,
	/** Bonus for winning first place */
	RaceWin,
	/** Bonus for finishing in top 3 */
	PodiumFinish,
	/** Daily login reward */
	DailyLogin,
	/** Weekly challenge completion bonus */
	WeeklyChallenge,
	/** Season milestone achievement */
	SeasonMilestone,
	/** In-game achievement unlock */
	AchievementUnlock,
	/** Crew activity participation reward */
	CrewContribution,
	/** Tournament placement rewards */
	TournamentPlacement,
	/** Story mission completion */
	StoryMilestone,
	/** Defeating a rival player */
	RivalDefeat,
	/** One-time bonus for first completion of content */
	FirstTimeBonus,
	/** Reward for gaining a player level */
	LevelUp,
	/** Participation in limited-time events */
	EventParticipation,
	/** Community goal completion rewards */
	CommunityGoal,
	/** Currency gifted by developers or other players */
	Gifted
};

// ============================================================================
// DATA STRUCTURES - Balances
// ============================================================================

/**
 * @brief Complete snapshot of all currency balances.
 *
 * Used for displaying the player's wallet and for save/load operations.
 * All values use int64 to support large accumulated amounts.
 */
USTRUCT(BlueprintType)
struct FMGCurrencyBalance
{
	GENERATED_BODY()

	/** Primary in-game currency balance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 GrindCash = 0;

	/** Premium currency balance (earnable + purchasable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 NeonCredits = 0;

	/** Crew activity token balance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CrewTokens = 0;

	/** Seasonal progression points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 SeasonPoints = 0;

	/** Story milestone commemorative marks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 LegacyMarks = 0;
};

// ============================================================================
// DATA STRUCTURES - Transactions
// ============================================================================

/**
 * @brief Record of a single currency transaction (earn or spend).
 *
 * Transactions are logged for history display, debugging, and
 * to provide detailed earning breakdowns to players.
 */
USTRUCT(BlueprintType)
struct FMGCurrencyTransaction
{
	GENERATED_BODY()

	/** Unique identifier for this transaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionID;

	/** Which currency was affected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCurrencyType CurrencyType = EMGCurrencyType::GrindCash;

	/** Amount earned (positive) or spent (negative) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Amount = 0;

	/** What activity generated this transaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEarnSource Source = EMGEarnSource::RaceFinish;

	/** Human-readable details (e.g., "Race on Midnight Highway") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SourceDetails;

	/** When the transaction occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/** Balance after this transaction was applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BalanceAfter = 0;
};

// ============================================================================
// DATA STRUCTURES - Daily Bonuses
// ============================================================================

/**
 * @brief Tracks daily login bonus status and streak.
 *
 * The daily bonus system rewards consistent play with escalating
 * rewards but does NOT punish players for missing days.
 */
USTRUCT(BlueprintType)
struct FMGDailyBonus
{
	GENERATED_BODY()

	/** Number of consecutive days the player has logged in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsecutiveDays = 0;

	/** Date of the last bonus claim */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastClaimDate;

	/** Grind Cash amount available in today's bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TodayGrindCash = 0;

	/** Neon Credits amount available in today's bonus (if any) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TodayNeonCredits = 0;

	/** Whether the bonus has already been claimed today */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClaimedToday = false;

	/** Progress toward weekly bonus (days 1-7, resets weekly) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeeklyBonusProgress = 0;
};

// ============================================================================
// DATA STRUCTURES - Multipliers
// ============================================================================

/**
 * @brief Earning multiplier that boosts currency gains from activities.
 *
 * Multipliers are EARNED through gameplay (never purchased) and
 * provide temporary or permanent boosts to specific currency types.
 */
USTRUCT(BlueprintType)
struct FMGEarningMultiplier
{
	GENERATED_BODY()

	/** Unique identifier for this multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MultiplierID;

	/** Player-facing description of the multiplier source */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Multiplier value (1.5 = 50% bonus, 2.0 = 100% bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	/** Which currency type this multiplier affects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCurrencyType AffectedCurrency = EMGCurrencyType::GrindCash;

	/** When this multiplier expires (ignored if bIsPermanent is true) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	/** If true, this multiplier never expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPermanent = false;
};

// ============================================================================
// DATA STRUCTURES - Race Earnings
// ============================================================================

/**
 * @brief Detailed breakdown of earnings from a single race.
 *
 * This struct provides transparency to players about how their
 * rewards were calculated, showing each bonus component.
 */
USTRUCT(BlueprintType)
struct FMGRaceEarnings
{
	GENERATED_BODY()

	/// @name Base Earnings
	/// @{

	/** Base earnings for completing the race (everyone gets this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BaseEarnings = 0;

	/// @}

	/// @name Performance Bonuses
	/// @{

	/** Bonus based on finishing position (1st gets most) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PositionBonus = 0;

	/** Bonus for completing race without collisions/penalties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CleanRaceBonus = 0;

	/** Bonus for defeating a designated rival */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 RivalBonus = 0;

	/** One-time bonus for first win on this track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 FirstWinBonus = 0;

	/// @}

	/// @name Multiplier Bonuses
	/// @{

	/** Additional earnings from active multipliers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 MultiplierBonus = 0;

	/** Bonus from crew membership and activities */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CrewBonus = 0;

	/// @}

	/// @name Summary
	/// @{

	/** Sum of all earnings (displayed prominently to player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalEarnings = 0;

	/** List of multipliers that were applied to this race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEarningMultiplier> AppliedMultipliers;

	/// @}
};

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// @brief Fired whenever a currency balance changes
/// @param Type Which currency changed
/// @param NewBalance The updated balance
/// @param Delta Amount of change (positive for earn, negative for spend)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMGOnCurrencyChanged, EMGCurrencyType, Type, int64, NewBalance, int64, Delta);

/// @brief Fired when a transaction is recorded
/// @param Transaction Complete transaction details
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTransactionCompleted, const FMGCurrencyTransaction&, Transaction);

/// @brief Fired when daily bonus is claimed
/// @param Bonus The claimed bonus details
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDailyBonusClaimed, const FMGDailyBonus&, Bonus);

/// @brief Fired when a new earning multiplier is activated
/// @param Multiplier The newly activated multiplier
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMultiplierActivated, const FMGEarningMultiplier&, Multiplier);

/// @brief Fired when an earning multiplier expires
/// @param MultiplierID ID of the expired multiplier
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMultiplierExpired, FName, MultiplierID);

// ============================================================================
// CURRENCY SUBSYSTEM CLASS
// ============================================================================

/**
 * @brief Central subsystem for managing all in-game currency and earnings.
 *
 * The Currency Subsystem handles all aspects of the game's economy from
 * the earning side (spending is handled by UMGShopSubsystem). It implements
 * a player-friendly economic model designed to be generous and fair.
 *
 * @section features Key Features
 *
 * **Balance Management:**
 * - Tracks five distinct currency types
 * - Supports large balances (64-bit integers)
 * - Full transaction history for transparency
 *
 * **Race Earnings:**
 * - Generous base rewards for all participants
 * - Scaling bonuses based on performance
 * - First-win bonuses to reward exploration
 * - Clean race bonuses to encourage skill
 *
 * **Daily/Weekly Bonuses:**
 * - Escalating rewards for consecutive logins
 * - Weekly completion bonuses with Neon Credits
 * - Designed to reward consistency, not punish absence
 *
 * **Earning Multipliers:**
 * - Temporary boosts earned through gameplay
 * - Permanent multipliers from achievements
 * - NEVER purchasable - always earned
 *
 * @note This system intentionally avoids pay-to-win mechanics. Premium
 *       currency (Neon Credits) is earnable through normal gameplay and
 *       only unlocks cosmetic items.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @name Lifecycle
	/// @{

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/// @}

	// ==========================================
	// BALANCE QUERIES
	// ==========================================

	/// @name Balance Queries
	/// @brief Functions for checking currency amounts.
	/// @{

	/**
	 * @brief Get complete balance for all currency types.
	 * @return Snapshot of all currency balances
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	FMGCurrencyBalance GetBalance() const { return CurrentBalance; }

	/**
	 * @brief Get balance of a specific currency type.
	 * @param Type The currency to query
	 * @return Current balance of specified currency
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	int64 GetCurrencyAmount(EMGCurrencyType Type) const;

	/**
	 * @brief Check if player has enough of a currency.
	 * @param Type The currency type to check
	 * @param Amount The amount needed
	 * @return True if balance >= Amount
	 */
	UFUNCTION(BlueprintPure, Category = "Currency")
	bool CanAfford(EMGCurrencyType Type, int64 Amount) const;

	/// @}

	// ==========================================
	// CURRENCY TRANSACTIONS
	// ==========================================

	/// @name Currency Transactions
	/// @brief Functions for earning and spending currency.
	/// @{

	/**
	 * @brief Award currency to the player from a gameplay activity.
	 * @param Type Which currency to award
	 * @param Amount How much to award (must be positive)
	 * @param Source What activity generated this reward
	 * @param Details Optional human-readable context (e.g., track name)
	 * @return True if successfully awarded
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool EarnCurrency(EMGCurrencyType Type, int64 Amount, EMGEarnSource Source, const FString& Details = TEXT(""));

	/**
	 * @brief Deduct currency for a purchase.
	 * @param Type Which currency to spend
	 * @param Amount How much to spend
	 * @param PurchaseDetails What was purchased (for history)
	 * @return True if successful, false if insufficient funds
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool SpendCurrency(EMGCurrencyType Type, int64 Amount, const FString& PurchaseDetails);

	/**
	 * @brief Transfer currency to another player (for trading).
	 * @param Type Which currency to transfer
	 * @param Amount How much to transfer
	 * @param RecipientID Player ID of the recipient
	 * @return True if transfer was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TransferCurrency(EMGCurrencyType Type, int64 Amount, const FString& RecipientID);

	/// @}

	// ==========================================
	// RACE EARNINGS
	// ==========================================

	/// @name Race Earnings (Generous by Design)
	/// @brief Functions for calculating and awarding race rewards.
	/// @{

	/**
	 * @brief Calculate earnings for a completed race.
	 *
	 * Calculates all bonuses and multipliers to produce a detailed
	 * earnings breakdown. This is "generous by design" - everyone
	 * earns meaningful rewards, with skill providing bonuses.
	 *
	 * @param Position Finishing position (1 = winner)
	 * @param TotalRacers Total racers in the event
	 * @param TrackID ID of the track raced on
	 * @param bCleanRace True if no collisions/penalties occurred
	 * @param bDefeatedRival True if player beat their designated rival
	 * @param bFirstWinOnTrack True if this is player's first win here
	 * @return Detailed earnings breakdown
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Racing")
	FMGRaceEarnings CalculateRaceEarnings(int32 Position, int32 TotalRacers, FName TrackID, bool bCleanRace, bool bDefeatedRival, bool bFirstWinOnTrack);

	/**
	 * @brief Apply calculated race earnings to player's balance.
	 * @param Earnings Pre-calculated earnings from CalculateRaceEarnings
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Racing")
	void AwardRaceEarnings(const FMGRaceEarnings& Earnings);

	/// @}

	// ==========================================
	// DAILY/WEEKLY BONUSES
	// ==========================================

	/// @name Daily/Weekly Bonuses
	/// @brief Rewards for consistent play (not punishment for absence).
	/// @{

	/**
	 * @brief Get current daily bonus status.
	 * @return Current bonus state including streak and availability
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Bonus")
	FMGDailyBonus GetDailyBonusStatus() const { return DailyBonus; }

	/**
	 * @brief Claim the daily login bonus.
	 * @return True if bonus was claimed, false if already claimed today
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Bonus")
	bool ClaimDailyBonus();

	/**
	 * @brief Check if daily bonus is available to claim.
	 * @return True if bonus hasn't been claimed today
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|Bonus")
	bool CanClaimDailyBonus() const;

	/**
	 * @brief Preview the daily bonus amount for a given streak.
	 * @param ConsecutiveDays The streak length to calculate for
	 * @return Grind Cash amount that would be awarded
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|Bonus")
	int64 GetDailyBonusAmount(int32 ConsecutiveDays) const;

	/**
	 * @brief Get the weekly completion bonus amount in Neon Credits.
	 * @return Neon Credits awarded for completing 7-day streak
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|Bonus")
	int64 GetWeeklyBonusNeonCredits() const;

	/// @}

	// ==========================================
	// MULTIPLIERS
	// ==========================================

	/// @name Multipliers (Earned Through Gameplay, Never Purchased)
	/// @brief Functions for managing earning multiplier boosts.
	/// @{

	/**
	 * @brief Activate a new earning multiplier.
	 * @param Multiplier The multiplier to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Multiplier")
	void AddMultiplier(const FMGEarningMultiplier& Multiplier);

	/**
	 * @brief Remove an active multiplier by ID.
	 * @param MultiplierID ID of the multiplier to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Currency|Multiplier")
	void RemoveMultiplier(FName MultiplierID);

	/**
	 * @brief Get all currently active multipliers.
	 * @return Array of active multipliers
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|Multiplier")
	TArray<FMGEarningMultiplier> GetActiveMultipliers() const { return ActiveMultipliers; }

	/**
	 * @brief Calculate total multiplier for a currency type.
	 * @param Type The currency type to calculate multiplier for
	 * @return Combined multiplier (base 1.0 + all bonuses)
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|Multiplier")
	float GetTotalMultiplier(EMGCurrencyType Type) const;

	/// @}

	// ==========================================
	// TRANSACTION HISTORY
	// ==========================================

	/// @name Transaction History
	/// @brief Functions for viewing earning/spending history.
	/// @{

	/**
	 * @brief Get recent transactions for history display.
	 * @param Count Maximum number of transactions to return
	 * @return Array of recent transactions, newest first
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|History")
	TArray<FMGCurrencyTransaction> GetRecentTransactions(int32 Count = 20) const;

	/**
	 * @brief Get lifetime total earned for a currency type.
	 * @param Type The currency type to query
	 * @return Total amount ever earned
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|History")
	int64 GetTotalEarned(EMGCurrencyType Type) const;

	/**
	 * @brief Get lifetime total spent for a currency type.
	 * @param Type The currency type to query
	 * @return Total amount ever spent
	 */
	UFUNCTION(BlueprintPure, Category = "Currency|History")
	int64 GetTotalSpent(EMGCurrencyType Type) const;

	/// @}

	// ==========================================
	// EVENTS
	// ==========================================

	/// @name Event Delegates
	/// @brief Subscribe to these to react to currency changes.
	/// @{

	/** Broadcast when any currency balance changes */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnCurrencyChanged OnCurrencyChanged;

	/** Broadcast when a transaction is recorded */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnTransactionCompleted OnTransactionCompleted;

	/** Broadcast when daily bonus is claimed */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnDailyBonusClaimed OnDailyBonusClaimed;

	/** Broadcast when a multiplier is activated */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnMultiplierActivated OnMultiplierActivated;

	/** Broadcast when a multiplier expires */
	UPROPERTY(BlueprintAssignable, Category = "Currency|Events")
	FMGOnMultiplierExpired OnMultiplierExpired;

	/// @}

protected:
	/// @name Internal Operations
	/// @{

	/** Load currency data from save file */
	void LoadCurrencyData();

	/** Save currency data to persistent storage */
	void SaveCurrencyData();

	/** Check for and remove expired multipliers */
	void UpdateMultipliers();

	/** Generate a unique transaction ID */
	FString GenerateTransactionID();

	/** Record a transaction in history */
	void RecordTransaction(EMGCurrencyType Type, int64 Amount, EMGEarnSource Source, const FString& Details);

	/// @}

private:
	/// @name Private Data
	/// @{

	/** Current balance across all currency types */
	UPROPERTY()
	FMGCurrencyBalance CurrentBalance;

	/** Daily bonus tracking state */
	UPROPERTY()
	FMGDailyBonus DailyBonus;

	/** Currently active earning multipliers */
	UPROPERTY()
	TArray<FMGEarningMultiplier> ActiveMultipliers;

	/** History of all transactions */
	UPROPERTY()
	TArray<FMGCurrencyTransaction> TransactionHistory;

	/** Lifetime Grind Cash earned (for stats) */
	int64 TotalGrindCashEarned = 0;

	/** Lifetime Grind Cash spent (for stats) */
	int64 TotalGrindCashSpent = 0;

	/** Lifetime Neon Credits earned (for stats) */
	int64 TotalNeonCreditsEarned = 0;

	/** Lifetime Neon Credits spent (for stats) */
	int64 TotalNeonCreditsSpent = 0;

	/** Timer for periodic multiplier expiration checks */
	FTimerHandle MultiplierUpdateTimer;

	/// @}
};
