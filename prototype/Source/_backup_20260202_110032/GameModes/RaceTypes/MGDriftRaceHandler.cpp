// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGDriftRaceHandler.h"
#include "GameModes/MGRaceGameMode.h"
#include "Vehicle/MGVehiclePawn.h"

UMGDriftRaceHandler::UMGDriftRaceHandler()
{
	// Default scoring values tuned for arcade feel
	BasePointsPerSecond = 100.0f;
	AngleScoreMultiplier = 1.0f;
	SpeedScoreMultiplier = 0.5f;
	ChainMultiplierIncrement = 0.25f;
	MaxChainMultiplier = 5.0f;
	TandemBonusMultiplier = 1.5f;

	// Detection thresholds
	MinDriftAngle = 15.0f;
	MinDriftSpeed = 1000.0f; // ~36 km/h
	DriftEndGracePeriod = 0.5f;
	ChainWindowSeconds = 2.0f;
	TandemDistance = 1500.0f; // 15 meters
}

void UMGDriftRaceHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	Super::Initialize(InGameMode);

	if (InGameMode)
	{
		FMGRaceConfig Config = InGameMode->GetRaceConfig();
		TotalLaps = Config.LapCount;
		bIsLapBased = true; // Default to lap-based
	}
}

void UMGDriftRaceHandler::Reset()
{
	Super::Reset();

	ActiveDrifts.Empty();
	TotalScores.Empty();
	BestDriftScores.Empty();
}

void UMGDriftRaceHandler::OnRaceStarted()
{
	Super::OnRaceStarted();

	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		TArray<FMGRacerData> Racers = GM->GetAllRacers();
		for (const FMGRacerData& Racer : Racers)
		{
			ActiveDrifts.Add(Racer.RacerIndex, FMGActiveDrift());
			TotalScores.Add(Racer.RacerIndex, 0.0f);
			BestDriftScores.Add(Racer.RacerIndex, 0.0f);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drift Race: Started - %s mode"),
		bIsLapBased ? TEXT("Lap-based") : TEXT("Target score"));
}

void UMGDriftRaceHandler::OnRaceTick(float DeltaTime)
{
	Super::OnRaceTick(DeltaTime);

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	// Process drift for all racers
	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	for (const FMGRacerData& Racer : Racers)
	{
		if (!Racer.bFinished && !Racer.bDNF)
		{
			if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
			{
				ProcessVehicleDrift(Racer.RacerIndex, Vehicle, DeltaTime);
			}
		}
	}
}

void UMGDriftRaceHandler::ProcessVehicleDrift(int32 RacerIndex, AMGVehiclePawn* Vehicle, float DeltaTime)
{
	if (!Vehicle)
	{
		return;
	}

	FMGActiveDrift* Drift = ActiveDrifts.Find(RacerIndex);
	if (!Drift)
	{
		return;
	}

	// Get current vehicle state
	float DriftAngle = GetVehicleDriftAngle(Vehicle);
	float Speed = Vehicle->GetVelocity().Size();

	// Update time since last drift (for chain tracking)
	if (Drift->State == EMGDriftState::None)
	{
		Drift->TimeSinceLastDrift += DeltaTime;
		if (Drift->TimeSinceLastDrift > ChainWindowSeconds)
		{
			// Chain broken
			Drift->ChainCount = 0;
			Drift->Multiplier = 1.0f;
		}
	}

	// Check if currently drifting
	bool bIsDrifting = (DriftAngle >= MinDriftAngle && Speed >= MinDriftSpeed);

	// State machine for drift tracking
	switch (Drift->State)
	{
	case EMGDriftState::None:
		if (bIsDrifting)
		{
			// Start new drift
			Drift->State = EMGDriftState::Building;
			Drift->DriftAngle = DriftAngle;
			Drift->Speed = Speed;
			Drift->Duration = 0.0f;
			Drift->Points = 0.0f;
			Drift->Grade = CalculateDriftGrade(DriftAngle, Speed);
			Drift->TimeSinceLastDrift = 0.0f;

			// Check for tandem
			Drift->bIsTandem = CheckTandemDrift(RacerIndex);

			// Increment chain
			Drift->ChainCount++;
			Drift->Multiplier = FMath::Min(1.0f + (Drift->ChainCount - 1) * ChainMultiplierIncrement, MaxChainMultiplier);

			OnDriftStarted.Broadcast(RacerIndex, Drift->Grade);

			if (Drift->ChainCount > 1)
			{
				OnChainIncreased.Broadcast(RacerIndex, Drift->ChainCount);
			}
		}
		break;

	case EMGDriftState::Building:
	case EMGDriftState::Sustained:
		if (bIsDrifting)
		{
			// Continue drift
			Drift->State = EMGDriftState::Sustained;
			Drift->DriftAngle = DriftAngle;
			Drift->Speed = Speed;
			Drift->Duration += DeltaTime;

			// Check grade change
			EMGDriftGrade NewGrade = CalculateDriftGrade(DriftAngle, Speed);
			if (NewGrade != Drift->Grade)
			{
				Drift->Grade = NewGrade;
				OnDriftGradeChanged.Broadcast(RacerIndex, NewGrade);
			}

			// Update tandem status
			Drift->bIsTandem = Drift->bIsTandem || CheckTandemDrift(RacerIndex);

			// Calculate points
			float FramePoints = CalculateDriftPoints(*Drift, DeltaTime);
			Drift->Points += FramePoints;
		}
		else
		{
			// Enter grace period
			Drift->State = EMGDriftState::Ending;
			Drift->Duration += DeltaTime;
		}
		break;

	case EMGDriftState::Ending:
		Drift->Duration += DeltaTime;

		if (bIsDrifting)
		{
			// Resume drift
			Drift->State = EMGDriftState::Sustained;
			Drift->DriftAngle = DriftAngle;
			Drift->Speed = Speed;
		}
		else if (Drift->Duration > DriftEndGracePeriod)
		{
			// Drift ended - finalize score
			FinalizeDrift(RacerIndex);
		}
		break;
	}
}

