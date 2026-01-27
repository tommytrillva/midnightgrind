# ITERATION 80 - Data Loading Infrastructure Plan
## Midnight Grind - Phase 3 Implementation Begin

**Date:** 2026-01-26 20:30 PST
**Phase:** Phase 3 - Implementation
**Focus:** Design data loading system for vehicle/parts catalogs

---

## MISSION

Unblock 8 economy TODOs by creating infrastructure to load vehicle and parts data from JSON files into runtime-accessible data structures.

---

## CURRENT STATE ANALYSIS

### Existing Infrastructure ✅

**Data Structures:**
- `UMGVehicleModelData` (DataAsset) - lines 1426-1497 of MGVehicleData.h
- Complete vehicle spec structs (FMGEngineSpec, FMGTuningSetup, etc.)
- 18+ USTRUCTs for vehicle components

**Content Files:**
- `/Content/Data/Vehicles/*.json` - 15+ vehicle definitions
- `/Content/Data/Parts/*.json` - 5+ parts catalogs
- Complete JSON schema with all required fields

**Example Vehicle Data:**
```json
{
  "VehicleID": "KAZE_CIVIC",
  "DisplayName": "Kaze Civic",
  "BasePurchasePrice": 12000,
  "StreetValue": 15000,
  "Engine": {...},
  "AvailableUpgrades": {...}
}
```

###Gaps Identified:
1. ❌ No JSON → Runtime data loader
2. ❌ No vehicle catalog registry/lookup system
3. ❌ No parts catalog registry/lookup system
4. ❌ Economy subsystems can't access vehicle pricing data

---

## DESIGN APPROACH

### Option 1: DataTable System (UE5 Native) ✅ RECOMMENDED

**How It Works:**
- UE5's built-in DataTable system
- Define USTRUCT for row format
- Import JSON to DataTable asset in editor
- Runtime lookup via FName keys

**Pros:**
- ✅ Native UE5 support
- ✅ Editor integration (can edit in UE5 UI)
- ✅ Fast runtime lookups (hash table)
- ✅ No custom parsing code needed
- ✅ Blueprint accessible

**Cons:**
- ⚠️ Requires UE5 editor import step
- ⚠️ JSON must match exact struct format

**Implementation Effort:** Low (1 iteration)

---

### Option 2: Custom JSON Loader

**How It Works:**
- Custom subsystem reads JSON at runtime
- Parses to runtime structs
- Caches in TMap<FName, Data>

**Pros:**
- ✅ Fully runtime (no editor step)
- ✅ Can transform/validate data
- ✅ Flexible JSON schema

**Cons:**
- ⚠️ Must write JSON parsing code
- ⚠️ More complex implementation
- ⚠️ Performance overhead (parsing)

**Implementation Effort:** Medium (2-3 iterations)

---

### Option 3: Hybrid Approach

**How It Works:**
- DataTable for static vehicle models
- Runtime loader for dynamic pricing/economy data

**Pros:**
- ✅ Best of both worlds
- ✅ Editor for content, runtime for economy

**Cons:**
- ⚠️ More complex architecture

**Implementation Effort:** Medium (2 iterations)

---

## RECOMMENDED SOLUTION: Option 1 (DataTables)

### Implementation Plan

**Step 1: Define DataTable Row Structs**
- Create `FMGVehicleCatalogRow` struct
- Create `FMGPartsCatalogRow` struct
- Match JSON schema exactly

**Step 2: Document Import Process**
- Create import guide for UE5 editor
- Document DataTable asset creation
- Add validation helpers

**Step 3: Create Catalog Accessor Subsystem**
- `UMGVehicleCatalogSubsystem`
- `UMGPartsCatalogSubsystem`
- Provide lookup functions

**Step 4: Hook Economy TODOs**
- Update `GetEstimatedMarketValue()`
- Update parts pricing lookups
- Update mechanic specialization

---

## DATA STRUCTURE DESIGN

### Vehicle Catalog Row

```cpp
/**
 * DataTable row for vehicle catalog
 * Matches JSON schema from Content/Data/Vehicles/*.json
 */
USTRUCT(BlueprintType)
struct FMGVehicleCatalogRow : public FTableRowBase
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString VehicleID; // e.g. "KAZE_CIVIC"

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName; // e.g. "Kaze Civic"

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Manufacturer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Year = 1999;

    // Economy
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 BasePurchasePrice = 25000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 StreetValue = 30000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 LegendaryValue = 60000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaintenanceCostMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float PartsPriceMultiplier = 1.0f;

    // Performance
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 BasePI = 420; // Performance Index

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString PerformanceClass; // "D", "C", "B", "A", "S", "X"

    // Full specs (nested struct)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FMGVehicleFullSpecs Specifications;
};
```

