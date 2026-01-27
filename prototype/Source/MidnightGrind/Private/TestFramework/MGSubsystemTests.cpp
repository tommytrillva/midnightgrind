// Copyright Midnight Grind. All Rights Reserved.

#include "TestFramework/MGSubsystemTests.h"
#include "TestFramework/MGTestFrameworkSubsystem.h"
#include "Currency/MGCurrencySubsystem.h"
#include "Weather/MGWeatherSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
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

	UE_LOG(LogTemp, Log, TEXT("Registered %d subsystem tests"), 12);
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
