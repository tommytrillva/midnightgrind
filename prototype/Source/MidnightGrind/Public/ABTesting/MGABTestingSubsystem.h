// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGABTestingSubsystem.h
 * @brief A/B Testing and Feature Flag Management Subsystem
 *
 * This subsystem provides comprehensive A/B testing and feature flag functionality
 * for Midnight Grind. It enables data-driven game development through controlled
 * experiments and gradual feature rollouts.
 *
 * Key Features:
 * - Server-controlled feature flags with local caching
 * - User segmentation for targeted test groups
 * - Metric tracking per experiment with conversion tracking
 * - Gradual rollout support with percentage-based targeting
 * - QA override capability for testing specific variants
 *
 * Usage Example:
 * @code
 * UMGABTestingSubsystem* ABTestingSubsystem = GameInstance->GetSubsystem<UMGABTestingSubsystem>();
 * if (ABTestingSubsystem->IsFeatureEnabled("new_garage_ui"))
 * {
 *     // Show new garage UI
 * }
 * @endcode
 *
 * @see UMGAnalyticsSubsystem for event tracking integration
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGABTestingSubsystem.generated.h"

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Status of an experiment in the A/B testing system
 *
 * Experiments progress through various states during their lifecycle,
 * from initial draft to completion or cancellation.
 */
UENUM(BlueprintType)
enum class EMGExperimentStatus : uint8
{
	/** Experiment is being configured, not yet active */
	Draft,
	/** Experiment is actively collecting data */
	Running,
	/** Experiment is temporarily stopped but can resume */
	Paused,
	/** Experiment has reached its end date or sample size goal */
	Completed,
	/** Experiment was manually stopped and will not resume */
	Cancelled
};

/**
 * @brief Variant types for experiment assignment
 *
 * Users are assigned to one of these variants when participating
 * in an experiment. Control is the baseline for comparison.
 */
UENUM(BlueprintType)
enum class EMGVariantType : uint8
{
	/** Baseline variant - existing behavior */
	Control,
	/** First test variant */
	VariantA,
	/** Second test variant */
	VariantB,
	/** Third test variant */
	VariantC,
	/** Fourth test variant */
	VariantD
};

// ============================================================================
// Data Structures - Experiment Configuration
// ============================================================================

/**
 * @brief Configuration for a single variant within an experiment
 *
 * Defines the allocation percentage, parameters, and tracking metrics
 * for a specific variant in an A/B test.
 */
USTRUCT(BlueprintType)
struct FMGExperimentVariant
{
	GENERATED_BODY()

	/** The type identifier for this variant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVariantType Type = EMGVariantType::Control;

	/** Human-readable name for this variant (e.g., "Blue Button") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VariantName;

	/** Percentage of eligible users assigned to this variant (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AllocationPercent = 50.0f;

	/** Key-value parameters specific to this variant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Parameters;

	/** Number of users assigned to this variant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParticipantCount = 0;

	/** Number of users who converted in this variant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConversionCount = 0;
};

/**
 * @brief Complete experiment definition
 *
 * Contains all configuration and metadata for an A/B test experiment,
 * including variants, targeting criteria, and statistical requirements.
 */
USTRUCT(BlueprintType)
struct FMGExperiment
{
	GENERATED_BODY()

	/** Unique identifier for this experiment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExperimentID;

	/** Human-readable name displayed in dashboards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExperimentName;

	/** Detailed description of the experiment hypothesis and goals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	/** Current lifecycle status of the experiment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGExperimentStatus Status = EMGExperimentStatus::Draft;

	/** Array of variants being tested in this experiment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGExperimentVariant> Variants;

	/** Primary metric being measured (e.g., "conversion_rate") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetMetric;

	/** When the experiment begins accepting participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	/** When the experiment stops accepting new participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	/** Minimum participants per variant for statistical significance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinSampleSize = 1000;

	/** Required confidence level (0.0-1.0, typically 0.95) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConfidenceLevel = 0.95f;

	/** Whether users must explicitly opt-in to participate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresOptIn = false;

	/** User segments eligible for this experiment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> TargetSegments;
};

// ============================================================================
// Data Structures - Feature Flags
// ============================================================================

/**
 * @brief Feature flag configuration for controlled rollouts
 *
 * Feature flags allow enabling/disabling features without code changes,
 * with support for gradual rollouts and segment-based targeting.
 */
USTRUCT(BlueprintType)
struct FMGFeatureFlag
{
	GENERATED_BODY()

