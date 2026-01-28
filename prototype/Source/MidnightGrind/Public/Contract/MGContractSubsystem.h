// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * @file MGContractSubsystem.h
 * @brief Contract System - Missions, Objectives, Sponsors, and Rewards
 * =============================================================================
 *
 * @section Overview
 * This subsystem manages the contract (mission) system for Midnight Grind.
 * Contracts are structured tasks that players accept and complete to earn
 * rewards. Think of it like the mission system in games like Need for Speed,
 * Forza Horizon, or GTA - you pick up jobs, complete objectives, and get paid.
 *
 * @section WhyContracts Why a Contract System?
 *
 * Contracts provide:
 * - PROGRESSION: Players have clear goals to work toward
 * - VARIETY: Different contract types keep gameplay fresh
 * - REWARDS: Meaningful progression through credits, XP, unlocks
 * - ENGAGEMENT: Daily/weekly contracts encourage regular play
 * - NARRATIVE: Sponsor relationships and story contracts add depth
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * 1. GAME INSTANCE SUBSYSTEM
 *    Inherits from UGameInstanceSubsystem:
 *    - One instance for entire game session
 *    - Persists across level loads
 *    - Access via: GetGameInstance()->GetSubsystem<UMGContractSubsystem>()
 *
 * 2. CONTRACT TYPES (EMGContractType)
 *    Different categories of contracts:
 *    - Race: Complete a specific race or series
 *    - Championship: Multi-race series with points
 *    - Sponsor: Contracts from in-game sponsors (brands)
 *    - Team: Crew/team-based objectives
 *    - Manufacturer: Use specific car brands
 *    - Special: Limited-time events
 *    - Daily: Refresh every 24 hours
 *    - Weekly: Refresh every 7 days
 *    - Story: Main campaign missions
 *
 * 3. CONTRACT STATES (EMGContractState)
 *    Lifecycle of a contract:
 *    - Available: Can be accepted
 *    - Locked: Requirements not met
 *    - Active: Currently in progress
 *    - Completed: Successfully finished
 *    - Failed: Did not meet objectives
 *    - Expired: Time limit passed
 *    - Abandoned: Player gave up
 *
 * 4. OBJECTIVES (FMGContractObjective, EMGObjectiveType)
 *    What players must accomplish:
 *    - Race objectives: WinRace, Podium (top 3), FinishPosition
 *    - Performance: FastestLap, LapTime, TopSpeed, PerfectStart
 *    - Style: DriftScore, NearMisses, CleanLaps (no collisions)
 *    - Progress: Overtakes, EarnCredits, GainXP, WinStreak
 *    - Restrictions: UseCar (specific vehicle), UseTrack
 *
 * 5. REWARDS (FMGContractReward, EMGRewardType)
 *    What players earn:
 *    - Credits: In-game currency
 *    - XP: Experience points for leveling
 *    - Vehicle: Unlock new cars
 *    - Part: Performance upgrades
 *    - Cosmetic: Visual customizations
 *    - Title/Badge: Profile decorations
 *    - Exclusive: Limited items
 *
 * 6. REQUIREMENTS (FMGContractRequirements)
 *    What players need before accepting:
 *    - MinLevel: Player level requirement
 *    - MinReputation: Street cred needed
 *    - RequiredVehicles: Must own specific cars
 *    - RequiredLicenses: Racing license tiers
 *    - CompletedContracts: Prerequisite missions
 *
 * 7. SPONSORS (FMGSponsorData)
 *    In-game brands that offer contracts:
 *    - Build reputation with each sponsor
 *    - Higher rep = better contracts, bonuses
 *    - Level up sponsors for exclusive rewards
 *    - BonusMultiplier increases earnings
 *
 * 8. DAILY/WEEKLY ROTATION
 *    Time-limited contracts that refresh:
 *    - Daily: 3 new contracts every 24 hours
 *    - Weekly: Bigger challenges, better rewards
 *    - Completing all daily = streak bonus
 *    - GetTimeUntilDailyReset() for countdown
 *
 * 9. CONTRACT STREAKS
 *    Consecutive contract completions:
 *    - ContractStreak: Current streak count
 *    - BestStreak: All-time record
 *    - Streaks may unlock bonus rewards
 *
 * @section Usage Common Usage Patterns
 *
 * @code
 * // Get the contract subsystem
 * UMGContractSubsystem* Contracts =
 *     GetGameInstance()->GetSubsystem<UMGContractSubsystem>();
 *
 * // Get available contracts to show in UI
 * TArray<FMGContract> Available = Contracts->GetAvailableContracts();
 * for (const FMGContract& Contract : Available)
 * {
 *     // Display contract title, rewards, difficulty
 *     ShowContractUI(Contract.Title, Contract.Rewards, Contract.Difficulty);
 * }
 *
 * // Check if player can accept a contract
 * FName ContractID = "SprintRace_Downtown_01";
 * if (Contracts->CanAcceptContract(ContractID))
 * {
 *     // Player meets all requirements
 *     if (Contracts->AcceptContract(ContractID))
 *     {
 *         // Contract accepted! Show objectives
 *     }
 * }
 *
 * // After completing a race, update contract progress
 * Contracts->ProcessRaceResult(
 *     Position,      // e.g., 1 for first place
 *     BestLapTime,   // in seconds
 *     OvertakeCount,
 *     DriftScore,
 *     bNoCollisions  // clean race?
 * );
 *
 * // Update specific objective manually
 * Contracts->UpdateObjectiveProgress(
 *     ContractID,
 *     "DriftMaster",  // ObjectiveID
 *     15000.0f        // Progress value (drift score)
 * );
 *
 * // Check if an objective is complete
 * if (Contracts->IsObjectiveComplete(ContractID, "DriftMaster"))
 * {
 *     ShowObjectiveComplete();
 * }
 *
 * // When all objectives complete, claim rewards
 * Contracts->ClaimRewards(ContractID);
 * // Optional bonus objectives
 * Contracts->ClaimBonusRewards(ContractID);
 *
 * // Get daily contracts for the rotating challenge board
 * TArray<FMGContract> Dailies = Contracts->GetDailyContracts();
 * FTimespan TimeUntilReset = Contracts->GetTimeUntilDailyReset();
 * // Show countdown: TimeUntilReset.GetHours(), TimeUntilReset.GetMinutes()
 *
 * // Work with sponsors
 * FMGSponsorData Sponsor = Contracts->GetSponsor("NitroEnergy");
 * // Show sponsor level, contracts, reputation progress
 *
 * // After completing sponsor contract, add reputation
 * Contracts->AddSponsorReputation("NitroEnergy", 500);
 *
 * // Listen for contract events
 * Contracts->OnContractCompleted.AddDynamic(this, &AMyActor::HandleContractComplete);
 * Contracts->OnObjectiveProgress.AddDynamic(this, &AMyActor::HandleProgress);
 * Contracts->OnSponsorLevelUp.AddDynamic(this, &AMyActor::HandleSponsorLevelUp);
 *
 * void AMyActor::HandleContractComplete(const FMGContract& Contract)
 * {
 *     ShowCompletionScreen(Contract.Title, Contract.Rewards);
 * }
 * @endcode
 *
 * @section Architecture Architecture Notes
 *
 * CONTRACT DATABASE:
 * - ContractDatabase: All registered contracts
 * - Contracts defined in data assets or code
 * - RegisterContract() to add new contracts
 *
 * STATE PERSISTENCE:
 * - SaveContractData() / LoadContractData()
 * - Saves progress, completed contracts, sponsor rep
 * - Called automatically on session start/end
 *
 * TIMING:
 * - OnContractTick() runs on timer
 * - Updates time-limited contracts
 * - Checks for expirations
 * - Handles daily/weekly resets
 *
 * OBJECTIVE CHECKING:
 * - ProcessRaceResult() bulk-updates race-related objectives
 * - UpdateObjectiveProgress() for manual progress
 * - CheckAllObjectivesComplete() auto-completes contracts
 *
 * EVENTS/DELEGATES:
 * - OnContractAccepted: Player started a contract
 * - OnContractCompleted: Successfully finished
 * - OnContractFailed: Did not meet objectives
 * - OnObjectiveProgress: Progress toward objective
 * - OnObjectiveCompleted: Single objective done
 * - OnRewardClaimed: Reward granted to player
 * - OnSponsorLevelUp: Sponsor reputation increased
 * - OnDailyContractsRefreshed: New daily contracts available
 *
 * @see UMGProgressionSubsystem - Player level and XP
 * @see UMGInventorySubsystem - Reward item storage
 * @see UMGCurrencySubsystem - Credit rewards
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGContractSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGContractType : uint8
{
	Race,
	Championship,
	Sponsor,
	Team,
	Manufacturer,
	Special,
	Daily,
	Weekly,
	Story
};

