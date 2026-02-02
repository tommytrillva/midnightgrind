// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/*******************************************************************************
 * MGLocalizationSubsystem.h - Localization and Internationalization Subsystem
 *******************************************************************************
 *
 * WHAT THIS FILE DOES:
 * ====================
 * This file defines the Localization Subsystem - a central manager responsible
 * for making the game playable in multiple languages and regions worldwide.
 * "Localization" (often abbreviated "L10N") is the process of adapting a game
 * for different languages and cultures. "Internationalization" (I18N) is the
 * underlying architecture that makes localization possible.
 *
 * This subsystem handles two main responsibilities:
 * 1. LANGUAGE: Translating text and audio into different languages
 * 2. REGIONAL FORMATTING: Displaying numbers, dates, units appropriately for
 *    different regions (e.g., "1,000 mph" in USA vs "1.609 km/h" in Europe)
 *
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * =================================
 *
 * 1. STRING TABLES AND STRING IDs
 *    - We NEVER hardcode player-visible text in C++ or Blueprints
 *    - Instead, each string has a unique ID like "UI.MainMenu.PlayButton"
 *    - The actual text for each language is stored in separate "string tables"
 *    - To display text: GetLocalizedString(FName("UI.MainMenu.PlayButton"))
 *    - This returns "Play" in English, "Jouer" in French, "Spielen" in German
 *
 * 2. FTEXT VS FSTRING
 *    - FText is Unreal's localization-aware string type - USE THIS for UI
 *    - FString is a raw string - use for internal logic, file paths, etc.
 *    - Never use FString for anything the player will see!
 *    - FText can be marked for translation, FString cannot
 *
 * 3. RIGHT-TO-LEFT (RTL) LANGUAGES
 *    - Some languages like Arabic and Hebrew read right-to-left
 *    - When RTL is active, the entire UI layout mirrors horizontally
 *    - Back buttons move to the right, text aligns differently, etc.
 *    - The bRightToLeftUI setting handles this automatically
 *
 * 4. GAME INSTANCE SUBSYSTEM PATTERN
 *    - Same as MGAccessibilitySubsystem - one instance for entire game
 *    - Persists across level loads
 *    - Access via: GetGameInstance()->GetSubsystem<UMGLocalizationSubsystem>()
 *
 * 5. DELEGATES FOR CHANGE NOTIFICATION
 *    - OnLanguageChanged fires when the player switches languages
 *    - All UI widgets should listen to this and refresh their text
 *    - OnRegionChanged fires when formatting preferences change
 *
 * 6. METRIC VS IMPERIAL UNITS
 *    - This is a racing game, so speed and distance are shown constantly
 *    - USA uses miles/mph, most of the world uses kilometers/km/h
 *    - FormatSpeed() and FormatDistance() handle this automatically
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ========================================
 *
 *    +-------------------+
 *    |   Game Instance   |
 *    +--------+----------+
 *             |
 *    +--------v----------+
 *    | Localization      |<---- Player changes language in Settings Menu
 *    | Subsystem         |
 *    +--------+----------+
 *             |
 *             | Provides localized strings to:
 *             |
 *    +--------v--------------------------------------------------+
 *    |                                                           |
 *    |  +-------------+  +-------------+  +------------------+   |
 *    |  | Main Menu   |  | HUD/UI      |  | Dialogue System  |   |
 *    |  | Widgets     |  | Widgets     |  | (Subtitles)      |   |
 *    |  +-------------+  +-------------+  +------------------+   |
 *    |                                                           |
 *    |  +-------------+  +-------------+  +------------------+   |
 *    |  | Race        |  | Garage/     |  | Notification     |   |
 *    |  | Results     |  | Store UI    |  | Popups           |   |
 *    |  +-------------+  +-------------+  +------------------+   |
 *    |                                                           |
 *    +-----------------------------------------------------------+
 *
 * DATA FLOW FOR DISPLAYING LOCALIZED TEXT:
 *
 *    String Tables (Data Assets)
 *    +---------------------------+
 *    | ID: "UI.Race.Lap"         |
 *    | EN: "Lap"                 |
 *    | FR: "Tour"                |
 *    | DE: "Runde"               |
 *    | JP: "Lap" (in Japanese)   |
 *    +---------------------------+
 *             |
 *             v
 *    +---------------------------+
 *    | Localization Subsystem    |
 *    | - Stores current language |
 *    | - Looks up string by ID   |
 *    +---------------------------+
 *             |
 *             v
 *    +---------------------------+
 *    | HUD Widget calls:         |
 *    | GetLocalizedString("...")  |
 *    | Returns "Runde" (if DE)   |
 *    +---------------------------+
 *
 *
 * COMMON TASKS FOR NEW DEVELOPERS:
 * =================================
 *
 * Displaying localized text in a Widget Blueprint:
 *   1. Get reference to LocalizationSubsystem
 *   2. Call GetLocalizedString with your string ID
 *   3. Set the returned FText to your Text widget
 *   4. Bind to OnLanguageChanged to refresh when language changes
 *
 * Adding a new localizable string:
 *   1. Add entry to the string table data asset for each supported language
 *   2. Use a hierarchical ID: "Category.Subcategory.StringName"
 *   3. Call GetLocalizedString("Category.Subcategory.StringName")
 *
 * Using format strings with variables:
 *   // String table: "Race.Position" = "You finished in {0} place!"
 *   TArray<FText> Args;
 *   Args.Add(FText::FromString("1st"));
 *   FText Result = Localization->FormatLocalizedString("Race.Position", Args);
 *   // Result: "You finished in 1st place!"
 *
 * Displaying formatted speed:
 *   float SpeedMetersPerSec = 44.7f;  // ~100 mph
 *   FText SpeedText = Localization->FormatSpeed(SpeedMetersPerSec);
 *   // Returns "100 mph" (USA) or "161 km/h" (Europe) based on settings
 *
 * Reading current language:
 *   EMGLanguage CurrentLang = Localization->GetCurrentLanguage();
 *   if (CurrentLang == EMGLanguage::Japanese)
 *   {
 *       // Use Japanese-specific font or layout adjustments
 *   }
 *
 *
 * SUPPORTED LANGUAGES:
 * ====================
 * - English (US) - Default
 * - Spanish, French, German, Italian, Portuguese (Brazilian)
 * - Japanese, Korean, Chinese (Simplified & Traditional)
 * - Russian, Polish, Turkish
 * - Arabic (Right-to-Left support)
 *
 *
 * IMPORTANT NOTES:
 * =================
 * - ALWAYS use FText for player-visible strings
 * - NEVER hardcode strings - always use string IDs
 * - Test UI layouts with German (longest translations) and Chinese (different fonts)
 * - Arabic requires special RTL testing
 * - Speed/distance formatting should always go through this subsystem
 *
 *
 * @see FMGLocalizationSettings - The data structure holding all settings
 * @see UMGAccessibilitySubsystem - For text size and subtitle settings
 * @see Unreal's FText documentation for more on localization best practices
 *
 ******************************************************************************/

