// Copyright Midnight Grind. All Rights Reserved.

#include "Online/MGOnlineProfile.h"
#include "Progression/MGPlayerProgression.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Base64.h"
#include "Misc/SecureHash.h"

// ==========================================
// FMGPlayerProfileData
// ==========================================

FMGProfileSummary FMGPlayerProfileData::GetSummary() const
{
	FMGProfileSummary Summary;
	Summary.PlayerId = PlayerId;
	Summary.PlayerName = PlayerName;
	Summary.PlayerLevel = LevelProgression.CurrentLevel;
	Summary.TotalXP = LevelProgression.TotalXP;
	Summary.Credits = Credits;
	Summary.VehicleCount = OwnedVehicles.Num();
	Summary.TotalRaces = Statistics.TotalRaces;
	Summary.TotalWins = Statistics.TotalWins;
	Summary.TotalPlayTime = Statistics.TotalPlayTime;
	Summary.LastOnline = LastServerSync;
	// Title would come from progression system
	return Summary;
}

// ==========================================
// UMGOnlineProfileSubsystem
// ==========================================

void UMGOnlineProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGOnlineProfileSubsystem initialized"));
}

void UMGOnlineProfileSubsystem::Deinitialize()
{
	// Flush any pending changes before shutdown
	if (HasPendingChanges() && IsAuthenticated())
	{
		UE_LOG(LogTemp, Warning, TEXT("MGOnlineProfileSubsystem: Pending changes exist at shutdown"));
		// In production, might want to force sync here
	}

	GetWorld()->GetTimerManager().ClearTimer(RetryTimerHandle);

	Super::Deinitialize();
}

// ==========================================
// AUTHENTICATION
// ==========================================

void UMGOnlineProfileSubsystem::SetAuthenticatedPlayer(const FString& InPlayerId, const FString& InAuthToken)
{
	PlayerId = InPlayerId;
	AuthToken = InAuthToken;
	bProfileLoaded = false;

	UE_LOG(LogTemp, Log, TEXT("MGOnlineProfile: Authenticated player %s"), *PlayerId);

	// Automatically request full profile sync
	RequestFullSync();
}

void UMGOnlineProfileSubsystem::ClearAuthentication()
{
	PlayerId.Empty();
	AuthToken.Empty();
	CachedProfile = FMGPlayerProfileData();
	bProfileLoaded = false;
	PendingRequests.Empty();
	SetSyncStatus(EMGSyncStatus::NotSynced);

	UE_LOG(LogTemp, Log, TEXT("MGOnlineProfile: Authentication cleared"));
}

// ==========================================
// SYNC OPERATIONS
// ==========================================

void UMGOnlineProfileSubsystem::RequestFullSync()
{
	if (!IsAuthenticated())
	{
		OnAuthenticationRequired.Broadcast(TEXT("Full sync requires authentication"));
		return;
	}

	SetSyncStatus(EMGSyncStatus::Syncing);

	FString Endpoint = FString::Printf(TEXT("/profile/%s"), *PlayerId);

	SendRequest(Endpoint, TEXT("GET"), TEXT(""),
		[this](bool bSuccess, const FString& Response)
		{
			if (bSuccess)
			{
				if (DeserializeProfileFromJson(Response, EMGProfileDataFlags::All))
				{
					bProfileLoaded = true;
					CachedProfile.LastServerSync = FDateTime::UtcNow();
					ApplyProfileToSubsystems();
					SetSyncStatus(EMGSyncStatus::Synced);
					OnProfileSyncComplete.Broadcast(true, TEXT(""));
					OnProfileDataUpdated.Broadcast();

					UE_LOG(LogTemp, Log, TEXT("MGOnlineProfile: Full sync completed for %s"), *PlayerId);
				}
				else
				{
					SetSyncStatus(EMGSyncStatus::SyncFailed);
					OnProfileSyncComplete.Broadcast(false, TEXT("Failed to parse profile data"));
				}
			}
			else
			{
				SetSyncStatus(EMGSyncStatus::SyncFailed);
				OnProfileSyncComplete.Broadcast(false, Response);
			}
		});
}

