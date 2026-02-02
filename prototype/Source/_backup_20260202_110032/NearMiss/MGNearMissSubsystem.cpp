// Copyright Epic Games, Inc. All Rights Reserved.

#include "NearMiss/MGNearMissSubsystem.h"
#include "Save/MGSaveManagerSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGNearMissSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultThresholds();
	InitializeDefaultBonuses();

	// Initialize default reward config
	RewardConfig.CashPerPoint = 0.1f;
	RewardConfig.ReputationPerPoint = 0.01f;
	RewardConfig.NitroPerPerfectMiss = 5.0f;
	RewardConfig.SpeedBoostOnInsane = 1.1f;
	RewardConfig.SpeedBoostDuration = 2.0f;
	RewardConfig.BonusCashOnFrenzy = 500;
	RewardConfig.ComboMilestoneBonus = 1000;
	RewardConfig.ComboMilestones = { 5, 10, 25, 50, 100 };

	LoadNearMissData();
}

void UMGNearMissSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboTickTimer);
	}

	SaveNearMissData();
	Super::Deinitialize();
}

void UMGNearMissSubsystem::InitializeDefaultThresholds()
{
	// Vehicle near miss
	{
		FMGNearMissThresholds Vehicle;
		Vehicle.MissType = EMGNearMissType::Vehicle;
		Vehicle.BasicDistance = 3.0f;
		Vehicle.GoodDistance = 2.0f;
		Vehicle.GreatDistance = 1.0f;
		Vehicle.PerfectDistance = 0.5f;
		Vehicle.InsaneDistance = 0.25f;
		Vehicle.BasePoints = 100;
		Vehicle.GoodMultiplier = 1.5f;
		Vehicle.GreatMultiplier = 2.0f;
		Vehicle.PerfectMultiplier = 3.0f;
		Vehicle.InsaneMultiplier = 5.0f;
		NearMissThresholds.Add(EMGNearMissType::Vehicle, Vehicle);
	}

	// Traffic
	{
		FMGNearMissThresholds Traffic;
		Traffic.MissType = EMGNearMissType::Traffic;
		Traffic.BasicDistance = 2.5f;
		Traffic.GoodDistance = 1.5f;
		Traffic.GreatDistance = 0.75f;
		Traffic.PerfectDistance = 0.4f;
		Traffic.InsaneDistance = 0.2f;
		Traffic.BasePoints = 75;
		Traffic.GoodMultiplier = 1.5f;
		Traffic.GreatMultiplier = 2.0f;
		Traffic.PerfectMultiplier = 3.0f;
		Traffic.InsaneMultiplier = 5.0f;
		NearMissThresholds.Add(EMGNearMissType::Traffic, Traffic);
	}

	// Oncoming traffic (higher risk, higher reward)
	{
		FMGNearMissThresholds Oncoming;
		Oncoming.MissType = EMGNearMissType::Oncoming;
		Oncoming.BasicDistance = 4.0f;
		Oncoming.GoodDistance = 2.5f;
		Oncoming.GreatDistance = 1.5f;
		Oncoming.PerfectDistance = 0.75f;
		Oncoming.InsaneDistance = 0.35f;
		Oncoming.BasePoints = 200;
		Oncoming.GoodMultiplier = 1.75f;
		Oncoming.GreatMultiplier = 2.5f;
		Oncoming.PerfectMultiplier = 4.0f;
		Oncoming.InsaneMultiplier = 7.0f;
		NearMissThresholds.Add(EMGNearMissType::Oncoming, Oncoming);
	}

	// Obstacle
	{
		FMGNearMissThresholds Obstacle;
		Obstacle.MissType = EMGNearMissType::Obstacle;
		Obstacle.BasicDistance = 1.5f;
		Obstacle.GoodDistance = 1.0f;
		Obstacle.GreatDistance = 0.5f;
		Obstacle.PerfectDistance = 0.25f;
		Obstacle.InsaneDistance = 0.1f;
		Obstacle.BasePoints = 50;
		Obstacle.GoodMultiplier = 1.5f;
		Obstacle.GreatMultiplier = 2.0f;
		Obstacle.PerfectMultiplier = 3.0f;
		Obstacle.InsaneMultiplier = 5.0f;
		NearMissThresholds.Add(EMGNearMissType::Obstacle, Obstacle);
	}

	// Wall graze
	{
		FMGNearMissThresholds Wall;
		Wall.MissType = EMGNearMissType::Wall;
		Wall.BasicDistance = 0.5f;
		Wall.GoodDistance = 0.3f;
		Wall.GreatDistance = 0.15f;
		Wall.PerfectDistance = 0.08f;
		Wall.InsaneDistance = 0.03f;
		Wall.BasePoints = 25;
		Wall.GoodMultiplier = 2.0f;
		Wall.GreatMultiplier = 3.0f;
		Wall.PerfectMultiplier = 5.0f;
		Wall.InsaneMultiplier = 10.0f;
		NearMissThresholds.Add(EMGNearMissType::Wall, Wall);
	}

	// Barrier
	{
		FMGNearMissThresholds Barrier;
		Barrier.MissType = EMGNearMissType::Barrier;
		Barrier.BasicDistance = 0.75f;
		Barrier.GoodDistance = 0.5f;
		Barrier.GreatDistance = 0.25f;
		Barrier.PerfectDistance = 0.1f;
		Barrier.InsaneDistance = 0.05f;
		Barrier.BasePoints = 30;
		Barrier.GoodMultiplier = 1.5f;
		Barrier.GreatMultiplier = 2.5f;
		Barrier.PerfectMultiplier = 4.0f;
		Barrier.InsaneMultiplier = 8.0f;
		NearMissThresholds.Add(EMGNearMissType::Barrier, Barrier);
	}

	// Cliff edge
	{
		FMGNearMissThresholds Cliff;
		Cliff.MissType = EMGNearMissType::Cliff;
		Cliff.BasicDistance = 2.0f;
		Cliff.GoodDistance = 1.0f;
		Cliff.GreatDistance = 0.5f;
		Cliff.PerfectDistance = 0.25f;
		Cliff.InsaneDistance = 0.1f;
		Cliff.BasePoints = 150;
		Cliff.GoodMultiplier = 2.0f;
		Cliff.GreatMultiplier = 3.0f;
		Cliff.PerfectMultiplier = 5.0f;
		Cliff.InsaneMultiplier = 10.0f;
		NearMissThresholds.Add(EMGNearMissType::Cliff, Cliff);
	}

	// Train
	{
		FMGNearMissThresholds Train;
		Train.MissType = EMGNearMissType::Train;
		Train.BasicDistance = 5.0f;
		Train.GoodDistance = 3.0f;
		Train.GreatDistance = 1.5f;
		Train.PerfectDistance = 0.75f;
		Train.InsaneDistance = 0.3f;
		Train.BasePoints = 500;
		Train.GoodMultiplier = 2.0f;
		Train.GreatMultiplier = 3.0f;
		Train.PerfectMultiplier = 5.0f;
		Train.InsaneMultiplier = 10.0f;
		NearMissThresholds.Add(EMGNearMissType::Train, Train);
	}

	// Police evade
	{
		FMGNearMissThresholds Police;
		Police.MissType = EMGNearMissType::Police;
		Police.BasicDistance = 4.0f;
		Police.GoodDistance = 2.5f;
		Police.GreatDistance = 1.5f;
		Police.PerfectDistance = 0.75f;
		Police.InsaneDistance = 0.35f;
		Police.BasePoints = 150;
		Police.GoodMultiplier = 1.5f;
		Police.GreatMultiplier = 2.5f;
		Police.PerfectMultiplier = 4.0f;
		Police.InsaneMultiplier = 7.0f;
		NearMissThresholds.Add(EMGNearMissType::Police, Police);
	}

	// Pedestrian (lowest distance but moderate points)
	{
		FMGNearMissThresholds Pedestrian;
		Pedestrian.MissType = EMGNearMissType::Pedestrian;
		Pedestrian.BasicDistance = 1.0f;
		Pedestrian.GoodDistance = 0.5f;
		Pedestrian.GreatDistance = 0.25f;
		Pedestrian.PerfectDistance = 0.1f;
		Pedestrian.InsaneDistance = 0.05f;
		Pedestrian.BasePoints = 40;
		Pedestrian.GoodMultiplier = 1.5f;
		Pedestrian.GreatMultiplier = 2.0f;
		Pedestrian.PerfectMultiplier = 3.0f;
		Pedestrian.InsaneMultiplier = 5.0f;
		NearMissThresholds.Add(EMGNearMissType::Pedestrian, Pedestrian);
	}
}

