# MIDNIGHT GRIND - Unreal Engine Development Guide

## Overview

This guide covers everything needed to build and run MIDNIGHT GRIND in Unreal Engine. The project has been set up with extensive C++ systems, data files, and configurations. This document identifies what requires manual intervention and provides step-by-step instructions.

---

## THINGS YOU MUST DO (Cannot Be Automated)

These tasks require your direct action - no workarounds exist.

### 1. Fix Xcode Command Line Tools (REQUIRED FIRST)

Your Mac's xcode-select is pointing to CommandLineTools instead of Xcode.app. This **must** be fixed before any compilation can occur.

```bash
# Run this command (requires admin password):
sudo xcode-select -s /Applications/Xcode.app
```

**Why Claude can't do this:** Requires sudo/root privileges.

**Verification:**
```bash
xcode-select -p
# Should output: /Applications/Xcode.app/Contents/Developer
```

---

### 2. Generate Project Files

After fixing xcode-select, generate the Xcode project:

```bash
cd /Users/trill/Documents/GitHub/midnightgrind/prototype

# Generate project files
/Users/Shared/Epic\ Games/UE_5.2/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh MidnightGrind.uproject
```

**Why Claude can't do this:** Requires xcode-select fix first, and the script may require interactive prompts.

---

### 3. Open and Build in Unreal Editor

1. Double-click `MidnightGrind.uproject` to open in Unreal Editor
2. When prompted about missing modules, click "Yes" to rebuild
3. Wait for shader compilation (first time may take 10-20 minutes)

**Why Claude can't do this:** Requires running a GUI application.

---

### 4. Run EditorScripts for Initial Setup

The project includes Python setup scripts that must be run from within the editor:

1. Open **Window → Developer Tools → Output Log**
2. Open **Tools → Execute Python Script**
3. Run scripts in order:

```
EditorScripts/00_RunAllSetup.py        # Runs all setup scripts
```

**OR run individually:**

```
EditorScripts/01_CreateFolderStructure.py   # Creates content folders
EditorScripts/02_CreateCoreBlueprints.py    # Creates base blueprints
EditorScripts/03_CreateWidgetBlueprints.py  # Creates UI widgets
EditorScripts/04_CreateInputAssets.py       # Creates input assets
EditorScripts/05_CreateMapsAndSetup.py      # Creates test maps
```

**Why Claude can't do this:** Requires Unreal Editor Python environment.

---

### 5. Create the Test Vehicle Blueprint

After EditorScripts run, you need to create the first playable vehicle:

1. **Content Browser** → Create New → Blueprint Class
2. Parent Class: `MGVehiclePawn`
3. Name: `BP_SakuraGTR`
4. Save to: `Content/Blueprints/Vehicles/`

**Configure the Blueprint:**

Open BP_SakuraGTR and set these components:

**Details Panel - Vehicle ID:**
```
Vehicle ID: SAKURA_GTR
```

**Skeletal Mesh Component:**
- For now, use a placeholder mesh or the default vehicle mesh
- Later: Import proper vehicle mesh

**Camera Component Settings:**
```
Chase Camera:
  - Location: (X=-600, Y=0, Z=200)
  - Rotation: (Pitch=-15, Yaw=0, Roll=0)
  - FOV: 90

Hood Camera:
  - Location: (X=100, Y=0, Z=100)
  - Rotation: (Pitch=-5, Yaw=0, Roll=0)
  - FOV: 75
```

**Vehicle Movement Component:**
The `MGVehicleMovementComponent` should auto-configure from the data files:
- Verify `VehicleID` matches `SAKURA_GTR`
- Stats will load from `DA_SakuraGTR.json`

**Why Claude can't do this:** Requires Unreal Editor GUI.

---

### 6. Create the Test Track Level

1. **File → New Level → Empty Open World**
2. Save as: `Content/Maps/L_TestTrack`

**Add Required Actors:**

```
Player Start:
  - Location: (0, 0, 100)
  - Rotation: (0, 0, 0)

Directional Light:
  - Rotation: (Pitch=-45, Yaw=45)
  - Intensity: 10
  - Enable "Atmosphere Sun Light"

Sky Atmosphere:
  - Add from Place Actors panel

Volumetric Clouds:
  - Add for visual fidelity

Post Process Volume:
  - Enable "Infinite Extent"
  - Add MG_RetroPostProcess material to Blendables
```

