# Midnight Grind - Master Setup Script
# Run this single script to execute all setup steps
# Run in Unreal Editor: Window → Developer Tools → Output Log → Cmd → Python

import unreal
import importlib
import sys
import os

# ============================================
# MASTER SETUP SCRIPT
# ============================================

def run_all_setup():
    """Execute all setup scripts in order"""

    unreal.log("╔════════════════════════════════════════════════════════════╗")
    unreal.log("║       MIDNIGHT GRIND - PROJECT SETUP STARTING              ║")
    unreal.log("╚════════════════════════════════════════════════════════════╝")
    unreal.log("")

    # Get the directory containing this script
    # Scripts should be in Content/Python or a known location
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Add to path if needed
    if script_dir not in sys.path:
        sys.path.append(script_dir)

    # ============================================
    # STEP 1: Create Folder Structure
    # ============================================
    unreal.log("")
    unreal.log("┌────────────────────────────────────────────────────────────┐")
    unreal.log("│ STEP 1: Creating Folder Structure                          │")
    unreal.log("└────────────────────────────────────────────────────────────┘")

    try:
        import importlib
        folder_script = importlib.import_module("01_CreateFolderStructure")
        importlib.reload(folder_script)
    except Exception as e:
        # Run inline if import fails
        exec(open(os.path.join(script_dir, "01_CreateFolderStructure.py")).read())

    # ============================================
    # STEP 2: Create Core Blueprints
    # ============================================
    unreal.log("")
    unreal.log("┌────────────────────────────────────────────────────────────┐")
    unreal.log("│ STEP 2: Creating Core Blueprints                           │")
    unreal.log("└────────────────────────────────────────────────────────────┘")

    try:
        bp_script = importlib.import_module("02_CreateCoreBlueprints")
        importlib.reload(bp_script)
    except Exception as e:
        exec(open(os.path.join(script_dir, "02_CreateCoreBlueprints.py")).read())

    # ============================================
    # STEP 3: Create Widget Blueprints
    # ============================================
    unreal.log("")
    unreal.log("┌────────────────────────────────────────────────────────────┐")
    unreal.log("│ STEP 3: Creating Widget Blueprints                         │")
    unreal.log("└────────────────────────────────────────────────────────────┘")

    try:
        widget_script = importlib.import_module("03_CreateWidgetBlueprints")
        importlib.reload(widget_script)
    except Exception as e:
        exec(open(os.path.join(script_dir, "03_CreateWidgetBlueprints.py")).read())

    # ============================================
    # STEP 4: Create Input Assets
    # ============================================
    unreal.log("")
    unreal.log("┌────────────────────────────────────────────────────────────┐")
    unreal.log("│ STEP 4: Creating Input Assets                              │")
    unreal.log("└────────────────────────────────────────────────────────────┘")

    try:
        input_script = importlib.import_module("04_CreateInputAssets")
        importlib.reload(input_script)
    except Exception as e:
        exec(open(os.path.join(script_dir, "04_CreateInputAssets.py")).read())

    # ============================================
    # STEP 5: Create Maps and Setup
    # ============================================
    unreal.log("")
    unreal.log("┌────────────────────────────────────────────────────────────┐")
    unreal.log("│ STEP 5: Creating Maps and Project Setup                    │")
    unreal.log("└────────────────────────────────────────────────────────────┘")

    try:
        maps_script = importlib.import_module("05_CreateMapsAndSetup")
        importlib.reload(maps_script)
    except Exception as e:
        exec(open(os.path.join(script_dir, "05_CreateMapsAndSetup.py")).read())

    # ============================================
    # COMPLETE
    # ============================================
    unreal.log("")
    unreal.log("╔════════════════════════════════════════════════════════════╗")
    unreal.log("║       MIDNIGHT GRIND - SETUP COMPLETE!                     ║")
    unreal.log("╠════════════════════════════════════════════════════════════╣")
    unreal.log("║                                                            ║")
    unreal.log("║  Created:                                                  ║")
    unreal.log("║  ✓ Folder structure (20+ folders)                          ║")
    unreal.log("║  ✓ Core Blueprints (GameMode, Controller, etc.)            ║")
    unreal.log("║  ✓ Widget Blueprints (30+ UI widgets)                      ║")
    unreal.log("║  ✓ Input Actions and Mapping Contexts                      ║")
    unreal.log("║  ✓ Game Maps (MainMenu, Garage, Test Track)                ║")
    unreal.log("║                                                            ║")
    unreal.log("╠════════════════════════════════════════════════════════════╣")
    unreal.log("║  NEXT STEPS:                                               ║")
    unreal.log("║                                                            ║")
    unreal.log("║  1. Configure Project Settings (see output above)          ║")
    unreal.log("║  2. Open BP_BaseVehicle and add components                 ║")
    unreal.log("║  3. Open IMC_Vehicle and configure key bindings            ║")
    unreal.log("║  4. Open Widget Blueprints and design layouts              ║")
    unreal.log("║  5. Wire up Blueprint Event Graphs to C++ Subsystems       ║")
    unreal.log("║  6. Create test track geometry                             ║")
    unreal.log("║  7. Test play!                                             ║")
    unreal.log("║                                                            ║")
    unreal.log("╚════════════════════════════════════════════════════════════╝")


# Run the setup
run_all_setup()
