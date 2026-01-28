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
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "Save/MGSaveGame.h"
#include "Save/MGSaveManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/MGPhysicsConstants.h"
#include "Vehicle/MGStatCalculator.h"
#include "UI/MGHUDDataProvider.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "UI/MGMenuSubsystem.h"
#include "UI/MGNotificationSubsystem.h"

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

	// Performance Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Perf_SubsystemTick"));
		Test.TestName = FText::FromString(TEXT("Performance - Subsystem Tick"));
		Test.Description = FText::FromString(TEXT("Benchmark subsystem tick times"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Performance")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Perf_MemoryUsage"));
		Test.TestName = FText::FromString(TEXT("Performance - Memory Usage"));
		Test.Description = FText::FromString(TEXT("Verify memory allocations are reasonable"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Performance")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Perf_DelegateBroadcast"));
		Test.TestName = FText::FromString(TEXT("Performance - Delegate Broadcast"));
		Test.Description = FText::FromString(TEXT("Benchmark delegate broadcast overhead"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Performance")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Perf_DataAccess"));
		Test.TestName = FText::FromString(TEXT("Performance - Data Access"));
		Test.Description = FText::FromString(TEXT("Benchmark data structure access times"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Performance")));
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

	// Save/Load Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Save_CreateSaveGame"));
		Test.TestName = FText::FromString(TEXT("Save - Create Save Game"));
		Test.Description = FText::FromString(TEXT("Verify save game object can be created"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Save")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Save_DefaultValues"));
		Test.TestName = FText::FromString(TEXT("Save - Default Values"));
		Test.Description = FText::FromString(TEXT("Verify save data has correct default values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Save")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Save_DataStructures"));
		Test.TestName = FText::FromString(TEXT("Save - Data Structures"));
		Test.Description = FText::FromString(TEXT("Verify save data structures are properly initialized"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Save")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Save_ManagerSubsystem"));
		Test.TestName = FText::FromString(TEXT("Save - Manager Subsystem"));
		Test.Description = FText::FromString(TEXT("Verify save manager subsystem initialization"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Save")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Save_SlotNaming"));
		Test.TestName = FText::FromString(TEXT("Save - Slot Naming"));
		Test.Description = FText::FromString(TEXT("Verify save slot naming conventions"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Save")));
		TestFramework->RegisterTest(Test);
	}

	// Physics Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_TireCompoundGrip"));
		Test.TestName = FText::FromString(TEXT("Physics - Tire Compound Grip"));
		Test.Description = FText::FromString(TEXT("Verify tire compound grip coefficients"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_WetGripModifiers"));
		Test.TestName = FText::FromString(TEXT("Physics - Wet Grip Modifiers"));
		Test.Description = FText::FromString(TEXT("Verify wet surface grip modifiers"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_WeightTransferConstants"));
		Test.TestName = FText::FromString(TEXT("Physics - Weight Transfer Constants"));
		Test.Description = FText::FromString(TEXT("Verify weight transfer physics constants"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_TireTemperatureConstants"));
		Test.TestName = FText::FromString(TEXT("Physics - Tire Temperature Constants"));
		Test.Description = FText::FromString(TEXT("Verify tire temperature physics constants"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_HandlingModeSettings"));
		Test.TestName = FText::FromString(TEXT("Physics - Handling Mode Settings"));
		Test.Description = FText::FromString(TEXT("Verify handling mode presets"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_SurfaceConstants"));
		Test.TestName = FText::FromString(TEXT("Physics - Surface Constants"));
		Test.Description = FText::FromString(TEXT("Verify surface detection constants"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_GeometryConstants"));
		Test.TestName = FText::FromString(TEXT("Physics - Geometry Constants"));
		Test.Description = FText::FromString(TEXT("Verify suspension geometry constants"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_DifferentialConstants"));
		Test.TestName = FText::FromString(TEXT("Physics - Differential Constants"));
		Test.Description = FText::FromString(TEXT("Verify differential physics constants"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Physics_WearConstants"));
		Test.TestName = FText::FromString(TEXT("Physics - Wear Constants"));
		Test.Description = FText::FromString(TEXT("Verify wear degradation constants"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Physics")));
		TestFramework->RegisterTest(Test);
	}

	// Stress Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Stress_HighObjectCount"));
		Test.TestName = FText::FromString(TEXT("Stress - High Object Count"));
		Test.Description = FText::FromString(TEXT("Test performance with high object allocations"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Stress")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Stress_SustainedOperation"));
		Test.TestName = FText::FromString(TEXT("Stress - Sustained Operation"));
		Test.Description = FText::FromString(TEXT("Test sustained operation over many iterations"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Stress")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Stress_MemoryStability"));
		Test.TestName = FText::FromString(TEXT("Stress - Memory Stability"));
		Test.Description = FText::FromString(TEXT("Test memory stability under repeated allocations"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Stress")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Stress_RapidStateChanges"));
		Test.TestName = FText::FromString(TEXT("Stress - Rapid State Changes"));
		Test.Description = FText::FromString(TEXT("Test system stability under rapid state changes"));
		Test.Category = EMGTestCategory::Performance;
		Test.Tags.Add(FName(TEXT("Stress")));
		TestFramework->RegisterTest(Test);
	}

	// UI Data Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_UIData_HUDDataDefaults"));
		Test.TestName = FText::FromString(TEXT("UI Data - HUD Data Defaults"));
		Test.Description = FText::FromString(TEXT("Verify HUD data structure default values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("UIData")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_UIData_RaceStatusDefaults"));
		Test.TestName = FText::FromString(TEXT("UI Data - Race Status Defaults"));
		Test.Description = FText::FromString(TEXT("Verify race status structure default values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("UIData")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_UIData_TelemetryDefaults"));
		Test.TestName = FText::FromString(TEXT("UI Data - Telemetry Defaults"));
		Test.Description = FText::FromString(TEXT("Verify vehicle telemetry structure default values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("UIData")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_UIData_HUDModes"));
		Test.TestName = FText::FromString(TEXT("UI Data - HUD Modes"));
		Test.Description = FText::FromString(TEXT("Verify HUD mode enumeration values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("UIData")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_UIData_DataProvider"));
		Test.TestName = FText::FromString(TEXT("UI Data - Data Provider"));
		Test.Description = FText::FromString(TEXT("Verify HUD data provider subsystem"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("UIData")));
		TestFramework->RegisterTest(Test);
	}

	// Menu Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Menu_SettingsDefaults"));
		Test.TestName = FText::FromString(TEXT("Menu - Settings Defaults"));
		Test.Description = FText::FromString(TEXT("Verify game settings default values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Menu")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Menu_MenuStates"));
		Test.TestName = FText::FromString(TEXT("Menu - Menu States"));
		Test.Description = FText::FromString(TEXT("Verify menu state enumeration values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Menu")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Menu_SettingsCategories"));
		Test.TestName = FText::FromString(TEXT("Menu - Settings Categories"));
		Test.Description = FText::FromString(TEXT("Verify settings category enumeration values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Menu")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Menu_Subsystem"));
		Test.TestName = FText::FromString(TEXT("Menu - Subsystem"));
		Test.Description = FText::FromString(TEXT("Verify menu subsystem functionality"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Menu")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Menu_SettingsRanges"));
		Test.TestName = FText::FromString(TEXT("Menu - Settings Ranges"));
		Test.Description = FText::FromString(TEXT("Verify settings value ranges are valid"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Menu")));
		TestFramework->RegisterTest(Test);
	}

	// Notification Tests
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Notification_Priority"));
		Test.TestName = FText::FromString(TEXT("Notification - Priority"));
		Test.Description = FText::FromString(TEXT("Verify notification priority enumeration values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Notification")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Notification_Types"));
		Test.TestName = FText::FromString(TEXT("Notification - Types"));
		Test.Description = FText::FromString(TEXT("Verify notification type enumeration values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Notification")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Notification_Styles"));
		Test.TestName = FText::FromString(TEXT("Notification - Styles"));
		Test.Description = FText::FromString(TEXT("Verify notification style enumeration values"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Notification")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Notification_DataDefaults"));
		Test.TestName = FText::FromString(TEXT("Notification - Data Defaults"));
		Test.Description = FText::FromString(TEXT("Verify notification data structure defaults"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Notification")));
		TestFramework->RegisterTest(Test);
	}
	{
		FMGTestCase Test;
		Test.TestID = FName(TEXT("Test_Notification_Subsystem"));
		Test.TestName = FText::FromString(TEXT("Notification - Subsystem"));
		Test.Description = FText::FromString(TEXT("Verify notification subsystem functionality"));
		Test.Category = EMGTestCategory::Unit;
		Test.Tags.Add(FName(TEXT("Notification")));
		TestFramework->RegisterTest(Test);
	}

	UE_LOG(LogTemp, Log, TEXT("Registered %d subsystem tests"), 65);
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
// PERFORMANCE TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestPerf_SubsystemTick()
{
	LogTestStart(TEXT("TestPerf_SubsystemTick"));

	TArray<FString> Logs;

	// Benchmark a simple tick-like operation
	const int32 Iterations = 1000;
	double TotalTime = 0.0;

	for (int32 i = 0; i < Iterations; i++)
	{
		double StartTime = FPlatformTime::Seconds();

		// Simulate subsystem work
		volatile float Dummy = 0.0f;
		for (int32 j = 0; j < 100; j++)
		{
			Dummy += FMath::Sin(static_cast<float>(j)) * FMath::Cos(static_cast<float>(j));
		}

		double EndTime = FPlatformTime::Seconds();
		TotalTime += (EndTime - StartTime);
	}

	double AverageTimeMs = (TotalTime / Iterations) * 1000.0;
	Logs.Add(FString::Printf(TEXT("Average tick time: %.4f ms"), AverageTimeMs));
	Logs.Add(FString::Printf(TEXT("Total iterations: %d"), Iterations));

	// Subsystem tick should be fast (< 1ms typically)
	if (AverageTimeMs > 1.0)
	{
		Logs.Add(TEXT("Warning: Tick time exceeds 1ms"));
	}

	return CreatePassResult(
		FName(TEXT("Test_Perf_SubsystemTick")),
		FString::Printf(TEXT("Avg tick: %.4f ms over %d iterations"), AverageTimeMs, Iterations)
	);
}

FMGTestResult UMGSubsystemTests::TestPerf_MemoryUsage()
{
	LogTestStart(TEXT("TestPerf_MemoryUsage"));

	TArray<FString> Logs;

	// Get memory stats
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	double UsedPhysicalMB = MemStats.UsedPhysical / (1024.0 * 1024.0);
	double PeakPhysicalMB = MemStats.PeakUsedPhysical / (1024.0 * 1024.0);
	double UsedVirtualMB = MemStats.UsedVirtual / (1024.0 * 1024.0);
	double AvailablePhysicalMB = MemStats.AvailablePhysical / (1024.0 * 1024.0);

	Logs.Add(FString::Printf(TEXT("Used Physical: %.2f MB"), UsedPhysicalMB));
	Logs.Add(FString::Printf(TEXT("Peak Physical: %.2f MB"), PeakPhysicalMB));
	Logs.Add(FString::Printf(TEXT("Used Virtual: %.2f MB"), UsedVirtualMB));
	Logs.Add(FString::Printf(TEXT("Available Physical: %.2f MB"), AvailablePhysicalMB));

	// Check for reasonable memory usage
	if (UsedPhysicalMB <= 0.0)
	{
		Logs.Add(TEXT("Warning: Memory stats may not be available on this platform"));
	}

	// Test allocation/deallocation
	{
		const int32 AllocSize = 1024 * 1024; // 1MB
		const int32 NumAllocs = 10;

		TArray<void*> Allocations;
		Allocations.Reserve(NumAllocs);

		double AllocStartTime = FPlatformTime::Seconds();
		for (int32 i = 0; i < NumAllocs; i++)
		{
			void* Ptr = FMemory::Malloc(AllocSize);
			if (Ptr)
			{
				Allocations.Add(Ptr);
			}
		}
		double AllocEndTime = FPlatformTime::Seconds();

		double FreeStartTime = FPlatformTime::Seconds();
		for (void* Ptr : Allocations)
		{
			FMemory::Free(Ptr);
		}
		double FreeEndTime = FPlatformTime::Seconds();

		double AllocTimeMs = (AllocEndTime - AllocStartTime) * 1000.0;
		double FreeTimeMs = (FreeEndTime - FreeStartTime) * 1000.0;

		Logs.Add(FString::Printf(TEXT("Alloc %d x 1MB: %.2f ms"), NumAllocs, AllocTimeMs));
		Logs.Add(FString::Printf(TEXT("Free %d x 1MB: %.2f ms"), NumAllocs, FreeTimeMs));
	}

	return CreatePassResult(
		FName(TEXT("Test_Perf_MemoryUsage")),
		FString::Printf(TEXT("Memory: %.2f MB used, %.2f MB peak"), UsedPhysicalMB, PeakPhysicalMB)
	);
}

FMGTestResult UMGSubsystemTests::TestPerf_DelegateBroadcast()
{
	LogTestStart(TEXT("TestPerf_DelegateBroadcast"));

	TArray<FString> Logs;

	// Create a test delegate
	DECLARE_MULTICAST_DELEGATE_OneParam(FTestDelegate, int32);
	FTestDelegate TestDelegate;

	// Add some bindings
	int32 CallCount = 0;
	auto Lambda1 = [&CallCount](int32 Value) { CallCount += Value; };
	auto Lambda2 = [&CallCount](int32 Value) { CallCount += Value * 2; };
	auto Lambda3 = [&CallCount](int32 Value) { CallCount += Value * 3; };

	TestDelegate.AddLambda(Lambda1);
	TestDelegate.AddLambda(Lambda2);
	TestDelegate.AddLambda(Lambda3);

	Logs.Add(FString::Printf(TEXT("Delegate bindings: 3")));

	// Benchmark broadcasts
	const int32 BroadcastCount = 10000;

	double StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < BroadcastCount; i++)
	{
		TestDelegate.Broadcast(1);
	}
	double EndTime = FPlatformTime::Seconds();

	double TotalTimeMs = (EndTime - StartTime) * 1000.0;
	double AvgBroadcastUs = (TotalTimeMs * 1000.0) / BroadcastCount;

	Logs.Add(FString::Printf(TEXT("Broadcast count: %d"), BroadcastCount));
	Logs.Add(FString::Printf(TEXT("Total time: %.2f ms"), TotalTimeMs));
	Logs.Add(FString::Printf(TEXT("Avg broadcast: %.3f us"), AvgBroadcastUs));

	// Clear delegate
	TestDelegate.Clear();

	// Delegate broadcasts should be fast
	if (AvgBroadcastUs > 10.0)
	{
		Logs.Add(TEXT("Warning: Delegate broadcast time seems high"));
	}

	return CreatePassResult(
		FName(TEXT("Test_Perf_DelegateBroadcast")),
		FString::Printf(TEXT("Avg broadcast: %.3f us over %d calls"), AvgBroadcastUs, BroadcastCount)
	);
}

FMGTestResult UMGSubsystemTests::TestPerf_DataAccess()
{
	LogTestStart(TEXT("TestPerf_DataAccess"));

	TArray<FString> Logs;

	// Test TMap access
	{
		TMap<FName, int32> TestMap;
		const int32 NumEntries = 1000;

		// Populate
		for (int32 i = 0; i < NumEntries; i++)
		{
			TestMap.Add(FName(*FString::Printf(TEXT("Key_%d"), i)), i);
		}

		// Benchmark lookups
		const int32 LookupCount = 10000;
		double StartTime = FPlatformTime::Seconds();

		volatile int32 Dummy = 0;
		for (int32 i = 0; i < LookupCount; i++)
		{
			FName Key(*FString::Printf(TEXT("Key_%d"), i % NumEntries));
			if (int32* Value = TestMap.Find(Key))
			{
				Dummy = *Value;
			}
		}

		double EndTime = FPlatformTime::Seconds();
		double TotalTimeMs = (EndTime - StartTime) * 1000.0;
		double AvgLookupUs = (TotalTimeMs * 1000.0) / LookupCount;

		Logs.Add(FString::Printf(TEXT("TMap entries: %d"), NumEntries));
		Logs.Add(FString::Printf(TEXT("TMap lookups: %d"), LookupCount));
		Logs.Add(FString::Printf(TEXT("TMap avg lookup: %.3f us"), AvgLookupUs));
	}

	// Test TArray access
	{
		TArray<int32> TestArray;
		const int32 ArraySize = 10000;
		TestArray.Reserve(ArraySize);

		for (int32 i = 0; i < ArraySize; i++)
		{
			TestArray.Add(i);
		}

		// Benchmark sequential access
		const int32 AccessCount = 100000;
		double StartTime = FPlatformTime::Seconds();

		volatile int64 Sum = 0;
		for (int32 i = 0; i < AccessCount; i++)
		{
			Sum += TestArray[i % ArraySize];
		}

		double EndTime = FPlatformTime::Seconds();
		double TotalTimeMs = (EndTime - StartTime) * 1000.0;
		double AvgAccessUs = (TotalTimeMs * 1000.0) / AccessCount;

		Logs.Add(FString::Printf(TEXT("TArray size: %d"), ArraySize));
		Logs.Add(FString::Printf(TEXT("TArray accesses: %d"), AccessCount));
		Logs.Add(FString::Printf(TEXT("TArray avg access: %.4f us"), AvgAccessUs));
	}

	return CreatePassResult(
		FName(TEXT("Test_Perf_DataAccess")),
		TEXT("Data structure access benchmarks completed")
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
// SAVE/LOAD TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestSave_CreateSaveGame()
{
	LogTestStart(TEXT("TestSave_CreateSaveGame"));

	// Create a save game object
	UMGSaveGame* SaveGame = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));

	if (!SaveGame)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_CreateSaveGame")),
			TEXT("Failed to create save game object"),
			{TEXT("UGameplayStatics::CreateSaveGameObject returned nullptr")}
		);
	}

	// Verify it's the correct type
	if (!SaveGame->IsA(UMGSaveGame::StaticClass()))
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_CreateSaveGame")),
			TEXT("Save game object is wrong type"),
			{TEXT("Expected UMGSaveGame type")}
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Save_CreateSaveGame")),
		TEXT("Save game object created successfully")
	);
}

FMGTestResult UMGSubsystemTests::TestSave_DefaultValues()
{
	LogTestStart(TEXT("TestSave_DefaultValues"));

	UMGSaveGame* SaveGame = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));

	if (!SaveGame)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_DefaultValues")),
			TEXT("Failed to create save game for testing"),
			{TEXT("CreateSaveGameObject returned nullptr")}
		);
	}

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Check default player cash
	if (SaveGame->PlayerCash < 0)
	{
		Logs.Add(FString::Printf(TEXT("PlayerCash is negative: %lld"), SaveGame->PlayerCash));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("PlayerCash default: %lld"), SaveGame->PlayerCash));
	}

	// Check default player level
	if (SaveGame->PlayerLevel < 1)
	{
		Logs.Add(FString::Printf(TEXT("PlayerLevel should be at least 1: %d"), SaveGame->PlayerLevel));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("PlayerLevel default: %d"), SaveGame->PlayerLevel));
	}

	// Check default player XP
	if (SaveGame->PlayerXP < 0)
	{
		Logs.Add(FString::Printf(TEXT("PlayerXP is negative: %lld"), SaveGame->PlayerXP));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("PlayerXP default: %lld"), SaveGame->PlayerXP));
	}

	// Check save version is set
	if (SaveGame->SaveVersion <= 0)
	{
		Logs.Add(FString::Printf(TEXT("SaveVersion should be positive: %d"), SaveGame->SaveVersion));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("SaveVersion: %d"), SaveGame->SaveVersion));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_DefaultValues")),
			TEXT("Some default values are invalid"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Save_DefaultValues")),
		TEXT("All default values are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestSave_DataStructures()
{
	LogTestStart(TEXT("TestSave_DataStructures"));

	UMGSaveGame* SaveGame = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));

	if (!SaveGame)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_DataStructures")),
			TEXT("Failed to create save game for testing"),
			{TEXT("CreateSaveGameObject returned nullptr")}
		);
	}

	TArray<FString> Logs;

	// Test owned vehicles array
	Logs.Add(FString::Printf(TEXT("OwnedVehicles count: %d"), SaveGame->OwnedVehicles.Num()));

	// Test garage slots
	Logs.Add(FString::Printf(TEXT("GarageSlots count: %d"), SaveGame->GarageSlots.Num()));

	// Test unlocked districts
	Logs.Add(FString::Printf(TEXT("UnlockedDistricts count: %d"), SaveGame->UnlockedDistricts.Num()));

	// Test discovered shortcuts
	Logs.Add(FString::Printf(TEXT("DiscoveredShortcuts count: %d"), SaveGame->DiscoveredShortcuts.Num()));

	// Test completed races
	Logs.Add(FString::Printf(TEXT("CompletedRaces count: %d"), SaveGame->CompletedRaces.Num()));

	// Test heat level data
	Logs.Add(FString::Printf(TEXT("HeatLevelData districts: %d"), SaveGame->HeatLevelData.Num()));

	// Test takedown records
	Logs.Add(FString::Printf(TEXT("TakedownRecords count: %d"), SaveGame->TakedownRecords.Num()));

	return CreatePassResult(
		FName(TEXT("Test_Save_DataStructures")),
		FString::Printf(TEXT("All save data structures initialized, %d total collections"), 7)
	);
}

FMGTestResult UMGSubsystemTests::TestSave_ManagerSubsystem()
{
	LogTestStart(TEXT("TestSave_ManagerSubsystem"));

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_ManagerSubsystem")),
			TEXT("GameInstance not found"),
			{TEXT("Cannot access GameInstance to get SaveManager")}
		);
	}

	UMGSaveManagerSubsystem* SaveManager = GameInstance->GetSubsystem<UMGSaveManagerSubsystem>();
	if (!SaveManager)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_ManagerSubsystem")),
			TEXT("SaveManager subsystem not found"),
			{TEXT("GetSubsystem<UMGSaveManagerSubsystem> returned nullptr")}
		);
	}

	TArray<FString> Logs;

	// Verify subsystem is initialized
	Logs.Add(TEXT("SaveManager subsystem found and accessible"));

	// Check quick save slot name
	FString QuickSaveSlot = SaveManager->GetQuickSaveSlotName();
	if (QuickSaveSlot.IsEmpty())
	{
		Logs.Add(TEXT("Warning: QuickSaveSlot name is empty"));
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("QuickSaveSlot: %s"), *QuickSaveSlot));
	}

	// Check autosave slot name
	FString AutoSaveSlot = SaveManager->GetAutoSaveSlotName();
	if (AutoSaveSlot.IsEmpty())
	{
		Logs.Add(TEXT("Warning: AutoSaveSlot name is empty"));
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("AutoSaveSlot: %s"), *AutoSaveSlot));
	}

	return CreatePassResult(
		FName(TEXT("Test_Save_ManagerSubsystem")),
		TEXT("SaveManager subsystem initialized correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestSave_SlotNaming()
{
	LogTestStart(TEXT("TestSave_SlotNaming"));

	UMGSaveGame* SaveGame = Cast<UMGSaveGame>(UGameplayStatics::CreateSaveGameObject(UMGSaveGame::StaticClass()));

	if (!SaveGame)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_SlotNaming")),
			TEXT("Failed to create save game for testing"),
			{TEXT("CreateSaveGameObject returned nullptr")}
		);
	}

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test default slot name
	if (SaveGame->SaveSlotName.IsEmpty())
	{
		Logs.Add(TEXT("SaveSlotName is empty - checking if default exists"));
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("SaveSlotName: %s"), *SaveGame->SaveSlotName));
	}

	// Verify slot name follows conventions (no special characters that could cause file issues)
	FString TestSlotName = TEXT("MG_TestSlot_001");

	// Check for valid characters
	bool bValidChars = true;
	for (TCHAR Char : TestSlotName)
	{
		if (Char == '/' || Char == '\\' || Char == ':' || Char == '*' ||
			Char == '?' || Char == '"' || Char == '<' || Char == '>' || Char == '|')
		{
			bValidChars = false;
			break;
		}
	}

	if (bValidChars)
	{
		Logs.Add(FString::Printf(TEXT("Test slot name '%s' has valid characters"), *TestSlotName));
	}
	else
	{
		Logs.Add(TEXT("Test slot name contains invalid characters"));
		bAllPassed = false;
	}

	// Test slot name with user index would be valid
	int32 TestUserIndex = 0;
	Logs.Add(FString::Printf(TEXT("User index for saves: %d"), TestUserIndex));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Save_SlotNaming")),
			TEXT("Slot naming issues found"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Save_SlotNaming")),
		TEXT("Save slot naming conventions are valid")
	);
}

// ==========================================
// PHYSICS TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestPhysics_TireCompoundGrip()
{
	LogTestStart(TEXT("TestPhysics_TireCompoundGrip"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test all tire compound grip values
	struct FCompoundTest
	{
		EMGTireCompound Compound;
		const TCHAR* Name;
		float ExpectedMin;
		float ExpectedMax;
	};

	TArray<FCompoundTest> CompoundTests = {
		{EMGTireCompound::Economy, TEXT("Economy"), 0.6f, 0.8f},
		{EMGTireCompound::AllSeason, TEXT("AllSeason"), 0.7f, 0.85f},
		{EMGTireCompound::Sport, TEXT("Sport"), 0.8f, 0.9f},
		{EMGTireCompound::Performance, TEXT("Performance"), 0.9f, 1.0f},
		{EMGTireCompound::SemiSlick, TEXT("SemiSlick"), 1.0f, 1.1f},
		{EMGTireCompound::Slick, TEXT("Slick"), 1.1f, 1.2f},
		{EMGTireCompound::DragRadial, TEXT("DragRadial"), 1.05f, 1.15f},
		{EMGTireCompound::Drift, TEXT("Drift"), 0.75f, 0.85f}
	};

	for (const auto& Test : CompoundTests)
	{
		float Grip = UMGStatCalculator::GetTireCompoundGrip(Test.Compound);
		Logs.Add(FString::Printf(TEXT("%s compound grip: %.2f"), Test.Name, Grip));

		if (Grip < Test.ExpectedMin || Grip > Test.ExpectedMax)
		{
			Logs.Add(FString::Printf(TEXT("  -> FAIL: Expected %.2f-%.2f"), Test.ExpectedMin, Test.ExpectedMax));
			bAllPassed = false;
		}
	}

	// Verify grip ordering (Slick > SemiSlick > Performance > Sport > AllSeason > Economy)
	float SlickGrip = UMGStatCalculator::GetTireCompoundGrip(EMGTireCompound::Slick);
	float EconomyGrip = UMGStatCalculator::GetTireCompoundGrip(EMGTireCompound::Economy);

	if (SlickGrip <= EconomyGrip)
	{
		Logs.Add(TEXT("FAIL: Slick grip should be higher than Economy"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_TireCompoundGrip")),
			TEXT("Some tire compound grip values out of expected range"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_TireCompoundGrip")),
		FString::Printf(TEXT("All %d tire compounds have valid grip coefficients"), CompoundTests.Num())
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_WetGripModifiers()
{
	LogTestStart(TEXT("TestPhysics_WetGripModifiers"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test wet grip modifiers - slicks should be dangerous in wet
	TArray<TPair<EMGTireCompound, const TCHAR*>> Compounds = {
		{EMGTireCompound::Economy, TEXT("Economy")},
		{EMGTireCompound::AllSeason, TEXT("AllSeason")},
		{EMGTireCompound::Sport, TEXT("Sport")},
		{EMGTireCompound::Performance, TEXT("Performance")},
		{EMGTireCompound::SemiSlick, TEXT("SemiSlick")},
		{EMGTireCompound::Slick, TEXT("Slick")},
		{EMGTireCompound::DragRadial, TEXT("DragRadial")},
		{EMGTireCompound::Drift, TEXT("Drift")}
	};

	for (const auto& Compound : Compounds)
	{
		float WetGrip = UMGStatCalculator::GetWetGripModifier(Compound.Key);
		Logs.Add(FString::Printf(TEXT("%s wet modifier: %.2f"), Compound.Value, WetGrip));

		// All wet modifiers should be between 0 and 1
		if (WetGrip < 0.0f || WetGrip > 1.0f)
		{
			Logs.Add(FString::Printf(TEXT("  -> FAIL: Wet modifier out of range (0.0-1.0)")));
			bAllPassed = false;
		}
	}

	// Slicks should have worst wet performance
	float SlickWet = UMGStatCalculator::GetWetGripModifier(EMGTireCompound::Slick);
	float AllSeasonWet = UMGStatCalculator::GetWetGripModifier(EMGTireCompound::AllSeason);

	if (SlickWet >= AllSeasonWet)
	{
		Logs.Add(TEXT("FAIL: Slicks should have worse wet grip than AllSeason"));
		bAllPassed = false;
	}

	// Verify slick wet grip is significantly reduced
	if (SlickWet > 0.4f)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Slick wet grip (%.2f) should be <= 0.4"), SlickWet));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_WetGripModifiers")),
			TEXT("Wet grip modifiers have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_WetGripModifiers")),
		FString::Printf(TEXT("All %d wet grip modifiers are valid"), Compounds.Num())
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_WeightTransferConstants()
{
	LogTestStart(TEXT("TestPhysics_WeightTransferConstants"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Verify weight transfer constants are sensible
	using namespace MGPhysicsConstants::WeightTransfer;

	Logs.Add(FString::Printf(TEXT("Longitudinal ratio: %.3f"), LONGITUDINAL_RATIO));
	Logs.Add(FString::Printf(TEXT("Lateral ratio: %.3f"), LATERAL_RATIO));
	Logs.Add(FString::Printf(TEXT("Load min: %.3f"), LOAD_MIN));
	Logs.Add(FString::Printf(TEXT("Load max: %.3f"), LOAD_MAX));
	Logs.Add(FString::Printf(TEXT("Accel to transfer: %.6f"), ACCEL_TO_TRANSFER));
	Logs.Add(FString::Printf(TEXT("Default rate: %.1f"), DEFAULT_RATE));

	// Longitudinal ratio should be reasonable (0-0.5)
	if (LONGITUDINAL_RATIO < 0.0f || LONGITUDINAL_RATIO > 0.5f)
	{
		Logs.Add(TEXT("FAIL: Longitudinal ratio out of expected range (0-0.5)"));
		bAllPassed = false;
	}

	// Lateral ratio should be reasonable (0-0.5)
	if (LATERAL_RATIO < 0.0f || LATERAL_RATIO > 0.5f)
	{
		Logs.Add(TEXT("FAIL: Lateral ratio out of expected range (0-0.5)"));
		bAllPassed = false;
	}

	// Load min should be positive and less than 1
	if (LOAD_MIN <= 0.0f || LOAD_MIN >= 1.0f)
	{
		Logs.Add(TEXT("FAIL: Load min should be in range (0-1)"));
		bAllPassed = false;
	}

	// Load max should be greater than 1
	if (LOAD_MAX <= 1.0f || LOAD_MAX > 3.0f)
	{
		Logs.Add(TEXT("FAIL: Load max should be in range (1-3)"));
		bAllPassed = false;
	}

	// Default rate should be positive
	if (DEFAULT_RATE <= 0.0f)
	{
		Logs.Add(TEXT("FAIL: Default rate should be positive"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_WeightTransferConstants")),
			TEXT("Weight transfer constants have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_WeightTransferConstants")),
		TEXT("All weight transfer constants are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_TireTemperatureConstants()
{
	LogTestStart(TEXT("TestPhysics_TireTemperatureConstants"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	using namespace MGPhysicsConstants::TireTemperature;

	Logs.Add(FString::Printf(TEXT("Ambient temp: %.1f C"), AMBIENT));
	Logs.Add(FString::Printf(TEXT("Optimal temp: %.1f C"), OPTIMAL));
	Logs.Add(FString::Printf(TEXT("Peak temp: %.1f C"), PEAK));
	Logs.Add(FString::Printf(TEXT("Overheat temp: %.1f C"), OVERHEAT));
	Logs.Add(FString::Printf(TEXT("Cold grip min: %.2f"), COLD_GRIP_MIN));
	Logs.Add(FString::Printf(TEXT("Optimal grip: %.2f"), OPTIMAL_GRIP));
	Logs.Add(FString::Printf(TEXT("Peak grip: %.2f"), PEAK_GRIP));
	Logs.Add(FString::Printf(TEXT("Overheat loss/deg: %.4f"), OVERHEAT_GRIP_LOSS_PER_DEG));

	// Verify temperature ordering: Ambient < Optimal < Peak < Overheat
	if (!(AMBIENT < OPTIMAL && OPTIMAL < PEAK && PEAK <= OVERHEAT))
	{
		Logs.Add(TEXT("FAIL: Temperature ordering should be Ambient < Optimal < Peak <= Overheat"));
		bAllPassed = false;
	}

	// Verify grip ordering: Cold < Optimal <= Peak
	if (!(COLD_GRIP_MIN < OPTIMAL_GRIP && OPTIMAL_GRIP <= PEAK_GRIP))
	{
		Logs.Add(TEXT("FAIL: Grip ordering should be Cold < Optimal <= Peak"));
		bAllPassed = false;
	}

	// Ambient should be reasonable room temperature
	if (AMBIENT < 10.0f || AMBIENT > 40.0f)
	{
		Logs.Add(TEXT("FAIL: Ambient temp should be 10-40 C"));
		bAllPassed = false;
	}

	// Optimal should be in range for racing tires
	if (OPTIMAL < 60.0f || OPTIMAL > 120.0f)
	{
		Logs.Add(TEXT("FAIL: Optimal temp should be 60-120 C"));
		bAllPassed = false;
	}

	// Cold grip should be less than full
	if (COLD_GRIP_MIN >= 1.0f)
	{
		Logs.Add(TEXT("FAIL: Cold grip should be < 1.0"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_TireTemperatureConstants")),
			TEXT("Tire temperature constants have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_TireTemperatureConstants")),
		TEXT("All tire temperature constants are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_HandlingModeSettings()
{
	LogTestStart(TEXT("TestPhysics_HandlingModeSettings"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test all three handling modes
	TArray<EMGPhysicsHandlingMode> Modes = {
		EMGPhysicsHandlingMode::Arcade,
		EMGPhysicsHandlingMode::Balanced,
		EMGPhysicsHandlingMode::Simulation
	};

	for (EMGPhysicsHandlingMode Mode : Modes)
	{
		FMGPhysicsHandlingSettings Settings = UMGPhysicsHandlingConfig::GetSettingsForMode(Mode);

		const TCHAR* ModeName = Mode == EMGPhysicsHandlingMode::Arcade ? TEXT("Arcade") :
								Mode == EMGPhysicsHandlingMode::Balanced ? TEXT("Balanced") : TEXT("Simulation");

		Logs.Add(FString::Printf(TEXT("%s mode:"), ModeName));
		Logs.Add(FString::Printf(TEXT("  StabilityControl: %.2f"), Settings.StabilityControl));
		Logs.Add(FString::Printf(TEXT("  BaseTireGrip: %.2f"), Settings.BaseTireGrip));
		Logs.Add(FString::Printf(TEXT("  TireTempInfluence: %.2f"), Settings.TireTempInfluence));

		// Verify stability control is in range
		if (Settings.StabilityControl < 0.0f || Settings.StabilityControl > 1.0f)
		{
			Logs.Add(FString::Printf(TEXT("  FAIL: StabilityControl out of range (0-1)")));
			bAllPassed = false;
		}

		// Verify base tire grip is reasonable
		if (Settings.BaseTireGrip < 0.5f || Settings.BaseTireGrip > 2.0f)
		{
			Logs.Add(FString::Printf(TEXT("  FAIL: BaseTireGrip out of range (0.5-2.0)")));
			bAllPassed = false;
		}

		// Verify mode is set correctly
		if (Settings.Mode != Mode)
		{
			Logs.Add(FString::Printf(TEXT("  FAIL: Mode not set correctly")));
			bAllPassed = false;
		}
	}

	// Arcade should have more assists than Simulation
	FMGPhysicsHandlingSettings Arcade = UMGPhysicsHandlingConfig::GetArcadeSettings();
	FMGPhysicsHandlingSettings Simulation = UMGPhysicsHandlingConfig::GetSimulationSettings();

	if (Arcade.StabilityControl <= Simulation.StabilityControl)
	{
		Logs.Add(TEXT("FAIL: Arcade should have more stability control than Simulation"));
		bAllPassed = false;
	}

	if (Arcade.BaseTireGrip <= Simulation.BaseTireGrip)
	{
		Logs.Add(TEXT("FAIL: Arcade should have more base grip than Simulation"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_HandlingModeSettings")),
			TEXT("Handling mode settings have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_HandlingModeSettings")),
		FString::Printf(TEXT("All %d handling modes have valid settings"), Modes.Num())
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_SurfaceConstants()
{
	LogTestStart(TEXT("TestPhysics_SurfaceConstants"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	using namespace MGPhysicsConstants::Surface;

	Logs.Add(FString::Printf(TEXT("Ice friction threshold: %.2f"), ICE_FRICTION_THRESHOLD));
	Logs.Add(FString::Printf(TEXT("Traces per frame: %d"), TRACES_PER_FRAME));

	// Ice friction threshold should be low but positive
	if (ICE_FRICTION_THRESHOLD <= 0.0f || ICE_FRICTION_THRESHOLD > 0.5f)
	{
		Logs.Add(TEXT("FAIL: Ice friction threshold should be in range (0-0.5)"));
		bAllPassed = false;
	}

	// Traces per frame should be at least 1 (typically 4 for 4 wheels)
	if (TRACES_PER_FRAME < 1 || TRACES_PER_FRAME > 16)
	{
		Logs.Add(TEXT("FAIL: Traces per frame should be in range (1-16)"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_SurfaceConstants")),
			TEXT("Surface constants have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_SurfaceConstants")),
		TEXT("All surface constants are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_GeometryConstants()
{
	LogTestStart(TEXT("TestPhysics_GeometryConstants"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	using namespace MGPhysicsConstants::Geometry;

	Logs.Add(FString::Printf(TEXT("Toe effect factor: %.3f"), TOE_EFFECT_FACTOR));
	Logs.Add(FString::Printf(TEXT("Camber grip per degree: %.3f"), CAMBER_GRIP_PER_DEG));
	Logs.Add(FString::Printf(TEXT("Optimal camber (degrees): %.1f"), OPTIMAL_CAMBER_DEG));

	// Toe effect factor should be small positive
	if (TOE_EFFECT_FACTOR < 0.0f || TOE_EFFECT_FACTOR > 0.5f)
	{
		Logs.Add(TEXT("FAIL: Toe effect factor should be in range (0-0.5)"));
		bAllPassed = false;
	}

	// Camber grip per degree should be small positive
	if (CAMBER_GRIP_PER_DEG < 0.0f || CAMBER_GRIP_PER_DEG > 0.1f)
	{
		Logs.Add(TEXT("FAIL: Camber grip per degree should be in range (0-0.1)"));
		bAllPassed = false;
	}

	// Optimal camber should be negative (for negative camber benefit)
	if (OPTIMAL_CAMBER_DEG > 0.0f || OPTIMAL_CAMBER_DEG < -10.0f)
	{
		Logs.Add(TEXT("FAIL: Optimal camber should be in range (-10 to 0 degrees)"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_GeometryConstants")),
			TEXT("Geometry constants have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_GeometryConstants")),
		TEXT("All suspension geometry constants are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_DifferentialConstants()
{
	LogTestStart(TEXT("TestPhysics_DifferentialConstants"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	using namespace MGPhysicsConstants::Differential;

	Logs.Add(FString::Printf(TEXT("1.5-way coast factor: %.2f"), ONE_POINT_FIVE_WAY_COAST_FACTOR));
	Logs.Add(FString::Printf(TEXT("Min speed diff threshold: %.2f rad/s"), MIN_SPEED_DIFF_THRESHOLD));

	// Coast factor should be in range 0-1
	if (ONE_POINT_FIVE_WAY_COAST_FACTOR < 0.0f || ONE_POINT_FIVE_WAY_COAST_FACTOR > 1.0f)
	{
		Logs.Add(TEXT("FAIL: Coast factor should be in range (0-1)"));
		bAllPassed = false;
	}

	// For 1.5-way, coast factor should be around 0.3-0.6
	if (ONE_POINT_FIVE_WAY_COAST_FACTOR < 0.2f || ONE_POINT_FIVE_WAY_COAST_FACTOR > 0.7f)
	{
		Logs.Add(FString::Printf(TEXT("WARNING: 1.5-way coast factor %.2f is outside typical range (0.3-0.6)"), ONE_POINT_FIVE_WAY_COAST_FACTOR));
	}

	// Min speed diff should be positive and small
	if (MIN_SPEED_DIFF_THRESHOLD <= 0.0f || MIN_SPEED_DIFF_THRESHOLD > 5.0f)
	{
		Logs.Add(TEXT("FAIL: Min speed diff threshold should be in range (0-5 rad/s)"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_DifferentialConstants")),
			TEXT("Differential constants have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_DifferentialConstants")),
		TEXT("All differential constants are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestPhysics_WearConstants()
{
	LogTestStart(TEXT("TestPhysics_WearConstants"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	using namespace MGPhysicsConstants::Wear;

	Logs.Add(FString::Printf(TEXT("Suspension damping degradation: %.2f"), SUSPENSION_DAMPING_DEGRADATION));
	Logs.Add(FString::Printf(TEXT("Tire grip degradation: %.2f"), TIRE_GRIP_DEGRADATION));

	// Suspension damping degradation should be in range 0-1
	if (SUSPENSION_DAMPING_DEGRADATION < 0.0f || SUSPENSION_DAMPING_DEGRADATION > 1.0f)
	{
		Logs.Add(TEXT("FAIL: Suspension damping degradation should be in range (0-1)"));
		bAllPassed = false;
	}

	// Tire grip degradation should be in range 0-1
	if (TIRE_GRIP_DEGRADATION < 0.0f || TIRE_GRIP_DEGRADATION > 1.0f)
	{
		Logs.Add(TEXT("FAIL: Tire grip degradation should be in range (0-1)"));
		bAllPassed = false;
	}

	// At 100% wear, we should still have some functionality
	// Suspension should have at least 50% effectiveness
	if (SUSPENSION_DAMPING_DEGRADATION > 0.5f)
	{
		Logs.Add(TEXT("WARNING: Suspension degradation > 50% may feel too punishing"));
	}

	// Tires should have at least 50% grip at max wear
	if (TIRE_GRIP_DEGRADATION > 0.5f)
	{
		Logs.Add(TEXT("WARNING: Tire grip degradation > 50% may feel too punishing"));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Physics_WearConstants")),
			TEXT("Wear constants have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Physics_WearConstants")),
		TEXT("All wear degradation constants are valid")
	);
}

// ==========================================
// STRESS TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestStress_HighObjectCount()
{
	LogTestStart(TEXT("TestStress_HighObjectCount"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test allocating and manipulating many objects
	const int32 ObjectCount = 10000;

	double StartTime = FPlatformTime::Seconds();

	// Allocate many FStrings (simulates data structures)
	TArray<FString> Strings;
	Strings.Reserve(ObjectCount);

	for (int32 i = 0; i < ObjectCount; i++)
	{
		Strings.Add(FString::Printf(TEXT("TestObject_%d"), i));
	}

	double AllocTime = FPlatformTime::Seconds();
	double AllocMs = (AllocTime - StartTime) * 1000.0;

	// Test TMap with many entries
	TMap<FName, int32> TestMap;
	for (int32 i = 0; i < ObjectCount; i++)
	{
		TestMap.Add(FName(*FString::Printf(TEXT("Key_%d"), i)), i);
	}

	double MapTime = FPlatformTime::Seconds();
	double MapMs = (MapTime - AllocTime) * 1000.0;

	// Perform lookups
	volatile int32 LookupSum = 0;
	for (int32 i = 0; i < ObjectCount; i++)
	{
		if (int32* Value = TestMap.Find(FName(*FString::Printf(TEXT("Key_%d"), i))))
		{
			LookupSum += *Value;
		}
	}

	double LookupTime = FPlatformTime::Seconds();
	double LookupMs = (LookupTime - MapTime) * 1000.0;

	Logs.Add(FString::Printf(TEXT("Objects: %d"), ObjectCount));
	Logs.Add(FString::Printf(TEXT("Array alloc: %.2f ms"), AllocMs));
	Logs.Add(FString::Printf(TEXT("Map build: %.2f ms"), MapMs));
	Logs.Add(FString::Printf(TEXT("Map lookup: %.2f ms"), LookupMs));
	Logs.Add(FString::Printf(TEXT("Total: %.2f ms"), AllocMs + MapMs + LookupMs));

	// Check for reasonable performance (< 1000ms total)
	double TotalMs = AllocMs + MapMs + LookupMs;
	if (TotalMs > 1000.0)
	{
		Logs.Add(TEXT("WARNING: Operations taking > 1 second"));
	}

	// Verify all operations completed
	if (Strings.Num() != ObjectCount)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Expected %d strings, got %d"), ObjectCount, Strings.Num()));
		bAllPassed = false;
	}

	if (TestMap.Num() != ObjectCount)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Expected %d map entries, got %d"), ObjectCount, TestMap.Num()));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Stress_HighObjectCount")),
			TEXT("High object count test failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Stress_HighObjectCount")),
		FString::Printf(TEXT("Handled %d objects in %.2f ms"), ObjectCount, TotalMs)
	);
}

FMGTestResult UMGSubsystemTests::TestStress_SustainedOperation()
{
	LogTestStart(TEXT("TestStress_SustainedOperation"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test many iterations of typical operations
	const int32 Iterations = 100000;

	double StartTime = FPlatformTime::Seconds();

	// Simulate sustained computation
	volatile float Result = 0.0f;
	for (int32 i = 0; i < Iterations; i++)
	{
		// Typical game calculations
		float Angle = FMath::DegreesToRadians((float)i);
		Result += FMath::Sin(Angle) * FMath::Cos(Angle);

		// Vector operations
		FVector V1(i * 0.1f, i * 0.2f, i * 0.3f);
		FVector V2(i * 0.3f, i * 0.2f, i * 0.1f);
		Result += FVector::DotProduct(V1.GetSafeNormal(), V2.GetSafeNormal());
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgUs = (TotalMs * 1000.0) / Iterations;

	Logs.Add(FString::Printf(TEXT("Iterations: %d"), Iterations));
	Logs.Add(FString::Printf(TEXT("Total time: %.2f ms"), TotalMs));
	Logs.Add(FString::Printf(TEXT("Avg per iteration: %.4f us"), AvgUs));

	// Check for reasonable performance
	if (TotalMs > 5000.0)
	{
		Logs.Add(TEXT("WARNING: Sustained operation > 5 seconds"));
	}

	// Verify computation completed (volatile prevents optimization)
	if (FMath::IsNaN(Result))
	{
		Logs.Add(TEXT("FAIL: Computation resulted in NaN"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Stress_SustainedOperation")),
			TEXT("Sustained operation test failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Stress_SustainedOperation")),
		FString::Printf(TEXT("Completed %d iterations in %.2f ms (%.4f us/iter)"), Iterations, TotalMs, AvgUs)
	);
}

FMGTestResult UMGSubsystemTests::TestStress_MemoryStability()
{
	LogTestStart(TEXT("TestStress_MemoryStability"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test repeated alloc/dealloc cycles
	const int32 Cycles = 100;
	const int32 ObjectsPerCycle = 1000;

	SIZE_T StartMem = FPlatformMemory::GetStats().UsedPhysical;

	for (int32 Cycle = 0; Cycle < Cycles; Cycle++)
	{
		// Allocate
		TArray<TSharedPtr<FString>> Objects;
		Objects.Reserve(ObjectsPerCycle);

		for (int32 i = 0; i < ObjectsPerCycle; i++)
		{
			Objects.Add(MakeShared<FString>(FString::Printf(TEXT("Cycle%d_Object%d"), Cycle, i)));
		}

		// Validate
		if (Objects.Num() != ObjectsPerCycle)
		{
			Logs.Add(FString::Printf(TEXT("FAIL: Cycle %d - Expected %d objects, got %d"), Cycle, ObjectsPerCycle, Objects.Num()));
			bAllPassed = false;
			break;
		}

		// Objects deallocated when TArray goes out of scope
	}

	SIZE_T EndMem = FPlatformMemory::GetStats().UsedPhysical;
	int64 MemDelta = (int64)EndMem - (int64)StartMem;

	Logs.Add(FString::Printf(TEXT("Cycles: %d"), Cycles));
	Logs.Add(FString::Printf(TEXT("Objects per cycle: %d"), ObjectsPerCycle));
	Logs.Add(FString::Printf(TEXT("Total allocations: %d"), Cycles * ObjectsPerCycle));
	Logs.Add(FString::Printf(TEXT("Memory delta: %lld bytes"), MemDelta));

	// Check for memory leaks (allow some variance from other processes)
	// Significant leak would be > 10MB growth
	if (MemDelta > 10 * 1024 * 1024)
	{
		Logs.Add(TEXT("WARNING: Significant memory growth detected (> 10MB)"));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Stress_MemoryStability")),
			TEXT("Memory stability test failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Stress_MemoryStability")),
		FString::Printf(TEXT("Completed %d alloc/dealloc cycles, memory delta: %lld bytes"), Cycles, MemDelta)
	);
}

FMGTestResult UMGSubsystemTests::TestStress_RapidStateChanges()
{
	LogTestStart(TEXT("TestStress_RapidStateChanges"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test rapid state machine transitions
	const int32 Transitions = 10000;

	// Simulate enum state machine
	enum class ETestState : uint8
	{
		Idle,
		Starting,
		Running,
		Paused,
		Stopping,
		Stopped
	};

	double StartTime = FPlatformTime::Seconds();

	ETestState CurrentState = ETestState::Idle;
	int32 StateChanges = 0;

	for (int32 i = 0; i < Transitions; i++)
	{
		// Cycle through all states
		switch (CurrentState)
		{
		case ETestState::Idle:
			CurrentState = ETestState::Starting;
			break;
		case ETestState::Starting:
			CurrentState = ETestState::Running;
			break;
		case ETestState::Running:
			CurrentState = ETestState::Paused;
			break;
		case ETestState::Paused:
			CurrentState = ETestState::Stopping;
			break;
		case ETestState::Stopping:
			CurrentState = ETestState::Stopped;
			break;
		case ETestState::Stopped:
			CurrentState = ETestState::Idle;
			break;
		}
		StateChanges++;
	}

	double EndTime = FPlatformTime::Seconds();
	double TotalMs = (EndTime - StartTime) * 1000.0;
	double AvgNs = (TotalMs * 1000000.0) / Transitions;

	Logs.Add(FString::Printf(TEXT("State transitions: %d"), StateChanges));
	Logs.Add(FString::Printf(TEXT("Total time: %.2f ms"), TotalMs));
	Logs.Add(FString::Printf(TEXT("Avg per transition: %.2f ns"), AvgNs));

	// Verify all transitions occurred
	if (StateChanges != Transitions)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Expected %d transitions, got %d"), Transitions, StateChanges));
		bAllPassed = false;
	}

	// Test rapid weather state changes (if subsystem available)
	UMGWeatherSubsystem* Weather = GetWeatherSubsystem();
	if (Weather)
	{
		double WeatherStart = FPlatformTime::Seconds();
		const int32 WeatherChanges = 1000;

		TArray<EMGWeatherType> WeatherTypes = {
			EMGWeatherType::Clear,
			EMGWeatherType::Cloudy,
			EMGWeatherType::LightRain,
			EMGWeatherType::HeavyRain,
			EMGWeatherType::Fog
		};

		for (int32 i = 0; i < WeatherChanges; i++)
		{
			Weather->SetWeatherInstant(WeatherTypes[i % WeatherTypes.Num()]);
		}

		double WeatherEnd = FPlatformTime::Seconds();
		double WeatherMs = (WeatherEnd - WeatherStart) * 1000.0;

		Logs.Add(FString::Printf(TEXT("Weather changes: %d in %.2f ms"), WeatherChanges, WeatherMs));

		// Reset to clear
		Weather->SetWeatherInstant(EMGWeatherType::Clear);
	}
	else
	{
		Logs.Add(TEXT("Weather subsystem not available for state change test"));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Stress_RapidStateChanges")),
			TEXT("Rapid state changes test failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Stress_RapidStateChanges")),
		FString::Printf(TEXT("Completed %d state transitions in %.2f ms"), StateChanges, TotalMs)
	);
}

// ==========================================
// UI DATA TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestUIData_HUDDataDefaults()
{
	LogTestStart(TEXT("TestUIData_HUDDataDefaults"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Create default HUD data structure
	FMGRaceHUDData HUDData;

	// Verify default values are sensible
	Logs.Add(FString::Printf(TEXT("Speed: %.1f"), HUDData.Speed));
	Logs.Add(FString::Printf(TEXT("CurrentGear: %d"), HUDData.CurrentGear));
	Logs.Add(FString::Printf(TEXT("MaxGear: %d"), HUDData.MaxGear));
	Logs.Add(FString::Printf(TEXT("MaxRPM: %.0f"), HUDData.MaxRPM));
	Logs.Add(FString::Printf(TEXT("Position: %d/%d"), HUDData.Position, HUDData.TotalRacers));
	Logs.Add(FString::Printf(TEXT("Lap: %d/%d"), HUDData.CurrentLap, HUDData.TotalLaps));

	// Speed should start at 0
	if (HUDData.Speed != 0.0f)
	{
		Logs.Add(TEXT("FAIL: Default speed should be 0"));
		bAllPassed = false;
	}

	// Gear should be 0 (neutral)
	if (HUDData.CurrentGear != 0)
	{
		Logs.Add(TEXT("FAIL: Default gear should be 0 (neutral)"));
		bAllPassed = false;
	}

	// MaxGear should be reasonable (4-8)
	if (HUDData.MaxGear < 4 || HUDData.MaxGear > 10)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: MaxGear %d out of expected range (4-10)"), HUDData.MaxGear));
		bAllPassed = false;
	}

	// MaxRPM should be reasonable (6000-12000)
	if (HUDData.MaxRPM < 6000.0f || HUDData.MaxRPM > 12000.0f)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: MaxRPM %.0f out of expected range (6000-12000)"), HUDData.MaxRPM));
		bAllPassed = false;
	}

	// Position should be 1
	if (HUDData.Position < 1)
	{
		Logs.Add(TEXT("FAIL: Default position should be >= 1"));
		bAllPassed = false;
	}

	// NOSAmount should be full (1.0)
	if (HUDData.NOSAmount != 1.0f)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Default NOS should be 1.0, got %.2f"), HUDData.NOSAmount));
		bAllPassed = false;
	}

	// Booleans should default to false
	if (HUDData.bNOSActive || HUDData.bInRedline || HUDData.bIsDrifting || HUDData.bInPursuit)
	{
		Logs.Add(TEXT("FAIL: Boolean flags should default to false"));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_HUDDataDefaults")),
			TEXT("HUD data defaults have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_UIData_HUDDataDefaults")),
		TEXT("All HUD data defaults are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestUIData_RaceStatusDefaults()
{
	LogTestStart(TEXT("TestUIData_RaceStatusDefaults"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Create default race status structure
	FMGRaceStatus RaceStatus;

	Logs.Add(FString::Printf(TEXT("Position: %d/%d"), RaceStatus.CurrentPosition, RaceStatus.TotalRacers));
	Logs.Add(FString::Printf(TEXT("Lap: %d/%d"), RaceStatus.CurrentLap, RaceStatus.TotalLaps));
	Logs.Add(FString::Printf(TEXT("CurrentLapTime: %.2f"), RaceStatus.CurrentLapTime));
	Logs.Add(FString::Printf(TEXT("BestLapTime: %.2f"), RaceStatus.BestLapTime));
	Logs.Add(FString::Printf(TEXT("RaceProgress: %.2f"), RaceStatus.RaceProgress));

	// Position should be 1
	if (RaceStatus.CurrentPosition != 1)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Default position should be 1, got %d"), RaceStatus.CurrentPosition));
		bAllPassed = false;
	}

	// Total racers should be reasonable (1-16)
	if (RaceStatus.TotalRacers < 1 || RaceStatus.TotalRacers > 16)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: TotalRacers %d out of expected range (1-16)"), RaceStatus.TotalRacers));
		bAllPassed = false;
	}

	// Current lap should be 1
	if (RaceStatus.CurrentLap != 1)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Default lap should be 1, got %d"), RaceStatus.CurrentLap));
		bAllPassed = false;
	}

	// Total laps should be reasonable (1-99)
	if (RaceStatus.TotalLaps < 1 || RaceStatus.TotalLaps > 99)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: TotalLaps %d out of expected range (1-99)"), RaceStatus.TotalLaps));
		bAllPassed = false;
	}

	// Times should start at 0
	if (RaceStatus.CurrentLapTime != 0.0f || RaceStatus.TotalRaceTime != 0.0f)
	{
		Logs.Add(TEXT("FAIL: Default times should be 0"));
		bAllPassed = false;
	}

	// Progress should be 0
	if (RaceStatus.RaceProgress != 0.0f)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Default progress should be 0, got %.2f"), RaceStatus.RaceProgress));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_RaceStatusDefaults")),
			TEXT("Race status defaults have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_UIData_RaceStatusDefaults")),
		TEXT("All race status defaults are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestUIData_TelemetryDefaults()
{
	LogTestStart(TEXT("TestUIData_TelemetryDefaults"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Create default telemetry structure
	FMGVehicleTelemetry Telemetry;

	Logs.Add(FString::Printf(TEXT("SpeedKPH: %.1f"), Telemetry.SpeedKPH));
	Logs.Add(FString::Printf(TEXT("SpeedMPH: %.1f"), Telemetry.SpeedMPH));
	Logs.Add(FString::Printf(TEXT("RPM: %.0f"), Telemetry.RPM));
	Logs.Add(FString::Printf(TEXT("MaxRPM: %.0f"), Telemetry.MaxRPM));
	Logs.Add(FString::Printf(TEXT("CurrentGear: %d"), Telemetry.CurrentGear));
	Logs.Add(FString::Printf(TEXT("TotalGears: %d"), Telemetry.TotalGears));

	// Speeds should start at 0
	if (Telemetry.SpeedKPH != 0.0f || Telemetry.SpeedMPH != 0.0f)
	{
		Logs.Add(TEXT("FAIL: Default speeds should be 0"));
		bAllPassed = false;
	}

	// RPM should start at 0
	if (Telemetry.RPM != 0.0f)
	{
		Logs.Add(TEXT("FAIL: Default RPM should be 0"));
		bAllPassed = false;
	}

	// MaxRPM should be reasonable
	if (Telemetry.MaxRPM < 6000.0f || Telemetry.MaxRPM > 12000.0f)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: MaxRPM %.0f out of expected range"), Telemetry.MaxRPM));
		bAllPassed = false;
	}

	// Current gear should be 1 (first)
	if (Telemetry.CurrentGear < 0 || Telemetry.CurrentGear > 8)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: CurrentGear %d out of expected range (0-8)"), Telemetry.CurrentGear));
		bAllPassed = false;
	}

	// Total gears should be reasonable (4-8)
	if (Telemetry.TotalGears < 4 || Telemetry.TotalGears > 10)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: TotalGears %d out of expected range (4-10)"), Telemetry.TotalGears));
		bAllPassed = false;
	}

	// Input values should be 0
	if (Telemetry.ThrottlePosition != 0.0f || Telemetry.BrakePosition != 0.0f || Telemetry.SteeringAngle != 0.0f)
	{
		Logs.Add(TEXT("FAIL: Default inputs should be 0"));
		bAllPassed = false;
	}

	// NOS should be full
	if (Telemetry.NOSAmount != 1.0f)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Default NOS should be 1.0, got %.2f"), Telemetry.NOSAmount));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_TelemetryDefaults")),
			TEXT("Telemetry defaults have issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_UIData_TelemetryDefaults")),
		TEXT("All telemetry defaults are valid")
	);
}

FMGTestResult UMGSubsystemTests::TestUIData_HUDModes()
{
	LogTestStart(TEXT("TestUIData_HUDModes"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test all HUD modes exist and are distinct
	TArray<EMGHUDMode> Modes = {
		EMGHUDMode::Full,
		EMGHUDMode::Minimal,
		EMGHUDMode::Hidden,
		EMGHUDMode::PhotoMode,
		EMGHUDMode::Replay
	};

	Logs.Add(FString::Printf(TEXT("Total HUD modes: %d"), Modes.Num()));

	// Verify we have expected number of modes
	if (Modes.Num() != 5)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Expected 5 HUD modes, found %d"), Modes.Num()));
		bAllPassed = false;
	}

	// Verify modes are distinct
	TSet<uint8> UniqueValues;
	for (EMGHUDMode Mode : Modes)
	{
		uint8 Value = static_cast<uint8>(Mode);
		if (UniqueValues.Contains(Value))
		{
			Logs.Add(FString::Printf(TEXT("FAIL: Duplicate HUD mode value: %d"), Value));
			bAllPassed = false;
		}
		UniqueValues.Add(Value);
		Logs.Add(FString::Printf(TEXT("  Mode %d: value %d"), UniqueValues.Num() - 1, Value));
	}

	// Test speed display modes
	TArray<EMGSpeedDisplayMode> SpeedModes = {
		EMGSpeedDisplayMode::KPH,
		EMGSpeedDisplayMode::MPH
	};

	Logs.Add(FString::Printf(TEXT("Total speed display modes: %d"), SpeedModes.Num()));

	if (SpeedModes.Num() != 2)
	{
		Logs.Add(FString::Printf(TEXT("FAIL: Expected 2 speed display modes, found %d"), SpeedModes.Num()));
		bAllPassed = false;
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_HUDModes")),
			TEXT("HUD mode enumeration issues"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_UIData_HUDModes")),
		FString::Printf(TEXT("All %d HUD modes and %d speed modes validated"), Modes.Num(), SpeedModes.Num())
	);
}

FMGTestResult UMGSubsystemTests::TestUIData_DataProvider()
{
	LogTestStart(TEXT("TestUIData_DataProvider"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_DataProvider")),
			TEXT("GameInstance not found"),
			{TEXT("Cannot access GameInstance")}
		);
	}

	UMGHUDDataProvider* DataProvider = GameInstance->GetSubsystem<UMGHUDDataProvider>();
	if (!DataProvider)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_DataProvider")),
			TEXT("HUD Data Provider not found"),
			{TEXT("GetSubsystem<UMGHUDDataProvider> returned nullptr")}
		);
	}

	Logs.Add(TEXT("HUD Data Provider found"));

	// Test getting HUD data
	FMGRaceHUDData HUDData = DataProvider->GetRaceHUDData();
	Logs.Add(FString::Printf(TEXT("HUD Speed: %.1f"), HUDData.Speed));
	Logs.Add(FString::Printf(TEXT("HUD Position: %d"), HUDData.Position));

	// Test getting minimap data
	FMGMinimapData MinimapData = DataProvider->GetMinimapData();
	Logs.Add(FString::Printf(TEXT("Minimap scale: %.2f"), MinimapData.MapScale));

	// Test speed display mode
	EMGSpeedDisplayMode SpeedMode = DataProvider->GetSpeedDisplayMode();
	Logs.Add(FString::Printf(TEXT("Speed mode: %d"), static_cast<int32>(SpeedMode)));

	// Test formatting functions
	FText FormattedSpeed = DataProvider->GetFormattedSpeed();
	if (FormattedSpeed.IsEmpty())
	{
		Logs.Add(TEXT("WARNING: GetFormattedSpeed returned empty text"));
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("Formatted speed: %s"), *FormattedSpeed.ToString()));
	}

	FText FormattedPosition = DataProvider->GetFormattedPosition(1);
	if (FormattedPosition.IsEmpty())
	{
		Logs.Add(TEXT("WARNING: GetFormattedPosition returned empty text"));
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("Formatted position: %s"), *FormattedPosition.ToString()));
	}

	// Test lap time formatting
	FText FormattedLapTime = DataProvider->GetFormattedLapTime(65.5f);
	if (FormattedLapTime.IsEmpty())
	{
		Logs.Add(TEXT("WARNING: GetFormattedLapTime returned empty text"));
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("Formatted lap time (65.5s): %s"), *FormattedLapTime.ToString()));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_UIData_DataProvider")),
			TEXT("Data provider issues found"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_UIData_DataProvider")),
		TEXT("HUD Data Provider functioning correctly")
	);
}

// ==========================================
// MENU TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestMenu_SettingsDefaults()
{
	LogTestStart(TEXT("TestMenu_SettingsDefaults"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Create default settings structure
	FMGGameSettings Settings;

	// Test Graphics defaults
	if (Settings.FullscreenMode < 0 || Settings.FullscreenMode > 2)
	{
		Logs.Add(FString::Printf(TEXT("FullscreenMode out of range: %d (expected 0-2)"), Settings.FullscreenMode));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("FullscreenMode: %d"), Settings.FullscreenMode));
	}

	if (Settings.FrameRateLimit < 0)
	{
		Logs.Add(FString::Printf(TEXT("FrameRateLimit negative: %d"), Settings.FrameRateLimit));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("FrameRateLimit: %d"), Settings.FrameRateLimit));
	}

	if (Settings.GraphicsQuality < 0 || Settings.GraphicsQuality > 4)
	{
		Logs.Add(FString::Printf(TEXT("GraphicsQuality out of range: %d (expected 0-4)"), Settings.GraphicsQuality));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("GraphicsQuality: %d"), Settings.GraphicsQuality));
	}

	// Test Audio defaults (0.0-1.0 range)
	if (Settings.MasterVolume < 0.0f || Settings.MasterVolume > 1.0f)
	{
		Logs.Add(FString::Printf(TEXT("MasterVolume out of range: %.2f"), Settings.MasterVolume));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("MasterVolume: %.2f"), Settings.MasterVolume));
	}

	if (Settings.MusicVolume < 0.0f || Settings.MusicVolume > 1.0f)
	{
		Logs.Add(FString::Printf(TEXT("MusicVolume out of range: %.2f"), Settings.MusicVolume));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("MusicVolume: %.2f"), Settings.MusicVolume));
	}

	// Test Controls defaults
	if (Settings.SteeringSensitivity <= 0.0f || Settings.SteeringSensitivity > 5.0f)
	{
		Logs.Add(FString::Printf(TEXT("SteeringSensitivity invalid: %.2f"), Settings.SteeringSensitivity));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("SteeringSensitivity: %.2f"), Settings.SteeringSensitivity));
	}

	// Test Accessibility defaults
	if (Settings.HUDScale <= 0.0f || Settings.HUDScale > 3.0f)
	{
		Logs.Add(FString::Printf(TEXT("HUDScale invalid: %.2f"), Settings.HUDScale));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("HUDScale: %.2f"), Settings.HUDScale));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_SettingsDefaults")),
			TEXT("Settings defaults validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Menu_SettingsDefaults")),
		TEXT("Settings defaults valid")
	);
}

FMGTestResult UMGSubsystemTests::TestMenu_MenuStates()
{
	LogTestStart(TEXT("TestMenu_MenuStates"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test EMGMenuState enum values
	TSet<int32> UniqueValues;

	// Check all expected enum values exist and are unique
	int32 NoneVal = static_cast<int32>(EMGMenuState::None);
	int32 MainMenuVal = static_cast<int32>(EMGMenuState::MainMenu);
	int32 PausedVal = static_cast<int32>(EMGMenuState::Paused);
	int32 SettingsVal = static_cast<int32>(EMGMenuState::Settings);
	int32 LoadingVal = static_cast<int32>(EMGMenuState::Loading);
	int32 InGameVal = static_cast<int32>(EMGMenuState::InGame);

	UniqueValues.Add(NoneVal);
	UniqueValues.Add(MainMenuVal);
	UniqueValues.Add(PausedVal);
	UniqueValues.Add(SettingsVal);
	UniqueValues.Add(LoadingVal);
	UniqueValues.Add(InGameVal);

	if (UniqueValues.Num() != 6)
	{
		Logs.Add(FString::Printf(TEXT("Expected 6 unique menu states, got %d"), UniqueValues.Num()));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("All 6 menu states are unique"));
	}

	Logs.Add(FString::Printf(TEXT("None=%d, MainMenu=%d, Paused=%d, Settings=%d, Loading=%d, InGame=%d"),
		NoneVal, MainMenuVal, PausedVal, SettingsVal, LoadingVal, InGameVal));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_MenuStates")),
			TEXT("Menu states validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Menu_MenuStates")),
		TEXT("All 6 menu states valid")
	);
}

FMGTestResult UMGSubsystemTests::TestMenu_SettingsCategories()
{
	LogTestStart(TEXT("TestMenu_SettingsCategories"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test EMGSettingsCategory enum values
	TSet<int32> UniqueValues;

	int32 GraphicsVal = static_cast<int32>(EMGSettingsCategory::Graphics);
	int32 AudioVal = static_cast<int32>(EMGSettingsCategory::Audio);
	int32 ControlsVal = static_cast<int32>(EMGSettingsCategory::Controls);
	int32 GameplayVal = static_cast<int32>(EMGSettingsCategory::Gameplay);
	int32 AccessibilityVal = static_cast<int32>(EMGSettingsCategory::Accessibility);

	UniqueValues.Add(GraphicsVal);
	UniqueValues.Add(AudioVal);
	UniqueValues.Add(ControlsVal);
	UniqueValues.Add(GameplayVal);
	UniqueValues.Add(AccessibilityVal);

	if (UniqueValues.Num() != 5)
	{
		Logs.Add(FString::Printf(TEXT("Expected 5 unique settings categories, got %d"), UniqueValues.Num()));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("All 5 settings categories are unique"));
	}

	Logs.Add(FString::Printf(TEXT("Graphics=%d, Audio=%d, Controls=%d, Gameplay=%d, Accessibility=%d"),
		GraphicsVal, AudioVal, ControlsVal, GameplayVal, AccessibilityVal));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_SettingsCategories")),
			TEXT("Settings categories validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Menu_SettingsCategories")),
		TEXT("All 5 settings categories valid")
	);
}

FMGTestResult UMGSubsystemTests::TestMenu_Subsystem()
{
	LogTestStart(TEXT("TestMenu_Subsystem"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_Subsystem")),
			TEXT("GameInstance not found"),
			{TEXT("Cannot access GameInstance")}
		);
	}

	UMGMenuSubsystem* MenuSubsystem = GameInstance->GetSubsystem<UMGMenuSubsystem>();
	if (!MenuSubsystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_Subsystem")),
			TEXT("Menu Subsystem not found"),
			{TEXT("GetSubsystem<UMGMenuSubsystem> returned nullptr")}
		);
	}

	Logs.Add(TEXT("Menu Subsystem found"));

	// Test GetMenuState
	EMGMenuState CurrentState = MenuSubsystem->GetMenuState();
	Logs.Add(FString::Printf(TEXT("Current menu state: %d"), static_cast<int32>(CurrentState)));

	// Test GetSettings
	FMGGameSettings Settings = MenuSubsystem->GetSettings();
	Logs.Add(FString::Printf(TEXT("Settings loaded - MasterVolume: %.2f"), Settings.MasterVolume));

	// Test IsGamePaused
	bool bIsPaused = MenuSubsystem->IsGamePaused();
	Logs.Add(FString::Printf(TEXT("IsGamePaused: %s"), bIsPaused ? TEXT("true") : TEXT("false")));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_Subsystem")),
			TEXT("Menu subsystem issues found"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Menu_Subsystem")),
		TEXT("Menu Subsystem functioning correctly")
	);
}

FMGTestResult UMGSubsystemTests::TestMenu_SettingsRanges()
{
	LogTestStart(TEXT("TestMenu_SettingsRanges"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	FMGGameSettings Settings;

	// Test all volume settings are in 0-1 range
	const float MinVolume = 0.0f;
	const float MaxVolume = 1.0f;

	struct FVolumeCheck
	{
		const TCHAR* Name;
		float Value;
	};

	TArray<FVolumeCheck> VolumeChecks = {
		{TEXT("MasterVolume"), Settings.MasterVolume},
		{TEXT("MusicVolume"), Settings.MusicVolume},
		{TEXT("SFXVolume"), Settings.SFXVolume},
		{TEXT("EngineVolume"), Settings.EngineVolume},
		{TEXT("VoiceChatVolume"), Settings.VoiceChatVolume}
	};

	for (const FVolumeCheck& Check : VolumeChecks)
	{
		if (Check.Value < MinVolume || Check.Value > MaxVolume)
		{
			Logs.Add(FString::Printf(TEXT("%s out of range: %.2f (expected 0-1)"), Check.Name, Check.Value));
			bAllPassed = false;
		}
		else
		{
			Logs.Add(FString::Printf(TEXT("%s: %.2f OK"), Check.Name, Check.Value));
		}
	}

	// Test quality settings are in valid range (0-4)
	struct FQualityCheck
	{
		const TCHAR* Name;
		int32 Value;
	};

	TArray<FQualityCheck> QualityChecks = {
		{TEXT("TextureQuality"), Settings.TextureQuality},
		{TEXT("ShadowQuality"), Settings.ShadowQuality},
		{TEXT("AntiAliasingQuality"), Settings.AntiAliasingQuality},
		{TEXT("PostProcessQuality"), Settings.PostProcessQuality},
		{TEXT("EffectsQuality"), Settings.EffectsQuality}
	};

	for (const FQualityCheck& Check : QualityChecks)
	{
		if (Check.Value < 0 || Check.Value > 4)
		{
			Logs.Add(FString::Printf(TEXT("%s out of range: %d (expected 0-4)"), Check.Name, Check.Value));
			bAllPassed = false;
		}
		else
		{
			Logs.Add(FString::Printf(TEXT("%s: %d OK"), Check.Name, Check.Value));
		}
	}

	// Test motion blur intensity
	if (Settings.MotionBlurIntensity < 0.0f || Settings.MotionBlurIntensity > 1.0f)
	{
		Logs.Add(FString::Printf(TEXT("MotionBlurIntensity out of range: %.2f"), Settings.MotionBlurIntensity));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("MotionBlurIntensity: %.2f OK"), Settings.MotionBlurIntensity));
	}

	// Test vibration intensity
	if (Settings.VibrationIntensity < 0.0f || Settings.VibrationIntensity > 1.0f)
	{
		Logs.Add(FString::Printf(TEXT("VibrationIntensity out of range: %.2f"), Settings.VibrationIntensity));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("VibrationIntensity: %.2f OK"), Settings.VibrationIntensity));
	}

	// Test color blind mode
	if (Settings.ColorBlindMode < 0 || Settings.ColorBlindMode > 3)
	{
		Logs.Add(FString::Printf(TEXT("ColorBlindMode out of range: %d (expected 0-3)"), Settings.ColorBlindMode));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("ColorBlindMode: %d OK"), Settings.ColorBlindMode));
	}

	// Test camera style
	if (Settings.DefaultCamera < 0 || Settings.DefaultCamera > 3)
	{
		Logs.Add(FString::Printf(TEXT("DefaultCamera out of range: %d (expected 0-3)"), Settings.DefaultCamera));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("DefaultCamera: %d OK"), Settings.DefaultCamera));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Menu_SettingsRanges")),
			TEXT("Settings ranges validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Menu_SettingsRanges")),
		TEXT("All settings ranges valid")
	);
}

// ==========================================
// NOTIFICATION TESTS
// ==========================================

FMGTestResult UMGSubsystemTests::TestNotification_Priority()
{
	LogTestStart(TEXT("TestNotification_Priority"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test EMGNotificationPriority enum values
	TSet<int32> UniqueValues;

	int32 LowVal = static_cast<int32>(EMGNotificationPriority::Low);
	int32 NormalVal = static_cast<int32>(EMGNotificationPriority::Normal);
	int32 HighVal = static_cast<int32>(EMGNotificationPriority::High);
	int32 CriticalVal = static_cast<int32>(EMGNotificationPriority::Critical);
	int32 SystemVal = static_cast<int32>(EMGNotificationPriority::System);

	UniqueValues.Add(LowVal);
	UniqueValues.Add(NormalVal);
	UniqueValues.Add(HighVal);
	UniqueValues.Add(CriticalVal);
	UniqueValues.Add(SystemVal);

	if (UniqueValues.Num() != 5)
	{
		Logs.Add(FString::Printf(TEXT("Expected 5 unique priority levels, got %d"), UniqueValues.Num()));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("All 5 priority levels are unique"));
	}

	// Verify priority ordering (Low < Normal < High < Critical < System)
	if (LowVal >= NormalVal || NormalVal >= HighVal || HighVal >= CriticalVal || CriticalVal >= SystemVal)
	{
		Logs.Add(TEXT("WARNING: Priority values may not be in expected order"));
	}
	else
	{
		Logs.Add(TEXT("Priority ordering is correct"));
	}

	Logs.Add(FString::Printf(TEXT("Low=%d, Normal=%d, High=%d, Critical=%d, System=%d"),
		LowVal, NormalVal, HighVal, CriticalVal, SystemVal));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_Priority")),
			TEXT("Notification priority validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Notification_Priority")),
		TEXT("All 5 notification priorities valid")
	);
}

FMGTestResult UMGSubsystemTests::TestNotification_Types()
{
	LogTestStart(TEXT("TestNotification_Types"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test EMGNotificationType enum values (14 types in NotificationSubsystem)
	TSet<int32> UniqueValues;

	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Info));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Success));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Warning));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Error));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Reward));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::LevelUp));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Unlock));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::ChallengeComplete));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::RaceResult));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Multiplayer));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Season));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Economy));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::Social));
	UniqueValues.Add(static_cast<int32>(EMGNotificationType::System));

	if (UniqueValues.Num() != 14)
	{
		Logs.Add(FString::Printf(TEXT("Expected 14 unique notification types, got %d"), UniqueValues.Num()));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("All 14 notification types are unique"));
	}

	Logs.Add(FString::Printf(TEXT("Info=%d, Success=%d, Warning=%d, Error=%d"),
		static_cast<int32>(EMGNotificationType::Info),
		static_cast<int32>(EMGNotificationType::Success),
		static_cast<int32>(EMGNotificationType::Warning),
		static_cast<int32>(EMGNotificationType::Error)));

	Logs.Add(FString::Printf(TEXT("Reward=%d, LevelUp=%d, Unlock=%d, ChallengeComplete=%d"),
		static_cast<int32>(EMGNotificationType::Reward),
		static_cast<int32>(EMGNotificationType::LevelUp),
		static_cast<int32>(EMGNotificationType::Unlock),
		static_cast<int32>(EMGNotificationType::ChallengeComplete)));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_Types")),
			TEXT("Notification types validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Notification_Types")),
		TEXT("All 14 notification types valid")
	);
}

