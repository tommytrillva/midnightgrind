// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCinematicSubsystem.cpp
 * @brief Implementation of the Cinematic Subsystem for camera sequences,
 *        race intros/outros, transitions, and visual effects.
 */

#include "Cinematic/MGCinematicSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"

void UMGCinematicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDefaultSequences();
}

void UMGCinematicSubsystem::Deinitialize()
{
	StopSequence();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PlaybackTimerHandle);
	}
	Super::Deinitialize();
}

void UMGCinematicSubsystem::PlaySequence(FName SequenceID)
{
	FMGCinematicSequence* Sequence = FindSequence(SequenceID);
	if (!Sequence || Sequence->Shots.Num() == 0)
		return;

	CurrentSequence = *Sequence;
	bIsPlaying = true;
	bIsPaused = false;
	CurrentPlaybackTime = 0.0f;
	CurrentShotIndex = 0;
	CurrentShotTime = 0.0f;

	if (CurrentSequence.bPauseGameplay)
	{
		// Would pause gameplay systems
	}

	EnableLetterbox(2.35f, 0.5f);
	ApplyCameraShot(CurrentSequence.Shots[0]);
	OnCinematicStarted.Broadcast(CurrentSequence.Type);
	OnShotChanged.Broadcast(0, CurrentSequence.Shots[0]);

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGCinematicSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			PlaybackTimerHandle,
			[WeakThis]() { if (WeakThis.IsValid()) WeakThis->UpdatePlayback(1.0f / 60.0f); },
			1.0f / 60.0f,
			true
		);
	}
}

void UMGCinematicSubsystem::StopSequence()
{
	if (!bIsPlaying) return;

	bIsPlaying = false;
	bIsPaused = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PlaybackTimerHandle);
	}

	DisableLetterbox(0.5f);
	OnCinematicEnded.Broadcast(CurrentSequence.Type);
}

void UMGCinematicSubsystem::PauseSequence()
{
	if (bIsPlaying)
	{
		bIsPaused = true;
	}
}

void UMGCinematicSubsystem::ResumeSequence()
{
	if (bIsPlaying && bIsPaused)
	{
		bIsPaused = false;
	}
}

void UMGCinematicSubsystem::SkipSequence()
{
	if (bIsPlaying && CurrentSequence.bSkippable)
	{
		OnCinematicSkipped.Broadcast();
		StopSequence();
	}
}

float UMGCinematicSubsystem::GetPlaybackProgress() const
{
	if (CurrentSequence.TotalDuration > 0.0f)
	{
		return CurrentPlaybackTime / CurrentSequence.TotalDuration;
	}
	return 0.0f;
}

void UMGCinematicSubsystem::PlayRaceIntro(FName TrackID, const TArray<FMGDriverIntroData>& Drivers)
{
	FMGCinematicSequence IntroSequence = GenerateRaceIntroSequence(TrackID, Drivers);
	RegisterSequence(IntroSequence);
	PlaySequence(IntroSequence.SequenceID);
}

void UMGCinematicSubsystem::PlayRaceOutro(const TArray<FMGPodiumResult>& Results)
{
	FMGCinematicSequence OutroSequence = GeneratePodiumSequence(Results);
	OutroSequence.Type = EMGCinematicType::RaceOutro;
	RegisterSequence(OutroSequence);
	PlaySequence(OutroSequence.SequenceID);
}

