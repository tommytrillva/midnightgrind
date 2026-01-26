// Copyright Midnight Grind. All Rights Reserved.

#include "Spectator/MGSpectatorSubsystem.h"
#include "Spectator/MGSpectatorPawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UMGSpectatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Default overlay settings
	OverlaySettings.bShowStandings = true;
	OverlaySettings.bShowTargetInfo = true;
	OverlaySettings.bShowLapTimes = true;
	OverlaySettings.bShowSpeedometer = true;
	OverlaySettings.bShowMiniMap = true;
}

void UMGSpectatorSubsystem::Deinitialize()
{
	if (bIsSpectating && SpectatingController)
	{
		ExitSpectatorMode(SpectatingController);
	}

	Super::Deinitialize();
}

void UMGSpectatorSubsystem::Tick(float DeltaTime)
{
	if (!bIsSpectating)
	{
		return;
	}

	// Update target info
	UpdateTargetInfo();

	// Update auto-director
	if (bAutoDirectorEnabled)
	{
		UpdateAutoDirector(DeltaTime);
	}

	// Update camera transition
	if (bIsTransitioning)
	{
		UpdateCameraTransition(DeltaTime);
	}
	else
	{
		// Update camera position based on mode
		UpdateCameraPosition(DeltaTime);
	}
}

// ==========================================
// SPECTATOR MODE
// ==========================================

void UMGSpectatorSubsystem::EnterSpectatorMode(APlayerController* Controller)
{
	if (!Controller || bIsSpectating)
	{
		return;
	}

	SpectatingController = Controller;

	// Store original pawn
	OriginalPawn = Controller->GetPawn();

	// Spawn spectator pawn
	SpawnSpectatorPawn(Controller);

	bIsSpectating = true;

	// Set default target
	if (AvailableTargets.Num() > 0)
	{
		SetTarget(AvailableTargets[0].Target);
	}

	OnSpectatorModeEntered.Broadcast();
}

void UMGSpectatorSubsystem::ExitSpectatorMode(APlayerController* Controller)
{
	if (!Controller || !bIsSpectating || Controller != SpectatingController)
	{
		return;
	}

	// Restore original pawn
	if (OriginalPawn)
	{
		Controller->Possess(OriginalPawn);
	}

	// Destroy spectator pawn
	DestroySpectatorPawn();

	bIsSpectating = false;
	SpectatingController = nullptr;

	OnSpectatorModeExited.Broadcast();
}

// ==========================================
// CAMERA MODE
// ==========================================

void UMGSpectatorSubsystem::SetCameraMode(EMGSpectatorCameraMode NewMode)
{
	if (NewMode == CurrentCameraMode)
	{
		return;
	}

	PreviousCameraMode = CurrentCameraMode;
	CurrentCameraMode = NewMode;

	// Broadcast cut info
	FMGCameraCut CutInfo;
	CutInfo.FromMode = PreviousCameraMode;
	CutInfo.ToMode = CurrentCameraMode;
	CutInfo.CutTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CutInfo.bSmoothTransition = (NewMode != EMGSpectatorCameraMode::FreeCam);

	OnCameraCut.Broadcast(CutInfo);
	OnCameraModeChanged.Broadcast(NewMode);
}

void UMGSpectatorSubsystem::CycleNextCameraMode()
{
	int32 Current = static_cast<int32>(CurrentCameraMode);
	int32 Next = (Current + 1) % static_cast<int32>(EMGSpectatorCameraMode::Director);

	// Skip director mode in cycling (it's accessed separately)
	if (Next == static_cast<int32>(EMGSpectatorCameraMode::Director))
	{
		Next = 0;
	}

	SetCameraMode(static_cast<EMGSpectatorCameraMode>(Next));
}

void UMGSpectatorSubsystem::CyclePreviousCameraMode()
{
	int32 Current = static_cast<int32>(CurrentCameraMode);
	int32 Prev = Current - 1;

	if (Prev < 0)
	{
		Prev = static_cast<int32>(EMGSpectatorCameraMode::TrackSide);
	}

	SetCameraMode(static_cast<EMGSpectatorCameraMode>(Prev));
}

