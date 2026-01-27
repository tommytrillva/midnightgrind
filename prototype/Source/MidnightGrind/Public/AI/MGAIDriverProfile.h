// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MGAIDriverProfile.generated.h"

/**
 * AI driver personality type
 */
UENUM(BlueprintType)
enum class EMGDriverPersonality : uint8
{
	/** Aggressive - takes risks, late braking, forces overtakes */
	Aggressive,
	/** Defensive - protects position, blocks overtakes */
	Defensive,
	/** Calculated - optimal lines, consistent pace */
	Calculated,
	/** Unpredictable - erratic behavior, mistakes */
	Unpredictable,
	/** Rookie - slower, more mistakes, wider lines */
	Rookie,
	/** Veteran - fast, consistent, good racecraft */
	Veteran,
	/** Rival - targets player specifically */
	Rival
};

/**
 * AI skill parameters
 */
USTRUCT(BlueprintType)
struct FMGAISkillParams
{
	GENERATED_BODY()

	/** Overall skill level (0-1, affects all other parameters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SkillLevel = 0.7f;

	/** Braking accuracy (how close to optimal braking point) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BrakingAccuracy = 0.7f;

	/** Racing line accuracy (how close to optimal line) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LineAccuracy = 0.7f;

	/** Reaction time (lower is better, in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float ReactionTime = 0.3f;

	/** Consistency (how stable the performance is) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Consistency = 0.8f;

	/** Mistake frequency (lower is better) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MistakeFrequency = 0.1f;

	/** Recovery skill (how quickly they recover from mistakes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RecoverySkill = 0.7f;

	/** Corner exit speed (how well they accelerate out of corners) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CornerExitSpeed = 0.7f;
};

/**
 * Contact response type - how AI reacts to being hit
 */
UENUM(BlueprintType)
enum class EMGContactResponse : uint8
{
	/** Ignore minor contact, focus on racing */
	Ignore,
	/** Back off to avoid further damage */
	BackOff,
	/** Increase aggression against the offender */
	Retaliate,
	/** Go into survival mode, protect the car */
	Protect,
	/** Match the aggressor's behavior */
	Mirror,
	/** Report to race stewards (if applicable) */
	Report
};

/**
 * Contact event data - records who hit whom
 */
USTRUCT(BlueprintType)
struct FMGAIContactEvent
{
	GENERATED_BODY()

	/** Actor that made contact */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Offender;

	/** Was this player-caused */
	UPROPERTY(BlueprintReadOnly)
	bool bWasPlayer = false;

	/** Impact severity (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float Severity = 0.0f;

	/** Time of contact */
	UPROPERTY(BlueprintReadOnly)
	float TimeStamp = 0.0f;

	/** Number of contacts from this offender */
	UPROPERTY(BlueprintReadOnly)
	int32 ContactCount = 1;

	/** Was this intentional (based on their trajectory) */
	UPROPERTY(BlueprintReadOnly)
	bool bSeemedIntentional = false;
};

/**
 * Aggression escalation stage
 */
UENUM(BlueprintType)
enum class EMGAggressionStage : uint8
{
	/** Normal racing behavior */
	Baseline,
	/** Slightly elevated - racing harder */
	Elevated,
	/** High aggression - taking more risks */
	High,
	/** Maximum aggression - win at all costs */
	Maximum,
	/** Rage mode - making mistakes from over-aggression */
	Rage
};

/**
 * Personality behavior modifiers - unique quirks per personality type
 */
USTRUCT(BlueprintType)
struct FMGPersonalityBehaviors
{
	GENERATED_BODY()

