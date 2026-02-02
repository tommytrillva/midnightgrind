// Copyright Midnight Grind. All Rights Reserved.

#include "Scoring/MGDriftComboSystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleMovementComponent.h"

UMGDriftComboSystem::UMGDriftComboSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Default combo tiers
	ComboTiers.Add({ FText::FromString(TEXT("DRIFT")), 1, 1.0f, FLinearColor::White });
	ComboTiers.Add({ FText::FromString(TEXT("NICE")), 2, 1.5f, FLinearColor(0.0f, 1.0f, 0.5f) });
	ComboTiers.Add({ FText::FromString(TEXT("GREAT")), 4, 2.0f, FLinearColor(1.0f, 1.0f, 0.0f) });
	ComboTiers.Add({ FText::FromString(TEXT("AWESOME")), 7, 3.0f, FLinearColor(1.0f, 0.5f, 0.0f) });
	ComboTiers.Add({ FText::FromString(TEXT("INSANE")), 10, 4.0f, FLinearColor(1.0f, 0.0f, 0.5f) });
	ComboTiers.Add({ FText::FromString(TEXT("LEGENDARY")), 15, 5.0f, FLinearColor(0.0f, 1.0f, 1.0f) });

	// Default style bonuses
	StyleBonuses.Add({ EMGDriftStyleBonus::Marathon, FText::FromString(TEXT("MARATHON")), 500, 0.2f });
	StyleBonuses.Add({ EMGDriftStyleBonus::Extreme, FText::FromString(TEXT("EXTREME")), 300, 0.15f });
	StyleBonuses.Add({ EMGDriftStyleBonus::NearMiss, FText::FromString(TEXT("NEAR MISS")), 200, 0.1f });
	StyleBonuses.Add({ EMGDriftStyleBonus::Overtake, FText::FromString(TEXT("DRIFT PASS")), 400, 0.2f });
	StyleBonuses.Add({ EMGDriftStyleBonus::ChainLink, FText::FromString(TEXT("CHAIN")), 150, 0.05f });
	StyleBonuses.Add({ EMGDriftStyleBonus::Transition, FText::FromString(TEXT("TRANSITION")), 350, 0.15f });
	StyleBonuses.Add({ EMGDriftStyleBonus::HighSpeed, FText::FromString(TEXT("HIGH SPEED")), 250, 0.1f });
	StyleBonuses.Add({ EMGDriftStyleBonus::Perfect, FText::FromString(TEXT("PERFECT")), 600, 0.25f });
	StyleBonuses.Add({ EMGDriftStyleBonus::Checkpoint, FText::FromString(TEXT("CHECKPOINT")), 200, 0.1f });
}

void UMGDriftComboSystem::BeginPlay()
{
	Super::BeginPlay();

	// Cache references
	VehiclePawn = Cast<AMGVehiclePawn>(GetOwner());
	if (VehiclePawn.IsValid())
	{
		MovementComponent = VehiclePawn->GetVehicleMovementComponent();
	}
}

void UMGDriftComboSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MovementComponent.IsValid())
	{
		return;
	}

	// Get current drift state from movement
	FMGDriftState DriftState = MovementComponent->GetDriftState();
	float CurrentAngle = FMath::Abs(DriftState.DriftAngle);
	float CurrentSpeed = MovementComponent->GetForwardSpeed() * 0.036f; // cm/s to km/h
	bool bIsLeftDrift = DriftState.DriftAngle < 0;

	// Check if currently drifting (meets minimum requirements)
	bool bMeetsDriftCriteria = CurrentAngle >= MinDriftAngle && CurrentSpeed >= MinDriftSpeed;

	// State machine
	switch (CurrentState)
	{
	case EMGDriftComboState::Idle:
		if (bMeetsDriftCriteria)
		{
			StartDrift(CurrentAngle, bIsLeftDrift);
		}
		TimeSinceLastDrift += DeltaTime;
		break;

	case EMGDriftComboState::Drifting:
		if (bMeetsDriftCriteria)
		{
			UpdateDriftScoring(DeltaTime, CurrentAngle, CurrentSpeed);
		}
		else
		{
			EndDrift();
		}
		break;

	case EMGDriftComboState::Grace:
		GraceTimer -= DeltaTime;
		if (bMeetsDriftCriteria)
		{
			// Resume drifting - continue combo
			StartDrift(CurrentAngle, bIsLeftDrift);
		}
		else if (GraceTimer <= 0.0f)
		{
			// Grace period expired - bank score
			BankComboScore();
			CurrentState = EMGDriftComboState::Idle;
			ComboCount = 0;
			CurrentMultiplier = 1.0f;
			EarnedBonusesThisCombo.Empty();
		}
		break;

	case EMGDriftComboState::Failed:
		// Wait a moment before returning to idle
		GraceTimer -= DeltaTime;
		if (GraceTimer <= 0.0f)
		{
			CurrentState = EMGDriftComboState::Idle;
		}
		break;
	}
}