FText UMGSpectatorSubsystem::GetCameraModeDisplayName(EMGSpectatorCameraMode Mode)
{
	switch (Mode)
	{
	case EMGSpectatorCameraMode::FreeCam:
		return NSLOCTEXT("Spectator", "FreeCam", "Free Camera");
	case EMGSpectatorCameraMode::Chase:
		return NSLOCTEXT("Spectator", "Chase", "Chase");
	case EMGSpectatorCameraMode::Orbit:
		return NSLOCTEXT("Spectator", "Orbit", "Orbit");
	case EMGSpectatorCameraMode::Cockpit:
		return NSLOCTEXT("Spectator", "Cockpit", "Cockpit");
	case EMGSpectatorCameraMode::Hood:
		return NSLOCTEXT("Spectator", "Hood", "Hood");
	case EMGSpectatorCameraMode::Bumper:
		return NSLOCTEXT("Spectator", "Bumper", "Bumper");
	case EMGSpectatorCameraMode::Broadcast:
		return NSLOCTEXT("Spectator", "Broadcast", "TV Cameras");
	case EMGSpectatorCameraMode::Helicopter:
		return NSLOCTEXT("Spectator", "Helicopter", "Helicopter");
	case EMGSpectatorCameraMode::TrackSide:
		return NSLOCTEXT("Spectator", "TrackSide", "Track Side");
	case EMGSpectatorCameraMode::Director:
		return NSLOCTEXT("Spectator", "Director", "Auto Director");
	default:
		return FText::GetEmpty();
	}
}

// ==========================================
// TARGET TRACKING
// ==========================================

void UMGSpectatorSubsystem::SetTarget(AActor* NewTarget)
{
	if (!NewTarget)
	{
		return;
	}

	// Find target in list
	int32 TargetIndex = AvailableTargets.IndexOfByPredicate([NewTarget](const FMGSpectatorTarget& T)
	{
		return T.Target == NewTarget;
	});

	if (TargetIndex == INDEX_NONE)
	{
		return;
	}

	CurrentTargetIndex = TargetIndex;
	CurrentTarget = AvailableTargets[TargetIndex];

	OnTargetChanged.Broadcast(CurrentTarget);
}

void UMGSpectatorSubsystem::CycleNextTarget()
{
	if (AvailableTargets.Num() == 0)
	{
		return;
	}

	CurrentTargetIndex = (CurrentTargetIndex + 1) % AvailableTargets.Num();
	CurrentTarget = AvailableTargets[CurrentTargetIndex];

	OnTargetChanged.Broadcast(CurrentTarget);
}

void UMGSpectatorSubsystem::CyclePreviousTarget()
{
	if (AvailableTargets.Num() == 0)
	{
		return;
	}

	CurrentTargetIndex = CurrentTargetIndex - 1;
	if (CurrentTargetIndex < 0)
	{
		CurrentTargetIndex = AvailableTargets.Num() - 1;
	}

	CurrentTarget = AvailableTargets[CurrentTargetIndex];

	OnTargetChanged.Broadcast(CurrentTarget);
}

void UMGSpectatorSubsystem::FocusOnLeader()
{
	SortTargetsByPosition();

	if (AvailableTargets.Num() > 0)
	{
		SetTarget(AvailableTargets[0].Target);
	}
}

void UMGSpectatorSubsystem::FocusOnLocalPlayer()
{
	for (const FMGSpectatorTarget& Target : AvailableTargets)
	{
		if (Target.bIsLocalPlayer)
		{
			SetTarget(Target.Target);
			return;
		}
	}
}

void UMGSpectatorSubsystem::FocusOnPosition(int32 Position)
{
	SortTargetsByPosition();

	int32 Index = Position - 1; // Position is 1-based
	if (Index >= 0 && Index < AvailableTargets.Num())
	{
		SetTarget(AvailableTargets[Index].Target);
	}
}

TArray<FMGSpectatorTarget> UMGSpectatorSubsystem::GetAllTargets() const
{
	return AvailableTargets;
}

