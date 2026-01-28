// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMechanicSubsystem.h
 * @brief Mechanic relationship and job management system for vehicle part installation and tuning.
 *
 * The Mechanic Subsystem manages all interactions with in-game mechanics who install,
 * tune, and repair vehicle parts. Rather than instant part swaps, Midnight Grind features
 * a realistic mechanic system where work takes time, costs money, and quality varies
 * based on the mechanic's skill and your relationship with them.
 *
 * @section mech_concepts Key Concepts
 *
 * **Mechanics as Characters:**
 * Mechanics are not just service points - they're characters with personalities,
 * backstories, and specializations that affect gameplay:
 * - Skill Tiers: Apprentice to Legend, affecting work quality and speed
 * - Specializations: Engine, Suspension, Transmission, etc. - better at specific jobs
 * - Personalities: Professional, Hustler, Perfectionist, etc. - affects pricing and behavior
 *
 * **The Job System:**
 * All work is tracked as "jobs" with the following properties:
 * - Estimated completion time (affected by mechanic speed and rush status)
 * - Cost (affected by mechanic rates, part complexity, and your loyalty discount)
 * - Quality outcome (Perfect, Good, Acceptable, Botched, Failed)
 * - Quality modifier applied to the installed part's stats
 *
 * **Work Results:**
 * Each job has a probabilistic outcome based on mechanic skill:
 * - Perfect: Above expected quality, part gets a stat bonus
 * - Good: Standard quality, part performs as specified
 * - Acceptable: Slight imperfections, minor stat penalty
 * - Botched: Mistakes made, noticeable performance reduction
 * - Failed: Complete failure, part may be damaged or destroyed
 *
 * **Trust and Relationships:**
 * Building trust with mechanics provides tangible benefits:
 * - Loyalty discounts on labor (up to 15% at high trust)
 * - Access to faster turnaround times
 * - Better quality outcomes (reduced chance of botched work)
 * - Special services unlocked (custom fabrication, black market referrals)
 * - Tips about rare parts and deals
 *
 * @section mech_services Service Types
 *
 * Mechanics can perform various services:
 * - Install: Basic part installation on a vehicle
 * - Remove: Remove a part (often to transfer to another vehicle)
 * - Tune: Fine-tune an installed part for better performance
 * - Repair: Fix damaged parts instead of replacing them
 * - Restore: Restore worn parts to full condition
 * - Custom: Fabricate custom one-off parts (high trust required)
 * - Rush: Any service done quickly for extra cost
 *
 * @section mech_usage Basic Usage Examples
 *
 * **Finding the Right Mechanic:**
 * @code
 * UMGMechanicSubsystem* MechanicSystem = GetGameInstance()->GetSubsystem<UMGMechanicSubsystem>();
 *
 * // Get all mechanics you have access to
 * TArray<FMGMechanic> AvailableMechanics = MechanicSystem->GetAvailableMechanics();
 *
 * // Find mechanics who specialize in engine work
 * TArray<FMGMechanic> EngineSpecialists = MechanicSystem->GetMechanicsBySpecialization(
 *     EMGMechanicSpecialization::Engine
 * );
 *
 * // Get the recommended mechanic for a specific job
 * FName BestMechanic = MechanicSystem->GetRecommendedMechanic(
 *     PartID,
 *     EMGMechanicService::Install
 * );
 * @endcode
 *
 * **Starting a Job:**
 * @code
 * // Get an estimate first
 * int32 Cost = MechanicSystem->GetJobEstimate(MechanicID, PartID, EMGMechanicService::Install, false);
 * float Hours = MechanicSystem->GetJobDuration(MechanicID, PartID, EMGMechanicService::Install, false);
 *
 * // Check expected quality
 * float ExpectedQuality = MechanicSystem->GetExpectedQuality(MechanicID, PartID, EMGMechanicService::Install);
 *
 * // Start the job
 * FGuid JobID = MechanicSystem->StartJob(
 *     MechanicID,
 *     VehicleID,
 *     PartID,
 *     EMGMechanicService::Install,
 *     false  // Not a rush job
 * );
 * @endcode
 *
 * **Tracking Active Jobs:**
 * @code
 * // Get all jobs in progress
 * TArray<FMGMechanicJob> ActiveJobs = MechanicSystem->GetActiveJobs();
 *
 * for (const FMGMechanicJob& Job : ActiveJobs)
 * {
 *     // Check if job is ready
 *     if (MechanicSystem->IsJobComplete(Job.JobID))
 *     {
 *         // Complete the job and get results
 *         FMGMechanicJob CompletedJob = MechanicSystem->CompleteJob(Job.JobID);
 *
 *         // Handle the result
 *         if (CompletedJob.Result == EMGWorkResult::Botched)
 *         {
 *             // Part has reduced performance
 *             UE_LOG(LogTemp, Warning, TEXT("Job was botched! Quality modifier: %d"),
 *                 CompletedJob.QualityModifier);
 *         }
 *     }
 * }
 * @endcode
 *
 * **Building Trust:**
 * @code
 * // Get your relationship with a mechanic
 * FMGMechanicRelationship Relationship = MechanicSystem->GetMechanicRelationship(MechanicID);
 *
 * // Check trust level and loyalty discount
 * int32 Trust = MechanicSystem->GetTrustLevel(MechanicID);
 * float Discount = MechanicSystem->GetLoyaltyDiscount(MechanicID);
 *
 * // Set this mechanic as your preferred (for quick access)
 * MechanicSystem->SetPreferredMechanic(MechanicID);
 *
 * // Check for unlocked special services
 * TArray<FName> UnlockedServices = MechanicSystem->GetUnlockedServices(MechanicID);
 * @endcode
 *
 * **Using Special Abilities:**
 * @code
 * // Check if mechanic has underground connections
 * FMGMechanic Mechanic;
 * if (MechanicSystem->GetMechanic(MechanicID, Mechanic) && Mechanic.bHasUndergroundConnections)
 * {
 *     // Request a black market referral
 *     FName DealerID;
 *     if (MechanicSystem->RequestBlackMarketReferral(MechanicID, DealerID))
 *     {
 *         // Got access to a new black market dealer!
 *     }
 *
 *     // Get tips about rare parts
 *     FText Tip = MechanicSystem->GetRarePartTip(MechanicID);
 * }
 * @endcode
 *
 * @see UMGGarageSubsystem For vehicle and part ownership
 * @see UMGBlackMarketSubsystem For underground part acquisition
 * @see UMGTransactionPipeline For payment processing
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMechanicSubsystem.generated.h"

