// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGSpatialAudioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGAcousticEnvironment : uint8
{
	Outdoor,
	Street,
	Tunnel,
	Underpass,
	Garage,
	Parking,
	Highway,
	Forest,
	Urban,
	Industrial,
	Downtown,
	Waterfront
};

UENUM(BlueprintType)
enum class EMGSoundPriority : uint8
{
	Background,
	Low,
	Normal,
	High,
	Critical,
	Player
};

UENUM(BlueprintType)
enum class EMGOcclusionType : uint8
{
	None,
	Partial,
	Full,
	Dynamic
};

USTRUCT(BlueprintType)
struct FMGAcousticZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAcousticEnvironment Environment = EMGAcousticEnvironment::Outdoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Extent = FVector(500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDecay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbWetLevel = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowPassFrequency = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EchoDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OcclusionFactor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FMGSpatialSoundSource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSoundPriority Priority = EMGSoundPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGOcclusionType OcclusionType = EMGOcclusionType::Dynamic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentOcclusion = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDopplerEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DopplerFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpatialized = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpatialBlend = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FMGListenerState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentZoneID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAcousticEnvironment CurrentEnvironment = EMGAcousticEnvironment::Outdoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInsideVehicle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGAudioReflection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ReflectionPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Normal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Delay = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGSpatialAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOcclusionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReflectionsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDopplerEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DopplerScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedOfSound = 34300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxReflections = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OcclusionUpdateRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxActiveSources = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttenuationScale = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAcousticZoneChanged, FName, OldZoneID, FName, NewZoneID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentChanged, EMGAcousticEnvironment, OldEnv, EMGAcousticEnvironment, NewEnv);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoundOccluded, FName, SourceID, float, OcclusionAmount);

UCLASS()
class MIDNIGHTGRIND_API UMGSpatialAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Zone Management
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Zone")
	void RegisterAcousticZone(const FMGAcousticZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Zone")
	void UnregisterAcousticZone(FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Zone")
	void UpdateAcousticZone(const FMGAcousticZone& Zone);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Zone")
	FMGAcousticZone GetAcousticZone(FName ZoneID) const;

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Zone")
	TArray<FMGAcousticZone> GetAllAcousticZones() const;

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Zone")
	FMGAcousticZone GetZoneAtLocation(FVector Location) const;

	// Sound Source Management
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void RegisterSoundSource(const FMGSpatialSoundSource& Source);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void UnregisterSoundSource(FName SourceID);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void UpdateSoundSource(FName SourceID, FVector Location, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void SetSourceVolume(FName SourceID, float Volume);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Source")
	void SetSourceActive(FName SourceID, bool bActive);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Source")
	FMGSpatialSoundSource GetSoundSource(FName SourceID) const;

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Source")
	TArray<FName> GetActiveSoundSources() const;

	// Listener
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Listener")
	void UpdateListener(FVector Location, FRotator Rotation, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Listener")
	void SetListenerInsideVehicle(bool bInside);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Listener")
	FMGListenerState GetListenerState() const { return ListenerState; }

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Listener")
	FName GetCurrentAcousticZone() const { return ListenerState.CurrentZoneID; }

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Listener")
	EMGAcousticEnvironment GetCurrentEnvironment() const { return ListenerState.CurrentEnvironment; }

	// Occlusion
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Occlusion")
	void SetOcclusionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Occlusion")
	bool IsOcclusionEnabled() const { return Settings.bOcclusionEnabled; }

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Occlusion")
	float GetOcclusionForSource(FName SourceID) const;

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Occlusion")
	void ForceOcclusionUpdate();

	// Doppler
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Doppler")
	void SetDopplerEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Doppler")
	bool IsDopplerEnabled() const { return Settings.bDopplerEnabled; }

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Doppler")
	void SetDopplerScale(float Scale);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Doppler")
	float CalculateDopplerPitch(FName SourceID) const;

	// Reflections
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Reflections")
	void SetReflectionsEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Reflections")
	bool AreReflectionsEnabled() const { return Settings.bReflectionsEnabled; }

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Reflections")
	TArray<FMGAudioReflection> GetReflectionsForSource(FName SourceID) const;

	// Environment Presets
	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Environment")
	void ApplyEnvironmentPreset(EMGAcousticEnvironment Environment, float TransitionTime = 0.5f);

	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Environment")
	FMGAcousticZone GetEnvironmentPreset(EMGAcousticEnvironment Environment) const;

	// Distance Attenuation
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Attenuation")
	float CalculateAttenuation(FName SourceID) const;

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Attenuation")
	void SetAttenuationScale(float Scale);

	// Settings
	UFUNCTION(BlueprintPure, Category = "SpatialAudio|Settings")
	FMGSpatialAudioSettings GetSettings() const { return Settings; }

	UFUNCTION(BlueprintCallable, Category = "SpatialAudio|Settings")
	void UpdateSettings(const FMGSpatialAudioSettings& NewSettings);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "SpatialAudio|Events")
	FOnAcousticZoneChanged OnAcousticZoneChanged;

	UPROPERTY(BlueprintAssignable, Category = "SpatialAudio|Events")
	FOnEnvironmentChanged OnEnvironmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "SpatialAudio|Events")
	FOnSoundOccluded OnSoundOccluded;

protected:
	void OnSpatialTick();
	void UpdateOcclusion();
	void UpdateZoneTransitions();
	void ApplyZoneEffects(const FMGAcousticZone& Zone);
	void CalculateReflections(FMGSpatialSoundSource& Source);
	float CalculateOcclusionForSource(const FMGSpatialSoundSource& Source) const;
	void SortSourcesByPriority();
	void InitializeEnvironmentPresets();

	UPROPERTY()
	TMap<FName, FMGAcousticZone> AcousticZones;

	UPROPERTY()
	TMap<FName, FMGSpatialSoundSource> SoundSources;

	UPROPERTY()
	TMap<FName, TArray<FMGAudioReflection>> SourceReflections;

	UPROPERTY()
	FMGListenerState ListenerState;

	UPROPERTY()
	FMGSpatialAudioSettings Settings;

	UPROPERTY()
	TMap<EMGAcousticEnvironment, FMGAcousticZone> EnvironmentPresets;

	UPROPERTY()
	float OcclusionUpdateTimer = 0.0f;

	FTimerHandle SpatialTickHandle;
};
