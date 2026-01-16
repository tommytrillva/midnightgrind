// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLiveEventsSubsystem.generated.h"

class UTexture2D;

/**
 * Event type
 */
UENUM(BlueprintType)
enum class EMGEventType : uint8
{
	/** Weekend special event */
	Weekend,
	/** Weekly challenge */
	Weekly,
	/** Daily challenge */
	Daily,
	/** Limited time event */
	LimitedTime,
	/** Community goal */
	CommunityGoal,
	/** Holiday event */
	Holiday,
	/** Collaboration event */
	Collaboration,
	/** Flash event (few hours) */
	Flash
};

/**
 * Event status
 */
UENUM(BlueprintType)
enum class EMGEventStatus : uint8
{
	/** Not yet started */
	Upcoming,
	/** Currently active */
	Active,
	/** Ending soon */
	EndingSoon,
	/** Completed */
	Completed,
	/** Expired (missed) */
	Expired
};

/**
 * Challenge type
 */
UENUM(BlueprintType)
enum class EMGChallengeType : uint8
{
	/** Win races */
	WinRaces,
	/** Complete races */
	CompleteRaces,
	/** Achieve position */
	AchievePosition,
	/** Beat lap time */
	BeatLapTime,
	/** Drive distance */
	DriveDistance,
	/** Reach top speed */
	ReachTopSpeed,
	/** Drift distance */
	DriftDistance,
	/** Near misses */
	NearMisses,
	/** Overtakes */
	Overtakes,
	/** Use specific vehicle */
	UseVehicle,
	/** Race on specific track */
	RaceOnTrack,
	/** Win streak */
	WinStreak,
	/** Perfect laps */
	PerfectLaps,
	/** Earn currency */
	EarnCurrency,
	/** Community total */
	CommunityTotal
};

/**
 * Event reward
 */
USTRUCT(BlueprintType)
struct FMGEventReward
{
	GENERATED_BODY()

	/** Reward ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	/** Reward type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardType;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Quantity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	/** Is exclusive to event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsExclusive = false;

	/** Rarity color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RarityColor = FLinearColor::White;
};

/**
 * Challenge objective
 */
USTRUCT(BlueprintType)
struct FMGChallengeObjective
{
	GENERATED_BODY()

	/** Objective ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType Type = EMGChallengeType::CompleteRaces;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Target value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetValue = 1;

	/** Current progress */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentValue = 0;

	/** Required track (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	/** Required vehicle (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;

	/** Required weather (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredWeather;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;
};

/**
 * Event challenge
 */
USTRUCT(BlueprintType)
struct FMGEventChallenge
{
	GENERATED_BODY()

	/** Challenge ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengeID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGChallengeObjective> Objectives;

	/** Rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> Rewards;

	/** XP reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPReward = 0;

	/** Is completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	/** Is claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsClaimed = false;

	/** Difficulty (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Difficulty = 1;

	/** Sort order */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

/**
 * Community goal data
 */
USTRUCT(BlueprintType)
struct FMGCommunityGoal
{
	GENERATED_BODY()

	/** Goal ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GoalID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGChallengeType Type = EMGChallengeType::CommunityTotal;

	/** Community target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CommunityTarget = 1000000;

	/** Current community progress */
	UPROPERTY(BlueprintReadOnly)
	int64 CommunityProgress = 0;

	/** Player's contribution */
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerContribution = 0;

	/** Tier thresholds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int64> TierThresholds;

	/** Tier rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> TierRewards;

	/** Current tier reached */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentTier = 0;

	/** Is goal completed */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;
};

/**
 * Playlist entry
 */
USTRUCT(BlueprintType)
struct FMGPlaylistEntry
{
	GENERATED_BODY()

	/** Track ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Weather override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeatherOverride;

	/** Time of day override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TimeOfDayOverride;

	/** Lap count override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 0;

	/** Vehicle restrictions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AllowedVehicles;

	/** Is reverse */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReverse = false;
};

/**
 * Featured playlist
 */
USTRUCT(BlueprintType)
struct FMGFeaturedPlaylist
{
	GENERATED_BODY()

	/** Playlist ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/** Playlist entries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPlaylistEntry> Entries;

	/** XP multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float XPMultiplier = 1.0f;

	/** Cash multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashMultiplier = 1.0f;

	/** Is featured */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;
};

/**
 * Live event data
 */
USTRUCT(BlueprintType)
struct FMGLiveEvent
{
	GENERATED_BODY()

	/** Event ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	/** Event type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEventType Type = EMGEventType::Weekly;

	/** Status */
	UPROPERTY(BlueprintReadOnly)
	EMGEventStatus Status = EMGEventStatus::Upcoming;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Banner image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* BannerImage = nullptr;

	/** Theme color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ThemeColor = FLinearColor::White;

	/** Start time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	/** End time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	/** Challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventChallenge> Challenges;

	/** Community goals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCommunityGoal> CommunityGoals;

	/** Featured playlists */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGFeaturedPlaylist> FeaturedPlaylists;

	/** Completion rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventReward> CompletionRewards;

	/** Required level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	/** Total event XP earned */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalXPEarned = 0;

	/** Is participated */
	UPROPERTY(BlueprintReadOnly)
	bool bHasParticipated = false;
};

/**
 * Daily challenge set
 */
USTRUCT(BlueprintType)
struct FMGDailyChallenges
{
	GENERATED_BODY()

	/** Date for these challenges */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Date;

	/** Challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEventChallenge> Challenges;

	/** Bonus reward for completing all */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEventReward BonusReward;

	/** All completed */
	UPROPERTY(BlueprintReadOnly)
	bool bAllCompleted = false;