void UMGOnlineProfileSubsystem::RequestPartialSync(EMGProfileDataFlags DataFlags)
{
	if (!IsAuthenticated())
	{
		OnAuthenticationRequired.Broadcast(TEXT("Partial sync requires authentication"));
		return;
	}

	// Build endpoint with query params for partial data
	FString Endpoint = FString::Printf(TEXT("/profile/%s?"), *PlayerId);

	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Progression))
	{
		Endpoint += TEXT("include=progression&");
	}
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Economy))
	{
		Endpoint += TEXT("include=economy&");
	}
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Garage))
	{
		Endpoint += TEXT("include=garage&");
	}
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Statistics))
	{
		Endpoint += TEXT("include=statistics&");
	}

	SetSyncStatus(EMGSyncStatus::Syncing);

	SendRequest(Endpoint, TEXT("GET"), TEXT(""),
		[this, DataFlags](bool bSuccess, const FString& Response)
		{
			if (bSuccess)
			{
				if (DeserializeProfileFromJson(Response, DataFlags))
				{
					ApplyProfileToSubsystems();
					SetSyncStatus(EMGSyncStatus::Synced);
					OnProfileDataUpdated.Broadcast();
				}
				else
				{
					SetSyncStatus(EMGSyncStatus::SyncFailed);
				}
			}
			else
			{
				SetSyncStatus(EMGSyncStatus::SyncFailed);
			}
		});
}

