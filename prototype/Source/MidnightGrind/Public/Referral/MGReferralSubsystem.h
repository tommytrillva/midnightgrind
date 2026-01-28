// Copyright Midnight Grind. All Rights Reserved.

/*******************************************************************************
 * MGReferralSubsystem.h - Player Referral and Invitation System
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This header defines the Referral Subsystem - an "invite a friend" feature that
 * rewards players for bringing new racers into Midnight Grind. Think of it like
 * a rewards program: when you share your special code with friends and they join,
 * BOTH of you get prizes!
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. SUBSYSTEMS (UGameInstanceSubsystem):
 *    - A subsystem is a "manager" class that Unreal Engine automatically creates
 *    - UGameInstanceSubsystem exists for the entire game session (persists between levels)
 *    - You access it via: GameInstance->GetSubsystem<UMGReferralSubsystem>()
 *    - No need to spawn or manage its lifetime - Unreal handles that!
 *
 * 2. REFERRAL CODES:
 *    - Each player gets a unique code they can share (like "RACER123")
 *    - New players enter this code when they start playing
 *    - The system tracks who referred whom and awards bonuses
 *
 * 3. MILESTONES & TIERS:
 *    - As the new player progresses (completes tutorial, wins races, etc.),
 *      BOTH players unlock more rewards
 *    - The more friends you successfully refer, the higher your "tier" (Bronze->Silver->Gold, etc.)
 *    - Higher tiers give access to exclusive cosmetics and bonuses
 *
 * 4. BLUEPRINT MACROS:
 *    - UFUNCTION(BlueprintCallable) = Can be called from Blueprints (visual scripting)
 *    - UFUNCTION(BlueprintPure) = Like above, but doesn't modify anything (getter)
 *    - UPROPERTY(BlueprintReadWrite) = Variable accessible from Blueprints
 *    - UPROPERTY(BlueprintAssignable) = Event/delegate that Blueprints can listen to
 *
 * 5. DELEGATES (FOn...):
 *    - These are "events" that broadcast when something happens
 *    - Example: OnReferralComplete fires when a referred player finishes all milestones
 *    - Other systems (like UI) can "subscribe" to these events to react accordingly
 *
 * HOW TO USE THIS SYSTEM:
 * -----------------------
 * 1. Get a player's referral code: GetReferralLink() or GetMyReferralCode()
 * 2. New player applies a code: ApplyReferralCode("FRIEND123")
 * 3. Check for rewards: GetPendingRewards()
 * 4. Collect rewards: ClaimAllRewards()
 * 5. Listen for events: Bind to OnReferralRewardAvailable delegate
 *
 * FILE STRUCTURE:
 * ---------------
 * - Enums (EMG...): Define fixed sets of options (status types, reward types, tiers)
 * - Structs (FMG...): Data containers holding related information
 * - Delegates: Event declarations for the observer pattern
 * - Class: The actual subsystem with all the functions
 *
 ******************************************************************************/

