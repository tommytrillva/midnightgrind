// Copyright Midnight Grind. All Rights Reserved.

#include "AmbientLife/MGAmbientLifeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGAmbientLifeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TrafficSettings = FMGTrafficSettings();
	PedestrianSettings = FMGPedestrianSettings();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			UpdateTimerHandle,
			[this]() { UpdateAmbientLife(0.5f); },
			0.5f,
			true
		);
	}
}

void UMGAmbientLifeSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
	Super::Deinitialize();
}

void UMGAmbientLifeSubsystem::SetTrafficSettings(const FMGTrafficSettings& Settings)
{
	EMGTrafficDensity OldDensity = TrafficSettings.Density;
	TrafficSettings = Settings;

	if (OldDensity != TrafficSettings.Density)
	{
		OnTrafficDensityChanged.Broadcast(OldDensity, TrafficSettings.Density);
	}
}

void UMGAmbientLifeSubsystem::SetTrafficDensity(EMGTrafficDensity Density)
{
	if (TrafficSettings.Density != Density)
	{
		EMGTrafficDensity OldDensity = TrafficSettings.Density;
		TrafficSettings.Density = Density;

		// Adjust multiplier based on density
		switch (Density)
		{
		case EMGTrafficDensity::None: TrafficSettings.DensityMultiplier = 0.0f; break;
		case EMGTrafficDensity::Light: TrafficSettings.DensityMultiplier = 0.3f; break;
		case EMGTrafficDensity::Medium: TrafficSettings.DensityMultiplier = 0.6f; break;
		case EMGTrafficDensity::Heavy: TrafficSettings.DensityMultiplier = 1.0f; break;
		case EMGTrafficDensity::Rush: TrafficSettings.DensityMultiplier = 1.5f; break;
		}

		OnTrafficDensityChanged.Broadcast(OldDensity, Density);
	}
}

void UMGAmbientLifeSubsystem::ClearTrafficInRadius(FVector Location, float Radius)
{
	TrafficVehicles.RemoveAll([Location, Radius](const FMGAmbientVehicle& Vehicle)
	{
		return FVector::Dist(Vehicle.Transform.GetLocation(), Location) <= Radius;
	});
}

void UMGAmbientLifeSubsystem::PauseTraffic(bool bPause)
{
	bTrafficPaused = bPause;
}

TArray<FMGAmbientVehicle> UMGAmbientLifeSubsystem::GetNearbyTraffic(FVector Location, float Radius) const
{
	TArray<FMGAmbientVehicle> Result;
	for (const FMGAmbientVehicle& Vehicle : TrafficVehicles)
	{
		if (FVector::Dist(Vehicle.Transform.GetLocation(), Location) <= Radius)
		{
			Result.Add(Vehicle);
		}
	}
	return Result;
}

void UMGAmbientLifeSubsystem::SetPedestrianSettings(const FMGPedestrianSettings& Settings)
{
	PedestrianSettings = Settings;
}

void UMGAmbientLifeSubsystem::TriggerPedestrianReaction(FVector Location, float Radius, EMGPedestrianBehavior Behavior)
{
	for (FMGAmbientPedestrian& Pedestrian : Pedestrians)
	{
		if (FVector::Dist(Pedestrian.Location, Location) <= Radius)
		{
			Pedestrian.Behavior = Behavior;

			if (Behavior == EMGPedestrianBehavior::Recording)
			{
				Pedestrian.bIsRecording = true;
			}

			OnPedestrianReaction.Broadcast(Pedestrian, Behavior);
		}
	}
}

void UMGAmbientLifeSubsystem::ClearPedestriansInRadius(FVector Location, float Radius)
{
	Pedestrians.RemoveAll([Location, Radius](const FMGAmbientPedestrian& Pedestrian)
	{
		return FVector::Dist(Pedestrian.Location, Location) <= Radius;
	});
}

TArray<FMGAmbientPedestrian> UMGAmbientLifeSubsystem::GetNearbyPedestrians(FVector Location, float Radius) const
{
	TArray<FMGAmbientPedestrian> Result;
	for (const FMGAmbientPedestrian& Pedestrian : Pedestrians)
	{
		if (FVector::Dist(Pedestrian.Location, Location) <= Radius)
		{
			Result.Add(Pedestrian);
		}
	}
	return Result;
}

void UMGAmbientLifeSubsystem::CreateCrowdZone(const FMGCrowdZone& Zone)
{
	// Check if zone already exists
	for (FMGCrowdZone& Existing : CrowdZones)
	{
		if (Existing.ZoneID == Zone.ZoneID)
		{
			Existing = Zone;
			return;
		}
	}

	CrowdZones.Add(Zone);
	OnCrowdGathered.Broadcast(Zone);
}

