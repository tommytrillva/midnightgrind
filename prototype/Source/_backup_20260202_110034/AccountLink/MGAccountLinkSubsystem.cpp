// Copyright Midnight Grind. All Rights Reserved.

#include "AccountLink/MGAccountLinkSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/Guid.h"

void UMGAccountLinkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentPlatform = DetectCurrentPlatform();
	InitializePlatformLogin();
}

void UMGAccountLinkSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TokenRefreshHandle);
	}

	// Unbind login delegate
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid())
		{
			Identity->ClearOnLoginCompleteDelegate_Handle(0, LoginCompleteDelegateHandle);
		}
	}

	Super::Deinitialize();
}

void UMGAccountLinkSubsystem::LoginWithPlatform(EMGPlatformType Platform)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Error, TEXT("AccountLink: No online subsystem available"));
		OnLoginStateChanged.Broadcast(false);
		return;
	}

	IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AccountLink: No identity interface available"));
		OnLoginStateChanged.Broadcast(false);
		return;
	}

	// Check if already logged in
	if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
	{
		FUniqueNetIdPtr UserId = Identity->GetUniquePlayerId(0);
		if (UserId.IsValid())
		{
			FString DisplayName = Identity->GetPlayerNickname(0);
			CreateUnifiedAccount(Platform, UserId->ToString(), DisplayName);
			bIsLoggedIn = true;
			OnLoginStateChanged.Broadcast(true);
			return;
		}
	}

	// Bind login complete delegate
	LoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(
		0,
		FOnLoginCompleteDelegate::CreateUObject(this, &UMGAccountLinkSubsystem::HandlePlatformLoginComplete)
	);

	// Initiate login
	Identity->AutoLogin(0);
}

void UMGAccountLinkSubsystem::Logout()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
		if (Identity.IsValid())
		{
			Identity->Logout(0);
		}
	}

	bIsLoggedIn = false;
	CurrentAccount = FMGUnifiedAccount();
	CurrentToken = FMGAuthToken();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TokenRefreshHandle);
	}

	OnLoginStateChanged.Broadcast(false);
}

void UMGAccountLinkSubsystem::RefreshAuthToken()
{
	if (CurrentToken.RefreshToken.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AccountLink: No refresh token available"));
		return;
	}

	// In production, this would call backend API to refresh token
	// For now, simulate refresh
	CurrentToken.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(1);
	OnAuthTokenRefreshed.Broadcast(CurrentPlatform);

	StartTokenRefreshTimer();
}

bool UMGAccountLinkSubsystem::IsTokenValid() const
{
	if (CurrentToken.AccessToken.IsEmpty())
		return false;

	return FDateTime::UtcNow() < CurrentToken.ExpiresAt;
}

float UMGAccountLinkSubsystem::GetTokenTimeRemaining() const
{
	if (!IsTokenValid())
		return 0.0f;

	FTimespan Remaining = CurrentToken.ExpiresAt - FDateTime::UtcNow();
	return static_cast<float>(Remaining.GetTotalSeconds());
}

void UMGAccountLinkSubsystem::LinkAccount(EMGPlatformType Platform)
{
	if (Platform == CurrentPlatform)
	{
		UE_LOG(LogTemp, Warning, TEXT("AccountLink: Cannot link current platform"));
		return;
	}

	if (IsAccountLinked(Platform))
	{
		UE_LOG(LogTemp, Warning, TEXT("AccountLink: Platform already linked"));
		return;
	}

	// In production, this would initiate OAuth flow for the target platform
	// Then call backend to associate the platform account

	FMGLinkedAccount NewLink;
	NewLink.Platform = Platform;
	NewLink.Status = EMGLinkStatus::Pending;
	CurrentAccount.LinkedAccounts.Add(NewLink);

	// Simulate successful link for development
	for (FMGLinkedAccount& Account : CurrentAccount.LinkedAccounts)
	{
		if (Account.Platform == Platform)
		{
			Account.Status = EMGLinkStatus::Linked;
			Account.LinkedAt = FDateTime::UtcNow();
			Account.PlatformUserID = FGuid::NewGuid().ToString();
			Account.PlatformDisplayName = FString::Printf(TEXT("%s_User"), *GetPlatformName(Platform));
			break;
		}
	}

	SaveAccountToBackend();
	OnAccountLinked.Broadcast(Platform, true);
}

