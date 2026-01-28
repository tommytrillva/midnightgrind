// Copyright Midnight Grind. All Rights Reserved.

/*******************************************************************************
 * MGAccessibilitySubsystem.h - Accessibility Subsystem for Midnight Grind
 *******************************************************************************
 *
 * WHAT THIS FILE DOES:
 * ====================
 * This file defines the Accessibility Subsystem - a central manager that handles
 * ALL accessibility features in the game. Accessibility features are options that
 * help players with disabilities (visual, hearing, motor, cognitive) enjoy the game,
 * but they're also useful for players who simply prefer alternative ways to play.
 *
 * Think of this subsystem as a "control center" for player accommodations. When a
 * player opens the accessibility menu and toggles an option (like enabling subtitles
 * or color-blind mode), this subsystem processes that change, saves it, and notifies
 * other parts of the game to update accordingly.
 *
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * =================================
 *
 * 1. GAME INSTANCE SUBSYSTEM
 *    - This class inherits from UGameInstanceSubsystem, which means:
 *      * There's exactly ONE instance that exists for the entire game session
 *      * It persists across level loads (unlike actors that get destroyed)
 *      * It's automatically created when the game starts and destroyed when it ends
 *      * You access it via: GetGameInstance()->GetSubsystem<UMGAccessibilitySubsystem>()
 *
 * 2. UPROPERTY() AND UFUNCTION() MACROS
 *    - These Unreal macros expose C++ code to the reflection system
 *    - UPROPERTY() makes variables visible to Blueprints and serialization
 *    - UFUNCTION(BlueprintCallable) lets Blueprints call the function
 *    - UFUNCTION(BlueprintPure) means it doesn't modify state (safe for getters)
 *
 * 3. DELEGATES (EVENT SYSTEM)
 *    - Delegates like FOnAccessibilitySettingsChanged are Unreal's event system
 *    - Other systems can "subscribe" to these events to be notified of changes
 *    - Example: The HUD subscribes to OnAccessibilitySettingsChanged to know
 *      when to update its text size or colors
 *
 * 4. USTRUCT FOR DATA CONTAINERS
 *    - FMGAccessibilitySettings is a USTRUCT - a data container
 *    - It groups related settings together (visual, audio, controls, etc.)
 *    - This entire struct can be saved/loaded as one unit
 *
 * 5. ENUMS FOR FIXED OPTIONS
 *    - Enums like EMGColorBlindMode define a fixed set of choices
 *    - They're type-safe (can't accidentally pass invalid values)
 *    - BlueprintType makes them usable in Blueprint visual scripting
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ========================================
 *
 *                    +-----------------------+
 *                    |    Game Instance      |
 *                    +-----------+-----------+
 *                                |
 *          +---------------------+---------------------+
 *          |                     |                     |
 *   +------v------+     +--------v--------+    +------v------+
 *   | Accessibility|    | Localization    |    | Settings    |
 *   | Subsystem   |    | Subsystem       |    | Subsystem   |
 *   +------+------+    +-----------------+    +-------------+
 *          |
 *          | Broadcasts events to:
 *          |
 *   +------v------------------------------------------+
 *   |  - UI/HUD (updates text size, colors)          |
 *   |  - Post-Process Volume (applies color filters) |
 *   |  - Audio System (enables mono, subtitles)      |
 *   |  - Input System (applies input remapping)      |
 *   |  - Vehicle Controller (enables auto-steering)  |
 *   +------------------------------------------------+
 *
 * FLOW OF A SETTING CHANGE:
 * 1. Player toggles "Enable Subtitles" in the Accessibility Menu
 * 2. Menu UI calls AccessibilitySubsystem->SetSubtitles(true, Size)
 * 3. Subsystem updates CurrentSettings.bSubtitlesEnabled
 * 4. Subsystem calls SaveSettings() to persist to disk
 * 5. Subsystem broadcasts OnAccessibilitySettingsChanged
 * 6. Subtitle Widget receives event and shows/hides itself
 *
 *
 * COMMON TASKS FOR NEW DEVELOPERS:
 * =================================
 *
 * Adding a new accessibility option:
 *   1. Add the variable to FMGAccessibilitySettings struct
 *   2. Create a setter function (SetMyNewOption)
 *   3. Call SaveSettings() and broadcast the event in the setter
 *   4. Add UI control in the accessibility menu Blueprint
 *
 * Reading a setting from elsewhere in code:
 *   if (auto* Accessibility = GetGameInstance()->GetSubsystem<UMGAccessibilitySubsystem>())
 *   {
 *       if (Accessibility->GetAccessibilitySettings().bReduceMotion)
 *       {
 *           // Skip the camera shake
 *       }
 *   }
 *
 * Reacting to setting changes:
 *   // In your class's BeginPlay or initialization:
 *   Accessibility->OnAccessibilitySettingsChanged.AddDynamic(this, &UMyClass::OnSettingsChanged);
 *
 *
 * ACCESSIBILITY CATEGORIES SUPPORTED:
 * ====================================
 * - Visual: Color blindness filters, text scaling, UI scaling, high contrast,
 *           motion reduction, screen shake disable, flash prevention
 * - Audio: Subtitles, mono audio, visual audio cues, speaker identification
 * - Motor: Auto-acceleration, auto-steering, one-handed mode, input remapping,
 *          dead zones, sensitivity adjustment
 * - Cognitive: Extended timers, simplified controls, gameplay assists
 * - Screen Reader: Text-to-speech for UI navigation
 *
 *
 * @see FMGAccessibilitySettings - The data structure holding all settings
 * @see UMGSettingsSubsystem - For general game settings (graphics, audio volumes)
 * @see UMGLocalizationSubsystem - For language/region settings (works with subtitles)
 *
 ******************************************************************************/

