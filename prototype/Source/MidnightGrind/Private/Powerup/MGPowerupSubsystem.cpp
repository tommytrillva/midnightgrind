// Copyright Epic Games, Inc. All Rights Reserved.

#include "Powerup/MGPowerupSubsystem.h"
#include "TimerManager.h"

void UMGPowerupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InstanceCounter = 0;

	// Set up default balance config
	BalanceConfig.bEnableRubberBanding = true;
	BalanceConfig.LeaderPowerupNerf = 0.8f;
	BalanceConfig.LastPlacePowerupBuff = 1.5f;
	BalanceConfig.GlobalCooldownMultiplier = 1.0f;
	BalanceConfig.GlobalDurationMultiplier = 1.0f;

	// Register default power-up definitions
	FMGPowerupDefinition SpeedBoostDef;
	SpeedBoostDef.PowerupId = TEXT("SpeedBoost");
	SpeedBoostDef.DisplayName = FText::FromString(TEXT("Speed Boost"));
	SpeedBoostDef.Description = FText::FromString(TEXT("Temporary speed increase"));
	SpeedBoostDef.Type = EMGPowerupType::SpeedBoost;
	SpeedBoostDef.Rarity = EMGPowerupRarity::Common;
	SpeedBoostDef.TargetType = EMGPowerupTarget::Self;
	SpeedBoostDef.Duration = 3.0f;
	SpeedBoostDef.EffectMagnitude = 1.3f;
	SpeedBoostDef.PowerupColor = FLinearColor(0.0f, 0.8f, 1.0f);
	RegisterPowerupDefinition(SpeedBoostDef);

	FMGPowerupDefinition ShieldDef;
	ShieldDef.PowerupId = TEXT("Shield");
	ShieldDef.DisplayName = FText::FromString(TEXT("Shield"));
	ShieldDef.Description = FText::FromString(TEXT("Blocks one incoming attack"));
	ShieldDef.Type = EMGPowerupType::Shield;
	ShieldDef.Rarity = EMGPowerupRarity::Uncommon;
	ShieldDef.TargetType = EMGPowerupTarget::Self;
	ShieldDef.Duration = 10.0f;
	ShieldDef.MaxCharges = 1;
	ShieldDef.PowerupColor = FLinearColor(0.0f, 1.0f, 0.5f);
	RegisterPowerupDefinition(ShieldDef);

	FMGPowerupDefinition MissileDef;
	MissileDef.PowerupId = TEXT("Missile");
	MissileDef.DisplayName = FText::FromString(TEXT("Homing Missile"));
	MissileDef.Description = FText::FromString(TEXT("Launches a homing missile at the nearest opponent"));
	MissileDef.Type = EMGPowerupType::Missile;
	MissileDef.Rarity = EMGPowerupRarity::Rare;
	MissileDef.TargetType = EMGPowerupTarget::Homing;
	MissileDef.Range = 10000.0f;
	MissileDef.EffectMagnitude = 50.0f;
	MissileDef.bRequiresTarget = true;
	MissileDef.bCanBeBlocked = true;
	MissileDef.PowerupColor = FLinearColor(1.0f, 0.3f, 0.0f);
	RegisterPowerupDefinition(MissileDef);

	FMGPowerupDefinition OilSlickDef;
	OilSlickDef.PowerupId = TEXT("OilSlick");
	OilSlickDef.DisplayName = FText::FromString(TEXT("Oil Slick"));
	OilSlickDef.Description = FText::FromString(TEXT("Drops an oil slick that causes spin outs"));
	OilSlickDef.Type = EMGPowerupType::OilSlick;
	OilSlickDef.Rarity = EMGPowerupRarity::Common;
	OilSlickDef.TargetType = EMGPowerupTarget::Backward;
	OilSlickDef.Radius = 300.0f;
	OilSlickDef.Duration = 15.0f;
	OilSlickDef.EffectMagnitude = 2.0f;
	OilSlickDef.bCanBeBlocked = false;
	OilSlickDef.PowerupColor = FLinearColor(0.2f, 0.2f, 0.2f);
	RegisterPowerupDefinition(OilSlickDef);

	FMGPowerupDefinition EMPDef;
	EMPDef.PowerupId = TEXT("EMP");
	EMPDef.DisplayName = FText::FromString(TEXT("EMP Blast"));
	EMPDef.Description = FText::FromString(TEXT("Disables nearby opponents temporarily"));
	EMPDef.Type = EMGPowerupType::EMPBlast;
	EMPDef.Rarity = EMGPowerupRarity::Epic;
	EMPDef.TargetType = EMGPowerupTarget::AreaOfEffect;
	EMPDef.Radius = 1500.0f;
	EMPDef.EffectMagnitude = 3.0f;
	EMPDef.bCanBeBlocked = true;
	EMPDef.PowerupColor = FLinearColor(0.5f, 0.0f, 1.0f);
	RegisterPowerupDefinition(EMPDef);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGPowerupSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			PowerupTickTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->TickPowerups(0.033f);
				}
			},
			0.033f,
			true
		);
	}

	LoadPowerupData();
}

void UMGPowerupSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PowerupTickTimer);
	}

	SavePowerupData();
	Super::Deinitialize();
}

// ============================================================================
// Power-up Definitions
// ============================================================================

void UMGPowerupSubsystem::RegisterPowerupDefinition(const FMGPowerupDefinition& Definition)
{
	PowerupDefinitions.Add(Definition.Type, Definition);
}

FMGPowerupDefinition UMGPowerupSubsystem::GetPowerupDefinition(EMGPowerupType Type) const
{
	if (const FMGPowerupDefinition* Found = PowerupDefinitions.Find(Type))
	{
		return *Found;
	}
	return FMGPowerupDefinition();
}

TArray<FMGPowerupDefinition> UMGPowerupSubsystem::GetAllPowerupDefinitions() const
{
	TArray<FMGPowerupDefinition> Result;
	for (const auto& Pair : PowerupDefinitions)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

// ============================================================================
// Spawn Point Management
// ============================================================================

void UMGPowerupSubsystem::RegisterSpawnPoint(const FMGPickupSpawnPoint& SpawnPoint)
{
	FMGPickupSpawnPoint NewPoint = SpawnPoint;
	if (NewPoint.SpawnPointId.IsEmpty())
	{
		NewPoint.SpawnPointId = FString::Printf(TEXT("SpawnPoint_%d"), ++InstanceCounter);
	}
	SpawnPoints.Add(NewPoint.SpawnPointId, NewPoint);
}

void UMGPowerupSubsystem::UnregisterSpawnPoint(const FString& SpawnPointId)
{
	SpawnPoints.Remove(SpawnPointId);
}

void UMGPowerupSubsystem::ActivateAllSpawnPoints()
{
	for (auto& Pair : SpawnPoints)
	{
		Pair.Value.bIsActive = true;
		RespawnPickup(Pair.Key);
	}
}

void UMGPowerupSubsystem::DeactivateAllSpawnPoints()
{
	for (auto& Pair : SpawnPoints)
	{
		Pair.Value.bIsActive = false;
		Pair.Value.CurrentPowerup = EMGPowerupType::None;
	}
}

void UMGPowerupSubsystem::RespawnPickup(const FString& SpawnPointId)
{
	FMGPickupSpawnPoint* Point = SpawnPoints.Find(SpawnPointId);
	if (!Point || !Point->bIsActive)
	{
		return;
	}

	// Roll for a powerup type
	TArray<EMGPowerupType> AllowedTypes = Point->AllowedPowerups;
	if (AllowedTypes.Num() == 0)
	{
		// Use all registered types
		for (const auto& Pair : PowerupDefinitions)
		{
			AllowedTypes.Add(Pair.Key);
		}
	}

	if (AllowedTypes.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, AllowedTypes.Num() - 1);
		Point->CurrentPowerup = AllowedTypes[Index];
		Point->TimeUntilRespawn = 0.0f;

		OnPickupSpawned.Broadcast(SpawnPointId, Point->CurrentPowerup);
	}
}

TArray<FMGPickupSpawnPoint> UMGPowerupSubsystem::GetActiveSpawnPoints() const
{
	TArray<FMGPickupSpawnPoint> Result;
	for (const auto& Pair : SpawnPoints)
	{
		if (Pair.Value.bIsActive && Pair.Value.CurrentPowerup != EMGPowerupType::None)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// ============================================================================
// Collection
// ============================================================================

bool UMGPowerupSubsystem::TryCollectPickup(const FString& PlayerId, const FString& SpawnPointId, int32 RacePosition)
{
	FMGPickupSpawnPoint* Point = SpawnPoints.Find(SpawnPointId);
	if (!Point || !Point->bIsActive || Point->CurrentPowerup == EMGPowerupType::None)
	{
		return false;
	}

	// Grant the powerup
	bool bSuccess = GrantPowerup(PlayerId, Point->CurrentPowerup);
	if (bSuccess)
	{
		// Clear spawn point and start respawn timer
		Point->CurrentPowerup = EMGPowerupType::None;
		Point->TimeUntilRespawn = Point->RespawnTime;
	}

	return bSuccess;
}

bool UMGPowerupSubsystem::GrantPowerup(const FString& PlayerId, EMGPowerupType Type, int32 SlotIndex)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		InitializePlayerInventory(PlayerId);
		Inventory = PlayerInventories.Find(PlayerId);
	}

	// Find target slot
	int32 TargetSlot = SlotIndex;
	if (TargetSlot < 0)
	{
		TargetSlot = GetEmptySlot(PlayerId);
	}

	if (TargetSlot < 0 || TargetSlot >= Inventory->Slots.Num())
	{
		return false;
	}

	FMGPowerupSlot& Slot = Inventory->Slots[TargetSlot];
	if (Slot.bIsLocked)
	{
		return false;
	}

	// Get definition
	const FMGPowerupDefinition* Def = PowerupDefinitions.Find(Type);
	if (!Def)
	{
		return false;
	}

	// Check for stacking
	if (Slot.Powerup.Type == Type && Def->bCanStack)
	{
		if (Slot.Powerup.CurrentStacks < Def->MaxStacks)
		{
			Slot.Powerup.CurrentStacks++;
			Slot.Powerup.CurrentCharges = FMath::Min(Slot.Powerup.CurrentCharges + 1, Def->MaxCharges);
		}
		else
		{
			return false; // Max stacks reached
		}
	}
	else if (Slot.Powerup.Type != EMGPowerupType::None)
	{
		// Slot occupied with different type
		return false;
	}
	else
	{
		// New powerup
		Slot.Powerup = FMGActivePowerup();
		Slot.Powerup.InstanceId = GenerateInstanceId();
		Slot.Powerup.PowerupId = Def->PowerupId;
		Slot.Powerup.Type = Type;
		Slot.Powerup.State = EMGPowerupState::Ready;
		Slot.Powerup.CurrentCharges = Def->MaxCharges;
		Slot.Powerup.CurrentStacks = 1;
		Slot.Powerup.SourcePlayerId = PlayerId;
	}

	// Update stats
	FMGPowerupStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		int32& Count = Stats->PowerupsCollected.FindOrAdd(Type);
		Count++;
	}

	Inventory->TotalPowerupsCollected++;

	OnPowerupCollected.Broadcast(PlayerId, Type, TargetSlot);
	return true;
}

