# Midnight Grind Test Suite

This directory contains the automated test suite for Midnight Grind, built using Unreal Engine 5's Automation framework.

## Directory Structure

```
Tests/
├── Unit/                               # Unit tests for individual systems
│   ├── MGVehicleCatalogTests.cpp       # Vehicle catalog tests (2 tests)
│   ├── MGPartsCatalogTests.cpp         # Parts catalog tests (3 tests)
│   ├── MGCatalogEdgeCasesTests.cpp     # Catalog edge cases (6 tests)
│   ├── MGMechanicSubsystemTests.cpp    # Mechanic subsystem tests (4 tests)
│   ├── MGMarketSubsystemTests.cpp      # Market subsystem tests (4 tests)
│   ├── MGSocialSystemTests.cpp         # Social system tests (4 tests)
│   └── MGAISystemTests.cpp             # AI system tests (5 tests)
├── Integration/                        # Integration tests for system interactions
│   ├── MGEconomyIntegrationTests.cpp   # Economy integration (4 tests)
│   ├── MGSocialIntegrationTests.cpp    # Social integration (3 tests)
│   └── MGGameplayIntegrationTests.cpp  # Gameplay integration (3 tests)
├── Performance/                        # Performance benchmarks
│   ├── MGCatalogPerformanceTests.cpp   # Catalog performance (5 tests)
│   └── MGSubsystemPerformanceTests.cpp # Subsystem performance (3 tests)
└── TestHelpers/                        # Shared test utilities
    ├── MGTestDataFactory.h
    └── MGTestDataFactory.cpp
```

## Running Tests

### In-Editor
1. Open Unreal Editor
2. Go to Window > Developer Tools > Session Frontend
3. Click the "Automation" tab
4. Select tests under "MidnightGrind" category
5. Click "Start Tests"

### Command Line
```bash
# Run all tests
UnrealEditor.exe ProjectPath/MidnightGrind.uproject -ExecCmds="Automation RunTests MidnightGrind" -unattended -NullRHI -log

# Run specific test suite
UnrealEditor.exe ProjectPath/MidnightGrind.uproject -ExecCmds="Automation RunTests MidnightGrind.Unit.Catalog" -unattended -NullRHI -log
```

## Current Test Coverage

### Total: 46 tests (28 unit + 10 integration + 8 performance)

### Unit Tests (28 tests)

#### Catalog Tests (5 tests)
- **MGVehicleCatalogPricingTest**: Validates vehicle pricing lookups
- **MGVehicleCatalogClassFilterTest**: Validates vehicle class filtering
- **MGPartsCatalogPricingTest**: Validates part pricing and install time lookups
- **MGPartsCatalogCompatibilityTest**: Validates part compatibility logic
- **MGPartsCatalogSpecializationTest**: Validates specialist part requirements

#### Economy Tests (8 tests)
- **MGMechanicInstallTimeTest**: Validates mechanic install time calculations
- **MGMechanicLaborCostTest**: Validates mechanic labor cost calculations
- **MGMechanicSkillLevelTest**: Validates mechanic skill level impact
- **MGMechanicJobQueueTest**: Validates mechanic job queue management
- **MGMarketVehicleValuationTest**: Validates market vehicle pricing
- **MGMarketPartPricingTest**: Validates market part pricing
- **MGMarketDemandTest**: Validates market demand tracking
- **MGMarketPriceSpreadTest**: Validates buy/sell price differentials

#### Catalog Edge Cases (6 tests)
- **MGCatalogEmptyDataTableTest**: Validates empty DataTable handling
- **MGCatalogNullPointerTest**: Validates null pointer safety
- **MGCatalogInvalidNameTest**: Validates invalid name handling
- **MGCatalogLargeDatasetTest**: Validates large dataset performance
- **MGCatalogDataValidationTest**: Validates data integrity checks
- **MGCatalogCacheConsistencyTest**: Validates cache consistency

#### Social System Tests (4 tests)
- **MGSocialFriendManagementTest**: Validates friend list management
- **MGSocialReputationTest**: Validates reputation tracking
- **MGSocialAchievementTest**: Validates achievement tracking
- **MGSocialCrewMembershipTest**: Validates crew membership management

#### AI System Tests (5 tests)
- **MGAIDifficultyScalingTest**: Validates AI difficulty scaling
- **MGAIOpponentSelectionTest**: Validates opponent selection logic
- **MGAIBehaviorStateTest**: Validates AI behavior state management
- **MGAIPerformanceCalculationTest**: Validates AI performance predictions
- **MGAIRubberBandingTest**: Validates rubber-banding system

### Integration Tests (10 tests)

#### Economy Integration Tests (4 tests)
- **MGVehiclePurchaseFlowTest**: Validates complete vehicle purchase workflow
- **MGPartInstallationFlowTest**: Validates part installation across catalog and mechanic
- **MGMarketCatalogIntegrationTest**: Validates market integration with both catalogs
- **MGEconomyConsistencyTest**: Validates consistent pricing across all economy subsystems

