// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MGGameInstance.generated.h"

class UMGGameStateSubsystem;
class UMGSessionSubsystem;
class UMGAccountLinkSubsystem;
class UMGProgressionSubsystem;
class UMGCurrencySubsystem;
class UMGVehicleManagerSubsystem;
class UMGInputRemapSubsystem;
class UMGAccessibilitySubsystem;
class UMGCloudSaveSubsystem;
class UMGAnalyticsSubsystem;
class UMGAntiCheatSubsystem;
class UMGServerAuthSubsystem;
class UMGAudioMixSubsystem;
class UMGLocalizationSubsystem;

/**
 * Network connection state
 */
UENUM(BlueprintType)
enum class EMGNetworkState : uint8
{
	/** Offline */
	Offline,
	/** Connecting to backend */
	Connecting,
	/** Connected and authenticated */
	Online,
	/** Connection lost, attempting reconnect */
	Reconnecting,
	/** Maintenance mode */
	Maintenance
};

/**
 * Platform type
 */
UENUM(BlueprintType)
enum class EMGPlatform : uint8
{
	Unknown,
	Steam,
	Epic,
	PlayStation,
	Xbox,
	Switch
};

/**
 * Player profile data
 */
USTRUCT(BlueprintType)
struct FMGPlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	UPROPERTY(BlueprintReadOnly)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly)
	FString AvatarURL;

	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly)
	int64 TotalXP = 0;

	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	UPROPERTY(BlueprintReadOnly)
	FString CrewTag;

	UPROPERTY(BlueprintReadOnly)
	EMGPlatform Platform = EMGPlatform::Unknown;

	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNetworkStateChanged, EMGNetworkState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerProfileReady, const FMGPlayerProfile&, Profile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLogout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubsystemsReady, bool, bSuccess);

/**
 * Game Instance
 * Central manager for game-wide state and subsystem initialization
 *
 * Features:
 * - Steam/Platform initialization
 * - Subsystem orchestration
 * - Player profile management
 * - Network state tracking
 * - Save/Load coordination
 */
