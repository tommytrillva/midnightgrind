// Copyright Midnight Grind. All Rights Reserved.

#include "Ghost/MGGhostSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// Backend API configuration
namespace MGGhostAPI
{
	const FString BaseURL = TEXT("https://api.midnightgrind.com/v1/ghosts");
	const FString APIKey = TEXT(""); // Set via config
}

void UMGGhostSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default settings
	Settings.bShowGhosts = true;
	Settings.DefaultVisibility = EMGGhostVisibility::Transparent;
	Settings.MaxGhostsOnTrack = 3;
	Settings.bShowPersonalBest = true;
	Settings.bShowWorldRecord = true;
	Settings.bShowRivalGhosts = true;
	Settings.bShowTimeDelta = true;
	Settings.GhostOpacity = 0.5f;
	Settings.PersonalBestColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
	Settings.WorldRecordColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
	Settings.RivalColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);
	Settings.RecordingInterval = 0.033f; // 30 FPS
	Settings.bAutoSavePersonalBest = true;
	Settings.bCompressGhostData = true;

	LoadGhostIndex();

	// Start ghost tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GhostTickHandle,
			this,
			&UMGGhostSubsystem::OnGhostTick,
			0.016f, // 60 FPS for smooth playback
			true
		);
	}
}

void UMGGhostSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GhostTickHandle);
	}

	SaveGhostIndex();

	Super::Deinitialize();
}

bool UMGGhostSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGGhostSubsystem::OnGhostTick()
{
	UpdatePlayback(0.016f);

	if (bComparing)
	{
		UpdateComparison();
	}
}

void UMGGhostSubsystem::UpdatePlayback(float DeltaTime)
{
	TArray<FGuid> CompletedGhosts;

	for (auto& Pair : ActivePlaybacks)
	{
		FMGGhostInstance& Instance = Pair.Value;

		if (Instance.State != EMGGhostState::Playing)
		{
			continue;
		}

		// Advance time
		Instance.CurrentTime += DeltaTime * Instance.PlaybackSpeed;

		// Check if playback complete
		if (Instance.CurrentTime >= Instance.GhostData.TotalTime)
		{
			if (Instance.bLooping)
			{
				Instance.CurrentTime = 0.0f;
				Instance.CurrentFrameIndex = 0;
			}
			else
			{
				Instance.State = EMGGhostState::Finished;
				CompletedGhosts.Add(Pair.Key);
				continue;
			}
		}

		// Interpolate frame
		FMGGhostFrame Frame = InterpolateFrame(Instance.GhostData, Instance.CurrentTime);
		Instance.CurrentPosition = Frame.Position;
		Instance.CurrentRotation = Frame.Rotation;
		Instance.CurrentSpeed = Frame.Speed;
		Instance.CurrentFrameIndex = FindFrameIndex(Instance.GhostData, Instance.CurrentTime);
	}

	// Broadcast completed events
	for (const FGuid& ID : CompletedGhosts)
	{
		OnGhostPlaybackCompleted.Broadcast(ID);
	}
}

void UMGGhostSubsystem::UpdateComparison()
{
	if (!ActivePlaybacks.Contains(CurrentComparison.PlayerGhostID) ||
		!ActivePlaybacks.Contains(CurrentComparison.RivalGhostID))
	{
		return;
	}

	const FMGGhostInstance& Player = ActivePlaybacks[CurrentComparison.PlayerGhostID];
	const FMGGhostInstance& Rival = ActivePlaybacks[CurrentComparison.RivalGhostID];

	// Calculate time difference
	CurrentComparison.TimeDifference = Player.CurrentTime - Rival.CurrentTime;

	// Calculate distance difference based on track progress
	FMGGhostFrame PlayerFrame = GetCurrentFrame(CurrentComparison.PlayerGhostID);
	FMGGhostFrame RivalFrame = GetCurrentFrame(CurrentComparison.RivalGhostID);
	CurrentComparison.DistanceDifference = PlayerFrame.DistanceAlongTrack - RivalFrame.DistanceAlongTrack;

	// Determine status
	if (FMath::Abs(CurrentComparison.TimeDifference) < 0.1f)
	{
		CurrentComparison.Status = EMGGhostComparison::Even;
	}
	else if (CurrentComparison.TimeDifference > 0)
	{
		CurrentComparison.Status = EMGGhostComparison::Behind;
	}
	else
	{
		CurrentComparison.Status = EMGGhostComparison::Ahead;
	}

	OnGhostComparison.Broadcast(CurrentComparison, CurrentComparison.Status);
}