void UMGNearMissSubsystem::InitializeDefaultBonuses()
{
	// Drift bonus
	{
		FMGStyleBonus DriftBonus;
		DriftBonus.BonusId = TEXT("DriftNearMiss");
		DriftBonus.BonusName = FText::FromString(TEXT("Drift Style"));
		DriftBonus.Category = EMGStyleCategory::Drift;
		DriftBonus.Multiplier = 1.5f;
		DriftBonus.bRequiresDrift = true;
		RegisteredBonuses.Add(DriftBonus.BonusId, DriftBonus);
	}

	// Air bonus
	{
		FMGStyleBonus AirBonus;
		AirBonus.BonusId = TEXT("AirNearMiss");
		AirBonus.BonusName = FText::FromString(TEXT("Air Style"));
		AirBonus.Category = EMGStyleCategory::Air;
		AirBonus.Multiplier = 2.0f;
		AirBonus.bRequiresAirborne = true;
		RegisteredBonuses.Add(AirBonus.BonusId, AirBonus);
	}

	// Oncoming bonus
	{
		FMGStyleBonus OncomingBonus;
		OncomingBonus.BonusId = TEXT("OncomingStyle");
		OncomingBonus.BonusName = FText::FromString(TEXT("Wrong Way"));
		OncomingBonus.Category = EMGStyleCategory::Skill;
		OncomingBonus.Multiplier = 1.75f;
		OncomingBonus.bRequiresOncoming = true;
		RegisteredBonuses.Add(OncomingBonus.BonusId, OncomingBonus);
	}

	// High speed bonus
	{
		FMGStyleBonus SpeedBonus;
		SpeedBonus.BonusId = TEXT("HighSpeedMiss");
		SpeedBonus.BonusName = FText::FromString(TEXT("Blazing Speed"));
		SpeedBonus.Category = EMGStyleCategory::Speed;
		SpeedBonus.Multiplier = 1.25f;
		SpeedBonus.MinSpeed = 200.0f; // 200 km/h
		RegisteredBonuses.Add(SpeedBonus.BonusId, SpeedBonus);
	}

	// Extreme speed bonus
	{
		FMGStyleBonus ExtremeSpeedBonus;
		ExtremeSpeedBonus.BonusId = TEXT("ExtremeSpeedMiss");
		ExtremeSpeedBonus.BonusName = FText::FromString(TEXT("Lightspeed"));
		ExtremeSpeedBonus.Category = EMGStyleCategory::Speed;
		ExtremeSpeedBonus.Multiplier = 2.0f;
		ExtremeSpeedBonus.MinSpeed = 300.0f; // 300 km/h
		RegisteredBonuses.Add(ExtremeSpeedBonus.BonusId, ExtremeSpeedBonus);
	}

	// Combo bonus
	{
		FMGStyleBonus ComboBonus;
		ComboBonus.BonusId = TEXT("ComboMaster");
		ComboBonus.BonusName = FText::FromString(TEXT("Combo Master"));
		ComboBonus.Category = EMGStyleCategory::Combo;
		ComboBonus.Multiplier = 1.5f;
		ComboBonus.MinCombo = 10;
		RegisteredBonuses.Add(ComboBonus.BonusId, ComboBonus);
	}

	// Drift + Oncoming combo
	{
		FMGStyleBonus DriftOncoming;
		DriftOncoming.BonusId = TEXT("DriftOncoming");
		DriftOncoming.BonusName = FText::FromString(TEXT("Sideways Suicide"));
		DriftOncoming.Category = EMGStyleCategory::Skill;
		DriftOncoming.Multiplier = 3.0f;
		DriftOncoming.bRequiresDrift = true;
		DriftOncoming.bRequiresOncoming = true;
		RegisteredBonuses.Add(DriftOncoming.BonusId, DriftOncoming);
	}
}