// ==========================================
// STATE QUERIES
// ==========================================

FMGComboTier UMGDriftComboSystem::GetCurrentTier() const
{
	if (CurrentTierIndex >= 0 && CurrentTierIndex < ComboTiers.Num())
	{
		return ComboTiers[CurrentTierIndex];
	}

	// Return default
	FMGComboTier Default;
	Default.TierName = FText::FromString(TEXT("DRIFT"));
	Default.MinComboCount = 0;
	Default.Multiplier = 1.0f;
	Default.TierColor = FLinearColor::White;
	return Default;
}

// ==========================================
// ACTIONS
// ==========================================

void UMGDriftComboSystem::AwardStyleBonus(EMGDriftStyleBonus BonusType)
{
	if (BonusType == EMGDriftStyleBonus::None)
	{
		return;
	}

	// Check if already earned this combo
	if (EarnedBonusesThisCombo.Contains(BonusType))
	{
		return;
	}

	const FMGStyleBonusConfig* Config = GetStyleBonusConfig(BonusType);
	if (!Config)
	{
		return;
	}

	EarnedBonusesThisCombo.Add(BonusType);

	// Add bonus points
	CurrentComboScore += Config->BonusPoints * CurrentMultiplier;

	// Add multiplier bonus
	CurrentMultiplier += Config->MultiplierBonus;

	OnStyleBonusEarned.Broadcast(BonusType, Config->BonusPoints);
	OnComboUpdated.Broadcast(ComboCount, CurrentMultiplier, CurrentComboScore);
}

void UMGDriftComboSystem::NotifyNearMiss()
{
	if (IsDrifting())
	{
		AwardStyleBonus(EMGDriftStyleBonus::NearMiss);
	}
}

void UMGDriftComboSystem::NotifyOvertake()
{
	if (IsDrifting())
	{
		AwardStyleBonus(EMGDriftStyleBonus::Overtake);
	}
}

void UMGDriftComboSystem::NotifyCheckpointCrossed()
{
	if (IsDrifting())
	{
		AwardStyleBonus(EMGDriftStyleBonus::Checkpoint);
	}
}

void UMGDriftComboSystem::DropCombo()
{
	if (CurrentState == EMGDriftComboState::Idle)
	{
		return;
	}

	float LostScore = CurrentComboScore;

	// Reset combo state
	CurrentState = EMGDriftComboState::Failed;
	GraceTimer = 0.5f; // Short delay before can start new combo
	CurrentComboScore = 0.0f;
	ComboCount = 0;
	CurrentMultiplier = 1.0f;
	CurrentTierIndex = 0;
	EarnedBonusesThisCombo.Empty();

	OnComboDropped.Broadcast(LostScore);
}

void UMGDriftComboSystem::ResetScore()
{
	CurrentState = EMGDriftComboState::Idle;
	CurrentComboScore = 0.0f;
	TotalBankedScore = 0.0f;
	ComboCount = 0;
	CurrentMultiplier = 1.0f;
	CurrentTierIndex = 0;
	GraceTimer = 0.0f;
	CurrentDrift = FMGDriftData();
	AngleSamples.Empty();
	SpeedSamples.Empty();
	EarnedBonusesThisCombo.Empty();
}

