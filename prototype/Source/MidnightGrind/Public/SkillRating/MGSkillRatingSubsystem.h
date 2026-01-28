// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSkillRatingSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Skill Rating Subsystem, which tracks player skill levels
 * using a matchmaking rating (MMR) system similar to those found in competitive
 * games like League of Legends, Rocket League, or chess (Elo). It determines
 * how skilled a player is and helps match them with appropriate opponents.
 *
 * WHAT IS A SKILL RATING SYSTEM?
 * ------------------------------
 * A skill rating system:
 *   - Assigns a numerical value (MMR) representing player skill
 *   - Updates that value based on match results
 *   - Considers opponent strength when calculating changes
 *   - Translates raw numbers into visible ranks (Bronze, Gold, Diamond, etc.)
 *   - Helps matchmaking find fair opponents
 *
 * The core idea: If you beat a stronger opponent, you gain more rating.
 * If you lose to a weaker opponent, you lose more rating.
 *
 * KEY CONCEPTS AND TERMINOLOGY:
 * -----------------------------
 *
 * 1. MMR (Matchmaking Rating):
 *    The hidden numerical skill value. Starts around 1500 (average).
 *    Higher = more skilled. Used for matchmaking calculations.
 *    Example: 1200 = below average, 1800 = above average, 2200 = elite
 *
 * 2. RANK TIERS (EMGRankTier):
 *    Visual representation of skill shown to players:
 *    - Unranked: Haven't completed placement matches
 *    - Bronze: Beginner tier
 *    - Silver: Below average
 *    - Gold: Average players
 *    - Platinum: Above average
 *    - Diamond: Skilled players
 *    - Master: Expert players
 *    - Grandmaster: Elite players
 *    - Legend: Top of the leaderboard
 *
 * 3. DIVISIONS (EMGRankDivision):
 *    Subdivisions within each tier: IV (lowest) -> III -> II -> I (highest)
 *    Example: Gold IV is the entry to Gold, Gold I is about to promote to Platinum
 *
 * 4. PLACEMENT MATCHES:
 *    The first 10 games a player must complete before receiving an initial rank.
 *    During placement, rating changes are larger (more volatile) to quickly
 *    find the player's true skill level.
 *
 * 5. UNCERTAINTY:
 *    A statistical measure of how confident the system is in your MMR.
 *    - High uncertainty (350): Not many games played, big rating swings
 *    - Low uncertainty (100): Many games played, small rating changes
 *    - Increases when you don't play for a while (inactivity)
 *
 * 6. K-FACTOR:
 *    Controls how much rating changes per game. Higher = more volatile.
 *    - Placement K-Factor (64): Large changes during initial games
 *    - Base K-Factor (32): Normal changes for established players
 *
 * 7. RATING CATEGORIES (EMGRatingCategory):
 *    Separate ratings for different game modes:
 *    - Overall: Combined rating
 *    - CircuitRacing: Multi-lap races
 *    - SprintRacing: Point-to-point races
 *    - Drifting: Drift scoring events
 *    - TimeAttack: Time trial competitions
 *    - TeamRacing: Team-based events
 *
 * 8. SEASONS:
 *    Time periods (usually months) after which ratings partially reset.
 *    Keeps the ranked experience fresh and gives everyone a new start.
 *    Past season achievements are recorded in history.
 *
 * HOW THE MATH WORKS (Simplified):
 * --------------------------------
 * The system uses an Elo-like formula:
 *
 *   Expected Score = 1 / (1 + 10^((OpponentMMR - YourMMR) / 400))
 *   Rating Change = K-Factor * (Actual Score - Expected Score)
 *
 * If you're 1600 MMR vs 1400 MMR opponent:
 *   - You're expected to win ~75% of the time
 *   - Win: Gain ~8 points (expected outcome)
 *   - Lose: Lose ~24 points (unexpected upset)
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UGameInstanceSubsystem, persisting across level loads
 * - Used by matchmaking to find appropriate opponents
 * - Tournament seeding uses these ratings
 * - Works with online backend for leaderboards
 * - Feeds into profile display and progression UI
 *
 * TYPICAL WORKFLOW:
 * -----------------
 * 1. New player starts (MMR = 1500, Uncertainty = 350, Unranked)
 * 2. Plays placement matches (10 games)
 * 3. Receives initial rank after placements (OnPlacementCompleted)
 * 4. Each match updates rating (ProcessMatchResult)
 * 5. Rank changes as MMR thresholds are crossed (OnPromoted, OnDemoted)
 * 6. Season ends, soft reset applied, history recorded
 *
 * DATA STRUCTURES:
 * ----------------
 * - FMGRank: The visible rank (tier + division + icon)
 * - FMGSkillRating: Complete rating data for one category
 * - FMGMatchResult: Outcome of a single match with rating changes
 * - FMGOpponentRating: Info about an opponent for calculations
 * - FMGSeasonStats: Historical record of a past season
 * - FMGRatingConfig: Tunable parameters for the rating algorithm
 *
 * DELEGATES (Event Notifications):
 * --------------------------------
 * - OnRatingChanged: Raw MMR value changed
 * - OnRankChanged: Visible rank changed
 * - OnPromoted: Moved up a tier or division
 * - OnDemoted: Moved down a tier or division
 * - OnMatchResultProcessed: Match result was calculated
 * - OnPlacementCompleted: Initial rank assigned after placements
 * - OnSeasonEnded: Season concluded, rewards distributed
 *
 * USEFUL FUNCTIONS FOR UI:
 * ------------------------
 * - GetRank(): Get current visible rank
 * - GetProgressToNextDivision(): Progress bar percentage
 * - IsInPlacements(): Show placement UI vs ranked UI
 * - GetLeaderboardPosition(): Where player stands globally
 * - GetTopPercentile(): "Top 5%" display
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ---------------------------
 * - FGuid: Globally Unique Identifier (for match IDs)
 * - FLinearColor: RGBA color value (for rank badge colors)
 * - TSoftObjectPtr<UTexture2D>: Lazy reference to a texture asset
 * - ShouldCreateSubsystem(): Override to conditionally create subsystem
 * - virtual void Initialize(): Called when subsystem is created
 * - virtual void Deinitialize(): Called when subsystem is destroyed
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSkillRatingSubsystem.generated.h"