void UMGSpectatorSubsystem::RegisterTarget(AActor* Target, FText DisplayName, bool bIsAI)
{
	if (!Target)
	{
		return;
	}

	// Check if already registered
	int32 ExistingIndex = AvailableTargets.IndexOfByPredicate([Target](const FMGSpectatorTarget& T)
	{
		return T.Target == Target;
	});

	if (ExistingIndex != INDEX_NONE)
	{
		return;
	}

	FMGSpectatorTarget NewTarget;
	NewTarget.Target = Target;
	NewTarget.TargetName = DisplayName;
	NewTarget.bIsAI = bIsAI;
	NewTarget.bIsLocalPlayer = false;

	// Check if this is the local player
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (PC->GetPawn() == Target)
		{
			NewTarget.bIsLocalPlayer = true;
		}
	}

	AvailableTargets.Add(NewTarget);
}

void UMGSpectatorSubsystem::UnregisterTarget(AActor* Target)
{
	AvailableTargets.RemoveAll([Target](const FMGSpectatorTarget& T)
	{
		return T.Target == Target;
	});

	// Update current target if needed
	if (CurrentTarget.Target == Target && AvailableTargets.Num() > 0)
	{
		CurrentTargetIndex = FMath::Clamp(CurrentTargetIndex, 0, AvailableTargets.Num() - 1);
		CurrentTarget = AvailableTargets[CurrentTargetIndex];
		OnTargetChanged.Broadcast(CurrentTarget);
	}
}

// ==========================================
// AUTO-DIRECTOR
// ==========================================

void UMGSpectatorSubsystem::EnableAutoDirector(bool bEnabled)
{
	bAutoDirectorEnabled = bEnabled;

	if (bEnabled)
	{
		SetCameraMode(EMGSpectatorCameraMode::Director);

		// Initialize timers
		DirectorNextCutTime = FMath::RandRange(DirectorCutInterval.X, DirectorCutInterval.Y);
		DirectorNextTargetTime = FMath::RandRange(DirectorTargetInterval.X, DirectorTargetInterval.Y);
	}
}

void UMGSpectatorSubsystem::SetDirectorCutInterval(float MinSeconds, float MaxSeconds)
{
	DirectorCutInterval = FVector2D(MinSeconds, MaxSeconds);
}

void UMGSpectatorSubsystem::SetDirectorTargetInterval(float MinSeconds, float MaxSeconds)
{
	DirectorTargetInterval = FVector2D(MinSeconds, MaxSeconds);
}

// ==========================================
// BROADCAST CAMERAS
// ==========================================

void UMGSpectatorSubsystem::RegisterBroadcastCamera(const FMGBroadcastCameraPoint& CameraPoint)
{
	BroadcastCameras.Add(CameraPoint);
}

void UMGSpectatorSubsystem::ClearBroadcastCameras()
{
	BroadcastCameras.Empty();
}

void UMGSpectatorSubsystem::ForceUseBroadcastCamera(int32 CameraIndex)
{
	ForcedBroadcastCameraIndex = CameraIndex;
}

// ==========================================
// OVERLAY SETTINGS
// ==========================================

void UMGSpectatorSubsystem::SetOverlaySettings(const FMGSpectatorOverlay& Settings)
{
	OverlaySettings = Settings;
}

void UMGSpectatorSubsystem::ToggleOverlayElement(FName ElementName)
{
	if (ElementName == TEXT("Standings"))
	{
		OverlaySettings.bShowStandings = !OverlaySettings.bShowStandings;
	}
	else if (ElementName == TEXT("TargetInfo"))
	{
		OverlaySettings.bShowTargetInfo = !OverlaySettings.bShowTargetInfo;
	}
	else if (ElementName == TEXT("LapTimes"))
	{
		OverlaySettings.bShowLapTimes = !OverlaySettings.bShowLapTimes;
	}
	else if (ElementName == TEXT("Speedometer"))
	{
		OverlaySettings.bShowSpeedometer = !OverlaySettings.bShowSpeedometer;
	}
	else if (ElementName == TEXT("MiniMap"))
	{
		OverlaySettings.bShowMiniMap = !OverlaySettings.bShowMiniMap;
	}
	else if (ElementName == TEXT("CameraInfo"))
	{
		OverlaySettings.bShowCameraInfo = !OverlaySettings.bShowCameraInfo;
	}
}

// ==========================================
// FREE CAM CONTROL
// ==========================================

