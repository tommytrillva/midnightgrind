// Copyright Midnight Grind. All Rights Reserved.

#include "Traffic/MGTrafficSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGTrafficSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Settings.DensityPreset = EMGTrafficDensityPreset::Medium;
	Settings.MaxActiveVehicles = 50;
}

void UMGTrafficSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
	DespawnAllTraffic();
	Super::Deinitialize();
}

void UMGTrafficSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	InWorld.GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]()
		{
			if (bTrafficEnabled)
			{
				UpdateTraffic(0.1f);
			}
		},
		0.1f,
		true
	);
}

// Settings
void UMGTrafficSubsystem::SetTrafficSettings(const FMGTrafficSettings& NewSettings)
{
	Settings = NewSettings;
}

void UMGTrafficSubsystem::SetTrafficDensity(EMGTrafficDensityPreset Density)
{
	if (Settings.DensityPreset != Density)
	{
		Settings.DensityPreset = Density;
		OnTrafficDensityChanged.Broadcast(Density);
	}
}

void UMGTrafficSubsystem::SetTrafficEnabled(bool bEnabled)
{
	bTrafficEnabled = bEnabled;
	if (!bEnabled)
	{
		DespawnAllTraffic();
	}
}

// Spawning
int32 UMGTrafficSubsystem::SpawnTrafficVehicle(FVector Location, FRotator Rotation, EMGTrafficVehicleType Type)
{
	if (GetActiveVehicleCount() >= Settings.MaxActiveVehicles)
	{
		return -1;
	}

	FMGTrafficVehicleState NewVehicle;
	NewVehicle.VehicleID = NextVehicleID++;
	NewVehicle.VehicleType = Type;
	NewVehicle.Location = Location;
	NewVehicle.Rotation = Rotation;
	NewVehicle.CurrentSpeed = 0.0f;
	NewVehicle.TargetSpeed = 35.0f;
	NewVehicle.Behavior = EMGTrafficBehavior::Normal;

	// Random behavior variation
	float BehaviorRoll = FMath::FRand();
	if (BehaviorRoll < Settings.AggressiveDriverChance)
	{
		NewVehicle.Behavior = EMGTrafficBehavior::Aggressive;
		NewVehicle.TargetSpeed *= 1.3f;
	}
	else if (BehaviorRoll < Settings.AggressiveDriverChance + Settings.DistractedDriverChance)
	{
		NewVehicle.Behavior = EMGTrafficBehavior::Distracted;
		NewVehicle.TargetSpeed *= 0.8f;
	}

	// TODO: Actually spawn the vehicle actor
	// NewVehicle.VehicleActor = SpawnedActor;

	ActiveVehicles.Add(NewVehicle.VehicleID, NewVehicle);
	TotalVehiclesSpawned++;

	OnTrafficVehicleSpawned.Broadcast(NewVehicle.VehicleID);

	return NewVehicle.VehicleID;
}

void UMGTrafficSubsystem::DespawnTrafficVehicle(int32 VehicleID)
{
	FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	if (Vehicle)
	{
		if (Vehicle->VehicleActor.IsValid())
		{
			Vehicle->VehicleActor->Destroy();
		}
		ActiveVehicles.Remove(VehicleID);
		OnTrafficVehicleDespawned.Broadcast(VehicleID);
	}
}

void UMGTrafficSubsystem::DespawnAllTraffic()
{
	TArray<int32> VehicleIDs;
	ActiveVehicles.GetKeys(VehicleIDs);

	for (int32 ID : VehicleIDs)
	{
		DespawnTrafficVehicle(ID);
	}
}

void UMGTrafficSubsystem::RegisterSpawnPoint(const FMGTrafficSpawnPoint& SpawnPoint)
{
	SpawnPoints.Add(SpawnPoint);
}

void UMGTrafficSubsystem::ClearSpawnPoints()
{
	SpawnPoints.Empty();
}