/**
 * @file MGReferralSubsystem.h
 * @brief Player referral and invitation system for Midnight Grind
 *
 * The Referral Subsystem manages the "invite a friend" feature, rewarding players
 * who bring new racers into the game. Both the referrer and the referred player
 * receive rewards as the new player progresses through milestones.
 *
 * @section referral_overview_sec System Overview
 * When a player shares their referral code with a friend:
 * 1. Friend applies the code during registration or first login
 * 2. Both players receive initial "welcome" rewards
 * 3. As the new player hits milestones (level 5, first win, etc.), more rewards unlock
 * 4. Completed referrals contribute to the referrer's tier progression
 *
 * @section referral_tiers_sec Referral Tiers
 * Players earn higher tiers by successfully referring more friends:
 * - **Bronze**: 1-2 completed referrals
 * - **Silver**: 3-5 referrals (bonus currency multiplier)
 * - **Gold**: 6-10 referrals (exclusive cosmetics)
 * - **Platinum**: 11-25 referrals (unique vehicles)
 * - **Diamond**: 26-50 referrals (premium rewards)
 * - **Ambassador**: 50+ referrals (special title, exclusive perks)
 *
 * @section referral_milestones_sec Progress Milestones
 * Referrals progress through these status stages:
 * - Pending: Code applied, awaiting account creation
 * - Registered: Account created
 * - FirstLogin: Completed first game session
 * - TutorialComplete: Finished the tutorial
 * - ReachedLevel5/10: Hit level milestones
 * - FirstWin: Won their first race
 * - PurchasedPremium: Made a real-money purchase (optional bonus)
 *
 * @section referral_usage_sec Example Usage
 * @code
 * UMGReferralSubsystem* ReferralSub = GameInstance->GetSubsystem<UMGReferralSubsystem>();
 *
 * // Get your referral code to share
 * FString Link = ReferralSub->GetReferralLink();
 * ReferralSub->CopyReferralCodeToClipboard();
 *
 * // New player applies a friend's code
 * ReferralSub->ApplyReferralCode(TEXT("FRIEND123"));
 *
 * // Check available rewards
 * TArray<FMGReferralReward> Pending = ReferralSub->GetPendingRewards();
 * ReferralSub->ClaimAllRewards();
 * @endcode
 *
 * @see UMGSocialSubsystem For adding referred players as friends
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGReferralSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGReferralStatus : uint8
{
	Pending,
	Registered,
	FirstLogin,
	TutorialComplete,
	ReachedLevel5,
	ReachedLevel10,
	FirstWin,
	PurchasedPremium,
	Claimed,
	Expired
};

UENUM(BlueprintType)
enum class EMGReferralRewardType : uint8
{
	Currency,
	Vehicle,
	Part,
	Cosmetic,
	XPBoost,
	CurrencyBoost,
	PremiumTime,
	UniqueTitle,
	UniqueLivery,
	ExclusiveDecal
};

UENUM(BlueprintType)
enum class EMGReferralTier : uint8
{
	Bronze,      // 1-2 referrals
	Silver,      // 3-5 referrals
	Gold,        // 6-10 referrals
	Platinum,    // 11-25 referrals
	Diamond,     // 26-50 referrals
	Ambassador   // 50+ referrals
};

USTRUCT(BlueprintType)
struct FMGReferralReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RewardID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReferralRewardType Type = EMGReferralRewardType::Currency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForReferrer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForReferred = true;
};

USTRUCT(BlueprintType)
struct FMGReferralMilestone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MilestoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsClaimed = false;
};

USTRUCT(BlueprintType)
struct FMGReferralCode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Code;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiryTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentUses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> BonusRewards;
};

USTRUCT(BlueprintType)
struct FMGReferredPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReferralStatus Status = EMGReferralStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ReferredTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastProgressTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPurchased = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> PendingRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGReferralReward> ClaimedRewards;
};

USTRUCT(BlueprintType)
struct FMGReferralStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletedReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PendingReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExpiredReferrals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGReferralTier CurrentTier = EMGReferralTier::Bronze;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReferralsToNextTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCurrencyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalItemsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstReferralTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastReferralTime;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralCodeGenerated, const FMGReferralCode&, Code);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralCodeApplied, const FMGReferralCode&, Code);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReferralCodeInvalid, const FString&, Code, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewReferral, const FMGReferredPlayer&, ReferredPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReferralProgressUpdated, const FString&, PlayerID, EMGReferralStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralComplete, const FMGReferredPlayer&, ReferredPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralRewardAvailable, const FMGReferralReward&, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReferralRewardClaimed, const FMGReferralReward&, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneReached, const FMGReferralMilestone&, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierChanged, EMGReferralTier, OldTier, EMGReferralTier, NewTier);

UCLASS()
class MIDNIGHTGRIND_API UMGReferralSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Referral Codes
	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	FMGReferralCode GenerateReferralCode();

	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	FMGReferralCode GenerateCustomCode(const FString& CustomCode, int32 MaxUses = 0);

	UFUNCTION(BlueprintPure, Category = "Referral|Code")
	FMGReferralCode GetMyReferralCode() const { return MyReferralCode; }

	UFUNCTION(BlueprintPure, Category = "Referral|Code")
	FString GetReferralLink() const;

	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	void CopyReferralCodeToClipboard();

	UFUNCTION(BlueprintCallable, Category = "Referral|Code")
	void ShareReferralCode();

	// Applying Codes
	UFUNCTION(BlueprintCallable, Category = "Referral|Apply")
	bool ApplyReferralCode(const FString& Code);

	UFUNCTION(BlueprintPure, Category = "Referral|Apply")
	bool HasAppliedReferralCode() const { return bHasAppliedCode; }

	UFUNCTION(BlueprintPure, Category = "Referral|Apply")
	FMGReferralCode GetAppliedReferralCode() const { return AppliedCode; }

	UFUNCTION(BlueprintCallable, Category = "Referral|Apply")
	bool ValidateCode(const FString& Code);

	// Referral Tracking
	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	TArray<FMGReferredPlayer> GetReferredPlayers() const { return ReferredPlayers; }

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	TArray<FMGReferredPlayer> GetReferredPlayersByStatus(EMGReferralStatus Status) const;

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	FMGReferralStats GetReferralStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	EMGReferralTier GetCurrentTier() const { return Stats.CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Referral|Track")
	float GetTierProgress() const;

	// Rewards
	UFUNCTION(BlueprintPure, Category = "Referral|Rewards")
	TArray<FMGReferralReward> GetPendingRewards() const { return PendingRewards; }

	UFUNCTION(BlueprintPure, Category = "Referral|Rewards")
	int32 GetPendingRewardCount() const { return PendingRewards.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Referral|Rewards")
	bool ClaimReward(FName RewardID);

	UFUNCTION(BlueprintCallable, Category = "Referral|Rewards")
	void ClaimAllRewards();

	// Milestones
	UFUNCTION(BlueprintPure, Category = "Referral|Milestones")
	TArray<FMGReferralMilestone> GetMilestones() const { return Milestones; }

	UFUNCTION(BlueprintPure, Category = "Referral|Milestones")
	FMGReferralMilestone GetNextMilestone() const;

	UFUNCTION(BlueprintPure, Category = "Referral|Milestones")
	int32 GetReferralsToNextMilestone() const;

	UFUNCTION(BlueprintCallable, Category = "Referral|Milestones")
	bool ClaimMilestoneReward(int32 MilestoneIndex);

	// Progress Reporting (for referred player)
	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportTutorialComplete();

	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportLevelReached(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportFirstWin();

	UFUNCTION(BlueprintCallable, Category = "Referral|Progress")
	void ReportPurchase();

	// Social
	UFUNCTION(BlueprintCallable, Category = "Referral|Social")
	void InviteFriend(const FString& FriendID);

	UFUNCTION(BlueprintCallable, Category = "Referral|Social")
	void InviteFriendByEmail(const FString& Email);

	UFUNCTION(BlueprintPure, Category = "Referral|Social")
	TArray<FString> GetInvitedFriends() const { return InvitedFriends; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralCodeGenerated OnReferralCodeGenerated;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralCodeApplied OnReferralCodeApplied;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralCodeInvalid OnReferralCodeInvalid;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnNewReferral OnNewReferral;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralProgressUpdated OnReferralProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralComplete OnReferralComplete;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralRewardAvailable OnReferralRewardAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnReferralRewardClaimed OnReferralRewardClaimed;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnMilestoneReached OnMilestoneReached;

	UPROPERTY(BlueprintAssignable, Category = "Referral|Events")
	FOnTierChanged OnTierChanged;

protected:
	void InitializeMilestones();
	void UpdateStats();
	void UpdateTier();
	void CheckMilestones();
	void GrantReward(const FMGReferralReward& Reward);
	FString GenerateCodeString() const;
	EMGReferralTier CalculateTier(int32 ReferralCount) const;

	UPROPERTY()
	FMGReferralCode MyReferralCode;

	UPROPERTY()
	FMGReferralCode AppliedCode;

	UPROPERTY()
	bool bHasAppliedCode = false;

	UPROPERTY()
	TArray<FMGReferredPlayer> ReferredPlayers;

	UPROPERTY()
	FMGReferralStats Stats;

	UPROPERTY()
	TArray<FMGReferralReward> PendingRewards;

	UPROPERTY()
	TArray<FMGReferralMilestone> Milestones;

	UPROPERTY()
	TArray<FString> InvitedFriends;

	FString BaseReferralURL = TEXT("https://midnightgrind.com/invite/");
};
