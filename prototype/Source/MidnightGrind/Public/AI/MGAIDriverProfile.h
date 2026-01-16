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