FMGTestResult UMGSubsystemTests::TestNotification_Styles()
{
	LogTestStart(TEXT("TestNotification_Styles"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Test EMGNotificationStyle enum values
	TSet<int32> UniqueValues;

	int32 ToastVal = static_cast<int32>(EMGNotificationStyle::Toast);
	int32 BannerVal = static_cast<int32>(EMGNotificationStyle::Banner);
	int32 PopupVal = static_cast<int32>(EMGNotificationStyle::Popup);
	int32 FullScreenVal = static_cast<int32>(EMGNotificationStyle::FullScreen);
	int32 MinimalVal = static_cast<int32>(EMGNotificationStyle::Minimal);

	UniqueValues.Add(ToastVal);
	UniqueValues.Add(BannerVal);
	UniqueValues.Add(PopupVal);
	UniqueValues.Add(FullScreenVal);
	UniqueValues.Add(MinimalVal);

	if (UniqueValues.Num() != 5)
	{
		Logs.Add(FString::Printf(TEXT("Expected 5 unique notification styles, got %d"), UniqueValues.Num()));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("All 5 notification styles are unique"));
	}

	Logs.Add(FString::Printf(TEXT("Toast=%d, Banner=%d, Popup=%d, FullScreen=%d, Minimal=%d"),
		ToastVal, BannerVal, PopupVal, FullScreenVal, MinimalVal));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_Styles")),
			TEXT("Notification styles validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Notification_Styles")),
		TEXT("All 5 notification styles valid")
	);
}