void UMGCinematicSubsystem::PlayVictoryLap(AActor* WinnerVehicle)
{
	if (!WinnerVehicle) return;

	FMGCinematicSequence VictorySequence;
	VictorySequence.SequenceID = FName(TEXT("Generated_VictoryLap"));
	VictorySequence.SequenceName = FText::FromString(TEXT("Victory Lap"));
	VictorySequence.Type = EMGCinematicType::VictoryLap;
	VictorySequence.bSkippable = true;
	VictorySequence.bPauseGameplay = false;

	// Chase camera following winner
	FMGCameraShot ChaseShot;
	ChaseShot.ShotID = FName(TEXT("VictoryChase"));
	ChaseShot.CameraStyle = EMGCameraStyle::Chase;
	ChaseShot.TargetActor = WinnerVehicle;
	ChaseShot.TargetOffset = FVector(-500.0f, 0.0f, 200.0f);
	ChaseShot.Duration = 8.0f;
	ChaseShot.FOV = 75.0f;
	VictorySequence.Shots.Add(ChaseShot);

	// Helicopter shot
	FMGCameraShot HeliShot;
	HeliShot.ShotID = FName(TEXT("VictoryHeli"));
	HeliShot.CameraStyle = EMGCameraStyle::Helicopter;
	HeliShot.TargetActor = WinnerVehicle;
	HeliShot.TargetOffset = FVector(0.0f, 500.0f, 300.0f);
	HeliShot.Duration = 5.0f;
	HeliShot.TransitionIn = EMGTransitionType::CrossDissolve;
	VictorySequence.Shots.Add(HeliShot);

	VictorySequence.TotalDuration = 13.0f;
	RegisterSequence(VictorySequence);
	PlaySequence(VictorySequence.SequenceID);
}

void UMGCinematicSubsystem::PlayPodiumCeremony(const TArray<FMGPodiumResult>& TopThree)
{
	FMGCinematicSequence PodiumSequence = GeneratePodiumSequence(TopThree);
	RegisterSequence(PodiumSequence);
	PlaySequence(PodiumSequence.SequenceID);
}

void UMGCinematicSubsystem::PlayStartingGridSequence()
{
	FMGCinematicSequence* GridSequence = FindSequence(FName(TEXT("RaceStartGrid")));
	if (GridSequence)
	{
		PlaySequence(GridSequence->SequenceID);
	}
}

void UMGCinematicSubsystem::PlayCountdown()
{
	FMGCinematicSequence* CountdownSequence = FindSequence(FName(TEXT("RaceCountdown")));
	if (CountdownSequence)
	{
		PlaySequence(CountdownSequence->SequenceID);
	}
}

void UMGCinematicSubsystem::PlayCarShowcase(AActor* Vehicle, float Duration)
{
	if (!Vehicle) return;

	FMGCinematicSequence ShowcaseSequence = GenerateShowcaseSequence(Vehicle, Duration);
	RegisterSequence(ShowcaseSequence);
	PlaySequence(ShowcaseSequence.SequenceID);
}

void UMGCinematicSubsystem::PlayTrackFlyover(FName TrackID)
{
	FMGCinematicSequence FlyoverSequence;
	FlyoverSequence.SequenceID = FName(*FString::Printf(TEXT("Flyover_%s"), *TrackID.ToString()));
	FlyoverSequence.SequenceName = FText::FromString(TEXT("Track Flyover"));
	FlyoverSequence.Type = EMGCinematicType::TrackFlyover;
	FlyoverSequence.bSkippable = true;

	// Would generate shots based on track spline/waypoints
	FMGCameraShot OverviewShot;
	OverviewShot.ShotID = FName(TEXT("TrackOverview"));
	OverviewShot.CameraStyle = EMGCameraStyle::Helicopter;
	OverviewShot.Duration = 5.0f;
	OverviewShot.FOV = 90.0f;
	FlyoverSequence.Shots.Add(OverviewShot);

	FlyoverSequence.TotalDuration = 5.0f;
	RegisterSequence(FlyoverSequence);
	PlaySequence(FlyoverSequence.SequenceID);
}

