// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGLobbyWidget.h - Multiplayer Lobby UI Widgets
 * ============================================================================
 *
 * PURPOSE:
 * This file defines the user interface widgets for multiplayer lobbies in
 * Midnight Grind. It contains three main widget classes that handle different
 * aspects of the multiplayer UI experience.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. UMG WIDGETS (Unreal Motion Graphics):
 *    - UUserWidget is the base class for all UI elements in Unreal
 *    - Widgets can be designed in UMG Blueprint Editor and extended in C++
 *    - "Abstract" class means you must create a Blueprint child class to use it
 *
 * 2. BLUEPRINT INTEGRATION:
 *    - BlueprintImplementableEvent: Override this function in Blueprint
 *    - BlueprintNativeEvent: Has C++ default, can be overridden in Blueprint
 *    - BlueprintCallable: Can be called from Blueprint graphs
 *    - BlueprintPure: Can be used in Blueprint without execution pins
 *
 * 3. MULTIPLAYER LOBBY FLOW:
 *    Player opens multiplayer menu -> Session Browser or Quick Match
 *    -> Joins/Creates Session -> Enters Lobby -> Selects Vehicle
 *    -> Sets Ready -> Host Starts Race -> All players load track
 *
 * ARCHITECTURE:
 * - UMGPlayerSlotWidget: Displays a single player's info in the lobby
 * - UMGLobbyWidget: Main lobby screen with player list and race settings
 * - UMGSessionBrowserWidget: Find and join existing game sessions
 * - UMGMatchmakingWidget: Quick match and ranked queue interface
 *
 * USAGE EXAMPLE:
 * 1. Create a Blueprint extending WBP_LobbyWidget from UMGLobbyWidget
 * 2. Design the visual layout in UMG Editor
 * 3. Bind UI elements to the exposed properties
 * 4. Override BlueprintImplementableEvent functions for custom behavior
 *
 * RELATED FILES:
 * - MGMultiplayerSubsystem.h: Backend logic for multiplayer features
 * - MGSessionManager.h: Session creation and joining logic
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Multiplayer/MGMultiplayerSubsystem.h"
#include "MGLobbyWidget.generated.h"

// Forward declarations - these classes are defined elsewhere
// Forward declarations reduce compilation dependencies and speed up builds
class UMGMultiplayerSubsystem;
class UVerticalBox;      // UMG container that stacks children vertically
class UButton;           // Clickable button widget
class UTextBlock;        // Text display widget
class UImage;            // Image display widget
class UComboBoxString;   // Dropdown selection widget