void UMGAccountLinkSubsystem::UnlinkAccount(EMGPlatformType Platform)
{
	if (Platform == CurrentAccount.PrimaryPlatform)
	{
		UE_LOG(LogTemp, Error, TEXT("AccountLink: Cannot unlink primary platform"));
		return;
	}

	CurrentAccount.LinkedAccounts.RemoveAll([Platform](const FMGLinkedAccount& Account)
	{
		return Account.Platform == Platform;
	});

	SaveAccountToBackend();
	OnAccountUnlinked.Broadcast(Platform);
}

bool UMGAccountLinkSubsystem::IsAccountLinked(EMGPlatformType Platform) const
{
	for (const FMGLinkedAccount& Account : CurrentAccount.LinkedAccounts)
	{
		if (Account.Platform == Platform && Account.Status == EMGLinkStatus::Linked)
		{
			return true;
		}
	}
	return false;
}

TArray<FMGLinkedAccount> UMGAccountLinkSubsystem::GetLinkedAccounts() const
{
	TArray<FMGLinkedAccount> Linked;
	for (const FMGLinkedAccount& Account : CurrentAccount.LinkedAccounts)
	{
		if (Account.Status == EMGLinkStatus::Linked)
		{
			Linked.Add(Account);
		}
	}
	return Linked;
}

FMGLinkedAccount UMGAccountLinkSubsystem::GetLinkedAccount(EMGPlatformType Platform) const
{
	for (const FMGLinkedAccount& Account : CurrentAccount.LinkedAccounts)
	{
		if (Account.Platform == Platform)
		{
			return Account;
		}
	}
	return FMGLinkedAccount();
}

void UMGAccountLinkSubsystem::SetPrimaryAccount(EMGPlatformType Platform)
{
	if (!IsAccountLinked(Platform) && Platform != CurrentPlatform)
	{
		UE_LOG(LogTemp, Warning, TEXT("AccountLink: Cannot set unlinked platform as primary"));
		return;
	}

	// Clear existing primary
	for (FMGLinkedAccount& Account : CurrentAccount.LinkedAccounts)
	{
		Account.bIsPrimary = (Account.Platform == Platform);
	}

	CurrentAccount.PrimaryPlatform = Platform;
	SaveAccountToBackend();
}

void UMGAccountLinkSubsystem::StartAccountMerge(const FString& SecondaryAccountCode)
{
	if (SecondaryAccountCode.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AccountLink: Invalid merge code"));
		return;
	}

	// In production, this would:
	// 1. Validate the merge code with backend
	// 2. Fetch the secondary account data
	// 3. Detect conflicts
	// 4. Return merge preview

	PendingMerge = FMGAccountMergeResult();
	PendingMerge.bSuccess = false;

	// Simulate detecting conflicts
	FMGMergeConflict CurrencyConflict;
	CurrencyConflict.ConflictID = FGuid::NewGuid().ToString();
	CurrencyConflict.FieldName = TEXT("GrindCash");
	CurrencyConflict.PrimaryValue = TEXT("50000");
	CurrencyConflict.SecondaryValue = TEXT("75000");
	CurrencyConflict.Resolution = EMGMergeConflictResolution::AskUser;
	PendingMerge.Conflicts.Add(CurrencyConflict);

	FMGMergeConflict VehicleConflict;
	VehicleConflict.ConflictID = FGuid::NewGuid().ToString();
	VehicleConflict.FieldName = TEXT("OwnedVehicles");
	VehicleConflict.PrimaryValue = TEXT("12 vehicles");
	VehicleConflict.SecondaryValue = TEXT("8 vehicles");
	VehicleConflict.Resolution = EMGMergeConflictResolution::MergeSum;
	VehicleConflict.bResolved = true;
	PendingMerge.Conflicts.Add(VehicleConflict);

	OnMergeConflictsDetected.Broadcast(PendingMerge.Conflicts);
}

