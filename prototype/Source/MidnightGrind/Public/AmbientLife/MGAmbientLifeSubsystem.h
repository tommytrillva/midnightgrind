// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAmbientLifeSubsystem.generated.h"

/**
 * Ambient Life System - Living World
 * - Traffic management and AI
 * - Pedestrian crowds and behavior
 * - Ambient NPC vehicles (parked cars, car meets)
 * - Environment reactivity to player
 * - World population density by time/location
 */

UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
	None,
	Light,
	Medium,
	Heavy,
	Rush
};

UENUM(BlueprintType)
enum class EMGPedestrianBehavior : uint8
{
	Walking,
	Standing,
	Spectating,
	Fleeing,
	Cheering,
	Recording
};

USTRUCT(BlueprintType)
struct FMGTrafficSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrafficDensity Density = EMGTrafficDensity::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DensityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DespawnRadius = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxVehicles = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowTrucks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowMotorcycles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionLevel = 0.3f;
};

USTRUCT(BlueprintType)
struct FMGPedestrianSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DensityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPedestrians = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReactToRacing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecordPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CheerChance = 0.3f;
};

USTRUCT(BlueprintType)
struct FMGAmbientVehicle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsParked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsTraffic = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PaintColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGAmbientPedestrian
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PedestrianID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPedestrianBehavior Behavior = EMGPedestrianBehavior::Walking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPhone = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRecording = false;
};

USTRUCT(BlueprintType)
struct FMGCrowdZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetCrowdSize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCrowdSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRaceSpectators = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCarMeet = false;
};

USTRUCT(BlueprintType)
struct FMGWorldPopulation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveTrafficVehicles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActivePedestrians = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParkedVehicles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpectatorCrowds = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AveragePedestrianDensity = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPedestrianReaction, const FMGAmbientPedestrian&, Pedestrian, EMGPedestrianBehavior, NewBehavior);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrowdGathered, const FMGCrowdZone&, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTrafficDensityChanged, EMGTrafficDensity, OldDensity, EMGTrafficDensity, NewDensity);

UCLASS()
class MIDNIGHTGRIND_API UMGAmbientLifeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Traffic Control
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void SetTrafficSettings(const FMGTrafficSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "AmbientLife|Traffic")
	FMGTrafficSettings GetTrafficSettings() const { return TrafficSettings; }

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void SetTrafficDensity(EMGTrafficDensity Density);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void ClearTrafficInRadius(FVector Location, float Radius);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Traffic")
	void PauseTraffic(bool bPause);

	UFUNCTION(BlueprintPure, Category = "AmbientLife|Traffic")
	TArray<FMGAmbientVehicle> GetNearbyTraffic(FVector Location, float Radius) const;

	// Pedestrian Control
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Pedestrians")
	void SetPedestrianSettings(const FMGPedestrianSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "AmbientLife|Pedestrians")
	FMGPedestrianSettings GetPedestrianSettings() const { return PedestrianSettings; }

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Pedestrians")
	void TriggerPedestrianReaction(FVector Location, float Radius, EMGPedestrianBehavior Behavior);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Pedestrians")
	void ClearPedestriansInRadius(FVector Location, float Radius);

	UFUNCTION(BlueprintPure, Category = "AmbientLife|Pedestrians")
	TArray<FMGAmbientPedestrian> GetNearbyPedestrians(FVector Location, float Radius) const;

	// Crowd Zones
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Crowds")
	void CreateCrowdZone(const FMGCrowdZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Crowds")
	void RemoveCrowdZone(FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Crowds")
	void SetupRaceSpectators(const TArray<FVector>& SpectatorLocations);

	UFUNCTION(BlueprintPure, Category = "AmbientLife|Crowds")
	TArray<FMGCrowdZone> GetActiveCrowdZones() const { return CrowdZones; }

	// Parked Vehicles
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Parked")
	void SpawnParkedCars(FVector Location, int32 Count, float Radius);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Parked")
	void SetupCarMeetVehicles(FVector Location, const TArray<FName>& VehicleTypes);

	// World Population
	UFUNCTION(BlueprintPure, Category = "AmbientLife")
	FMGWorldPopulation GetWorldPopulation() const;

	UFUNCTION(BlueprintCallable, Category = "AmbientLife")
	void SetTimeOfDayMultipliers(float TrafficMult, float PedestrianMult);

	// Race Mode
	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Race")
	void OnRaceStarting(const TArray<FVector>& RaceRoute);

	UFUNCTION(BlueprintCallable, Category = "AmbientLife|Race")
	void OnRaceEnded();

	UFUNCTION(BlueprintPure, Category = "AmbientLife|Race")
	bool IsRaceActive() const { return bRaceActive; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "AmbientLife|Events")
	FMGOnPedestrianReaction OnPedestrianReaction;

	UPROPERTY(BlueprintAssignable, Category = "AmbientLife|Events")
	FMGOnCrowdGathered OnCrowdGathered;

	UPROPERTY(BlueprintAssignable, Category = "AmbientLife|Events")
	FMGOnTrafficDensityChanged OnTrafficDensityChanged;

protected:
	void UpdateAmbientLife(float DeltaTime);
	void UpdateCrowds(float DeltaTime);
	void SpawnTrafficVehicle();
	void SpawnPedestrian();
	int32 GetTargetTrafficCount() const;
	int32 GetTargetPedestrianCount() const;

private:
	UPROPERTY()
	TArray<FMGAmbientVehicle> TrafficVehicles;

	UPROPERTY()
	TArray<FMGAmbientVehicle> ParkedVehicles;

	UPROPERTY()
	TArray<FMGAmbientPedestrian> Pedestrians;

	UPROPERTY()
	TArray<FMGCrowdZone> CrowdZones;

	FMGTrafficSettings TrafficSettings;
	FMGPedestrianSettings PedestrianSettings;
	FTimerHandle UpdateTimerHandle;
	float TimeOfDayTrafficMultiplier = 1.0f;
	float TimeOfDayPedestrianMultiplier = 1.0f;
	bool bTrafficPaused = false;
	bool bRaceActive = false;
	TArray<FVector> CurrentRaceRoute;
};