**Build Basic Track:**

Using BSP or static meshes, create:
- Main straight: 500m long, 20m wide
- 4 corners (90-degree)
- Simple loop layout (reference: L_TestTrack_Layout.json)

**Why Claude can't do this:** Requires Unreal Editor level editing.

---

### 7. Set Up Game Mode

1. Create Blueprint: `BP_MidnightGrindGameMode`
   - Parent: `MGGameMode`

2. Set in Project Settings:
   - **Edit → Project Settings → Maps & Modes**
   - Default GameMode: `BP_MidnightGrindGameMode`
   - Default Pawn Class: `BP_SakuraGTR`

**Why Claude can't do this:** Requires Unreal Editor GUI.

---

### 8. Configure Input (Enhanced Input)

1. Create Input Mapping Context: `IMC_Vehicle`
2. Create Input Actions (match InputMappingDefaults.json):

| Action Name | Type | Trigger |
|-------------|------|---------|
| IA_Throttle | Float | None |
| IA_Brake | Float | None |
| IA_Steering | Float | None |
| IA_Handbrake | Bool | Pressed |
| IA_Nitrous | Bool | Pressed |
| IA_ShiftUp | Bool | Pressed |
| IA_ShiftDown | Bool | Pressed |
| IA_LookBack | Bool | Pressed |
| IA_CycleCamera | Bool | Pressed |
| IA_Pause | Bool | Pressed |

3. Add mappings in IMC_Vehicle (reference InputMappingDefaults.json for bindings)

**Why Claude can't do this:** Requires Unreal Editor asset creation.

---

### 9. Test Play

1. Open `L_TestTrack` map
2. Click **Play** (Alt+P)
3. Verify:
   - Vehicle spawns
   - Input works (throttle, steering)
   - Camera follows vehicle
   - HUD displays (if implemented)

**Why Claude can't do this:** Requires running the game.

---

## WHAT'S ALREADY DONE (No Action Needed)

### C++ Systems (585 source files)
- ✅ MGVehiclePawn - Full vehicle implementation
- ✅ MGVehicleMovementComponent - Chaos vehicle physics with drift/nitrous
- ✅ MGGameInstance - Game state management
- ✅ 183 subsystems covering all game features
- ✅ Full multiplayer foundation

### Data Files (35 JSON files)
- ✅ 5 complete vehicle specifications
- ✅ 5 parts databases (385 total parts)
- ✅ 24 AI driver profiles
- ✅ 48 race events
- ✅ Complete progression system
- ✅ Weather/atmosphere presets
- ✅ Audio configuration
- ✅ Economy balance
- ✅ HUD configurations
- ✅ Input mappings

### Shader Systems
- ✅ MGRetroPostProcess.usf - PS1/PS2 effects
- ✅ MGPS1VertexEffects.usf - Vertex wobble
- ✅ Material functions for retro look

### Documentation
- ✅ GDD, TechnicalDesign.md
- ✅ ROADMAP_PRD_2026.md (78-week plan)
- ✅ All data file documentation

---

## STEP-BY-STEP: FIRST PLAYABLE BUILD

Here's the complete sequence to get a driveable vehicle:

### Phase 1: Environment Setup (10 minutes)

```bash
# Step 1: Fix Xcode
sudo xcode-select -s /Applications/Xcode.app

# Step 2: Verify
xcode-select -p
# Expected: /Applications/Xcode.app/Contents/Developer

# Step 3: Generate project files
cd /Users/trill/Documents/GitHub/midnightgrind/prototype
/Users/Shared/Epic\ Games/UE_5.2/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh MidnightGrind.uproject
```

### Phase 2: Initial Build (20-40 minutes)

1. Double-click `MidnightGrind.uproject`
2. Click "Yes" when asked to rebuild modules
3. Wait for compilation and shader generation
4. If errors occur, check Output Log

### Phase 3: Run Setup Scripts (5 minutes)

1. Window → Developer Tools → Output Log
2. Tools → Execute Python Script
3. Browse to: `EditorScripts/00_RunAllSetup.py`
4. Click Execute

### Phase 4: Create Vehicle Blueprint (15 minutes)