void UMGOnlineProfileSubsystem::FlushPendingChanges()
{
	if (!IsAuthenticated())
	{
		return;
	}

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::CancelPendingRequests()
{
	PendingRequests.Empty();
	bRequestInFlight = false;

	if (SyncStatus == EMGSyncStatus::PendingUpload)
	{
		SetSyncStatus(EMGSyncStatus::Synced);
	}
}

// ==========================================
// SERVER REQUESTS
// ==========================================

void UMGOnlineProfileSubsystem::SubmitRaceResult(const FMGRaceResult& RaceResult)
{
	if (!IsAuthenticated())
	{
		OnAuthenticationRequired.Broadcast(TEXT("Race submission requires authentication"));
		return;
	}

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Progression | EMGProfileDataFlags::Economy | EMGProfileDataFlags::Statistics;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = SerializeRaceResultToJson(RaceResult);

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::RequestVehiclePurchase(FName VehicleModelId)
{
	if (!IsAuthenticated())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("purchase_vehicle"));
	JsonObject->SetStringField(TEXT("vehicle_model_id"), VehicleModelId.ToString());

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Economy | EMGProfileDataFlags::Garage;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = Payload;

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::RequestPartPurchase(FGuid VehicleId, FName PartId)
{
	if (!IsAuthenticated())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("purchase_part"));
	JsonObject->SetStringField(TEXT("vehicle_id"), VehicleId.ToString());
	JsonObject->SetStringField(TEXT("part_id"), PartId.ToString());

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Economy | EMGProfileDataFlags::Garage;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = Payload;

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::RequestVehicleSale(FGuid VehicleId)
{
	if (!IsAuthenticated())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("sell_vehicle"));
	JsonObject->SetStringField(TEXT("vehicle_id"), VehicleId.ToString());

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Economy | EMGProfileDataFlags::Garage;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = Payload;

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::RequestCustomizationChange(FGuid VehicleId, const FMGPaintConfiguration& NewPaint)
{
	if (!IsAuthenticated())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("customize_vehicle"));
	JsonObject->SetStringField(TEXT("vehicle_id"), VehicleId.ToString());

	// Serialize paint config
	TSharedPtr<FJsonObject> PaintJson = MakeShareable(new FJsonObject);
	PaintJson->SetStringField(TEXT("primary_color"), NewPaint.PrimaryColor.ToHex());
	PaintJson->SetStringField(TEXT("secondary_color"), NewPaint.SecondaryColor.ToHex());
	PaintJson->SetStringField(TEXT("finish"), StaticEnum<EMGPaintFinish>()->GetNameStringByValue(static_cast<int64>(NewPaint.Finish)));
	JsonObject->SetObjectField(TEXT("paint"), PaintJson);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Garage;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = Payload;

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::RequestSelectVehicle(FGuid VehicleId)
{
	if (!IsAuthenticated())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("select_vehicle"));
	JsonObject->SetStringField(TEXT("vehicle_id"), VehicleId.ToString());

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Garage;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = Payload;

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

void UMGOnlineProfileSubsystem::RequestSettingsSync(const TMap<FName, float>& Settings, const TMap<FName, FString>& StringSettings)
{
	if (!IsAuthenticated())
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("sync_settings"));

	TSharedPtr<FJsonObject> SettingsJson = MakeShareable(new FJsonObject);
	for (const auto& Pair : Settings)
	{
		SettingsJson->SetNumberField(Pair.Key.ToString(), Pair.Value);
	}
	JsonObject->SetObjectField(TEXT("float_settings"), SettingsJson);

	TSharedPtr<FJsonObject> StringSettingsJson = MakeShareable(new FJsonObject);
	for (const auto& Pair : StringSettings)
	{
		StringSettingsJson->SetStringField(Pair.Key.ToString(), Pair.Value);
	}
	JsonObject->SetObjectField(TEXT("string_settings"), StringSettingsJson);

	FString Payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FMGPendingSyncRequest Request;
	Request.RequestId = FGuid::NewGuid();
	Request.Type = EMGSyncRequestType::Upload;
	Request.DataFlags = EMGProfileDataFlags::Settings;
	Request.RequestTime = FDateTime::UtcNow();
	Request.Payload = Payload;

	PendingRequests.Add(Request);
	SetSyncStatus(EMGSyncStatus::PendingUpload);

	ProcessPendingRequests();
}

// ==========================================
// HTTP INTEGRATION
// ==========================================

void UMGOnlineProfileSubsystem::SendRequest(const FString& Endpoint, const FString& Verb, const FString& Payload,
	TFunction<void(bool, const FString&)> Callback)
{
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	FString FullURL = BackendBaseURL + Endpoint;
	Request->SetURL(FullURL);
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));

	if (!Payload.IsEmpty())
	{
		Request->SetContentAsString(Payload);
	}

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			if (bConnectedSuccessfully && Response.IsValid())
			{
				int32 ResponseCode = Response->GetResponseCode();
				FString ResponseContent = Response->GetContentAsString();

				if (ResponseCode >= 200 && ResponseCode < 300)
				{
					Callback(true, ResponseContent);
				}
				else if (ResponseCode == 401)
				{
					Callback(false, TEXT("Authentication expired"));
				}
				else
				{
					Callback(false, FString::Printf(TEXT("Server error: %d"), ResponseCode));
				}
			}
			else
			{
				Callback(false, TEXT("Connection failed"));
			}
		});

	Request->ProcessRequest();
}

void UMGOnlineProfileSubsystem::ProcessPendingRequests()
{
	if (bRequestInFlight || PendingRequests.Num() == 0)
	{
		return;
	}

	if (!IsAuthenticated())
	{
		OnAuthenticationRequired.Broadcast(TEXT("Session expired"));
		return;
	}

	bRequestInFlight = true;
	FMGPendingSyncRequest& Request = PendingRequests[0];

	FString Endpoint = FString::Printf(TEXT("/profile/%s/action"), *PlayerId);

	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = HttpModule.CreateRequest();

	HttpRequest->SetURL(BackendBaseURL + Endpoint);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	HttpRequest->SetContentAsString(Request.Payload);

	FGuid RequestId = Request.RequestId;

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UMGOnlineProfileSubsystem::HandleActionResponse, RequestId);
	HttpRequest->ProcessRequest();
}

