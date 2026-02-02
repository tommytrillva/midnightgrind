// Copyright Midnight Grind. All Rights Reserved.

#include "Progression/MGExtendedProgressionSubsystem.h"
#include "Progression/MGPlayerProgression.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGProgression, Log, All);

void UMGExtendedProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize with starting housing
	HousingData.CurrentTier = EMGHousingTier::Garage;
	HousingData.MaxDisplayCapacity = 1;
	LastUnlockTime = FDateTime::Now();

	UE_LOG(LogMGProgression, Log, TEXT("Extended Progression System initialized"));
}

void UMGExtendedProgressionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// =============================================================================
// MONEY SYSTEM
// =============================================================================

void UMGExtendedProgressionSubsystem::AddMoney(int64 Amount, bool bNotify)
{
	int64 OldMoney = PlayerMoney;
	PlayerMoney += Amount;
	PlayerMoney = FMath::Max<int64>(0, PlayerMoney); // Never go negative

	if (bNotify && Amount != 0)
	{
		OnMoneyChanged.Broadcast(PlayerMoney, Amount);
	}

	UE_LOG(LogMGProgression, Verbose, TEXT("Money changed: %lld -> %lld (Delta: %lld)"), 
		OldMoney, PlayerMoney, Amount);
}

bool UMGExtendedProgressionSubsystem::SpendMoney(int64 Amount)
{
	if (!CanAfford(Amount))
	{
		UE_LOG(LogMGProgression, Warning, TEXT("Cannot afford %lld (Have: %lld)"), Amount, PlayerMoney);
		return false;
	}

	AddMoney(-Amount, true);
	return true;
}

// =============================================================================
// CAR PROGRESSION
// =============================================================================

bool UMGExtendedProgressionSubsystem::OwnsCar(FName CarID) const
{
	return OwnedCars.ContainsByPredicate([CarID](const FMGOwnedCar& Car)
	{
		return Car.CarID == CarID;
	});
}

bool UMGExtendedProgressionSubsystem::OwnsCarInTier(EMGCarTier Tier) const
{
	return OwnedCars.ContainsByPredicate([Tier](const FMGOwnedCar& Car)
	{
		return Car.Tier == Tier;
	});
}

TArray<FMGOwnedCar> UMGExtendedProgressionSubsystem::GetOwnedCarsByTier(EMGCarTier Tier) const
{
	TArray<FMGOwnedCar> Result;
	for (const FMGOwnedCar& Car : OwnedCars)
	{
		if (Car.Tier == Tier)
		{
			Result.Add(Car);
		}
	}
	return Result;
}

bool UMGExtendedProgressionSubsystem::GetOwnedCarData(FName CarID, FMGOwnedCar& OutCarData) const
{
	const FMGOwnedCar* Found = OwnedCars.FindByPredicate([CarID](const FMGOwnedCar& Car)
	{
		return Car.CarID == CarID;
	});

	if (Found)
	{
		OutCarData = *Found;
		return true;
	}
	return false;
}

bool UMGExtendedProgressionSubsystem::CanUnlockCar(FName CarID, FText& OutReason) const
{
	// Check if car exists in database
	const FMGCarUnlockData* CarData = CarUnlockDatabase.Find(CarID);
	if (!CarData)
	{
		OutReason = FText::FromString(TEXT("Car not found"));
		return false;
	}

	// Check if already owned
	if (OwnsCar(CarID))
	{
		OutReason = FText::FromString(TEXT("Already owned"));
		return false;
	}

	// Check unlock requirements
	if (!CheckUnlockRequirements(CarData->Requirements, OutReason))
	{
		return false;
	}

	// Check if player can afford it
	if (CarData->PurchaseCost > 0 && !CanAfford(CarData->PurchaseCost))
	{
		OutReason = FText::Format(
			FText::FromString(TEXT("Need ${0} (Have: ${1})")),
			FText::AsNumber(CarData->PurchaseCost),
			FText::AsNumber(PlayerMoney)
		);
		return false;
	}

	return true;
}

