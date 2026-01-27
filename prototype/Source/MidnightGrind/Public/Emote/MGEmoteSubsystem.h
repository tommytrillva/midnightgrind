// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGEmoteSubsystem.h
 * @brief Emote and Expression Subsystem for Midnight Grind
 *
 * This subsystem manages all player expression features including character animations,
 * vehicle effects, sounds, and visual flourishes that allow players to express themselves
 * during gameplay. Emotes are a key social feature enabling non-verbal communication
 * and celebration.
 *
 * Key Features:
 * - Character and vehicle emote animations (dances, celebrations, taunts)
 * - Vehicle-specific expressions (burnouts, drifts, horn honks)
 * - Context-aware emotes (victory, defeat, podium, lobby)
 * - Emote wheel UI with customizable slots
 * - Quick-select hotkeys for favorite emotes
 * - Emote loadouts for different situations
 * - Unlock and collection tracking system
 * - Rarity tiers for cosmetic progression
 * - Usage statistics and favorites
 *
 * Usage:
 * Access via UGameInstance::GetSubsystem<UMGEmoteSubsystem>().
 * Configure emote loadouts in the garage/customization menu, then trigger
 * emotes during gameplay using the wheel, quick-select, or context triggers.
 *
 * @see UMGQuickChatSubsystem for text-based communication
 * @see EMGEmoteType for emote classifications
 * @see EMGEmoteContext for situational triggers
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGEmoteSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Classification of emote types
 *
 * Different emote types have different visual presentations and may be
 * restricted to certain contexts or vehicle/character states.
 */
UENUM(BlueprintType)
enum class EMGEmoteType : uint8
{
	None,           ///< No emote / invalid
	Celebration,    ///< Victory celebrations and positive expressions
	Taunt,          ///< Competitive taunts directed at opponents
	Greeting,       ///< Friendly greetings and acknowledgments
	Reaction,       ///< Reactive expressions (surprised, frustrated, etc.)
	Dance,          ///< Character dance animations (lobby/podium only)
	Horn,           ///< Custom vehicle horn sounds
	Burnout,        ///< Vehicle burnout/tire smoke effects
	Drift,          ///< Special drift flourish effects
	Sticker,        ///< Temporary stickers applied to vehicle
	Banner          ///< Banner or flag display effects
};

/**
 * @brief Contexts in which emotes can be triggered
 *
 * Some emotes are only appropriate in certain game states. This enum
 * controls when each emote type is available to play.
 */
UENUM(BlueprintType)
enum class EMGEmoteContext : uint8
{
	Any,            ///< Can be used in any context
	PreRace,        ///< Before race starts (starting grid)
	Victory,        ///< After winning a race
	Defeat,         ///< After losing a race
	Podium,         ///< On the victory podium
	Garage,         ///< In the garage/customization screen
	Lobby,          ///< In multiplayer lobby
	InRace,         ///< During active racing
	Spectating      ///< While spectating other players
};

/**
 * @brief How an emote was triggered
 *
 * Tracks the input method used to activate an emote, useful for
 * analytics and ensuring appropriate UI feedback.
 */
UENUM(BlueprintType)
enum class EMGEmoteTrigger : uint8
{
	Manual,         ///< Directly activated by player input
	Automatic,      ///< System-triggered (e.g., auto victory emote)
	WheelMenu,      ///< Selected from the emote wheel UI
	QuickSelect,    ///< Triggered via quick-select hotkey
	Random          ///< Randomly selected from available emotes
};

/**
 * @brief Rarity tiers for emotes
 *
 * Rarity affects visual presentation (color coding), unlock difficulty,
 * and collectibility. Higher rarities are harder to obtain.
 */
UENUM(BlueprintType)
enum class EMGEmoteRarity : uint8
{
	Common,         ///< Basic emotes, easily obtained
	Uncommon,       ///< Slightly harder to obtain
	Rare,           ///< Requires significant progression
	Epic,           ///< Difficult challenges or premium
	Legendary,      ///< Extremely rare, prestigious
	Exclusive       ///< Limited-time or special event only
};

