// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPlayerTitleSubsystem.h
 * @brief Player Title, Banner, and Profile Customization Subsystem for Midnight Grind
 *
 * This subsystem manages all aspects of player identity customization including
 * titles, banners, nameplates, and profile display settings. These cosmetic
 * elements appear throughout the game in lobbies, leaderboards, race results,
 * and player profiles, allowing players to express their achievements and style.
 *
 * Key Features:
 * - Player titles with rarity tiers and animated effects
 * - Multi-layer banner customization (background, emblem, frame, effects)
 * - Nameplate styles for in-game display
 * - Profile configuration with visibility toggles
 * - Preset saving for quick loadout switching
 * - Unlock tracking tied to achievements, ranks, and seasons
 * - Collection progress statistics
 *
 * Usage:
 * Access via UGameInstance::GetSubsystem<UMGPlayerTitleSubsystem>().
 * Players can customize their profile through the garage/customization menu,
 * and their configured title/banner will appear to other players automatically.
 *
 * @see UMGEmoteSubsystem for expression features
 * @see EMGTitleRarity for rarity classifications
 * @see EMGTitleCategory for title groupings
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPlayerTitleSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Rarity tiers for titles and banner elements
 *
 * Rarity indicates how difficult an item is to obtain and affects
 * visual presentation (color coding, effects) in the UI.
 */
UENUM(BlueprintType)
enum class EMGTitleRarity : uint8
{
	Common,         ///< Basic items, easily obtained
	Uncommon,       ///< Requires some progression
	Rare,           ///< Significant achievement or grind
	Epic,           ///< Difficult challenges
	Legendary,      ///< Extremely prestigious
	Mythic,         ///< Near-impossible feats
	Unique          ///< One-of-a-kind special awards
};

/**
 * @brief Categories for organizing player titles
 *
 * Titles are grouped by how they were obtained to help players
 * find and showcase relevant accomplishments.
 */
UENUM(BlueprintType)
enum class EMGTitleCategory : uint8
{
	Achievement,    ///< Earned through specific achievements
	Rank,           ///< Based on competitive ranking
	Season,         ///< Seasonal rewards (time-limited)
	Event,          ///< Special event participation
	Social,         ///< Social features (crews, referrals)
	Collection,     ///< Collection milestones (cars, tracks)
	Mastery,        ///< Vehicle or track mastery
	Special,        ///< Miscellaneous special titles
	Developer       ///< Reserved for development team
};

/**
 * @brief Slots within the banner customization system
 *
 * Banners are composed of multiple layers that stack to create
 * a unique visual identity for each player.
 */
UENUM(BlueprintType)
enum class EMGBannerSlot : uint8
{
	Background,     ///< Base background layer (furthest back)
	Emblem,         ///< Central emblem/logo
	Frame,          ///< Border/frame around the banner
	Effect,         ///< Animated overlay effects (particles, glow)
	Accent          ///< Decorative accent elements
};

//=============================================================================
// Data Structures - Titles
//=============================================================================

/**
 * @brief Definition of a player title
 *
 * Titles are text labels displayed alongside a player's name to
 * showcase achievements, rank, or special status.
 */
USTRUCT(BlueprintType)
struct FMGPlayerTitle
{
	GENERATED_BODY()

	/// Unique identifier for this title
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TitleID;

	/// The displayed title text (e.g., "World Champion", "Road Legend")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TitleText;

	/// Description of how to earn this title
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Rarity tier of this title
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleRarity Rarity = EMGTitleRarity::Common;

	/// Category this title belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleCategory Category = EMGTitleCategory::Achievement;

	/// Primary color of the title text
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TitleColor = FLinearColor::White;

	/// Whether the title has a glowing effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasGlow = false;

	/// Color of the glow effect (if enabled)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor = FLinearColor::White;

	/// Whether the title has an animated effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnimated = false;

	/// Name of the animation style to use (shimmer, pulse, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AnimationStyle;

	/// Has the player unlocked this title?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	/// When this title was unlocked (if unlocked)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime UnlockedAt;

	/// ID of the requirement to unlock (achievement ID, rank threshold, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockRequirement;

	/// Hint text shown for locked titles
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockHint;

	/// Is this a limited-time title that can no longer be earned?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLimited = false;

	/// Season ID if this is a seasonal title
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	/// Sort priority for display ordering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

//=============================================================================
// Data Structures - Banner Elements
//=============================================================================

/**
 * @brief A single customizable element within a banner
 *
 * Banner elements are individual pieces (backgrounds, emblems, etc.)
 * that combine to create the player's complete banner design.
 */
USTRUCT(BlueprintType)
struct FMGBannerElement
{
	GENERATED_BODY()

	/// Unique identifier for this element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ElementID;

	/// Display name of this element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ElementName;