bool UMGExtendedProgressionSubsystem::UnlockCar(FName CarID)
{
	FText Reason;
	if (!CanUnlockCar(CarID, Reason))
	{
		UE_LOG(LogMGProgression, Warning, TEXT("Cannot unlock car %s: %s"), 
			*CarID.ToString(), *Reason.ToString());
		return false;
	}

	const FMGCarUnlockData* CarData = CarUnlockDatabase.Find(CarID);
	check(CarData); // Should be valid if CanUnlockCar passed

	// Deduct cost if applicable
	if (CarData->PurchaseCost > 0)
	{
		SpendMoney(CarData->PurchaseCost);
	}

	// Add to owned cars
	FMGOwnedCar NewCar;
	NewCar.CarID = CarID;
	NewCar.Tier = CarData->Tier;
	NewCar.AcquiredAt = FDateTime::Now();
	OwnedCars.Add(NewCar);

	// Update last unlock time (for dead zone tracking)
	LastUnlockTime = FDateTime::Now();

	// Broadcast event
	OnCarAcquired.Broadcast(CarID, CarData->Tier);

	// Check for newly available unlocks
	CheckAndGrantMilestones();

	UE_LOG(LogMGProgression, Log, TEXT("Unlocked car: %s (Tier: %d)"), 
		*CarID.ToString(), static_cast<int32>(CarData->Tier));

	return true;
}

void UMGExtendedProgressionSubsystem::AddCarUsage(FName CarID, float DistanceKm, bool bRaceWon)
{
	FMGOwnedCar* Car = FindOwnedCar(CarID);
	if (!Car)
	{
		UE_LOG(LogMGProgression, Warning, TEXT("Cannot add usage for unowned car: %s"), *CarID.ToString());
		return;
	}

	Car->DistanceDrivenKm += DistanceKm;
	if (bRaceWon)
	{
		Car->RacesWon++;
	}

	// Check for performance upgrade unlock
	int32 CurrentStage, NextStage;
	if (CanUpgradeCarPerformance(CarID, CurrentStage, NextStage))
	{
		UE_LOG(LogMGProgression, Log, TEXT("Car %s can now upgrade to stage %d"), 
			*CarID.ToString(), NextStage);
		// Could auto-upgrade or notify player
	}
}

bool UMGExtendedProgressionSubsystem::CanUpgradeCarPerformance(FName CarID, int32& OutCurrentStage, int32& OutNextStage) const
{
	const FMGOwnedCar* Car = OwnedCars.FindByPredicate([CarID](const FMGOwnedCar& C) { return C.CarID == CarID; });
	if (!Car)
	{
		return false;
	}

	OutCurrentStage = Car->PerformanceStage;
	OutNextStage = Car->PerformanceStage + 1;

	// Max stage is 5
	if (Car->PerformanceStage >= 5)
	{
		return false;
	}

	// Check usage requirements (from design doc):
	// Stage 2: 50km driven
	// Stage 3: 10 races won
	// Stage 4: Rep milestone (check with base progression)
	// Stage 5: Car-specific achievement
	switch (OutNextStage)
	{
		case 2:
			return Car->DistanceDrivenKm >= 50.0f;
		case 3:
			return Car->RacesWon >= 10;
		case 4:
		{
			UMGPlayerProgression* BaseProgression = GetBaseProgression();
			return BaseProgression && BaseProgression->GetTotalReputation() >= 10000;
		}
		case 5:
			// For now, require high usage and rep
			return Car->DistanceDrivenKm >= 500.0f && Car->RacesWon >= 50;
		default:
			return false;
	}
}

bool UMGExtendedProgressionSubsystem::UpgradeCarPerformance(FName CarID)
{
	FMGOwnedCar* Car = FindOwnedCar(CarID);
	if (!Car)
	{
		return false;
	}

	int32 CurrentStage, NextStage;
	if (!CanUpgradeCarPerformance(CarID, CurrentStage, NextStage))
	{
		return false;
	}

	Car->PerformanceStage = NextStage;

	UE_LOG(LogMGProgression, Log, TEXT("Upgraded car %s to performance stage %d"), 
		*CarID.ToString(), NextStage);

	return true;
}

EMGCarTier UMGExtendedProgressionSubsystem::GetHighestOwnedTier() const
{
	EMGCarTier Highest = EMGCarTier::None;
	for (const FMGOwnedCar& Car : OwnedCars)
	{
		if (static_cast<uint8>(Car.Tier) > static_cast<uint8>(Highest))
		{
			Highest = Car.Tier;
		}
	}
	return Highest;
}

FMGOwnedCar* UMGExtendedProgressionSubsystem::FindOwnedCar(FName CarID)
{
	return OwnedCars.FindByPredicate([CarID](const FMGOwnedCar& Car)
	{
		return Car.CarID == CarID;
	});
}