	/** Unique identifier for this flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FlagID;

	/** Human-readable name for this flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FlagName;

	/** Master enable/disable switch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

	/** Percentage of users who see this feature when enabled (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RolloutPercent = 0.0f;

	/** Segments that always see this feature when enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EnabledSegments;

	/** Specific user IDs that always see this feature */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EnabledUserIDs;

	/** Additional configuration parameters for this flag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Configuration;

	/** Priority for flag evaluation order (higher = checked first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

/**
 * @brief User segment definition for targeting
 *
 * Segments group users by shared characteristics for targeted
 * experiments and feature rollouts.
 */
USTRUCT(BlueprintType)
struct FMGUserSegment
{
	GENERATED_BODY()

	/** Unique identifier for this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SegmentID;

	/** Human-readable name for this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SegmentName;

	/** Key-value criteria for segment membership */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Criteria;
};

/**
 * @brief Record of a user's assignment to an experiment
 *
 * Tracks which variant a user was assigned to and their
 * exposure and conversion status.
 */
USTRUCT(BlueprintType)
struct FMGExperimentAssignment
{
	GENERATED_BODY()

	/** ID of the experiment this assignment is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExperimentID;

	/** The variant the user was assigned to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVariantType AssignedVariant = EMGVariantType::Control;

	/** Timestamp when the user was assigned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime AssignedAt;

	/** Whether the user has been exposed to the experiment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExposed = false;

	/** Whether the user has converted (completed target action) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConverted = false;
};

// ============================================================================
// Delegate Declarations
// ============================================================================

/** Broadcast when a user is assigned to an experiment variant */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnExperimentAssigned, const FString&, ExperimentID, EMGVariantType, Variant);

/** Broadcast when a feature flag's enabled state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnFeatureFlagChanged, const FString&, FlagID, bool, bEnabled);

/** Broadcast when configuration is refreshed from the server */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnConfigurationRefreshed, int32, FlagsUpdated);

// ============================================================================
// Subsystem Class
// ============================================================================

/**
 * @brief A/B Testing and Feature Flag Subsystem
 *
 * Manages feature flags, A/B experiments, and user segmentation for
 * Midnight Grind. Provides both Blueprint and C++ interfaces for
 * checking feature states and tracking experiment metrics.
 *
 * This subsystem persists across level transitions as a GameInstance subsystem.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGABTestingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @brief Called when the subsystem is created. Loads configuration and initializes experiments.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// @brief Called when the subsystem is destroyed. Saves local assignments and cleans up timers.
	virtual void Deinitialize() override;

	// ========================================================================
	// Feature Flags
	// ========================================================================

	/**
	 * @brief Check if a feature flag is enabled for the current user
	 * @param FlagID The unique identifier of the feature flag
	 * @return True if the feature is enabled, false otherwise
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	bool IsFeatureEnabled(const FString& FlagID) const;

	/**
	 * @brief Get a string configuration value from a feature flag
	 * @param FlagID The feature flag identifier
	 * @param Key The configuration key to retrieve
	 * @param DefaultValue Value to return if the key doesn't exist
	 * @return The configuration value or DefaultValue
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	FString GetFeatureConfig(const FString& FlagID, const FString& Key, const FString& DefaultValue = TEXT("")) const;

	/**
	 * @brief Get an integer configuration value from a feature flag
	 * @param FlagID The feature flag identifier
	 * @param Key The configuration key to retrieve
	 * @param DefaultValue Value to return if the key doesn't exist
	 * @return The integer value or DefaultValue
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	int32 GetFeatureConfigInt(const FString& FlagID, const FString& Key, int32 DefaultValue = 0) const;

	/**
	 * @brief Get a float configuration value from a feature flag
	 * @param FlagID The feature flag identifier
	 * @param Key The configuration key to retrieve
	 * @param DefaultValue Value to return if the key doesn't exist
	 * @return The float value or DefaultValue
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	float GetFeatureConfigFloat(const FString& FlagID, const FString& Key, float DefaultValue = 0.0f) const;

	/**
	 * @brief Get all registered feature flags
	 * @return Array of all feature flag configurations
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Flags")
	TArray<FMGFeatureFlag> GetAllFeatureFlags() const { return FeatureFlags; }

	/**
	 * @brief Refresh feature flags from the backend server
	 * Updates local cache with latest server configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|Flags")
	void RefreshFeatureFlags();

	// ========================================================================
	// Experiments
	// ========================================================================

	/**
	 * @brief Get the variant assigned to the current user for an experiment
	 * @param ExperimentID The unique identifier of the experiment
	 * @return The assigned variant type
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	EMGVariantType GetExperimentVariant(const FString& ExperimentID) const;

	/**
	 * @brief Check if the current user is participating in an experiment
	 * @param ExperimentID The unique identifier of the experiment
	 * @return True if the user has been assigned to this experiment
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	bool IsInExperiment(const FString& ExperimentID) const;

	/**
	 * @brief Get a parameter value for the user's assigned variant
	 * @param ExperimentID The experiment identifier
	 * @param ParamKey The parameter key to retrieve
	 * @param DefaultValue Value to return if parameter doesn't exist
	 * @return The parameter value or DefaultValue
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	FString GetExperimentParam(const FString& ExperimentID, const FString& ParamKey, const FString& DefaultValue = TEXT("")) const;

	/**
	 * @brief Track that the user has been exposed to an experiment
	 * Call this when the user actually sees the experimental feature
	 * @param ExperimentID The experiment to track exposure for
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|Experiment")
	void TrackExperimentExposure(const FString& ExperimentID);

	/**
	 * @brief Track a conversion event for an experiment
	 * @param ExperimentID The experiment to track conversion for
	 * @param MetricName The name of the conversion metric
	 * @param Value The conversion value (default 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|Experiment")
	void TrackExperimentConversion(const FString& ExperimentID, const FString& MetricName, float Value = 1.0f);

	/**
	 * @brief Get all currently running experiments
	 * @return Array of active experiment configurations
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	TArray<FMGExperiment> GetActiveExperiments() const;

	/**
	 * @brief Get the current user's experiment assignments
	 * @return Array of all experiment assignments for this user
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Experiment")
	TArray<FMGExperimentAssignment> GetMyAssignments() const { return MyAssignments; }

	// ========================================================================
	// Segmentation
	// ========================================================================

	/**
	 * @brief Get all segments the current user belongs to
	 * @return Array of segment IDs
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Segment")
	TArray<FString> GetUserSegments() const { return UserSegments; }

	/**
	 * @brief Recalculate which segments the current user belongs to
	 * Should be called after user properties change
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|Segment")
	void UpdateUserSegments();

	/**
	 * @brief Check if the current user is in a specific segment
	 * @param SegmentID The segment identifier to check
	 * @return True if the user is in the segment
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Segment")
	bool IsInSegment(const FString& SegmentID) const;

	// ========================================================================
	// QA Overrides
	// ========================================================================

	/**
	 * @brief Override a feature flag state for QA testing
	 * @param FlagID The flag to override
	 * @param bEnabled The override value
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|QA")
	void OverrideFeatureFlag(const FString& FlagID, bool bEnabled);

	/**
	 * @brief Override experiment variant assignment for QA testing
	 * @param ExperimentID The experiment to override
	 * @param Variant The variant to force assignment to
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|QA")
	void OverrideExperimentVariant(const FString& ExperimentID, EMGVariantType Variant);

	/**
	 * @brief Clear all QA overrides and return to normal operation
	 */
	UFUNCTION(BlueprintCallable, Category = "ABTesting|QA")
	void ClearAllOverrides();

