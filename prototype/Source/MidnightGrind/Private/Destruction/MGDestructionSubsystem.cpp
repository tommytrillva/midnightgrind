// Copyright Epic Games, Inc. All Rights Reserved.

#include "Destruction/MGDestructionSubsystem.h"
#include "TimerManager.h"

void UMGDestructionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InstanceCounter = 0;
	EventCounter = 0;
	ChainCounter = 0;
	TotalPropertyDamage = 0.0f;

	// Set up default scoring config
	ScoringConfig.ComboWindowSeconds = 2.0f;
	ScoringConfig.ComboMultiplierPerHit = 0.1f;
	ScoringConfig.MaxComboMultiplier = 5.0f;
	ScoringConfig.SpeedBonusThreshold = 100.0f;
	ScoringConfig.SpeedBonusMultiplier = 1.5f;
	ScoringConfig.ChainReactionMultiplier = 2.0f;
	ScoringConfig.SpectacularThreshold = 5;
	ScoringConfig.SpectacularBonus = 500;

	// Category base points
	ScoringConfig.CategoryBasePoints.Add(EMGDestructionCategory::Minor, 10);
	ScoringConfig.CategoryBasePoints.Add(EMGDestructionCategory::Standard, 25);
	ScoringConfig.CategoryBasePoints.Add(EMGDestructionCategory::Major, 50);
	ScoringConfig.CategoryBasePoints.Add(EMGDestructionCategory::Spectacular, 100);
	ScoringConfig.CategoryBasePoints.Add(EMGDestructionCategory::Legendary, 250);

	// Register some default destructible definitions
	FMGDestructibleDefinition TrashCanDef;
	TrashCanDef.DestructibleId = TEXT("TrashCan");
	TrashCanDef.DisplayName = FText::FromString(TEXT("Trash Can"));
	TrashCanDef.Type = EMGDestructibleType::StreetFurniture;
	TrashCanDef.Category = EMGDestructionCategory::Minor;
	TrashCanDef.DestructionEffect = EMGDestructionEffect::Shatter;
	TrashCanDef.Health = 25.0f;
	TrashCanDef.Mass = 10.0f;
	TrashCanDef.MinImpactSpeed = 20.0f;
	TrashCanDef.BasePoints = 10;
	TrashCanDef.SlowdownFactor = 0.98f;
	TrashCanDef.DamageToVehicle = 1.0f;
	RegisterDestructibleDefinition(TrashCanDef);

	FMGDestructibleDefinition HydrantDef;
	HydrantDef.DestructibleId = TEXT("FireHydrant");
	HydrantDef.DisplayName = FText::FromString(TEXT("Fire Hydrant"));
	HydrantDef.Type = EMGDestructibleType::Hydrant;
	HydrantDef.Category = EMGDestructionCategory::Standard;
	HydrantDef.DestructionEffect = EMGDestructionEffect::Spray;
	HydrantDef.Health = 100.0f;
	HydrantDef.Mass = 100.0f;
	HydrantDef.MinImpactSpeed = 50.0f;
	HydrantDef.BasePoints = 50;
	HydrantDef.SlowdownFactor = 0.90f;
	HydrantDef.DamageToVehicle = 10.0f;
	RegisterDestructibleDefinition(HydrantDef);

	FMGDestructibleDefinition BillboardDef;
	BillboardDef.DestructibleId = TEXT("Billboard");
	BillboardDef.DisplayName = FText::FromString(TEXT("Billboard"));
	BillboardDef.Type = EMGDestructibleType::Billboard;
	BillboardDef.Category = EMGDestructionCategory::Major;
	BillboardDef.DestructionEffect = EMGDestructionEffect::Collapse;
	BillboardDef.Health = 200.0f;
	BillboardDef.Mass = 500.0f;
	BillboardDef.MinImpactSpeed = 80.0f;
	BillboardDef.BasePoints = 100;
	BillboardDef.SlowdownFactor = 0.75f;
	BillboardDef.DamageToVehicle = 25.0f;
	BillboardDef.bCanChainReact = true;
	BillboardDef.ChainReactRadius = 1000.0f;
	RegisterDestructibleDefinition(BillboardDef);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGDestructionSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			DestructionTickTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->TickDestruction(0.033f);
				}
			},
			0.033f,
			true
		);
	}

	LoadDestructionData();
}

void UMGDestructionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DestructionTickTimer);
	}

	SaveDestructionData();
	Super::Deinitialize();
}

// ============================================================================
// Definition Registration
// ============================================================================

void UMGDestructionSubsystem::RegisterDestructibleDefinition(const FMGDestructibleDefinition& Definition)
{
	if (Definition.DestructibleId.IsEmpty())
	{
		return;
	}
	Definitions.Add(Definition.DestructibleId, Definition);
}

FMGDestructibleDefinition UMGDestructionSubsystem::GetDestructibleDefinition(const FString& DestructibleId) const
{
	if (const FMGDestructibleDefinition* Found = Definitions.Find(DestructibleId))
	{
		return *Found;
	}
	return FMGDestructibleDefinition();
}

TArray<FMGDestructibleDefinition> UMGDestructionSubsystem::GetAllDefinitions() const
{
	TArray<FMGDestructibleDefinition> Result;
	for (const auto& Pair : Definitions)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

// ============================================================================
// Instance Management
// ============================================================================

FString UMGDestructionSubsystem::SpawnDestructible(const FString& DestructibleId, FVector Location, FRotator Rotation)
{
	const FMGDestructibleDefinition* Def = Definitions.Find(DestructibleId);
	if (!Def)
	{
		return FString();
	}

	FMGDestructibleInstance Instance;
	Instance.InstanceId = GenerateInstanceId();
	Instance.DestructibleId = DestructibleId;
	Instance.Location = Location;
	Instance.Rotation = Rotation;
	Instance.CurrentHealth = Def->Health;
	Instance.bIsDestroyed = false;

	Instances.Add(Instance.InstanceId, Instance);
	return Instance.InstanceId;
}

void UMGDestructionSubsystem::RemoveDestructible(const FString& InstanceId)
{
	Instances.Remove(InstanceId);
}

FMGDestructibleInstance UMGDestructionSubsystem::GetDestructibleInstance(const FString& InstanceId) const
{
	if (const FMGDestructibleInstance* Found = Instances.Find(InstanceId))
	{
		return *Found;
	}
	return FMGDestructibleInstance();
}

TArray<FMGDestructibleInstance> UMGDestructionSubsystem::GetDestructiblesInRadius(FVector Center, float Radius) const
{
	TArray<FMGDestructibleInstance> Result;
	float RadiusSq = Radius * Radius;

	for (const auto& Pair : Instances)
	{
		if (!Pair.Value.bIsDestroyed)
		{
			float DistSq = FVector::DistSquared(Center, Pair.Value.Location);
			if (DistSq <= RadiusSq)
			{
				Result.Add(Pair.Value);
			}
		}
	}

	return Result;
}

TArray<FMGDestructibleInstance> UMGDestructionSubsystem::GetDestroyedInstances() const
{
	TArray<FMGDestructibleInstance> Result;
	for (const auto& Pair : Instances)
	{
		if (Pair.Value.bIsDestroyed)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// ============================================================================
// Destruction
// ============================================================================

FMGDestructionEvent UMGDestructionSubsystem::DestroyDestructible(const FString& InstanceId, const FString& PlayerId, FVector ImpactVelocity)
{
	FMGDestructionEvent Event;

	FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance || Instance->bIsDestroyed)
	{
		return Event;
	}

	const FMGDestructibleDefinition* Def = Definitions.Find(Instance->DestructibleId);
	if (!Def)
	{
		return Event;
	}

	// Mark as destroyed
	Instance->bIsDestroyed = true;
	Instance->DestroyedByPlayerId = PlayerId;
	Instance->DestructionTime = FDateTime::Now();
	Instance->CurrentHealth = 0.0f;

	// Set up respawn if applicable
	if (Def->RespawnTime > 0.0f)
	{
		Instance->bIsRespawning = true;
		Instance->RespawnTimer = Def->RespawnTime;
	}

	// Get combo info
	float ComboMultiplier = 1.0f;
	if (const FMGDestructionCombo* Combo = ActiveCombos.Find(PlayerId))
	{
		ComboMultiplier = Combo->ComboMultiplier;
	}

	// Calculate points
	float ImpactSpeed = ImpactVelocity.Size() / 44.704f; // Convert to MPH
	int32 Points = CalculateDestructionPoints(Instance->DestructibleId, ImpactSpeed, ComboMultiplier);

	// Create event
	Event.EventId = GenerateEventId();
	Event.PlayerId = PlayerId;
	Event.InstanceId = InstanceId;
	Event.DestructibleId = Instance->DestructibleId;
	Event.Type = Def->Type;
	Event.Category = Def->Category;
	Event.Location = Instance->Location;
	Event.ImpactVelocity = ImpactVelocity;
	Event.ImpactSpeed = ImpactSpeed;
	Event.PointsEarned = Points;
	Event.ComboMultiplier = ComboMultiplier;
	Event.Timestamp = FDateTime::Now();

	// Update player stats
	FMGDestructionStats& Stats = PlayerStats.FindOrAdd(PlayerId);
	Stats.PlayerId = PlayerId;
	Stats.TotalDestroyed++;
	Stats.TotalPoints += Points;
	Stats.TotalPropertyDamage += Def->Mass * 10.0f; // Rough damage estimate

	int32& TypeCount = Stats.TypeCounts.FindOrAdd(Def->Type);
	TypeCount++;

	int32& CategoryCount = Stats.CategoryCounts.FindOrAdd(Def->Category);
	CategoryCount++;

	// Update global damage
	TotalPropertyDamage += Def->Mass * 10.0f;

	// Extend combo
	ExtendCombo(PlayerId, Event);

	// Check for spectacular destruction
	CheckSpectacularDestruction(PlayerId, Event);

	// Update zone progress
	UpdateZoneProgress(InstanceId);

	// Check for chain reaction
	if (Def->bCanChainReact && Def->ChainReactRadius > 0.0f)
	{
		FString ChainId = StartChainReaction(PlayerId, InstanceId);
		Event.bWasChainReaction = true;
	}

	OnDestructibleDestroyed.Broadcast(PlayerId, Event);

	return Event;
}

bool UMGDestructionSubsystem::DamageDestructible(const FString& InstanceId, float Damage, const FString& PlayerId)
{
	FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance || Instance->bIsDestroyed)
	{
		return false;
	}

	Instance->CurrentHealth -= Damage;

	if (Instance->CurrentHealth <= 0.0f)
	{
		DestroyDestructible(InstanceId, PlayerId, FVector::ZeroVector);
		return true;
	}

	return false;
}

bool UMGDestructionSubsystem::TryDestroyOnImpact(const FString& InstanceId, const FString& PlayerId, FVector ImpactVelocity, float ImpactForce)
{
	float ImpactSpeed = ImpactVelocity.Size() / 44.704f; // Convert to MPH

	if (!CanBeDestroyed(InstanceId, ImpactSpeed))
	{
		return false;
	}

	FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance)
	{
		return false;
	}

	const FMGDestructibleDefinition* Def = Definitions.Find(Instance->DestructibleId);
	if (!Def)
	{
		return false;
	}

	// Calculate damage based on impact
	float Damage = ImpactForce * (ImpactSpeed / Def->MinImpactSpeed);

	if (Damage >= Instance->CurrentHealth)
	{
		DestroyDestructible(InstanceId, PlayerId, ImpactVelocity);
		return true;
	}
	else
	{
		return DamageDestructible(InstanceId, Damage, PlayerId);
	}
}

