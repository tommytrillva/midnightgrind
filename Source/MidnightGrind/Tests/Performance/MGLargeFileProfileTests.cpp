// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "Player/MGPlayerController.h"
#include "AI/MGAIRacerController.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMemory.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Performance Profile Test: Vehicle Movement Component
 * Profiles MG_VHCL_MovementComponent.cpp (4,031 lines)
 * Focus: Physics calculation performance, memory usage
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGVehicleMovementComponentProfileTest,
	"MidnightGrind.Performance.Profile.VehicleMovementComponent",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGVehicleMovementComponentProfileTest::RunTest(const FString& Parameters)
{
	// Create test world and vehicle
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World created"), World))
		return false;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UMGVehicleMovementComponent* MovementComp = NewObject<UMGVehicleMovementComponent>(GameInstance);

	if (!TestNotNull(TEXT("MovementComponent created"), MovementComp))
		return false;

	// Profile: Component initialization
	double StartTime = FPlatformTime::Seconds();
	MovementComp->Initialize();
	double InitTime = FPlatformTime::Seconds() - StartTime;

	TestTrue(TEXT("Component initializes quickly (<10ms)"), InitTime < 0.01);
	AddInfo(FString::Printf(TEXT("Initialization time: %.3f ms"), InitTime * 1000.0));

	// Profile: Physics tick simulation (100 frames)
	const int32 TickCount = 100;
	const float MGDeltaTime = 1.0f / 60.0f; // 60 FPS

	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < TickCount; ++i)
	{
		MovementComp->TickComponent(DeltaTime, LEVELTICK_All, nullptr);
	}
	double TickTime = FPlatformTime::Seconds() - StartTime;
	double AvgTickTimeMs = (TickTime * 1000.0) / TickCount;

	TestTrue(TEXT("Average tick time <1ms (60 FPS target)"), AvgTickTimeMs < 1.0);
	AddInfo(FString::Printf(TEXT("Total tick time: %.3f ms"), TickTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Average tick time: %.3f ms"), AvgTickTimeMs));
	AddInfo(FString::Printf(TEXT("Estimated FPS impact: %.1f FPS"), 1000.0 / AvgTickTimeMs));

	// Profile: Memory footprint
	SIZE_T ComponentSize = MovementComp->GetClass()->GetStructureSize();
	AddInfo(FString::Printf(TEXT("Component memory size: %llu bytes"), ComponentSize));

	// Profile: Physics calculation hotspots
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		MovementComp->UpdateEngineForce(DeltaTime);
	}
	double EngineForceTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Engine force calc (1000x): %.3f ms (%.3f ms avg)"),
		EngineForceTime, EngineForceTime / 1000.0));

	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		MovementComp->UpdateSuspension(DeltaTime);
	}
	double SuspensionTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Suspension calc (1000x): %.3f ms (%.3f ms avg)"),
		SuspensionTime, SuspensionTime / 1000.0));

	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		MovementComp->UpdateTireForces(DeltaTime);
	}
	double TireTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Tire forces calc (1000x): %.3f ms (%.3f ms avg)"),
		TireTime, TireTime / 1000.0));

	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		MovementComp->UpdateAerodynamics(DeltaTime);
	}
	double AeroTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Aerodynamics calc (1000x): %.3f ms (%.3f ms avg)"),
		AeroTime, AeroTime / 1000.0));

	// Identify hotspot
	TArray<TPair<FString, double>> Timings;
	Timings.Add(TPair<FString, double>(TEXT("Engine Force"), EngineForceTime));
	Timings.Add(TPair<FString, double>(TEXT("Suspension"), SuspensionTime));
	Timings.Add(TPair<FString, double>(TEXT("Tire Forces"), TireTime));
	Timings.Add(TPair<FString, double>(TEXT("Aerodynamics"), AeroTime));

	Timings.Sort([](const TPair<FString, double>& A, const TPair<FString, double>& B) {
		return A.Value > B.Value;
	});

	AddInfo(TEXT("===== HOTSPOT RANKING ====="));
	for (int32 i = 0; i < Timings.Num(); ++i)
	{
		AddInfo(FString::Printf(TEXT("%d. %s: %.3f ms"), i + 1, *Timings[i].Key, Timings[i].Value));
	}

	World->DestroyWorld(false);
	return true;
}

