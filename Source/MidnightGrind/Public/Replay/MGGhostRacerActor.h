// Copyright Midnight Grind. All Rights Reserved.

#pragma once
/**
 * @file MGGhostRacerActor.h
 * @brief Ghost Racer Actor - Visual representation of recorded race data
 *
 * Ghost racers are semi-transparent vehicles that replay recorded race data,
 * allowing players to race against their personal best, friends, or world records.
 *
 * KEY FEATURES:
 * -------------
 * 1. REPLAY PLAYBACK
 *    - Smoothly interpolates between recorded frames
 *    - Syncs with race timer for accurate comparison
 *    - Supports pause, seek, and speed adjustment
 *
 * 2. VISUAL APPEARANCE
 *    - Configurable transparency and color
 *    - Dynamic material for ghost effect
 *    - Optional glow/trail effects
 *
 * 3. DELTA DISPLAY
 *    - Shows time difference to player
 *    - Updates in real-time based on track position
 *    - Color-coded (green = ahead, red = behind)
 *
 * 4. DRAFTING (Optional)
 *    - Can enable collision for drafting mechanics
 *    - Player can slipstream behind ghost
 *
 * HOW IT WORKS:
 * 1. UMGReplaySubsystem spawns AMGGhostRacerActor with FMGGhostConfig
 * 2. InitializeGhost() sets up replay data and appearance
 * 3. Each tick, UpdateTransform() interpolates to current time
 * 4. Race systems sync ghost time with race time
 *
 * TYPICAL USAGE:
 * @code
 * // Spawn personal best ghost
 * FMGGhostConfig Config;
 * Config.ReplayData = PersonalBestReplay;
 * Config.Transparency = 0.5f;
 * Config.GhostColor = FLinearColor::Green;
 * Config.bShowDelta = true;
 *
 * AMGGhostRacerActor* Ghost = ReplaySubsystem->SpawnGhost(Config);
 *
 * // Start when race starts
 * Ghost->SyncWithRaceTime(0.0f);
 * Ghost->StartPlayback();
 * @endcode
 *
 * @see UMGReplaySubsystem - Manages ghost spawning and lifetime
 * @see FMGGhostConfig - Configuration structure
 * @see FMGReplayData - Source replay data
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

// EMGGhostState - REMOVED (duplicate)
// Canonical definition in: Core/MGSharedTypes.h

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Core/MGSharedTypes.h"
#include "Replay/MGReplaySubsystem.h"
#include "MGGhostRacerActor.generated.h"

/**
 * Ghost Racer Actor
 *
 * Visual representation of a recorded race for ghost racing feature.
 * Plays back replay data with smooth interpolation and provides
 * real-time delta feedback to the player.
 *
 * @see UMGReplaySubsystem::SpawnGhost
 */
UCLASS()
class MIDNIGHTGRIND_API AMGGhostRacerActor : public AActor
{
	GENERATED_BODY()

public:
	AMGGhostRacerActor();

	virtual void BeginPlay() override;
	virtual void Tick(float MGDeltaTime) override;

	// ==========================================
	// INITIALIZATION
	// ==========================================

	/** Initialize ghost with config */
	UFUNCTION(BlueprintCallable, Category = "Ghost")
	void InitializeGhost(const FMGGhostConfig& Config);

	/** Set the mesh to display */
	UFUNCTION(BlueprintCallable, Category = "Ghost")
	void SetGhostMesh(USkeletalMesh* Mesh);

	/** Set static mesh for ghost */
	UFUNCTION(BlueprintCallable, Category = "Ghost")
	void SetGhostStaticMesh(UStaticMesh* Mesh);

	// ==========================================
	// PLAYBACK CONTROL
	// ==========================================

	/** Start playing ghost replay */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void StartPlayback();

	/** Pause playback */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void PausePlayback();

	/** Resume playback */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void ResumePlayback();

	/** Reset to start */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void ResetPlayback();

	/** Seek to specific time */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void SeekToTime(float Time);

	/** Set playback speed */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void SetPlaybackSpeed(float Speed);

	/** Sync with race timer */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Playback")
	void SyncWithRaceTime(float RaceTime);

	// ==========================================
	// QUERY
	// ==========================================

	/** Get current playback time */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	float GetCurrentTime() const { return CurrentTime; }

	/** Get total duration */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	float GetDuration() const { return ReplayData.GetDuration(); }

	/** Get playback progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	float GetPlaybackProgress() const;

	/** Get current ghost state */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	EMGGhostState GetGhostState() const { return CurrentState; }

	/** Get delta time at distance (positive = ghost ahead) */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	float GetDeltaAtDistance(float PlayerDistance) const;

	/** Get replay data */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	const FMGReplayData& GetReplayData() const { return ReplayData; }

	/** Is ghost ahead of given position */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	bool IsAheadOfPosition(FVector Position) const;

	/** Get distance traveled */
	UFUNCTION(BlueprintPure, Category = "Ghost")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	// ==========================================
	// APPEARANCE
	// ==========================================

	/** Set ghost transparency */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Appearance")
	void SetTransparency(float Transparency);

	/** Set ghost color */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Appearance")
	void SetGhostColor(FLinearColor Color);

	/** Set ghost visible */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Appearance")
	void SetGhostVisible(bool bVisible);

	/** Show/hide delta widget */
	UFUNCTION(BlueprintCallable, Category = "Ghost|Appearance")
	void SetDeltaWidgetVisible(bool bVisible);

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** Skeletal mesh for ghost vehicle */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* GhostMesh;

	/** Static mesh alternative */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* GhostStaticMesh;

	/** Widget for delta display */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* DeltaWidget;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Ghost configuration */
	UPROPERTY(BlueprintReadOnly, Category = "Config")
	FMGGhostConfig GhostConfig;

	/** Replay data */
	UPROPERTY(BlueprintReadOnly, Category = "Config")
	FMGReplayData ReplayData;

	/** Ghost material */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	UMaterialInterface* GhostMaterial;

	/** Dynamic material instance */
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	/** Default transparency */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float DefaultTransparency = 0.5f;

	/** Interpolation speed for smooth motion */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float InterpolationSpeed = 10.0f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current playback state */
	EMGGhostState CurrentState = EMGGhostState::Waiting;

	/** Current playback time */
	float CurrentTime = 0.0f;

	/** Playback speed multiplier */
	float PlaybackSpeed = 1.0f;

	/** Current transparency */
	float CurrentTransparency = 0.5f;

	/** Current ghost color */
	FLinearColor CurrentColor = FLinearColor::White;

	/** Distance traveled along track */
	float DistanceTraveled = 0.0f;

	/** Previous position for distance calculation */
	FVector PreviousPosition = FVector::ZeroVector;

	/** Target position for interpolation */
	FVector TargetPosition = FVector::ZeroVector;

	/** Target rotation for interpolation */
	FRotator TargetRotation = FRotator::ZeroRotator;

	/** Distance at each replay time (for delta calculations) */
	TArray<float> DistanceAtTime;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update ghost transform from replay data */
	void UpdateTransform(float MGDeltaTime);

	/** Update ghost appearance */
	void UpdateAppearance();

	/** Build distance lookup table */
	void BuildDistanceLookup();

	/** Get time at distance (for delta calculation) */
	float GetTimeAtDistance(float Distance) const;

	/** Create dynamic material */
	void SetupMaterial();
};