/**
 * @file MGAccessibilitySubsystem.h
 * @brief Accessibility Subsystem for Midnight Grind
 *
 * This subsystem manages all accessibility features to ensure the game is playable
 * by players with various disabilities or preferences. It provides comprehensive
 * support for:
 *
 * - **Visual Accessibility**: Color blindness filters, text scaling, UI scaling,
 *   high contrast modes, and motion reduction options.
 *
 * - **Audio Accessibility**: Subtitles with customizable size and background,
 *   speaker identification, mono audio output, and visual audio cues.
 *
 * - **Motor Accessibility**: Auto-acceleration, auto-steering, one-handed mode,
 *   customizable input dead zones, and full input remapping support.
 *
 * - **Cognitive Accessibility**: Extended timers, simplified controls, and
 *   various gameplay assists like collision and braking assistance.
 *
 * - **Screen Reader Support**: Text-to-speech for UI elements with configurable
 *   speed and verbosity settings.
 *
 * @note This subsystem persists settings to local storage and loads them
 *       automatically on game startup.
 *
 * @see FMGAccessibilitySettings for the complete settings structure
 * @see UMGSettingsSubsystem for general game settings
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAccessibilitySubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Color blindness simulation modes
 *
 * These modes apply post-processing filters to transform colors so that
 * color-blind players can distinguish game elements that rely on color coding.
 * The filters simulate how colors appear to players with different types of
 * color vision deficiency, then remap them to be distinguishable.
 */
UENUM(BlueprintType)
enum class EMGColorBlindMode : uint8
{
	/// No color correction applied - default vision
	None			UMETA(DisplayName = "None"),

	/// Green-blind: Difficulty distinguishing green from red (most common, ~6% of males)
	Deuteranopia	UMETA(DisplayName = "Deuteranopia (Green-Blind)"),

	/// Red-blind: Difficulty distinguishing red from green (~1% of males)
	Protanopia		UMETA(DisplayName = "Protanopia (Red-Blind)"),

	/// Blue-blind: Difficulty distinguishing blue from yellow (rare, ~0.01% of population)
	Tritanopia		UMETA(DisplayName = "Tritanopia (Blue-Blind)"),

	/// Total color blindness: Only sees shades of gray (very rare)
	Achromatopsia	UMETA(DisplayName = "Achromatopsia (Total Color Blindness)")
};

