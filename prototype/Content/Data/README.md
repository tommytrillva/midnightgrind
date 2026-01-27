# MIDNIGHT GRIND - Game Data Files

This directory contains JSON data files that define game content, balance, and configuration. These files are designed to be loaded by the game's data subsystems and can be modified without recompiling code.

## Directory Structure

```
Content/Data/
├── AI/
│   ├── AIDriverProfiles.json         # AI behavior profiles
│   └── DB_RivalDrivers.json          # 24 rival drivers with backstories
├── Audio/
│   ├── AudioProfiles.json            # Base audio settings
│   └── DB_AudioConfig.json           # Engine sounds, exhausts, ambient audio
├── Career/
│   └── CareerChapters.json           # Story mode chapter definitions
├── Crews/
│   └── CrewTerritories.json          # Crew system territories
├── Customization/
│   └── DB_VisualCustomization.json   # Paint, wheels, vinyls, body kits
├── Economy/
│   └── EconomyBalanceConfig.json     # Currency, payouts, pricing, REP system
├── Environment/
│   └── DB_WeatherAtmosphere.json     # Weather, time of day, retro effects
├── Events/
│   ├── DailyWeeklyChallenges.json    # Daily/weekly event rotation
│   └── DB_RaceEvents.json            # 66 race events across 6 chapters
├── Features/
│   └── DB_PhotoMode.json             # Photo mode configuration
├── Gameplay/
│   └── DB_PoliceHeatSystem.json      # Police pursuits and heat levels
├── Input/
│   ├── InputMappingDefaults.json     # Controller/keyboard bindings
│   └── WheelProfiles/                # Racing wheel configurations
├── Multiplayer/
│   └── DB_MultiplayerConfig.json     # Online, matchmaking, ranked, crews
├── Parts/
│   ├── DB_BeaterSedan_Parts.json     # Beater Sedan upgrades (68 parts)
│   ├── DB_KazeCivic_Parts.json       # Kaze Civic upgrades (72 parts)
│   ├── DB_SakuraGTR_Parts.json       # Sakura GTR upgrades (85 parts)
│   ├── DB_StallionGT_Parts.json      # Stallion GT upgrades (82 parts)
│   └── DB_Tenshi240_Parts.json       # Tenshi 240 upgrades (78 parts)
├── Progression/
│   └── DB_ProgressionSystem.json     # REP tiers, achievements, prestige
├── Seasons/
│   └── SeasonPassStructure.json      # Seasonal content structure
├── Settings/
│   └── DB_DifficultyAccessibility.json # Difficulty and accessibility options
├── Shop/
│   └── DB_GarageShop.json            # Garage, dealers, part shops, auctions
├── Tracks/
│   ├── L_HarunaPass_Layout.json      # Mountain pass track
│   ├── L_IndustrialDocks_Layout.json # Industrial area track
│   ├── L_TestTrack_Layout.json       # Development test track
│   └── L_WanganHighway_Layout.json   # Highway racing track
├── Tuning/
│   └── TuningPresets.json            # Pre-built tuning configurations
├── Tutorials/
│   └── DB_TutorialOnboarding.json    # Tutorials, FTUE, contextual hints
├── UI/
│   └── DB_HUDConfiguration.json      # HUD profiles, elements, notifications
├── Vehicles/
│   ├── DA_BeaterSedan.json           # Starter vehicle specification
│   ├── DA_KazeCivic.json             # Entry JDM hatchback
│   ├── DA_SakuraGTR.json             # Hero vehicle specification
│   ├── DA_StallionGT.json            # American muscle entry
│   ├── DA_Tenshi240.json             # Drift platform
│   └── VehicleRosterPlan.json        # 30 vehicle development roadmap
└── README.md                          # This file
```

**Total: 42 JSON data files defining complete game systems**

## Key File Descriptions

### AI/DB_RivalDrivers.json
- **Purpose:** Defines all AI racing opponents
- **Contains:** 24 unique drivers across 5 difficulty tiers (Rookie → Legend), driving styles, behavior profiles, dialogue
- **Used by:** MGAIRacerSubsystem, MGDialogueSubsystem

