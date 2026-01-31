# Midnight Grind - MCP Blueprint Builder Quick Start

## Overview

The Unreal MCP (Model Context Protocol) allows AI assistants to directly control Unreal Engine - creating Blueprints, wiring up event graphs, building UI widgets, and more.

This guide will get you from zero to automated Blueprint generation.

---

## Prerequisites

- ✅ Unreal Engine 5.7 installed at `C:\Program Files\Epic Games\UE_5.7`
- ✅ Python 3.12+ installed
- ✅ UnrealMCP plugin copied to `Plugins/UnrealMCP`
- ✅ MCP Blueprint Builder script at `Scripts/mcp_blueprint_builder.py`

---

## Step 1: Generate Visual Studio Files

```powershell
# Right-click MidnightGrind.uproject → Generate Visual Studio project files
# OR run from command line:
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" MidnightGrind Win64 Development -Project="C:\Users\trill\Documents\GitHub\midnightgrind\prototype\MidnightGrind.uproject" -WaitMutex -FromMsBuild
```

---

## Step 2: Build the Project

Open `MidnightGrind.sln` in Visual Studio and build with:
- Configuration: `Development Editor`
- Platform: `Win64`

Or use command line:
```powershell
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" MidnightGrindEditor Win64 Development -Project="C:\Users\trill\Documents\GitHub\midnightgrind\prototype\MidnightGrind.uproject"
```

---

## Step 3: Open Unreal Editor

Double-click `MidnightGrind.uproject` or:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\trill\Documents\GitHub\midnightgrind\prototype\MidnightGrind.uproject"
```

Wait for shaders to compile (first launch takes a while).

---

## Step 4: Verify MCP Plugin Active

1. In Unreal Editor, go to **Edit → Plugins**
2. Search for "UnrealMCP"
3. Ensure it's enabled (should be auto-enabled from .uproject)
4. Check **Output Log** for: `UnrealMCP: TCP Server started on port 55557`

---

## Step 5: Run MCP Blueprint Builder

Open PowerShell in the `Scripts` folder:

```powershell
cd C:\Users\trill\Documents\GitHub\midnightgrind\prototype\Scripts

# Option A: Using uv (recommended)
uv run mcp_blueprint_builder.py

# Option B: Using standard Python
python mcp_blueprint_builder.py
```

---

## What Gets Created

The script generates **55+ Blueprints** organized by category:

### Core Blueprints (6)
| Blueprint | Parent C++ Class | Purpose |
|-----------|------------------|---------|
| BP_MGGameInstance | MGGameInstance | Global game state, save/load |
| BP_MGGameMode | MGGameModeBase | Match rules, spawn logic |
| BP_MGPlayerController | MGPlayerController | Input handling, HUD |
| BP_MGPlayerState | MGPlayerState | Per-player stats |
| BP_MGHUD | MGHUD | HUD management |
| BP_MGGameState | MGGameState | Replicated match state |

### Vehicle Blueprints (4)
| Blueprint | Parent C++ Class | Purpose |
|-----------|------------------|---------|
| BP_BaseVehicle | MGVehiclePawn | Base driveable vehicle |
| BP_SakuraGTR | MGVehiclePawn | Hero car (JDM) |
| BP_TrafficVehicle | MGTrafficVehiclePawn | NPC traffic cars |
| BP_PoliceVehicle | MGPoliceVehiclePawn | Police pursuit cars |

### Racing Blueprints (6)
| Blueprint | Purpose |
|-----------|---------|
| BP_RaceManager | Orchestrates race flow |
| BP_Checkpoint | Track checkpoints |
| BP_StartLine | Race start trigger |
| BP_FinishLine | Race finish trigger |
| BP_RaceRoute | Spline-based track definition |
| BP_DragStageTree | Drag race staging lights |

### AI Blueprints (3)
| Blueprint | Purpose |
|-----------|---------|
| BP_RaceAIController | AI opponent driving |
| BP_TrafficAIController | Traffic NPC behavior |
| BP_PoliceAIController | Police chase AI |

### UI Widgets (28)
- Main menu, pause menu, options
- Full racing HUD (speedometer, tachometer, minimap, position, lap counter, nitro, drift score, heat level)
- Garage UI (stats, parts list, tuning, livery editor, dyno)
- Race screens (select, countdown, results, pink slip confirm)
- Social (player card, crew panel, leaderboard, chat)

---

## After Running the Script

### 1. Set Project Defaults

Go to **Edit → Project Settings → Maps & Modes**:
- Default GameMode: `BP_MGGameMode`
- Game Instance Class: `BP_MGGameInstance`
- Player Controller Class: `BP_MGPlayerController`
- Default Pawn Class: `BP_BaseVehicle`
- HUD Class: `BP_MGHUD`

### 2. Create a Test Level

1. **File → New Level → Empty Open World**
2. Add a floor (simple cube scaled up)
3. Drag in `BP_BaseVehicle` from Content Browser
4. Add `PlayerStart` actor
5. **Play** - You should be driving!

### 3. Customize Blueprints

Open any Blueprint in the Content Browser:
- `/Game/MidnightGrind/Blueprints/` - All game Blueprints
- `/Game/MidnightGrind/UI/` - All widget Blueprints

The MCP has already:
- Added BeginPlay/Tick events
- Set up vehicle variables (MaxSpeed, CurrentGear, bNitroActive)
- Added trigger boxes to checkpoints
- Built basic UI elements

Now you extend them with custom logic!

---

## Troubleshooting

### "Could not connect to Unreal Engine"
- Make sure Unreal Editor is open
- Check Output Log for "TCP Server started on port 55557"
- Try restarting the editor

### "Blueprint parent class not found"
- The C++ code needs to compile first
- Build the project in Visual Studio before running MCP script

### "Permission denied"
- Run PowerShell as Administrator
- Or check file permissions on the project folder

---

## Advanced: Direct MCP Commands

You can send individual commands via Python:

```python
from mcp_blueprint_builder import UnrealMCP

mcp = UnrealMCP()

# Create a custom Blueprint
mcp.create_blueprint("BP_MyActor", "Actor", "/Game/MyBlueprints")

# Add a component
mcp.add_component("BP_MyActor", "StaticMeshComponent", "Mesh")

# Add event
mcp.add_event_node("BP_MyActor", "BeginPlay")

# Compile
mcp.compile_blueprint("BP_MyActor")
```

---

## MCP Tool Reference

| Command | Description |
|---------|-------------|
| `create_blueprint` | Create new Blueprint class |
| `add_component_to_blueprint` | Add component to BP |
| `add_blueprint_event_node` | Add BeginPlay, Tick, etc. |
| `add_blueprint_function_node` | Add function call node |
| `add_blueprint_variable` | Add variable to BP |
| `connect_blueprint_nodes` | Wire up node graph |
| `compile_blueprint` | Compile changes |
| `set_pawn_properties` | Configure pawn settings |
| `create_umg_widget_blueprint` | Create UI widget |
| `add_text_block_to_widget` | Add text to widget |
| `add_button_to_widget` | Add button to widget |

Full docs: `C:\Users\trill\Documents\unreal-mcp\Docs\Tools\`

---

## Next Steps

1. ✅ Run MCP Blueprint Builder
2. ⬜ Configure Project Settings
3. ⬜ Create test level
4. ⬜ Import placeholder vehicle mesh
5. ⬜ Tune vehicle physics
6. ⬜ Build first race track
7. ⬜ Test core loop: Drive → Race → Win → Earn → Upgrade

**You're ready to build!** 🏎️
