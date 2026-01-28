// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGAccountLinkSubsystem.h
 * Cross-Platform Account Linking and Identity Management Subsystem
 * ============================================================================
 *
 * OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file defines the Account Link Subsystem, which is responsible for managing
 * player identity across multiple gaming platforms. In modern games, players often
 * own the game on multiple platforms (e.g., Steam and PlayStation), and this system
 * allows them to link those accounts together into one unified identity.
 *
 * WHAT IS A SUBSYSTEM?
 * --------------------
 * In Unreal Engine, a "Subsystem" is a way to organize game-wide functionality.
 * This particular class inherits from UGameInstanceSubsystem, which means:
 *   - It is automatically created when the game starts
 *   - It persists across level changes (unlike Actors that get destroyed)
 *   - There is exactly one instance per game session
 *   - You access it via: GameInstance->GetSubsystem<UMGAccountLinkSubsystem>()
 *
 * KEY CONCEPTS:
 * -------------
 * 1. UNIFIED ACCOUNT: A single account in our backend that represents the player,
 *    regardless of which platform they log in from.
 *
 * 2. LINKED ACCOUNTS: Platform-specific accounts (Steam, Epic, PSN, etc.) that
 *    are connected to the unified account. A player can have multiple linked
 *    accounts but one unified identity.
 *
 * 3. AUTHENTICATION TOKENS: Secure credentials (like temporary passwords) that
 *    prove the player's identity. These expire and need refreshing.
 *
 * 4. ACCOUNT MERGING: When a player has progress on two separate accounts and
 *    wants to combine them into one. This requires resolving conflicts when
 *    both accounts have different values for the same data.
 *
 * ARCHITECTURE NOTES:
 * -------------------
 * - UENUM: Creates an enumeration that can be used in both C++ and Blueprints
 * - USTRUCT: Creates a data structure usable in C++ and Blueprints
 * - UFUNCTION: Exposes a function to Blueprints and/or the reflection system
 * - UPROPERTY: Exposes a variable to Blueprints and/or serialization
 * - BlueprintCallable: Function can be called from Blueprint graphs
 * - BlueprintPure: Function has no side effects, can be used as a getter
 * - BlueprintAssignable: Delegate/event that Blueprints can bind to
 *
 * SECURITY CONSIDERATIONS:
 * ------------------------
 * This subsystem handles sensitive authentication data. All tokens should be:
 *   - Stored securely (never in plain text save files)
 *   - Transmitted only over encrypted connections (HTTPS)
 *   - Automatically refreshed before expiration
 *   - Cleared on logout
 *
 * USAGE EXAMPLE:
 * --------------
 * @code
 * // Get the subsystem from anywhere in your game
 * UMGAccountLinkSubsystem* AccountLink = GameInstance->GetSubsystem<UMGAccountLinkSubsystem>();
 *
 * // Login with the current platform
 * AccountLink->LoginWithPlatform(EMGPlatformType::Steam);
 *
 * // Listen for when account linking completes
 * AccountLink->OnAccountLinked.AddDynamic(this, &MyClass::HandleAccountLinked);
 *
 * // Check if logged in
 * if (AccountLink->IsLoggedIn())
 * {
 *     FString DisplayName = AccountLink->GetDisplayName();
 * }
 * @endcode
 *
 * RELATED FILES:
 * --------------
 * - MGCrossProgressionSubsystem.h: Syncs save data across platforms
 * - MGCrossPlaySubsystem.h: Enables multiplayer with players on other platforms
 * - MGPlatformIntegrationSubsystem.h: Platform-specific features (achievements, etc.)
 *
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "MGAccountLinkSubsystem.generated.h"

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Supported gaming platforms for account linking
 *
 * Each platform type corresponds to a specific authentication provider
 * and has its own login flow and credential handling.
 */
UENUM(BlueprintType)
enum class EMGPlatformType : uint8
{
	/** Platform not detected or not applicable */
	Unknown,
	/** Valve Steam platform */
	Steam,
	/** Epic Games Store */
	Epic,
	/** Sony PlayStation Network */
	PlayStation,
	/** Microsoft Xbox Live */
	Xbox,
	/** Nintendo Switch Online */
	Nintendo,
	/** Apple iOS mobile */
	Mobile_iOS,
	/** Google Android mobile */
	Mobile_Android,
	/** Custom/internal authentication */
	Custom
};

/**
 * @brief Status of an account link
 *
 * Represents the current state of a link between a platform account
 * and the unified Midnight Grind account.
 */
