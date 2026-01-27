// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGQuickChatSubsystem.h
 * @brief Quick Chat and Ping Communication Subsystem for Midnight Grind
 *
 * This subsystem provides the core communication infrastructure for player-to-player
 * interactions during gameplay. It enables rapid, pre-defined message communication
 * through a "quick chat" wheel system, as well as world-space pings for tactical
 * callouts and location marking.
 *
 * Key Features:
 * - Quick chat message wheel with customizable slots
 * - Category-based message organization (Greetings, Racing, Tactics, etc.)
 * - World-space ping system for marking locations, hazards, and opponents
 * - Visibility controls (All, Team Only, Nearby, Private)
 * - Voice line playback for chat messages
 * - Cooldown management to prevent spam
 * - Mute/unmute functionality for player management
 * - Chat history tracking
 *
 * Usage:
 * Access this subsystem via UGameInstance::GetSubsystem<UMGQuickChatSubsystem>().
 * Configure the chat wheel in the player's settings, then call SendQuickChat() or
 * SendQuickChatFromSlot() to transmit messages during gameplay.
 *
 * @see UMGEmoteSubsystem for visual expression features
 * @see EMGQuickChatCategory for message categorization
 * @see EMGPingType for ping classifications
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGQuickChatSubsystem.generated.h"

//=============================================================================
// Enumerations
//=============================================================================

/**
 * @brief Categories for organizing quick chat messages
 *
 * Messages are grouped into categories to help players quickly find
 * appropriate responses during fast-paced gameplay.
 */
UENUM(BlueprintType)
enum class EMGQuickChatCategory : uint8
{
	Greetings,      ///< Friendly greetings and salutations ("Hello!", "Good luck!")
	Racing,         ///< Race-specific callouts ("Watch your left!", "Drafting!")
	TeamTactics,    ///< Team coordination messages ("Follow me!", "Split up!")
	Reactions,      ///< Emotional reactions ("Nice!", "Oops!", "Wow!")
	Compliments,    ///< Positive feedback to other players ("Great move!", "Well played!")
	Taunts,         ///< Playful competitive messages ("See you at the finish!", "Too slow!")
	Callouts,       ///< Hazard and situation alerts ("Shortcut ahead!", "Cops!")
	Custom          ///< User-created custom messages
};

/**
 * @brief Visibility scope for quick chat messages
 *
 * Controls who can see/hear a quick chat message when sent.
 * This allows for both public banter and private team communication.
 */
UENUM(BlueprintType)
enum class EMGQuickChatVisibility : uint8
{
	All,            ///< Visible to all players in the session
	TeamOnly,       ///< Only visible to teammates
	NearbyOnly,     ///< Only visible to players within NearbyRange distance
	Private         ///< Only visible to specific targeted player(s)
};

/**
 * @brief Types of world-space pings
 *
 * Pings are visual markers placed in the game world to communicate
 * locations and tactical information without voice chat.
 */
UENUM(BlueprintType)
enum class EMGPingType : uint8
{
	Location,       ///< Generic location marker ("Look here")
	Warning,        ///< Danger/caution marker ("Watch out!")
	Shortcut,       ///< Shortcut or alternate route marker
	Police,         ///< Police/authority presence warning
	Obstacle,       ///< Road hazard or obstacle warning
	Opponent,       ///< Enemy player position marker
	Help,           ///< Request for assistance
	Custom          ///< User-defined ping type
};

//=============================================================================
// Data Structures - Messages
//=============================================================================

/**
 * @brief Definition of a single quick chat message
 *
 * Contains all data needed to display, play, and manage a quick chat message
 * including localization, audio, and unlock requirements.
 */
USTRUCT(BlueprintType)
struct FMGQuickChatMessage
{
	GENERATED_BODY()

	/// Unique identifier for this message (used for saving/loading loadouts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MessageID;

	/// Category this message belongs to (for filtering in UI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGQuickChatCategory Category = EMGQuickChatCategory::Greetings;

	/// Short display text shown in the chat wheel UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayText;

	/// Full localized text shown when message is sent
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LocalizedText;

	/// Optional voice line audio to play with the message
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> VoiceLine;

	/// Wwise/FMOD audio event name for voice line (alternative to VoiceLine asset)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AudioEventName;