### Parts Catalog Row

```cpp
/**
 * DataTable row for parts catalog
 */
USTRUCT(BlueprintType)
struct FMGPartCatalogRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString PartID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Category; // "Engine", "Exhaust", etc.

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Tier; // "Stock", "Street", "Race", etc.

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Price = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString CompatibleVehicles; // Comma-separated list

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Specialization; // For mechanic specialization lookup

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float InstallTime = 60.0f; // Minutes
};
```

---

## CATALOG ACCESSOR SUBSYSTEM

### Vehicle Catalog Subsystem

```cpp
/**
 * Vehicle catalog accessor
 * Provides lookups for vehicle pricing and specs
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVehicleCatalogSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Initialize from DataTable
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /**
     * Get vehicle pricing data
     * @param VehicleID Vehicle identifier (e.g. "KAZE_CIVIC")
     * @return Pricing info, or default if not found
     */
    UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
    FMGVehiclePricingInfo GetVehiclePricing(FName VehicleID) const;

    /**
     * Get vehicle base specs
     */
    UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
    FMGVehicleCatalogRow GetVehicleData(FName VehicleID) const;

    /**
     * Get all vehicles in performance class
     */
    UFUNCTION(BlueprintPure, Category = "Vehicle Catalog")
    TArray<FMGVehicleCatalogRow> GetVehiclesByClass(FString PerformanceClass) const;

protected:
    // Reference to DataTable asset
    UPROPERTY(EditDefaultsOnly, Category = "Config")
    UDataTable* VehicleCatalogTable = nullptr;

    // Cached lookup for fast access
    TMap<FName, FMGVehicleCatalogRow*> VehicleCache;
};
```

---

## ECONOMY TODO RESOLUTION

### TODO 1: GetEstimatedMarketValue()

**Before:**
```cpp
int64 UMGPlayerMarketSubsystem::GetEstimatedMarketValue(FGuid VehicleID) const
{
    // TODO: Look up vehicle data and calculate based on:
    // - Base model value
    // - Parts installed
    // - Condition
    // - Recent sale prices for similar vehicles

    return 50000; // Placeholder
}
```

**After:**
```cpp
int64 UMGPlayerMarketSubsystem::GetEstimatedMarketValue(FGuid VehicleID) const
{
    // Get vehicle instance from garage
    if (UMGGarageSubsystem* Garage = GetGarageSubsystem())
    {
        FMGOwnedVehicle Vehicle;
        if (Garage->GetVehicle(VehicleID, Vehicle))
        {
            // Look up base value from catalog
            if (UMGVehicleCatalogSubsystem* Catalog = GetVehicleCatalog())
            {
                FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing(Vehicle.ModelID);

                int64 BaseValue = Pricing.StreetValue;

                // Apply condition modifier (0.5 - 1.0)
                BaseValue *= Vehicle.Condition;

                // Add parts value (simplified for now)
                int64 PartsValue = Vehicle.InstalledParts.Num() * 5000;

                return BaseValue + PartsValue;
            }
        }
    }

    return 50000; // Fallback
}
```

**Status:** ✅ Resolvable with catalog subsystem

---

### TODO 2: Price History Filtering

**Before:**
```cpp
TArray<FMGMarketTransaction> GetPriceHistory(FName ModelID, int32 DaysBack) const
{
    TArray<FMGMarketTransaction> Results;

    // TODO: Filter by ModelID
    // For now, return all transactions

    return CompletedTransactions;
}
```

**After:**
```cpp
TArray<FMGMarketTransaction> GetPriceHistory(FName ModelID, int32 DaysBack) const
{
    TArray<FMGMarketTransaction> Results;

    // Filter transactions by model ID
    for (const FMGMarketTransaction& Transaction : CompletedTransactions)
    {
        // Get vehicle data to check model
        if (UMGGarageSubsystem* Garage = GetGarageSubsystem())
        {
            FMGOwnedVehicle Vehicle;
            if (Garage->GetVehicle(Transaction.ItemID, Vehicle))
            {
                if (Vehicle.ModelID == ModelID)
                {
                    Results.Add(Transaction);
                }
            }
        }
    }

    return Results;
}
```

