// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStreamingSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGStreamingPriority : uint8
{
	Critical,    // Must be loaded (player vehicle, nearby track)
	High,        // Should be loaded soon (upcoming track sections)
	Normal,      // Standard streaming
	Low,         // Background loading
	Preload      // Speculative loading
};

UENUM(BlueprintType)
enum class EMGAssetType : uint8
{
	Track,
	Vehicle,
	Environment,
	Audio,
	Texture,
	Animation
};

USTRUCT(BlueprintType)
struct FMGStreamingRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AssetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAssetType AssetType = EMGAssetType::Track;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGStreamingPriority Priority = EMGStreamingPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> AssetPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLoaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LoadProgress = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGStreamingStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PendingRequests = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ActiveLoads = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 MemoryUsedMB = 0;

	UPROPERTY(BlueprintReadOnly)
	int64 MemoryBudgetMB = 0;

	UPROPERTY(BlueprintReadOnly)
	float BandwidthUsage = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAssetLoaded, FName, AssetID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAssetUnloaded, FName, AssetID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLoadProgress, FName, AssetID, float, Progress);

UCLASS()
class MIDNIGHTGRIND_API UMGStreamingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Request Management
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RequestLoad(const FMGStreamingRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void RequestUnload(FName AssetID);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetPriority(FName AssetID, EMGStreamingPriority Priority);

	UFUNCTION(BlueprintPure, Category = "Streaming")
	bool IsLoaded(FName AssetID) const;

	UFUNCTION(BlueprintPure, Category = "Streaming")
	float GetLoadProgress(FName AssetID) const;

	// Track Streaming
	UFUNCTION(BlueprintCallable, Category = "Streaming|Track")
	void PreloadTrackSection(FName TrackID, int32 SectionIndex);

	UFUNCTION(BlueprintCallable, Category = "Streaming|Track")
	void UpdatePlayerPosition(FVector Position, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "Streaming|Track")
	void SetTrackStreamingRadius(float Radius);

	// Vehicle Streaming
	UFUNCTION(BlueprintCallable, Category = "Streaming|Vehicle")
	void PreloadVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Streaming|Vehicle")
	void PreloadVehicleLivery(FName VehicleID, FName LiveryID);

	// Bandwidth Control
	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetMaxConcurrentLoads(int32 MaxLoads);

	UFUNCTION(BlueprintCallable, Category = "Streaming")
	void SetStreamingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Streaming")
	bool IsStreamingEnabled() const { return bStreamingEnabled; }

	// Stats
	UFUNCTION(BlueprintPure, Category = "Streaming")
	FMGStreamingStats GetStats() const { return Stats; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Streaming|Events")
	FMGOnAssetLoaded OnAssetLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Streaming|Events")
	FMGOnAssetUnloaded OnAssetUnloaded;

	UPROPERTY(BlueprintAssignable, Category = "Streaming|Events")
	FMGOnLoadProgress OnLoadProgress;

protected:
	void ProcessQueue();
	void UpdateStats();
	void PredictRequiredAssets(FVector Position, FVector Velocity);

private:
	UPROPERTY()
	TArray<FMGStreamingRequest> PendingRequests;

	UPROPERTY()
	TArray<FMGStreamingRequest> ActiveLoads;

	UPROPERTY()
	TMap<FName, UObject*> LoadedAssets;

	FMGStreamingStats Stats;
	bool bStreamingEnabled = true;
	int32 MaxConcurrentLoads = 4;
	float TrackStreamingRadius = 500.0f;
	FVector LastPlayerPosition;
	FTimerHandle ProcessTimerHandle;
};
