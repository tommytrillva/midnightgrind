// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGCinematicSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines the Cinematic Subsystem for Midnight Grind, a Y2K-themed
 * arcade street racing game. The subsystem manages all in-game cinematics,
 * including race intros, victory sequences, car showcases, and camera transitions.
 *
 * KEY CONCEPTS FOR ENTRY-LEVEL DEVELOPERS:
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    - This class inherits from UGameInstanceSubsystem, which means it lives
 *      for the entire lifetime of the game session (not just one level).
 *    - Subsystems are automatically created and managed by Unreal Engine.
 *    - Access it from anywhere using: GetGameInstance()->GetSubsystem<UMGCinematicSubsystem>()
 *
 * 2. CINEMATIC SEQUENCES:
 *    - A sequence is a pre-defined series of camera shots that play together.
 *    - Think of it like a mini movie: race intro, podium ceremony, car reveal, etc.
 *    - Each sequence has multiple "shots" (camera angles) that play in order.
 *
 * 3. CAMERA SHOTS:
 *    - Individual camera positions/movements within a sequence.
 *    - Each shot has properties like position, duration, FOV (field of view),
 *      and depth of field settings for that cinematic blur effect.
 *
 * 4. TRANSITIONS:
 *    - How the screen moves from one shot/scene to another.
 *    - Examples: Cut (instant), Fade, CrossDissolve, VHS (retro effect).
 *
 * 5. DELEGATES (Events):
 *    - The DECLARE_DYNAMIC_MULTICAST_DELEGATE macros create "events" that other
 *      parts of the game can subscribe to.
 *    - Example: When a cinematic starts, the UI can hide the HUD by listening
 *      to OnCinematicStarted.
 *
 * 6. UPROPERTY AND UFUNCTION MACROS:
 *    - UPROPERTY: Exposes variables to Unreal's reflection system (Blueprints,
 *      serialization, garbage collection).
 *    - UFUNCTION: Exposes functions to Blueprints and the reflection system.
 *    - BlueprintCallable: Can be called from Blueprints.
 *    - BlueprintPure: A "getter" function with no side effects.
 *
 * USAGE EXAMPLES:
 * - Play race intro before a race starts
 * - Show podium ceremony after race ends
 * - Display car showcase in garage menu
 * - Add cinematic transitions between game states
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCinematicSubsystem.generated.h"

/**
 * Cinematic Sequence Type - Categories of cinematic content in Midnight Grind.
 *
 * Each type triggers different camera behaviors, UI overlays, and gameplay
 * pausing rules. The type is used by UI systems to show appropriate overlays.
 */
UENUM(BlueprintType)
enum class EMGCinematicType : uint8
{
	/** No cinematic - normal gameplay camera. */
	None,
	/** Pre-race sequence showing track and drivers. */
	RaceIntro,
	/** Post-race sequence showing results. */
	RaceOutro,
	/** Winner celebration lap around the track. */
	VictoryLap,
	/** Podium ceremony for top 3 finishers. */
	PodiumCeremony,
	/** Rotating showcase of a vehicle in garage/menus. */
	CarShowcase,
	/** Aerial tour of a race track. */
	TrackFlyover,
	/** Individual driver introduction shot. */
	DriverIntro,
	/** Story cutscene (triggers MGCampaignSubsystem dialogue). */
	Cutscene,
	/** Screen transition effect (not a full sequence). */
	Transition
};

/**
 * Camera Style - Behavior preset for cinematic camera shots.
 *
 * Determines how the camera moves and tracks subjects during a shot.
 * Each style has predefined movement patterns and typical use cases.
 */
UENUM(BlueprintType)
enum class EMGCameraStyle : uint8
{
	/** Fixed position, no movement. Good for establishing shots. */
	Static,
	/** Smoothly follows a target actor. Standard gameplay camera. */
	Tracking,
	/** Behind-vehicle chase cam with spring physics. */
	Chase,
	/** High-altitude tracking shot, simulates TV broadcast helicopter. */
	Helicopter,
	/** Dynamic drone-style movement with smooth acceleration. */
	Drone,
	/** Director-controlled artistic framing with DOF effects. */
	Cinematic,
	/** First-person point-of-view from driver's perspective. */
	POV,
	/** Low bumper-mounted camera for dramatic ground-level shots. */
	Bumper,
	/** Wide-angle shot capturing multiple subjects or environment. */
	Wide,
	/** Tight framing on a single subject with shallow DOF. */
	CloseUp
};

