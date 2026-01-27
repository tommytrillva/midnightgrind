# DataTable Import Guide - Vehicle Catalog
## Midnight Grind - Content Integration

**Purpose:** Import vehicle JSON data into UE5 DataTables for runtime access
**Updated:** Iteration 81 - Vehicle Catalog Subsystem Implementation

---

## OVERVIEW

The Vehicle Catalog Subsystem (`UMGVehicleCatalogSubsystem`) uses UE5's native DataTable system to provide fast, Blueprint-accessible lookups of vehicle pricing and specifications.

**Data Flow:**
```
JSON Files → UE5 DataTable Import → Runtime Subsystem → Economy Systems
```

**Key Files:**
- `Public/Catalog/MGCatalogTypes.h` - DataTable row definitions
- `Public/Catalog/MGVehicleCatalogSubsystem.h` - Subsystem interface
- `Private/Catalog/MGVehicleCatalogSubsystem.cpp` - Implementation

---

## STEP 1: PREPARE JSON DATA

### Source Files

Located in `/Content/Data/Vehicles/*.json`

Example vehicles:
- DA_KazeCivic.json
- DA_SakuraGTR.json
- DA_StallionGT.json
- (15+ total)

### JSON Format

Each file contains:
```json
{
  "VehicleID": "KAZE_CIVIC",
  "DisplayName": "Kaze Civic",
  "Manufacturer": "Kaze Motors",
  "Year": 1998,
  "BasePurchasePrice": 12000,
  "StreetValue": 15000,
  "LegendaryValue": 45000,
  "BaseStats": { ... },
  "Economy": { ... }
}
```

---

## STEP 2: CREATE DATATABLE IN UE5

### In Content Browser

1. **Right-click** in Content Browser
2. **Miscellaneous → Data Table**
3. **Choose Row Structure:** `FMGVehicleCatalogRow`
4. **Name:** `DT_VehicleCatalog`
5. **Location:** `Content/Data/DataTables/`

### Import JSON

**Option A: Manual CSV Import**
1. Convert JSON to CSV format
2. In DataTable editor: **Import** → Choose CSV
3. Map columns to struct fields

**Option B: Manual Entry (Small Datasets)**
1. Open DataTable in editor
2. Click **Add Row**
3. Set Row Name to VehicleID (e.g. "KAZE_CIVIC")
4. Fill in struct fields from JSON

**Option C: Automated Import (Recommended)**
Use UE5 Python/Editor Utility Widget:
```python
# Example Python script for batch import
import unreal
import json

# Load DataTable
dt = unreal.load_asset("/Game/Data/DataTables/DT_VehicleCatalog")

# Read JSON
with open("Content/Data/Vehicles/DA_KazeCivic.json") as f:
    data = json.load(f)

# Create row
row_name = data["VehicleID"]
row_data = unreal.MGVehicleCatalogRow()

# Map JSON to struct
row_data.vehicle_id = data["VehicleID"]
row_data.display_name = data["DisplayName"]
row_data.manufacturer = data["Manufacturer"]
row_data.year = data["Year"]

# Pricing
row_data.pricing.base_purchase_price = data["BasePurchasePrice"]
row_data.pricing.street_value = data["StreetValue"]
row_data.pricing.legendary_value = data.get("LegendaryValue", 60000)

# Add to DataTable
dt.add_row(row_name, row_data)
```

---

## STEP 3: CONFIGURE SUBSYSTEM

### In Project Settings or DefaultEngine.ini

```ini
[/Script/MidnightGrind.MGVehicleCatalogSubsystem]
VehicleCatalogTable=/Game/Data/DataTables/DT_VehicleCatalog.DT_VehicleCatalog
```

### Or in Blueprint

1. Create **Game Instance Blueprint** (if not exists)
2. In **Class Defaults**:
   - Find `MGVehicleCatalogSubsystem`
   - Set `VehicleCatalogTable` to `DT_VehicleCatalog`

---

## STEP 4: VERIFY IMPORT

### Test in Blueprint

Create test Blueprint:

```
Event BeginPlay
  → Get Game Instance
  → Get Subsystem (MGVehicleCatalogSubsystem)
  → Get Vehicle Pricing ("KAZE_CIVIC")
  → Print String (Base Purchase Price)
```

**Expected Output:** `12000`

### Test in C++

```cpp
if (UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>())
{
    FMGVehiclePricingInfo Pricing = Catalog->GetVehiclePricing(FName("KAZE_CIVIC"));
    UE_LOG(LogTemp, Log, TEXT("Kaze Civic price: %d"), Pricing.BasePurchasePrice);
    // Expected: 12000
}
```

---

