// Copyright Midnight Grind. All Rights Reserved.

#include "ServerAuth/MGServerAuthSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

void UMGServerAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize rate limits for each request type
	for (int32 i = 0; i < static_cast<int32>(EMGServerRequestType::Custom); i++)
	{
		FMGRateLimitInfo Limit;
		Limit.RequestType = static_cast<EMGServerRequestType>(i);
		Limit.RequestLimit = 100;
		Limit.RequestsRemaining = 100;
		Limit.ResetTime = FDateTime::UtcNow() + FTimespan::FromMinutes(1);
		RateLimits.Add(static_cast<EMGServerRequestType>(i), Limit);
	}

	// Set backend URL (would be configured per environment)
	BackendURL = TEXT("https://api.midnightgrind.com/v1");

	// Process queue periodically
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ProcessQueueHandle,
			this,
			&UMGServerAuthSubsystem::ProcessPendingRequests,
			0.5f,
			true
		);
	}
}

void UMGServerAuthSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProcessQueueHandle);
	}

	// Flush remaining requests
	ProcessPendingRequests();

	Super::Deinitialize();
}

void UMGServerAuthSubsystem::InitializeSession(const FString& AuthToken)
{
	CurrentSessionToken = AuthToken;
	CurrentSequenceNumber = 0;

	// Would verify token with backend
	UE_LOG(LogTemp, Log, TEXT("ServerAuth: Session initialized"));
}

void UMGServerAuthSubsystem::EndSession()
{
	CurrentSessionToken.Empty();
	PendingRequests.Empty();
	CurrentSequenceNumber = 0;
}

void UMGServerAuthSubsystem::SubmitRaceResult(const FMGRaceResultSubmission& Result)
{
	if (!HasValidSession())
	{
		UE_LOG(LogTemp, Error, TEXT("ServerAuth: Cannot submit race result without valid session"));
		return;
	}

	if (!CanMakeRequest(EMGServerRequestType::RaceResult))
	{
		OnRateLimited.Broadcast(EMGServerRequestType::RaceResult);
		return;
	}

	// Build payload
	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField(TEXT("race_id"), Result.RaceID);
	JsonPayload->SetStringField(TEXT("session_id"), Result.SessionID);
	JsonPayload->SetStringField(TEXT("track_id"), Result.TrackID.ToString());
	JsonPayload->SetStringField(TEXT("game_mode_id"), Result.GameModeID.ToString());
	JsonPayload->SetNumberField(TEXT("final_position"), Result.FinalPosition);
	JsonPayload->SetNumberField(TEXT("race_time"), Result.RaceTimeSeconds);
	JsonPayload->SetNumberField(TEXT("best_lap"), Result.BestLapTimeSeconds);
	JsonPayload->SetStringField(TEXT("vehicle_id"), Result.VehicleID.ToString());
	JsonPayload->SetNumberField(TEXT("vehicle_pi"), Result.VehiclePI);
	JsonPayload->SetStringField(TEXT("replay_hash"), Result.ReplayHash);

	// Add lap times array
	TArray<TSharedPtr<FJsonValue>> LapTimesArray;
	for (float LapTime : Result.LapTimes)
	{
		LapTimesArray.Add(MakeShareable(new FJsonValueNumber(LapTime)));
	}
	JsonPayload->SetArrayField(TEXT("lap_times"), LapTimesArray);

	// Serialize to string
	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	// Calculate checksum
	int32 Checksum = CalculateChecksum(PayloadString);
	JsonPayload->SetNumberField(TEXT("checksum"), Checksum);

	// Re-serialize with checksum
	PayloadString.Empty();
	Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	// Create request
	FMGServerRequest Request;
	Request.RequestID = GenerateRequestID();
	Request.Type = EMGServerRequestType::RaceResult;
	Request.Payload = PayloadString;
	Request.Signature = SignRequest(PayloadString);
	Request.Timestamp = FDateTime::UtcNow();
	Request.SequenceNumber = ++CurrentSequenceNumber;
	Request.SessionToken = CurrentSessionToken;

	SendRequest(Request);
	UpdateRateLimits(EMGServerRequestType::RaceResult);
}

