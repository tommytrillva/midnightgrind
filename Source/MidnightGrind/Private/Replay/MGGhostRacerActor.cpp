// Copyright Midnight Grind. All Rights Reserved.

#include "Replay/MGGhostRacerActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AMGGhostRacerActor::AMGGhostRacerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// Create skeletal mesh
	GhostMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GhostMesh"));
	GhostMesh->SetupAttachment(RootComponent);
	GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create static mesh
	GhostStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GhostStaticMesh"));
	GhostStaticMesh->SetupAttachment(RootComponent);
	GhostStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GhostStaticMesh->SetVisibility(false);

	// Create delta widget
	DeltaWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DeltaWidget"));
	DeltaWidget->SetupAttachment(RootComponent);
	DeltaWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	DeltaWidget->SetDrawSize(FVector2D(200.0f, 50.0f));
	DeltaWidget->SetWidgetSpace(EWidgetSpace::Screen);
}

void AMGGhostRacerActor::BeginPlay()
{
	Super::BeginPlay();

	SetupMaterial();
}

void AMGGhostRacerActor::Tick(float MGDeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EMGGhostState::Playing)
	{
		// Advance time
		CurrentTime += DeltaTime * PlaybackSpeed;

		// Check for end
		if (CurrentTime >= ReplayData.GetDuration())
		{
			CurrentTime = ReplayData.GetDuration();
			CurrentState = EMGGhostState::Finished;
		}
	}

	// Always update transform for smooth interpolation
	UpdateTransform(DeltaTime);
}

// ==========================================
// INITIALIZATION
// ==========================================

void AMGGhostRacerActor::InitializeGhost(const FMGGhostConfig& Config)
{
	GhostConfig = Config;
	ReplayData = Config.ReplayData;

	// Set appearance
	SetTransparency(Config.Transparency);
	SetGhostColor(Config.GhostColor);

	// Set collision
	if (Config.bEnableCollision)
	{
		GhostMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GhostStaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Show/hide delta widget
	SetDeltaWidgetVisible(Config.bShowDelta);

	// Build distance lookup
	BuildDistanceLookup();

	// Set initial position
	if (ReplayData.Frames.Num() > 0)
	{
		const FMGReplayFrame& FirstFrame = ReplayData.Frames[0];
		SetActorLocation(FirstFrame.Position);
		SetActorRotation(FirstFrame.Rotation);
		TargetPosition = FirstFrame.Position;
		TargetRotation = FirstFrame.Rotation;
		PreviousPosition = FirstFrame.Position;
	}

	CurrentState = EMGGhostState::Waiting;
}

void AMGGhostRacerActor::SetGhostMesh(USkeletalMesh* Mesh)
{
	if (GhostMesh && Mesh)
	{
		GhostMesh->SetSkeletalMesh(Mesh);
		GhostMesh->SetVisibility(true);
		GhostStaticMesh->SetVisibility(false);

		SetupMaterial();
	}
}

void AMGGhostRacerActor::SetGhostStaticMesh(UStaticMesh* Mesh)
{
	if (GhostStaticMesh && Mesh)
	{
		GhostStaticMesh->SetStaticMesh(Mesh);
		GhostStaticMesh->SetVisibility(true);
		GhostMesh->SetVisibility(false);

		SetupMaterial();
	}
}

// ==========================================
// PLAYBACK CONTROL
// ==========================================

void AMGGhostRacerActor::StartPlayback()
{
	CurrentState = EMGGhostState::Playing;
}

void AMGGhostRacerActor::PausePlayback()
{
	if (CurrentState == EMGGhostState::Playing)
	{
		CurrentState = EMGGhostState::Paused;
	}
}

void AMGGhostRacerActor::ResumePlayback()
{
	if (CurrentState == EMGGhostState::Paused)
	{
		CurrentState = EMGGhostState::Playing;
	}
}

void AMGGhostRacerActor::ResetPlayback()
{
	CurrentTime = 0.0f;
	DistanceTraveled = 0.0f;

	if (ReplayData.Frames.Num() > 0)
	{
		const FMGReplayFrame& FirstFrame = ReplayData.Frames[0];
		SetActorLocation(FirstFrame.Position);
		SetActorRotation(FirstFrame.Rotation);
		TargetPosition = FirstFrame.Position;
		TargetRotation = FirstFrame.Rotation;
		PreviousPosition = FirstFrame.Position;
	}

	CurrentState = EMGGhostState::Waiting;
}

void AMGGhostRacerActor::SeekToTime(float Time)
{
	CurrentTime = FMath::Clamp(Time, 0.0f, ReplayData.GetDuration());

	// Update position immediately
	if (ReplayData.IsValid())
	{
		FMGReplayFrame Frame = ReplayData.GetFrameAtTime(CurrentTime);
		SetActorLocation(Frame.Position);
		SetActorRotation(Frame.Rotation);
		TargetPosition = Frame.Position;
		TargetRotation = Frame.Rotation;
	}
}

void AMGGhostRacerActor::SetPlaybackSpeed(float Speed)
{
	PlaybackSpeed = FMath::Clamp(Speed, 0.1f, 4.0f);
}

void AMGGhostRacerActor::SyncWithRaceTime(float RaceTime)
{
	CurrentTime = RaceTime;

	if (CurrentTime >= ReplayData.GetDuration())
	{
		CurrentState = EMGGhostState::Finished;
	}
	else if (CurrentState == EMGGhostState::Waiting)
	{
		CurrentState = EMGGhostState::Playing;
	}
}

// ==========================================
// QUERY
// ==========================================

float AMGGhostRacerActor::GetPlaybackProgress() const
{
	float Duration = ReplayData.GetDuration();
	return Duration > 0.0f ? CurrentTime / Duration : 0.0f;
}

float AMGGhostRacerActor::GetDeltaAtDistance(float PlayerDistance) const
{
	if (DistanceAtTime.Num() == 0)
	{
		return 0.0f;
	}

	// Get ghost time at player's distance
	float GhostTimeAtPlayerDistance = GetTimeAtDistance(PlayerDistance);

	// Delta = ghost time - current race time
	// Positive = ghost reached this distance earlier (ghost ahead)
	// Negative = ghost reached this distance later (player ahead)
	return GhostTimeAtPlayerDistance - CurrentTime;
}

bool AMGGhostRacerActor::IsAheadOfPosition(FVector Position) const
{
	// Simple check: compare distances traveled
	// This would be more accurate with track distance calculation
	return DistanceTraveled > FVector::Dist(FVector::ZeroVector, Position);
}

// ==========================================
// APPEARANCE
// ==========================================

void AMGGhostRacerActor::SetTransparency(float Transparency)
{
	CurrentTransparency = FMath::Clamp(Transparency, 0.0f, 1.0f);
	UpdateAppearance();
}

void AMGGhostRacerActor::SetGhostColor(FLinearColor Color)
{
	CurrentColor = Color;
	UpdateAppearance();
}

void AMGGhostRacerActor::SetGhostVisible(bool bVisible)
{
	GhostMesh->SetVisibility(bVisible);
	GhostStaticMesh->SetVisibility(bVisible);
}

void AMGGhostRacerActor::SetDeltaWidgetVisible(bool bVisible)
{
	if (DeltaWidget)
	{
		DeltaWidget->SetVisibility(bVisible);
	}
}

// ==========================================
// INTERNAL
// ==========================================

void AMGGhostRacerActor::UpdateTransform(float MGDeltaTime)
{
	if (!ReplayData.IsValid())
	{
		return;
	}

	// Get interpolated frame at current time
	FMGReplayFrame Frame = ReplayData.GetFrameAtTime(CurrentTime);

	TargetPosition = Frame.Position;
	TargetRotation = Frame.Rotation;

	// Smooth interpolation
	FVector CurrentPos = GetActorLocation();
	FRotator CurrentRot = GetActorRotation();

	FVector NewPos = FMath::VInterpTo(CurrentPos, TargetPosition, DeltaTime, InterpolationSpeed);
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRotation, DeltaTime, InterpolationSpeed);

	SetActorLocation(NewPos);
	SetActorRotation(NewRot);

	// Update distance traveled
	float FrameDistance = FVector::Dist(PreviousPosition, NewPos);
	DistanceTraveled += FrameDistance;
	PreviousPosition = NewPos;
}

