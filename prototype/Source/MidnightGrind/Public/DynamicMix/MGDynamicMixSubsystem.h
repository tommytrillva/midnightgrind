// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGDynamicMixSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGAudioBusType : uint8
{
	Master,
	Music,
	Ambience,
	SFX,
	Vehicle,
	Engine,
	UI,
	Voice,
	Crowd,
	Weather,
	Police,
	Cinematics
};

UENUM(BlueprintType)
enum class EMGAudioState : uint8
{
	Idle,
	Cruising,
	Racing,
	Intense,
	PoliceChase,
	PhotoMode,
	Cutscene,
	Menu,
	Loading,
	Victory,
	Defeat
};

UENUM(BlueprintType)
enum class EMGAudioPriority : uint8
{
	Low,
	Normal,
	High,
	Critical,
	Override
};

USTRUCT(BlueprintType)
struct FMGAudioBusSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioBusType BusType = EMGAudioBusType::Master;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LowPassFrequency = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighPassFrequency = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbSend = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelaySend = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMuted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSolo = false;
};

USTRUCT(BlueprintType)
struct FMGAudioMixSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SnapshotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGAudioBusType, FMGAudioBusSettings> BusSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioPriority Priority = EMGAudioPriority::Normal;
};

USTRUCT(BlueprintType)
struct FMGAudioIntensityParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacePosition = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRacers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToLeader = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToAhead = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PoliceHeatLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInNitro = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearMiss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFinalLap = false;
};

USTRUCT(BlueprintType)
struct FMGAudioDuckingRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioBusType SourceBus = EMGAudioBusType::Voice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAudioBusType TargetBus = EMGAudioBusType::Music;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DuckAmount = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReleaseTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Threshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FMGAudioEffectPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDecayTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbWetLevel = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDryLevel = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DelayFeedback = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChorusDepth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistortionAmount = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGAudioZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ZoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAudioMixSnapshot ZoneSnapshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGAudioEffectPreset ZoneEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlendDistance = 200.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioStateChanged, EMGAudioState, OldState, EMGAudioState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSnapshotTransitionStarted, FName, FromSnapshot, FName, ToSnapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSnapshotTransitionComplete, FName, SnapshotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntensityChanged, float, NewIntensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioZoneEntered, const FMGAudioZone&, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioZoneExited, const FMGAudioZone&, Zone);