**Status:** ✅ Resolvable (doesn't even need catalog, just garage lookup)

---

### TODO 3-6: Mechanic Specialization (Parts Catalog)

**All 4 mechanic TODOs require parts catalog:**
1. Determine specialization from part type
2. Check if part matches specialization
3. Look up install time
4. Look up part complexity

**Solution:**
```cpp
FString UMGMechanicSubsystem::GetPartSpecialization(FName PartID) const
{
    if (UMGPartsCatalogSubsystem* Catalog = GetPartsCatalog())
    {
        FMGPartCatalogRow Part = Catalog->GetPartData(PartID);
        return Part.Specialization; // "Engine", "Drivetrain", "Suspension", etc.
    }

    return "General"; // Fallback
}
```

**Status:** ✅ Resolvable with parts catalog subsystem

---

## IMPLEMENTATION ROADMAP

### Iteration 80 (Current)
- ✅ Design architecture (this document)
- ✅ Define data structure approach
- ✅ Document implementation plan

### Iteration 81
- Create FMGVehicleCatalogRow struct
- Create UMGVehicleCatalogSubsystem
- Document DataTable import process

### Iteration 82
- Create FMGPartCatalogRow struct
- Create UMGPartsCatalogSubsystem
- Add lookup helper functions

### Iteration 83
- Implement GetEstimatedMarketValue()
- Implement price history filtering
- Test with sample data

### Iteration 84
- Implement mechanic specialization lookups
- Hook all 4 mechanic TODOs
- Verify all 8 economy TODOs resolved

---

## TESTING STRATEGY

### Manual Verification

**Vehicle Catalog:**
```cpp
// In BeginPlay or test function:
if (UMGVehicleCatalogSubsystem* Catalog = GetVehicleCatalog())
{
    FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing("KAZE_CIVIC");
    UE_LOG(LogTemp, Log, TEXT("Kaze Civic base price: %d"), Pricing.BasePurchasePrice);
    // Expected: 12000
}
```

**Parts Catalog:**
```cpp
if (UMGPartsCatalogSubsystem* Catalog = GetPartsCatalog())
{
    FMGPartCatalogRow Part = Catalog->GetPartData("TURBO_SMALL_CIVIC");
    UE_LOG(LogTemp, Log, TEXT("Part specialization: %s"), *Part.Specialization);
    // Expected: "Engine"
}
```

### Integration Testing

1. **Market Value Calculation:**
   - Create test vehicle
   - Add parts
   - Verify market value includes base + parts
   - Test condition modifiers

2. **Price History:**
   - Create transactions for specific model
   - Query price history
   - Verify correct filtering

3. **Mechanic Specialization:**
   - Test each part category
   - Verify specialization matches
   - Test install time bonuses

---

## DELIVERABLES

### Code Files to Create

1. `MGVehicleCatalogSubsystem.h` - Vehicle catalog accessor
2. `MGVehicleCatalogSubsystem.cpp` - Implementation
3. `MGPartsCatalogSubsystem.h` - Parts catalog accessor
4. `MGPartsCatalogSubsystem.cpp` - Implementation
5. `MGDataTableStructs.h` - Catalog row definitions

### Documentation Files

1. DataTable import guide (for UE5 editor)
2. Catalog usage examples (for other developers)
3. JSON schema validation guide

### Updated Files

1. `MGPlayerMarketSubsystem.cpp` - Resolve 2 TODOs
2. `MGMechanicSubsystem.cpp` - Resolve 4 TODOs

---

## RISK ASSESSMENT

### Low Risk ✅

**Why:**
- Using native UE5 DataTable system (proven)
- Data structures already exist
- JSON files comprehensive
- Clear integration points

**Mitigation:**
- Start with vehicle catalog (simpler)
- Test thoroughly before parts catalog
- Incremental integration

### Dependencies

**Required:**
- UE5 editor access (for DataTable import)
- JSON schema validation

**Nice-to-Have:**
- Automated JSON → DataTable converter
- Validation tools

---

## ALTERNATIVE: Interim Solution

**If UE5 editor unavailable:**

Create simplified hardcoded catalog:
```cpp
TMap<FName, int32> HardcodedPrices = {
    { "KAZE_CIVIC", 12000 },
    { "SAKURA_GTR", 85000 },
    // etc.
};
```

**Pros:** Unblocks TODOs immediately
**Cons:** Not scalable, manual updates

**Use Case:** Temporary until DataTable system ready

---

## SUCCESS CRITERIA

### Iteration 84 Complete When:

- ✅ All 8 economy TODOs resolved
- ✅ Vehicle pricing lookups functional
- ✅ Parts catalog lookups functional
- ✅ Mechanic specialization working
- ✅ Test cases passing
- ✅ No compilation errors
- ✅ Documentation complete

---

## NEXT STEPS

**Iteration 81:** Begin implementation
- Create catalog subsystem headers
- Define row structs
- Document import process

---

**Alignment:** REFINEMENT_PLAN.md Phase 3 - Implementation
**Priority:** P1 (High - unblocks 8 TODOs)
**Type:** Implementation Planning

---
