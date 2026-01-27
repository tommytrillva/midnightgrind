# Midnight Grind - API Documentation

**Version**: 1.0
**Last Updated**: January 27, 2026
**Audience**: Developers, Technical Designers

## Table of Contents

1. [Overview](#overview)
2. [Core Subsystems](#core-subsystems)
3. [Catalog System](#catalog-system)
4. [Economy System](#economy-system)
5. [Social System](#social-system)
6. [AI System](#ai-system)
7. [Common Patterns](#common-patterns)
8. [Blueprint Usage](#blueprint-usage)
9. [Best Practices](#best-practices)

---

## Overview

Midnight Grind uses Unreal Engine 5's subsystem architecture to organize game functionality into modular, testable components. All major systems are implemented as `UGameInstanceSubsystem` classes, ensuring they:

- Initialize automatically when the game starts
- Persist throughout the game session
- Are globally accessible via the GameInstance
- Support Blueprint access for designers

### Architecture Principles

**Subsystem Pattern:**
```cpp
// Access any subsystem from C++
UGameInstance* GameInstance = GetGameInstance();
UMGVehicleCatalogSubsystem* Catalog = GameInstance->GetSubsystem<UMGVehicleCatalogSubsystem>();

// Access from Blueprint
// Get Game Instance -> Get Subsystem (MGVehicleCatalogSubsystem)
```

**Key Benefits:**
- Automatic lifecycle management
- No manual initialization/cleanup needed
- Global accessibility without singletons
- Blueprint-friendly design

---

## Core Subsystems

### Subsystem Categories

| Category | Subsystems | Purpose |
|----------|------------|---------|
| **Data** | Vehicle Catalog, Parts Catalog | DataTable-based data access |
| **Economy** | Market, Mechanic, Player Inventory | Economic simulation |
| **Social** | Friends, Crew, Achievements | Social features |
| **AI** | AI Racer, Traffic, Police | NPC behavior |
| **Progression** | XP, Levels, Unlocks | Player progression |
| **Gameplay** | Race, Pursuit, Pink Slip | Core gameplay loops |

### Initialization Order

Subsystems initialize in dependency order:
1. **Data Catalogs** (Vehicle, Parts) - Load DataTables
2. **Core Systems** (Economy, Progression) - Depend on catalogs
3. **Gameplay Systems** (Race, AI) - Depend on core systems

---

## Catalog System

### Vehicle Catalog Subsystem

**Purpose**: Central repository for all vehicle data with O(1) hash table lookups.

**Class**: `UMGVehicleCatalogSubsystem`

#### Key Features
- DataTable-based vehicle storage
- Multi-index caching (by ID, by class)
- Blueprint-accessible API
- O(1) lookup performance

#### API Reference

##### Get Vehicle Data
```cpp
// C++ Example
FMGVehicleData VehicleData;
bool bFound = VehicleCatalog->GetVehicleData(TEXT("Vehicle_Skyline_R34"), VehicleData);

if (bFound)
{
    float TopSpeed = VehicleData.BaseStats.TopSpeed;
    float Price = VehicleData.BasePrice;
}
```

**Blueprint Node**: `Get Vehicle Data`
- **Input**: Vehicle ID (Name)
- **Output**: Vehicle Data (struct), Found (bool)

##### Get Vehicle Base Price
```cpp
// C++ Example
float Price = VehicleCatalog->GetVehicleBasePrice(TEXT("Vehicle_Skyline_R34"));
// Returns: 35000.0 (or 0.0 if not found)
```

**Blueprint Node**: `Get Vehicle Base Price`
- **Input**: Vehicle ID (Name)
- **Output**: Base Price (float)
- **Performance**: O(1) - Uses hash table cache

##### Get Vehicles by Class
```cpp
// C++ Example
TArray<FMGVehicleData> SportVehicles = VehicleCatalog->GetVehiclesByClass(EMGVehicleClass::Sport);

for (const FMGVehicleData& Vehicle : SportVehicles)
{
    UE_LOG(LogTemp, Log, TEXT("Sport Vehicle: %s"), *Vehicle.DisplayName.ToString());
}
```

**Blueprint Node**: `Get Vehicles By Class`
- **Input**: Vehicle Class (enum)
- **Output**: Vehicles (array of FMGVehicleData)
- **Use Case**: Filter vehicles for dealership UI

##### Get All Vehicles
```cpp
// C++ Example
TArray<FMGVehicleData> AllVehicles = VehicleCatalog->GetAllVehicles();
int32 VehicleCount = AllVehicles.Num();
```

**Blueprint Node**: `Get All Vehicles`
- **Output**: All Vehicles (array)
- **Use Case**: Populate vehicle selection menus

#### Data Structure

```cpp
struct FMGVehicleData : public FTableRowBase
{
    UPROPERTY(BlueprintReadOnly)
    FName VehicleID;                    // Unique identifier

    UPROPERTY(BlueprintReadOnly)
    FText DisplayName;                  // User-facing name

    UPROPERTY(BlueprintReadOnly)
    float BasePrice;                    // Purchase price

    UPROPERTY(BlueprintReadOnly)
    EMGVehicleClass VehicleClass;       // Class (Economy, Sport, etc.)

    UPROPERTY(BlueprintReadOnly)
    FMGVehicleStats BaseStats;          // Performance stats

    // ... additional fields
};
```

#### Usage Example: Dealership

```cpp
// Example: Populate dealership with sport vehicles
void UMGDealershipWidget::PopulateSportVehicles()
{
    UGameInstance* GI = GetGameInstance();
    UMGVehicleCatalogSubsystem* Catalog = GI->GetSubsystem<UMGVehicleCatalogSubsystem>();

    TArray<FMGVehicleData> SportVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::Sport);

    for (const FMGVehicleData& Vehicle : SportVehicles)
    {
        // Create UI widget for each vehicle
        UMGVehicleListItem* ItemWidget = CreateWidget<UMGVehicleListItem>(this, VehicleItemClass);
        ItemWidget->SetVehicleData(Vehicle);
        VehicleListPanel->AddChild(ItemWidget);
    }
}
```

---

### Parts Catalog Subsystem

**Purpose**: Central repository for all vehicle parts with compatibility checking.

**Class**: `UMGPartsCatalogSubsystem`

#### Key Features
- DataTable-based parts storage
- Compatibility validation (vehicle-specific, class-based)
- Pricing information (base cost + labor)
- Specialization requirements

#### API Reference

##### Get Part Data
```cpp
// C++ Example
FMGPartData PartData;
bool bFound = PartsCatalog->GetPartData(TEXT("Part_Turbo_T3"), PartData);

if (bFound)
{
    float BaseCost = PartData.BaseCost;
    float LaborCost = PartData.LaborCost;
    EMGPartCategory Category = PartData.Category;
}
```

**Blueprint Node**: `Get Part Data`
- **Input**: Part ID (Name)
- **Output**: Part Data (struct), Found (bool)

##### Get Part Pricing
```cpp
// C++ Example
FMGPartPricingInfo Pricing = PartsCatalog->GetPartPricing(TEXT("Part_Turbo_T3"));

if (Pricing.bIsValid)
{
    float TotalCost = Pricing.TotalCost;  // BaseCost + LaborCost
    UE_LOG(LogTemp, Log, TEXT("Total installation cost: $%.2f"), TotalCost);
}
```

**Blueprint Node**: `Get Part Pricing`
- **Input**: Part ID (Name)
- **Output**: Pricing Info (struct)
- **Fields**: BaseCost, LaborCost, TotalCost, bIsValid

##### Check Part Compatibility
```cpp
// C++ Example
bool bCompatible = PartsCatalog->IsPartCompatibleWithVehicle(
    TEXT("Part_Turbo_T3"),
    TEXT("Vehicle_Skyline_R34"),
    EMGVehicleClass::Sport
);

if (bCompatible)
{
    // Allow installation
}
```

**Blueprint Node**: `Is Part Compatible With Vehicle`
- **Input**: Part ID, Vehicle ID, Vehicle Class
- **Output**: Is Compatible (bool)
- **Logic**:
  - Checks vehicle-specific whitelist
  - Falls back to class compatibility
  - Returns true for universal parts

##### Get Parts by Category
```cpp
// C++ Example
TArray<FMGPartData> EngineParts = PartsCatalog->GetPartsByCategory(EMGPartCategory::Engine);

for (const FMGPartData& Part : EngineParts)
{
    UE_LOG(LogTemp, Log, TEXT("Engine Part: %s - $%.2f"),
        *Part.DisplayName.ToString(), Part.BaseCost);
}
```

**Blueprint Node**: `Get Parts By Category`
- **Input**: Category (enum)
- **Output**: Parts (array)
- **Categories**: Engine, Turbo, Exhaust, Suspension, Brakes, Tires, etc.

#### Data Structure

```cpp
struct FMGPartData : public FTableRowBase
{
    UPROPERTY(BlueprintReadOnly)
    FName PartID;                       // Unique identifier

    UPROPERTY(BlueprintReadOnly)
    FText DisplayName;                  // User-facing name

    UPROPERTY(BlueprintReadOnly)
    EMGPartCategory Category;           // Part category

    UPROPERTY(BlueprintReadOnly)
    float BaseCost;                     // Part purchase price

    UPROPERTY(BlueprintReadOnly)
    float LaborCost;                    // Installation labor cost

    UPROPERTY(BlueprintReadOnly)
    float InstallTimeMinutes;           // Time to install

    UPROPERTY(BlueprintReadOnly)
    TArray<FName> CompatibleVehicles;   // Vehicle whitelist

    UPROPERTY(BlueprintReadOnly)
    TArray<EMGVehicleClass> CompatibleVehicleClasses;  // Class whitelist

    UPROPERTY(BlueprintReadOnly)
    bool bRequiresSpecialist;           // Needs specialist mechanic

    // ... additional fields
};
```

#### Usage Example: Part Shop

```cpp
// Example: Display compatible parts for player's vehicle
void UMGPartShopWidget::ShowCompatibleParts()
{
    UGameInstance* GI = GetGameInstance();
    UMGPartsCatalogSubsystem* PartsCatalog = GI->GetSubsystem<UMGPartsCatalogSubsystem>();

    FName PlayerVehicleID = PlayerInventory->GetCurrentVehicleID();
    EMGVehicleClass VehicleClass = PlayerInventory->GetCurrentVehicleClass();

    // Get all engine parts
    TArray<FMGPartData> EngineParts = PartsCatalog->GetPartsByCategory(EMGPartCategory::Engine);

    // Filter by compatibility
    for (const FMGPartData& Part : EngineParts)
    {
        bool bCompatible = PartsCatalog->IsPartCompatibleWithVehicle(
            Part.PartID, PlayerVehicleID, VehicleClass);

        if (bCompatible)
        {
            // Show part in shop UI
            CreatePartListItem(Part);
        }
    }
}
```

---

## Economy System

### Market Subsystem

**Purpose**: Vehicle buying, selling, and market value calculations.

**Class**: `UMGPlayerMarketSubsystem`

#### API Reference

##### Get Vehicle Buy Price
```cpp
// C++ Example
float BuyPrice = Market->GetVehicleBuyPrice(TEXT("Vehicle_Skyline_R34"));
// Factors in market conditions, demand, player reputation
```

**Blueprint Node**: `Get Vehicle Buy Price`
- **Input**: Vehicle ID
- **Output**: Buy Price (float)
- **Factors**: Base price, market trend, player reputation discount

##### Get Vehicle Sell Price
```cpp
// C++ Example
float SellPrice = Market->GetVehicleSellPrice(TEXT("Vehicle_Skyline_R34"));
// Typically 60-80% of buy price (market spread)
```

**Blueprint Node**: `Get Vehicle Sell Price`
- **Input**: Vehicle ID
- **Output**: Sell Price (float)
- **Note**: Always less than buy price (dealer spread)

##### Get Estimated Market Value
```cpp
// C++ Example
float MarketValue = Market->GetEstimatedMarketValue(TEXT("Vehicle_Skyline_R34"));
// Includes upgrades, condition, mileage
```

**Blueprint Node**: `Get Estimated Market Value`
- **Input**: Vehicle ID
- **Output**: Market Value (float)
- **Use Case**: Trade-in values, pink slip calculations

---

### Mechanic Subsystem

**Purpose**: Part installation, labor costs, and mechanic services.

**Class**: `UMGMechanicSubsystem`

#### API Reference

##### Get Part Install Time
```cpp
// C++ Example
int32 Hours = Mechanic->GetPartBaseInstallTime(TEXT("Part_Turbo_T3"));
// Returns time in hours (converted from minutes, rounded up)
```

**Blueprint Node**: `Get Part Base Install Time`
- **Input**: Part ID
- **Output**: Hours (integer)
- **Source**: Parts catalog (minutes) → converted to hours

##### Get Part Install Cost
```cpp
// C++ Example
int32 LaborCost = Mechanic->GetPartBaseInstallCost(TEXT("Part_Turbo_T3"));
// Returns labor cost from catalog
```

**Blueprint Node**: `Get Part Base Install Cost`
- **Input**: Part ID
- **Output**: Labor Cost (integer)
- **Source**: Parts catalog labor cost

##### Get Available Mechanic Slots
```cpp
// C++ Example
int32 AvailableSlots = Mechanic->GetAvailableMechanicSlots();
// Number of mechanics not currently working on jobs
```

**Blueprint Node**: `Get Available Mechanic Slots`
- **Output**: Available Slots (integer)
- **Use Case**: Check if player can start new installation job

##### Get Active Jobs
```cpp
// C++ Example
TArray<FMGMechanicJob> ActiveJobs = Mechanic->GetActiveJobs();

for (const FMGMechanicJob& Job : ActiveJobs)
{
    float TimeRemaining = Job.TimeRemainingHours;
    FName PartID = Job.PartID;
}
```

**Blueprint Node**: `Get Active Jobs`
- **Output**: Jobs (array of FMGMechanicJob)
- **Use Case**: Display mechanic garage status

#### Usage Example: Part Installation

```cpp
// Example: Calculate total installation cost
void UMGMechanicWidget::ShowInstallationCost(FName PartID)
{
    UGameInstance* GI = GetGameInstance();
    UMGPartsCatalogSubsystem* PartsCatalog = GI->GetSubsystem<UMGPartsCatalogSubsystem>();
    UMGMechanicSubsystem* Mechanic = GI->GetSubsystem<UMGMechanicSubsystem>();

    // Get part pricing
    FMGPartPricingInfo Pricing = PartsCatalog->GetPartPricing(PartID);

    // Get install details
    int32 InstallHours = Mechanic->GetPartBaseInstallTime(PartID);
    int32 LaborCost = Mechanic->GetPartBaseInstallCost(PartID);

    // Calculate total
    float TotalCost = Pricing.BaseCost + LaborCost;

    // Display to player
    CostText->SetText(FText::Format(
        LOCTEXT("InstallCost", "Part: ${0}\nLabor: ${1}\nTotal: ${2}\nTime: {3} hours"),
        FText::AsNumber(Pricing.BaseCost),
        FText::AsNumber(LaborCost),
        FText::AsNumber(TotalCost),
        FText::AsNumber(InstallHours)
    ));
}
```

---

## Social System

### Player Social Subsystem

**Purpose**: Friends, reputation, achievements, crew management.

**Class**: `UMGPlayerSocialSubsystem`

#### API Reference

##### Get Player Reputation
```cpp
// C++ Example
int32 Reputation = Social->GetPlayerReputation();
// Returns current reputation points (0-10000+)
```

**Blueprint Node**: `Get Player Reputation`
- **Output**: Reputation (integer)

##### Get Reputation Tier
```cpp
// C++ Example
EMGReputationTier Tier = Social->GetReputationTier();
// Returns: Unknown, Street, Known, Respected, Famous, Legend
```

**Blueprint Node**: `Get Reputation Tier`
- **Output**: Tier (enum)
- **Tiers**: Unknown (0-99), Street (100-499), Known (500-1999), etc.

##### Get Friends List
```cpp
// C++ Example
TArray<FMGFriendInfo> Friends = Social->GetFriendsList();

for (const FMGFriendInfo& Friend : Friends)
{
    FString FriendName = Friend.DisplayName;
    bool bIsOnline = Friend.bIsOnline;
}
```

**Blueprint Node**: `Get Friends List`
- **Output**: Friends (array of FMGFriendInfo)

##### Check Achievement Progress
```cpp
// C++ Example
float Progress = Social->GetAchievementProgress(TEXT("FirstWin"));
// Returns progress percentage (0.0 - 100.0)
```

**Blueprint Node**: `Get Achievement Progress`
- **Input**: Achievement ID
- **Output**: Progress (float, 0-100)

---

## AI System

### AI Subsystem

**Purpose**: AI opponent management, difficulty scaling, rubber-banding.

**Class**: `UMGAISubsystem`

#### API Reference

##### Select Opponents
```cpp
// C++ Example
TArray<FMGAIDriverProfile> Opponents = AI->SelectOpponents(5);
// Selects 5 AI opponents based on player skill
```

**Blueprint Node**: `Select Opponents`
- **Input**: Count (integer)
- **Output**: Opponents (array of FMGAIDriverProfile)
- **Logic**: Matches player skill level with variance

##### Get Difficulty Multiplier
```cpp
// C++ Example
float Multiplier = AI->GetDifficultyMultiplier();
// Returns 0.5 (Easy) to 2.0 (Hard)
```

**Blueprint Node**: `Get Difficulty Multiplier`
- **Output**: Multiplier (float)
- **Use Case**: Scale AI performance stats

##### Predict AI Lap Time
```cpp
// C++ Example
float PredictedTime = AI->PredictAILapTime(EMGAIDifficulty::Medium, TEXT("TrackName"));
// Returns estimated lap time in seconds
```

**Blueprint Node**: `Predict AI Lap Time`
- **Input**: Difficulty, Track Name
- **Output**: Lap Time (float, seconds)

---

## Common Patterns

### Pattern 1: Accessing Subsystems

**C++ Pattern:**
```cpp
// In any UObject-derived class
UGameInstance* GI = GetGameInstance();
UMGVehicleCatalogSubsystem* Catalog = GI->GetSubsystem<UMGVehicleCatalogSubsystem>();

// Null check for safety
if (Catalog)
{
    // Use subsystem
}
```

**Blueprint Pattern:**
1. Get Game Instance node
2. Get Subsystem node (select subsystem class)
3. Use subsystem functions

### Pattern 2: Data Lookup

**Always check for valid data:**
```cpp
// Pattern: Get data with validation
FMGVehicleData VehicleData;
if (Catalog->GetVehicleData(VehicleID, VehicleData))
{
    // Data is valid, use it
}
else
{
    // Handle missing data
    UE_LOG(LogTemp, Warning, TEXT("Vehicle not found: %s"), *VehicleID.ToString());
}
```

### Pattern 3: Pricing Calculations

**Get total cost from multiple sources:**
```cpp
// Pattern: Calculate total installation cost
FMGPartPricingInfo Pricing = PartsCatalog->GetPartPricing(PartID);
int32 LaborCost = Mechanic->GetPartBaseInstallCost(PartID);
float TotalCost = Pricing.BaseCost + LaborCost;
```

---

## Blueprint Usage

### Common Blueprint Workflows

#### 1. Display Vehicle in Dealership
```
[Get Game Instance]
    ↓
[Get Subsystem: MGVehicleCatalogSubsystem]
    ↓
[Get Vehicle Data] (VehicleID)
    ↓
[Branch] (Found?)
    True → [Display Vehicle Info]
    False → [Show Error Message]
```

#### 2. Check Part Compatibility
```
[Get Game Instance]
    ↓
[Get Subsystem: MGPartsCatalogSubsystem]
    ↓
[Is Part Compatible With Vehicle] (PartID, VehicleID, VehicleClass)
    ↓
[Branch] (Compatible?)
    True → [Enable Purchase Button]
    False → [Show Incompatibility Message]
```

#### 3. Calculate Installation Cost
```
[Get Part Pricing] (PartID)
    ↓
[Get Part Base Install Cost] (PartID)
    ↓
[Add] (BaseCost + LaborCost)
    ↓
[Display Total Cost]
```

---

## Best Practices

### 1. Always Validate Data

**DO:**
```cpp
FMGVehicleData VehicleData;
if (Catalog->GetVehicleData(VehicleID, VehicleData))
{
    // Use VehicleData safely
}
```

**DON'T:**
```cpp
FMGVehicleData VehicleData = Catalog->GetVehicleData(VehicleID);  // May be invalid!
float Price = VehicleData.BasePrice;  // Crash if data not found!
```

### 2. Cache Subsystem References

**For frequent access, cache the subsystem:**
```cpp
class UMyWidget : public UUserWidget
{
private:
    UPROPERTY()
    UMGVehicleCatalogSubsystem* CachedCatalog;

public:
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();

        // Cache on initialization
        CachedCatalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>();
    }

    void UpdateDisplay()
    {
        // Use cached reference
        if (CachedCatalog)
        {
            // No need to get subsystem every time
        }
    }
};
```

### 3. Use Appropriate Data Types

**FName for IDs (fast comparison):**
```cpp
FName VehicleID = TEXT("Vehicle_Skyline_R34");  // Use for IDs
```

**FText for display strings (localization-friendly):**
```cpp
FText DisplayName = VehicleData.DisplayName;  // Use for UI
```

**FString for manipulation:**
```cpp
FString FormattedText = FString::Printf(TEXT("Price: $%.2f"), Price);
```

### 4. Handle Edge Cases

**Check for valid pricing:**
```cpp
FMGPartPricingInfo Pricing = PartsCatalog->GetPartPricing(PartID);

if (!Pricing.bIsValid)
{
    // Handle missing pricing data
    UE_LOG(LogTemp, Warning, TEXT("Invalid pricing for part: %s"), *PartID.ToString());
    return;
}
```

### 5. Use Enums for Type Safety

**Prefer enums over strings:**
```cpp
// Good
EMGVehicleClass VehicleClass = EMGVehicleClass::Sport;

// Bad
FString VehicleClass = "Sport";  // Typo-prone, no compile-time checking
```

---

## Performance Considerations

### 1. Catalog Lookups are O(1)
All catalog lookups use hash tables - they're very fast:
```cpp
// These are all O(1) operations
float Price = VehicleCatalog->GetVehicleBasePrice(VehicleID);
FMGPartData Part = PartsCatalog->GetPartData(PartID);
```

### 2. Filtering is O(n)
Filtering operations iterate through all items:
```cpp
// O(n) - iterates through all vehicles
TArray<FMGVehicleData> SportVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::Sport);

// Cache results if using multiple times
CachedSportVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::Sport);
```

### 3. Avoid Repeated GetSubsystem Calls
Getting subsystems is cheap, but caching is better:
```cpp
// Less efficient (but still fast)
for (int32 i = 0; i < 1000; ++i)
{
    UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>();
    Catalog->GetVehicleBasePrice(VehicleID);
}

// More efficient
UMGVehicleCatalogSubsystem* Catalog = GetGameInstance()->GetSubsystem<UMGVehicleCatalogSubsystem>();
for (int32 i = 0; i < 1000; ++i)
{
    Catalog->GetVehicleBasePrice(VehicleID);
}
```

---

## Additional Resources

### Code Examples
See test files for comprehensive usage examples:
- `Source/MidnightGrind/Tests/Unit/MGVehicleCatalogTests.cpp`
- `Source/MidnightGrind/Tests/Unit/MGPartsCatalogTests.cpp`
- `Source/MidnightGrind/Tests/Integration/MGEconomyIntegrationTests.cpp`

### Data Tables
Example DataTable assets:
- `Content/Data/DT_Vehicles.uasset`
- `Content/Data/DT_Parts.uasset`

### Further Reading
- Unreal Engine Subsystems Documentation
- DataTable Best Practices
- Blueprint API Design Guidelines

---

**Document Version**: 1.0
**Last Updated**: January 27, 2026
**Maintained By**: Midnight Grind Development Team