void UMGNearMissSubsystem::RegisterProximityTarget(const FMGProximityTarget& Target)
{
	if (!Target.TargetId.IsEmpty())
	{
		ProximityTargets.Add(Target.TargetId, Target);
	}
}

void UMGNearMissSubsystem::UnregisterProximityTarget(const FString& TargetId)
{
	ProximityTargets.Remove(TargetId);
}

void UMGNearMissSubsystem::UpdateProximityTarget(const FString& TargetId, FVector NewLocation, FVector NewVelocity)
{
	if (FMGProximityTarget* Target = ProximityTargets.Find(TargetId))
	{
		Target->Location = NewLocation;
		Target->Velocity = NewVelocity;

		float NewDistance = FVector::Dist(PlayerLocation, NewLocation) - Target->BoundingRadius;
		Target->bIsApproaching = NewDistance < Target->CurrentDistance;
		Target->CurrentDistance = FMath::Max(0.0f, NewDistance);

		if (NewDistance < Target->ClosestApproach)
		{
			Target->ClosestApproach = NewDistance;
		}

		if (UWorld* World = GetWorld())
		{
			Target->LastUpdateTime = World->GetTimeSeconds();
		}
	}
}

void UMGNearMissSubsystem::UpdatePlayerState(FVector InPlayerLocation, FVector InPlayerVelocity, bool bIsDrifting, bool bIsAirborne)
{
	PlayerLocation = InPlayerLocation;
	PlayerVelocity = InPlayerVelocity;
	bPlayerDrifting = bIsDrifting;
	bPlayerAirborne = bIsAirborne;
}