	/** Brake point adjustment (-1 = early, +1 = late) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float BrakePointBias = 0.0f;

	/** Line preference (-1 = inside, +1 = outside) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float LineBias = 0.0f;

	/** How much they weave/move on straights (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StraightLineWeaving = 0.0f;

	/** Tendency to use bump-drafting (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BumpDraftingTendency = 0.0f;

	/** Chance to feint before making a move (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FeintChance = 0.0f;

	/** Willingness to go side-by-side through corners (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SideBySideWillingness = 0.5f;

	/** Tendency to push others wide (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PushWideTendency = 0.0f;

	/** Tendency to chop across opponent's nose (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChopTendency = 0.0f;

	/** How much they adjust based on opponent skill (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AdaptToOpponentSkill = 0.5f;

	/** Special move probability (dive bombs, last-second passes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpecialMoveProbability = 0.1f;
};

/**
 * AI aggression parameters
 */
USTRUCT(BlueprintType)
struct FMGAIAggressionParams
{
	GENERATED_BODY()

	/** Overall aggression (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Aggression = 0.5f;

	/** Overtake aggression (how hard they push to pass) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OvertakeAggression = 0.5f;

	/** Defense aggression (how hard they defend) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefenseAggression = 0.5f;

	/** Willingness to take risks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiskTaking = 0.5f;

	/** How close they're willing to get to other cars */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ProximityTolerance = 0.5f;

	/** Patience before attempting overtake (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float OvertakePatience = 3.0f;

	/** Will specifically target/block player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTargetsPlayer = false;

	// ==========================================
	// NEW: Aggression Escalation System
	// ==========================================

	/** How quickly aggression escalates when provoked (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EscalationRate = 0.5f;

	/** How quickly aggression de-escalates when left alone (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DeescalationRate = 0.3f;

	/** Threshold to enter high aggression (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HighAggressionThreshold = 0.7f;

	/** Threshold to enter maximum aggression (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxAggressionThreshold = 0.9f;

	/** Can this AI enter rage mode (makes mistakes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanEnterRageMode = false;

	// ==========================================
	// NEW: Contact Response System
	// ==========================================

	/** How AI responds to minor contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGContactResponse MinorContactResponse = EMGContactResponse::Ignore;

	/** How AI responds to major contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGContactResponse MajorContactResponse = EMGContactResponse::Retaliate;

	/** Contact severity threshold for "major" (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MajorContactThreshold = 0.5f;

	/** How long AI remembers contact grudges (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "120.0"))
	float GrudgeMemoryDuration = 30.0f;

	/** Retaliation intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float RetaliationIntensity = 1.0f;

	// ==========================================
	// NEW: Dirty Driving Thresholds
	// ==========================================

	/** Willingness to make contact to defend (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefenseContactWillingness = 0.2f;

	/** Willingness to make contact to attack (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackContactWillingness = 0.1f;

	/** Only goes dirty when losing (preserves sportsmanship when ahead) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnlyDirtyWhenLosing = true;

	/** Position threshold where dirty tactics become acceptable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 DirtyTacticsPositionThreshold = 4;
};

/**
 * AI racecraft parameters
 */
USTRUCT(BlueprintType)
struct FMGAIRacecraftParams
{
	GENERATED_BODY()

	/** Awareness of surroundings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Awareness = 0.7f;

	/** Ability to predict other drivers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Anticipation = 0.7f;

	/** Strategic thinking (tire management, fuel, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Strategy = 0.5f;

	/** Ability to manage traffic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TrafficManagement = 0.7f;

	/** Slipstream awareness and usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlipstreamUsage = 0.5f;

	/** Ability to find and exploit gaps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GapFinding = 0.6f;

	/** Preferred overtake side (0 = either, -1 = left, 1 = right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float PreferredOvertakeSide = 0.0f;
};

/**
 * AI weather sensitivity parameters
 * Controls how AI adapts to various weather conditions
 */
USTRUCT(BlueprintType)
struct FMGAIWeatherParams
{
	GENERATED_BODY()

