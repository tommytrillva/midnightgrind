# MIDNIGHT GRIND - Game Data Files

This directory contains JSON data files that define game content, balance, and configuration. These files are designed to be loaded by the game's data subsystems and can be modified without recompiling code.

## Directory Structure

```
Content/Data/
├── Economy/
│   └── EconomyBalanceConfig.json     # Currency, payouts, pricing, REP system
├── Input/
│   └── InputMappingDefaults.json     # Controller/keyboard bindings
├── Parts/
│   └── DB_SakuraGTR_Parts.json       # Upgrade parts database
├── Tracks/
│   └── L_TestTrack_Layout.json       # Track layout specification
├── UI/
│   └── HUDConfiguration.json         # HUD styles and element positions
├── Vehicles/
│   ├── DA_SakuraGTR.json             # Hero vehicle specification
│   └── VehicleRosterPlan.json        # Full vehicle roster plan
└── README.md                          # This file
```

## File Descriptions

### Economy/EconomyBalanceConfig.json
- **Purpose:** Defines all economic values in the game
- **Contains:** Currency types, REP tier thresholds, race payouts, part pricing multipliers, maintenance costs, daily rewards
- **Used by:** MGEconomySubsystem, MGShopSubsystem, MGProgressionSubsystem

### Input/InputMappingDefaults.json
- **Purpose:** Default input bindings for all platforms
- **Contains:** Input actions, mapping contexts, controller profiles, dead zones
- **Used by:** MGInputRemapSubsystem, Enhanced Input system

### Parts/DB_SakuraGTR_Parts.json
- **Purpose:** Complete parts catalog for the Sakura GTR
- **Contains:** 85 upgrade parts across all categories with stats, prices, and PI impacts
- **Used by:** MGCustomizationSubsystem, MGShopSubsystem

### Tracks/L_TestTrack_Layout.json
- **Purpose:** Layout specification for the development test track
- **Contains:** Track sections, checkpoints, start grid, lighting, props
- **Used by:** Level designers, MGCheckpointSubsystem

### UI/HUDConfiguration.json
- **Purpose:** HUD element positioning and styling
- **Contains:** 4 HUD styles, all element configurations, color schemes, accessibility options
- **Used by:** MGRaceHUDSubsystem, UI widgets

### Vehicles/DA_SakuraGTR.json
- **Purpose:** Complete specification for the hero vehicle
- **Contains:** Base stats, power curves, transmission ratios, suspension settings, dimensions, audio profiles, lore
- **Used by:** MGVehicleDatabase, MGDynoTuningSubsystem

### Vehicles/VehicleRosterPlan.json
- **Purpose:** Development roadmap for all vehicles
- **Contains:** 30 planned vehicles across 5 development phases, class/category distribution
- **Used by:** Planning reference (not loaded at runtime)

## How to Use

### In Blueprints
```
Get Game Instance Subsystem → MGVehicleDatabase → LoadVehicleData("SAKURA_GTR")
```

### In C++
```cpp
UMGVehicleDatabase* VehicleDB = GetGameInstance()->GetSubsystem<UMGVehicleDatabase>();
FMGVehicleData Data = VehicleDB->GetVehicleData(TEXT("SAKURA_GTR"));
```

## Modifying Values

1. Edit the JSON file directly
2. Restart the game/PIE session
3. Changes take effect immediately (no recompile needed)

**Hot Reload Support:** Some subsystems support hot-reloading. Check individual subsystem documentation.

## Validation

JSON files should be validated before committing:
```bash
python -m json.tool filename.json
```

## Adding New Vehicles

1. Copy `DA_SakuraGTR.json` as a template
2. Rename to `DA_NewVehicleName.json`
3. Update all fields with new vehicle data
4. Create corresponding parts database in `Parts/DB_NewVehicleName_Parts.json`
5. Add to vehicle registry in code

## Version Control Notes

- All JSON files should use 2-space indentation
- Keep files under 500 lines when possible (split large databases)
- Document any breaking changes in commit messages

---
*Created: January 26, 2026*
*Project: MIDNIGHT GRIND*