void UMGNearMissSubsystem::ProcessProximityCheck()
{
	if (!bSessionActive || bSessionPaused)
	{
		return;
	}

	for (auto& TargetPair : ProximityTargets)
	{
		FMGProximityTarget& Target = TargetPair.Value;

		// Update distance
		float Distance = FVector::Dist(PlayerLocation, Target.Location) - Target.BoundingRadius;
		bool bWasApproaching = Target.bIsApproaching;
		Target.bIsApproaching = Distance < Target.CurrentDistance;
		Target.CurrentDistance = FMath::Max(0.0f, Distance);

		if (Distance < Target.ClosestApproach)
		{
			Target.ClosestApproach = Distance;
		}

		// Check for near miss - target was approaching and is now moving away
		if (!Target.bNearMissTriggered && bWasApproaching && !Target.bIsApproaching)
		{
			// Check if closest approach qualifies as near miss
			FMGNearMissThresholds Thresholds = GetThresholds(Target.TargetType);

			if (Target.ClosestApproach <= Thresholds.BasicDistance)
			{
				Target.bNearMissTriggered = true;

				float PlayerSpeed = PlayerVelocity.Size() * 0.036f; // Convert to km/h
				RegisterNearMiss(Target.TargetType, Target.ClosestApproach, PlayerSpeed, Target.TargetId);
			}
		}

		// Reset trigger if target is far enough away
		if (Target.bNearMissTriggered && Distance > GetThresholds(Target.TargetType).BasicDistance * 2.0f)
		{
			Target.bNearMissTriggered = false;
			Target.ClosestApproach = FLT_MAX;
		}
	}
}

void UMGNearMissSubsystem::ClearAllTargets()
{
	ProximityTargets.Empty();
}

FMGNearMissEvent UMGNearMissSubsystem::RegisterNearMiss(EMGNearMissType MissType, float Distance, float Speed, const FString& TargetId)
{
	FMGNearMissEvent Event;
	Event.EventId = FGuid::NewGuid().ToString();
	Event.MissType = MissType;
	Event.Quality = CalculateQuality(MissType, Distance);
	Event.Distance = Distance;
	Event.Speed = Speed;
	Event.BasePoints = CalculateBasePoints(MissType, Event.Quality, Speed);
	Event.Location = PlayerLocation;
	Event.Timestamp = FDateTime::Now();
	Event.TargetId = TargetId;
	Event.bWasDrifting = bPlayerDrifting;
	Event.bWasAirborne = bPlayerAirborne;
	Event.bWasOncoming = (MissType == EMGNearMissType::Oncoming);

	// Calculate relative speed for oncoming
	if (const FMGProximityTarget* Target = ProximityTargets.Find(TargetId))
	{
		Event.RelativeSpeed = (PlayerVelocity - Target->Velocity).Size() * 0.036f;
	}

	// Apply style bonuses
	ApplyStyleBonuses(Event);

	// Apply combo multiplier
	Event.ComboMultiplier = GetComboMultiplier();
	Event.MultipliedPoints = FMath::RoundToInt(Event.BasePoints * Event.ComboMultiplier);

	// Extend combo
	ExtendCombo(Event);

	// Update session stats
	SessionStats.TotalNearMisses++;
	SessionStats.TotalStylePoints += Event.MultipliedPoints;

	if (Event.MultipliedPoints > SessionStats.BestSingleEvent)
	{
		SessionStats.BestSingleEvent = Event.MultipliedPoints;
	}

	if (Distance < SessionStats.ClosestDistance)
	{
		SessionStats.ClosestDistance = Distance;
	}

	// Track by type
	int32& TypeCount = SessionStats.NearMissByType.FindOrAdd(MissType);
	TypeCount++;

	// Track by quality
	int32& QualityCount = SessionStats.NearMissByQuality.FindOrAdd(Event.Quality);
	QualityCount++;

	// Track special conditions
	if (Event.Quality == EMGNearMissQuality::Perfect)
	{
		SessionStats.PerfectMisses++;
	}
	if (Event.Quality == EMGNearMissQuality::Insane)
	{
		SessionStats.InsaneMisses++;
	}
	if (Event.bWasOncoming)
	{
		SessionStats.OncomingMisses++;
	}
	if (Event.bWasDrifting)
	{
		SessionStats.DriftNearMisses++;
	}
	if (Event.bWasAirborne)
	{
		SessionStats.AirNearMisses++;
	}

	// Store recent event
	RecentEvents.Insert(Event, 0);
	if (RecentEvents.Num() > MaxRecentEvents)
	{
		RecentEvents.SetNum(MaxRecentEvents);
	}

	OnNearMissOccurred.Broadcast(Event, SessionStats.TotalStylePoints);

	return Event;
}