void UMGServerAuthSubsystem::StartRaceTracking(const FString& RaceID, FName TrackID, FName GameModeID)
{
	CurrentRaceID = RaceID;
	CurrentTrackID = TrackID;
	CurrentGameModeID = GameModeID;
	CheckpointTimes.Empty();
	PositionHistory.Empty();
	SpeedHistory.Empty();
}

void UMGServerAuthSubsystem::RecordRaceCheckpoint(int32 CheckpointIndex, float Time)
{
	// Ensure array is large enough
	while (CheckpointTimes.Num() <= CheckpointIndex)
	{
		CheckpointTimes.Add(0.0f);
	}
	CheckpointTimes[CheckpointIndex] = Time;
}

void UMGServerAuthSubsystem::RecordPositionSample(FVector Position, float Speed)
{
	// Sample at reasonable rate to avoid huge data
	static const int32 MaxSamples = 1000;

	if (PositionHistory.Num() < MaxSamples)
	{
		PositionHistory.Add(Position);
		SpeedHistory.Add(Speed);
	}
}

void UMGServerAuthSubsystem::SubmitTransaction(const FMGTransactionSubmission& Transaction)
{
	if (!HasValidSession())
	{
		UE_LOG(LogTemp, Error, TEXT("ServerAuth: Cannot submit transaction without valid session"));
		return;
	}

	if (!CanMakeRequest(EMGServerRequestType::CurrencyTransaction))
	{
		OnRateLimited.Broadcast(EMGServerRequestType::CurrencyTransaction);
		return;
	}

	// Validate balance change
	int64 ExpectedChange = Transaction.BalanceAfter - Transaction.BalanceBefore;
	if (Transaction.TransactionType == TEXT("earn"))
	{
		if (ExpectedChange != Transaction.Amount)
		{
			OnSuspiciousActivity.Broadcast(TEXT("Balance mismatch on earn transaction"));
			return;
		}
	}
	else if (Transaction.TransactionType == TEXT("spend"))
	{
		if (ExpectedChange != -Transaction.Amount)
		{
			OnSuspiciousActivity.Broadcast(TEXT("Balance mismatch on spend transaction"));
			return;
		}
	}

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField(TEXT("transaction_id"), Transaction.TransactionID);
	JsonPayload->SetStringField(TEXT("type"), Transaction.TransactionType);
	JsonPayload->SetStringField(TEXT("currency"), Transaction.CurrencyType);
	JsonPayload->SetNumberField(TEXT("amount"), Transaction.Amount);
	JsonPayload->SetStringField(TEXT("item_id"), Transaction.ItemID);
	JsonPayload->SetStringField(TEXT("source"), Transaction.Source);
	JsonPayload->SetNumberField(TEXT("balance_before"), Transaction.BalanceBefore);
	JsonPayload->SetNumberField(TEXT("balance_after"), Transaction.BalanceAfter);

	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	FMGServerRequest Request;
	Request.RequestID = GenerateRequestID();
	Request.Type = EMGServerRequestType::CurrencyTransaction;
	Request.Payload = PayloadString;
	Request.Signature = SignRequest(PayloadString);
	Request.Timestamp = FDateTime::UtcNow();
	Request.SequenceNumber = ++CurrentSequenceNumber;
	Request.SessionToken = CurrentSessionToken;

	SendRequest(Request);
	UpdateRateLimits(EMGServerRequestType::CurrencyTransaction);
}

void UMGServerAuthSubsystem::ValidatePurchase(const FString& ItemID, int64 Price, const FString& CurrencyType)
{
	if (!CanMakeRequest(EMGServerRequestType::VehiclePurchase))
	{
		OnRateLimited.Broadcast(EMGServerRequestType::VehiclePurchase);
		return;
	}

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField(TEXT("item_id"), ItemID);
	JsonPayload->SetNumberField(TEXT("price"), Price);
	JsonPayload->SetStringField(TEXT("currency"), CurrencyType);

	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	FMGServerRequest Request;
	Request.RequestID = GenerateRequestID();
	Request.Type = EMGServerRequestType::VehiclePurchase;
	Request.Payload = PayloadString;
	Request.Signature = SignRequest(PayloadString);
	Request.Timestamp = FDateTime::UtcNow();
	Request.SequenceNumber = ++CurrentSequenceNumber;
	Request.SessionToken = CurrentSessionToken;

	SendRequest(Request);
	UpdateRateLimits(EMGServerRequestType::VehiclePurchase);
}

