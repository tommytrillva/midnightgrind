// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGRivalrySubsystem.h
 * @brief Named Rivals and Relationship System for Single-Player
 *
 * @section overview Overview
 * This subsystem creates persistent AI opponents that remember past encounters
 * with the player, developing rivalries, grudges, or respect over time. Named
 * rivals provide narrative structure through emergent gameplay, creating
 * memorable moments and personal stakes in races.
 *
 * Key Features:
 * - **Named Rivals:** Pre-designed characters with personalities
 * - **Dynamic Rivalries:** Auto-generated from repeated encounters
 * - **Relationship Tracking:** Respect, hostility, memorable moments
 * - **Adaptive Behavior:** AI adjusts tactics based on rivalry history
 * - **Story Integration:** Rivals appear at campaign milestones
 *
 * @section concepts Key Concepts
 *
 * @subsection rivalry_levels Rivalry Levels
 * Relationships progress through stages:
 * - **Unknown (0 races):** First encounter
 * - **Acquaintance (1-2):** Noticed each other
 * - **Known Opponent (3-5):** Regular competition
 * - **Rival (6-10):** Emerging rivalry
 * - **Nemesis (10+):** Deep rivalry, personal
 * - **Respected:** High respect, clean racing
 * - **Bitter:** High hostility, grudge match
 *
 * @subsection relationship_dimensions Relationship Dimensions
 * Two independent axes define relationships:
 * - **Respect (0-1):** Earned through clean racing, close finishes
 * - **Hostility (-1 to 1):** Affected by contact, dirty tactics
 *
 * Examples:
 * - High Respect + Low Hostility = **Honored Rival**
 * - High Respect + High Hostility = **Nemesis** (respectful but fierce)
 * - Low Respect + High Hostility = **Bitter Enemy**
 *
 * @section usage Usage Examples
 *
 * @subsection spawn_rival Spawning Named Rival
 * @code
 * UMGRivalrySubsystem* RivalrySystem = 
 *     GetGameInstance()->GetSubsystem<UMGRivalrySubsystem>();
 *
 * // Spawn story rival for campaign mission
 * FMGRivalryData* Rival = RivalrySystem->GetRival("TheProdigy");
 * if (Rival && Rival->bIsUnlocked)
 * {
 *     UMGAIDriverProfile* Profile = Rival->DriverProfile;
 *     // Apply rivalry-based modifiers
 *     RivalrySystem->ApplyRivalryModifiersToProfile(Profile, Rival);
 *     SpawnAIOpponent(Profile, SpawnTransform);
 * }
 * @endcode
 *
 * @subsection record_encounter Recording Race Encounter
 * @code
 * // After race with rival present
 * RivalrySystem->RecordRaceEncounter(
 *     "TheProdigy",        // Rival ID
 *     PlayerPosition,      // 1-8
 *     RivalPosition,       // 1-8
 *     PlayerFinishTime,    // 123.45f
 *     RivalFinishTime,     // 125.20f
 *     ContactIncidents,    // 2
 *     bCleanRace           // false
 * );
 *
 * // Check if rivalry progressed
 * if (RivalrySystem->DidRivalryLevelUp("TheProdigy"))
 * {
 *     // Show rivalry notification
 *     ShowRivalryNotification(Rival);
 * }
 * @endcode
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AI/MG_AI_DriverProfile.h"
#include "MGRivalrySubsystem.generated.h"

/**
 * Rivalry status enumeration
 */
UENUM(BlueprintType)
enum class EMGRivalryStatus : uint8
{
	/** Never raced */
	Unknown          UMETA(DisplayName = "Unknown"),
	
	/** 1-2 races together */
	Acquaintance     UMETA(DisplayName = "Acquaintance"),
	
	/** 3-5 races, starting to recognize */
	KnownOpponent    UMETA(DisplayName = "Known Opponent"),
	
	/** 6-10 competitive races */
	Rival            UMETA(DisplayName = "Rival"),
	
	/** 10+ races, intense rivalry */
	Nemesis          UMETA(DisplayName = "Nemesis"),
	
	/** High respect, honorable competition */
	Respected        UMETA(DisplayName = "Respected Rival"),
	
	/** High hostility, grudge racing */
	Bitter           UMETA(DisplayName = "Bitter Enemy")
};

/**
 * Memorable moment type
 * Significant events that define a rivalry
 */