UENUM(BlueprintType)
enum class EMGLinkStatus : uint8
{
	/** Account is not linked */
	NotLinked,
	/** Link request in progress */
	Pending,
	/** Account successfully linked */
	Linked,
	/** Link failed with an error */
	Error,
	/** Link credentials have expired */
	Expired
};

/**
 * @brief Strategies for resolving data conflicts during account merges
 *
 * When merging two accounts with different data, these strategies
 * determine which value to keep.
 */
UENUM(BlueprintType)
enum class EMGMergeConflictResolution : uint8
{
	/** Keep the value from the primary (older) account */
	KeepPrimary,
	/** Keep the value from the secondary (newer) account */
	KeepSecondary,
	/** Keep the higher of the two values (e.g., levels, scores) */
	MergeHighest,
	/** Sum both values together (e.g., currencies, play time) */
	MergeSum,
	/** Present both options to the user for manual selection */
	AskUser
};

// ============================================================================
// Data Structures - Account Information
// ============================================================================

/**
 * @brief Information about a single linked platform account
 *
 * Stores the connection details and metadata for one platform
 * account linked to the unified profile.
 */
USTRUCT(BlueprintType)
struct FMGLinkedAccount
{
	GENERATED_BODY()

	/** The platform this account is from */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformType Platform = EMGPlatformType::Unknown;

	/** Platform-specific unique user identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlatformUserID;

	/** Display name on this platform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlatformDisplayName;

	/** Current link status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLinkStatus Status = EMGLinkStatus::NotLinked;

	/** When this account was linked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LinkedAt;

	/** When this account was last used for login */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUsed;

	/** Whether this is the primary account for display name/avatar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPrimary = false;

	/** URL to the platform avatar image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AvatarURL;
};

/**
 * @brief Unified player account spanning all linked platforms
 *
 * The central identity record that ties together all platform
 * accounts and stores unified profile information.
 */
USTRUCT(BlueprintType)
struct FMGUnifiedAccount
{
	GENERATED_BODY()

	/** Midnight Grind's internal unique identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnifiedID;

	/** Unified display name shown in-game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	/** All platform accounts linked to this unified account */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLinkedAccount> LinkedAccounts;

	/** When this unified account was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	/** Timestamp of most recent login from any platform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastLogin;

	/** Platform designated as primary for name/avatar */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformType PrimaryPlatform = EMGPlatformType::Unknown;

	/** Whether the email address has been verified */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEmailVerified = false;

	/** Whether this account allows cross-platform play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrossplayEnabled = true;
};

// ============================================================================
// Data Structures - Authentication
// ============================================================================

/**
 * @brief Authentication token for platform or backend access
 *
 * Stores OAuth-style tokens with expiration tracking for
 * secure API communication.
 */
USTRUCT(BlueprintType)
struct FMGAuthToken
{
	GENERATED_BODY()

	/** The bearer token for API authentication */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AccessToken;

	/** Token used to obtain new access tokens when expired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RefreshToken;

	/** When the access token expires */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	/** Platform this token authenticates with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformType Platform = EMGPlatformType::Unknown;

	/** Permission scopes granted by this token */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Scopes;
};

// ============================================================================
// Data Structures - Account Merging
// ============================================================================

/**
 * @brief A single data conflict discovered during account merge
 *
 * Represents one field where two accounts have different values
 * that need resolution before completing the merge.
 */
USTRUCT(BlueprintType)
struct FMGMergeConflict
{
	GENERATED_BODY()

	/** Unique identifier for this conflict */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ConflictID;

	/** Name of the conflicting data field (e.g., "PlayerLevel") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FieldName;

	/** Value from the primary (target) account */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PrimaryValue;

	/** Value from the secondary (source) account */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SecondaryValue;

	/** How this conflict should be or was resolved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMergeConflictResolution Resolution = EMGMergeConflictResolution::AskUser;

	/** Whether a resolution has been selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bResolved = false;
};

/**
 * @brief Result of an account merge operation
 *
 * Contains the outcome of a merge attempt, including any
 * conflicts that need resolution and the resulting merged account.
 */
USTRUCT(BlueprintType)
struct FMGAccountMergeResult
{
	GENERATED_BODY()