UCLASS()
class MIDNIGHTGRIND_API UMGDynamicMixSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// State Management
	UFUNCTION(BlueprintCallable, Category = "Audio|State")
	void SetAudioState(EMGAudioState NewState, float TransitionTime = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Audio|State")
	EMGAudioState GetCurrentAudioState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Audio|State")
	void PushAudioState(EMGAudioState State, float TransitionTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|State")
	void PopAudioState(float TransitionTime = 1.0f);

	// Bus Control
	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void SetBusVolume(EMGAudioBusType Bus, float Volume, float FadeTime = 0.0f);

	UFUNCTION(BlueprintPure, Category = "Audio|Bus")
	float GetBusVolume(EMGAudioBusType Bus) const;

	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void SetBusMuted(EMGAudioBusType Bus, bool bMuted);

	UFUNCTION(BlueprintPure, Category = "Audio|Bus")
	bool IsBusMuted(EMGAudioBusType Bus) const;

	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void SetBusSettings(EMGAudioBusType Bus, const FMGAudioBusSettings& Settings, float TransitionTime = 0.0f);

	UFUNCTION(BlueprintPure, Category = "Audio|Bus")
	FMGAudioBusSettings GetBusSettings(EMGAudioBusType Bus) const;

	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void ApplyLowPassFilter(EMGAudioBusType Bus, float Frequency, float TransitionTime = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Bus")
	void ClearLowPassFilter(EMGAudioBusType Bus, float TransitionTime = 0.0f);

	// Snapshots
	UFUNCTION(BlueprintCallable, Category = "Audio|Snapshot")
	void TransitionToSnapshot(FName SnapshotName, float TransitionTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Snapshot")
	void RegisterSnapshot(const FMGAudioMixSnapshot& Snapshot);

	UFUNCTION(BlueprintPure, Category = "Audio|Snapshot")
	FName GetCurrentSnapshotName() const { return CurrentSnapshotName; }

	UFUNCTION(BlueprintPure, Category = "Audio|Snapshot")
	bool IsTransitioning() const { return bIsTransitioning; }

	UFUNCTION(BlueprintPure, Category = "Audio|Snapshot")
	float GetTransitionProgress() const { return TransitionProgress; }

	// Intensity
	UFUNCTION(BlueprintCallable, Category = "Audio|Intensity")
	void UpdateIntensityParams(const FMGAudioIntensityParams& Params);

	UFUNCTION(BlueprintPure, Category = "Audio|Intensity")
	float GetCurrentIntensity() const { return CurrentIntensity; }

	UFUNCTION(BlueprintCallable, Category = "Audio|Intensity")
	void SetIntensityOverride(float Intensity, float Duration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Intensity")
	void ClearIntensityOverride();

	// Ducking
	UFUNCTION(BlueprintCallable, Category = "Audio|Ducking")
	void AddDuckingRule(const FMGAudioDuckingRule& Rule);

	UFUNCTION(BlueprintCallable, Category = "Audio|Ducking")
	void RemoveDuckingRule(EMGAudioBusType SourceBus, EMGAudioBusType TargetBus);

	UFUNCTION(BlueprintCallable, Category = "Audio|Ducking")
	void SetDuckingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Audio|Ducking")
	bool IsDuckingEnabled() const { return bDuckingEnabled; }

	// Effects
	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void ApplyEffectPreset(const FMGAudioEffectPreset& Preset, float TransitionTime = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void SetReverbSettings(float DecayTime, float WetLevel, float TransitionTime = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Audio|Effects")
	void SetGlobalPitch(float Pitch, float TransitionTime = 0.0f);

	UFUNCTION(BlueprintPure, Category = "Audio|Effects")
	float GetGlobalPitch() const { return GlobalPitch; }

	// Audio Zones
	UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
	void RegisterAudioZone(const FMGAudioZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
	void UnregisterAudioZone(FName ZoneID);

	UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
	void UpdateListenerPosition(FVector Position);

	UFUNCTION(BlueprintPure, Category = "Audio|Zone")
	FMGAudioZone GetCurrentAudioZone() const;

	// Slow Motion
	UFUNCTION(BlueprintCallable, Category = "Audio|SlowMo")
	void SetSlowMotionAudio(float TimeScale, float TransitionTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Audio|SlowMo")
	void ResetSlowMotionAudio(float TransitionTime = 0.1f);

	// Master Control
	UFUNCTION(BlueprintCallable, Category = "Audio|Master")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "Audio|Master")
	float GetMasterVolume() const { return MasterVolume; }

	UFUNCTION(BlueprintCallable, Category = "Audio|Master")
	void MuteAll(bool bMute);

	UFUNCTION(BlueprintCallable, Category = "Audio|Master")
	void PauseAllAudio(bool bPause);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnAudioStateChanged OnAudioStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnSnapshotTransitionStarted OnSnapshotTransitionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnSnapshotTransitionComplete OnSnapshotTransitionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnIntensityChanged OnIntensityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnAudioZoneEntered OnAudioZoneEntered;

	UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
	FOnAudioZoneExited OnAudioZoneExited;

protected:
	void OnMixTick();
	void UpdateTransition(float DeltaTime);
	void UpdateIntensity();
	void UpdateDucking();
	void UpdateAudioZones();
	void ApplyBusSettings(EMGAudioBusType Bus, const FMGAudioBusSettings& Settings);
	float CalculateIntensity(const FMGAudioIntensityParams& Params) const;
	void InitializeDefaultSnapshots();
	void InitializeDefaultBusSettings();

	UPROPERTY()
	EMGAudioState CurrentState = EMGAudioState::Idle;

	UPROPERTY()
	TArray<EMGAudioState> StateStack;

	UPROPERTY()
	TMap<EMGAudioBusType, FMGAudioBusSettings> BusSettingsMap;

	UPROPERTY()
	TMap<FName, FMGAudioMixSnapshot> Snapshots;

	UPROPERTY()
	FName CurrentSnapshotName;

	UPROPERTY()
	FMGAudioMixSnapshot CurrentSnapshot;

	UPROPERTY()
	FMGAudioMixSnapshot TargetSnapshot;

	UPROPERTY()
	bool bIsTransitioning = false;

	UPROPERTY()
	float TransitionProgress = 0.0f;

	UPROPERTY()
	float TransitionDuration = 1.0f;

	UPROPERTY()
	float CurrentIntensity = 0.0f;

	UPROPERTY()
	FMGAudioIntensityParams IntensityParams;

	UPROPERTY()
	bool bIntensityOverride = false;

	UPROPERTY()
	float IntensityOverrideValue = 0.0f;

	UPROPERTY()
	TArray<FMGAudioDuckingRule> DuckingRules;

	UPROPERTY()
	bool bDuckingEnabled = true;

	UPROPERTY()
	TArray<FMGAudioZone> AudioZones;

	UPROPERTY()
	FName CurrentZoneID = NAME_None;

	UPROPERTY()
	FVector ListenerPosition = FVector::ZeroVector;

	UPROPERTY()
	float MasterVolume = 1.0f;

	UPROPERTY()
	float GlobalPitch = 1.0f;

	UPROPERTY()
	bool bAllMuted = false;

	UPROPERTY()
	bool bAllPaused = false;

	FTimerHandle MixTickHandle;
};