/**
 * Transition Type - Visual effect for switching between shots or scenes.
 *
 * Transitions affect how one shot blends into the next. The VHS and Glitch
 * transitions match Midnight Grind's Y2K/retro aesthetic.
 */
UENUM(BlueprintType)
enum class EMGTransitionType : uint8
{
	/** Instant switch, no transition effect. */
	Cut,
	/** Fade to/from black. Classic cinematic transition. */
	Fade,
	/** Blend between two shots simultaneously. */
	CrossDissolve,
	/** Directional wipe (left-to-right, etc.). */
	Wipe,
	/** Zoom blur into next shot. High-energy feel. */
	Zoom,
	/** Bright white flash transition. Good for reveals. */
	Flash,
	/** Digital glitch/corruption effect. Y2K aesthetic. */
	Glitch,
	/** VHS tape tracking distortion. Retro aesthetic. */
	VHS
};

/**
 * Camera Shot - A single camera position/movement within a cinematic sequence.
 *
 * Each shot defines where the camera is, what it's looking at, how long it lasts,
 * and what visual effects are applied (DOF, motion blur, etc.). Shots are the
 * building blocks of cinematic sequences.
 *
 * @see FMGCinematicSequence - Contains an array of shots
 */
USTRUCT(BlueprintType)
struct FMGCameraShot
{
	GENERATED_BODY()

	/** Unique identifier for this shot within the sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	FName ShotID;

	/** Camera behavior preset (Static, Chase, Cinematic, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	EMGCameraStyle CameraStyle = EMGCameraStyle::Cinematic;

	/** World transform for the camera (position, rotation, scale). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	FTransform CameraTransform;

	/** Actor to track/focus on (optional - can use transform only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	AActor* TargetActor = nullptr;

	/** Offset from target actor's location for camera positioning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	FVector TargetOffset = FVector::ZeroVector;

	/** How long this shot plays (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shot")
	float Duration = 3.0f;

	/** Camera field of view in degrees. Lower = more zoom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float FOV = 60.0f;

	/** Distance from camera where objects are in perfect focus (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DepthOfField")
	float DepthOfFieldFocalDistance = 0.0f;

	/** Aperture f-stop. Lower = more blur outside focal plane. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DepthOfField")
	float DepthOfFieldFstop = 4.0f;

	/** Enable cinematic depth of field blur effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DepthOfField")
	bool bUseDepthOfField = false;

	/** Enable motion blur for fast camera/subject movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	bool bUseMotionBlur = true;

	/** Transition effect when entering this shot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	EMGTransitionType TransitionIn = EMGTransitionType::Cut;

	/** Transition effect when leaving this shot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	EMGTransitionType TransitionOut = EMGTransitionType::Cut;

	/** Duration of transition effects (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionDuration = 0.5f;
};

/**
 * Cinematic Sequence - A complete cinematic comprised of multiple camera shots.
 *
 * Sequences are self-contained "mini-movies" like race intros, podium ceremonies,
 * or car showcases. Each sequence contains an ordered array of camera shots that
 * play in succession, along with audio and timing information.
 *
 * Sequences can be:
 * - Pre-defined (registered at initialization)
 * - Dynamically generated (e.g., race intro based on participants)
 *
 * @see FMGCameraShot - Individual shots within the sequence
 */
USTRUCT(BlueprintType)
struct FMGCinematicSequence
{
	GENERATED_BODY()

	/** Unique identifier for this sequence (e.g., "RaceIntro_Downtown"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	FName SequenceID;

	/** Display name shown in UI (e.g., "Race Introduction"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	FText SequenceName;

	/** Category of cinematic (RaceIntro, Podium, Showcase, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	EMGCinematicType Type = EMGCinematicType::None;

	/** Ordered array of camera shots to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	TArray<FMGCameraShot> Shots;

	/** Total duration of all shots combined (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	float TotalDuration = 0.0f;

	/** Can player press button to skip this cinematic? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bSkippable = true;

	/** Should gameplay systems pause during playback? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bPauseGameplay = true;

	/** Music track to play during sequence (references audio system). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	FName MusicTrackID;

	/** Sound effects triggered during sequence. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<FName> SoundCues;
};

/**
 * Cinematic Event - A timed event that triggers during sequence playback.
 *
 * Events allow cinematics to trigger game actions at specific times, such as
 * playing sound effects, spawning particles, or triggering gameplay events.
 */
USTRUCT(BlueprintType)
struct FMGCinematicEvent
{
	GENERATED_BODY()

