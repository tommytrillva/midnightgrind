// Copyright Midnight Grind. All Rights Reserved.

#include "Navigation/MGNavigationSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGNavigationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMGNavigationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
	Super::Deinitialize();
}

void UMGNavigationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	InWorld.GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]() { UpdateNavigation(); },
		0.1f,
		true
	);
}

// Destination
bool UMGNavigationSubsystem::SetDestination(FName POIID)
{
	FMGPointOfInterest* POI = RegisteredPOIs.Find(POIID);
	if (!POI)
	{
		return false;
	}

	CurrentDestinationPOI = *POI;
	bHasDestination = true;
	CurrentRoute = CalculateRoute(PlayerPosition, POI->WorldLocation);
	CurrentWaypointIndex = 0;

	OnDestinationSet.Broadcast(CurrentDestinationPOI);
	return true;
}

bool UMGNavigationSubsystem::SetDestinationLocation(FVector Location)
{
	FMGPointOfInterest CustomPOI;
	CustomPOI.POIID = FName("CustomDestination");
	CustomPOI.DisplayName = FText::FromString("Waypoint");
	CustomPOI.Type = EMGPOIType::Custom;
	CustomPOI.WorldLocation = Location;
	CustomPOI.bShowOnMap = true;

	CurrentDestinationPOI = CustomPOI;
	bHasDestination = true;
	CurrentRoute = CalculateRoute(PlayerPosition, Location);
	CurrentWaypointIndex = 0;

	OnDestinationSet.Broadcast(CurrentDestinationPOI);
	return true;
}

void UMGNavigationSubsystem::ClearDestination()
{
	bHasDestination = false;
	CurrentDestinationPOI = FMGPointOfInterest();
	CurrentRoute = FMGNavRoute();
	CurrentWaypointIndex = 0;

	OnDestinationCleared.Broadcast();
}

FMGPointOfInterest UMGNavigationSubsystem::GetCurrentDestination() const
{
	return CurrentDestinationPOI;
}

float UMGNavigationSubsystem::GetDistanceToDestination() const
{
	if (!bHasDestination)
	{
		return 0.0f;
	}
	return FVector::Dist(PlayerPosition, CurrentDestinationPOI.WorldLocation) / 100.0f; // cm to m
}

float UMGNavigationSubsystem::GetETAToDestination() const
{
	if (!bHasDestination || PlayerSpeed < 1.0f)
	{
		return 0.0f;
	}
	float DistanceMeters = GetDistanceToDestination();
	float SpeedMPS = PlayerSpeed * 0.44704f; // mph to m/s
	return DistanceMeters / SpeedMPS;
}

// Route
void UMGNavigationSubsystem::RecalculateRoute()
{
	if (bHasDestination)
	{
		CurrentRoute = CalculateRoute(PlayerPosition, CurrentDestinationPOI.WorldLocation);
		CurrentWaypointIndex = 0;
		OnRouteRecalculated.Broadcast(CurrentRoute);
	}
}

FMGNavWaypoint UMGNavigationSubsystem::GetNextWaypoint() const
{
	if (CurrentRoute.Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		return CurrentRoute.Waypoints[CurrentWaypointIndex];
	}
	return FMGNavWaypoint();
}

EMGNavInstruction UMGNavigationSubsystem::GetCurrentInstruction() const
{
	FMGNavWaypoint NextWP = GetNextWaypoint();
	return NextWP.Instruction;
}

float UMGNavigationSubsystem::GetDistanceToNextTurn() const
{
	if (!CurrentRoute.Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		return 0.0f;
	}
	return FVector::Dist(PlayerPosition, CurrentRoute.Waypoints[CurrentWaypointIndex].Location) / 100.0f;
}

void UMGNavigationSubsystem::SetRoutePreferences(bool bInAvoidHighways, bool bInAvoidPolice, bool bInShortestRoute)
{
	bAvoidHighways = bInAvoidHighways;
	bAvoidPolice = bInAvoidPolice;
	bShortestRoute = bInShortestRoute;

	if (bHasDestination)
	{
		RecalculateRoute();
	}
}

// POI Management
void UMGNavigationSubsystem::RegisterPOI(const FMGPointOfInterest& POI)
{
	RegisteredPOIs.Add(POI.POIID, POI);
}

void UMGNavigationSubsystem::UnregisterPOI(FName POIID)
{
	RegisteredPOIs.Remove(POIID);
}

FMGPointOfInterest UMGNavigationSubsystem::GetPOI(FName POIID) const
{
	const FMGPointOfInterest* POI = RegisteredPOIs.Find(POIID);
	return POI ? *POI : FMGPointOfInterest();
}

