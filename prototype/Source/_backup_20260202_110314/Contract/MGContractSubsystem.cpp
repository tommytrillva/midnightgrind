// Copyright Midnight Grind. All Rights Reserved.

#include "Contract/MGContractSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Save/MGSaveManagerSubsystem.h"
#include "Save/MGSaveGame.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

void UMGContractSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MaxActiveContracts = 5;
	LastDailyReset = FDateTime::Now();
	LastWeeklyReset = FDateTime::Now();

	LoadContractData();

	// Start contract tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ContractTickHandle,
			this,
			&UMGContractSubsystem::OnContractTick,
			1.0f,
			true
		);
	}
}

void UMGContractSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ContractTickHandle);
	}

	SaveContractData();

	Super::Deinitialize();
}

bool UMGContractSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGContractSubsystem::OnContractTick()
{
	UpdateContractTimers(1.0f);
	CheckContractExpiration();

	// Check for daily/weekly reset
	FDateTime Now = FDateTime::Now();

	if ((Now - LastDailyReset).GetTotalHours() >= 24)
	{
		RefreshDailyContracts();
	}

	if ((Now - LastWeeklyReset).GetTotalDays() >= 7)
	{
		RefreshWeeklyContracts();
	}
}

void UMGContractSubsystem::UpdateContractTimers(float DeltaTime)
{
	for (auto& Pair : ContractDatabase)
	{
		FMGContract& Contract = Pair.Value;

		if (Contract.State == EMGContractState::Active && Contract.TimeLimit > 0)
		{
			Contract.TimeRemaining -= DeltaTime;
		}
	}
}

void UMGContractSubsystem::CheckContractExpiration()
{
	for (auto& Pair : ContractDatabase)
	{
		FMGContract& Contract = Pair.Value;

		if (Contract.State == EMGContractState::Active)
		{
			// Check time limit
			if (Contract.TimeLimit > 0 && Contract.TimeRemaining <= 0)
			{
				FailContract(Pair.Key);
			}

			// Check attempts
			if (Contract.AttemptsAllowed > 0 && Contract.AttemptsUsed >= Contract.AttemptsAllowed)
			{
				FailContract(Pair.Key);
			}
		}

		// Check expiration date
		if (Contract.State == EMGContractState::Available)
		{
			if (Contract.EndDate != FDateTime() && FDateTime::Now() > Contract.EndDate)
			{
				Contract.State = EMGContractState::Expired;
			}
		}
	}
}

bool UMGContractSubsystem::AcceptContract(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return false;
	}

	FMGContract& Contract = ContractDatabase[ContractID];

	if (Contract.State != EMGContractState::Available)
	{
		return false;
	}

	if (!CanAcceptContract(ContractID))
	{
		return false;
	}

	if (GetActiveContractCount() >= MaxActiveContracts)
	{
		return false;
	}

	Contract.State = EMGContractState::Active;
	Contract.StartDate = FDateTime::Now();
	Contract.TimeRemaining = Contract.TimeLimit;
	Contract.AttemptsUsed = 0;

	// Reset objective progress
	for (FMGContractObjective& Obj : Contract.Objectives)
	{
		Obj.CurrentValue = 0.0f;
		Obj.bCompleted = false;
	}

	OnContractAccepted.Broadcast(Contract);

	return true;
}

void UMGContractSubsystem::AbandonContract(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	FMGContract& Contract = ContractDatabase[ContractID];

	if (Contract.State != EMGContractState::Active)
	{
		return;
	}

	Contract.State = EMGContractState::Abandoned;

	// Break streak
	Progress.ContractStreak = 0;
}