// =============================================================================
// LOCATION PROGRESSION
// =============================================================================

bool UMGExtendedProgressionSubsystem::IsLocationUnlocked(FName LocationID) const
{
	return UnlockedLocations.ContainsByPredicate([LocationID](const FMGUnlockedLocation& Loc)
	{
		return Loc.LocationID == LocationID;
	});
}

bool UMGExtendedProgressionSubsystem::IsDistrictUnlocked(EMGDistrict District) const
{
	return UnlockedLocations.ContainsByPredicate([District](const FMGUnlockedLocation& Loc)
	{
		return Loc.District == District;
	});
}

TArray<FMGUnlockedLocation> UMGExtendedProgressionSubsystem::GetUnlockedLocationsInDistrict(EMGDistrict District) const
{
	TArray<FMGUnlockedLocation> Result;
	for (const FMGUnlockedLocation& Loc : UnlockedLocations)
	{
		if (Loc.District == District)
		{
			Result.Add(Loc);
		}
	}
	return Result;
}

bool UMGExtendedProgressionSubsystem::CanUnlockLocation(FName LocationID, FText& OutReason) const
{
	// Check if location exists
	const FMGLocationUnlockData* LocData = LocationUnlockDatabase.Find(LocationID);
	if (!LocData)
	{
		OutReason = FText::FromString(TEXT("Location not found"));
		return false;
	}

	// Check if already unlocked
	if (IsLocationUnlocked(LocationID))
	{
		OutReason = FText::FromString(TEXT("Already unlocked"));
		return false;
	}

	// Check requirements
	return CheckUnlockRequirements(LocData->Requirements, OutReason);
}

bool UMGExtendedProgressionSubsystem::UnlockLocation(FName LocationID)
{
	FText Reason;
	if (!CanUnlockLocation(LocationID, Reason))
	{
		UE_LOG(LogMGProgression, Warning, TEXT("Cannot unlock location %s: %s"), 
			*LocationID.ToString(), *Reason.ToString());
		return false;
	}

	const FMGLocationUnlockData* LocData = LocationUnlockDatabase.Find(LocationID);
	check(LocData);

	FMGUnlockedLocation NewLocation;
	NewLocation.LocationID = LocationID;
	NewLocation.District = LocData->District;
	NewLocation.UnlockedAt = FDateTime::Now();
	UnlockedLocations.Add(NewLocation);

	LastUnlockTime = FDateTime::Now();

	OnLocationUnlocked.Broadcast(LocationID, LocData->District);

	CheckAndGrantMilestones();

	UE_LOG(LogMGProgression, Log, TEXT("Unlocked location: %s (District: %d)"), 
		*LocationID.ToString(), static_cast<int32>(LocData->District));

	return true;
}

void UMGExtendedProgressionSubsystem::RecordLocationRaceCompletion(FName LocationID)
{
	FMGUnlockedLocation* Loc = FindUnlockedLocation(LocationID);
	if (Loc)
	{
		Loc->RacesCompleted++;
	}
}

FMGUnlockedLocation* UMGExtendedProgressionSubsystem::FindUnlockedLocation(FName LocationID)
{
	return UnlockedLocations.FindByPredicate([LocationID](const FMGUnlockedLocation& Loc)
	{
		return Loc.LocationID == LocationID;
	});
}

// =============================================================================
// HOUSING PROGRESSION
// =============================================================================

bool UMGExtendedProgressionSubsystem::CanUpgradeHousing(EMGHousingTier TargetTier, FText& OutReason) const
{
	// Check if already at or above target tier
	if (static_cast<uint8>(HousingData.CurrentTier) >= static_cast<uint8>(TargetTier))
	{
		OutReason = FText::FromString(TEXT("Already at or above target tier"));
		return false;
	}

	// Check if upgrading sequentially (can't skip tiers)
	if (static_cast<uint8>(TargetTier) - static_cast<uint8>(HousingData.CurrentTier) > 1)
	{
		OutReason = FText::FromString(TEXT("Must upgrade sequentially"));
		return false;
	}

	// Check housing unlock data
	const FMGHousingUnlockData* HousingUnlock = HousingUnlockDatabase.Find(TargetTier);
	if (!HousingUnlock)
	{
		OutReason = FText::FromString(TEXT("Housing tier not found"));
		return false;
	}

	// Check cost
	if (!CanAfford(HousingUnlock->PurchaseCost))
	{
		OutReason = FText::Format(
			FText::FromString(TEXT("Need ${0} (Have: ${1})")),
			FText::AsNumber(HousingUnlock->PurchaseCost),
			FText::AsNumber(PlayerMoney)
		);
		return false;
	}

	// Check requirements
	return CheckUnlockRequirements(HousingUnlock->Requirements, OutReason);
}