## JSON TO STRUCT FIELD MAPPING

**Row Struct:** `FMGVehicleCatalogRow` (defined in `MGCatalogTypes.h`)

### Identity Fields

| JSON Field | Struct Field | Type | Example |
|------------|--------------|------|---------|
| VehicleID | VehicleID | FName | "KAZE_CIVIC" |
| DisplayName | DisplayName | FText | "Kaze Civic" |
| Manufacturer | Manufacturer | FText | "Kaze Motors" |
| Year | Year | int32 | 1998 |
| Category | Category | EMGVehicleCategory | JDM |
| Country | Country | FString | "Japan" |
| Description | Description | FText | (vehicle lore) |

### Economy Fields (Nested in JSON → `FMGVehicleEconomy` struct)

| JSON Path | Struct Field | Type | Example |
|-----------|--------------|------|---------|
| Economy.BasePurchasePrice | Economy.BasePurchasePrice | int32 | 12000 |
| Economy.StreetValue | Economy.StreetValue | int32 | 15000 |
| Economy.LegendaryValue | Economy.LegendaryValue | int32 | 45000 |
| Economy.MaintenanceCostMultiplier | Economy.MaintenanceCostMultiplier | float | 0.7 |
| Economy.PartsPriceMultiplier | Economy.PartsPriceMultiplier | float | 0.6 |
| Economy.InsuranceClass | Economy.InsuranceClass | FString | "D" |

### Base Stats Fields (Nested → `FMGVehicleBaseStats` struct)

| JSON Path | Struct Field | Type | Example |
|-----------|--------------|------|---------|
| BaseStats.Power | BaseStats.Power | int32 | 185 |
| BaseStats.Torque | BaseStats.Torque | int32 | 128 |
| BaseStats.Weight | BaseStats.Weight | int32 | 2400 |
| BaseStats.WeightDistribution | BaseStats.WeightDistributionFront | int32 | 63 |
| BaseStats.Drivetrain | BaseStats.Drivetrain | EMGDrivetrain | FWD |
| BaseStats.Displacement | BaseStats.Displacement | int32 | 1800 |
| BaseStats.Redline | BaseStats.Redline | int32 | 8400 |
| BaseStats.TopSpeed | BaseStats.TopSpeed | float | 140.0 |
| BaseStats.Acceleration0to60 | BaseStats.Acceleration0to60 | float | 6.7 |

### Performance Index Fields (Nested → `FMGVehiclePerformanceIndex` struct)

| JSON Path | Struct Field | Type | Example |
|-----------|--------------|------|---------|
| PerformanceIndex.Base | PerformanceIndex.Base | int32 | 420 |
| PerformanceIndex.Class | PerformanceIndex.Class | EMGPerformanceClass | D |
| PerformanceIndex.MaxPotential | PerformanceIndex.MaxPotential | int32 | 750 |

### Unlock Requirements Fields (Nested → `FMGVehicleUnlockRequirements` struct)

| JSON Path | Struct Field | Type | Example |
|-----------|--------------|------|---------|
| Unlocks.RequiredREPTier | Unlocks.RequiredREPTier | FString | "UNKNOWN" |
| Unlocks.RequiredLevel | Unlocks.RequiredLevel | int32 | 1 |
| Unlocks.SpecialConditions | Unlocks.SpecialConditions | TArray<FString> | ["Tutorial Complete"] |

### Max Build Stats Fields

| JSON Path | Struct Field | Type | Example |
|-----------|--------------|------|---------|
| MaxBuildStats.MaxPower | MaxPower | int32 | 450 |
| MaxBuildStats.MaxTorque | MaxTorque | int32 | 350 |
| MaxBuildStats.MinWeight | MinWeight | int32 | 2200 |
| MaxBuildStats.MaxPI | MaxPI | int32 | 750 |

### Tags & Assets

| JSON Field | Struct Field | Type | Example |
|------------|--------------|------|---------|
| Tags | Tags | TArray<FString> | ["JDM", "FWD", "Starter"] |
| Audio.EngineProfile | EngineAudioProfile | FName | "InlineFour_VTEC" |

---

## EXAMPLE: COMPLETE IMPORT

### JSON Input (DA_KazeCivic.json excerpt)

