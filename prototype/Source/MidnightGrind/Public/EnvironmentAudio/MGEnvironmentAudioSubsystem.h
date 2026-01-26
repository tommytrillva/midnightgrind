// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGEnvironmentAudioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGEnvironmentType : uint8
{
	Urban,
	Downtown,
	Industrial,
	Residential,
	Highway,
	Tunnel,
	Bridge,
	Waterfront,
	Park,
	Suburbs,
	Commercial,
	Underground
};

UENUM(BlueprintType)
enum class EMGAmbientLayerType : uint8
{
	Base,
	Traffic,
	Pedestrian,
	Nature,
	Industrial,
	Weather,
	TimeOfDay,
	Special
};

UENUM(BlueprintType)
enum class EMGTimeOfDayAudio : uint8
{
	Dawn,
	Morning,
	Afternoon,
	Evening,
	Dusk,
	Night,
	LateNight
};

USTRUCT(BlueprintType)
struct FMGAmbientSoundLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAmbientLayerType LayerType = EMGAmbientLayerType::Base;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeInTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOutTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLooping = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpatialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectedBySpeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedVolumeMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGEnvironmentSoundscape
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SoundscapeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEnvironmentType EnvironmentType = EMGEnvironmentType::Urban;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGAmbientSoundLayer> Layers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FMGEnvironmentZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGEnvironmentSoundscape Soundscape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Extent = FVector(1000.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseBoxShape = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SphereRadius = 500.0f;
};

USTRUCT(BlueprintType)
struct FMGOneShot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OneShotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinInterval = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxInterval = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMin = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMax = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMin = 0.9f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMax = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGEnvironmentType> ValidEnvironments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGTimeOfDayAudio> ValidTimes;
};

USTRUCT(BlueprintType)
struct FMGEnvironmentAudioState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentZoneID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEnvironmentType CurrentEnvironment = EMGEnvironmentType::Urban;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTimeOfDayAudio TimeOfDay = EMGTimeOfDayAudio::Afternoon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRaining = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RainIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInsideVehicle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MasterVolume = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentZoneChanged, FName, OldZone, FName, NewZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, EMGTimeOfDayAudio, OldTime, EMGTimeOfDayAudio, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOneShotPlayed, FName, OneShotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeatherAudioChanged, float, RainIntensity);

UCLASS()
class MIDNIGHTGRIND_API UMGEnvironmentAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Zone Management
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Zone")
	void RegisterEnvironmentZone(const FMGEnvironmentZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Zone")
	void UnregisterEnvironmentZone(FName ZoneID);

	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Zone")
	FMGEnvironmentZone GetEnvironmentZone(FName ZoneID) const;

	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Zone")
	TArray<FMGEnvironmentZone> GetAllZones() const;

	// Soundscape Management
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void RegisterSoundscape(const FMGEnvironmentSoundscape& Soundscape);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void TransitionToSoundscape(FName SoundscapeID, float TransitionTime = 3.0f);

	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Soundscape")
	FMGEnvironmentSoundscape GetCurrentSoundscape() const { return CurrentSoundscape; }

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void SetLayerVolume(FName LayerID, float Volume, float FadeTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Soundscape")
	void MuteLayer(FName LayerID, bool bMute, float FadeTime = 1.0f);

	// One-Shots
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void RegisterOneShot(const FMGOneShot& OneShot);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void PlayOneShot(FName OneShotID, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void PlayRandomOneShot(EMGEnvironmentType Environment);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|OneShot")
	void SetOneShotEnabled(bool bEnabled);

	// State Updates
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void UpdateListenerLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void SetPlayerSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void SetTimeOfDay(EMGTimeOfDayAudio Time);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|State")
	void SetInsideVehicle(bool bInside);

	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|State")
	FMGEnvironmentAudioState GetAudioState() const { return AudioState; }

	// Weather Integration
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Weather")
	void SetRainIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Weather")
	void SetWindIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Weather")
	void TriggerThunder(float Distance, float Intensity = 1.0f);

	// Wind Audio
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Wind")
	void UpdateWindAudio(float Speed, FVector Direction);

	// Volume Control
	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Volume")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "EnvironmentAudio|Volume")
	float GetMasterVolume() const { return AudioState.MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Volume")
	void FadeOutAll(float FadeTime = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "EnvironmentAudio|Volume")
	void FadeInAll(float FadeTime = 2.0f);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnEnvironmentZoneChanged OnEnvironmentZoneChanged;

	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnOneShotPlayed OnOneShotPlayed;

	UPROPERTY(BlueprintAssignable, Category = "EnvironmentAudio|Events")
	FOnWeatherAudioChanged OnWeatherAudioChanged;

protected:
	void OnEnvironmentTick();
	void UpdateZoneTransitions();
	void UpdateLayerVolumes();
	void ProcessOneShots();
	void UpdateWeatherAudio();
	void UpdateSpeedBasedAudio();
	FMGEnvironmentZone FindZoneAtLocation(FVector Location) const;
	void InitializeDefaultSoundscapes();
	void PlayOneShotAtLocation(const FMGOneShot& OneShot, FVector Location);

	UPROPERTY()
	TMap<FName, FMGEnvironmentZone> EnvironmentZones;

	UPROPERTY()
	TMap<FName, FMGEnvironmentSoundscape> Soundscapes;

	UPROPERTY()
	TArray<FMGOneShot> OneShots;

	UPROPERTY()
	FMGEnvironmentSoundscape CurrentSoundscape;

	UPROPERTY()
	FMGEnvironmentAudioState AudioState;

	UPROPERTY()
	FVector ListenerLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bOneShotsEnabled = true;

	UPROPERTY()
	float OneShotTimer = 0.0f;

	UPROPERTY()
	float NextOneShotTime = 5.0f;

	UPROPERTY()
	TMap<FName, float> LayerVolumes;

	UPROPERTY()
	TMap<FName, float> TargetLayerVolumes;

	FTimerHandle EnvironmentTickHandle;
};