bool UMGDestructionSubsystem::CanBeDestroyed(const FString& InstanceId, float ImpactSpeed) const
{
	const FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance || Instance->bIsDestroyed)
	{
		return false;
	}

	const FMGDestructibleDefinition* Def = Definitions.Find(Instance->DestructibleId);
	if (!Def)
	{
		return false;
	}

	return ImpactSpeed >= Def->MinImpactSpeed;
}

// ============================================================================
// Combo System
// ============================================================================

FMGDestructionCombo UMGDestructionSubsystem::GetCurrentCombo(const FString& PlayerId) const
{
	if (const FMGDestructionCombo* Found = ActiveCombos.Find(PlayerId))
	{
		return *Found;
	}
	return FMGDestructionCombo();
}

bool UMGDestructionSubsystem::HasActiveCombo(const FString& PlayerId) const
{
	const FMGDestructionCombo* Combo = ActiveCombos.Find(PlayerId);
	return Combo && Combo->CurrentCount > 0 && Combo->TimeRemaining > 0.0f;
}

void UMGDestructionSubsystem::ExtendCombo(const FString& PlayerId, const FMGDestructionEvent& Event)
{
	FMGDestructionCombo& Combo = ActiveCombos.FindOrAdd(PlayerId);
	Combo.PlayerId = PlayerId;
	Combo.CurrentCount++;
	Combo.ComboMultiplier = FMath::Min(
		1.0f + (Combo.CurrentCount * ScoringConfig.ComboMultiplierPerHit),
		ScoringConfig.MaxComboMultiplier
	);
	Combo.TimeRemaining = ScoringConfig.ComboWindowSeconds;
	Combo.TotalPoints += Event.PointsEarned;
	Combo.ComboEvents.Add(Event);

	// Update highest combo
	FMGDestructionStats& Stats = PlayerStats.FindOrAdd(PlayerId);
	if (Combo.CurrentCount > Stats.HighestCombo)
	{
		Stats.HighestCombo = Combo.CurrentCount;
	}

	OnDestructionComboUpdated.Broadcast(PlayerId, Combo.CurrentCount, Combo.ComboMultiplier);
}

