// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MG_AI_TierSystem.h
 * @brief AI Tier-Based Progression System
 *
 * @section overview Overview
 * This system implements tier-based AI progression that scales with the player's
 * car tier advancement (Street Beaters → Tuner Cars → Super Cars → Hypercars → Legends).
 * AI opponents become progressively more skilled, consistent, and challenging as
 * the player progresses through the 150-200h single-player campaign.
 *
 * The tier system applies multipliers and modifiers to base UMGAIDriverProfile
 * configurations, creating distinct skill brackets without requiring separate
 * profiles for each tier.
 *
 * @section concepts Key Concepts
 *
 * @subsection tiers AI Tiers
 * Five distinct tiers matching car progression:
 * - **Street Beaters (Tier 1):** Novice AI, high mistakes, basic tactics
 * - **Tuner Cars (Tier 2):** Intermediate AI, learning advanced techniques
 * - **Super Cars (Tier 3):** Skilled AI, rare mistakes, strong racecraft
 * - **Hypercars (Tier 4):** Expert AI, minimal errors, elite tactics
 * - **Legends (Tier 5):** Legendary AI, near-perfect execution, ultimate challenge
 *
 * @subsection modifiers Tier Modifiers
 * Each tier defines multipliers for:
 * - Skill level (braking, line accuracy, reaction time)
 * - Speed (base speed percentage)
 * - Consistency (mistake frequency reduction)
 * - Aggression (overtaking behavior)
 * - Racecraft (awareness, anticipation, strategy)
 *
 * @section usage Usage Examples
 *
 * @subsection spawn_tiered Spawning Tiered AI
 * @code
 * // Get tier modifiers for current progression
 * UMGAITierSubsystem* TierSystem = GetGameInstance()->GetSubsystem<UMGAITierSubsystem>();
 * FMGAITierModifiers Modifiers = TierSystem->GetTierModifiers(EMGAITier::SuperCars);
 *
 * // Apply to driver profile
 * UMGAIDriverProfile* Profile = LoadDriverProfile();
 * TierSystem->ApplyTierModifiers(Profile, EMGAITier::SuperCars);
 *
 * // Spawn AI with tiered profile
 * SpawnAIOpponent(Profile, SpawnTransform);
 * @endcode
 *
 * @subsection query_tier Querying Tier Requirements
 * @code
 * // Check if player has unlocked tier
 * bool bUnlocked = TierSystem->IsTierUnlocked(EMGAITier::Hypercars);
 *
 * // Get recommended tier for current progression
 * EMGAITier RecommendedTier = TierSystem->GetRecommendedTierForPlayer();
 *
 * // Get difficulty range within tier
 * float MinDifficulty, MaxDifficulty;
 * TierSystem->GetTierDifficultyRange(EMGAITier::TunerCars, MinDifficulty, MaxDifficulty);
 * @endcode
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AI/MG_AI_DriverProfile.h"
#include "MG_AI_TierSystem.generated.h"

/**
 * AI tier enumeration
 * Matches car tier progression in single-player campaign
 */
UENUM(BlueprintType)
enum class EMGAITier : uint8
{
	/** Tier 1: Street Beaters (0-20h) - Novice opponents */
	StreetBeaters    UMETA(DisplayName = "Street Beaters"),
	
	/** Tier 2: Tuner Cars (20-60h) - Intermediate opponents */
	TunerCars        UMETA(DisplayName = "Tuner Cars"),
	
	/** Tier 3: Super Cars (60-120h) - Skilled opponents */
	SuperCars        UMETA(DisplayName = "Super Cars"),
	
	/** Tier 4: Hypercars (120-180h) - Expert opponents */
	Hypercars        UMETA(DisplayName = "Hypercars"),
	
	/** Tier 5: Legends (180-200h) - Legendary opponents */
	Legends          UMETA(DisplayName = "Legends")
};

/**
 * Tier-based profile modifiers
 * Applied to base driver profiles to create tier-appropriate AI
 */
USTRUCT(BlueprintType)
struct FMGAITierModifiers
{
	GENERATED_BODY()

	// ==========================================
	// SKILL MODIFIERS
	// ==========================================