	/**
	 * @brief Check if a flag or experiment has an active override
	 * @param ID The flag or experiment ID to check
	 * @return True if an override is active
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|QA")
	bool HasOverride(const FString& ID) const;

	// ========================================================================
	// Gradual Rollout
	// ========================================================================

	/**
	 * @brief Get the rollout percentage for a feature flag
	 * @param FlagID The flag to check
	 * @return The rollout percentage (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Rollout")
	float GetRolloutPercent(const FString& FlagID) const;

	/**
	 * @brief Check if the current user is within a flag's rollout percentage
	 * @param FlagID The flag to check
	 * @return True if the user is in the rollout group
	 */
	UFUNCTION(BlueprintPure, Category = "ABTesting|Rollout")
	bool IsInRollout(const FString& FlagID) const;

	// ========================================================================
	// Events
	// ========================================================================

	/** Broadcast when user is assigned to a new experiment */
	UPROPERTY(BlueprintAssignable, Category = "ABTesting|Events")
	FMGOnExperimentAssigned OnExperimentAssigned;

	/** Broadcast when a feature flag state changes */
	UPROPERTY(BlueprintAssignable, Category = "ABTesting|Events")
	FMGOnFeatureFlagChanged OnFeatureFlagChanged;

	/** Broadcast when configuration is refreshed from server */
	UPROPERTY(BlueprintAssignable, Category = "ABTesting|Events")
	FMGOnConfigurationRefreshed OnConfigurationRefreshed;

protected:
	/// @brief Load feature flags and experiments from local storage and server
	void LoadConfiguration();