/**
 * UMGPlayerSlotWidget
 *
 * Represents a single player's slot in the lobby player list.
 * Each slot shows player info like name, level, vehicle, and ready status.
 *
 * DESIGN PATTERN:
 * This is an "Abstract" class - you cannot use it directly. Instead:
 * 1. Create a Blueprint class that inherits from this
 * 2. Design the visual layout in the Blueprint
 * 3. The C++ code handles data while Blueprint handles visuals
 *
 * USAGE:
 * The parent UMGLobbyWidget creates instances of this widget for each
 * player slot and calls UpdatePlayerData() when player info changes.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGPlayerSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Updates the slot with new player data.
	 * Called when a player joins, changes vehicle, or updates ready status.
	 *
	 * BlueprintNativeEvent means:
	 * - There's a default C++ implementation (in the .cpp file)
	 * - Blueprint can override by implementing "UpdatePlayerData" event
	 *
	 * @param PlayerData - Structure containing all player information
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PlayerSlot")
	void UpdatePlayerData(const FMGNetPlayer& PlayerData);

	/**
	 * Clears the slot to show it's empty/available.
	 * Called when a player leaves or for unfilled slots.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PlayerSlot")
	void SetEmpty();

	/**
	 * Marks this slot as the local player's slot.
	 * Local player slots often have different visual styling (highlighted border, etc.)
	 *
	 * @param bIsLocal - True if this slot represents the local player
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PlayerSlot")
	void SetLocalPlayer(bool bIsLocal);

protected:
	// ==========================================
	// STATE VARIABLES
	// These are readable in Blueprint for UI binding
	// ==========================================

	/**
	 * Cached player data for this slot.
	 * Use this in Blueprint to bind UI elements to player properties.
	 * Example: Bind a TextBlock's text to CurrentPlayerData.DisplayName
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerSlot")
	FMGNetPlayer CurrentPlayerData;

	/**
	 * Whether this slot shows the local player.
	 * Use this to apply special styling to your own slot.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerSlot")
	bool bIsLocalPlayer = false;

	/**
	 * Whether this slot is currently empty (no player).
	 * Empty slots might show "Waiting for player..." or similar.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PlayerSlot")
	bool bIsEmpty = true;
};

/**
 * UMGLobbyWidget
 *
 * The main lobby interface widget. This is the primary screen players see
 * while waiting for a race to start. It displays:
 * - List of all players in the lobby
 * - Current track and race settings
 * - Vehicle selection
 * - Ready button
 * - Host controls (if player is host)
 *
 * LIFECYCLE:
 * 1. Widget is created when entering a lobby
 * 2. NativeConstruct() is called - binds to multiplayer events
 * 3. InitializeLobby() sets up initial state
 * 4. Events update the UI as players join/leave/ready up
 * 5. NativeDestruct() cleans up when leaving lobby
 *
 * EVENT-DRIVEN DESIGN:
 * This widget subscribes to events from UMGMultiplayerSubsystem.
 * When something happens (player joins, settings change), the subsystem
 * broadcasts an event, and this widget updates its display.
 *
 * HOST VS CLIENT:
 * - Host can change track, lap count, kick players, start race
 * - Clients can only change their vehicle and ready status
 * - Use IsHost() to show/hide host-only controls
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Called when the widget is added to the viewport.
	 * This is where we bind to multiplayer events.
	 *
	 * IMPORTANT: Always call Super::NativeConstruct() in child classes!
	 */
	virtual void NativeConstruct() override;

	/**
	 * Called when the widget is removed from the viewport.
	 * This is where we unbind from events to prevent crashes.
	 */
	virtual void NativeDestruct() override;

	// ==========================================
	// PUBLIC INTERFACE
	// These functions can be called from Blueprint
	// ==========================================

	/**
	 * Sets up the lobby widget with initial data.
	 * Call this after creating the widget to populate it with current state.
	 *
	 * This will:
	 * - Get reference to the multiplayer subsystem
	 * - Create player slot widgets
	 * - Update with current lobby state
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InitializeLobby();

	/**
	 * Refreshes the player list display.
	 * Called automatically when players join/leave, but can be called
	 * manually if needed (e.g., after UI becomes visible again).
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdatePlayerList();

	/**
	 * Updates the display of lobby settings (track, laps, etc.).
	 * Called when host changes settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void UpdateLobbySettings();

	/**
	 * Sets the local player's ready status.
	 * When all players are ready, the host can start the race.
	 *
	 * @param bReady - True to mark as ready, false to unready
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bReady);

	/**
	 * Changes the local player's selected vehicle.
	 *
	 * @param VehicleID - The FName identifier of the vehicle to select
	 *                    (e.g., "Nissan_Skyline_R34")
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SelectVehicle(FName VehicleID);

	/**
	 * Leaves the current lobby and returns to the main menu.
	 * This properly disconnects from the session.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void LeaveLobby();

	/**
	 * Starts the race (host only).
	 * Will fail if not all players are ready or if called by non-host.
	 * Triggers the countdown sequence.
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartRace();

	// ==========================================
	// HOST CONTROLS
	// These functions only work if the local player is the host
	// ==========================================

	/**
	 * Changes the selected track (host only).
	 * All players will see the track update.
	 *
	 * @param TrackID - The FName identifier of the track (e.g., "Track_Downtown")
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Host")
	void ChangeTrack(FName TrackID);

	/**
	 * Changes the number of laps for the race (host only).
	 *
	 * @param Laps - Number of laps (typically 1-10)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Host")
	void ChangeLapCount(int32 Laps);

	/**
	 * Removes a player from the lobby (host only).
	 * The kicked player will be returned to the main menu.
	 *
	 * @param PlayerID - The unique ID of the player to kick
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Host")
	void KickPlayer(const FString& PlayerID);

	// ==========================================
	// QUERY FUNCTIONS
	// BlueprintPure functions can be used in Blueprint without
	// execution pins - they just return values
	// ==========================================

	/**
	 * Gets the currently selected track.
	 *
	 * @return The FName of the selected track
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	FName GetSelectedTrack() const;

	/**
	 * Checks if the race can be started.
	 * Requirements: Player is host, all players ready, minimum player count met.
	 *
	 * @return True if StartRace() would succeed
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool CanStartRace() const;

	/**
	 * Checks if the local player is the session host.
	 * Use this to show/hide host-only controls.
	 *
	 * @return True if local player is host
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsHost() const;

	/**
	 * Gets the invite code for this session.
	 * Friends can use this code to join the lobby directly.
	 *
	 * @return A short alphanumeric code (e.g., "ABC123")
	 */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	FString GetInviteCode() const;

