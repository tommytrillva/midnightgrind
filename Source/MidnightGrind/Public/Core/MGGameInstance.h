// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGGameInstance.h - The Root of the Game
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * The Game Instance is the ROOT MANAGER for Midnight Grind. It's the first
 * game-specific class created when the game starts, and it lives until the
 * game exits. Think of it as the "brain" that coordinates everything.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. GAME INSTANCE (UGameInstance):
 *    - THE top-level game class - there's only ONE for the entire game
 *    - Created before anything else, destroyed last
 *    - Survives level transitions (unlike World, Actors, etc.)
 *    - Where you put "global" game logic and state
 *
 *    LIFECYCLE:
 *    Game Launch -> Init() -> StartGameInstance() -> OnStart() -> [Game Running] -> Shutdown()
 *
 * 2. SUBSYSTEM PATTERN:
 *    Instead of putting everything in GameInstance, we use Subsystems:
 *    - Each subsystem handles one area (save/load, sessions, input, etc.)
 *    - GameInstance creates and owns all subsystems
 *    - Access via: GameInstance->GetSubsystem<UMySubsystem>()
 *
 *    This keeps code organized and maintainable.
 *
 * 3. PLATFORM SERVICES:
 *    The game can run on different platforms (Steam, Epic, PlayStation, etc.)
 *    GameInstance detects the platform and initializes the right services.
 *
 *    Steam provides: friends list, achievements, matchmaking, cloud saves
 *    Epic provides: similar features via Epic Online Services
 *    Consoles: PSN, Xbox Live, Nintendo Online
 *
 * 4. NETWORK STATE:
 *    Tracks whether we're connected to online services:
 *    - Offline: No online features
 *    - Connecting: Establishing connection
 *    - Online: Full online features available
 *    - Reconnecting: Lost connection, trying to restore
 *    - Maintenance: Servers are down
 *
 * 5. PLAYER PROFILE:
 *    The local player's identity (name, level, crew) loaded from
 *    platform services and/or our backend.
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *   [UMGGameInstance] <-- This file (THE ROOT)
 *         |
 *         +-- Platform Detection (Steam, Epic, Console)
 *         |
 *         +-- Network State (Online, Offline, Reconnecting)
 *         |
 *         +-- Player Profile (Name, Level, Crew)
 *         |
 *         +-- Subsystems (auto-created, accessed via GetSubsystem<T>)
 *                |
 *                +-- [UMGGameStateSubsystem] - Game flow
 *                +-- [UMGSaveSubsystem] - Save/Load
 *                +-- [UMGSessionSubsystem] - Multiplayer
 *                +-- [UMGCurrencySubsystem] - In-game money
 *                +-- [UMGProgressionSubsystem] - XP/Levels
 *                +-- [UMGVehicleManagerSubsystem] - Garage
 *                +-- [UMGInputRemapSubsystem] - Controls
 *                +-- [UMGAccessibilitySubsystem] - Accessibility
 *                +-- [UMGCloudSaveSubsystem] - Cloud sync
 *                +-- [UMGAnalyticsSubsystem] - Telemetry
 *                +-- [UMGAntiCheatSubsystem] - Anti-cheat
 *                +-- [UMGAudioMixSubsystem] - Audio
 *                +-- [UMGLocalizationSubsystem] - Languages
 *
 * HOW TO ACCESS THE GAME INSTANCE:
 * --------------------------------
 * From almost anywhere in game code, you can get the GameInstance:
 *
 * @code
 * // From any Actor
 * UMGGameInstance* GI = Cast<UMGGameInstance>(GetGameInstance());
 *
 * // From a UObject with access to World
 * UMGGameInstance* GI = Cast<UMGGameInstance>(GetWorld()->GetGameInstance());
 *
 * // Then access subsystems:
 * UMGSaveSubsystem* Save = GI->GetSubsystem<UMGSaveSubsystem>();
 *
 * // Or use convenience getters:
 * UMGSessionSubsystem* Sessions = GI->GetSessionSubsystem();
 * @endcode
 *
 * INITIALIZATION ORDER:
 * ---------------------
 * 1. Init() - Very early, engine not fully ready
 * 2. DetectPlatform() - Figure out where we're running
 * 3. InitializeSteam() - Set up Steam API if on Steam
 * 4. InitializeSubsystems() - Create all game subsystems
 * 5. LoadPlayerProfile() - Get player identity
 * 6. OnSubsystemsReady broadcast - Safe to use everything now
 *
 * @see UMGGameStateSubsystem For game flow control
 * @see UMGSessionSubsystem For multiplayer session management
 * @see UMGSaveSubsystem For save/load functionality
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MGGameInstance.generated.h"