UCLASS()
class MIDNIGHTGRIND_API UMGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMGGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;
	virtual void OnStart() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNetworkStateChanged OnNetworkStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerProfileReady OnPlayerProfileReady;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLogout OnLogout;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSubsystemsReady OnSubsystemsReady;

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Check if all subsystems are initialized */
	UFUNCTION(BlueprintPure, Category = "Init")
	bool AreSubsystemsReady() const { return bSubsystemsReady; }

	/** Get initialization progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Init")
	float GetInitProgress() const { return InitProgress; }

	/** Force reinitialize subsystems */
	UFUNCTION(BlueprintCallable, Category = "Init")
	void ReinitializeSubsystems();

	// ==========================================
	// PLATFORM/STEAM
	// ==========================================

	/** Get current platform */
	UFUNCTION(BlueprintPure, Category = "Platform")
	EMGPlatform GetPlatform() const { return CurrentPlatform; }

	/** Is Steam available */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsSteamAvailable() const;

	/** Get Steam ID */
	UFUNCTION(BlueprintPure, Category = "Platform")
	FString GetSteamID() const;

	/** Get Steam display name */
	UFUNCTION(BlueprintPure, Category = "Platform")
	FString GetSteamDisplayName() const;

	/** Check if running through Steam */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsRunningSteam() const;

	// ==========================================
	// NETWORK STATE
	// ==========================================

	/** Get network state */
	UFUNCTION(BlueprintPure, Category = "Network")
	EMGNetworkState GetNetworkState() const { return NetworkState; }

	/** Is online */
	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsOnline() const { return NetworkState == EMGNetworkState::Online; }

	/** Is connecting */
	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsConnecting() const { return NetworkState == EMGNetworkState::Connecting || NetworkState == EMGNetworkState::Reconnecting; }

	/** Attempt to go online */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void GoOnline();

	/** Go offline */
	UFUNCTION(BlueprintCallable, Category = "Network")
	void GoOffline();

	// ==========================================
	// PLAYER PROFILE
	// ==========================================

	/** Get local player profile */
	UFUNCTION(BlueprintPure, Category = "Profile")
	FMGPlayerProfile GetPlayerProfile() const { return LocalPlayerProfile; }

	/** Is logged in */
	UFUNCTION(BlueprintPure, Category = "Profile")
	bool IsLoggedIn() const { return bIsLoggedIn; }

	/** Get player ID */
	UFUNCTION(BlueprintPure, Category = "Profile")
	FString GetPlayerID() const { return LocalPlayerProfile.PlayerID; }

	/** Get display name */
	UFUNCTION(BlueprintPure, Category = "Profile")
	FString GetDisplayName() const { return LocalPlayerProfile.DisplayName; }

	// ==========================================
	// SUBSYSTEM ACCESS (Convenience)
	// ==========================================

	/** Get game state subsystem */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGGameStateSubsystem* GetGameStateSubsystem() const;

	/** Get session subsystem */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGSessionSubsystem* GetSessionSubsystem() const;

	/** Get account link subsystem */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGAccountLinkSubsystem* GetAccountLinkSubsystem() const;

	/** Get input remap subsystem */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGInputRemapSubsystem* GetInputRemapSubsystem() const;

	/** Get accessibility subsystem */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGAccessibilitySubsystem* GetAccessibilitySubsystem() const;

	/** Get cloud save subsystem */
	UFUNCTION(BlueprintPure, Category = "Subsystems")
	UMGCloudSaveSubsystem* GetCloudSaveSubsystem() const;

	// ==========================================
	// GAME FLOW
	// ==========================================

	/** Start new game (go to garage) */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void StartNewGame();

	/** Continue from save */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void ContinueGame();

	/** Return to main menu */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void ReturnToMainMenu();

	/** Quit game */
	UFUNCTION(BlueprintCallable, Category = "Flow")
	void QuitGame();

	// ==========================================
	// SAVE/LOAD
	// ==========================================

	/** Save all game data */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveAll();

	/** Load all game data */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadAll();

	/** Has save data */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSaveData() const;

	/** Get last save time */
	UFUNCTION(BlueprintPure, Category = "Save")
	FDateTime GetLastSaveTime() const { return LastSaveTime; }

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Current platform */
	EMGPlatform CurrentPlatform = EMGPlatform::Unknown;

	/** Network state */
	EMGNetworkState NetworkState = EMGNetworkState::Offline;

	/** Local player profile */
	FMGPlayerProfile LocalPlayerProfile;

	/** Is logged in */
	bool bIsLoggedIn = false;

	/** Subsystems ready */
	bool bSubsystemsReady = false;

	/** Init progress */
	float InitProgress = 0.0f;

	/** Steam initialized */
	bool bSteamInitialized = false;

	/** Last save time */
	FDateTime LastSaveTime;

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Detect platform */
	void DetectPlatform();

	/** Initialize Steam */
	void InitializeSteam();

	/** Initialize subsystems */
	void InitializeSubsystems();

	/** Load player profile from platform */
	void LoadPlayerProfile();

	/** Handle Steam login result */
	void OnSteamLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	// ==========================================
	// NETWORK
	// ==========================================

	/** Set network state */
	void SetNetworkState(EMGNetworkState NewState);

	/** Handle connection status change */
	void OnConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);

	/** Handle network error */
	void OnNetworkError(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	/** Handle travel error */
	void OnTravelError(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);

	/** Timer handle for auto-reconnect */
	FTimerHandle ReconnectTimerHandle;

	/** Reconnect attempt */
	void AttemptReconnect();

	/** Current reconnect attempt */
	int32 ReconnectAttempt = 0;

	/** Max reconnect attempts */
	static constexpr int32 MaxReconnectAttempts = 5;
};