void UMGAccountLinkSubsystem::ResolveConflict(const FString& ConflictID, EMGMergeConflictResolution Resolution)
{
	for (FMGMergeConflict& Conflict : PendingMerge.Conflicts)
	{
		if (Conflict.ConflictID == ConflictID)
		{
			Conflict.Resolution = Resolution;
			Conflict.bResolved = true;
			break;
		}
	}
}

void UMGAccountLinkSubsystem::ConfirmMerge()
{
	// Check all conflicts resolved
	for (const FMGMergeConflict& Conflict : PendingMerge.Conflicts)
	{
		if (!Conflict.bResolved)
		{
			UE_LOG(LogTemp, Warning, TEXT("AccountLink: Unresolved conflicts remain"));
			return;
		}
	}

	// In production, this would send merge request to backend
	// Backend would apply resolutions and merge data

	PendingMerge.bSuccess = true;
	PendingMerge.ResultMessage = TEXT("Accounts merged successfully");
	PendingMerge.Conflicts.Empty();

	SaveAccountToBackend();
	OnAccountMergeComplete.Broadcast();
}

void UMGAccountLinkSubsystem::CancelMerge()
{
	PendingMerge = FMGAccountMergeResult();
}

FString UMGAccountLinkSubsystem::GenerateLinkCode()
{
	// Generate a short code for account linking
	// In production, this would be registered with backend and have expiry

	FString Code;
	const FString Chars = TEXT("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");

	for (int32 i = 0; i < 8; i++)
	{
		int32 Index = FMath::RandRange(0, Chars.Len() - 1);
		Code.AppendChar(Chars[Index]);

		if (i == 3)
		{
			Code.AppendChar('-');
		}
	}

	return Code; // Format: XXXX-XXXX
}

void UMGAccountLinkSubsystem::SetCrossplayEnabled(bool bEnabled)
{
	CurrentAccount.bCrossplayEnabled = bEnabled;
	SaveAccountToBackend();
}

void UMGAccountLinkSubsystem::SetDisplayName(const FString& NewName)
{
	if (NewName.Len() < 3 || NewName.Len() > 20)
	{
		UE_LOG(LogTemp, Warning, TEXT("AccountLink: Display name must be 3-20 characters"));
		return;
	}

	CurrentAccount.DisplayName = NewName;
	SaveAccountToBackend();
}

void UMGAccountLinkSubsystem::SyncDisplayNameFromPlatform(EMGPlatformType Platform)
{
	for (const FMGLinkedAccount& Account : CurrentAccount.LinkedAccounts)
	{
		if (Account.Platform == Platform && !Account.PlatformDisplayName.IsEmpty())
		{
			CurrentAccount.DisplayName = Account.PlatformDisplayName;
			SaveAccountToBackend();
			return;
		}
	}

	// If syncing from current platform
	if (Platform == CurrentPlatform)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				FString PlatformName = Identity->GetPlayerNickname(0);
				if (!PlatformName.IsEmpty())
				{
					CurrentAccount.DisplayName = PlatformName;
					SaveAccountToBackend();
				}
			}
		}
	}
}

void UMGAccountLinkSubsystem::InitializePlatformLogin()
{
	// Attempt auto-login on startup
	LoginWithPlatform(CurrentPlatform);
}

void UMGAccountLinkSubsystem::HandlePlatformLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				FString DisplayName = Identity->GetPlayerNickname(LocalUserNum);
				CreateUnifiedAccount(CurrentPlatform, UserId.ToString(), DisplayName);
				bIsLoggedIn = true;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AccountLink: Platform login failed - %s"), *Error);
		bIsLoggedIn = false;
	}

	OnLoginStateChanged.Broadcast(bIsLoggedIn);
}