// Vehicle Queries
int32 UMGTrafficSubsystem::GetActiveVehicleCount() const
{
	return ActiveVehicles.Num();
}

TArray<FMGTrafficVehicleState> UMGTrafficSubsystem::GetAllTrafficVehicles() const
{
	TArray<FMGTrafficVehicleState> Result;
	ActiveVehicles.GenerateValueArray(Result);
	return Result;
}

FMGTrafficVehicleState UMGTrafficSubsystem::GetTrafficVehicle(int32 VehicleID) const
{
	const FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	return Vehicle ? *Vehicle : FMGTrafficVehicleState();
}

TArray<FMGTrafficVehicleState> UMGTrafficSubsystem::GetVehiclesInRadius(FVector Center, float Radius) const
{
	TArray<FMGTrafficVehicleState> Result;
	float RadiusSq = Radius * Radius;

	for (const auto& Pair : ActiveVehicles)
	{
		if (FVector::DistSquared(Pair.Value.Location, Center) <= RadiusSq)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

FMGTrafficVehicleState UMGTrafficSubsystem::GetNearestVehicle(FVector Location) const
{
	FMGTrafficVehicleState Nearest;
	float NearestDistSq = FLT_MAX;

	for (const auto& Pair : ActiveVehicles)
	{
		float DistSq = FVector::DistSquared(Pair.Value.Location, Location);
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			Nearest = Pair.Value;
		}
	}

	return Nearest;
}

// Vehicle Control
void UMGTrafficSubsystem::SetVehicleBehavior(int32 VehicleID, EMGTrafficBehavior Behavior)
{
	FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	if (Vehicle)
	{
		Vehicle->Behavior = Behavior;
	}
}

void UMGTrafficSubsystem::SetVehicleTargetSpeed(int32 VehicleID, float Speed)
{
	FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	if (Vehicle)
	{
		Vehicle->TargetSpeed = Speed;
	}
}

void UMGTrafficSubsystem::ForceVehicleLaneChange(int32 VehicleID, bool bMoveRight)
{
	FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	if (Vehicle)
	{
		Vehicle->CurrentLaneIndex += bMoveRight ? 1 : -1;
		Vehicle->CurrentLaneIndex = FMath::Max(0, Vehicle->CurrentLaneIndex);
	}
}

void UMGTrafficSubsystem::PanicVehiclesInRadius(FVector Center, float Radius)
{
	float RadiusSq = Radius * Radius;

	for (auto& Pair : ActiveVehicles)
	{
		if (FVector::DistSquared(Pair.Value.Location, Center) <= RadiusSq)
		{
			Pair.Value.Behavior = EMGTrafficBehavior::Panicked;
			Pair.Value.TargetSpeed *= 1.5f;
			OnTrafficPanicked.Broadcast(Pair.Key, Center);
		}
	}
}

void UMGTrafficSubsystem::StopVehicle(int32 VehicleID)
{
	FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	if (Vehicle)
	{
		Vehicle->TargetSpeed = 0.0f;
		Vehicle->CurrentSpeed = 0.0f;
	}
}

// Roads
void UMGTrafficSubsystem::RegisterRoad(const FMGRoadSegment& Road)
{
	Roads.Add(Road.RoadID, Road);
}

void UMGTrafficSubsystem::RegisterIntersection(const FMGIntersection& Intersection)
{
	Intersections.Add(Intersection.IntersectionID, Intersection);
}

FMGRoadSegment UMGTrafficSubsystem::GetRoad(int32 RoadID) const
{
	const FMGRoadSegment* Road = Roads.Find(RoadID);
	return Road ? *Road : FMGRoadSegment();
}

FMGIntersection UMGTrafficSubsystem::GetIntersection(int32 IntersectionID) const
{
	const FMGIntersection* Intersection = Intersections.Find(IntersectionID);
	return Intersection ? *Intersection : FMGIntersection();
}

TArray<FMGRoadSegment> UMGTrafficSubsystem::GetAllRoads() const
{
	TArray<FMGRoadSegment> Result;
	Roads.GenerateValueArray(Result);
	return Result;
}

// Traffic Lights
void UMGTrafficSubsystem::SetTrafficLightState(int32 IntersectionID, int32 GreenRoadIndex)
{
	FMGIntersection* Intersection = Intersections.Find(IntersectionID);
	if (Intersection)
	{
		int32 OldGreen = Intersection->CurrentGreenRoadIndex;
		Intersection->CurrentGreenRoadIndex = GreenRoadIndex;
		Intersection->LightTimer = 0.0f;
		Intersection->bIsYellow = false;

		if (OldGreen != GreenRoadIndex)
		{
			OnTrafficLightChanged.Broadcast(IntersectionID, true);
		}
	}
}

void UMGTrafficSubsystem::ForceAllLightsGreen()
{
	bForcedGreenLights = true;
}

void UMGTrafficSubsystem::ResumeNormalLightCycle()
{
	bForcedGreenLights = false;
}

bool UMGTrafficSubsystem::IsLightGreenForRoad(int32 IntersectionID, int32 RoadID) const
{
	if (bForcedGreenLights)
	{
		return true;
	}

	const FMGIntersection* Intersection = Intersections.Find(IntersectionID);
	if (!Intersection)
	{
		return true;
	}

	if (Intersection->ConnectedRoadIDs.IsValidIndex(Intersection->CurrentGreenRoadIndex))
	{
		return Intersection->ConnectedRoadIDs[Intersection->CurrentGreenRoadIndex] == RoadID;
	}

	return false;
}

// Player
void UMGTrafficSubsystem::UpdatePlayerPosition(FVector Position, float Speed)
{
	PlayerPosition = Position;
	PlayerSpeed = Speed;

	// Update distance from player for all vehicles
	for (auto& Pair : ActiveVehicles)
	{
		Pair.Value.DistanceFromPlayer = FVector::Dist(Pair.Value.Location, PlayerPosition);
	}
}

void UMGTrafficSubsystem::NotifyPlayerCollision(int32 VehicleID)
{
	FMGTrafficVehicleState* Vehicle = ActiveVehicles.Find(VehicleID);
	if (Vehicle)
	{
		Vehicle->bHasCollided = true;
		TotalCollisions++;

		// Vehicle panics after collision
		Vehicle->Behavior = EMGTrafficBehavior::Panicked;

		OnTrafficCollision.Broadcast(VehicleID, nullptr);
	}
}

// Internal
void UMGTrafficSubsystem::UpdateTraffic(float DeltaTime)
{
	// Update traffic lights
	UpdateTrafficLights(DeltaTime);

	// Update vehicle AI
	for (auto& Pair : ActiveVehicles)
	{
		UpdateVehicleAI(Pair.Value, DeltaTime);
		Pair.Value.TimeAlive += DeltaTime;
	}

	// Spawn new traffic
	SpawnTrafficIfNeeded();

	// Despawn distant vehicles
	DespawnDistantVehicles();
}

void UMGTrafficSubsystem::UpdateVehicleAI(FMGTrafficVehicleState& Vehicle, float DeltaTime)
{
	// Accelerate/decelerate towards target speed
	float SpeedDiff = Vehicle.TargetSpeed - Vehicle.CurrentSpeed;
	float Acceleration = 10.0f; // mph/s
	float Deceleration = 20.0f;

	if (SpeedDiff > 0)
	{
		Vehicle.CurrentSpeed += FMath::Min(Acceleration * DeltaTime, SpeedDiff);
	}
	else if (SpeedDiff < 0)
	{
		Vehicle.CurrentSpeed += FMath::Max(-Deceleration * DeltaTime, SpeedDiff);
	}

	// Behavior-specific updates
	switch (Vehicle.Behavior)
	{
		case EMGTrafficBehavior::Panicked:
			// Try to get away, erratic movement
			Vehicle.TargetSpeed = FMath::RandRange(40.0f, 60.0f);
			break;

		case EMGTrafficBehavior::Pulling:
			// Slow down and move to side
			Vehicle.TargetSpeed = FMath::Max(0.0f, Vehicle.TargetSpeed - 5.0f * DeltaTime);
			break;

		case EMGTrafficBehavior::Aggressive:
			// Tailgate, lane change frequently
			break;

		case EMGTrafficBehavior::Cautious:
			// Keep extra distance
			break;

		default:
			break;
	}

	// React to player if enabled
	if (Settings.bReactToPlayer && Vehicle.DistanceFromPlayer < 5000.0f)
	{
		// Check if player is approaching fast
		if (PlayerSpeed > Vehicle.CurrentSpeed + 30.0f && Vehicle.DistanceFromPlayer < 3000.0f)
		{
			// Move over if possible
			if (Vehicle.Behavior == EMGTrafficBehavior::Normal)
			{
				Vehicle.Behavior = EMGTrafficBehavior::Cautious;
			}
		}
	}

	// Move vehicle forward
	float SpeedCmPerSec = Vehicle.CurrentSpeed * 44.704f; // mph to cm/s
	FVector Forward = Vehicle.Rotation.Vector();
	Vehicle.Location += Forward * SpeedCmPerSec * DeltaTime;
}

void UMGTrafficSubsystem::UpdateTrafficLights(float DeltaTime)
{
	if (bForcedGreenLights)
	{
		return;
	}

	for (auto& Pair : Intersections)
	{
		FMGIntersection& Intersection = Pair.Value;

		if (!Intersection.bHasTrafficLight)
		{
			continue;
		}

		Intersection.LightTimer += DeltaTime;

		// Yellow light phase
		float YellowDuration = 3.0f;
		if (Intersection.LightTimer >= Intersection.LightCycleDuration - YellowDuration && !Intersection.bIsYellow)
		{
			Intersection.bIsYellow = true;
		}

		// Switch to next road
		if (Intersection.LightTimer >= Intersection.LightCycleDuration)
		{
			Intersection.LightTimer = 0.0f;
			Intersection.bIsYellow = false;

			int32 NumRoads = Intersection.ConnectedRoadIDs.Num();
			if (NumRoads > 0)
			{
				Intersection.CurrentGreenRoadIndex = (Intersection.CurrentGreenRoadIndex + 1) % NumRoads;
				OnTrafficLightChanged.Broadcast(Pair.Key, true);
			}
		}
	}
}

void UMGTrafficSubsystem::SpawnTrafficIfNeeded()
{
	if (Settings.DensityPreset == EMGTrafficDensityPreset::None)
	{
		return;
	}

	TimeSinceLastSpawn += 0.1f;

	if (TimeSinceLastSpawn < NextSpawnInterval)
	{
		return;
	}

	int32 TargetVehicles = static_cast<int32>(Settings.MaxActiveVehicles * GetDensityMultiplier());
	if (GetActiveVehicleCount() >= TargetVehicles)
	{
		return;
	}

	// Find a valid spawn point
	if (SpawnPoints.Num() > 0)
	{
		for (int32 Attempt = 0; Attempt < 5; ++Attempt)
		{
			int32 SpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
			const FMGTrafficSpawnPoint& SpawnPoint = SpawnPoints[SpawnIndex];

			// Check if far enough from player
			float DistToPlayer = FVector::Dist(SpawnPoint.Location, PlayerPosition);
			if (DistToPlayer > 5000.0f && DistToPlayer < Settings.SpawnDistance)
			{
				EMGTrafficVehicleType Type = SelectRandomVehicleType();
				SpawnTrafficVehicle(SpawnPoint.Location, SpawnPoint.Rotation, Type);
				break;
			}
		}
	}

	TimeSinceLastSpawn = 0.0f;
	NextSpawnInterval = FMath::RandRange(Settings.MinSpawnInterval, Settings.MaxSpawnInterval);
}

void UMGTrafficSubsystem::DespawnDistantVehicles()
{
	TArray<int32> ToRemove;

	for (const auto& Pair : ActiveVehicles)
	{
		if (Pair.Value.DistanceFromPlayer > Settings.DespawnDistance)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (int32 ID : ToRemove)
	{
		DespawnTrafficVehicle(ID);
	}
}

FVector UMGTrafficSubsystem::GetLanePosition(int32 RoadID, int32 LaneIndex, float Distance) const
{
	const FMGRoadSegment* Road = Roads.Find(RoadID);
	if (!Road || Road->SplinePoints.Num() < 2)
	{
		return FVector::ZeroVector;
	}

	// Simple linear interpolation along road
	float TotalLength = 0.0f;
	for (int32 i = 1; i < Road->SplinePoints.Num(); ++i)
	{
		TotalLength += FVector::Dist(Road->SplinePoints[i-1], Road->SplinePoints[i]);
	}

	float TargetDist = FMath::Fmod(Distance, TotalLength);
	float CurrentDist = 0.0f;

	for (int32 i = 1; i < Road->SplinePoints.Num(); ++i)
	{
		float SegmentLength = FVector::Dist(Road->SplinePoints[i-1], Road->SplinePoints[i]);
		if (CurrentDist + SegmentLength >= TargetDist)
		{
			float Alpha = (TargetDist - CurrentDist) / SegmentLength;
			FVector BasePos = FMath::Lerp(Road->SplinePoints[i-1], Road->SplinePoints[i], Alpha);

			// Offset for lane
			FVector Direction = (Road->SplinePoints[i] - Road->SplinePoints[i-1]).GetSafeNormal();
			FVector Right = FVector::CrossProduct(Direction, FVector::UpVector);
			float LaneOffset = (LaneIndex - Road->NumLanes / 2.0f + 0.5f) * Road->LaneWidth;

			return BasePos + Right * LaneOffset;
		}
		CurrentDist += SegmentLength;
	}

	return Road->SplinePoints.Last();
}

EMGTrafficVehicleType UMGTrafficSubsystem::SelectRandomVehicleType() const
{
	// Weighted selection
	TArray<TPair<EMGTrafficVehicleType, float>> Weights;
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::Sedan, 0.35f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::SUV, 0.25f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::Truck, Settings.bEnableTrucks ? 0.1f : 0.0f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::Van, 0.1f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::SportsCar, 0.05f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::Motorcycle, Settings.bEnableMotorcycles ? 0.05f : 0.0f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::Taxi, 0.05f));
	Weights.Add(TPair<EMGTrafficVehicleType, float>(EMGTrafficVehicleType::DeliveryVan, 0.05f));

	float TotalWeight = 0.0f;
	for (const auto& Pair : Weights)
	{
		TotalWeight += Pair.Value;
	}

	float Roll = FMath::FRand() * TotalWeight;
	float Cumulative = 0.0f;

	for (const auto& Pair : Weights)
	{
		Cumulative += Pair.Value;
		if (Roll <= Cumulative)
		{
			return Pair.Key;
		}
	}

	return EMGTrafficVehicleType::Sedan;
}

float UMGTrafficSubsystem::GetDensityMultiplier() const
{
	switch (Settings.DensityPreset)
	{
		case EMGTrafficDensityPreset::None: return 0.0f;
		case EMGTrafficDensityPreset::VeryLight: return 0.2f;
		case EMGTrafficDensityPreset::Light: return 0.4f;
		case EMGTrafficDensityPreset::Medium: return 0.6f;
		case EMGTrafficDensityPreset::Heavy: return 0.8f;
		case EMGTrafficDensityPreset::RushHour: return 1.0f;
		case EMGTrafficDensityPreset::Gridlock: return 1.2f;
		default: return 0.5f;
	}
}