void UMGContractSubsystem::CompleteContract(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	FMGContract& Contract = ContractDatabase[ContractID];

	if (Contract.State != EMGContractState::Active)
	{
		return;
	}

	Contract.State = EMGContractState::Completed;
	Contract.CompletedDate = FDateTime::Now();
	Contract.TimesCompleted++;

	// Update progress
	Progress.TotalContractsCompleted++;
	Progress.ContractStreak++;
	Progress.BestStreak = FMath::Max(Progress.BestStreak, Progress.ContractStreak);

	if (!Progress.CompletedContractIDs.Contains(ContractID))
	{
		Progress.CompletedContractIDs.Add(ContractID);
	}

	// Track by type
	if (!Progress.ContractsByType.Contains(Contract.Type))
	{
		Progress.ContractsByType.Add(Contract.Type, 0);
	}
	Progress.ContractsByType[Contract.Type]++;

	// Claim rewards
	ClaimRewards(ContractID);

	// Add sponsor reputation
	if (!Contract.SponsorID.IsNone())
	{
		int32 RepGain = 100;
		switch (Contract.Difficulty)
		{
			case EMGContractDifficulty::Easy: RepGain = 50; break;
			case EMGContractDifficulty::Normal: RepGain = 100; break;
			case EMGContractDifficulty::Hard: RepGain = 200; break;
			case EMGContractDifficulty::Expert: RepGain = 350; break;
			case EMGContractDifficulty::Legendary: RepGain = 500; break;
		}
		AddSponsorReputation(Contract.SponsorID, RepGain);
	}

	OnContractCompleted.Broadcast(Contract);

	// Make contract available again if repeatable
	if (Contract.bRepeatable)
	{
		Contract.State = EMGContractState::Available;
	}
}

void UMGContractSubsystem::FailContract(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	FMGContract& Contract = ContractDatabase[ContractID];

	if (Contract.State != EMGContractState::Active)
	{
		return;
	}

	Contract.State = EMGContractState::Failed;

	// Break streak
	Progress.ContractStreak = 0;

	if (!Progress.FailedContractIDs.Contains(ContractID))
	{
		Progress.FailedContractIDs.Add(ContractID);
	}

	OnContractFailed.Broadcast(Contract);

	// Make available again if repeatable
	if (Contract.bRepeatable)
	{
		Contract.State = EMGContractState::Available;
	}
}

FMGContract UMGContractSubsystem::GetContract(FName ContractID) const
{
	if (ContractDatabase.Contains(ContractID))
	{
		return ContractDatabase[ContractID];
	}
	return FMGContract();
}

TArray<FMGContract> UMGContractSubsystem::GetActiveContracts() const
{
	TArray<FMGContract> Active;
	for (const auto& Pair : ContractDatabase)
	{
		if (Pair.Value.State == EMGContractState::Active)
		{
			Active.Add(Pair.Value);
		}
	}
	return Active;
}

TArray<FMGContract> UMGContractSubsystem::GetAvailableContracts() const
{
	TArray<FMGContract> Available;
	for (const auto& Pair : ContractDatabase)
	{
		if (Pair.Value.State == EMGContractState::Available && CanAcceptContract(Pair.Key))
		{
			Available.Add(Pair.Value);
		}
	}
	return Available;
}

TArray<FMGContract> UMGContractSubsystem::GetContractsByType(EMGContractType Type) const
{
	TArray<FMGContract> Contracts;
	for (const auto& Pair : ContractDatabase)
	{
		if (Pair.Value.Type == Type)
		{
			Contracts.Add(Pair.Value);
		}
	}
	return Contracts;
}

bool UMGContractSubsystem::CanAcceptContract(FName ContractID) const
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return false;
	}

	const FMGContract& Contract = ContractDatabase[ContractID];

	if (Contract.State != EMGContractState::Available)
	{
		return false;
	}

	return MeetsRequirements(Contract.Requirements);
}

bool UMGContractSubsystem::MeetsRequirements(const FMGContractRequirements& Requirements) const
{
	// Would check player level, reputation, vehicles owned, etc.
	// For now, return true (would integrate with other subsystems)
	return true;
}

int32 UMGContractSubsystem::GetActiveContractCount() const
{
	return GetActiveContracts().Num();
}

