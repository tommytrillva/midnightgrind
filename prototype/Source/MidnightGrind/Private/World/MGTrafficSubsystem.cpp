// Copyright Midnight Grind. All Rights Reserved.

#include "World/MGTrafficSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UMGTrafficSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default district configs
	FMGTrafficConfig DowntownConfig;
	DowntownConfig.BaseDensity = EMGTrafficDensity::Heavy;
	DowntownConfig.AggressionLevel = 0.4f;
	DistrictConfigs.Add(FName(TEXT("Downtown")), DowntownConfig);

	FMGTrafficConfig IndustrialConfig;
	IndustrialConfig.BaseDensity = EMGTrafficDensity::Light;
	IndustrialConfig.TypeDistribution.Add(EMGTrafficVehicleType::Semi, 0.15f);
	IndustrialConfig.TypeDistribution.Add(EMGTrafficVehicleType::Truck, 0.2f);
	DistrictConfigs.Add(FName(TEXT("Industrial")), IndustrialConfig);

	FMGTrafficConfig HighwayConfig;
	HighwayConfig.BaseDensity = EMGTrafficDensity::Moderate;
	HighwayConfig.SpeedVariance = 0.1f;
	DistrictConfigs.Add(FName(TEXT("Highway")), HighwayConfig);

	FMGTrafficConfig HillsConfig;
	HillsConfig.BaseDensity = EMGTrafficDensity::VeryLight;
	HillsConfig.TypeDistribution.Add(EMGTrafficVehicleType::Sports, 0.15f);
	DistrictConfigs.Add(FName(TEXT("Hills")), HillsConfig);

	FMGTrafficConfig SuburbsConfig;
	SuburbsConfig.BaseDensity = EMGTrafficDensity::Moderate;
	SuburbsConfig.AggressionLevel = 0.2f;
	DistrictConfigs.Add(FName(TEXT("Suburbs")), SuburbsConfig);

	FMGTrafficConfig PortConfig;
	PortConfig.BaseDensity = EMGTrafficDensity::Light;
	PortConfig.TypeDistribution.Add(EMGTrafficVehicleType::Semi, 0.1f);
	DistrictConfigs.Add(FName(TEXT("Port")), PortConfig);

	// Set up tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGTrafficSubsystem::OnTick,
			0.1f, // 10 Hz
			true
		);
	}
}

void UMGTrafficSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	// Cleanup all traffic
	for (AMGTrafficVehicle* Vehicle : ActiveTrafficVehicles)
	{
		if (Vehicle)
		{
			Vehicle->Destroy();
		}
	}
	ActiveTrafficVehicles.Empty();

	Super::Deinitialize();
}

void UMGTrafficSubsystem::SetTrafficDensity(EMGTrafficDensity Density)
{
	if (CurrentDensity != Density)
	{
		CurrentDensity = Density;
		OnTrafficDensityChanged.Broadcast(Density);

		// Adjust max vehicles based on density
		MaxTrafficVehicles = GetTargetCountForDensity(Density);
	}
}

void UMGTrafficSubsystem::SetTrafficEnabled(bool bEnabled)
{
	bTrafficEnabled = bEnabled;

	if (!bEnabled)
	{
		// Clear all traffic
		for (AMGTrafficVehicle* Vehicle : ActiveTrafficVehicles)
		{
			if (Vehicle)
			{
				Vehicle->Destroy();
			}
		}
		ActiveTrafficVehicles.Empty();
	}
}

void UMGTrafficSubsystem::SetDistrictConfig(FName DistrictID, const FMGTrafficConfig& Config)
{
	DistrictConfigs.Add(DistrictID, Config);
}

FMGTrafficConfig UMGTrafficSubsystem::GetDistrictConfig(FName DistrictID) const
{
	if (const FMGTrafficConfig* Config = DistrictConfigs.Find(DistrictID))
	{
		return *Config;
	}
	return DefaultConfig;
}

void UMGTrafficSubsystem::ClearDistrictTraffic(FName DistrictID)
{
	// Remove traffic in specific district
	for (int32 i = ActiveTrafficVehicles.Num() - 1; i >= 0; --i)
	{
		AMGTrafficVehicle* Vehicle = ActiveTrafficVehicles[i];
		if (Vehicle)
		{
			// Would check vehicle's district here
			// For now, just keep the structure
		}
	}
}