EMGNearMissQuality UMGNearMissSubsystem::CalculateQuality(EMGNearMissType MissType, float Distance) const
{
	FMGNearMissThresholds Thresholds = GetThresholds(MissType);

	if (Distance <= Thresholds.InsaneDistance)
	{
		return EMGNearMissQuality::Insane;
	}
	if (Distance <= Thresholds.PerfectDistance)
	{
		return EMGNearMissQuality::Perfect;
	}
	if (Distance <= Thresholds.GreatDistance)
	{
		return EMGNearMissQuality::Great;
	}
	if (Distance <= Thresholds.GoodDistance)
	{
		return EMGNearMissQuality::Good;
	}
	return EMGNearMissQuality::Basic;
}

int32 UMGNearMissSubsystem::CalculateBasePoints(EMGNearMissType MissType, EMGNearMissQuality Quality, float Speed) const
{
	FMGNearMissThresholds Thresholds = GetThresholds(MissType);

	float Points = static_cast<float>(Thresholds.BasePoints);

	// Apply quality multiplier
	switch (Quality)
	{
		case EMGNearMissQuality::Insane:
			Points *= Thresholds.InsaneMultiplier;
			break;
		case EMGNearMissQuality::Perfect:
			Points *= Thresholds.PerfectMultiplier;
			break;
		case EMGNearMissQuality::Great:
			Points *= Thresholds.GreatMultiplier;
			break;
		case EMGNearMissQuality::Good:
			Points *= Thresholds.GoodMultiplier;
			break;
		default:
			break;
	}

	// Speed bonus (1% per 10 km/h above 100)
	if (Speed > 100.0f)
	{
		float SpeedBonus = (Speed - 100.0f) / 10.0f * 0.01f;
		Points *= (1.0f + SpeedBonus);
	}

	return FMath::RoundToInt(Points);
}

void UMGNearMissSubsystem::ExtendCombo(const FMGNearMissEvent& Event)
{
	ActiveCombo.ComboCount++;
	ActiveCombo.TotalPoints += Event.MultipliedPoints;
	ActiveCombo.CurrentMultiplier = CalculateComboMultiplier(ActiveCombo.ComboCount);
	ActiveCombo.TimeRemaining = ActiveCombo.MaxTime;
	ActiveCombo.ComboEvents.Add(Event);

	if (ActiveCombo.ComboCount > ActiveCombo.MaxComboReached)
	{
		ActiveCombo.MaxComboReached = ActiveCombo.ComboCount;
	}

	// Track variety
	if (Event.bWasDrifting)
	{
		ActiveCombo.bHasDrift = true;
	}
	if (Event.bWasAirborne)
	{
		ActiveCombo.bHasAir = true;
	}
	if (Event.bWasOncoming)
	{
		ActiveCombo.bHasOncoming = true;
	}

	// Update state
	if (ActiveCombo.State == EMGComboState::Inactive)
	{
		ActiveCombo.State = EMGComboState::Building;

		// Start combo timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ComboTickTimer, FTimerDelegate::CreateUObject(this, &UMGNearMissSubsystem::TickCombo, 0.1f), 0.1f, true);
		}
	}

	CheckFrenzyState();
	CheckMilestones();

	OnComboUpdated.Broadcast(ActiveCombo.ComboCount, ActiveCombo.CurrentMultiplier, ActiveCombo.TotalPoints);
}