UENUM(BlueprintType)
enum class EMGMemorableMomentType : uint8
{
	/** First encounter */
	FirstMeeting,
	
	/** Close finish (<1 second) */
	CloseFinish,
	
	/** Last-lap overtake */
	LastLapOvertake,
	
	/** Dramatic crash */
	MajorIncident,
	
	/** Dominant victory (>10 second gap) */
	Domination,
	
	/** Pink slip race victory */
	PinkSlipVictory,
	
	/** Pink slip race loss */
	PinkSlipLoss,
	
	/** Came from behind to win */
	ComebackWin,
	
	/** Blocked opponent's overtake */
	DefensiveVictory,
	
	/** Retaliation for past incident */
	Payback,
	
	/** Clean racing in intense battle */
	HonorableRacing,
	
	/** Helped opponent (didn't capitalize on their mistake) */
	Sportsmanship
};

/**
 * Memorable moment data
 * Records a significant event in rivalry history
 */
USTRUCT(BlueprintType)
struct FMGMemorableMoment
{
	GENERATED_BODY()

	/** Type of moment */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGMemorableMomentType MomentType;

	/** Short description */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FString Description;

	/** Track where it occurred */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName TrackID;

	/** Timestamp */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime Timestamp;

	/** Player position in race */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 PlayerPosition = 0;

	/** Rival position in race */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RivalPosition = 0;

	/** Time difference (seconds, positive = rival ahead) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float TimeDifference = 0.0f;

	/** Impact on relationship (-1 to 1) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float RelationshipImpact = 0.0f;
};

/**
 * Rivalry progression data
 * Tracks relationship between player and a specific rival
 */
USTRUCT(BlueprintType)
struct FMGRivalryData
{
	GENERATED_BODY()

	// ==========================================
	// IDENTITY
	// ==========================================

	/** Unique rival identifier */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName RivalID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FText RivalName;

	/** Short name for HUD */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FString ShortName;

	/** Driver profile reference */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMGAIDriverProfile> DriverProfile;

	/** Portrait texture */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> Portrait;

	/** Is this a story rival (vs dynamic) */
	UPROPERTY(BlueprintReadOnly)
	bool bIsStoryRival = false;

	/** Has player unlocked this rival */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bIsUnlocked = false;

	// ==========================================
	// RIVALRY PROGRESSION
	// ==========================================

	/** Rivalry level (1-5, scales with encounters) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RivalryLevel = 1;

	/** Current rivalry status */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	EMGRivalryStatus Status = EMGRivalryStatus::Unknown;

	/** Total encounters */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 TotalRaces = 0;

	/** Player wins against this rival */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 PlayerWins = 0;

	/** Rival wins against player */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 RivalWins = 0;

	/** Close finishes (<1 second) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 CloseFinishes = 0;

	/** Contact incidents */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 ContactIncidents = 0;

	/** Clean races (no contact) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 CleanRaces = 0;

	// ==========================================
	// RELATIONSHIP DIMENSIONS
	// ==========================================

	/** Respect level (0-1, earned through skill) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float Respect = 0.5f;

	/** Hostility level (-1 to 1, affected by incidents) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float Hostility = 0.0f;

	/** Rivalry intensity (0-1, how memorable the rivalry is) */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	float Intensity = 0.0f;

	// ==========================================
	// HISTORY
	// ==========================================

	/** Memorable moments that defined rivalry */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	TArray<FMGMemorableMoment> MemorableMoments;

	/** Most recent encounter timestamp */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FDateTime LastEncounter;

	/** Tracks where rivalry began */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	FName OriginTrack;

	// ==========================================
	// CAMPAIGN INTEGRATION
	// ==========================================

	/** Campaign milestone where rival appears */
	UPROPERTY(BlueprintReadOnly)
	FName UnlockMilestone;

	/** Minimum player level to encounter */
	UPROPERTY(BlueprintReadOnly)
	int32 MinPlayerLevel = 1;

	/** Preferred tracks for encounters */
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> PreferredTracks;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get win rate against this rival (0-1) */
	FORCEINLINE float GetWinRate() const
	{
		if (TotalRaces == 0) return 0.5f;
		return static_cast<float>(PlayerWins) / static_cast<float>(TotalRaces);
	}

	/** Check if this is a nemesis (intense rival) */
	FORCEINLINE bool IsNemesis() const
	{
		return Status == EMGRivalryStatus::Nemesis || 
		       (Intensity > 0.7f && TotalRaces >= 10);
	}

