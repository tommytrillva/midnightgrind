// Copyright Midnight Grind. All Rights Reserved.

#include "Tuning/MGPartInstallation.h"
#include "Economy/MGEconomySubsystem.h"
#include "Kismet/GameplayStatics.h"

// ============================================================================
// Subsystem Lifecycle
// ============================================================================

void UMGPartInstallationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default shop config
	ShopConfig.LaborRatePerHour = 75;
	ShopConfig.MinimumLaborCharge = 50;
	ShopConfig.DynoTuningCost = 250;
	ShopConfig.AlignmentCost = 100;
	ShopConfig.RushJobMultiplier = 1.5f;

	// Initialize mechanic skill at level 1
	MechanicSkill.SkillLevel = 1;
	MechanicSkill.CurrentXP = 0;

	LoadProgressionData();
}

void UMGPartInstallationSubsystem::Deinitialize()
{
	SaveProgressionData();
	Super::Deinitialize();
}

bool UMGPartInstallationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Installation
// ============================================================================

bool UMGPartInstallationSubsystem::InstallPart(FGuid VehicleID, FName PartID, EMGInstallMethod Method, FMGInstallationResult& OutResult)
{
	// Get installation requirements
	FMGInstallationRequirements Requirements;
	if (!GetInstallationRequirements(PartID, Requirements))
	{
		// Part not found - use default moderate difficulty
		Requirements.Difficulty = EMGInstallDifficulty::Moderate;
		Requirements.InstallTimeMinutes = 60;
	}

	// Validate DIY attempt
	if (Method == EMGInstallMethod::DIY)
	{
		FText Reason;
		if (!CanInstallDIY(PartID, Reason))
		{
			OutResult.Result = EMGInstallResult::CannotInstall;
			OutResult.ResultMessage = Reason;
			return false;
		}
	}

	// Calculate installation time
	int32 BaseTime = Requirements.InstallTimeMinutes;
	int32 ActualTime = BaseTime;

	if (Method == EMGInstallMethod::DIY)
	{
		// DIY takes longer for less skilled mechanics
		ActualTime = static_cast<int32>(BaseTime * MechanicSkill.GetTimeMultiplier());
		// Minimum 80% of base time even at max skill
		ActualTime = FMath::Max(ActualTime, static_cast<int32>(BaseTime * 0.8f));
	}

	// Broadcast installation started
	OnInstallationStarted.Broadcast(PartID, VehicleID, Method, ActualTime);

	// Perform installation based on method
	if (Method == EMGInstallMethod::Shop)
	{
		// Shop installation - always successful, costs money
		const int64 LaborCost = ShopConfig.CalculateLaborCost(Requirements);

		// Check if player can afford
		if (UMGEconomySubsystem* EconomySubsystem = GetGameInstance()->GetSubsystem<UMGEconomySubsystem>())
		{
			if (!EconomySubsystem->CanAfford(LaborCost))
			{
				OutResult.Result = EMGInstallResult::CannotInstall;
				OutResult.ResultMessage = NSLOCTEXT("MGInstall", "CannotAfford", "Cannot afford shop labor cost.");
				return false;
			}

			// Deduct labor cost
			EconomySubsystem->SpendCredits(
				LaborCost,
				EMGTransactionType::PartInstallLabor,
				FText::Format(NSLOCTEXT("MGInstall", "LaborCostDesc", "Labor: {0} installation"), FText::FromName(PartID)),
				PartID
			);
		}

		OutResult = FMGInstallationResult::Success(ActualTime, LaborCost, 0);

		// Create installed part instance
		FMGInstalledPartInstance NewInstance;
		NewInstance.PartID = PartID;
		NewInstance.InstallMethod = EMGInstallMethod::Shop;
		NewInstance.InstallResult = EMGInstallResult::Success;
		NewInstance.EffectivenessMultiplier = 1.0f;
		NewInstance.WearRateMultiplier = 1.0f;
		NewInstance.InstalledBy = TEXT("Professional Shop");
		NewInstance.LaborCostPaid = LaborCost;

		// Store the installed part
		TMap<FName, FMGInstalledPartInstance>& VehicleParts = InstalledPartsMap.FindOrAdd(VehicleID);
		VehicleParts.Add(PartID, NewInstance);
	}
	else // DIY Installation
	{
		float Effectiveness = 1.0f;
		float WearRate = 1.0f;
		const EMGInstallResult DIYResult = RollDIYInstallation(Requirements, Effectiveness, WearRate);

		const int32 XPGained = CalculateInstallXP(Requirements.Difficulty, DIYResult);

		switch (DIYResult)
		{
		case EMGInstallResult::Success:
			OutResult = FMGInstallationResult::Success(ActualTime, 0, XPGained);
			MechanicSkill.SuccessfulInstallations++;
			break;

		case EMGInstallResult::Botched:
			OutResult = FMGInstallationResult::Botched(ActualTime, Effectiveness, WearRate, XPGained);
			MechanicSkill.BotchedInstallations++;
			break;

		case EMGInstallResult::Failed:
			{
				// 30% chance of part damage on failed install
				const bool bDamaged = FMath::FRand() < 0.30f;
				const float Damage = bDamaged ? FMath::FRandRange(0.10f, 0.40f) : 0.0f;
				OutResult = FMGInstallationResult::Failed(bDamaged, Damage);
				MechanicSkill.FailedInstallations++;
			}
			break;

		default:
			OutResult.Result = EMGInstallResult::CannotInstall;
			return false;
		}

		MechanicSkill.TotalInstallations++;

		// Only create installed part instance if installation completed
		if (OutResult.bInstallationComplete)
		{
			FMGInstalledPartInstance NewInstance;
			NewInstance.PartID = PartID;
			NewInstance.InstallMethod = EMGInstallMethod::DIY;
			NewInstance.InstallResult = DIYResult;
			NewInstance.EffectivenessMultiplier = Effectiveness;
			NewInstance.WearRateMultiplier = WearRate;
			NewInstance.InstalledBy = TEXT("DIY");
			NewInstance.LaborCostPaid = 0;

			TMap<FName, FMGInstalledPartInstance>& VehicleParts = InstalledPartsMap.FindOrAdd(VehicleID);
			VehicleParts.Add(PartID, NewInstance);
		}

		// Award XP
		if (XPGained > 0)
		{
			AddMechanicXP(XPGained);
		}
	}

	// Broadcast completion
	OnPartInstallationComplete.Broadcast(PartID, VehicleID, OutResult);

	SaveProgressionData();
	return OutResult.bInstallationComplete;
}