/**
 * @brief Text size presets for UI and subtitles
 *
 * These presets scale text throughout the game's UI. The actual pixel sizes
 * are determined by the base font size multiplied by a scale factor:
 * - Small: 0.8x
 * - Medium: 1.0x (default)
 * - Large: 1.25x
 * - ExtraLarge: 1.5x
 */
UENUM(BlueprintType)
enum class EMGTextSize : uint8
{
	/// Compact text for players who prefer more content on screen
	Small		UMETA(DisplayName = "Small"),

	/// Default text size, balanced for readability
	Medium		UMETA(DisplayName = "Medium"),

	/// Larger text for improved readability
	Large		UMETA(DisplayName = "Large"),

	/// Maximum text size for players with vision impairments
	ExtraLarge	UMETA(DisplayName = "Extra Large")
};

//=============================================================================
// Data Structures
//=============================================================================

/**
 * @brief Complete accessibility settings configuration
 *
 * This structure contains all accessibility-related settings for a player.
 * It is saved to local storage and loaded when the game starts. Settings
 * are organized into logical groups:
 *
 * - Visual: Display and UI adjustments
 * - Audio: Sound and subtitle options
 * - Controls: Input and motor accessibility
 * - Gameplay: Difficulty and assistance options
 * - Screen Reader: Text-to-speech configuration
 *
 * @note All boolean settings default to the least assistive option to ensure
 *       experienced players don't encounter unexpected behavior.
 */
USTRUCT(BlueprintType)
struct FMGAccessibilitySettings
{
	GENERATED_BODY()

	//-------------------------------------------------------------------------
	// Visual Settings
	//-------------------------------------------------------------------------

	/// Color blindness filter mode (default: None)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGColorBlindMode ColorBlindMode = EMGColorBlindMode::None;

	/// Strength of the color blind filter [0.0-1.0], allows partial correction
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ColorBlindIntensity = 1.0f;

	/// Global text size for UI elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGTextSize TextSize = EMGTextSize::Medium;

	/// UI scale multiplier [0.75-1.5], affects all UI elements proportionally
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float UIScale = 1.0f;

	/// When true, uses high contrast colors for better UI visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHighContrastUI = false;

	/// Screen brightness adjustment [0.5-2.0]
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Brightness = 1.0f;

	/// Screen contrast adjustment [0.5-2.0]
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Contrast = 1.0f;

	/// Reduces or eliminates non-essential animations and camera movements
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bReduceMotion = false;

	/// Disables camera shake effects during impacts and nitro boosts
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDisableScreenShake = false;

	/// Prevents rapid flashing effects that may trigger photosensitive conditions
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDisableFlashing = false;

	//-------------------------------------------------------------------------
	// Audio Settings
	//-------------------------------------------------------------------------

	/// Master toggle for subtitle display during dialogue and cinematics
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSubtitlesEnabled = true;

	/// Text size specifically for subtitles (independent of UI text size)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EMGTextSize SubtitleSize = EMGTextSize::Medium;

	/// Adds a semi-transparent background behind subtitles for readability
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSubtitleBackground = true;

	/// Shows character names before dialogue (e.g., "Marcus: Let's race!")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSpeakerNames = true;

	/// Converts stereo audio to mono for players with hearing in one ear
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMonoAudio = false;

	/// Shows visual indicators for important sounds (footsteps, engines, horns)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bVisualizeAudio = false;

	//-------------------------------------------------------------------------
	// Control Settings
	//-------------------------------------------------------------------------

	/// Vehicle accelerates automatically without player input
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoAccelerate = false;

	/// Vehicle follows optimal racing line with minimal player steering
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoSteering = false;

	/// When true, must hold accelerate button; when false, tap to toggle
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHoldToAccelerate = true;

	/// Steering input sensitivity multiplier [0.25-2.0]
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SteeringSensitivity = 1.0f;

	/// Inverts vertical camera/look axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bInvertYAxis = false;

	/// Dead zone for analog stick inputs [0.0-0.5] to prevent drift
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float InputDeadzone = 0.15f;

