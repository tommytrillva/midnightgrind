// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/AudioComponent.h"
#include "MGMusicManager.generated.h"

/**
 * Music context/state
 */
UENUM(BlueprintType)
enum class EMGMusicState : uint8
{
	/** No music */
	None,
	/** Main menu */
	MainMenu,
	/** Garage/customization */
	Garage,
	/** Pre-race/lobby */
	PreRace,
	/** Racing - low intensity */
	RacingLow,
	/** Racing - medium intensity */
	RacingMedium,
	/** Racing - high intensity (final lap, close race) */
	RacingHigh,
	/** Victory celebration */
	Victory,
	/** Loss/defeat */
	Defeat,
	/** Results screen */
	Results
};

/**
 * Music track with intensity layers
 */
USTRUCT(BlueprintType)
struct FMGMusicTrack
{
	GENERATED_BODY()

	/** Track name/ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName TrackName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText DisplayName;

	/** Base/ambient layer (always playing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	USoundBase* BaseLayer = nullptr;

	/** Low intensity layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	USoundBase* LowLayer = nullptr;

	/** Medium intensity layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	USoundBase* MediumLayer = nullptr;

	/** High intensity layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	USoundBase* HighLayer = nullptr;

	/** Climax/peak layer (final lap, photo finish) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	USoundBase* ClimaxLayer = nullptr;

	/** BPM for synchronization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float BPM = 120.0f;

	/** Track duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float Duration = 180.0f;

	/** Can this track loop? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bCanLoop = true;
};

/**
 * Stinger/one-shot music cue
 */
USTRUCT(BlueprintType)
struct FMGMusicStinger
{
	GENERATED_BODY()

	/** Stinger name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName StingerName;

	/** Sound to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	USoundBase* Sound = nullptr;

	/** Duck other music while playing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bDuckMusic = true;

	/** Duck amount (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float DuckAmount = 0.5f;
};

/**
 * Music Manager Subsystem
 * Handles all game music with dynamic intensity layers
 *
 * Features:
 * - Multi-layer tracks that blend based on intensity
 * - Smooth transitions between music states
 * - Beat-synced transitions
 * - Stinger support for events
 * - Playlist management
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMusicManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// MUSIC CONTROL
	// ==========================================

	/** Set the current music state */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetMusicState(EMGMusicState NewState);

	/** Get current music state */
	UFUNCTION(BlueprintPure, Category = "Music")
	EMGMusicState GetMusicState() const { return CurrentState; }

	/** Set intensity (0-1) for racing music */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetIntensity(float Intensity);

	/** Get current intensity */
	UFUNCTION(BlueprintPure, Category = "Music")
	float GetIntensity() const { return CurrentIntensity; }

	/** Play a specific track */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void PlayTrack(FName TrackName, float FadeTime = 1.0f);

	/** Stop all music */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void StopMusic(float FadeTime = 1.0f);

	/** Pause music */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void PauseMusic();

	/** Resume music */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void ResumeMusic();

	// ==========================================
	// STINGERS
	// ==========================================

	/** Play a stinger */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void PlayStinger(FName StingerName);

	/** Register a stinger */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void RegisterStinger(const FMGMusicStinger& Stinger);

	// ==========================================
	// TRACK MANAGEMENT
	// ==========================================

	/** Register a music track */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void RegisterTrack(const FMGMusicTrack& Track);

	/** Get current track name */
	UFUNCTION(BlueprintPure, Category = "Music")
	FName GetCurrentTrackName() const { return CurrentTrackName; }

	/** Get track by name */
	UFUNCTION(BlueprintPure, Category = "Music")
	bool GetTrack(FName TrackName, FMGMusicTrack& OutTrack) const;

	/** Skip to next track in playlist */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void NextTrack();

	/** Go to previous track */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void PreviousTrack();

	/** Shuffle the playlist */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void ShufflePlaylist();

	// ==========================================
	// VOLUME
	// ==========================================

	/** Set music volume */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void SetMusicVolume(float Volume);

	/** Get music volume */
	UFUNCTION(BlueprintPure, Category = "Music")
	float GetMusicVolume() const { return MusicVolume; }

	// ==========================================
	// EVENTS
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicStateChanged, EMGMusicState, NewState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackChanged, FName, TrackName);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeat);

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnMusicStateChanged OnMusicStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnTrackChanged OnTrackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Music|Events")
	FOnBeat OnBeat;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Fade time for layer transitions */
	UPROPERTY(EditDefaultsOnly, Category = "Music|Config")
	float LayerFadeTime = 0.5f;

	/** Fade time for track transitions */
	UPROPERTY(EditDefaultsOnly, Category = "Music|Config")
	float TrackFadeTime = 2.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current music state */
	EMGMusicState CurrentState = EMGMusicState::None;

	/** Current intensity (0-1) */
	float CurrentIntensity = 0.0f;

	/** Target intensity (for smoothing) */
	float TargetIntensity = 0.0f;

	/** Music volume */
	float MusicVolume = 0.8f;

	/** Current track name */
	FName CurrentTrackName;

	/** Is music paused? */
	bool bIsPaused = false;

	/** Current playback time */
	float PlaybackTime = 0.0f;

	/** Beat timer */
	float BeatTimer = 0.0f;

	/** Playlist index */
	int32 PlaylistIndex = 0;

	// ==========================================
	// DATA
	// ==========================================

	/** Registered tracks */
	UPROPERTY()
	TMap<FName, FMGMusicTrack> Tracks;

	/** Registered stingers */
	UPROPERTY()
	TMap<FName, FMGMusicStinger> Stingers;

	/** Playlist (track names) */
	UPROPERTY()
	TArray<FName> Playlist;

	// ==========================================
	// AUDIO COMPONENTS
	// ==========================================

	UPROPERTY()
	UAudioComponent* BaseLayerComp = nullptr;

	UPROPERTY()
	UAudioComponent* LowLayerComp = nullptr;

	UPROPERTY()
	UAudioComponent* MediumLayerComp = nullptr;

	UPROPERTY()
	UAudioComponent* HighLayerComp = nullptr;

	UPROPERTY()
	UAudioComponent* ClimaxLayerComp = nullptr;

	UPROPERTY()
	UAudioComponent* StingerComp = nullptr;

	// ==========================================
	// INTERNAL
	// ==========================================

	void InitializeAudioComponents();
	void CleanupAudioComponents();
	void UpdateLayerVolumes(float DeltaTime);
	void StartTrack(const FMGMusicTrack& Track, float FadeTime);
	void UpdateBeatTracking(float DeltaTime);

	/** Get appropriate track for state */
	FName GetTrackForState(EMGMusicState State) const;

	/** Timer handle for updates */
	FTimerHandle UpdateTimerHandle;
	void OnUpdateTimer();
};