EMGPowerupType UMGPowerupSubsystem::RollPowerup(int32 RacePosition, int32 TotalRacers, const TArray<EMGPowerupType>& AllowedTypes)
{
	if (AllowedTypes.Num() == 0)
	{
		return EMGPowerupType::None;
	}

	// Build weighted list based on position and rarity
	TArray<TPair<EMGPowerupType, float>> WeightedTypes;
	float TotalWeight = 0.0f;

	for (EMGPowerupType Type : AllowedTypes)
	{
		const FMGPowerupDefinition* Def = PowerupDefinitions.Find(Type);
		if (!Def)
		{
			continue;
		}

		float Weight = 1.0f;

		// Adjust weight based on rarity
		switch (Def->Rarity)
		{
			case EMGPowerupRarity::Common:
				Weight = 4.0f;
				break;
			case EMGPowerupRarity::Uncommon:
				Weight = 3.0f;
				break;
			case EMGPowerupRarity::Rare:
				Weight = 2.0f;
				break;
			case EMGPowerupRarity::Epic:
				Weight = 1.0f;
				break;
			case EMGPowerupRarity::Legendary:
				Weight = 0.5f;
				break;
		}

		// Position-based rubber banding
		if (BalanceConfig.bEnableRubberBanding && TotalRacers > 1)
		{
			float PositionPercent = static_cast<float>(RacePosition - 1) / FMath::Max(1, TotalRacers - 1);

			// Higher position gets better powerups
			if (PositionPercent > 0.5f)
			{
				// Back of pack - boost offensive and powerful items
				if (Def->Rarity >= EMGPowerupRarity::Rare)
				{
					Weight *= (1.0f + PositionPercent);
				}
			}
			else
			{
				// Front of pack - nerf powerful items
				if (Def->Rarity >= EMGPowerupRarity::Rare)
				{
					Weight *= BalanceConfig.LeaderPowerupNerf;
				}
			}

			// Check restricted powerups for leader
			if (RacePosition == 1)
			{
				if (BalanceConfig.LeaderRestrictedPowerups.Contains(Type))
				{
					Weight = 0.0f;
				}
			}
		}

		if (Weight > 0.0f)
		{
			WeightedTypes.Add(TPair<EMGPowerupType, float>(Type, Weight));
			TotalWeight += Weight;
		}
	}

	if (TotalWeight <= 0.0f)
	{
		return AllowedTypes[0];
	}

	// Roll
	float Roll = FMath::FRand() * TotalWeight;
	float Accumulated = 0.0f;

	for (const auto& Pair : WeightedTypes)
	{
		Accumulated += Pair.Value;
		if (Roll <= Accumulated)
		{
			return Pair.Key;
		}
	}

	return WeightedTypes.Last().Key;
}

// ============================================================================
// Inventory Management
// ============================================================================

void UMGPowerupSubsystem::InitializePlayerInventory(const FString& PlayerId, int32 MaxSlots)
{
	FMGPowerupInventory Inventory;
	Inventory.PlayerId = PlayerId;
	Inventory.MaxSlots = MaxSlots;

	for (int32 i = 0; i < MaxSlots; i++)
	{
		FMGPowerupSlot Slot;
		Slot.SlotIndex = i;
		Inventory.Slots.Add(Slot);
	}

	PlayerInventories.Add(PlayerId, Inventory);

	// Initialize stats
	FMGPowerupStats Stats;
	Stats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, Stats);
}

FMGPowerupInventory UMGPowerupSubsystem::GetPlayerInventory(const FString& PlayerId) const
{
	if (const FMGPowerupInventory* Found = PlayerInventories.Find(PlayerId))
	{
		return *Found;
	}
	return FMGPowerupInventory();
}