void UMGDriftRaceHandler::FinalizeDrift(int32 RacerIndex)
{
	FMGActiveDrift* Drift = ActiveDrifts.Find(RacerIndex);
	if (!Drift || Drift->Points <= 0.0f)
	{
		if (Drift)
		{
			Drift->Reset();
		}
		return;
	}

	// Apply multipliers
	float FinalPoints = Drift->Points * Drift->Multiplier;

	// Tandem bonus
	if (Drift->bIsTandem)
	{
		FinalPoints *= TandemBonusMultiplier;
	}

	// Create completed drift data
	FMGCompletedDrift CompletedDrift;
	CompletedDrift.RacerIndex = RacerIndex;
	CompletedDrift.TotalPoints = FinalPoints;
	CompletedDrift.Duration = Drift->Duration;
	CompletedDrift.PeakGrade = Drift->Grade;
	CompletedDrift.Multiplier = Drift->Multiplier;
	CompletedDrift.ChainCount = Drift->ChainCount;
	CompletedDrift.bWasTandem = Drift->bIsTandem;

	// Update total score
	float* TotalScore = TotalScores.Find(RacerIndex);
	if (TotalScore)
	{
		*TotalScore += FinalPoints;
	}

	// Update best drift
	float* BestDrift = BestDriftScores.Find(RacerIndex);
	if (BestDrift && FinalPoints > *BestDrift)
	{
		*BestDrift = FinalPoints;
	}

	// Broadcast score update
	BroadcastScoreUpdate(RacerIndex, FinalPoints, TotalScore ? *TotalScore : 0.0f,
		FText::Format(NSLOCTEXT("Drift", "DriftComplete", "{0} Drift!"),
			StaticEnum<EMGDriftGrade>()->GetDisplayNameTextByValue(static_cast<int64>(Drift->Grade))),
		Drift->Multiplier);

	OnDriftEnded.Broadcast(RacerIndex, CompletedDrift);

	UE_LOG(LogTemp, Log, TEXT("Drift Race: Racer %d scored %.0f points (Grade: %d, Chain: %d, Tandem: %d)"),
		RacerIndex, FinalPoints, static_cast<int32>(Drift->Grade), Drift->ChainCount, Drift->bIsTandem);

	// Reset drift state (but keep chain info)
	float TimeSinceLast = Drift->TimeSinceLastDrift;
	int32 Chain = Drift->ChainCount;
	float Mult = Drift->Multiplier;

	Drift->Reset();
	Drift->TimeSinceLastDrift = TimeSinceLast;
	Drift->ChainCount = Chain;
	Drift->Multiplier = Mult;
}

void UMGDriftRaceHandler::OnLapCompleted(int32 RacerIndex, float LapTime)
{
	Super::OnLapCompleted(RacerIndex, LapTime);

	// In lap-based drift, laps are for race structure, not win condition
	// Score is what matters
}