void UMGContractSubsystem::UpdateObjectiveProgress(FName ContractID, FName ObjectiveID, float Progress)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	FMGContract& Contract = ContractDatabase[ContractID];

	if (Contract.State != EMGContractState::Active)
	{
		return;
	}

	for (FMGContractObjective& Obj : Contract.Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
		{
			Obj.CurrentValue = Progress;

			OnObjectiveProgress.Broadcast(ContractID, Obj);

			if (Obj.CurrentValue >= Obj.TargetValue)
			{
				CompleteObjective(ContractID, ObjectiveID);
			}

			break;
		}
	}

	// Update contract progress percentage
	float TotalProgress = 0.0f;
	int32 RequiredCount = 0;

	for (const FMGContractObjective& Obj : Contract.Objectives)
	{
		if (!Obj.bOptional)
		{
			RequiredCount++;
			TotalProgress += FMath::Clamp(Obj.CurrentValue / Obj.TargetValue, 0.0f, 1.0f);
		}
	}

	if (RequiredCount > 0)
	{
		Contract.ProgressPercentage = (TotalProgress / RequiredCount) * 100.0f;
	}
}

void UMGContractSubsystem::CompleteObjective(FName ContractID, FName ObjectiveID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	FMGContract& Contract = ContractDatabase[ContractID];

	for (FMGContractObjective& Obj : Contract.Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID)
		{
			Obj.bCompleted = true;
			Obj.CurrentValue = Obj.TargetValue;

			OnObjectiveCompleted.Broadcast(ContractID, Obj);
			break;
		}
	}

	CheckAllObjectivesComplete(ContractID);
}

void UMGContractSubsystem::CheckAllObjectivesComplete(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	const FMGContract& Contract = ContractDatabase[ContractID];

	bool bAllComplete = true;

	for (const FMGContractObjective& Obj : Contract.Objectives)
	{
		if (!Obj.bOptional && !Obj.bCompleted)
		{
			bAllComplete = false;
			break;
		}
	}

	if (bAllComplete)
	{
		CompleteContract(ContractID);
	}
}

void UMGContractSubsystem::ProcessRaceResult(int32 Position, float LapTime, int32 Overtakes, float DriftScore, bool bCleanRace)
{
	TArray<FMGContract> Active = GetActiveContracts();

	for (const FMGContract& Contract : Active)
	{
		for (const FMGContractObjective& Obj : Contract.Objectives)
		{
			if (Obj.bCompleted) continue;

			switch (Obj.Type)
			{
				case EMGObjectiveType::FinishRace:
					UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, 1.0f);
					break;

				case EMGObjectiveType::FinishPosition:
					if (Position <= Obj.TargetValue)
					{
						UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, Obj.TargetValue);
					}
					break;

				case EMGObjectiveType::WinRace:
					if (Position == 1)
					{
						UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, 1.0f);
					}
					break;

				case EMGObjectiveType::Podium:
					if (Position <= 3)
					{
						UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, 1.0f);
					}
					break;

				case EMGObjectiveType::LapTime:
					if (LapTime <= Obj.TargetValue)
					{
						UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, Obj.TargetValue);
					}
					break;

				case EMGObjectiveType::DriftScore:
					UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, DriftScore);
					break;

				case EMGObjectiveType::CleanLaps:
					if (bCleanRace)
					{
						UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID,
							Obj.CurrentValue + 1.0f);
					}
					break;

				case EMGObjectiveType::Overtakes:
					UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID,
						Obj.CurrentValue + Overtakes);
					break;

				case EMGObjectiveType::NoCollisions:
					if (bCleanRace)
					{
						UpdateObjectiveProgress(Contract.ContractID, Obj.ObjectiveID, 1.0f);
					}
					break;

				default:
					break;
			}
		}
	}
}

float UMGContractSubsystem::GetObjectiveProgress(FName ContractID, FName ObjectiveID) const
{
	FMGContract Contract = GetContract(ContractID);

	for (const FMGContractObjective& Obj : Contract.Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID)
		{
			return Obj.CurrentValue / Obj.TargetValue;
		}
	}

	return 0.0f;
}

bool UMGContractSubsystem::IsObjectiveComplete(FName ContractID, FName ObjectiveID) const
{
	FMGContract Contract = GetContract(ContractID);

	for (const FMGContractObjective& Obj : Contract.Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID)
		{
			return Obj.bCompleted;
		}
	}

	return false;
}

TArray<FMGContractObjective> UMGContractSubsystem::GetIncompleteObjectives(FName ContractID) const
{
	TArray<FMGContractObjective> Incomplete;
	FMGContract Contract = GetContract(ContractID);

	for (const FMGContractObjective& Obj : Contract.Objectives)
	{
		if (!Obj.bCompleted)
		{
			Incomplete.Add(Obj);
		}
	}

	return Incomplete;
}