/**
 * @file MGLocalizationSubsystem.h
 * @brief Localization and Internationalization Subsystem for Midnight Grind
 *
 * This subsystem manages all language and regional settings for the game,
 * ensuring players worldwide can enjoy the game in their preferred language
 * with appropriate formatting for their region.
 *
 * ## Key Features
 *
 * - **Language Support**: Text localization for 14+ languages including
 *   full support for right-to-left (RTL) languages like Arabic.
 *
 * - **Regional Formatting**: Automatic formatting of numbers, currency,
 *   dates, times, distances, and speeds based on player's region.
 *
 * - **Dual Audio/Text**: Separate language settings for audio (voice acting)
 *   and text, allowing players to hear original voice acting with subtitles.
 *
 * - **System Integration**: Can automatically detect and use the device's
 *   system language setting.
 *
 * - **Unit Systems**: Support for both metric and imperial measurement units.
 *
 * ## String Localization
 *
 * All localizable strings are stored in string tables and accessed via
 * string IDs. Use GetLocalizedString() for simple strings or
 * FormatLocalizedString() for strings with dynamic arguments.
 *
 * @note This subsystem should be accessed early during game initialization
 *       to ensure all UI text is properly localized from the start.
 *
 * @see FMGLocalizationSettings for the complete settings structure
 * @see UMGAccessibilitySubsystem for accessibility-related text settings
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLocalizationSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Supported languages for text and audio localization
 *
 * Each language has its own string table and may have separate audio
 * localization (voice acting). Not all languages may have full audio
 * localization - check GetAvailableLanguages() for current support.
 */
