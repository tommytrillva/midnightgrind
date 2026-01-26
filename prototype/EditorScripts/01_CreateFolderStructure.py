# Midnight Grind - Unreal Editor Setup Scripts
# Run these in Unreal Editor's Python console or via MCP
# Editor → Developer Tools → Output Log → Cmd dropdown → Python

import unreal

# ============================================
# SCRIPT 1: Create Folder Structure
# ============================================
def create_folder_structure():
    """Creates the Content folder structure for Midnight Grind"""

    folders = [
        "/Game/MidnightGrind",
        "/Game/MidnightGrind/Blueprints",
        "/Game/MidnightGrind/Blueprints/Core",
        "/Game/MidnightGrind/Blueprints/Vehicles",
        "/Game/MidnightGrind/Blueprints/UI",
        "/Game/MidnightGrind/Blueprints/UI/Widgets",
        "/Game/MidnightGrind/Blueprints/UI/Menus",
        "/Game/MidnightGrind/Blueprints/Racing",
        "/Game/MidnightGrind/Blueprints/Characters",
        "/Game/MidnightGrind/Blueprints/AI",
        "/Game/MidnightGrind/Maps",
        "/Game/MidnightGrind/Maps/Tracks",
        "/Game/MidnightGrind/Maps/Menus",
        "/Game/MidnightGrind/Materials",
        "/Game/MidnightGrind/Materials/Vehicles",
        "/Game/MidnightGrind/Materials/Environment",
        "/Game/MidnightGrind/Materials/UI",
        "/Game/MidnightGrind/Meshes",
        "/Game/MidnightGrind/Meshes/Vehicles",
        "/Game/MidnightGrind/Meshes/Environment",
        "/Game/MidnightGrind/Meshes/Props",
        "/Game/MidnightGrind/Textures",
        "/Game/MidnightGrind/Audio",
        "/Game/MidnightGrind/Audio/Music",
        "/Game/MidnightGrind/Audio/SFX",
        "/Game/MidnightGrind/Audio/Vehicles",
        "/Game/MidnightGrind/VFX",
        "/Game/MidnightGrind/VFX/Particles",
        "/Game/MidnightGrind/VFX/Niagara",
        "/Game/MidnightGrind/Data",
        "/Game/MidnightGrind/Data/Vehicles",
        "/Game/MidnightGrind/Data/Parts",
        "/Game/MidnightGrind/Input",
    ]

    editor_asset_lib = unreal.EditorAssetLibrary()

    for folder in folders:
        if not editor_asset_lib.does_directory_exist(folder):
            editor_asset_lib.make_directory(folder)
            unreal.log(f"Created: {folder}")
        else:
            unreal.log(f"Exists: {folder}")

    unreal.log("Folder structure created successfully!")

# Run it
create_folder_structure()
