// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGWorldEventsSubsystem.generated.h"

/**
 * World Events System - Dynamic World Encounters
 * - Random world encounters while free roaming
 * - Impromptu street races when rivals appear
 * - Police encounters and chases
 * - Street meets and gatherings
 * - Special time-based events
 */

UENUM(BlueprintType)
enum class EMGWorldEventType : uint8
{
	StreetRace,          // Random racer challenges you
	StreetMeet,          // Gathering of cars
	PoliceChase,         // Cops spot you
	RivalAppearance,     // Your rival shows up
	HiddenRace,          // Secret race starting
	TimeAttack,          // Beat the clock challenge
	DrivebyChallenge,    // Beat a specific score
	SpecialVehicle,      // Rare car spotted
	MechanicShop,        // Pop-up tune shop
	Underground          // Secret underground meet
};

UENUM(BlueprintType)
enum class EMGWorldEventState : uint8
{
	Pending,
	Active,
	PlayerEngaged,
	Completed,
	Failed,
	Expired
};

USTRUCT(BlueprintType)
struct FMGWorldEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWorldEventType Type = EMGWorldEventType::StreetRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWorldEventState State = EMGWorldEventState::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RadiusMeters = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CashReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ReputationReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedNPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RelatedTrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresProximity = true;
};

USTRUCT(BlueprintType)
struct FMGStreetMeet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MeetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAttendees = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttendees = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> FeaturedVehicleTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRacesStartingSoon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilDispersal = 1800.0f; // 30 minutes
};

USTRUCT(BlueprintType)
struct FMGPoliceEncounter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EncounterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PursuitUnits = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterDeployed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRoadblocksActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeInPursuit = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGWorldEventSpawnSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpawnDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpawnDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EventSpawnCooldown = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxConcurrentEvents = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoliceSpawnChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RivalSpawnChance = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreetRaceChance = 0.3f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnWorldEventSpawned, const FMGWorldEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnWorldEventStateChanged, const FMGWorldEvent&, Event, EMGWorldEventState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStreetMeetFound, const FMGStreetMeet&, Meet);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPoliceEncounterStarted, const FMGPoliceEncounter&, Encounter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnPoliceEscaped);

UCLASS()
class MIDNIGHTGRIND_API UMGWorldEventsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Event Queries
	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	TArray<FMGWorldEvent> GetActiveEvents() const;

	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	TArray<FMGWorldEvent> GetNearbyEvents(FVector Location, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	FMGWorldEvent GetEvent(const FString& EventID) const;

	UFUNCTION(BlueprintPure, Category = "WorldEvents")
	bool HasActiveEventOfType(EMGWorldEventType Type) const;

	// Event Interaction
	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	bool JoinEvent(const FString& EventID);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	void LeaveEvent(const FString& EventID);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents")
	void CompleteEvent(const FString& EventID, bool bSuccess);

	// Event Spawning
	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Spawn")
	void SpawnEvent(EMGWorldEventType Type, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Spawn")
	void ForceSpawnNearPlayer(EMGWorldEventType Type);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Spawn")
	void SetSpawnSettings(const FMGWorldEventSpawnSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "WorldEvents|Spawn")
	FMGWorldEventSpawnSettings GetSpawnSettings() const { return SpawnSettings; }

	// Street Meets
	UFUNCTION(BlueprintPure, Category = "WorldEvents|Meets")
	TArray<FMGStreetMeet> GetActiveStreetMeets() const { return ActiveStreetMeets; }

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Meets")
	void JoinStreetMeet(const FString& MeetID);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Meets")
	void LeaveStreetMeet(const FString& MeetID);

	// Police
	UFUNCTION(BlueprintPure, Category = "WorldEvents|Police")
	bool IsInPoliceChase() const { return CurrentPoliceEncounter.PursuitUnits > 0; }

	UFUNCTION(BlueprintPure, Category = "WorldEvents|Police")
	FMGPoliceEncounter GetPoliceEncounter() const { return CurrentPoliceEncounter; }

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Police")
	void TriggerPoliceChase(int32 InitialHeat = 1);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Police")
	void IncreaseHeat(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "WorldEvents|Police")
	void UpdateEscapeProgress(float Progress);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnWorldEventSpawned OnWorldEventSpawned;

	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnWorldEventStateChanged OnWorldEventStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnStreetMeetFound OnStreetMeetFound;

	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnPoliceEncounterStarted OnPoliceEncounterStarted;

	UPROPERTY(BlueprintAssignable, Category = "WorldEvents|Events")
	FMGOnPoliceEscaped OnPoliceEscaped;

protected:
	void UpdateEvents(float DeltaTime);
	void UpdatePoliceChase(float DeltaTime);
	void TrySpawnRandomEvent();
	void CleanupExpiredEvents();
	FMGWorldEvent GenerateRandomEvent(EMGWorldEventType Type, FVector Location);

private:
	UPROPERTY()
	TArray<FMGWorldEvent> ActiveEvents;

	UPROPERTY()
	TArray<FMGStreetMeet> ActiveStreetMeets;

	FMGPoliceEncounter CurrentPoliceEncounter;
	FMGWorldEventSpawnSettings SpawnSettings;
	FTimerHandle EventUpdateHandle;
	float TimeSinceLastSpawn = 0.0f;
	FVector LastPlayerLocation = FVector::ZeroVector;
};
