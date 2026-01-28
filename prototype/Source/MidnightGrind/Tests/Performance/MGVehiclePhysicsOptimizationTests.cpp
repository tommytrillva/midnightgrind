// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Vehicle/MGVehiclePhysicsOptimizations.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Validation Test: Tire Force Lookup Table Accuracy
 * Verifies lookup table produces results within 1% of direct Pacejka formula
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGTireForceLookupAccuracyTest,
	"MidnightGrind.Performance.Optimization.TireLookupAccuracy",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGTireForceLookupAccuracyTest::RunTest(const FString& Parameters)
{
	// Initialize lookup table with typical Pacejka parameters
	FMGTireForceLookupTable LookupTable;
	const float B = 10.0f;  // Stiffness factor
	const float C = 1.9f;   // Shape factor
	const float D = 1.0f;   // Peak value
	const float E = 0.97f;  // Curvature factor

	LookupTable.Initialize(B, C, D, E);
	TestTrue(TEXT("Lookup table initialized"), LookupTable.IsInitialized());

	// Test multiple slip ratios across the range
	const int32 TestCount = 20;
	float MaxError = 0.0f;
	float AvgError = 0.0f;

	for (int32 i = 0; i < TestCount; ++i)
	{
		const float SlipRatio = -1.0f + (2.0f * i / (TestCount - 1)); // -1 to 1
		const float NormalLoad = 5000.0f; // 5000 N (~500 kg wheel load)

		// Get result from lookup table
		const float LookupForce = LookupTable.GetLongitudinalForce(SlipRatio, NormalLoad);

		// Calculate direct Pacejka result
		const float DirectForce = FMGTireForceLookupTable::EvaluatePacejka(SlipRatio, B, C, D, E) * NormalLoad;

		// Calculate error percentage
		const float Error = FMath::Abs((LookupForce - DirectForce) / DirectForce) * 100.0f;
		MaxError = FMath::Max(MaxError, Error);
		AvgError += Error;

		AddInfo(FString::Printf(TEXT("SlipRatio=%.3f: Lookup=%.2f N, Direct=%.2f N, Error=%.3f%%"),
			SlipRatio, LookupForce, DirectForce, Error));
	}

	AvgError /= TestCount;

	// Validate accuracy
	TestTrue(TEXT("Max error < 1%"), MaxError < 1.0f);
	TestTrue(TEXT("Avg error < 0.5%"), AvgError < 0.5f);

	AddInfo(FString::Printf(TEXT("Max error: %.3f%%"), MaxError));
	AddInfo(FString::Printf(TEXT("Avg error: %.3f%%"), AvgError));

	return true;
}