	/** Time offset from sequence start when event triggers (seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float TimeStamp = 0.0f;

	/** Event type identifier (e.g., "PlaySound", "SpawnParticle"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName EventType;

	/** Event-specific data as JSON or simple string. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString EventData;
};

/**
 * Subtitle - Timed text display for dialogue or narration during cinematics.
 *
 * Subtitles are accessibility features that display spoken dialogue as text.
 * They're also used for non-voiced narration in cutscenes.
 */
USTRUCT(BlueprintType)
struct FMGSubtitle
{
	GENERATED_BODY()

	/** Time offset when subtitle appears (seconds from sequence start). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	float StartTime = 0.0f;

	/** Time offset when subtitle disappears. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	float EndTime = 0.0f;

	/** The subtitle text to display. Supports localization. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	FText Text;

	/** Character speaking (for speaker name display). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	FName SpeakerID;
};

/**
 * Podium Result - Data for displaying a racer on the victory podium.
 *
 * Contains all information needed to render a podium position in the
 * post-race ceremony cinematic.
 */
USTRUCT(BlueprintType)
struct FMGPodiumResult
{
	GENERATED_BODY()

	/** Unique player/AI identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Podium")
	FString PlayerID;

	/** Display name for UI and announcer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Podium")
	FText PlayerName;

	/** Finishing position (1 = winner, 2 = second, 3 = third). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Podium")
	int32 Position = 0;

	/** Vehicle used in the race (for podium display). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Podium")
	FName VehicleID;

	/** Total race time in seconds (for results display). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Podium")
	float RaceTime = 0.0f;

	/** Custom livery/paint applied to vehicle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Podium")
	FName LiveryID;
};

/**
 * Driver Intro Data - Information displayed during pre-race driver introductions.
 *
 * Used to generate dynamic race intro sequences that showcase each driver
 * with their stats, vehicle, and crew affiliation.
 */
USTRUCT(BlueprintType)
struct FMGDriverIntroData
{
	GENERATED_BODY()

	/** Unique player/AI identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver")
	FString PlayerID;

	/** Driver's display name (gamertag or AI name). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver")
	FText PlayerName;

	/** Racing crew/team affiliation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver")
	FText CrewName;

	/** Vehicle being driven. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Driver")
	FName VehicleID;

	/** Starting position on the grid (1 = pole position). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 GridPosition = 0;

	/** Season race wins (for stats overlay). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 SeasonWins = 0;

	/** Season podium finishes (for stats overlay). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 SeasonPodiums = 0;
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================

/** Broadcast when a cinematic sequence begins playback. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCinematicStarted, EMGCinematicType, Type);

/** Broadcast when a cinematic sequence finishes normally. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCinematicEnded, EMGCinematicType, Type);

/** Broadcast when player skips a cinematic (bSkippable must be true). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnCinematicSkipped);

/** Broadcast when advancing to a new camera shot within a sequence. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnShotChanged, int32, ShotIndex, const FMGCameraShot&, Shot);

/** Broadcast when subtitle text changes (or clears). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSubtitleChanged, const FMGSubtitle&, Subtitle);

/** Broadcast when a timed cinematic event triggers. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCinematicEvent, const FMGCinematicEvent&, Event);

/**
 * =============================================================================
 * UMGCinematicSubsystem - Cinematic Camera and Sequence Management
 * =============================================================================
 *
 * Manages all in-game cinematics for Midnight Grind including:
 * - Race introductions and outros
 * - Victory celebrations and podium ceremonies
 * - Car showcases in garage/menus
 * - Track flyovers and driver introductions
 * - Screen transitions (fades, VHS effects, glitches)
 *
 * The subsystem supports both pre-defined sequences and dynamically generated
 * cinematics (e.g., race intros based on actual participants).
 *
 * As a UGameInstanceSubsystem:
 * - Persists across level loads
 * - One instance for the entire game session
 * - Access via: GetGameInstance()->GetSubsystem<UMGCinematicSubsystem>()
 *
 * USAGE EXAMPLE:
 * @code
 * UMGCinematicSubsystem* Cinematic = GetGameInstance()->GetSubsystem<UMGCinematicSubsystem>();
 *
 * // Play a pre-race intro
 * TArray<FMGDriverIntroData> Drivers = GetRaceParticipants();
 * Cinematic->PlayRaceIntro(TrackID, Drivers);
 *
 * // Listen for completion
 * Cinematic->OnCinematicEnded.AddDynamic(this, &UMyWidget::HandleCinematicEnded);
 * @endcode
 *
 * DESIGN NOTES:
 * - Cinematics automatically enable letterbox (2.35:1 aspect ratio)
 * - VHS and Glitch transitions match the Y2K aesthetic
 * - Sequences can optionally pause gameplay during playback
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCinematicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//-------------------------------------------------------------------------
	// Subsystem Lifecycle
	//-------------------------------------------------------------------------

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//-------------------------------------------------------------------------
	// Sequence Playback Control
	//-------------------------------------------------------------------------

	/**
	 * Plays a registered cinematic sequence by ID.
	 * @param SequenceID The unique identifier of the sequence to play.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PlaySequence(FName SequenceID);

	/** Stops the current sequence immediately. */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void StopSequence();