bool UMGExtendedProgressionSubsystem::UpgradeHousing(EMGHousingTier TargetTier)
{
	FText Reason;
	if (!CanUpgradeHousing(TargetTier, Reason))
	{
		UE_LOG(LogMGProgression, Warning, TEXT("Cannot upgrade housing to tier %d: %s"), 
			static_cast<int32>(TargetTier), *Reason.ToString());
		return false;
	}

	const FMGHousingUnlockData* HousingUnlock = HousingUnlockDatabase.Find(TargetTier);
	check(HousingUnlock);

	// Deduct cost
	SpendMoney(HousingUnlock->PurchaseCost);

	EMGHousingTier OldTier = HousingData.CurrentTier;
	HousingData.CurrentTier = TargetTier;
	HousingData.MaxDisplayCapacity = HousingUnlock->MaxCarDisplay;

	LastUnlockTime = FDateTime::Now();

	OnHousingUpgraded.Broadcast(TargetTier, OldTier);

	CheckAndGrantMilestones();

	UE_LOG(LogMGProgression, Log, TEXT("Upgraded housing: %d -> %d"), 
		static_cast<int32>(OldTier), static_cast<int32>(TargetTier));

	return true;
}

bool UMGExtendedProgressionSubsystem::IsHousingCosmeticUnlocked(FName CosmeticID) const
{
	return HousingData.UnlockedCosmetics.Contains(CosmeticID);
}

bool UMGExtendedProgressionSubsystem::UnlockHousingCosmetic(FName CosmeticID)
{
	if (IsHousingCosmeticUnlocked(CosmeticID))
	{
		return false;
	}

	HousingData.UnlockedCosmetics.Add(CosmeticID);
	return true;
}

// =============================================================================
// CUSTOMIZATION PROGRESSION
// =============================================================================

bool UMGExtendedProgressionSubsystem::IsCustomizationUnlocked(FName ItemID, EMGCustomizationType Category) const
{
	const FMGCustomizationProgress* Progress = CustomizationProgress.FindByPredicate(
		[Category](const FMGCustomizationProgress& P) { return P.Category == Category; });

	return Progress && Progress->UnlockedItems.Contains(ItemID);
}

TArray<FName> UMGExtendedProgressionSubsystem::GetUnlockedCustomization(EMGCustomizationType Category) const
{
	const FMGCustomizationProgress* Progress = CustomizationProgress.FindByPredicate(
		[Category](const FMGCustomizationProgress& P) { return P.Category == Category; });

	return Progress ? Progress->UnlockedItems : TArray<FName>();
}

int32 UMGExtendedProgressionSubsystem::GetCustomizationUnlockCount(EMGCustomizationType Category) const
{
	return GetUnlockedCustomization(Category).Num();
}

bool UMGExtendedProgressionSubsystem::CanUnlockCustomization(FName ItemID, FText& OutReason) const
{
	const FMGCustomizationUnlockData* ItemData = CustomizationUnlockDatabase.Find(ItemID);
	if (!ItemData)
	{
		OutReason = FText::FromString(TEXT("Item not found"));
		return false;
	}

	if (IsCustomizationUnlocked(ItemID, ItemData->Category))
	{
		OutReason = FText::FromString(TEXT("Already unlocked"));
		return false;
	}

	if (ItemData->PurchaseCost > 0 && !CanAfford(ItemData->PurchaseCost))
	{
		OutReason = FText::Format(
			FText::FromString(TEXT("Need ${0} (Have: ${1})")),
			FText::AsNumber(ItemData->PurchaseCost),
			FText::AsNumber(PlayerMoney)
		);
		return false;
	}

	return CheckUnlockRequirements(ItemData->Requirements, OutReason);
}