//=============================================================================
// ENUMS - Rating System Types
//=============================================================================

/**
 * Rank Tier
 *
 * The major skill tiers displayed to players. Each tier has 4 divisions (IV-I).
 * Tiers correspond to MMR ranges (approximately):
 *
 * - Unranked: 0-??? (Haven't completed placements)
 * - Bronze: ~0-1100 (Learning the basics)
 * - Silver: ~1100-1300 (Getting competent)
 * - Gold: ~1300-1500 (Average player)
 * - Platinum: ~1500-1700 (Above average)
 * - Diamond: ~1700-1900 (Skilled players)
 * - Master: ~1900-2100 (Expert players)
 * - Grandmaster: ~2100-2300 (Elite players)
 * - Legend: 2300+ (Top of the leaderboard)
 *
 * The distribution is designed so most players are Gold (the middle).
 */
UENUM(BlueprintType)
enum class EMGRankTier : uint8
{
	Unranked,      ///< No rank yet - complete placement matches first
	Bronze,        ///< Entry level tier
	Silver,        ///< Developing skills
	Gold,          ///< Average tier - most players end up here
	Platinum,      ///< Above average performance
	Diamond,       ///< High skill tier
	Master,        ///< Expert tier
	Grandmaster,   ///< Elite tier - top 1%
	Legend         ///< The best of the best - top 0.1%
};

/**
 * Rank Division
 *
 * Subdivisions within each tier. IV is the entry point, I is the highest.
 * Roman numerals are used traditionally in ranking systems.
 *
 * Progression within a tier: IV -> III -> II -> I -> (next tier IV)
 *
 * Example progression:
 * Gold IV -> Gold III -> Gold II -> Gold I -> Platinum IV
 *
 * Each division spans ~50 rank points (configurable).
 */
UENUM(BlueprintType)
enum class EMGRankDivision : uint8
{
	IV,   ///< Division 4 - Entry to the tier (lowest)
	III,  ///< Division 3
	II,   ///< Division 2
	I     ///< Division 1 - About to promote (highest)
};