TArray<FMGPointOfInterest> UMGNavigationSubsystem::GetAllPOIs() const
{
	TArray<FMGPointOfInterest> Result;
	RegisteredPOIs.GenerateValueArray(Result);
	return Result;
}

TArray<FMGPointOfInterest> UMGNavigationSubsystem::GetPOIsByType(EMGPOIType Type) const
{
	TArray<FMGPointOfInterest> Result;
	for (const auto& Pair : RegisteredPOIs)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGPointOfInterest> UMGNavigationSubsystem::GetNearbyPOIs(FVector Location, float Radius) const
{
	TArray<FMGPointOfInterest> Result;
	float RadiusCm = Radius * 100.0f;

	for (const auto& Pair : RegisteredPOIs)
	{
		if (FVector::Dist(Pair.Value.WorldLocation, Location) <= RadiusCm)
		{
			Result.Add(Pair.Value);
		}
	}

	Result.Sort([&Location](const FMGPointOfInterest& A, const FMGPointOfInterest& B)
	{
		return FVector::DistSquared(A.WorldLocation, Location) < FVector::DistSquared(B.WorldLocation, Location);
	});

	return Result;
}

void UMGNavigationSubsystem::DiscoverPOI(FName POIID)
{
	FMGPointOfInterest* POI = RegisteredPOIs.Find(POIID);
	if (POI && !POI->bDiscovered)
	{
		POI->bDiscovered = true;
		OnPOIDiscovered.Broadcast(*POI);
	}
}

void UMGNavigationSubsystem::CompletePOI(FName POIID)
{
	FMGPointOfInterest* POI = RegisteredPOIs.Find(POIID);
	if (POI)
	{
		POI->bCompleted = true;
	}
}

// Regions
void UMGNavigationSubsystem::RegisterRegion(const FMGMapRegion& Region)
{
	RegisteredRegions.Add(Region.RegionID, Region);
}

FMGMapRegion UMGNavigationSubsystem::GetCurrentRegion() const
{
	const FMGMapRegion* Region = RegisteredRegions.Find(CurrentRegionID);
	return Region ? *Region : FMGMapRegion();
}

TArray<FMGMapRegion> UMGNavigationSubsystem::GetAllRegions() const
{
	TArray<FMGMapRegion> Result;
	RegisteredRegions.GenerateValueArray(Result);
	return Result;
}

void UMGNavigationSubsystem::UnlockRegion(FName RegionID)
{
	FMGMapRegion* Region = RegisteredRegions.Find(RegionID);
	if (Region)
	{
		Region->bUnlocked = true;
	}
}

bool UMGNavigationSubsystem::IsRegionUnlocked(FName RegionID) const
{
	const FMGMapRegion* Region = RegisteredRegions.Find(RegionID);
	return Region && Region->bUnlocked;
}

// Custom Markers
FGuid UMGNavigationSubsystem::AddCustomMarker(FVector Location, FLinearColor Color, const FText& Label)
{
	FMGMapMarker Marker;
	Marker.MarkerID = FGuid::NewGuid();
	Marker.WorldLocation = Location;
	Marker.Color = Color;
	Marker.Label = Label;
	Marker.MarkerType = EMGPOIType::Custom;
	Marker.bShowDistance = true;
	Marker.bShowDirection = true;

	CustomMarkers.Add(Marker);
	OnWaypointAdded.Broadcast(Marker);

	return Marker.MarkerID;
}

void UMGNavigationSubsystem::RemoveCustomMarker(const FGuid& MarkerID)
{
	CustomMarkers.RemoveAll([&MarkerID](const FMGMapMarker& Marker)
	{
		return Marker.MarkerID == MarkerID;
	});
}

void UMGNavigationSubsystem::ClearAllCustomMarkers()
{
	CustomMarkers.Empty();
}

TArray<FMGMapMarker> UMGNavigationSubsystem::GetAllMarkers() const
{
	return CustomMarkers;
}

// Player Position
void UMGNavigationSubsystem::UpdatePlayerPosition(FVector Position, FRotator Rotation)
{
	FVector OldPosition = PlayerPosition;
	PlayerPosition = Position;
	PlayerRotation = Rotation;

	float Distance = FVector::Dist(Position, OldPosition);
	PlayerSpeed = Distance / 10.0f; // Rough speed estimate
}

// Minimap
FVector2D UMGNavigationSubsystem::WorldToMapCoordinates(FVector WorldLocation) const
{
	FVector2D WorldRange = MapWorldMax - MapWorldMin;
	FVector2D NormalizedPos = FVector2D(
		(WorldLocation.X - MapWorldMin.X) / WorldRange.X,
		(WorldLocation.Y - MapWorldMin.Y) / WorldRange.Y
	);
	return NormalizedPos;
}

FVector UMGNavigationSubsystem::MapToWorldCoordinates(FVector2D MapLocation) const
{
	FVector2D WorldRange = MapWorldMax - MapWorldMin;
	return FVector(
		MapWorldMin.X + MapLocation.X * WorldRange.X,
		MapWorldMin.Y + MapLocation.Y * WorldRange.Y,
		0.0f
	);
}

void UMGNavigationSubsystem::SetMinimapZoom(float ZoomLevel)
{
	MinimapZoom = FMath::Clamp(ZoomLevel, 0.5f, 5.0f);
}

void UMGNavigationSubsystem::SetMinimapRotation(bool bRotateWithPlayer)
{
	bMinimapRotatesWithPlayer = bRotateWithPlayer;
}

// GPS Voice
void UMGNavigationSubsystem::SetGPSVoiceEnabled(bool bEnabled)
{
	bGPSVoiceEnabled = bEnabled;
}

// Internal
void UMGNavigationSubsystem::UpdateNavigation()
{
	CheckPOIProximity();
	CheckRegionChange();

	if (!bHasDestination)
	{
		return;
	}

	// Check if reached destination
	float DistToDest = FVector::Dist(PlayerPosition, CurrentDestinationPOI.WorldLocation);
	if (DistToDest < CurrentDestinationPOI.InteractionRadius)
	{
		OnDestinationReached.Broadcast(CurrentDestinationPOI);
		ClearDestination();
		return;
	}

	// Check if passed current waypoint
	if (CurrentRoute.Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		float DistToWP = FVector::Dist(PlayerPosition, CurrentRoute.Waypoints[CurrentWaypointIndex].Location);
		if (DistToWP < 1000.0f) // 10m threshold
		{
			CurrentWaypointIndex++;

			if (CurrentRoute.Waypoints.IsValidIndex(CurrentWaypointIndex))
			{
				FMGNavWaypoint& NextWP = CurrentRoute.Waypoints[CurrentWaypointIndex];
				OnNavInstructionChanged.Broadcast(NextWP.Instruction, NextWP.DistanceToNext);
			}
		}
	}
}

void UMGNavigationSubsystem::CheckPOIProximity()
{
	for (auto& Pair : RegisteredPOIs)
	{
		FMGPointOfInterest& POI = Pair.Value;
		if (POI.bDiscovered)
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerPosition, POI.WorldLocation);
		if (Distance <= POI.VisibilityDistance)
		{
			DiscoverPOI(POI.POIID);
		}
	}
}