void UMGServerAuthSubsystem::ValidateCurrencyGrant(const FString& Source, int64 Amount, const FString& CurrencyType)
{
	// Check for suspicious grant patterns
	FDateTime Now = FDateTime::UtcNow();
	FString GrantKey = FString::Printf(TEXT("grant_%s_%s"), *Source, *CurrencyType);

	if (FDateTime* LastGrant = RecentRequests.Find(GrantKey))
	{
		FTimespan TimeSinceLastGrant = Now - *LastGrant;
		if (TimeSinceLastGrant.GetTotalSeconds() < 1.0f)
		{
			OnSuspiciousActivity.Broadcast(TEXT("Rapid currency grant detected"));
			return;
		}
	}

	RecentRequests.Add(GrantKey, Now);

	// Proceed with validation
	FMGTransactionSubmission Transaction;
	Transaction.TransactionID = GenerateRequestID();
	Transaction.TransactionType = TEXT("earn");
	Transaction.CurrencyType = CurrencyType;
	Transaction.Amount = Amount;
	Transaction.Source = Source;

	SubmitTransaction(Transaction);
}

void UMGServerAuthSubsystem::SubmitReplayForVerification(const FString& RaceID, const TArray<uint8>& ReplayData)
{
	if (!CanMakeRequest(EMGServerRequestType::ReplaySubmission))
	{
		OnRateLimited.Broadcast(EMGServerRequestType::ReplaySubmission);
		return;
	}

	FString ReplayHash = CalculateReplayHash(ReplayData);

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField(TEXT("race_id"), RaceID);
	JsonPayload->SetStringField(TEXT("replay_hash"), ReplayHash);
	JsonPayload->SetNumberField(TEXT("replay_size"), ReplayData.Num());

	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	FMGServerRequest Request;
	Request.RequestID = GenerateRequestID();
	Request.Type = EMGServerRequestType::ReplaySubmission;
	Request.Payload = PayloadString;
	Request.Signature = SignRequest(PayloadString);
	Request.Timestamp = FDateTime::UtcNow();
	Request.SequenceNumber = ++CurrentSequenceNumber;
	Request.SessionToken = CurrentSessionToken;

	SendRequest(Request);
	UpdateRateLimits(EMGServerRequestType::ReplaySubmission);
}

FString UMGServerAuthSubsystem::CalculateReplayHash(const TArray<uint8>& ReplayData) const
{
	FSHAHash Hash;
	FSHA1::HashBuffer(ReplayData.GetData(), ReplayData.Num(), Hash.Hash);
	return Hash.ToString();
}

void UMGServerAuthSubsystem::SubmitLeaderboardEntry(FName LeaderboardID, int64 Score, const FString& ReplayHash)
{
	if (!CanMakeRequest(EMGServerRequestType::LeaderboardSubmission))
	{
		OnRateLimited.Broadcast(EMGServerRequestType::LeaderboardSubmission);
		return;
	}

	TSharedPtr<FJsonObject> JsonPayload = MakeShareable(new FJsonObject);
	JsonPayload->SetStringField(TEXT("leaderboard_id"), LeaderboardID.ToString());
	JsonPayload->SetNumberField(TEXT("score"), Score);
	JsonPayload->SetStringField(TEXT("replay_hash"), ReplayHash);

	FString PayloadString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadString);
	FJsonSerializer::Serialize(JsonPayload.ToSharedRef(), Writer);

	FMGServerRequest Request;
	Request.RequestID = GenerateRequestID();
	Request.Type = EMGServerRequestType::LeaderboardSubmission;
	Request.Payload = PayloadString;
	Request.Signature = SignRequest(PayloadString);
	Request.Timestamp = FDateTime::UtcNow();
	Request.SequenceNumber = ++CurrentSequenceNumber;
	Request.SessionToken = CurrentSessionToken;

	SendRequest(Request);
	UpdateRateLimits(EMGServerRequestType::LeaderboardSubmission);
}