	/**
	 * Overall weather adaptation skill (0-1)
	 * Higher values = better performance in adverse conditions
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeatherAdaptation = 0.5f;

	/**
	 * Wet weather driving skill (0-1)
	 * Affects grip utilization and throttle control in rain
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WetWeatherSkill = 0.5f;

	/**
	 * Night driving skill (0-1)
	 * Affects confidence and speed in low visibility
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NightDrivingSkill = 0.5f;

	/**
	 * Fog driving skill (0-1)
	 * Affects perception and reaction time in fog
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FogDrivingSkill = 0.5f;

	/**
	 * Wind compensation skill (0-1)
	 * Affects ability to counter crosswind effects
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindCompensation = 0.5f;

	/**
	 * Caution multiplier in bad weather (0.5-2.0)
	 * Lower = more cautious, Higher = more aggressive despite conditions
	 * 1.0 = standard caution, 0.5 = very cautious, 2.0 = ignores weather risks
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float WeatherRiskTolerance = 1.0f;

	/**
	 * Aquaplaning recovery skill (0-1)
	 * How well the AI recovers from losing grip in puddles
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AquaplaningRecovery = 0.5f;

	/**
	 * Get effective skill modifier for current weather
	 * @param WeatherDifficulty Weather difficulty (1-5)
	 * @return Skill modifier (0.5-1.2)
	 */
	float GetWeatherSkillModifier(int32 WeatherDifficulty) const
	{
		// Higher adaptation = less penalty in bad weather
		const float DifficultyFactor = (WeatherDifficulty - 1) / 4.0f; // 0-1
		const float Penalty = DifficultyFactor * (1.0f - WeatherAdaptation) * 0.4f;
		return FMath::Clamp(1.0f - Penalty, 0.5f, 1.2f);
	}

	/**
	 * Get caution adjustment for weather
	 * @param WeatherDifficulty Weather difficulty (1-5)
	 * @return Caution multiplier for speed calculations
	 */
	float GetWeatherCautionMultiplier(int32 WeatherDifficulty) const
	{
		if (WeatherDifficulty <= 1)
		{
			return 1.0f; // No caution needed in clear weather
		}

		// Base caution increases with difficulty
		const float BaseCaution = 0.05f * (WeatherDifficulty - 1);

		// Risk tolerance affects how much caution is applied
		const float AdjustedCaution = BaseCaution / WeatherRiskTolerance;

		return 1.0f - FMath::Clamp(AdjustedCaution, 0.0f, 0.3f);
	}
};

/**
 * AI speed parameters
 */
USTRUCT(BlueprintType)
struct FMGAISpeedParams
{
	GENERATED_BODY()

	/** Base speed percentage (0.8 = 80% of optimal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.1"))
	float BaseSpeedPercent = 0.9f;

	/** Speed boost when behind (rubber banding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float CatchUpBoost = 0.1f;

	/** Speed reduction when ahead (rubber banding) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.2"))
	float SlowDownAmount = 0.05f;

	/** Corner speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.7", ClampMax = "1.1"))
	float CornerSpeedMultiplier = 0.95f;

	/** Straight speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.1"))
	float StraightSpeedMultiplier = 1.0f;

	/** NOS usage frequency (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NOSUsageFrequency = 0.5f;

	/** Drift speed penalty tolerance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DriftTolerance = 0.3f;
};

/**
 * AI mood that affects behavior during race
 */
UENUM(BlueprintType)
enum class EMGAIMood : uint8
{
	/** Normal racing behavior */
	Neutral,
	/** More aggressive, took risks */
	Frustrated,
	/** Playing it safe, confident */
	Confident,
	/** Desperate, makes mistakes */
	Desperate,
	/** Focused, optimal performance */
	InTheZone,
	/** Intimidated by player */
	Intimidated,
	/** Seeking revenge */
	Vengeful
};

/**
 * Adaptive learning data - how AI learns from races against player
 */
USTRUCT(BlueprintType)
struct FMGAIAdaptiveData
{
	GENERATED_BODY()