protected:
	// ==========================================
	// BLUEPRINT EVENTS
	// Override these in Blueprint to respond to lobby events
	// These are called automatically when things happen
	// ==========================================

	/**
	 * Called when a new player joins the lobby.
	 * Use this to play a sound effect or show a notification.
	 *
	 * @param Player - Data about the player who joined
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnPlayerJoinedLobby(const FMGNetPlayer& Player);

	/**
	 * Called when a player leaves the lobby.
	 *
	 * @param PlayerID - The unique ID of the player who left
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnPlayerLeftLobby(const FString& PlayerID);

	/**
	 * Called when any player's ready status changes.
	 *
	 * @param PlayerID - The player whose status changed
	 * @param bReady - Their new ready status
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnPlayerReadyChanged(const FString& PlayerID, bool bReady);

	/**
	 * Called when lobby settings change (track, laps, etc.).
	 *
	 * @param Settings - The new lobby settings
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnSettingsChanged(const FMGLobbySettings& Settings);

	/**
	 * Called when the race start sequence begins.
	 * Use this to show a "Get Ready!" message.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnRaceStarting();

	/**
	 * Called every frame during the pre-race countdown.
	 * Use this to display the countdown timer (3... 2... 1... GO!)
	 *
	 * @param TimeRemaining - Seconds until race starts
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby|Events")
	void OnCountdownUpdate(float TimeRemaining);

	// ==========================================
	// CONFIGURATION
	// Set these in the Blueprint defaults panel
	// ==========================================

	/**
	 * The widget class to use for player slots.
	 * Must be a Blueprint class inheriting from UMGPlayerSlotWidget.
	 * Set this in your lobby widget Blueprint's defaults.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<UMGPlayerSlotWidget> PlayerSlotClass;

	/**
	 * Maximum number of player slots to display.
	 * This should match or exceed the max players allowed in a session.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	int32 MaxDisplaySlots = 8;

	// ==========================================
	// STATE
	// Runtime state variables
	// ==========================================

	/**
	 * Reference to the multiplayer subsystem.
	 * This is the "backend" that handles all the networking logic.
	 * We call functions on this to perform actions and listen to its events.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	UMGMultiplayerSubsystem* MultiplayerSubsystem;

	/**
	 * Array of player slot widgets we've created.
	 * These are reused as players join/leave.
	 */
	UPROPERTY()
	TArray<UMGPlayerSlotWidget*> PlayerSlotWidgets;

	/**
	 * Cached copy of current lobby settings.
	 * Updated when settings change, used for UI binding.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FMGLobbySettings CurrentSettings;

	// ==========================================
	// INTERNAL FUNCTIONS
	// These handle the event binding and processing
	// ==========================================

	/** Subscribes to events from the multiplayer subsystem. */
	void BindEvents();

	/** Unsubscribes from events (important for cleanup!). */
	void UnbindEvents();

	// Event handlers - these receive events from the multiplayer subsystem
	// and forward them to the Blueprint events above

	UFUNCTION()
	void HandlePlayerJoined(const FMGNetPlayer& Player);

	UFUNCTION()
	void HandlePlayerLeft(const FString& PlayerID);

	UFUNCTION()
	void HandlePlayerReady(const FString& PlayerID);

	UFUNCTION()
	void HandleSettingsChanged(const FMGLobbySettings& Settings);

	UFUNCTION()
	void HandleRaceStarting();
};