void UMGSpectatorSubsystem::SetFreeCamPosition(FVector Position, FRotator Rotation)
{
	if (SpectatorPawn)
	{
		SpectatorPawn->SetActorLocation(Position);
		SpectatorPawn->GetController()->SetControlRotation(Rotation);
	}
}

// ==========================================
// ORBIT CAM SETTINGS
// ==========================================

void UMGSpectatorSubsystem::SetOrbitAngle(float YawAngle, float PitchAngle)
{
	OrbitYaw = YawAngle;
	OrbitPitch = PitchAngle;
}

void UMGSpectatorSubsystem::SetOrbitAutoRotate(bool bEnabled, float Speed)
{
	bOrbitAutoRotate = bEnabled;
	OrbitAutoRotateSpeed = Speed;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGSpectatorSubsystem::UpdateAutoDirector(float DeltaTime)
{
	// Update camera cut timer
	DirectorNextCutTime -= DeltaTime;
	if (DirectorNextCutTime <= 0.0f)
	{
		// Select new camera mode
		EMGSpectatorCameraMode NewMode = SelectDramaticCameraMode();
		SetCameraMode(NewMode);

		DirectorNextCutTime = FMath::RandRange(DirectorCutInterval.X, DirectorCutInterval.Y);
	}

	// Update target switch timer
	DirectorNextTargetTime -= DeltaTime;
	if (DirectorNextTargetTime <= 0.0f)
	{
		// Select interesting target
		AActor* NewTarget = SelectInterestingTarget();
		if (NewTarget && NewTarget != CurrentTarget.Target)
		{
			SetTarget(NewTarget);
		}

		DirectorNextTargetTime = FMath::RandRange(DirectorTargetInterval.X, DirectorTargetInterval.Y);
	}
}

void UMGSpectatorSubsystem::UpdateCameraPosition(float DeltaTime)
{
	switch (CurrentCameraMode)
	{
	case EMGSpectatorCameraMode::FreeCam:
		// Controlled by player input, no auto-update needed
		break;

	case EMGSpectatorCameraMode::Chase:
		UpdateChaseCamera(DeltaTime);
		break;

	case EMGSpectatorCameraMode::Orbit:
		UpdateOrbitCamera(DeltaTime);
		break;

	case EMGSpectatorCameraMode::Broadcast:
		UpdateBroadcastCamera(DeltaTime);
		break;

	case EMGSpectatorCameraMode::Helicopter:
		UpdateHelicopterCamera(DeltaTime);
		break;

	case EMGSpectatorCameraMode::TrackSide:
		UpdateTracksideCamera(DeltaTime);
		break;

	case EMGSpectatorCameraMode::Director:
		// Director mode delegates to other camera modes
		break;

	default:
		// Cockpit, Hood, Bumper - handled by target vehicle
		break;
	}
}

void UMGSpectatorSubsystem::UpdateChaseCamera(float DeltaTime)
{
	if (!SpectatorPawn || !CurrentTarget.Target)
	{
		return;
	}

	// Get target transform
	FVector TargetLocation = CurrentTarget.Target->GetActorLocation();
	FVector TargetForward = CurrentTarget.Target->GetActorForwardVector();
	FVector TargetUp = CurrentTarget.Target->GetActorUpVector();

	// Calculate desired camera position
	FVector DesiredLocation = TargetLocation - (TargetForward * ChaseDistance) + (TargetUp * ChaseHeight);

	// Smooth interpolation
	FVector CurrentLocation = SpectatorPawn->GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, DesiredLocation, DeltaTime, ChaseLagSpeed);

	// Look at target
	FRotator LookAtRotation = (TargetLocation - NewLocation).Rotation();

	SpectatorPawn->SetActorLocation(NewLocation);
	if (APlayerController* PC = Cast<APlayerController>(SpectatorPawn->GetController()))
	{
		PC->SetControlRotation(LookAtRotation);
	}
}