/**
 * Rating Category
 *
 * Separate skill ratings are tracked for different game modes.
 * This allows a player to be Diamond in Drifting but Gold in Sprints.
 *
 * Categories:
 * - Overall: Composite rating (weighted average of all categories)
 * - CircuitRacing: Multi-lap track races
 * - SprintRacing: Point-to-point races
 * - Drifting: Drift scoring competitions
 * - TimeAttack: Time trial competitions
 * - TeamRacing: Team-based events (2v2, 3v3, etc.)
 *
 * Each category has independent MMR, rank, and placement status.
 */
UENUM(BlueprintType)
enum class EMGRatingCategory : uint8
{
	Overall,        ///< Combined rating across all modes
	CircuitRacing,  ///< Traditional lap-based racing
	SprintRacing,   ///< A-to-B race events
	Drifting,       ///< Drift scoring competitions
	TimeAttack,     ///< Solo time trial challenges
	TeamRacing      ///< Team-based competitive modes
};

/**
 * Placement Status
 *
 * Tracks whether a player has completed their placement matches.
 * Placement matches are the first N games (usually 10) that determine
 * initial rank. During placements, rating changes are larger.
 *
 * - NotStarted: Player hasn't played any ranked games in this category
 * - InProgress: Player is in the middle of placement matches
 * - Completed: Placement done, player has an official rank
 *
 * After placement, future seasons may require "re-placement" with fewer games.
 */
UENUM(BlueprintType)
enum class EMGPlacementStatus : uint8
{
	NotStarted,   ///< No ranked games played yet
	InProgress,   ///< Playing placement matches (1-9 games typically)
	Completed     ///< Placements done, rank assigned
};

//=============================================================================
// STRUCTS - Rating Data Containers
//=============================================================================

/**
 * Rank
 *
 * The visible representation of a player's skill level.
 * Contains both the logical rank (tier/division) and visual assets.
 *
 * This is what players see and care about - the MMR is hidden internally.
 *
 * Key properties:
 * - Tier/Division: The actual rank (e.g., Gold II)
 * - RankPoints: Points within the rank system (not the same as MMR)
 * - PointsToNextDivision: How many points needed to promote
 * - PointsInCurrentDivision: Progress toward next division
 * - RankName: Display text (e.g., "Gold II")
 * - RankIcon: Badge/emblem texture
 * - RankColor: Color for UI highlighting
 */
USTRUCT(BlueprintType)
struct FMGRank
{
	GENERATED_BODY()

	/** Which major tier (Bronze, Silver, Gold, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRankTier Tier = EMGRankTier::Unranked;

	/** Which subdivision within the tier (IV, III, II, I) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRankDivision Division = EMGRankDivision::IV;

	/** Total rank points (derived from MMR) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPoints = 0;

	/** Points required for next division/tier promotion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsToNextDivision = 100;

	/** Current progress in this division (for progress bar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsInCurrentDivision = 0;

	/** Display name (e.g., "Diamond III") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RankName;

	/** Badge icon texture (lazy-loaded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RankIcon;

	/** Color for UI elements (tier-specific) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RankColor = FLinearColor::White;
};

/**
 * Skill Rating
 *
 * Complete rating data for one category (e.g., Overall, CircuitRacing).
 * Contains both the hidden MMR and the visible rank, plus statistics.
 *
 * Key concepts:
 * - MMR: Hidden numerical rating (starts at 1500)
 * - Uncertainty: How confident the system is (starts high, decreases)
 * - CurrentRank: What the player sees
 * - PeakRank: Highest rank achieved (for profile bragging)
 *
 * The relationship: MMR is the "true skill estimate", Rank is derived from MMR
 * with some smoothing to avoid constant fluctuation.
 */
USTRUCT(BlueprintType)
struct FMGSkillRating
{
	GENERATED_BODY()