void UMGPartInstallationSubsystem::PreviewInstallation(FName PartID, EMGInstallMethod Method, int64& OutLaborCost, int32& OutTimeMinutes, float& OutSuccessRate) const
{
	FMGInstallationRequirements Requirements;
	if (!GetInstallationRequirements(PartID, Requirements))
	{
		// Default to moderate if not found
		Requirements.Difficulty = EMGInstallDifficulty::Moderate;
		Requirements.InstallTimeMinutes = 60;
	}

	OutTimeMinutes = Requirements.InstallTimeMinutes;

	if (Method == EMGInstallMethod::Shop)
	{
		OutLaborCost = ShopConfig.CalculateLaborCost(Requirements);
		OutSuccessRate = 1.0f; // Shop is always successful
	}
	else
	{
		OutLaborCost = 0;
		OutTimeMinutes = static_cast<int32>(Requirements.InstallTimeMinutes * MechanicSkill.GetTimeMultiplier());
		OutSuccessRate = MechanicSkill.GetSuccessRate(Requirements.Difficulty);
	}
}

bool UMGPartInstallationSubsystem::CanInstallDIY(FName PartID, FText& OutReason) const
{
	FMGInstallationRequirements Requirements;
	if (!GetInstallationRequirements(PartID, Requirements))
	{
		// If no requirements registered, allow DIY with moderate difficulty assumptions
		return true;
	}

	// Check if shop-only
	if (Requirements.Difficulty == EMGInstallDifficulty::ShopOnly)
	{
		OutReason = NSLOCTEXT("MGInstall", "ShopOnlyReason", "This part requires professional installation and cannot be done DIY.");
		return false;
	}

	// Check skill requirement
	if (!MechanicSkill.CanAttemptDifficulty(Requirements.Difficulty))
	{
		const int32 RequiredSkill = FMGMechanicSkill::GetMinSkillForDifficulty(Requirements.Difficulty);
		OutReason = FText::Format(
			NSLOCTEXT("MGInstall", "SkillTooLowReason", "Requires Mechanic Skill Level {0}. Your current level: {1}"),
			FText::AsNumber(RequiredSkill),
			FText::AsNumber(MechanicSkill.SkillLevel)
		);
		return false;
	}

	// Check lift requirement
	if (Requirements.bRequiresLift && !bHasLiftAccess)
	{
		OutReason = NSLOCTEXT("MGInstall", "RequiresLiftReason", "This installation requires a vehicle lift. Use shop installation or upgrade your garage.");
		return false;
	}

	// Check special tools requirement
	if (Requirements.bRequiresSpecialTools)
	{
		for (const FName& ToolID : Requirements.RequiredToolIDs)
		{
			if (!OwnedTools.Contains(ToolID))
			{
				OutReason = FText::Format(
					NSLOCTEXT("MGInstall", "MissingToolReason", "Missing required tool: {0}. Purchase it or use shop installation."),
					FText::FromName(ToolID)
				);
				return false;
			}
		}
	}

	return true;
}