void UMGSpectatorSubsystem::UpdateOrbitCamera(float DeltaTime)
{
	if (!SpectatorPawn || !CurrentTarget.Target)
	{
		return;
	}

	// Auto-rotate if enabled
	if (bOrbitAutoRotate)
	{
		OrbitYaw += OrbitAutoRotateSpeed * DeltaTime;
	}

	// Get target location
	FVector TargetLocation = CurrentTarget.Target->GetActorLocation();

	// Calculate orbit position
	FRotator OrbitRotation(OrbitPitch, OrbitYaw, 0.0f);
	FVector OrbitOffset = OrbitRotation.Vector() * OrbitDistance;

	FVector CameraLocation = TargetLocation - OrbitOffset;

	// Look at target
	FRotator LookAtRotation = (TargetLocation - CameraLocation).Rotation();

	SpectatorPawn->SetActorLocation(CameraLocation);
	if (APlayerController* PC = Cast<APlayerController>(SpectatorPawn->GetController()))
	{
		PC->SetControlRotation(LookAtRotation);
	}
}

void UMGSpectatorSubsystem::UpdateBroadcastCamera(float DeltaTime)
{
	if (!SpectatorPawn || BroadcastCameras.Num() == 0)
	{
		return;
	}

	// Select best camera if not forced
	int32 CameraIndex = ForcedBroadcastCameraIndex >= 0 ? ForcedBroadcastCameraIndex : SelectBestBroadcastCamera();

	if (CameraIndex < 0 || CameraIndex >= BroadcastCameras.Num())
	{
		return;
	}

	const FMGBroadcastCameraPoint& Camera = BroadcastCameras[CameraIndex];

	// Set camera position
	SpectatorPawn->SetActorLocation(Camera.Location);

	// Auto-track target if enabled
	if (Camera.bAutoTrack && CurrentTarget.Target)
	{
		FVector TargetLocation = CurrentTarget.Target->GetActorLocation();
		FRotator LookAtRotation = (TargetLocation - Camera.Location).Rotation();

		if (APlayerController* PC = Cast<APlayerController>(SpectatorPawn->GetController()))
		{
			PC->SetControlRotation(LookAtRotation);
		}
	}
	else
	{
		if (APlayerController* PC = Cast<APlayerController>(SpectatorPawn->GetController()))
		{
			PC->SetControlRotation(Camera.Rotation);
		}
	}
}

void UMGSpectatorSubsystem::UpdateHelicopterCamera(float DeltaTime)
{
	if (!SpectatorPawn || !CurrentTarget.Target)
	{
		return;
	}

	// High overhead position
	FVector TargetLocation = CurrentTarget.Target->GetActorLocation();
	FVector CameraLocation = TargetLocation + FVector(0, 0, 2000.0f) + FVector(-500.0f, 0, 0);

	// Slow follow
	FVector CurrentLocation = SpectatorPawn->GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, CameraLocation, DeltaTime, 2.0f);

	// Look down at target
	FRotator LookAtRotation = (TargetLocation - NewLocation).Rotation();

	SpectatorPawn->SetActorLocation(NewLocation);
	if (APlayerController* PC = Cast<APlayerController>(SpectatorPawn->GetController()))
	{
		PC->SetControlRotation(LookAtRotation);
	}
}

void UMGSpectatorSubsystem::UpdateTracksideCamera(float DeltaTime)
{
	// Similar to broadcast but using fixed track-side positions
	UpdateBroadcastCamera(DeltaTime);
}

void UMGSpectatorSubsystem::UpdateCameraTransition(float DeltaTime)
{
	TransitionProgress += DeltaTime / TransitionDuration;

	if (TransitionProgress >= 1.0f)
	{
		TransitionProgress = 1.0f;
		bIsTransitioning = false;
	}

	// Smooth step interpolation
	float Alpha = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);

	// Interpolate transform
	FVector NewLocation = FMath::Lerp(TransitionStartTransform.GetLocation(), TransitionEndTransform.GetLocation(), Alpha);
	FQuat NewRotation = FQuat::Slerp(TransitionStartTransform.GetRotation(), TransitionEndTransform.GetRotation(), Alpha);

	if (SpectatorPawn)
	{
		SpectatorPawn->SetActorLocation(NewLocation);
		if (APlayerController* PC = Cast<APlayerController>(SpectatorPawn->GetController()))
		{
			PC->SetControlRotation(NewRotation.Rotator());
		}
	}
}