UENUM(BlueprintType)
enum class EMGContractState : uint8
{
	Available,
	Locked,
	Active,
	Completed,
	Failed,
	Expired,
	Abandoned
};

UENUM(BlueprintType)
enum class EMGObjectiveType : uint8
{
	FinishRace,
	FinishPosition,
	WinRace,
	Podium,
	PointsFinish,
	FastestLap,
	LapTime,
	DriftScore,
	CleanLaps,
	Overtakes,
	NearMisses,
	TopSpeed,
	PerfectStart,
	NoCollisions,
	UseCar,
	UseTrack,
	EarnCredits,
	GainXP,
	WinStreak,
	ChampionshipPoints
};

UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
	Credits,
	XP,
	Vehicle,
	Part,
	Cosmetic,
	Title,
	Badge,
	Emote,
	CrateKey,
	RepBoost,
	Exclusive
};

UENUM(BlueprintType)
enum class EMGContractDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	Expert,
	Legendary
};

USTRUCT(BlueprintType)
struct FMGContractObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGObjectiveType Type = EMGObjectiveType::FinishRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusReward = 0;
};

USTRUCT(BlueprintType)
struct FMGContractReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRewardType Type = EMGRewardType::Credits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBonusReward = false;
};

USTRUCT(BlueprintType)
struct FMGContractRequirements
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredLicenses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompletedContracts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredCrew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCareerWins = 0;
};