EMGRaceCompletionResult UMGDriftRaceHandler::CheckCompletionCondition(int32 RacerIndex)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return EMGRaceCompletionResult::InProgress;
	}

	FMGRacerData RacerData = GM->GetRacerData(RacerIndex);

	if (RacerData.bFinished)
	{
		return EMGRaceCompletionResult::Finished;
	}

	if (RacerData.bDNF)
	{
		return EMGRaceCompletionResult::DNF;
	}

	// Check completion based on mode
	if (bIsLapBased)
	{
		// Lap-based: finish when all laps complete
		if (RacerData.CurrentLap > TotalLaps)
		{
			return EMGRaceCompletionResult::Finished;
		}
	}
	else
	{
		// Score-based: finish when target reached
		float* Score = TotalScores.Find(RacerIndex);
		if (Score && *Score >= TargetScore)
		{
			return EMGRaceCompletionResult::Finished;
		}
	}

	// Check time limit
	FMGRaceConfig Config = GM->GetRaceConfig();
	if (Config.TimeLimit > 0.0f && GM->GetRaceTime() >= Config.TimeLimit)
	{
		return EMGRaceCompletionResult::Finished; // Time up, but not DNF in drift
	}

	return EMGRaceCompletionResult::InProgress;
}

void UMGDriftRaceHandler::CalculatePositions(TArray<int32>& OutPositions)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	OutPositions.SetNum(Racers.Num());

	// Sort by score (descending)
	TArray<TPair<int32, float>> RacerRanking;
	for (const FMGRacerData& Racer : Racers)
	{
		float Score = GetTotalScore(Racer.RacerIndex);

		// Add current drift points being accumulated
		const FMGActiveDrift* Drift = ActiveDrifts.Find(Racer.RacerIndex);
		if (Drift && Drift->State != EMGDriftState::None)
		{
			Score += Drift->Points * Drift->Multiplier;
		}

		RacerRanking.Add(TPair<int32, float>(Racer.RacerIndex, Score));
	}

	// Sort descending (higher score = better position)
	RacerRanking.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	for (int32 i = 0; i < RacerRanking.Num(); ++i)
	{
		OutPositions[RacerRanking[i].Key] = i + 1;
	}
}

float UMGDriftRaceHandler::GetRacerScore(int32 RacerIndex) const
{
	return GetTotalScore(RacerIndex);
}

FText UMGDriftRaceHandler::GetDisplayName() const
{
	return NSLOCTEXT("RaceType", "DriftName", "Drift Battle");
}

FText UMGDriftRaceHandler::GetDescription() const
{
	return NSLOCTEXT("RaceType", "DriftDesc", "Score points by drifting! Chain drifts for multipliers. Highest score wins!");
}

FText UMGDriftRaceHandler::GetProgressFormat() const
{
	return NSLOCTEXT("RaceType", "DriftProgress", "SCORE: {0}");
}

int64 UMGDriftRaceHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	int64 BaseCredits = Super::CalculateCreditsForPosition(Position, TotalRacers);

	// Drift races reward high scores with bonus
	// Winner typically has multiplier
	if (Position == 1)
	{
		BaseCredits = static_cast<int64>(BaseCredits * 1.2f);
	}

	return BaseCredits;
}

FMGActiveDrift UMGDriftRaceHandler::GetActiveDrift(int32 RacerIndex) const
{
	const FMGActiveDrift* Drift = ActiveDrifts.Find(RacerIndex);
	return Drift ? *Drift : FMGActiveDrift();
}

bool UMGDriftRaceHandler::IsDrifting(int32 RacerIndex) const
{
	const FMGActiveDrift* Drift = ActiveDrifts.Find(RacerIndex);
	return Drift && Drift->State != EMGDriftState::None;
}

float UMGDriftRaceHandler::GetTotalScore(int32 RacerIndex) const
{
	const float* Score = TotalScores.Find(RacerIndex);
	return Score ? *Score : 0.0f;
}

float UMGDriftRaceHandler::GetCurrentMultiplier(int32 RacerIndex) const
{
	const FMGActiveDrift* Drift = ActiveDrifts.Find(RacerIndex);
	return Drift ? Drift->Multiplier : 1.0f;
}

int32 UMGDriftRaceHandler::GetChainCount(int32 RacerIndex) const
{
	const FMGActiveDrift* Drift = ActiveDrifts.Find(RacerIndex);
	return Drift ? Drift->ChainCount : 0;
}

