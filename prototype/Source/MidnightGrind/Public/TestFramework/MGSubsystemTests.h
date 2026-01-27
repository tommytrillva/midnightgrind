// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TestFramework/MGTestFrameworkSubsystem.h"
#include "MGSubsystemTests.generated.h"

class UMGCurrencySubsystem;
class UMGWeatherSubsystem;
class UMGEconomySubsystem;
class UMGVehicleDamageSystem;
class UMGAIDriverProfile;
class UMGSaveManagerSubsystem;
class UMGSaveGame;

/**
 * Subsystem Unit Tests
 * Provides actual test implementations for core subsystems
 * Total Tests: 55
 *
 * Test Categories:
 * - Currency (6): Earning, spending, balance tracking, multipliers
 * - Weather (6): State changes, transitions, road conditions, visibility
 * - Economy (3): Shop purchases, transaction pipeline
 * - Vehicle (6): Damage system, repair, performance degradation
 * - AI (5): Driving states, skills, personality, strategies
 * - Performance (4): Tick time, memory, delegates, data access
 * - Save/Load (5): Save game creation, data structures, slot naming
 * - Physics (9): Tire grip, wet modifiers, weight transfer, handling modes, surface, geometry, differential, wear
 * - Stress (4): High object count, sustained operation, memory stability, rapid state changes
 * - UI Data (5): HUD data, race status, telemetry, HUD modes, data provider
 * - Integration (2): Cross-system verification
 *
 * Console Commands:
 * - MG.RunAllTests - Run all 55 tests
 * - MG.RunCurrencyTests - Run 6 currency subsystem tests
 * - MG.RunWeatherTests - Run 6 weather subsystem tests
 * - MG.RunEconomyTests - Run 3 economy tests
 * - MG.RunVehicleTests - Run 6 vehicle tests
 * - MG.RunAITests - Run 5 AI tests
 * - MG.RunPerformanceTests - Run 4 performance tests
 * - MG.RunSaveTests - Run 5 save/load tests
 * - MG.RunPhysicsTests - Run 9 physics tests
 * - MG.RunStressTests - Run 4 stress tests
 * - MG.RunUIDataTests - Run 5 UI data tests
 * - MG.RunSmokeTests - Run quick smoke tests
 * - MG.PrintTestReport - Print last test report
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSubsystemTests : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// TEST REGISTRATION
	// ==========================================

	/** Register all subsystem tests with the test framework */
	void RegisterAllTests();

	// ==========================================
	// CURRENCY TESTS
	// ==========================================

	/** Test currency earning mechanics */
	UFUNCTION(BlueprintCallable, Category = "Tests|Currency")
	FMGTestResult TestCurrency_EarnGrindCash();

	/** Test currency spending */
	UFUNCTION(BlueprintCallable, Category = "Tests|Currency")
	FMGTestResult TestCurrency_SpendGrindCash();

	/** Test insufficient funds handling */
	UFUNCTION(BlueprintCallable, Category = "Tests|Currency")
	FMGTestResult TestCurrency_InsufficientFunds();

	/** Test race earnings calculation */
	UFUNCTION(BlueprintCallable, Category = "Tests|Currency")
	FMGTestResult TestCurrency_RaceEarnings();

	/** Test earning multipliers */
	UFUNCTION(BlueprintCallable, Category = "Tests|Currency")
	FMGTestResult TestCurrency_Multipliers();

	/** Test balance cannot go negative */
	UFUNCTION(BlueprintCallable, Category = "Tests|Currency")
	FMGTestResult TestCurrency_BalanceNonNegative();

	// ==========================================
	// WEATHER TESTS
	// ==========================================

	/** Test weather state change */
	UFUNCTION(BlueprintCallable, Category = "Tests|Weather")
	FMGTestResult TestWeather_SetWeatherType();

	/** Test weather transition */
	UFUNCTION(BlueprintCallable, Category = "Tests|Weather")
	FMGTestResult TestWeather_Transition();

	/** Test road grip calculation */
	UFUNCTION(BlueprintCallable, Category = "Tests|Weather")
	FMGTestResult TestWeather_RoadGrip();

	/** Test visibility calculation */
	UFUNCTION(BlueprintCallable, Category = "Tests|Weather")
	FMGTestResult TestWeather_Visibility();

	/** Test time of day */
	UFUNCTION(BlueprintCallable, Category = "Tests|Weather")
	FMGTestResult TestWeather_TimeOfDay();

	/** Test weather difficulty rating */
	UFUNCTION(BlueprintCallable, Category = "Tests|Weather")
	FMGTestResult TestWeather_DifficultyRating();

	// ==========================================
	// ECONOMY TESTS
	// ==========================================

	/** Test transaction pipeline */
	UFUNCTION(BlueprintCallable, Category = "Tests|Economy")
	FMGTestResult TestEconomy_TransactionCreate();

	/** Test purchase flow */
	UFUNCTION(BlueprintCallable, Category = "Tests|Economy")
	FMGTestResult TestEconomy_PurchaseFlow();

	/** Test transaction history */
	UFUNCTION(BlueprintCallable, Category = "Tests|Economy")
	FMGTestResult TestEconomy_TransactionHistory();

	// ==========================================
	// VEHICLE TESTS
	// ==========================================

	/** Test damage system initialization */
	UFUNCTION(BlueprintCallable, Category = "Tests|Vehicle")
	FMGTestResult TestVehicle_DamageSystemInit();

	/** Test component damage application */
	UFUNCTION(BlueprintCallable, Category = "Tests|Vehicle")
	FMGTestResult TestVehicle_ComponentDamage();

	/** Test damage resistance calculation */
	UFUNCTION(BlueprintCallable, Category = "Tests|Vehicle")
	FMGTestResult TestVehicle_DamageResistance();

	/** Test repair functionality */
	UFUNCTION(BlueprintCallable, Category = "Tests|Vehicle")
	FMGTestResult TestVehicle_Repair();

	/** Test performance degradation from damage */
	UFUNCTION(BlueprintCallable, Category = "Tests|Vehicle")
	FMGTestResult TestVehicle_PerformanceDegradation();

	/** Test totaled state detection */
	UFUNCTION(BlueprintCallable, Category = "Tests|Vehicle")
	FMGTestResult TestVehicle_TotaledState();

	// ==========================================
	// AI TESTS
	// ==========================================

	/** Test AI driving state enumeration */
	UFUNCTION(BlueprintCallable, Category = "Tests|AI")
	FMGTestResult TestAI_DrivingStates();

	/** Test AI skill parameters validation */
	UFUNCTION(BlueprintCallable, Category = "Tests|AI")
	FMGTestResult TestAI_SkillParams();

	/** Test AI spawn configuration */
	UFUNCTION(BlueprintCallable, Category = "Tests|AI")
	FMGTestResult TestAI_SpawnConfig();

	/** Test AI driver personality system */
	UFUNCTION(BlueprintCallable, Category = "Tests|AI")
	FMGTestResult TestAI_DriverPersonality();

	/** Test AI overtake/defense strategies */
	UFUNCTION(BlueprintCallable, Category = "Tests|AI")
	FMGTestResult TestAI_Strategies();

	// ==========================================
	// PERFORMANCE TESTS
	// ==========================================

	/** Test subsystem tick performance */
	UFUNCTION(BlueprintCallable, Category = "Tests|Performance")
	FMGTestResult TestPerf_SubsystemTick();

	/** Test memory allocations */
	UFUNCTION(BlueprintCallable, Category = "Tests|Performance")
	FMGTestResult TestPerf_MemoryUsage();

	/** Test delegate broadcast overhead */
	UFUNCTION(BlueprintCallable, Category = "Tests|Performance")
	FMGTestResult TestPerf_DelegateBroadcast();

	/** Test data structure access times */
	UFUNCTION(BlueprintCallable, Category = "Tests|Performance")
	FMGTestResult TestPerf_DataAccess();

	// ==========================================
	// SAVE/LOAD TESTS
	// ==========================================

	/** Test save game object creation */
	UFUNCTION(BlueprintCallable, Category = "Tests|Save")
	FMGTestResult TestSave_CreateSaveGame();

	/** Test save data default values */
	UFUNCTION(BlueprintCallable, Category = "Tests|Save")
	FMGTestResult TestSave_DefaultValues();

	/** Test save data structures */
	UFUNCTION(BlueprintCallable, Category = "Tests|Save")
	FMGTestResult TestSave_DataStructures();

	/** Test save manager subsystem */
	UFUNCTION(BlueprintCallable, Category = "Tests|Save")
	FMGTestResult TestSave_ManagerSubsystem();

	/** Test save slot naming */
	UFUNCTION(BlueprintCallable, Category = "Tests|Save")
	FMGTestResult TestSave_SlotNaming();

	// ==========================================
	// INTEGRATION TESTS
	// ==========================================

	/** Test currency + economy integration */
	UFUNCTION(BlueprintCallable, Category = "Tests|Integration")
	FMGTestResult TestIntegration_CurrencyEconomy();

	/** Test weather + road conditions integration */
	UFUNCTION(BlueprintCallable, Category = "Tests|Integration")
	FMGTestResult TestIntegration_WeatherRoad();

	// ==========================================
	// PHYSICS TESTS
	// ==========================================

	/** Test tire compound grip coefficients */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_TireCompoundGrip();

	/** Test wet grip modifiers */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_WetGripModifiers();

	/** Test weight transfer constants */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_WeightTransferConstants();

	/** Test tire temperature constants */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_TireTemperatureConstants();

	/** Test physics handling mode settings */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_HandlingModeSettings();

	/** Test surface detection constants */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_SurfaceConstants();

	/** Test suspension geometry constants */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_GeometryConstants();

	/** Test differential constants */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_DifferentialConstants();

	/** Test wear degradation constants */
	UFUNCTION(BlueprintCallable, Category = "Tests|Physics")
	FMGTestResult TestPhysics_WearConstants();

	// ==========================================
	// STRESS TESTS
	// ==========================================

	/** Test high object allocation stress */
	UFUNCTION(BlueprintCallable, Category = "Tests|Stress")
	FMGTestResult TestStress_HighObjectCount();

	/** Test sustained operation over time */
	UFUNCTION(BlueprintCallable, Category = "Tests|Stress")
	FMGTestResult TestStress_SustainedOperation();

	/** Test memory stability under repeated allocations */
	UFUNCTION(BlueprintCallable, Category = "Tests|Stress")
	FMGTestResult TestStress_MemoryStability();

	/** Test rapid state changes */
	UFUNCTION(BlueprintCallable, Category = "Tests|Stress")
	FMGTestResult TestStress_RapidStateChanges();

	// ==========================================
	// UI DATA TESTS
	// ==========================================

	/** Test HUD data structure defaults */
	UFUNCTION(BlueprintCallable, Category = "Tests|UIData")
	FMGTestResult TestUIData_HUDDataDefaults();

	/** Test race status structure defaults */
	UFUNCTION(BlueprintCallable, Category = "Tests|UIData")
	FMGTestResult TestUIData_RaceStatusDefaults();

	/** Test vehicle telemetry structure defaults */
	UFUNCTION(BlueprintCallable, Category = "Tests|UIData")
	FMGTestResult TestUIData_TelemetryDefaults();

	/** Test HUD mode enumeration */
	UFUNCTION(BlueprintCallable, Category = "Tests|UIData")
	FMGTestResult TestUIData_HUDModes();

	/** Test HUD data provider subsystem */
	UFUNCTION(BlueprintCallable, Category = "Tests|UIData")
	FMGTestResult TestUIData_DataProvider();

	// ==========================================
	// CONSOLE COMMANDS
	// ==========================================

	/** Run all tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunAllTests();

	/** Run currency tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunCurrencyTests();

	/** Run weather tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunWeatherTests();

	/** Run economy tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunEconomyTests();

	/** Run vehicle tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunVehicleTests();

	/** Run AI tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunAITests();

	/** Run performance tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunPerformanceTests();

	/** Run save/load tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunSaveTests();

	/** Run physics tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunPhysicsTests();

	/** Run stress tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunStressTests();

	/** Run UI data tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunUIDataTests();

	/** Run smoke tests via console */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void RunSmokeTests();

	/** Print last test report */
	UFUNCTION(Exec, BlueprintCallable, Category = "Tests|Commands")
	void PrintTestReport();

protected:
	// ==========================================
	// HELPERS
	// ==========================================

	UMGCurrencySubsystem* GetCurrencySubsystem() const;
	UMGWeatherSubsystem* GetWeatherSubsystem() const;
	UMGTestFrameworkSubsystem* GetTestFramework() const;

	void LogTestStart(const FString& TestName);
	void LogTestResult(const FMGTestResult& Result);

	FMGTestResult CreatePassResult(FName TestID, const FString& Message);
	FMGTestResult CreateFailResult(FName TestID, const FString& Message, const TArray<FString>& Logs);

private:
	UPROPERTY()
	TArray<FMGTestResult> TestResults;

	int32 TotalTests = 0;
	int32 PassedTests = 0;
	int32 FailedTests = 0;
};
