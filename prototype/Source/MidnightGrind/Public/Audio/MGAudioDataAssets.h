// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Audio/MGEngineAudioComponent.h"
#include "Audio/MGVehicleSFXComponent.h"
#include "Audio/MGMusicManager.h"
#include "MGAudioDataAssets.generated.h"

/**
 * Engine Audio Preset Data Asset
 * Configure engine sounds for different vehicle types
 *
 * Create in Editor: Right-click > Miscellaneous > Data Asset > MGEngineAudioPresetData
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGEngineAudioPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FName PresetName;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio", meta = (MultiLine = true))
	FText Description;

	/** Engine type tag (for categorization) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FName EngineType; // "I4", "V6", "V8", "Flat6", "Rotary", etc.

	/** The actual preset configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Audio")
	FMGEngineAudioPreset Preset;

	/** Convert to preset struct */
	UFUNCTION(BlueprintPure, Category = "Engine Audio")
	FMGEngineAudioPreset GetPreset() const { return Preset; }
};

/**
 * Vehicle SFX Preset Data Asset
 * Configure vehicle sound effects (tires, collisions, etc.)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGVehicleSFXPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Preset name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	FName PresetName;

	/** Surface sound configurations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	TArray<FMGSurfaceSoundConfig> SurfaceConfigs;

	/** Collision sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	FMGCollisionSoundConfig CollisionConfig;

	/** Wind noise sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	USoundBase* WindNoiseSound = nullptr;

	/** Suspension thump sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	USoundBase* SuspensionSound = nullptr;

	/** Brake squeal sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle SFX")
	USoundBase* BrakeSquealSound = nullptr;

	/** Apply this preset to a component */
	UFUNCTION(BlueprintCallable, Category = "Vehicle SFX")
	void ApplyToComponent(UMGVehicleSFXComponent* Component) const;
};

/**
 * Music Track Data Asset
 * Configure a single music track with layers
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMusicTrackData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Track configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FMGMusicTrack Track;

	/** Genre tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName Genre; // "Electronic", "HipHop", "Rock", "DrumAndBass", etc.

	/** Mood/energy tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName Mood; // "Aggressive", "Chill", "Hype", "Dark", etc.

	/** Suitable music states */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<EMGMusicState> SuitableStates;

	/** Get track struct */
	UFUNCTION(BlueprintPure, Category = "Music")
	FMGMusicTrack GetTrack() const { return Track; }

	/** Is suitable for state? */
	UFUNCTION(BlueprintPure, Category = "Music")
	bool IsSuitableForState(EMGMusicState State) const
	{
		return SuitableStates.Contains(State);
	}
};

/**
 * Music Playlist Data Asset
 * Configure a playlist of tracks
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGMusicPlaylistData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Playlist name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName PlaylistName;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FText DisplayName;

	/** Tracks in this playlist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	TArray<UMGMusicTrackData*> Tracks;

	/** Should shuffle by default? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	bool bShuffleByDefault = true;

	/** Get all track names */
	UFUNCTION(BlueprintPure, Category = "Music")
	TArray<FName> GetTrackNames() const;

	/** Register all tracks with music manager */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void RegisterWithMusicManager(UMGMusicManager* MusicManager) const;
};

/**
 * Stinger Collection Data Asset
 * Configure event stingers
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGStingerCollectionData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Collection name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	FName CollectionName;

	/** Countdown stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger CountdownStinger;

	/** Race start stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger RaceStartStinger;

	/** Final lap stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger FinalLapStinger;

	/** Position gained stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger PositionGainedStinger;

	/** Position lost stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger PositionLostStinger;

	/** Victory stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger VictoryStinger;

	/** Defeat stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger DefeatStinger;

	/** New record stinger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Race")
	FMGMusicStinger NewRecordStinger;

	/** Additional custom stingers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music|Custom")
	TArray<FMGMusicStinger> CustomStingers;

	/** Register all stingers with music manager */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void RegisterWithMusicManager(UMGMusicManager* MusicManager) const;
};

/**
 * Complete Audio Config Data Asset
 * Master configuration combining all audio settings
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAudioConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Config name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	FName ConfigName;

	/** Default audio settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	FMGAudioSettings DefaultSettings;

	/** Engine presets by vehicle class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	TMap<FName, UMGEngineAudioPresetData*> EnginePresetsByClass;

	/** Default vehicle SFX preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGVehicleSFXPresetData* DefaultVehicleSFX = nullptr;

	/** Main music playlist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGMusicPlaylistData* MainPlaylist = nullptr;

	/** Menu music playlist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGMusicPlaylistData* MenuPlaylist = nullptr;

	/** Stinger collection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
	UMGStingerCollectionData* Stingers = nullptr;

	/** Get engine preset for vehicle class */
	UFUNCTION(BlueprintPure, Category = "Audio Config")
	UMGEngineAudioPresetData* GetEnginePresetForClass(FName VehicleClass) const;
};
