// Copyright Midnight Grind. All Rights Reserved.

/**
 * ============================================================================
 * MGAudioSubsystem.h
 * ============================================================================
 *
 * PURPOSE:
 * This file defines the central audio management system for Midnight Grind.
 * Think of it as the "audio control center" that manages all sounds in the game.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    - A "subsystem" is a helper object that lives as long as the game is running
 *    - "GameInstance" means it persists across level changes (unlike actors)
 *    - You can access it from anywhere using: GetGameInstance()->GetSubsystem<UMGAudioSubsystem>()
 *
 * 2. SOUND CATEGORIES:
 *    - Games organize sounds into categories (Music, SFX, Voice, etc.)
 *    - Each category can have its own volume slider in the options menu
 *    - This allows players to customize their audio experience
 *
 * 3. AUDIO DUCKING:
 *    - "Ducking" means temporarily lowering one sound to make another more audible
 *    - Example: Lower music volume when a character speaks, then restore it
 *    - Essential for creating a professional audio mix
 *
 * 4. POOLING:
 *    - Instead of creating/destroying audio components constantly, we reuse them
 *    - This improves performance, especially with many simultaneous sounds
 *
 * HOW IT FITS IN THE GAME ARCHITECTURE:
 *
 *    [Game Instance] (lives for entire game session)
 *          |
 *          +-- [UMGAudioSubsystem] (this class - manages all audio)
 *                    |
 *                    +-- Controls volume for all categories
 *                    +-- Plays 2D sounds (UI, music)
 *                    +-- Plays 3D sounds (engine, collisions)
 *                    +-- Handles ducking during cutscenes/dialogs
 *
 * USAGE EXAMPLE (in Blueprint or C++):
 *
 *    // Get the audio subsystem
 *    UMGAudioSubsystem* AudioSys = GetGameInstance()->GetSubsystem<UMGAudioSubsystem>();
 *
 *    // Set music volume to 50%
 *    AudioSys->SetCategoryVolume(EMGSoundCategory::Music, 0.5f);
 *
 *    // Play a UI click sound
 *    AudioSys->PlaySound2D(this, ClickSound, EMGSoundCategory::UI);
 *
 *    // Play engine sound at vehicle location
 *    AudioSys->PlaySoundAtLocation(this, EngineSound, VehicleLocation, EMGSoundCategory::Engine);
 *
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "MGAudioSubsystem.generated.h"

// Forward declarations - tells the compiler these classes exist without including their full headers
// This speeds up compilation and reduces dependencies between files
class USoundBase;      // Base class for all sound assets (SoundWave, SoundCue, MetaSound, etc.)
class UAudioComponent; // Component that actually plays sounds in the world

/**
 * Sound categories for volume control.
 *
 * WHAT IS THIS?
 * An enumeration (enum) that defines the different "buckets" of sounds in the game.
 * Each category can have its own volume setting, allowing players to customize
 * their audio experience (e.g., "I want loud engine sounds but quiet music").
 *
 * WHY SEPARATE CATEGORIES?
 * - Player preference: Some players want music off during competitive play
 * - Accessibility: Hard-of-hearing players may want voice louder than SFX
 * - Game design: Certain sounds should never be muted (like engine feedback)
 *
 * BLUEPRINTTYPE:
 * This macro exposes the enum to Blueprints, so designers can use it in visual scripting.
 */
UENUM(BlueprintType)
enum class EMGSoundCategory : uint8
{
	Master,      // Overall game volume - affects ALL other categories
	Music,       // Background music, radio, soundtrack
	SFX,         // General sound effects (explosions, UI clicks, etc.)
	Engine,      // Vehicle engine sounds - separate so racers can prioritize this
	Environment, // Ambient sounds (wind, crowd noise, city ambiance)
	UI,          // Menu clicks, notifications, HUD sounds
	Voice        // Character dialog, announcer, voice chat
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