void UMGOnlineProfileSubsystem::HandleActionResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, const FGuid& RequestId)
{
	bRequestInFlight = false;

	// Find the pending request
	int32 RequestIndex = PendingRequests.IndexOfByPredicate([RequestId](const FMGPendingSyncRequest& R)
	{
		return R.RequestId == RequestId;
	});

	if (RequestIndex == INDEX_NONE)
	{
		return; // Request was cancelled
	}

	if (bSuccess && Response.IsValid())
	{
		int32 ResponseCode = Response->GetResponseCode();

		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			// Success - remove request and apply server response
			FString ResponseContent = Response->GetContentAsString();

			// Server returns updated profile data after action
			EMGProfileDataFlags UpdatedFlags = PendingRequests[RequestIndex].DataFlags;
			if (DeserializeProfileFromJson(ResponseContent, UpdatedFlags))
			{
				ApplyProfileToSubsystems();
				OnProfileDataUpdated.Broadcast();
			}

			PendingRequests.RemoveAt(RequestIndex);

			UE_LOG(LogTemp, Log, TEXT("MGOnlineProfile: Action completed successfully"));

			// Process next request if any
			if (PendingRequests.Num() > 0)
			{
				ProcessPendingRequests();
			}
			else
			{
				SetSyncStatus(EMGSyncStatus::Synced);
			}
		}
		else if (ResponseCode == 401)
		{
			// Auth expired
			SetSyncStatus(EMGSyncStatus::SyncFailed);
			OnAuthenticationRequired.Broadcast(TEXT("Session expired"));
		}
		else if (ResponseCode == 409)
		{
			// Conflict - profile version mismatch, need full resync
			UE_LOG(LogTemp, Warning, TEXT("MGOnlineProfile: Profile conflict, requesting full sync"));
			PendingRequests.RemoveAt(RequestIndex);
			RequestFullSync();
		}
		else
		{
			// Server error - retry
			RetryRequest(RequestId);
		}
	}
	else
	{
		// Network error - retry
		RetryRequest(RequestId);
	}
}

void UMGOnlineProfileSubsystem::RetryRequest(const FGuid& RequestId)
{
	int32 RequestIndex = PendingRequests.IndexOfByPredicate([RequestId](const FMGPendingSyncRequest& R)
	{
		return R.RequestId == RequestId;
	});

	if (RequestIndex == INDEX_NONE)
	{
		return;
	}

	FMGPendingSyncRequest& Request = PendingRequests[RequestIndex];
	Request.RetryCount++;

	if (Request.RetryCount >= MaxRetryAttempts)
	{
		UE_LOG(LogTemp, Error, TEXT("MGOnlineProfile: Request %s failed after %d retries"),
			*RequestId.ToString(), MaxRetryAttempts);

		PendingRequests.RemoveAt(RequestIndex);
		SetSyncStatus(EMGSyncStatus::SyncFailed);
		OnProfileSyncComplete.Broadcast(false, TEXT("Request failed after max retries"));
		return;
	}

	// Schedule retry with exponential backoff
	float Delay = RetryDelay * FMath::Pow(2.0f, Request.RetryCount - 1);

	UE_LOG(LogTemp, Warning, TEXT("MGOnlineProfile: Retrying request %s in %.1f seconds (attempt %d/%d)"),
		*RequestId.ToString(), Delay, Request.RetryCount, MaxRetryAttempts);

	GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle,
		FTimerDelegate::CreateUObject(this, &UMGOnlineProfileSubsystem::ProcessPendingRequests),
		Delay, false);
}

// ==========================================
// DATA SERIALIZATION
// ==========================================