	/** Skill level multiplier (0.5-1.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.5", ClampMax = "1.2"))
	float SkillMultiplier = 1.0f;

	/** Braking accuracy multiplier (0.5-1.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.5", ClampMax = "1.3"))
	float BrakingAccuracyMultiplier = 1.0f;

	/** Line accuracy multiplier (0.5-1.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.5", ClampMax = "1.3"))
	float LineAccuracyMultiplier = 1.0f;

	/** Reaction time multiplier (0.7-1.5, higher = slower reactions) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.7", ClampMax = "1.5"))
	float ReactionTimeMultiplier = 1.0f;

	/** Corner exit speed multiplier (0.6-1.2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill", meta = (ClampMin = "0.6", ClampMax = "1.2"))
	float CornerExitSpeedMultiplier = 1.0f;

	// ==========================================
	// CONSISTENCY MODIFIERS
	// ==========================================

	/** Consistency boost (0.0-0.3, additive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consistency", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ConsistencyBoost = 0.0f;

	/** Mistake frequency reduction (0.0-0.8, subtractive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consistency", meta = (ClampMin = "0.0", ClampMax = "0.8"))
	float MistakeReduction = 0.0f;

	/** Recovery skill boost (0.0-0.3, additive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consistency", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float RecoverySkillBoost = 0.0f;

	// ==========================================
	// SPEED MODIFIERS
	// ==========================================

	/** Base speed multiplier (0.75-1.1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.75", ClampMax = "1.1"))
	float SpeedMultiplier = 1.0f;

	/** Corner speed multiplier (0.7-1.15) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.7", ClampMax = "1.15"))
	float CornerSpeedMultiplier = 1.0f;

	/** Straight speed multiplier (0.85-1.1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed", meta = (ClampMin = "0.85", ClampMax = "1.1"))
	float StraightSpeedMultiplier = 1.0f;

	// ==========================================
	// AGGRESSION MODIFIERS
	// ==========================================

	/** Aggression adjustment (-0.2 to +0.3, additive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression", meta = (ClampMin = "-0.2", ClampMax = "0.3"))
	float AggressionAdjustment = 0.0f;

	/** Overtake aggression multiplier (0.6-1.4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression", meta = (ClampMin = "0.6", ClampMax = "1.4"))
	float OvertakeAggressionMultiplier = 1.0f;

	/** Risk-taking adjustment (-0.2 to +0.3, additive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aggression", meta = (ClampMin = "-0.2", ClampMax = "0.3"))
	float RiskTakingAdjustment = 0.0f;

	// ==========================================
	// RACECRAFT MODIFIERS
	// ==========================================

	/** Awareness multiplier (0.5-1.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racecraft", meta = (ClampMin = "0.5", ClampMax = "1.3"))
	float AwarenessMultiplier = 1.0f;

	/** Anticipation multiplier (0.5-1.3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racecraft", meta = (ClampMin = "0.5", ClampMax = "1.3"))
	float AnticipationMultiplier = 1.0f;

	/** Strategic thinking multiplier (0.4-1.4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racecraft", meta = (ClampMin = "0.4", ClampMax = "1.4"))
	float StrategyMultiplier = 1.0f;

	/** Slipstream usage multiplier (0.3-1.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racecraft", meta = (ClampMin = "0.3", ClampMax = "1.5"))
	float SlipstreamUsageMultiplier = 1.0f;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get display name for these modifiers */
	FORCEINLINE FText GetTierName() const
	{
		// Inferred from modifiers (ugly but functional)
		if (SkillMultiplier < 0.7f)
			return FText::FromString(TEXT("Street Beaters"));
		if (SkillMultiplier < 0.85f)
			return FText::FromString(TEXT("Tuner Cars"));
		if (SkillMultiplier < 0.95f)
			return FText::FromString(TEXT("Super Cars"));
		if (SkillMultiplier < 1.05f)
			return FText::FromString(TEXT("Hypercars"));
		return FText::FromString(TEXT("Legends"));
	}
};

/**
 * Tier unlock requirements
 * Defines what player must achieve to face this tier
 */
USTRUCT(BlueprintType)
struct FMGAITierUnlockRequirements
{
	GENERATED_BODY()

	/** Minimum player level required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MinPlayerLevel = 1;

	/** Minimum playtime hours required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	float MinPlaytimeHours = 0.0f;

	/** Required campaign progress percentage (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	float RequiredCampaignProgress = 0.0f;

	/** Minimum car tier player must own */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	int32 MinCarTier = 1;

	/** Specific locations that must be unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
	TArray<FName> RequiredLocations;

	/** Check if requirements are met */
	bool AreMet(int32 PlayerLevel, float PlaytimeHours, float CampaignProgress, int32 PlayerCarTier) const
	{
		return PlayerLevel >= MinPlayerLevel &&
		       PlaytimeHours >= MinPlaytimeHours &&
		       CampaignProgress >= RequiredCampaignProgress &&
		       PlayerCarTier >= MinCarTier;
	}
};