	/** Times raced against player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacesAgainstPlayer = 0;

	/** Wins against player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WinsAgainstPlayer = 0;

	/** Losses against player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LossesAgainstPlayer = 0;

	/** Track-specific skill adjustments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> TrackSkillModifiers;

	/** Learned player tendencies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerAggressionEstimate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerBrakingPointEstimate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerOvertakePreference = 0.0f; // -1 left, +1 right

	/** Performance improvement over time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillGrowthFactor = 0.0f;

	/** Get effective skill multiplier based on learning */
	float GetLearningMultiplier() const
	{
		// AI improves slightly with experience
		return 1.0f + FMath::Clamp(SkillGrowthFactor, 0.0f, 0.1f);
	}

	/** Get win rate against player */
	float GetWinRateAgainstPlayer() const
	{
		if (RacesAgainstPlayer == 0) return 0.5f;
		return static_cast<float>(WinsAgainstPlayer) / static_cast<float>(RacesAgainstPlayer);
	}
};

/**
 * Rival relationship data
 */
USTRUCT(BlueprintType)
struct FMGRivalRelationship
{
	GENERATED_BODY()

	/** Rivalry intensity (-1 = friendly, +1 = hostile) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Intensity = 0.0f;

	/** Respect level (affects behavior) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Respect = 0.5f;

	/** History events that shaped rivalry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RivalryHistory;

	/** Last race result against this rival */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LastRacePosition = 0; // 0 = no race yet

	/** Pink slips won from rival */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PinkSlipsWon = 0;

	/** Pink slips lost to rival */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PinkSlipsLost = 0;

	/** Is this an active nemesis */
	bool IsNemesis() const { return Intensity > 0.7f && Respect > 0.5f; }

	/** Is this a friendly rival */
	bool IsFriendlyRival() const { return Intensity < -0.3f || Respect > 0.8f; }
};

/**
 * AI Driver Profile
 * Defines personality, skill, and behavior of AI racers
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAIDriverProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// ==========================================
	// IDENTITY
	// ==========================================

	/** Driver name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DriverName;

	/** Driver short name (for HUD) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString ShortName;

	/** Driver nationality */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString Nationality;

	/** Driver portrait */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	UTexture2D* Portrait;

	/** Personality type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMGDriverPersonality Personality = EMGDriverPersonality::Calculated;

	/** Driver backstory/bio */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Bio;

	// ==========================================
	// PARAMETERS
	// ==========================================

	/** Skill parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	FMGAISkillParams Skill;

	/** Aggression parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	FMGAIAggressionParams Aggression;

	/** Racecraft parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	FMGAIRacecraftParams Racecraft;

	/** Speed parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	FMGAISpeedParams Speed;

	/** Weather sensitivity parameters */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	FMGAIWeatherParams Weather;

	// ==========================================
	// PREFERENCES
	// ==========================================

	/** Preferred vehicle class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preferences")
	FName PreferredVehicleClass;

	/** Preferred vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preferences")
	FName PreferredVehicle;

	/** Preferred vehicle color */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preferences")
	FLinearColor PreferredColor = FLinearColor::White;

	/** Preferred tracks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Preferences")
	TArray<FName> PreferredTracks;

	// ==========================================
	// AUDIO
	// ==========================================

	/** Radio chatter voice ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	FName VoiceID;

	/** Chatter frequency (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ChatterFrequency = 0.5f;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get overall rating (0-100) */
	UFUNCTION(BlueprintPure, Category = "Utility")
	int32 GetOverallRating() const;

	/** Get speed rating (0-100) */
	UFUNCTION(BlueprintPure, Category = "Utility")
	int32 GetSpeedRating() const;

	/** Get aggression rating (0-100) */
	UFUNCTION(BlueprintPure, Category = "Utility")
	int32 GetAggressionRating() const;

	/** Get consistency rating (0-100) */
	UFUNCTION(BlueprintPure, Category = "Utility")
	int32 GetConsistencyRating() const;

	/** Create modified profile with difficulty adjustment */
	UFUNCTION(BlueprintCallable, Category = "Utility")
	void ApplyDifficultyModifier(float Modifier);

	// ==========================================
	// PERSONALITY BEHAVIORS
	// ==========================================

