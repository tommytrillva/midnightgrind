// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGCrowdAudioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCrowdMood : uint8
{
	Calm,
	Excited,
	Cheering,
	Gasping,
	Booing,
	Celebrating,
	Tense,
	Disappointed
};

UENUM(BlueprintType)
enum class EMGCrowdEventType : uint8
{
	RaceStart,
	RaceFinish,
	Overtake,
	NearMiss,
	Crash,
	DriftCombo,
	LeadChange,
	FinalLap,
	PhotoFinish,
	Podium,
	PlayerWin,
	PlayerCrash,
	BigJump,
	NitroActivation,
	PoliceEscape
};

UENUM(BlueprintType)
enum class EMGCrowdZoneType : uint8
{
	StartFinish,
	Grandstand,
	Roadside,
	Overpass,
	Spectator,
	VIP,
	PitLane
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
	float Radius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdZoneType ZoneType = EMGCrowdZoneType::Roadside;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrowdDensity = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExcitementMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> AmbientLoop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<USoundBase>> CheerSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<USoundBase>> GaspSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<USoundBase>> BooSounds;
};

USTRUCT(BlueprintType)
struct FMGCrowdReaction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdEventType TriggerEvent = EMGCrowdEventType::Overtake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdMood ResultingMood = EMGCrowdMood::Cheering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ReactionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsAllZones = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 2000.0f;
};

USTRUCT(BlueprintType)
struct FMGCrowdState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrowdMood CurrentMood = EMGCrowdMood::Calm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExcitementLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TensionLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActiveZoneID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeSinceLastReaction = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGCrowdWaveSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveDecay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinIntensityForWave = 0.5f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdMoodChanged, EMGCrowdMood, OldMood, EMGCrowdMood, NewMood);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrowdReaction, EMGCrowdEventType, Event, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdZoneEntered, const FMGCrowdZone&, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrowdZoneExited, const FMGCrowdZone&, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExcitementChanged, float, NewExcitement);

UCLASS()
class MIDNIGHTGRIND_API UMGCrowdAudioSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Zone Management
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Zone")
	void RegisterCrowdZone(const FMGCrowdZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Zone")
	void UnregisterCrowdZone(FName ZoneID);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Zone")
	TArray<FMGCrowdZone> GetAllCrowdZones() const;

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Zone")
	FMGCrowdZone GetCrowdZone(FName ZoneID) const;

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Zone")
	FMGCrowdZone GetNearestCrowdZone(FVector Location) const;

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Zone")
	void SetZoneDensity(FName ZoneID, int32 Density);

	// Event Triggers
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Events")
	void TriggerCrowdEvent(EMGCrowdEventType Event, FVector EventLocation, float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Events")
	void TriggerGlobalCrowdEvent(EMGCrowdEventType Event, float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Events")
	void TriggerZoneCrowdEvent(FName ZoneID, EMGCrowdEventType Event, float Intensity = 1.0f);

	// Mood Control
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Mood")
	void SetGlobalMood(EMGCrowdMood Mood, float TransitionTime = 1.0f);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Mood")
	EMGCrowdMood GetCurrentMood() const { return CrowdState.CurrentMood; }

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Mood")
	void SetExcitementLevel(float Level);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Mood")
	float GetExcitementLevel() const { return CrowdState.ExcitementLevel; }

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Mood")
	void SetTensionLevel(float Level);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Mood")
	float GetTensionLevel() const { return CrowdState.TensionLevel; }

	// Race State Integration
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnRaceStarted();

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnRaceFinished(bool bPlayerWon);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnFinalLapStarted();

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnLeadChange(const FString& NewLeaderID);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnOvertake(const FString& OvertakerID, const FString& OvertakenID, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Race")
	void OnCrash(const FString& PlayerID, FVector Location, float Severity);

	// Listener
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Listener")
	void UpdateListenerLocation(FVector Location);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Listener")
	FMGCrowdState GetCrowdState() const { return CrowdState; }

	// Reaction Configuration
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Config")
	void RegisterReaction(const FMGCrowdReaction& Reaction);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Config")
	void SetWaveSettings(const FMGCrowdWaveSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Config")
	FMGCrowdWaveSettings GetWaveSettings() const { return WaveSettings; }

	// Volume Control
	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Volume")
	void SetCrowdVolume(float Volume);

	UFUNCTION(BlueprintPure, Category = "CrowdAudio|Volume")
	float GetCrowdVolume() const { return MasterCrowdVolume; }

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Volume")
	void FadeOutCrowd(float FadeTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CrowdAudio|Volume")
	void FadeInCrowd(float FadeTime = 1.0f);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdMoodChanged OnCrowdMoodChanged;

	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdReaction OnCrowdReaction;

	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdZoneEntered OnCrowdZoneEntered;

	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnCrowdZoneExited OnCrowdZoneExited;

	UPROPERTY(BlueprintAssignable, Category = "CrowdAudio|Events")
	FOnExcitementChanged OnExcitementChanged;

protected:
	void OnCrowdTick();
	void UpdateZoneAudio();
	void UpdateMood();
	void DecayExcitement(float DeltaTime);
	void ProcessCrowdWave(FVector Origin, float Intensity);
	void PlayReactionSound(const FMGCrowdReaction& Reaction, FVector Location);
	FMGCrowdReaction GetReactionForEvent(EMGCrowdEventType Event) const;
	void InitializeDefaultReactions();

	UPROPERTY()
	TArray<FMGCrowdZone> CrowdZones;

	UPROPERTY()
	TArray<FMGCrowdReaction> Reactions;

	UPROPERTY()
	FMGCrowdState CrowdState;

	UPROPERTY()
	FMGCrowdWaveSettings WaveSettings;

	UPROPERTY()
	FVector ListenerLocation = FVector::ZeroVector;

	UPROPERTY()
	float MasterCrowdVolume = 1.0f;

	UPROPERTY()
	float ExcitementDecayRate = 0.1f;

	UPROPERTY()
	float TensionDecayRate = 0.05f;

	UPROPERTY()
	TMap<EMGCrowdEventType, float> EventCooldowns;

	FTimerHandle CrowdTickHandle;
};