bool UMGExtendedProgressionSubsystem::UnlockCustomization(FName ItemID, EMGCustomizationType Category)
{
	FText Reason;
	if (!CanUnlockCustomization(ItemID, Reason))
	{
		return false;
	}

	const FMGCustomizationUnlockData* ItemData = CustomizationUnlockDatabase.Find(ItemID);
	check(ItemData);

	if (ItemData->PurchaseCost > 0)
	{
		SpendMoney(ItemData->PurchaseCost);
	}

	FMGCustomizationProgress& Progress = GetOrCreateCustomizationProgress(Category);
	Progress.UnlockedItems.AddUnique(ItemID);

	OnCustomizationUnlocked.Broadcast(Category, ItemID);

	UE_LOG(LogMGProgression, Log, TEXT("Unlocked customization: %s (Category: %d)"), 
		*ItemID.ToString(), static_cast<int32>(Category));

	return true;
}

FMGCustomizationProgress& UMGExtendedProgressionSubsystem::GetOrCreateCustomizationProgress(EMGCustomizationType Category)
{
	FMGCustomizationProgress* Found = CustomizationProgress.FindByPredicate(
		[Category](const FMGCustomizationProgress& P) { return P.Category == Category; });

	if (Found)
	{
		return *Found;
	}

	FMGCustomizationProgress NewProgress;
	NewProgress.Category = Category;
	return CustomizationProgress[CustomizationProgress.Add(NewProgress)];
}

// =============================================================================
// MILESTONE SYSTEM
// =============================================================================

bool UMGExtendedProgressionSubsystem::GetNextMilestone(FMGMilestone& OutMilestone) const
{
	// Find lowest uncompleted milestone by target hour
	const FMGMilestoneData* NextData = nullptr;
	int32 LowestHour = INT_MAX;

	for (const auto& Pair : MilestoneDatabase)
	{
		if (!IsMilestoneCompleted(Pair.Key) && Pair.Value.TargetHour < LowestHour)
		{
			NextData = &Pair.Value;
			LowestHour = Pair.Value.TargetHour;
		}
	}

	if (NextData)
	{
		OutMilestone.MilestoneID = NextData->MilestoneID;
		OutMilestone.DisplayName = NextData->DisplayName;
		OutMilestone.Description = NextData->Description;
		OutMilestone.TargetHour = NextData->TargetHour;
		OutMilestone.NotificationTier = NextData->NotificationTier;
		return true;
	}

	return false;
}

bool UMGExtendedProgressionSubsystem::IsMilestoneCompleted(FName MilestoneID) const
{
	return CompletedMilestones.ContainsByPredicate([MilestoneID](const FMGMilestone& M)
	{
		return M.MilestoneID == MilestoneID;
	});
}

TArray<FMGMilestone> UMGExtendedProgressionSubsystem::CheckAndGrantMilestones()
{
	TArray<FMGMilestone> NewlyCompleted;

	for (const auto& Pair : MilestoneDatabase)
	{
		if (IsMilestoneCompleted(Pair.Key))
		{
			continue; // Already completed
		}

		FText Reason;
		if (CheckUnlockRequirements(Pair.Value.Requirements, Reason))
		{
			// Milestone requirements met!
			FMGMilestone NewMilestone;
			NewMilestone.MilestoneID = Pair.Key;
			NewMilestone.DisplayName = Pair.Value.DisplayName;
			NewMilestone.Description = Pair.Value.Description;
			NewMilestone.TargetHour = Pair.Value.TargetHour;
			NewMilestone.NotificationTier = Pair.Value.NotificationTier;
			NewMilestone.bCompleted = true;
			NewMilestone.CompletedAt = FDateTime::Now();

			CompletedMilestones.Add(NewMilestone);
			NewlyCompleted.Add(NewMilestone);

			OnMilestoneCompleted.Broadcast(NewMilestone);

			// Grant rewards
			if (!Pair.Value.RewardType.IsEmpty())
			{
				if (Pair.Value.RewardType == "Money")
				{
					AddMoney(Pair.Value.RewardMoney, true);
				}
				else if (Pair.Value.RewardType == "Car" && Pair.Value.RewardID != NAME_None)
				{
					// Auto-unlock the car as a reward
					UnlockCar(Pair.Value.RewardID);
				}
				// Add more reward types as needed
			}

			UE_LOG(LogMGProgression, Log, TEXT("Milestone completed: %s"), *Pair.Key.ToString());
		}
	}

	return NewlyCompleted;
}

// =============================================================================
// PREREQUISITE SYSTEM
// =============================================================================