/**
 * Mechanic skill tier - affects quality of work and available services
 */
UENUM(BlueprintType)
enum class EMGMechanicSkillTier : uint8
{
	Apprentice,     // Basic work, slow, higher chance of mistakes
	Journeyman,     // Competent work, standard speed
	Expert,         // High quality work, faster, can tune
	Master,         // Premium work, fastest, precision tuning
	Legend          // Legendary mechanics, unique abilities
};

/**
 * Mechanic specialization - what they're best at
 */
UENUM(BlueprintType)
enum class EMGMechanicSpecialization : uint8
{
	General,        // Jack of all trades
	Engine,         // Engine builds, turbo, supercharger
	Suspension,     // Suspension tuning, alignment, handling
	Transmission,   // Gearbox, clutch, differential
	Bodywork,       // Weight reduction, aero, appearance
	Electrical,     // Nitrous, electronics, ECU tuning
	Restoration     // Classic car specialist, rare part sourcing
};

/**
 * Mechanic personality - affects pricing and interactions
 */
UENUM(BlueprintType)
enum class EMGMechanicPersonality : uint8
{
	Professional,   // Fair prices, reliable, by-the-book
	Hustler,        // Expensive but fast, may cut corners
	Perfectionist,  // Slow but highest quality, expensive
	OldSchool,      // Classic techniques, distrust of new tech
	Underground,    // Questionable methods, knows black market
	Mentor          // Teaches you, discounts for loyalty
};

/**
 * Result of a mechanic's work
 */
UENUM(BlueprintType)
enum class EMGWorkResult : uint8
{
	Perfect,        // Above expected quality
	Good,           // Standard quality
	Acceptable,     // Slight imperfections, still functional
	Botched,        // Mistakes made, reduced performance
	Failed          // Complete failure, part damaged
};

/**
 * Service types mechanics can perform
 */
UENUM(BlueprintType)
enum class EMGMechanicService : uint8
{
	Install,        // Basic part installation
	Remove,         // Part removal
	Tune,           // Performance tuning
	Repair,         // Fix damaged parts
	Restore,        // Restore worn parts
	Custom,         // Custom fabrication
	Rush            // Emergency rush job
};