void UMGContractSubsystem::ClaimRewards(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	const FMGContract& Contract = ContractDatabase[ContractID];

	for (const FMGContractReward& Reward : Contract.Rewards)
	{
		GrantReward(Reward);
	}
}

void UMGContractSubsystem::ClaimBonusRewards(FName ContractID)
{
	if (!ContractDatabase.Contains(ContractID))
	{
		return;
	}

	const FMGContract& Contract = ContractDatabase[ContractID];

	// Check if all optional objectives complete
	bool bAllOptionalComplete = true;
	for (const FMGContractObjective& Obj : Contract.Objectives)
	{
		if (Obj.bOptional && !Obj.bCompleted)
		{
			bAllOptionalComplete = false;
			break;
		}
	}

	if (bAllOptionalComplete)
	{
		for (const FMGContractReward& Reward : Contract.BonusRewards)
		{
			GrantReward(Reward);
		}
	}
}

void UMGContractSubsystem::GrantReward(const FMGContractReward& Reward)
{
	switch (Reward.Type)
	{
		case EMGRewardType::Credits:
			Progress.TotalCreditsEarned += Reward.Amount;
			break;

		case EMGRewardType::XP:
			Progress.TotalXPEarned += Reward.Amount;
			break;

		default:
			// Would integrate with inventory/progression systems
			break;
	}

	OnRewardClaimed.Broadcast(Reward);
}

TArray<FMGContractReward> UMGContractSubsystem::GetPendingRewards() const
{
	TArray<FMGContractReward> Pending;

	for (const auto& Pair : ContractDatabase)
	{
		if (Pair.Value.State == EMGContractState::Completed)
		{
			Pending.Append(Pair.Value.Rewards);
		}
	}

	return Pending;
}

int32 UMGContractSubsystem::CalculateTotalCreditsReward(FName ContractID) const
{
	int32 Total = 0;
	FMGContract Contract = GetContract(ContractID);

	for (const FMGContractReward& Reward : Contract.Rewards)
	{
		if (Reward.Type == EMGRewardType::Credits)
		{
			Total += Reward.Amount;
		}
	}

	return Total;
}

int32 UMGContractSubsystem::CalculateTotalXPReward(FName ContractID) const
{
	int32 Total = 0;
	FMGContract Contract = GetContract(ContractID);

	for (const FMGContractReward& Reward : Contract.Rewards)
	{
		if (Reward.Type == EMGRewardType::XP)
		{
			Total += Reward.Amount;
		}
	}

	return Total;
}

void UMGContractSubsystem::RefreshDailyContracts()
{
	LastDailyReset = FDateTime::Now();
	GenerateDailyContracts();
	OnDailyContractsRefreshed.Broadcast();
}

void UMGContractSubsystem::RefreshWeeklyContracts()
{
	LastWeeklyReset = FDateTime::Now();
	GenerateWeeklyContracts();
	OnWeeklyContractsRefreshed.Broadcast();
}

void UMGContractSubsystem::GenerateDailyContracts()
{
	DailyContractIDs.Empty();

	// Would generate 3 daily contracts
	// For now, just clear and broadcast
}

void UMGContractSubsystem::GenerateWeeklyContracts()
{
	WeeklyContractIDs.Empty();

	// Would generate weekly contracts
}

TArray<FMGContract> UMGContractSubsystem::GetDailyContracts() const
{
	TArray<FMGContract> Daily;
	for (const FName& ID : DailyContractIDs)
	{
		if (ContractDatabase.Contains(ID))
		{
			Daily.Add(ContractDatabase[ID]);
		}
	}
	return Daily;
}

TArray<FMGContract> UMGContractSubsystem::GetWeeklyContracts() const
{
	TArray<FMGContract> Weekly;
	for (const FName& ID : WeeklyContractIDs)
	{
		if (ContractDatabase.Contains(ID))
		{
			Weekly.Add(ContractDatabase[ID]);
		}
	}
	return Weekly;
}