bool UMGPowerupSubsystem::HasPowerup(const FString& PlayerId, EMGPowerupType Type) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return false;
	}

	for (const FMGPowerupSlot& Slot : Inventory->Slots)
	{
		if (Slot.Powerup.Type == Type && Slot.Powerup.State == EMGPowerupState::Ready)
		{
			return true;
		}
	}

	return false;
}

FMGPowerupSlot UMGPowerupSubsystem::GetSlot(const FString& PlayerId, int32 SlotIndex) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory || SlotIndex < 0 || SlotIndex >= Inventory->Slots.Num())
	{
		return FMGPowerupSlot();
	}
	return Inventory->Slots[SlotIndex];
}

int32 UMGPowerupSubsystem::GetEmptySlot(const FString& PlayerId) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return -1;
	}

	for (int32 i = 0; i < Inventory->Slots.Num(); i++)
	{
		if (!Inventory->Slots[i].bIsLocked &&
		    Inventory->Slots[i].Powerup.Type == EMGPowerupType::None)
		{
			return i;
		}
	}

	return -1;
}

void UMGPowerupSubsystem::SwapSlots(const FString& PlayerId, int32 SlotA, int32 SlotB)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return;
	}

	if (SlotA < 0 || SlotA >= Inventory->Slots.Num() ||
	    SlotB < 0 || SlotB >= Inventory->Slots.Num())
	{
		return;
	}

	if (Inventory->Slots[SlotA].bIsLocked || Inventory->Slots[SlotB].bIsLocked)
	{
		return;
	}

	FMGActivePowerup Temp = Inventory->Slots[SlotA].Powerup;
	Inventory->Slots[SlotA].Powerup = Inventory->Slots[SlotB].Powerup;
	Inventory->Slots[SlotB].Powerup = Temp;
}

void UMGPowerupSubsystem::DiscardSlot(const FString& PlayerId, int32 SlotIndex)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory || SlotIndex < 0 || SlotIndex >= Inventory->Slots.Num())
	{
		return;
	}

	if (Inventory->Slots[SlotIndex].bIsLocked)
	{
		return;
	}

	Inventory->Slots[SlotIndex].Powerup = FMGActivePowerup();
}

// ============================================================================
// Activation
// ============================================================================

bool UMGPowerupSubsystem::UsePowerup(const FString& PlayerId, int32 SlotIndex, const FString& TargetId)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory || SlotIndex < 0 || SlotIndex >= Inventory->Slots.Num())
	{
		return false;
	}

	FMGPowerupSlot& Slot = Inventory->Slots[SlotIndex];
	if (Slot.Powerup.Type == EMGPowerupType::None ||
	    Slot.Powerup.State != EMGPowerupState::Ready)
	{
		return false;
	}

	const FMGPowerupDefinition* Def = PowerupDefinitions.Find(Slot.Powerup.Type);
	if (!Def)
	{
		return false;
	}

	// Check if target is required
	if (Def->bRequiresTarget && TargetId.IsEmpty())
	{
		return false;
	}

	// Activate the effect
	bool bSuccess = ActivatePowerupEffect(PlayerId, Slot.Powerup.Type, TargetId);
	if (!bSuccess)
	{
		return false;
	}

	// Consume charge
	Slot.Powerup.CurrentCharges--;
	if (Slot.Powerup.CurrentCharges <= 0)
	{
		// Clear slot
		Slot.Powerup = FMGActivePowerup();
	}

	// Update stats
	FMGPowerupStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		int32& Count = Stats->PowerupsUsed.FindOrAdd(Def->Type);
		Count++;
	}

	Inventory->TotalPowerupsUsed++;

	return true;
}

