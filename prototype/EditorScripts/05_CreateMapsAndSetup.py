# Midnight Grind - Create Maps and Configure Project
# Run in Unreal Editor Python console or via MCP

import unreal

# ============================================
# SCRIPT 5: Create Maps and Project Setup
# ============================================

def create_empty_map(name, folder):
    """Create an empty level/map"""

    # Use EditorLevelLibrary to create new level
    editor_level_lib = unreal.EditorLevelLibrary()

    # Create new level
    success = unreal.EditorLevelLibrary.new_level(folder + "/" + name)

    if success:
        unreal.log(f"Created map: {folder}/{name}")
        return True
    else:
        unreal.log_warning(f"Failed to create map: {name}")
        return False


def create_game_maps():
    """Create all game maps/levels"""

    maps_folder = "/Game/MidnightGrind/Maps"
    tracks_folder = "/Game/MidnightGrind/Maps/Tracks"
    menus_folder = "/Game/MidnightGrind/Maps/Menus"

    # ============================================
    # MENU LEVELS
    # ============================================

    # Main Menu
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Menus/L_MainMenu",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_MainMenu")

    # Garage
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Menus/L_Garage",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_Garage")

    # ============================================
    # TRACK LEVELS
    # ============================================

    # Test Track (development)
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Tracks/L_TestTrack",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_TestTrack")

    # Downtown (main hub area)
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Tracks/L_Downtown",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_Downtown")

    # Highway (high-speed races)
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Tracks/L_Highway",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_Highway")

    # Canyon (touge/mountain passes)
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Tracks/L_Canyon",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_Canyon")

    # Industrial (drift area)
    unreal.EditorLevelLibrary.new_level_from_template(
        "/Game/MidnightGrind/Maps/Tracks/L_Industrial",
        "/Engine/Maps/Templates/Template_Default"
    )
    unreal.log("Created: L_Industrial")

    unreal.log("All maps created!")


def setup_test_track():
    """Set up basic test track level with required actors"""

    # Load the test track level
    level_path = "/Game/MidnightGrind/Maps/Tracks/L_TestTrack"

    # Check if level exists
    if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
        unreal.log_warning("L_TestTrack doesn't exist. Create maps first.")
        return

    # Load the level
    unreal.EditorLevelLibrary.load_level(level_path)

    # Get editor actor subsystem
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # ============================================
    # ADD ESSENTIAL ACTORS
    # ============================================

    # Add Directional Light (Sun/Moon)
    directional_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 1000),
        unreal.Rotator(-45, 0, 0)
    )
    if directional_light:
        directional_light.set_actor_label("MainLight")
        light_component = directional_light.get_component_by_class(unreal.DirectionalLightComponent)
        if light_component:
            light_component.set_editor_property("intensity", 3.0)
            light_component.set_editor_property("light_color", unreal.Color(200, 200, 255, 255))
        unreal.log("Added: Directional Light")

    # Add Sky Atmosphere
    sky = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0)
    )
    if sky:
        sky.set_actor_label("SkyAtmosphere")
        unreal.log("Added: Sky Atmosphere")

    # Add Exponential Height Fog
    fog = editor_actor_subsystem.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, 100)
    )
    if fog:
        fog.set_actor_label("HeightFog")
        fog_component = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
        if fog_component:
            fog_component.set_editor_property("fog_density", 0.02)
            fog_component.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.1, 0.1, 0.2, 1.0))
        unreal.log("Added: Exponential Height Fog")

    # Add Sky Light
    sky_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 500)
    )
    if sky_light:
        sky_light.set_actor_label("SkyLight")
        sky_light_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sky_light_component:
            sky_light_component.set_editor_property("intensity", 1.0)
        unreal.log("Added: Sky Light")

    # Add Post Process Volume (unbound)
    pp_volume = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0)
    )
    if pp_volume:
        pp_volume.set_actor_label("GlobalPostProcess")
        pp_volume.set_editor_property("unbound", True)
        unreal.log("Added: Post Process Volume")

    # Add Player Start
    player_start = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(0, 0, 100),
        unreal.Rotator(0, 0, 0)
    )
    if player_start:
        player_start.set_actor_label("PlayerStart_Race")
        unreal.log("Added: Player Start")

    # Add ground plane (temporary)
    # Note: In production, use proper landscape or static mesh

    unreal.log("========================================")
    unreal.log("Test Track basic setup complete!")
    unreal.log("========================================")
    unreal.log("")
    unreal.log("NEXT STEPS:")
    unreal.log("1. Add BP_RaceManager to level")
    unreal.log("2. Create road/track geometry")
    unreal.log("3. Add checkpoints and finish line")
    unreal.log("4. Set World Settings game mode")
    unreal.log("========================================")

    # Save the level
    unreal.EditorLevelLibrary.save_current_level()


def configure_project_settings():
    """Configure essential project settings via Python

    Note: Some settings must still be done manually in Project Settings
    """

    # Get project settings
    # Note: Not all settings are accessible via Python

    unreal.log("========================================")
    unreal.log("PROJECT SETTINGS - Manual Steps Required:")
    unreal.log("========================================")
    unreal.log("")
    unreal.log("Edit → Project Settings:")
    unreal.log("")
    unreal.log("1. Maps & Modes:")
    unreal.log("   - Editor Startup Map: L_MainMenu")
    unreal.log("   - Game Default Map: L_MainMenu")
    unreal.log("   - Default GameMode: BP_MidnightGrindGameMode")
    unreal.log("   - Game Instance Class: BP_MidnightGrindGameInstance")
    unreal.log("")
    unreal.log("2. Engine → Collision:")
    unreal.log("   - New Object Channel: Vehicle")
    unreal.log("   - New Object Channel: VehicleWheel")
    unreal.log("   - Configure collision responses")
    unreal.log("")
    unreal.log("3. Engine → Input:")
    unreal.log("   - Default Input Component: EnhancedInputComponent")
    unreal.log("")
    unreal.log("4. Plugins → Enable:")
    unreal.log("   - Chaos Vehicles")
    unreal.log("   - Enhanced Input")
    unreal.log("   - Gameplay Abilities")
    unreal.log("   - Niagara")
    unreal.log("========================================")


# ============================================
# RUN ALL
# ============================================

def full_setup():
    """Run complete project setup"""
    unreal.log("Starting Midnight Grind project setup...")

    # Create maps
    create_game_maps()

    # Setup test track
    setup_test_track()

    # Show manual config steps
    configure_project_settings()

    unreal.log("")
    unreal.log("========================================")
    unreal.log("SETUP COMPLETE!")
    unreal.log("========================================")


# Execute
full_setup()