UENUM(BlueprintType)
enum class EMGLanguage : uint8
{
	English,           ///< English (US) - Default language
	Spanish,           ///< Spanish (Castilian)
	French,            ///< French (France)
	German,            ///< German (Germany)
	Italian,           ///< Italian
	Portuguese,        ///< Portuguese (Brazilian)
	Japanese,          ///< Japanese - Uses CJK character support
	Korean,            ///< Korean - Uses CJK character support
	ChineseSimplified, ///< Simplified Chinese (Mainland China)
	ChineseTraditional,///< Traditional Chinese (Taiwan/Hong Kong)
	Russian,           ///< Russian - Uses Cyrillic characters
	Polish,            ///< Polish
	Arabic,            ///< Arabic - Right-to-left language
	Turkish            ///< Turkish
};

/**
 * @brief Geographic regions for formatting preferences
 *
 * Regions determine default formatting for dates, times, numbers, and
 * measurement units. Players can override individual formatting options
 * if their preferences differ from regional defaults.
 */
UENUM(BlueprintType)
enum class EMGRegion : uint8
{
	NorthAmerica, ///< USA/Canada - Imperial units, MM/DD/YYYY, 12-hour time
	Europe,       ///< European Union - Metric units, DD/MM/YYYY, 24-hour time
	Asia,         ///< East Asia - Metric units, YYYY/MM/DD, 24-hour time
	LatinAmerica, ///< Central/South America - Metric units, DD/MM/YYYY
	MiddleEast,   ///< Middle East - Metric units, right-to-left where applicable
	Oceania       ///< Australia/New Zealand - Metric units, DD/MM/YYYY
};

//=============================================================================
// Data Structures
//=============================================================================

/**
 * @brief A localizable string with translations in multiple languages
 *
 * Used internally by the localization system to store strings that have
 * been translated into multiple languages. Each string is identified by
 * a unique StringID.
 */
USTRUCT(BlueprintType)
struct FMGLocalizedString
{
	GENERATED_BODY()

	/// Unique identifier for this string (e.g., "UI.MainMenu.PlayButton")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StringID;

	/// Map of language to translated text
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGLanguage, FText> Translations;
};

/**
 * @brief Complete localization settings for a player
 *
 * Contains all language and regional preferences. These settings are
 * persisted to local storage and loaded automatically on game start.
 */
USTRUCT(BlueprintType)
struct FMGLocalizationSettings
{
	GENERATED_BODY()

	//-------------------------------------------------------------------------
	// Language Settings
	//-------------------------------------------------------------------------

	/// Primary language for all UI text and subtitles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLanguage CurrentLanguage = EMGLanguage::English;

	/// Language for voice acting and audio dialogue (can differ from text)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLanguage AudioLanguage = EMGLanguage::English;

	//-------------------------------------------------------------------------
	// Region Settings
	//-------------------------------------------------------------------------

	/// Geographic region for formatting defaults
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRegion Region = EMGRegion::NorthAmerica;

	/// When true, automatically detects language from device settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseSystemLanguage = true;

	//-------------------------------------------------------------------------
	// Display Settings
	//-------------------------------------------------------------------------

	/// Enable subtitles for dialogue and cinematics
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSubtitles = true;

	/// Enable right-to-left UI layout (automatically set for Arabic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRightToLeftUI = false;