bool UMGPartInstallationSubsystem::GetInstallationRequirements(FName PartID, FMGInstallationRequirements& OutRequirements) const
{
	if (const FMGInstallationRequirements* Found = PartRequirementsDatabase.Find(PartID))
	{
		OutRequirements = *Found;
		return true;
	}

	return false;
}

// ============================================================================
// Mechanic Skill
// ============================================================================

float UMGPartInstallationSubsystem::GetDIYSuccessRate(EMGInstallDifficulty Difficulty) const
{
	return MechanicSkill.GetSuccessRate(Difficulty);
}

bool UMGPartInstallationSubsystem::MeetsSkillRequirement(EMGInstallDifficulty Difficulty) const
{
	return MechanicSkill.CanAttemptDifficulty(Difficulty);
}

void UMGPartInstallationSubsystem::AddMechanicXP(int32 XPAmount)
{
	if (XPAmount <= 0 || MechanicSkill.SkillLevel >= 100)
	{
		return;
	}

	MechanicSkill.CurrentXP += XPAmount;
	CheckSkillLevelUp();
}

void UMGPartInstallationSubsystem::CheckSkillLevelUp()
{
	while (MechanicSkill.SkillLevel < 100)
	{
		const int32 XPRequired = MechanicSkill.GetXPForNextLevel();
		if (MechanicSkill.CurrentXP >= XPRequired)
		{
			MechanicSkill.CurrentXP -= XPRequired;
			const int32 OldLevel = MechanicSkill.SkillLevel;
			MechanicSkill.SkillLevel++;

			OnMechanicSkillLevelUp.Broadcast(MechanicSkill.SkillLevel, OldLevel);
		}
		else
		{
			break;
		}
	}
}

// ============================================================================
// Shop Configuration
// ============================================================================

int64 UMGPartInstallationSubsystem::CalculateLaborCost(FName PartID) const
{
	FMGInstallationRequirements Requirements;
	if (GetInstallationRequirements(PartID, Requirements))
	{
		return ShopConfig.CalculateLaborCost(Requirements);
	}

	// Default cost for unknown parts (1 hour moderate difficulty)
	return ShopConfig.LaborRatePerHour;
}

// ============================================================================
// Installed Parts Query
// ============================================================================

bool UMGPartInstallationSubsystem::GetInstalledPartInstance(FGuid VehicleID, FName SlotID, FMGInstalledPartInstance& OutInstance) const
{
	if (const TMap<FName, FMGInstalledPartInstance>* VehicleParts = InstalledPartsMap.Find(VehicleID))
	{
		if (const FMGInstalledPartInstance* Instance = VehicleParts->Find(SlotID))
		{
			OutInstance = *Instance;
			return true;
		}
	}

	return false;
}

