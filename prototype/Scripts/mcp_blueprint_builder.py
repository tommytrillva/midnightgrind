"""
Midnight Grind - MCP Blueprint Builder
======================================
This script uses the Unreal MCP to create and wire up all Blueprints
based on the PRD requirements.

Run with: uv run mcp_blueprint_builder.py
Requires: Unreal Editor open with MCP plugin active on port 55557
"""

import socket
import json
import time
from typing import Dict, Any, Optional, List

# ============================================
# MCP CONNECTION
# ============================================

UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557

class UnrealMCP:
    """Simple MCP client for Unreal Engine"""
    
    def __init__(self):
        self.socket = None
    
    def send_command(self, command: str, params: Dict[str, Any] = None) -> Dict:
        """Send command to Unreal MCP and get response"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(30)
            sock.connect((UNREAL_HOST, UNREAL_PORT))
            
            cmd = {"type": command, "params": params or {}}
            sock.sendall(json.dumps(cmd).encode('utf-8'))
            
            # Receive response
            chunks = []
            while True:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                chunks.append(chunk)
                try:
                    data = b''.join(chunks).decode('utf-8')
                    response = json.loads(data)
                    sock.close()
                    return response
                except json.JSONDecodeError:
                    continue
            
            sock.close()
            return {"success": False, "error": "No response"}
            
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def create_blueprint(self, name: str, parent_class: str, path: str = "/Game/MidnightGrind/Blueprints") -> Dict:
        """Create a new Blueprint"""
        return self.send_command("create_blueprint", {
            "name": name,
            "parent_class": parent_class,
            "path": path
        })
    
    def add_component(self, blueprint: str, component_type: str, component_name: str, 
                      location: List[float] = None, properties: Dict = None) -> Dict:
        """Add component to Blueprint"""
        return self.send_command("add_component_to_blueprint", {
            "blueprint_name": blueprint,
            "component_type": component_type,
            "component_name": component_name,
            "location": location or [0, 0, 0],
            "component_properties": properties or {}
        })
    
    def add_event_node(self, blueprint: str, event_type: str) -> Dict:
        """Add an event node to Blueprint graph"""
        return self.send_command("add_blueprint_event_node", {
            "blueprint_name": blueprint,
            "event_type": event_type
        })
    
    def add_function_node(self, blueprint: str, target: str, function_name: str) -> Dict:
        """Add a function call node"""
        return self.send_command("add_blueprint_function_node", {
            "blueprint_name": blueprint,
            "target": target,
            "function_name": function_name
        })
    
    def add_variable(self, blueprint: str, var_name: str, var_type: str, default_value: Any = None) -> Dict:
        """Add a variable to Blueprint"""
        params = {
            "blueprint_name": blueprint,
            "variable_name": var_name,
            "variable_type": var_type
        }
        if default_value is not None:
            params["default_value"] = default_value
        return self.send_command("add_blueprint_variable", params)
    
    def connect_nodes(self, blueprint: str, source_node: str, source_pin: str, 
                      target_node: str, target_pin: str) -> Dict:
        """Connect two nodes in Blueprint graph"""
        return self.send_command("connect_blueprint_nodes", {
            "blueprint_name": blueprint,
            "source_node_id": source_node,
            "source_pin": source_pin,
            "target_node_id": target_node,
            "target_pin": target_pin
        })
    
    def compile_blueprint(self, blueprint: str) -> Dict:
        """Compile a Blueprint"""
        return self.send_command("compile_blueprint", {
            "blueprint_name": blueprint
        })
    
    def set_pawn_properties(self, blueprint: str, auto_possess: str = "Player0") -> Dict:
        """Set pawn properties"""
        return self.send_command("set_pawn_properties", {
            "blueprint_name": blueprint,
            "auto_possess_player": auto_possess,
            "use_controller_rotation_yaw": True,
            "can_be_damaged": True
        })
    
    def create_widget(self, name: str, parent_class: str = "UserWidget", path: str = "/Game/MidnightGrind/UI") -> Dict:
        """Create a UMG Widget Blueprint"""
        return self.send_command("create_umg_widget_blueprint", {
            "widget_name": name,
            "parent_class": parent_class,
            "path": path
        })
    
    def add_text_block(self, widget: str, name: str, text: str, position: List[int], 
                       size: List[int], font_size: int = 24) -> Dict:
        """Add text block to widget"""
        return self.send_command("add_text_block_to_widget", {
            "widget_name": widget,
            "text_block_name": name,
            "text": text,
            "position": position,
            "size": size,
            "font_size": font_size,
            "color": [1, 1, 1, 1]
        })
    
    def add_button(self, widget: str, name: str, text: str, position: List[int], size: List[int]) -> Dict:
        """Add button to widget"""
        return self.send_command("add_button_to_widget", {
            "widget_name": widget,
            "button_name": name,
            "text": text,
            "position": position,
            "size": size,
            "font_size": 18,
            "color": [1, 1, 1, 1],
            "background_color": [0.1, 0.1, 0.1, 1]
        })


# ============================================
# BLUEPRINT DEFINITIONS
# ============================================

# Core system blueprints - inherit from C++ classes
CORE_BLUEPRINTS = [
    # (Name, C++ Parent Class, Folder)
    ("BP_MGGameInstance", "MGGameInstance", "/Game/MidnightGrind/Blueprints/Core"),
    ("BP_MGGameMode", "MGGameModeBase", "/Game/MidnightGrind/Blueprints/Core"),
    ("BP_MGPlayerController", "MGPlayerController", "/Game/MidnightGrind/Blueprints/Core"),
    ("BP_MGPlayerState", "MGPlayerState", "/Game/MidnightGrind/Blueprints/Core"),
    ("BP_MGHUD", "MGHUD", "/Game/MidnightGrind/Blueprints/Core"),
    ("BP_MGGameState", "MGGameState", "/Game/MidnightGrind/Blueprints/Core"),
]

# Vehicle blueprints
VEHICLE_BLUEPRINTS = [
    ("BP_BaseVehicle", "MGVehiclePawn", "/Game/MidnightGrind/Blueprints/Vehicles"),
    ("BP_SakuraGTR", "MGVehiclePawn", "/Game/MidnightGrind/Blueprints/Vehicles"),  # Hero car
    ("BP_TrafficVehicle", "MGTrafficVehiclePawn", "/Game/MidnightGrind/Blueprints/Vehicles"),
    ("BP_PoliceVehicle", "MGPoliceVehiclePawn", "/Game/MidnightGrind/Blueprints/Vehicles"),
]

# Racing system blueprints
RACING_BLUEPRINTS = [
    ("BP_RaceManager", "MGRaceManager", "/Game/MidnightGrind/Blueprints/Racing"),
    ("BP_Checkpoint", "MGCheckpoint", "/Game/MidnightGrind/Blueprints/Racing"),
    ("BP_StartLine", "MGStartLine", "/Game/MidnightGrind/Blueprints/Racing"),
    ("BP_FinishLine", "MGFinishLine", "/Game/MidnightGrind/Blueprints/Racing"),
    ("BP_RaceRoute", "MGRaceRoute", "/Game/MidnightGrind/Blueprints/Racing"),
    ("BP_DragStageTree", "MGDragStageTree", "/Game/MidnightGrind/Blueprints/Racing"),
]

# AI blueprints
AI_BLUEPRINTS = [
    ("BP_RaceAIController", "MGRaceAIController", "/Game/MidnightGrind/Blueprints/AI"),
    ("BP_TrafficAIController", "MGTrafficAIController", "/Game/MidnightGrind/Blueprints/AI"),
    ("BP_PoliceAIController", "MGPoliceAIController", "/Game/MidnightGrind/Blueprints/AI"),
]

# Environment blueprints
ENVIRONMENT_BLUEPRINTS = [
    ("BP_WeatherController", "MGWeatherSubsystem", "/Game/MidnightGrind/Blueprints/Environment"),
    ("BP_TimeOfDayController", "MGTimeOfDaySubsystem", "/Game/MidnightGrind/Blueprints/Environment"),
    ("BP_StreetLight", "Actor", "/Game/MidnightGrind/Blueprints/Environment"),
    ("BP_NeonSign", "Actor", "/Game/MidnightGrind/Blueprints/Environment"),
    ("BP_TrafficLight", "MGTrafficLight", "/Game/MidnightGrind/Blueprints/Environment"),
]

# Garage/customization blueprints  
GARAGE_BLUEPRINTS = [
    ("BP_Garage", "MGGarage", "/Game/MidnightGrind/Blueprints/Garage"),
    ("BP_VehicleDisplay", "MGVehicleDisplay", "/Game/MidnightGrind/Blueprints/Garage"),
    ("BP_Dyno", "MGDyno", "/Game/MidnightGrind/Blueprints/Garage"),
    ("BP_PartShop", "MGPartShop", "/Game/MidnightGrind/Blueprints/Garage"),
]

# UI Widgets
UI_WIDGETS = [
    # Main menus
    ("WBP_MainMenu", "UserWidget", "/Game/MidnightGrind/UI/Menus"),
    ("WBP_PauseMenu", "UserWidget", "/Game/MidnightGrind/UI/Menus"),
    ("WBP_OptionsMenu", "UserWidget", "/Game/MidnightGrind/UI/Menus"),
    
    # HUD elements
    ("WBP_RaceHUD", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_Speedometer", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_Tachometer", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_Minimap", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_PositionIndicator", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_LapCounter", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_NitroGauge", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_DriftScore", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    ("WBP_HeatLevel", "UserWidget", "/Game/MidnightGrind/UI/HUD"),
    
    # Garage UI
    ("WBP_GarageMain", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    ("WBP_VehicleStats", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    ("WBP_PartsList", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    ("WBP_PartDetails", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    ("WBP_TuningSliders", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    ("WBP_LiveryEditor", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    ("WBP_DynoResults", "UserWidget", "/Game/MidnightGrind/UI/Garage"),
    
    # Race screens
    ("WBP_RaceSelect", "UserWidget", "/Game/MidnightGrind/UI/Racing"),
    ("WBP_PreRaceCountdown", "UserWidget", "/Game/MidnightGrind/UI/Racing"),
    ("WBP_RaceResults", "UserWidget", "/Game/MidnightGrind/UI/Racing"),
    ("WBP_PinkSlipConfirm", "UserWidget", "/Game/MidnightGrind/UI/Racing"),
    
    # Social/multiplayer
    ("WBP_PlayerCard", "UserWidget", "/Game/MidnightGrind/UI/Social"),
    ("WBP_CrewPanel", "UserWidget", "/Game/MidnightGrind/UI/Social"),
    ("WBP_Leaderboard", "UserWidget", "/Game/MidnightGrind/UI/Social"),
    ("WBP_ChatBox", "UserWidget", "/Game/MidnightGrind/UI/Social"),
]


# ============================================
# BUILD FUNCTIONS
# ============================================

def log(msg: str, level: str = "INFO"):
    """Print formatted log message"""
    print(f"[{level}] {msg}")

def build_core_blueprints(mcp: UnrealMCP):
    """Create core game Blueprints"""
    log("=" * 50)
    log("BUILDING CORE BLUEPRINTS")
    log("=" * 50)
    
    for name, parent, path in CORE_BLUEPRINTS:
        log(f"Creating {name} (parent: {parent})")
        result = mcp.create_blueprint(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            
            # Add BeginPlay event
            mcp.add_event_node(name, "BeginPlay")
            log(f"  + Added BeginPlay event")
            
            # Compile
            mcp.compile_blueprint(name)
            log(f"  ✓ Compiled")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.5)  # Rate limiting

def build_vehicle_blueprints(mcp: UnrealMCP):
    """Create vehicle Blueprints"""
    log("=" * 50)
    log("BUILDING VEHICLE BLUEPRINTS")
    log("=" * 50)
    
    for name, parent, path in VEHICLE_BLUEPRINTS:
        log(f"Creating {name} (parent: {parent})")
        result = mcp.create_blueprint(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            
            # Set pawn properties for player possession
            if "Base" in name:
                mcp.set_pawn_properties(name, "Player0")
                log(f"  + Set as Player0 possession")
            
            # Add common events
            mcp.add_event_node(name, "BeginPlay")
            mcp.add_event_node(name, "Tick")
            log(f"  + Added BeginPlay and Tick events")
            
            # Add vehicle-specific variables
            mcp.add_variable(name, "MaxSpeed", "Float", 200.0)
            mcp.add_variable(name, "CurrentGear", "Integer", 1)
            mcp.add_variable(name, "bNitroActive", "Boolean", False)
            log(f"  + Added vehicle variables")
            
            mcp.compile_blueprint(name)
            log(f"  ✓ Compiled")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.5)

def build_racing_blueprints(mcp: UnrealMCP):
    """Create racing system Blueprints"""
    log("=" * 50)
    log("BUILDING RACING BLUEPRINTS")
    log("=" * 50)
    
    for name, parent, path in RACING_BLUEPRINTS:
        log(f"Creating {name} (parent: {parent})")
        result = mcp.create_blueprint(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            
            # Add collision component for checkpoints
            if "Checkpoint" in name or "Line" in name:
                mcp.add_component(name, "BoxComponent", "TriggerBox", [0, 0, 0])
                log(f"  + Added TriggerBox component")
            
            mcp.add_event_node(name, "BeginPlay")
            mcp.compile_blueprint(name)
            log(f"  ✓ Compiled")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.5)

def build_ai_blueprints(mcp: UnrealMCP):
    """Create AI controller Blueprints"""
    log("=" * 50)
    log("BUILDING AI BLUEPRINTS")
    log("=" * 50)
    
    for name, parent, path in AI_BLUEPRINTS:
        log(f"Creating {name} (parent: {parent})")
        result = mcp.create_blueprint(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            mcp.add_event_node(name, "BeginPlay")
            mcp.add_event_node(name, "Tick")
            mcp.compile_blueprint(name)
            log(f"  ✓ Compiled")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.5)

def build_environment_blueprints(mcp: UnrealMCP):
    """Create environment Blueprints"""
    log("=" * 50)
    log("BUILDING ENVIRONMENT BLUEPRINTS")
    log("=" * 50)
    
    for name, parent, path in ENVIRONMENT_BLUEPRINTS:
        log(f"Creating {name} (parent: {parent})")
        result = mcp.create_blueprint(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            
            # Add light components to light actors
            if "Light" in name or "Sign" in name:
                mcp.add_component(name, "PointLightComponent", "Light", [0, 0, 100])
                log(f"  + Added PointLight component")
            
            mcp.add_event_node(name, "BeginPlay")
            mcp.compile_blueprint(name)
            log(f"  ✓ Compiled")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.5)

def build_garage_blueprints(mcp: UnrealMCP):
    """Create garage/customization Blueprints"""
    log("=" * 50)
    log("BUILDING GARAGE BLUEPRINTS")
    log("=" * 50)
    
    for name, parent, path in GARAGE_BLUEPRINTS:
        log(f"Creating {name} (parent: {parent})")
        result = mcp.create_blueprint(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            mcp.add_event_node(name, "BeginPlay")
            mcp.compile_blueprint(name)
            log(f"  ✓ Compiled")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.5)

def build_ui_widgets(mcp: UnrealMCP):
    """Create UI Widget Blueprints"""
    log("=" * 50)
    log("BUILDING UI WIDGETS")
    log("=" * 50)
    
    for name, parent, path in UI_WIDGETS:
        log(f"Creating {name}")
        result = mcp.create_widget(name, parent, path)
        
        if result.get("success") or result.get("status") == "success":
            log(f"  ✓ Created {name}")
            
            # Add specific widgets based on type
            if "Speedometer" in name:
                mcp.add_text_block(name, "SpeedText", "0", [50, 50], [200, 80], 48)
                mcp.add_text_block(name, "UnitText", "MPH", [260, 80], [60, 30], 18)
                log(f"  + Added speedometer elements")
            
            elif "MainMenu" in name:
                mcp.add_text_block(name, "TitleText", "MIDNIGHT GRIND", [400, 100], [400, 100], 64)
                mcp.add_button(name, "PlayButton", "PLAY", [500, 300], [200, 60])
                mcp.add_button(name, "GarageButton", "GARAGE", [500, 380], [200, 60])
                mcp.add_button(name, "OptionsButton", "OPTIONS", [500, 460], [200, 60])
                mcp.add_button(name, "QuitButton", "QUIT", [500, 540], [200, 60])
                log(f"  + Added main menu elements")
            
            elif "RaceResults" in name:
                mcp.add_text_block(name, "PositionText", "1ST", [500, 150], [200, 100], 72)
                mcp.add_text_block(name, "TimeText", "0:00.000", [500, 280], [200, 50], 36)
                mcp.add_text_block(name, "CashEarned", "+$0", [500, 350], [200, 40], 28)
                mcp.add_button(name, "ContinueButton", "CONTINUE", [500, 500], [200, 60])
                log(f"  + Added race results elements")
            
            elif "HeatLevel" in name:
                mcp.add_text_block(name, "HeatText", "HEAT: 0", [10, 10], [150, 40], 24)
                log(f"  + Added heat level elements")
        else:
            error = result.get("error", result.get("message", "Unknown error"))
            log(f"  ✗ Failed: {error}", "ERROR")
        
        time.sleep(0.3)


# ============================================
# MAIN EXECUTION
# ============================================

def build_all():
    """Build all Blueprints via MCP"""
    log("=" * 60)
    log("MIDNIGHT GRIND - MCP BLUEPRINT BUILDER")
    log("=" * 60)
    log("")
    log("Connecting to Unreal MCP...")
    
    mcp = UnrealMCP()
    
    # Test connection
    test = mcp.send_command("get_actors_in_level", {})
    if test.get("success") or test.get("status") == "success" or "actors" in str(test):
        log("✓ Connected to Unreal Engine!")
    else:
        log("✗ Could not connect to Unreal Engine!", "ERROR")
        log("  Make sure Unreal Editor is open with MCP plugin active", "ERROR")
        log(f"  Response: {test}", "DEBUG")
        return False
    
    log("")
    
    # Build all blueprints
    build_core_blueprints(mcp)
    build_vehicle_blueprints(mcp)
    build_racing_blueprints(mcp)
    build_ai_blueprints(mcp)
    build_environment_blueprints(mcp)
    build_garage_blueprints(mcp)
    build_ui_widgets(mcp)
    
    log("")
    log("=" * 60)
    log("BUILD COMPLETE!")
    log("=" * 60)
    log("")
    log("Summary:")
    log(f"  - Core Blueprints: {len(CORE_BLUEPRINTS)}")
    log(f"  - Vehicle Blueprints: {len(VEHICLE_BLUEPRINTS)}")
    log(f"  - Racing Blueprints: {len(RACING_BLUEPRINTS)}")
    log(f"  - AI Blueprints: {len(AI_BLUEPRINTS)}")
    log(f"  - Environment Blueprints: {len(ENVIRONMENT_BLUEPRINTS)}")
    log(f"  - Garage Blueprints: {len(GARAGE_BLUEPRINTS)}")
    log(f"  - UI Widgets: {len(UI_WIDGETS)}")
    log(f"  - TOTAL: {len(CORE_BLUEPRINTS) + len(VEHICLE_BLUEPRINTS) + len(RACING_BLUEPRINTS) + len(AI_BLUEPRINTS) + len(ENVIRONMENT_BLUEPRINTS) + len(GARAGE_BLUEPRINTS) + len(UI_WIDGETS)}")
    log("")
    log("Next steps:")
    log("  1. Open Unreal Editor Content Browser")
    log("  2. Navigate to /Game/MidnightGrind/")
    log("  3. Review and customize the generated Blueprints")
    log("  4. Set up project settings to use BP_MGGameMode, BP_MGGameInstance")
    
    return True


if __name__ == "__main__":
    build_all()
