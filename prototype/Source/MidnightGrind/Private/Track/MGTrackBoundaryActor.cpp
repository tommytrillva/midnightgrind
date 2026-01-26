// Copyright Midnight Grind. All Rights Reserved.

#include "Track/MGTrackBoundaryActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

AMGTrackBoundaryActor::AMGTrackBoundaryActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // 20Hz update

	// Create boundary spline
	BoundarySpline = CreateDefaultSubobject<USplineComponent>(TEXT("BoundarySpline"));
	RootComponent = BoundarySpline;

	BoundarySpline->SetClosedLoop(bIsClosedLoop);
}

void AMGTrackBoundaryActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BuildVisuals();
}

void AMGTrackBoundaryActor::BeginPlay()
{
	Super::BeginPlay();

	// Set up collision
	BoundarySpline->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoundarySpline->SetGenerateOverlapEvents(true);

	BoundarySpline->OnComponentBeginOverlap.AddDynamic(this, &AMGTrackBoundaryActor::OnBoundaryOverlapBegin);
	BoundarySpline->OnComponentEndOverlap.AddDynamic(this, &AMGTrackBoundaryActor::OnBoundaryOverlapEnd);

	// Set visibility
	SetBoundaryVisible(bShowInGame);
}

void AMGTrackBoundaryActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bBoundaryEnabled)
	{
		UpdateBoundaryActors(DeltaTime);
	}
}

// ==========================================
// QUERY
// ==========================================

FVector AMGTrackBoundaryActor::GetClosestPointOnBoundary(FVector WorldPosition) const
{
	if (!BoundarySpline)
	{
		return WorldPosition;
	}

	float Key = BoundarySpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	return BoundarySpline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
}

float AMGTrackBoundaryActor::GetDistanceToBoundary(FVector WorldPosition) const
{
	FVector ClosestPoint = GetClosestPointOnBoundary(WorldPosition);
	return FVector::Dist(WorldPosition, ClosestPoint);
}

FVector AMGTrackBoundaryActor::GetBoundaryNormalAtPoint(FVector WorldPosition) const
{
	if (!BoundarySpline)
	{
		return FVector::RightVector;
	}

	float Key = BoundarySpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	FVector SplineRight = BoundarySpline->GetRightVectorAtSplineInputKey(Key, ESplineCoordinateSpace::World);

	// Normal points toward track (inward)
	return bIsLeftBoundary ? SplineRight : -SplineRight;
}

bool AMGTrackBoundaryActor::IsPositionOffTrack(FVector WorldPosition) const
{
	if (!BoundarySpline)
	{
		return false;
	}

	FVector ClosestPoint = GetClosestPointOnBoundary(WorldPosition);
	FVector ToBoundary = WorldPosition - ClosestPoint;
	FVector Normal = GetBoundaryNormalAtPoint(WorldPosition);

	// Off-track if position is on opposite side of normal (outside boundary)
	float Dot = FVector::DotProduct(ToBoundary, Normal);
	return Dot < 0.0f;
}

FMGBoundaryHitResult AMGTrackBoundaryActor::GetRecoveryInfo(FVector WorldPosition, FVector Velocity) const
{
	FMGBoundaryHitResult Result;

	if (!BoundarySpline)
	{
		Result.RecoveryPosition = WorldPosition;
		return Result;
	}

	float Key = BoundarySpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	FVector ClosestPoint = BoundarySpline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);

	Result.HitLocation = ClosestPoint;
	Result.HitNormal = GetBoundaryNormalAtPoint(WorldPosition);
	Result.BoundaryType = BoundaryType;
	Result.SpeedPenaltyMultiplier = SpeedPenaltyMultiplier;
	Result.DistanceAlongBoundary = BoundarySpline->GetDistanceAlongSplineAtSplineInputKey(Key);

	// Calculate recovery position (on track, facing forward)
	FVector SplineForward = BoundarySpline->GetDirectionAtSplineInputKey(Key, ESplineCoordinateSpace::World);

	// Recovery position is on the boundary, offset toward track center
	float RecoveryOffset = BoundaryWidth * 2.0f;
	Result.RecoveryPosition = ClosestPoint + Result.HitNormal * RecoveryOffset;
	Result.RecoveryPosition.Z = WorldPosition.Z; // Maintain height

	// Recovery rotation faces along the track
	Result.RecoveryRotation = SplineForward.Rotation();

	return Result;
}

float AMGTrackBoundaryActor::GetBoundaryLength() const
{
	if (!BoundarySpline)
	{
		return 0.0f;
	}

	return BoundarySpline->GetSplineLength();
}

// ==========================================
// MODIFICATION
// ==========================================

void AMGTrackBoundaryActor::SetBoundaryType(EMGBoundaryType NewType)
{
	BoundaryType = NewType;
}

void AMGTrackBoundaryActor::SetSpeedPenalty(float Multiplier)
{
	SpeedPenaltyMultiplier = FMath::Clamp(Multiplier, 0.0f, 1.0f);
}

void AMGTrackBoundaryActor::SetBoundaryEnabled(bool bEnabled)
{
	bBoundaryEnabled = bEnabled;

	if (!bEnabled)
	{
		// Clear all actors when disabled
		ActorsInBoundary.Empty();
	}
}

void AMGTrackBoundaryActor::SetBoundaryVisible(bool bVisible)
{
	for (USplineMeshComponent* MeshComp : SplineMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->SetVisibility(bVisible);
		}
	}
}

// ==========================================
// INTERNAL
// ==========================================