void UMGDestructionSubsystem::EndCombo(const FString& PlayerId)
{
	FMGDestructionCombo* Combo = ActiveCombos.Find(PlayerId);
	if (!Combo || Combo->CurrentCount == 0)
	{
		return;
	}

	OnDestructionComboEnded.Broadcast(PlayerId, Combo->TotalPoints);
	ResetCombo(PlayerId);
}

void UMGDestructionSubsystem::ResetCombo(const FString& PlayerId)
{
	FMGDestructionCombo* Combo = ActiveCombos.Find(PlayerId);
	if (Combo)
	{
		Combo->CurrentCount = 0;
		Combo->ComboMultiplier = 1.0f;
		Combo->TimeRemaining = 0.0f;
		Combo->TotalPoints = 0;
		Combo->ComboEvents.Empty();
	}
}

// ============================================================================
// Chain Reactions
// ============================================================================

FString UMGDestructionSubsystem::StartChainReaction(const FString& PlayerId, const FString& InitialInstanceId)
{
	const FMGDestructibleInstance* Instance = Instances.Find(InitialInstanceId);
	if (!Instance)
	{
		return FString();
	}

	const FMGDestructibleDefinition* Def = Definitions.Find(Instance->DestructibleId);
	if (!Def || !Def->bCanChainReact)
	{
		return FString();
	}

	// Find chainable instances
	TArray<FString> Chainable = GetChainableInstances(
		Instance->Location,
		Def->ChainReactRadius,
		TArray<FString>({InitialInstanceId})
	);

	if (Chainable.Num() == 0)
	{
		return FString();
	}

	FMGChainReaction Chain;
	Chain.ChainId = GenerateChainId();
	Chain.InitiatorPlayerId = PlayerId;
	Chain.AffectedInstances = Chainable;
	Chain.ChainLength = 1;
	Chain.bIsActive = true;

	ActiveChainReactions.Add(Chain.ChainId, Chain);

	OnChainReactionStarted.Broadcast(PlayerId, Chain.ChainId);

	return Chain.ChainId;
}

void UMGDestructionSubsystem::ProcessChainReaction(const FString& ChainId)
{
	FMGChainReaction* Chain = ActiveChainReactions.Find(ChainId);
	if (!Chain || !Chain->bIsActive)
	{
		return;
	}

	ProcessChainReactionStep(ChainId);
}

FMGChainReaction UMGDestructionSubsystem::GetChainReaction(const FString& ChainId) const
{
	if (const FMGChainReaction* Found = ActiveChainReactions.Find(ChainId))
	{
		return *Found;
	}
	return FMGChainReaction();
}

TArray<FString> UMGDestructionSubsystem::GetChainableInstances(FVector Origin, float Radius, const TArray<FString>& ExcludeIds) const
{
	TArray<FString> Result;
	float RadiusSq = Radius * Radius;

	for (const auto& Pair : Instances)
	{
		if (Pair.Value.bIsDestroyed || ExcludeIds.Contains(Pair.Key))
		{
			continue;
		}

		// Check if the definition allows chain reaction triggers
		const FMGDestructibleDefinition* Def = Definitions.Find(Pair.Value.DestructibleId);
		if (!Def)
		{
			continue;
		}

		float DistSq = FVector::DistSquared(Origin, Pair.Value.Location);
		if (DistSq <= RadiusSq)
		{
			Result.Add(Pair.Key);
		}
	}

	return Result;
}

