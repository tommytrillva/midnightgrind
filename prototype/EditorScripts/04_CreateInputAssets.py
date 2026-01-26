# Midnight Grind - Create Input Actions and Mapping Context
# Run in Unreal Editor Python console or via MCP

import unreal

# ============================================
# SCRIPT 4: Create Enhanced Input Assets
# ============================================

def create_input_action(name, value_type, folder):
    """Create an Input Action asset

    value_type can be:
    - "Digital" (bool)
    - "Axis1D" (float)
    - "Axis2D" (Vector2D)
    - "Axis3D" (Vector)
    """

    factory = unreal.InputActionFactory()

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    action = asset_tools.create_asset(
        asset_name=name,
        package_path=folder,
        asset_class=None,
        factory=factory
    )

    if action:
        # Set the value type
        if value_type == "Digital":
            action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
        elif value_type == "Axis1D":
            action.set_editor_property("value_type", unreal.InputActionValueType.AXIS1D)
        elif value_type == "Axis2D":
            action.set_editor_property("value_type", unreal.InputActionValueType.AXIS2D)
        elif value_type == "Axis3D":
            action.set_editor_property("value_type", unreal.InputActionValueType.AXIS3D)

        unreal.log(f"Created Input Action: {name} ({value_type})")
        return action
    else:
        unreal.log_warning(f"Failed to create: {name}")
        return None


def create_input_mapping_context(name, folder):
    """Create an Input Mapping Context asset"""

    factory = unreal.InputMappingContextFactory()

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    context = asset_tools.create_asset(
        asset_name=name,
        package_path=folder,
        asset_class=None,
        factory=factory
    )

    if context:
        unreal.log(f"Created Input Mapping Context: {name}")
        return context
    else:
        unreal.log_warning(f"Failed to create: {name}")
        return None


def create_all_input_assets():
    """Create all input actions and mapping contexts"""

    input_folder = "/Game/MidnightGrind/Input"

    # ============================================
    # VEHICLE INPUT ACTIONS
    # ============================================

    # Driving
    create_input_action("IA_Throttle", "Axis1D", input_folder)
    create_input_action("IA_Brake", "Axis1D", input_folder)
    create_input_action("IA_Steering", "Axis1D", input_folder)
    create_input_action("IA_Handbrake", "Digital", input_folder)

    # Nitrous
    create_input_action("IA_Nitrous", "Digital", input_folder)

    # Transmission
    create_input_action("IA_ShiftUp", "Digital", input_folder)
    create_input_action("IA_ShiftDown", "Digital", input_folder)
    create_input_action("IA_ClutchManual", "Axis1D", input_folder)

    # Camera/Look
    create_input_action("IA_LookAround", "Axis2D", input_folder)
    create_input_action("IA_CameraSwitch", "Digital", input_folder)
    create_input_action("IA_LookBack", "Digital", input_folder)

    # Vehicle Actions
    create_input_action("IA_ResetVehicle", "Digital", input_folder)
    create_input_action("IA_Horn", "Digital", input_folder)
    create_input_action("IA_Headlights", "Digital", input_folder)

    # ============================================
    # MENU INPUT ACTIONS
    # ============================================

    create_input_action("IA_Pause", "Digital", input_folder)
    create_input_action("IA_Confirm", "Digital", input_folder)
    create_input_action("IA_Cancel", "Digital", input_folder)
    create_input_action("IA_MenuNavigate", "Axis2D", input_folder)

    # ============================================
    # DEBUG INPUT ACTIONS
    # ============================================

    create_input_action("IA_Debug_SpawnAI", "Digital", input_folder)
    create_input_action("IA_Debug_ToggleWeather", "Digital", input_folder)
    create_input_action("IA_Debug_GodMode", "Digital", input_folder)

    # ============================================
    # INPUT MAPPING CONTEXTS
    # ============================================

    # Vehicle controls
    create_input_mapping_context("IMC_Vehicle", input_folder)

    # Menu navigation
    create_input_mapping_context("IMC_Menu", input_folder)

    # On-foot (if needed)
    create_input_mapping_context("IMC_OnFoot", input_folder)

    # Debug/Development
    create_input_mapping_context("IMC_Debug", input_folder)

    unreal.log("========================================")
    unreal.log("All Input Assets created!")
    unreal.log("========================================")
    unreal.log("")
    unreal.log("NEXT STEPS:")
    unreal.log("1. Open each IMC (Input Mapping Context)")
    unreal.log("2. Add mappings for keyboard/gamepad")
    unreal.log("3. Link Input Actions to keys")
    unreal.log("========================================")


# Execute
create_all_input_assets()