void UMGNearMissSubsystem::BankCombo()
{
	if (ActiveCombo.State == EMGComboState::Inactive)
	{
		return;
	}

	int32 FinalCombo = ActiveCombo.ComboCount;
	int32 FinalPoints = ActiveCombo.TotalPoints;

	if (FinalCombo > SessionStats.BestCombo)
	{
		SessionStats.BestCombo = FinalCombo;
	}

	ActiveCombo.State = EMGComboState::Banked;
	OnComboBanked.Broadcast(FinalCombo, FinalPoints);

	ResetCombo();
}

void UMGNearMissSubsystem::LoseCombo()
{
	if (ActiveCombo.State == EMGComboState::Inactive)
	{
		return;
	}

	int32 LostPoints = ActiveCombo.TotalPoints;
	OnComboLost.Broadcast(LostPoints);

	ResetCombo();
}

void UMGNearMissSubsystem::ResetCombo()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboTickTimer);
	}

	ActiveCombo = FMGActiveCombo();
	NextMilestoneIndex = 0;
}

FMGActiveCombo UMGNearMissSubsystem::GetActiveCombo() const
{
	return ActiveCombo;
}

bool UMGNearMissSubsystem::IsComboActive() const
{
	return ActiveCombo.State != EMGComboState::Inactive && ActiveCombo.State != EMGComboState::Banked;
}

float UMGNearMissSubsystem::GetComboTimeRemaining() const
{
	return ActiveCombo.TimeRemaining;
}

float UMGNearMissSubsystem::GetComboMultiplier() const
{
	return ActiveCombo.CurrentMultiplier;
}

void UMGNearMissSubsystem::SetThresholds(EMGNearMissType MissType, const FMGNearMissThresholds& Thresholds)
{
	NearMissThresholds.Add(MissType, Thresholds);
}

FMGNearMissThresholds UMGNearMissSubsystem::GetThresholds(EMGNearMissType MissType) const
{
	if (const FMGNearMissThresholds* Thresholds = NearMissThresholds.Find(MissType))
	{
		return *Thresholds;
	}

	// Return vehicle thresholds as default
	if (const FMGNearMissThresholds* Default = NearMissThresholds.Find(EMGNearMissType::Vehicle))
	{
		return *Default;
	}

	return FMGNearMissThresholds();
}

void UMGNearMissSubsystem::RegisterStyleBonus(const FMGStyleBonus& Bonus)
{
	if (!Bonus.BonusId.IsEmpty())
	{
		RegisteredBonuses.Add(Bonus.BonusId, Bonus);
	}
}

TArray<FMGStyleBonus> UMGNearMissSubsystem::GetApplicableBonuses(const FMGNearMissEvent& Event) const
{
	TArray<FMGStyleBonus> Result;

	for (const auto& BonusPair : RegisteredBonuses)
	{
		const FMGStyleBonus& Bonus = BonusPair.Value;

		bool bApplicable = true;

		if (Bonus.bRequiresDrift && !Event.bWasDrifting)
		{
			bApplicable = false;
		}
		if (Bonus.bRequiresAirborne && !Event.bWasAirborne)
		{
			bApplicable = false;
		}
		if (Bonus.bRequiresOncoming && !Event.bWasOncoming)
		{
			bApplicable = false;
		}
		if (Bonus.MinSpeed > 0.0f && Event.Speed < Bonus.MinSpeed)
		{
			bApplicable = false;
		}
		if (Bonus.MinCombo > 0 && ActiveCombo.ComboCount < Bonus.MinCombo)
		{
			bApplicable = false;
		}

		if (bApplicable)
		{
			Result.Add(Bonus);
		}
	}

	return Result;
}

int32 UMGNearMissSubsystem::ApplyStyleBonuses(FMGNearMissEvent& Event)
{
	TArray<FMGStyleBonus> Bonuses = GetApplicableBonuses(Event);

	int32 TotalBonusPoints = 0;
	float TotalMultiplier = 1.0f;

	for (const FMGStyleBonus& Bonus : Bonuses)
	{
		TotalMultiplier *= Bonus.Multiplier;
		TotalBonusPoints += Bonus.FlatBonus;

		OnStyleBonusApplied.Broadcast(Bonus, Bonus.FlatBonus);
	}

	Event.BasePoints = FMath::RoundToInt(Event.BasePoints * TotalMultiplier) + TotalBonusPoints;
	return TotalBonusPoints;
}

void UMGNearMissSubsystem::SetRewardConfig(const FMGNearMissRewards& Config)
{
	RewardConfig = Config;
}

