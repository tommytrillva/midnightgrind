// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "MGServerAuthSubsystem.generated.h"

/**
 * Server Auth System - Server-Authoritative Validation
 * - All game state changes validated server-side
 * - Secure race results submission
 * - Economy transaction validation
 * - Replay verification for anti-cheat
 * - Rate limiting and abuse prevention
 */

UENUM(BlueprintType)
enum class EMGServerRequestType : uint8
{
	RaceResult,
	CurrencyTransaction,
	VehiclePurchase,
	UpgradePurchase,
	ReplaySubmission,
	LeaderboardSubmission,
	ProgressionUpdate,
	AchievementUnlock,
	Custom
};

UENUM(BlueprintType)
enum class EMGValidationResult : uint8
{
	Valid,
	Invalid_Signature,
	Invalid_Timestamp,
	Invalid_Data,
	Invalid_Session,
	RateLimited,
	ServerError,
	Suspicious,
	Banned
};

USTRUCT(BlueprintType)
struct FMGServerRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGServerRequestType Type = EMGServerRequestType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Payload;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Signature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SequenceNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionToken;
};

USTRUCT(BlueprintType)
struct FMGServerResponse
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGValidationResult Result = EMGValidationResult::Valid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResponseData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ServerSequenceNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ServerTimestamp;
};

USTRUCT(BlueprintType)
struct FMGRaceResultSubmission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> LapTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclePI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> PositionSamples;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SpeedSamples;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayHash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Checksum = 0;
};

USTRUCT(BlueprintType)
struct FMGTransactionSubmission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransactionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrencyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Source;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BalanceBefore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 BalanceAfter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Checksum = 0;
};

USTRUCT(BlueprintType)
struct FMGRateLimitInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGServerRequestType RequestType = EMGServerRequestType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequestsRemaining = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequestLimit = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ResetTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RetryAfterSeconds = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnServerValidationComplete, const FString&, RequestID, EMGValidationResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRaceResultValidated, const FString&, RaceID, bool, bValid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTransactionValidated, const FString&, TransactionID, bool, bValid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRateLimited, EMGServerRequestType, RequestType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSuspiciousActivity, const FString&, Reason);

UCLASS()
class MIDNIGHTGRIND_API UMGServerAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Session")
	void InitializeSession(const FString& AuthToken);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Session")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "ServerAuth|Session")
	bool HasValidSession() const { return !CurrentSessionToken.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "ServerAuth|Session")
	FString GetSessionToken() const { return CurrentSessionToken; }

	// Race Result Submission
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void SubmitRaceResult(const FMGRaceResultSubmission& Result);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void StartRaceTracking(const FString& RaceID, FName TrackID, FName GameModeID);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void RecordRaceCheckpoint(int32 CheckpointIndex, float Time);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Race")
	void RecordPositionSample(FVector Position, float Speed);

	// Transaction Validation
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Transaction")
	void SubmitTransaction(const FMGTransactionSubmission& Transaction);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Transaction")
	void ValidatePurchase(const FString& ItemID, int64 Price, const FString& CurrencyType);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Transaction")
	void ValidateCurrencyGrant(const FString& Source, int64 Amount, const FString& CurrencyType);

	// Replay Verification
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Replay")
	void SubmitReplayForVerification(const FString& RaceID, const TArray<uint8>& ReplayData);

	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Replay")
	FString CalculateReplayHash(const TArray<uint8>& ReplayData) const;

	// Leaderboard Submission
	UFUNCTION(BlueprintCallable, Category = "ServerAuth|Leaderboard")
	void SubmitLeaderboardEntry(FName LeaderboardID, int64 Score, const FString& ReplayHash);

	// Rate Limiting
	UFUNCTION(BlueprintPure, Category = "ServerAuth|RateLimit")
	bool CanMakeRequest(EMGServerRequestType Type) const;

	UFUNCTION(BlueprintPure, Category = "ServerAuth|RateLimit")
	FMGRateLimitInfo GetRateLimitInfo(EMGServerRequestType Type) const;

	UFUNCTION(BlueprintPure, Category = "ServerAuth|RateLimit")
	float GetTimeUntilReset(EMGServerRequestType Type) const;

	// Validation Helpers
	UFUNCTION(BlueprintPure, Category = "ServerAuth|Validation")
	int32 CalculateChecksum(const FString& Data) const;

	UFUNCTION(BlueprintPure, Category = "ServerAuth|Validation")
	FString SignRequest(const FString& Payload) const;

	UFUNCTION(BlueprintPure, Category = "ServerAuth|Validation")
	bool ValidateServerResponse(const FMGServerResponse& Response) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnServerValidationComplete OnServerValidationComplete;

	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnRaceResultValidated OnRaceResultValidated;

	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnTransactionValidated OnTransactionValidated;

	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnRateLimited OnRateLimited;

	UPROPERTY(BlueprintAssignable, Category = "ServerAuth|Events")
	FMGOnSuspiciousActivity OnSuspiciousActivity;

protected:
	void SendRequest(const FMGServerRequest& Request);
	void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void UpdateRateLimits(EMGServerRequestType Type);
	void ProcessPendingRequests();
	FString GenerateRequestID() const;
	bool ValidateTimestamp(const FDateTime& Timestamp) const;
	void DetectSuspiciousPatterns();

private:
	FString CurrentSessionToken;
	FString ServerSecretKey;
	FString BackendURL;
	TArray<FMGServerRequest> PendingRequests;
	TMap<EMGServerRequestType, FMGRateLimitInfo> RateLimits;
	TMap<FString, FDateTime> RecentRequests;
	FTimerHandle ProcessQueueHandle;
	int32 CurrentSequenceNumber = 0;
	float MaxTimestampDrift = 300.0f; // 5 minutes
	int32 MaxPendingRequests = 50;

	// Race tracking
	FString CurrentRaceID;
	FName CurrentTrackID;
	FName CurrentGameModeID;
	TArray<float> CheckpointTimes;
	TArray<FVector> PositionHistory;
	TArray<float> SpeedHistory;
};