FMGGhostFrame UMGGhostSubsystem::InterpolateFrame(const FMGGhostData& GhostData, float Time) const
{
	if (GhostData.Frames.Num() == 0)
	{
		return FMGGhostFrame();
	}

	if (GhostData.Frames.Num() == 1)
	{
		return GhostData.Frames[0];
	}

	// Clamp time
	Time = FMath::Clamp(Time, 0.0f, GhostData.TotalTime);

	// Find surrounding frames
	int32 LowerIndex = 0;
	int32 UpperIndex = 0;

	for (int32 i = 0; i < GhostData.Frames.Num() - 1; i++)
	{
		if (GhostData.Frames[i].Timestamp <= Time &&
			GhostData.Frames[i + 1].Timestamp > Time)
		{
			LowerIndex = i;
			UpperIndex = i + 1;
			break;
		}
	}

	if (UpperIndex >= GhostData.Frames.Num())
	{
		return GhostData.Frames.Last();
	}

	const FMGGhostFrame& Lower = GhostData.Frames[LowerIndex];
	const FMGGhostFrame& Upper = GhostData.Frames[UpperIndex];

	float TimeDelta = Upper.Timestamp - Lower.Timestamp;
	float Alpha = (TimeDelta > 0) ? (Time - Lower.Timestamp) / TimeDelta : 0.0f;

	// Interpolate frame
	FMGGhostFrame Result;
	Result.Timestamp = Time;
	Result.Position = FMath::Lerp(Lower.Position, Upper.Position, Alpha);
	Result.Rotation = FMath::Lerp(Lower.Rotation, Upper.Rotation, Alpha);
	Result.Velocity = FMath::Lerp(Lower.Velocity, Upper.Velocity, Alpha);
	Result.Speed = FMath::Lerp(Lower.Speed, Upper.Speed, Alpha);
	Result.Throttle = FMath::Lerp(Lower.Throttle, Upper.Throttle, Alpha);
	Result.Brake = FMath::Lerp(Lower.Brake, Upper.Brake, Alpha);
	Result.Steering = FMath::Lerp(Lower.Steering, Upper.Steering, Alpha);
	Result.Gear = Alpha < 0.5f ? Lower.Gear : Upper.Gear;
	Result.EngineRPM = FMath::Lerp(Lower.EngineRPM, Upper.EngineRPM, Alpha);
	Result.bNitroActive = Alpha < 0.5f ? Lower.bNitroActive : Upper.bNitroActive;
	Result.bDrifting = Alpha < 0.5f ? Lower.bDrifting : Upper.bDrifting;
	Result.WheelFL = FMath::Lerp(Lower.WheelFL, Upper.WheelFL, Alpha);
	Result.WheelFR = FMath::Lerp(Lower.WheelFR, Upper.WheelFR, Alpha);
	Result.WheelRL = FMath::Lerp(Lower.WheelRL, Upper.WheelRL, Alpha);
	Result.WheelRR = FMath::Lerp(Lower.WheelRR, Upper.WheelRR, Alpha);
	Result.DistanceAlongTrack = FMath::Lerp(Lower.DistanceAlongTrack, Upper.DistanceAlongTrack, Alpha);
	Result.LapNumber = Lower.LapNumber;
	Result.Sector = Lower.Sector;

	return Result;
}

int32 UMGGhostSubsystem::FindFrameIndex(const FMGGhostData& GhostData, float Time) const
{
	for (int32 i = 0; i < GhostData.Frames.Num() - 1; i++)
	{
		if (GhostData.Frames[i].Timestamp <= Time &&
			GhostData.Frames[i + 1].Timestamp > Time)
		{
			return i;
		}
	}
	return GhostData.Frames.Num() - 1;
}

FGuid UMGGhostSubsystem::StartRecording(FName TrackID, FName VehicleID, FName PlayerID)
{
	FMGGhostData NewGhost;
	NewGhost.GhostID = FGuid::NewGuid();
	NewGhost.TrackID = TrackID;
	NewGhost.VehicleID = VehicleID;
	NewGhost.PlayerID = PlayerID;
	NewGhost.GhostType = EMGGhostType::Personal;
	NewGhost.RecordedDate = FDateTime::Now();
	NewGhost.GameVersion = TEXT("1.0.0");

	ActiveRecordings.Add(NewGhost.GhostID, NewGhost);
	ActiveRecordingID = NewGhost.GhostID;

	OnGhostRecordingStarted.Broadcast(NewGhost.GhostID, TrackID);

	return NewGhost.GhostID;
}