	/** Whether the merge completed successfully */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	/** Human-readable result message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultMessage;

	/** List of conflicts requiring resolution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMergeConflict> Conflicts;

	/** The resulting merged account (if successful) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGUnifiedAccount MergedAccount;
};

// ============================================================================
// Delegate Declarations
// ============================================================================

/** Broadcast when an account link attempt completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAccountLinked, EMGPlatformType, Platform, bool, bSuccess);

/** Broadcast when an account is unlinked */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAccountUnlinked, EMGPlatformType, Platform);

/** Broadcast when an authentication token is refreshed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAuthTokenRefreshed, EMGPlatformType, Platform);

/** Broadcast when merge conflicts are detected and need resolution */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMergeConflictsDetected, const TArray<FMGMergeConflict>&, Conflicts);

/** Broadcast when an account merge completes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnAccountMergeComplete);

/** Broadcast when login state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLoginStateChanged, bool, bLoggedIn);

// ============================================================================
// Subsystem Class
// ============================================================================

/**
 * @brief Cross-Platform Account Link Subsystem
 *
 * Manages player identity across multiple gaming platforms, enabling
 * cross-progression and account unification for Midnight Grind.
 *
 * This subsystem handles:
 * - Platform authentication (Steam, Epic, PSN, Xbox, Nintendo, Mobile)
 * - Account linking and unlinking
 * - Account merging with conflict resolution
 * - Token lifecycle management
 * - Cross-play preferences
 *
 * Persists across level transitions as a GameInstance subsystem.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAccountLinkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @brief Called when the subsystem is created. Initializes platform login.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// @brief Called when the subsystem is destroyed. Cleans up tokens and delegates.
	virtual void Deinitialize() override;

	// ========================================================================
	// Authentication
	// ========================================================================

	/**
	 * @brief Initiate login with a specific platform
	 * This triggers the platform's native authentication flow
	 * @param Platform The platform to authenticate with
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Auth")
	void LoginWithPlatform(EMGPlatformType Platform);

	/**
	 * @brief Log out of the current session
	 * Clears tokens but preserves linked account data
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Auth")
	void Logout();

	/**
	 * @brief Check if the player is currently logged in
	 * @return True if authenticated with valid tokens
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	bool IsLoggedIn() const { return bIsLoggedIn; }

	/**
	 * @brief Get the current unified account information
	 * @return The player's unified account data
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	FMGUnifiedAccount GetCurrentAccount() const { return CurrentAccount; }

	/**
	 * @brief Get the platform used for the current session
	 * @return The current login platform
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	EMGPlatformType GetCurrentPlatform() const { return CurrentPlatform; }

	/**
	 * @brief Manually trigger an authentication token refresh
	 * Called automatically before expiration, but can be invoked manually
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Auth")
	void RefreshAuthToken();

	/**
	 * @brief Check if the current authentication token is valid
	 * @return True if token exists and hasn't expired
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	bool IsTokenValid() const;

	/**
	 * @brief Get time remaining until token expires
	 * @return Seconds until token expiration (negative if expired)
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	float GetTokenTimeRemaining() const;

	// ========================================================================
	// Account Linking
	// ========================================================================

	/**
	 * @brief Link a new platform account to the unified account
	 * Opens platform authentication flow to complete linking
	 * @param Platform The platform to link
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Link")
	void LinkAccount(EMGPlatformType Platform);

	/**
	 * @brief Unlink a platform account from the unified account
	 * Cannot unlink the last remaining platform account
	 * @param Platform The platform to unlink
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Link")
	void UnlinkAccount(EMGPlatformType Platform);

	/**
	 * @brief Check if a platform account is currently linked
	 * @param Platform The platform to check
	 * @return True if the platform is linked to the unified account
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Link")
	bool IsAccountLinked(EMGPlatformType Platform) const;

	/**
	 * @brief Get all currently linked platform accounts
	 * @return Array of linked account information
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Link")
	TArray<FMGLinkedAccount> GetLinkedAccounts() const;

	/**
	 * @brief Get information for a specific linked platform
	 * @param Platform The platform to get info for
	 * @return The linked account data (check Status for validity)
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Link")
	FMGLinkedAccount GetLinkedAccount(EMGPlatformType Platform) const;

	/**
	 * @brief Set which platform provides the display name and avatar
	 * @param Platform The platform to use as primary
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Link")
	void SetPrimaryAccount(EMGPlatformType Platform);

	// ========================================================================
	// Account Merging
	// ========================================================================

	/**
	 * @brief Begin merging another account into the current one
	 * Use GenerateLinkCode() on the source account to get the code
	 * @param SecondaryAccountCode The link code from the account to merge
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void StartAccountMerge(const FString& SecondaryAccountCode);

	/**
	 * @brief Resolve a specific merge conflict
	 * @param ConflictID The conflict to resolve
	 * @param Resolution The chosen resolution strategy
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void ResolveConflict(const FString& ConflictID, EMGMergeConflictResolution Resolution);

	/**
	 * @brief Confirm and execute the account merge
	 * All conflicts must be resolved before calling this
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void ConfirmMerge();

	/**
	 * @brief Cancel the current merge operation
	 * No changes will be made to either account
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void CancelMerge();

	/**
	 * @brief Check if there's an active merge operation
	 * @return True if a merge is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Merge")
	bool HasPendingMerge() const { return PendingMerge.Conflicts.Num() > 0; }

	/**
	 * @brief Get all unresolved conflicts from the pending merge
	 * @return Array of merge conflicts
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Merge")
	TArray<FMGMergeConflict> GetPendingConflicts() const { return PendingMerge.Conflicts; }

	/**
	 * @brief Generate a temporary code for account merging
	 * Give this code to the target account to initiate merge
	 * @return A time-limited link code
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	FString GenerateLinkCode();

	// ========================================================================
	// Cross-Play Settings
	// ========================================================================

	/**
	 * @brief Enable or disable cross-platform play
	 * When disabled, matchmaking only pairs with same-platform players
	 * @param bEnabled True to enable cross-play
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|CrossPlay")
	void SetCrossplayEnabled(bool bEnabled);

	/**
	 * @brief Check if cross-platform play is enabled
	 * @return True if cross-play is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|CrossPlay")
	bool IsCrossplayEnabled() const { return CurrentAccount.bCrossplayEnabled; }

	// ========================================================================
	// Profile Management
	// ========================================================================

	/**
	 * @brief Set a custom display name for the unified account
	 * Subject to content moderation rules
	 * @param NewName The new display name to set
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Profile")
	void SetDisplayName(const FString& NewName);

	/**
	 * @brief Get the current unified display name
	 * @return The player's display name
	 */
	UFUNCTION(BlueprintPure, Category = "AccountLink|Profile")
	FString GetDisplayName() const { return CurrentAccount.DisplayName; }