FMGNearMissRewards UMGNearMissSubsystem::GetRewardConfig() const
{
	return RewardConfig;
}

int32 UMGNearMissSubsystem::CalculateCashReward(int32 StylePoints) const
{
	return FMath::RoundToInt(StylePoints * RewardConfig.CashPerPoint);
}

float UMGNearMissSubsystem::CalculateReputationReward(int32 StylePoints) const
{
	return StylePoints * RewardConfig.ReputationPerPoint;
}

float UMGNearMissSubsystem::CalculateNitroBonus(const FMGNearMissEvent& Event) const
{
	if (Event.Quality >= EMGNearMissQuality::Perfect)
	{
		return RewardConfig.NitroPerPerfectMiss;
	}
	return 0.0f;
}

void UMGNearMissSubsystem::StartSession()
{
	bSessionActive = true;
	bSessionPaused = false;
	SessionStats = FMGStyleSessionStats();
	ResetCombo();
	RecentEvents.Empty();
}

void UMGNearMissSubsystem::EndSession()
{
	BankCombo();
	bSessionActive = false;
	SaveNearMissData();
}

void UMGNearMissSubsystem::PauseSession()
{
	bSessionPaused = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().PauseTimer(ComboTickTimer);
	}
}

void UMGNearMissSubsystem::ResumeSession()
{
	bSessionPaused = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().UnPauseTimer(ComboTickTimer);
	}
}

bool UMGNearMissSubsystem::IsSessionActive() const
{
	return bSessionActive && !bSessionPaused;
}

FMGStyleSessionStats UMGNearMissSubsystem::GetSessionStats() const
{
	return SessionStats;
}

int32 UMGNearMissSubsystem::GetTotalStylePoints() const
{
	return SessionStats.TotalStylePoints;
}

int32 UMGNearMissSubsystem::GetTotalNearMisses() const
{
	return SessionStats.TotalNearMisses;
}

int32 UMGNearMissSubsystem::GetBestCombo() const
{
	return SessionStats.BestCombo;
}

float UMGNearMissSubsystem::GetClosestNearMiss() const
{
	return SessionStats.ClosestDistance;
}

TArray<FMGNearMissEvent> UMGNearMissSubsystem::GetRecentEvents(int32 Count) const
{
	TArray<FMGNearMissEvent> Result;

	int32 NumToReturn = FMath::Min(Count, RecentEvents.Num());
	for (int32 i = 0; i < NumToReturn; ++i)
	{
		Result.Add(RecentEvents[i]);
	}

	return Result;
}

FText UMGNearMissSubsystem::GetQualityDisplayText(EMGNearMissQuality Quality) const
{
	switch (Quality)
	{
		case EMGNearMissQuality::Basic: return FText::FromString(TEXT("CLOSE!"));
		case EMGNearMissQuality::Good: return FText::FromString(TEXT("VERY CLOSE!"));
		case EMGNearMissQuality::Great: return FText::FromString(TEXT("DANGEROUSLY CLOSE!"));
		case EMGNearMissQuality::Perfect: return FText::FromString(TEXT("HAIR'S BREADTH!"));
		case EMGNearMissQuality::Insane: return FText::FromString(TEXT("IMPOSSIBLE!"));
		default: return FText::FromString(TEXT("NEAR MISS"));
	}
}

FText UMGNearMissSubsystem::GetMissTypeDisplayText(EMGNearMissType MissType) const
{
	switch (MissType)
	{
		case EMGNearMissType::Vehicle: return FText::FromString(TEXT("Vehicle"));
		case EMGNearMissType::Traffic: return FText::FromString(TEXT("Traffic"));
		case EMGNearMissType::Oncoming: return FText::FromString(TEXT("Oncoming!"));
		case EMGNearMissType::Pedestrian: return FText::FromString(TEXT("Close Call"));
		case EMGNearMissType::Obstacle: return FText::FromString(TEXT("Obstacle"));
		case EMGNearMissType::Wall: return FText::FromString(TEXT("Wall Graze"));
		case EMGNearMissType::Barrier: return FText::FromString(TEXT("Barrier Scrape"));
		case EMGNearMissType::Cliff: return FText::FromString(TEXT("Cliff Edge"));
		case EMGNearMissType::Train: return FText::FromString(TEXT("Train Dodge!"));
		case EMGNearMissType::Police: return FText::FromString(TEXT("Police Evade"));
		default: return FText::FromString(TEXT("Near Miss"));
	}
}