void UMGGhostSubsystem::StopRecording(FGuid GhostID)
{
	if (!ActiveRecordings.Contains(GhostID))
	{
		return;
	}

	FMGGhostData& GhostData = ActiveRecordings[GhostID];

	if (GhostData.Frames.Num() > 0)
	{
		GhostData.TotalTime = GhostData.Frames.Last().Timestamp;
	}

	// Compress if enabled
	if (Settings.bCompressGhostData)
	{
		CompressGhostData(GhostData);
	}

	GhostData.bValidated = true;

	// Check if new personal best
	if (Settings.bAutoSavePersonalBest)
	{
		if (!PersonalBests.Contains(GhostData.TrackID) ||
			GhostData.BestLapTime < PersonalBests[GhostData.TrackID].BestLapTime)
		{
			SetPersonalBest(GhostData.TrackID, GhostData);
			OnNewPersonalBest.Broadcast(GhostData.TrackID, GhostData.BestLapTime);
		}
	}

	float TotalTime = GhostData.TotalTime;

	// Save ghost
	SaveGhost(GhostData);

	ActiveRecordings.Remove(GhostID);

	if (ActiveRecordingID == GhostID)
	{
		ActiveRecordingID = FGuid();
	}

	OnGhostRecordingCompleted.Broadcast(GhostID, TotalTime);
}

void UMGGhostSubsystem::CancelRecording(FGuid GhostID)
{
	ActiveRecordings.Remove(GhostID);

	if (ActiveRecordingID == GhostID)
	{
		ActiveRecordingID = FGuid();
	}
}

void UMGGhostSubsystem::RecordFrame(FGuid GhostID, const FMGGhostFrame& Frame)
{
	if (!ActiveRecordings.Contains(GhostID))
	{
		return;
	}

	FMGGhostData& GhostData = ActiveRecordings[GhostID];

	// Only record at specified interval
	if (GhostData.Frames.Num() > 0)
	{
		float LastTime = GhostData.Frames.Last().Timestamp;
		if (Frame.Timestamp - LastTime < Settings.RecordingInterval)
		{
			return;
		}
	}

	GhostData.Frames.Add(Frame);
}

bool UMGGhostSubsystem::IsRecording() const
{
	return ActiveRecordingID.IsValid();
}

void UMGGhostSubsystem::MarkLapComplete(FGuid GhostID, float LapTime)
{
	if (!ActiveRecordings.Contains(GhostID))
	{
		return;
	}

	FMGGhostData& GhostData = ActiveRecordings[GhostID];
	GhostData.LapTimes.Add(LapTime);

	if (GhostData.BestLapTime == 0.0f || LapTime < GhostData.BestLapTime)
	{
		GhostData.BestLapTime = LapTime;
	}
}

void UMGGhostSubsystem::MarkSectorComplete(FGuid GhostID, int32 Sector, float SectorTime)
{
	if (!ActiveRecordings.Contains(GhostID))
	{
		return;
	}

	FMGGhostData& GhostData = ActiveRecordings[GhostID];

	while (GhostData.SectorTimes.Num() <= Sector)
	{
		GhostData.SectorTimes.Add(0.0f);
	}

	GhostData.SectorTimes[Sector] = SectorTime;
}

FGuid UMGGhostSubsystem::StartPlayback(const FMGGhostData& GhostData)
{
	// Check max ghosts limit
	if (ActivePlaybacks.Num() >= Settings.MaxGhostsOnTrack)
	{
		return FGuid();
	}

	FMGGhostInstance NewInstance;
	NewInstance.InstanceID = FGuid::NewGuid();
	NewInstance.GhostData = GhostData;
	NewInstance.State = EMGGhostState::Playing;
	NewInstance.CurrentTime = 0.0f;
	NewInstance.CurrentFrameIndex = 0;
	NewInstance.Visibility = Settings.DefaultVisibility;
	NewInstance.Opacity = Settings.GhostOpacity;
	NewInstance.PlaybackSpeed = 1.0f;
	NewInstance.bLooping = false;

	// Set color based on ghost type
	switch (GhostData.GhostType)
	{
		case EMGGhostType::Personal:
			NewInstance.GhostColor = Settings.PersonalBestColor;
			break;
		case EMGGhostType::WorldRecord:
			NewInstance.GhostColor = Settings.WorldRecordColor;
			break;
		case EMGGhostType::Rival:
			NewInstance.GhostColor = Settings.RivalColor;
			break;
		default:
			NewInstance.GhostColor = FLinearColor::White;
			break;
	}

	if (GhostData.Frames.Num() > 0)
	{
		NewInstance.CurrentPosition = GhostData.Frames[0].Position;
		NewInstance.CurrentRotation = GhostData.Frames[0].Rotation;
	}

	ActivePlaybacks.Add(NewInstance.InstanceID, NewInstance);

	OnGhostPlaybackStarted.Broadcast(NewInstance.InstanceID, GhostData);

	return NewInstance.InstanceID;
}

void UMGGhostSubsystem::StopPlayback(FGuid InstanceID)
{
	ActivePlaybacks.Remove(InstanceID);
}

void UMGGhostSubsystem::PausePlayback(FGuid InstanceID)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].State = EMGGhostState::Paused;
	}
}

void UMGGhostSubsystem::ResumePlayback(FGuid InstanceID)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].State = EMGGhostState::Playing;
	}
}