	/** Pauses playback (can resume later). */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PauseSequence();

	/** Resumes a paused sequence. */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void ResumeSequence();

	/** Skips the current sequence (if bSkippable is true). */
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void SkipSequence();

	/** Returns true if a cinematic is currently playing. */
	UFUNCTION(BlueprintPure, Category = "Cinematic")
	bool IsPlayingCinematic() const { return bIsPlaying; }

	/** Returns playback progress as 0.0-1.0 (0 = start, 1 = complete). */
	UFUNCTION(BlueprintPure, Category = "Cinematic")
	float GetPlaybackProgress() const;

	//-------------------------------------------------------------------------
	// Race Cinematics - Pre/Post Race Sequences
	//-------------------------------------------------------------------------

	/**
	 * Generates and plays a race intro sequence with track flyover and driver intros.
	 * @param TrackID Track identifier for flyover camera path.
	 * @param Drivers Array of participating drivers for intro shots.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayRaceIntro(FName TrackID, const TArray<FMGDriverIntroData>& Drivers);

	/**
	 * Plays post-race results sequence showing finishing order.
	 * @param Results Array of all race finishers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayRaceOutro(const TArray<FMGPodiumResult>& Results);

	/**
	 * Plays a chase-cam victory lap following the winner.
	 * @param WinnerVehicle The winning vehicle actor to track.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayVictoryLap(AActor* WinnerVehicle);

	/**
	 * Plays podium ceremony for top 3 finishers.
	 * @param TopThree Array of 1st, 2nd, 3rd place data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayPodiumCeremony(const TArray<FMGPodiumResult>& TopThree);

	/** Plays the pre-race starting grid showcase sequence. */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayStartingGridSequence();

	/** Plays the race start countdown sequence (3-2-1-GO). */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayCountdown();

	//-------------------------------------------------------------------------
	// Showcase Cinematics - Garage/Menu Presentations
	//-------------------------------------------------------------------------

	/**
	 * Plays orbiting showcase camera around a vehicle.
	 * @param Vehicle The vehicle actor to showcase.
	 * @param Duration Total showcase length in seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Showcase")
	void PlayCarShowcase(AActor* Vehicle, float Duration = 10.0f);

	/**
	 * Plays aerial flyover of a race track.
	 * @param TrackID Track identifier for camera path.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Showcase")
	void PlayTrackFlyover(FName TrackID);

	/**
	 * Plays unlock reveal cinematic (new car, part, etc.).
	 * @param UnlockType Category of unlock ("Vehicle", "Part", "Livery").
	 * @param UnlockID Specific item identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Showcase")
	void PlayUnlockCinematic(FName UnlockType, FName UnlockID);

	//-------------------------------------------------------------------------
	// Screen Transitions - Scene Change Effects
	//-------------------------------------------------------------------------

	/**
	 * Plays a screen transition effect.
	 * @param Type The type of transition (Fade, Glitch, VHS, etc.).
	 * @param Duration Transition length in seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void PlayTransition(EMGTransitionType Type, float Duration = 0.5f);

	/**
	 * Fades screen to black (typically before scene change).
	 * @param Duration Fade duration in seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void FadeToBlack(float Duration = 1.0f);

	/**
	 * Fades screen from black to visible (after scene change).
	 * @param Duration Fade duration in seconds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void FadeFromBlack(float Duration = 1.0f);

	/**
	 * Plays digital glitch distortion effect (Y2K aesthetic).
	 * @param Intensity Glitch severity (0.0-1.0).
	 */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void PlayGlitchTransition(float Intensity = 1.0f);