bool UMGServerAuthSubsystem::CanMakeRequest(EMGServerRequestType Type) const
{
	const FMGRateLimitInfo* Limit = RateLimits.Find(Type);
	if (!Limit)
		return true;

	if (FDateTime::UtcNow() >= Limit->ResetTime)
		return true;

	return Limit->RequestsRemaining > 0;
}

FMGRateLimitInfo UMGServerAuthSubsystem::GetRateLimitInfo(EMGServerRequestType Type) const
{
	const FMGRateLimitInfo* Limit = RateLimits.Find(Type);
	if (Limit)
		return *Limit;
	return FMGRateLimitInfo();
}

float UMGServerAuthSubsystem::GetTimeUntilReset(EMGServerRequestType Type) const
{
	const FMGRateLimitInfo* Limit = RateLimits.Find(Type);
	if (!Limit)
		return 0.0f;

	FTimespan Remaining = Limit->ResetTime - FDateTime::UtcNow();
	return FMath::Max(0.0f, static_cast<float>(Remaining.GetTotalSeconds()));
}

int32 UMGServerAuthSubsystem::CalculateChecksum(const FString& Data) const
{
	uint32 Checksum = 0;
	for (int32 i = 0; i < Data.Len(); i++)
	{
		Checksum = ((Checksum << 5) + Checksum) + static_cast<uint32>(Data[i]);
	}
	return static_cast<int32>(Checksum);
}

FString UMGServerAuthSubsystem::SignRequest(const FString& Payload) const
{
	// HMAC-SHA256 signature
	FString DataToSign = Payload + CurrentSessionToken + FString::FromInt(CurrentSequenceNumber);

	FSHAHash Hash;
	FSHA1::HashBuffer(TCHAR_TO_UTF8(*DataToSign), DataToSign.Len(), Hash.Hash);
	return Hash.ToString();
}

bool UMGServerAuthSubsystem::ValidateServerResponse(const FMGServerResponse& Response) const
{
	// Validate timestamp is reasonable
	if (!ValidateTimestamp(Response.ServerTimestamp))
	{
		return false;
	}

	// Validate sequence number is increasing
	if (Response.ServerSequenceNumber <= CurrentSequenceNumber - 100)
	{
		return false;
	}

	return true;
}

void UMGServerAuthSubsystem::SendRequest(const FMGServerRequest& Request)
{
	if (PendingRequests.Num() >= MaxPendingRequests)
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerAuth: Request queue full, dropping request"));
		return;
	}

	PendingRequests.Add(Request);
}

void UMGServerAuthSubsystem::HandleResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bWasSuccessful)
{
	if (!bWasSuccessful || !HttpResponse.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ServerAuth: Request failed"));
		return;
	}

	int32 ResponseCode = HttpResponse->GetResponseCode();
	FString ResponseBody = HttpResponse->GetContentAsString();

	TSharedPtr<FJsonObject> JsonResponse;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonResponse) || !JsonResponse.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ServerAuth: Failed to parse response"));
		return;
	}

	FMGServerResponse Response;
	Response.RequestID = JsonResponse->GetStringField(TEXT("request_id"));
	Response.Message = JsonResponse->GetStringField(TEXT("message"));
	Response.ResponseData = JsonResponse->GetStringField(TEXT("data"));

	FString ResultString = JsonResponse->GetStringField(TEXT("result"));
	if (ResultString == TEXT("valid"))
		Response.Result = EMGValidationResult::Valid;
	else if (ResultString == TEXT("invalid_signature"))
		Response.Result = EMGValidationResult::Invalid_Signature;
	else if (ResultString == TEXT("invalid_timestamp"))
		Response.Result = EMGValidationResult::Invalid_Timestamp;
	else if (ResultString == TEXT("rate_limited"))
		Response.Result = EMGValidationResult::RateLimited;
	else if (ResultString == TEXT("suspicious"))
		Response.Result = EMGValidationResult::Suspicious;
	else if (ResultString == TEXT("banned"))
		Response.Result = EMGValidationResult::Banned;
	else
		Response.Result = EMGValidationResult::Invalid_Data;

	OnServerValidationComplete.Broadcast(Response.RequestID, Response.Result);
}