void AMGTrackBoundaryActor::BuildVisuals()
{
	ClearVisuals();

	if (!BoundarySpline || !BoundaryMesh)
	{
		return;
	}

	// Create spline meshes along the boundary
	float SplineLength = BoundarySpline->GetSplineLength();
	float MeshLength = 500.0f; // Length of each mesh segment
	int32 NumSegments = FMath::CeilToInt(SplineLength / MeshLength);

	for (int32 i = 0; i < NumSegments; ++i)
	{
		float StartDist = i * MeshLength;
		float EndDist = FMath::Min((i + 1) * MeshLength, SplineLength);

		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
		if (SplineMesh)
		{
			SplineMesh->SetupAttachment(RootComponent);
			SplineMesh->RegisterComponent();

			SplineMesh->SetStaticMesh(BoundaryMesh);
			if (BoundaryMaterial)
			{
				SplineMesh->SetMaterial(0, BoundaryMaterial);
			}

			// Set start and end positions
			FVector StartPos = BoundarySpline->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
			FVector StartTangent = BoundarySpline->GetTangentAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
			FVector EndPos = BoundarySpline->GetLocationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);
			FVector EndTangent = BoundarySpline->GetTangentAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);

			SplineMesh->SetStartAndEnd(StartPos, StartTangent.GetSafeNormal() * MeshLength,
				EndPos, EndTangent.GetSafeNormal() * MeshLength);

			SplineMesh->SetVisibility(bShowInGame);

			SplineMeshComponents.Add(SplineMesh);
		}
	}
}

void AMGTrackBoundaryActor::ClearVisuals()
{
	for (USplineMeshComponent* MeshComp : SplineMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}
	SplineMeshComponents.Empty();
}

void AMGTrackBoundaryActor::OnBoundaryOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || !bBoundaryEnabled)
	{
		return;
	}

	// Only track pawns (vehicles)
	if (!Cast<APawn>(OtherActor))
	{
		return;
	}

	// Add to tracking
	if (!ActorsInBoundary.Contains(OtherActor))
	{
		ActorsInBoundary.Add(OtherActor, 0.0f);
		OnBoundaryEnter.Broadcast(OtherActor, BoundaryType);
	}
}

void AMGTrackBoundaryActor::OnBoundaryOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	float* TimeInBoundary = ActorsInBoundary.Find(OtherActor);
	if (TimeInBoundary)
	{
		OnBoundaryExit.Broadcast(OtherActor, *TimeInBoundary);
		ActorsInBoundary.Remove(OtherActor);
	}
}

void AMGTrackBoundaryActor::UpdateBoundaryActors(float DeltaTime)
{
	TArray<AActor*> ActorsToRemove;

	for (auto& Pair : ActorsInBoundary)
	{
		AActor* Actor = Pair.Key;
		float& TimeInBoundary = Pair.Value;

		if (!Actor || !IsValid(Actor))
		{
			ActorsToRemove.Add(Actor);
			continue;
		}

		TimeInBoundary += DeltaTime;

		ApplyBoundaryEffect(Actor, DeltaTime);

		// Kill zone check
		if (BoundaryType == EMGBoundaryType::KillZone && TimeInBoundary >= KillZoneDelay)
		{
			// Vehicle needs to be reset - broadcast hit event
			FMGBoundaryHitResult HitResult = GetRecoveryInfo(Actor->GetActorLocation(), Actor->GetVelocity());
			OnBoundaryHit.Broadcast(Actor, HitResult, 0.0f);
		}
	}

	for (AActor* Actor : ActorsToRemove)
	{
		ActorsInBoundary.Remove(Actor);
	}
}

void AMGTrackBoundaryActor::ApplyBoundaryEffect(AActor* Actor, float DeltaTime)
{
	if (!Actor)
	{
		return;
	}

	FVector Velocity = Actor->GetVelocity();
	FVector Normal = GetBoundaryNormalAtPoint(Actor->GetActorLocation());
	float ImpactForce = CalculateImpactForce(Velocity, Normal);

	switch (BoundaryType)
	{
		case EMGBoundaryType::Soft:
		{
			// Speed penalty is applied elsewhere via the penalty multiplier
			// Just broadcast the hit for effects
			if (ImpactForce > 100.0f)
			{
				FMGBoundaryHitResult HitResult = GetRecoveryInfo(Actor->GetActorLocation(), Velocity);
				OnBoundaryHit.Broadcast(Actor, HitResult, ImpactForce);
			}
			break;
		}

		case EMGBoundaryType::Hard:
		{
			// Apply bounce if moving into boundary
			float VelocityTowardsBoundary = -FVector::DotProduct(Velocity, Normal);
			if (VelocityTowardsBoundary > 0.0f)
			{
				FMGBoundaryHitResult HitResult = GetRecoveryInfo(Actor->GetActorLocation(), Velocity);
				OnBoundaryHit.Broadcast(Actor, HitResult, ImpactForce);

				// Bounce calculation would be applied by the vehicle's physics component
				// This just notifies that a collision occurred
			}
			break;
		}

		case EMGBoundaryType::Invisible:
		{
			// Will be teleported by the track subsystem
			FMGBoundaryHitResult HitResult = GetRecoveryInfo(Actor->GetActorLocation(), Velocity);
			OnBoundaryHit.Broadcast(Actor, HitResult, ImpactForce);
			break;
		}

		case EMGBoundaryType::KillZone:
		{
			// Handled in UpdateBoundaryActors with delay
			break;
		}
	}
}

float AMGTrackBoundaryActor::CalculateImpactForce(FVector Velocity, FVector Normal) const
{
	// Force is the velocity component perpendicular to boundary
	float NormalVelocity = -FVector::DotProduct(Velocity, Normal);
	return FMath::Max(0.0f, NormalVelocity);
}