	/** Check if this is a respected rival */
	FORCEINLINE bool IsRespected() const
	{
		return Status == EMGRivalryStatus::Respected || 
		       (Respect > 0.7f && Hostility < 0.3f);
	}

	/** Check if this is a bitter enemy */
	FORCEINLINE bool IsBitter() const
	{
		return Status == EMGRivalryStatus::Bitter || 
		       (Hostility > 0.6f && Respect < 0.4f);
	}

	/** Get relationship description */
	FText GetRelationshipDescription() const;
};

/**
 * Rivalry encounter result
 * Data recorded after racing against a rival
 */
USTRUCT(BlueprintType)
struct FMGRivalryEncounterResult
{
	GENERATED_BODY()

	/** Rival identifier */
	UPROPERTY(BlueprintReadWrite)
	FName RivalID;

	/** Did player win */
	UPROPERTY(BlueprintReadWrite)
	bool bPlayerWon = false;

	/** Finish time difference (seconds) */
	UPROPERTY(BlueprintReadWrite)
	float TimeDifference = 0.0f;

	/** Was it a close race (<1 sec) */
	UPROPERTY(BlueprintReadWrite)
	bool bCloseRace = false;

	/** Number of contact incidents */
	UPROPERTY(BlueprintReadWrite)
	int32 ContactIncidents = 0;

	/** Was racing clean (no contact) */
	UPROPERTY(BlueprintReadWrite)
	bool bCleanRace = true;

	/** Track where encounter occurred */
	UPROPERTY(BlueprintReadWrite)
	FName TrackID;

	/** Did rivalry level up from this encounter */
	UPROPERTY(BlueprintReadOnly)
	bool bRivalryLeveledUp = false;

	/** Generated memorable moments */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGMemorableMoment> NewMemorableMoments;
};

/** Delegate for rivalry events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRivalryProgressed, FName, RivalID, EMGRivalryStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMemorableMomentCreated, FName, RivalID, const FMGMemorableMoment&, Moment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRivalUnlocked, FName, RivalID);

/**
 * Rivalry Subsystem
 * Manages named rivals and relationship progression
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRivalrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// INITIALIZATION
	// ==========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// RIVAL MANAGEMENT
	// ==========================================

	/**
	 * Get rival data by ID
	 * @param RivalID Unique rival identifier
	 * @return Rival data, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	FMGRivalryData* GetRival(FName RivalID);

	/**
	 * Get all unlocked rivals
	 * @return Array of unlocked rivals
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	TArray<FMGRivalryData> GetUnlockedRivals() const;

	/**
	 * Get all story rivals
	 * @return Array of pre-designed story rivals
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	TArray<FMGRivalryData> GetStoryRivals() const;

	/**
	 * Get top rivals by intensity
	 * @param Count Number of rivals to return
	 * @return Top rivals sorted by intensity
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	TArray<FMGRivalryData> GetTopRivals(int32 Count = 5) const;

	/**
	 * Check if player has encountered a rival
	 * @param RivalID Rival to check
	 * @return True if at least one race together
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry System")
	bool HasEncounteredRival(FName RivalID) const;

	/**
	 * Unlock a story rival
	 * @param RivalID Rival to unlock
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	bool UnlockRival(FName RivalID);

	// ==========================================
	// ENCOUNTER RECORDING
	// ==========================================

	/**
	 * Record race encounter with rival
	 * Call after every race involving a named rival
	 * @param RivalID Rival identifier
	 * @param PlayerPosition Player's finish position
	 * @param RivalPosition Rival's finish position
	 * @param PlayerFinishTime Player's finish time
	 * @param RivalFinishTime Rival's finish time
	 * @param ContactIncidents Number of contact events
	 * @param bCleanRace Was race clean (no contact)
	 * @return Encounter result with progression info
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	FMGRivalryEncounterResult RecordRaceEncounter(
		FName RivalID,
		int32 PlayerPosition,
		int32 RivalPosition,
		float PlayerFinishTime,
		float RivalFinishTime,
		int32 ContactIncidents,
		bool bCleanRace
	);

	/**
	 * Record memorable moment manually
	 * Use for scripted story moments
	 * @param RivalID Rival involved
	 * @param Moment Memorable moment data
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	void RecordMemorableMoment(FName RivalID, const FMGMemorableMoment& Moment);

	/**
	 * Check if rivalry just leveled up
	 * Check this after RecordRaceEncounter
	 * @param RivalID Rival to check
	 * @return True if leveled up in last encounter
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry System")
	bool DidRivalryLevelUp(FName RivalID) const;

	// ==========================================
	// AI BEHAVIOR MODIFICATION
	// ==========================================

	/**
	 * Apply rivalry-based modifiers to AI profile
	 * Adjusts behavior based on relationship history
	 * @param Profile Profile to modify
	 * @param RivalData Rivalry data
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	void ApplyRivalryModifiersToProfile(
		UMGAIDriverProfile* Profile,
		const FMGRivalryData& RivalData
	);

	/**
	 * Get recommended aggression adjustment for rival
	 * Based on hostility and relationship
	 * @param RivalID Rival identifier
	 * @return Aggression adjustment (-0.3 to +0.3)
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	float GetRivalAggressionAdjustment(FName RivalID) const;

	// ==========================================
	// DYNAMIC RIVALRY GENERATION
	// ==========================================

	/**
	 * Create dynamic rivalry from repeated encounters
	 * Auto-generates rival from AI opponent after 3+ races
	 * @param OpponentProfile AI profile of frequent opponent
	 * @param EncounterCount Number of times faced
	 * @return Generated rival ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	FName CreateDynamicRivalry(
		UMGAIDriverProfile* OpponentProfile,
		int32 EncounterCount
	);

	// ==========================================
	// QUERIES
	// ==========================================

	/**
	 * Get win rate against rival
	 * @param RivalID Rival identifier
	 * @return Win rate (0-1), or 0.5 if no data
	 */
	UFUNCTION(BlueprintPure, Category = "Rivalry System")
	float GetWinRateAgainstRival(FName RivalID) const;