/**
 * Performance Profile Test: Player Controller
 * Profiles MGPlayerController.cpp (3,013 lines)
 * Focus: Input processing, UI updates, subsystem coordination
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGPlayerControllerProfileTest,
	"MidnightGrind.Performance.Profile.PlayerController",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGPlayerControllerProfileTest::RunTest(const FString& Parameters)
{
	// Create test world and controller
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World created"), World))
		return false;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	AMGPlayerController* Controller = World->SpawnActor<AMGPlayerController>();

	if (!TestNotNull(TEXT("PlayerController created"), Controller))
		return false;

	// Profile: Input processing throughput
	const int32 InputCount = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < InputCount; ++i)
	{
		Controller->ProcessPlayerInput(1.0f / 60.0f, false);
	}

	double InputTime = FPlatformTime::Seconds() - StartTime;
	double AvgInputTimeMs = (InputTime * 1000.0) / InputCount;

	TestTrue(TEXT("Input processing <0.1ms avg"), AvgInputTimeMs < 0.1);
	AddInfo(FString::Printf(TEXT("Total input processing: %.3f ms"), InputTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Average input time: %.6f ms"), AvgInputTimeMs));
	AddInfo(FString::Printf(TEXT("Input throughput: %.0f inputs/sec"), InputCount / InputTime));

	// Profile: UI update cycles
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 100; ++i)
	{
		Controller->UpdateHUDInfo();
	}
	double UIUpdateTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("UI updates (100x): %.3f ms (%.3f ms avg)"),
		UIUpdateTime, UIUpdateTime / 100.0));

	// Profile: Subsystem coordination
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		Controller->CoordinateSubsystems();
	}
	double CoordinationTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Subsystem coordination (1000x): %.3f ms (%.6f ms avg)"),
		CoordinationTime, CoordinationTime / 1000.0));

	// Memory analysis
	SIZE_T ControllerSize = Controller->GetClass()->GetStructureSize();
	AddInfo(FString::Printf(TEXT("Controller memory size: %llu bytes"), ControllerSize));

	World->DestroyWorld(false);
	return true;
}

/**
 * Performance Profile Test: AI Racer Controller
 * Profiles MGAIRacerController.cpp (2,237 lines)
 * Focus: AI decision making, pathfinding, behavior computation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIRacerControllerProfileTest,
	"MidnightGrind.Performance.Profile.AIRacerController",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIRacerControllerProfileTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World created"), World))
		return false;

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	AMGAIRacerController* AIController = World->SpawnActor<AMGAIRacerController>();

	if (!TestNotNull(TEXT("AIRacerController created"), AIController))
		return false;

	// Profile: AI decision making loop
	const int32 DecisionCount = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < DecisionCount; ++i)
	{
		AIController->MakeRacingDecisions(1.0f / 60.0f);
	}

	double DecisionTime = FPlatformTime::Seconds() - StartTime;
	double AvgDecisionTimeMs = (DecisionTime * 1000.0) / DecisionCount;

	TestTrue(TEXT("AI decision making <0.5ms avg"), AvgDecisionTimeMs < 0.5);
	AddInfo(FString::Printf(TEXT("Total decision time: %.3f ms"), DecisionTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Average decision time: %.3f ms"), AvgDecisionTimeMs));
	AddInfo(FString::Printf(TEXT("AI updates per second: %.0f"), DecisionCount / DecisionTime));

	// Profile: Pathfinding
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 100; ++i)
	{
		AIController->UpdatePathfinding();
	}
	double PathfindingTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Pathfinding (100x): %.3f ms (%.3f ms avg)"),
		PathfindingTime, PathfindingTime / 100.0));

	// Profile: Opponent awareness
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		AIController->UpdateOpponentAwareness();
	}
	double AwarenessTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Opponent awareness (1000x): %.3f ms (%.3f ms avg)"),
		AwarenessTime, AwarenessTime / 1000.0));

	// Profile: Racing line calculation
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 100; ++i)
	{
		AIController->CalculateRacingLine();
	}
	double RacingLineTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Racing line calc (100x): %.3f ms (%.3f ms avg)"),
		RacingLineTime, RacingLineTime / 100.0));

	// Identify AI hotspots
	TArray<TPair<FString, double>> AITimings;
	AITimings.Add(TPair<FString, double>(TEXT("Decision Making"), DecisionTime * 1000.0));
	AITimings.Add(TPair<FString, double>(TEXT("Pathfinding"), PathfindingTime));
	AITimings.Add(TPair<FString, double>(TEXT("Opponent Awareness"), AwarenessTime));
	AITimings.Add(TPair<FString, double>(TEXT("Racing Line"), RacingLineTime));

	AITimings.Sort([](const TPair<FString, double>& A, const TPair<FString, double>& B) {
		return A.Value > B.Value;
	});

	AddInfo(TEXT("===== AI HOTSPOT RANKING ====="));
	for (int32 i = 0; i < AITimings.Num(); ++i)
	{
		AddInfo(FString::Printf(TEXT("%d. %s: %.3f ms"), i + 1, *AITimings[i].Key, AITimings[i].Value));
	}

	// Memory analysis
	SIZE_T AISize = AIController->GetClass()->GetStructureSize();
	AddInfo(FString::Printf(TEXT("AI Controller memory size: %llu bytes"), AISize));

	World->DestroyWorld(false);
	return true;
}

/**
 * Performance Profile Test: Comprehensive Subsystem Tests
 * Profiles all major subsystems under realistic load
 * Focus: Real-world scenario performance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGComprehensiveSubsystemProfileTest,
	"MidnightGrind.Performance.Profile.ComprehensiveSubsystems",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGComprehensiveSubsystemProfileTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Profile memory usage before subsystem creation
	FPlatformMemoryStats MemStatsBefore = FPlatformMemory::GetStats();
	int64 MemoryBefore = MemStatsBefore.UsedPhysical;

	// Create all catalog subsystems
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);

	// Setup large data sets (realistic production scale)
	TArray<FMGVehicleData> Vehicles = FMGTestDataFactory::CreateTestVehicleArray(200); // 200 vehicles
	TArray<FMGPartData> Parts = FMGTestDataFactory::CreateTestPartArray(1000); // 1000 parts

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, Vehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, Parts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	// Profile: Subsystem initialization with large datasets
	double StartTime = FPlatformTime::Seconds();
	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);
	double InitTime = FPlatformTime::Seconds() - StartTime;

	TestTrue(TEXT("Large dataset initialization <5 seconds"), InitTime < 5.0);
	AddInfo(FString::Printf(TEXT("Large dataset initialization: %.3f ms"), InitTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Vehicles loaded: %d"), Vehicles.Num()));
	AddInfo(FString::Printf(TEXT("Parts loaded: %d"), Parts.Num()));

	// Profile memory usage after initialization
	FPlatformMemoryStats MemStatsAfter = FPlatformMemory::GetStats();
	int64 MemoryAfter = MemStatsAfter.UsedPhysical;
	int64 MemoryDelta = MemoryAfter - MemoryBefore;

	AddInfo(FString::Printf(TEXT("Memory usage: %.2f MB"), MemoryDelta / (1024.0 * 1024.0)));
	AddInfo(FString::Printf(TEXT("Avg memory per vehicle: %.2f KB"),
		(MemoryDelta / Vehicles.Num()) / 1024.0));

	// Profile: Mixed workload simulation (1000 operations)
	const int32 OperationCount = 1000;
	StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < OperationCount; ++i)
	{
		// Simulate realistic mixed operations
		int32 VehicleIndex = i % Vehicles.Num();
		int32 PartIndex = i % Parts.Num();

		// Vehicle lookup
		FMGVehicleData VehicleData;
		VehicleCatalog->GetVehicleData(Vehicles[VehicleIndex].VehicleID, VehicleData);

		// Part lookup
		FMGPartData PartData;
		PartsCatalog->GetPartData(Parts[PartIndex].PartID, PartData);

		// Filtering operation every 10 iterations
		if (i % 10 == 0)
		{
			TArray<FMGVehicleData> ClassVehicles = VehicleCatalog->GetVehiclesByClass(
				EMGVehicleClass::Sport);
		}
	}

	double MixedWorkloadTime = FPlatformTime::Seconds() - StartTime;
	double AvgOpTimeMs = (MixedWorkloadTime * 1000.0) / OperationCount;

	TestTrue(TEXT("Mixed workload avg operation <0.1ms"), AvgOpTimeMs < 0.1);
	AddInfo(FString::Printf(TEXT("Mixed workload total: %.3f ms"), MixedWorkloadTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Average operation time: %.6f ms"), AvgOpTimeMs));
	AddInfo(FString::Printf(TEXT("Operations per second: %.0f"), OperationCount / MixedWorkloadTime));

	// Profile: Concurrent access simulation
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 100; ++i)
	{
		// Simulate 10 concurrent systems accessing catalogs
		for (int32 j = 0; j < 10; ++j)
		{
			VehicleCatalog->GetVehicleBasePrice(Vehicles[j % Vehicles.Num()].VehicleID);
			PartsCatalog->GetPartBasePrice(Parts[j % Parts.Num()].PartID);
		}
	}
	double ConcurrentTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;
	AddInfo(FString::Printf(TEXT("Concurrent access (1000 ops): %.3f ms"), ConcurrentTime));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
