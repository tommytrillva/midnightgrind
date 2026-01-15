// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Progression/MGPlayerProgression.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "MGOnlineProfile.generated.h"

/**
 * Sync status for online data
 */
UENUM(BlueprintType)
enum class EMGSyncStatus : uint8
{
	/** Not synced, needs server data */
	NotSynced,
	/** Currently syncing with server */
	Syncing,
	/** Synced and up to date */
	Synced,
	/** Local changes pending upload */
	PendingUpload,
	/** Sync failed, retry needed */
	SyncFailed,
	/** Offline mode (degraded) */
	Offline
};

/**
 * Profile data categories for partial sync
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMGProfileDataFlags : uint8
{
	None			= 0,
	PlayerInfo		= 1 << 0,
	Progression		= 1 << 1,
	Economy			= 1 << 2,
	Garage			= 1 << 3,
	Statistics		= 1 << 4,
	Settings		= 1 << 5,
	All				= 0xFF
};
ENUM_CLASS_FLAGS(EMGProfileDataFlags);

/**
 * Server sync request types
 */
UENUM(BlueprintType)
enum class EMGSyncRequestType : uint8
{
	/** Full profile download */
	FullSync,
	/** Partial data update */
	PartialSync,
	/** Upload local changes */
	Upload,
	/** Validate data integrity */
	Validate
};

/**
 * Player profile summary for display
 */
USTRUCT(BlueprintType)
struct FMGProfileSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerLevel = 1;

	UPROPERTY(BlueprintReadOnly)
	int64 TotalXP = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 Credits = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 VehicleCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalRaces = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalWins = 0;

	UPROPERTY(BlueprintReadOnly)
	FTimespan TotalPlayTime;

	UPROPERTY(BlueprintReadOnly)
	FDateTime LastOnline;

	UPROPERTY(BlueprintReadOnly)
	FString CurrentTitle;
};

/**
 * Player profile data - server authoritative
 * This is a local cache of server data, NOT the source of truth
 */
USTRUCT(BlueprintType)
struct FMGPlayerProfileData
{
	GENERATED_BODY()

	// ==========================================
	// IDENTITY
	// ==========================================

	/** Unique player ID from authentication */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerId;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/** Profile version for conflict resolution */
	UPROPERTY()
	int64 ProfileVersion = 0;

	/** Server timestamp of last sync */
	UPROPERTY()
	FDateTime LastServerSync;

	// ==========================================
	// PROGRESSION DATA
	// ==========================================

	UPROPERTY(BlueprintReadOnly)
	FMGLevelProgression LevelProgression;

	UPROPERTY(BlueprintReadOnly)
	TMap<EMGCrew, FMGCrewReputation> CrewReputations;

	UPROPERTY(BlueprintReadOnly)
	TArray<FMGUnlock> Unlocks;

	UPROPERTY(BlueprintReadOnly)
	FMGRaceStatistics Statistics;

	// ==========================================
	// ECONOMY DATA
	// ==========================================

	UPROPERTY(BlueprintReadOnly)
	int64 Credits = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 TotalEarned = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 TotalSpent = 0;

	/** Recent transactions (server may limit history) */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGTransaction> RecentTransactions;

	// ==========================================
	// GARAGE DATA
	// ==========================================

	UPROPERTY(BlueprintReadOnly)
	TArray<FMGOwnedVehicle> OwnedVehicles;

	UPROPERTY(BlueprintReadOnly)
	FGuid SelectedVehicleId;

	// ==========================================
	// SETTINGS (synced across devices)
	// ==========================================

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, float> GameplaySettings;

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FString> StringSettings;

	// ==========================================
	// UTILITY
	// ==========================================

	FMGProfileSummary GetSummary() const;
	bool IsValid() const { return !PlayerId.IsEmpty(); }
};

/**
 * Pending server request
 */
USTRUCT()
struct FMGPendingSyncRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid RequestId;

	UPROPERTY()
	EMGSyncRequestType Type = EMGSyncRequestType::FullSync;

	UPROPERTY()
	EMGProfileDataFlags DataFlags = EMGProfileDataFlags::All;

	UPROPERTY()
	FDateTime RequestTime;

	UPROPERTY()
	int32 RetryCount = 0;

	/** Serialized payload for upload requests */
	UPROPERTY()
	FString Payload;
};

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProfileSyncComplete, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyncStatusChanged, EMGSyncStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfileDataUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthenticationRequired, const FString&, Reason);