FString UMGOnlineProfileSubsystem::SerializeRaceResultToJson(const FMGRaceResult& Result) const
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("action"), TEXT("submit_race"));
	JsonObject->SetStringField(TEXT("race_id"), Result.RaceId.ToString());
	JsonObject->SetStringField(TEXT("track_id"), Result.TrackId.ToString());
	JsonObject->SetStringField(TEXT("race_type"), Result.RaceType.ToString());
	JsonObject->SetNumberField(TEXT("finish_position"), Result.FinishPosition);
	JsonObject->SetNumberField(TEXT("total_racers"), Result.TotalRacers);
	JsonObject->SetNumberField(TEXT("finish_time"), Result.FinishTime);
	JsonObject->SetNumberField(TEXT("best_lap_time"), Result.BestLapTime);
	JsonObject->SetStringField(TEXT("vehicle_id"), Result.VehicleId.ToString());
	JsonObject->SetBoolField(TEXT("pink_slip"), Result.bPinkSlipRace);

	if (Result.bPinkSlipRace)
	{
		JsonObject->SetStringField(TEXT("pink_slip_vehicle"), Result.PinkSlipVehicleModelId.ToString());
	}

	JsonObject->SetNumberField(TEXT("entry_fee"), Result.EntryFee);
	JsonObject->SetNumberField(TEXT("base_prize"), Result.BasePrize);
	JsonObject->SetNumberField(TEXT("wager"), Result.WagerAmount);
	JsonObject->SetNumberField(TEXT("difficulty_multiplier"), Result.DifficultyMultiplier);
	JsonObject->SetNumberField(TEXT("crew"), static_cast<int32>(Result.RaceCrew));

	// Generate validation hash for anti-cheat
	FString HashInput = FString::Printf(TEXT("%s_%d_%f_%s_%lld"),
		*Result.RaceId.ToString(),
		Result.FinishPosition,
		Result.FinishTime,
		*PlayerId,
		FDateTime::UtcNow().GetTicks());

	JsonObject->SetStringField(TEXT("validation"), FMD5::HashAnsiString(*HashInput));
	JsonObject->SetStringField(TEXT("timestamp"), FDateTime::UtcNow().ToIso8601());

	FString Output;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	return Output;
}