void UMGAmbientLifeSubsystem::RemoveCrowdZone(FName ZoneID)
{
	CrowdZones.RemoveAll([ZoneID](const FMGCrowdZone& Zone)
	{
		return Zone.ZoneID == ZoneID;
	});
}

void UMGAmbientLifeSubsystem::SetupRaceSpectators(const TArray<FVector>& SpectatorLocations)
{
	// Clear existing race spectator zones
	CrowdZones.RemoveAll([](const FMGCrowdZone& Zone)
	{
		return Zone.bIsRaceSpectators;
	});

	// Create new spectator zones
	for (int32 i = 0; i < SpectatorLocations.Num(); i++)
	{
		FMGCrowdZone Zone;
		Zone.ZoneID = FName(*FString::Printf(TEXT("RaceSpectator_%d"), i));
		Zone.Location = SpectatorLocations[i];
		Zone.Radius = 30.0f;
		Zone.TargetCrowdSize = FMath::RandRange(10, 30);
		Zone.bIsRaceSpectators = true;
		CrowdZones.Add(Zone);
	}
}

void UMGAmbientLifeSubsystem::SpawnParkedCars(FVector Location, int32 Count, float Radius)
{
	for (int32 i = 0; i < Count; i++)
	{
		FMGAmbientVehicle Vehicle;
		Vehicle.VehicleID = FGuid::NewGuid().ToString();
		Vehicle.bIsParked = true;
		Vehicle.bIsTraffic = false;
		Vehicle.CurrentSpeed = 0.0f;

		// Random position within radius
		FVector Offset = FVector(
			FMath::RandRange(-Radius, Radius),
			FMath::RandRange(-Radius, Radius),
			0.0f
		);
		Vehicle.Transform.SetLocation(Location + Offset);

		// Random rotation
		Vehicle.Transform.SetRotation(FQuat(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)));

		// Random color
		Vehicle.PaintColor = FLinearColor(
			FMath::FRand(),
			FMath::FRand(),
			FMath::FRand()
		);

		ParkedVehicles.Add(Vehicle);
	}
}

void UMGAmbientLifeSubsystem::SetupCarMeetVehicles(FVector Location, const TArray<FName>& VehicleTypes)
{
	for (int32 i = 0; i < VehicleTypes.Num(); i++)
	{
		FMGAmbientVehicle Vehicle;
		Vehicle.VehicleID = FGuid::NewGuid().ToString();
		Vehicle.VehicleType = VehicleTypes[i];
		Vehicle.bIsParked = true;
		Vehicle.bIsTraffic = false;

		// Arrange in a circle
		float Angle = (2.0f * PI * i) / VehicleTypes.Num();
		float Radius = 50.0f + (VehicleTypes.Num() * 5.0f);
		FVector Offset = FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);
		Vehicle.Transform.SetLocation(Location + Offset);

		// Face center
		FRotator FaceCenter = (Location - (Location + Offset)).Rotation();
		Vehicle.Transform.SetRotation(FQuat(FaceCenter));

		ParkedVehicles.Add(Vehicle);
	}
}

FMGWorldPopulation UMGAmbientLifeSubsystem::GetWorldPopulation() const
{
	FMGWorldPopulation Population;
	Population.ActiveTrafficVehicles = TrafficVehicles.Num();
	Population.ActivePedestrians = Pedestrians.Num();
	Population.ParkedVehicles = ParkedVehicles.Num();
	Population.SpectatorCrowds = CrowdZones.Num();

	if (Pedestrians.Num() > 0)
	{
		Population.AveragePedestrianDensity = static_cast<float>(Pedestrians.Num()) / PedestrianSettings.MaxPedestrians;
	}

	return Population;
}

void UMGAmbientLifeSubsystem::SetTimeOfDayMultipliers(float TrafficMult, float PedestrianMult)
{
	TimeOfDayTrafficMultiplier = FMath::Clamp(TrafficMult, 0.0f, 2.0f);
	TimeOfDayPedestrianMultiplier = FMath::Clamp(PedestrianMult, 0.0f, 2.0f);
}

