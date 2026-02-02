// Copyright Midnight Grind. All Rights Reserved.

#include "Economy/MGMechanicSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

namespace
{
	/**
	 * Maps part category to mechanic specialization
	 * Used for matching mechanics to parts they specialize in
	 */
	EMGMechanicSpecialization PartCategoryToMechanicSpecialization(EMGPartCategory Category)
	{
		switch (Category)
		{
		case EMGPartCategory::Engine:
			return EMGMechanicSpecialization::Engine;

		case EMGPartCategory::Drivetrain:
			return EMGMechanicSpecialization::Transmission;

		case EMGPartCategory::Suspension:
		case EMGPartCategory::Brakes:
			return EMGMechanicSpecialization::Suspension;

		case EMGPartCategory::Wheels:
		case EMGPartCategory::Tires:
			return EMGMechanicSpecialization::General;

		case EMGPartCategory::Aero:
		case EMGPartCategory::Body:
			return EMGMechanicSpecialization::Bodywork;

		case EMGPartCategory::Nitrous:
		case EMGPartCategory::Electronics:
			return EMGMechanicSpecialization::Electrical;

		case EMGPartCategory::Interior:
		default:
			return EMGMechanicSpecialization::General;
		}
	}
}

void UMGMechanicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeMechanics();
	InitializeSpecialAbilities();
}

void UMGMechanicSubsystem::Deinitialize()
{
	Mechanics.Empty();
	Relationships.Empty();
	ActiveJobs.Empty();
	JobHistory.Empty();
	Super::Deinitialize();
}

