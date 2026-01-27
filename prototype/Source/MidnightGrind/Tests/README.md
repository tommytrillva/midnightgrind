# Midnight Grind Test Suite

This directory contains the automated test suite for Midnight Grind, built using Unreal Engine 5's Automation framework.

## Directory Structure

```
Tests/
├── Unit/                    # Unit tests for individual systems
│   ├── MGVehicleCatalogTests.cpp
│   └── MGPartsCatalogTests.cpp
├── Integration/             # Integration tests for system interactions
├── Performance/             # Performance benchmarks
└── TestHelpers/             # Shared test utilities
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

### Unit Tests (5 tests)
- **MGVehicleCatalogPricingTest**: Validates vehicle pricing lookups
- **MGVehicleCatalogClassFilterTest**: Validates vehicle class filtering
- **MGPartsCatalogPricingTest**: Validates part pricing and install time lookups
- **MGPartsCatalogCompatibilityTest**: Validates part compatibility logic
- **MGPartsCatalogSpecializationTest**: Validates specialist part requirements

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

- **Current**: 5 tests, ~15% catalog coverage
- **Iteration 91 Target**: 25 tests, 70% unit test coverage
- **Iteration 92 Target**: 35+ tests, 75% overall coverage

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