float UMGDriftRaceHandler::GetBestDriftScore(int32 RacerIndex) const
{
	const float* Score = BestDriftScores.Find(RacerIndex);
	return Score ? *Score : 0.0f;
}

EMGDriftGrade UMGDriftRaceHandler::CalculateDriftGrade(float Angle, float Speed) const
{
	if (Angle < MinDriftAngle)
	{
		return EMGDriftGrade::None;
	}

	// Speed factor (bonus for high speed)
	float SpeedFactor = FMath::Clamp(Speed / 5000.0f, 0.0f, 1.0f); // Max at ~180 km/h

	// Combined score for grading
	float GradeScore = Angle + (SpeedFactor * 30.0f); // Speed can add up to 30 degrees equivalent

	if (GradeScore >= 105.0f)
	{
		return EMGDriftGrade::SS;
	}
	else if (GradeScore >= 90.0f)
	{
		return EMGDriftGrade::S;
	}
	else if (GradeScore >= 75.0f)
	{
		return EMGDriftGrade::A;
	}
	else if (GradeScore >= 60.0f)
	{
		return EMGDriftGrade::B;
	}
	else if (GradeScore >= 45.0f)
	{
		return EMGDriftGrade::C;
	}
	else if (GradeScore >= 30.0f)
	{
		return EMGDriftGrade::D;
	}

	return EMGDriftGrade::None;
}

float UMGDriftRaceHandler::CalculateDriftPoints(const FMGActiveDrift& Drift, float DeltaTime) const
{
	float BasePoints = BasePointsPerSecond * DeltaTime;

	// Angle bonus
	float AngleBonus = 1.0f + (Drift.DriftAngle / 90.0f) * AngleScoreMultiplier;

	// Speed bonus
	float SpeedBonus = 1.0f + (Drift.Speed / 5000.0f) * SpeedScoreMultiplier;

	// Grade multiplier
	float GradeMultiplier = 1.0f;
	switch (Drift.Grade)
	{
	case EMGDriftGrade::D:  GradeMultiplier = 1.0f; break;
	case EMGDriftGrade::C:  GradeMultiplier = 1.25f; break;
	case EMGDriftGrade::B:  GradeMultiplier = 1.5f; break;
	case EMGDriftGrade::A:  GradeMultiplier = 2.0f; break;
	case EMGDriftGrade::S:  GradeMultiplier = 3.0f; break;
	case EMGDriftGrade::SS: GradeMultiplier = 4.0f; break;
	default: break;
	}

	return BasePoints * AngleBonus * SpeedBonus * GradeMultiplier;
}

bool UMGDriftRaceHandler::CheckTandemDrift(int32 RacerIndex) const
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return false;
	}

	// Get this racer's position
	FMGRacerData ThisRacer = GM->GetRacerData(RacerIndex);
	if (!ThisRacer.Vehicle.IsValid())
	{
		return false;
	}

	FVector ThisLocation = ThisRacer.Vehicle->GetActorLocation();

	// Check all other racers
	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	for (const FMGRacerData& OtherRacer : Racers)
	{
		if (OtherRacer.RacerIndex == RacerIndex)
		{
			continue;
		}

		// Check if other racer is drifting
		const FMGActiveDrift* OtherDrift = ActiveDrifts.Find(OtherRacer.RacerIndex);
		if (!OtherDrift || OtherDrift->State == EMGDriftState::None)
		{
			continue;
		}

		// Check distance
		if (OtherRacer.Vehicle.IsValid())
		{
			float Distance = FVector::Dist(ThisLocation, OtherRacer.Vehicle->GetActorLocation());
			if (Distance <= TandemDistance)
			{
				return true;
			}
		}
	}

	return false;
}

float UMGDriftRaceHandler::GetVehicleDriftAngle(AMGVehiclePawn* Vehicle) const
{
	if (!Vehicle)
	{
		return 0.0f;
	}

	// Get velocity direction vs facing direction
	FVector Velocity = Vehicle->GetVelocity();
	if (Velocity.IsNearlyZero())
	{
		return 0.0f;
	}

	FVector ForwardVector = Vehicle->GetActorForwardVector();
	FVector VelocityDir = Velocity.GetSafeNormal();

	// Calculate angle between velocity and facing direction
	float DotProduct = FVector::DotProduct(ForwardVector, VelocityDir);
	float AngleRad = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));
	float AngleDeg = FMath::RadiansToDegrees(AngleRad);

	return AngleDeg;
}