bool UMGExtendedProgressionSubsystem::CheckPrerequisite(const FMGPrerequisite& Prereq) const
{
	UMGPlayerProgression* BaseProgression = GetBaseProgression();

	switch (Prereq.Type)
	{
		case EMGPrerequisiteType::Reputation:
			return BaseProgression && BaseProgression->GetTotalReputation() >= Prereq.RequiredValue;

		case EMGPrerequisiteType::Money:
			return PlayerMoney >= Prereq.RequiredValue;

		case EMGPrerequisiteType::Level:
			return BaseProgression && BaseProgression->GetCurrentLevel() >= Prereq.RequiredValue;

		case EMGPrerequisiteType::RaceWins:
			return BaseProgression && BaseProgression->GetRaceStatistics().TotalWins >= Prereq.RequiredValue;

		case EMGPrerequisiteType::CarOwnership:
			return OwnsCar(Prereq.TargetID);

		case EMGPrerequisiteType::CarTierOwnership:
			return OwnsCarInTier(static_cast<EMGCarTier>(Prereq.RequiredEnumValue));

		case EMGPrerequisiteType::LocationUnlocked:
			return IsLocationUnlocked(Prereq.TargetID);

		case EMGPrerequisiteType::DistrictUnlocked:
			return IsDistrictUnlocked(static_cast<EMGDistrict>(Prereq.RequiredEnumValue));

		case EMGPrerequisiteType::ChallengeCompleted:
			// Check with achievements or challenge system (not implemented here)
			return false;

		case EMGPrerequisiteType::HousingTier:
			return static_cast<uint8>(HousingData.CurrentTier) >= static_cast<uint8>(Prereq.RequiredEnumValue);

		case EMGPrerequisiteType::PlayTime:
		{
			if (!BaseProgression)
			{
				return false;
			}
			float PlayTimeHours = BaseProgression->GetRaceStatistics().PlayTimeSeconds / 3600.0f;
			return PlayTimeHours >= Prereq.RequiredValue;
		}

		case EMGPrerequisiteType::CarUsage:
		{
			const FMGOwnedCar* Car = OwnedCars.FindByPredicate([&Prereq](const FMGOwnedCar& C)
			{
				return C.CarID == Prereq.TargetID;
			});
			return Car && Car->DistanceDrivenKm >= Prereq.RequiredValue;
		}

		case EMGPrerequisiteType::CustomizationCount:
		{
			EMGCustomizationType Category = static_cast<EMGCustomizationType>(Prereq.RequiredEnumValue);
			return GetCustomizationUnlockCount(Category) >= Prereq.RequiredValue;
		}

		default:
			return false;
	}
}

bool UMGExtendedProgressionSubsystem::CheckUnlockRequirements(const FMGUnlockRequirement& Requirements, FText& OutReason) const
{
	// Check ALL prerequisites (AND logic)
	for (const FMGPrerequisite& Prereq : Requirements.AllPrerequisites)
	{
		if (!CheckPrerequisite(Prereq))
		{
			OutReason = FText::FromString(TEXT("Requirement not met")); // TODO: Better messages
			return false;
		}
	}

	// Check prerequisite groups (each group is OR, groups together are AND)
	for (const FMGPrerequisiteGroup& Group : Requirements.PrerequisiteGroups)
	{
		bool bGroupSatisfied = false;
		for (const FMGPrerequisite& Prereq : Group.Prerequisites)
		{
			if (CheckPrerequisite(Prereq))
			{
				bGroupSatisfied = true;
				break;
			}
		}

		if (!bGroupSatisfied)
		{
			OutReason = FText::FromString(TEXT("Alternative requirement not met"));
			return false;
		}
	}

	return true;
}

FText UMGExtendedProgressionSubsystem::GetRequirementDescription(const FMGUnlockRequirement& Requirements) const
{
	// TODO: Build human-readable requirement string
	return FText::FromString(TEXT("Requirements not met"));
}

// =============================================================================
// PROGRESSION QUERIES
// =============================================================================

TArray<FMGCarUnlockData> UMGExtendedProgressionSubsystem::GetAvailableCarUnlocks() const
{
	TArray<FMGCarUnlockData> Available;

	for (const auto& Pair : CarUnlockDatabase)
	{
		if (!OwnsCar(Pair.Key))
		{
			FText Reason;
			if (CheckUnlockRequirements(Pair.Value.Requirements, Reason))
			{
				Available.Add(Pair.Value);
			}
		}
	}

	return Available;
}

TArray<FMGLocationUnlockData> UMGExtendedProgressionSubsystem::GetAvailableLocationUnlocks() const
{
	TArray<FMGLocationUnlockData> Available;

	for (const auto& Pair : LocationUnlockDatabase)
	{
		if (!IsLocationUnlocked(Pair.Key))
		{
			FText Reason;
			if (CheckUnlockRequirements(Pair.Value.Requirements, Reason))
			{
				Available.Add(Pair.Value);
			}
		}
	}

	return Available;
}