	/// Remaps all controls to be operable with one hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bOneHandedMode = false;

	//-------------------------------------------------------------------------
	// Gameplay Assist Settings
	//-------------------------------------------------------------------------

	/// Increases time limits for timed challenges and events
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bExtendedTimers = false;

	/// Multiplier for timer extensions when bExtendedTimers is true [1.0-3.0]
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimerMultiplier = 1.5f;

	/// Reduces the number of required inputs for complex actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSimplifiedControls = false;

	/// Automatically activates nitro boost when available
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutoNitro = false;

	/// Reduces collision severity and helps avoid obstacles
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCollisionAssist = false;

	/// Assists with braking before sharp turns
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bBrakingAssist = false;

	//-------------------------------------------------------------------------
	// Screen Reader Settings
	//-------------------------------------------------------------------------

	/// Master toggle for screen reader functionality
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bScreenReaderEnabled = false;

	/// Speech rate for screen reader [0.5-2.0]
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ScreenReaderSpeed = 1.0f;

	/// Announces UI element types (button, checkbox, slider, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bDescribeUIElements = true;

	/// Reads aloud notification popups and alerts
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAnnounceNotifications = true;
};

/**
 * @brief Input remapping entry for custom key bindings
 *
 * Stores the mapping between a game action and its assigned input keys.
 * Players can customize primary (keyboard), secondary (keyboard), and
 * gamepad bindings independently.
 *
 * @note Empty FKey values indicate unbound inputs
 */
USTRUCT(BlueprintType)
struct FMGInputRemapping
{
	GENERATED_BODY()

	/// The internal action name (e.g., "Accelerate", "Brake", "Nitro")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActionName;

	/// Primary keyboard/mouse binding
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey PrimaryKey;

	/// Alternative keyboard/mouse binding
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey SecondaryKey;

	/// Gamepad/controller binding
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FKey GamepadKey;
};

//=============================================================================
// Delegate Declarations
//=============================================================================

/// Broadcast when any accessibility setting changes; provides the complete new settings
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessibilitySettingsChanged, const FMGAccessibilitySettings&, NewSettings);

/// Broadcast when the screen reader should speak text aloud
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenReaderSpeak, const FString&, Text);