bool UMGPowerupSubsystem::ActivatePowerupEffect(const FString& PlayerId, EMGPowerupType Type, const FString& TargetId)
{
	const FMGPowerupDefinition* Def = PowerupDefinitions.Find(Type);
	if (!Def)
	{
		return false;
	}

	FMGActivePowerup ActivePowerup;
	ActivePowerup.InstanceId = GenerateInstanceId();
	ActivePowerup.PowerupId = Def->PowerupId;
	ActivePowerup.Type = Type;
	ActivePowerup.State = EMGPowerupState::Active;
	ActivePowerup.TimeRemaining = Def->Duration * BalanceConfig.GlobalDurationMultiplier;
	ActivePowerup.SourcePlayerId = PlayerId;
	ActivePowerup.TargetPlayerId = TargetId.IsEmpty() ? PlayerId : TargetId;
	ActivePowerup.ActivationTime = FDateTime::Now();

	// Handle different target types
	switch (Def->TargetType)
	{
		case EMGPowerupTarget::Self:
		{
			// Apply to self
			FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
			if (Inventory)
			{
				Inventory->ActiveEffects.Add(ActivePowerup);
				ApplyPowerupEffect(PlayerId, *Def);
			}
			break;
		}

		case EMGPowerupTarget::SingleEnemy:
		case EMGPowerupTarget::Homing:
		{
			// Launch projectile
			FMGPowerupProjectile Projectile;
			Projectile.ProjectileId = GenerateInstanceId();
			Projectile.PowerupType = Type;
			Projectile.SourcePlayerId = PlayerId;
			Projectile.TargetPlayerId = TargetId;
			Projectile.Speed = 5000.0f;
			Projectile.LifetimeRemaining = 10.0f;
			Projectile.Damage = Def->EffectMagnitude;
			Projectile.bIsHoming = (Def->TargetType == EMGPowerupTarget::Homing);
			Projectile.HomingStrength = 0.5f;

			LaunchProjectile(Projectile);

			// Update stats
			FMGPowerupStats* Stats = PlayerStats.Find(PlayerId);
			if (Stats)
			{
				Stats->TotalProjectilesLaunched++;
			}
			break;
		}

		case EMGPowerupTarget::Backward:
		{
			// Drop hazard behind vehicle
			// This would be handled by the calling code which has position info
			break;
		}

		case EMGPowerupTarget::AreaOfEffect:
		{
			// Immediate area effect - targets would be determined by game code
			break;
		}

		default:
			break;
	}

	// Special handling for shield
	if (Type == EMGPowerupType::Shield)
	{
		ActivateShield(PlayerId, Def->Duration);
	}

	OnPowerupActivated.Broadcast(PlayerId, ActivePowerup, TargetId);
	return true;
}

void UMGPowerupSubsystem::DeactivatePowerup(const FString& PlayerId, const FString& InstanceId)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return;
	}

	for (int32 i = Inventory->ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (Inventory->ActiveEffects[i].InstanceId == InstanceId)
		{
			RemovePowerupEffect(PlayerId, Inventory->ActiveEffects[i].Type);
			Inventory->ActiveEffects.RemoveAt(i);
			break;
		}
	}
}

void UMGPowerupSubsystem::DeactivateAllPowerups(const FString& PlayerId)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return;
	}

	for (const FMGActivePowerup& Effect : Inventory->ActiveEffects)
	{
		RemovePowerupEffect(PlayerId, Effect.Type);
	}

	Inventory->ActiveEffects.Empty();
	Inventory->bHasShield = false;
	Inventory->ShieldTimeRemaining = 0.0f;
}

// ============================================================================
// Shield
// ============================================================================

void UMGPowerupSubsystem::ActivateShield(const FString& PlayerId, float Duration)
{
	FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return;
	}

	Inventory->bHasShield = true;
	Inventory->ShieldTimeRemaining = Duration * BalanceConfig.GlobalDurationMultiplier;

	OnShieldActivated.Broadcast(PlayerId, Inventory->ShieldTimeRemaining);
}

bool UMGPowerupSubsystem::HasActiveShield(const FString& PlayerId) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	return Inventory && Inventory->bHasShield;
}

float UMGPowerupSubsystem::GetShieldTimeRemaining(const FString& PlayerId) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return 0.0f;
	}
	return Inventory->ShieldTimeRemaining;
}

bool UMGPowerupSubsystem::TryBlockAttack(const FString& TargetId, EMGPowerupType AttackType)
{
	const FMGPowerupDefinition* Def = PowerupDefinitions.Find(AttackType);
	if (Def && !Def->bCanBeBlocked)
	{
		return false;
	}

	FMGPowerupInventory* Inventory = PlayerInventories.Find(TargetId);
	if (!Inventory || !Inventory->bHasShield)
	{
		return false;
	}

	// Shield blocks the attack
	Inventory->bHasShield = false;
	Inventory->ShieldTimeRemaining = 0.0f;

	// Update stats
	FMGPowerupStats* Stats = PlayerStats.Find(TargetId);
	if (Stats)
	{
		int32& Count = Stats->HitsBlocked.FindOrAdd(AttackType);
		Count++;
	}

	OnPowerupBlocked.Broadcast(TargetId, AttackType);
	OnShieldDepleted.Broadcast(TargetId);

	return true;
}

// ============================================================================
// Projectiles
// ============================================================================

FString UMGPowerupSubsystem::LaunchProjectile(const FMGPowerupProjectile& Projectile)
{
	FMGPowerupProjectile NewProjectile = Projectile;
	if (NewProjectile.ProjectileId.IsEmpty())
	{
		NewProjectile.ProjectileId = GenerateInstanceId();
	}

	ActiveProjectiles.Add(NewProjectile);
	OnProjectileLaunched.Broadcast(NewProjectile.ProjectileId, NewProjectile);

	return NewProjectile.ProjectileId;
}

void UMGPowerupSubsystem::UpdateProjectiles(float DeltaTime)
{
	for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; i--)
	{
		FMGPowerupProjectile& Proj = ActiveProjectiles[i];

		Proj.LifetimeRemaining -= DeltaTime;
		if (Proj.LifetimeRemaining <= 0.0f)
		{
			ActiveProjectiles.RemoveAt(i);
			continue;
		}

		// Update position (direction would need target position from game code)
		Proj.Location += Proj.Velocity * DeltaTime;
	}
}