	/// @brief Persist experiment assignments to local storage
	void SaveLocalAssignments();

	/// @brief Load previously saved experiment assignments
	void LoadLocalAssignments();

	/// @brief Assign the current user to eligible experiments
	void AssignToExperiments();

	/// @brief Determine which variant to assign a user to based on allocation
	/// @param Experiment The experiment configuration
	/// @return The variant to assign
	EMGVariantType DetermineVariantAssignment(const FMGExperiment& Experiment) const;

	/// @brief Evaluate whether the current user matches segment criteria
	/// @param Segment The segment to evaluate
	/// @return True if the user matches all criteria
	bool EvaluateSegmentCriteria(const FMGUserSegment& Segment) const;

	/// @brief Generate a deterministic bucket number for the user
	/// @param ExperimentID The experiment for bucketing (for consistent assignment)
	/// @return A bucket number 0-99
	uint32 GenerateUserBucket(const FString& ExperimentID) const;

	/// @brief Initialize default feature flags for development
	void InitializeDefaultFlags();

private:
	/** All registered feature flags */
	UPROPERTY()
	TArray<FMGFeatureFlag> FeatureFlags;

	/** All registered experiments */
	UPROPERTY()
	TArray<FMGExperiment> Experiments;

	/** Current user's experiment assignments */
	UPROPERTY()
	TArray<FMGExperimentAssignment> MyAssignments;

	/** All defined user segments */
	UPROPERTY()
	TArray<FMGUserSegment> Segments;

	/** Segment IDs the current user belongs to */
	TArray<FString> UserSegments;

	/** QA overrides for feature flags */
	TMap<FString, bool> FlagOverrides;

	/** QA overrides for experiment variants */
	TMap<FString, EMGVariantType> ExperimentOverrides;

	/** Timer for periodic configuration refresh */
	FTimerHandle RefreshTimerHandle;

	/** Current user's unique identifier for bucketing */
	FString UserID;

	/** Interval between configuration refreshes (default 5 minutes) */
	float RefreshIntervalSeconds = 300.0f;
};