void UMGNavigationSubsystem::CheckRegionChange()
{
	FVector2D PlayerPos2D(PlayerPosition.X, PlayerPosition.Y);

	for (const auto& Pair : RegisteredRegions)
	{
		const FMGMapRegion& Region = Pair.Value;

		if (PlayerPos2D.X >= Region.MapBoundsMin.X && PlayerPos2D.X <= Region.MapBoundsMax.X &&
			PlayerPos2D.Y >= Region.MapBoundsMin.Y && PlayerPos2D.Y <= Region.MapBoundsMax.Y)
		{
			if (CurrentRegionID != Region.RegionID)
			{
				CurrentRegionID = Region.RegionID;
				OnRegionEntered.Broadcast(Region);
			}
			break;
		}
	}
}

FMGNavRoute UMGNavigationSubsystem::CalculateRoute(FVector Start, FVector End)
{
	FMGNavRoute Route;
	Route.RouteID = FGuid::NewGuid();
	Route.StartLocation = Start;
	Route.Destination = End;
	Route.bAvoidHighways = bAvoidHighways;
	Route.bAvoidPolice = bAvoidPolice;
	Route.bShortestRoute = bShortestRoute;

	// Simple straight-line route for now
	// Real implementation would use navigation mesh or road network
	FMGNavWaypoint StartWP;
	StartWP.Location = Start;
	StartWP.Instruction = EMGNavInstruction::Straight;
	StartWP.DistanceFromStart = 0.0f;
	Route.Waypoints.Add(StartWP);

	FMGNavWaypoint EndWP;
	EndWP.Location = End;
	EndWP.Instruction = EMGNavInstruction::Arrived;
	EndWP.DistanceFromStart = FVector::Dist(Start, End) / 100.0f;
	Route.Waypoints.Add(EndWP);

	Route.TotalDistance = EndWP.DistanceFromStart;
	Route.EstimatedTime = Route.TotalDistance / 20.0f; // Assume 20 m/s average

	return Route;
}