TArray<FMGPowerupProjectile> UMGPowerupSubsystem::GetActiveProjectiles() const
{
	return ActiveProjectiles;
}

void UMGPowerupSubsystem::DestroyProjectile(const FString& ProjectileId)
{
	for (int32 i = ActiveProjectiles.Num() - 1; i >= 0; i--)
	{
		if (ActiveProjectiles[i].ProjectileId == ProjectileId)
		{
			ActiveProjectiles.RemoveAt(i);
			return;
		}
	}
}

bool UMGPowerupSubsystem::CheckProjectileHit(const FString& ProjectileId, const FString& TargetId, FVector TargetLocation)
{
	int32 ProjectileIndex = -1;
	for (int32 i = 0; i < ActiveProjectiles.Num(); i++)
	{
		if (ActiveProjectiles[i].ProjectileId == ProjectileId)
		{
			ProjectileIndex = i;
			break;
		}
	}

	if (ProjectileIndex < 0)
	{
		return false;
	}

	const FMGPowerupProjectile& Proj = ActiveProjectiles[ProjectileIndex];

	// Check collision
	float Distance = FVector::Dist(Proj.Location, TargetLocation);
	if (Distance > 200.0f) // Hit radius
	{
		return false;
	}

	// Check if blocked by shield
	if (TryBlockAttack(TargetId, Proj.PowerupType))
	{
		DestroyProjectile(ProjectileId);
		return false;
	}

	// Hit confirmed
	FMGPowerupStats* SourceStats = PlayerStats.Find(Proj.SourcePlayerId);
	if (SourceStats)
	{
		int32& Count = SourceStats->HitsDealt.FindOrAdd(Proj.PowerupType);
		Count++;
		SourceStats->TotalProjectilesHit++;

		if (SourceStats->TotalProjectilesLaunched > 0)
		{
			SourceStats->ProjectileAccuracy = static_cast<float>(SourceStats->TotalProjectilesHit) /
			                                   SourceStats->TotalProjectilesLaunched;
		}
	}

	FMGPowerupStats* TargetStats = PlayerStats.Find(TargetId);
	if (TargetStats)
	{
		int32& Count = TargetStats->HitsReceived.FindOrAdd(Proj.PowerupType);
		Count++;
	}

	OnPowerupHit.Broadcast(Proj.SourcePlayerId, TargetId, Proj.PowerupType);
	DestroyProjectile(ProjectileId);

	return true;
}

// ============================================================================
// Hazards
// ============================================================================

FString UMGPowerupSubsystem::DropHazard(const FMGDroppedHazard& Hazard)
{
	FMGDroppedHazard NewHazard = Hazard;
	if (NewHazard.HazardId.IsEmpty())
	{
		NewHazard.HazardId = GenerateInstanceId();
	}

	ActiveHazards.Add(NewHazard);
	OnHazardDropped.Broadcast(NewHazard);

	return NewHazard.HazardId;
}

void UMGPowerupSubsystem::UpdateHazards(float DeltaTime)
{
	for (int32 i = ActiveHazards.Num() - 1; i >= 0; i--)
	{
		ActiveHazards[i].LifetimeRemaining -= DeltaTime;
		if (ActiveHazards[i].LifetimeRemaining <= 0.0f)
		{
			ActiveHazards.RemoveAt(i);
		}
	}
}

TArray<FMGDroppedHazard> UMGPowerupSubsystem::GetActiveHazards() const
{
	return ActiveHazards;
}

void UMGPowerupSubsystem::RemoveHazard(const FString& HazardId)
{
	for (int32 i = ActiveHazards.Num() - 1; i >= 0; i--)
	{
		if (ActiveHazards[i].HazardId == HazardId)
		{
			ActiveHazards.RemoveAt(i);
			return;
		}
	}
}

bool UMGPowerupSubsystem::CheckHazardCollision(const FString& PlayerId, FVector PlayerLocation)
{
	for (FMGDroppedHazard& Hazard : ActiveHazards)
	{
		// Skip if hazard doesn't affect owner
		if (!Hazard.bAffectsOwner && Hazard.SourcePlayerId == PlayerId)
		{
			continue;
		}

		float Distance = FVector::Dist(PlayerLocation, Hazard.Location);
		if (Distance <= Hazard.Radius)
		{
			// Check for shield
			if (!TryBlockAttack(PlayerId, Hazard.SourcePowerup))
			{
				// Hit by hazard
				OnPowerupHit.Broadcast(Hazard.SourcePlayerId, PlayerId, Hazard.SourcePowerup);
				return true;
			}
		}
	}

	return false;
}

// ============================================================================
// Effects Query
// ============================================================================

bool UMGPowerupSubsystem::HasActiveEffect(const FString& PlayerId, EMGPowerupType EffectType) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return false;
	}

	for (const FMGActivePowerup& Effect : Inventory->ActiveEffects)
	{
		if (Effect.Type == EffectType && Effect.State == EMGPowerupState::Active)
		{
			return true;
		}
	}

	return false;
}