	/** Personality-specific behavior modifiers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality")
	FMGPersonalityBehaviors PersonalityBehaviors;

	// ==========================================
	// ADAPTIVE BEHAVIOR
	// ==========================================

	/** Adaptive learning data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive")
	FMGAIAdaptiveData AdaptiveData;

	/** Current mood (affects behavior in race) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
	EMGAIMood CurrentMood = EMGAIMood::Neutral;

	/** Rival relationship with player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive")
	FMGRivalRelationship PlayerRivalry;

	// ==========================================
	// RUNTIME AGGRESSION STATE
	// ==========================================

	/** Current aggression escalation stage */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	EMGAggressionStage CurrentAggressionStage = EMGAggressionStage::Baseline;

	/** Current accumulated aggression level (0-1) */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	float AccumulatedAggression = 0.0f;

	/** Recent contact events for grudge tracking */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TArray<FMGAIContactEvent> RecentContacts;

	/** Current grudge target (who we're mad at) */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TWeakObjectPtr<AActor> CurrentGrudgeTarget;

	/** Time in current aggression stage */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	float TimeInAggressionStage = 0.0f;

	/** Is currently in battle mode (fighting for position) */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	bool bInBattleMode = false;

	/** Current battle opponent */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TWeakObjectPtr<AActor> BattleOpponent;

	/** Update adaptive data after race */
	UFUNCTION(BlueprintCallable, Category = "Adaptive")
	void RecordRaceResult(bool bWon, FName TrackID, float FinishTimeDelta);

	/** Update mood based on race events */
	UFUNCTION(BlueprintCallable, Category = "Adaptive")
	void UpdateMood(float PositionDelta, float DamageReceived, bool bWasOvertaken);

	/** Get effective skill with adaptive modifiers */
	UFUNCTION(BlueprintPure, Category = "Adaptive")
	float GetEffectiveSkill() const;

	/** Get effective aggression with mood modifiers */
	UFUNCTION(BlueprintPure, Category = "Adaptive")
	float GetEffectiveAggression() const;

	/** Learn from player behavior */
	UFUNCTION(BlueprintCallable, Category = "Adaptive")
	void LearnPlayerBehavior(float ObservedAggression, float ObservedBraking, float OvertakeSide);

	/** Get predicted player behavior */
	UFUNCTION(BlueprintPure, Category = "Adaptive")
	void GetPredictedPlayerBehavior(float& OutAggression, float& OutBrakingPoint, float& OutOvertakeSide) const;

	/** Update rivalry based on race outcome */
	UFUNCTION(BlueprintCallable, Category = "Adaptive")
	void UpdateRivalry(bool bPlayerWon, bool bWasPinkSlip, const FString& EventDescription);

	// ==========================================
	// AGGRESSION ESCALATION SYSTEM
	// ==========================================

	/**
	 * Record a contact event
	 * @param Offender Who hit us
	 * @param Severity Impact severity (0-1)
	 * @param bWasPlayer Was this caused by the player
	 * @param bSeemedIntentional Did it seem deliberate
	 */
	UFUNCTION(BlueprintCallable, Category = "Aggression")
	void RecordContact(AActor* Offender, float Severity, bool bWasPlayer, bool bSeemedIntentional);

	/**
	 * Update aggression state (call each frame during race)
	 * @param DeltaTime Frame delta time
	 * @param CurrentPosition Current race position
	 * @param bUnderPressure Is there an opponent close behind
	 * @param bApplyingPressure Is there an opponent close ahead we're chasing
	 */
	UFUNCTION(BlueprintCallable, Category = "Aggression")
	void UpdateAggressionState(float DeltaTime, int32 CurrentPosition, bool bUnderPressure, bool bApplyingPressure);

	/**
	 * Get current effective aggression including escalation
	 * @return Effective aggression level (0-1, can exceed base)
	 */
	UFUNCTION(BlueprintPure, Category = "Aggression")
	float GetEscalatedAggression() const;