void AMGGhostRacerActor::UpdateAppearance()
{
	if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), CurrentTransparency);
		DynamicMaterial->SetVectorParameterValue(TEXT("GhostColor"), CurrentColor);
	}
}

void AMGGhostRacerActor::BuildDistanceLookup()
{
	DistanceAtTime.Empty();

	if (!ReplayData.IsValid() || ReplayData.Frames.Num() == 0)
	{
		return;
	}

	DistanceAtTime.Reserve(ReplayData.Frames.Num());

	float TotalDistance = 0.0f;
	FVector PrevPos = ReplayData.Frames[0].Position;

	for (const FMGReplayFrame& Frame : ReplayData.Frames)
	{
		float FrameDist = FVector::Dist(PrevPos, Frame.Position);
		TotalDistance += FrameDist;
		DistanceAtTime.Add(TotalDistance);
		PrevPos = Frame.Position;
	}
}

float AMGGhostRacerActor::GetTimeAtDistance(float Distance) const
{
	if (DistanceAtTime.Num() == 0)
	{
		return 0.0f;
	}

	// Binary search for distance
	int32 Low = 0;
	int32 High = DistanceAtTime.Num() - 1;

	while (Low < High)
	{
		int32 Mid = (Low + High) / 2;
		if (DistanceAtTime[Mid] < Distance)
		{
			Low = Mid + 1;
		}
		else
		{
			High = Mid;
		}
	}

	// Interpolate between frames
	if (Low > 0)
	{
		float PrevDist = DistanceAtTime[Low - 1];
		float CurrDist = DistanceAtTime[Low];
		float Alpha = (Distance - PrevDist) / (CurrDist - PrevDist);
		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

		float PrevTime = ReplayData.Frames[Low - 1].Timestamp;
		float CurrTime = ReplayData.Frames[Low].Timestamp;

		return FMath::Lerp(PrevTime, CurrTime, Alpha);
	}

	return ReplayData.Frames[Low].Timestamp;
}

void AMGGhostRacerActor::SetupMaterial()
{
	if (!GhostMaterial)
	{
		return;
	}

	DynamicMaterial = UMaterialInstanceDynamic::Create(GhostMaterial, this);

	if (DynamicMaterial)
	{
		// Apply to skeletal mesh
		if (GhostMesh && GhostMesh->GetSkeletalMeshAsset())
		{
			int32 NumMaterials = GhostMesh->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; ++i)
			{
				GhostMesh->SetMaterial(i, DynamicMaterial);
			}
		}

		// Apply to static mesh
		if (GhostStaticMesh && GhostStaticMesh->GetStaticMesh())
		{
			int32 NumMaterials = GhostStaticMesh->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; ++i)
			{
				GhostStaticMesh->SetMaterial(i, DynamicMaterial);
			}
		}

		UpdateAppearance();
	}
}