//=============================================================================
// Data Structures - Emote Definition
//=============================================================================

/**
 * @brief Complete definition of an emote
 *
 * Contains all assets, metadata, and configuration needed to display,
 * play, and manage an emote throughout the game.
 */
USTRUCT(BlueprintType)
struct FMGEmoteDefinition
{
	GENERATED_BODY()

	/// Unique identifier for this emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	/// Localized display name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/// Localized description of the emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/// Classification of this emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteType EmoteType = EMGEmoteType::Celebration;

	/// Rarity tier affecting presentation and unlock difficulty
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteRarity Rarity = EMGEmoteRarity::Common;

	/// Game states where this emote can be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGEmoteContext> ValidContexts;

	/// How long the emote plays (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	/// Time before emote can be used again (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 5.0f;

	/// Can this emote be cancelled mid-play?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInterruptible = true;

	/// Does this emote loop until manually stopped?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = false;

	/// Animation montage for character (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> CharacterAnimation;

	/// Animation montage for vehicle (if applicable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> VehicleAnimation;

	/// Sound effect or music to play with emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	/// Particle effect spawned during emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UParticleSystem> ParticleEffect;

	/// Icon texture for UI display
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	/// Sticker texture (for Sticker type emotes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> StickerTexture;

	/// Has the player unlocked this emote?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;

	/// Player level required to unlock (0 = no level requirement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockLevel = 0;

	/// In-game currency price to purchase (0 = not purchasable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PurchasePrice = 0;

	/// Achievement ID required to unlock (None = no achievement required)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredAchievement;

	/// Searchable tags for filtering (e.g., "funny", "cool", "victory")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;
};

//=============================================================================
// Data Structures - Active Emote State
//=============================================================================

/**
 * @brief Runtime state of an emote being played
 *
 * Tracks the current playback state of an emote, including timing,
 * position, and ownership information.
 */
USTRUCT(BlueprintType)
struct FMGActiveEmote
{
	GENERATED_BODY()

	/// ID of the emote being played
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	/// Player who triggered this emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	/// Context in which the emote was triggered
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteContext Context = EMGEmoteContext::Any;

	/// World time when emote started
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	/// How long the emote has been playing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	/// Total duration of this emote instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	/// Is the emote currently playing?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlaying = false;

	/// World position where emote is being performed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldPosition = FVector::ZeroVector;

	/// World rotation of the performer
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator WorldRotation = FRotator::ZeroRotator;
};

//=============================================================================
// Data Structures - Emote Wheel Configuration
//=============================================================================

/**
 * @brief A single slot in the emote wheel
 *
 * Represents one position in the radial emote selection UI.
 */
USTRUCT(BlueprintType)
struct FMGEmoteWheelSlot
{
	GENERATED_BODY()

	/// Position index in the wheel (0-7 for 8-slot wheel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	/// ID of the emote assigned to this slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	/// Optional type filter for this slot (None = any type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteType FilterType = EMGEmoteType::None;
};

/**
 * @brief Complete emote loadout configuration
 *
 * A loadout contains the player's emote wheel configuration plus
 * context-specific emote assignments (victory, defeat, etc.).
 * Players can save multiple loadouts for different situations.
 */
USTRUCT(BlueprintType)
struct FMGEmoteLoadout
{
	GENERATED_BODY()

	/// Display name for this loadout
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LoadoutName;

	/// Emotes assigned to wheel slots
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEmoteWheelSlot> WheelSlots;

	/// Emote to auto-play when winning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VictoryEmote;

	/// Emote to auto-play when losing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DefeatEmote;

	/// Emote to play on the victory podium
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PodiumEmote;

	/// Emote to play when greeting other players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GreetingEmote;

	/// Emotes bound to quick-select hotkeys (typically 1-4)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> QuickSelectEmotes;
};

//=============================================================================
// Data Structures - Statistics and Collection
//=============================================================================

/**
 * @brief Usage statistics for a single emote
 *
 * Tracks how often and in what contexts each emote has been used.
 * Used for analytics, achievements, and "most used" displays.
 */