//=============================================================================
// Accessibility Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing all accessibility features
 *
 * This subsystem provides a centralized API for reading and modifying
 * accessibility settings. It automatically persists settings to local
 * storage and broadcasts events when settings change.
 *
 * ## Usage Example (Blueprint)
 * @code
 * // Get the subsystem
 * UMGAccessibilitySubsystem* Accessibility = GetGameInstance()->GetSubsystem<UMGAccessibilitySubsystem>();
 *
 * // Enable color blind mode
 * Accessibility->SetColorBlindMode(EMGColorBlindMode::Deuteranopia, 1.0f);
 *
 * // Enable subtitles with large text
 * Accessibility->SetSubtitles(true, EMGTextSize::Large);
 * @endcode
 *
 * ## Usage Example (C++)
 * @code
 * if (UMGAccessibilitySubsystem* Accessibility = GetGameInstance()->GetSubsystem<UMGAccessibilitySubsystem>())
 * {
 *     Accessibility->SetReduceMotion(true);
 *     Accessibility->SetOneHandedMode(true);
 * }
 * @endcode
 *
 * @note Settings are automatically saved when changed and loaded on Initialize()
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAccessibilitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Lifecycle
	//-------------------------------------------------------------------------

	/// Called when the game instance creates this subsystem; loads saved settings
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when the game instance is shutting down; saves current settings
	virtual void Deinitialize() override;

	//-------------------------------------------------------------------------
	// Events
	//-------------------------------------------------------------------------

	/// Broadcast whenever accessibility settings are modified
	UPROPERTY(BlueprintAssignable) FOnAccessibilitySettingsChanged OnAccessibilitySettingsChanged;

	/// Broadcast when text should be spoken by the screen reader
	UPROPERTY(BlueprintAssignable) FOnScreenReaderSpeak OnScreenReaderSpeak;

	//-------------------------------------------------------------------------
	// General Settings Management
	//-------------------------------------------------------------------------

	/**
	 * @brief Applies a complete accessibility settings configuration
	 * @param Settings The new settings to apply
	 * @note Triggers OnAccessibilitySettingsChanged and saves to storage
	 */
	UFUNCTION(BlueprintCallable) void SetAccessibilitySettings(const FMGAccessibilitySettings& Settings);

	/**
	 * @brief Returns the current accessibility settings
	 * @return Copy of the current settings structure
	 */
	UFUNCTION(BlueprintPure) FMGAccessibilitySettings GetAccessibilitySettings() const { return CurrentSettings; }

	/**
	 * @brief Resets all accessibility settings to their default values
	 * @note This cannot be undone - consider confirming with the player first
	 */
	UFUNCTION(BlueprintCallable) void ResetToDefaults();

	/**
	 * @brief Applies a named accessibility preset
	 * @param PresetName Name of the preset (e.g., "LowVision", "Motor", "Cognitive")
	 * @see GetAvailablePresets() for valid preset names
	 */
	UFUNCTION(BlueprintCallable) void ApplyPreset(const FString& PresetName);

	//-------------------------------------------------------------------------
	// Visual Accessibility
	//-------------------------------------------------------------------------

	/**
	 * @brief Configures color blindness correction
	 * @param Mode The type of color blindness to correct for
	 * @param Intensity Strength of the filter [0.0-1.0], default 1.0
	 */
	UFUNCTION(BlueprintCallable) void SetColorBlindMode(EMGColorBlindMode Mode, float Intensity = 1.0f);

	/**
	 * @brief Sets the global UI text size
	 * @param Size The text size preset to apply
	 */
	UFUNCTION(BlueprintCallable) void SetTextSize(EMGTextSize Size);

	/**
	 * @brief Sets the global UI scale multiplier
	 * @param Scale Scale factor [0.75-1.5] where 1.0 is default
	 */
	UFUNCTION(BlueprintCallable) void SetUIScale(float Scale);

	/**
	 * @brief Enables or disables high contrast UI mode
	 * @param bEnabled True to enable high contrast colors
	 */
	UFUNCTION(BlueprintCallable) void SetHighContrast(bool bEnabled);

	/**
	 * @brief Enables or disables motion reduction
	 * @param bEnabled True to reduce camera movements and animations
	 */
	UFUNCTION(BlueprintCallable) void SetReduceMotion(bool bEnabled);

	/**
	 * @brief Transforms a color for the current color blindness mode
	 * @param OriginalColor The color to transform
	 * @return The transformed color appropriate for the current mode
	 * @note Returns the original color if no color blind mode is active
	 */
	UFUNCTION(BlueprintPure) FLinearColor GetAccessibleColor(FLinearColor OriginalColor) const;

	//-------------------------------------------------------------------------
	// Audio Accessibility
	//-------------------------------------------------------------------------

	/**
	 * @brief Configures subtitle display
	 * @param bEnabled True to show subtitles
	 * @param Size Text size for subtitles (default: Medium)
	 */
	UFUNCTION(BlueprintCallable) void SetSubtitles(bool bEnabled, EMGTextSize Size = EMGTextSize::Medium);

	/**
	 * @brief Enables or disables mono audio output
	 * @param bEnabled True to convert stereo to mono
	 * @note Useful for players with hearing loss in one ear
	 */
	UFUNCTION(BlueprintCallable) void SetMonoAudio(bool bEnabled);

	/**
	 * @brief Enables or disables visual audio indicators
	 * @param bEnabled True to show visual cues for important sounds
	 */
	UFUNCTION(BlueprintCallable) void SetVisualizeAudio(bool bEnabled);

	//-------------------------------------------------------------------------
	// Control Accessibility
	//-------------------------------------------------------------------------

	/**
	 * @brief Enables or disables automatic acceleration
	 * @param bEnabled True for automatic forward movement
	 */
	UFUNCTION(BlueprintCallable) void SetAutoAccelerate(bool bEnabled);

	/**
	 * @brief Enables or disables steering assistance
	 * @param bEnabled True for AI-assisted steering along optimal lines
	 */
	UFUNCTION(BlueprintCallable) void SetAutoSteering(bool bEnabled);

	/**
	 * @brief Enables or disables one-handed control mode
	 * @param bEnabled True to remap controls for single-hand operation
	 */
	UFUNCTION(BlueprintCallable) void SetOneHandedMode(bool bEnabled);

	/**
	 * @brief Remaps a game action to a new input key
	 * @param ActionName The action to remap (e.g., "Accelerate")
	 * @param NewKey The new key to assign
	 * @param bIsGamepad True if remapping a gamepad button, false for keyboard
	 */
	UFUNCTION(BlueprintCallable) void RemapInput(FName ActionName, FKey NewKey, bool bIsGamepad = false);

	/**
	 * @brief Returns all current input remappings
	 * @return Array of input remapping configurations
	 */
	UFUNCTION(BlueprintPure) TArray<FMGInputRemapping> GetInputRemappings() const { return InputRemappings; }

	/**
	 * @brief Resets all input bindings to defaults
	 */
	UFUNCTION(BlueprintCallable) void ResetInputRemappings();

	//-------------------------------------------------------------------------
	// Screen Reader
	//-------------------------------------------------------------------------

	/**
	 * @brief Queues text to be spoken by the screen reader
	 * @param Text The text to speak
	 * @param bInterrupt If true, stops current speech and speaks immediately
	 */
	UFUNCTION(BlueprintCallable) void Speak(const FString& Text, bool bInterrupt = false);

	/**
	 * @brief Announces a UI element for screen reader users
	 * @param ElementType Type of element (e.g., "Button", "Slider", "Checkbox")
	 * @param ElementName Display name of the element
	 * @param Description Additional context or current value
	 */
	UFUNCTION(BlueprintCallable) void SpeakUIElement(const FString& ElementType, const FString& ElementName, const FString& Description);

	/**
	 * @brief Immediately stops any ongoing screen reader speech
	 */
	UFUNCTION(BlueprintCallable) void StopSpeaking();

	/**
	 * @brief Checks if the screen reader is currently enabled
	 * @return True if screen reader is active
	 */
	UFUNCTION(BlueprintPure) bool IsScreenReaderActive() const { return CurrentSettings.bScreenReaderEnabled; }

	//-------------------------------------------------------------------------
	// Presets
	//-------------------------------------------------------------------------

	/**
	 * @brief Returns names of all available accessibility presets
	 * @return Array of preset names that can be passed to ApplyPreset()
	 */
	UFUNCTION(BlueprintPure) TArray<FString> GetAvailablePresets() const;