/**
 * UMGSessionBrowserWidget
 *
 * Widget for finding and joining existing multiplayer sessions.
 * Shows a list of available games that the player can join.
 *
 * TYPICAL FLOW:
 * 1. Player opens session browser
 * 2. RefreshSessions() queries the backend for available games
 * 3. OnSessionsUpdated() is called with results
 * 4. Player selects a session from the list
 * 5. JoinSelectedSession() attempts to join
 * 6. Success -> Player enters lobby, or OnJoinFailed() is called
 *
 * FILTERING:
 * Players can filter sessions by track using SetTrackFilter().
 * Additional filters (player count, ping, etc.) can be added.
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGSessionBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Queries the backend for available sessions.
	 * Results are returned via OnSessionsUpdated().
	 * This is an async operation - don't expect immediate results.
	 */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void RefreshSessions();

	/**
	 * Sets a filter to only show sessions on a specific track.
	 *
	 * @param TrackID - Track to filter by, or NAME_None for all tracks
	 */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void SetTrackFilter(FName TrackID);

	/**
	 * Attempts to join the currently selected session.
	 * Make sure SelectedSession is set before calling this.
	 */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void JoinSelectedSession();

	/**
	 * Joins a session using an invite code.
	 * Invite codes are short alphanumeric strings that friends share.
	 *
	 * @param InviteCode - The code to join with (e.g., "ABC123")
	 */
	UFUNCTION(BlueprintCallable, Category = "SessionBrowser")
	void JoinByCode(const FString& InviteCode);

protected:
	/**
	 * Called when the session list is updated.
	 * Implement this in Blueprint to populate your session list UI.
	 *
	 * @param Sessions - Array of available sessions
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SessionBrowser")
	void OnSessionsUpdated(const TArray<FMGSessionInfo>& Sessions);

	/**
	 * Called when joining a session fails.
	 * Show an error message to the player.
	 *
	 * @param Reason - Human-readable failure reason
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SessionBrowser")
	void OnJoinFailed(const FString& Reason);

	/**
	 * The session that the player has selected in the list.
	 * Set this when the player clicks on a session entry.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "SessionBrowser")
	FMGSessionInfo SelectedSession;

	/**
	 * Current track filter, if any.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "SessionBrowser")
	FName CurrentTrackFilter;
};

/**
 * UMGMatchmakingWidget
 *
 * Widget for automatic matchmaking (Quick Match and Ranked).
 * Unlike the session browser, this automatically finds and joins
 * a suitable game based on player skill and preferences.
 *
 * MATCHMAKING FLOW:
 * 1. Player presses "Quick Match" or "Ranked"
 * 2. Widget shows "Searching for players..." with a timer
 * 3. OnMatchmakingProgress() updates as players are found
 * 4. When enough players found, OnMatchFound() is called
 * 5. All matched players are placed in a lobby together
 *
 * QUICK MATCH VS RANKED:
 * - Quick Match: Casual play, loose skill matching
 * - Ranked: Competitive play, strict skill matching, affects rank
 */
UCLASS(Abstract, Blueprintable)
class MIDNIGHTGRIND_API UMGMatchmakingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Starts searching for a casual Quick Match game.
	 * The system will find players of similar skill level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartQuickMatch();

	/**
	 * Starts searching for a Ranked competitive match.
	 * Results affect the player's competitive rank.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void StartRankedMatch();

	/**
	 * Cancels the current matchmaking search.
	 * Player returns to the multiplayer menu.
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

protected:
	/**
	 * Called periodically during matchmaking with progress updates.
	 * Use this to update the UI with search status.
	 *
	 * @param PlayersFound - Number of compatible players found so far
	 * @param PlayersNeeded - Total players needed to start (e.g., 8)
	 * @param TimeElapsed - Seconds spent searching
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
	void OnMatchmakingProgress(int32 PlayersFound, int32 PlayersNeeded, float TimeElapsed);

	/**
	 * Called when a match has been found.
	 * The player will automatically be placed in the lobby.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
	void OnMatchFound();

	/**
	 * Called when matchmaking is cancelled (by player or system).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Matchmaking")
	void OnMatchmakingCancelled();

	/**
	 * Whether we're currently searching for a match.
	 * Use this to update button states (disable Start, show Cancel).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Matchmaking")
	bool bIsMatchmaking = false;

	/**
	 * When matchmaking started (used to calculate elapsed time).
	 * This is in game time, not real world time.
	 */
	float MatchmakingStartTime = 0.0f;
};