// ============================================================================
// Zones
// ============================================================================

void UMGDestructionSubsystem::RegisterDestructionZone(const FMGDestructionZone& Zone)
{
	if (Zone.ZoneId.IsEmpty())
	{
		return;
	}
	Zones.Add(Zone.ZoneId, Zone);
}

FMGDestructionZone UMGDestructionSubsystem::GetDestructionZone(const FString& ZoneId) const
{
	if (const FMGDestructionZone* Found = Zones.Find(ZoneId))
	{
		return *Found;
	}
	return FMGDestructionZone();
}

TArray<FMGDestructionZone> UMGDestructionSubsystem::GetAllZones() const
{
	TArray<FMGDestructionZone> Result;
	for (const auto& Pair : Zones)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

void UMGDestructionSubsystem::UpdateZoneProgress(const FString& InstanceId)
{
	const FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance)
	{
		return;
	}

	for (auto& ZonePair : Zones)
	{
		FMGDestructionZone& Zone = ZonePair.Value;

		if (Zone.bIsCompleted)
		{
			continue;
		}

		// Check if instance is in zone
		if (Zone.DestructibleInstances.Contains(InstanceId))
		{
			Zone.DestroyedCount++;

			float Percent = GetZoneCompletionPercent(Zone.ZoneId);
			OnDestructionZoneProgress.Broadcast(Zone.ZoneId, Percent);

			// Check for completion
			if (Zone.DestroyedCount >= Zone.TotalDestructibles)
			{
				Zone.bIsCompleted = true;
				OnDestructionZoneCompleted.Broadcast(Zone.ZoneId, Zone.CompletionBonus);
			}
		}
		// Also check by distance if no explicit list
		else if (Zone.DestructibleInstances.Num() == 0)
		{
			float Distance = FVector::Dist(Instance->Location, Zone.Center);
			if (Distance <= Zone.Radius)
			{
				Zone.DestroyedCount++;
				float Percent = static_cast<float>(Zone.DestroyedCount) / FMath::Max(1, Zone.TotalDestructibles) * 100.0f;
				OnDestructionZoneProgress.Broadcast(Zone.ZoneId, Percent);
			}
		}
	}
}

float UMGDestructionSubsystem::GetZoneCompletionPercent(const FString& ZoneId) const
{
	const FMGDestructionZone* Zone = Zones.Find(ZoneId);
	if (!Zone || Zone->TotalDestructibles == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(Zone->DestroyedCount) / Zone->TotalDestructibles * 100.0f;
}

// ============================================================================
// Scoring
// ============================================================================

int32 UMGDestructionSubsystem::CalculateDestructionPoints(const FString& DestructibleId, float ImpactSpeed, float ComboMultiplier) const
{
	const FMGDestructibleDefinition* Def = Definitions.Find(DestructibleId);
	if (!Def)
	{
		return 0;
	}

	int32 BasePoints = Def->BasePoints;

	// Add category bonus
	if (const int32* CategoryBonus = ScoringConfig.CategoryBasePoints.Find(Def->Category))
	{
		BasePoints += *CategoryBonus;
	}

	// Apply speed bonus
	float SpeedMultiplier = 1.0f;
	if (ImpactSpeed >= ScoringConfig.SpeedBonusThreshold)
	{
		SpeedMultiplier = ScoringConfig.SpeedBonusMultiplier;
	}

	// Calculate final points
	int32 Points = FMath::RoundToInt(BasePoints * SpeedMultiplier * ComboMultiplier);

	return Points;
}

void UMGDestructionSubsystem::SetScoringConfig(const FMGDestructionScoringConfig& Config)
{
	ScoringConfig = Config;
}

FMGDestructionScoringConfig UMGDestructionSubsystem::GetScoringConfig() const
{
	return ScoringConfig;
}

// ============================================================================
// Stats
// ============================================================================

FMGDestructionStats UMGDestructionSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGDestructionStats* Found = PlayerStats.Find(PlayerId))
	{
		return *Found;
	}
	return FMGDestructionStats();
}

void UMGDestructionSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGDestructionStats Stats;
	Stats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, Stats);
}

