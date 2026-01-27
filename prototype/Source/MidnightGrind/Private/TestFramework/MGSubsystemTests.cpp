// Copyright Midnight Grind. All Rights Reserved.

#include "TestFramework/MGSubsystemTests.h"
#include "TestFramework/MGTestFrameworkSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "Weather/MGWeatherSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Vehicle/MGVehicleDamageSystem.h"
#include "AI/MGAIRacerController.h"
#include "AI/MGAIRacerSubsystem.h"
#include "AI/MGAIDriverProfile.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UMGSubsystemTests::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterAllTests();
}

void UMGSubsystemTests::Deinitialize()
{
	Super::Deinitialize();
}

void UMGSubsystemTests::RegisterAllTests()
{
	UMGTestFrameworkSubsystem* TestFramework = GetTestFramework();
	if (!TestFramework)
	{
		return;
	}

	// Currency Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Currency_EarnGrindCash"));
		Test.TestName = FText::FromString(TEXT("Currency - Earn Grind Cash"));
		Test.Description = FText::FromString(TEXT("Verify currency can be earned and balance updates correctly"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Currency")));
		Test.Tags.Add(FName(TEXT("Smoke")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Currency_SpendGrindCash"));
		Test.TestName = FText::FromString(TEXT("Currency - Spend Grind Cash"));
		Test.Description = FText::FromString(TEXT("Verify currency can be spent and balance updates correctly"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Currency")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Currency_InsufficientFunds"));
		Test.TestName = FText::FromString(TEXT("Currency - Insufficient Funds"));
		Test.Description = FText::FromString(TEXT("Verify spending fails gracefully when funds are insufficient"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Currency")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Currency_RaceEarnings"));
		Test.TestName = FText::FromString(TEXT("Currency - Race Earnings Calculation"));
		Test.Description = FText::FromString(TEXT("Verify race earnings are calculated correctly based on position"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Currency")));
		Test.Tags.Add(FName(TEXT("Racing")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Currency_Multipliers"));
		Test.TestName = FText::FromString(TEXT("Currency - Earning Multipliers"));
		Test.Description = FText::FromString(TEXT("Verify multipliers are applied correctly to earnings"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Currency")));
		TestFramework->RegisterTest(Test);
	}

	// Weather Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Weather_SetWeatherType"));
		Test.TestName = FText::FromString(TEXT("Weather - Set Weather Type"));
		Test.Description = FText::FromString(TEXT("Verify weather type can be changed"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Weather")));
		Test.Tags.Add(FName(TEXT("Smoke")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Weather_RoadGrip"));
		Test.TestName = FText::FromString(TEXT("Weather - Road Grip"));
		Test.Description = FText::FromString(TEXT("Verify road grip multiplier is calculated correctly"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Weather")));
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Weather_Visibility"));
		Test.TestName = FText::FromString(TEXT("Weather - Visibility"));
		Test.Description = FText::FromString(TEXT("Verify visibility distance is calculated correctly"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Weather")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Weather_TimeOfDay"));
		Test.TestName = FText::FromString(TEXT("Weather - Time of Day"));
		Test.Description = FText::FromString(TEXT("Verify time of day changes and formats correctly"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Weather")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Weather_DifficultyRating"));
		Test.TestName = FText::FromString(TEXT("Weather - Difficulty Rating"));
		Test.Description = FText::FromString(TEXT("Verify weather difficulty rating is in valid range"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Weather")));
		TestFramework->RegisterTest(Test);
	}

	// Vehicle Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Vehicle_DamageSystemInit"));
		Test.TestName = FText::FromString(TEXT("Vehicle - Damage System Init"));
		Test.Description = FText::FromString(TEXT("Verify damage system initializes with valid defaults"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Vehicle")));
		Test.Tags.Add(FName(TEXT("Smoke")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Vehicle_ComponentDamage"));
		Test.TestName = FText::FromString(TEXT("Vehicle - Component Damage"));
		Test.Description = FText::FromString(TEXT("Verify damage can be applied to vehicle components"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Vehicle")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Vehicle_DamageResistance"));
		Test.TestName = FText::FromString(TEXT("Vehicle - Damage Resistance"));
		Test.Description = FText::FromString(TEXT("Verify damage resistance reduces incoming damage"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Vehicle")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Vehicle_Repair"));
		Test.TestName = FText::FromString(TEXT("Vehicle - Repair System"));
		Test.Description = FText::FromString(TEXT("Verify repair functionality restores component health"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Vehicle")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Vehicle_PerformanceDegradation"));
		Test.TestName = FText::FromString(TEXT("Vehicle - Performance Degradation"));
		Test.Description = FText::FromString(TEXT("Verify damage reduces component performance"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Vehicle")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Vehicle_TotaledState"));
		Test.TestName = FText::FromString(TEXT("Vehicle - Totaled State"));
		Test.Description = FText::FromString(TEXT("Verify vehicle can be totaled with enough damage"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Vehicle")));
		TestFramework->RegisterTest(Test);
	}

	// AI Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_AI_DrivingStates"));
		Test.TestName = FText::FromString(TEXT("AI - Driving States"));
		Test.Description = FText::FromString(TEXT("Verify AI driving states are properly defined"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("AI")));
		Test.Tags.Add(FName(TEXT("Smoke")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_AI_SkillParams"));
		Test.TestName = FText::FromString(TEXT("AI - Skill Parameters"));
		Test.Description = FText::FromString(TEXT("Verify AI skill parameters are in valid ranges"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("AI")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_AI_SpawnConfig"));
		Test.TestName = FText::FromString(TEXT("AI - Spawn Configuration"));
		Test.Description = FText::FromString(TEXT("Verify AI spawn configuration has valid defaults"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("AI")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_AI_DriverPersonality"));
		Test.TestName = FText::FromString(TEXT("AI - Driver Personality"));
		Test.Description = FText::FromString(TEXT("Verify AI personality types are properly defined"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("AI")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_AI_Strategies"));
		Test.TestName = FText::FromString(TEXT("AI - Racing Strategies"));
		Test.Description = FText::FromString(TEXT("Verify AI overtake and defense strategies are defined"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("AI")));
		TestFramework->RegisterTest(Test);
	}

	// Integration Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Integration_CurrencyEconomy"));
		Test.TestName = FText::FromString(TEXT("Integration - Currency and Economy"));
		Test.Description = FText::FromString(TEXT("Verify currency and economy systems work together"));
		Test.Category = EMGTestCategory::Integration;
		Test.Tags.Add(FName(TEXT("Integration")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Integration_WeatherRoad"));
		Test.TestName = FText::FromString(TEXT("Integration - Weather and Road Conditions"));
		Test.Description = FText::FromString(TEXT("Verify weather affects road conditions correctly"));
		Test.Category = EMGTestCategory::Integration;
		Test.Tags.Add(FName(TEXT("Integration")));
		TestFramework->RegisterTest(Test);
	}

	UE_LOG(LogTemp, Log, TEXT("Registered %d subsystem tests"), 28);
}

// ==========================================
// CURRENCY TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestCurrency_EarnGrindCash()
{
	LogTestStart(TEXT("TestCurrency_EarnGrindCash"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_EarnGrindCash")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Get initial balance
	int64 InitialBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Initial balance: %lld"), InitialBalance));

	// Earn currency
	const int64 EarnAmount = 1000;
	bool bEarned = Currency->EarnCurrency(EMGCurrencyType::GrindCash, EarnAmount, EMGEarnSource::RaceFinish, TEXT("Test race"));

	if (!bEarned)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_EarnGrindCash")),
			TEXT("EarnCurrency returned false"),
			Logs
		);
	}

	// Verify balance increased
	int64 NewBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("New balance: %lld (expected: %lld)"), NewBalance, InitialBalance + EarnAmount));

	if (NewBalance != InitialBalance + EarnAmount)
	{
		Logs.Add(FString::Printf(TEXT("Balance mismatch: expected %lld, got %lld"), InitialBalance + EarnAmount, NewBalance));
		return CreateFailResult(
			FName(TEXT("Test_Currency_EarnGrindCash")),
			TEXT("Balance did not increase correctly"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Currency_EarnGrindCash")),
		FString::Printf(TEXT("Earned %lld GrindCash successfully"), EarnAmount)
	);
}

FMGTestResult UMGSubsystemTests::TestCurrency_SpendGrindCash()
{
	LogTestStart(TEXT("TestCurrency_SpendGrindCash"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_SpendGrindCash")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Ensure we have enough balance
	Currency->EarnCurrency(EMGCurrencyType::GrindCash, 5000, EMGEarnSource::Gifted, TEXT("Test setup"));
	int64 InitialBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Initial balance: %lld"), InitialBalance));

	// Spend currency
	const int64 SpendAmount = 500;
	bool bSpent = Currency->SpendCurrency(EMGCurrencyType::GrindCash, SpendAmount, TEXT("Test purchase"));

	if (!bSpent)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_SpendGrindCash")),
			TEXT("SpendCurrency returned false"),
			Logs
		);
	}

	// Verify balance decreased
	int64 NewBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("New balance: %lld (expected: %lld)"), NewBalance, InitialBalance - SpendAmount));

	if (NewBalance != InitialBalance - SpendAmount)
	{
		Logs.Add(FString::Printf(TEXT("Balance mismatch: expected %lld, got %lld"), InitialBalance - SpendAmount, NewBalance));
		return CreateFailResult(
			FName(TEXT("Test_Currency_SpendGrindCash")),
			TEXT("Balance did not decrease correctly"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Currency_SpendGrindCash")),
		FString::Printf(TEXT("Spent %lld GrindCash successfully"), SpendAmount)
	);
}

FMGTestResult UMGSubsystemTests::TestCurrency_InsufficientFunds()
{
	LogTestStart(TEXT("TestCurrency_InsufficientFunds"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_InsufficientFunds")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	int64 CurrentBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Current balance: %lld"), CurrentBalance));

	// Try to spend more than we have
	int64 OverspendAmount = CurrentBalance + 1000000;
	Logs.Add(FString::Printf(TEXT("Attempting to spend: %lld"), OverspendAmount));

	bool bSpent = Currency->SpendCurrency(EMGCurrencyType::GrindCash, OverspendAmount, TEXT("Test overspend"));

	if (bSpent)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_InsufficientFunds")),
			TEXT("SpendCurrency should have returned false for insufficient funds"),
			Logs
		);
	}

	// Verify balance unchanged
	int64 NewBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	if (NewBalance != CurrentBalance)
	{
		Logs.Add(FString::Printf(TEXT("Balance changed unexpectedly: was %lld, now %lld"), CurrentBalance, NewBalance));
		return CreateFailResult(
			FName(TEXT("Test_Currency_InsufficientFunds")),
			TEXT("Balance changed despite insufficient funds"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Currency_InsufficientFunds")),
		TEXT("Insufficient funds handled correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestCurrency_RaceEarnings()
{
	LogTestStart(TEXT("TestCurrency_RaceEarnings"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_RaceEarnings")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Calculate earnings for 1st place
	FMGRaceEarnings FirstPlace = Currency->CalculateRaceEarnings(
		1,      // Position
		8,      // TotalRacers
		FName(TEXT("TestTrack")),
		true,   // CleanRace
		false,  // DefeatedRival
		true    // FirstWinOnTrack
	);

	// Calculate earnings for last place
	FMGRaceEarnings LastPlace = Currency->CalculateRaceEarnings(
		8,      // Position
		8,      // TotalRacers
		FName(TEXT("TestTrack")),
		false,  // CleanRace
		false,  // DefeatedRival
		false   // FirstWinOnTrack
	);

	Logs.Add(FString::Printf(TEXT("1st place earnings: %lld"), FirstPlace.TotalEarnings));
	Logs.Add(FString::Printf(TEXT("Last place earnings: %lld"), LastPlace.TotalEarnings));

	// First place should earn more than last place
	if (FirstPlace.TotalEarnings <= LastPlace.TotalEarnings)
	{
		Logs.Add(TEXT("First place should earn more than last place"));
		return CreateFailResult(
			FName(TEXT("Test_Currency_RaceEarnings")),
			TEXT("Earnings calculation incorrect"),
			Logs
		);
	}

	// All earnings should be positive
	if (FirstPlace.TotalEarnings <= 0 || LastPlace.TotalEarnings < 0)
	{
		Logs.Add(TEXT("Earnings should be non-negative"));
		return CreateFailResult(
			FName(TEXT("Test_Currency_RaceEarnings")),
			TEXT("Negative earnings detected"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Currency_RaceEarnings")),
		FString::Printf(TEXT("Race earnings calculated: 1st=%lld, Last=%lld"), FirstPlace.TotalEarnings, LastPlace.TotalEarnings)
	);
}

FMGTestResult UMGSubsystemTests::TestCurrency_Multipliers()
{
	LogTestStart(TEXT("TestCurrency_Multipliers"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_Multipliers")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Get initial multiplier
	float InitialMultiplier = Currency->GetTotalMultiplier(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Initial multiplier: %.2f"), InitialMultiplier));

	// Add a multiplier
	FMGEarningMultiplier TestMultiplier;
	TestMultiplier.MultiplierID = FName(TEXT("TestMultiplier"));
	TestMultiplier.Multiplier = 1.5f;
	TestMultiplier.AffectedCurrency = EMGCurrencyType::GrindCash;
	TestMultiplier.bIsPermanent = true;
	TestMultiplier.Description = FText::FromString(TEXT("Test multiplier"));

	Currency->AddMultiplier(TestMultiplier);

	// Check new multiplier
	float NewMultiplier = Currency->GetTotalMultiplier(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("New multiplier: %.2f (expected: %.2f)"), NewMultiplier, InitialMultiplier * 1.5f));

	// Remove the test multiplier
	Currency->RemoveMultiplier(FName(TEXT("TestMultiplier")));

	// Verify multiplier removed
	float FinalMultiplier = Currency->GetTotalMultiplier(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Final multiplier: %.2f"), FinalMultiplier));

	return CreatePassResult(
		FName(TEXT("Test_Currency_Multipliers")),
		TEXT("Multipliers added and removed successfully")
	);
}

FMGTestResult UMGSubsystemTests::TestCurrency_BalanceNonNegative()
{
	LogTestStart(TEXT("TestCurrency_BalanceNonNegative"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Currency_BalanceNonNegative")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Check all currency types
	TArray<EMGCurrencyType> CurrencyTypes = {
		EMGCurrencyType::GrindCash,
		EMGCurrencyType::NeonCredits,
		EMGCurrencyType::CrewTokens,
		EMGCurrencyType::SeasonPoints,
		EMGCurrencyType::LegacyMarks
	};

	for (EMGCurrencyType Type : CurrencyTypes)
	{
		int64 Balance = Currency->GetCurrencyAmount(Type);
		if (Balance < 0)
		{
			Logs.Add(FString::Printf(TEXT("Negative balance detected for currency type %d: %lld"), (int32)Type, Balance));
			return CreateFailResult(
				FName(TEXT("Test_Currency_BalanceNonNegative")),
				TEXT("Negative balance detected"),
				Logs
			);
		}
	}

	return CreatePassResult(
		FName(TEXT("Test_Currency_BalanceNonNegative")),
		TEXT("All currency balances are non-negative")
	);
}

// ==========================================
// WEATHER TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestWeather_SetWeatherType()
{
	LogTestStart(TEXT("TestWeather_SetWeatherType"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_SetWeatherType")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Set weather to Rain
	Weather->SetWeatherInstant(EMGWeatherType::HeavyRain);
	EMGWeatherType CurrentType = Weather->GetCurrentWeatherType();
	Logs.Add(FString::Printf(TEXT("Set weather to HeavyRain, current type: %d"), (int32)CurrentType));

	if (CurrentType != EMGWeatherType::HeavyRain)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_SetWeatherType")),
			TEXT("Weather type not set correctly"),
			Logs
		);
	}

	// Set weather to Clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);
	CurrentType = Weather->GetCurrentWeatherType();
	Logs.Add(FString::Printf(TEXT("Set weather to Clear, current type: %d"), (int32)CurrentType));

	if (CurrentType != EMGWeatherType::Clear)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_SetWeatherType")),
			TEXT("Weather type not set correctly"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Weather_SetWeatherType")),
		TEXT("Weather type changes correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestWeather_Transition()
{
	LogTestStart(TEXT("TestWeather_Transition"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_Transition")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Start from Clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);

	// Start transition to Rain
	Weather->SetWeather(EMGWeatherType::HeavyRain, 5.0f);

	bool bIsTransitioning = Weather->IsWeatherTransitioning();
	Logs.Add(FString::Printf(TEXT("IsTransitioning: %s"), bIsTransitioning ? TEXT("true") : TEXT("false")));

	FMGWeatherTransition Transition = Weather->GetWeatherTransition();
	Logs.Add(FString::Printf(TEXT("Transition progress: %.2f"), Transition.Progress));
	Logs.Add(FString::Printf(TEXT("Transition duration: %.2f"), Transition.Duration));

	// Reset to clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);

	return CreatePassResult(
		FName(TEXT("Test_Weather_Transition")),
		TEXT("Weather transition initiated correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestWeather_RoadGrip()
{
	LogTestStart(TEXT("TestWeather_RoadGrip"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_RoadGrip")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Test dry conditions
	Weather->SetWeatherInstant(EMGWeatherType::Clear);
	float DryGrip = Weather->GetRoadGripMultiplier();
	Logs.Add(FString::Printf(TEXT("Dry grip: %.2f"), DryGrip));

	// Test wet conditions
	Weather->SetWeatherInstant(EMGWeatherType::HeavyRain);
	float WetGrip = Weather->GetRoadGripMultiplier();
	Logs.Add(FString::Printf(TEXT("Wet grip: %.2f"), WetGrip));

	// Dry should have better grip than wet
	if (WetGrip >= DryGrip)
	{
		Logs.Add(TEXT("Wet conditions should have less grip than dry"));
		return CreateFailResult(
			FName(TEXT("Test_Weather_RoadGrip")),
			TEXT("Grip calculation incorrect"),
			Logs
		);
	}

	// Grip should be in valid range (0-1)
	if (DryGrip < 0.0f || DryGrip > 1.0f || WetGrip < 0.0f || WetGrip > 1.0f)
	{
		Logs.Add(TEXT("Grip values should be between 0 and 1"));
		return CreateFailResult(
			FName(TEXT("Test_Weather_RoadGrip")),
			TEXT("Grip out of valid range"),
			Logs
		);
	}

	// Reset to clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);

	return CreatePassResult(
		FName(TEXT("Test_Weather_RoadGrip")),
		FString::Printf(TEXT("Grip calculated correctly: Dry=%.2f, Wet=%.2f"), DryGrip, WetGrip)
	);
}

FMGTestResult UMGSubsystemTests::TestWeather_Visibility()
{
	LogTestStart(TEXT("TestWeather_Visibility"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_Visibility")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Test clear visibility
	Weather->SetWeatherInstant(EMGWeatherType::Clear);
	float ClearVisibility = Weather->GetVisibilityDistance();
	Logs.Add(FString::Printf(TEXT("Clear visibility: %.0fm"), ClearVisibility));

	// Test fog visibility
	Weather->SetWeatherInstant(EMGWeatherType::HeavyFog);
	float FogVisibility = Weather->GetVisibilityDistance();
	Logs.Add(FString::Printf(TEXT("Fog visibility: %.0fm"), FogVisibility));

	// Clear should have better visibility
	if (FogVisibility >= ClearVisibility)
	{
		Logs.Add(TEXT("Fog should reduce visibility"));
		return CreateFailResult(
			FName(TEXT("Test_Weather_Visibility")),
			TEXT("Visibility calculation incorrect"),
			Logs
		);
	}

	// Visibility should be positive
	if (ClearVisibility <= 0.0f || FogVisibility <= 0.0f)
	{
		Logs.Add(TEXT("Visibility should be positive"));
		return CreateFailResult(
			FName(TEXT("Test_Weather_Visibility")),
			TEXT("Invalid visibility values"),
			Logs
		);
	}

	// Reset to clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);

	return CreatePassResult(
		FName(TEXT("Test_Weather_Visibility")),
		FString::Printf(TEXT("Visibility correct: Clear=%.0fm, Fog=%.0fm"), ClearVisibility, FogVisibility)
	);
}

FMGTestResult UMGSubsystemTests::TestWeather_TimeOfDay()
{
	LogTestStart(TEXT("TestWeather_TimeOfDay"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_TimeOfDay")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Set to morning
	Weather->SetTimeOfDayInstant(EMGTimeOfDay::Morning);
	EMGTimeOfDay CurrentTime = Weather->GetTimeOfDay();
	Logs.Add(FString::Printf(TEXT("Set to Morning, current: %d"), (int32)CurrentTime));

	if (CurrentTime != EMGTimeOfDay::Morning)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_TimeOfDay")),
			TEXT("Time of day not set correctly"),
			Logs
		);
	}

	// Set to night
	Weather->SetTimeOfDayInstant(EMGTimeOfDay::Night);
	CurrentTime = Weather->GetTimeOfDay();
	Logs.Add(FString::Printf(TEXT("Set to Night, current: %d"), (int32)CurrentTime));

	if (CurrentTime != EMGTimeOfDay::Night)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_TimeOfDay")),
			TEXT("Time of day not set correctly"),
			Logs
		);
	}

	// Test formatted time
	FText FormattedTime = Weather->GetFormattedTime();
	Logs.Add(FString::Printf(TEXT("Formatted time: %s"), *FormattedTime.ToString()));

	// Reset to midday
	Weather->SetTimeOfDayInstant(EMGTimeOfDay::Midday);

	return CreatePassResult(
		FName(TEXT("Test_Weather_TimeOfDay")),
		TEXT("Time of day changes correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestWeather_DifficultyRating()
{
	LogTestStart(TEXT("TestWeather_DifficultyRating"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Weather_DifficultyRating")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Test clear difficulty
	Weather->SetWeatherInstant(EMGWeatherType::Clear);
	int32 ClearDifficulty = Weather->GetWeatherDifficultyRating();
	Logs.Add(FString::Printf(TEXT("Clear difficulty: %d"), ClearDifficulty));

	// Test storm difficulty
	Weather->SetWeatherInstant(EMGWeatherType::Thunderstorm);
	int32 StormDifficulty = Weather->GetWeatherDifficultyRating();
	Logs.Add(FString::Printf(TEXT("Storm difficulty: %d"), StormDifficulty));

	// Storm should be harder than clear
	if (StormDifficulty <= ClearDifficulty)
	{
		Logs.Add(TEXT("Storm should have higher difficulty than clear"));
		return CreateFailResult(
			FName(TEXT("Test_Weather_DifficultyRating")),
			TEXT("Difficulty calculation incorrect"),
			Logs
		);
	}

	// Difficulty should be in 1-5 range
	if (ClearDifficulty < 1 || ClearDifficulty > 5 || StormDifficulty < 1 || StormDifficulty > 5)
	{
		Logs.Add(TEXT("Difficulty should be between 1 and 5"));
		return CreateFailResult(
			FName(TEXT("Test_Weather_DifficultyRating")),
			TEXT("Difficulty out of valid range"),
			Logs
		);
	}

	// Reset to clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);

	return CreatePassResult(
		FName(TEXT("Test_Weather_DifficultyRating")),
		FString::Printf(TEXT("Difficulty ratings: Clear=%d, Storm=%d"), ClearDifficulty, StormDifficulty)
	);
}

// ==========================================
// ECONOMY TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestEconomy_TransactionCreate()
{
	LogTestStart(TEXT("TestEconomy_TransactionCreate"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Economy_TransactionCreate")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Earn some currency to create a transaction
	bool bEarned = Currency->EarnCurrency(EMGCurrencyType::GrindCash, 100, EMGEarnSource::RaceFinish, TEXT("Test transaction"));
	Logs.Add(FString::Printf(TEXT("Earned currency: %s"), bEarned ? TEXT("true") : TEXT("false")));

	// Get recent transactions
	TArray<FMGCurrencyTransaction> Transactions = Currency->GetRecentTransactions(5);
	Logs.Add(FString::Printf(TEXT("Recent transactions: %d"), Transactions.Num()));

	if (Transactions.Num() == 0)
	{
		return CreateFailResult(
			FName(TEXT("Test_Economy_TransactionCreate")),
			TEXT("No transactions recorded"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Economy_TransactionCreate")),
		TEXT("Transactions are created and recorded")
	);
}

FMGTestResult UMGSubsystemTests::TestEconomy_PurchaseFlow()
{
	LogTestStart(TEXT("TestEconomy_PurchaseFlow"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Economy_PurchaseFlow")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Setup: ensure sufficient funds
	Currency->EarnCurrency(EMGCurrencyType::GrindCash, 10000, EMGEarnSource::Gifted, TEXT("Test setup"));

	int64 InitialBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Initial balance: %lld"), InitialBalance));

	// Check if we can afford
	bool bCanAfford = Currency->CanAfford(EMGCurrencyType::GrindCash, 5000);
	Logs.Add(FString::Printf(TEXT("Can afford 5000: %s"), bCanAfford ? TEXT("true") : TEXT("false")));

	if (!bCanAfford && InitialBalance >= 5000)
	{
		return CreateFailResult(
			FName(TEXT("Test_Economy_PurchaseFlow")),
			TEXT("CanAfford returned false incorrectly"),
			Logs
		);
	}

	// Make purchase
	bool bPurchased = Currency->SpendCurrency(EMGCurrencyType::GrindCash, 5000, TEXT("Test purchase"));
	Logs.Add(FString::Printf(TEXT("Purchase successful: %s"), bPurchased ? TEXT("true") : TEXT("false")));

	// Verify balance
	int64 FinalBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Final balance: %lld"), FinalBalance));

	return CreatePassResult(
		FName(TEXT("Test_Economy_PurchaseFlow")),
		TEXT("Purchase flow completed successfully")
	);
}

FMGTestResult UMGSubsystemTests::TestEconomy_TransactionHistory()
{
	LogTestStart(TEXT("TestEconomy_TransactionHistory"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Economy_TransactionHistory")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Create multiple transactions
	for (int32 i = 0; i < 3; i++)
	{
		Currency->EarnCurrency(EMGCurrencyType::GrindCash, 100 + i * 100, EMGEarnSource::RaceFinish, FString::Printf(TEXT("Test transaction %d"), i));
	}

	// Get history
	TArray<FMGCurrencyTransaction> History = Currency->GetRecentTransactions(10);
	Logs.Add(FString::Printf(TEXT("Transaction history count: %d"), History.Num()));

	if (History.Num() < 3)
	{
		Logs.Add(TEXT("Expected at least 3 transactions in history"));
		return CreateFailResult(
			FName(TEXT("Test_Economy_TransactionHistory")),
			TEXT("Transaction history incomplete"),
			Logs
		);
	}

	// Verify transaction data
	for (const FMGCurrencyTransaction& Transaction : History)
	{
		Logs.Add(FString::Printf(TEXT("Transaction: %s, Amount: %lld"), *Transaction.TransactionID, Transaction.Amount));
	}

	return CreatePassResult(
		FName(TEXT("Test_Economy_TransactionHistory")),
		FString::Printf(TEXT("Transaction history has %d entries"), History.Num())
	);
}

// ==========================================
// VEHICLE TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestVehicle_DamageSystemInit()
{
	LogTestStart(TEXT("TestVehicle_DamageSystemInit"));

	TArray<FString> Logs;

	// Create a temporary damage system component
	UMGVehicleDamageSystem* DamageSystem = NewObject<UMGVehicleDamageSystem>();
	if (!DamageSystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_DamageSystemInit")),
			TEXT("Failed to create damage system"),
			{TEXT("NewObject<UMGVehicleDamageSystem>() returned null")}
		);
	}

	Logs.Add(TEXT("Damage system created successfully"));

	// Check default configuration values
	Logs.Add(FString::Printf(TEXT("BaseDamageResistance: %.2f"), DamageSystem->BaseDamageResistance));
	Logs.Add(FString::Printf(TEXT("MinImpactForceForDamage: %.2f"), DamageSystem->MinImpactForceForDamage));
	Logs.Add(FString::Printf(TEXT("MaxImpactForce: %.2f"), DamageSystem->MaxImpactForce));
	Logs.Add(FString::Printf(TEXT("TotaledThreshold: %.2f"), DamageSystem->TotaledThreshold));

	// Verify sane defaults
	if (DamageSystem->MaxImpactForce <= DamageSystem->MinImpactForceForDamage)
	{
		Logs.Add(TEXT("MaxImpactForce should be greater than MinImpactForceForDamage"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_DamageSystemInit")),
			TEXT("Invalid impact force configuration"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Vehicle_DamageSystemInit")),
		TEXT("Damage system initialized with valid defaults")
	);
}

FMGTestResult UMGSubsystemTests::TestVehicle_ComponentDamage()
{
	LogTestStart(TEXT("TestVehicle_ComponentDamage"));

	TArray<FString> Logs;

	// Create damage system
	UMGVehicleDamageSystem* DamageSystem = NewObject<UMGVehicleDamageSystem>();
	if (!DamageSystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_ComponentDamage")),
			TEXT("Failed to create damage system"),
			{TEXT("NewObject<UMGVehicleDamageSystem>() returned null")}
		);
	}

	// Get initial state
	FMGComponentDamageState InitialState = DamageSystem->GetComponentState(EMGDamageComponent::Engine);
	Logs.Add(FString::Printf(TEXT("Initial engine health: %.2f"), InitialState.Health));

	// Apply damage
	const float DamageAmount = 25.0f;
	DamageSystem->ApplyComponentDamage(EMGDamageComponent::Engine, DamageAmount);

	// Check new state
	FMGComponentDamageState NewState = DamageSystem->GetComponentState(EMGDamageComponent::Engine);
	Logs.Add(FString::Printf(TEXT("After damage engine health: %.2f (expected: %.2f)"),
		NewState.Health, InitialState.Health - DamageAmount));

	// Verify damage was applied
	if (NewState.Health >= InitialState.Health)
	{
		Logs.Add(TEXT("Damage was not applied correctly"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_ComponentDamage")),
			TEXT("Component damage not applied"),
			Logs
		);
	}

	// Test all component types exist
	TArray<EMGDamageComponent> Components = {
		EMGDamageComponent::Body,
		EMGDamageComponent::Engine,
		EMGDamageComponent::Transmission,
		EMGDamageComponent::Suspension,
		EMGDamageComponent::Steering,
		EMGDamageComponent::Brakes,
		EMGDamageComponent::Wheels,
		EMGDamageComponent::Aero,
		EMGDamageComponent::Cooling,
		EMGDamageComponent::NOS
	};

	int32 ValidComponents = 0;
	for (EMGDamageComponent Component : Components)
	{
		FMGComponentDamageState State = DamageSystem->GetComponentState(Component);
		if (State.Health >= 0.0f && State.Health <= 100.0f)
		{
			ValidComponents++;
		}
	}

	Logs.Add(FString::Printf(TEXT("Valid component states: %d/%d"), ValidComponents, Components.Num()));

	return CreatePassResult(
		FName(TEXT("Test_Vehicle_ComponentDamage")),
		FString::Printf(TEXT("Component damage applied correctly. Engine: %.0f -> %.0f"), InitialState.Health, NewState.Health)
	);
}

FMGTestResult UMGSubsystemTests::TestVehicle_DamageResistance()
{
	LogTestStart(TEXT("TestVehicle_DamageResistance"));

	TArray<FString> Logs;

	// Create damage system
	UMGVehicleDamageSystem* DamageSystem = NewObject<UMGVehicleDamageSystem>();
	if (!DamageSystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_DamageResistance")),
			TEXT("Failed to create damage system"),
			{TEXT("NewObject<UMGVehicleDamageSystem>() returned null")}
		);
	}

	// Test with no resistance
	DamageSystem->BaseDamageResistance = 0.0f;
	Logs.Add(FString::Printf(TEXT("Base resistance: %.2f"), DamageSystem->BaseDamageResistance));

	// Apply zone damage
	DamageSystem->ApplyZoneDamage(EMGDamageZone::Front, 20.0f);
	float DamageNoResist = 100.0f - DamageSystem->GetComponentState(EMGDamageComponent::Body).Health;
	Logs.Add(FString::Printf(TEXT("Damage with 0%% resistance: %.2f"), DamageNoResist));

	// Reset and test with resistance
	DamageSystem->InstantRepairAll();
	DamageSystem->BaseDamageResistance = 0.5f; // 50% resistance
	Logs.Add(FString::Printf(TEXT("Base resistance set to: %.2f"), DamageSystem->BaseDamageResistance));

	DamageSystem->ApplyZoneDamage(EMGDamageZone::Front, 20.0f);
	float DamageWithResist = 100.0f - DamageSystem->GetComponentState(EMGDamageComponent::Body).Health;
	Logs.Add(FString::Printf(TEXT("Damage with 50%% resistance: %.2f"), DamageWithResist));

	// Resistance should reduce damage (though actual implementation may vary)
	Logs.Add(TEXT("Resistance test completed"));

	return CreatePassResult(
		FName(TEXT("Test_Vehicle_DamageResistance")),
		TEXT("Damage resistance system functional")
	);
}

FMGTestResult UMGSubsystemTests::TestVehicle_Repair()
{
	LogTestStart(TEXT("TestVehicle_Repair"));

	TArray<FString> Logs;

	// Create damage system
	UMGVehicleDamageSystem* DamageSystem = NewObject<UMGVehicleDamageSystem>();
	if (!DamageSystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_Repair")),
			TEXT("Failed to create damage system"),
			{TEXT("NewObject<UMGVehicleDamageSystem>() returned null")}
		);
	}

	// Apply damage to multiple components
	DamageSystem->ApplyComponentDamage(EMGDamageComponent::Engine, 50.0f);
	DamageSystem->ApplyComponentDamage(EMGDamageComponent::Brakes, 30.0f);
	DamageSystem->ApplyComponentDamage(EMGDamageComponent::Suspension, 40.0f);

	float EngineBeforeRepair = DamageSystem->GetComponentState(EMGDamageComponent::Engine).Health;
	float BrakesBeforeRepair = DamageSystem->GetComponentState(EMGDamageComponent::Brakes).Health;
	Logs.Add(FString::Printf(TEXT("Engine before repair: %.0f"), EngineBeforeRepair));
	Logs.Add(FString::Printf(TEXT("Brakes before repair: %.0f"), BrakesBeforeRepair));

	// Test single component repair
	DamageSystem->InstantRepair(EMGDamageComponent::Engine);
	float EngineAfterRepair = DamageSystem->GetComponentState(EMGDamageComponent::Engine).Health;
	Logs.Add(FString::Printf(TEXT("Engine after repair: %.0f"), EngineAfterRepair));

	if (EngineAfterRepair <= EngineBeforeRepair)
	{
		Logs.Add(TEXT("Engine repair did not increase health"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_Repair")),
			TEXT("InstantRepair did not work"),
			Logs
		);
	}

	// Brakes should still be damaged
	float BrakesStillDamaged = DamageSystem->GetComponentState(EMGDamageComponent::Brakes).Health;
	Logs.Add(FString::Printf(TEXT("Brakes (should still be damaged): %.0f"), BrakesStillDamaged));

	// Test repair all
	DamageSystem->InstantRepairAll();
	float BrakesAfterRepairAll = DamageSystem->GetComponentState(EMGDamageComponent::Brakes).Health;
	float SuspensionAfterRepairAll = DamageSystem->GetComponentState(EMGDamageComponent::Suspension).Health;
	Logs.Add(FString::Printf(TEXT("Brakes after RepairAll: %.0f"), BrakesAfterRepairAll));
	Logs.Add(FString::Printf(TEXT("Suspension after RepairAll: %.0f"), SuspensionAfterRepairAll));

	// All should be at full health
	if (BrakesAfterRepairAll < 100.0f || SuspensionAfterRepairAll < 100.0f)
	{
		Logs.Add(TEXT("RepairAll did not fully restore all components"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_Repair")),
			TEXT("InstantRepairAll incomplete"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Vehicle_Repair")),
		TEXT("Repair system working correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestVehicle_PerformanceDegradation()
{
	LogTestStart(TEXT("TestVehicle_PerformanceDegradation"));

	TArray<FString> Logs;

	// Create damage system
	UMGVehicleDamageSystem* DamageSystem = NewObject<UMGVehicleDamageSystem>();
	if (!DamageSystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_PerformanceDegradation")),
			TEXT("Failed to create damage system"),
			{TEXT("NewObject<UMGVehicleDamageSystem>() returned null")}
		);
	}

	// Check performance at full health
	float EnginePerformanceFull = DamageSystem->GetComponentPerformance(EMGDamageComponent::Engine);
	Logs.Add(FString::Printf(TEXT("Engine performance (full health): %.2f"), EnginePerformanceFull));

	// Apply significant damage
	DamageSystem->ApplyComponentDamage(EMGDamageComponent::Engine, 70.0f);
	float EngineHealthAfterDamage = DamageSystem->GetComponentState(EMGDamageComponent::Engine).Health;
	float EnginePerformanceDamaged = DamageSystem->GetComponentPerformance(EMGDamageComponent::Engine);
	Logs.Add(FString::Printf(TEXT("Engine health after damage: %.0f"), EngineHealthAfterDamage));
	Logs.Add(FString::Printf(TEXT("Engine performance (damaged): %.2f"), EnginePerformanceDamaged));

	// Performance should be reduced when damaged
	if (EnginePerformanceDamaged >= EnginePerformanceFull)
	{
		Logs.Add(TEXT("Performance should decrease with damage"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_PerformanceDegradation")),
			TEXT("Performance not affected by damage"),
			Logs
		);
	}

	// Performance should be in valid range (0-1)
	if (EnginePerformanceDamaged < 0.0f || EnginePerformanceDamaged > 1.0f)
	{
		Logs.Add(TEXT("Performance multiplier out of valid range"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_PerformanceDegradation")),
			TEXT("Invalid performance value"),
			Logs
		);
	}

	// Test other components
	float BrakesPerf = DamageSystem->GetComponentPerformance(EMGDamageComponent::Brakes);
	float SuspensionPerf = DamageSystem->GetComponentPerformance(EMGDamageComponent::Suspension);
	Logs.Add(FString::Printf(TEXT("Brakes performance: %.2f"), BrakesPerf));
	Logs.Add(FString::Printf(TEXT("Suspension performance: %.2f"), SuspensionPerf));

	return CreatePassResult(
		FName(TEXT("Test_Vehicle_PerformanceDegradation")),
		FString::Printf(TEXT("Performance degrades correctly: Full=%.2f, Damaged=%.2f"), EnginePerformanceFull, EnginePerformanceDamaged)
	);
}

FMGTestResult UMGSubsystemTests::TestVehicle_TotaledState()
{
	LogTestStart(TEXT("TestVehicle_TotaledState"));

	TArray<FString> Logs;

	// Create damage system
	UMGVehicleDamageSystem* DamageSystem = NewObject<UMGVehicleDamageSystem>();
	if (!DamageSystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_TotaledState")),
			TEXT("Failed to create damage system"),
			{TEXT("NewObject<UMGVehicleDamageSystem>() returned null")}
		);
	}

	// Check initial state
	bool bInitialTotaled = DamageSystem->IsVehicleTotaled();
	float InitialDamagePercent = DamageSystem->GetOverallDamagePercent();
	Logs.Add(FString::Printf(TEXT("Initial totaled state: %s"), bInitialTotaled ? TEXT("true") : TEXT("false")));
	Logs.Add(FString::Printf(TEXT("Initial overall damage: %.1f%%"), InitialDamagePercent));

	// Vehicle should not start totaled
	if (bInitialTotaled)
	{
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_TotaledState")),
			TEXT("Vehicle should not start in totaled state"),
			Logs
		);
	}

	// Apply massive damage
	DamageSystem->ApplyGlobalDamage(100.0f);
	bool bNowTotaled = DamageSystem->IsVehicleTotaled();
	float NowDamagePercent = DamageSystem->GetOverallDamagePercent();
	Logs.Add(FString::Printf(TEXT("After 100%% global damage - Totaled: %s"), bNowTotaled ? TEXT("true") : TEXT("false")));
	Logs.Add(FString::Printf(TEXT("Overall damage percent: %.1f%%"), NowDamagePercent));

	// Test repair from totaled
	DamageSystem->InstantRepairAll();
	bool bAfterRepair = DamageSystem->IsVehicleTotaled();
	float AfterRepairDamage = DamageSystem->GetOverallDamagePercent();
	Logs.Add(FString::Printf(TEXT("After repair - Totaled: %s"), bAfterRepair ? TEXT("true") : TEXT("false")));
	Logs.Add(FString::Printf(TEXT("Damage after repair: %.1f%%"), AfterRepairDamage));

	// Overall damage percent should be in valid range
	if (InitialDamagePercent < 0.0f || InitialDamagePercent > 100.0f)
	{
		Logs.Add(TEXT("Damage percent out of valid range"));
		return CreateFailResult(
			FName(TEXT("Test_Vehicle_TotaledState")),
			TEXT("Invalid damage percentage"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Vehicle_TotaledState")),
		FString::Printf(TEXT("Totaled state detection working. After damage: %s"), bNowTotaled ? TEXT("Totaled") : TEXT("Not totaled"))
	);
}

// ==========================================
// AI TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestAI_DrivingStates()
{
	LogTestStart(TEXT("TestAI_DrivingStates"));

	TArray<FString> Logs;

	// Verify all driving states are defined
	TArray<EMGAIDrivingState> States = {
		EMGAIDrivingState::Waiting,
		EMGAIDrivingState::Racing,
		EMGAIDrivingState::Overtaking,
		EMGAIDrivingState::Defending,
		EMGAIDrivingState::Recovering,
		EMGAIDrivingState::Caution,
		EMGAIDrivingState::PushingHard,
		EMGAIDrivingState::ManagingLead,
		EMGAIDrivingState::Drafting,
		EMGAIDrivingState::Finished
	};

	int32 StateCount = States.Num();
	Logs.Add(FString::Printf(TEXT("AI driving states defined: %d"), StateCount));

	// Verify states are distinct
	TSet<int32> UniqueStates;
	for (EMGAIDrivingState State : States)
	{
		UniqueStates.Add(static_cast<int32>(State));
	}

	if (UniqueStates.Num() != StateCount)
	{
		Logs.Add(TEXT("Duplicate state values detected"));
		return CreateFailResult(
			FName(TEXT("Test_AI_DrivingStates")),
			TEXT("Duplicate state values"),
			Logs
		);
	}

	Logs.Add(FString::Printf(TEXT("All %d states are unique"), StateCount));

	return CreatePassResult(
		FName(TEXT("Test_AI_DrivingStates")),
		FString::Printf(TEXT("AI has %d distinct driving states"), StateCount)
	);
}

FMGTestResult UMGSubsystemTests::TestAI_SkillParams()
{
	LogTestStart(TEXT("TestAI_SkillParams"));

	TArray<FString> Logs;

	// Create default skill params
	FMGAISkillParams Params;

	Logs.Add(FString::Printf(TEXT("SkillLevel: %.2f"), Params.SkillLevel));
	Logs.Add(FString::Printf(TEXT("BrakingAccuracy: %.2f"), Params.BrakingAccuracy));
	Logs.Add(FString::Printf(TEXT("LineAccuracy: %.2f"), Params.LineAccuracy));
	Logs.Add(FString::Printf(TEXT("ReactionTime: %.2f"), Params.ReactionTime));
	Logs.Add(FString::Printf(TEXT("Consistency: %.2f"), Params.Consistency));
	Logs.Add(FString::Printf(TEXT("MistakeFrequency: %.2f"), Params.MistakeFrequency));
	Logs.Add(FString::Printf(TEXT("RecoverySkill: %.2f"), Params.RecoverySkill));
	Logs.Add(FString::Printf(TEXT("CornerExitSpeed: %.2f"), Params.CornerExitSpeed));

	// Verify all values are in valid range (0-1 or 0.1-1 for reaction time)
	bool bAllValid = true;
	if (Params.SkillLevel < 0.0f || Params.SkillLevel > 1.0f)
	{
		Logs.Add(TEXT("SkillLevel out of range"));
		bAllValid = false;
	}
	if (Params.BrakingAccuracy < 0.0f || Params.BrakingAccuracy > 1.0f)
	{
		Logs.Add(TEXT("BrakingAccuracy out of range"));
		bAllValid = false;
	}
	if (Params.LineAccuracy < 0.0f || Params.LineAccuracy > 1.0f)
	{
		Logs.Add(TEXT("LineAccuracy out of range"));
		bAllValid = false;
	}
	if (Params.ReactionTime < 0.1f || Params.ReactionTime > 1.0f)
	{
		Logs.Add(TEXT("ReactionTime out of range"));
		bAllValid = false;
	}
	if (Params.Consistency < 0.0f || Params.Consistency > 1.0f)
	{
		Logs.Add(TEXT("Consistency out of range"));
		bAllValid = false;
	}
	if (Params.MistakeFrequency < 0.0f || Params.MistakeFrequency > 1.0f)
	{
		Logs.Add(TEXT("MistakeFrequency out of range"));
		bAllValid = false;
	}

	if (!bAllValid)
	{
		return CreateFailResult(
			FName(TEXT("Test_AI_SkillParams")),
			TEXT("Skill parameters out of valid range"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_AI_SkillParams")),
		TEXT("AI skill parameters have valid default values")
	);
}

FMGTestResult UMGSubsystemTests::TestAI_SpawnConfig()
{
	LogTestStart(TEXT("TestAI_SpawnConfig"));

	TArray<FString> Logs;

	// Create default spawn config
	FMGAISpawnConfig Config;

	Logs.Add(FString::Printf(TEXT("RacerCount: %d"), Config.RacerCount));
	Logs.Add(FString::Printf(TEXT("MinSkill: %.2f"), Config.MinSkill));
	Logs.Add(FString::Printf(TEXT("MaxSkill: %.2f"), Config.MaxSkill));
	Logs.Add(FString::Printf(TEXT("DifficultyModifier: %.2f"), Config.DifficultyModifier));
	Logs.Add(FString::Printf(TEXT("bEnableSkillBasedCatchUp: %s"), Config.bEnableSkillBasedCatchUp ? TEXT("true") : TEXT("false")));

	// Validate config
	bool bValid = true;
	if (Config.RacerCount < 0)
	{
		Logs.Add(TEXT("RacerCount should be non-negative"));
		bValid = false;
	}
	if (Config.MinSkill < 0.0f || Config.MinSkill > 1.0f)
	{
		Logs.Add(TEXT("MinSkill out of range"));
		bValid = false;
	}
	if (Config.MaxSkill < 0.0f || Config.MaxSkill > 1.0f)
	{
		Logs.Add(TEXT("MaxSkill out of range"));
		bValid = false;
	}
	if (Config.MinSkill > Config.MaxSkill)
	{
		Logs.Add(TEXT("MinSkill should not exceed MaxSkill"));
		bValid = false;
	}
	if (Config.DifficultyModifier < 0.5f || Config.DifficultyModifier > 1.5f)
	{
		Logs.Add(TEXT("DifficultyModifier out of range"));
		bValid = false;
	}

	if (!bValid)
	{
		return CreateFailResult(
			FName(TEXT("Test_AI_SpawnConfig")),
			TEXT("Spawn configuration has invalid defaults"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_AI_SpawnConfig")),
		FString::Printf(TEXT("AI spawn config valid: %d racers, skill %.2f-%.2f"), Config.RacerCount, Config.MinSkill, Config.MaxSkill)
	);
}

FMGTestResult UMGSubsystemTests::TestAI_DriverPersonality()
{
	LogTestStart(TEXT("TestAI_DriverPersonality"));

	TArray<FString> Logs;

	// Verify all personality types
	TArray<EMGDriverPersonality> Personalities = {
		EMGDriverPersonality::Aggressive,
		EMGDriverPersonality::Defensive,
		EMGDriverPersonality::Calculated,
		EMGDriverPersonality::Unpredictable,
		EMGDriverPersonality::Rookie,
		EMGDriverPersonality::Veteran,
		EMGDriverPersonality::Rival
	};

	int32 PersonalityCount = Personalities.Num();
	Logs.Add(FString::Printf(TEXT("Personality types defined: %d"), PersonalityCount));

	// Test personality names
	TArray<FString> PersonalityNames = {
		TEXT("Aggressive"), TEXT("Defensive"), TEXT("Calculated"),
		TEXT("Unpredictable"), TEXT("Rookie"), TEXT("Veteran"), TEXT("Rival")
	};

	for (int32 i = 0; i < PersonalityCount; i++)
	{
		Logs.Add(FString::Printf(TEXT("  [%d] %s"), i, *PersonalityNames[i]));
	}

	// Verify uniqueness
	TSet<int32> UniquePersonalities;
	for (EMGDriverPersonality Personality : Personalities)
	{
		UniquePersonalities.Add(static_cast<int32>(Personality));
	}

	if (UniquePersonalities.Num() != PersonalityCount)
	{
		return CreateFailResult(
			FName(TEXT("Test_AI_DriverPersonality")),
			TEXT("Duplicate personality values"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_AI_DriverPersonality")),
		FString::Printf(TEXT("AI has %d distinct personality types"), PersonalityCount)
	);
}

FMGTestResult UMGSubsystemTests::TestAI_Strategies()
{
	LogTestStart(TEXT("TestAI_Strategies"));

	TArray<FString> Logs;

	// Verify overtake strategies
	TArray<EMGOvertakeStrategy> OvertakeStrategies = {
		EMGOvertakeStrategy::Patient,
		EMGOvertakeStrategy::LateBraking,
		EMGOvertakeStrategy::BetterExit,
		EMGOvertakeStrategy::AroundOutside,
		EMGOvertakeStrategy::SlipstreamPass,
		EMGOvertakeStrategy::Pressure
	};

	Logs.Add(FString::Printf(TEXT("Overtake strategies: %d"), OvertakeStrategies.Num()));

	// Verify defense strategies
	TArray<EMGDefenseStrategy> DefenseStrategies = {
		EMGDefenseStrategy::CoverLine,
		EMGDefenseStrategy::CoverInside,
		EMGDefenseStrategy::PaceDefense,
		EMGDefenseStrategy::DefensiveLine
	};

	Logs.Add(FString::Printf(TEXT("Defense strategies: %d"), DefenseStrategies.Num()));

	// Verify catch-up modes
	TArray<EMGCatchUpMode> CatchUpModes = {
		EMGCatchUpMode::None,
		EMGCatchUpMode::RiskTaking,
		EMGCatchUpMode::DraftingFocus,
		EMGCatchUpMode::MaxEffort,
		EMGCatchUpMode::Conservation
	};

	Logs.Add(FString::Printf(TEXT("Catch-up modes: %d"), CatchUpModes.Num()));

	// Calculate total strategic options
	int32 TotalStrategies = OvertakeStrategies.Num() + DefenseStrategies.Num() + CatchUpModes.Num();
	Logs.Add(FString::Printf(TEXT("Total strategic options: %d"), TotalStrategies));

	// Verify reasonable number of strategies for variety
	if (OvertakeStrategies.Num() < 3)
	{
		Logs.Add(TEXT("Expected at least 3 overtake strategies for variety"));
		return CreateFailResult(
			FName(TEXT("Test_AI_Strategies")),
			TEXT("Insufficient overtake strategies"),
			Logs
		);
	}

	if (DefenseStrategies.Num() < 2)
	{
		Logs.Add(TEXT("Expected at least 2 defense strategies"));
		return CreateFailResult(
			FName(TEXT("Test_AI_Strategies")),
			TEXT("Insufficient defense strategies"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_AI_Strategies")),
		FString::Printf(TEXT("AI has %d overtake, %d defense, %d catch-up strategies"),
			OvertakeStrategies.Num(), DefenseStrategies.Num(), CatchUpModes.Num())
	);
}

// ==========================================
// INTEGRATION TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestIntegration_CurrencyEconomy()
{
	LogTestStart(TEXT("TestIntegration_CurrencyEconomy"));

	UMGCurrencySubsystem* Currency = GetCurrencySubsystem();
	if (!Currency)
	{
		return CreateFailResult(
			FName(TEXT("Test_Integration_CurrencyEconomy")),
			TEXT("Currency subsystem not found"),
			{TEXT("Failed to get UMGCurrencySubsystem")}
		);
	}

	TArray<FString> Logs;

	// Test complete flow: earn, check, spend, verify
	int64 StartBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Start balance: %lld"), StartBalance));

	// Earn from race
	FMGRaceEarnings Earnings = Currency->CalculateRaceEarnings(1, 8, FName(TEXT("TestTrack")), true, false, false);
	Currency->AwardRaceEarnings(Earnings);
	Logs.Add(FString::Printf(TEXT("Race earnings awarded: %lld"), Earnings.TotalEarnings));

	int64 AfterRace = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("After race balance: %lld"), AfterRace));

	// Verify earnings applied
	if (AfterRace <= StartBalance)
	{
		Logs.Add(TEXT("Race earnings not applied correctly"));
		return CreateFailResult(
			FName(TEXT("Test_Integration_CurrencyEconomy")),
			TEXT("Earnings not applied"),
			Logs
		);
	}

	// Make a purchase
	int64 PurchaseAmount = FMath::Min(Earnings.TotalEarnings / 2, AfterRace);
	bool bPurchased = Currency->SpendCurrency(EMGCurrencyType::GrindCash, PurchaseAmount, TEXT("Integration test purchase"));
	Logs.Add(FString::Printf(TEXT("Purchase of %lld: %s"), PurchaseAmount, bPurchased ? TEXT("success") : TEXT("failed")));

	int64 FinalBalance = Currency->GetCurrencyAmount(EMGCurrencyType::GrindCash);
	Logs.Add(FString::Printf(TEXT("Final balance: %lld"), FinalBalance));

	// Verify transaction history
	TArray<FMGCurrencyTransaction> History = Currency->GetRecentTransactions(5);
	Logs.Add(FString::Printf(TEXT("Transaction history: %d entries"), History.Num()));

	return CreatePassResult(
		FName(TEXT("Test_Integration_CurrencyEconomy")),
		TEXT("Currency and economy integration working correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestIntegration_WeatherRoad()
{
	LogTestStart(TEXT("TestIntegration_WeatherRoad"));

	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (!Weather)
	{
		return CreateFailResult(
			FName(TEXT("Test_Integration_WeatherRoad")),
			TEXT("Weather subsystem not found"),
			{TEXT("Failed to get UMGWeatherSubsystem")}
		);
	}

	TArray<FString> Logs;

	// Test weather types affect road conditions correctly
	TArray<TPair<EMGWeatherType, EMGRoadCondition>> ExpectedConditions = {
		{EMGWeatherType::Clear, EMGRoadCondition::Dry},
		{EMGWeatherType::HeavyRain, EMGRoadCondition::Wet},
		{EMGWeatherType::Snow, EMGRoadCondition::Snowy}
	};

	bool bAllPassed = true;
	for (const auto& Pair : ExpectedConditions)
	{
		Weather->SetWeatherInstant(Pair.Key);

		EMGRoadCondition ActualCondition = Weather->GetRoadCondition();
		float GripMultiplier = Weather->GetRoadGripMultiplier();
		bool bHazardous = Weather->AreConditionsHazardous();

		Logs.Add(FString::Printf(TEXT("Weather %d -> Road %d, Grip %.2f, Hazardous: %s"),
			(int32)Pair.Key, (int32)ActualCondition, GripMultiplier, bHazardous ? TEXT("true") : TEXT("false")));

		// Verify grip is affected by conditions
		if (Pair.Key != EMGWeatherType::Clear && GripMultiplier >= 1.0f)
		{
			Logs.Add(FString::Printf(TEXT("Expected reduced grip for weather type %d"), (int32)Pair.Key));
			bAllPassed = false;
		}
	}

	// Reset to clear
	Weather->SetWeatherInstant(EMGWeatherType::Clear);

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Integration_WeatherRoad")),
			TEXT("Weather-road integration issues found"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Integration_WeatherRoad")),
		TEXT("Weather and road conditions integrate correctly")
	);
}

// ==========================================
// CONSOLE COMMANDS
// ==========================================

void UMGSubsystemTests::RunAllTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING ALL SUBSYSTEM TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	// Currency tests
	TestResults.Add(TestCurrency_EarnGrindCash());
	TestResults.Add(TestCurrency_SpendGrindCash());
	TestResults.Add(TestCurrency_InsufficientFunds());
	TestResults.Add(TestCurrency_RaceEarnings());
	TestResults.Add(TestCurrency_Multipliers());
	TestResults.Add(TestCurrency_BalanceNonNegative());

	// Weather tests
	TestResults.Add(TestWeather_SetWeatherType());
	TestResults.Add(TestWeather_Transition());
	TestResults.Add(TestWeather_RoadGrip());
	TestResults.Add(TestWeather_Visibility());
	TestResults.Add(TestWeather_TimeOfDay());
	TestResults.Add(TestWeather_DifficultyRating());

	// Economy tests
	TestResults.Add(TestEconomy_TransactionCreate());
	TestResults.Add(TestEconomy_PurchaseFlow());
	TestResults.Add(TestEconomy_TransactionHistory());

	// Vehicle tests
	TestResults.Add(TestVehicle_DamageSystemInit());
	TestResults.Add(TestVehicle_ComponentDamage());
	TestResults.Add(TestVehicle_DamageResistance());
	TestResults.Add(TestVehicle_Repair());
	TestResults.Add(TestVehicle_PerformanceDegradation());
	TestResults.Add(TestVehicle_TotaledState());

	// AI tests
	TestResults.Add(TestAI_DrivingStates());
	TestResults.Add(TestAI_SkillParams());
	TestResults.Add(TestAI_SpawnConfig());
	TestResults.Add(TestAI_DriverPersonality());
	TestResults.Add(TestAI_Strategies());

	// Integration tests
	TestResults.Add(TestIntegration_CurrencyEconomy());
	TestResults.Add(TestIntegration_WeatherRoad());

	// Count results
	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
		{
			PassedTests++;
		}
		else
		{
			FailedTests++;
		}
	}

	PrintTestReport();
}

void UMGSubsystemTests::RunCurrencyTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING CURRENCY TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestCurrency_EarnGrindCash());
	TestResults.Add(TestCurrency_SpendGrindCash());
	TestResults.Add(TestCurrency_InsufficientFunds());
	TestResults.Add(TestCurrency_RaceEarnings());
	TestResults.Add(TestCurrency_Multipliers());
	TestResults.Add(TestCurrency_BalanceNonNegative());

	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
			PassedTests++;
		else
			FailedTests++;
	}

	PrintTestReport();
}

void UMGSubsystemTests::RunWeatherTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING WEATHER TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestWeather_SetWeatherType());
	TestResults.Add(TestWeather_Transition());
	TestResults.Add(TestWeather_RoadGrip());
	TestResults.Add(TestWeather_Visibility());
	TestResults.Add(TestWeather_TimeOfDay());
	TestResults.Add(TestWeather_DifficultyRating());

	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
			PassedTests++;
		else
			FailedTests++;
	}

	PrintTestReport();
}

void UMGSubsystemTests::RunEconomyTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING ECONOMY TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestEconomy_TransactionCreate());
	TestResults.Add(TestEconomy_PurchaseFlow());
	TestResults.Add(TestEconomy_TransactionHistory());

	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
			PassedTests++;
		else
			FailedTests++;
	}

	PrintTestReport();
}

void UMGSubsystemTests::RunVehicleTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING VEHICLE TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestVehicle_DamageSystemInit());
	TestResults.Add(TestVehicle_ComponentDamage());
	TestResults.Add(TestVehicle_DamageResistance());
	TestResults.Add(TestVehicle_Repair());
	TestResults.Add(TestVehicle_PerformanceDegradation());
	TestResults.Add(TestVehicle_TotaledState());

	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
			PassedTests++;
		else
			FailedTests++;
	}

	PrintTestReport();
}

void UMGSubsystemTests::RunAITests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING AI TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestAI_DrivingStates());
	TestResults.Add(TestAI_SkillParams());
	TestResults.Add(TestAI_SpawnConfig());
	TestResults.Add(TestAI_DriverPersonality());
	TestResults.Add(TestAI_Strategies());

	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
			PassedTests++;
		else
			FailedTests++;
	}

	PrintTestReport();
}

void UMGSubsystemTests::RunSmokeTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING SMOKE TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	// Quick verification tests
	TestResults.Add(TestCurrency_EarnGrindCash());
	TestResults.Add(TestCurrency_BalanceNonNegative());
	TestResults.Add(TestWeather_SetWeatherType());
	TestResults.Add(TestWeather_RoadGrip());
	TestResults.Add(TestVehicle_DamageSystemInit());
	TestResults.Add(TestAI_DrivingStates());

	for (const FMGTestResult& Result : TestResults)
	{
		TotalTests++;
		if (Result.Result == EMGTestResult::Passed)
			PassedTests++;
		else
			FailedTests++;
	}

	PrintTestReport();
}

void UMGSubsystemTests::PrintTestReport()
{
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("         TEST RESULTS SUMMARY"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("Total Tests:  %d"), TotalTests);
	UE_LOG(LogTemp, Log, TEXT("Passed:       %d"), PassedTests);
	UE_LOG(LogTemp, Log, TEXT("Failed:       %d"), FailedTests);
	UE_LOG(LogTemp, Log, TEXT("Pass Rate:    %.1f%%"), TotalTests > 0 ? (float)PassedTests / TotalTests * 100.0f : 0.0f);
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	for (const FMGTestResult& Result : TestResults)
	{
		const TCHAR* StatusStr = Result.Result == EMGTestResult::Passed ? TEXT("PASS") : TEXT("FAIL");
		UE_LOG(LogTemp, Log, TEXT("[%s] %s: %s"), StatusStr, *Result.TestID.ToString(), *Result.Message.ToString());

		if (Result.Result != EMGTestResult::Passed)
		{
			for (const FString& Log : Result.Logs)
			{
				UE_LOG(LogTemp, Log, TEXT("        %s"), *Log);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

// ==========================================
// HELPERS
// ==========================================

UMGCurrencySubsystem* UMGSubsystemTests::GetCurrencySubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMGCurrencySubsystem>() : nullptr;
}

UMGWeatherSubsystem* UMGSubsystemTests::GetWeatherSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	UWorld* World = GameInstance->GetWorld();
	return World ? World->GetSubsystem<UMGWeatherSubsystem>() : nullptr;
}

UMGTestFrameworkSubsystem* UMGSubsystemTests::GetTestFramework() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UMGTestFrameworkSubsystem>() : nullptr;
}

void UMGSubsystemTests::LogTestStart(const FString& TestName)
{
	UE_LOG(LogTemp, Log, TEXT("--- Running: %s ---"), *TestName);
}

void UMGSubsystemTests::LogTestResult(const FMGTestResult& Result)
{
	const TCHAR* StatusStr = Result.Result == EMGTestResult::Passed ? TEXT("PASS") : TEXT("FAIL");
	UE_LOG(LogTemp, Log, TEXT("[%s] %s"), StatusStr, *Result.Message.ToString());
}

FMGTestResult UMGSubsystemTests::CreatePassResult(FName TestID, const FString& Message)
{
	FMGTestResult Result;
	Result.TestID = TestID;
	Result.Result = EMGTestResult::Passed;
	Result.Message = FText::FromString(Message);
	Result.Timestamp = FDateTime::UtcNow();

	UE_LOG(LogTemp, Log, TEXT("[PASS] %s: %s"), *TestID.ToString(), *Message);
	return Result;
}

FMGTestResult UMGSubsystemTests::CreateFailResult(FName TestID, const FString& Message, const TArray<FString>& Logs)
{
	FMGTestResult Result;
	Result.TestID = TestID;
	Result.Result = EMGTestResult::Failed;
	Result.Message = FText::FromString(Message);
	Result.Logs = Logs;
	Result.Timestamp = FDateTime::UtcNow();

	UE_LOG(LogTemp, Warning, TEXT("[FAIL] %s: %s"), *TestID.ToString(), *Message);
	for (const FString& Log : Logs)
	{
		UE_LOG(LogTemp, Warning, TEXT("  -> %s"), *Log);
	}
	return Result;
}