	/** Which game mode this rating is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRatingCategory Category = EMGRatingCategory::Overall;

	/**
	 * Matchmaking Rating - the hidden skill number.
	 * 1500 = average. Higher = more skilled.
	 * Used for matchmaking and rating change calculations.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MMR = 1500.0f;

	/**
	 * Uncertainty in the MMR estimate.
	 * High (350) = not confident, larger rating swings.
	 * Low (100) = confident, smaller rating changes.
	 * Decreases with games played, increases with inactivity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Uncertainty = 350.0f;

	/** Current visible rank shown to the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank CurrentRank;

	/** Highest rank ever achieved (never decreases) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank PeakRank;

	/** Total ranked games played in this category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GamesPlayed = 0;

	/** Total wins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	/** Total losses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Losses = 0;

	/** Win rate percentage (Wins / GamesPlayed * 100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.0f;

	/** Current streak (positive = wins, negative = losses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0;

	/** Best winning streak ever achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestWinStreak = 0;

	/** Worst losing streak (stored as positive number) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WorstLossStreak = 0;

	/** When the last ranked game was played (for inactivity detection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastPlayed;

	/** Whether placement matches are complete */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlacementStatus PlacementStatus = EMGPlacementStatus::NotStarted;

	/** How many placement matches completed so far */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlacementGamesPlayed = 0;

	/** How many placement matches needed for initial rank */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlacementGamesRequired = 10;
};

/**
 * Match Result
 *
 * The outcome of a single ranked match, including rating changes.
 * Returned from ProcessMatchResult() for display and history.
 *
 * Use this for:
 * - Post-match rating change display ("+24 MMR")
 * - Promotion/demotion animations
 * - Match history screen
 */
USTRUCT(BlueprintType)
struct FMGMatchResult
{
	GENERATED_BODY()

	/** Unique identifier for this match (for history lookup) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid MatchID;

	/** Which rating category was affected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRatingCategory Category = EMGRatingCategory::Overall;

	/** Where the player finished (1st, 2nd, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	/** How many players were in the match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPlayers = 0;

	/** Average MMR of opponents (for context) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageOpponentMMR = 1500.0f;

	/** How much MMR changed (+/-) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RatingChange = 0.0f;

	/** New MMR after this match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NewMMR = 0.0f;

	/** How many rank points changed (+/-) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPointsChange = 0;

	/** True if this match caused a promotion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPromoted = false;

	/** True if this match caused a demotion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDemoted = false;

	/** Rank before this match (for comparison) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank OldRank;

	/** Rank after this match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank NewRank;

	/** When this match occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Opponent Rating
 *
 * Rating information for an opponent in a match.
 * Used as input to ProcessMatchResult() to calculate rating changes.
 *
 * The system needs to know opponent skill to properly adjust ratings.
 * Beating a higher-rated opponent = bigger gain.
 * Losing to a lower-rated opponent = bigger loss.
 */
USTRUCT(BlueprintType)
struct FMGOpponentRating
{
	GENERATED_BODY()

	/** Unique player identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	/** Display name (for match history) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	/** Opponent's MMR (for calculations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MMR = 1500.0f;

	/** Opponent's visible rank (for display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank Rank;

	/** Where this opponent finished in the match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;
};

/**
 * Season Stats
 *
 * Historical record of a player's performance in a past season.
 * Seasons typically last 2-3 months and end with a soft rating reset.
 *
 * Use for:
 * - Season end rewards display
 * - Profile history ("Season 3: Diamond II")
 * - Tracking improvement over time
 */
USTRUCT(BlueprintType)
struct FMGSeasonStats
{
	GENERATED_BODY()

	/** Unique season identifier (e.g., "S3_2025") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	/** Highest rank achieved during the season */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank HighestRank;

	/** Rank when the season ended (determines rewards) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank FinalRank;

	/** Peak MMR reached (for personal stats) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakMMR = 0.0f;

	/** Total ranked games played in the season */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalGames = 0;

	/** Total wins in the season */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	/** Win rate for the season */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.0f;

	/** Rewards earned (icons, titles, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RewardsEarned;
};

/**
 * Rating Config
 *
 * Tunable parameters for the rating algorithm. Designers can adjust these
 * to control how fast/slow rating changes, and how the system feels.
 *
 * Understanding the key parameters:
 *
 * K-Factor: Controls rating volatility
 * - Higher K = bigger swings per game
 * - Lower K = more stable ratings
 *
 * Uncertainty: Confidence in the rating
 * - Decays with games played
 * - Increases with inactivity
 * - Affects K-factor (higher uncertainty = bigger changes)
 *
 * Streaks: Bonus/penalty for consecutive wins/losses
 * - Helps players climb/fall faster when on a roll/tilt
 */
USTRUCT(BlueprintType)
struct FMGRatingConfig
{
	GENERATED_BODY()