	/// Which banner layer this element occupies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBannerSlot Slot = EMGBannerSlot::Background;

	/// Rarity tier of this element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleRarity Rarity = EMGTitleRarity::Common;

	/// Texture asset for this element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ElementTexture;

	/// Optional material for advanced rendering (animated, special effects)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> ElementMaterial;

	/// Primary color applied to the element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	/// Secondary color for two-tone elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::White;

	/// Can the player customize this element's colors?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsColorizable = true;

	/// Does this element have animated effects?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnimated = false;

	/// Has the player unlocked this element?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	/// ID of unlock requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockRequirement;

	/// Hint text for locked elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnlockHint;

	/// Sort priority for display ordering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SortOrder = 0;
};

/**
 * @brief Complete banner configuration
 *
 * Represents the player's full banner setup including all layer
 * elements and color selections.
 */
USTRUCT(BlueprintType)
struct FMGPlayerBanner
{
	GENERATED_BODY()

	/// ID of the background element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BackgroundID;

	/// ID of the central emblem
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmblemID;

	/// ID of the frame/border
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FrameID;

	/// ID of the overlay effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EffectID;

	/// ID of the accent element
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AccentID;

	/// Primary color applied to colorizable elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	/// Secondary color for two-tone elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::White;

	/// Accent color for highlights and details
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AccentColor = FLinearColor::White;
};

//=============================================================================
// Data Structures - Nameplate
//=============================================================================

/**
 * @brief Nameplate style for in-game display
 *
 * Nameplates are the visual presentation of a player's name as it
 * appears above their vehicle or in UI elements during gameplay.
 */
USTRUCT(BlueprintType)
struct FMGNameplate
{
	GENERATED_BODY()

	/// Unique identifier for this nameplate style
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NameplateID;

	/// Display name of this style
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText NameplateName;

	/// Rarity tier of this nameplate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTitleRarity Rarity = EMGTitleRarity::Common;

	/// Background texture for the nameplate
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> NameplateTexture;

	/// Color of the player name text
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TextColor = FLinearColor::White;

	/// Color of the nameplate background
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BackgroundColor = FLinearColor::Black;

	/// Color of the nameplate border
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BorderColor = FLinearColor::White;

	/// Does this nameplate have animated effects?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnimated = false;

	/// Has the player unlocked this nameplate?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = false;

	/// ID of unlock requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockRequirement;
};

//=============================================================================
// Data Structures - Profile Configuration
//=============================================================================

/**
 * @brief Complete player profile display settings
 *
 * Controls what identity elements are shown and how they're configured
 * when other players view this player's profile or see them in-game.
 */
USTRUCT(BlueprintType)
struct FMGPlayerProfile
{
	GENERATED_BODY()

	/// Currently equipped title ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveTitleID;

	/// Complete banner configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerBanner Banner;

	/// Currently equipped nameplate style ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveNameplateID;

	/// Avatar frame style ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveAvatarFrameID;

	/// Featured titles to display on profile (up to 3)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ShowcaseTitles;

	/// Show competitive rank in profile?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRank = true;

	/// Show player level in profile?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLevel = true;

	/// Show crew affiliation in profile?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCrew = true;
};

/**
 * @brief Saved profile preset
 *
 * Allows players to save complete profile configurations and
 * quickly switch between different looks.
 */
USTRUCT(BlueprintType)
struct FMGTitlePreset
{
	GENERATED_BODY()

	/// Unique identifier for this preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetID;

	/// Display name for this preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PresetName;

	/// Complete profile configuration stored in this preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerProfile Profile;
};

//=============================================================================
// Delegates
//=============================================================================

/// Fires when a new title is unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleUnlocked, const FMGPlayerTitle&, Title);

/// Fires when a new banner element is unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBannerElementUnlocked, const FMGBannerElement&, Element);

/// Fires when the player equips a different title
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleEquipped, FName, TitleID);

/// Fires when the banner configuration changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBannerChanged);