	/** Bonus claimed */
	UPROPERTY(BlueprintReadOnly)
	bool bBonusClaimed = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventStarted, const FMGLiveEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventEnded, const FMGLiveEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChallengeProgress, FName, ChallengeID, const FMGEventChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeCompleted, const FMGEventChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityGoalProgress, FName, GoalID, const FMGCommunityGoal&, Goal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCommunityGoalTierReached, FName, GoalID, int32, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDailyChallengesRefreshed);

/**
 * Live Events Subsystem
 * Manages time-limited events, challenges, and community goals
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLiveEventsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime);

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventStarted OnEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEventEnded OnEventEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeProgress OnChallengeProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChallengeCompleted OnChallengeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCommunityGoalProgress OnCommunityGoalProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCommunityGoalTierReached OnCommunityGoalTierReached;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDailyChallengesRefreshed OnDailyChallengesRefreshed;

	// ==========================================
	// LIVE EVENTS
	// ==========================================

	/** Get all active events */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	TArray<FMGLiveEvent> GetActiveEvents() const;

	/** Get upcoming events */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	TArray<FMGLiveEvent> GetUpcomingEvents() const;

	/** Get event by ID */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	FMGLiveEvent GetEventByID(FName EventID) const;

	/** Is event active */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	bool IsEventActive(FName EventID) const;

	/** Get time remaining for event */
	UFUNCTION(BlueprintPure, Category = "Live Events")
	FTimespan GetEventTimeRemaining(FName EventID) const;

	/** Refresh events from server */
	UFUNCTION(BlueprintCallable, Category = "Live Events")
	void RefreshEvents();

	// ==========================================
	// CHALLENGES
	// ==========================================

	/** Get challenge progress */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	FMGEventChallenge GetChallengeProgress(FName EventID, FName ChallengeID) const;

	/** Report challenge progress */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	void ReportChallengeProgress(EMGChallengeType Type, int32 Value, FName TrackID = NAME_None, FName VehicleID = NAME_None);

	/** Claim challenge reward */
	UFUNCTION(BlueprintCallable, Category = "Challenges")
	bool ClaimChallengeReward(FName EventID, FName ChallengeID);

	/** Get unclaimed challenges */
	UFUNCTION(BlueprintPure, Category = "Challenges")
	TArray<FMGEventChallenge> GetUnclaimedChallenges() const;

	// ==========================================
	// DAILY CHALLENGES
	// ==========================================

	/** Get today's daily challenges */
	UFUNCTION(BlueprintPure, Category = "Daily")
	FMGDailyChallenges GetDailyChallenges() const;

	/** Get daily challenge streak */
	UFUNCTION(BlueprintPure, Category = "Daily")
	int32 GetDailyStreak() const { return DailyStreak; }

	/** Claim daily bonus reward */
	UFUNCTION(BlueprintCallable, Category = "Daily")
	bool ClaimDailyBonusReward();

	// ==========================================
	// COMMUNITY GOALS
	// ==========================================

	/** Get community goal progress */
	UFUNCTION(BlueprintPure, Category = "Community")
	FMGCommunityGoal GetCommunityGoalProgress(FName EventID, FName GoalID) const;

	/** Contribute to community goal */
	UFUNCTION(BlueprintCallable, Category = "Community")
	void ContributeToCommunityGoal(FName EventID, FName GoalID, int32 Contribution);

	/** Get community goal percentage */
	UFUNCTION(BlueprintPure, Category = "Community")
	float GetCommunityGoalPercentage(FName EventID, FName GoalID) const;

	// ==========================================
	// PLAYLISTS
	// ==========================================

	/** Get featured playlists */
	UFUNCTION(BlueprintPure, Category = "Playlists")
	TArray<FMGFeaturedPlaylist> GetFeaturedPlaylists() const;

	/** Get playlist by ID */
	UFUNCTION(BlueprintPure, Category = "Playlists")
	FMGFeaturedPlaylist GetPlaylistByID(FName PlaylistID) const;

	/** Get current playlist multipliers */
	UFUNCTION(BlueprintPure, Category = "Playlists")
	void GetPlaylistMultipliers(FName PlaylistID, float& OutXPMultiplier, float& OutCashMultiplier) const;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get event type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetEventTypeDisplayName(EMGEventType Type);

	/** Get event status display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetEventStatusDisplayName(EMGEventStatus Status);

	/** Get challenge type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetChallengeTypeDisplayName(EMGChallengeType Type);

	/** Format time remaining */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatTimeRemaining(FTimespan TimeRemaining);

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** All events */
	UPROPERTY()
	TArray<FMGLiveEvent> AllEvents;

	/** Daily challenges */
	UPROPERTY()
	FMGDailyChallenges DailyChallenges;

	/** Daily streak */
	int32 DailyStreak = 0;

	/** Last daily completion date */
	FDateTime LastDailyCompletion;

	/** Featured playlists */
	UPROPERTY()
	TArray<FMGFeaturedPlaylist> FeaturedPlaylists;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update event statuses */
	void UpdateEventStatuses();

	/** Check for expired events */
	void CheckExpiredEvents();

	/** Generate daily challenges */
	void GenerateDailyChallenges();

	/** Check challenge completion */
	void CheckChallengeCompletion(FMGEventChallenge& Challenge);

	/** Check community goal tiers */
	void CheckCommunityGoalTiers(FMGCommunityGoal& Goal, FName EventID);

	/** Update objective progress */
	void UpdateObjectiveProgress(FMGChallengeObjective& Objective, EMGChallengeType Type, int32 Value, FName TrackID, FName VehicleID);

	/** Create mock events (for testing) */
	void CreateMockEvents();

	/** Save progress */
	void SaveProgress();

	/** Load progress */
	void LoadProgress();
};