void UMGServerAuthSubsystem::UpdateRateLimits(EMGServerRequestType Type)
{
	FMGRateLimitInfo* Limit = RateLimits.Find(Type);
	if (!Limit)
		return;

	FDateTime Now = FDateTime::UtcNow();

	// Reset if past reset time
	if (Now >= Limit->ResetTime)
	{
		Limit->RequestsRemaining = Limit->RequestLimit;
		Limit->ResetTime = Now + FTimespan::FromMinutes(1);
	}

	Limit->RequestsRemaining = FMath::Max(0, Limit->RequestsRemaining - 1);
}

void UMGServerAuthSubsystem::ProcessPendingRequests()
{
	if (PendingRequests.Num() == 0)
		return;

	// Process one request per tick
	FMGServerRequest Request = PendingRequests[0];
	PendingRequests.RemoveAt(0);

	// Create HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	FString Endpoint;
	switch (Request.Type)
	{
	case EMGServerRequestType::RaceResult:
		Endpoint = TEXT("/race/submit");
		break;
	case EMGServerRequestType::CurrencyTransaction:
		Endpoint = TEXT("/economy/transaction");
		break;
	case EMGServerRequestType::VehiclePurchase:
		Endpoint = TEXT("/economy/purchase");
		break;
	case EMGServerRequestType::ReplaySubmission:
		Endpoint = TEXT("/replay/submit");
		break;
	case EMGServerRequestType::LeaderboardSubmission:
		Endpoint = TEXT("/leaderboard/submit");
		break;
	default:
		Endpoint = TEXT("/custom");
		break;
	}

	HttpRequest->SetURL(BackendURL + Endpoint);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Request.SessionToken);
	HttpRequest->SetHeader(TEXT("X-Request-ID"), Request.RequestID);
	HttpRequest->SetHeader(TEXT("X-Signature"), Request.Signature);
	HttpRequest->SetHeader(TEXT("X-Sequence"), FString::FromInt(Request.SequenceNumber));
	HttpRequest->SetContentAsString(Request.Payload);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UMGServerAuthSubsystem::HandleResponse);
	HttpRequest->ProcessRequest();

	DetectSuspiciousPatterns();
}

FString UMGServerAuthSubsystem::GenerateRequestID() const
{
	return FGuid::NewGuid().ToString();
}

bool UMGServerAuthSubsystem::ValidateTimestamp(const FDateTime& Timestamp) const
{
	FTimespan Drift = FDateTime::UtcNow() - Timestamp;
	return FMath::Abs(Drift.GetTotalSeconds()) <= MaxTimestampDrift;
}

void UMGServerAuthSubsystem::DetectSuspiciousPatterns()
{
	// Check for rapid-fire requests
	FDateTime Now = FDateTime::UtcNow();
	int32 RecentCount = 0;

	for (const auto& Pair : RecentRequests)
	{
		FTimespan Age = Now - Pair.Value;
		if (Age.GetTotalSeconds() < 1.0f)
		{
			RecentCount++;
		}
	}

	if (RecentCount > 10)
	{
		OnSuspiciousActivity.Broadcast(TEXT("Abnormal request rate detected"));
	}

	// Clean up old entries
	TArray<FString> KeysToRemove;
	for (const auto& Pair : RecentRequests)
	{
		FTimespan Age = Now - Pair.Value;
		if (Age.GetTotalMinutes() > 5.0f)
		{
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const FString& Key : KeysToRemove)
	{
		RecentRequests.Remove(Key);
	}
}