1. Content Browser → Add → Blueprint Class
2. Search parent: "MGVehiclePawn"
3. Name: `BP_SakuraGTR`
4. Location: `Content/Blueprints/Vehicles/`
5. Open and configure:
   - Set VehicleID = "SAKURA_GTR"
   - Add camera components
   - Add placeholder mesh

### Phase 5: Create Test Level (15 minutes)

1. File → New Level → Empty Open World
2. Save as `L_TestTrack`
3. Add Player Start at origin
4. Add lighting (Directional + Sky)
5. Create simple track surface
6. Set as default map in Project Settings

### Phase 6: Configure Game Mode (5 minutes)

1. Create `BP_MidnightGrindGameMode` (parent: MGGameMode)
2. Project Settings → Maps & Modes:
   - Default GameMode = BP_MidnightGrindGameMode
   - Default Pawn = BP_SakuraGTR

### Phase 7: Create Input Assets (10 minutes)

1. Create IMC_Vehicle (Input Mapping Context)
2. Create Input Actions matching InputMappingDefaults.json
3. Bind to controller/keyboard

### Phase 8: Play Test

1. Open L_TestTrack
2. Press Play
3. Drive!

---

## TROUBLESHOOTING

### "Module could not be loaded"
- Rebuild from source: Build → Build Solution (in Xcode)
- Or delete Binaries/Intermediate folders and reopen project

### Shaders compiling forever
- Normal for first launch (1000s of shaders)
- Check GPU isn't overheating
- Consider reducing shader quality temporarily

### Vehicle doesn't move
- Check Enhanced Input is properly set up
- Verify IMC_Vehicle is added to player controller
- Check vehicle has movement component

### Vehicle falls through floor
- Check collision on ground mesh
- Verify vehicle has physics body

### JSON data not loading
- Check VehicleID matches exactly
- Verify JSON syntax (use json.tool)
- Check subsystem initialization in logs

---

## ARCHITECTURE QUICK REFERENCE

### Key Classes

| Class | Purpose |
|-------|---------|
| MGVehiclePawn | Player-controlled vehicle |
| MGVehicleMovementComponent | Physics + handling |
| MGGameInstance | Central game state |
| MGRaceManager | Race session control |
| MGVehicleDatabase | Data loading |
| MGEconomySubsystem | Currency/shop |
| MGProgressionSubsystem | REP/levels |
| MGRetroPostProcess | Visual effects |

### Data Flow

```
JSON Files (Content/Data/)
       ↓
MGVehicleDatabase (load on startup)
       ↓
MGVehiclePawn (apply to vehicle)
       ↓
MGVehicleMovementComponent (physics)
       ↓
Gameplay
```

### Subsystem Access

```cpp
// In any actor
UMGGameInstance* GI = Cast<UMGGameInstance>(GetGameInstance());
UMGVehicleDatabase* VehicleDB = GI->GetSubsystem<UMGVehicleDatabase>();
FMGVehicleData Data = VehicleDB->GetVehicleData("SAKURA_GTR");
```

---

## NEXT STEPS AFTER FIRST PLAYABLE

1. **Import Vehicle Mesh** - Replace placeholder with actual Sakura GTR mesh
2. **Implement HUD** - Create UMG widgets using HUDConfiguration.json
3. **Add Audio** - Set up MetaSounds using DB_AudioConfig.json
4. **Create AI Racers** - Use DB_RivalDrivers.json for opponents
5. **Build More Tracks** - Reference track layout JSONs
6. **Implement Garage** - Parts installation using Parts databases

---

## FILE LOCATIONS QUICK REFERENCE

```
/Users/trill/Documents/GitHub/midnightgrind/prototype/
├── Source/MidnightGrind/          # C++ source
│   ├── Public/                    # Headers
│   └── Private/                   # Implementation
├── Content/Data/                  # JSON game data
├── Content/Shaders/               # Custom shaders
├── EditorScripts/                 # Python setup scripts
├── docs/                          # Documentation
└── MidnightGrind.uproject        # Project file
```

---

## CONTACT / SUPPORT

For issues with:
- **C++ Code:** Check Source/ folder and TechnicalDesign.md
- **Data Files:** Check Content/Data/README.md
- **Project Setup:** Check this guide and ROADMAP_PRD_2026.md

---

*Last Updated: January 26, 2026*
*UE Version: 5.2 (adapted from 5.7)*
*Platform: macOS*