void UMGGhostSubsystem::SetPlaybackTime(FGuid InstanceID, float Time)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		FMGGhostInstance& Instance = ActivePlaybacks[InstanceID];
		Instance.CurrentTime = FMath::Clamp(Time, 0.0f, Instance.GhostData.TotalTime);
		Instance.CurrentFrameIndex = FindFrameIndex(Instance.GhostData, Instance.CurrentTime);
	}
}

void UMGGhostSubsystem::SetPlaybackSpeed(FGuid InstanceID, float Speed)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 4.0f);
	}
}

void UMGGhostSubsystem::SetLooping(FGuid InstanceID, bool bLoop)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].bLooping = bLoop;
	}
}

FMGGhostInstance UMGGhostSubsystem::GetGhostInstance(FGuid InstanceID) const
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		return ActivePlaybacks[InstanceID];
	}
	return FMGGhostInstance();
}

TArray<FMGGhostInstance> UMGGhostSubsystem::GetActiveGhosts() const
{
	TArray<FMGGhostInstance> Ghosts;
	for (const auto& Pair : ActivePlaybacks)
	{
		Ghosts.Add(Pair.Value);
	}
	return Ghosts;
}

FMGGhostFrame UMGGhostSubsystem::GetCurrentFrame(FGuid InstanceID) const
{
	if (!ActivePlaybacks.Contains(InstanceID))
	{
		return FMGGhostFrame();
	}

	const FMGGhostInstance& Instance = ActivePlaybacks[InstanceID];
	return InterpolateFrame(Instance.GhostData, Instance.CurrentTime);
}

bool UMGGhostSubsystem::SaveGhost(const FMGGhostData& GhostData)
{
	GhostCache.Add(GhostData.GhostID, GhostData);

	if (!GhostIndex.Contains(GhostData.GhostID))
	{
		GhostIndex.Add(GhostData.GhostID);
	}

	SaveGhostToFile(GhostData);

	return true;
}

bool UMGGhostSubsystem::LoadGhost(FGuid GhostID, FMGGhostData& OutGhostData)
{
	// Check cache first
	if (GhostCache.Contains(GhostID))
	{
		OutGhostData = GhostCache[GhostID];
		return true;
	}

	// Try loading from file
	if (LoadGhostFromFile(GhostID, OutGhostData))
	{
		GhostCache.Add(GhostID, OutGhostData);
		return true;
	}

	return false;
}

bool UMGGhostSubsystem::DeleteGhost(FGuid GhostID)
{
	GhostCache.Remove(GhostID);
	GhostIndex.Remove(GhostID);
	// Would also delete file
	return true;
}

TArray<FMGGhostData> UMGGhostSubsystem::GetSavedGhosts(FName TrackID) const
{
	TArray<FMGGhostData> Ghosts;

	for (const auto& Pair : GhostCache)
	{
		if (Pair.Value.TrackID == TrackID)
		{
			Ghosts.Add(Pair.Value);
		}
	}

	return Ghosts;
}

FMGGhostData UMGGhostSubsystem::GetPersonalBest(FName TrackID) const
{
	if (PersonalBests.Contains(TrackID))
	{
		return PersonalBests[TrackID];
	}
	return FMGGhostData();
}

void UMGGhostSubsystem::SetPersonalBest(FName TrackID, const FMGGhostData& GhostData)
{
	FMGGhostData PB = GhostData;
	PB.GhostType = EMGGhostType::Personal;
	PersonalBests.Add(TrackID, PB);
	SaveGhost(PB);
}

bool UMGGhostSubsystem::HasPersonalBest(FName TrackID) const
{
	return PersonalBests.Contains(TrackID);
}

void UMGGhostSubsystem::StartComparison(FGuid PlayerGhostID, FGuid RivalGhostID)
{
	CurrentComparison = FMGGhostComparator();
	CurrentComparison.PlayerGhostID = PlayerGhostID;
	CurrentComparison.RivalGhostID = RivalGhostID;
	bComparing = true;
}

void UMGGhostSubsystem::StopComparison()
{
	bComparing = false;
	CurrentComparison = FMGGhostComparator();
}

float UMGGhostSubsystem::GetTimeDelta(FGuid PlayerInstance, FGuid RivalInstance) const
{
	if (!ActivePlaybacks.Contains(PlayerInstance) || !ActivePlaybacks.Contains(RivalInstance))
	{
		return 0.0f;
	}

	return ActivePlaybacks[PlayerInstance].CurrentTime - ActivePlaybacks[RivalInstance].CurrentTime;
}

float UMGGhostSubsystem::GetDistanceDelta(FGuid PlayerInstance, FGuid RivalInstance) const
{
	FMGGhostFrame PlayerFrame = GetCurrentFrame(PlayerInstance);
	FMGGhostFrame RivalFrame = GetCurrentFrame(RivalInstance);
	return PlayerFrame.DistanceAlongTrack - RivalFrame.DistanceAlongTrack;
}