/// Fires when any profile setting is updated
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfileUpdated);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing player identity customization
 *
 * UMGPlayerTitleSubsystem handles all aspects of how players present
 * themselves to others, including:
 * - Title selection and unlocking
 * - Banner layer customization
 * - Nameplate styling
 * - Profile visibility settings
 * - Preset management
 *
 * This subsystem persists profile data and broadcasts changes to ensure
 * other players see updated information.
 *
 * Example usage:
 * @code
 * UMGPlayerTitleSubsystem* TitleSys = GetGameInstance()->GetSubsystem<UMGPlayerTitleSubsystem>();
 *
 * // Equip a new title
 * if (TitleSys->IsTitleUnlocked(TEXT("Title_Champion")))
 * {
 *     TitleSys->EquipTitle(TEXT("Title_Champion"));
 * }
 *
 * // Customize banner colors
 * TitleSys->SetBannerColors(FLinearColor::Red, FLinearColor::Black, FLinearColor::Gold);
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPlayerTitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Subsystem Lifecycle
	//-------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	//-------------------------------------------------------------------------
	// Titles - Query functions
	//-------------------------------------------------------------------------

	/** @brief Get all titles in the database (locked and unlocked) */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetAllTitles() const;

	/** @brief Get only titles the player has unlocked */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetUnlockedTitles() const;

	/**
	 * @brief Get titles in a specific category
	 * @param Category The category to filter by
	 * @return Array of matching titles
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetTitlesByCategory(EMGTitleCategory Category) const;

	/**
	 * @brief Get titles of a specific rarity
	 * @param Rarity The rarity tier to filter by
	 * @return Array of matching titles
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	TArray<FMGPlayerTitle> GetTitlesByRarity(EMGTitleRarity Rarity) const;

	/**
	 * @brief Get a specific title by ID
	 * @param TitleID The title to retrieve
	 * @return The title definition (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	FMGPlayerTitle GetTitle(FName TitleID) const;

	/** @brief Get the currently equipped title */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	FMGPlayerTitle GetActiveTitle() const;

	/**
	 * @brief Check if a title is unlocked
	 * @param TitleID The title to check
	 * @return True if the player has unlocked this title
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Query")
	bool IsTitleUnlocked(FName TitleID) const;

	//-------------------------------------------------------------------------
	// Titles - Equip functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Equip a title to display
	 * @param TitleID The title to equip
	 * @return True if successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Equip")
	bool EquipTitle(FName TitleID);

	/** @brief Remove the currently equipped title (show no title) */
	UFUNCTION(BlueprintCallable, Category = "Titles|Equip")
	void UnequipTitle();

	//-------------------------------------------------------------------------
	// Titles - Unlock functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Unlock a title for the player
	 * @param TitleID The title to unlock
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Unlock")
	bool UnlockTitle(FName TitleID);

	//-------------------------------------------------------------------------
	// Banner - Element management
	//-------------------------------------------------------------------------

	/** @brief Get all banner elements in the database */
	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	TArray<FMGBannerElement> GetAllBannerElements() const;

	/** @brief Get only banner elements the player has unlocked */
	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	TArray<FMGBannerElement> GetUnlockedBannerElements() const;

	/**
	 * @brief Get banner elements for a specific slot
	 * @param Slot The banner layer to filter by
	 * @return Array of elements for that slot
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	TArray<FMGBannerElement> GetBannerElementsBySlot(EMGBannerSlot Slot) const;

	/**
	 * @brief Get a specific banner element by ID
	 * @param ElementID The element to retrieve
	 * @return The element definition (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	FMGBannerElement GetBannerElement(FName ElementID) const;

	/** @brief Get the current complete banner configuration */
	UFUNCTION(BlueprintPure, Category = "Titles|Banner")
	FMGPlayerBanner GetCurrentBanner() const;

	/**
	 * @brief Set an element in a banner slot
	 * @param Slot Which slot to modify
	 * @param ElementID The element to place in that slot
	 * @return True if successfully set
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Banner")
	bool SetBannerElement(EMGBannerSlot Slot, FName ElementID);

	/**
	 * @brief Set all banner colors at once
	 * @param Primary Primary color for colorizable elements
	 * @param Secondary Secondary color for two-tone elements
	 * @param Accent Accent color for highlights
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Banner")
	void SetBannerColors(FLinearColor Primary, FLinearColor Secondary, FLinearColor Accent);

	/**
	 * @brief Unlock a banner element
	 * @param ElementID The element to unlock
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Banner")
	bool UnlockBannerElement(FName ElementID);

	//-------------------------------------------------------------------------
	// Nameplates - Style management
	//-------------------------------------------------------------------------

	/** @brief Get all nameplate styles in the database */
	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	TArray<FMGNameplate> GetAllNameplates() const;

	/** @brief Get only nameplate styles the player has unlocked */
	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	TArray<FMGNameplate> GetUnlockedNameplates() const;

	/**
	 * @brief Get a specific nameplate by ID
	 * @param NameplateID The nameplate to retrieve
	 * @return The nameplate definition (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	FMGNameplate GetNameplate(FName NameplateID) const;

	/** @brief Get the currently equipped nameplate */
	UFUNCTION(BlueprintPure, Category = "Titles|Nameplate")
	FMGNameplate GetActiveNameplate() const;

	/**
	 * @brief Equip a nameplate style
	 * @param NameplateID The nameplate to equip
	 * @return True if successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Nameplate")
	bool EquipNameplate(FName NameplateID);

	/**
	 * @brief Unlock a nameplate style
	 * @param NameplateID The nameplate to unlock
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Nameplate")
	bool UnlockNameplate(FName NameplateID);

	//-------------------------------------------------------------------------
	// Profile - Complete profile management
	//-------------------------------------------------------------------------

	/** @brief Get the complete current profile configuration */
	UFUNCTION(BlueprintPure, Category = "Titles|Profile")
	FMGPlayerProfile GetProfile() const { return CurrentProfile; }

	/**
	 * @brief Apply a complete profile configuration
	 * @param Profile The profile settings to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Profile")
	void SetProfile(const FMGPlayerProfile& Profile);

	/**
	 * @brief Set titles to feature on the profile showcase
	 * @param TitleIDs Array of title IDs to showcase (up to 3)
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Profile")
	void SetShowcaseTitles(const TArray<FName>& TitleIDs);

	/**
	 * @brief Configure profile visibility options
	 * @param bShowRank Show competitive rank
	 * @param bShowLevel Show player level
	 * @param bShowCrew Show crew affiliation
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Profile")
	void SetDisplayOptions(bool bShowRank, bool bShowLevel, bool bShowCrew);

	//-------------------------------------------------------------------------
	// Presets - Profile preset management
	//-------------------------------------------------------------------------

	/**
	 * @brief Save current profile as a preset
	 * @param PresetID Unique ID for this preset
	 * @param PresetName Display name for this preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Presets")
	void SavePreset(FName PresetID, const FText& PresetName);

	/**
	 * @brief Load and apply a saved preset
	 * @param PresetID The preset to load
	 * @return True if preset was found and applied
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Presets")
	bool LoadPreset(FName PresetID);

	/**
	 * @brief Delete a saved preset
	 * @param PresetID The preset to delete
	 * @return True if preset was found and deleted
	 */
	UFUNCTION(BlueprintCallable, Category = "Titles|Presets")
	bool DeletePreset(FName PresetID);

	/** @brief Get all saved presets */
	UFUNCTION(BlueprintPure, Category = "Titles|Presets")
	TArray<FMGTitlePreset> GetPresets() const;

	//-------------------------------------------------------------------------
	// Statistics - Collection progress tracking
	//-------------------------------------------------------------------------

	/** @brief Get total count of unlocked titles */
	UFUNCTION(BlueprintPure, Category = "Titles|Stats")
	int32 GetTotalTitlesUnlocked() const;

	/** @brief Get total count of unlocked banner elements */
	UFUNCTION(BlueprintPure, Category = "Titles|Stats")
	int32 GetTotalBannerElementsUnlocked() const;

	/**
	 * @brief Get overall collection completion percentage
	 * @return Progress from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "Titles|Stats")
	float GetCollectionProgress() const;

	//-------------------------------------------------------------------------
	// Persistence - Save/Load functions
	//-------------------------------------------------------------------------

	/** @brief Save all title data to persistent storage */
	UFUNCTION(BlueprintCallable, Category = "Titles|Save")
	void SaveTitleData();

	/** @brief Load title data from persistent storage */
	UFUNCTION(BlueprintCallable, Category = "Titles|Save")
	void LoadTitleData();

	//-------------------------------------------------------------------------
	// Delegates - Bindable events
	//-------------------------------------------------------------------------

	/// Fires when a new title is unlocked
	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnTitleUnlocked OnTitleUnlocked;

	/// Fires when a new banner element is unlocked
	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnBannerElementUnlocked OnBannerElementUnlocked;

	/// Fires when the equipped title changes
	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnTitleEquipped OnTitleEquipped;

	/// Fires when banner configuration changes
	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnBannerChanged OnBannerChanged;

	/// Fires when any profile setting updates
	UPROPERTY(BlueprintAssignable, Category = "Titles|Events")
	FOnProfileUpdated OnProfileUpdated;

protected:
	//-------------------------------------------------------------------------
	// Internal Implementation
	//-------------------------------------------------------------------------

	/// Populate database with default titles
	void InitializeDefaultTitles();

	/// Populate database with default banner elements
	void InitializeDefaultBannerElements();

	/// Populate database with default nameplates
	void InitializeDefaultNameplates();

	/**
	 * @brief Get the standard color for a rarity tier
	 * @param Rarity The rarity to get color for
	 * @return Display color for that rarity
	 */
	FLinearColor GetRarityColor(EMGTitleRarity Rarity) const;

	//-------------------------------------------------------------------------
	// Data Members
	//-------------------------------------------------------------------------

	/// All registered titles
	UPROPERTY()
	TMap<FName, FMGPlayerTitle> AllTitles;

	/// All registered banner elements
	UPROPERTY()
	TMap<FName, FMGBannerElement> AllBannerElements;

	/// All registered nameplates
	UPROPERTY()
	TMap<FName, FMGNameplate> AllNameplates;

	/// Current profile configuration
	UPROPERTY()
	FMGPlayerProfile CurrentProfile;

	/// Saved profile presets
	UPROPERTY()
	TArray<FMGTitlePreset> Presets;
};