	/// Slot index in the quick chat wheel (-1 if not assigned to wheel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = -1;

	/// Whether the player has unlocked this message
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = true;

	/// Player level required to unlock (0 = unlocked by default)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockLevel = 0;

	/// In-game currency cost to unlock (0 = free)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnlockCost = 0;
};

/**
 * @brief A chat event representing a message sent by a player
 *
 * Encapsulates all context about a sent message including sender info,
 * timestamp, and visibility settings. Used for display and history tracking.
 */
USTRUCT(BlueprintType)
struct FMGChatEvent
{
	GENERATED_BODY()

	/// Unique identifier of the player who sent the message
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SenderID;

	/// Display name of the sender
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SenderName;

	/// The message that was sent
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGQuickChatMessage Message;

	/// When the message was sent (UTC)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/// Visibility scope of this message
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::All;

	/// World position of sender when message was sent (for proximity checks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SenderLocation = FVector::ZeroVector;

	/// Team ID of the sender (-1 = no team)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamID = -1;
};

//=============================================================================
// Data Structures - Pings
//=============================================================================

/**
 * @brief A world-space ping marker
 *
 * Pings are temporary markers placed in the 3D world to communicate
 * locations, threats, or points of interest to other players.
 */
USTRUCT(BlueprintType)
struct FMGWorldPing
{
	GENERATED_BODY()

	/// Unique identifier for this ping instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PingID;

	/// Player who created the ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OwnerID;

	/// Display name of the player who created the ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerName;

	/// Classification of what this ping represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPingType PingType = EMGPingType::Location;

	/// 3D world position of the ping marker
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	/// Direction the ping is facing (for directional pings like shortcuts)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldDirection = FVector::ForwardVector;

	/// When the ping was created
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	/// Total duration the ping will be active (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0f;

	/// Remaining time before ping expires (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 5.0f;

	/// Color used to render the ping (often based on PingType)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PingColor = FLinearColor::White;

	/// Optional text label displayed near the ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PingLabel;

	/// Who can see this ping
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::TeamOnly;

	/// Team ID for team-visibility filtering (-1 = no team)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamID = -1;

	/// Whether this ping is currently active and visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;
};

//=============================================================================
// Data Structures - Chat Wheel Configuration
//=============================================================================

/**
 * @brief A chat wheel containing multiple message slots
 *
 * The chat wheel is the radial UI element players use to quickly select
 * and send messages. Players can have multiple wheels for different situations.
 */
USTRUCT(BlueprintType)
struct FMGQuickChatWheel
{
	GENERATED_BODY()

	/// Unique identifier for this wheel configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WheelID;

	/// Display name of this wheel (e.g., "Racing", "Social")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WheelName;

	/// Messages assigned to this wheel's slots
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGQuickChatMessage> Messages;

	/// Maximum number of message slots in this wheel
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSlots = 8;
};

/**
 * @brief Configuration settings for the quick chat system
 *
 * Controls behavior like cooldowns, volume, range thresholds, and muting.
 */
USTRUCT(BlueprintType)
struct FMGQuickChatConfig
{
	GENERATED_BODY()

	/// Minimum time between sending messages (seconds, anti-spam)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MessageCooldown = 1.0f;

	/// Minimum time between creating pings (seconds, anti-spam)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PingCooldown = 2.0f;

	/// Maximum number of active pings a player can have simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPingsPerPlayer = 3;

	/// How long pings remain visible by default (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PingDefaultDuration = 5.0f;

	/// Distance threshold for "Nearby" visibility (Unreal units, typically cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearbyRange = 5000.0f;

	/// Whether to play voice line audio with messages
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPlayVoiceLines = true;

	/// Volume multiplier for voice lines (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VoiceLineVolume = 1.0f;

	/// Whether to show floating chat bubbles above players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowChatBubbles = true;

	/// How long chat bubbles remain visible (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChatBubbleDuration = 3.0f;

	/// If true, messages from opponents are not displayed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuteOpponents = false;

	/// List of individually muted player IDs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> MutedPlayers;
};

//=============================================================================
// Delegates
//=============================================================================

/// Broadcast when a quick chat message is received from any player
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickChatReceived, const FMGChatEvent&, ChatEvent);