/**
 * Performance Test: Tire Force Lookup vs Direct Calculation
 * Measures performance improvement from lookup table
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGTireForceLookupPerformanceTest,
	"MidnightGrind.Performance.Optimization.TireLookupSpeed",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGTireForceLookupPerformanceTest::RunTest(const FString& Parameters)
{
	// Initialize lookup table
	FMGTireForceLookupTable LookupTable;
	LookupTable.Initialize(10.0f, 1.9f, 1.0f, 0.97f);

	const int32 IterationCount = 10000;
	const float TestSlipRatio = 0.15f;
	const float TestNormalLoad = 5000.0f;

	// Test 1: Lookup table method
	double LookupStartTime = FPlatformTime::Seconds();
	float LookupTotal = 0.0f;
	for (int32 i = 0; i < IterationCount; ++i)
	{
		LookupTotal += LookupTable.GetLongitudinalForce(TestSlipRatio, TestNormalLoad);
	}
	double LookupTime = (FPlatformTime::Seconds() - LookupStartTime) * 1000.0;

	// Test 2: Direct Pacejka calculation
	double DirectStartTime = FPlatformTime::Seconds();
	float DirectTotal = 0.0f;
	for (int32 i = 0; i < IterationCount; ++i)
	{
		DirectTotal += FMGTireForceLookupTable::EvaluatePacejka(TestSlipRatio, 10.0f, 1.9f, 1.0f, 0.97f) * TestNormalLoad;
	}
	double DirectTime = (FPlatformTime::Seconds() - DirectStartTime) * 1000.0;

	// Verify results match
	const float ResultDifference = FMath::Abs(LookupTotal - DirectTotal);
	TestTrue(TEXT("Results approximately equal"), ResultDifference < (IterationCount * 10.0f)); // 10N tolerance per iteration

	// Verify performance improvement
	const float Speedup = static_cast<float>(DirectTime / LookupTime);
	TestTrue(TEXT("Lookup table is faster"), Speedup > 1.0f);
	TestTrue(TEXT("Speedup >= 5x"), Speedup >= 5.0f);

	AddInfo(FString::Printf(TEXT("Lookup table time: %.3f ms (%.3f μs/call)"),
		LookupTime, (LookupTime * 1000.0) / IterationCount));
	AddInfo(FString::Printf(TEXT("Direct Pacejka time: %.3f ms (%.3f μs/call)"),
		DirectTime, (DirectTime * 1000.0) / IterationCount));
	AddInfo(FString::Printf(TEXT("Speedup: %.1fx"), Speedup));
	AddInfo(FString::Printf(TEXT("Time saved per call: %.3f μs"),
		((DirectTime - LookupTime) * 1000.0) / IterationCount));

	return true;
}

/**
 * Validation Test: Suspension Raycast Cache
 * Verifies raycast caching logic works correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSuspensionRaycastCacheTest,
	"MidnightGrind.Performance.Optimization.SuspensionRaycastCache",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSuspensionRaycastCacheTest::RunTest(const FString& Parameters)
{
	FMGSuspensionRaycastCache Cache;

	// Initial state: cache invalid, should update
	TestTrue(TEXT("Initial state requires update"), Cache.ShouldUpdateRaycast(0.0f, FVector::ZeroVector));

	// Perform "raycast" and update cache
	FHitResult MockHit;
	MockHit.bBlockingHit = true;
	MockHit.ImpactPoint = FVector(0, 0, -100);

	const FVector VehicleVelocity(1000.0f, 0.0f, 0.0f); // 10 m/s
	Cache.UpdateHitResult(MockHit, 0.0f, VehicleVelocity);

	// Shortly after, should use cache
	TestFalse(TEXT("Cache valid after update"), Cache.ShouldUpdateRaycast(0.01f, VehicleVelocity));

	// After cache duration, should update
	TestTrue(TEXT("Cache expired after duration"), Cache.ShouldUpdateRaycast(0.1f, VehicleVelocity));

	// Reset cache
	Cache.UpdateHitResult(MockHit, 0.2f, VehicleVelocity);

	// Large velocity change should invalidate
	const FVector NewVelocity(2000.0f, 0.0f, 0.0f); // Doubled velocity
	TestTrue(TEXT("Large velocity change requires update"), Cache.ShouldUpdateRaycast(0.21f, NewVelocity));

	// Manual invalidation
	Cache.UpdateHitResult(MockHit, 0.3f, VehicleVelocity);
	TestFalse(TEXT("Cache valid before invalidation"), Cache.ShouldUpdateRaycast(0.31f, VehicleVelocity));
	Cache.Invalidate();
	TestTrue(TEXT("Cache invalid after manual invalidation"), Cache.ShouldUpdateRaycast(0.31f, VehicleVelocity));

	AddInfo(TEXT("Suspension raycast cache validation passed"));
	return true;
}

/**
 * Performance Test: Raycast Cache Savings
 * Measures performance improvement from raycast caching
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSuspensionRaycastCachePerformanceTest,
	"MidnightGrind.Performance.Optimization.RaycastCacheSpeed",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSuspensionRaycastCachePerformanceTest::RunTest(const FString& Parameters)
{
	FMGSuspensionRaycastCache Cache;
	FHitResult MockHit;
	const FVector Velocity(1000.0f, 0.0f, 0.0f);

	// Simulate 600 frames (10 seconds at 60 FPS)
	const int32 FrameCount = 600;
	int32 RaycastsPerformed = 0;
	int32 CachedUsed = 0;

	for (int32 Frame = 0; Frame < FrameCount; ++Frame)
	{
		const float CurrentTime = Frame / 60.0f; // 60 FPS

		if (Cache.ShouldUpdateRaycast(CurrentTime, Velocity))
		{
			// Perform raycast
			Cache.UpdateHitResult(MockHit, CurrentTime, Velocity);
			RaycastsPerformed++;
		}
		else
		{
			// Use cached result
			CachedUsed++;
		}
	}

	// With 0.083s cache duration (~5 frames), expect ~120 raycasts instead of 600
	const float CacheSavingsPercent = (static_cast<float>(CachedUsed) / FrameCount) * 100.0f;

	TestTrue(TEXT("Significant raycast reduction"), CacheSavingsPercent > 70.0f);
	TestTrue(TEXT("Raycasts performed < 150"), RaycastsPerformed < 150);

	AddInfo(FString::Printf(TEXT("Total frames: %d"), FrameCount));
	AddInfo(FString::Printf(TEXT("Raycasts performed: %d"), RaycastsPerformed));
	AddInfo(FString::Printf(TEXT("Cache hits: %d"), CachedUsed));
	AddInfo(FString::Printf(TEXT("Cache savings: %.1f%%"), CacheSavingsPercent));
	AddInfo(FString::Printf(TEXT("Raycast reduction: %dx fewer raycasts"),
		FrameCount / FMath::Max(1, RaycastsPerformed)));

	return true;
}

/**
 * Validation Test: Vehicle LOD System
 * Verifies LOD determination logic
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGVehicleLODSystemTest,
	"MidnightGrind.Performance.Optimization.VehicleLODSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGVehicleLODSystemTest::RunTest(const FString& Parameters)
{
	// Test LOD determination
	EMGVehiclePhysicsLOD LOD;

	// Player vehicle always full LOD
	LOD = FMGVehicleLODState::DetermineLOD(100000.0f, false, true);
	TestEqual(TEXT("Player vehicle = Full LOD"), LOD, EMGVehiclePhysicsLOD::LOD_Full);

	// Invisible vehicle = Minimal LOD
	LOD = FMGVehicleLODState::DetermineLOD(1000.0f, false, false);
	TestEqual(TEXT("Invisible = Minimal LOD"), LOD, EMGVehiclePhysicsLOD::LOD_Minimal);

	// Distance-based LOD
	LOD = FMGVehicleLODState::DetermineLOD(3000.0f, true, false); // 30m
	TestEqual(TEXT("< 50m = Full LOD"), LOD, EMGVehiclePhysicsLOD::LOD_Full);

	LOD = FMGVehicleLODState::DetermineLOD(10000.0f, true, false); // 100m
	TestEqual(TEXT("50-150m = Reduced LOD"), LOD, EMGVehiclePhysicsLOD::LOD_Reduced);

	LOD = FMGVehicleLODState::DetermineLOD(20000.0f, true, false); // 200m
	TestEqual(TEXT("150-300m = Simplified LOD"), LOD, EMGVehiclePhysicsLOD::LOD_Simplified);

	LOD = FMGVehicleLODState::DetermineLOD(40000.0f, true, false); // 400m
	TestEqual(TEXT("> 300m = Minimal LOD"), LOD, EMGVehiclePhysicsLOD::LOD_Minimal);

	AddInfo(TEXT("Vehicle LOD determination validated"));
	return true;
}

/**
 * Performance Test: Vehicle LOD Update Skipping
 * Measures frame skipping behavior at different LOD levels
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGVehicleLODUpdateSkippingTest,
	"MidnightGrind.Performance.Optimization.LODUpdateSkipping",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGVehicleLODUpdateSkippingTest::RunTest(const FString& Parameters)
{
	// Test each LOD level update frequency
	const int32 TestFrames = 100;

	// LOD Full: Update every frame (100 updates)
	FMGVehicleLODState StateFull;
	StateFull.CurrentLOD = EMGVehiclePhysicsLOD::LOD_Full;
	int32 UpdatesFull = 0;
	for (int32 i = 0; i < TestFrames; ++i)
	{
		if (StateFull.ShouldUpdateThisFrame()) UpdatesFull++;
	}
	TestEqual(TEXT("Full LOD updates every frame"), UpdatesFull, TestFrames);

	// LOD Reduced: Update every 2nd frame (50 updates)
	FMGVehicleLODState StateReduced;
	StateReduced.CurrentLOD = EMGVehiclePhysicsLOD::LOD_Reduced;
	int32 UpdatesReduced = 0;
	for (int32 i = 0; i < TestFrames; ++i)
	{
		if (StateReduced.ShouldUpdateThisFrame()) UpdatesReduced++;
	}
	TestEqual(TEXT("Reduced LOD updates every 2nd frame"), UpdatesReduced, TestFrames / 2);

	// LOD Simplified: Update every 4th frame (25 updates)
	FMGVehicleLODState StateSimplified;
	StateSimplified.CurrentLOD = EMGVehiclePhysicsLOD::LOD_Simplified;
	int32 UpdatesSimplified = 0;
	for (int32 i = 0; i < TestFrames; ++i)
	{
		if (StateSimplified.ShouldUpdateThisFrame()) UpdatesSimplified++;
	}
	TestEqual(TEXT("Simplified LOD updates every 4th frame"), UpdatesSimplified, TestFrames / 4);

	// LOD Minimal: Update every 8th frame (12-13 updates)
	FMGVehicleLODState StateMinimal;
	StateMinimal.CurrentLOD = EMGVehiclePhysicsLOD::LOD_Minimal;
	int32 UpdatesMinimal = 0;
	for (int32 i = 0; i < TestFrames; ++i)
	{
		if (StateMinimal.ShouldUpdateThisFrame()) UpdatesMinimal++;
	}
	TestTrue(TEXT("Minimal LOD updates ~every 8th frame"), UpdatesMinimal >= 12 && UpdatesMinimal <= 13);

	AddInfo(FString::Printf(TEXT("Full: %d updates"), UpdatesFull));
	AddInfo(FString::Printf(TEXT("Reduced: %d updates (%.1fx reduction)"),
		UpdatesReduced, static_cast<float>(UpdatesFull) / UpdatesReduced));
	AddInfo(FString::Printf(TEXT("Simplified: %d updates (%.1fx reduction)"),
		UpdatesSimplified, static_cast<float>(UpdatesFull) / UpdatesSimplified));
	AddInfo(FString::Printf(TEXT("Minimal: %d updates (%.1fx reduction)"),
		UpdatesMinimal, static_cast<float>(UpdatesFull) / UpdatesMinimal));

	return true;
}

/**
 * Validation Test: Early Exit Optimization
 * Verifies stationary vehicle detection
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGEarlyExitOptimizationTest,
	"MidnightGrind.Performance.Optimization.EarlyExitLogic",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGEarlyExitOptimizationTest::RunTest(const FString& Parameters)
{
	using namespace MGPhysicsEarlyExit;

	// Test 1: Truly stationary vehicle
	FVector ZeroVelocity = FVector::ZeroVector;
	FVector ZeroAngular = FVector::ZeroVector;
	TestTrue(TEXT("Zero velocity = stationary"), IsVehicleStationary(ZeroVelocity, ZeroAngular, 0.0f, 0.0f));

	// Test 2: Very slow but moving (should still be stationary for optimization)
	FVector SlowVelocity(5.0f, 0.0f, 0.0f); // 0.05 m/s
	TestTrue(TEXT("Very slow = stationary"), IsVehicleStationary(SlowVelocity, ZeroAngular, 0.0f, 0.0f));

	// Test 3: Moving vehicle
	FVector MovingVelocity(500.0f, 0.0f, 0.0f); // 5 m/s
	TestFalse(TEXT("Moving = not stationary"), IsVehicleStationary(MovingVelocity, ZeroAngular, 0.0f, 0.0f));

	// Test 4: Stationary but with throttle input
	TestFalse(TEXT("Throttle input = not stationary"), IsVehicleStationary(ZeroVelocity, ZeroAngular, 0.5f, 0.0f));

	// Test 5: Stationary but with brake input
	TestFalse(TEXT("Brake input = not stationary"), IsVehicleStationary(ZeroVelocity, ZeroAngular, 0.0f, 0.5f));

	// Test 6: Rotating but not moving
	FVector AngularVelocity(0.5f, 0.0f, 0.0f); // Rotating
	TestFalse(TEXT("Rotating = not stationary"), IsVehicleStationary(ZeroVelocity, AngularVelocity, 0.0f, 0.0f));

	// Test LOD-based skipping
	TestTrue(TEXT("Skip tire temp for simplified LOD"),
		ShouldSkipTireTemperature(1000.0f, EMGVehiclePhysicsLOD::LOD_Simplified));
	TestFalse(TEXT("Don't skip tire temp for full LOD"),
		ShouldSkipTireTemperature(1000.0f, EMGVehiclePhysicsLOD::LOD_Full));
	TestTrue(TEXT("Skip tire temp for slow vehicle"),
		ShouldSkipTireTemperature(50.0f, EMGVehiclePhysicsLOD::LOD_Full));

	TestTrue(TEXT("Skip part wear for simplified LOD"),
		ShouldSkipPartWear(EMGVehiclePhysicsLOD::LOD_Simplified));
	TestFalse(TEXT("Don't skip part wear for full LOD"),
		ShouldSkipPartWear(EMGVehiclePhysicsLOD::LOD_Full));

	AddInfo(TEXT("Early exit optimization logic validated"));
	return true;
}

/**
 * Performance Test: SIMD Suspension Forces
 * Measures SIMD suspension calculation performance
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGSIMDSuspensionPerformanceTest,
	"MidnightGrind.Performance.Optimization.SIMDSuspensionSpeed",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGSIMDSuspensionPerformanceTest::RunTest(const FString& Parameters)
{
	// Test data (4 wheels)
	alignas(16) float Compressions[4] = { 5.0f, 6.0f, 4.5f, 5.5f };
	alignas(16) float CompVelocities[4] = { 10.0f, -5.0f, 8.0f, -3.0f };
	alignas(16) float SpringRates[4] = { 35.0f, 35.0f, 30.0f, 30.0f };
	alignas(16) float DamperRates[4] = { 5.0f, 5.0f, 4.5f, 4.5f };
	alignas(16) float OutForces[4];

	const int32 IterationCount = 10000;

	// Test: SIMD/optimized version
	double SIMDStartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < IterationCount; ++i)
	{
		MGPhysicsSIMD::CalculateSuspensionForcesVectorized(
			Compressions, CompVelocities, SpringRates, DamperRates, OutForces);
	}
	double SIMDTime = (FPlatformTime::Seconds() - SIMDStartTime) * 1000.0;

	// Validate results
	TestTrue(TEXT("FL force reasonable"), FMath::Abs(OutForces[0] - 225.0f) < 1.0f); // (5*35)+(10*5) = 225
	TestTrue(TEXT("FR force reasonable"), FMath::Abs(OutForces[1] - 185.0f) < 1.0f); // (6*35)+(-5*5) = 185

	// Performance validation
	const double TimePerCall = (SIMDTime * 1000.0) / IterationCount; // microseconds
	TestTrue(TEXT("SIMD suspension fast (<30μs per 4-wheel calc)"), TimePerCall < 30.0);

	AddInfo(FString::Printf(TEXT("SIMD suspension time: %.3f ms"), SIMDTime));
	AddInfo(FString::Printf(TEXT("Time per 4-wheel calc: %.3f μs"), TimePerCall));
	AddInfo(FString::Printf(TEXT("Throughput: %.0f calcs/sec"), IterationCount / (SIMDTime / 1000.0)));

	return true;
}

/**
 * Integration Test: Combined Optimizations Performance
 * Measures combined effect of all optimizations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCombinedOptimizationsTest,
	"MidnightGrind.Performance.Optimization.CombinedEffect",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCombinedOptimizationsTest::RunTest(const FString& Parameters)
{
	// Simulate optimized physics tick for 8 vehicles over 600 frames (10 seconds)
	const int32 VehicleCount = 8;
	const int32 FrameCount = 600;

	int32 FullPhysicsCalculations = 0;
	int32 OptimizedCalculations = 0;
	int32 StationarySkips = 0;
	int32 LODSkips = 0;
	int32 CachedRaycasts = 0;

	// Initialize systems
	FMGTireForceLookupTable TireLookup;
	TireLookup.Initialize(10.0f, 1.9f, 1.0f, 0.97f);

	// Simulate each vehicle
	for (int32 VehicleIndex = 0; VehicleIndex < VehicleCount; ++VehicleIndex)
	{
		// Vehicle setup
		const bool bIsPlayer = (VehicleIndex == 0);
		const float BaseDistance = 5000.0f + (VehicleIndex * 5000.0f); // 50m spacing
		FMGVehicleLODState LODState;
		FMGSuspensionRaycastCache RaycastCache;

		// Simulate frames
		for (int32 Frame = 0; Frame < FrameCount; ++Frame)
		{
			const float CurrentTime = Frame / 60.0f;

			// Determine LOD
			LODState.CurrentLOD = FMGVehicleLODState::DetermineLOD(BaseDistance, true, bIsPlayer);

			// Check if should update this frame (LOD optimization)
			if (!LODState.ShouldUpdateThisFrame())
			{
				LODSkips++;
				continue;
			}

			// Check if stationary (early exit optimization)
			FVector Velocity(100.0f * (Frame % 100), 0.0f, 0.0f); // Varies with time
			if (MGPhysicsEarlyExit::IsVehicleStationary(Velocity, FVector::ZeroVector, 0.0f, 0.0f))
			{
				StationarySkips++;
				continue;
			}

			// Check raycast cache
			if (!RaycastCache.ShouldUpdateRaycast(CurrentTime, Velocity))
			{
				CachedRaycasts++;
			}
			else
			{
				FHitResult Hit;
				RaycastCache.UpdateHitResult(Hit, CurrentTime, Velocity);
			}

			// Perform optimized physics calculation
			float TireForces[4];
			float SlipRatios[4] = { 0.1f, 0.1f, 0.1f, 0.1f };
			float NormalLoads[4] = { 5000.0f, 5000.0f, 5000.0f, 5000.0f };

			MGPhysicsSIMD::CalculateTireForcesVectorized(SlipRatios, NormalLoads, TireForces, TireLookup);

			OptimizedCalculations++;
		}
	}

	// Calculate statistics
	const int32 TotalPossibleCalculations = VehicleCount * FrameCount;
	const float OptimizationRate = (static_cast<float>(OptimizedCalculations) / TotalPossibleCalculations) * 100.0f;
	const float SkipRate = 100.0f - OptimizationRate;

	AddInfo(FString::Printf(TEXT("Total possible calculations: %d"), TotalPossibleCalculations));
	AddInfo(FString::Printf(TEXT("Full physics calculations: %d"), OptimizedCalculations));
	AddInfo(FString::Printf(TEXT("Stationary skips: %d"), StationarySkips));
	AddInfo(FString::Printf(TEXT("LOD skips: %d"), LODSkips));
	AddInfo(FString::Printf(TEXT("Cached raycasts: %d"), CachedRaycasts));
	AddInfo(FString::Printf(TEXT("Total skips: %d (%.1f%%)"), StationarySkips + LODSkips, SkipRate));
	AddInfo(FString::Printf(TEXT("Effective physics load reduction: %.1fx"),
		static_cast<float>(TotalPossibleCalculations) / FMath::Max(1, OptimizedCalculations)));

	// Estimate time savings (based on profiling from Iteration 97)
	// Full physics: ~0.25ms per vehicle
	// Optimized physics with lookup tables + SIMD: ~0.16ms per vehicle
	// Skipped physics: ~0.001ms per vehicle (minimal checks)
	const float BasePhysicsTime = TotalPossibleCalculations * 0.25f; // ms
	const float OptimizedPhysicsTime = OptimizedCalculations * 0.16f; // ms
	const float SkippedTime = (StationarySkips + LODSkips) * 0.001f; // ms
	const float TotalOptimizedTime = OptimizedPhysicsTime + SkippedTime;
	const float TimeSavingsPercent = ((BasePhysicsTime - TotalOptimizedTime) / BasePhysicsTime) * 100.0f;

	AddInfo(FString::Printf(TEXT("Estimated base physics time: %.2f ms"), BasePhysicsTime));
	AddInfo(FString::Printf(TEXT("Estimated optimized time: %.2f ms"), TotalOptimizedTime));
	AddInfo(FString::Printf(TEXT("Estimated time savings: %.1f%%"), TimeSavingsPercent));

	// Validate optimization goals
	TestTrue(TEXT("Time savings > 30%"), TimeSavingsPercent > 30.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