USTRUCT(BlueprintType)
struct FMGContract
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ContractID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGContractType Type = EMGContractType::Race;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGContractState State = EMGContractState::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGContractDifficulty Difficulty = EMGContractDifficulty::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGContractObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGContractReward> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGContractReward> BonusRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGContractRequirements Requirements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SponsorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TeamID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ContractIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptsAllowed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptsUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressPercentage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedDate;
};

USTRUCT(BlueprintType)
struct FMGSponsorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SponsorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SponsorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Logo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxReputationLevel = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentReputation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationToNextLevel = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AvailableContracts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGContractReward> LevelRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGContractProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalContractsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCreditsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalXPEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ContractStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGContractType, int32> ContractsByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> CompletedContractIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FailedContractIDs;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContractAccepted, const FMGContract&, Contract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContractCompleted, const FMGContract&, Contract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContractFailed, const FMGContract&, Contract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgress, FName, ContractID, const FMGContractObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, FName, ContractID, const FMGContractObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardClaimed, const FMGContractReward&, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSponsorLevelUp, FName, SponsorID, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDailyContractsRefreshed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeeklyContractsRefreshed);

UCLASS()
class MIDNIGHTGRIND_API UMGContractSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Contract Management
	UFUNCTION(BlueprintCallable, Category = "Contract|Manage")
	bool AcceptContract(FName ContractID);

	UFUNCTION(BlueprintCallable, Category = "Contract|Manage")
	void AbandonContract(FName ContractID);

	UFUNCTION(BlueprintCallable, Category = "Contract|Manage")
	void CompleteContract(FName ContractID);

	UFUNCTION(BlueprintCallable, Category = "Contract|Manage")
	void FailContract(FName ContractID);

	UFUNCTION(BlueprintPure, Category = "Contract|Manage")
	FMGContract GetContract(FName ContractID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Manage")
	TArray<FMGContract> GetActiveContracts() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Manage")
	TArray<FMGContract> GetAvailableContracts() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Manage")
	TArray<FMGContract> GetContractsByType(EMGContractType Type) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Manage")
	bool CanAcceptContract(FName ContractID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Manage")
	int32 GetActiveContractCount() const;

	// Objectives
	UFUNCTION(BlueprintCallable, Category = "Contract|Objective")
	void UpdateObjectiveProgress(FName ContractID, FName ObjectiveID, float Progress);

	UFUNCTION(BlueprintCallable, Category = "Contract|Objective")
	void CompleteObjective(FName ContractID, FName ObjectiveID);

	UFUNCTION(BlueprintCallable, Category = "Contract|Objective")
	void ProcessRaceResult(int32 Position, float LapTime, int32 Overtakes, float DriftScore, bool bCleanRace);

	UFUNCTION(BlueprintPure, Category = "Contract|Objective")
	float GetObjectiveProgress(FName ContractID, FName ObjectiveID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Objective")
	bool IsObjectiveComplete(FName ContractID, FName ObjectiveID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Objective")
	TArray<FMGContractObjective> GetIncompleteObjectives(FName ContractID) const;

	// Rewards
	UFUNCTION(BlueprintCallable, Category = "Contract|Reward")
	void ClaimRewards(FName ContractID);

	UFUNCTION(BlueprintCallable, Category = "Contract|Reward")
	void ClaimBonusRewards(FName ContractID);

	UFUNCTION(BlueprintPure, Category = "Contract|Reward")
	TArray<FMGContractReward> GetPendingRewards() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Reward")
	int32 CalculateTotalCreditsReward(FName ContractID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Reward")
	int32 CalculateTotalXPReward(FName ContractID) const;

	// Daily/Weekly
	UFUNCTION(BlueprintCallable, Category = "Contract|Daily")
	void RefreshDailyContracts();

	UFUNCTION(BlueprintCallable, Category = "Contract|Daily")
	void RefreshWeeklyContracts();

	UFUNCTION(BlueprintPure, Category = "Contract|Daily")
	TArray<FMGContract> GetDailyContracts() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Daily")
	TArray<FMGContract> GetWeeklyContracts() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Daily")
	FTimespan GetTimeUntilDailyReset() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Daily")
	FTimespan GetTimeUntilWeeklyReset() const;

	// Sponsors
	UFUNCTION(BlueprintCallable, Category = "Contract|Sponsor")
	void RegisterSponsor(const FMGSponsorData& Sponsor);

	UFUNCTION(BlueprintPure, Category = "Contract|Sponsor")
	FMGSponsorData GetSponsor(FName SponsorID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Sponsor")
	TArray<FMGSponsorData> GetAllSponsors() const;

	UFUNCTION(BlueprintCallable, Category = "Contract|Sponsor")
	void AddSponsorReputation(FName SponsorID, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Contract|Sponsor")
	int32 GetSponsorLevel(FName SponsorID) const;

	UFUNCTION(BlueprintPure, Category = "Contract|Sponsor")
	TArray<FMGContract> GetSponsorContracts(FName SponsorID) const;

	// Progress
	UFUNCTION(BlueprintPure, Category = "Contract|Progress")
	FMGContractProgress GetContractProgress() const { return Progress; }

	UFUNCTION(BlueprintPure, Category = "Contract|Progress")
	int32 GetContractStreak() const { return Progress.ContractStreak; }

	UFUNCTION(BlueprintPure, Category = "Contract|Progress")
	bool IsContractCompleted(FName ContractID) const;

	UFUNCTION(BlueprintCallable, Category = "Contract|Progress")
	void ResetProgress();

	// Contract Database
	UFUNCTION(BlueprintCallable, Category = "Contract|Database")
	void RegisterContract(const FMGContract& Contract);

	UFUNCTION(BlueprintCallable, Category = "Contract|Database")
	void UnregisterContract(FName ContractID);

	UFUNCTION(BlueprintPure, Category = "Contract|Database")
	TArray<FMGContract> GetAllContracts() const;

	UFUNCTION(BlueprintPure, Category = "Contract|Database")
	int32 GetTotalContractCount() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnContractAccepted OnContractAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnContractCompleted OnContractCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnContractFailed OnContractFailed;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnObjectiveProgress OnObjectiveProgress;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnObjectiveCompleted OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnRewardClaimed OnRewardClaimed;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnSponsorLevelUp OnSponsorLevelUp;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnDailyContractsRefreshed OnDailyContractsRefreshed;

	UPROPERTY(BlueprintAssignable, Category = "Contract|Events")
	FOnWeeklyContractsRefreshed OnWeeklyContractsRefreshed;

protected:
	void OnContractTick();
	void UpdateContractTimers(float DeltaTime);
	void CheckContractExpiration();
	bool MeetsRequirements(const FMGContractRequirements& Requirements) const;
	void GrantReward(const FMGContractReward& Reward);
	void CheckAllObjectivesComplete(FName ContractID);
	void GenerateDailyContracts();
	void GenerateWeeklyContracts();
	void SaveContractData();
	void LoadContractData();

	UPROPERTY()
	TMap<FName, FMGContract> ContractDatabase;

	UPROPERTY()
	TMap<FName, FMGSponsorData> Sponsors;

	UPROPERTY()
	FMGContractProgress Progress;

	UPROPERTY()
	TArray<FName> DailyContractIDs;

	UPROPERTY()
	TArray<FName> WeeklyContractIDs;

	UPROPERTY()
	FDateTime LastDailyReset;

	UPROPERTY()
	FDateTime LastWeeklyReset;

	UPROPERTY()
	int32 MaxActiveContracts = 5;

	FTimerHandle ContractTickHandle;
};