/**
 * Online Profile Subsystem
 * Manages player profile data with server-authoritative sync
 *
 * All progression, economy, and garage data is stored on the server.
 * This subsystem maintains a local cache for UI responsiveness
 * and queues changes for server validation.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGOnlineProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// AUTHENTICATION
	// ==========================================

	/** Set authenticated player ID (called after login) */
	UFUNCTION(BlueprintCallable, Category = "Online Profile")
	void SetAuthenticatedPlayer(const FString& InPlayerId, const FString& InAuthToken);

	/** Clear authentication (logout) */
	UFUNCTION(BlueprintCallable, Category = "Online Profile")
	void ClearAuthentication();

	/** Is player authenticated? */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	bool IsAuthenticated() const { return !AuthToken.IsEmpty() && !PlayerId.IsEmpty(); }

	/** Get current player ID */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	FString GetPlayerId() const { return PlayerId; }

	// ==========================================
	// SYNC OPERATIONS
	// ==========================================

	/** Request full profile sync from server */
	UFUNCTION(BlueprintCallable, Category = "Online Profile")
	void RequestFullSync();

	/** Request partial sync for specific data */
	UFUNCTION(BlueprintCallable, Category = "Online Profile")
	void RequestPartialSync(EMGProfileDataFlags DataFlags);

	/** Force upload pending changes */
	UFUNCTION(BlueprintCallable, Category = "Online Profile")
	void FlushPendingChanges();

	/** Cancel all pending sync requests */
	UFUNCTION(BlueprintCallable, Category = "Online Profile")
	void CancelPendingRequests();

	/** Get current sync status */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	EMGSyncStatus GetSyncStatus() const { return SyncStatus; }

	/** Is currently syncing? */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	bool IsSyncing() const { return SyncStatus == EMGSyncStatus::Syncing; }

	/** Has pending changes to upload? */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	bool HasPendingChanges() const { return PendingRequests.Num() > 0; }

	// ==========================================
	// PROFILE DATA ACCESS (Read-Only Cache)
	// ==========================================

	/** Get cached profile data */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	const FMGPlayerProfileData& GetProfileData() const { return CachedProfile; }

	/** Get profile summary for UI */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	FMGProfileSummary GetProfileSummary() const { return CachedProfile.GetSummary(); }

	/** Is profile data loaded? */
	UFUNCTION(BlueprintPure, Category = "Online Profile")
	bool IsProfileLoaded() const { return bProfileLoaded; }

	// ==========================================
	// SERVER REQUESTS (Queue changes for upload)
	// ==========================================

	/**
	 * Queue a race result for server processing
	 * Server validates and updates progression/economy
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Race")
	void SubmitRaceResult(const FMGRaceResult& RaceResult);

	/**
	 * Queue a vehicle purchase request
	 * Server validates credits and adds vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Economy")
	void RequestVehiclePurchase(FName VehicleModelId);

	/**
	 * Queue a part purchase request
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Economy")
	void RequestPartPurchase(FGuid VehicleId, FName PartId);

	/**
	 * Queue a vehicle sale request
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Economy")
	void RequestVehicleSale(FGuid VehicleId);

	/**
	 * Queue a customization change
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Garage")
	void RequestCustomizationChange(FGuid VehicleId, const FMGPaintConfiguration& NewPaint);

	/**
	 * Queue selected vehicle change
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Garage")
	void RequestSelectVehicle(FGuid VehicleId);

	/**
	 * Queue settings sync
	 */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Settings")
	void RequestSettingsSync(const TMap<FName, float>& Settings, const TMap<FName, FString>& StringSettings);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when profile sync completes */
	UPROPERTY(BlueprintAssignable, Category = "Online Profile|Events")
	FOnProfileSyncComplete OnProfileSyncComplete;

	/** Called when sync status changes */
	UPROPERTY(BlueprintAssignable, Category = "Online Profile|Events")
	FOnSyncStatusChanged OnSyncStatusChanged;

	/** Called when profile data is updated */
	UPROPERTY(BlueprintAssignable, Category = "Online Profile|Events")
	FOnProfileDataUpdated OnProfileDataUpdated;

	/** Called when re-authentication is needed */
	UPROPERTY(BlueprintAssignable, Category = "Online Profile|Events")
	FOnAuthenticationRequired OnAuthenticationRequired;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set the backend API base URL */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Config")
	void SetBackendURL(const FString& URL) { BackendBaseURL = URL; }

	/** Set sync retry parameters */
	UFUNCTION(BlueprintCallable, Category = "Online Profile|Config")
	void SetRetryConfig(int32 MaxRetries, float RetryDelaySeconds);