	/**
	 * Base K-factor for normal games.
	 * Standard is 32. Lower = more stable, Higher = more volatile.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseKFactor = 32.0f;

	/**
	 * K-factor during placement matches.
	 * Usually 2x base to quickly find player's true skill.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlacementKFactor = 64.0f;

	/**
	 * How much uncertainty decreases per game.
	 * 0.98 = loses 2% per game. Smaller = faster stabilization.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UncertaintyDecay = 0.98f;

	/** Minimum uncertainty (for veteran players) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinUncertainty = 100.0f;

	/** Maximum uncertainty (for new/returning players) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxUncertainty = 350.0f;

	/**
	 * How much uncertainty increases per inactive day.
	 * Returning players should have more volatile ratings.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InactivityUncertaintyGain = 10.0f;

	/** Days of inactivity before uncertainty starts increasing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InactivityDaysThreshold = 14;

	/** Bonus rank points when promoting to a new tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PromotionBonus = 25;

	/**
	 * Games of protection after promoting before you can demote.
	 * Prevents frustrating yo-yo promotions/demotions.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DemotionProtectionGames = 3;

	/**
	 * Extra rating change per streak game.
	 * 0.1 = 10% bonus per streak game.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreakBonusMultiplier = 0.1f;

	/** Maximum streak bonus (caps the bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStreakBonus = 5;
};

//=============================================================================
// DELEGATES - Event Notifications
//=============================================================================

/**
 * Delegates for rating system events. Subscribe to react to changes.
 * Most useful for UI updates (showing rating change animations, etc.)
 */

/** Fired when hidden MMR changes. For debug/analytics. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRatingChanged, EMGRatingCategory, Category, float, NewRating);

/** Fired when visible rank changes (any tier or division change). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRankChanged, EMGRatingCategory, Category, const FMGRank&, NewRank);

/** Fired specifically on promotion. Trigger celebration UI! */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPromoted, EMGRatingCategory, Category, const FMGRank&, NewRank);

/** Fired specifically on demotion. Show consolation message. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDemoted, EMGRatingCategory, Category, const FMGRank&, NewRank);

/** Fired after any match is processed. Contains full result details. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchResultProcessed, const FMGMatchResult&, Result);

/** Fired when placement matches are complete and initial rank is assigned. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlacementCompleted, EMGRatingCategory, Category, const FMGRank&, InitialRank);

/** Fired when a ranked season ends. Trigger season recap screen. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSeasonEnded);

//=============================================================================
// SUBSYSTEM CLASS
//=============================================================================

/**
 * Skill Rating Subsystem
 *
 * The main subsystem managing player skill ratings and ranks.
 * This is a GameInstanceSubsystem - one instance per game session.
 *
 * Access via: GetGameInstance()->GetSubsystem<UMGSkillRatingSubsystem>()
 *
 * Key Responsibilities:
 * - Track MMR and visible rank per category
 * - Process match results and calculate rating changes
 * - Handle placement matches for new players
 * - Manage seasons (start, end, soft reset)
 * - Provide leaderboard position information
 * - Persist rating data across sessions
 *
 * The rating algorithm is based on Elo/Glicko principles:
 * - Win probability calculated from MMR difference
 * - Rating change based on actual vs expected result
 * - Uncertainty reduces volatility over time
 *
 * Usage Example:
 * @code
 *     // After a race finishes
 *     TArray<FMGOpponentRating> Opponents;
 *     // ... fill Opponents with data from the race ...
 *     FMGMatchResult Result = SkillRating->ProcessMatchResult(
 *         EMGRatingCategory::CircuitRacing,
 *         MyPosition,
 *         Opponents
 *     );
 *     // Result contains rating changes to display
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSkillRatingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initialize subsystem, load saved ratings, set up rank thresholds. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Clean up and save rating data. */
	virtual void Deinitialize() override;