/**
 * A mechanic character with skills and trust
 */
USTRUCT(BlueprintType)
struct FMGMechanic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MechanicID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Backstory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMechanicSkillTier SkillTier = EMGMechanicSkillTier::Journeyman;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMechanicSpecialization PrimarySpecialization = EMGMechanicSpecialization::General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMechanicSpecialization SecondarySpecialization = EMGMechanicSpecialization::General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMechanicPersonality Personality = EMGMechanicPersonality::Professional;

	// Base cost multiplier (1.0 = standard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CostMultiplier = 1.0f;

	// Work speed multiplier (1.0 = standard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	// Quality rating 0-100
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QualityRating = 70;

	// Whether this mechanic has underground connections
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasUndergroundConnections = false;

	// Minimum trust level to access this mechanic
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinimumTrustRequired = 0;

	// Location/garage name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText GarageName;

	// Special abilities unlocked at high trust
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> SpecialAbilities;
};

/**
 * Player's relationship with a mechanic
 */
USTRUCT(BlueprintType)
struct FMGMechanicRelationship
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MechanicID;

	// Trust level 0-100
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrustLevel = 0;

	// Total money spent with this mechanic
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalMoneySpent = 0;

	// Number of jobs completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 JobsCompleted = 0;

	// Number of parts referred (from black market connections)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PartsReferred = 0;

	// Have they made a mistake on your car?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BotchedJobs = 0;

	// Timestamp of first interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstInteraction;

	// Is this your preferred mechanic?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreferred = false;

	// Discount percentage earned through loyalty
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoyaltyDiscount = 0.0f;

	// Special services unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> UnlockedServices;
};

/**
 * A job in progress or completed
 */
USTRUCT(BlueprintType)
struct FMGMechanicJob
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid JobID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MechanicID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMechanicService ServiceType = EMGMechanicService::Install;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EstimatedCompletion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRushJob = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWorkResult Result = EMGWorkResult::Good;

	// Quality bonus/penalty applied to part (-20 to +20)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QualityModifier = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMechanicJobStarted, FName, MechanicID, FGuid, JobID, EMGMechanicService, ServiceType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMechanicJobCompleted, FGuid, JobID, EMGWorkResult, Result, int32, QualityModifier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMechanicTrustChanged, FName, MechanicID, int32, NewTrustLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMechanicServiceUnlocked, FName, MechanicID, FName, ServiceName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMechanicRelationshipMilestone, FName, MechanicID, FText, MilestoneDescription);