bool UMGPartInstallationSubsystem::IsPartBotched(FGuid VehicleID, FName SlotID) const
{
	FMGInstalledPartInstance Instance;
	if (GetInstalledPartInstance(VehicleID, SlotID, Instance))
	{
		return Instance.IsBotched();
	}

	return false;
}

float UMGPartInstallationSubsystem::GetPartEffectiveness(FGuid VehicleID, FName SlotID) const
{
	FMGInstalledPartInstance Instance;
	if (GetInstalledPartInstance(VehicleID, SlotID, Instance))
	{
		return Instance.GetCurrentEffectiveness();
	}

	return 1.0f; // Default to full effectiveness if not found
}

// ============================================================================
// Player Facilities
// ============================================================================

bool UMGPartInstallationSubsystem::OwnsTool(FName ToolID) const
{
	return OwnedTools.Contains(ToolID);
}

void UMGPartInstallationSubsystem::GrantTool(FName ToolID)
{
	OwnedTools.Add(ToolID);
	SaveProgressionData();
}

// ============================================================================
// Part Registration
// ============================================================================

void UMGPartInstallationSubsystem::RegisterPartRequirements(FName PartID, const FMGInstallationRequirements& Requirements)
{
	PartRequirementsDatabase.Add(PartID, Requirements);
}

// ============================================================================
// Protected Helpers
// ============================================================================

EMGInstallResult UMGPartInstallationSubsystem::RollDIYInstallation(const FMGInstallationRequirements& Requirements, float& OutEffectiveness, float& OutWearRate)
{
	const float SuccessRate = MechanicSkill.GetSuccessRate(Requirements.Difficulty);
	const float Roll = FMath::FRand();

	OutEffectiveness = 1.0f;
	OutWearRate = 1.0f;

	if (Roll <= SuccessRate)
	{
		// Perfect success
		return EMGInstallResult::Success;
	}
	else if (Roll <= SuccessRate + 0.20f) // 20% window for botched install
	{
		// Botched install - part works but not optimally
		// Effectiveness: 80-95% based on how badly it was botched
		const float BotchSeverity = (Roll - SuccessRate) / 0.20f; // 0.0 = barely botched, 1.0 = badly botched
		OutEffectiveness = FMath::Lerp(0.95f, 0.80f, BotchSeverity);

		// Wear rate: 10-50% faster based on severity
		OutWearRate = FMath::Lerp(1.10f, 1.50f, BotchSeverity);

		return EMGInstallResult::Botched;
	}
	else
	{
		// Complete failure
		return EMGInstallResult::Failed;
	}
}

int32 UMGPartInstallationSubsystem::CalculateInstallXP(EMGInstallDifficulty Difficulty, EMGInstallResult Result) const
{
	// Base XP by difficulty
	int32 BaseXP = 0;
	switch (Difficulty)
	{
	case EMGInstallDifficulty::Simple:
		BaseXP = 10;
		break;
	case EMGInstallDifficulty::Moderate:
		BaseXP = 25;
		break;
	case EMGInstallDifficulty::Complex:
		BaseXP = 75;
		break;
	case EMGInstallDifficulty::Expert:
		BaseXP = 150;
		break;
	case EMGInstallDifficulty::ShopOnly:
		return 0; // No XP for shop-only parts
	}

	// Result multiplier
	float Multiplier = 1.0f;
	switch (Result)
	{
	case EMGInstallResult::Success:
		Multiplier = 1.5f; // Bonus for perfect install
		break;
	case EMGInstallResult::Botched:
		Multiplier = 1.0f; // Standard XP
		break;
	case EMGInstallResult::Failed:
		Multiplier = 0.25f; // Reduced XP for failure
		break;
	default:
		return 0;
	}

	return static_cast<int32>(BaseXP * Multiplier);
}

void UMGPartInstallationSubsystem::SaveProgressionData()
{
	// Implementation would use USaveGame or cloud save
	// Saves: MechanicSkill, InstalledPartsMap, bHasLiftAccess, OwnedTools
}

void UMGPartInstallationSubsystem::LoadProgressionData()
{
	// Implementation would use USaveGame or cloud save
	// Loads: MechanicSkill, InstalledPartsMap, bHasLiftAccess, OwnedTools
}
