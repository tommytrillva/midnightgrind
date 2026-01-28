# MIDNIGHT GRIND - CODING STANDARDS

**Version:** 1.0
**Last Updated:** January 28, 2026
**Standard:** Unreal Engine C++ Best Practices + AAA Game Studio Quality

---

## TABLE OF CONTENTS

1. [General Principles](#1-general-principles)
2. [File Organization](#2-file-organization)
3. [Naming Conventions](#3-naming-conventions)
4. [Code Formatting](#4-code-formatting)
5. [Comments & Documentation](#5-comments--documentation)
6. [UE5 Specific Guidelines](#6-ue5-specific-guidelines)
7. [Memory Management](#7-memory-management)
8. [Error Handling](#8-error-handling)
9. [Performance](#9-performance)
10. [Testing](#10-testing)

---

## 1. General Principles

### Core Philosophy

1. **Readability First** - Code is read more than written. Optimize for understanding.
2. **Consistency** - Follow existing patterns. Don't introduce new styles arbitrarily.
3. **Explicit Over Implicit** - Clear intentions trump clever shortcuts.
4. **Fail Fast** - Validate early, provide clear error messages.
5. **YAGNI** - Don't add features until needed.

### Quality Standards

| Metric | Target | Current |
|--------|--------|---------|
| TODO/FIXME count | 0 | 0 |
| HACK comments | 0 | 0 |
| Test coverage | 75%+ | ~80% |
| Comment density | 8-12% | ~9.6% |
| Max file length | 3000 lines | Achieved |

---

## 2. File Organization

### Header File Template

```cpp
// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMySubsystem.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class UMGOtherClass;
struct FMGOtherStruct;

// ============================================================================
// ENUMS
// ============================================================================

/**
 * Description of what this enum represents.
 */
UENUM(BlueprintType)
enum class EMGMyState : uint8
{
    None        UMETA(DisplayName = "None"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed")
};

// ============================================================================
// STRUCTS
// ============================================================================

/**
 * Description of what this struct represents.
 */
USTRUCT(BlueprintType)
struct FMGMyData
{
    GENERATED_BODY()

    /** Description of field */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    int32 Value = 0;

    /** Description of field */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    FString Name;
};

// ============================================================================
// DELEGATES
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMyEvent, int32, EventData);

// ============================================================================
// CLASS DECLARATION
// ============================================================================

/**
 * Brief description of the subsystem (one sentence).
 *
 * Detailed description explaining:
 * - Purpose and responsibility
 * - Key features
 * - Usage examples
 * - Dependencies
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ========================================================================
    // LIFECYCLE
    // ========================================================================

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // PUBLIC API - QUERIES
    // ========================================================================

    /**
     * Get current state.
     * @return The current state of the system.
     */
    UFUNCTION(BlueprintPure, Category = "Midnight Grind|MySystem")
    EMGMyState GetState() const { return CurrentState; }

    // ========================================================================
    // PUBLIC API - COMMANDS
    // ========================================================================

    /**
     * Perform an action that modifies state.
     * @param Data The data to process.
     * @return True if successful, false otherwise.
     */
    UFUNCTION(BlueprintCallable, Category = "Midnight Grind|MySystem")
    bool DoAction(const FMGMyData& Data);

    // ========================================================================
    // EVENTS
    // ========================================================================

    /** Fired when my event occurs. */
    UPROPERTY(BlueprintAssignable, Category = "Midnight Grind|MySystem|Events")
    FOnMyEvent OnMyEvent;

protected:
    // ========================================================================
    // PROTECTED MEMBERS
    // ========================================================================

    /** Current system state */
    UPROPERTY()
    EMGMyState CurrentState = EMGMyState::None;

private:
    // ========================================================================
    // PRIVATE MEMBERS
    // ========================================================================

    /** Internal data storage */
    TMap<FName, FMGMyData> DataMap;

    // ========================================================================
    // PRIVATE METHODS
    // ========================================================================

    void InternalHelper();
    void ProcessData(const FMGMyData& Data);
};
```

### Implementation File Template

```cpp
// Copyright Midnight Grind. All Rights Reserved.

#include "MySystem/MGMySubsystem.h"

// Include other MG headers
#include "OtherSystem/MGOtherSubsystem.h"

// Include UE5 headers
#include "Engine/World.h"

// ============================================================================
// LIFECYCLE
// ============================================================================

void UMGMySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogMidnightGrind, Log, TEXT("MGMySubsystem initialized"));

    // Initialization code here
}

void UMGMySubsystem::Deinitialize()
{
    // Cleanup code here

    UE_LOG(LogMidnightGrind, Log, TEXT("MGMySubsystem deinitialized"));

    Super::Deinitialize();
}

// ============================================================================
// PUBLIC API - QUERIES
// ============================================================================

// Getters implementation here

// ============================================================================
// PUBLIC API - COMMANDS
// ============================================================================

bool UMGMySubsystem::DoAction(const FMGMyData& Data)
{
    // Validate input
    if (Data.Name.IsEmpty())
    {
        UE_LOG(LogMidnightGrind, Warning, TEXT("DoAction: Empty name provided"));
        return false;
    }

    // Process
    ProcessData(Data);

    // Broadcast event
    OnMyEvent.Broadcast(Data.Value);

    return true;
}

// ============================================================================
// PRIVATE METHODS
// ============================================================================

void UMGMySubsystem::InternalHelper()
{
    // Implementation
}

void UMGMySubsystem::ProcessData(const FMGMyData& Data)
{
    // Implementation
}
```

### Directory Structure Rules

```
Public/
├── SystemName/
│   ├── MGMainSubsystem.h       # Primary subsystem
│   ├── MGRelatedComponent.h    # Related components
│   ├── MGDataTypes.h           # System-specific data types
│   └── MGWidgets.h             # UI widgets (if applicable)

Private/
├── SystemName/
│   ├── MGMainSubsystem.cpp
│   ├── MGRelatedComponent.cpp
│   └── MGWidgets.cpp
```

---

## 3. Naming Conventions

### Classes

| Type | Prefix | Example |
|------|--------|---------|
| UObject classes | UMG | `UMGVehicleCatalogSubsystem` |
| Actor classes | AMG | `AMGRaceVehicle` |
| Component classes | UMG | `UMGVehicleSFXComponent` |
| Interface classes | IMG | `IMGDamageable` |
| Widget classes | UMG | `UMGRaceHUDWidget` |

### Data Types

| Type | Prefix | Example |
|------|--------|---------|
| Enums | EMG | `EMGRaceState` |
| Structs | FMG | `FMGVehicleData` |
| Delegates | FOn/FMG | `FOnRaceFinished` |

### Variables

| Type | Convention | Example |
|------|------------|---------|
| Member variables | PascalCase | `CurrentSpeed` |
| Local variables | PascalCase | `TempValue` |
| Parameters | PascalCase | `InVehicleID` |
| Out parameters | Out prefix | `OutData` |
| Boolean | b prefix | `bIsValid` |
| Pointer | No prefix | `VehicleActor` |

### Functions

| Type | Convention | Example |
|------|------------|---------|
| Public getters | Get prefix | `GetCurrentSpeed()` |
| Public setters | Set prefix | `SetMaxSpeed()` |
| Boolean getters | Is/Has/Can | `IsValid()`, `HasData()`, `CanAfford()` |
| Event handlers | Handle prefix | `HandleRaceFinished()` |
| Internal helpers | Descriptive | `ProcessData()`, `ValidateInput()` |

### Files

| Type | Convention | Example |
|------|------------|---------|
| Headers | MG prefix, PascalCase | `MGVehicleCatalogSubsystem.h` |
| Source | Same as header | `MGVehicleCatalogSubsystem.cpp` |
| Tests | MG prefix + Tests | `MGVehicleCatalogTests.cpp` |
| Data assets | DA_ prefix | `DA_Skyline_R34.uasset` |
| DataTables | DT_ prefix | `DT_Vehicles.uasset` |

---

## 4. Code Formatting

### Indentation & Spacing

```cpp
// Use tabs for indentation (Unreal standard)
// Tab width: 4 spaces

// Braces on same line for functions
void MyFunction()
{
    // Code here
}

// Braces on same line for control structures
if (bCondition)
{
    // Code
}
else
{
    // Code
}

// Single-line if statements: NEVER (always use braces)
// BAD:
if (bCondition) DoThing();

// GOOD:
if (bCondition)
{
    DoThing();
}
```

### Line Length

- **Maximum:** 120 characters
- **Preferred:** 80-100 characters
- Break long lines at logical points:

```cpp
// Long function call
FMGRaceResult Result = RaceSubsystem->CalculateRaceResult(
    RacerID,
    LapTimes,
    PenaltyTime,
    bDisqualified
);

// Long condition
if (Vehicle.IsValid() &&
    Vehicle.GetCurrentSpeed() > MinSpeed &&
    !Vehicle.IsDamaged())
{
    // Code
}
```

### Spacing Rules

```cpp
// Operators: space around binary operators
int32 Sum = A + B;
bool bResult = A && B;

// No space for unary operators
++Counter;
!bIsValid;

// Function calls: no space before parenthesis
DoSomething(Param1, Param2);

// Commas: space after, not before
TArray<int32> Values = {1, 2, 3, 4};

// Template parameters: no spaces inside angle brackets
TMap<FName, FMGData> DataMap;

// Casts: space after cast type
float SpeedFloat = static_cast<float>(SpeedInt);
```

### Blank Lines

```cpp
// One blank line between:
// - Function definitions
// - Logical sections within functions
// - After variable declarations block

void FunctionOne()
{
    // Implementation
}

void FunctionTwo()
{
    // Variable declarations
    int32 Value = 0;
    FString Name;

    // Logic section 1
    Value = CalculateValue();

    // Logic section 2
    if (Value > 0)
    {
        ProcessValue(Value);
    }
}
```

---

## 5. Comments & Documentation

### Doxygen Style

```cpp
/**
 * @brief Brief description (one line).
 *
 * Detailed description that explains:
 * - Purpose of the function
 * - Algorithm used (if complex)
 * - Edge cases handled
 * - Side effects
 *
 * @param VehicleID The unique identifier for the vehicle to look up.
 * @param OutData [out] Populated with vehicle data if found.
 * @return True if the vehicle was found, false otherwise.
 *
 * @note This function performs a hash table lookup (O(1)).
 * @see GetAllVehicles, GetVehiclesByClass
 */
UFUNCTION(BlueprintPure, Category = "Midnight Grind|Catalog")
bool GetVehicleData(FName VehicleID, FMGVehicleData& OutData) const;
```

### Inline Comments

```cpp
// Use // for single-line comments
// Explain WHY, not WHAT (code should explain what)

// Calculate grip modifier based on surface type and conditions
// Weather affects grip more on already-slippery surfaces
float GripModifier = BaseGrip * SurfaceMultiplier * WeatherMultiplier;

// Don't state the obvious:
// BAD: Increment counter
Counter++;

// GOOD: Track frames since last input for idle detection
FramesSinceInput++;
```

### Section Comments

```cpp
// ============================================================================
// SECTION NAME
// ============================================================================

// Use for major code sections in implementation files
// Keep consistent format throughout codebase
```

### Forbidden Comments

```cpp
// NEVER use these (they indicate incomplete work):
// TODO: ...     // Must be resolved before merge
// FIXME: ...    // Must be resolved before merge
// HACK: ...     // Must be resolved before merge
// XXX: ...      // Must be resolved before merge

// OK to use temporarily during development, but:
// - Zero TODOs in committed code
// - Zero HACKs in committed code
// - Current codebase: 0 TODOs, 0 HACKs
```

---

## 6. UE5 Specific Guidelines

### UPROPERTY Usage

```cpp
// EditAnywhere - Editable in editor and on instances
UPROPERTY(EditAnywhere, Category = "Config")
float MaxSpeed = 200.0f;

// EditDefaultsOnly - Only editable on class defaults
UPROPERTY(EditDefaultsOnly, Category = "Config")
TSubclassOf<AActor> SpawnClass;

// VisibleAnywhere - Read-only in editor
UPROPERTY(VisibleAnywhere, Category = "State")
EMGRaceState CurrentState;

// BlueprintReadWrite - Full Blueprint access
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
FString DisplayName;

// BlueprintReadOnly - Blueprint can read but not write
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
int32 CurrentLap;

// Transient - Not saved
UPROPERTY(Transient)
float CachedValue;

// Replicated - Networked
UPROPERTY(Replicated)
int32 Score;
```

### UFUNCTION Usage

```cpp
// BlueprintCallable - Can be called from Blueprint
UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Race")
void StartRace();

// BlueprintPure - No side effects, can be used in getter nodes
UFUNCTION(BlueprintPure, Category = "Midnight Grind|Race")
float GetCurrentSpeed() const;

// BlueprintNativeEvent - Can be overridden in Blueprint
UFUNCTION(BlueprintNativeEvent, Category = "Midnight Grind|Race")
void OnRaceStarted();

// BlueprintImplementableEvent - Must be implemented in Blueprint
UFUNCTION(BlueprintImplementableEvent, Category = "Midnight Grind|Race")
void OnCustomEvent();

// Server/Client - Networking
UFUNCTION(Server, Reliable)
void ServerStartRace();

UFUNCTION(Client, Reliable)
void ClientNotifyResult(int32 Position);
```

### Category Organization

```cpp
// Use hierarchical categories for organization
// Format: "Midnight Grind|System|Subsystem"

UFUNCTION(BlueprintCallable, Category = "Midnight Grind|Vehicle|Physics")
void ApplyForce(FVector Force);

UFUNCTION(BlueprintPure, Category = "Midnight Grind|Economy|Transactions")
int32 GetBalance() const;

// Keeps Blueprint node menus organized
```

### Meta Specifiers

```cpp
// DisplayName - Custom name in Blueprint
UPROPERTY(EditAnywhere, meta = (DisplayName = "Maximum Speed (KPH)"))
float MaxSpeedKPH;

// ClampMin/ClampMax - Value limits
UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "100"))
float Percentage;

// ToolTip - Hover text
UPROPERTY(EditAnywhere, meta = (ToolTip = "Speed in kilometers per hour"))
float Speed;

// EditCondition - Conditional editing
UPROPERTY(EditAnywhere)
bool bUseCustomValue;

UPROPERTY(EditAnywhere, meta = (EditCondition = "bUseCustomValue"))
float CustomValue;
```

---

## 7. Memory Management

### Smart Pointers

```cpp
// Use TSharedPtr for shared ownership (non-UObject)
TSharedPtr<FMyData> SharedData = MakeShared<FMyData>();

// Use TUniquePtr for exclusive ownership (non-UObject)
TUniquePtr<FMyData> UniqueData = MakeUnique<FMyData>();

// Use TWeakPtr for non-owning references
TWeakPtr<FMyData> WeakRef = SharedData;
if (TSharedPtr<FMyData> Pinned = WeakRef.Pin())
{
    // Use Pinned
}

// For UObjects, use TWeakObjectPtr
UPROPERTY()
TWeakObjectPtr<AActor> WeakActor;

if (WeakActor.IsValid())
{
    WeakActor->DoSomething();
}
```

### UObject Lifetime

```cpp
// UObjects managed by garbage collector
// Use UPROPERTY() to prevent GC

// BAD - may be garbage collected
UMyObject* Object = NewObject<UMyObject>();

// GOOD - protected from GC
UPROPERTY()
UMyObject* Object;

// Or use AddToRoot for static lifetime
Object->AddToRoot();
// Remember to call RemoveFromRoot() when done
```

### Array Best Practices

```cpp
// Reserve capacity when size is known
TArray<FMGData> Data;
Data.Reserve(ExpectedSize);

// Use Empty() instead of Num() == 0
if (Data.IsEmpty())  // Good
if (Data.Num() == 0) // Works but less clear

// Use ranged-for when not modifying
for (const FMGData& Item : Data)
{
    ProcessItem(Item);
}

// Use index when modifying
for (int32 i = Data.Num() - 1; i >= 0; --i)
{
    if (ShouldRemove(Data[i]))
    {
        Data.RemoveAt(i);
    }
}
```

---

## 8. Error Handling

### Null Checks

```cpp
// Always null-check pointers before use
// Pattern: Early return with logging

void ProcessActor(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogMidnightGrind, Warning, TEXT("ProcessActor: Null actor"));
        return;
    }

    // Safe to use Actor
}

// For subsystems, use if-with-initialization
if (UMGSubsystem* Sub = GetGameInstance()->GetSubsystem<UMGSubsystem>())
{
    Sub->DoThing();
}
```

### Validation Pattern

```cpp
// Validate all inputs at function entry
bool UMGCatalog::GetVehicleData(FName VehicleID, FMGVehicleData& OutData) const
{
    // Validate input
    if (VehicleID.IsNone())
    {
        UE_LOG(LogMidnightGrind, Warning, TEXT("GetVehicleData: Invalid vehicle ID"));
        return false;
    }

    // Check if data exists
    const FMGVehicleData* Found = VehicleMap.Find(VehicleID);
    if (!Found)
    {
        UE_LOG(LogMidnightGrind, Log, TEXT("GetVehicleData: Vehicle not found: %s"),
            *VehicleID.ToString());
        return false;
    }

    OutData = *Found;
    return true;
}
```

### Check/Ensure/Verify

```cpp
// check() - Halts in Debug/Development, disabled in Shipping
check(Pointer != nullptr);
checkf(Value > 0, TEXT("Value must be positive: %d"), Value);

// ensure() - Logs and continues
if (ensure(Pointer != nullptr))
{
    // Use Pointer
}

// verify() - Always evaluates expression
verify(ImportantFunction());

// Use check for programmer errors
// Use ensure for recoverable errors
// Use verify when result is needed but failure is logged
```

### Logging Levels

```cpp
// Fatal - Crashes the application
UE_LOG(LogMidnightGrind, Fatal, TEXT("Unrecoverable error"));

// Error - Serious issue
UE_LOG(LogMidnightGrind, Error, TEXT("Failed to load critical data"));

// Warning - Something unexpected but recoverable
UE_LOG(LogMidnightGrind, Warning, TEXT("Using fallback value"));

// Log - Normal information
UE_LOG(LogMidnightGrind, Log, TEXT("System initialized"));

// Verbose - Detailed debugging
UE_LOG(LogMidnightGrind, Verbose, TEXT("Processing item %d"), Index);

// VeryVerbose - Extreme detail
UE_LOG(LogMidnightGrind, VeryVerbose, TEXT("Frame tick"));
```

---

## 9. Performance

### General Guidelines

```cpp
// Prefer stack allocation over heap
FMGData LocalData;  // Good - stack
FMGData* HeapData = new FMGData();  // Avoid unless necessary

// Avoid allocation in tight loops
// BAD:
for (int32 i = 0; i < 1000; ++i)
{
    FString Temp = FString::Printf(TEXT("%d"), i);  // Allocation each iteration
}

// GOOD:
FString Temp;
for (int32 i = 0; i < 1000; ++i)
{
    Temp = FString::FromInt(i);  // Reuses buffer
}
```

### Container Selection

```cpp
// TArray - Default for most uses
TArray<FMGData> Items;

// TMap - O(1) lookup by key
TMap<FName, FMGData> ItemMap;

// TSet - O(1) contains check, unique values
TSet<FName> UniqueIds;

// Use appropriate container:
// - Need ordered iteration? TArray
// - Need fast lookup by key? TMap
// - Need fast membership check? TSet
// - Need both? TMap + TArray of keys
```

### Caching

```cpp
// Cache expensive lookups
class UMGCatalog : public UGameInstanceSubsystem
{
private:
    // Cache for O(1) lookups
    TMap<FName, FMGVehicleData> VehicleCache;
    TMap<EMGClass, TArray<FMGVehicleData>> ClassCache;

public:
    void Initialize(FSubsystemCollectionBase& Collection) override
    {
        // Build caches from DataTable once at startup
        BuildCaches();
    }
};
```

### Avoiding Expensive Operations

```cpp
// Avoid string operations in tight loops
// BAD:
FString GetDebugName() const
{
    return FString::Printf(TEXT("%s_%d"), *Name, ID);  // Allocates
}

// GOOD - cache if called frequently:
void CacheDebugName()
{
    CachedDebugName = FString::Printf(TEXT("%s_%d"), *Name, ID);
}

// Avoid virtual calls in tight loops when possible
// Use final on classes/functions that won't be overridden
class MIDNIGHTGRIND_API UMGFinalSubsystem final : public UGameInstanceSubsystem
```

---

## 10. Testing

### Test File Organization

```cpp
// File: Tests/Unit/MGMySubsystemTests.cpp

// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "MySystem/MGMySubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"

#if WITH_DEV_AUTOMATION_TESTS

// ============================================================================
// TEST: Basic Functionality
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMGMySubsystem_BasicTest,
    "MidnightGrind.Unit.MySystem.BasicFunctionality",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMySubsystem_BasicTest::RunTest(const FString& Parameters)
{
    // Arrange
    UGameInstance* GI = NewObject<UGameInstance>();
    UMGMySubsystem* System = NewObject<UMGMySubsystem>(GI);
    System->Initialize(/* ... */);

    // Act
    bool bResult = System->DoAction(FMGTestDataFactory::CreateTestData());

    // Assert
    TestTrue(TEXT("Action should succeed"), bResult);
    TestEqual(TEXT("State should be Active"), System->GetState(), EMGMyState::Active);

    return true;
}

// ============================================================================
// TEST: Edge Cases
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMGMySubsystem_EdgeCases,
    "MidnightGrind.Unit.MySystem.EdgeCases",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMySubsystem_EdgeCases::RunTest(const FString& Parameters)
{
    // Test null inputs
    // Test empty arrays
    // Test boundary values
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
```

### Test Naming Convention

```
MidnightGrind.[Suite].[System].[TestName]

Examples:
- MidnightGrind.Unit.Catalog.VehicleLookup
- MidnightGrind.Unit.Catalog.InvalidID
- MidnightGrind.Integration.Economy.PurchaseFlow
- MidnightGrind.Performance.Catalog.BulkLookup
```

### Test Assertions

```cpp
// Basic assertions
TestTrue(TEXT("Description"), bCondition);
TestFalse(TEXT("Description"), bCondition);
TestEqual(TEXT("Description"), Actual, Expected);
TestNotEqual(TEXT("Description"), Actual, NotExpected);
TestNull(TEXT("Description"), Pointer);
TestNotNull(TEXT("Description"), Pointer);

// Float comparison with tolerance
TestNearlyEqual(TEXT("Description"), ActualFloat, ExpectedFloat, Tolerance);

// Array assertions
TestEqual(TEXT("Array size"), Array.Num(), ExpectedSize);

// Always provide descriptive message
// BAD:
TestTrue(TEXT(""), bResult);

// GOOD:
TestTrue(TEXT("Vehicle purchase should succeed with sufficient funds"), bResult);
```

---

## Summary Checklist

### Before Committing Code

- [ ] No TODOs, FIXMEs, or HACKs
- [ ] All functions documented with Doxygen comments
- [ ] Null checks for all pointers
- [ ] Input validation at function entry
- [ ] Consistent naming conventions
- [ ] Proper UPROPERTY/UFUNCTION macros
- [ ] Appropriate Blueprint exposure
- [ ] Unit tests for new functionality
- [ ] No warnings in build output
- [ ] Copyright header present

### Code Review Criteria

- [ ] Follows existing patterns
- [ ] No unnecessary complexity
- [ ] Performance considerations addressed
- [ ] Error handling is appropriate
- [ ] Tests cover happy path and edge cases
- [ ] Documentation is clear and complete

---

**Document Version:** 1.0
**Last Updated:** January 28, 2026
**Standard Compliance:** AAA Game Studio Quality
