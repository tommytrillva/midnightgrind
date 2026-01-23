# Midnight Grind - Create Core Blueprint Classes
# Run in Unreal Editor Python console or via MCP

import unreal

# ============================================
# SCRIPT 2: Create Core Blueprint Classes
# ============================================

def create_blueprint(name, parent_class, folder):
    """Create a Blueprint class with specified parent"""

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    blueprint = asset_tools.create_asset(
        asset_name=name,
        package_path=folder,
        asset_class=unreal.Blueprint,
        factory=factory
    )

    if blueprint:
        unreal.log(f"Created Blueprint: {folder}/{name}")
        return blueprint
    else:
        unreal.log_warning(f"Failed to create: {name}")
        return None


def create_core_blueprints():
    """Create all core game Blueprints"""

    # ============================================
    # GAME MODE
    # ============================================

    # Main Game Mode
    game_mode_class = unreal.load_class(None, "/Script/Engine.GameModeBase")
    create_blueprint(
        "BP_MidnightGrindGameMode",
        game_mode_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # Main Menu Game Mode
    create_blueprint(
        "BP_MainMenuGameMode",
        game_mode_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # Racing Game Mode
    create_blueprint(
        "BP_RacingGameMode",
        game_mode_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # ============================================
    # PLAYER CONTROLLER
    # ============================================

    player_controller_class = unreal.load_class(None, "/Script/Engine.PlayerController")
    create_blueprint(
        "BP_MidnightGrindPlayerController",
        player_controller_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # ============================================
    # GAME INSTANCE
    # ============================================

    game_instance_class = unreal.load_class(None, "/Script/Engine.GameInstance")
    create_blueprint(
        "BP_MidnightGrindGameInstance",
        game_instance_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # ============================================
    # HUD
    # ============================================

    hud_class = unreal.load_class(None, "/Script/Engine.HUD")
    create_blueprint(
        "BP_MidnightGrindHUD",
        hud_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # ============================================
    # PLAYER STATE
    # ============================================

    player_state_class = unreal.load_class(None, "/Script/Engine.PlayerState")
    create_blueprint(
        "BP_MidnightGrindPlayerState",
        player_state_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # ============================================
    # GAME STATE
    # ============================================

    game_state_class = unreal.load_class(None, "/Script/Engine.GameStateBase")
    create_blueprint(
        "BP_MidnightGrindGameState",
        game_state_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    unreal.log("Core Blueprints created successfully!")


def create_vehicle_blueprints():
    """Create vehicle-related Blueprints"""

    # Try to load our custom vehicle class first, fall back to engine class
    try:
        vehicle_class = unreal.load_class(None, "/Script/MidnightGrind.MGVehiclePawn")
    except:
        vehicle_class = unreal.load_class(None, "/Script/ChaosVehicles.WheeledVehiclePawn")
        unreal.log_warning("Using WheeledVehiclePawn - MGVehiclePawn not found")

    # Base Vehicle
    create_blueprint(
        "BP_BaseVehicle",
        vehicle_class,
        "/Game/MidnightGrind/Blueprints/Vehicles"
    )

    # Specific vehicle types (children of base)
    # These would inherit from BP_BaseVehicle once it exists

    actor_class = unreal.load_class(None, "/Script/Engine.Actor")

    # Vehicle-related actors
    create_blueprint(
        "BP_VehicleSpawnPoint",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Vehicles"
    )

    create_blueprint(
        "BP_GarageVehicleDisplay",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Vehicles"
    )

    unreal.log("Vehicle Blueprints created successfully!")


def create_racing_blueprints():
    """Create racing system Blueprints"""

    actor_class = unreal.load_class(None, "/Script/Engine.Actor")

    # Race Manager
    create_blueprint(
        "BP_RaceManager",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Racing"
    )

    # Checkpoint
    create_blueprint(
        "BP_Checkpoint",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Racing"
    )

    # Start/Finish Line
    create_blueprint(
        "BP_StartFinishLine",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Racing"
    )

    # Race Route (spline-based)
    create_blueprint(
        "BP_RaceRoute",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Racing"
    )

    # AI Opponent
    try:
        ai_controller_class = unreal.load_class(None, "/Script/AIModule.AIController")
    except:
        ai_controller_class = unreal.load_class(None, "/Script/Engine.AIController")

    create_blueprint(
        "BP_RaceAIController",
        ai_controller_class,
        "/Game/MidnightGrind/Blueprints/AI"
    )

    unreal.log("Racing Blueprints created successfully!")


def create_environment_blueprints():
    """Create environment/world Blueprints"""

    actor_class = unreal.load_class(None, "/Script/Engine.Actor")

    # Weather Controller
    create_blueprint(
        "BP_WeatherController",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # Day/Night Controller
    create_blueprint(
        "BP_TimeOfDayController",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Core"
    )

    # Street Light
    create_blueprint(
        "BP_StreetLight",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Environment"
    )

    # Neon Sign
    create_blueprint(
        "BP_NeonSign",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Environment"
    )

    # Traffic Light
    create_blueprint(
        "BP_TrafficLight",
        actor_class,
        "/Game/MidnightGrind/Blueprints/Environment"
    )

    unreal.log("Environment Blueprints created successfully!")


# ============================================
# RUN ALL
# ============================================
def create_all_blueprints():
    """Run all Blueprint creation functions"""
    create_core_blueprints()
    create_vehicle_blueprints()
    create_racing_blueprints()
    create_environment_blueprints()
    unreal.log("========================================")
    unreal.log("All Blueprints created successfully!")
    unreal.log("========================================")

# Execute
create_all_blueprints()