	/**
	 * Get most recent memorable moment with rival
	 * @param RivalID Rival identifier
	 * @param OutMoment [out] Most recent moment
	 * @return True if moment found
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	bool GetMostRecentMoment(FName RivalID, FMGMemorableMoment& OutMoment) const;

	/**
	 * Get rivalry story summary
	 * Generates text summary of rivalry history
	 * @param RivalID Rival identifier
	 * @return Story summary
	 */
	UFUNCTION(BlueprintCallable, Category = "Rivalry System")
	FText GetRivalryStorySummary(FName RivalID) const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Fired when rivalry status changes */
	UPROPERTY(BlueprintAssignable, Category = "Rivalry System|Events")
	FOnRivalryProgressed OnRivalryProgressed;

	/** Fired when memorable moment is created */
	UPROPERTY(BlueprintAssignable, Category = "Rivalry System|Events")
	FOnMemorableMomentCreated OnMemorableMomentCreated;

	/** Fired when story rival is unlocked */
	UPROPERTY(BlueprintAssignable, Category = "Rivalry System|Events")
	FOnRivalUnlocked OnRivalUnlocked;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** All rivalry data (saved) */
	UPROPERTY(SaveGame)
	TMap<FName, FMGRivalryData> Rivalries;

	/** Story rival configurations (loaded from data assets) */
	UPROPERTY()
	TArray<TObjectPtr<UDataAsset>> StoryRivalAssets;

	/** Rivalry level-up tracking (for querying after encounter) */
	UPROPERTY()
	TMap<FName, bool> RecentLevelUps;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Load story rival definitions */
	void LoadStoryRivals();

	/** Update rivalry status based on current metrics */
	void UpdateRivalryStatus(FMGRivalryData& Rivalry);

	/** Calculate respect change from encounter */
	float CalculateRespectChange(const FMGRivalryEncounterResult& Result) const;

	/** Calculate hostility change from encounter */
	float CalculateHostilityChange(const FMGRivalryEncounterResult& Result) const;

	/** Calculate intensity change from encounter */
	float CalculateIntensityChange(const FMGRivalryEncounterResult& Result) const;

	/** Detect and create memorable moments from encounter */
	TArray<FMGMemorableMoment> DetectMemorableMoments(
		const FMGRivalryEncounterResult& Result,
		const FMGRivalryData& Rivalry
	) const;

	/** Generate text description for moment */
	FString GenerateMomentDescription(
		EMGMemorableMomentType MomentType,
		const FMGRivalryEncounterResult& Result
	) const;
};