// Forward declarations for subsystems managed by the game instance
class UMGGameStateSubsystem;      ///< Game flow state machine
class UMGSessionSubsystem;        ///< Multiplayer session management
class UMGAccountLinkSubsystem;    ///< Cross-platform account linking
class UMGProgressionSubsystem;    ///< Player XP, level, and unlocks
class UMGCurrencySubsystem;       ///< In-game currency (credits, premium)
class UMGVehicleManagerSubsystem; ///< Vehicle collection and garage
class UMGInputRemapSubsystem;     ///< Controller/keyboard remapping
class UMGAccessibilitySubsystem;  ///< Accessibility options
class UMGCloudSaveSubsystem;      ///< Cloud save synchronization
class UMGAnalyticsSubsystem;      ///< Telemetry and analytics
class UMGAntiCheatSubsystem;      ///< Anti-cheat validation
class UMGServerAuthSubsystem;     ///< Server authentication
class UMGAudioMixSubsystem;       ///< Audio mixing and volume
class UMGLocalizationSubsystem;   ///< Language and localization

// ============================================================================
// ENUMS - Network and Platform States
// ============================================================================

/**
 * @brief Network connection state for online features
 *
 * Tracks the current connectivity status with backend services.
 * Used to determine if online features (multiplayer, leaderboards) are available.
 */
UENUM(BlueprintType)
enum class EMGNetworkState : uint8
{
	/** Not connected to any network services */
	Offline,
	/** Currently establishing connection to backend */
	Connecting,
	/** Successfully connected and authenticated - online features available */
	Online,
	/** Lost connection, automatically attempting to reconnect */
	Reconnecting,
	/** Backend is in maintenance mode - check again later */
	Maintenance
};

// EMGPlatform - REMOVED (duplicate)
// Canonical definition in: Certification/MGCertificationSubsystem.h

// ============================================================================
// STRUCTS - Player Data
// ============================================================================

// FMGPlayerProfile - REMOVED (duplicate)
// Canonical definition in: ProfileManager/MGProfileManagerSubsystem.h
// This file now uses FMGLocalPlayerInfo for local player identity data.

/**
 * @brief Local player identity information
 *
 * Contains the authenticated player's basic identity and stats for UI display.
 * This is a lightweight struct for local player info; for full player data,
 * see FMGPlayerProfile in ProfileManager/MGProfileManagerSubsystem.h.
 *
 * Populated during login from platform services and backend.
 * Used throughout the game for displaying local player info.
 */
USTRUCT(BlueprintType)
struct FMGLocalPlayerInfo
{
	GENERATED_BODY()

	/// Unique backend identifier for this player (not platform-specific)
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/// Name shown to other players
	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	/// URL to player's avatar image (from platform or custom)
	UPROPERTY(BlueprintReadOnly)
	FString AvatarURL;

	/// Player's current level (1-100)
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/// Total experience points earned
	UPROPERTY(BlueprintReadOnly)
	int64 TotalXP = 0;

	/// Name of the player's crew/club (empty if not in one)
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/// Short crew tag shown in races (e.g., "[MG]")
	UPROPERTY(BlueprintReadOnly)
	FString CrewTag;

	/// Which platform this profile is from
	UPROPERTY(BlueprintReadOnly)
	EMGPlatform Platform = EMGPlatform::Unknown;

	/// Whether the player is currently online
	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline = false;
};

// ============================================================================
// DELEGATES - Event Signatures
// ============================================================================

/// Broadcast when network connectivity state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkStateChanged, EMGNetworkState, NewState);

/// Broadcast when player profile is loaded and ready to use
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerProfileReady, const FMGLocalPlayerInfo&, Profile);

/// Broadcast when login attempt completes (success or failure)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginComplete, bool, bSuccess);

/// Broadcast when player logs out
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLogout);

/// Broadcast when all subsystems finish initialization
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubsystemsReady, bool, bSuccess);

// ============================================================================
// CLASS - UMGGameInstance
// ============================================================================

/**
 * @brief Game Instance - Central manager for game-wide state and subsystems
 *
 * The root class that initializes platform services, manages subsystems,
 * tracks player profile and network state, and coordinates save/load operations.
 *
 * @section Lifetime
 * Created when the game starts, destroyed when the game exits.
 * Persists across all level transitions.
 *
 * @section Features
 * - Platform detection and initialization (Steam, Epic, consoles)
 * - Subsystem creation and orchestration
 * - Player profile management (login, profile data)
 * - Network state tracking with auto-reconnect
 * - Coordinated save/load across all subsystems
 */