EMGGhostComparison UMGGhostSubsystem::GetComparisonStatus() const
{
	return CurrentComparison.Status;
}

void UMGGhostSubsystem::SetGhostVisibility(FGuid InstanceID, EMGGhostVisibility Visibility)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].Visibility = Visibility;
	}
}

void UMGGhostSubsystem::SetGhostColor(FGuid InstanceID, FLinearColor Color)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].GhostColor = Color;
	}
}

void UMGGhostSubsystem::SetGhostOpacity(FGuid InstanceID, float Opacity)
{
	if (ActivePlaybacks.Contains(InstanceID))
	{
		ActivePlaybacks[InstanceID].Opacity = FMath::Clamp(Opacity, 0.0f, 1.0f);
	}
}

void UMGGhostSubsystem::HideAllGhosts()
{
	for (auto& Pair : ActivePlaybacks)
	{
		Pair.Value.Visibility = EMGGhostVisibility::Hidden;
	}
}

void UMGGhostSubsystem::ShowAllGhosts()
{
	for (auto& Pair : ActivePlaybacks)
	{
		Pair.Value.Visibility = Settings.DefaultVisibility;
	}
}

void UMGGhostSubsystem::DownloadGhost(FGuid GhostID)
{
	// Create HTTP request to download ghost data
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	FString URL = FString::Printf(TEXT("%s/%s"), *MGGhostAPI::BaseURL, *GhostID.ToString());
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Handle response
	Request->OnProcessRequestComplete().BindWeakLambda(this,
		[this, GhostID](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			bool bDownloadSuccess = false;

			if (bSuccess && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				// Parse JSON response into ghost data
				FString ResponseBody = Resp->GetContentAsString();
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					FMGGhostData DownloadedGhost;
					if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &DownloadedGhost))
					{
						// Cache the downloaded ghost
						DownloadedGhosts.Add(GhostID, DownloadedGhost);
						bDownloadSuccess = true;
					}
				}
			}

			OnGhostDownloaded.Broadcast(GhostID, bDownloadSuccess);
		});

	Request->ProcessRequest();
}

void UMGGhostSubsystem::UploadGhost(const FMGGhostData& GhostData)
{
	// Serialize ghost data to JSON
	FString JsonString;
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(GhostData);
	if (!JsonObject.IsValid())
	{
		OnGhostUploaded.Broadcast(GhostData.GhostID, false);
		return;
	}

	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Create HTTP request to upload ghost
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	Request->SetURL(MGGhostAPI::BaseURL);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonString);

	// Handle response
	FGuid GhostID = GhostData.GhostID;
	Request->OnProcessRequestComplete().BindWeakLambda(this,
		[this, GhostID](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			bool bUploadSuccess = bSuccess && Resp.IsValid() &&
				(Resp->GetResponseCode() == 200 || Resp->GetResponseCode() == 201);

			OnGhostUploaded.Broadcast(GhostID, bUploadSuccess);
		});

	Request->ProcessRequest();
}

void UMGGhostSubsystem::FetchLeaderboard(FName TrackID, int32 StartRank, int32 Count)
{
	// Create HTTP request to fetch leaderboard
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	FString URL = FString::Printf(TEXT("%s/leaderboard/%s?start=%d&count=%d"),
		*MGGhostAPI::BaseURL, *TrackID.ToString(), StartRank, Count);
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Handle response
	Request->OnProcessRequestComplete().BindWeakLambda(this,
		[this, TrackID](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (bSuccess && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				FString ResponseBody = Resp->GetContentAsString();
				TArray<TSharedPtr<FJsonValue>> JsonArray;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

				if (FJsonSerializer::Deserialize(Reader, JsonArray))
				{
					TArray<FMGGhostLeaderboardEntry> Entries;
					for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
					{
						if (JsonValue->Type == EJson::Object)
						{
							FMGGhostLeaderboardEntry Entry;
							if (FJsonObjectConverter::JsonObjectToUStruct(
								JsonValue->AsObject().ToSharedRef(), &Entry))
							{
								Entries.Add(Entry);
							}
						}
					}

					// Cache leaderboard
					Leaderboards.Add(TrackID, Entries);

					// If this is the top entry, cache as world record
					if (Entries.Num() > 0)
					{
						DownloadGhost(Entries[0].GhostID);
					}
				}
			}

			OnLeaderboardFetched.Broadcast(TrackID);
		});

	Request->ProcessRequest();
}

TArray<FMGGhostLeaderboardEntry> UMGGhostSubsystem::GetLeaderboard(FName TrackID) const
{
	if (Leaderboards.Contains(TrackID))
	{
		return Leaderboards[TrackID];
	}
	return TArray<FMGGhostLeaderboardEntry>();
}