/// Broadcast when a new ping is created in the world
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPingCreated, const FMGWorldPing&, Ping);

/// Broadcast when a ping expires or is manually removed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPingExpired, const FMGWorldPing&, Ping);

/// Broadcast when the message cooldown starts (parameter is duration in seconds)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatCooldownStarted, float, Duration);

/// Broadcast when the message cooldown ends and player can send again
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChatCooldownEnded);

/// Broadcast when a new quick chat message is unlocked
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickChatUnlocked, FName, MessageID, EMGQuickChatCategory, Category);

//=============================================================================
// Main Subsystem Class
//=============================================================================

/**
 * @brief Game instance subsystem managing quick chat and ping communication
 *
 * UMGQuickChatSubsystem is the central hub for all quick chat and ping functionality.
 * It manages message libraries, chat wheels, ping lifecycles, cooldowns, and
 * visibility filtering.
 *
 * This subsystem persists for the lifetime of the game instance and maintains
 * chat history across sessions.
 *
 * Example usage:
 * @code
 * UMGQuickChatSubsystem* ChatSys = GetGameInstance()->GetSubsystem<UMGQuickChatSubsystem>();
 * if (ChatSys->CanSendMessage())
 * {
 *     ChatSys->SendQuickChat(TEXT("Greeting_Hello"), EMGQuickChatVisibility::All);
 * }
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGQuickChatSubsystem : public UGameInstanceSubsystem
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
	// Send Messages - Core message transmission functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Send a quick chat message by its ID
	 * @param MessageID The unique identifier of the message to send
	 * @param Visibility Who should receive this message
	 * @return True if message was sent successfully, false if on cooldown or invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Send")
	bool SendQuickChat(FName MessageID, EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::All);

	/**
	 * @brief Send the message assigned to a specific wheel slot
	 * @param SlotIndex Index of the slot in the active chat wheel (0-based)
	 * @return True if message was sent successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Send")
	bool SendQuickChatFromSlot(int32 SlotIndex);

	/**
	 * @brief Send a custom text message (for user-typed messages)
	 * @param Text The custom message text to send
	 * @param Visibility Who should receive this message
	 * @return True if message was sent successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Send")
	bool SendCustomMessage(const FText& Text, EMGQuickChatVisibility Visibility = EMGQuickChatVisibility::All);

	/**
	 * @brief Check if the player can currently send a message
	 * @return True if not on cooldown and system is ready
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Send")
	bool CanSendMessage() const;

	/**
	 * @brief Get remaining cooldown time before next message can be sent
	 * @return Seconds remaining (0 if no cooldown active)
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Send")
	float GetMessageCooldownRemaining() const;

	//-------------------------------------------------------------------------
	// Pings - World-space marker functions
	//-------------------------------------------------------------------------

	/**
	 * @brief Create a ping at a world location
	 * @param Location World position to place the ping
	 * @param PingType Classification of the ping
	 * @return GUID of the created ping (invalid if creation failed)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	FGuid CreatePing(FVector Location, EMGPingType PingType = EMGPingType::Location);

	/**
	 * @brief Create a directional ping (e.g., for indicating a path)
	 * @param Location World position to place the ping
	 * @param Direction Direction the ping should point
	 * @param PingType Classification of the ping
	 * @return GUID of the created ping
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	FGuid CreateDirectionalPing(FVector Location, FVector Direction, EMGPingType PingType);

	/**
	 * @brief Remove a specific ping by its ID
	 * @param PingID The GUID of the ping to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	void RemovePing(FGuid PingID);

	/** @brief Remove all pings created by the local player */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Ping")
	void RemoveAllMyPings();

	/**
	 * @brief Check if the player can create a new ping
	 * @return True if not on cooldown and under max ping limit
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	bool CanCreatePing() const;

	/**
	 * @brief Get all currently active pings in the world
	 * @return Array of active ping data
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	TArray<FMGWorldPing> GetActivePings() const;

	/**
	 * @brief Get only pings created by the local player
	 * @return Array of the local player's active pings
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	TArray<FMGWorldPing> GetMyPings() const;

	/**
	 * @brief Get remaining cooldown time before next ping can be created
	 * @return Seconds remaining (0 if no cooldown active)
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Ping")
	float GetPingCooldownRemaining() const;

	//-------------------------------------------------------------------------
	// Message Library - Access to available messages
	//-------------------------------------------------------------------------

	/**
	 * @brief Get all messages in a specific category
	 * @param Category The category to filter by
	 * @return Array of messages in that category
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	TArray<FMGQuickChatMessage> GetMessagesByCategory(EMGQuickChatCategory Category) const;

	/** @brief Get all available messages regardless of category or unlock status */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	TArray<FMGQuickChatMessage> GetAllMessages() const;

	/** @brief Get only messages the player has unlocked */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	TArray<FMGQuickChatMessage> GetUnlockedMessages() const;

	/**
	 * @brief Get a specific message by its ID
	 * @param MessageID The unique identifier of the message
	 * @return The message data (empty if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Library")
	FMGQuickChatMessage GetMessage(FName MessageID) const;

	/**
	 * @brief Unlock a message for the player
	 * @param MessageID The message to unlock
	 * @return True if successfully unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Library")
	bool UnlockMessage(FName MessageID);

	//-------------------------------------------------------------------------
	// Chat Wheel - Wheel configuration and management
	//-------------------------------------------------------------------------

	/** @brief Get the currently active chat wheel configuration */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Wheel")
	FMGQuickChatWheel GetActiveWheel() const { return ActiveWheel; }

	/**
	 * @brief Switch to a different chat wheel
	 * @param WheelID The ID of the wheel to activate
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void SetActiveWheel(FName WheelID);

	/**
	 * @brief Assign a message to a wheel slot
	 * @param MessageID The message to assign
	 * @param SlotIndex The slot index (0-based)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void AssignMessageToSlot(FName MessageID, int32 SlotIndex);

	/**
	 * @brief Clear a wheel slot (remove assigned message)
	 * @param SlotIndex The slot to clear
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void ClearSlot(int32 SlotIndex);

	/**
	 * @brief Get the message assigned to a specific slot
	 * @param SlotIndex The slot to query
	 * @return The message at that slot (empty if slot is empty)
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Wheel")
	FMGQuickChatMessage GetMessageAtSlot(int32 SlotIndex) const;

	/** @brief Save the current wheel configuration to persistent storage */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void SaveWheelConfiguration();

	/** @brief Load wheel configuration from persistent storage */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Wheel")
	void LoadWheelConfiguration();

	//-------------------------------------------------------------------------
	// Chat History - Message history tracking
	//-------------------------------------------------------------------------

	/**
	 * @brief Get recent chat history
	 * @param MaxEntries Maximum number of entries to return
	 * @return Array of recent chat events (newest first)
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|History")
	TArray<FMGChatEvent> GetChatHistory(int32 MaxEntries = 50) const;

	/** @brief Clear all chat history */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|History")
	void ClearChatHistory();

	//-------------------------------------------------------------------------
	// Configuration - System settings
	//-------------------------------------------------------------------------

	/**
	 * @brief Apply new configuration settings
	 * @param NewConfig The new configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void SetConfig(const FMGQuickChatConfig& NewConfig);

	/** @brief Get current configuration settings */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Config")
	FMGQuickChatConfig GetConfig() const { return Config; }

	/**
	 * @brief Mute a specific player
	 * @param PlayerID The player to mute
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void MutePlayer(FName PlayerID);

	/**
	 * @brief Unmute a specific player
	 * @param PlayerID The player to unmute
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void UnmutePlayer(FName PlayerID);

	/**
	 * @brief Check if a player is muted
	 * @param PlayerID The player to check
	 * @return True if the player is muted
	 */
	UFUNCTION(BlueprintPure, Category = "QuickChat|Config")
	bool IsPlayerMuted(FName PlayerID) const;

	/**
	 * @brief Enable or disable voice line playback
	 * @param bEnabled True to enable voice lines
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Config")
	void SetVoiceLinesEnabled(bool bEnabled);

	//-------------------------------------------------------------------------
	// Player Info - Local player context
	//-------------------------------------------------------------------------

	/**
	 * @brief Set the local player's identification info
	 * @param PlayerID Unique player identifier
	 * @param PlayerName Display name
	 * @param TeamID Team identifier (-1 for no team)
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Player")
	void SetLocalPlayerInfo(FName PlayerID, const FString& PlayerName, int32 TeamID);

	/**
	 * @brief Update the local player's world location (for proximity checks)
	 * @param Location Current world position
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Player")
	void SetLocalPlayerLocation(FVector Location);

	//-------------------------------------------------------------------------
	// Network Receive - Handle incoming messages from network
	//-------------------------------------------------------------------------

	/**
	 * @brief Process a received quick chat message from the network
	 * @param ChatEvent The incoming chat event
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Network")
	void ReceiveQuickChat(const FMGChatEvent& ChatEvent);

	/**
	 * @brief Process a received ping from the network
	 * @param Ping The incoming ping data
	 */
	UFUNCTION(BlueprintCallable, Category = "QuickChat|Network")
	void ReceivePing(const FMGWorldPing& Ping);

	//-------------------------------------------------------------------------
	// Delegates - Bindable events
	//-------------------------------------------------------------------------

	/// Fires when a quick chat message is received (from any visible player)
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnQuickChatReceived OnQuickChatReceived;

	/// Fires when a new ping appears in the world
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnPingCreated OnPingCreated;

	/// Fires when a ping is removed or expires
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnPingExpired OnPingExpired;

	/// Fires when message cooldown begins
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnChatCooldownStarted OnChatCooldownStarted;

	/// Fires when message cooldown ends
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnChatCooldownEnded OnChatCooldownEnded;

	/// Fires when a new message is unlocked
	UPROPERTY(BlueprintAssignable, Category = "QuickChat|Events")
	FOnQuickChatUnlocked OnQuickChatUnlocked;