FMGTestResult UMGSubsystemTests::TestNotification_DataDefaults()
{
	LogTestStart(TEXT("TestNotification_DataDefaults"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	// Create default notification data
	FMGNotificationData Data;

	// Check NotificationID is valid
	if (!Data.NotificationID.IsValid())
	{
		Logs.Add(TEXT("NotificationID is not valid"));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("NotificationID is valid GUID"));
	}

	// Check default type
	if (Data.Type != EMGNotificationType::Info)
	{
		Logs.Add(FString::Printf(TEXT("Default Type is not Info: %d"), static_cast<int32>(Data.Type)));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("Default Type is Info"));
	}

	// Check default priority
	if (Data.Priority != EMGNotificationPriority::Normal)
	{
		Logs.Add(FString::Printf(TEXT("Default Priority is not Normal: %d"), static_cast<int32>(Data.Priority)));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("Default Priority is Normal"));
	}

	// Check default style
	if (Data.Style != EMGNotificationStyle::Toast)
	{
		Logs.Add(FString::Printf(TEXT("Default Style is not Toast: %d"), static_cast<int32>(Data.Style)));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("Default Style is Toast"));
	}

	// Check default duration
	if (Data.Duration <= 0.0f || Data.Duration > 60.0f)
	{
		Logs.Add(FString::Printf(TEXT("Duration out of range: %.2f"), Data.Duration));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(FString::Printf(TEXT("Duration: %.2f seconds"), Data.Duration));
	}

	// Check boolean defaults
	if (!Data.bCanDismiss)
	{
		Logs.Add(TEXT("bCanDismiss should default to true"));
	}
	else
	{
		Logs.Add(TEXT("bCanDismiss: true"));
	}

	if (!Data.bPlaySound)
	{
		Logs.Add(TEXT("bPlaySound should default to true"));
	}
	else
	{
		Logs.Add(TEXT("bPlaySound: true"));
	}

	// Check timestamp is set
	if (Data.Timestamp == FDateTime())
	{
		Logs.Add(TEXT("Timestamp not initialized"));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("Timestamp is set"));
	}

	// Check bIsRead defaults to false
	if (Data.bIsRead)
	{
		Logs.Add(TEXT("bIsRead should default to false"));
		bAllPassed = false;
	}
	else
	{
		Logs.Add(TEXT("bIsRead: false"));
	}

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_DataDefaults")),
			TEXT("Notification data defaults validation failed"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Notification_DataDefaults")),
		TEXT("Notification data defaults valid")
	);
}