FTimespan UMGContractSubsystem::GetTimeUntilDailyReset() const
{
	FDateTime NextReset = LastDailyReset + FTimespan::FromHours(24);
	return NextReset - FDateTime::Now();
}

FTimespan UMGContractSubsystem::GetTimeUntilWeeklyReset() const
{
	FDateTime NextReset = LastWeeklyReset + FTimespan::FromDays(7);
	return NextReset - FDateTime::Now();
}

void UMGContractSubsystem::RegisterSponsor(const FMGSponsorData& Sponsor)
{
	Sponsors.Add(Sponsor.SponsorID, Sponsor);
}

FMGSponsorData UMGContractSubsystem::GetSponsor(FName SponsorID) const
{
	if (Sponsors.Contains(SponsorID))
	{
		return Sponsors[SponsorID];
	}
	return FMGSponsorData();
}

TArray<FMGSponsorData> UMGContractSubsystem::GetAllSponsors() const
{
	TArray<FMGSponsorData> All;
	for (const auto& Pair : Sponsors)
	{
		All.Add(Pair.Value);
	}
	return All;
}

void UMGContractSubsystem::AddSponsorReputation(FName SponsorID, int32 Amount)
{
	if (!Sponsors.Contains(SponsorID))
	{
		return;
	}

	FMGSponsorData& Sponsor = Sponsors[SponsorID];
	Sponsor.CurrentReputation += Amount;

	// Check for level up
	while (Sponsor.CurrentReputation >= Sponsor.ReputationToNextLevel &&
		   Sponsor.ReputationLevel < Sponsor.MaxReputationLevel)
	{
		Sponsor.CurrentReputation -= Sponsor.ReputationToNextLevel;
		Sponsor.ReputationLevel++;
		Sponsor.ReputationToNextLevel = Sponsor.ReputationLevel * 1000;

		OnSponsorLevelUp.Broadcast(SponsorID, Sponsor.ReputationLevel);
	}
}

int32 UMGContractSubsystem::GetSponsorLevel(FName SponsorID) const
{
	if (Sponsors.Contains(SponsorID))
	{
		return Sponsors[SponsorID].ReputationLevel;
	}
	return 0;
}

TArray<FMGContract> UMGContractSubsystem::GetSponsorContracts(FName SponsorID) const
{
	TArray<FMGContract> SponsorContracts;
	for (const auto& Pair : ContractDatabase)
	{
		if (Pair.Value.SponsorID == SponsorID)
		{
			SponsorContracts.Add(Pair.Value);
		}
	}
	return SponsorContracts;
}

bool UMGContractSubsystem::IsContractCompleted(FName ContractID) const
{
	return Progress.CompletedContractIDs.Contains(ContractID);
}

void UMGContractSubsystem::ResetProgress()
{
	Progress = FMGContractProgress();

	for (auto& Pair : ContractDatabase)
	{
		Pair.Value.State = EMGContractState::Available;
		Pair.Value.TimesCompleted = 0;
		Pair.Value.ProgressPercentage = 0.0f;

		for (FMGContractObjective& Obj : Pair.Value.Objectives)
		{
			Obj.CurrentValue = 0.0f;
			Obj.bCompleted = false;
		}
	}
}

void UMGContractSubsystem::RegisterContract(const FMGContract& Contract)
{
	ContractDatabase.Add(Contract.ContractID, Contract);
}

void UMGContractSubsystem::UnregisterContract(FName ContractID)
{
	ContractDatabase.Remove(ContractID);
}

TArray<FMGContract> UMGContractSubsystem::GetAllContracts() const
{
	TArray<FMGContract> All;
	for (const auto& Pair : ContractDatabase)
	{
		All.Add(Pair.Value);
	}
	return All;
}

int32 UMGContractSubsystem::GetTotalContractCount() const
{
	return ContractDatabase.Num();
}