void UMGGhostSubsystem::DownloadRivalGhost(FName TrackID, int32 Rank)
{
	TArray<FMGGhostLeaderboardEntry> LB = GetLeaderboard(TrackID);
	if (LB.IsValidIndex(Rank))
	{
		DownloadGhost(LB[Rank].GhostID);
	}
}

FMGGhostData UMGGhostSubsystem::GetWorldRecord(FName TrackID) const
{
	if (WorldRecords.Contains(TrackID))
	{
		return WorldRecords[TrackID];
	}
	return FMGGhostData();
}

void UMGGhostSubsystem::SetGhostSettings(const FMGGhostSettings& NewSettings)
{
	Settings = NewSettings;

	// Apply visibility to active ghosts
	for (auto& Pair : ActivePlaybacks)
	{
		if (!Settings.bShowGhosts)
		{
			Pair.Value.Visibility = EMGGhostVisibility::Hidden;
		}
		else
		{
			Pair.Value.Visibility = Settings.DefaultVisibility;
		}
		Pair.Value.Opacity = Settings.GhostOpacity;
	}
}

void UMGGhostSubsystem::RacePersonalBest(FName TrackID)
{
	if (HasPersonalBest(TrackID))
	{
		FMGGhostData PB = GetPersonalBest(TrackID);
		StartPlayback(PB);
	}
}

void UMGGhostSubsystem::RaceWorldRecord(FName TrackID)
{
	if (WorldRecords.Contains(TrackID))
	{
		FMGGhostData WR = GetWorldRecord(TrackID);
		StartPlayback(WR);
	}
	else
	{
		// Fetch world record first
		FetchLeaderboard(TrackID, 0, 1);
	}
}

void UMGGhostSubsystem::RaceRival(FName TrackID, FName RivalID)
{
	// Fetch rival's ghost from server
	FHttpModule& HttpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

	FString URL = FString::Printf(TEXT("%s/rival/%s/%s"),
		*MGGhostAPI::BaseURL, *TrackID.ToString(), *RivalID.ToString());
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Handle response
	Request->OnProcessRequestComplete().BindWeakLambda(this,
		[this, TrackID, RivalID](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (bSuccess && Resp.IsValid() && Resp->GetResponseCode() == 200)
			{
				FString ResponseBody = Resp->GetContentAsString();
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					FMGGhostData RivalGhost;
					if (FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &RivalGhost))
					{
						// Start playback with rival ghost
						RivalGhost.GhostType = EMGGhostType::Rival;
						StartPlayback(RivalGhost);
						OnRivalGhostLoaded.Broadcast(TrackID, RivalID, true);
						return;
					}
				}
			}

			OnRivalGhostLoaded.Broadcast(TrackID, RivalID, false);
		});

	Request->ProcessRequest();
}

void UMGGhostSubsystem::ClearActiveGhosts()
{
	ActivePlaybacks.Empty();
}

void UMGGhostSubsystem::CompressGhostData(FMGGhostData& GhostData)
{
	// Simple compression: Remove redundant frames where position hasn't changed much
	if (GhostData.Frames.Num() < 3)
	{
		return;
	}

	TArray<FMGGhostFrame> CompressedFrames;
	CompressedFrames.Add(GhostData.Frames[0]); // Always keep first

	for (int32 i = 1; i < GhostData.Frames.Num() - 1; i++)
	{
		const FMGGhostFrame& Prev = CompressedFrames.Last();
		const FMGGhostFrame& Curr = GhostData.Frames[i];

		// Keep frame if significant change
		float PosDelta = FVector::Dist(Prev.Position, Curr.Position);
		float RotDelta = FMath::Abs(Prev.Rotation.Yaw - Curr.Rotation.Yaw);

		if (PosDelta > 1.0f || RotDelta > 5.0f)
		{
			CompressedFrames.Add(Curr);
		}
	}

	CompressedFrames.Add(GhostData.Frames.Last()); // Always keep last

	GhostData.CompressedSize = CompressedFrames.Num();
	GhostData.Frames = CompressedFrames;
}

void UMGGhostSubsystem::DecompressGhostData(FMGGhostData& GhostData)
{
	// Data is stored in a way that can be interpolated, so no decompression needed
	// Just mark as decompressed
}