/**
 * Manages mechanic relationships, jobs, and trust progression
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMechanicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnMechanicJobStarted OnMechanicJobStarted;

	UPROPERTY(BlueprintAssignable)
	FOnMechanicJobCompleted OnMechanicJobCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnMechanicTrustChanged OnMechanicTrustChanged;

	UPROPERTY(BlueprintAssignable)
	FOnMechanicServiceUnlocked OnMechanicServiceUnlocked;

	UPROPERTY(BlueprintAssignable)
	FOnMechanicRelationshipMilestone OnMechanicRelationshipMilestone;

	// ==================== Mechanic Discovery ====================

	/** Get all available mechanics */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	TArray<FMGMechanic> GetAllMechanics() const;

	/** Get mechanics the player has access to based on trust/reputation */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	TArray<FMGMechanic> GetAvailableMechanics() const;

	/** Get mechanic by ID */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	bool GetMechanic(FName MechanicID, FMGMechanic& OutMechanic) const;

	/** Get mechanics by specialization */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	TArray<FMGMechanic> GetMechanicsBySpecialization(EMGMechanicSpecialization Specialization) const;

	/** Get the best mechanic for a specific job */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	FName GetRecommendedMechanic(FName PartID, EMGMechanicService ServiceType) const;

	// ==================== Job Management ====================

	/** Start a new job with a mechanic */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	FGuid StartJob(FName MechanicID, FName VehicleID, FName PartID, EMGMechanicService ServiceType, bool bRushJob = false);

	/** Get estimated cost for a job */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	int32 GetJobEstimate(FName MechanicID, FName PartID, EMGMechanicService ServiceType, bool bRushJob = false) const;

	/** Get estimated completion time in game hours */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	float GetJobDuration(FName MechanicID, FName PartID, EMGMechanicService ServiceType, bool bRushJob = false) const;

	/** Check if a job is complete */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	bool IsJobComplete(const FGuid& JobID) const;

	/** Complete a job and get the result */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	FMGMechanicJob CompleteJob(const FGuid& JobID);

	/** Get all active jobs */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	TArray<FMGMechanicJob> GetActiveJobs() const;

	/** Get job history */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	TArray<FMGMechanicJob> GetJobHistory(int32 Count = 10) const;

	/** Cancel a job (may lose deposit) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	bool CancelJob(const FGuid& JobID, int32& OutRefundAmount);

	// ==================== Trust & Relationships ====================

	/** Get relationship with a mechanic */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	FMGMechanicRelationship GetMechanicRelationship(FName MechanicID) const;

	/** Get trust level with a mechanic (0-100) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	int32 GetTrustLevel(FName MechanicID) const;

	/** Add trust with a mechanic */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	void AddTrust(FName MechanicID, int32 Amount);

	/** Get loyalty discount percentage */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	float GetLoyaltyDiscount(FName MechanicID) const;

	/** Set preferred mechanic */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	void SetPreferredMechanic(FName MechanicID);

	/** Get preferred mechanic */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	FName GetPreferredMechanic() const;

	/** Check if a special service is unlocked */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	bool IsServiceUnlocked(FName MechanicID, FName ServiceName) const;

	/** Get all unlocked services for a mechanic */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	TArray<FName> GetUnlockedServices(FName MechanicID) const;

	// ==================== Work Quality ====================

	/** Calculate expected work quality for a job */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	float GetExpectedQuality(FName MechanicID, FName PartID, EMGMechanicService ServiceType) const;

	/** Get probability of each work result */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	void GetWorkResultProbabilities(FName MechanicID, EMGMechanicService ServiceType,
		float& OutPerfect, float& OutGood, float& OutAcceptable, float& OutBotched, float& OutFailed) const;

	/** Simulate work result based on mechanic skill and RNG */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	EMGWorkResult SimulateWorkResult(FName MechanicID, EMGMechanicService ServiceType, bool bRushJob) const;

	// ==================== Special Abilities ====================

	/** Check if mechanic has a special ability */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	bool HasSpecialAbility(FName MechanicID, FName AbilityName) const;

	/** Request black market part referral (requires underground connections) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	bool RequestBlackMarketReferral(FName MechanicID, FName& OutDealerID);

	/** Get mechanic's tip about rare parts */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	FText GetRarePartTip(FName MechanicID) const;

	// ==================== Utility ====================

	/** Get service name for UI */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	static FText GetServiceDisplayName(EMGMechanicService ServiceType);

	/** Get skill tier name for UI */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	static FText GetSkillTierDisplayName(EMGMechanicSkillTier SkillTier);

	/** Tick active jobs (call from game tick) */
	UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Mechanics")
	void TickJobs(float DeltaGameHours);

protected:
	void InitializeMechanics();
	void InitializeSpecialAbilities();

	void UpdateTrustMilestones(FName MechanicID, int32 OldTrust, int32 NewTrust);
	void UpdateLoyaltyDiscount(FName MechanicID);
	void UnlockTrustServices(FName MechanicID, int32 TrustLevel);

	int32 CalculateJobCost(const FMGMechanic& Mechanic, FName PartID, EMGMechanicService ServiceType, bool bRushJob) const;
	float CalculateJobDuration(const FMGMechanic& Mechanic, FName PartID, EMGMechanicService ServiceType, bool bRushJob) const;
	int32 CalculateQualityModifier(const FMGMechanic& Mechanic, EMGWorkResult Result) const;

	bool IsMechanicAvailable(const FMGMechanic& Mechanic) const;
	int32 GetPartBaseInstallTime(FName PartID) const;
	int32 GetPartBaseInstallCost(FName PartID) const;

private:
	// All mechanics in the game
	UPROPERTY()
	TMap<FName, FMGMechanic> Mechanics;

	// Player relationships with mechanics
	UPROPERTY()
	TMap<FName, FMGMechanicRelationship> Relationships;

	// Active jobs
	UPROPERTY()
	TMap<FGuid, FMGMechanicJob> ActiveJobs;

	// Completed job history
	UPROPERTY()
	TArray<FMGMechanicJob> JobHistory;

	// Current preferred mechanic
	UPROPERTY()
	FName PreferredMechanicID;

	// Special abilities and their unlock requirements
	TMap<FName, int32> AbilityTrustRequirements;
};