/**
 * Tier configuration data asset
 * Defines all parameters for a specific AI tier
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAITierProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Tier identifier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	EMGAITier Tier = EMGAITier::StreetBeaters;

	/** Tier display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	FText TierName;

	/** Tier description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier", meta = (MultiLine = true))
	FText Description;

	/** Modifiers to apply to driver profiles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	FMGAITierModifiers Modifiers;

	/** Unlock requirements */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier")
	FMGAITierUnlockRequirements UnlockRequirements;

	/** Expected difficulty range (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinDifficulty = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxDifficulty = 1.0f;

	/** Recommended opponent count for this tier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tier", meta = (ClampMin = "1", ClampMax = "20"))
	int32 RecommendedOpponentCount = 7;
};

/**
 * AI Tier Subsystem
 * Manages tier-based AI progression throughout campaign
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAITierSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ==========================================
	// INITIALIZATION
	// ==========================================

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// TIER QUERIES
	// ==========================================

	/**
	 * Get tier modifiers for specified tier
	 * @param Tier The AI tier to get modifiers for
	 * @return Tier modifiers struct
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	FMGAITierModifiers GetTierModifiers(EMGAITier Tier) const;

	/**
	 * Get tier profile data asset
	 * @param Tier The AI tier
	 * @return Tier profile asset, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	UMGAITierProfile* GetTierProfile(EMGAITier Tier) const;

	/**
	 * Get recommended tier based on player progression
	 * Considers player level, playtime, campaign progress, car tier
	 * @return Recommended AI tier for next race
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	EMGAITier GetRecommendedTierForPlayer() const;

	/**
	 * Check if player has unlocked a specific tier
	 * @param Tier The tier to check
	 * @return True if tier is unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	bool IsTierUnlocked(EMGAITier Tier) const;

	/**
	 * Get difficulty range for tier
	 * @param Tier The tier
	 * @param OutMinDifficulty [out] Minimum difficulty
	 * @param OutMaxDifficulty [out] Maximum difficulty
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	void GetTierDifficultyRange(EMGAITier Tier, float& OutMinDifficulty, float& OutMaxDifficulty) const;

	// ==========================================
	// TIER APPLICATION
	// ==========================================

	/**
	 * Apply tier modifiers to a driver profile
	 * Modifies profile in-place
	 * @param Profile The driver profile to modify
	 * @param Tier The tier to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	void ApplyTierModifiers(UMGAIDriverProfile* Profile, EMGAITier Tier);

	/**
	 * Create a tier-modified copy of a profile
	 * Does not modify original profile
	 * @param SourceProfile Profile to copy from
	 * @param Tier Tier to apply
	 * @return New profile with tier modifiers applied
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	UMGAIDriverProfile* CreateTieredProfile(UMGAIDriverProfile* SourceProfile, EMGAITier Tier);

	/**
	 * Generate random AI field for tier
	 * Creates varied opponents within tier's difficulty range
	 * @param Tier The tier
	 * @param Count Number of AI opponents to generate
	 * @return Array of AI profiles
	 */
	UFUNCTION(BlueprintCallable, Category = "AI Tier System")
	TArray<UMGAIDriverProfile*> GenerateAIFieldForTier(EMGAITier Tier, int32 Count);

	// ==========================================
	// UTILITY
	// ==========================================

	/**
	 * Get tier name as text
	 * @param Tier The tier
	 * @return Display name
	 */
	UFUNCTION(BlueprintPure, Category = "AI Tier System")
	FText GetTierName(EMGAITier Tier) const;

	/**
	 * Get tier description
	 * @param Tier The tier
	 * @return Description text
	 */
	UFUNCTION(BlueprintPure, Category = "AI Tier System")
	FText GetTierDescription(EMGAITier Tier) const;

	/**
	 * Convert tier to integer index (0-4)
	 * @param Tier The tier
	 * @return Index value
	 */
	UFUNCTION(BlueprintPure, Category = "AI Tier System")
	static int32 TierToIndex(EMGAITier Tier) { return static_cast<int32>(Tier); }

	/**
	 * Convert integer index to tier (0-4)
	 * @param Index The index
	 * @return Tier enum value
	 */
	UFUNCTION(BlueprintPure, Category = "AI Tier System")
	static EMGAITier IndexToTier(int32 Index) { return static_cast<EMGAITier>(FMath::Clamp(Index, 0, 4)); }

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Tier profile data assets (loaded from config) */
	UPROPERTY()
	TMap<EMGAITier, TObjectPtr<UMGAITierProfile>> TierProfiles;

	/** Cache of default tier modifiers (loaded from data assets) */
	UPROPERTY()
	TMap<EMGAITier, FMGAITierModifiers> DefaultModifiers;

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Load tier profile data assets */
	void LoadTierProfiles();

	/** Apply modifiers to skill parameters */
	void ApplySkillModifiers(FMGAISkillParams& Skill, const FMGAITierModifiers& Modifiers) const;

	/** Apply modifiers to speed parameters */
	void ApplySpeedModifiers(FMGAISpeedParams& Speed, const FMGAITierModifiers& Modifiers) const;

	/** Apply modifiers to aggression parameters */
	void ApplyAggressionModifiers(FMGAIAggressionParams& Aggression, const FMGAITierModifiers& Modifiers) const;

	/** Apply modifiers to racecraft parameters */
	void ApplyRacecraftModifiers(FMGAIRacecraftParams& Racecraft, const FMGAITierModifiers& Modifiers) const;
};