void UMGGhostSubsystem::SaveGhostToFile(const FMGGhostData& GhostData)
{
	// Create ghosts directory if needed
	FString GhostsDir = FPaths::ProjectSavedDir() / TEXT("Ghosts");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*GhostsDir))
	{
		PlatformFile.CreateDirectory(*GhostsDir);
	}

	// Create file path using Ghost ID
	FString FilePath = GhostsDir / FString::Printf(TEXT("%s.ghost"), *GhostData.GhostID.ToString());

	// Serialize to buffer
	FBufferArchive Archive;

	// Write header/version
	int32 Version = 1;
	Archive << Version;

	// Write metadata
	FGuid GhostID = GhostData.GhostID;
	FName TrackID = GhostData.TrackID;
	FName VehicleID = GhostData.VehicleID;
	FName PlayerID = GhostData.PlayerID;
	FString PlayerNameStr = GhostData.PlayerName.ToString();
	int32 GhostTypeInt = static_cast<int32>(GhostData.GhostType);
	float TotalTime = GhostData.TotalTime;
	float BestLapTime = GhostData.BestLapTime;
	FDateTime RecordedDate = GhostData.RecordedDate;
	FString GameVersion = GhostData.GameVersion;
	bool bValidated = GhostData.bValidated;
	bool bIsWorldRecord = GhostData.bIsWorldRecord;

	Archive << GhostID;
	Archive << TrackID;
	Archive << VehicleID;
	Archive << PlayerID;
	Archive << PlayerNameStr;
	Archive << GhostTypeInt;
	Archive << TotalTime;
	Archive << BestLapTime;
	Archive << RecordedDate;
	Archive << GameVersion;
	Archive << bValidated;
	Archive << bIsWorldRecord;

	// Write lap times
	TArray<float> LapTimes = GhostData.LapTimes;
	Archive << LapTimes;

	// Write sector times
	TArray<float> SectorTimes = GhostData.SectorTimes;
	Archive << SectorTimes;

	// Write frames
	int32 FrameCount = GhostData.Frames.Num();
	Archive << FrameCount;

	for (const FMGGhostFrame& Frame : GhostData.Frames)
	{
		float Timestamp = Frame.Timestamp;
		FVector Position = Frame.Position;
		FRotator Rotation = Frame.Rotation;
		FVector Velocity = Frame.Velocity;
		float Speed = Frame.Speed;
		float Throttle = Frame.Throttle;
		float Brake = Frame.Brake;
		float Steering = Frame.Steering;
		int32 Gear = Frame.Gear;
		float EngineRPM = Frame.EngineRPM;
		bool bNitroActive = Frame.bNitroActive;
		bool bDrifting = Frame.bDrifting;
		float WheelFL = Frame.WheelFL;
		float WheelFR = Frame.WheelFR;
		float WheelRL = Frame.WheelRL;
		float WheelRR = Frame.WheelRR;
		float DistanceAlongTrack = Frame.DistanceAlongTrack;
		int32 LapNumber = Frame.LapNumber;
		int32 Sector = Frame.Sector;

		Archive << Timestamp;
		Archive << Position;
		Archive << Rotation;
		Archive << Velocity;
		Archive << Speed;
		Archive << Throttle;
		Archive << Brake;
		Archive << Steering;
		Archive << Gear;
		Archive << EngineRPM;
		Archive << bNitroActive;
		Archive << bDrifting;
		Archive << WheelFL;
		Archive << WheelFR;
		Archive << WheelRL;
		Archive << WheelRR;
		Archive << DistanceAlongTrack;
		Archive << LapNumber;
		Archive << Sector;
	}

	// Write to file
	FFileHelper::SaveArrayToFile(Archive, *FilePath);
	Archive.FlushCache();
	Archive.Empty();

	UE_LOG(LogTemp, Log, TEXT("MGGhost: Saved ghost %s to %s (%d frames)"),
		*GhostData.GhostID.ToString(), *FilePath, GhostData.Frames.Num());
}