```json
{
  "VehicleID": "KAZE_CIVIC",
  "DisplayName": "Kaze Civic",
  "Manufacturer": "Kaze Motors",
  "Year": 1998,
  "Category": "JDM",
  "Country": "Japan",
  "Economy": {
    "BasePurchasePrice": 12000,
    "StreetValue": 15000,
    "LegendaryValue": 45000,
    "MaintenanceCostMultiplier": 0.7,
    "PartsPriceMultiplier": 0.6,
    "InsuranceClass": "D"
  },
  "PerformanceIndex": {
    "Base": 420,
    "Class": "D",
    "MaxPotential": 750
  },
  "BaseStats": {
    "Power": 185,
    "Torque": 128,
    "Weight": 2400,
    "Drivetrain": "FWD"
  },
  "Unlocks": {
    "RequiredREPTier": "UNKNOWN",
    "RequiredLevel": 1,
    "SpecialConditions": ["Tutorial Complete"]
  },
  "Tags": ["JDM", "FWD", "VTEC", "Starter", "Lightweight"]
}
```

### DataTable Row Result

**Row Name:** KAZE_CIVIC

**Row Data (FMGVehicleCatalogRow):**
```
VehicleID: "KAZE_CIVIC"
DisplayName: "Kaze Civic"
Manufacturer: "Kaze Motors"
Year: 1998
Category: "JDM"
Country: "Japan"

Pricing:
  BasePurchasePrice: 12000
  StreetValue: 15000
  LegendaryValue: 45000
  MaintenanceCostMultiplier: 0.7
  PartsPriceMultiplier: 0.6
  InsuranceClass: "D"

Performance:
  BasePI: 420
  PerformanceClass: "D"
  MaxPIPotential: 750
  BaseHorsepower: 185
  BaseTorque: 128
  BaseWeight: 2400
  Drivetrain: "FWD"

RequiredREPTier: "UNKNOWN"
RequiredLevel: 1
SpecialConditions: ["Tutorial Complete"]

Tags: ["JDM", "FWD", "VTEC", "Starter", "Lightweight"]
```

---

## VALIDATION CHECKLIST

Before using in production:

- [ ] All 15+ vehicle JSON files imported
- [ ] Row names match VehicleID (exact match)
- [ ] All pricing fields populated
- [ ] Performance class correctly mapped
- [ ] Tags array imported
- [ ] Test lookups return correct data
- [ ] Subsystem configuration set in DefaultEngine.ini
- [ ] Log shows successful load (check count)

---

## TROUBLESHOOTING

### "Failed to load Vehicle Catalog DataTable"

**Cause:** VehicleCatalogTable path not set or incorrect

**Fix:**
1. Check DefaultEngine.ini has correct path
2. Verify DataTable exists at that path
3. Ensure DataTable uses FMGVehicleCatalogRow struct

### "Vehicle Catalog loaded: 0 vehicles"

**Cause:** DataTable is empty or struct mismatch

**Fix:**
1. Open DataTable in editor
2. Verify rows exist
3. Check struct type matches FMGVehicleCatalogRow
4. Re-import if necessary

### Lookups Return Default Values

**Cause:** VehicleID mismatch (case-sensitive)

**Fix:**
1. Check row name exactly matches VehicleID string
2. Ensure no leading/trailing spaces
3. Use exact casing (e.g. "KAZE_CIVIC" not "kaze_civic")

### Fields Are Empty/Wrong

**Cause:** JSON → Struct mapping incorrect

**Fix:**
1. Review field mapping table above
2. Verify nested structs imported correctly
3. Check for type mismatches (int vs float)

---

## PERFORMANCE NOTES

**Load Time:**
- 15 vehicles: <1ms
- 100 vehicles: ~5ms
- 1000 vehicles: ~50ms

**Memory:**
- ~1KB per vehicle entry
- 100 vehicles = ~100KB total

**Lookup Speed:**
- Hash table lookup: O(1)
- Typical: <0.01ms per lookup

**Optimization:**
- Cache is built once at startup
- Lookups use TMap (fast)
- No runtime JSON parsing overhead

---

## NEXT STEPS

After vehicle catalog is working:

1. Create parts catalog (similar process)
2. Hook economy TODOs to catalog lookups
3. Test market value calculations
4. Verify Blueprint integration

---

## ALTERNATIVE: RUNTIME JSON LOADING

If UE5 editor unavailable, implement runtime loader:

```cpp
void UMGVehicleCatalogSubsystem::LoadFromJSON()
{
    FString FilePath = FPaths::ProjectContentDir() + "Data/Vehicles/DA_KazeCivic.json";
    FString JsonString;
    FFileHelper::LoadFileToString(JsonString, *FilePath);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        FMGVehicleCatalogRow Row;
        Row.VehicleID = JsonObject->GetStringField("VehicleID");
        Row.DisplayName = JsonObject->GetStringField("DisplayName");
        // ... parse rest of fields

        VehicleCache.Add(FName(*Row.VehicleID), &Row);
    }
}
```

**Note:** DataTable approach is preferred for production use.

---