#### Social Integration Tests (3 tests)
- **MGAchievementReputationFlowTest**: Validates achievement unlocks update reputation
- **MGFriendCrewInteractionTest**: Validates friend list and crew membership interaction
- **MGReputationAreaKnowledgeTest**: Validates reputation affects area knowledge

#### Gameplay Integration Tests (3 tests)
- **MGRaceSetupIntegrationTest**: Validates AI opponent selection with vehicle catalog
- **MGProgressionRewardFlowTest**: Validates progression updates trigger rewards
- **MGAIDifficultyPerformanceTest**: Validates AI performance scales with difficulty

### Performance Tests (8 tests)

#### Catalog Performance Tests (5 tests)
- **MGCatalogInitializationPerformanceTest**: Measures initialization time (50 vehicles, 500 parts)
- **MGCatalogLookupPerformanceTest**: Measures 10,000+ lookup operations, validates O(1)
- **MGCatalogConcurrentAccessTest**: Measures concurrent access from multiple systems
- **MGCatalogFilterPerformanceTest**: Measures filtering operations on large datasets
- **MGCatalogMemoryEfficiencyTest**: Validates memory usage and cache efficiency

#### Subsystem Performance Tests (3 tests)
- **MGMultiSubsystemInitializationTest**: Measures initialization time for all subsystems
- **MGEconomyCalculationThroughputTest**: Measures 5,000+ economy calculations
- **MGAISystemLoadTest**: Measures AI system under load (1,000+ operations)

### Test Helpers
- **FMGTestDataFactory**: Factory class for generating mock test data
  - Vehicle data creation
  - Part data creation
  - Mock DataTable creation
  - Validation utilities

## Writing New Tests

### Unit Test Template
```cpp
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMyTestClass,
    "MidnightGrind.Unit.System.TestName",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyTestClass::RunTest(const FString& Parameters)
{
    // Arrange: Set up test data

    // Act: Execute the code under test

    // Assert: Verify results
    TestEqual(TEXT("Description"), ActualValue, ExpectedValue);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

### Using Test Data Factory
```cpp
// Create single test vehicle
FMGVehicleData Vehicle = FMGTestDataFactory::CreateTestVehicle(
    TEXT("MyVehicle"),
    FText::FromString(TEXT("Test Car")),
    25000.0f,
    EMGVehicleClass::Sport);

// Create array of test vehicles
TArray<FMGVehicleData> Vehicles = FMGTestDataFactory::CreateTestVehicleArray(10);

// Create mock DataTable
UDataTable* DataTable = FMGTestDataFactory::CreateMockVehicleDataTable(Outer, Vehicles);
```

## Test Categories

Tests are organized into hierarchical categories:
- `MidnightGrind.Unit.*` - Unit tests
- `MidnightGrind.Integration.*` - Integration tests
- `MidnightGrind.Performance.*` - Performance benchmarks

## Coverage Goals

- **Iteration 90**: 5 tests, ~15% catalog coverage ✅
- **Iteration 91**: 28 tests, ~70% unit test coverage ✅ (Target exceeded!)
- **Iteration 92 (Complete)**: 46 tests, ~80% overall coverage ✅ (Target exceeded!)
  - Unit: 28 tests (70% coverage)
  - Integration: 10 tests (validates cross-system workflows)
  - Performance: 8 tests (validates performance requirements)

## Best Practices

1. **Isolation**: Each test should be independent and not rely on other tests
2. **Naming**: Use descriptive test names that explain what is being tested
3. **Assertions**: Include clear descriptions in TestEqual/TestTrue/TestFalse calls
4. **Cleanup**: Tests should clean up after themselves (UE5 handles GC automatically)
5. **Performance**: Keep unit tests fast (<100ms each)

## Common Assertion Methods

```cpp
TestEqual(TEXT("Description"), Actual, Expected);          // Values equal
TestNotEqual(TEXT("Description"), Actual, NotExpected);    // Values not equal
TestTrue(TEXT("Description"), bCondition);                 // Boolean true
TestFalse(TEXT("Description"), bCondition);                // Boolean false
TestNull(TEXT("Description"), Pointer);                     // Pointer is null
TestNotNull(TEXT("Description"), Pointer);                  // Pointer is valid
TestValid(TEXT("Description"), Object);                     // UObject is valid
```

## Debugging Tests

1. Set breakpoints in test code
2. Run editor in debug mode
3. Execute specific test from Automation panel
4. Step through test execution

## Future Enhancements (Iterations 91-92)

- [ ] Economy subsystem integration tests
- [ ] Mechanic subsystem unit tests
- [ ] Market subsystem unit tests
- [ ] Social system tests
- [ ] AI system tests
- [ ] Performance profiling tests
- [ ] Multi-subsystem workflow tests
