// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGABTestingSubsystem.generated.h"

/**
 * A/B Testing System - Feature Experimentation
 * - Server-controlled feature flags
 * - User segmentation for test groups
 * - Metric tracking per experiment
 * - Gradual rollout support
 * - Override capability for QA
 */

UENUM(BlueprintType)
enum class EMGExperimentStatus : uint8
{
	Draft,
	Running,
	Paused,
	Completed,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGVariantType : uint8
{
	Control,
	VariantA,
	VariantB,
	VariantC,
	VariantD
};

USTRUCT(BlueprintType)
struct FMGExperimentVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVariantType Type = EMGVariantType::Control;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VariantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AllocationPercent = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Parameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParticipantCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConversionCount = 0;
};

USTRUCT(BlueprintType)
struct FMGExperiment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExperimentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExperimentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGExperimentStatus Status = EMGExperimentStatus::Draft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGExperimentVariant> Variants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetMetric;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinSampleSize = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConfidenceLevel = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresOptIn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> TargetSegments;
};

USTRUCT(BlueprintType)
struct FMGFeatureFlag
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FlagID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FlagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RolloutPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EnabledSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EnabledUserIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Configuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FMGUserSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SegmentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SegmentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Criteria;
};

USTRUCT(BlueprintType)
struct FMGExperimentAssignment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExperimentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVariantType AssignedVariant = EMGVariantType::Control;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AssignedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExposed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConverted = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnExperimentAssigned, const FString&, ExperimentID, EMGVariantType, Variant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnFeatureFlagChanged, const FString&, FlagID, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnConfigurationRefreshed, int32, FlagsUpdated);

UCLASS()
class MIDNIGHTGRIND_API UMGABTestingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Feature Flags
	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	bool IsFeatureEnabled(const FString& FlagID) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	FString GetFeatureConfig(const FString& FlagID, const FString& Key, const FString& DefaultValue = TEXT("")) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	int32 GetFeatureConfigInt(const FString& FlagID, const FString& Key, int32 DefaultValue = 0) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	float GetFeatureConfigFloat(const FString& FlagID, const FString& Key, float DefaultValue = 0.0f) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	TArray<FMGFeatureFlag> GetAllFeatureFlags() const { return FeatureFlags; }

	UFUNCTION(BlueprintCallable, Category = "ABTesting|Flags")
	void RefreshFeatureFlags();

	// Experiments
	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	EMGVariantType GetExperimentVariant(const FString& ExperimentID) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	bool IsInExperiment(const FString& ExperimentID) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	FString GetExperimentParam(const FString& ExperimentID, const FString& ParamKey, const FString& DefaultValue = TEXT("")) const;

	UFUNCTION(BlueprintCallable, Category = "ABTesting|Experiment")
	void TrackExperimentExposure(const FString& ExperimentID);

	UFUNCTION(BlueprintCallable, Category = "ABTesting|Experiment")
	void TrackExperimentConversion(const FString& ExperimentID, const FString& MetricName, float Value = 1.0f);

	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	TArray<FMGExperiment> GetActiveExperiments() const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	TArray<FMGExperimentAssignment> GetMyAssignments() const { return MyAssignments; }

	// Segmentation
	UFUNCTION(BlueprintPure, Category = "ABTesting|Segment")
	TArray<FString> GetUserSegments() const { return UserSegments; }

	UFUNCTION(BlueprintCallable, Category = "ABTesting|Segment")
	void UpdateUserSegments();

	UFUNCTION(BlueprintPure, Category = "ABTesting|Segment")
	bool IsInSegment(const FString& SegmentID) const;

	// QA Overrides
	UFUNCTION(BlueprintCallable, Category = "ABTesting|QA")
	void OverrideFeatureFlag(const FString& FlagID, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ABTesting|QA")
	void OverrideExperimentVariant(const FString& ExperimentID, EMGVariantType Variant);

	UFUNCTION(BlueprintCallable, Category = "ABTesting|QA")
	void ClearAllOverrides();

	UFUNCTION(BlueprintPure, Category = "ABTesting|QA")
	bool HasOverride(const FString& ID) const;

	// Gradual Rollout
	UFUNCTION(BlueprintPure, Category = "ABTesting|Rollout")
	float GetRolloutPercent(const FString& FlagID) const;

	UFUNCTION(BlueprintPure, Category = "ABTesting|Rollout")
	bool IsInRollout(const FString& FlagID) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "ABTesting|Events")
	FMGOnExperimentAssigned OnExperimentAssigned;

	UPROPERTY(BlueprintAssignable, Category = "ABTesting|Events")
	FMGOnFeatureFlagChanged OnFeatureFlagChanged;

	UPROPERTY(BlueprintAssignable, Category = "ABTesting|Events")
	FMGOnConfigurationRefreshed OnConfigurationRefreshed;

protected:
	void LoadConfiguration();
	void SaveLocalAssignments();
	void LoadLocalAssignments();
	void AssignToExperiments();
	EMGVariantType DetermineVariantAssignment(const FMGExperiment& Experiment) const;
	bool EvaluateSegmentCriteria(const FMGUserSegment& Segment) const;
	uint32 GenerateUserBucket(const FString& ExperimentID) const;
	void InitializeDefaultFlags();

private:
	UPROPERTY()
	TArray<FMGFeatureFlag> FeatureFlags;

	UPROPERTY()
	TArray<FMGExperiment> Experiments;

	UPROPERTY()
	TArray<FMGExperimentAssignment> MyAssignments;

	UPROPERTY()
	TArray<FMGUserSegment> Segments;

	TArray<FString> UserSegments;
	TMap<FString, bool> FlagOverrides;
	TMap<FString, EMGVariantType> ExperimentOverrides;
	FTimerHandle RefreshTimerHandle;
	FString UserID;
	float RefreshIntervalSeconds = 300.0f;
};