float UMGDriftComboSystem::BankComboScore()
{
	float BankedAmount = CurrentComboScore;
	TotalBankedScore += BankedAmount;
	CurrentComboScore = 0.0f;

	return BankedAmount;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGDriftComboSystem::StartDrift(float InitialAngle, bool bIsLeft)
{
	// Check for transition bonus (quick direction change)
	if (CurrentState == EMGDriftComboState::Grace && TimeSinceLastDrift < TransitionTimeWindow)
	{
		if (bLastDriftWasLeft != bIsLeft)
		{
			AwardStyleBonus(EMGDriftStyleBonus::Transition);
		}
	}

	// If resuming from grace, this is a chain
	if (CurrentState == EMGDriftComboState::Grace)
	{
		AwardStyleBonus(EMGDriftStyleBonus::ChainLink);
	}

	CurrentState = EMGDriftComboState::Drifting;

	// Initialize drift data
	CurrentDrift = FMGDriftData();
	CurrentDrift.bWasLeftDrift = bIsLeft;
	AngleSamples.Empty();
	SpeedSamples.Empty();

	ComboCount++;
	CurrentMultiplier = CalculateMultiplier();
	CheckTierAdvancement();

	OnDriftStarted.Broadcast(InitialAngle, bIsLeft);
	OnComboUpdated.Broadcast(ComboCount, CurrentMultiplier, CurrentComboScore);
}

void UMGDriftComboSystem::EndDrift()
{
	// Finalize drift data
	if (AngleSamples.Num() > 0)
	{
		float AngleSum = 0.0f;
		for (float Angle : AngleSamples)
		{
			AngleSum += Angle;
		}
		CurrentDrift.AverageAngle = AngleSum / AngleSamples.Num();
	}

	if (SpeedSamples.Num() > 0)
	{
		float SpeedSum = 0.0f;
		for (float Speed : SpeedSamples)
		{
			SpeedSum += Speed;
		}
		CurrentDrift.AverageSpeed = SpeedSum / SpeedSamples.Num();
	}

	// Calculate score for this drift
	CurrentDrift.BaseScore = CalculateDriftScore(CurrentDrift);
	float ScoreEarned = CurrentDrift.BaseScore * CurrentMultiplier;
	CurrentComboScore += ScoreEarned;

	// Check for style bonuses
	CheckStyleBonuses(CurrentDrift);

	// Enter grace period
	CurrentState = EMGDriftComboState::Grace;
	GraceTimer = ComboGracePeriod;
	TimeSinceLastDrift = 0.0f;
	bLastDriftWasLeft = CurrentDrift.bWasLeftDrift;

	OnDriftEnded.Broadcast(CurrentDrift, ScoreEarned);
	OnComboUpdated.Broadcast(ComboCount, CurrentMultiplier, CurrentComboScore);
}

void UMGDriftComboSystem::UpdateDriftScoring(float DeltaTime, float CurrentAngle, float CurrentSpeed)
{
	CurrentDrift.Duration += DeltaTime;

	// Track max values
	if (CurrentAngle > CurrentDrift.MaxAngle)
	{
		CurrentDrift.MaxAngle = CurrentAngle;
	}
	if (CurrentSpeed > CurrentDrift.MaxSpeed)
	{
		CurrentDrift.MaxSpeed = CurrentSpeed;
	}

	// Calculate distance
	CurrentDrift.Distance += CurrentSpeed * 0.277778f * DeltaTime; // km/h to m/s * time

	// Store samples for averaging
	AngleSamples.Add(CurrentAngle);
	SpeedSamples.Add(CurrentSpeed);

	// Cap samples to prevent memory bloat
	if (AngleSamples.Num() > 1000)
	{
		AngleSamples.RemoveAt(0, 500);
		SpeedSamples.RemoveAt(0, 500);
	}

	// Calculate running score
	float RunningScore = CalculateDriftScore(CurrentDrift) * CurrentMultiplier;

	// Broadcast update
	OnComboUpdated.Broadcast(ComboCount, CurrentMultiplier, CurrentComboScore + RunningScore);
}

float UMGDriftComboSystem::CalculateDriftScore(const FMGDriftData& DriftData) const
{
	// Base score from duration
	float Score = DriftData.Duration * BasePointsPerSecond;

	// Angle bonus
	float AngleBonus = 1.0f + (DriftData.AverageAngle * AngleMultiplier);
	Score *= AngleBonus;

	// Speed bonus
	float SpeedBonus = 1.0f + ((DriftData.AverageSpeed / 10.0f) * SpeedMultiplier);
	Score *= SpeedBonus;

	return Score;
}

float UMGDriftComboSystem::CalculateMultiplier() const
{
	float Multiplier = 1.0f;

	// Find applicable tier
	for (int32 i = ComboTiers.Num() - 1; i >= 0; --i)
	{
		if (ComboCount >= ComboTiers[i].MinComboCount)
		{
			Multiplier = ComboTiers[i].Multiplier;
			break;
		}
	}

	return Multiplier;
}

void UMGDriftComboSystem::CheckStyleBonuses(const FMGDriftData& DriftData)
{
	// Marathon - long drift
	if (DriftData.Duration >= MarathonDriftThreshold)
	{
		AwardStyleBonus(EMGDriftStyleBonus::Marathon);
	}

	// Extreme - high angle
	if (DriftData.MaxAngle >= ExtremeAngleThreshold)
	{
		AwardStyleBonus(EMGDriftStyleBonus::Extreme);
	}

	// High Speed
	if (DriftData.MaxSpeed >= HighSpeedThreshold)
	{
		AwardStyleBonus(EMGDriftStyleBonus::HighSpeed);
	}
}

const FMGStyleBonusConfig* UMGDriftComboSystem::GetStyleBonusConfig(EMGDriftStyleBonus BonusType) const
{
	for (const FMGStyleBonusConfig& Config : StyleBonuses)
	{
		if (Config.BonusType == BonusType)
		{
			return &Config;
		}
	}
	return nullptr;
}

void UMGDriftComboSystem::CheckTierAdvancement()
{
	// Find current tier based on combo count
	int32 NewTierIndex = 0;
	for (int32 i = ComboTiers.Num() - 1; i >= 0; --i)
	{
		if (ComboCount >= ComboTiers[i].MinComboCount)
		{
			NewTierIndex = i;
			break;
		}
	}

	// Check if tier advanced
	if (NewTierIndex > CurrentTierIndex)
	{
		CurrentTierIndex = NewTierIndex;
		OnComboTierReached.Broadcast(ComboTiers[CurrentTierIndex], ComboCount);
	}
}