int32 UMGSpectatorSubsystem::SelectBestBroadcastCamera()
{
	if (BroadcastCameras.Num() == 0 || !CurrentTarget.Target)
	{
		return -1;
	}

	// Simple selection: closest camera to target
	FVector TargetLocation = CurrentTarget.Target->GetActorLocation();
	int32 BestIndex = 0;
	float BestScore = -1.0f;

	for (int32 i = 0; i < BroadcastCameras.Num(); i++)
	{
		const FMGBroadcastCameraPoint& Camera = BroadcastCameras[i];

		float Distance = FVector::Dist(Camera.Location, TargetLocation);

		// Score based on distance and priority
		float Score = Camera.Priority * 1000.0f / (Distance + 100.0f);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestIndex = i;
		}
	}

	return BestIndex;
}

AActor* UMGSpectatorSubsystem::SelectInterestingTarget()
{
	if (AvailableTargets.Num() == 0)
	{
		return nullptr;
	}

	// Prioritize:
	// 1. Battles for position (close gaps)
	// 2. Leader
	// 3. Local player

	// For now, simple random selection
	int32 RandomIndex = FMath::RandRange(0, AvailableTargets.Num() - 1);
	return AvailableTargets[RandomIndex].Target;
}

EMGSpectatorCameraMode UMGSpectatorSubsystem::SelectDramaticCameraMode()
{
	// Select camera mode for dramatic effect
	TArray<EMGSpectatorCameraMode> DramaticModes = {
		EMGSpectatorCameraMode::Chase,
		EMGSpectatorCameraMode::Broadcast,
		EMGSpectatorCameraMode::Helicopter,
		EMGSpectatorCameraMode::Bumper,
		EMGSpectatorCameraMode::Hood
	};

	// Don't repeat same mode
	TArray<EMGSpectatorCameraMode> FilteredModes;
	for (EMGSpectatorCameraMode Mode : DramaticModes)
	{
		if (Mode != CurrentCameraMode)
		{
			FilteredModes.Add(Mode);
		}
	}

	if (FilteredModes.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, FilteredModes.Num() - 1);
		return FilteredModes[Index];
	}

	return EMGSpectatorCameraMode::Chase;
}

void UMGSpectatorSubsystem::UpdateTargetInfo()
{
	// Update current target info (speed, position, lap, etc.)
	// This would typically query the race manager for updated data

	// Update all targets
	for (FMGSpectatorTarget& Target : AvailableTargets)
	{
		if (!Target.Target)
		{
			continue;
		}

		// Get vehicle speed if available
		// Target.CurrentSpeed = ...

		// Get race position if available
		// Target.RacePosition = ...

		// Get lap info if available
		// Target.CurrentLap = ...
	}

	// Update current target reference
	if (CurrentTargetIndex >= 0 && CurrentTargetIndex < AvailableTargets.Num())
	{
		CurrentTarget = AvailableTargets[CurrentTargetIndex];
	}
}

void UMGSpectatorSubsystem::SortTargetsByPosition()
{
	AvailableTargets.Sort([](const FMGSpectatorTarget& A, const FMGSpectatorTarget& B)
	{
		return A.RacePosition < B.RacePosition;
	});
}

void UMGSpectatorSubsystem::SpawnSpectatorPawn(APlayerController* Controller)
{
	if (!Controller || !SpectatorPawnClass)
	{
		// Use default spectator pawn
		SpectatorPawnClass = AMGSpectatorPawn::StaticClass();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn at current camera location
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (Controller->GetPawn())
	{
		SpawnLocation = Controller->GetPawn()->GetActorLocation();
		SpawnRotation = Controller->GetControlRotation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpectatorPawn = World->SpawnActor<AMGSpectatorPawn>(SpectatorPawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpectatorPawn)
	{
		Controller->Possess(SpectatorPawn);
	}
}

void UMGSpectatorSubsystem::DestroySpectatorPawn()
{
	if (SpectatorPawn)
	{
		SpectatorPawn->Destroy();
		SpectatorPawn = nullptr;
	}
}

void UMGSpectatorSubsystem::BeginCameraTransition(const FTransform& StartTransform, const FTransform& EndTransform, float Duration)
{
	TransitionStartTransform = StartTransform;
	TransitionEndTransform = EndTransform;
	TransitionDuration = Duration;
	TransitionProgress = 0.0f;
	bIsTransitioning = true;
}