void UMGCinematicSubsystem::PlayUnlockCinematic(FName UnlockType, FName UnlockID)
{
	FMGCinematicSequence UnlockSequence;
	UnlockSequence.SequenceID = FName(*FString::Printf(TEXT("Unlock_%s_%s"), *UnlockType.ToString(), *UnlockID.ToString()));
	UnlockSequence.SequenceName = FText::FromString(TEXT("New Unlock"));
	UnlockSequence.Type = EMGCinematicType::CarShowcase;
	UnlockSequence.bSkippable = true;
	UnlockSequence.TotalDuration = 4.0f;

	// Flash transition in
	FMGCameraShot RevealShot;
	RevealShot.ShotID = FName(TEXT("UnlockReveal"));
	RevealShot.CameraStyle = EMGCameraStyle::Cinematic;
	RevealShot.Duration = 4.0f;
	RevealShot.TransitionIn = EMGTransitionType::Flash;
	RevealShot.bUseDepthOfField = true;
	RevealShot.DepthOfFieldFocalDistance = 300.0f;
	UnlockSequence.Shots.Add(RevealShot);

	RegisterSequence(UnlockSequence);
	PlaySequence(UnlockSequence.SequenceID);
}

void UMGCinematicSubsystem::PlayTransition(EMGTransitionType Type, float Duration)
{
	// Would trigger post-process transition effect
	switch (Type)
	{
	case EMGTransitionType::Fade:
		FadeToBlack(Duration / 2.0f);
		break;
	case EMGTransitionType::Glitch:
		PlayGlitchTransition(1.0f);
		break;
	case EMGTransitionType::VHS:
		PlayVHSTransition();
		break;
	default:
		break;
	}
}

void UMGCinematicSubsystem::FadeToBlack(float Duration)
{
	// Would animate fade post-process
}

void UMGCinematicSubsystem::FadeFromBlack(float Duration)
{
	// Would animate fade post-process
}

void UMGCinematicSubsystem::PlayGlitchTransition(float Intensity)
{
	// Would trigger glitch shader effect matching PS1/PS2 aesthetic
}

void UMGCinematicSubsystem::PlayVHSTransition()
{
	// Would trigger VHS-style tracking/distortion effect
}

void UMGCinematicSubsystem::SetActiveCamera(const FMGCameraShot& Shot)
{
	ApplyCameraShot(Shot);
}

void UMGCinematicSubsystem::BlendToCamera(const FMGCameraShot& Shot, float BlendTime)
{
	// Would blend camera position/rotation over time
	ApplyCameraShot(Shot);
}

void UMGCinematicSubsystem::ShakeCamera(float Intensity, float Duration)
{
	// Would trigger camera shake
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PC->ClientStartCameraShake(nullptr, Intensity);
		}
	}
}

void UMGCinematicSubsystem::SetCameraTarget(AActor* Target)
{
	// Would update camera tracking target
}

void UMGCinematicSubsystem::EnableLetterbox(float AspectRatio, float TransitionTime)
{
	bLetterboxEnabled = true;
	LetterboxRatio = AspectRatio;
	// Would animate letterbox bars
}

void UMGCinematicSubsystem::DisableLetterbox(float TransitionTime)
{
	bLetterboxEnabled = false;
	// Would animate letterbox bars out
}

void UMGCinematicSubsystem::EnableFilmGrain(float Intensity)
{
	FilmGrainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	// Would enable post-process film grain
}

void UMGCinematicSubsystem::DisableFilmGrain()
{
	FilmGrainIntensity = 0.0f;
}

