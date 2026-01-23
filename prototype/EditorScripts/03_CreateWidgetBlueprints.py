# Midnight Grind - Create Widget Blueprints
# Run in Unreal Editor Python console or via MCP

import unreal

# ============================================
# SCRIPT 3: Create Widget Blueprints (UMG)
# ============================================

def create_widget_blueprint(name, folder):
    """Create a Widget Blueprint"""

    factory = unreal.WidgetBlueprintFactory()

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    widget = asset_tools.create_asset(
        asset_name=name,
        package_path=folder,
        asset_class=unreal.WidgetBlueprint,
        factory=factory
    )

    if widget:
        unreal.log(f"Created Widget: {folder}/{name}")
        return widget
    else:
        unreal.log_warning(f"Failed to create widget: {name}")
        return None


def create_all_widgets():
    """Create all UI widgets for the game"""

    widgets_folder = "/Game/MidnightGrind/Blueprints/UI/Widgets"
    menus_folder = "/Game/MidnightGrind/Blueprints/UI/Menus"

    # ============================================
    # MENUS
    # ============================================

    # Main Menu
    create_widget_blueprint("WBP_MainMenu", menus_folder)

    # Pause Menu
    create_widget_blueprint("WBP_PauseMenu", menus_folder)

    # Options Menu
    create_widget_blueprint("WBP_OptionsMenu", menus_folder)

    # Loading Screen
    create_widget_blueprint("WBP_LoadingScreen", menus_folder)

    # ============================================
    # GARAGE UI
    # ============================================

    # Main Garage Screen
    create_widget_blueprint("WBP_GarageMain", menus_folder)

    # Vehicle List Entry
    create_widget_blueprint("WBP_VehicleListEntry", widgets_folder)

    # Vehicle Stats Display
    create_widget_blueprint("WBP_VehicleStats", widgets_folder)

    # Part Selection
    create_widget_blueprint("WBP_PartSelection", menus_folder)

    # Part List Entry
    create_widget_blueprint("WBP_PartListEntry", widgets_folder)

    # Paint/Color Picker
    create_widget_blueprint("WBP_PaintEditor", menus_folder)

    # Tuning Screen
    create_widget_blueprint("WBP_TuningScreen", menus_folder)

    # Dyno Results
    create_widget_blueprint("WBP_DynoResults", widgets_folder)

    # ============================================
    # RACING HUD
    # ============================================

    # Main Race HUD
    create_widget_blueprint("WBP_RaceHUD", widgets_folder)

    # Speedometer
    create_widget_blueprint("WBP_Speedometer", widgets_folder)

    # Tachometer
    create_widget_blueprint("WBP_Tachometer", widgets_folder)

    # Minimap
    create_widget_blueprint("WBP_Minimap", widgets_folder)

    # Lap Counter
    create_widget_blueprint("WBP_LapCounter", widgets_folder)

    # Position Display
    create_widget_blueprint("WBP_PositionDisplay", widgets_folder)

    # Race Timer
    create_widget_blueprint("WBP_RaceTimer", widgets_folder)

    # Nitrous Bar
    create_widget_blueprint("WBP_NitrousBar", widgets_folder)

    # Countdown Overlay
    create_widget_blueprint("WBP_Countdown", widgets_folder)

    # Wrong Way Warning
    create_widget_blueprint("WBP_WrongWayWarning", widgets_folder)

    # ============================================
    # RACE RESULTS
    # ============================================

    # Results Screen
    create_widget_blueprint("WBP_RaceResults", menus_folder)

    # Leaderboard Entry
    create_widget_blueprint("WBP_LeaderboardEntry", widgets_folder)

    # Rewards Display
    create_widget_blueprint("WBP_RewardsDisplay", widgets_folder)

    # ============================================
    # SOCIAL UI
    # ============================================

    # Player Card
    create_widget_blueprint("WBP_PlayerCard", widgets_folder)

    # Crew Panel
    create_widget_blueprint("WBP_CrewPanel", menus_folder)

    # Rival Callout
    create_widget_blueprint("WBP_RivalCallout", widgets_folder)

    # Chat/Messages
    create_widget_blueprint("WBP_ChatPanel", widgets_folder)

    # ============================================
    # ECONOMY UI
    # ============================================

    # Currency Display (top bar)
    create_widget_blueprint("WBP_CurrencyDisplay", widgets_folder)

    # Purchase Confirmation
    create_widget_blueprint("WBP_PurchaseConfirm", widgets_folder)

    # Market Listing
    create_widget_blueprint("WBP_MarketListing", widgets_folder)

    # ============================================
    # NOTIFICATIONS
    # ============================================

    # Toast Notification
    create_widget_blueprint("WBP_ToastNotification", widgets_folder)

    # Achievement Popup
    create_widget_blueprint("WBP_AchievementPopup", widgets_folder)

    # Level Up Display
    create_widget_blueprint("WBP_LevelUp", widgets_folder)

    # ============================================
    # WEATHER/ENVIRONMENT
    # ============================================

    # Weather Indicator
    create_widget_blueprint("WBP_WeatherIndicator", widgets_folder)

    # Time Display
    create_widget_blueprint("WBP_TimeDisplay", widgets_folder)

    unreal.log("========================================")
    unreal.log("All Widget Blueprints created!")
    unreal.log("========================================")


# Execute
create_all_widgets()