	/**
	 * Determines if this subsystem should be created.
	 * Can be used to disable ranked play in certain builds or modes.
	 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Rating Access
	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	FMGSkillRating GetRating(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	TMap<EMGRatingCategory, FMGSkillRating> GetAllRatings() const { return Ratings; }

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	FMGRank GetRank(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	float GetMMR(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	float GetDisplayRating(EMGRatingCategory Category) const;

	// Match Processing
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Match")
	FMGMatchResult ProcessMatchResult(EMGRatingCategory Category, int32 Position, const TArray<FMGOpponentRating>& Opponents);

	UFUNCTION(BlueprintCallable, Category = "SkillRating|Match")
	FMGMatchResult ProcessSimpleResult(EMGRatingCategory Category, bool bWon, float OpponentMMR);

	UFUNCTION(BlueprintPure, Category = "SkillRating|Match")
	float PredictRatingChange(EMGRatingCategory Category, int32 ExpectedPosition, float AverageOpponentMMR) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Match")
	float CalculateWinProbability(float PlayerMMR, float OpponentMMR) const;

	// Rank Info
	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	FMGRank GetRankFromMMR(float MMR) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	int32 GetRankPointsForMMR(float MMR) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	float GetProgressToNextDivision(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	bool IsInPromotionSeries(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	bool IsInDemotionZone(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	TArray<FMGRank> GetAllRankTiers() const;

	// Placement
	UFUNCTION(BlueprintPure, Category = "SkillRating|Placement")
	bool IsInPlacements(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Placement")
	int32 GetPlacementGamesRemaining(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Placement")
	float GetPlacementProgress(EMGRatingCategory Category) const;

	// History
	UFUNCTION(BlueprintPure, Category = "SkillRating|History")
	TArray<FMGMatchResult> GetMatchHistory(EMGRatingCategory Category, int32 MaxEntries = 20) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|History")
	TArray<FMGSeasonStats> GetSeasonHistory() const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|History")
	FMGSeasonStats GetCurrentSeasonStats() const;

	// Leaderboard Position
	UFUNCTION(BlueprintPure, Category = "SkillRating|Leaderboard")
	int32 GetLeaderboardPosition(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Leaderboard")
	int32 GetRegionalPosition(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Leaderboard")
	float GetTopPercentile(EMGRatingCategory Category) const;

	// Season
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Season")
	void StartNewSeason(FName SeasonID);

	UFUNCTION(BlueprintCallable, Category = "SkillRating|Season")
	void EndSeason();

	UFUNCTION(BlueprintPure, Category = "SkillRating|Season")
	FName GetCurrentSeasonID() const { return CurrentSeasonID; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Config")
	void SetConfig(const FMGRatingConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "SkillRating|Config")
	FMGRatingConfig GetConfig() const { return Config; }

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Save")
	void SaveRatingData();

	UFUNCTION(BlueprintCallable, Category = "SkillRating|Save")
	void LoadRatingData();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnRatingChanged OnRatingChanged;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnRankChanged OnRankChanged;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnPromoted OnPromoted;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnDemoted OnDemoted;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnMatchResultProcessed OnMatchResultProcessed;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnPlacementCompleted OnPlacementCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnSeasonEnded OnSeasonEnded;

protected:
	float CalculateRatingChange(float PlayerMMR, float PlayerUncertainty, float OpponentMMR, float Score, bool bIsPlacement) const;
	float CalculateExpectedScore(float PlayerMMR, float OpponentMMR) const;
	void UpdateRank(FMGSkillRating& Rating);
	void ApplySoftReset(FMGSkillRating& Rating);
	void UpdateStreak(FMGSkillRating& Rating, bool bWon);
	void InitializeRatings();
	void InitializeRankThresholds();
	void CheckInactivity();

	UPROPERTY()
	TMap<EMGRatingCategory, FMGSkillRating> Ratings;

	UPROPERTY()
	TMap<EMGRatingCategory, TArray<FMGMatchResult>> MatchHistory;

	UPROPERTY()
	TArray<FMGSeasonStats> SeasonHistory;

	UPROPERTY()
	FMGSeasonStats CurrentSeasonStats;

	UPROPERTY()
	FMGRatingConfig Config;

	UPROPERTY()
	FName CurrentSeasonID;

	UPROPERTY()
	TArray<int32> RankThresholds;

	UPROPERTY()
	int32 GlobalLeaderboardPosition = 0;

	UPROPERTY()
	int32 RegionalLeaderboardPosition = 0;

	UPROPERTY()
	float TopPercentile = 100.0f;
};