	/** Plays VHS tape tracking distortion effect (retro aesthetic). */
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void PlayVHSTransition();

	// Camera Control
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Camera")
	void SetActiveCamera(const FMGCameraShot& Shot);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Camera")
	void BlendToCamera(const FMGCameraShot& Shot, float BlendTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Camera")
	void ShakeCamera(float Intensity = 1.0f, float Duration = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Camera")
	void SetCameraTarget(AActor* Target);

	// Letterbox & Effects
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Effects")
	void EnableLetterbox(float AspectRatio = 2.35f, float TransitionTime = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Effects")
	void DisableLetterbox(float TransitionTime = 0.5f);

	UFUNCTION(BlueprintPure, Category = "Cinematic|Effects")
	bool IsLetterboxEnabled() const { return bLetterboxEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Effects")
	void EnableFilmGrain(float Intensity = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Effects")
	void DisableFilmGrain();

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Effects")
	void SetVignetteIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Effects")
	void ApplyColorGrade(FName ColorGradePreset);

	// Subtitles
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Subtitles")
	void ShowSubtitle(const FMGSubtitle& Subtitle);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Subtitles")
	void HideSubtitle();

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Subtitles")
	void SetSubtitlesEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Cinematic|Subtitles")
	bool AreSubtitlesEnabled() const { return bSubtitlesEnabled; }

	// Sequence Management
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Sequence")
	void RegisterSequence(const FMGCinematicSequence& Sequence);

	UFUNCTION(BlueprintPure, Category = "Cinematic|Sequence")
	FMGCinematicSequence GetSequence(FName SequenceID) const;

	UFUNCTION(BlueprintPure, Category = "Cinematic|Sequence")
	TArray<FMGCinematicSequence> GetSequencesByType(EMGCinematicType Type) const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Cinematic|Events")
	FMGOnCinematicStarted OnCinematicStarted;

	UPROPERTY(BlueprintAssignable, Category = "Cinematic|Events")
	FMGOnCinematicEnded OnCinematicEnded;

	UPROPERTY(BlueprintAssignable, Category = "Cinematic|Events")
	FMGOnCinematicSkipped OnCinematicSkipped;

	UPROPERTY(BlueprintAssignable, Category = "Cinematic|Events")
	FMGOnShotChanged OnShotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Cinematic|Events")
	FMGOnSubtitleChanged OnSubtitleChanged;

	UPROPERTY(BlueprintAssignable, Category = "Cinematic|Events")
	FMGOnCinematicEvent OnCinematicEvent;

protected:
	void InitializeDefaultSequences();
	void UpdatePlayback(float MGDeltaTime);
	void AdvanceToNextShot();
	void ProcessCinematicEvents(float CurrentTime);
	void ApplyCameraShot(const FMGCameraShot& Shot);
	FMGCinematicSequence* FindSequence(FName SequenceID);
	FMGCinematicSequence GenerateRaceIntroSequence(FName TrackID, const TArray<FMGDriverIntroData>& Drivers);
	FMGCinematicSequence GeneratePodiumSequence(const TArray<FMGPodiumResult>& Results);
	FMGCinematicSequence GenerateShowcaseSequence(AActor* Vehicle, float Duration);
	TArray<FMGCameraShot> GenerateOrbitingShots(FVector Center, float Radius, int32 NumShots, float TotalDuration);

private:
	UPROPERTY()
	TArray<FMGCinematicSequence> RegisteredSequences;

	UPROPERTY()
	FMGCinematicSequence CurrentSequence;

	UPROPERTY()
	TArray<FMGSubtitle> CurrentSubtitles;

	UPROPERTY()
	TArray<FMGCinematicEvent> PendingEvents;

	bool bIsPlaying = false;
	bool bIsPaused = false;
	bool bLetterboxEnabled = false;
	bool bSubtitlesEnabled = true;
	float CurrentPlaybackTime = 0.0f;
	int32 CurrentShotIndex = 0;
	float CurrentShotTime = 0.0f;
	float LetterboxRatio = 2.35f;
	float FilmGrainIntensity = 0.0f;
	float VignetteIntensity = 0.0f;

	FTimerHandle PlaybackTimerHandle;
};