void UMGTrafficSubsystem::UpdateForTimeOfDay(float GameHour)
{
	// Per PRD Section 5.2 Time System
	EMGTrafficDensity NewDensity;

	if (GameHour >= 19.0f && GameHour < 21.0f)
	{
		// Dusk (7PM-9PM): Moderate
		NewDensity = EMGTrafficDensity::Moderate;
	}
	else if (GameHour >= 21.0f || GameHour < 0.0f)
	{
		// Night (9PM-12AM): Light-Moderate
		NewDensity = EMGTrafficDensity::Moderate;
	}
	else if (GameHour >= 0.0f && GameHour < 3.0f)
	{
		// Late Night (12AM-3AM): Light
		NewDensity = EMGTrafficDensity::Light;
	}
	else if (GameHour >= 3.0f && GameHour < 4.0f)
	{
		// Very Late Night (3AM-4AM): Very Light
		NewDensity = EMGTrafficDensity::VeryLight;
	}
	else if (GameHour >= 4.0f && GameHour < 6.0f)
	{
		// Dawn (4AM-6AM): Very Light
		NewDensity = EMGTrafficDensity::VeryLight;
	}
	else
	{
		// Default
		NewDensity = EMGTrafficDensity::Light;
	}

	SetTrafficDensity(NewDensity);
}

void UMGTrafficSubsystem::SetRaceActive(bool bActive, TArray<AActor*> Participants)
{
	bRaceActive = bActive;

	RaceParticipants.Empty();
	for (AActor* Participant : Participants)
	{
		RaceParticipants.Add(Participant);
	}
}

void UMGTrafficSubsystem::ClearRacePath(const TArray<FVector>& PathPoints, float ClearRadius)
{
	for (int32 i = ActiveTrafficVehicles.Num() - 1; i >= 0; --i)
	{
		AMGTrafficVehicle* Vehicle = ActiveTrafficVehicles[i];
		if (!Vehicle)
		{
			continue;
		}

		FVector VehicleLocation = Vehicle->GetActorLocation();

		for (const FVector& PathPoint : PathPoints)
		{
			if (FVector::Dist(VehicleLocation, PathPoint) < ClearRadius)
			{
				DespawnTrafficVehicle(Vehicle);
				break;
			}
		}
	}
}

void UMGTrafficSubsystem::RegisterLane(const FMGTrafficLane& Lane)
{
	// Check for duplicate
	for (const FMGTrafficLane& Existing : RegisteredLanes)
	{
		if (Existing.LaneID == Lane.LaneID)
		{
			return;
		}
	}

	RegisteredLanes.Add(Lane);
}

void UMGTrafficSubsystem::RegisterSpawnPoint(const FMGTrafficSpawnPoint& SpawnPoint)
{
	RegisteredSpawnPoints.Add(SpawnPoint);
}

AMGTrafficVehicle* UMGTrafficSubsystem::SpawnTrafficVehicle(const FMGTrafficSpawnPoint& SpawnPoint, EMGTrafficVehicleType Type)
{
	if (!bTrafficEnabled || ActiveTrafficVehicles.Num() >= MaxTrafficVehicles)
	{
		return nullptr;
	}

	// In real implementation, would spawn actual traffic vehicle actor
	// For now, return nullptr as placeholder
	return nullptr;
}

void UMGTrafficSubsystem::DespawnTrafficVehicle(AMGTrafficVehicle* Vehicle)
{
	if (Vehicle)
	{
		ActiveTrafficVehicles.Remove(Vehicle);
		Vehicle->Destroy();
	}
}

AMGTrafficVehicle* UMGTrafficSubsystem::GetNearestTrafficVehicle(FVector Location, float MaxDistance) const
{
	AMGTrafficVehicle* Nearest = nullptr;
	float NearestDistance = MaxDistance;

	for (AMGTrafficVehicle* Vehicle : ActiveTrafficVehicles)
	{
		if (Vehicle)
		{
			float Distance = FVector::Dist(Location, Vehicle->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				Nearest = Vehicle;
			}
		}
	}

	return Nearest;
}

TArray<AMGTrafficVehicle*> UMGTrafficSubsystem::GetTrafficInRadius(FVector Location, float Radius) const
{
	TArray<AMGTrafficVehicle*> Result;

	for (AMGTrafficVehicle* Vehicle : ActiveTrafficVehicles)
	{
		if (Vehicle && FVector::Dist(Location, Vehicle->GetActorLocation()) <= Radius)
		{
			Result.Add(Vehicle);
		}
	}

	return Result;
}

bool UMGTrafficSubsystem::IsLocationInLane(FVector Location, FName& OutLaneID) const
{
	for (const FMGTrafficLane& Lane : RegisteredLanes)
	{
		if (Lane.LaneSpline.IsValid())
		{
			USplineComponent* Spline = Lane.LaneSpline.Get();
			float ClosestKey = Spline->FindInputKeyClosestToWorldLocation(Location);
			FVector ClosestPoint = Spline->GetLocationAtSplineInputKey(ClosestKey, ESplineCoordinateSpace::World);

			if (FVector::Dist(Location, ClosestPoint) < 500.0f) // Lane width threshold
			{
				OutLaneID = Lane.LaneID;
				return true;
			}
		}
	}

	OutLaneID = NAME_None;
	return false;
}