	/**
	 * Get the appropriate contact response for given severity
	 * @param Severity Contact severity (0-1)
	 * @return Response type
	 */
	UFUNCTION(BlueprintPure, Category = "Aggression")
	EMGContactResponse GetContactResponse(float Severity) const;

	/**
	 * Check if we have a grudge against a specific actor
	 * @param Actor Actor to check
	 * @return True if we're holding a grudge
	 */
	UFUNCTION(BlueprintPure, Category = "Aggression")
	bool HasGrudgeAgainst(AActor* Actor) const;

	/**
	 * Get grudge intensity against a specific actor (0-1)
	 * @param Actor Actor to check
	 * @return Grudge intensity, 0 if no grudge
	 */
	UFUNCTION(BlueprintPure, Category = "Aggression")
	float GetGrudgeIntensity(AActor* Actor) const;

	/**
	 * Check if dirty tactics are acceptable in current situation
	 * @param CurrentPosition Current race position
	 * @param bIsDefending True if defending position
	 * @return True if willing to use dirty tactics
	 */
	UFUNCTION(BlueprintPure, Category = "Aggression")
	bool WillUseDirtyTactics(int32 CurrentPosition, bool bIsDefending) const;

	/**
	 * Enter battle mode with opponent
	 * @param Opponent The opponent we're battling
	 */
	UFUNCTION(BlueprintCallable, Category = "Aggression")
	void EnterBattleMode(AActor* Opponent);

	/**
	 * Exit battle mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Aggression")
	void ExitBattleMode();

	/**
	 * Reset all aggression state (call at race start)
	 */
	UFUNCTION(BlueprintCallable, Category = "Aggression")
	void ResetAggressionState();

	/**
	 * Get personality-adjusted behavior values
	 * @param OutBehaviors Output behavior modifiers with personality applied
	 */
	UFUNCTION(BlueprintPure, Category = "Personality")
	FMGPersonalityBehaviors GetEffectivePersonalityBehaviors() const;

	/**
	 * Should this AI feint before making a move
	 * @return True if should feint this time
	 */
	UFUNCTION(BlueprintPure, Category = "Personality")
	bool ShouldFeint() const;

	/**
	 * Get special move probability based on situation
	 * @param bIsFinalLap Is this the final lap
	 * @param bIsForPosition Is this for a meaningful position
	 * @return Probability (0-1) of attempting special move
	 */
	UFUNCTION(BlueprintPure, Category = "Personality")
	float GetSpecialMoveProbability(bool bIsFinalLap, bool bIsForPosition) const;

protected:
	/** Clean up expired grudges */
	void CleanupExpiredGrudges(float CurrentTime);

	/** Calculate aggression stage from accumulated level */
	EMGAggressionStage CalculateAggressionStage(float AggressionLevel) const;

	/** Apply personality defaults to behavior struct */
	void ApplyPersonalityDefaults();
};

/**
 * AI Driver Roster
 * Collection of available AI drivers
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAIDriverRoster : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All available drivers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	TArray<UMGAIDriverProfile*> Drivers;

	/** Get random drivers */
	UFUNCTION(BlueprintCallable, Category = "Roster")
	TArray<UMGAIDriverProfile*> GetRandomDrivers(int32 Count, float MinSkill = 0.0f, float MaxSkill = 1.0f) const;

	/** Get drivers by personality */
	UFUNCTION(BlueprintCallable, Category = "Roster")
	TArray<UMGAIDriverProfile*> GetDriversByPersonality(EMGDriverPersonality Personality) const;

	/** Get driver by name */
	UFUNCTION(BlueprintCallable, Category = "Roster")
	UMGAIDriverProfile* GetDriverByName(const FString& Name) const;

	/** Get drivers for skill bracket */
	UFUNCTION(BlueprintCallable, Category = "Roster")
	TArray<UMGAIDriverProfile*> GetDriversForBracket(int32 PlayerRating, int32 Variance = 10) const;
};