FString UMGExtendedProgressionSubsystem::GetRecommendedNextUnlock() const
{
	// Simple logic: recommend cheapest available car or next location
	TArray<FMGCarUnlockData> AvailableCars = GetAvailableCarUnlocks();
	if (AvailableCars.Num() > 0)
	{
		// Find cheapest
		AvailableCars.Sort([](const FMGCarUnlockData& A, const FMGCarUnlockData& B)
		{
			return A.PurchaseCost < B.PurchaseCost;
		});
		return FString::Printf(TEXT("Car: %s"), *AvailableCars[0].DisplayName.ToString());
	}

	TArray<FMGLocationUnlockData> AvailableLocations = GetAvailableLocationUnlocks();
	if (AvailableLocations.Num() > 0)
	{
		return FString::Printf(TEXT("Location: %s"), *AvailableLocations[0].DisplayName.ToString());
	}

	return TEXT("Keep racing!");
}

bool UMGExtendedProgressionSubsystem::IsInDeadZone(float& OutHoursSinceLastUnlock) const
{
	FTimespan TimeSince = FDateTime::Now() - LastUnlockTime;
	OutHoursSinceLastUnlock = static_cast<float>(TimeSince.GetTotalHours());

	// Consider it a dead zone if no unlock in 5+ hours
	return OutHoursSinceLastUnlock > 5.0f;
}

// =============================================================================
// ANTI-FRUSTRATION SYSTEMS
// =============================================================================

bool UMGExtendedProgressionSubsystem::ShouldApplyCatchUpBonus(float& OutBonusMultiplier) const
{
	// TODO: Compare actual progression to expected curve
	// For now, simple check: if player has been playing for X hours but has low progression
	UMGPlayerProgression* BaseProgression = GetBaseProgression();
	if (!BaseProgression)
	{
		return false;
	}

	float PlayTimeHours = BaseProgression->GetRaceStatistics().PlayTimeSeconds / 3600.0f;
	int32 ExpectedRep = static_cast<int32>(PlayTimeHours * 1000); // ~1000 rep/hour expected

	if (BaseProgression->GetTotalReputation() < ExpectedRep * 0.5f) // 50% behind curve
	{
		OutBonusMultiplier = 1.5f; // 50% bonus
		return true;
	}

	return false;
}

void UMGExtendedProgressionSubsystem::ApplyCatchUpBonus()
{
	// Could add hidden bonuses to rep/money gains
	UE_LOG(LogMGProgression, Log, TEXT("Applied catch-up bonus"));
}

float UMGExtendedProgressionSubsystem::GetDifficultyAdjustment() const
{
	// Reduce difficulty after 3 consecutive losses
	if (ConsecutiveLosses >= 3)
	{
		return 0.9f; // 10% easier
	}
	return 1.0f;
}

// =============================================================================
// DATA TABLE LOADING
// =============================================================================

void UMGExtendedProgressionSubsystem::LoadProgressionData()
{
	LoadCarUnlocks();
	LoadLocationUnlocks();
	LoadHousingUnlocks();
	LoadCustomizationUnlocks();
	LoadMilestones();

	UE_LOG(LogMGProgression, Log, TEXT("Loaded progression data: %d cars, %d locations, %d customizations, %d milestones"),
		CarUnlockDatabase.Num(), LocationUnlockDatabase.Num(), CustomizationUnlockDatabase.Num(), MilestoneDatabase.Num());
}

void UMGExtendedProgressionSubsystem::SetDataTables(
	UDataTable* Cars,
	UDataTable* Locations,
	UDataTable* Housing,
	UDataTable* Customizations,
	UDataTable* Milestones)
{
	CarUnlockTable = Cars;
	LocationUnlockTable = Locations;
	HousingUnlockTable = Housing;
	CustomizationUnlockTable = Customizations;
	MilestoneTable = Milestones;

	LoadProgressionData();
}

void UMGExtendedProgressionSubsystem::LoadCarUnlocks()
{
	if (!CarUnlockTable)
	{
		return;
	}

	TArray<FMGCarUnlockData*> Rows;
	CarUnlockTable->GetAllRows<FMGCarUnlockData>(TEXT("LoadCarUnlocks"), Rows);

	for (FMGCarUnlockData* Row : Rows)
	{
		if (Row)
		{
			CarUnlockDatabase.Add(Row->CarID, *Row);
		}
	}
}