protected:
	//-------------------------------------------------------------------------
	// Internal State
	//-------------------------------------------------------------------------

	/// Current accessibility settings
	UPROPERTY() FMGAccessibilitySettings CurrentSettings;

	/// Custom input key bindings
	UPROPERTY() TArray<FMGInputRemapping> InputRemappings;

	/// Queue of text waiting to be spoken
	UPROPERTY() TArray<FString> SpeechQueue;

	/// True when the screen reader is actively speaking
	UPROPERTY() bool bIsSpeaking = false;

	//-------------------------------------------------------------------------
	// Internal Methods
	//-------------------------------------------------------------------------

	/// Applies the current color blind filter to the post-process chain
	void ApplyColorBlindFilter();

	/// Updates UI scaling based on current settings
	void ApplyUIScaling();

	/// Refreshes subtitle display settings
	void UpdateSubtitleSettings();

	/// Loads settings from local storage
	void LoadSettings();

	/// Saves current settings to local storage
	void SaveSettings();

	/**
	 * @brief Transforms a color based on color blindness settings
	 * @param Color The original color
	 * @param Mode The color blindness type to correct for
	 * @param Intensity The strength of the transformation
	 * @return The transformed color
	 */
	FLinearColor TransformColorForColorBlindness(FLinearColor Color, EMGColorBlindMode Mode, float Intensity) const;
};