	//-------------------------------------------------------------------------
	// Formatting Preferences
	//-------------------------------------------------------------------------

	/// Date format string (e.g., "MM/DD/YYYY", "DD/MM/YYYY", "YYYY-MM-DD")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DateFormat = TEXT("MM/DD/YYYY");

	/// Time format: "12h" for 12-hour with AM/PM, "24h" for 24-hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TimeFormat = TEXT("12h");

	/// Use metric units (km, m/s) instead of imperial (mi, mph)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMetricUnits = false;
};

//=============================================================================
// Delegate Declarations
//=============================================================================

/// Broadcast when the display language changes; UI should refresh
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLanguageChanged, EMGLanguage, NewLanguage);

/// Broadcast when the region changes; formatters should update
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRegionChanged, EMGRegion, NewRegion);

//=============================================================================
// Localization Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing localization and internationalization
 *
 * Provides APIs for language selection, string localization, and regional
 * formatting. All UI and gameplay systems should use this subsystem for
 * any player-visible text or formatted values.
 *
 * ## Usage Example (Blueprint)
 * @code
 * // Get localized text
 * FText ButtonText = LocalizationSubsystem->GetLocalizedString("UI.MainMenu.Play");
 *
 * // Format a number based on locale
 * FText Score = LocalizationSubsystem->FormatNumber(1234567); // "1,234,567" or "1.234.567"
 *
 * // Format speed for display
 * FText Speed = LocalizationSubsystem->FormatSpeed(44.7f); // "100 mph" or "161 km/h"
 * @endcode
 *
 * ## Usage Example (C++)
 * @code
 * if (UMGLocalizationSubsystem* Localization = GetGameInstance()->GetSubsystem<UMGLocalizationSubsystem>())
 * {
 *     Localization->SetLanguage(EMGLanguage::Japanese);
 *     FText WelcomeText = Localization->GetLocalizedString("UI.Welcome");
 * }
 * @endcode
 *
 * @note Always bind to OnLanguageChanged to refresh UI when language changes
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Lifecycle
	//-------------------------------------------------------------------------

	/// Called when game instance creates this subsystem; loads settings and string tables
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// Called when game instance shuts down; saves current settings
	virtual void Deinitialize() override;

	//-------------------------------------------------------------------------
	// Language Management
	//-------------------------------------------------------------------------

	/**
	 * @brief Changes the current display language
	 * @param Language The language to switch to
	 * @note Triggers OnLanguageChanged event; UI should refresh in response
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(EMGLanguage Language);

	/**
	 * @brief Returns the currently active display language
	 * @return Current language enum value
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	EMGLanguage GetCurrentLanguage() const { return Settings.CurrentLanguage; }

	/**
	 * @brief Returns all languages that have been localized
	 * @return Array of available language options
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	TArray<EMGLanguage> GetAvailableLanguages() const;

	/**
	 * @brief Gets the display name of a language in that language
	 * @param Language The language to get the name for
	 * @return Localized name (e.g., "Deutsch" for German, "Nihongo" for Japanese)
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLanguageDisplayName(EMGLanguage Language) const;

	//-------------------------------------------------------------------------
	// String Localization
	//-------------------------------------------------------------------------

	/**
	 * @brief Retrieves a localized string by its ID
	 * @param StringID Unique identifier for the string (e.g., "UI.Menu.Start")
	 * @return Localized text in the current language, or the ID if not found
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedString(FName StringID) const;

	/**
	 * @brief Retrieves and formats a localized string with arguments
	 * @param StringID Unique identifier for the string
	 * @param Arguments Array of values to substitute into placeholders ({0}, {1}, etc.)
	 * @return Formatted localized text
	 *
	 * @example
	 * // String table: "Race.Win" = "Congratulations {0}! You finished in {1} place!"
	 * FText Result = FormatLocalizedString("Race.Win", {"PlayerName", "1st"});
	 * // Result: "Congratulations PlayerName! You finished in 1st place!"
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatLocalizedString(FName StringID, const TArray<FText>& Arguments) const;

	//-------------------------------------------------------------------------
	// Region and Formatting
	//-------------------------------------------------------------------------

	/**
	 * @brief Sets the player's geographic region
	 * @param Region The region to use for formatting defaults
	 * @note Triggers OnRegionChanged event
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetRegion(EMGRegion Region);

	/**
	 * @brief Formats a number according to regional conventions
	 * @param Number The number to format
	 * @return Formatted number (e.g., "1,234,567" in US, "1.234.567" in Germany)
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatNumber(int64 Number) const;

	/**
	 * @brief Formats a currency amount
	 * @param Amount The amount in the smallest currency unit (e.g., cents)
	 * @param bIncludeSymbol Whether to include the currency symbol
	 * @return Formatted currency string (e.g., "$12.34" or "12,34 EUR")
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatCurrency(int64 Amount, bool bIncludeSymbol = true) const;

	/**
	 * @brief Formats a distance value with appropriate units
	 * @param Meters Distance in meters
	 * @return Formatted distance (e.g., "1.5 km" or "0.9 mi" based on settings)
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatDistance(float Meters) const;

	/**
	 * @brief Formats a speed value with appropriate units
	 * @param MetersPerSecond Speed in meters per second
	 * @return Formatted speed (e.g., "161 km/h" or "100 mph" based on settings)
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatSpeed(float MetersPerSecond) const;

	/**
	 * @brief Formats a date and time according to regional settings
	 * @param DateTime The date/time to format
	 * @return Formatted date/time string
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatDateTime(const FDateTime& DateTime) const;

	/**
	 * @brief Formats a time duration
	 * @param Duration The duration to format
	 * @return Formatted duration (e.g., "2:34:56" or "5m 30s")
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FText FormatDuration(const FTimespan& Duration) const;

	//-------------------------------------------------------------------------
	// Settings Access
	//-------------------------------------------------------------------------

	/**
	 * @brief Returns the complete localization settings
	 * @return Copy of current settings structure
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	FMGLocalizationSettings GetSettings() const { return Settings; }

	/**
	 * @brief Sets whether to use metric or imperial units
	 * @param bMetric True for metric (km, km/h), false for imperial (mi, mph)
	 */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetUseMetricUnits(bool bMetric);

	/**
	 * @brief Checks if metric units are enabled
	 * @return True if using metric, false if using imperial
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	bool UsesMetricUnits() const { return Settings.bUseMetricUnits; }

	/**
	 * @brief Checks if the current language uses right-to-left layout
	 * @return True if UI should be mirrored for RTL languages
	 */
	UFUNCTION(BlueprintPure, Category = "Localization")
	bool IsRightToLeft() const { return Settings.bRightToLeftUI; }

	//-------------------------------------------------------------------------
	// Events
	//-------------------------------------------------------------------------

	/// Broadcast when display language changes; bind to refresh UI
	UPROPERTY(BlueprintAssignable, Category = "Localization|Events")
	FMGOnLanguageChanged OnLanguageChanged;

	/// Broadcast when region changes; bind to update formatted values
	UPROPERTY(BlueprintAssignable, Category = "Localization|Events")
	FMGOnRegionChanged OnRegionChanged;

protected:
	//-------------------------------------------------------------------------
	// Internal Methods
	//-------------------------------------------------------------------------

	/// Loads localization settings from local storage
	void LoadSettings();

	/// Saves current settings to local storage
	void SaveSettings();

	/// Detects and applies the device's system language
	void DetectSystemLanguage();

	/// Loads the string table for a specific language
	void LoadStringTable(EMGLanguage Language);

	/// Updates UI direction based on current language (LTR or RTL)
	void UpdateUIDirection();

private:
	//-------------------------------------------------------------------------
	// Internal State
	//-------------------------------------------------------------------------

	/// Current localization settings
	UPROPERTY()
	FMGLocalizationSettings Settings;

	/// Loaded string table mapping IDs to localized strings
	UPROPERTY()
	TMap<FName, FMGLocalizedString> StringTable;
};