void UMGAmbientLifeSubsystem::OnRaceStarting(const TArray<FVector>& RaceRoute)
{
	bRaceActive = true;
	CurrentRaceRoute = RaceRoute;

	// Clear traffic along race route
	for (const FVector& Point : RaceRoute)
	{
		ClearTrafficInRadius(Point, 100.0f);
	}

	// Set up spectators along the route
	TArray<FVector> SpectatorPoints;
	for (int32 i = 0; i < RaceRoute.Num(); i += 3)
	{
		SpectatorPoints.Add(RaceRoute[i] + FVector(50.0f, 0.0f, 0.0f));
		SpectatorPoints.Add(RaceRoute[i] + FVector(-50.0f, 0.0f, 0.0f));
	}
	SetupRaceSpectators(SpectatorPoints);

	// Trigger cheering behavior
	for (const FVector& Point : SpectatorPoints)
	{
		TriggerPedestrianReaction(Point, 40.0f, EMGPedestrianBehavior::Cheering);
	}
}

void UMGAmbientLifeSubsystem::OnRaceEnded()
{
	bRaceActive = false;
	CurrentRaceRoute.Empty();

	// Remove race spectator zones
	CrowdZones.RemoveAll([](const FMGCrowdZone& Zone)
	{
		return Zone.bIsRaceSpectators;
	});

	// Return pedestrians to normal behavior
	for (FMGAmbientPedestrian& Pedestrian : Pedestrians)
	{
		if (Pedestrian.Behavior == EMGPedestrianBehavior::Cheering ||
			Pedestrian.Behavior == EMGPedestrianBehavior::Spectating)
		{
			Pedestrian.Behavior = EMGPedestrianBehavior::Walking;
		}
	}
}

void UMGAmbientLifeSubsystem::UpdateAmbientLife(float DeltaTime)
{
	// Spawn/despawn traffic to meet target
	if (!bTrafficPaused)
	{
		int32 TargetTraffic = GetTargetTrafficCount();
		while (TrafficVehicles.Num() < TargetTraffic)
		{
			SpawnTrafficVehicle();
		}
	}

	// Spawn/despawn pedestrians
	int32 TargetPedestrians = GetTargetPedestrianCount();
	while (Pedestrians.Num() < TargetPedestrians)
	{
		SpawnPedestrian();
	}

	// Update crowds
	UpdateCrowds(DeltaTime);
}

void UMGAmbientLifeSubsystem::UpdateCrowds(float DeltaTime)
{
	for (FMGCrowdZone& Zone : CrowdZones)
	{
		// Gradually fill crowds
		if (Zone.CurrentCrowdSize < Zone.TargetCrowdSize)
		{
			Zone.CurrentCrowdSize = FMath::Min(
				Zone.CurrentCrowdSize + 1,
				Zone.TargetCrowdSize
			);
		}
	}
}

void UMGAmbientLifeSubsystem::SpawnTrafficVehicle()
{
	FMGAmbientVehicle Vehicle;
	Vehicle.VehicleID = FGuid::NewGuid().ToString();
	Vehicle.bIsTraffic = true;
	Vehicle.bIsParked = false;
	Vehicle.CurrentSpeed = FMath::RandRange(20.0f, 60.0f);

	// Would spawn at valid road location
	Vehicle.Transform.SetLocation(FVector(
		FMath::RandRange(-1000.0f, 1000.0f),
		FMath::RandRange(-1000.0f, 1000.0f),
		0.0f
	));

	TrafficVehicles.Add(Vehicle);
}

void UMGAmbientLifeSubsystem::SpawnPedestrian()
{
	FMGAmbientPedestrian Pedestrian;
	Pedestrian.PedestrianID = FGuid::NewGuid().ToString();
	Pedestrian.Behavior = EMGPedestrianBehavior::Walking;
	Pedestrian.bHasPhone = FMath::FRand() < 0.3f; // 30% have phones

	// Would spawn at valid sidewalk location
	Pedestrian.Location = FVector(
		FMath::RandRange(-500.0f, 500.0f),
		FMath::RandRange(-500.0f, 500.0f),
		0.0f
	);

	Pedestrians.Add(Pedestrian);
}

int32 UMGAmbientLifeSubsystem::GetTargetTrafficCount() const
{
	float Multiplier = TrafficSettings.DensityMultiplier * TimeOfDayTrafficMultiplier;

	// Reduce during races
	if (bRaceActive)
	{
		Multiplier *= 0.3f;
	}

	return FMath::RoundToInt(TrafficSettings.MaxVehicles * Multiplier);
}

int32 UMGAmbientLifeSubsystem::GetTargetPedestrianCount() const
{
	float Multiplier = PedestrianSettings.DensityMultiplier * TimeOfDayPedestrianMultiplier;
	return FMath::RoundToInt(PedestrianSettings.MaxPedestrians * Multiplier);
}