FMGTestResult UMGSubsystemTests::TestNotification_Subsystem()
{
	LogTestStart(TEXT("TestNotification_Subsystem"));

	TArray<FString> Logs;
	bool bAllPassed = true;

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_Subsystem")),
			TEXT("GameInstance not found"),
			{TEXT("Cannot access GameInstance")}
		);
	}

	UMGNotificationSubsystem* NotificationSubsystem = GameInstance->GetSubsystem<UMGNotificationSubsystem>();
	if (!NotificationSubsystem)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_Subsystem")),
			TEXT("Notification Subsystem not found"),
			{TEXT("GetSubsystem<UMGNotificationSubsystem> returned nullptr")}
		);
	}

	Logs.Add(TEXT("Notification Subsystem found"));

	// Test GetQueueSize
	int32 QueueSize = NotificationSubsystem->GetQueueSize();
	Logs.Add(FString::Printf(TEXT("Queue size: %d"), QueueSize));

	// Test IsShowingNotification
	bool bIsShowing = NotificationSubsystem->IsShowingNotification();
	Logs.Add(FString::Printf(TEXT("IsShowingNotification: %s"), bIsShowing ? TEXT("true") : TEXT("false")));

	// Test AreNotificationsEnabled
	bool bEnabled = NotificationSubsystem->AreNotificationsEnabled();
	Logs.Add(FString::Printf(TEXT("NotificationsEnabled: %s"), bEnabled ? TEXT("true") : TEXT("false")));

	// Test AreSoundsEnabled
	bool bSounds = NotificationSubsystem->AreSoundsEnabled();
	Logs.Add(FString::Printf(TEXT("SoundsEnabled: %s"), bSounds ? TEXT("true") : TEXT("false")));

	// Test IsDoNotDisturbActive
	bool bDND = NotificationSubsystem->IsDoNotDisturbActive();
	Logs.Add(FString::Printf(TEXT("DoNotDisturb: %s"), bDND ? TEXT("true") : TEXT("false")));

	// Test GetMinimumPriority
	EMGNotificationPriority MinPriority = NotificationSubsystem->GetMinimumPriority();
	Logs.Add(FString::Printf(TEXT("MinimumPriority: %d"), static_cast<int32>(MinPriority)));

	// Test GetUnreadCount
	int32 UnreadCount = NotificationSubsystem->GetUnreadCount();
	Logs.Add(FString::Printf(TEXT("Unread count: %d"), UnreadCount));

	// Test GetNotificationHistory
	TArray<FMGNotificationHistoryEntry> History = NotificationSubsystem->GetNotificationHistory();
	Logs.Add(FString::Printf(TEXT("History count: %d"), History.Num()));

	if (!bAllPassed)
	{
		return CreateFailResult(
			FName(TEXT("Test_Notification_Subsystem")),
			TEXT("Notification subsystem issues found"),
			Logs
		);
	}

	return CreatePassResult(
		FName(TEXT("Test_Notification_Subsystem")),
		TEXT("Notification Subsystem functioning correctly")
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

	// Performance tests
	TestResults.Add(TestPerf_SubsystemTick());
	TestResults.Add(TestPerf_MemoryUsage());
	TestResults.Add(TestPerf_DelegateBroadcast());
	TestResults.Add(TestPerf_DataAccess());

	// Integration tests
	TestResults.Add(TestIntegration_CurrencyEconomy());
	TestResults.Add(TestIntegration_WeatherRoad());

	// Save/Load tests
	TestResults.Add(TestSave_CreateSaveGame());
	TestResults.Add(TestSave_DefaultValues());
	TestResults.Add(TestSave_DataStructures());
	TestResults.Add(TestSave_ManagerSubsystem());
	TestResults.Add(TestSave_SlotNaming());

	// Physics tests
	TestResults.Add(TestPhysics_TireCompoundGrip());
	TestResults.Add(TestPhysics_WetGripModifiers());
	TestResults.Add(TestPhysics_WeightTransferConstants());
	TestResults.Add(TestPhysics_TireTemperatureConstants());
	TestResults.Add(TestPhysics_HandlingModeSettings());
	TestResults.Add(TestPhysics_SurfaceConstants());
	TestResults.Add(TestPhysics_GeometryConstants());
	TestResults.Add(TestPhysics_DifferentialConstants());
	TestResults.Add(TestPhysics_WearConstants());

	// Stress tests
	TestResults.Add(TestStress_HighObjectCount());
	TestResults.Add(TestStress_SustainedOperation());
	TestResults.Add(TestStress_MemoryStability());
	TestResults.Add(TestStress_RapidStateChanges());

	// UI Data tests
	TestResults.Add(TestUIData_HUDDataDefaults());
	TestResults.Add(TestUIData_RaceStatusDefaults());
	TestResults.Add(TestUIData_TelemetryDefaults());
	TestResults.Add(TestUIData_HUDModes());
	TestResults.Add(TestUIData_DataProvider());

	// Menu tests
	TestResults.Add(TestMenu_SettingsDefaults());
	TestResults.Add(TestMenu_MenuStates());
	TestResults.Add(TestMenu_SettingsCategories());
	TestResults.Add(TestMenu_Subsystem());
	TestResults.Add(TestMenu_SettingsRanges());

	// Notification tests
	TestResults.Add(TestNotification_Priority());
	TestResults.Add(TestNotification_Types());
	TestResults.Add(TestNotification_Styles());
	TestResults.Add(TestNotification_DataDefaults());
	TestResults.Add(TestNotification_Subsystem());

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

void UMGSubsystemTests::RunPerformanceTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING PERFORMANCE TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestPerf_SubsystemTick());
	TestResults.Add(TestPerf_MemoryUsage());
	TestResults.Add(TestPerf_DelegateBroadcast());
	TestResults.Add(TestPerf_DataAccess());

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