void UMGExtendedProgressionSubsystem::LoadLocationUnlocks()
{
	if (!LocationUnlockTable)
	{
		return;
	}

	TArray<FMGLocationUnlockData*> Rows;
	LocationUnlockTable->GetAllRows<FMGLocationUnlockData>(TEXT("LoadLocationUnlocks"), Rows);

	for (FMGLocationUnlockData* Row : Rows)
	{
		if (Row)
		{
			LocationUnlockDatabase.Add(Row->LocationID, *Row);
		}
	}
}

void UMGExtendedProgressionSubsystem::LoadHousingUnlocks()
{
	if (!HousingUnlockTable)
	{
		return;
	}

	TArray<FMGHousingUnlockData*> Rows;
	HousingUnlockTable->GetAllRows<FMGHousingUnlockData>(TEXT("LoadHousingUnlocks"), Rows);

	for (FMGHousingUnlockData* Row : Rows)
	{
		if (Row)
		{
			HousingUnlockDatabase.Add(Row->Tier, *Row);
		}
	}
}

void UMGExtendedProgressionSubsystem::LoadCustomizationUnlocks()
{
	if (!CustomizationUnlockTable)
	{
		return;
	}

	TArray<FMGCustomizationUnlockData*> Rows;
	CustomizationUnlockTable->GetAllRows<FMGCustomizationUnlockData>(TEXT("LoadCustomizationUnlocks"), Rows);

	for (FMGCustomizationUnlockData* Row : Rows)
	{
		if (Row)
		{
			CustomizationUnlockDatabase.Add(Row->ItemID, *Row);
		}
	}
}

void UMGExtendedProgressionSubsystem::LoadMilestones()
{
	if (!MilestoneTable)
	{
		return;
	}

	TArray<FMGMilestoneData*> Rows;
	MilestoneTable->GetAllRows<FMGMilestoneData>(TEXT("LoadMilestones"), Rows);

	for (FMGMilestoneData* Row : Rows)
	{
		if (Row)
		{
			MilestoneDatabase.Add(Row->MilestoneID, *Row);
		}
	}
}

// =============================================================================
// HELPER
// =============================================================================

UMGPlayerProgression* UMGExtendedProgressionSubsystem::GetBaseProgression() const
{
	return GetGameInstance()->GetSubsystem<UMGPlayerProgression>();
}

// =============================================================================
// DEBUG / DEVELOPMENT
// =============================================================================

#if WITH_EDITOR
void UMGExtendedProgressionSubsystem::Debug_UnlockAll()
{
	// Unlock all cars
	for (const auto& Pair : CarUnlockDatabase)
	{
		if (!OwnsCar(Pair.Key))
		{
			FMGOwnedCar Car;
			Car.CarID = Pair.Key;
			Car.Tier = Pair.Value.Tier;
			OwnedCars.Add(Car);
		}
	}

	// Unlock all locations
	for (const auto& Pair : LocationUnlockDatabase)
	{
		if (!IsLocationUnlocked(Pair.Key))
		{
			FMGUnlockedLocation Loc;
			Loc.LocationID = Pair.Key;
			Loc.District = Pair.Value.District;
			UnlockedLocations.Add(Loc);
		}
	}

	// Max housing
	HousingData.CurrentTier = EMGHousingTier::Penthouse;
	HousingData.MaxDisplayCapacity = 15;

	// Tons of money
	AddMoney(100000000, false);

	UE_LOG(LogMGProgression, Warning, TEXT("DEBUG: Unlocked all content!"));
}

void UMGExtendedProgressionSubsystem::Debug_ResetProgression()
{
	OwnedCars.Empty();
	UnlockedLocations.Empty();
	CustomizationProgress.Empty();
	CompletedMilestones.Empty();
	HousingData = FMGHousingData();
	PlayerMoney = 0;

	UE_LOG(LogMGProgression, Warning, TEXT("DEBUG: Reset all progression!"));
}

void UMGExtendedProgressionSubsystem::Debug_SetProgressionHour(int32 TargetHour)
{
	// TODO: Auto-unlock content up to that hour mark based on progression curve
	UE_LOG(LogMGProgression, Warning, TEXT("DEBUG: Set progression to hour %d (not fully implemented)"), TargetHour);
}
#endif