protected:
	// ==========================================
	// HTTP INTEGRATION
	// ==========================================

	/** Send HTTP request to backend */
	void SendRequest(const FString& Endpoint, const FString& Verb, const FString& Payload,
		TFunction<void(bool, const FString&)> Callback);

	/** Handle sync response from server */
	void HandleSyncResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

	/** Handle action response from server */
	void HandleActionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess,
		const FGuid& RequestId);

	/** Process pending request queue */
	void ProcessPendingRequests();

	/** Retry failed request */
	void RetryRequest(const FGuid& RequestId);

	// ==========================================
	// DATA SERIALIZATION
	// ==========================================

	/** Serialize profile data to JSON */
	FString SerializeProfileToJson(EMGProfileDataFlags DataFlags) const;

	/** Deserialize profile data from JSON */
	bool DeserializeProfileFromJson(const FString& JsonString, EMGProfileDataFlags DataFlags);

	/** Serialize race result to JSON */
	FString SerializeRaceResultToJson(const FMGRaceResult& Result) const;

	// ==========================================
	// LOCAL CACHE
	// ==========================================

	/** Apply server data to local subsystems */
	void ApplyProfileToSubsystems();

	/** Update sync status and broadcast */
	void SetSyncStatus(EMGSyncStatus NewStatus);

	// ==========================================
	// DATA
	// ==========================================

	/** Authentication token */
	UPROPERTY()
	FString AuthToken;

	/** Current player ID */
	UPROPERTY()
	FString PlayerId;

	/** Cached profile data */
	UPROPERTY()
	FMGPlayerProfileData CachedProfile;

	/** Is profile loaded from server? */
	bool bProfileLoaded = false;

	/** Current sync status */
	UPROPERTY()
	EMGSyncStatus SyncStatus = EMGSyncStatus::NotSynced;

	/** Pending sync requests */
	UPROPERTY()
	TArray<FMGPendingSyncRequest> PendingRequests;

	/** Backend API URL */
	UPROPERTY()
	FString BackendBaseURL = TEXT("https://api.midnightgrind.com/v1");

	/** Maximum retry attempts */
	int32 MaxRetryAttempts = 3;

	/** Retry delay in seconds */
	float RetryDelay = 2.0f;

	/** Is request in flight? */
	bool bRequestInFlight = false;

	/** Request timeout handle */
	FTimerHandle RetryTimerHandle;
};

/**
 * Race result structure for server submission
 */
USTRUCT(BlueprintType)
struct FMGRaceResult
{
	GENERATED_BODY()

	/** Race identifier */
	UPROPERTY(BlueprintReadWrite)
	FName RaceId;

	/** Track identifier */
	UPROPERTY(BlueprintReadWrite)
	FName TrackId;

	/** Race type */
	UPROPERTY(BlueprintReadWrite)
	FName RaceType;

	/** Finishing position (1-indexed) */
	UPROPERTY(BlueprintReadWrite)
	int32 FinishPosition = 0;

	/** Total racers */
	UPROPERTY(BlueprintReadWrite)
	int32 TotalRacers = 0;

	/** Finish time in seconds */
	UPROPERTY(BlueprintReadWrite)
	float FinishTime = 0.0f;

	/** Best lap time */
	UPROPERTY(BlueprintReadWrite)
	float BestLapTime = 0.0f;

	/** Vehicle used */
	UPROPERTY(BlueprintReadWrite)
	FGuid VehicleId;

	/** Was this a pink slip race? */
	UPROPERTY(BlueprintReadWrite)
	bool bPinkSlipRace = false;

	/** Opponent vehicle ID for pink slip */
	UPROPERTY(BlueprintReadWrite)
	FName PinkSlipVehicleModelId;

	/** Entry fee paid */
	UPROPERTY(BlueprintReadWrite)
	int64 EntryFee = 0;

	/** Base prize pool */
	UPROPERTY(BlueprintReadWrite)
	int64 BasePrize = 0;

	/** Wager amount (if any) */
	UPROPERTY(BlueprintReadWrite)
	int64 WagerAmount = 0;

	/** Difficulty multiplier */
	UPROPERTY(BlueprintReadWrite)
	float DifficultyMultiplier = 1.0f;

	/** Crew this race was for (if any) */
	UPROPERTY(BlueprintReadWrite)
	EMGCrew RaceCrew = EMGCrew::None;

	/** Anti-cheat validation hash */
	UPROPERTY()
	FString ValidationHash;

	/** Client timestamp */
	UPROPERTY()
	FDateTime ClientTimestamp;
};
