// Copyright Epic Games, Inc. All Rights Reserved.

#include "License/MGLicenseSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGLicenseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default licenses for all categories
	for (int32 i = 0; i < static_cast<int32>(EMGLicenseCategory::Endurance) + 1; ++i)
	{
		EMGLicenseCategory Category = static_cast<EMGLicenseCategory>(i);
		FMGPlayerLicense License;
		License.Category = Category;
		License.CurrentTier = EMGLicenseTier::None;
		License.LicensePoints = 0;
		PlayerLicenses.Add(Category, License);
	}

	// Setup tier privileges
	{
		FMGLicensePrivileges Learner;
		Learner.Tier = EMGLicenseTier::Learner;
		Learner.AllowedVehicleClasses = { TEXT("D") };
		Learner.AllowedEventTypes = { TEXT("FreeDrive"), TEXT("Practice") };
		Learner.MaxPurchasePrice = 25000;
		Learner.MaxUpgradeLevel = 1;
		Learner.bCanEnterOnlineRaces = false;
		Learner.bCanEnterTournaments = false;
		Learner.bCanCreateCrew = false;
		Learner.ReputationMultiplier = 0.5f;
		Learner.CashEarningsMultiplier = 0.5f;
		TierPrivileges.Add(EMGLicenseTier::Learner, Learner);
	}
	{
		FMGLicensePrivileges Street;
		Street.Tier = EMGLicenseTier::Street;
		Street.AllowedVehicleClasses = { TEXT("D"), TEXT("C") };
		Street.AllowedEventTypes = { TEXT("FreeDrive"), TEXT("Practice"), TEXT("Sprint"), TEXT("Circuit") };
		Street.MaxPurchasePrice = 50000;
		Street.MaxUpgradeLevel = 2;
		Street.bCanEnterOnlineRaces = true;
		Street.bCanEnterTournaments = false;
		Street.bCanCreateCrew = false;
		Street.ReputationMultiplier = 1.0f;
		Street.CashEarningsMultiplier = 1.0f;
		TierPrivileges.Add(EMGLicenseTier::Street, Street);
	}
	{
		FMGLicensePrivileges Club;
		Club.Tier = EMGLicenseTier::Club;
		Club.AllowedVehicleClasses = { TEXT("D"), TEXT("C"), TEXT("B") };
		Club.AllowedEventTypes = { TEXT("FreeDrive"), TEXT("Practice"), TEXT("Sprint"), TEXT("Circuit"), TEXT("TimeAttack") };
		Club.MaxPurchasePrice = 100000;
		Club.MaxUpgradeLevel = 3;
		Club.bCanEnterOnlineRaces = true;
		Club.bCanEnterTournaments = false;
		Club.bCanCreateCrew = true;
		Club.ReputationMultiplier = 1.1f;
		Club.CashEarningsMultiplier = 1.1f;
		TierPrivileges.Add(EMGLicenseTier::Club, Club);
	}
	{
		FMGLicensePrivileges Regional;
		Regional.Tier = EMGLicenseTier::Regional;
		Regional.AllowedVehicleClasses = { TEXT("D"), TEXT("C"), TEXT("B"), TEXT("A") };
		Regional.AllowedEventTypes = { TEXT("FreeDrive"), TEXT("Practice"), TEXT("Sprint"), TEXT("Circuit"), TEXT("TimeAttack"), TEXT("Drift") };
		Regional.MaxPurchasePrice = 250000;
		Regional.MaxUpgradeLevel = 4;
		Regional.bCanEnterOnlineRaces = true;
		Regional.bCanEnterTournaments = true;
		Regional.bCanCreateCrew = true;
		Regional.ReputationMultiplier = 1.2f;
		Regional.CashEarningsMultiplier = 1.15f;
		TierPrivileges.Add(EMGLicenseTier::Regional, Regional);
	}
	{
		FMGLicensePrivileges National;
		National.Tier = EMGLicenseTier::National;
		National.AllowedVehicleClasses = { TEXT("D"), TEXT("C"), TEXT("B"), TEXT("A"), TEXT("S") };
		National.AllowedEventTypes = { TEXT("FreeDrive"), TEXT("Practice"), TEXT("Sprint"), TEXT("Circuit"), TEXT("TimeAttack"), TEXT("Drift"), TEXT("Endurance") };
		National.MaxPurchasePrice = 500000;
		National.MaxUpgradeLevel = 5;
		National.bCanEnterOnlineRaces = true;
		National.bCanEnterTournaments = true;
		National.bCanCreateCrew = true;
		National.ReputationMultiplier = 1.3f;
		National.CashEarningsMultiplier = 1.2f;
		TierPrivileges.Add(EMGLicenseTier::National, National);
	}
	{
		FMGLicensePrivileges International;
		International.Tier = EMGLicenseTier::International;
		International.AllowedVehicleClasses = { TEXT("D"), TEXT("C"), TEXT("B"), TEXT("A"), TEXT("S"), TEXT("S+") };
		International.AllowedEventTypes = { TEXT("FreeDrive"), TEXT("Practice"), TEXT("Sprint"), TEXT("Circuit"), TEXT("TimeAttack"), TEXT("Drift"), TEXT("Endurance"), TEXT("Championship") };
		International.MaxPurchasePrice = 1000000;
		International.MaxUpgradeLevel = 6;
		International.bCanEnterOnlineRaces = true;
		International.bCanEnterTournaments = true;
		International.bCanCreateCrew = true;
		International.ReputationMultiplier = 1.5f;
		International.CashEarningsMultiplier = 1.3f;
		TierPrivileges.Add(EMGLicenseTier::International, International);
	}
	{
		FMGLicensePrivileges Professional;
		Professional.Tier = EMGLicenseTier::Professional;
		Professional.AllowedVehicleClasses = { TEXT("D"), TEXT("C"), TEXT("B"), TEXT("A"), TEXT("S"), TEXT("S+"), TEXT("Hyper") };
		Professional.AllowedEventTypes = { TEXT("All") };
		Professional.MaxPurchasePrice = 5000000;
		Professional.MaxUpgradeLevel = 7;
		Professional.bCanEnterOnlineRaces = true;
		Professional.bCanEnterTournaments = true;
		Professional.bCanCreateCrew = true;
		Professional.ReputationMultiplier = 2.0f;
		Professional.CashEarningsMultiplier = 1.5f;
		TierPrivileges.Add(EMGLicenseTier::Professional, Professional);
	}
	{
		FMGLicensePrivileges Elite;
		Elite.Tier = EMGLicenseTier::Elite;
		Elite.AllowedVehicleClasses = { TEXT("All") };
		Elite.AllowedEventTypes = { TEXT("All") };
		Elite.MaxPurchasePrice = 10000000;
		Elite.MaxUpgradeLevel = 8;
		Elite.bCanEnterOnlineRaces = true;
		Elite.bCanEnterTournaments = true;
		Elite.bCanCreateCrew = true;
		Elite.ReputationMultiplier = 2.5f;
		Elite.CashEarningsMultiplier = 1.75f;
		TierPrivileges.Add(EMGLicenseTier::Elite, Elite);
	}
	{
		FMGLicensePrivileges Legend;
		Legend.Tier = EMGLicenseTier::Legend;
		Legend.AllowedVehicleClasses = { TEXT("All") };
		Legend.AllowedEventTypes = { TEXT("All") };
		Legend.MaxPurchasePrice = 999999999;
		Legend.MaxUpgradeLevel = 10;
		Legend.bCanEnterOnlineRaces = true;
		Legend.bCanEnterTournaments = true;
		Legend.bCanCreateCrew = true;
		Legend.ReputationMultiplier = 3.0f;
		Legend.CashEarningsMultiplier = 2.0f;
		TierPrivileges.Add(EMGLicenseTier::Legend, Legend);
	}

	LoadLicenseData();
}

void UMGLicenseSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TestUpdateTimer);
	}

	SaveLicenseData();
	Super::Deinitialize();
}

EMGLicenseTier UMGLicenseSubsystem::GetCurrentLicenseTier(EMGLicenseCategory Category) const
{
	if (const FMGPlayerLicense* License = PlayerLicenses.Find(Category))
	{
		return License->CurrentTier;
	}
	return EMGLicenseTier::None;
}

FMGPlayerLicense UMGLicenseSubsystem::GetPlayerLicense(EMGLicenseCategory Category) const
{
	if (const FMGPlayerLicense* License = PlayerLicenses.Find(Category))
	{
		return *License;
	}
	return FMGPlayerLicense();
}

bool UMGLicenseSubsystem::HasLicenseTier(EMGLicenseCategory Category, EMGLicenseTier RequiredTier) const
{
	EMGLicenseTier CurrentTier = GetCurrentLicenseTier(Category);
	return static_cast<int32>(CurrentTier) >= static_cast<int32>(RequiredTier);
}

bool UMGLicenseSubsystem::CanUpgradeLicense(EMGLicenseCategory Category, EMGLicenseTier TargetTier) const
{
	EMGLicenseTier CurrentTier = GetCurrentLicenseTier(Category);

	// Can only upgrade by one tier at a time
	int32 CurrentIndex = static_cast<int32>(CurrentTier);
	int32 TargetIndex = static_cast<int32>(TargetTier);

	if (TargetIndex != CurrentIndex + 1)
	{
		return false;
	}

	// Check if required schools are completed for this tier
	for (const auto& SchoolPair : RegisteredSchools)
	{
		const FMGLicenseSchool& School = SchoolPair.Value;
		if (School.Category == Category && School.TargetTier == TargetTier)
		{
			if (!IsSchoolCompleted(School.SchoolId))
			{
				return false;
			}
		}
	}

	return true;
}

