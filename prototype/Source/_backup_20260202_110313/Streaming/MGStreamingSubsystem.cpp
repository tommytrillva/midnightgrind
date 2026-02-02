// Copyright Midnight Grind. All Rights Reserved.

#include "Streaming/MGStreamingSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/StreamableManager.h"

void UMGStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Stats.MemoryBudgetMB = 2048; // 2GB default budget

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ProcessTimerHandle,
			this,
			&UMGStreamingSubsystem::ProcessQueue,
			0.1f,
			true
		);
	}
}

void UMGStreamingSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProcessTimerHandle);
	}

	// Unload all assets
	LoadedAssets.Empty();

	Super::Deinitialize();
}

void UMGStreamingSubsystem::RequestLoad(const FMGStreamingRequest& Request)
{
	if (!bStreamingEnabled)
		return;

	// Check if already loaded or pending
	if (IsLoaded(Request.AssetID))
		return;

	for (const FMGStreamingRequest& Pending : PendingRequests)
	{
		if (Pending.AssetID == Request.AssetID)
			return;
	}

	// Add to queue based on priority
	int32 InsertIndex = PendingRequests.Num();
	for (int32 i = 0; i < PendingRequests.Num(); i++)
	{
		if (Request.Priority < PendingRequests[i].Priority)
		{
			InsertIndex = i;
			break;
		}
	}

	PendingRequests.Insert(Request, InsertIndex);
	UpdateStats();
}

void UMGStreamingSubsystem::RequestUnload(FName AssetID)
{
	// Remove from loaded assets
	if (LoadedAssets.Remove(AssetID) > 0)
	{
		OnAssetUnloaded.Broadcast(AssetID);
		UpdateStats();
	}

	// Remove from pending if exists
	PendingRequests.RemoveAll([AssetID](const FMGStreamingRequest& R) { return R.AssetID == AssetID; });
}

void UMGStreamingSubsystem::SetPriority(FName AssetID, EMGStreamingPriority Priority)
{
	for (FMGStreamingRequest& Request : PendingRequests)
	{
		if (Request.AssetID == AssetID)
		{
			Request.Priority = Priority;
			break;
		}
	}

	// Re-sort queue
	PendingRequests.Sort([](const FMGStreamingRequest& A, const FMGStreamingRequest& B)
	{
		return A.Priority < B.Priority;
	});
}

bool UMGStreamingSubsystem::IsLoaded(FName AssetID) const
{
	return LoadedAssets.Contains(AssetID);
}

float UMGStreamingSubsystem::GetLoadProgress(FName AssetID) const
{
	if (IsLoaded(AssetID))
		return 1.0f;

	for (const FMGStreamingRequest& Request : ActiveLoads)
	{
		if (Request.AssetID == AssetID)
			return Request.LoadProgress;
	}

	return 0.0f;
}

void UMGStreamingSubsystem::PreloadTrackSection(FName TrackID, int32 SectionIndex)
{
	FMGStreamingRequest Request;
	Request.AssetID = FName(*FString::Printf(TEXT("%s_Section_%d"), *TrackID.ToString(), SectionIndex));
	Request.AssetType = EMGAssetType::Track;
	Request.Priority = EMGStreamingPriority::High;
	RequestLoad(Request);
}

void UMGStreamingSubsystem::UpdatePlayerPosition(FVector Position, FVector Velocity)
{
	LastPlayerPosition = Position;
	PredictRequiredAssets(Position, Velocity);
}

void UMGStreamingSubsystem::SetTrackStreamingRadius(float Radius)
{
	TrackStreamingRadius = FMath::Max(100.0f, Radius);
}

void UMGStreamingSubsystem::PreloadVehicle(FName VehicleID)
{
	FMGStreamingRequest Request;
	Request.AssetID = VehicleID;
	Request.AssetType = EMGAssetType::Vehicle;
	Request.Priority = EMGStreamingPriority::High;
	RequestLoad(Request);
}

void UMGStreamingSubsystem::PreloadVehicleLivery(FName VehicleID, FName LiveryID)
{
	FMGStreamingRequest Request;
	Request.AssetID = FName(*FString::Printf(TEXT("%s_%s"), *VehicleID.ToString(), *LiveryID.ToString()));
	Request.AssetType = EMGAssetType::Texture;
	Request.Priority = EMGStreamingPriority::Normal;
	RequestLoad(Request);
}

void UMGStreamingSubsystem::SetMaxConcurrentLoads(int32 MaxLoads)
{
	MaxConcurrentLoads = FMath::Clamp(MaxLoads, 1, 16);
}

void UMGStreamingSubsystem::SetStreamingEnabled(bool bEnabled)
{
	bStreamingEnabled = bEnabled;
}

void UMGStreamingSubsystem::ProcessQueue()
{
	if (!bStreamingEnabled)
		return;

	// Process completed loads
	for (int32 i = ActiveLoads.Num() - 1; i >= 0; i--)
	{
		FMGStreamingRequest& Load = ActiveLoads[i];

		// Simulate load progress
		Load.LoadProgress = FMath::Min(1.0f, Load.LoadProgress + 0.2f);
		OnLoadProgress.Broadcast(Load.AssetID, Load.LoadProgress);

		if (Load.LoadProgress >= 1.0f)
		{
			Load.bIsLoaded = true;
			LoadedAssets.Add(Load.AssetID, nullptr); // Would store actual asset
			OnAssetLoaded.Broadcast(Load.AssetID);
			ActiveLoads.RemoveAt(i);
		}
	}

	// Start new loads
	while (ActiveLoads.Num() < MaxConcurrentLoads && PendingRequests.Num() > 0)
	{
		FMGStreamingRequest Request = PendingRequests[0];
		PendingRequests.RemoveAt(0);

		// Would start actual async load here
		Request.LoadProgress = 0.0f;
		ActiveLoads.Add(Request);
	}

	UpdateStats();
}

void UMGStreamingSubsystem::UpdateStats()
{
	Stats.PendingRequests = PendingRequests.Num();
	Stats.ActiveLoads = ActiveLoads.Num();
	// Would calculate actual memory usage
}

void UMGStreamingSubsystem::PredictRequiredAssets(FVector Position, FVector Velocity)
{
	// Predict where player will be in 2-3 seconds
	FVector PredictedPosition = Position + (Velocity * 3.0f);

	// Would query world for assets near predicted position
	// and request preloading of those assets
}