void UMGAccountLinkSubsystem::CreateUnifiedAccount(EMGPlatformType Platform, const FString& PlatformUserID, const FString& PlatformDisplayName)
{
	// In production, this would:
	// 1. Check if unified account exists for this platform user
	// 2. If exists, load it
	// 3. If not, create new unified account

	CurrentAccount.UnifiedID = FGuid::NewGuid().ToString();
	CurrentAccount.DisplayName = PlatformDisplayName;
	CurrentAccount.CreatedAt = FDateTime::UtcNow();
	CurrentAccount.LastLogin = FDateTime::UtcNow();
	CurrentAccount.PrimaryPlatform = Platform;
	CurrentAccount.bCrossplayEnabled = true;

	FMGLinkedAccount PrimaryAccount;
	PrimaryAccount.Platform = Platform;
	PrimaryAccount.PlatformUserID = PlatformUserID;
	PrimaryAccount.PlatformDisplayName = PlatformDisplayName;
	PrimaryAccount.Status = EMGLinkStatus::Linked;
	PrimaryAccount.LinkedAt = FDateTime::UtcNow();
	PrimaryAccount.LastUsed = FDateTime::UtcNow();
	PrimaryAccount.bIsPrimary = true;
	CurrentAccount.LinkedAccounts.Add(PrimaryAccount);

	// Generate auth token
	CurrentToken.AccessToken = FGuid::NewGuid().ToString();
	CurrentToken.RefreshToken = FGuid::NewGuid().ToString();
	CurrentToken.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(1);
	CurrentToken.Platform = Platform;

	StartTokenRefreshTimer();
	SaveAccountToBackend();
}

void UMGAccountLinkSubsystem::LoadAccountFromBackend(const FString& UnifiedID)
{
	// In production, this would fetch account data from game backend
}

void UMGAccountLinkSubsystem::SaveAccountToBackend()
{
	// In production, this would sync account data to game backend
	CurrentAccount.LastLogin = FDateTime::UtcNow();
}

void UMGAccountLinkSubsystem::StartTokenRefreshTimer()
{
	if (UWorld* World = GetWorld())
	{
		float RefreshTime = GetTokenTimeRemaining() - TokenRefreshBuffer;
		if (RefreshTime > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				TokenRefreshHandle,
				this,
				&UMGAccountLinkSubsystem::RefreshAuthToken,
				RefreshTime,
				false
			);
		}
	}
}

FString UMGAccountLinkSubsystem::GetPlatformName(EMGPlatformType Platform) const
{
	switch (Platform)
	{
	case EMGPlatformType::Steam: return TEXT("Steam");
	case EMGPlatformType::Epic: return TEXT("Epic");
	case EMGPlatformType::PlayStation: return TEXT("PlayStation");
	case EMGPlatformType::Xbox: return TEXT("Xbox");
	case EMGPlatformType::Nintendo: return TEXT("Nintendo");
	case EMGPlatformType::Mobile_iOS: return TEXT("iOS");
	case EMGPlatformType::Mobile_Android: return TEXT("Android");
	default: return TEXT("Unknown");
	}
}

EMGPlatformType UMGAccountLinkSubsystem::DetectCurrentPlatform() const
{
#if PLATFORM_WINDOWS
	// Check for Steam or Epic
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		FString SubsystemName = OnlineSub->GetSubsystemName().ToString();
		if (SubsystemName == TEXT("Steam"))
			return EMGPlatformType::Steam;
		if (SubsystemName == TEXT("EOS") || SubsystemName == TEXT("Epic"))
			return EMGPlatformType::Epic;
	}
	return EMGPlatformType::Steam; // Default to Steam on PC
#elif PLATFORM_PS5
	return EMGPlatformType::PlayStation;
#elif PLATFORM_XBOXONE || PLATFORM_XSX
	return EMGPlatformType::Xbox;
#elif PLATFORM_SWITCH
	return EMGPlatformType::Nintendo;
#elif PLATFORM_IOS
	return EMGPlatformType::Mobile_iOS;
#elif PLATFORM_ANDROID
	return EMGPlatformType::Mobile_Android;
#else
	return EMGPlatformType::Unknown;
#endif
}