float UMGPowerupSubsystem::GetEffectTimeRemaining(const FString& PlayerId, EMGPowerupType EffectType) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return 0.0f;
	}

	for (const FMGActivePowerup& Effect : Inventory->ActiveEffects)
	{
		if (Effect.Type == EffectType && Effect.State == EMGPowerupState::Active)
		{
			return Effect.TimeRemaining;
		}
	}

	return 0.0f;
}

float UMGPowerupSubsystem::GetEffectMultiplier(const FString& PlayerId, EMGPowerupType EffectType) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return 1.0f;
	}

	for (const FMGActivePowerup& Effect : Inventory->ActiveEffects)
	{
		if (Effect.Type == EffectType && Effect.State == EMGPowerupState::Active)
		{
			return Effect.EffectMultiplier;
		}
	}

	return 1.0f;
}

TArray<FMGActivePowerup> UMGPowerupSubsystem::GetAllActiveEffects(const FString& PlayerId) const
{
	const FMGPowerupInventory* Inventory = PlayerInventories.Find(PlayerId);
	if (!Inventory)
	{
		return TArray<FMGActivePowerup>();
	}

	return Inventory->ActiveEffects;
}

// ============================================================================
// Balance
// ============================================================================

void UMGPowerupSubsystem::SetBalanceConfig(const FMGPowerupBalanceConfig& Config)
{
	BalanceConfig = Config;
}

FMGPowerupBalanceConfig UMGPowerupSubsystem::GetBalanceConfig() const
{
	return BalanceConfig;
}

float UMGPowerupSubsystem::GetPositionMultiplier(int32 Position, int32 TotalRacers) const
{
	if (TotalRacers <= 1 || !BalanceConfig.bEnableRubberBanding)
	{
		return 1.0f;
	}

	float PositionPercent = static_cast<float>(Position - 1) / FMath::Max(1, TotalRacers - 1);

	// Interpolate between leader nerf and last place buff
	return FMath::Lerp(BalanceConfig.LeaderPowerupNerf, BalanceConfig.LastPlacePowerupBuff, PositionPercent);
}

// ============================================================================
// Stats
// ============================================================================

FMGPowerupStats UMGPowerupSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGPowerupStats* Found = PlayerStats.Find(PlayerId))
	{
		return *Found;
	}
	return FMGPowerupStats();
}

void UMGPowerupSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGPowerupStats Stats;
	Stats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, Stats);
}

// ============================================================================
// Update
// ============================================================================

void UMGPowerupSubsystem::UpdatePowerups(float DeltaTime)
{
	// Update all player inventories
	for (auto& Pair : PlayerInventories)
	{
		FMGPowerupInventory& Inventory = Pair.Value;

		// Update shield
		if (Inventory.bHasShield)
		{
			Inventory.ShieldTimeRemaining -= DeltaTime;
			if (Inventory.ShieldTimeRemaining <= 0.0f)
			{
				Inventory.bHasShield = false;
				Inventory.ShieldTimeRemaining = 0.0f;
				OnShieldDepleted.Broadcast(Inventory.PlayerId);
			}
		}

		// Update active effects
		for (int32 i = Inventory.ActiveEffects.Num() - 1; i >= 0; i--)
		{
			FMGActivePowerup& Effect = Inventory.ActiveEffects[i];
			if (Effect.State == EMGPowerupState::Active)
			{
				Effect.TimeRemaining -= DeltaTime;
				if (Effect.TimeRemaining <= 0.0f)
				{
					ProcessPowerupExpiration(Inventory.PlayerId, Effect);
					Inventory.ActiveEffects.RemoveAt(i);
				}
			}
		}
	}

	UpdateProjectiles(DeltaTime);
	UpdateHazards(DeltaTime);
}

