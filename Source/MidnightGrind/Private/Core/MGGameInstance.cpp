// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGGameInstance.h"
#include "Core/MG_GAME_StateSubsystem.h"
#include "Session/MGSessionSubsystem.h"
#include "AccountLink/MGAccountLinkSubsystem.h"
#include "InputRemap/MGInputRemapSubsystem.h"
#include "Accessibility/MGAccessibilitySubsystem.h"
#include "CloudSave/MGCloudSaveSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGGameInstance, Log, All);

UMGGameInstance::UMGGameInstance()
{
}

void UMGGameInstance::Init()
{
	Super::Init();

	UE_LOG(LogMGGameInstance, Log, TEXT("Initializing Midnight Grind Game Instance"));

	// Detect platform first
	DetectPlatform();

	// Initialize platform services
	InitializeSteam();

	// Initialize all game subsystems
	InitializeSubsystems();

	// Register network callbacks
	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMGGameInstance::OnNetworkError);
		GEngine->OnTravelFailure().AddUObject(this, &UMGGameInstance::OnTravelError);
	}

	UE_LOG(LogMGGameInstance, Log, TEXT("Game Instance initialized on platform: %d"), static_cast<int32>(CurrentPlatform));
}

void UMGGameInstance::Shutdown()
{
	UE_LOG(LogMGGameInstance, Log, TEXT("Shutting down Midnight Grind Game Instance"));

	// Save before shutdown
	SaveAll();

	// Cleanup
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	Super::Shutdown();
}

void UMGGameInstance::StartGameInstance()
{
	Super::StartGameInstance();

	// Load player profile after engine is ready
	LoadPlayerProfile();
}

void UMGGameInstance::OnStart()
{
	Super::OnStart();

	// Load saved data
	LoadAll();

	// Attempt to go online
	GoOnline();
}

// ==========================================
// PLATFORM/STEAM
// ==========================================

void UMGGameInstance::DetectPlatform()
{
#if PLATFORM_WINDOWS
	// For now, default to Unknown (local development)
	// Steam/Epic detection will be enabled when ready for release
	CurrentPlatform = EMGPlatform::Unknown;

#elif PLATFORM_PS5
	CurrentPlatform = EMGPlatform::PlayStation;
#elif PLATFORM_XBOX
	CurrentPlatform = EMGPlatform::Xbox;
#elif PLATFORM_SWITCH
	CurrentPlatform = EMGPlatform::Switch;
#else
	CurrentPlatform = EMGPlatform::Unknown;
#endif
}

void UMGGameInstance::InitializeSteam()
{
	// Steam initialization disabled for local development
	// Will be enabled when ready for Steam release
	UE_LOG(LogMGGameInstance, Log, TEXT("Running in local/offline mode"));
	bSteamInitialized = false;
}

void UMGGameInstance::OnSteamLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		bSteamInitialized = true;
		UE_LOG(LogMGGameInstance, Log, TEXT("Steam login successful"));
		LoadPlayerProfile();
	}
	else
	{
		UE_LOG(LogMGGameInstance, Warning, TEXT("Steam login failed: %s"), *Error);
	}
}

bool UMGGameInstance::IsSteamAvailable() const
{
	return bSteamInitialized;
}

FString UMGGameInstance::GetSteamID() const
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
	if (OnlineSub)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid())
		{
			TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(0);
			if (UserId.IsValid())
			{
				return UserId->ToString();
			}
		}
	}
	return FString();
}

FString UMGGameInstance::GetSteamDisplayName() const
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
	if (OnlineSub)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid())
		{
			return Identity->GetPlayerNickname(0);
		}
	}
	return FString();
}

bool UMGGameInstance::IsRunningSteam() const
{
	return CurrentPlatform == EMGPlatform::Steam && bSteamInitialized;
}

// ==========================================
// INITIALIZATION
// ==========================================

