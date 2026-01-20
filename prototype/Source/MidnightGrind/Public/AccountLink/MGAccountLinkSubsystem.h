// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "MGAccountLinkSubsystem.generated.h"

/**
 * Account Link System - Cross-Platform Identity
 * - Platform account linking (Steam, Epic, PSN, Xbox, Nintendo)
 * - Unified player identity across platforms
 * - Cross-progression management
 * - Account merging with conflict resolution
 * - Session token management
 */

UENUM(BlueprintType)
enum class EMGPlatformType : uint8
{
	Unknown,
	Steam,
	Epic,
	PlayStation,
	Xbox,
	Nintendo,
	Mobile_iOS,
	Mobile_Android,
	Custom
};

UENUM(BlueprintType)
enum class EMGLinkStatus : uint8
{
	NotLinked,
	Pending,
	Linked,
	Error,
	Expired
};

UENUM(BlueprintType)
enum class EMGMergeConflictResolution : uint8
{
	KeepPrimary,
	KeepSecondary,
	MergeHighest,
	MergeSum,
	AskUser
};

USTRUCT(BlueprintType)
struct FMGLinkedAccount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformType Platform = EMGPlatformType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlatformUserID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlatformDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLinkStatus Status = EMGLinkStatus::NotLinked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LinkedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPrimary = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AvatarURL;
};

USTRUCT(BlueprintType)
struct FMGUnifiedAccount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnifiedID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLinkedAccount> LinkedAccounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastLogin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformType PrimaryPlatform = EMGPlatformType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEmailVerified = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrossplayEnabled = true;
};

USTRUCT(BlueprintType)
struct FMGAuthToken
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AccessToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RefreshToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformType Platform = EMGPlatformType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Scopes;
};

USTRUCT(BlueprintType)
struct FMGMergeConflict
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ConflictID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FieldName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PrimaryValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SecondaryValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMergeConflictResolution Resolution = EMGMergeConflictResolution::AskUser;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bResolved = false;
};

USTRUCT(BlueprintType)
struct FMGAccountMergeResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMergeConflict> Conflicts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGUnifiedAccount MergedAccount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnAccountLinked, EMGPlatformType, Platform, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAccountUnlinked, EMGPlatformType, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAuthTokenRefreshed, EMGPlatformType, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMergeConflictsDetected, const TArray<FMGMergeConflict>&, Conflicts);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnAccountMergeComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLoginStateChanged, bool, bLoggedIn);

UCLASS()
class MIDNIGHTGRIND_API UMGAccountLinkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Authentication
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Auth")
	void LoginWithPlatform(EMGPlatformType Platform);

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Auth")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	bool IsLoggedIn() const { return bIsLoggedIn; }

	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	FMGUnifiedAccount GetCurrentAccount() const { return CurrentAccount; }

	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	EMGPlatformType GetCurrentPlatform() const { return CurrentPlatform; }

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Auth")
	void RefreshAuthToken();

	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	bool IsTokenValid() const;

	UFUNCTION(BlueprintPure, Category = "AccountLink|Auth")
	float GetTokenTimeRemaining() const;

	// Account Linking
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Link")
	void LinkAccount(EMGPlatformType Platform);

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Link")
	void UnlinkAccount(EMGPlatformType Platform);

	UFUNCTION(BlueprintPure, Category = "AccountLink|Link")
	bool IsAccountLinked(EMGPlatformType Platform) const;

	UFUNCTION(BlueprintPure, Category = "AccountLink|Link")
	TArray<FMGLinkedAccount> GetLinkedAccounts() const;

	UFUNCTION(BlueprintPure, Category = "AccountLink|Link")
	FMGLinkedAccount GetLinkedAccount(EMGPlatformType Platform) const;

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Link")
	void SetPrimaryAccount(EMGPlatformType Platform);

	// Account Merging
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void StartAccountMerge(const FString& SecondaryAccountCode);

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void ResolveConflict(const FString& ConflictID, EMGMergeConflictResolution Resolution);

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void ConfirmMerge();

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	void CancelMerge();

	UFUNCTION(BlueprintPure, Category = "AccountLink|Merge")
	bool HasPendingMerge() const { return PendingMerge.Conflicts.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "AccountLink|Merge")
	TArray<FMGMergeConflict> GetPendingConflicts() const { return PendingMerge.Conflicts; }

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Merge")
	FString GenerateLinkCode();

	// Cross-Play
	UFUNCTION(BlueprintCallable, Category = "AccountLink|CrossPlay")
	void SetCrossplayEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "AccountLink|CrossPlay")
	bool IsCrossplayEnabled() const { return CurrentAccount.bCrossplayEnabled; }

	// Display Name
	UFUNCTION(BlueprintCallable, Category = "AccountLink|Profile")
	void SetDisplayName(const FString& NewName);

	UFUNCTION(BlueprintPure, Category = "AccountLink|Profile")
	FString GetDisplayName() const { return CurrentAccount.DisplayName; }

	UFUNCTION(BlueprintCallable, Category = "AccountLink|Profile")
	void SyncDisplayNameFromPlatform(EMGPlatformType Platform);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAccountLinked OnAccountLinked;

	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAccountUnlinked OnAccountUnlinked;

	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAuthTokenRefreshed OnAuthTokenRefreshed;

	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnMergeConflictsDetected OnMergeConflictsDetected;

	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnAccountMergeComplete OnAccountMergeComplete;

	UPROPERTY(BlueprintAssignable, Category = "AccountLink|Events")
	FMGOnLoginStateChanged OnLoginStateChanged;

protected:
	void InitializePlatformLogin();
	void HandlePlatformLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void CreateUnifiedAccount(EMGPlatformType Platform, const FString& PlatformUserID, const FString& PlatformDisplayName);
	void LoadAccountFromBackend(const FString& UnifiedID);
	void SaveAccountToBackend();
	void StartTokenRefreshTimer();
	FString GetPlatformName(EMGPlatformType Platform) const;
	EMGPlatformType DetectCurrentPlatform() const;

private:
	FMGUnifiedAccount CurrentAccount;
	FMGAuthToken CurrentToken;
	FMGAccountMergeResult PendingMerge;
	EMGPlatformType CurrentPlatform = EMGPlatformType::Unknown;
	FTimerHandle TokenRefreshHandle;
	FDelegateHandle LoginCompleteDelegateHandle;
	bool bIsLoggedIn = false;
	float TokenRefreshBuffer = 300.0f; // Refresh 5 minutes before expiry
};