bool UMGOnlineProfileSubsystem::DeserializeProfileFromJson(const FString& JsonString, EMGProfileDataFlags DataFlags)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("MGOnlineProfile: Failed to parse JSON response"));
		return false;
	}

	// Player info
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::PlayerInfo))
	{
		CachedProfile.PlayerId = JsonObject->GetStringField(TEXT("player_id"));
		CachedProfile.PlayerName = JsonObject->GetStringField(TEXT("player_name"));
		CachedProfile.ProfileVersion = static_cast<int64>(JsonObject->GetNumberField(TEXT("version")));
	}

	// Progression
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Progression))
	{
		const TSharedPtr<FJsonObject>* ProgressionObj;
		if (JsonObject->TryGetObjectField(TEXT("progression"), ProgressionObj))
		{
			CachedProfile.LevelProgression.CurrentLevel = static_cast<int32>((*ProgressionObj)->GetNumberField(TEXT("level")));
			CachedProfile.LevelProgression.CurrentXP = static_cast<int64>((*ProgressionObj)->GetNumberField(TEXT("current_xp")));
			CachedProfile.LevelProgression.TotalXP = static_cast<int64>((*ProgressionObj)->GetNumberField(TEXT("total_xp")));
			CachedProfile.LevelProgression.XPToNextLevel = static_cast<int64>((*ProgressionObj)->GetNumberField(TEXT("xp_to_next")));

			// Crew reputations
			const TArray<TSharedPtr<FJsonValue>>* CrewsArray;
			if ((*ProgressionObj)->TryGetArrayField(TEXT("crews"), CrewsArray))
			{
				CachedProfile.CrewReputations.Empty();
				for (const auto& CrewValue : *CrewsArray)
				{
					const TSharedPtr<FJsonObject>* CrewObj;
					if (CrewValue->TryGetObject(CrewObj))
					{
						FMGCrewReputation Rep;
						Rep.Crew = static_cast<EMGCrew>(static_cast<int32>((*CrewObj)->GetNumberField(TEXT("crew_id"))));
						Rep.ReputationPoints = static_cast<int32>((*CrewObj)->GetNumberField(TEXT("reputation")));
						Rep.Tier = static_cast<EMGReputationTier>(static_cast<int32>((*CrewObj)->GetNumberField(TEXT("tier"))));

						CachedProfile.CrewReputations.Add(Rep.Crew, Rep);
					}
				}
			}

			// Unlocks
			const TArray<TSharedPtr<FJsonValue>>* UnlocksArray;
			if ((*ProgressionObj)->TryGetArrayField(TEXT("unlocks"), UnlocksArray))
			{
				CachedProfile.Unlocks.Empty();
				for (const auto& UnlockValue : *UnlocksArray)
				{
					const TSharedPtr<FJsonObject>* UnlockObj;
					if (UnlockValue->TryGetObject(UnlockObj))
					{
						FMGUnlock Unlock;
						Unlock.UnlockID = FName(*(*UnlockObj)->GetStringField(TEXT("id")));
						Unlock.bUnlocked = (*UnlockObj)->GetBoolField(TEXT("unlocked"));

						CachedProfile.Unlocks.Add(Unlock);
					}
				}
			}
		}
	}

	// Statistics
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Statistics))
	{
		const TSharedPtr<FJsonObject>* StatsObj;
		if (JsonObject->TryGetObjectField(TEXT("statistics"), StatsObj))
		{
			CachedProfile.Statistics.TotalRaces = static_cast<int32>((*StatsObj)->GetNumberField(TEXT("total_races")));
			CachedProfile.Statistics.TotalWins = static_cast<int32>((*StatsObj)->GetNumberField(TEXT("wins")));
			CachedProfile.Statistics.TotalLosses = static_cast<int32>((*StatsObj)->GetNumberField(TEXT("losses")));
			CachedProfile.Statistics.TotalDNFs = static_cast<int32>((*StatsObj)->GetNumberField(TEXT("dnfs")));
			CachedProfile.Statistics.TotalPlayTime = FTimespan::FromSeconds((*StatsObj)->GetNumberField(TEXT("play_time_seconds")));
		}
	}

	// Economy
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Economy))
	{
		const TSharedPtr<FJsonObject>* EconomyObj;
		if (JsonObject->TryGetObjectField(TEXT("economy"), EconomyObj))
		{
			CachedProfile.Credits = static_cast<int64>((*EconomyObj)->GetNumberField(TEXT("credits")));
			CachedProfile.TotalEarned = static_cast<int64>((*EconomyObj)->GetNumberField(TEXT("total_earned")));
			CachedProfile.TotalSpent = static_cast<int64>((*EconomyObj)->GetNumberField(TEXT("total_spent")));

			// Recent transactions
			const TArray<TSharedPtr<FJsonValue>>* TransactionsArray;
			if ((*EconomyObj)->TryGetArrayField(TEXT("recent_transactions"), TransactionsArray))
			{
				CachedProfile.RecentTransactions.Empty();
				for (const auto& TransValue : *TransactionsArray)
				{
					const TSharedPtr<FJsonObject>* TransObj;
					if (TransValue->TryGetObject(TransObj))
					{
						FMGTransaction Trans;
						Trans.Type = static_cast<EMGTransactionType>(static_cast<int32>((*TransObj)->GetNumberField(TEXT("type"))));
						Trans.Amount = static_cast<int64>((*TransObj)->GetNumberField(TEXT("amount")));
						Trans.BalanceAfter = static_cast<int64>((*TransObj)->GetNumberField(TEXT("balance")));
						Trans.Description = FText::FromString((*TransObj)->GetStringField(TEXT("description")));

						CachedProfile.RecentTransactions.Add(Trans);
					}
				}
			}
		}
	}

	// Garage
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Garage))
	{
		const TSharedPtr<FJsonObject>* GarageObj;
		if (JsonObject->TryGetObjectField(TEXT("garage"), GarageObj))
		{
			FGuid::Parse((*GarageObj)->GetStringField(TEXT("selected_vehicle")), CachedProfile.SelectedVehicleId);

			const TArray<TSharedPtr<FJsonValue>>* VehiclesArray;
			if ((*GarageObj)->TryGetArrayField(TEXT("vehicles"), VehiclesArray))
			{
				CachedProfile.OwnedVehicles.Empty();
				for (const auto& VehicleValue : *VehiclesArray)
				{
					const TSharedPtr<FJsonObject>* VehicleObj;
					if (VehicleValue->TryGetObject(VehicleObj))
					{
						FMGOwnedVehicle Vehicle;
						FGuid::Parse((*VehicleObj)->GetStringField(TEXT("id")), Vehicle.VehicleId);
						Vehicle.ModelID = FName(*(*VehicleObj)->GetStringField(TEXT("model_id")));
						Vehicle.Nickname = FText::FromString((*VehicleObj)->GetStringField(TEXT("nickname")));
						Vehicle.PurchasePrice = static_cast<int64>((*VehicleObj)->GetNumberField(TEXT("purchase_price")));

						// Paint config
						const TSharedPtr<FJsonObject>* PaintObj;
						if ((*VehicleObj)->TryGetObjectField(TEXT("paint"), PaintObj))
						{
							Vehicle.Paint.PrimaryColor = FColor::FromHex((*PaintObj)->GetStringField(TEXT("primary")));
							Vehicle.Paint.SecondaryColor = FColor::FromHex((*PaintObj)->GetStringField(TEXT("secondary")));
						}

						CachedProfile.OwnedVehicles.Add(Vehicle);
					}
				}
			}
		}
	}

	// Settings
	if (EnumHasAnyFlags(DataFlags, EMGProfileDataFlags::Settings))
	{
		const TSharedPtr<FJsonObject>* SettingsObj;
		if (JsonObject->TryGetObjectField(TEXT("settings"), SettingsObj))
		{
			CachedProfile.GameplaySettings.Empty();
			CachedProfile.StringSettings.Empty();

			for (const auto& Pair : (*SettingsObj)->Values)
			{
				if (Pair.Value->Type == EJson::Number)
				{
					CachedProfile.GameplaySettings.Add(FName(*Pair.Key), static_cast<float>(Pair.Value->AsNumber()));
				}
				else if (Pair.Value->Type == EJson::String)
				{
					CachedProfile.StringSettings.Add(FName(*Pair.Key), Pair.Value->AsString());
				}
			}
		}
	}

	return true;
}