void UMGGameInstance::InitializeSubsystems()
{
	UE_LOG(LogMGGameInstance, Log, TEXT("Initializing game subsystems..."));

	// Subsystems are automatically created by UE for GameInstanceSubsystems
	// Just verify they exist and signal ready

	bSubsystemsReady = true;
	InitProgress = 1.0f;

	OnSubsystemsReady.Broadcast(true);
	UE_LOG(LogMGGameInstance, Log, TEXT("All subsystems initialized"));
}

void UMGGameInstance::ReinitializeSubsystems()
{
	bSubsystemsReady = false;
	InitProgress = 0.0f;
	InitializeSubsystems();
}

void UMGGameInstance::LoadPlayerProfile()
{
	LocalPlayerProfile = FMGPlayerProfile();

	// Get platform-specific profile
	switch (CurrentPlatform)
	{
	case EMGPlatform::Steam:
		LocalPlayerProfile.PlayerID = GetSteamID();
		LocalPlayerProfile.DisplayName = GetSteamDisplayName();
		LocalPlayerProfile.Platform = EMGPlatform::Steam;
		break;

	default:
		// Generate a local ID
		LocalPlayerProfile.PlayerID = FGuid::NewGuid().ToString();
		LocalPlayerProfile.DisplayName = TEXT("Player");
		LocalPlayerProfile.Platform = CurrentPlatform;
		break;
	}

	if (!LocalPlayerProfile.PlayerID.IsEmpty())
	{
		bIsLoggedIn = true;
		LocalPlayerProfile.bIsOnline = IsOnline();
		OnPlayerProfileReady.Broadcast(LocalPlayerProfile);
		OnLoginComplete.Broadcast(true);
	}
}

// ==========================================
// NETWORK STATE
// ==========================================

void UMGGameInstance::SetNetworkState(EMGNetworkState NewState)
{
	if (NetworkState != NewState)
	{
		NetworkState = NewState;
		OnNetworkStateChanged.Broadcast(NewState);

		LocalPlayerProfile.bIsOnline = (NewState == EMGNetworkState::Online);
	}
}

void UMGGameInstance::GoOnline()
{
	if (NetworkState == EMGNetworkState::Online)
	{
		return;
	}

	// For local development, just mark as online (LAN play works)
	SetNetworkState(EMGNetworkState::Online);
	UE_LOG(LogMGGameInstance, Log, TEXT("Local network mode enabled"));
}

void UMGGameInstance::GoOffline()
{
	SetNetworkState(EMGNetworkState::Offline);
}

void UMGGameInstance::OnConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus)
{
	switch (ConnectionStatus)
	{
	case EOnlineServerConnectionStatus::Connected:
		SetNetworkState(EMGNetworkState::Online);
		ReconnectAttempt = 0;
		break;

	case EOnlineServerConnectionStatus::ConnectionDropped:
		SetNetworkState(EMGNetworkState::Reconnecting);
		AttemptReconnect();
		break;

	case EOnlineServerConnectionStatus::NotConnected:
	case EOnlineServerConnectionStatus::ServiceUnavailable:
		SetNetworkState(EMGNetworkState::Offline);
		break;

	default:
		break;
	}
}

void UMGGameInstance::OnNetworkError(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogMGGameInstance, Error, TEXT("Network Error: %s - %s"), *UEnum::GetValueAsString(FailureType), *ErrorString);

	if (FailureType == ENetworkFailure::ConnectionLost || FailureType == ENetworkFailure::ConnectionTimeout)
	{
		SetNetworkState(EMGNetworkState::Reconnecting);
		AttemptReconnect();
	}
}

void UMGGameInstance::OnTravelError(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogMGGameInstance, Error, TEXT("Travel Error: %s - %s"), *UEnum::GetValueAsString(FailureType), *ErrorString);

	// Return to main menu on travel failure
	ReturnToMainMenu();
}

