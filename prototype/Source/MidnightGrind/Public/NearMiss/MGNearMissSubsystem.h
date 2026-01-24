// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNearMissSubsystem.generated.h"

/**
 * Type of near miss encounter
 */
UENUM(BlueprintType)
enum class EMGNearMissType : uint8
{
	Vehicle			UMETA(DisplayName = "Vehicle Near Miss"),
	Traffic			UMETA(DisplayName = "Traffic Near Miss"),
	Oncoming		UMETA(DisplayName = "Oncoming Traffic"),
	Pedestrian		UMETA(DisplayName = "Pedestrian Close Call"),
	Obstacle		UMETA(DisplayName = "Obstacle Near Miss"),
	Wall			UMETA(DisplayName = "Wall Graze"),
	Barrier			UMETA(DisplayName = "Barrier Scrape"),
	Cliff			UMETA(DisplayName = "Cliff Edge"),
	Train			UMETA(DisplayName = "Train Dodge"),
	Police			UMETA(DisplayName = "Police Evade")
};

/**
 * Near miss quality tier
 */
UENUM(BlueprintType)
enum class EMGNearMissQuality : uint8
{
	Basic			UMETA(DisplayName = "Close"),
	Good			UMETA(DisplayName = "Very Close"),
	Great			UMETA(DisplayName = "Dangerously Close"),
	Perfect			UMETA(DisplayName = "Hair's Breadth"),
	Insane			UMETA(DisplayName = "Impossible")
};

/**
 * Style point category
 */
UENUM(BlueprintType)
enum class EMGStyleCategory : uint8
{
	NearMiss		UMETA(DisplayName = "Near Miss"),
	Drift			UMETA(DisplayName = "Drift"),
	Air				UMETA(DisplayName = "Air Time"),
	Speed			UMETA(DisplayName = "High Speed"),
	Combo			UMETA(DisplayName = "Combo"),
	Skill			UMETA(DisplayName = "Skill Move")
};

/**
 * Combo state
 */
UENUM(BlueprintType)
enum class EMGComboState : uint8
{
	Inactive		UMETA(DisplayName = "Inactive"),
	Building		UMETA(DisplayName = "Building"),
	Active			UMETA(DisplayName = "Active"),
	Frenzy			UMETA(DisplayName = "Frenzy"),
	Expiring		UMETA(DisplayName = "Expiring"),
	Banked			UMETA(DisplayName = "Banked")
};

/**
 * Individual near miss event
 */
USTRUCT(BlueprintType)
struct FMGNearMissEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissType MissType = EMGNearMissType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissQuality Quality = EMGNearMissQuality::Basic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MultipliedPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasOncoming = false;
};

/**
 * Distance thresholds for near miss quality
 */
USTRUCT(BlueprintType)
struct FMGNearMissThresholds
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissType MissType = EMGNearMissType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasicDistance = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodDistance = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatDistance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectDistance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InsaneDistance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InsaneMultiplier = 5.0f;
};

/**
 * Active combo data
 */
USTRUCT(BlueprintType)
struct FMGActiveCombo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGComboState State = EMGComboState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGNearMissEvent> ComboEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxComboReached = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasDrift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasOncoming = false;
};

/**
 * Style bonus modifier
 */
USTRUCT(BlueprintType)
struct FMGStyleBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BonusName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStyleCategory Category = EMGStyleCategory::NearMiss;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FlatBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDrift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresOncoming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCombo = 0;
};

/**
 * Style session statistics
 */
USTRUCT(BlueprintType)
struct FMGStyleSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalNearMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalStylePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestSingleEvent = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClosestDistance = FLT_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGNearMissType, int32> NearMissByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGNearMissQuality, int32> NearMissByQuality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InsaneMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OncomingMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftNearMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AirNearMisses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeDrifting = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalAirTime = 0.0f;
};

/**
 * Near miss reward configuration
 */
USTRUCT(BlueprintType)
struct FMGNearMissRewards
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashPerPoint = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationPerPoint = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroPerPerfectMiss = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBoostOnInsane = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBoostDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusCashOnFrenzy = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboMilestoneBonus = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ComboMilestones;
};

/**
 * Player proximity data for detection
 */
USTRUCT(BlueprintType)
struct FMGProximityTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNearMissType TargetType = EMGNearMissType::Vehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoundingRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDistance = FLT_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClosestApproach = FLT_MAX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsApproaching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearMissTriggered = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastUpdateTime = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearMissOccurred, const FMGNearMissEvent&, Event, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnComboUpdated, int32, ComboCount, float, Multiplier, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboBanked, int32, FinalCombo, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboLost, int32, LostPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrenzyActivated, int32, ComboCount, float, Multiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMilestoneReached, int32, Milestone, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStyleBonusApplied, const FMGStyleBonus&, Bonus, int32, BonusPoints);