USTRUCT(BlueprintType)
struct FMGEmoteUsageStats
{
	GENERATED_BODY()

	/// ID of the emote these stats belong to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	/// Total number of times this emote has been used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUses = 0;

	/// Number of times used after winning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VictoryUses = 0;

	/// Number of times used as a taunt
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TauntUses = 0;

	/// When this emote was last used
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUsed;

	/// Has the player marked this as a favorite?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFavorite = false;
};

/**
 * @brief Player's complete emote collection
 *
 * Tracks all unlocked emotes, favorites, and aggregate statistics
 * for the player's emote library.
 */
USTRUCT(BlueprintType)
struct FMGEmoteCollection
{
	GENERATED_BODY()

	/// IDs of all emotes the player has unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> UnlockedEmotes;

	/// IDs of emotes marked as favorites
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FavoriteEmotes;

	/// Usage statistics per emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FMGEmoteUsageStats> UsageStats;

	/// Total count of unlocked emotes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalEmotesUnlocked = 0;

	/// ID of the player's most frequently used emote
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MostUsedEmote;
};

//=============================================================================
// Delegates
//=============================================================================

/// Fires when any player starts playing an emote
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteStarted, FName, PlayerID, const FMGEmoteDefinition&, Emote);

/// Fires when an emote finishes playing naturally
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteEnded, FName, PlayerID, FName, EmoteID);

/// Fires when an emote is cancelled before completion
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteInterrupted, FName, PlayerID, FName, EmoteID);

/// Fires when the player unlocks a new emote
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmoteUnlocked, const FMGEmoteDefinition&, Emote);

/// Fires when an emote is assigned to a wheel slot
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmoteEquipped, int32, SlotIndex, FName, EmoteID);

/// Fires when the emote wheel UI is opened
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmoteWheelOpened);

