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

/**
 * Subsystem Unit Tests
 * Provides actual test implementations for core subsystems
 *
 * Test Categories:
 * - Currency: Earning, spending, balance tracking
 * - Weather: State changes, transitions, road conditions
 * - Economy: Shop purchases, transaction pipeline
 * - Vehicle: Physics calculations, damage states
 *
 * Console Commands:
 * - MG.RunAllTests - Run all registered tests
 * - MG.RunCurrencyTests - Run currency subsystem tests
 * - MG.RunWeatherTests - Run weather subsystem tests
 * - MG.RunSmokeTests - Run quick smoke tests
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
	// INTEGRATION TESTS
	// ==========================================

	/** Test currency + economy integration */
	UFUNCTION(BlueprintCallable, Category = "Tests|Integration")
	FMGTestResult TestIntegration_CurrencyEconomy();

	/** Test weather + road conditions integration */
	UFUNCTION(BlueprintCallable, Category = "Tests|Integration")
	FMGTestResult TestIntegration_WeatherRoad();

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
