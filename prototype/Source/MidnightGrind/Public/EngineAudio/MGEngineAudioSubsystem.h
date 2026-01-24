// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGEngineAudioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGEngineType : uint8
{
	Inline4,
	Inline6,
	V6,
	V8,
	V10,
	V12,
	Flat4,
	Flat6,
	Rotary,
	Electric,
	Hybrid,
	Turbocharged,
	Supercharged,
	TwinTurbo
};

UENUM(BlueprintType)
enum class EMGExhaustType : uint8
{
	Stock,
	Sport,
	Performance,
	Racing,
	Straight,
	Catback,
	Headers,
	Custom
};

UENUM(BlueprintType)
enum class EMGEngineState : uint8
{
	Off,
	Starting,
	Idle,
	Revving,
	OnThrottle,
	OffThrottle,
	Redline,
	Backfire,
	Shifting,
	Stalling
};

USTRUCT(BlueprintType)
struct FMGEngineSoundLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRPM = 8000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinPitch = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxPitch = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrossfadeWidth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FMGEngineAudioProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ProfileID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEngineType EngineType = EMGEngineType::V8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGExhaustType ExhaustType = EMGExhaustType::Stock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleRPM = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RedlineRPM = 7000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RevLimiterRPM = 7200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEngineSoundLayer> OnThrottleLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGEngineSoundLayer> OffThrottleLayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> StartupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ShutdownSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BackfireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> RevLimiterSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> TurboSpoolSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> TurboBlowoffSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SuperchargerWhineSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurboLag = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BackfireChance = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExhaustPop = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rumble = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BassPunch = 0.5f;
};

USTRUCT(BlueprintType)
struct FMGEngineAudioState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetRPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Load = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentGear = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurboBoost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRevLimited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBackfiring = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEngineState State = EMGEngineState::Off;
};

USTRUCT(BlueprintType)
struct FMGTransmissionAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> GearChangeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> GearGrindSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShiftTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPMDropOnShift = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSequential = false;
};

USTRUCT(BlueprintType)
struct FMGVehicleAudioInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEngineAudioProfile Profile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEngineAudioState State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayerVehicle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToListener = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Audibility = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEngineStateChanged, FName, VehicleID, EMGEngineState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGearChanged, FName, VehicleID, int32, NewGear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackfire, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRevLimiterHit, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurboBlowoff, FName, VehicleID);

UCLASS()
class MIDNIGHTGRIND_API UMGEngineAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Vehicle")
	void RegisterVehicle(FName VehicleID, const FMGEngineAudioProfile& Profile, bool bIsPlayer = false);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Vehicle")
	void UnregisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Vehicle")
	void SetPlayerVehicle(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Vehicle")
	FName GetPlayerVehicleID() const { return PlayerVehicleID; }

	// Engine State Updates
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void UpdateEngineState(FName VehicleID, const FMGEngineAudioState& State);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetRPM(FName VehicleID, float RPM);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetThrottle(FName VehicleID, float ThrottleInput);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetGear(FName VehicleID, int32 Gear);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetTurboBoost(FName VehicleID, float Boost);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|State")
	void SetVehicleLocation(FName VehicleID, FVector Location);

	// Engine Actions
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void StartEngine(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void StopEngine(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void Rev(FName VehicleID, float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void TriggerBackfire(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void TriggerTurboBlowoff(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Actions")
	void TriggerGearShift(FName VehicleID, int32 FromGear, int32 ToGear);

	// Queries
	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	FMGEngineAudioState GetEngineState(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	FMGEngineAudioProfile GetEngineProfile(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	TArray<FName> GetActiveVehicles() const;

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	float GetCurrentRPM(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Query")
	bool IsEngineRunning(FName VehicleID) const;

	// Profile Management
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Profile")
	void RegisterProfile(const FMGEngineAudioProfile& Profile);

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Profile")
	FMGEngineAudioProfile GetProfile(FName ProfileID) const;

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Profile")
	void ApplyExhaustUpgrade(FName VehicleID, EMGExhaustType ExhaustType);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Profile")
	void ApplyTurboUpgrade(FName VehicleID, bool bTurbo, float BoostPressure = 1.0f);

	// Listener
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Listener")
	void SetListenerLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Listener")
	void SetInteriorMode(bool bInterior);

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Listener")
	bool IsInteriorMode() const { return bInteriorMode; }

	// Settings
	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Settings")
	void SetMaxAudibleVehicles(int32 MaxVehicles);

	UFUNCTION(BlueprintCallable, Category = "EngineAudio|Settings")
	void SetEngineVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "EngineAudio|Settings")
	float GetEngineVolume() const { return EngineVolume; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnEngineStateChanged OnEngineStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnGearChanged OnGearChanged;

	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnBackfire OnBackfire;

	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnRevLimiterHit OnRevLimiterHit;

	UPROPERTY(BlueprintAssignable, Category = "EngineAudio|Events")
	FOnTurboBlowoff OnTurboBlowoff;

protected:
	void OnEngineTick();
	void UpdateVehicleAudio(FMGVehicleAudioInstance& Instance);
	void CalculateAudibility();
	void ProcessRPMInterpolation(FMGVehicleAudioInstance& Instance, float DeltaTime);
	void ProcessBackfireChance(FMGVehicleAudioInstance& Instance);
	void PlayEngineLayer(FMGVehicleAudioInstance& Instance, const FMGEngineSoundLayer& Layer, float Volume);
	float CalculatePitchFromRPM(float RPM, const FMGEngineSoundLayer& Layer) const;
	float CalculateLayerVolume(float RPM, const FMGEngineSoundLayer& Layer) const;
	void InitializeDefaultProfiles();

	UPROPERTY()
	TMap<FName, FMGVehicleAudioInstance> ActiveVehicles;

	UPROPERTY()
	TMap<FName, FMGEngineAudioProfile> RegisteredProfiles;

	UPROPERTY()
	FName PlayerVehicleID;

	UPROPERTY()
	FVector ListenerLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bInteriorMode = false;

	UPROPERTY()
	int32 MaxAudibleVehicles = 8;

	UPROPERTY()
	float EngineVolume = 1.0f;

	UPROPERTY()
	float MaxAudibleDistance = 5000.0f;

	UPROPERTY()
	float RPMInterpolationSpeed = 5.0f;

	FTimerHandle EngineTickHandle;
};