void UMGCinematicSubsystem::SetVignetteIntensity(float Intensity)
{
	VignetteIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UMGCinematicSubsystem::ApplyColorGrade(FName ColorGradePreset)
{
	// Would apply LUT or color grading based on preset
	// Presets: "NightRace", "Sunset", "Neon", "VHS", "Film"
}

void UMGCinematicSubsystem::ShowSubtitle(const FMGSubtitle& Subtitle)
{
	if (bSubtitlesEnabled)
	{
		OnSubtitleChanged.Broadcast(Subtitle);
	}
}

void UMGCinematicSubsystem::HideSubtitle()
{
	OnSubtitleChanged.Broadcast(FMGSubtitle());
}

void UMGCinematicSubsystem::SetSubtitlesEnabled(bool bEnabled)
{
	bSubtitlesEnabled = bEnabled;
}

void UMGCinematicSubsystem::RegisterSequence(const FMGCinematicSequence& Sequence)
{
	int32 ExistingIndex = RegisteredSequences.IndexOfByPredicate(
		[&Sequence](const FMGCinematicSequence& S) { return S.SequenceID == Sequence.SequenceID; });

	if (ExistingIndex != INDEX_NONE)
		RegisteredSequences[ExistingIndex] = Sequence;
	else
		RegisteredSequences.Add(Sequence);
}

FMGCinematicSequence UMGCinematicSubsystem::GetSequence(FName SequenceID) const
{
	const FMGCinematicSequence* Found = RegisteredSequences.FindByPredicate(
		[SequenceID](const FMGCinematicSequence& S) { return S.SequenceID == SequenceID; });

	return Found ? *Found : FMGCinematicSequence();
}

TArray<FMGCinematicSequence> UMGCinematicSubsystem::GetSequencesByType(EMGCinematicType Type) const
{
	TArray<FMGCinematicSequence> Result;
	for (const FMGCinematicSequence& Sequence : RegisteredSequences)
	{
		if (Sequence.Type == Type)
			Result.Add(Sequence);
	}
	return Result;
}

void UMGCinematicSubsystem::InitializeDefaultSequences()
{
	// Race countdown sequence
	FMGCinematicSequence Countdown;
	Countdown.SequenceID = FName(TEXT("RaceCountdown"));
	Countdown.SequenceName = FText::FromString(TEXT("Race Countdown"));
	Countdown.Type = EMGCinematicType::RaceIntro;
	Countdown.bSkippable = false;
	Countdown.bPauseGameplay = true;

	FMGCameraShot CountdownShot;
	CountdownShot.ShotID = FName(TEXT("CountdownMain"));
	CountdownShot.CameraStyle = EMGCameraStyle::Wide;
	CountdownShot.Duration = 4.0f;
	CountdownShot.FOV = 70.0f;
	Countdown.Shots.Add(CountdownShot);

	Countdown.TotalDuration = 4.0f;
	RegisteredSequences.Add(Countdown);

	// Starting grid sequence
	FMGCinematicSequence StartGrid;
	StartGrid.SequenceID = FName(TEXT("RaceStartGrid"));
	StartGrid.SequenceName = FText::FromString(TEXT("Starting Grid"));
	StartGrid.Type = EMGCinematicType::RaceIntro;
	StartGrid.bSkippable = true;

	FMGCameraShot GridWideShot;
	GridWideShot.ShotID = FName(TEXT("GridWide"));
	GridWideShot.CameraStyle = EMGCameraStyle::Wide;
	GridWideShot.Duration = 3.0f;
	StartGrid.Shots.Add(GridWideShot);

	FMGCameraShot GridDroneShot;
	GridDroneShot.ShotID = FName(TEXT("GridDrone"));
	GridDroneShot.CameraStyle = EMGCameraStyle::Drone;
	GridDroneShot.Duration = 4.0f;
	GridDroneShot.TransitionIn = EMGTransitionType::CrossDissolve;
	StartGrid.Shots.Add(GridDroneShot);

	StartGrid.TotalDuration = 7.0f;
	RegisteredSequences.Add(StartGrid);
}

void UMGCinematicSubsystem::UpdatePlayback(float MGDeltaTime)
{
	if (!bIsPlaying || bIsPaused)
		return;

	CurrentPlaybackTime += DeltaTime;
	CurrentShotTime += DeltaTime;

	// Check for cinematic events
	ProcessCinematicEvents(CurrentPlaybackTime);

	// Check if current shot is finished
	if (CurrentShotIndex < CurrentSequence.Shots.Num())
	{
		const FMGCameraShot& CurrentShot = CurrentSequence.Shots[CurrentShotIndex];
		if (CurrentShotTime >= CurrentShot.Duration)
		{
			AdvanceToNextShot();
		}
	}

	// Check if sequence is finished
	if (CurrentPlaybackTime >= CurrentSequence.TotalDuration)
	{
		StopSequence();
	}
}

void UMGCinematicSubsystem::AdvanceToNextShot()
{
	CurrentShotIndex++;
	CurrentShotTime = 0.0f;

	if (CurrentShotIndex < CurrentSequence.Shots.Num())
	{
		const FMGCameraShot& NextShot = CurrentSequence.Shots[CurrentShotIndex];

		// Handle transition
		if (NextShot.TransitionIn != EMGTransitionType::Cut)
		{
			PlayTransition(NextShot.TransitionIn, NextShot.TransitionDuration);
		}

		ApplyCameraShot(NextShot);
		OnShotChanged.Broadcast(CurrentShotIndex, NextShot);
	}
}

void UMGCinematicSubsystem::ProcessCinematicEvents(float CurrentTime)
{
	for (int32 i = PendingEvents.Num() - 1; i >= 0; i--)
	{
		if (PendingEvents[i].TimeStamp <= CurrentTime)
		{
			OnCinematicEvent.Broadcast(PendingEvents[i]);
			PendingEvents.RemoveAt(i);
		}
	}
}

void UMGCinematicSubsystem::ApplyCameraShot(const FMGCameraShot& Shot)
{
	// Would apply camera settings to active camera
	// - Position/Rotation from Transform or relative to target
	// - FOV
	// - Depth of field settings
	// - Motion blur settings
}

FMGCinematicSequence* UMGCinematicSubsystem::FindSequence(FName SequenceID)
{
	return RegisteredSequences.FindByPredicate(
		[SequenceID](const FMGCinematicSequence& S) { return S.SequenceID == SequenceID; });
}

FMGCinematicSequence UMGCinematicSubsystem::GenerateRaceIntroSequence(FName TrackID, const TArray<FMGDriverIntroData>& Drivers)
{
	FMGCinematicSequence IntroSequence;
	IntroSequence.SequenceID = FName(*FString::Printf(TEXT("RaceIntro_%s"), *TrackID.ToString()));
	IntroSequence.SequenceName = FText::FromString(TEXT("Race Introduction"));
	IntroSequence.Type = EMGCinematicType::RaceIntro;
	IntroSequence.bSkippable = true;

	// Track flyover shot
	FMGCameraShot FlyoverShot;
	FlyoverShot.ShotID = FName(TEXT("IntroFlyover"));
	FlyoverShot.CameraStyle = EMGCameraStyle::Helicopter;
	FlyoverShot.Duration = 4.0f;
	FlyoverShot.FOV = 80.0f;
	IntroSequence.Shots.Add(FlyoverShot);

	// Individual driver shots (up to first 3)
	int32 DriverShotCount = FMath::Min(3, Drivers.Num());
	for (int32 i = 0; i < DriverShotCount; i++)
	{
		FMGCameraShot DriverShot;
		DriverShot.ShotID = FName(*FString::Printf(TEXT("Driver_%d"), i));
		DriverShot.CameraStyle = EMGCameraStyle::CloseUp;
		DriverShot.Duration = 2.5f;
		DriverShot.TransitionIn = EMGTransitionType::CrossDissolve;
		DriverShot.TransitionDuration = 0.3f;
		DriverShot.bUseDepthOfField = true;
		DriverShot.DepthOfFieldFocalDistance = 200.0f;
		IntroSequence.Shots.Add(DriverShot);
	}

	// Grid wide shot
	FMGCameraShot GridShot;
	GridShot.ShotID = FName(TEXT("IntroGrid"));
	GridShot.CameraStyle = EMGCameraStyle::Wide;
	GridShot.Duration = 3.0f;
	GridShot.TransitionIn = EMGTransitionType::CrossDissolve;
	IntroSequence.Shots.Add(GridShot);

	IntroSequence.TotalDuration = 4.0f + (DriverShotCount * 2.5f) + 3.0f;
	return IntroSequence;
}

FMGCinematicSequence UMGCinematicSubsystem::GeneratePodiumSequence(const TArray<FMGPodiumResult>& Results)
{
	FMGCinematicSequence PodiumSequence;
	PodiumSequence.SequenceID = FName(TEXT("Generated_Podium"));
	PodiumSequence.SequenceName = FText::FromString(TEXT("Podium Ceremony"));
	PodiumSequence.Type = EMGCinematicType::PodiumCeremony;
	PodiumSequence.bSkippable = true;

	// Wide podium shot
	FMGCameraShot WideShot;
	WideShot.ShotID = FName(TEXT("PodiumWide"));
	WideShot.CameraStyle = EMGCameraStyle::Wide;
	WideShot.Duration = 3.0f;
	PodiumSequence.Shots.Add(WideShot);

	// Winner close-up (if we have a winner)
	if (Results.Num() > 0)
	{
		FMGCameraShot WinnerShot;
		WinnerShot.ShotID = FName(TEXT("PodiumWinner"));
		WinnerShot.CameraStyle = EMGCameraStyle::CloseUp;
		WinnerShot.Duration = 4.0f;
		WinnerShot.TransitionIn = EMGTransitionType::CrossDissolve;
		WinnerShot.bUseDepthOfField = true;
		PodiumSequence.Shots.Add(WinnerShot);
	}

	// Top 3 medium shot
	FMGCameraShot TopThreeShot;
	TopThreeShot.ShotID = FName(TEXT("PodiumTopThree"));
	TopThreeShot.CameraStyle = EMGCameraStyle::Cinematic;
	TopThreeShot.Duration = 4.0f;
	TopThreeShot.TransitionIn = EMGTransitionType::CrossDissolve;
	PodiumSequence.Shots.Add(TopThreeShot);

	// Orbiting celebration shot
	FMGCameraShot OrbitShot;
	OrbitShot.ShotID = FName(TEXT("PodiumOrbit"));
	OrbitShot.CameraStyle = EMGCameraStyle::Drone;
	OrbitShot.Duration = 5.0f;
	OrbitShot.TransitionIn = EMGTransitionType::CrossDissolve;
	PodiumSequence.Shots.Add(OrbitShot);

	PodiumSequence.TotalDuration = 16.0f;
	return PodiumSequence;
}

FMGCinematicSequence UMGCinematicSubsystem::GenerateShowcaseSequence(AActor* Vehicle, float Duration)
{
	FMGCinematicSequence ShowcaseSequence;
	ShowcaseSequence.SequenceID = FName(TEXT("Generated_CarShowcase"));
	ShowcaseSequence.SequenceName = FText::FromString(TEXT("Car Showcase"));
	ShowcaseSequence.Type = EMGCinematicType::CarShowcase;
	ShowcaseSequence.bSkippable = true;

	FVector VehicleLocation = Vehicle ? Vehicle->GetActorLocation() : FVector::ZeroVector;
	TArray<FMGCameraShot> OrbitShots = GenerateOrbitingShots(VehicleLocation, 400.0f, 4, Duration);
	ShowcaseSequence.Shots = OrbitShots;
	ShowcaseSequence.TotalDuration = Duration;

	return ShowcaseSequence;
}

TArray<FMGCameraShot> UMGCinematicSubsystem::GenerateOrbitingShots(FVector Center, float Radius, int32 NumShots, float TotalDuration)
{
	TArray<FMGCameraShot> Shots;
	if (NumShots <= 0)
	{
		return Shots;
	}
	float ShotDuration = TotalDuration / NumShots;

	for (int32 i = 0; i < NumShots; i++)
	{
		float Angle = (2.0f * PI * i) / NumShots;
		FVector Offset(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			150.0f + (i * 30.0f)
		);

		FMGCameraShot Shot;
		Shot.ShotID = FName(*FString::Printf(TEXT("Orbit_%d"), i));
		Shot.CameraStyle = EMGCameraStyle::Cinematic;
		Shot.CameraTransform.SetLocation(Center + Offset);
		Shot.Duration = ShotDuration;
		Shot.FOV = 50.0f + (i * 5.0f);
		Shot.bUseDepthOfField = true;
		Shot.DepthOfFieldFocalDistance = Radius;

		if (i > 0)
		{
			Shot.TransitionIn = EMGTransitionType::CrossDissolve;
			Shot.TransitionDuration = 0.5f;
		}

		Shots.Add(Shot);
	}

	return Shots;
}