void UMGContractSubsystem::SaveContractData()
{
	// Save contract progress to file
	FString ContractsDir = FPaths::ProjectSavedDir() / TEXT("Contracts");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*ContractsDir))
	{
		PlatformFile.CreateDirectory(*ContractsDir);
	}

	FString FilePath = ContractsDir / TEXT("contract_progress.dat");

	FBufferArchive Archive;

	// Write version
	int32 Version = 1;
	Archive << Version;

	// Write progress data
	Archive << Progress.TotalContractsCompleted;
	Archive << Progress.TotalCreditsEarned;
	Archive << Progress.TotalXPEarned;
	Archive << Progress.ContractStreak;
	Archive << Progress.BestStreak;
	Archive << Progress.CompletedContractIDs;
	Archive << Progress.FailedContractIDs;

	int32 TypeCount = Progress.ContractsByType.Num();
	Archive << TypeCount;
	for (const auto& Pair : Progress.ContractsByType)
	{
		int32 TypeInt = static_cast<int32>(Pair.Key);
		int32 Count = Pair.Value;
		Archive << TypeInt;
		Archive << Count;
	}

	// Write reset timestamps
	Archive << LastDailyReset;
	Archive << LastWeeklyReset;

	// Write active contracts state
	TArray<FName> ActiveContractIDs;
	for (const auto& Pair : ContractDatabase)
	{
		if (Pair.Value.State == EMGContractState::Active)
		{
			ActiveContractIDs.Add(Pair.Key);
		}
	}
	Archive << ActiveContractIDs;

	// Write sponsor reputation
	int32 SponsorCount = Sponsors.Num();
	Archive << SponsorCount;
	for (const auto& Pair : Sponsors)
	{
		FName SponsorID = Pair.Key;
		int32 RepLevel = Pair.Value.ReputationLevel;
		int32 RepCurrent = Pair.Value.CurrentReputation;
		Archive << SponsorID;
		Archive << RepLevel;
		Archive << RepCurrent;
	}

	FFileHelper::SaveArrayToFile(Archive, *FilePath);
	Archive.FlushCache();
	Archive.Empty();

	UE_LOG(LogTemp, Log, TEXT("MGContract: Saved contract progress (%d completed, streak %d)"),
		Progress.TotalContractsCompleted, Progress.ContractStreak);
}

void UMGContractSubsystem::LoadContractData()
{
	FString ContractsDir = FPaths::ProjectSavedDir() / TEXT("Contracts");
	FString FilePath = ContractsDir / TEXT("contract_progress.dat");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		// No save file yet
		return;
	}

	FMemoryReader Archive(FileData, true);

	// Read version
	int32 Version;
	Archive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGContract: Unknown contract save version %d"), Version);
		return;
	}

	// Read progress data
	Archive << Progress.TotalContractsCompleted;
	Archive << Progress.TotalCreditsEarned;
	Archive << Progress.TotalXPEarned;
	Archive << Progress.ContractStreak;
	Archive << Progress.BestStreak;
	Archive << Progress.CompletedContractIDs;
	Archive << Progress.FailedContractIDs;

	int32 TypeCount;
	Archive << TypeCount;
	for (int32 i = 0; i < TypeCount; i++)
	{
		int32 TypeInt;
		int32 Count;
		Archive << TypeInt;
		Archive << Count;
		Progress.ContractsByType.Add(static_cast<EMGContractType>(TypeInt), Count);
	}

	// Read reset timestamps
	Archive << LastDailyReset;
	Archive << LastWeeklyReset;

	// Read active contracts state
	TArray<FName> ActiveContractIDs;
	Archive << ActiveContractIDs;

	// Restore active contract states
	for (const FName& ContractID : ActiveContractIDs)
	{
		if (ContractDatabase.Contains(ContractID))
		{
			ContractDatabase[ContractID].State = EMGContractState::Active;
		}
	}

	// Read sponsor reputation
	int32 SponsorCount;
	Archive << SponsorCount;
	for (int32 i = 0; i < SponsorCount; i++)
	{
		FName SponsorID;
		int32 RepLevel;
		int32 RepCurrent;
		Archive << SponsorID;
		Archive << RepLevel;
		Archive << RepCurrent;

		if (Sponsors.Contains(SponsorID))
		{
			Sponsors[SponsorID].ReputationLevel = RepLevel;
			Sponsors[SponsorID].CurrentReputation = RepCurrent;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MGContract: Loaded contract progress (%d completed, streak %d)"),
		Progress.TotalContractsCompleted, Progress.ContractStreak);
}