bool UMGLicenseSubsystem::UpgradeLicense(EMGLicenseCategory Category, EMGLicenseTier NewTier)
{
	if (!CanUpgradeLicense(Category, NewTier))
	{
		return false;
	}

	FMGPlayerLicense* License = PlayerLicenses.Find(Category);
	if (!License)
	{
		return false;
	}

	License->CurrentTier = NewTier;
	License->LastUpgradeDate = FDateTime::Now();

	if (License->LicenseObtainedDate == FDateTime())
	{
		License->LicenseObtainedDate = FDateTime::Now();
	}

	OnLicenseUpgraded.Broadcast(Category, NewTier);
	SaveLicenseData();

	return true;
}

FMGLicensePrivileges UMGLicenseSubsystem::GetLicensePrivileges(EMGLicenseTier Tier) const
{
	if (const FMGLicensePrivileges* Privileges = TierPrivileges.Find(Tier))
	{
		return *Privileges;
	}
	return FMGLicensePrivileges();
}

EMGLicenseTier UMGLicenseSubsystem::GetHighestLicenseTier() const
{
	EMGLicenseTier Highest = EMGLicenseTier::None;

	for (const auto& LicensePair : PlayerLicenses)
	{
		if (static_cast<int32>(LicensePair.Value.CurrentTier) > static_cast<int32>(Highest))
		{
			Highest = LicensePair.Value.CurrentTier;
		}
	}

	return Highest;
}

int32 UMGLicenseSubsystem::GetTotalGoldMedals() const
{
	int32 Total = 0;
	for (const auto& LicensePair : PlayerLicenses)
	{
		Total += LicensePair.Value.TotalGoldMedals;
	}
	return Total;
}

int32 UMGLicenseSubsystem::GetTotalPlatinumMedals() const
{
	int32 Total = 0;
	for (const auto& LicensePair : PlayerLicenses)
	{
		Total += LicensePair.Value.TotalPlatinumMedals;
	}
	return Total;
}

bool UMGLicenseSubsystem::RegisterSchool(const FMGLicenseSchool& School)
{
	if (School.SchoolId.IsEmpty())
	{
		return false;
	}

	RegisteredSchools.Add(School.SchoolId, School);
	return true;
}

FMGLicenseSchool UMGLicenseSubsystem::GetSchool(const FString& SchoolId) const
{
	if (const FMGLicenseSchool* School = RegisteredSchools.Find(SchoolId))
	{
		return *School;
	}
	return FMGLicenseSchool();
}

TArray<FMGLicenseSchool> UMGLicenseSubsystem::GetAvailableSchools(EMGLicenseCategory Category) const
{
	TArray<FMGLicenseSchool> Result;

	for (const auto& SchoolPair : RegisteredSchools)
	{
		const FMGLicenseSchool& School = SchoolPair.Value;
		if (School.Category == Category)
		{
			// Check if player has required tier to attempt this school
			EMGLicenseTier RequiredTier = static_cast<EMGLicenseTier>(FMath::Max(0, static_cast<int32>(School.TargetTier) - 1));
			if (HasLicenseTier(Category, RequiredTier))
			{
				Result.Add(School);
			}
		}
	}

	return Result;
}