int32 UMGDestructionSubsystem::GetTotalDestroyedCount() const
{
	int32 Count = 0;
	for (const auto& Pair : Instances)
	{
		if (Pair.Value.bIsDestroyed)
		{
			Count++;
		}
	}
	return Count;
}

float UMGDestructionSubsystem::GetTotalPropertyDamage() const
{
	return TotalPropertyDamage;
}

// ============================================================================
// Respawn
// ============================================================================

void UMGDestructionSubsystem::RespawnDestructible(const FString& InstanceId)
{
	FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance)
	{
		return;
	}

	const FMGDestructibleDefinition* Def = Definitions.Find(Instance->DestructibleId);
	if (!Def || Def->bBlocksRespawn)
	{
		return;
	}

	Instance->bIsDestroyed = false;
	Instance->bIsRespawning = false;
	Instance->CurrentHealth = Def->Health;
	Instance->RespawnTimer = 0.0f;
	Instance->DestroyedByPlayerId = TEXT("");

	OnDestructibleRespawned.Broadcast(InstanceId, Instance->Location);
}

void UMGDestructionSubsystem::RespawnAll()
{
	for (auto& Pair : Instances)
	{
		if (Pair.Value.bIsDestroyed)
		{
			RespawnDestructible(Pair.Key);
		}
	}
}

void UMGDestructionSubsystem::RespawnInRadius(FVector Center, float Radius)
{
	float RadiusSq = Radius * Radius;

	for (auto& Pair : Instances)
	{
		if (Pair.Value.bIsDestroyed)
		{
			float DistSq = FVector::DistSquared(Center, Pair.Value.Location);
			if (DistSq <= RadiusSq)
			{
				RespawnDestructible(Pair.Key);
			}
		}
	}
}

// ============================================================================
// Update
// ============================================================================