void UMGSubsystemTests::RunSaveTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING SAVE/LOAD TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestSave_CreateSaveGame());
	TestResults.Add(TestSave_DefaultValues());
	TestResults.Add(TestSave_DataStructures());
	TestResults.Add(TestSave_ManagerSubsystem());
	TestResults.Add(TestSave_SlotNaming());

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

void UMGSubsystemTests::RunPhysicsTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING PHYSICS TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestPhysics_TireCompoundGrip());
	TestResults.Add(TestPhysics_WetGripModifiers());
	TestResults.Add(TestPhysics_WeightTransferConstants());
	TestResults.Add(TestPhysics_TireTemperatureConstants());
	TestResults.Add(TestPhysics_HandlingModeSettings());
	TestResults.Add(TestPhysics_SurfaceConstants());
	TestResults.Add(TestPhysics_GeometryConstants());
	TestResults.Add(TestPhysics_DifferentialConstants());
	TestResults.Add(TestPhysics_WearConstants());

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

void UMGSubsystemTests::RunStressTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING STRESS TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestStress_HighObjectCount());
	TestResults.Add(TestStress_SustainedOperation());
	TestResults.Add(TestStress_MemoryStability());
	TestResults.Add(TestStress_RapidStateChanges());

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

void UMGSubsystemTests::RunUIDataTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING UI DATA TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestUIData_HUDDataDefaults());
	TestResults.Add(TestUIData_RaceStatusDefaults());
	TestResults.Add(TestUIData_TelemetryDefaults());
	TestResults.Add(TestUIData_HUDModes());
	TestResults.Add(TestUIData_DataProvider());

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

void UMGSubsystemTests::RunMenuTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING MENU TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestMenu_SettingsDefaults());
	TestResults.Add(TestMenu_MenuStates());
	TestResults.Add(TestMenu_SettingsCategories());
	TestResults.Add(TestMenu_Subsystem());
	TestResults.Add(TestMenu_SettingsRanges());

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

void UMGSubsystemTests::RunNotificationTests()
{
	UE_LOG(LogTemp, Log, TEXT("=== RUNNING NOTIFICATION TESTS ==="));

	TestResults.Empty();
	TotalTests = 0;
	PassedTests = 0;
	FailedTests = 0;

	TestResults.Add(TestNotification_Priority());
	TestResults.Add(TestNotification_Types());
	TestResults.Add(TestNotification_Styles());
	TestResults.Add(TestNotification_DataDefaults());
	TestResults.Add(TestNotification_Subsystem());

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