bool UMGGhostSubsystem::LoadGhostFromFile(FGuid GhostID, FMGGhostData& OutData)
{
	FString GhostsDir = FPaths::ProjectSavedDir() / TEXT("Ghosts");
	FString FilePath = GhostsDir / FString::Printf(TEXT("%s.ghost"), *GhostID.ToString());

	// Load file data
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return false;
	}

	// Deserialize from buffer
	FMemoryReader Archive(FileData, true);

	// Read header/version
	int32 Version;
	Archive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGGhost: Unknown ghost file version %d"), Version);
		return false;
	}

	// Read metadata
	FGuid LoadedGhostID;
	FName TrackID;
	FName VehicleID;
	FName PlayerID;
	FString PlayerNameStr;
	int32 GhostTypeInt;
	float TotalTime;
	float BestLapTime;
	FDateTime RecordedDate;
	FString GameVersion;
	bool bValidated;
	bool bIsWorldRecord;

	Archive << LoadedGhostID;
	Archive << TrackID;
	Archive << VehicleID;
	Archive << PlayerID;
	Archive << PlayerNameStr;
	Archive << GhostTypeInt;
	Archive << TotalTime;
	Archive << BestLapTime;
	Archive << RecordedDate;
	Archive << GameVersion;
	Archive << bValidated;
	Archive << bIsWorldRecord;

	OutData.GhostID = LoadedGhostID;
	OutData.TrackID = TrackID;
	OutData.VehicleID = VehicleID;
	OutData.PlayerID = PlayerID;
	OutData.PlayerName = FText::FromString(PlayerNameStr);
	OutData.GhostType = static_cast<EMGGhostType>(GhostTypeInt);
	OutData.TotalTime = TotalTime;
	OutData.BestLapTime = BestLapTime;
	OutData.RecordedDate = RecordedDate;
	OutData.GameVersion = GameVersion;
	OutData.bValidated = bValidated;
	OutData.bIsWorldRecord = bIsWorldRecord;

	// Read lap times
	Archive << OutData.LapTimes;

	// Read sector times
	Archive << OutData.SectorTimes;

	// Read frames
	int32 FrameCount;
	Archive << FrameCount;

	OutData.Frames.Reserve(FrameCount);
	for (int32 i = 0; i < FrameCount; i++)
	{
		FMGGhostFrame Frame;

		Archive << Frame.Timestamp;
		Archive << Frame.Position;
		Archive << Frame.Rotation;
		Archive << Frame.Velocity;
		Archive << Frame.Speed;
		Archive << Frame.Throttle;
		Archive << Frame.Brake;
		Archive << Frame.Steering;
		Archive << Frame.Gear;
		Archive << Frame.EngineRPM;
		Archive << Frame.bNitroActive;
		Archive << Frame.bDrifting;
		Archive << Frame.WheelFL;
		Archive << Frame.WheelFR;
		Archive << Frame.WheelRL;
		Archive << Frame.WheelRR;
		Archive << Frame.DistanceAlongTrack;
		Archive << Frame.LapNumber;
		Archive << Frame.Sector;

		OutData.Frames.Add(Frame);
	}

	UE_LOG(LogTemp, Log, TEXT("MGGhost: Loaded ghost %s (%d frames)"),
		*GhostID.ToString(), FrameCount);

	return true;
}

void UMGGhostSubsystem::SaveGhostIndex()
{
	FString GhostsDir = FPaths::ProjectSavedDir() / TEXT("Ghosts");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*GhostsDir))
	{
		PlatformFile.CreateDirectory(*GhostsDir);
	}

	FString FilePath = GhostsDir / TEXT("ghost_index.dat");

	FBufferArchive Archive;

	// Write version
	int32 Version = 1;
	Archive << Version;

	// Write ghost index
	int32 IndexCount = GhostIndex.Num();
	Archive << IndexCount;
	for (const FGuid& ID : GhostIndex)
	{
		FGuid GhostID = ID;
		Archive << GhostID;
	}

	// Write personal bests metadata (just track -> ghost ID mapping)
	int32 PBCount = PersonalBests.Num();
	Archive << PBCount;
	for (const auto& Pair : PersonalBests)
	{
		FName TrackID = Pair.Key;
		FGuid GhostID = Pair.Value.GhostID;
		float BestLapTime = Pair.Value.BestLapTime;

		Archive << TrackID;
		Archive << GhostID;
		Archive << BestLapTime;
	}

	FFileHelper::SaveArrayToFile(Archive, *FilePath);
	Archive.FlushCache();
	Archive.Empty();

	UE_LOG(LogTemp, Log, TEXT("MGGhost: Saved ghost index (%d ghosts, %d personal bests)"),
		IndexCount, PBCount);
}

void UMGGhostSubsystem::LoadGhostIndex()
{
	FString GhostsDir = FPaths::ProjectSavedDir() / TEXT("Ghosts");
	FString FilePath = GhostsDir / TEXT("ghost_index.dat");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		// No index file yet, that's OK
		return;
	}

	FMemoryReader Archive(FileData, true);

	// Read version
	int32 Version;
	Archive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGGhost: Unknown ghost index version %d"), Version);
		return;
	}

	// Read ghost index
	int32 IndexCount;
	Archive << IndexCount;
	GhostIndex.Reserve(IndexCount);
	for (int32 i = 0; i < IndexCount; i++)
	{
		FGuid GhostID;
		Archive << GhostID;
		GhostIndex.Add(GhostID);
	}

	// Read personal bests metadata
	int32 PBCount;
	Archive << PBCount;
	for (int32 i = 0; i < PBCount; i++)
	{
		FName TrackID;
		FGuid GhostID;
		float BestLapTime;

		Archive << TrackID;
		Archive << GhostID;
		Archive << BestLapTime;

		// Try to load the full ghost data
		FMGGhostData GhostData;
		if (LoadGhostFromFile(GhostID, GhostData))
		{
			PersonalBests.Add(TrackID, GhostData);
			GhostCache.Add(GhostID, GhostData);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MGGhost: Loaded ghost index (%d ghosts, %d personal bests)"),
		IndexCount, PBCount);
}