UCLASS()
class MIDNIGHTGRIND_API UMGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMGGameInstance();

	/// Called very early during engine initialization
	virtual void Init() override;

	/// Called when game is shutting down - cleanup
	virtual void Shutdown() override;

	/// Called to start the game instance
	virtual void StartGameInstance() override;

	/// Called when game instance is ready to start gameplay
	virtual void OnStart() override;

	// ==========================================
	// EVENTS
	// Bind to these to react to game-wide state changes.
	// Essential for UI systems that need to respond to connectivity/login.
	// ==========================================

	/**
	 * @brief Fired when network state changes
	 * Use to show/hide online-only features
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNetworkStateChanged OnNetworkStateChanged;

	/**
	 * @brief Fired when player profile is loaded
	 * Safe to access GetPlayerProfile() after this fires
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerProfileReady OnPlayerProfileReady;

	/**
	 * @brief Fired when login attempt completes
	 * Check bSuccess to determine if login succeeded
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoginComplete OnLoginComplete;

	/**
	 * @brief Fired when player logs out
	 * Clear any cached profile data when this fires
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLogout OnLogout;

	/**
	 * @brief Fired when all subsystems finish initialization
	 * Safe to access all subsystems after this fires
	 */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSubsystemsReady OnSubsystemsReady;

	// ==========================================
	// INITIALIZATION
	// Query startup progress for loading screens.
	// ==========================================

	/**
	 * @brief Check if all subsystems have finished initializing
	 * @return True when all subsystems are ready to use
	 * Wait for this before accessing subsystem functionality
	 */
	UFUNCTION(BlueprintPure, Category = "Init")
	bool AreSubsystemsReady() const { return bSubsystemsReady; }

	/**
	 * @brief Get overall initialization progress
	 * @return Value from 0.0 (starting) to 1.0 (complete)
	 * Useful for boot/splash screen progress bars
	 */
	UFUNCTION(BlueprintPure, Category = "Init")
	float GetInitProgress() const { return InitProgress; }

	/**
	 * @brief Force reinitialize all subsystems
	 * Use for error recovery - triggers full re-init sequence
	 */
	UFUNCTION(BlueprintCallable, Category = "Init")
	void ReinitializeSubsystems();

	// ==========================================
	// PLATFORM/STEAM
	// Query platform-specific information.
	// ==========================================

	/**
	 * @brief Get which platform we're running on
	 * @return The detected platform type
	 */
	UFUNCTION(BlueprintPure, Category = "Platform")
	EMGPlatform GetPlatform() const { return CurrentPlatform; }

	/**
	 * @brief Check if Steam API is available and initialized
	 * @return True if Steam features can be used
	 */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsSteamAvailable() const;

	/**
	 * @brief Get the player's Steam ID (if on Steam)
	 * @return Steam64 ID as string, or empty if not on Steam
	 */
	UFUNCTION(BlueprintPure, Category = "Platform")
	FString GetSteamID() const;

	/**
	 * @brief Get the player's Steam persona name
	 * @return Steam display name, or empty if not on Steam
	 */
	UFUNCTION(BlueprintPure, Category = "Platform")
	FString GetSteamDisplayName() const;

	/**
	 * @brief Check if game was launched through Steam client
	 * @return True if Steam overlay and API are available
	 */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsRunningSteam() const;

	// ==========================================
	// NETWORK STATE
	// Query and control network connectivity.
	// ==========================================

	/**
	 * @brief Get current network connection state
	 * @return The current connectivity status
	 */
	UFUNCTION(BlueprintPure, Category = "Network")
	EMGNetworkState GetNetworkState() const { return NetworkState; }

	/**
	 * @brief Check if fully connected to online services
	 * @return True if online features (multiplayer, leaderboards) work
	 */
	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsOnline() const { return NetworkState == EMGNetworkState::Online; }

	/**
	 * @brief Check if currently attempting to connect
	 * @return True during initial connect or reconnect attempts
	 */
	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsConnecting() const { return NetworkState == EMGNetworkState::Connecting || NetworkState == EMGNetworkState::Reconnecting; }

	/**
	 * @brief Attempt to establish connection to online services
	 * Results delivered via OnNetworkStateChanged
	 */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void GoOnline();

	/**
	 * @brief Disconnect from online services
	 * Use when player wants offline mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void GoOffline();

	// ==========================================
	// PLAYER PROFILE
	// Query the local player's identity and stats.
	// ==========================================

	/**
	 * @brief Get the local player's identity info
	 * @return Local player info struct with basic identity data
	 * Wait for OnPlayerProfileReady before calling
	 */
	UFUNCTION(BlueprintPure, Category = "Profile")
	FMGLocalPlayerInfo GetPlayerProfile() const { return LocalPlayerProfile; }

	/**
	 * @brief Check if player is logged in
	 * @return True if authenticated with backend
	 */
	UFUNCTION(BlueprintPure, Category = "Profile")
	bool IsLoggedIn() const { return bIsLoggedIn; }

	/**
	 * @brief Get unique player ID
	 * @return Backend player ID (not platform-specific)
	 */
	UFUNCTION(BlueprintPure, Category = "Profile")
	FString GetPlayerID() const { return LocalPlayerProfile.PlayerID; }

	/**
	 * @brief Get player's display name
	 * @return Name shown in-game to other players
	 */
	UFUNCTION(BlueprintPure, Category = "Profile")
	FString GetDisplayName() const { return LocalPlayerProfile.DisplayName; }

	// ==========================================
	// SUBSYSTEM ACCESS (Convenience)
	// Shortcut getters for commonly-used subsystems.
	// Can also use GetSubsystem<T>() for any subsystem type.
	// ==========================================

	/**
	 * @brief Get the game state subsystem
	 * @return Game flow state machine subsystem
	 * @see UMGGameStateSubsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGGameStateSubsystem* GetGameStateSubsystem() const;

	/**
	 * @brief Get the session subsystem
	 * @return Multiplayer session management subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGSessionSubsystem* GetSessionSubsystem() const;

	/**
	 * @brief Get the account link subsystem
	 * @return Cross-platform account linking subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGAccountLinkSubsystem* GetAccountLinkSubsystem() const;

	/**
	 * @brief Get the input remap subsystem
	 * @return Controller/keyboard remapping subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGInputRemapSubsystem* GetInputRemapSubsystem() const;

	/**
	 * @brief Get the accessibility subsystem
	 * @return Accessibility features subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGAccessibilitySubsystem* GetAccessibilitySubsystem() const;

	/**
	 * @brief Get the cloud save subsystem
	 * @return Cloud save synchronization subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGCloudSaveSubsystem* GetCloudSaveSubsystem() const;

	// ==========================================
	// GAME FLOW
	// High-level game navigation from main menu.
	// ==========================================

	/**
	 * @brief Start a new game
	 * Creates fresh save data and transitions to garage
	 */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void StartNewGame();

	/**
	 * @brief Continue from existing save
	 * Loads save data and transitions to garage
	 */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void ContinueGame();

	/**
	 * @brief Return to main menu from anywhere
	 * Saves progress and resets to main menu state
	 */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void ReturnToMainMenu();

	/**
	 * @brief Quit the game
	 * Saves progress and exits the application
	 */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void QuitGame();

	// ==========================================
	// SAVE/LOAD
	// Coordinate saving/loading across all subsystems.
	// ==========================================

	/**
	 * @brief Save all game data to disk (and cloud if enabled)
	 * Coordinates save across all subsystems that have saveable data
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveAll();

	/**
	 * @brief Load all game data from disk (or cloud)
	 * Coordinates load across all subsystems
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadAll();

	/**
	 * @brief Check if save data exists
	 * @return True if there's existing save data to continue from
	 * Use to determine if "Continue" button should be enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSaveData() const;

	/**
	 * @brief Get when the game was last saved
	 * @return DateTime of most recent save
	 * Display this in save/load UI
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	FDateTime GetLastSaveTime() const { return LastSaveTime; }

protected:
	// ==========================================
	// STATE
	// Internal state - do not modify directly from outside.
	// ==========================================

	/// Detected platform (set during Init)
	EMGPlatform CurrentPlatform = EMGPlatform::Unknown;

	/// Current network connectivity state
	EMGNetworkState NetworkState = EMGNetworkState::Offline;

	/// Cached local player identity data
	FMGLocalPlayerInfo LocalPlayerProfile;

	/// True if player has successfully authenticated
	bool bIsLoggedIn = false;

	/// True when all subsystems have finished initialization
	bool bSubsystemsReady = false;

	/// Initialization progress (0.0 to 1.0) for loading UI
	float InitProgress = 0.0f;

	/// True if Steam API initialized successfully
	bool bSteamInitialized = false;

	/// When game was last saved (for display in UI)
	FDateTime LastSaveTime;

	// ==========================================
	// INITIALIZATION
	// Internal initialization sequence functions.
	// ==========================================

	/// Detect which platform we're running on
	void DetectPlatform();

	/// Initialize Steam API if available
	void InitializeSteam();

	/// Create and initialize all game subsystems
	void InitializeSubsystems();

	/// Load player profile from platform services
	void LoadPlayerProfile();

	/// Callback when Steam authentication completes
	void OnSteamLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	// ==========================================
	// NETWORK
	// Internal network state management.
	// ==========================================

	/// Update network state and broadcast change event
	void SetNetworkState(EMGNetworkState NewState);

	/// Callback when platform connection status changes
	void OnConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

	/// Callback when network error occurs during gameplay
	void OnNetworkError(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	/// Callback when level travel fails
	void OnTravelError(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	/// Timer for scheduling reconnection attempts
	FTimerHandle ReconnectTimerHandle;

	/// Attempt to reconnect to online services
	void AttemptReconnect();

	/// How many times we've tried to reconnect (resets on success)
	int32 ReconnectAttempt = 0;

	/// Maximum reconnection attempts before giving up
	static constexpr int32 MaxReconnectAttempts = 5;
};