/**
 * Near Miss Subsystem
 * Manages arcade-style near miss detection, scoring, and combos
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNearMissSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnNearMissOccurred OnNearMissOccurred;

	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnComboUpdated OnComboUpdated;

	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnComboBanked OnComboBanked;

	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnComboLost OnComboLost;

	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnFrenzyActivated OnFrenzyActivated;

	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnMilestoneReached OnMilestoneReached;

	UPROPERTY(BlueprintAssignable, Category = "NearMiss|Events")
	FOnStyleBonusApplied OnStyleBonusApplied;

	// Detection
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void RegisterProximityTarget(const FMGProximityTarget& Target);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void UnregisterProximityTarget(const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void UpdateProximityTarget(const FString& TargetId, FVector NewLocation, FVector NewVelocity);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void UpdatePlayerState(FVector PlayerLocation, FVector PlayerVelocity, bool bIsDrifting, bool bIsAirborne);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void ProcessProximityCheck();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Detection")
	void ClearAllTargets();

	// Near Miss Registration
	UFUNCTION(BlueprintCallable, Category = "NearMiss")
	FMGNearMissEvent RegisterNearMiss(EMGNearMissType MissType, float Distance, float Speed, const FString& TargetId);

	UFUNCTION(BlueprintCallable, Category = "NearMiss")
	EMGNearMissQuality CalculateQuality(EMGNearMissType MissType, float Distance) const;

	UFUNCTION(BlueprintCallable, Category = "NearMiss")
	int32 CalculateBasePoints(EMGNearMissType MissType, EMGNearMissQuality Quality, float Speed) const;

	// Combo Management
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void ExtendCombo(const FMGNearMissEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void BankCombo();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void LoseCombo();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Combo")
	void ResetCombo();

	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	FMGActiveCombo GetActiveCombo() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	bool IsComboActive() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	float GetComboTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Combo")
	float GetComboMultiplier() const;

	// Thresholds
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Config")
	void SetThresholds(EMGNearMissType MissType, const FMGNearMissThresholds& Thresholds);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Config")
	FMGNearMissThresholds GetThresholds(EMGNearMissType MissType) const;

	// Style Bonuses
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Style")
	void RegisterStyleBonus(const FMGStyleBonus& Bonus);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Style")
	TArray<FMGStyleBonus> GetApplicableBonuses(const FMGNearMissEvent& Event) const;

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Style")
	int32 ApplyStyleBonuses(FMGNearMissEvent& Event);

	// Rewards
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	void SetRewardConfig(const FMGNearMissRewards& Config);

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	FMGNearMissRewards GetRewardConfig() const;

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	int32 CalculateCashReward(int32 StylePoints) const;

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	float CalculateReputationReward(int32 StylePoints) const;

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Rewards")
	float CalculateNitroBonus(const FMGNearMissEvent& Event) const;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void EndSession();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void PauseSession();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Session")
	void ResumeSession();

	UFUNCTION(BlueprintPure, Category = "NearMiss|Session")
	bool IsSessionActive() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Session")
	FMGStyleSessionStats GetSessionStats() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	int32 GetTotalStylePoints() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	int32 GetTotalNearMisses() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	int32 GetBestCombo() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	float GetClosestNearMiss() const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Stats")
	TArray<FMGNearMissEvent> GetRecentEvents(int32 Count) const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "NearMiss|Utility")
	FText GetQualityDisplayText(EMGNearMissQuality Quality) const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Utility")
	FText GetMissTypeDisplayText(EMGNearMissType MissType) const;

	UFUNCTION(BlueprintPure, Category = "NearMiss|Utility")
	FLinearColor GetQualityColor(EMGNearMissQuality Quality) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "NearMiss|Persistence")
	void SaveNearMissData();

	UFUNCTION(BlueprintCallable, Category = "NearMiss|Persistence")
	void LoadNearMissData();

protected:
	void TickCombo(float DeltaTime);
	void CheckMilestones();
	void CheckFrenzyState();
	float CalculateComboMultiplier(int32 ComboCount) const;
	void InitializeDefaultThresholds();
	void InitializeDefaultBonuses();

private:
	UPROPERTY()
	TMap<EMGNearMissType, FMGNearMissThresholds> NearMissThresholds;

	UPROPERTY()
	TMap<FString, FMGStyleBonus> RegisteredBonuses;

	UPROPERTY()
	FMGActiveCombo ActiveCombo;

	UPROPERTY()
	FMGStyleSessionStats SessionStats;

	UPROPERTY()
	FMGNearMissRewards RewardConfig;

	UPROPERTY()
	TMap<FString, FMGProximityTarget> ProximityTargets;

	UPROPERTY()
	TArray<FMGNearMissEvent> RecentEvents;

	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	FVector PlayerVelocity;

	UPROPERTY()
	bool bPlayerDrifting = false;

	UPROPERTY()
	bool bPlayerAirborne = false;

	UPROPERTY()
	bool bSessionActive = false;

	UPROPERTY()
	bool bSessionPaused = false;

	UPROPERTY()
	int32 NextMilestoneIndex = 0;

	FTimerHandle ComboTickTimer;

	static constexpr int32 MaxRecentEvents = 100;
};