void UMGTrafficSubsystem::UpdateSpawning(float DeltaTime)
{
	if (!bTrafficEnabled)
	{
		return;
	}

	SpawnTimer += DeltaTime;

	if (SpawnTimer >= SpawnInterval)
	{
		SpawnTimer = 0.0f;

		// Check if we need more traffic
		int32 TargetCount = GetTargetCountForDensity(CurrentDensity);
		if (ActiveTrafficVehicles.Num() < TargetCount)
		{
			// Find valid spawn point
			FMGTrafficSpawnPoint SpawnPoint;
			if (FindValidSpawnPoint(NAME_None, SpawnPoint))
			{
				FMGTrafficConfig Config = GetDistrictConfig(SpawnPoint.DistrictID);
				EMGTrafficVehicleType Type = SelectRandomVehicleType(Config);
				SpawnTrafficVehicle(SpawnPoint, Type);
			}
		}
	}
}

void UMGTrafficSubsystem::UpdateCulling()
{
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		FVector PlayerLocation = PlayerPawn->GetActorLocation();

		for (int32 i = ActiveTrafficVehicles.Num() - 1; i >= 0; --i)
		{
			AMGTrafficVehicle* Vehicle = ActiveTrafficVehicles[i];
			if (Vehicle)
			{
				float Distance = FVector::Dist(PlayerLocation, Vehicle->GetActorLocation());
				if (Distance > CullDistance)
				{
					DespawnTrafficVehicle(Vehicle);
				}
			}
		}
	}
}

int32 UMGTrafficSubsystem::GetTargetCountForDensity(EMGTrafficDensity Density) const
{
	switch (Density)
	{
	case EMGTrafficDensity::None:
		return 0;
	case EMGTrafficDensity::VeryLight:
		return 10;
	case EMGTrafficDensity::Light:
		return 20;
	case EMGTrafficDensity::Moderate:
		return 35;
	case EMGTrafficDensity::Heavy:
		return 50;
	default:
		return 25;
	}
}

EMGTrafficVehicleType UMGTrafficSubsystem::SelectRandomVehicleType(const FMGTrafficConfig& Config) const
{
	float Random = FMath::FRand();
	float Cumulative = 0.0f;

	for (const auto& Pair : Config.TypeDistribution)
	{
		Cumulative += Pair.Value;
		if (Random <= Cumulative)
		{
			return Pair.Key;
		}
	}

	return EMGTrafficVehicleType::Sedan;
}

bool UMGTrafficSubsystem::FindValidSpawnPoint(FName DistrictID, FMGTrafficSpawnPoint& OutSpawnPoint) const
{
	TArray<FMGTrafficSpawnPoint> ValidPoints;

	for (const FMGTrafficSpawnPoint& Point : RegisteredSpawnPoints)
	{
		if (Point.bIsEntryPoint)
		{
			if (DistrictID.IsNone() || Point.DistrictID == DistrictID)
			{
				ValidPoints.Add(Point);
			}
		}
	}

	if (ValidPoints.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, ValidPoints.Num() - 1);
		OutSpawnPoint = ValidPoints[Index];
		return true;
	}

	return false;
}

void UMGTrafficSubsystem::CheckNearMisses()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector PlayerVelocity = PlayerPawn->GetVelocity();

	// Only check if player is moving
	if (PlayerVelocity.SizeSquared() < 10000.0f) // ~36 km/h minimum
	{
		return;
	}

	for (AMGTrafficVehicle* Vehicle : ActiveTrafficVehicles)
	{
		if (!Vehicle)
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerLocation, Vehicle->GetActorLocation());

		if (Distance < NearMissThreshold && Distance > 50.0f) // Not a collision
		{
			// Calculate relative velocity to ensure we're actually passing close
			FVector ToVehicle = (Vehicle->GetActorLocation() - PlayerLocation).GetSafeNormal();
			float DotProduct = FVector::DotProduct(PlayerVelocity.GetSafeNormal(), ToVehicle);

			// If we're moving toward/past the vehicle, it's a near miss
			if (FMath::Abs(DotProduct) > 0.3f)
			{
				OnNearMiss.Broadcast(Vehicle, Distance);
			}
		}
	}
}

void UMGTrafficSubsystem::OnTick()
{
	const float DeltaTime = 0.1f;

	if (!bTrafficEnabled)
	{
		return;
	}

	UpdateSpawning(DeltaTime);
	UpdateCulling();
	CheckNearMisses();
}