	/**
	 * @brief Sync the display name from a linked platform account
	 * @param Platform The platform to sync the name from
	 */
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Profile")
	void SyncDisplayNameFromPlatform(EMGPlatformType Platform);

	// ========================================================================
	// Events
	// ========================================================================

	/** Broadcast when a platform account is linked or link fails */
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAccountLinked OnAccountLinked;

	/** Broadcast when a platform account is unlinked */
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAccountUnlinked OnAccountUnlinked;

	/** Broadcast when authentication tokens are refreshed */
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAuthTokenRefreshed OnAuthTokenRefreshed;

	/** Broadcast when merge conflicts need user resolution */
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnMergeConflictsDetected OnMergeConflictsDetected;

	/** Broadcast when account merge completes */
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAccountMergeComplete OnAccountMergeComplete;

	/** Broadcast when login/logout state changes */
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnLoginStateChanged OnLoginStateChanged;

protected:
	/// @brief Initialize the platform-specific login system
	void InitializePlatformLogin();

	/// @brief Callback when platform authentication completes
	/// @param LocalUserNum The local user index
	/// @param bWasSuccessful Whether authentication succeeded
	/// @param UserId The platform user ID
	/// @param Error Error message if authentication failed
	void HandlePlatformLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	/// @brief Create a new unified account for a first-time user
	/// @param Platform The platform being used for initial login
	/// @param PlatformUserID The platform's user identifier
	/// @param PlatformDisplayName The user's name on the platform
	void CreateUnifiedAccount(EMGPlatformType Platform, const FString& PlatformUserID, const FString& PlatformDisplayName);

	/// @brief Load unified account data from the backend
	/// @param UnifiedID The account ID to load
	void LoadAccountFromBackend(const FString& UnifiedID);

	/// @brief Save current account state to the backend
	void SaveAccountToBackend();

	/// @brief Start the automatic token refresh timer
	void StartTokenRefreshTimer();

	/// @brief Get the human-readable name for a platform
	/// @param Platform The platform type
	/// @return Platform name string
	FString GetPlatformName(EMGPlatformType Platform) const;

	/// @brief Detect which platform the game is running on
	/// @return The detected platform type
	EMGPlatformType DetectCurrentPlatform() const;

private:
	/** The current user's unified account */
	FMGUnifiedAccount CurrentAccount;

	/** Current session authentication token */
	FMGAuthToken CurrentToken;

	/** Pending account merge operation */
	FMGAccountMergeResult PendingMerge;

	/** Platform used for current session login */
	EMGPlatformType CurrentPlatform = EMGPlatformType::Unknown;

	/** Timer for automatic token refresh */
	FTimerHandle TokenRefreshHandle;

	/** Handle for platform login completion delegate */
	FDelegateHandle LoginCompleteDelegateHandle;

	/** Whether the user is currently logged in */
	bool bIsLoggedIn = false;

	/** Seconds before token expiry to trigger refresh (default 5 minutes) */
	float TokenRefreshBuffer = 300.0f;
};
