# Midnight Grind - Unreal Editor Setup Scripts

These Python scripts automate the creation of Blueprints, Widgets, Maps, and project structure in Unreal Editor.

## Quick Start

### Option 1: Run Master Script (Recommended)

1. Copy the `EditorScripts` folder to your UE project's `Content/Python/` folder
2. Open Unreal Editor
3. Go to: `Window → Developer Tools → Output Log`
4. In the Output Log, change dropdown from "Cmd" to "Python"
5. Type: `exec(open("00_RunAllSetup.py").read())`
6. Press Enter

### Option 2: Run Individual Scripts

Run scripts in order:
```python
exec(open("01_CreateFolderStructure.py").read())
exec(open("02_CreateCoreBlueprints.py").read())
exec(open("03_CreateWidgetBlueprints.py").read())
exec(open("04_CreateInputAssets.py").read())
exec(open("05_CreateMapsAndSetup.py").read())
```

### Option 3: Use with MCP Server

If using an Unreal MCP server (like chongdashu/unreal-mcp):

```
Claude/AI: "Run the Python script at Content/Python/00_RunAllSetup.py"
```

The MCP will execute it in the editor.

---

## What Each Script Creates

### 01_CreateFolderStructure.py
Creates organized Content folder structure:
```
Content/MidnightGrind/
├── Blueprints/
│   ├── Core/
│   ├── Vehicles/
│   ├── UI/
│   ├── Racing/
│   └── AI/
├── Maps/
├── Materials/
├── Meshes/
├── Audio/
├── VFX/
└── Data/
```

### 02_CreateCoreBlueprints.py
Creates Blueprint classes:
- `BP_MidnightGrindGameMode` - Main game mode
- `BP_MainMenuGameMode` - Menu game mode
- `BP_RacingGameMode` - Race game mode
- `BP_MidnightGrindPlayerController` - Player controller
- `BP_MidnightGrindGameInstance` - Game instance
- `BP_MidnightGrindHUD` - HUD class
- `BP_BaseVehicle` - Base vehicle pawn
- `BP_RaceManager` - Race logic controller
- `BP_Checkpoint` - Race checkpoint
- `BP_WeatherController` - Weather visuals sync
- And more...

### 03_CreateWidgetBlueprints.py
Creates UI Widget Blueprints:
- Menus: MainMenu, PauseMenu, OptionsMenu, GarageMain
- HUD: RaceHUD, Speedometer, Tachometer, Minimap
- Garage: VehicleStats, PartSelection, TuningScreen
- Social: PlayerCard, CrewPanel, RivalCallout
- And 25+ more widgets...

### 04_CreateInputAssets.py
Creates Enhanced Input assets:
- Input Actions: Throttle, Brake, Steering, Handbrake, Nitrous, etc.
- Input Mapping Contexts: IMC_Vehicle, IMC_Menu, IMC_Debug

### 05_CreateMapsAndSetup.py
Creates game levels:
- `L_MainMenu` - Main menu level
- `L_Garage` - Garage level
- `L_TestTrack` - Development test track
- `L_Downtown`, `L_Highway`, `L_Canyon` - Race tracks

Also sets up test track with:
- Directional Light (moon)
- Sky Atmosphere
- Exponential Height Fog
- Post Process Volume
- Player Start

---

## After Running Scripts

### Manual Steps Required

1. **Project Settings** (`Edit → Project Settings`):
   - Maps & Modes → Default GameMode: `BP_MidnightGrindGameMode`
   - Maps & Modes → Game Instance: `BP_MidnightGrindGameInstance`
   - Maps & Modes → Default Map: `L_MainMenu`

2. **Configure Input Mappings**:
   - Open `IMC_Vehicle` in Content Browser
   - Add key/gamepad bindings for each Input Action

3. **Setup Vehicle Blueprint**:
   - Open `BP_BaseVehicle`
   - Add Skeletal Mesh component (car body)
   - Add Wheel components (4x)
   - Add Camera and Spring Arm
   - Wire up input events

4. **Design Widget Layouts**:
   - Open each WBP_ widget
   - Add UI elements in Designer tab
   - Wire up Event Graph to C++ subsystems

---

## Connecting to C++ Subsystems

In any Blueprint Event Graph:

```
Get Game Instance Subsystem → MGGarageSubsystem
                           → MGEconomySubsystem
                           → MGProgressionSubsystem
                           → MGDynoTuningSubsystem
                           → MGRivalSubsystem
                           → MGCrewSubsystem

Get World Subsystem → MGWeatherSubsystem
                   → MGVehicleWearSubsystem
```

### Example: Display Player Cash

```
Event Construct
    ↓
Get Game Instance Subsystem (MGEconomySubsystem)
    ↓
Get Player Cash (returns int64)
    ↓
Format Text ("${0}")
    ↓
Set Text (Text Block)
```

---

## Troubleshooting

### "Module not found" error
- Ensure scripts are in `Content/Python/` folder
- Or provide full path: `exec(open("C:/Projects/MG/Content/Python/script.py").read())`

### "Class not found" error
- Make sure C++ code is compiled first
- Check module name matches: "MidnightGrind"

### Scripts run but nothing created
- Check Output Log for errors
- Ensure you have write permissions
- Try running as Administrator

### Blueprint parent class not found
- Compile C++ first
- Scripts fall back to engine classes if custom classes missing

---

## Script Customization

Edit the scripts to:
- Add more Blueprints
- Change folder paths
- Add different components
- Modify default values

All scripts are standalone Python - no external dependencies.
