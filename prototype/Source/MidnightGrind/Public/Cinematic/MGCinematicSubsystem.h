// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCinematicSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCinematicType : uint8
{
	None,
	RaceIntro,
	RaceOutro,
	VictoryLap,
	PodiumCeremony,
	CarShowcase,
	TrackFlyover,
	DriverIntro,
	Cutscene,
	Transition
};

UENUM(BlueprintType)
enum class EMGCameraStyle : uint8
{
	Static,
	Tracking,
	Chase,
	Helicopter,
	Drone,
	Cinematic,
	POV,
	Bumper,
	Wide,
	CloseUp
};

UENUM(BlueprintType)
enum class EMGTransitionType : uint8
{
	Cut,
	Fade,
	CrossDissolve,
	Wipe,
	Zoom,
	Flash,
	Glitch,
	VHS
};

USTRUCT(BlueprintType)
struct FMGCameraShot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ShotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCameraStyle CameraStyle = EMGCameraStyle::Cinematic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DepthOfFieldFocalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DepthOfFieldFstop = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDepthOfField = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMotionBlur = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTransitionType TransitionIn = EMGTransitionType::Cut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTransitionType TransitionOut = EMGTransitionType::Cut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionDuration = 0.5f;
};

USTRUCT(BlueprintType)
struct FMGCinematicSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SequenceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SequenceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCinematicType Type = EMGCinematicType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCameraShot> Shots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkippable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPauseGameplay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MusicTrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> SoundCues;
};

USTRUCT(BlueprintType)
struct FMGCinematicEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeStamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventData;
};

USTRUCT(BlueprintType)
struct FMGSubtitle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SpeakerID;
};

USTRUCT(BlueprintType)
struct FMGPodiumResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LiveryID;
};

USTRUCT(BlueprintType)
struct FMGDriverIntroData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonPodiums = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCinematicStarted, EMGCinematicType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCinematicEnded, EMGCinematicType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnCinematicSkipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnShotChanged, int32, ShotIndex, const FMGCameraShot&, Shot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSubtitleChanged, const FMGSubtitle&, Subtitle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCinematicEvent, const FMGCinematicEvent&, Event);

UCLASS()
class MIDNIGHTGRIND_API UMGCinematicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Sequence Playback
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PlaySequence(FName SequenceID);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void StopSequence();

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void PauseSequence();

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void ResumeSequence();

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void SkipSequence();

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	bool IsPlayingCinematic() const { return bIsPlaying; }

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	float GetPlaybackProgress() const;

	// Race Cinematics
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayRaceIntro(FName TrackID, const TArray<FMGDriverIntroData>& Drivers);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayRaceOutro(const TArray<FMGPodiumResult>& Results);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayVictoryLap(AActor* WinnerVehicle);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayPodiumCeremony(const TArray<FMGPodiumResult>& TopThree);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayStartingGridSequence();

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Race")
	void PlayCountdown();

	// Showcase Cinematics
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Showcase")
	void PlayCarShowcase(AActor* Vehicle, float Duration = 10.0f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Showcase")
	void PlayTrackFlyover(FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Showcase")
	void PlayUnlockCinematic(FName UnlockType, FName UnlockID);

	// Transitions
	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void PlayTransition(EMGTransitionType Type, float Duration = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void FadeToBlack(float Duration = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void FadeFromBlack(float Duration = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Cinematic|Transition")
	void PlayGlitchTransition(float Intensity = 1.0f);

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
	void UpdatePlayback(float DeltaTime);
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