void UMGDestructionSubsystem::UpdateDestruction(float DeltaTime)
{
	UpdateCombos(DeltaTime);
	UpdateRespawns(DeltaTime);
	UpdateChainReactions(DeltaTime);
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGDestructionSubsystem::SaveDestructionData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Destruction");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("destruction_stats.dat");

	FBufferArchive SaveArchive;

	// Version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save player stats
	int32 NumPlayers = PlayerStats.Num();
	SaveArchive << NumPlayers;

	for (const auto& Pair : PlayerStats)
	{
		FString PlayerId = Pair.Key;
		SaveArchive << PlayerId;

		const FMGDestructionStats& Stats = Pair.Value;

		// Save type counts map
		int32 NumTypes = Stats.TypeCounts.Num();
		SaveArchive << NumTypes;
		for (const auto& TypePair : Stats.TypeCounts)
		{
			int32 TypeInt = static_cast<int32>(TypePair.Key);
			int32 Count = TypePair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save category counts map
		int32 NumCategories = Stats.CategoryCounts.Num();
		SaveArchive << NumCategories;
		for (const auto& CatPair : Stats.CategoryCounts)
		{
			int32 CatInt = static_cast<int32>(CatPair.Key);
			int32 Count = CatPair.Value;
			SaveArchive << CatInt;
			SaveArchive << Count;
		}

		// Save scalar stats
		int32 TotalDestroyed = Stats.TotalDestroyed;
		int32 TotalPoints = Stats.TotalPoints;
		int32 HighestCombo = Stats.HighestCombo;
		int32 LongestChainReaction = Stats.LongestChainReaction;
		float TotalPropertyDamage = Stats.TotalPropertyDamage;
		int32 SpectacularDestructions = Stats.SpectacularDestructions;

		SaveArchive << TotalDestroyed;
		SaveArchive << TotalPoints;
		SaveArchive << HighestCombo;
		SaveArchive << LongestChainReaction;
		SaveArchive << TotalPropertyDamage;
		SaveArchive << SpectacularDestructions;
	}

	// Save global property damage
	float GlobalDamage = TotalPropertyDamage;
	SaveArchive << GlobalDamage;

	// Save zone completion status
	int32 NumZones = Zones.Num();
	SaveArchive << NumZones;

	for (const auto& Pair : Zones)
	{
		FString ZoneId = Pair.Key;
		SaveArchive << ZoneId;

		const FMGDestructionZone& Zone = Pair.Value;
		int32 DestroyedCount = Zone.DestroyedCount;
		bool bIsCompleted = Zone.bIsCompleted;

		SaveArchive << DestroyedCount;
		SaveArchive << bIsCompleted;
	}

	// Write to file
	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	UE_LOG(LogTemp, Log, TEXT("MGDestructionSubsystem: Saved destruction data for %d players, %d zones"), NumPlayers, NumZones);
}

void UMGDestructionSubsystem::LoadDestructionData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Destruction") / TEXT("destruction_stats.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("MGDestructionSubsystem: No saved destruction data found"));
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDestructionSubsystem: Unknown save version %d"), Version);
		return;
	}

	// Load player stats
	int32 NumPlayers;
	LoadArchive << NumPlayers;

	for (int32 i = 0; i < NumPlayers; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGDestructionStats Stats;
		Stats.PlayerId = PlayerId;

		// Load type counts map
		int32 NumTypes;
		LoadArchive << NumTypes;
		for (int32 j = 0; j < NumTypes; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.TypeCounts.Add(static_cast<EMGDestructibleType>(TypeInt), Count);
		}

		// Load category counts map
		int32 NumCategories;
		LoadArchive << NumCategories;
		for (int32 j = 0; j < NumCategories; ++j)
		{
			int32 CatInt;
			int32 Count;
			LoadArchive << CatInt;
			LoadArchive << Count;
			Stats.CategoryCounts.Add(static_cast<EMGDestructionCategory>(CatInt), Count);
		}

		// Load scalar stats
		LoadArchive << Stats.TotalDestroyed;
		LoadArchive << Stats.TotalPoints;
		LoadArchive << Stats.HighestCombo;
		LoadArchive << Stats.LongestChainReaction;
		LoadArchive << Stats.TotalPropertyDamage;
		LoadArchive << Stats.SpectacularDestructions;

		PlayerStats.Add(PlayerId, Stats);
	}

	// Load global property damage
	LoadArchive << TotalPropertyDamage;

	// Load zone completion status
	int32 NumZones;
	LoadArchive << NumZones;

	for (int32 i = 0; i < NumZones; ++i)
	{
		FString ZoneId;
		LoadArchive << ZoneId;

		int32 DestroyedCount;
		bool bIsCompleted;

		LoadArchive << DestroyedCount;
		LoadArchive << bIsCompleted;

		// Update zone if it exists
		if (FMGDestructionZone* Zone = Zones.Find(ZoneId))
		{
			Zone->DestroyedCount = DestroyedCount;
			Zone->bIsCompleted = bIsCompleted;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MGDestructionSubsystem: Loaded destruction data for %d players"), NumPlayers);
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGDestructionSubsystem::TickDestruction(float DeltaTime)
{
	UpdateDestruction(DeltaTime);
}

void UMGDestructionSubsystem::UpdateCombos(float DeltaTime)
{
	TArray<FString> ExpiredCombos;

	for (auto& Pair : ActiveCombos)
	{
		FMGDestructionCombo& Combo = Pair.Value;

		if (Combo.CurrentCount > 0 && Combo.TimeRemaining > 0.0f)
		{
			Combo.TimeRemaining -= DeltaTime;

			if (Combo.TimeRemaining <= 0.0f)
			{
				ExpiredCombos.Add(Pair.Key);
			}
		}
	}

	// End expired combos
	for (const FString& PlayerId : ExpiredCombos)
	{
		EndCombo(PlayerId);
	}
}

void UMGDestructionSubsystem::UpdateRespawns(float DeltaTime)
{
	for (auto& Pair : Instances)
	{
		FMGDestructibleInstance& Instance = Pair.Value;

		if (Instance.bIsRespawning && Instance.RespawnTimer > 0.0f)
		{
			Instance.RespawnTimer -= DeltaTime;

			if (Instance.RespawnTimer <= 0.0f)
			{
				RespawnDestructible(Pair.Key);
			}
		}
	}
}

void UMGDestructionSubsystem::UpdateChainReactions(float DeltaTime)
{
	TArray<FString> CompletedChains;

	for (auto& Pair : ActiveChainReactions)
	{
		FMGChainReaction& Chain = Pair.Value;

		if (Chain.bIsActive)
		{
			Chain.ChainDuration += DeltaTime;

			// Process next step
			ProcessChainReactionStep(Pair.Key);

			// Check if chain is complete
			if (Chain.AffectedInstances.Num() == 0)
			{
				Chain.bIsActive = false;
				CompletedChains.Add(Pair.Key);
			}
		}
	}

	// Notify completed chains
	for (const FString& ChainId : CompletedChains)
	{
		const FMGChainReaction* Chain = ActiveChainReactions.Find(ChainId);
		if (Chain)
		{
			// Update stats
			FMGDestructionStats* Stats = PlayerStats.Find(Chain->InitiatorPlayerId);
			if (Stats)
			{
				if (Chain->ChainLength > Stats->LongestChainReaction)
				{
					Stats->LongestChainReaction = Chain->ChainLength;
				}
			}

			OnChainReactionEnded.Broadcast(ChainId, Chain->ChainLength, Chain->TotalPoints);
		}
	}
}

void UMGDestructionSubsystem::ProcessChainReactionStep(const FString& ChainId)
{
	FMGChainReaction* Chain = ActiveChainReactions.Find(ChainId);
	if (!Chain || !Chain->bIsActive || Chain->AffectedInstances.Num() == 0)
	{
		return;
	}

	// Destroy first instance in queue
	FString InstanceId = Chain->AffectedInstances[0];
	Chain->AffectedInstances.RemoveAt(0);

	FMGDestructibleInstance* Instance = Instances.Find(InstanceId);
	if (!Instance || Instance->bIsDestroyed)
	{
		return;
	}

	// Destroy with chain multiplier
	FMGDestructionEvent Event = DestroyDestructible(InstanceId, Chain->InitiatorPlayerId, FVector::ZeroVector);

	// Apply chain reaction multiplier
	int32 ChainPoints = FMath::RoundToInt(Event.PointsEarned * ScoringConfig.ChainReactionMultiplier);
	Event.PointsEarned = ChainPoints;
	Event.ChainCount = Chain->ChainLength;
	Event.bWasChainReaction = true;

	Chain->ChainLength++;
	Chain->TotalPoints += ChainPoints;

	// Check for more chainable instances
	const FMGDestructibleDefinition* Def = Definitions.Find(Instance->DestructibleId);
	if (Def && Def->bCanChainReact)
	{
		TArray<FString> Exclude = Chain->AffectedInstances;
		Exclude.Add(InstanceId);

		TArray<FString> MoreChainable = GetChainableInstances(
			Instance->Location,
			Def->ChainReactRadius,
			Exclude
		);

		for (const FString& NewId : MoreChainable)
		{
			if (!Chain->AffectedInstances.Contains(NewId))
			{
				Chain->AffectedInstances.Add(NewId);
			}
		}
	}

	OnChainReactionExtended.Broadcast(ChainId, Chain->ChainLength, ChainPoints);
}

void UMGDestructionSubsystem::CheckSpectacularDestruction(const FString& PlayerId, const FMGDestructionEvent& Event)
{
	const FMGDestructionCombo* Combo = ActiveCombos.Find(PlayerId);
	if (!Combo)
	{
		return;
	}

	// Check if combo count exceeds spectacular threshold
	if (Combo->CurrentCount >= ScoringConfig.SpectacularThreshold &&
	    Combo->CurrentCount % ScoringConfig.SpectacularThreshold == 0)
	{
		FMGDestructionStats* Stats = PlayerStats.Find(PlayerId);
		if (Stats)
		{
			Stats->SpectacularDestructions++;
			Stats->TotalPoints += ScoringConfig.SpectacularBonus;
		}

		OnSpectacularDestruction.Broadcast(PlayerId, ScoringConfig.SpectacularBonus);
	}
}

FString UMGDestructionSubsystem::GenerateInstanceId() const
{
	return FString::Printf(TEXT("DEST_%d_%lld"), ++const_cast<UMGDestructionSubsystem*>(this)->InstanceCounter,
	                       FDateTime::Now().GetTicks());
}

FString UMGDestructionSubsystem::GenerateEventId() const
{
	return FString::Printf(TEXT("EVT_%d_%lld"), ++const_cast<UMGDestructionSubsystem*>(this)->EventCounter,
	                       FDateTime::Now().GetTicks());
}

FString UMGDestructionSubsystem::GenerateChainId() const
{
	return FString::Printf(TEXT("CHAIN_%d_%lld"), ++const_cast<UMGDestructionSubsystem*>(this)->ChainCounter,
	                       FDateTime::Now().GetTicks());
}