// ==========================================
// LOCAL CACHE
// ==========================================

void UMGOnlineProfileSubsystem::ApplyProfileToSubsystems()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// Apply progression data
	UMGPlayerProgression* Progression = GameInstance->GetSubsystem<UMGPlayerProgression>();
	if (Progression)
	{
		Progression->SetPlayerName(CachedProfile.PlayerName);
		// Additional LoadFromCache method would be needed on the progression subsystem
	}

	// Apply economy data
	UMGEconomySubsystem* Economy = GameInstance->GetSubsystem<UMGEconomySubsystem>();
	if (Economy)
	{
		Economy->SetCredits(CachedProfile.Credits);
	}

	// Apply garage data - subsystem would need LoadFromCache method
	// UMGGarageSubsystem* Garage = GameInstance->GetSubsystem<UMGGarageSubsystem>();

	UE_LOG(LogTemp, Log, TEXT("MGOnlineProfile: Applied cached profile data to subsystems"));
}

void UMGOnlineProfileSubsystem::SetSyncStatus(EMGSyncStatus NewStatus)
{
	if (SyncStatus != NewStatus)
	{
		SyncStatus = NewStatus;
		OnSyncStatusChanged.Broadcast(NewStatus);
	}
}

void UMGOnlineProfileSubsystem::SetRetryConfig(int32 MaxRetries, float RetryDelaySeconds)
{
	MaxRetryAttempts = FMath::Clamp(MaxRetries, 1, 10);
	RetryDelay = FMath::Clamp(RetryDelaySeconds, 0.5f, 30.0f);
}