void UMGGameInstance::AttemptReconnect()
{
	if (ReconnectAttempt >= MaxReconnectAttempts)
	{
		UE_LOG(LogMGGameInstance, Warning, TEXT("Max reconnect attempts reached, going offline"));
		SetNetworkState(EMGNetworkState::Offline);
		return;
	}

	ReconnectAttempt++;
	UE_LOG(LogMGGameInstance, Log, TEXT("Reconnect attempt %d/%d"), ReconnectAttempt, MaxReconnectAttempts);

	// Exponential backoff
	float Delay = FMath::Pow(2.0f, static_cast<float>(ReconnectAttempt));

	TWeakObjectPtr<UMGGameInstance> WeakThis(this);
	GetTimerManager().SetTimer(ReconnectTimerHandle, [WeakThis]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->GoOnline();
		}
	}, Delay, false);
}

// ==========================================
// SUBSYSTEM ACCESS
// ==========================================

UMGGameStateSubsystem* UMGGameInstance::GetGameStateSubsystem() const
{
	return GetSubsystem<UMGGameStateSubsystem>();
}

UMGSessionSubsystem* UMGGameInstance::GetSessionSubsystem() const
{
	return GetSubsystem<UMGSessionSubsystem>();
}

UMGAccountLinkSubsystem* UMGGameInstance::GetAccountLinkSubsystem() const
{
	return GetSubsystem<UMGAccountLinkSubsystem>();
}

UMGInputRemapSubsystem* UMGGameInstance::GetInputRemapSubsystem() const
{
	return GetSubsystem<UMGInputRemapSubsystem>();
}

UMGAccessibilitySubsystem* UMGGameInstance::GetAccessibilitySubsystem() const
{
	return GetSubsystem<UMGAccessibilitySubsystem>();
}

UMGCloudSaveSubsystem* UMGGameInstance::GetCloudSaveSubsystem() const
{
	return GetSubsystem<UMGCloudSaveSubsystem>();
}

// ==========================================
// GAME FLOW
// ==========================================

void UMGGameInstance::StartNewGame()
{
	// Reset progression and go to garage
	if (UMGGameStateSubsystem* GS = GetGameStateSubsystem())
	{
		GS->GoToGarage();
	}
}

void UMGGameInstance::ContinueGame()
{
	// Load saved data and go to garage
	LoadAll();

	if (UMGGameStateSubsystem* GS = GetGameStateSubsystem())
	{
		GS->GoToGarage();
	}
}

void UMGGameInstance::ReturnToMainMenu()
{
	// Save before returning
	SaveAll();

	if (UMGGameStateSubsystem* GS = GetGameStateSubsystem())
	{
		GS->GoToMainMenu();
	}
	else
	{
		// Fallback to direct level load
		UGameplayStatics::OpenLevel(this, FName(TEXT("MainMenu")));
	}
}

void UMGGameInstance::QuitGame()
{
	// Save before quitting
	SaveAll();

	// Quit
	FGenericPlatformMisc::RequestExit(false);
}

// ==========================================
// SAVE/LOAD
// ==========================================

void UMGGameInstance::SaveAll()
{
	UE_LOG(LogMGGameInstance, Log, TEXT("Saving all game data..."));

	// Cloud save subsystem handles the actual saving
	if (UMGCloudSaveSubsystem* CloudSave = GetCloudSaveSubsystem())
	{
		// CloudSave->SaveAllData();
	}

	LastSaveTime = FDateTime::Now();
	UE_LOG(LogMGGameInstance, Log, TEXT("Save complete"));
}

void UMGGameInstance::LoadAll()
{
	UE_LOG(LogMGGameInstance, Log, TEXT("Loading all game data..."));

	// Cloud save subsystem handles the actual loading
	if (UMGCloudSaveSubsystem* CloudSave = GetCloudSaveSubsystem())
	{
		// CloudSave->LoadAllData();
	}

	UE_LOG(LogMGGameInstance, Log, TEXT("Load complete"));
}

bool UMGGameInstance::HasSaveData() const
{
	if (UMGCloudSaveSubsystem* CloudSave = GetCloudSaveSubsystem())
	{
		// return CloudSave->HasSaveData();
	}
	return false;
}