void UMGPowerupSubsystem::UpdateSpawnPoints(float DeltaTime)
{
	for (auto& Pair : SpawnPoints)
	{
		FMGPickupSpawnPoint& Point = Pair.Value;

		if (!Point.bIsActive)
		{
			continue;
		}

		// Handle respawn timer
		if (Point.CurrentPowerup == EMGPowerupType::None && Point.TimeUntilRespawn > 0.0f)
		{
			Point.TimeUntilRespawn -= DeltaTime;
			if (Point.TimeUntilRespawn <= 0.0f)
			{
				RespawnPickup(Point.SpawnPointId);
			}
		}
	}
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGPowerupSubsystem::SavePowerupData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Powerup");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("powerup_stats.dat");

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

		const FMGPowerupStats& Stats = Pair.Value;

		// Save powerups collected map
		int32 NumCollected = Stats.PowerupsCollected.Num();
		SaveArchive << NumCollected;
		for (const auto& CollPair : Stats.PowerupsCollected)
		{
			int32 TypeInt = static_cast<int32>(CollPair.Key);
			int32 Count = CollPair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save powerups used map
		int32 NumUsed = Stats.PowerupsUsed.Num();
		SaveArchive << NumUsed;
		for (const auto& UsedPair : Stats.PowerupsUsed)
		{
			int32 TypeInt = static_cast<int32>(UsedPair.Key);
			int32 Count = UsedPair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save hits dealt map
		int32 NumHitsDealt = Stats.HitsDealt.Num();
		SaveArchive << NumHitsDealt;
		for (const auto& HitPair : Stats.HitsDealt)
		{
			int32 TypeInt = static_cast<int32>(HitPair.Key);
			int32 Count = HitPair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save hits received map
		int32 NumHitsReceived = Stats.HitsReceived.Num();
		SaveArchive << NumHitsReceived;
		for (const auto& HitPair : Stats.HitsReceived)
		{
			int32 TypeInt = static_cast<int32>(HitPair.Key);
			int32 Count = HitPair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save hits blocked map
		int32 NumHitsBlocked = Stats.HitsBlocked.Num();
		SaveArchive << NumHitsBlocked;
		for (const auto& HitPair : Stats.HitsBlocked)
		{
			int32 TypeInt = static_cast<int32>(HitPair.Key);
			int32 Count = HitPair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save scalar stats
		int32 TotalProjectilesLaunched = Stats.TotalProjectilesLaunched;
		int32 TotalProjectilesHit = Stats.TotalProjectilesHit;
		float ProjectileAccuracy = Stats.ProjectileAccuracy;

		SaveArchive << TotalProjectilesLaunched;
		SaveArchive << TotalProjectilesHit;
		SaveArchive << ProjectileAccuracy;
	}

	// Write to file
	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	UE_LOG(LogTemp, Log, TEXT("MGPowerupSubsystem: Saved powerup data for %d players"), NumPlayers);
}

void UMGPowerupSubsystem::LoadPowerupData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Powerup") / TEXT("powerup_stats.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("MGPowerupSubsystem: No saved powerup data found"));
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGPowerupSubsystem: Unknown save version %d"), Version);
		return;
	}

	// Load player stats
	int32 NumPlayers;
	LoadArchive << NumPlayers;

	for (int32 i = 0; i < NumPlayers; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGPowerupStats Stats;
		Stats.PlayerId = PlayerId;

		// Load powerups collected map
		int32 NumCollected;
		LoadArchive << NumCollected;
		for (int32 j = 0; j < NumCollected; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.PowerupsCollected.Add(static_cast<EMGPowerupType>(TypeInt), Count);
		}

		// Load powerups used map
		int32 NumUsed;
		LoadArchive << NumUsed;
		for (int32 j = 0; j < NumUsed; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.PowerupsUsed.Add(static_cast<EMGPowerupType>(TypeInt), Count);
		}

		// Load hits dealt map
		int32 NumHitsDealt;
		LoadArchive << NumHitsDealt;
		for (int32 j = 0; j < NumHitsDealt; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.HitsDealt.Add(static_cast<EMGPowerupType>(TypeInt), Count);
		}

		// Load hits received map
		int32 NumHitsReceived;
		LoadArchive << NumHitsReceived;
		for (int32 j = 0; j < NumHitsReceived; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.HitsReceived.Add(static_cast<EMGPowerupType>(TypeInt), Count);
		}

		// Load hits blocked map
		int32 NumHitsBlocked;
		LoadArchive << NumHitsBlocked;
		for (int32 j = 0; j < NumHitsBlocked; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.HitsBlocked.Add(static_cast<EMGPowerupType>(TypeInt), Count);
		}

		// Load scalar stats
		LoadArchive << Stats.TotalProjectilesLaunched;
		LoadArchive << Stats.TotalProjectilesHit;
		LoadArchive << Stats.ProjectileAccuracy;

		PlayerStats.Add(PlayerId, Stats);
	}

	UE_LOG(LogTemp, Log, TEXT("MGPowerupSubsystem: Loaded powerup data for %d players"), NumPlayers);
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGPowerupSubsystem::TickPowerups(float DeltaTime)
{
	UpdatePowerups(DeltaTime);
	UpdateSpawnPoints(DeltaTime);
}

void UMGPowerupSubsystem::ProcessPowerupExpiration(const FString& PlayerId, FMGActivePowerup& Powerup)
{
	Powerup.State = EMGPowerupState::Depleted;
	RemovePowerupEffect(PlayerId, Powerup.Type);
	OnPowerupExpired.Broadcast(PlayerId, Powerup.Type);
}

void UMGPowerupSubsystem::ApplyPowerupEffect(const FString& PlayerId, const FMGPowerupDefinition& Definition)
{
	// Placeholder - actual effect application would be handled by game systems
	// This would typically modify vehicle stats, apply visual effects, etc.
}

void UMGPowerupSubsystem::RemovePowerupEffect(const FString& PlayerId, EMGPowerupType Type)
{
	// Placeholder - actual effect removal would be handled by game systems
}

FString UMGPowerupSubsystem::GenerateInstanceId() const
{
	return FString::Printf(TEXT("PWR_%d_%lld"), ++const_cast<UMGPowerupSubsystem*>(this)->InstanceCounter,
	                       FDateTime::Now().GetTicks());
}