/// Fires when a slot is selected from the emote wheel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmoteWheelSelection, int32, SlotIndex);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing emotes and player expressions
 *
 * UMGEmoteSubsystem handles all aspects of the emote system including:
 * - Emote playback and animation control
 * - Emote wheel and quick-select management
 * - Loadout saving and switching
 * - Collection tracking and unlocks
 * - Usage statistics
 * - Context-aware emote availability
 *
 * The subsystem coordinates with animation, audio, and particle systems
 * to deliver the full emote experience.
 *
 * Example usage:
 * @code
 * UMGEmoteSubsystem* EmoteSys = GetGameInstance()->GetSubsystem<UMGEmoteSubsystem>();
 * if (!EmoteSys->IsPlayingEmote() && !EmoteSys->IsEmoteOnCooldown(TEXT("Celebration_Dance")))
 * {
 *     EmoteSys->PlayEmote(TEXT("Celebration_Dance"), EMGEmoteContext::Victory);
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGEmoteSubsystem : public UGameInstanceSubsystem
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
	// Emote Playback - Core emote triggering functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Play an emote by its ID
	 * @param EmoteID The unique identifier of the emote to play
	 * @param Context The game context in which this is being triggered
	 * @return True if emote started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	bool PlayEmote(FName EmoteID, EMGEmoteContext Context = EMGEmoteContext::Any);

	/**
	 * @brief Play the emote assigned to a wheel slot
	 * @param WheelSlotIndex Index of the wheel slot (0-based)
	 * @return True if emote started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	bool PlayEmoteAtIndex(int32 WheelSlotIndex);

	/**
	 * @brief Play a random unlocked emote matching filters
	 * @param TypeFilter Only consider emotes of this type (None = any)
	 * @param ContextFilter Only consider emotes valid in this context
	 * @return True if an emote was found and played
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	bool PlayRandomEmote(EMGEmoteType TypeFilter = EMGEmoteType::None, EMGEmoteContext ContextFilter = EMGEmoteContext::Any);

	/** @brief Stop the currently playing emote (if interruptible) */
	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	void StopCurrentEmote();

	/**
	 * @brief Forcibly interrupt another player's emote
	 * @param PlayerID The player whose emote to interrupt
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Playback")
	void InterruptEmote(FName PlayerID);

	/**
	 * @brief Check if the local player is currently playing an emote
	 * @return True if an emote is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	bool IsPlayingEmote() const;

	/**
	 * @brief Check if a specific emote is on cooldown
	 * @param EmoteID The emote to check
	 * @return True if the emote cannot be used yet
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	bool IsEmoteOnCooldown(FName EmoteID) const;

	/**
	 * @brief Get remaining cooldown time for an emote
	 * @param EmoteID The emote to check
	 * @return Seconds until emote can be used (0 if ready)
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	float GetEmoteCooldownRemaining(FName EmoteID) const;

	/** @brief Get the currently playing emote state */
	UFUNCTION(BlueprintPure, Category = "Emote|Playback")
	FMGActiveEmote GetActiveEmote() const { return ActiveEmote; }

	//-------------------------------------------------------------------------
	// Remote Emotes - Handle emotes from other players
	//-------------------------------------------------------------------------

	/**
	 * @brief Play an emote for a remote player (network received)
	 * @param PlayerID The player performing the emote
	 * @param EmoteID The emote being performed
	 * @param Position World position of the performer
	 * @param Rotation World rotation of the performer
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Remote")
	void PlayRemoteEmote(FName PlayerID, FName EmoteID, const FVector& Position, const FRotator& Rotation);

	/** @brief Get all emotes currently being played by other players */
	UFUNCTION(BlueprintPure, Category = "Emote|Remote")
	TArray<FMGActiveEmote> GetActiveRemoteEmotes() const { return ActiveRemoteEmotes; }

	//-------------------------------------------------------------------------
	// Contextual Emotes - Auto-triggered situational emotes
	//-------------------------------------------------------------------------

	/** @brief Play the configured victory emote */
	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayVictoryEmote();

	/** @brief Play the configured defeat emote */
	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayDefeatEmote();

	/**
	 * @brief Play the configured podium emote
	 * @param Position The podium position (1st, 2nd, 3rd, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayPodiumEmote(int32 Position);

	/** @brief Play the configured greeting emote */
	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void PlayGreetingEmote();

	/**
	 * @brief Set the current game context (affects emote availability)
	 * @param Context The new context
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Context")
	void SetCurrentContext(EMGEmoteContext Context);

	/** @brief Get the current game context */
	UFUNCTION(BlueprintPure, Category = "Emote|Context")
	EMGEmoteContext GetCurrentContext() const { return CurrentContext; }

	//-------------------------------------------------------------------------
	// Emote Wheel - Radial selection UI management
	//-------------------------------------------------------------------------

	/** @brief Open the emote wheel UI */
	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void OpenEmoteWheel();

	/** @brief Close the emote wheel UI */
	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void CloseEmoteWheel();

	/**
	 * @brief Select and play an emote from the wheel
	 * @param SlotIndex The slot that was selected
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void SelectWheelSlot(int32 SlotIndex);

	/** @brief Check if the emote wheel is currently open */
	UFUNCTION(BlueprintPure, Category = "Emote|Wheel")
	bool IsEmoteWheelOpen() const { return bWheelOpen; }

	/** @brief Get the current wheel slot configuration */
	UFUNCTION(BlueprintPure, Category = "Emote|Wheel")
	TArray<FMGEmoteWheelSlot> GetWheelSlots() const;

	/**
	 * @brief Assign an emote to a wheel slot
	 * @param SlotIndex The slot to modify
	 * @param EmoteID The emote to assign
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Wheel")
	void SetWheelSlot(int32 SlotIndex, FName EmoteID);

	//-------------------------------------------------------------------------
	// Quick Select - Hotkey-bound emotes
	//-------------------------------------------------------------------------

	/**
	 * @brief Play a quick-select emote by its index
	 * @param Index Quick-select slot index (typically 0-3)
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|QuickSelect")
	void PlayQuickSelectEmote(int32 Index);

	/**
	 * @brief Assign an emote to a quick-select slot
	 * @param Index Quick-select slot index
	 * @param EmoteID The emote to assign
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|QuickSelect")
	void SetQuickSelectEmote(int32 Index, FName EmoteID);

	/** @brief Get IDs of all quick-select emotes */
	UFUNCTION(BlueprintPure, Category = "Emote|QuickSelect")
	TArray<FName> GetQuickSelectEmotes() const;

	//-------------------------------------------------------------------------
	// Loadouts - Emote configuration presets
	//-------------------------------------------------------------------------

	/**
	 * @brief Apply a loadout configuration
	 * @param Loadout The loadout to activate
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Loadout")
	void SetActiveLoadout(const FMGEmoteLoadout& Loadout);

	/** @brief Get the currently active loadout */
	UFUNCTION(BlueprintPure, Category = "Emote|Loadout")
	FMGEmoteLoadout GetActiveLoadout() const { return ActiveLoadout; }

	/**
	 * @brief Save a loadout to a slot
	 * @param Loadout The loadout configuration to save
	 * @param SlotIndex Which save slot to use
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Loadout")
	void SaveLoadout(const FMGEmoteLoadout& Loadout, int32 SlotIndex);

	/**
	 * @brief Get a saved loadout by slot index
	 * @param SlotIndex The save slot to retrieve
	 * @return The loadout configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Loadout")
	FMGEmoteLoadout GetLoadout(int32 SlotIndex) const;

	//-------------------------------------------------------------------------
	// Collection - Unlock and inventory management
	//-------------------------------------------------------------------------

	/**
	 * @brief Unlock an emote for the player
	 * @param EmoteID The emote to unlock
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Collection")
	bool UnlockEmote(FName EmoteID);

	/**
	 * @brief Check if an emote is unlocked
	 * @param EmoteID The emote to check
	 * @return True if the player owns this emote
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	bool IsEmoteUnlocked(FName EmoteID) const;

	/** @brief Get definitions of all unlocked emotes */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetUnlockedEmotes() const;

	/**
	 * @brief Get all emotes of a specific type
	 * @param Type The emote type to filter by
	 * @return Array of matching emote definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetEmotesByType(EMGEmoteType Type) const;

	/**
	 * @brief Get all emotes of a specific rarity
	 * @param Rarity The rarity tier to filter by
	 * @return Array of matching emote definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetEmotesByRarity(EMGEmoteRarity Rarity) const;

	/**
	 * @brief Get all emotes valid for a specific context
	 * @param Context The game context to filter by
	 * @return Array of matching emote definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetEmotesForContext(EMGEmoteContext Context) const;

	/** @brief Get the complete collection data */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	FMGEmoteCollection GetCollection() const { return Collection; }

	/**
	 * @brief Mark or unmark an emote as a favorite
	 * @param EmoteID The emote to modify
	 * @param bFavorite True to add to favorites, false to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Collection")
	void SetEmoteFavorite(FName EmoteID, bool bFavorite);

	/** @brief Get definitions of all favorited emotes */
	UFUNCTION(BlueprintPure, Category = "Emote|Collection")
	TArray<FMGEmoteDefinition> GetFavoriteEmotes() const;

	//-------------------------------------------------------------------------
	// Emote Database - Emote registration and lookup
	//-------------------------------------------------------------------------

	/**
	 * @brief Register a new emote definition
	 * @param Emote The emote definition to add to the database
	 */
	UFUNCTION(BlueprintCallable, Category = "Emote|Database")
	void RegisterEmote(const FMGEmoteDefinition& Emote);

	/**
	 * @brief Get an emote definition by ID
	 * @param EmoteID The emote to look up
	 * @return The emote definition (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Database")
	FMGEmoteDefinition GetEmoteDefinition(FName EmoteID) const;

	/** @brief Get all registered emote definitions */
	UFUNCTION(BlueprintPure, Category = "Emote|Database")
	TArray<FMGEmoteDefinition> GetAllEmotes() const;

	/** @brief Get total number of emotes in the database */
	UFUNCTION(BlueprintPure, Category = "Emote|Database")
	int32 GetTotalEmoteCount() const { return EmoteDatabase.Num(); }

	//-------------------------------------------------------------------------
	// Stats - Usage tracking and analytics
	//-------------------------------------------------------------------------

	/**
	 * @brief Get usage statistics for a specific emote
	 * @param EmoteID The emote to query
	 * @return Usage statistics struct
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Stats")
	FMGEmoteUsageStats GetEmoteStats(FName EmoteID) const;

	/** @brief Get the ID of the player's most used emote */
	UFUNCTION(BlueprintPure, Category = "Emote|Stats")
	FName GetMostUsedEmote() const;

	/**
	 * @brief Get a leaderboard of most-used emotes
	 * @param MaxEntries Maximum number of entries to return
	 * @return Array of usage stats, sorted by usage count
	 */
	UFUNCTION(BlueprintPure, Category = "Emote|Stats")
	TArray<FMGEmoteUsageStats> GetEmoteLeaderboard(int32 MaxEntries = 10) const;

	//-------------------------------------------------------------------------
	// Delegates - Bindable events
	//-------------------------------------------------------------------------

	/// Fires when any player starts an emote
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteStarted OnEmoteStarted;

	/// Fires when an emote finishes playing
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteEnded OnEmoteEnded;

	/// Fires when an emote is interrupted
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteInterrupted OnEmoteInterrupted;

	/// Fires when a new emote is unlocked
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteUnlocked OnEmoteUnlocked;

	/// Fires when an emote is assigned to a slot
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteEquipped OnEmoteEquipped;

	/// Fires when the emote wheel opens
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteWheelOpened OnEmoteWheelOpened;

	/// Fires when a wheel slot is selected
	UPROPERTY(BlueprintAssignable, Category = "Emote|Events")
	FOnEmoteWheelSelection OnEmoteWheelSelection;

protected:
	//-------------------------------------------------------------------------
	// Internal Implementation
	//-------------------------------------------------------------------------

	/// Called periodically to update emote state and cooldowns
	void OnEmoteTick();

	/// Update timing for all active emotes (local and remote)
	void UpdateActiveEmotes(float DeltaTime);

	/// Record emote usage for statistics
	void TrackEmoteUsage(FName EmoteID, EMGEmoteContext Context);

	/// Check if an emote can be played in a given context
	bool CanPlayEmoteInContext(const FMGEmoteDefinition& Emote, EMGEmoteContext Context) const;

	/// Persist emote collection and loadouts to save file
	void SaveEmoteData();

	/// Load emote collection and loadouts from save file
	void LoadEmoteData();

	/// Create default loadout for new players
	void InitializeDefaultLoadout();

	//-------------------------------------------------------------------------
	// Data Members
	//-------------------------------------------------------------------------

	/// All registered emote definitions
	UPROPERTY()
	TMap<FName, FMGEmoteDefinition> EmoteDatabase;

	/// Player's emote collection (unlocks, favorites, stats)
	UPROPERTY()
	FMGEmoteCollection Collection;

	/// Currently active loadout
	UPROPERTY()
	FMGEmoteLoadout ActiveLoadout;

	/// All saved loadout configurations
	UPROPERTY()
	TArray<FMGEmoteLoadout> SavedLoadouts;

	/// Local player's currently playing emote
	UPROPERTY()
	FMGActiveEmote ActiveEmote;

	/// Emotes being played by other players
	UPROPERTY()
	TArray<FMGActiveEmote> ActiveRemoteEmotes;

	/// Per-emote cooldown timers
	UPROPERTY()
	TMap<FName, float> EmoteCooldowns;

	/// Current game context for emote filtering
	UPROPERTY()
	EMGEmoteContext CurrentContext = EMGEmoteContext::Any;

	/// Is the emote wheel UI currently displayed?
	UPROPERTY()
	bool bWheelOpen = false;

	/// Local player's unique identifier
	UPROPERTY()
	FName LocalPlayerID;

	/// Timer handle for the tick function
	FTimerHandle EmoteTickHandle;
};
