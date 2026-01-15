// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "MGAudioSubsystem.generated.h"

class USoundBase;
class UAudioComponent;

/**
 * Sound categories for volume control
 */
UENUM(BlueprintType)
enum class EMGSoundCategory : uint8
{
	Master,
	Music,
	SFX,
	Engine,
	Environment,
	UI,
	Voice
};

/**
 * Audio settings
 */
USTRUCT(BlueprintType)
struct FMGAudioSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EngineVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnvironmentVolume = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UIVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VoiceVolume = 1.0f;
};

/**
 * One-shot sound request
 */
USTRUCT(BlueprintType)
struct FMGSoundRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSoundCategory Category = EMGSoundCategory::SFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIs3D = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttenuationRadius = 5000.0f;
};

/**
 * Audio Subsystem
 * Central management for all game audio
 *
 * Handles:
 * - Volume control per category
 * - Sound playback with pooling
 * - Audio ducking and mixing
 * - Settings persistence
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// VOLUME CONTROL
	// ==========================================

	/** Set volume for a category */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetCategoryVolume(EMGSoundCategory Category, float Volume);

	/** Get volume for a category */
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetCategoryVolume(EMGSoundCategory Category) const;

	/** Get effective volume (category * master) */
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetEffectiveVolume(EMGSoundCategory Category) const;

	/** Set all audio settings at once */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAudioSettings(const FMGAudioSettings& Settings);

	/** Get current audio settings */
	UFUNCTION(BlueprintPure, Category = "Audio")
	FMGAudioSettings GetAudioSettings() const { return AudioSettings; }

	// ==========================================
	// SOUND PLAYBACK
	// ==========================================

	/** Play a 2D sound */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	UAudioComponent* PlaySound2D(UObject* WorldContextObject, USoundBase* Sound,
		EMGSoundCategory Category = EMGSoundCategory::SFX,
		float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

	/** Play a 3D sound at location */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	UAudioComponent* PlaySoundAtLocation(UObject* WorldContextObject, USoundBase* Sound,
		FVector Location, EMGSoundCategory Category = EMGSoundCategory::SFX,
		float VolumeMultiplier = 1.0f, float PitchMultiplier = 1.0f);

	/** Play sound from request struct */
	UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
	UAudioComponent* PlaySoundRequest(UObject* WorldContextObject, const FMGSoundRequest& Request);

	/** Stop all sounds in a category */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopCategorySounds(EMGSoundCategory Category);

	// ==========================================
	// AUDIO DUCKING
	// ==========================================

	/** Duck a category (reduce volume temporarily) */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void DuckCategory(EMGSoundCategory Category, float DuckAmount, float FadeTime = 0.5f);

	/** Restore ducked category */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void RestoreCategory(EMGSoundCategory Category, float FadeTime = 0.5f);

	/** Duck all except specified category */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void DuckAllExcept(EMGSoundCategory Exception, float DuckAmount, float FadeTime = 0.5f);

	/** Restore all ducked categories */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void RestoreAllDucking(float FadeTime = 0.5f);

	// ==========================================
	// UTILITY
	// ==========================================

	/** Pause all game audio */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PauseAllAudio();

	/** Resume all game audio */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void ResumeAllAudio();

	/** Set global pitch (for slow-mo effects) */
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetGlobalPitch(float Pitch);

	/** Get global pitch */
	UFUNCTION(BlueprintPure, Category = "Audio")
	float GetGlobalPitch() const { return GlobalPitch; }

protected:
	/** Current audio settings */
	UPROPERTY()
	FMGAudioSettings AudioSettings;

	/** Current duck amounts per category */
	UPROPERTY()
	TMap<EMGSoundCategory, float> DuckAmounts;

	/** Active audio components per category */
	UPROPERTY()
	TMap<EMGSoundCategory, TArray<TWeakObjectPtr<UAudioComponent>>> ActiveComponents;

	/** Global pitch multiplier */
	float GlobalPitch = 1.0f;

	/** Is audio paused? */
	bool bAudioPaused = false;

	/** Apply volume to active components */
	void ApplyCategoryVolume(EMGSoundCategory Category);

	/** Register active component */
	void RegisterActiveComponent(EMGSoundCategory Category, UAudioComponent* Component);

	/** Clean up finished components */
	void CleanupFinishedComponents();
};