void UMGMechanicSubsystem::InitializeMechanics()
{
	// Dwayne "Wrenches" Washington - The OG mentor mechanic
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("Wrenches");
		Mechanic.DisplayName = FText::FromString(TEXT("Dwayne \"Wrenches\" Washington"));
		Mechanic.Backstory = FText::FromString(TEXT("Former NASCAR pit crew chief who got tired of the politics. Runs a legit shop by day, but his real passion is building street monsters. He's seen it all and knows everyone."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Master;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::Engine;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::General;
		Mechanic.Personality = EMGMechanicPersonality::Mentor;
		Mechanic.CostMultiplier = 1.2f;
		Mechanic.SpeedMultiplier = 1.3f;
		Mechanic.QualityRating = 95;
		Mechanic.bHasUndergroundConnections = true;
		Mechanic.MinimumTrustRequired = 0;
		Mechanic.GarageName = FText::FromString(TEXT("Washington's Performance"));
		Mechanic.SpecialAbilities.Add(TEXT("TeachBasics"));
		Mechanic.SpecialAbilities.Add(TEXT("NetworkIntro"));
		Mechanic.SpecialAbilities.Add(TEXT("EngineSecrets"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// "Fast Eddie" Tanaka - The speed specialist
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("FastEddie");
		Mechanic.DisplayName = FText::FromString(TEXT("\"Fast Eddie\" Tanaka"));
		Mechanic.Backstory = FText::FromString(TEXT("Tokyo drift scene legend who moved to the States. His work is expensive and he's got an ego, but nobody tunes imports better. If you want JDM perfection, Eddie's your guy."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Legend;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::Engine;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::Electrical;
		Mechanic.Personality = EMGMechanicPersonality::Perfectionist;
		Mechanic.CostMultiplier = 2.0f;
		Mechanic.SpeedMultiplier = 0.7f; // Slow but perfect
		Mechanic.QualityRating = 100;
		Mechanic.bHasUndergroundConnections = true;
		Mechanic.MinimumTrustRequired = 40;
		Mechanic.GarageName = FText::FromString(TEXT("Tanaka Tuning"));
		Mechanic.SpecialAbilities.Add(TEXT("JDMParts"));
		Mechanic.SpecialAbilities.Add(TEXT("PrecisionTune"));
		Mechanic.SpecialAbilities.Add(TEXT("ECUMagic"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// Maria "La Diabla" Reyes - Suspension queen
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("LaDiabla");
		Mechanic.DisplayName = FText::FromString(TEXT("Maria \"La Diabla\" Reyes"));
		Mechanic.Backstory = FText::FromString(TEXT("Started in lowrider custom shops, evolved into the most sought-after suspension specialist in the underground. Her cars handle like they're on rails."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Expert;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::Suspension;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::Bodywork;
		Mechanic.Personality = EMGMechanicPersonality::Professional;
		Mechanic.CostMultiplier = 1.4f;
		Mechanic.SpeedMultiplier = 1.0f;
		Mechanic.QualityRating = 90;
		Mechanic.bHasUndergroundConnections = false;
		Mechanic.MinimumTrustRequired = 15;
		Mechanic.GarageName = FText::FromString(TEXT("Diablo Customs"));
		Mechanic.SpecialAbilities.Add(TEXT("CornerBalance"));
		Mechanic.SpecialAbilities.Add(TEXT("CustomCoilovers"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// Jerome "Gearbox" Mitchell - Transmission wizard
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("Gearbox");
		Mechanic.DisplayName = FText::FromString(TEXT("Jerome \"Gearbox\" Mitchell"));
		Mechanic.Backstory = FText::FromString(TEXT("Salvage yard kid who taught himself everything about transmissions. Can rebuild any gearbox blindfolded. Runs a sketchy-looking shop but does pristine work."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Expert;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::Transmission;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::General;
		Mechanic.Personality = EMGMechanicPersonality::Underground;
		Mechanic.CostMultiplier = 0.9f;
		Mechanic.SpeedMultiplier = 1.1f;
		Mechanic.QualityRating = 88;
		Mechanic.bHasUndergroundConnections = true;
		Mechanic.MinimumTrustRequired = 25;
		Mechanic.GarageName = FText::FromString(TEXT("Mitchell's Transmissions"));
		Mechanic.SpecialAbilities.Add(TEXT("QuickShiftKit"));
		Mechanic.SpecialAbilities.Add(TEXT("LSDTuning"));
		Mechanic.SpecialAbilities.Add(TEXT("SalvageParts"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// Bobby "Nitro" Kowalski - The rush job king
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("Nitro");
		Mechanic.DisplayName = FText::FromString(TEXT("Bobby \"Nitro\" Kowalski"));
		Mechanic.Backstory = FText::FromString(TEXT("High-energy, caffeine-fueled mechanic who never sleeps. His work is decent and he's the fastest wrench in town. When you need it done NOW, Nitro's your call."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Journeyman;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::Electrical;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::Engine;
		Mechanic.Personality = EMGMechanicPersonality::Hustler;
		Mechanic.CostMultiplier = 1.3f;
		Mechanic.SpeedMultiplier = 2.0f; // Fastest in town
		Mechanic.QualityRating = 70;
		Mechanic.bHasUndergroundConnections = true;
		Mechanic.MinimumTrustRequired = 0;
		Mechanic.GarageName = FText::FromString(TEXT("Nitro's 24/7"));
		Mechanic.SpecialAbilities.Add(TEXT("EmergencyService"));
		Mechanic.SpecialAbilities.Add(TEXT("NitrousInstall"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// Old Man Henderson - Classic car specialist
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("Henderson");
		Mechanic.DisplayName = FText::FromString(TEXT("Old Man Henderson"));
		Mechanic.Backstory = FText::FromString(TEXT("Been wrenching since the muscle car era. Knows every classic American V8 inside and out. Doesn't trust computers and thinks fuel injection is cheating, but his carb work is unmatched."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Legend;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::Restoration;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::Engine;
		Mechanic.Personality = EMGMechanicPersonality::OldSchool;
		Mechanic.CostMultiplier = 1.5f;
		Mechanic.SpeedMultiplier = 0.6f; // Slow and methodical
		Mechanic.QualityRating = 98;
		Mechanic.bHasUndergroundConnections = true;
		Mechanic.MinimumTrustRequired = 30;
		Mechanic.GarageName = FText::FromString(TEXT("Henderson's Classics"));
		Mechanic.SpecialAbilities.Add(TEXT("RarePartSource"));
		Mechanic.SpecialAbilities.Add(TEXT("ClassicTuning"));
		Mechanic.SpecialAbilities.Add(TEXT("CarbMagic"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// Rookie - Entry level mechanic
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("Rookie");
		Mechanic.DisplayName = FText::FromString(TEXT("Jake \"Rookie\" Palmer"));
		Mechanic.Backstory = FText::FromString(TEXT("Fresh out of trade school with enthusiasm but limited experience. Cheap as dirt and eager to learn. Sometimes things don't go as planned, but he's getting better."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Apprentice;
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::General;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::General;
		Mechanic.Personality = EMGMechanicPersonality::Professional;
		Mechanic.CostMultiplier = 0.5f;
		Mechanic.SpeedMultiplier = 0.7f;
		Mechanic.QualityRating = 50;
		Mechanic.bHasUndergroundConnections = false;
		Mechanic.MinimumTrustRequired = 0;
		Mechanic.GarageName = FText::FromString(TEXT("Jake's Garage"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}

	// DIY - Do it yourself option
	{
		FMGMechanic Mechanic;
		Mechanic.MechanicID = TEXT("DIY");
		Mechanic.DisplayName = FText::FromString(TEXT("Do It Yourself"));
		Mechanic.Backstory = FText::FromString(TEXT("Your own two hands and whatever tools you can find. Free, but success depends entirely on your skill and luck."));
		Mechanic.SkillTier = EMGMechanicSkillTier::Apprentice; // Player skill determines actual tier
		Mechanic.PrimarySpecialization = EMGMechanicSpecialization::General;
		Mechanic.SecondarySpecialization = EMGMechanicSpecialization::General;
		Mechanic.Personality = EMGMechanicPersonality::Professional;
		Mechanic.CostMultiplier = 0.0f; // Free (labor)
		Mechanic.SpeedMultiplier = 0.5f;
		Mechanic.QualityRating = 40;
		Mechanic.bHasUndergroundConnections = false;
		Mechanic.MinimumTrustRequired = 0;
		Mechanic.GarageName = FText::FromString(TEXT("Your Garage"));
		Mechanics.Add(Mechanic.MechanicID, Mechanic);
	}
}

void UMGMechanicSubsystem::InitializeSpecialAbilities()
{
	// Trust levels required to unlock abilities
	AbilityTrustRequirements.Add(TEXT("TeachBasics"), 10);
	AbilityTrustRequirements.Add(TEXT("NetworkIntro"), 30);
	AbilityTrustRequirements.Add(TEXT("EngineSecrets"), 60);
	AbilityTrustRequirements.Add(TEXT("JDMParts"), 50);
	AbilityTrustRequirements.Add(TEXT("PrecisionTune"), 70);
	AbilityTrustRequirements.Add(TEXT("ECUMagic"), 90);
	AbilityTrustRequirements.Add(TEXT("CornerBalance"), 25);
	AbilityTrustRequirements.Add(TEXT("CustomCoilovers"), 50);
	AbilityTrustRequirements.Add(TEXT("QuickShiftKit"), 30);
	AbilityTrustRequirements.Add(TEXT("LSDTuning"), 50);
	AbilityTrustRequirements.Add(TEXT("SalvageParts"), 40);
	AbilityTrustRequirements.Add(TEXT("EmergencyService"), 20);
	AbilityTrustRequirements.Add(TEXT("NitrousInstall"), 35);
	AbilityTrustRequirements.Add(TEXT("RarePartSource"), 50);
	AbilityTrustRequirements.Add(TEXT("ClassicTuning"), 40);
	AbilityTrustRequirements.Add(TEXT("CarbMagic"), 70);
}

// ==================== Mechanic Discovery ====================

TArray<FMGMechanic> UMGMechanicSubsystem::GetAllMechanics() const
{
	TArray<FMGMechanic> Result;
	Mechanics.GenerateValueArray(Result);
	return Result;
}

TArray<FMGMechanic> UMGMechanicSubsystem::GetAvailableMechanics() const
{
	TArray<FMGMechanic> Result;
	for (const auto& Pair : Mechanics)
	{
		if (IsMechanicAvailable(Pair.Value))
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool UMGMechanicSubsystem::GetMechanic(FName MechanicID, FMGMechanic& OutMechanic) const
{
	if (const FMGMechanic* Found = Mechanics.Find(MechanicID))
	{
		OutMechanic = *Found;
		return true;
	}
	return false;
}

TArray<FMGMechanic> UMGMechanicSubsystem::GetMechanicsBySpecialization(EMGMechanicSpecialization Specialization) const
{
	TArray<FMGMechanic> Result;
	for (const auto& Pair : Mechanics)
	{
		if (Pair.Value.PrimarySpecialization == Specialization ||
			Pair.Value.SecondarySpecialization == Specialization)
		{
			Result.Add(Pair.Value);
		}
	}

	// Sort by quality rating (best first)
	Result.Sort([](const FMGMechanic& A, const FMGMechanic& B)
	{
		return A.QualityRating > B.QualityRating;
	});

	return Result;
}

FName UMGMechanicSubsystem::GetRecommendedMechanic(FName PartID, EMGMechanicService ServiceType) const
{
	TArray<FMGMechanic> Available = GetAvailableMechanics();
	if (Available.Num() == 0)
	{
		return TEXT("DIY");
	}

	// Determine required specialization from part catalog
	EMGMechanicSpecialization RequiredSpec = EMGMechanicSpecialization::General;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGPartsCatalogSubsystem* PartsCatalog = GI->GetSubsystem<UMGPartsCatalogSubsystem>())
		{
			EMGPartCategory PartCategory = PartsCatalog->GetPartCategory(PartID);
			RequiredSpec = PartCategoryToMechanicSpecialization(PartCategory);
		}
	}

	// Sort by: specialization match > quality > trust
	Available.Sort([this, RequiredSpec](const FMGMechanic& A, const FMGMechanic& B)
	{
		// Check specialization match (primary or secondary)
		const bool AMatchesPrimary = (A.PrimarySpecialization == RequiredSpec);
		const bool AMatchesSecondary = (A.SecondarySpecialization == RequiredSpec);
		const bool AMatches = AMatchesPrimary || AMatchesSecondary || (RequiredSpec == EMGMechanicSpecialization::General);

		const bool BMatchesPrimary = (B.PrimarySpecialization == RequiredSpec);
		const bool BMatchesSecondary = (B.SecondarySpecialization == RequiredSpec);
		const bool BMatches = BMatchesPrimary || BMatchesSecondary || (RequiredSpec == EMGMechanicSpecialization::General);

		// Specialization match is most important
		if (AMatches != BMatches)
		{
			return AMatches;
		}

		// Primary specialization match > secondary
		if (AMatches && BMatches)
		{
			if (AMatchesPrimary != BMatchesPrimary)
			{
				return AMatchesPrimary;
			}
		}

		// Then sort by quality and trust
		const int32 TrustA = GetTrustLevel(A.MechanicID);
		const int32 TrustB = GetTrustLevel(B.MechanicID);

		const float ScoreA = A.QualityRating * 0.7f + TrustA * 0.3f;
		const float ScoreB = B.QualityRating * 0.7f + TrustB * 0.3f;

		return ScoreA > ScoreB;
	});

	return Available[0].MechanicID;
}

// ==================== Job Management ====================

FGuid UMGMechanicSubsystem::StartJob(FName MechanicID, FName VehicleID, FName PartID, EMGMechanicService ServiceType, bool bRushJob)
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return FGuid();
	}

	if (!IsMechanicAvailable(Mechanic))
	{
		return FGuid();
	}

	FMGMechanicJob Job;
	Job.JobID = FGuid::NewGuid();
	Job.MechanicID = MechanicID;
	Job.VehicleID = VehicleID;
	Job.PartID = PartID;
	Job.ServiceType = ServiceType;
	Job.bIsRushJob = bRushJob;
	Job.Cost = CalculateJobCost(Mechanic, PartID, ServiceType, bRushJob);
	Job.StartTime = FDateTime::Now();

	const float Duration = CalculateJobDuration(Mechanic, PartID, ServiceType, bRushJob);
	Job.EstimatedCompletion = Job.StartTime + FTimespan::FromHours(Duration);
	Job.bIsComplete = false;

	ActiveJobs.Add(Job.JobID, Job);

	// Update relationship - they're spending money
	if (FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		Relationship->TotalMoneySpent += Job.Cost;
	}
	else
	{
		FMGMechanicRelationship NewRelationship;
		NewRelationship.MechanicID = MechanicID;
		NewRelationship.TotalMoneySpent = Job.Cost;
		NewRelationship.FirstInteraction = FDateTime::Now();
		Relationships.Add(MechanicID, NewRelationship);
	}

	OnMechanicJobStarted.Broadcast(MechanicID, Job.JobID, ServiceType);

	return Job.JobID;
}

int32 UMGMechanicSubsystem::GetJobEstimate(FName MechanicID, FName PartID, EMGMechanicService ServiceType, bool bRushJob) const
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return 0;
	}

	int32 BaseCost = CalculateJobCost(Mechanic, PartID, ServiceType, bRushJob);

	// Apply loyalty discount
	const float Discount = GetLoyaltyDiscount(MechanicID);
	return FMath::RoundToInt(BaseCost * (1.0f - Discount));
}

float UMGMechanicSubsystem::GetJobDuration(FName MechanicID, FName PartID, EMGMechanicService ServiceType, bool bRushJob) const
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return 0.0f;
	}

	return CalculateJobDuration(Mechanic, PartID, ServiceType, bRushJob);
}

bool UMGMechanicSubsystem::IsJobComplete(const FGuid& JobID) const
{
	if (const FMGMechanicJob* Job = ActiveJobs.Find(JobID))
	{
		return Job->bIsComplete || FDateTime::Now() >= Job->EstimatedCompletion;
	}
	return false;
}

FMGMechanicJob UMGMechanicSubsystem::CompleteJob(const FGuid& JobID)
{
	FMGMechanicJob CompletedJob;

	if (FMGMechanicJob* Job = ActiveJobs.Find(JobID))
	{
		FMGMechanic Mechanic;
		if (GetMechanic(Job->MechanicID, Mechanic))
		{
			// Determine work result
			Job->Result = SimulateWorkResult(Job->MechanicID, Job->ServiceType, Job->bIsRushJob);
			Job->QualityModifier = CalculateQualityModifier(Mechanic, Job->Result);
			Job->bIsComplete = true;

			// Update relationship
			if (FMGMechanicRelationship* Relationship = Relationships.Find(Job->MechanicID))
			{
				Relationship->JobsCompleted++;

				if (Job->Result == EMGWorkResult::Botched || Job->Result == EMGWorkResult::Failed)
				{
					Relationship->BotchedJobs++;
					// Lose some trust on bad work
					AddTrust(Job->MechanicID, -5);
				}
				else
				{
					// Gain trust for completed jobs
					int32 TrustGain = 2;
					if (Job->Result == EMGWorkResult::Perfect)
					{
						TrustGain = 5;
					}
					AddTrust(Job->MechanicID, TrustGain);
				}
			}
		}

		CompletedJob = *Job;

		// Move to history
		JobHistory.Insert(CompletedJob, 0);
		if (JobHistory.Num() > 100)
		{
			JobHistory.SetNum(100);
		}

		ActiveJobs.Remove(JobID);

		OnMechanicJobCompleted.Broadcast(JobID, CompletedJob.Result, CompletedJob.QualityModifier);
	}

	return CompletedJob;
}

TArray<FMGMechanicJob> UMGMechanicSubsystem::GetActiveJobs() const
{
	TArray<FMGMechanicJob> Result;
	ActiveJobs.GenerateValueArray(Result);
	return Result;
}

TArray<FMGMechanicJob> UMGMechanicSubsystem::GetJobHistory(int32 Count) const
{
	TArray<FMGMechanicJob> Result;
	const int32 NumToReturn = FMath::Min(Count, JobHistory.Num());
	for (int32 i = 0; i < NumToReturn; ++i)
	{
		Result.Add(JobHistory[i]);
	}
	return Result;
}

bool UMGMechanicSubsystem::CancelJob(const FGuid& JobID, int32& OutRefundAmount)
{
	if (FMGMechanicJob* Job = ActiveJobs.Find(JobID))
	{
		// Refund is based on how far along the job is
		const FDateTime Now = FDateTime::Now();
		const FTimespan TotalDuration = Job->EstimatedCompletion - Job->StartTime;
		const FTimespan Elapsed = Now - Job->StartTime;

		float CompletionRatio = 0.0f;
		if (TotalDuration.GetTotalSeconds() > 0)
		{
			CompletionRatio = FMath::Clamp(Elapsed.GetTotalSeconds() / TotalDuration.GetTotalSeconds(), 0.0, 1.0);
		}

		// Get 50% base refund, minus work already done
		const float RefundRatio = FMath::Max(0.0f, 0.5f - CompletionRatio * 0.5f);
		OutRefundAmount = FMath::RoundToInt(Job->Cost * RefundRatio);

		ActiveJobs.Remove(JobID);
		return true;
	}

	OutRefundAmount = 0;
	return false;
}

// ==================== Trust & Relationships ====================

FMGMechanicRelationship UMGMechanicSubsystem::GetMechanicRelationship(FName MechanicID) const
{
	if (const FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		return *Relationship;
	}

	FMGMechanicRelationship Default;
	Default.MechanicID = MechanicID;
	return Default;
}

int32 UMGMechanicSubsystem::GetTrustLevel(FName MechanicID) const
{
	if (const FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		return Relationship->TrustLevel;
	}
	return 0;
}

void UMGMechanicSubsystem::AddTrust(FName MechanicID, int32 Amount)
{
	FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID);
	if (!Relationship)
	{
		FMGMechanicRelationship NewRelationship;
		NewRelationship.MechanicID = MechanicID;
		NewRelationship.FirstInteraction = FDateTime::Now();
		Relationships.Add(MechanicID, NewRelationship);
		Relationship = Relationships.Find(MechanicID);
	}

	const int32 OldTrust = Relationship->TrustLevel;
	Relationship->TrustLevel = FMath::Clamp(Relationship->TrustLevel + Amount, 0, 100);
	const int32 NewTrust = Relationship->TrustLevel;

	if (OldTrust != NewTrust)
	{
		OnMechanicTrustChanged.Broadcast(MechanicID, NewTrust);
		UpdateTrustMilestones(MechanicID, OldTrust, NewTrust);
		UpdateLoyaltyDiscount(MechanicID);
		UnlockTrustServices(MechanicID, NewTrust);
	}
}

float UMGMechanicSubsystem::GetLoyaltyDiscount(FName MechanicID) const
{
	if (const FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		return Relationship->LoyaltyDiscount;
	}
	return 0.0f;
}

void UMGMechanicSubsystem::SetPreferredMechanic(FName MechanicID)
{
	// Clear old preferred
	for (auto& Pair : Relationships)
	{
		Pair.Value.bIsPreferred = false;
	}

	PreferredMechanicID = MechanicID;

	if (FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		Relationship->bIsPreferred = true;
	}
}

FName UMGMechanicSubsystem::GetPreferredMechanic() const
{
	return PreferredMechanicID;
}

bool UMGMechanicSubsystem::IsServiceUnlocked(FName MechanicID, FName ServiceName) const
{
	if (const FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		return Relationship->UnlockedServices.Contains(ServiceName);
	}
	return false;
}

TArray<FName> UMGMechanicSubsystem::GetUnlockedServices(FName MechanicID) const
{
	if (const FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		return Relationship->UnlockedServices;
	}
	return TArray<FName>();
}

// ==================== Work Quality ====================

float UMGMechanicSubsystem::GetExpectedQuality(FName MechanicID, FName PartID, EMGMechanicService ServiceType) const
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return 50.0f;
	}

	// Base quality from mechanic's rating
	float Quality = Mechanic.QualityRating;

	// Bonus for trust
	const int32 Trust = GetTrustLevel(MechanicID);
	Quality += Trust * 0.05f;

	// Check if part matches mechanic's specialization for bonus
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGPartsCatalogSubsystem* PartsCatalog = GI->GetSubsystem<UMGPartsCatalogSubsystem>())
		{
			EMGPartCategory PartCategory = PartsCatalog->GetPartCategory(PartID);
			EMGMechanicSpecialization RequiredSpec = PartCategoryToMechanicSpecialization(PartCategory);

			// Primary specialization match: +10% quality bonus
			if (Mechanic.PrimarySpecialization == RequiredSpec)
			{
				Quality += 10.0f;
			}
			// Secondary specialization match: +5% quality bonus
			else if (Mechanic.SecondarySpecialization == RequiredSpec)
			{
				Quality += 5.0f;
			}
			// General specialists get a small bonus on everything
			else if (Mechanic.PrimarySpecialization == EMGMechanicSpecialization::General)
			{
				Quality += 2.0f;
			}
		}
	}

	return FMath::Clamp(Quality, 0.0f, 100.0f);
}

void UMGMechanicSubsystem::GetWorkResultProbabilities(FName MechanicID, EMGMechanicService ServiceType,
	float& OutPerfect, float& OutGood, float& OutAcceptable, float& OutBotched, float& OutFailed) const
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		OutPerfect = 0.0f;
		OutGood = 0.2f;
		OutAcceptable = 0.3f;
		OutBotched = 0.3f;
		OutFailed = 0.2f;
		return;
	}

	// Base probabilities by skill tier
	switch (Mechanic.SkillTier)
	{
	case EMGMechanicSkillTier::Apprentice:
		OutPerfect = 0.05f;
		OutGood = 0.25f;
		OutAcceptable = 0.40f;
		OutBotched = 0.20f;
		OutFailed = 0.10f;
		break;

	case EMGMechanicSkillTier::Journeyman:
		OutPerfect = 0.15f;
		OutGood = 0.45f;
		OutAcceptable = 0.30f;
		OutBotched = 0.08f;
		OutFailed = 0.02f;
		break;

	case EMGMechanicSkillTier::Expert:
		OutPerfect = 0.25f;
		OutGood = 0.55f;
		OutAcceptable = 0.15f;
		OutBotched = 0.04f;
		OutFailed = 0.01f;
		break;

	case EMGMechanicSkillTier::Master:
		OutPerfect = 0.40f;
		OutGood = 0.50f;
		OutAcceptable = 0.08f;
		OutBotched = 0.02f;
		OutFailed = 0.0f;
		break;

	case EMGMechanicSkillTier::Legend:
		OutPerfect = 0.60f;
		OutGood = 0.35f;
		OutAcceptable = 0.05f;
		OutBotched = 0.0f;
		OutFailed = 0.0f;
		break;
	}

	// Modify based on trust
	const int32 Trust = GetTrustLevel(MechanicID);
	const float TrustBonus = Trust * 0.001f;
	OutPerfect += TrustBonus;
	OutBotched = FMath::Max(0.0f, OutBotched - TrustBonus);

	// Normalize
	const float Total = OutPerfect + OutGood + OutAcceptable + OutBotched + OutFailed;
	if (Total > 0.0f)
	{
		OutPerfect /= Total;
		OutGood /= Total;
		OutAcceptable /= Total;
		OutBotched /= Total;
		OutFailed /= Total;
	}
}

EMGWorkResult UMGMechanicSubsystem::SimulateWorkResult(FName MechanicID, EMGMechanicService ServiceType, bool bRushJob) const
{
	float Perfect, Good, Acceptable, Botched, Failed;
	GetWorkResultProbabilities(MechanicID, ServiceType, Perfect, Good, Acceptable, Botched, Failed);

	// Rush jobs increase failure chance
	if (bRushJob)
	{
		Perfect *= 0.5f;
		Botched *= 2.0f;
		Failed *= 2.0f;

		// Renormalize
		const float Total = Perfect + Good + Acceptable + Botched + Failed;
		Perfect /= Total;
		Good /= Total;
		Acceptable /= Total;
		Botched /= Total;
		Failed /= Total;
	}

	const float Roll = FMath::FRand();
	float Cumulative = 0.0f;

	Cumulative += Perfect;
	if (Roll < Cumulative) return EMGWorkResult::Perfect;

	Cumulative += Good;
	if (Roll < Cumulative) return EMGWorkResult::Good;

	Cumulative += Acceptable;
	if (Roll < Cumulative) return EMGWorkResult::Acceptable;

	Cumulative += Botched;
	if (Roll < Cumulative) return EMGWorkResult::Botched;

	return EMGWorkResult::Failed;
}

// ==================== Special Abilities ====================

bool UMGMechanicSubsystem::HasSpecialAbility(FName MechanicID, FName AbilityName) const
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return false;
	}

	// Check if mechanic has this ability
	if (!Mechanic.SpecialAbilities.Contains(AbilityName))
	{
		return false;
	}

	// Check if trust level is high enough
	if (const int32* RequiredTrust = AbilityTrustRequirements.Find(AbilityName))
	{
		const int32 CurrentTrust = GetTrustLevel(MechanicID);
		return CurrentTrust >= *RequiredTrust;
	}

	return true;
}

bool UMGMechanicSubsystem::RequestBlackMarketReferral(FName MechanicID, FName& OutDealerID)
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return false;
	}

	if (!Mechanic.bHasUndergroundConnections)
	{
		return false;
	}

	const int32 Trust = GetTrustLevel(MechanicID);
	if (Trust < 30)
	{
		return false;
	}

	// Update relationship
	if (FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		Relationship->PartsReferred++;
	}

	// Each mechanic refers to different dealers based on their connections
	if (MechanicID == TEXT("Wrenches"))
	{
		OutDealerID = TEXT("Vinnie"); // Knows everyone
	}
	else if (MechanicID == TEXT("FastEddie"))
	{
		OutDealerID = TEXT("Miko"); // JDM connections
	}
	else if (MechanicID == TEXT("Gearbox"))
	{
		OutDealerID = TEXT("Shadow"); // Underground
	}
	else if (MechanicID == TEXT("Nitro"))
	{
		OutDealerID = TEXT("Rico"); // Fast and loose
	}
	else if (MechanicID == TEXT("Henderson"))
	{
		OutDealerID = TEXT("Ghost"); // Classic parts
	}
	else
	{
		OutDealerID = TEXT("Vinnie"); // Default
	}

	return true;
}

FText UMGMechanicSubsystem::GetRarePartTip(FName MechanicID) const
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return FText::GetEmpty();
	}

	const int32 Trust = GetTrustLevel(MechanicID);

	if (Trust < 20)
	{
		return FText::FromString(TEXT("I don't really know you well enough to share that kind of info."));
	}

	// Tips based on mechanic's specialization and trust level
	TArray<FText> Tips;

	if (Mechanic.PrimarySpecialization == EMGMechanicSpecialization::Engine)
	{
		Tips.Add(FText::FromString(TEXT("Heard there's a guy at the docks with some JDM turbos. Ask around for 'Ghost'.")));
		Tips.Add(FText::FromString(TEXT("If you want real power, you need forged internals. I might know someone...")));
	}
	else if (Mechanic.PrimarySpecialization == EMGMechanicSpecialization::Suspension)
	{
		Tips.Add(FText::FromString(TEXT("The rally teams sometimes sell off their custom suspension setups. Keep an eye out.")));
		Tips.Add(FText::FromString(TEXT("I know a fabricator who makes one-off control arms. Want me to connect you?")));
	}
	else if (Mechanic.PrimarySpecialization == EMGMechanicSpecialization::Transmission)
	{
		Tips.Add(FText::FromString(TEXT("There's a shipment of sequential gearboxes coming in next week. Know a guy.")));
		Tips.Add(FText::FromString(TEXT("The dog box from a WRC car went 'missing' recently. Just saying.")));
	}
	else if (Mechanic.PrimarySpecialization == EMGMechanicSpecialization::Restoration)
	{
		Tips.Add(FText::FromString(TEXT("Estate sale next weekend. Old timer had a barn full of NOS parts.")));
		Tips.Add(FText::FromString(TEXT("I got connections to some overseas parts catalogues. Original stuff.")));
	}
	else
	{
		Tips.Add(FText::FromString(TEXT("The scene's been quiet lately. I'll let you know if I hear anything.")));
	}

	return Tips[FMath::RandRange(0, Tips.Num() - 1)];
}

// ==================== Utility ====================

FText UMGMechanicSubsystem::GetServiceDisplayName(EMGMechanicService ServiceType)
{
	switch (ServiceType)
	{
	case EMGMechanicService::Install:    return FText::FromString(TEXT("Install"));
	case EMGMechanicService::Remove:     return FText::FromString(TEXT("Remove"));
	case EMGMechanicService::Tune:       return FText::FromString(TEXT("Tune"));
	case EMGMechanicService::Repair:     return FText::FromString(TEXT("Repair"));
	case EMGMechanicService::Restore:    return FText::FromString(TEXT("Restore"));
	case EMGMechanicService::Custom:     return FText::FromString(TEXT("Custom Fabrication"));
	case EMGMechanicService::Rush:       return FText::FromString(TEXT("Rush Job"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UMGMechanicSubsystem::GetSkillTierDisplayName(EMGMechanicSkillTier SkillTier)
{
	switch (SkillTier)
	{
	case EMGMechanicSkillTier::Apprentice:  return FText::FromString(TEXT("Apprentice"));
	case EMGMechanicSkillTier::Journeyman:  return FText::FromString(TEXT("Journeyman"));
	case EMGMechanicSkillTier::Expert:      return FText::FromString(TEXT("Expert"));
	case EMGMechanicSkillTier::Master:      return FText::FromString(TEXT("Master"));
	case EMGMechanicSkillTier::Legend:      return FText::FromString(TEXT("Legend"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

void UMGMechanicSubsystem::TickJobs(float DeltaGameHours)
{
	TArray<FGuid> CompletedJobIDs;

	for (auto& Pair : ActiveJobs)
	{
		if (!Pair.Value.bIsComplete && FDateTime::Now() >= Pair.Value.EstimatedCompletion)
		{
			CompletedJobIDs.Add(Pair.Key);
		}
	}

	for (const FGuid& JobID : CompletedJobIDs)
	{
		CompleteJob(JobID);
	}
}

// ==================== Protected Helpers ====================

void UMGMechanicSubsystem::UpdateTrustMilestones(FName MechanicID, int32 OldTrust, int32 NewTrust)
{
	TArray<int32> Milestones = { 10, 25, 50, 75, 100 };

	for (int32 Milestone : Milestones)
	{
		if (OldTrust < Milestone && NewTrust >= Milestone)
		{
			FText MilestoneText;
			switch (Milestone)
			{
			case 10:  MilestoneText = FText::FromString(TEXT("Acquaintance - They remember your name")); break;
			case 25:  MilestoneText = FText::FromString(TEXT("Regular - You get priority service")); break;
			case 50:  MilestoneText = FText::FromString(TEXT("Trusted - They share trade secrets")); break;
			case 75:  MilestoneText = FText::FromString(TEXT("Friend - You're part of the family")); break;
			case 100: MilestoneText = FText::FromString(TEXT("Legend - They'd do anything for you")); break;
			}

			OnMechanicRelationshipMilestone.Broadcast(MechanicID, MilestoneText);
		}
	}
}

void UMGMechanicSubsystem::UpdateLoyaltyDiscount(FName MechanicID)
{
	if (FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID))
	{
		// Max 15% discount at 100 trust
		Relationship->LoyaltyDiscount = Relationship->TrustLevel * 0.0015f;
	}
}

void UMGMechanicSubsystem::UnlockTrustServices(FName MechanicID, int32 TrustLevel)
{
	FMGMechanic Mechanic;
	if (!GetMechanic(MechanicID, Mechanic))
	{
		return;
	}

	FMGMechanicRelationship* Relationship = Relationships.Find(MechanicID);
	if (!Relationship)
	{
		return;
	}

	for (const FName& Ability : Mechanic.SpecialAbilities)
	{
		if (Relationship->UnlockedServices.Contains(Ability))
		{
			continue;
		}

		if (const int32* RequiredTrust = AbilityTrustRequirements.Find(Ability))
		{
			if (TrustLevel >= *RequiredTrust)
			{
				Relationship->UnlockedServices.Add(Ability);
				OnMechanicServiceUnlocked.Broadcast(MechanicID, Ability);
			}
		}
	}
}

int32 UMGMechanicSubsystem::CalculateJobCost(const FMGMechanic& Mechanic, FName PartID, EMGMechanicService ServiceType, bool bRushJob) const
{
	int32 BaseCost = GetPartBaseInstallCost(PartID);

	// Service type multipliers
	switch (ServiceType)
	{
	case EMGMechanicService::Install:  break; // 1.0x
	case EMGMechanicService::Remove:   BaseCost = BaseCost / 2; break;
	case EMGMechanicService::Tune:     BaseCost = BaseCost * 2; break;
	case EMGMechanicService::Repair:   BaseCost = BaseCost * 3 / 2; break;
	case EMGMechanicService::Restore:  BaseCost = BaseCost * 3; break;
	case EMGMechanicService::Custom:   BaseCost = BaseCost * 5; break;
	case EMGMechanicService::Rush:     BaseCost = BaseCost * 2; break;
	}

	// Apply mechanic's cost multiplier
	float FinalCost = BaseCost * Mechanic.CostMultiplier;

	// Rush job premium
	if (bRushJob)
	{
		FinalCost *= 1.5f;
	}

	return FMath::RoundToInt(FinalCost);
}

float UMGMechanicSubsystem::CalculateJobDuration(const FMGMechanic& Mechanic, FName PartID, EMGMechanicService ServiceType, bool bRushJob) const
{
	float BaseHours = GetPartBaseInstallTime(PartID);

	// Service type multipliers
	switch (ServiceType)
	{
	case EMGMechanicService::Install:  break; // 1.0x
	case EMGMechanicService::Remove:   BaseHours *= 0.5f; break;
	case EMGMechanicService::Tune:     BaseHours *= 1.5f; break;
	case EMGMechanicService::Repair:   BaseHours *= 2.0f; break;
	case EMGMechanicService::Restore:  BaseHours *= 4.0f; break;
	case EMGMechanicService::Custom:   BaseHours *= 8.0f; break;
	case EMGMechanicService::Rush:     break; // Rush doesn't change base time, just speed
	}

	// Apply mechanic's speed multiplier (higher = faster)
	float FinalHours = BaseHours / Mechanic.SpeedMultiplier;

	// Rush job cuts time in half
	if (bRushJob)
	{
		FinalHours *= 0.5f;
	}

	return FMath::Max(0.5f, FinalHours);
}

int32 UMGMechanicSubsystem::CalculateQualityModifier(const FMGMechanic& Mechanic, EMGWorkResult Result) const
{
	int32 BaseModifier = 0;

	switch (Result)
	{
	case EMGWorkResult::Perfect:    BaseModifier = 10; break;
	case EMGWorkResult::Good:       BaseModifier = 0; break;
	case EMGWorkResult::Acceptable: BaseModifier = -5; break;
	case EMGWorkResult::Botched:    BaseModifier = -15; break;
	case EMGWorkResult::Failed:     BaseModifier = -25; break;
	}

	// Skill tier bonus
	switch (Mechanic.SkillTier)
	{
	case EMGMechanicSkillTier::Legend: BaseModifier += 5; break;
	case EMGMechanicSkillTier::Master: BaseModifier += 3; break;
	case EMGMechanicSkillTier::Expert: BaseModifier += 1; break;
	default: break;
	}

	return FMath::Clamp(BaseModifier, -20, 20);
}

bool UMGMechanicSubsystem::IsMechanicAvailable(const FMGMechanic& Mechanic) const
{
	// Check minimum trust requirement
	const int32 TotalTrust = GetTrustLevel(Mechanic.MechanicID);

	// Some mechanics require you to have some reputation first
	if (Mechanic.MinimumTrustRequired > 0)
	{
		// For locked mechanics, you need trust with OTHER mechanics to unlock them
		// This represents word-of-mouth reputation
		int32 TotalNetworkTrust = 0;
		for (const auto& Pair : Relationships)
		{
			if (Pair.Key != Mechanic.MechanicID)
			{
				TotalNetworkTrust += Pair.Value.TrustLevel;
			}
		}

		return TotalNetworkTrust >= Mechanic.MinimumTrustRequired;
	}

	return true;
}

int32 UMGMechanicSubsystem::GetPartBaseInstallTime(FName PartID) const
{
	// Look up from parts catalog
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGPartsCatalogSubsystem* PartsCatalog = GI->GetSubsystem<UMGPartsCatalogSubsystem>())
		{
			float InstallTimeMinutes = PartsCatalog->GetPartInstallTime(PartID);
			if (InstallTimeMinutes > 0)
			{
				// Convert minutes to hours (rounding up, minimum 1 hour)
				return FMath::Max(1, FMath::CeilToInt(InstallTimeMinutes / 60.0f));
			}
		}
	}

	// Fallback: estimate based on part type naming conventions
	FString PartString = PartID.ToString();

	if (PartString.Contains(TEXT("Engine")) || PartString.Contains(TEXT("Motor")))
	{
		return 8; // 8 hours for engine work
	}
	else if (PartString.Contains(TEXT("Turbo")) || PartString.Contains(TEXT("Supercharger")))
	{
		return 6; // 6 hours for forced induction
	}
	else if (PartString.Contains(TEXT("Transmission")) || PartString.Contains(TEXT("Gearbox")))
	{
		return 5; // 5 hours for trans work
	}
	else if (PartString.Contains(TEXT("Suspension")) || PartString.Contains(TEXT("Coilover")))
	{
		return 4; // 4 hours for suspension
	}
	else if (PartString.Contains(TEXT("Exhaust")))
	{
		return 2; // 2 hours for exhaust
	}
	else if (PartString.Contains(TEXT("Intake")) || PartString.Contains(TEXT("Filter")))
	{
		return 1; // 1 hour for intake
	}
	else if (PartString.Contains(TEXT("ECU")) || PartString.Contains(TEXT("Tune")))
	{
		return 3; // 3 hours for ECU work
	}
	else if (PartString.Contains(TEXT("Brake")))
	{
		return 2; // 2 hours for brakes
	}
	else if (PartString.Contains(TEXT("Wheel")) || PartString.Contains(TEXT("Tire")))
	{
		return 1; // 1 hour for wheels/tires
	}

	return 2; // Default 2 hours
}

int32 UMGMechanicSubsystem::GetPartBaseInstallCost(FName PartID) const
{
	// Look up from parts catalog
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGPartsCatalogSubsystem* PartsCatalog = GI->GetSubsystem<UMGPartsCatalogSubsystem>())
		{
			FMGPartPricingInfo PricingInfo = PartsCatalog->GetPartPricing(PartID);

			if (PricingInfo.bIsValid && PricingInfo.LaborCost > 0)
			{
				// Return the labor cost from catalog
				return PricingInfo.LaborCost;
			}
		}
	}

	// Fallback: estimate based on install time
	const int32 Hours = GetPartBaseInstallTime(PartID);
	const int32 HourlyRate = 75; // $75/hour base labor

	return Hours * HourlyRate;
}