TArray<FMGLicenseSchool> UMGLicenseSubsystem::GetAllSchools() const
{
	TArray<FMGLicenseSchool> Result;
	RegisteredSchools.GenerateValueArray(Result);
	return Result;
}

bool UMGLicenseSubsystem::IsSchoolCompleted(const FString& SchoolId) const
{
	const FMGLicenseSchool* School = RegisteredSchools.Find(SchoolId);
	if (!School)
	{
		return false;
	}

	const FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (!License)
	{
		return false;
	}

	return License->CompletedSchools.Contains(SchoolId);
}

bool UMGLicenseSubsystem::IsSchoolAllGold(const FString& SchoolId) const
{
	const FMGLicenseSchool* School = RegisteredSchools.Find(SchoolId);
	if (!School)
	{
		return false;
	}

	const FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (!License)
	{
		return false;
	}

	for (const FMGLicenseTest& Test : School->Tests)
	{
		if (const FMGTestResult* Result = License->TestResults.Find(Test.TestId))
		{
			if (static_cast<int32>(Result->BestGrade) < static_cast<int32>(EMGTestGrade::Gold))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

float UMGLicenseSubsystem::GetSchoolCompletionPercent(const FString& SchoolId) const
{
	const FMGLicenseSchool* School = RegisteredSchools.Find(SchoolId);
	if (!School || School->Tests.Num() == 0)
	{
		return 0.0f;
	}

	const FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (!License)
	{
		return 0.0f;
	}

	int32 CompletedTests = 0;
	for (const FMGLicenseTest& Test : School->Tests)
	{
		if (const FMGTestResult* Result = License->TestResults.Find(Test.TestId))
		{
			if (Result->BestGrade >= EMGTestGrade::Bronze)
			{
				CompletedTests++;
			}
		}
	}

	return static_cast<float>(CompletedTests) / static_cast<float>(School->Tests.Num()) * 100.0f;
}

int32 UMGLicenseSubsystem::GetSchoolGoldCount(const FString& SchoolId) const
{
	const FMGLicenseSchool* School = RegisteredSchools.Find(SchoolId);
	if (!School)
	{
		return 0;
	}

	const FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (!License)
	{
		return 0;
	}

	int32 GoldCount = 0;
	for (const FMGLicenseTest& Test : School->Tests)
	{
		if (const FMGTestResult* Result = License->TestResults.Find(Test.TestId))
		{
			if (Result->BestGrade >= EMGTestGrade::Gold)
			{
				GoldCount++;
			}
		}
	}

	return GoldCount;
}

bool UMGLicenseSubsystem::StartTest(const FString& TestId, const FString& SchoolId)
{
	if (bTestActive)
	{
		return false;
	}

	if (!IsTestAvailable(TestId))
	{
		return false;
	}

	ActiveTestSession = FMGActiveTestSession();
	ActiveTestSession.TestId = TestId;
	ActiveTestSession.SchoolId = SchoolId;
	ActiveTestSession.StartTime = FDateTime::Now();
	ActiveTestSession.bIsValid = true;
	bTestActive = true;

	OnTestStarted.Broadcast(TestId);
	return true;
}

bool UMGLicenseSubsystem::EndTest(float FinalTime, int32 FinalScore, bool bCompleted)
{
	if (!bTestActive)
	{
		return false;
	}

	FMGLicenseSchool* School = nullptr;
	FMGLicenseTest* Test = FindTest(ActiveTestSession.TestId, &School);

	if (!Test || !School)
	{
		bTestActive = false;
		return false;
	}

	float TotalTime = FinalTime + ActiveTestSession.PenaltyTime;

	EMGTestGrade Grade = EMGTestGrade::Failed;
	if (bCompleted && ActiveTestSession.bIsValid)
	{
		// Determine grade based on test type
		if (Test->TestType == EMGLicenseTestType::Written)
		{
			Grade = CalculateGradeFromScore(*Test, FinalScore);
		}
		else
		{
			Grade = CalculateGradeFromTime(*Test, TotalTime);
		}
	}

	// Update player license
	FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (License)
	{
		FMGTestResult* Result = License->TestResults.Find(ActiveTestSession.TestId);
		if (!Result)
		{
			FMGTestResult NewResult;
			NewResult.TestId = ActiveTestSession.TestId;
			License->TestResults.Add(ActiveTestSession.TestId, NewResult);
			Result = License->TestResults.Find(ActiveTestSession.TestId);
		}

		Result->TotalAttempts++;
		Result->LastAttemptDate = FDateTime::Now();

		bool bNewBestTime = false;
		if (Grade > EMGTestGrade::Failed)
		{
			if (Result->BestGrade == EMGTestGrade::NotAttempted)
			{
				Result->FirstCompletedDate = FDateTime::Now();
			}

			// Check for new best
			if (static_cast<int32>(Grade) > static_cast<int32>(Result->BestGrade))
			{
				// Track medal upgrades
				if (Grade >= EMGTestGrade::Gold && Result->BestGrade < EMGTestGrade::Gold)
				{
					License->TotalGoldMedals++;
				}
				if (Grade >= EMGTestGrade::Platinum && Result->BestGrade < EMGTestGrade::Platinum)
				{
					License->TotalPlatinumMedals++;
				}

				Result->BestGrade = Grade;
				Result->BestGradeDate = FDateTime::Now();
			}

			if (Result->BestTime == 0.0f || TotalTime < Result->BestTime)
			{
				float OldTime = Result->BestTime;
				Result->BestTime = TotalTime;
				bNewBestTime = OldTime > 0.0f;

				if (bNewBestTime)
				{
					OnNewBestTime.Broadcast(ActiveTestSession.TestId, OldTime, TotalTime);
				}
			}

			if (FinalScore > Result->BestScore)
			{
				Result->BestScore = FinalScore;
			}
		}

		// Check if school is now completed
		UpdateLicenseFromSchoolCompletion(School->SchoolId);
	}

	OnTestCompleted.Broadcast(ActiveTestSession.TestId, Grade, TotalTime);

	bTestActive = false;
	ActiveTestSession = FMGActiveTestSession();

	SaveLicenseData();
	return true;
}

void UMGLicenseSubsystem::CancelTest()
{
	if (!bTestActive)
	{
		return;
	}

	bTestActive = false;
	ActiveTestSession = FMGActiveTestSession();
}

void UMGLicenseSubsystem::AddPenalty(float PenaltySeconds, const FString& Reason)
{
	if (!bTestActive)
	{
		return;
	}

	ActiveTestSession.PenaltyTime += PenaltySeconds;
	ActiveTestSession.PenaltyCount++;
}

void UMGLicenseSubsystem::RecordSectorTime(float SectorTime)
{
	if (!bTestActive)
	{
		return;
	}

	ActiveTestSession.SectorTimes.Add(SectorTime);
}

void UMGLicenseSubsystem::InvalidateTest(const FString& Reason)
{
	if (!bTestActive)
	{
		return;
	}

	ActiveTestSession.bIsValid = false;
	OnTestFailed.Broadcast(ActiveTestSession.TestId, Reason);
}

bool UMGLicenseSubsystem::IsTestActive() const
{
	return bTestActive;
}

FMGActiveTestSession UMGLicenseSubsystem::GetActiveTestSession() const
{
	return ActiveTestSession;
}

bool UMGLicenseSubsystem::IsTestAvailable(const FString& TestId) const
{
	FMGLicenseSchool* School = nullptr;
	FMGLicenseTest* Test = const_cast<UMGLicenseSubsystem*>(this)->FindTest(TestId, &School);

	if (!Test || !School)
	{
		return false;
	}

	// Check license tier requirement
	if (!HasLicenseTier(School->Category, Test->RequiredTier))
	{
		return false;
	}

	// Check prerequisites
	const FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (License)
	{
		for (const FString& PrereqId : Test->PrerequisiteTestIds)
		{
			if (const FMGTestResult* Result = License->TestResults.Find(PrereqId))
			{
				if (Result->BestGrade < EMGTestGrade::Bronze)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

FMGTestResult UMGLicenseSubsystem::GetTestResult(const FString& TestId) const
{
	for (const auto& LicensePair : PlayerLicenses)
	{
		if (const FMGTestResult* Result = LicensePair.Value.TestResults.Find(TestId))
		{
			return *Result;
		}
	}
	return FMGTestResult();
}

EMGTestGrade UMGLicenseSubsystem::GetTestGrade(const FString& TestId) const
{
	FMGTestResult Result = GetTestResult(TestId);
	return Result.BestGrade;
}

EMGTestGrade UMGLicenseSubsystem::CalculateGradeFromTime(const FMGLicenseTest& Test, float Time) const
{
	if (Time <= Test.PlatinumTime)
	{
		return EMGTestGrade::Platinum;
	}
	if (Time <= Test.GoldTime)
	{
		return EMGTestGrade::Gold;
	}
	if (Time <= Test.SilverTime)
	{
		return EMGTestGrade::Silver;
	}
	if (Time <= Test.BronzeTime)
	{
		return EMGTestGrade::Bronze;
	}
	return EMGTestGrade::Failed;
}

EMGTestGrade UMGLicenseSubsystem::CalculateGradeFromScore(const FMGLicenseTest& Test, int32 Score) const
{
	if (Score >= Test.PlatinumScore)
	{
		return EMGTestGrade::Platinum;
	}
	if (Score >= Test.GoldScore)
	{
		return EMGTestGrade::Gold;
	}
	if (Score >= Test.SilverScore)
	{
		return EMGTestGrade::Silver;
	}
	if (Score >= Test.BronzeScore)
	{
		return EMGTestGrade::Bronze;
	}
	return EMGTestGrade::Failed;
}

bool UMGLicenseSubsystem::CanAccessVehicleClass(const FString& VehicleClassId) const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return Privileges.AllowedVehicleClasses.Contains(TEXT("All")) ||
		   Privileges.AllowedVehicleClasses.Contains(VehicleClassId);
}

bool UMGLicenseSubsystem::CanAccessEvent(const FString& EventType) const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return Privileges.AllowedEventTypes.Contains(TEXT("All")) ||
		   Privileges.AllowedEventTypes.Contains(EventType);
}

bool UMGLicenseSubsystem::CanAccessTrack(const FString& TrackId) const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return Privileges.AllowedTracks.IsEmpty() ||
		   Privileges.AllowedTracks.Contains(TEXT("All")) ||
		   Privileges.AllowedTracks.Contains(TrackId);
}

bool UMGLicenseSubsystem::CanPurchaseVehicle(int32 VehiclePrice) const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return VehiclePrice <= Privileges.MaxPurchasePrice;
}

int32 UMGLicenseSubsystem::GetMaxUpgradeLevel() const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return Privileges.MaxUpgradeLevel;
}

float UMGLicenseSubsystem::GetReputationMultiplier() const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return Privileges.ReputationMultiplier;
}

float UMGLicenseSubsystem::GetCashMultiplier() const
{
	EMGLicenseTier HighestTier = GetHighestLicenseTier();
	FMGLicensePrivileges Privileges = GetLicensePrivileges(HighestTier);

	return Privileges.CashEarningsMultiplier;
}

int32 UMGLicenseSubsystem::GetTotalTestsCompleted() const
{
	int32 Total = 0;
	for (const auto& LicensePair : PlayerLicenses)
	{
		for (const auto& TestPair : LicensePair.Value.TestResults)
		{
			if (TestPair.Value.BestGrade >= EMGTestGrade::Bronze)
			{
				Total++;
			}
		}
	}
	return Total;
}

int32 UMGLicenseSubsystem::GetTotalTestAttempts() const
{
	int32 Total = 0;
	for (const auto& LicensePair : PlayerLicenses)
	{
		for (const auto& TestPair : LicensePair.Value.TestResults)
		{
			Total += TestPair.Value.TotalAttempts;
		}
	}
	return Total;
}

float UMGLicenseSubsystem::GetAverageTestGrade() const
{
	int32 TotalGrade = 0;
	int32 Count = 0;

	for (const auto& LicensePair : PlayerLicenses)
	{
		for (const auto& TestPair : LicensePair.Value.TestResults)
		{
			if (TestPair.Value.BestGrade > EMGTestGrade::NotAttempted)
			{
				TotalGrade += static_cast<int32>(TestPair.Value.BestGrade);
				Count++;
			}
		}
	}

	return Count > 0 ? static_cast<float>(TotalGrade) / static_cast<float>(Count) : 0.0f;
}

float UMGLicenseSubsystem::GetOverallLicenseProgress() const
{
	int32 MaxTier = static_cast<int32>(EMGLicenseTier::Legend);
	int32 NumCategories = static_cast<int32>(EMGLicenseCategory::Endurance) + 1;
	int32 TotalPossible = MaxTier * NumCategories;
	int32 TotalAchieved = 0;

	for (const auto& LicensePair : PlayerLicenses)
	{
		TotalAchieved += static_cast<int32>(LicensePair.Value.CurrentTier);
	}

	return TotalPossible > 0 ? static_cast<float>(TotalAchieved) / static_cast<float>(TotalPossible) * 100.0f : 0.0f;
}

void UMGLicenseSubsystem::SaveLicenseData()
{
	// TODO: Implement save to SaveGame object
}

void UMGLicenseSubsystem::LoadLicenseData()
{
	// TODO: Implement load from SaveGame object
}

void UMGLicenseSubsystem::UpdateLicenseFromSchoolCompletion(const FString& SchoolId)
{
	const FMGLicenseSchool* School = RegisteredSchools.Find(SchoolId);
	if (!School)
	{
		return;
	}

	FMGPlayerLicense* License = PlayerLicenses.Find(School->Category);
	if (!License)
	{
		return;
	}

	// Count completed tests
	int32 CompletedTests = 0;
	int32 GoldTests = 0;
	int32 PlatinumTests = 0;

	for (const FMGLicenseTest& Test : School->Tests)
	{
		if (const FMGTestResult* Result = License->TestResults.Find(Test.TestId))
		{
			if (Result->BestGrade >= EMGTestGrade::Bronze)
			{
				CompletedTests++;
			}
			if (Result->BestGrade >= EMGTestGrade::Gold)
			{
				GoldTests++;
			}
			if (Result->BestGrade >= EMGTestGrade::Platinum)
			{
				PlatinumTests++;
			}
		}
	}

	int32 TestsRequired = School->TestsRequiredToPass > 0 ? School->TestsRequiredToPass : School->Tests.Num();

	if (CompletedTests >= TestsRequired && !License->CompletedSchools.Contains(SchoolId))
	{
		License->CompletedSchools.Add(SchoolId);
		GrantSchoolRewards(*School, GoldTests, PlatinumTests);
		OnSchoolCompleted.Broadcast(SchoolId, GoldTests);
		CheckLicenseUpgrade(School->Category);
	}
}

void UMGLicenseSubsystem::GrantSchoolRewards(const FMGLicenseSchool& School, int32 GoldCount, int32 PlatinumCount)
{
	// Base cash reward
	int32 TotalCash = School.CashReward;

	// Gold bonus
	if (GoldCount >= School.GoldTestsForBonus)
	{
		TotalCash += School.GoldBonusCash;
	}

	// All platinum bonus
	if (PlatinumCount == School.Tests.Num())
	{
		TotalCash += School.PlatinumBonusCash;
	}

	// TODO: Grant cash through economy subsystem
	// TODO: Grant vehicle rewards
}

void UMGLicenseSubsystem::CheckLicenseUpgrade(EMGLicenseCategory Category)
{
	EMGLicenseTier CurrentTier = GetCurrentLicenseTier(Category);
	EMGLicenseTier NextTier = static_cast<EMGLicenseTier>(static_cast<int32>(CurrentTier) + 1);

	if (NextTier <= EMGLicenseTier::Legend && CanUpgradeLicense(Category, NextTier))
	{
		UpgradeLicense(Category, NextTier);
	}
}

FMGLicenseTest* UMGLicenseSubsystem::FindTest(const FString& TestId, FMGLicenseSchool** OutSchool)
{
	for (auto& SchoolPair : RegisteredSchools)
	{
		for (FMGLicenseTest& Test : SchoolPair.Value.Tests)
		{
			if (Test.TestId == TestId)
			{
				if (OutSchool)
				{
					*OutSchool = &SchoolPair.Value;
				}
				return &Test;
			}
		}
	}
	return nullptr;
}