FLinearColor UMGNearMissSubsystem::GetQualityColor(EMGNearMissQuality Quality) const
{
	switch (Quality)
	{
		case EMGNearMissQuality::Basic: return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		case EMGNearMissQuality::Good: return FLinearColor(0.0f, 1.0f, 0.5f, 1.0f);
		case EMGNearMissQuality::Great: return FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
		case EMGNearMissQuality::Perfect: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		case EMGNearMissQuality::Insane: return FLinearColor(1.0f, 0.0f, 0.5f, 1.0f);
		default: return FLinearColor::White;
	}
}

void UMGNearMissSubsystem::SaveNearMissData()
{
	// Save is handled centrally by MGSaveManagerSubsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			SaveManager->QuickSave();
		}
	}
}

void UMGNearMissSubsystem::LoadNearMissData()
{
	// Load near miss data from central save manager
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			if (const UMGSaveGame* SaveData = SaveManager->GetCurrentSaveData())
			{
				// Restore career stats
				SessionStats.TotalNearMisses = SaveData->NearMissData.TotalNearMisses;
				SessionStats.TotalStylePoints = SaveData->NearMissData.TotalNearMissScore;
				SessionStats.BestCombo = SaveData->NearMissData.NearMissChainMax;
				SessionStats.ClosestDistance = SaveData->NearMissData.ClosestNearMissDistance;
				UE_LOG(LogTemp, Log, TEXT("NearMissSubsystem: Loaded near miss data - Total: %d, Score: %lld"),
					SaveData->NearMissData.TotalNearMisses, SaveData->NearMissData.TotalNearMissScore);
			}
		}
	}
}

void UMGNearMissSubsystem::TickCombo(float DeltaTime)
{
	if (!IsComboActive())
	{
		return;
	}

	ActiveCombo.TimeRemaining -= DeltaTime;

	if (ActiveCombo.TimeRemaining <= 0.5f && ActiveCombo.State == EMGComboState::Active)
	{
		ActiveCombo.State = EMGComboState::Expiring;
	}

	if (ActiveCombo.TimeRemaining <= 0.0f)
	{
		BankCombo();
	}
}

void UMGNearMissSubsystem::CheckMilestones()
{
	if (NextMilestoneIndex >= RewardConfig.ComboMilestones.Num())
	{
		return;
	}

	int32 NextMilestone = RewardConfig.ComboMilestones[NextMilestoneIndex];

	if (ActiveCombo.ComboCount >= NextMilestone)
	{
		int32 BonusPoints = RewardConfig.ComboMilestoneBonus * (NextMilestoneIndex + 1);
		ActiveCombo.TotalPoints += BonusPoints;
		SessionStats.TotalStylePoints += BonusPoints;

		OnMilestoneReached.Broadcast(NextMilestone, BonusPoints);

		NextMilestoneIndex++;
	}
}

void UMGNearMissSubsystem::CheckFrenzyState()
{
	if (ActiveCombo.ComboCount >= 25 && ActiveCombo.State != EMGComboState::Frenzy)
	{
		ActiveCombo.State = EMGComboState::Frenzy;
		OnFrenzyActivated.Broadcast(ActiveCombo.ComboCount, ActiveCombo.CurrentMultiplier);
	}
	else if (ActiveCombo.ComboCount >= 5 && ActiveCombo.State == EMGComboState::Building)
	{
		ActiveCombo.State = EMGComboState::Active;
	}
}

float UMGNearMissSubsystem::CalculateComboMultiplier(int32 ComboCount) const
{
	if (ComboCount <= 1)
	{
		return 1.0f;
	}

	// Progressive multiplier
	// 2-5: 1.1-1.4
	// 6-10: 1.5-2.0
	// 11-25: 2.0-3.0
	// 26-50: 3.0-4.0
	// 51+: 4.0-5.0 (capped)

	if (ComboCount <= 5)
	{
		return 1.0f + (ComboCount - 1) * 0.1f;
	}
	if (ComboCount <= 10)
	{
		return 1.4f + (ComboCount - 5) * 0.12f;
	}
	if (ComboCount <= 25)
	{
		return 2.0f + (ComboCount - 10) * 0.067f;
	}
	if (ComboCount <= 50)
	{
		return 3.0f + (ComboCount - 25) * 0.04f;
	}

	return FMath::Min(5.0f, 4.0f + (ComboCount - 50) * 0.02f);
}
