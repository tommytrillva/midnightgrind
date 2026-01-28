// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGGhostRacerActor.h
 * @brief Ghost Racer Actor - Visual representation of recorded race replays
 *
 * @section overview Overview
 * The Ghost Racer Actor provides a visual "ghost" representation of a previously
 * recorded race. This allows players to race against their previous attempts or
 * other players' best times, creating a competitive time-trial experience.
 *
 * Ghost racers are semi-transparent vehicles that replay recorded race data,
 * showing the exact path, speed, and inputs from a previous run. They provide
 * real-time delta timing so players know if they are ahead or behind.
 *
 * @section concepts Key Concepts for Beginners
 *
 * **What is a Ghost Racer?**
 * In racing games, a "ghost" is a transparent replay of a previous race. Unlike
 * AI opponents, ghosts follow a pre-recorded path exactly. They cannot react to
 * the player - they simply replay historical data. This is commonly used in
 * time-attack modes to help players improve their times.
 *
 * **Delta Time**
 * Delta time is the time difference between the player and the ghost at any given
 * point on the track. A negative delta means the player is ahead (faster), while
 * a positive delta means the ghost is ahead (player is slower). Delta is typically
 * calculated based on track distance rather than elapsed time for accuracy.
 *
 * **Interpolation**
 * Since replays are recorded at a fixed frame rate (e.g., 30 FPS), the ghost's
 * position between recorded frames must be interpolated (smoothly blended) to
 * avoid jerky movement during playback.
 *
 * **Ghost States**
 * - Waiting: Ghost is initialized but not yet playing
 * - Playing: Ghost is actively replaying recorded data
 * - Paused: Playback is temporarily stopped
 * - Finished: Ghost has completed its replay
 *
 * @section usage Usage Examples
 *
 * **Spawning a Ghost from Replay Data:**
 * @code
 * // Get the replay subsystem
 * UMGReplaySubsystem* ReplaySubsystem = GetWorld()->GetSubsystem<UMGReplaySubsystem>();
 *
 * // Configure the ghost
 * FMGGhostConfig Config;
 * Config.ReplayData = MyReplayData;
 * Config.Transparency = 0.5f;
 * Config.GhostColor = FLinearColor(0.0f, 0.5f, 1.0f);
 * Config.bShowDelta = true;
 *
 * // Spawn the ghost
 * AMGGhostRacerActor* Ghost = ReplaySubsystem->SpawnGhost(Config);
 * @endcode
 *
 * **Controlling Ghost Playback:**
 * @code
 * // Start playback when race begins
 * Ghost->StartPlayback();
 *
 * // Sync ghost with race timer for accurate delta
 * Ghost->SyncWithRaceTime(CurrentRaceTime);
 *
 * // Check if player is ahead
 * float DeltaTime = Ghost->GetDeltaAtDistance(PlayerDistance);
 * if (DeltaTime < 0.0f)
 * {
 *     // Player is ahead of ghost by |DeltaTime| seconds
 * }
 * @endcode
 *
 * **Customizing Ghost Appearance:**
 * @code
 * // Make ghost more visible
 * Ghost->SetTransparency(0.7f);
 *
 * // Change ghost color to gold for world record
 * Ghost->SetGhostColor(FLinearColor(1.0f, 0.84f, 0.0f));
 *
 * // Hide delta widget for cleaner HUD
 * Ghost->SetDeltaWidgetVisible(false);
 * @endcode
 *
 * @section bestpractices Best Practices
 * - Always call InitializeGhost() before StartPlayback()
 * - Use SyncWithRaceTime() to keep ghost and player in sync
 * - Consider spawning ghosts slightly before race start for seamless experience
 * - Limit the number of simultaneous ghosts (3-5) for performance
 * - Use different colors to distinguish ghost types (personal best, friend, world record)
 *
 * @see UMGReplaySubsystem
 * @see UMGReplayRecordingComponent
 * @see FMGGhostConfig
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Replay/MGReplaySubsystem.h"
#include "MGGhostRacerActor.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UWidgetComponent;

/**
 * Ghost playback state
 */
UENUM(BlueprintType)
enum class EMGGhostState : uint8
{
	Waiting,
	Playing,
	Paused,
	Finished
};

/**
 * Ghost Racer Actor
 * Visual representation of a recorded race
 *
 * Features:
 * - Plays back recorded replay data
 * - Semi-transparent ghost visual
 * - Time delta display
 * - Optional collision for drafting
 */
UCLASS()
class MIDNIGHTGRIND_API AMGGhostRacerActor : public AActor
{
	GENERATED_BODY()

public:
	AMGGhostRacerActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

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
	void UpdateTransform(float DeltaTime);

	/** Update ghost appearance */
	void UpdateAppearance();

	/** Build distance lookup table */
	void BuildDistanceLookup();

	/** Get time at distance (for delta calculation) */
	float GetTimeAtDistance(float Distance) const;

	/** Create dynamic material */
	void SetupMaterial();
};