### Audio/DB_AudioConfig.json
- **Purpose:** Complete audio configuration
- **Contains:** 7 engine profiles (I4 econobox → V8 supercharged), exhaust types, turbo/supercharger sounds, ambient audio
- **Used by:** MGAudioSubsystem, MetaSounds

### Economy/EconomyBalanceConfig.json
- **Purpose:** Defines all economic values in the game
- **Contains:** Currency types, REP tier thresholds, race payouts, part pricing multipliers, maintenance costs
- **Used by:** MGEconomySubsystem, MGShopSubsystem, MGProgressionSubsystem

### Environment/DB_WeatherAtmosphere.json
- **Purpose:** Weather, time of day, and visual effects
- **Contains:** 8 time periods, 5 weather types, retro effect presets (PS1/PS2 modes), neon lighting config
- **Used by:** MGTimeWeatherSubsystem, MGRetroPostProcess

### Events/DB_RaceEvents.json
- **Purpose:** All race event definitions
- **Contains:** 66 events across 6 story chapters, 8 event types, boss races, track-specific events
- **Used by:** MGRaceManagerSubsystem, MGCareerSubsystem

### Features/DB_PhotoMode.json
- **Purpose:** Complete photo mode configuration
- **Contains:** Camera controls, filters, effects, stickers, borders, output settings, gallery, sharing, challenges
- **Used by:** MGPhotoModeSubsystem, MGSharingSubsystem

### Gameplay/DB_PoliceHeatSystem.json
- **Purpose:** Police pursuit and heat system
- **Contains:** 5 heat levels, police unit types, pursuit tactics, bounty system, safe houses, escape rewards
- **Used by:** MGPoliceSubsystem, MGPursuitSubsystem

### Multiplayer/DB_MultiplayerConfig.json
- **Purpose:** All online and multiplayer configuration
- **Contains:** Network settings, matchmaking, 15 game modes, ranked system, crews, voice chat, anti-cheat
- **Used by:** MGOnlineSubsystem, MGMatchmakingSubsystem, MGCrewSubsystem

### Settings/DB_DifficultyAccessibility.json
- **Purpose:** Difficulty and accessibility options
- **Contains:** 4 difficulty presets, driving assists, colorblind modes, input profiles, UI scaling
- **Used by:** MGSettingsSubsystem, MGAccessibilitySubsystem

### Shop/DB_GarageShop.json
- **Purpose:** Garage features and shop configuration
- **Contains:** Garage slots, 5 dealers, 5 part shops, transactions, auctions, maintenance
- **Used by:** MGGarageSubsystem, MGShopSubsystem, MGAuctionSubsystem

### Tutorials/DB_TutorialOnboarding.json
- **Purpose:** Tutorial and new player experience
- **Contains:** FTUE sequence, driving/drifting/tuning tutorials, contextual hints, loading tips
- **Used by:** MGTutorialSubsystem, MGOnboardingSubsystem

### UI/DB_HUDConfiguration.json
- **Purpose:** HUD layout and display configuration
- **Contains:** 5 HUD profiles, element variants, notifications, countdown styles, customization
- **Used by:** MGHUDSubsystem, MGWidgetManager

### Parts/DB_*_Parts.json (5 files)
- **Purpose:** Vehicle-specific upgrade catalogs
- **Contains:** 68-85 parts per vehicle with stats, prices, PI impacts, upgrade packages
- **Used by:** MGCustomizationSubsystem, MGShopSubsystem

### Progression/DB_ProgressionSystem.json
- **Purpose:** Complete progression and achievement system
- **Contains:** 6 REP tiers, 100 levels, 50+ achievements, crew progression, prestige system, titles
- **Used by:** MGProgressionSubsystem, MGAchievementSubsystem

### Vehicles/DA_*.json (5 files)
- **Purpose:** Complete vehicle specifications
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