protected:
	//-------------------------------------------------------------------------
	// Internal Implementation
	//-------------------------------------------------------------------------

	/// Called periodically to update pings and cooldowns
	void OnQuickChatTick();

	/// Update ping lifetimes and remove expired pings
	void UpdatePings(float DeltaTime);

	/// Update message and ping cooldown timers
	void UpdateCooldowns(float DeltaTime);

	/// Populate the message library with default messages
	void InitializeDefaultMessages();

	/// Create the default chat wheel configuration
	void InitializeDefaultWheel();

	/// Play the voice line associated with a message
	void PlayVoiceLine(const FMGQuickChatMessage& Message);

	/// Check visibility rules to determine if message should be shown
	bool ShouldReceiveMessage(const FMGChatEvent& ChatEvent) const;

	/// Get the display color for a ping type
	FLinearColor GetPingColor(EMGPingType PingType) const;

	//-------------------------------------------------------------------------
	// Data Members
	//-------------------------------------------------------------------------

	/// All available messages mapped by their ID
	UPROPERTY()
	TMap<FName, FMGQuickChatMessage> MessageLibrary;

	/// All chat wheel configurations mapped by wheel ID
	UPROPERTY()
	TMap<FName, FMGQuickChatWheel> Wheels;

	/// The currently active chat wheel
	UPROPERTY()
	FMGQuickChatWheel ActiveWheel;

	/// All currently active world pings
	UPROPERTY()
	TArray<FMGWorldPing> ActivePings;

	/// Recent chat message history
	UPROPERTY()
	TArray<FMGChatEvent> ChatHistory;

	/// Current system configuration
	UPROPERTY()
	FMGQuickChatConfig Config;

	/// Local player's unique identifier
	UPROPERTY()
	FName LocalPlayerID;

	/// Local player's display name
	UPROPERTY()
	FString LocalPlayerName;

	/// Local player's team (-1 = no team)
	UPROPERTY()
	int32 LocalTeamID = -1;

	/// Local player's current world position
	UPROPERTY()
	FVector LocalPlayerLocation = FVector::ZeroVector;

	/// Remaining cooldown time for messages (seconds)
	UPROPERTY()
	float MessageCooldownRemaining = 0.0f;

	/// Remaining cooldown time for pings (seconds)
	UPROPERTY()
	float PingCooldownRemaining = 0.0f;

	/// Maximum number of chat events to retain in history
	UPROPERTY()
	int32 MaxChatHistory = 100;

	/// Timer handle for the tick function
	FTimerHandle QuickChatTickHandle;
};
