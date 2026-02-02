// Copyright Midnight Grind. All Rights Reserved.

#include "Scoring/MGScoringSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGScoringSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Register default score event types
	{
		FMGScoreEventDefinition Drift;
		Drift.Type = EMGScoreEventType::Drift;
		Drift.DisplayName = FText::FromString(TEXT("DRIFT"));
		Drift.Category = EMGScoreCategory::Style;
		Drift.BasePoints = 100;
		Drift.MinPoints = 10;
		Drift.MaxPoints = 5000;
		Drift.ChainBonusPerEvent = 0.15f;
		Drift.MaxChainBonus = 3.0f;
		Drift.bCanChain = true;
		Drift.ChainTimeExtension = 2.0f;
		Drift.DisplayColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
		RegisterScoreEventType(Drift);
	}
	{
		FMGScoreEventDefinition NearMiss;
		NearMiss.Type = EMGScoreEventType::NearMiss;
		NearMiss.DisplayName = FText::FromString(TEXT("NEAR MISS"));
		NearMiss.Category = EMGScoreCategory::Style;
		NearMiss.BasePoints = 50;
		NearMiss.MinPoints = 25;
		NearMiss.MaxPoints = 500;
		NearMiss.ChainBonusPerEvent = 0.2f;
		NearMiss.MaxChainBonus = 4.0f;
		NearMiss.bCanChain = true;
		NearMiss.ChainTimeExtension = 1.5f;
		NearMiss.DisplayColor = FLinearColor(0.0f, 1.0f, 1.0f, 1.0f);
		RegisterScoreEventType(NearMiss);
	}
	{
		FMGScoreEventDefinition Takedown;
		Takedown.Type = EMGScoreEventType::Takedown;
		Takedown.DisplayName = FText::FromString(TEXT("TAKEDOWN"));
		Takedown.Category = EMGScoreCategory::Combat;
		Takedown.BasePoints = 500;
		Takedown.MinPoints = 300;
		Takedown.MaxPoints = 2000;
		Takedown.ChainBonusPerEvent = 0.25f;
		Takedown.MaxChainBonus = 5.0f;
		Takedown.bCanChain = true;
		Takedown.ChainTimeExtension = 3.0f;
		Takedown.DisplayColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
		RegisterScoreEventType(Takedown);
	}
	{
		FMGScoreEventDefinition Airtime;
		Airtime.Type = EMGScoreEventType::Airtime;
		Airtime.DisplayName = FText::FromString(TEXT("BIG AIR"));
		Airtime.Category = EMGScoreCategory::Style;
		Airtime.BasePoints = 200;
		Airtime.MinPoints = 50;
		Airtime.MaxPoints = 3000;
		Airtime.ChainBonusPerEvent = 0.1f;
		Airtime.MaxChainBonus = 2.0f;
		Airtime.bCanChain = true;
		Airtime.ChainTimeExtension = 2.0f;
		Airtime.DisplayColor = FLinearColor(0.5f, 0.0f, 1.0f, 1.0f);
		RegisterScoreEventType(Airtime);
	}
	{
		FMGScoreEventDefinition Nitro;
		Nitro.Type = EMGScoreEventType::Nitro;
		Nitro.DisplayName = FText::FromString(TEXT("NITRO"));
		Nitro.Category = EMGScoreCategory::Technical;
		Nitro.BasePoints = 25;
		Nitro.MinPoints = 10;
		Nitro.MaxPoints = 500;
		Nitro.ChainBonusPerEvent = 0.05f;
		Nitro.MaxChainBonus = 1.5f;
		Nitro.bCanChain = true;
		Nitro.ChainTimeExtension = 1.0f;
		Nitro.DisplayColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
		RegisterScoreEventType(Nitro);
	}
	{
		FMGScoreEventDefinition SpeedTrap;
		SpeedTrap.Type = EMGScoreEventType::SpeedTrap;
		SpeedTrap.DisplayName = FText::FromString(TEXT("SPEED TRAP"));
		SpeedTrap.Category = EMGScoreCategory::Racing;
		SpeedTrap.BasePoints = 100;
		SpeedTrap.MinPoints = 50;
		SpeedTrap.MaxPoints = 1000;
		SpeedTrap.bCanChain = false;
		SpeedTrap.DisplayColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		RegisterScoreEventType(SpeedTrap);
	}
	{
		FMGScoreEventDefinition Destruction;
		Destruction.Type = EMGScoreEventType::Destruction;
		Destruction.DisplayName = FText::FromString(TEXT("DESTRUCTION"));
		Destruction.Category = EMGScoreCategory::Combat;
		Destruction.BasePoints = 75;
		Destruction.MinPoints = 25;
		Destruction.MaxPoints = 500;
		Destruction.ChainBonusPerEvent = 0.1f;
		Destruction.MaxChainBonus = 2.0f;
		Destruction.bCanChain = true;
		Destruction.ChainTimeExtension = 1.0f;
		Destruction.DisplayColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f);
		RegisterScoreEventType(Destruction);
	}
	{
		FMGScoreEventDefinition Overtake;
		Overtake.Type = EMGScoreEventType::Overtake;
		Overtake.DisplayName = FText::FromString(TEXT("OVERTAKE"));
		Overtake.Category = EMGScoreCategory::Racing;
		Overtake.BasePoints = 150;
		Overtake.MinPoints = 100;
		Overtake.MaxPoints = 500;
		Overtake.ChainBonusPerEvent = 0.15f;
		Overtake.MaxChainBonus = 2.5f;
		Overtake.bCanChain = true;
		Overtake.ChainTimeExtension = 2.0f;
		Overtake.DisplayColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
		RegisterScoreEventType(Overtake);
	}
	{
		FMGScoreEventDefinition CleanSection;
		CleanSection.Type = EMGScoreEventType::CleanSection;
		CleanSection.DisplayName = FText::FromString(TEXT("CLEAN SECTION"));
		CleanSection.Category = EMGScoreCategory::Technical;
		CleanSection.BasePoints = 200;
		CleanSection.bCanChain = false;
		CleanSection.DisplayColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
		RegisterScoreEventType(CleanSection);
	}
	{
		FMGScoreEventDefinition PerfectLanding;
		PerfectLanding.Type = EMGScoreEventType::PerfectLanding;
		PerfectLanding.DisplayName = FText::FromString(TEXT("PERFECT LANDING"));
		PerfectLanding.Category = EMGScoreCategory::Style;
		PerfectLanding.BasePoints = 300;
		PerfectLanding.bCanChain = true;
		PerfectLanding.ChainTimeExtension = 2.0f;
		PerfectLanding.DisplayColor = FLinearColor(1.0f, 0.9f, 0.0f, 1.0f);
		RegisterScoreEventType(PerfectLanding);
	}
	{
		FMGScoreEventDefinition Trick;
		Trick.Type = EMGScoreEventType::Trick;
		Trick.DisplayName = FText::FromString(TEXT("TRICK"));
		Trick.Category = EMGScoreCategory::Style;
		Trick.BasePoints = 250;
		Trick.MinPoints = 100;
		Trick.MaxPoints = 2000;
		Trick.ChainBonusPerEvent = 0.2f;
		Trick.MaxChainBonus = 4.0f;
		Trick.bCanChain = true;
		Trick.ChainTimeExtension = 2.5f;
		Trick.DisplayColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
		RegisterScoreEventType(Trick);
	}

	// Register grade thresholds
	{
		FMGScoreGradeThreshold GradeF;
		GradeF.Grade = EMGScoreGrade::F;
		GradeF.MinScore = 0;
		GradeF.GradeText = FText::FromString(TEXT("F"));
		GradeF.GradeColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		RegisterGradeThreshold(GradeF);
	}
	{
		FMGScoreGradeThreshold GradeD;
		GradeD.Grade = EMGScoreGrade::D;
		GradeD.MinScore = 5000;
		GradeD.GradeText = FText::FromString(TEXT("D"));
		GradeD.GradeColor = FLinearColor(0.7f, 0.3f, 0.3f, 1.0f);
		RegisterGradeThreshold(GradeD);
	}
	{
		FMGScoreGradeThreshold GradeC;
		GradeC.Grade = EMGScoreGrade::C;
		GradeC.MinScore = 15000;
		GradeC.GradeText = FText::FromString(TEXT("C"));
		GradeC.GradeColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		RegisterGradeThreshold(GradeC);
	}
	{
		FMGScoreGradeThreshold GradeB;
		GradeB.Grade = EMGScoreGrade::B;
		GradeB.MinScore = 30000;
		GradeB.MinChainLength = 3;
		GradeB.GradeText = FText::FromString(TEXT("B"));
		GradeB.GradeColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
		RegisterGradeThreshold(GradeB);
	}
	{
		FMGScoreGradeThreshold GradeA;
		GradeA.Grade = EMGScoreGrade::A;
		GradeA.MinScore = 50000;
		GradeA.MinChainLength = 5;
		GradeA.MinMultiplierAverage = 1.5f;
		GradeA.GradeText = FText::FromString(TEXT("A"));
		GradeA.GradeColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
		RegisterGradeThreshold(GradeA);
	}
	{
		FMGScoreGradeThreshold GradeS;
		GradeS.Grade = EMGScoreGrade::S;
		GradeS.MinScore = 80000;
		GradeS.MinChainLength = 10;
		GradeS.MinMultiplierAverage = 2.0f;
		GradeS.GradeText = FText::FromString(TEXT("S"));
		GradeS.GradeColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);
		RegisterGradeThreshold(GradeS);
	}
	{
		FMGScoreGradeThreshold GradeSS;
		GradeSS.Grade = EMGScoreGrade::SS;
		GradeSS.MinScore = 120000;
		GradeSS.MinChainLength = 15;
		GradeSS.MinMultiplierAverage = 2.5f;
		GradeSS.GradeText = FText::FromString(TEXT("SS"));
		GradeSS.GradeColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f);
		RegisterGradeThreshold(GradeSS);
	}
	{
		FMGScoreGradeThreshold GradeSSS;
		GradeSSS.Grade = EMGScoreGrade::SSS;
		GradeSSS.MinScore = 200000;
		GradeSSS.MinChainLength = 25;
		GradeSSS.MinMultiplierAverage = 3.0f;
		GradeSSS.GradeText = FText::FromString(TEXT("SSS"));
		GradeSSS.GradeColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);
		RegisterGradeThreshold(GradeSSS);
	}

	// Initialize score milestones
	ScoreMilestones.Add(10000);
	ScoreMilestones.Add(25000);
	ScoreMilestones.Add(50000);
	ScoreMilestones.Add(100000);
	ScoreMilestones.Add(150000);
	ScoreMilestones.Add(200000);
	ScoreMilestones.Add(300000);
	ScoreMilestones.Add(500000);

	// Start scoring tick
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGScoringSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(ScoringTickTimer, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->UpdateScoringSystem(0.033f);
			}
		}, 0.033f, true);
	}
}

void UMGScoringSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScoringTickTimer);
	}

	EventDefinitions.Empty();
	GradeThresholds.Empty();
	PlayerScores.Empty();
	ActiveChains.Empty();
	PlayerMultipliers.Empty();
	ActiveRaceIds.Empty();
	ReachedMilestones.Empty();

	Super::Deinitialize();
}

// Registration
void UMGScoringSubsystem::RegisterScoreEventType(const FMGScoreEventDefinition& Definition)
{
	EventDefinitions.Add(Definition.Type, Definition);
}

void UMGScoringSubsystem::RegisterGradeThreshold(const FMGScoreGradeThreshold& Threshold)
{
	GradeThresholds.Add(Threshold.Grade, Threshold);
}

// Score Events
FMGScoreEvent UMGScoringSubsystem::AddScore(const FString& PlayerId, EMGScoreEventType Type, int32 BasePoints)
{
	return AddScoreWithMultiplier(PlayerId, Type, BasePoints, 1.0f);
}

FMGScoreEvent UMGScoringSubsystem::AddScoreWithMultiplier(const FString& PlayerId, EMGScoreEventType Type, int32 BasePoints, float ExtraMultiplier)
{
	FMGScoreEvent Event;
	Event.EventId = GenerateEventId();
	Event.Type = Type;
	Event.PlayerId = PlayerId;
	Event.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Get definition
	const FMGScoreEventDefinition* Definition = EventDefinitions.Find(Type);
	if (Definition)
	{
		Event.Category = Definition->Category;
		Event.DisplayText = Definition->DisplayName;

		if (BasePoints == 0)
		{
			BasePoints = Definition->BasePoints;
		}
		BasePoints = FMath::Clamp(BasePoints, Definition->MinPoints, Definition->MaxPoints);
	}

	Event.BasePoints = BasePoints;

	// Calculate multiplier
	float TotalMultiplier = GetTotalMultiplier(PlayerId) * ExtraMultiplier;

	// Add chain bonus if applicable
	FMGScoreChain* Chain = ActiveChains.Find(PlayerId);
	if (Chain && Chain->bIsActive && Definition && Definition->bCanChain)
	{
		float ChainBonus = FMath::Min(Chain->ChainLength * Definition->ChainBonusPerEvent, Definition->MaxChainBonus);
		TotalMultiplier *= (1.0f + ChainBonus);

		Event.bIsChainEvent = true;
		Event.ChainIndex = Chain->ChainLength;

		// Extend chain
		ExtendChain(PlayerId, Type, BasePoints);
	}
	else if (Definition && Definition->bCanChain)
	{
		// Start new chain
		StartChain(PlayerId, Type, BasePoints);
	}

	Event.Multiplier = TotalMultiplier;
	Event.FinalPoints = CalculateFinalPoints(BasePoints, TotalMultiplier, Definition ? *Definition : FMGScoreEventDefinition());

	// Update player score
	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (!Score)
	{
		FMGPlayerScore NewScore;
		NewScore.PlayerId = PlayerId;
		Score = &PlayerScores.Add(PlayerId, NewScore);
	}

	Score->TotalScore += Event.FinalPoints;

	// Update category score
	int32& CategoryScore = Score->CategoryScores.FindOrAdd(Event.Category);
	CategoryScore += Event.FinalPoints;

	// Update event type counts
	int32& TypeCount = Score->EventTypeCounts.FindOrAdd(Type);
	TypeCount++;
	int32& TypePoints = Score->EventTypePoints.FindOrAdd(Type);
	TypePoints += Event.FinalPoints;

	// Update multiplier tracking
	if (Event.Multiplier > Score->HighestMultiplier)
	{
		Score->HighestMultiplier = Event.Multiplier;
	}

	// Update highest single event
	if (Event.FinalPoints > Score->HighestSingleEvent)
	{
		Score->HighestSingleEvent = Event.FinalPoints;
	}

	// Add to recent events
	Score->RecentEvents.Insert(Event, 0);
	if (Score->RecentEvents.Num() > Score->MaxRecentEvents)
	{
		Score->RecentEvents.SetNum(Score->MaxRecentEvents);
	}

	// Update grade
	UpdateGrade(PlayerId);

	// Check milestones
	CheckMilestones(PlayerId, Score->TotalScore);

	OnScoreEvent.Broadcast(PlayerId, Event, Score->TotalScore);

	return Event;
}

void UMGScoringSubsystem::AddPenalty(const FString& PlayerId, int32 PenaltyPoints, const FText& Reason)
{
	FMGScoreEvent Event;
	Event.EventId = GenerateEventId();
	Event.Type = EMGScoreEventType::None;
	Event.Category = EMGScoreCategory::Penalty;
	Event.PlayerId = PlayerId;
	Event.BasePoints = -PenaltyPoints;
	Event.FinalPoints = -PenaltyPoints;
	Event.Multiplier = 1.0f;
	Event.DisplayText = Reason;
	Event.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (Score)
	{
		Score->TotalScore = FMath::Max(0, Score->TotalScore - PenaltyPoints);

		int32& PenaltyScore = Score->CategoryScores.FindOrAdd(EMGScoreCategory::Penalty);
		PenaltyScore += PenaltyPoints;

		Score->RecentEvents.Insert(Event, 0);
		if (Score->RecentEvents.Num() > Score->MaxRecentEvents)
		{
			Score->RecentEvents.SetNum(Score->MaxRecentEvents);
		}
	}

	// Break any active chain
	BreakChain(PlayerId);

	OnScoreEvent.Broadcast(PlayerId, Event, Score ? Score->TotalScore : 0);
}

void UMGScoringSubsystem::AddBonusPoints(const FString& PlayerId, int32 BonusPoints, const FText& Reason)
{
	FMGScoreEvent Event;
	Event.EventId = GenerateEventId();
	Event.Type = EMGScoreEventType::Bonus;
	Event.Category = EMGScoreCategory::Bonus;
	Event.PlayerId = PlayerId;
	Event.BasePoints = BonusPoints;
	Event.FinalPoints = BonusPoints;
	Event.Multiplier = 1.0f;
	Event.DisplayText = Reason;
	Event.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (!Score)
	{
		FMGPlayerScore NewScore;
		NewScore.PlayerId = PlayerId;
		Score = &PlayerScores.Add(PlayerId, NewScore);
	}

	Score->TotalScore += BonusPoints;
	int32& BonusCategory = Score->CategoryScores.FindOrAdd(EMGScoreCategory::Bonus);
	BonusCategory += BonusPoints;

	CheckMilestones(PlayerId, Score->TotalScore);
	OnScoreEvent.Broadcast(PlayerId, Event, Score->TotalScore);
}

// Chain Management
void UMGScoringSubsystem::StartChain(const FString& PlayerId, EMGScoreEventType StartType, int32 BasePoints)
{
	FMGScoreChain Chain;
	Chain.ChainId = GenerateChainId();
	Chain.PlayerId = PlayerId;
	Chain.bIsActive = true;
	Chain.ChainLength = 1;
	Chain.TotalBasePoints = BasePoints;
	Chain.ChainMultiplier = 1.0f;
	Chain.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	const FMGScoreEventDefinition* Def = EventDefinitions.Find(StartType);
	if (Def)
	{
		Chain.MaxChainTime = 3.0f;
		Chain.ChainTimer = Chain.MaxChainTime;
	}

	ActiveChains.Add(PlayerId, Chain);

	// Update player chain tracking
	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (Score)
	{
		Score->CurrentChainLength = 1;
		Score->CurrentMultiplier = 1.0f;
	}

	OnChainStarted.Broadcast(PlayerId, StartType, BasePoints);
}

void UMGScoringSubsystem::ExtendChain(const FString& PlayerId, EMGScoreEventType Type, int32 Points)
{
	FMGScoreChain* Chain = ActiveChains.Find(PlayerId);
	if (!Chain || !Chain->bIsActive)
	{
		StartChain(PlayerId, Type, Points);
		return;
	}

	Chain->ChainLength++;
	Chain->TotalBasePoints += Points;

	// Update multiplier
	const FMGScoreEventDefinition* Def = EventDefinitions.Find(Type);
	if (Def)
	{
		float ChainBonus = Chain->ChainLength * Def->ChainBonusPerEvent;
		Chain->ChainMultiplier = 1.0f + FMath::Min(ChainBonus, Def->MaxChainBonus);

		// Extend timer
		if (Def->bExtendsChainTimer)
		{
			Chain->ChainTimer = FMath::Min(Chain->ChainTimer + Def->ChainTimeExtension, Chain->MaxChainTime * 2.0f);
		}
	}

	// Calculate chain points
	Chain->TotalFinalPoints = FMath::RoundToInt(Chain->TotalBasePoints * Chain->ChainMultiplier);

	// Update player tracking
	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (Score)
	{
		Score->CurrentChainLength = Chain->ChainLength;
		Score->CurrentMultiplier = Chain->ChainMultiplier;

		if (Chain->ChainLength > Score->LongestChain)
		{
			Score->LongestChain = Chain->ChainLength;
		}
	}

	OnChainExtended.Broadcast(PlayerId, Chain->ChainLength, Chain->ChainMultiplier, Chain->TotalFinalPoints);
}

void UMGScoringSubsystem::EndChain(const FString& PlayerId)
{
	FMGScoreChain* Chain = ActiveChains.Find(PlayerId);
	if (!Chain || !Chain->bIsActive)
	{
		return;
	}

	Chain->bIsActive = false;

	OnChainEnded.Broadcast(PlayerId, Chain->ChainLength, Chain->TotalFinalPoints);

	// Update player tracking
	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (Score)
	{
		Score->CurrentChainLength = 0;
		Score->CurrentMultiplier = 1.0f;
	}

	ActiveChains.Remove(PlayerId);
}

void UMGScoringSubsystem::BreakChain(const FString& PlayerId)
{
	FMGScoreChain* Chain = ActiveChains.Find(PlayerId);
	if (!Chain || !Chain->bIsActive)
	{
		return;
	}

	// Chain broken - lose some points potential
	Chain->bIsActive = false;

	OnChainEnded.Broadcast(PlayerId, Chain->ChainLength, Chain->TotalFinalPoints);

	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (Score)
	{
		Score->CurrentChainLength = 0;
		Score->CurrentMultiplier = 1.0f;
	}

	ActiveChains.Remove(PlayerId);
}

FMGScoreChain UMGScoringSubsystem::GetActiveChain(const FString& PlayerId) const
{
	if (const FMGScoreChain* Chain = ActiveChains.Find(PlayerId))
	{
		return *Chain;
	}
	return FMGScoreChain();
}

bool UMGScoringSubsystem::HasActiveChain(const FString& PlayerId) const
{
	const FMGScoreChain* Chain = ActiveChains.Find(PlayerId);
	return Chain && Chain->bIsActive;
}

float UMGScoringSubsystem::GetChainTimeRemaining(const FString& PlayerId) const
{
	if (const FMGScoreChain* Chain = ActiveChains.Find(PlayerId))
	{
		return Chain->ChainTimer;
	}
	return 0.0f;
}

// Multipliers
void UMGScoringSubsystem::AddMultiplierSource(const FString& PlayerId, const FMGScoreMultiplierSource& Source)
{
	TArray<FMGScoreMultiplierSource>* Multipliers = PlayerMultipliers.Find(PlayerId);
	if (!Multipliers)
	{
		TArray<FMGScoreMultiplierSource> NewArray;
		Multipliers = &PlayerMultipliers.Add(PlayerId, NewArray);
	}

	// Check if already exists
	for (FMGScoreMultiplierSource& Existing : *Multipliers)
	{
		if (Existing.SourceId == Source.SourceId)
		{
			if (Source.bIsStackable)
			{
				Existing.MultiplierValue += Source.MultiplierValue - 1.0f;
			}
			else
			{
				Existing.MultiplierValue = FMath::Max(Existing.MultiplierValue, Source.MultiplierValue);
			}
			Existing.RemainingTime = Source.Duration;
			return;
		}
	}

	Multipliers->Add(Source);

	float NewTotal = GetTotalMultiplier(PlayerId);
	OnMultiplierChanged.Broadcast(PlayerId, 1.0f, NewTotal);
}

void UMGScoringSubsystem::RemoveMultiplierSource(const FString& PlayerId, const FString& SourceId)
{
	TArray<FMGScoreMultiplierSource>* Multipliers = PlayerMultipliers.Find(PlayerId);
	if (!Multipliers)
	{
		return;
	}

	float OldTotal = GetTotalMultiplier(PlayerId);

	Multipliers->RemoveAll([&SourceId](const FMGScoreMultiplierSource& Source)
	{
		return Source.SourceId == SourceId;
	});

	float NewTotal = GetTotalMultiplier(PlayerId);
	if (OldTotal != NewTotal)
	{
		OnMultiplierChanged.Broadcast(PlayerId, OldTotal, NewTotal);
	}
}

float UMGScoringSubsystem::GetTotalMultiplier(const FString& PlayerId) const
{
	float Total = 1.0f;

	const TArray<FMGScoreMultiplierSource>* Multipliers = PlayerMultipliers.Find(PlayerId);
	if (Multipliers)
	{
		for (const FMGScoreMultiplierSource& Source : *Multipliers)
		{
			Total *= Source.MultiplierValue;
		}
	}

	return Total;
}

TArray<FMGScoreMultiplierSource> UMGScoringSubsystem::GetActiveMultipliers(const FString& PlayerId) const
{
	if (const TArray<FMGScoreMultiplierSource>* Multipliers = PlayerMultipliers.Find(PlayerId))
	{
		return *Multipliers;
	}
	return TArray<FMGScoreMultiplierSource>();
}

// Player Score
FMGPlayerScore UMGScoringSubsystem::GetPlayerScore(const FString& PlayerId) const
{
	if (const FMGPlayerScore* Score = PlayerScores.Find(PlayerId))
	{
		return *Score;
	}
	return FMGPlayerScore();
}

int32 UMGScoringSubsystem::GetTotalScore(const FString& PlayerId) const
{
	if (const FMGPlayerScore* Score = PlayerScores.Find(PlayerId))
	{
		return Score->TotalScore;
	}
	return 0;
}

int32 UMGScoringSubsystem::GetCategoryScore(const FString& PlayerId, EMGScoreCategory Category) const
{
	if (const FMGPlayerScore* Score = PlayerScores.Find(PlayerId))
	{
		if (const int32* CatScore = Score->CategoryScores.Find(Category))
		{
			return *CatScore;
		}
	}
	return 0;
}

EMGScoreGrade UMGScoringSubsystem::GetCurrentGrade(const FString& PlayerId) const
{
	if (const FMGPlayerScore* Score = PlayerScores.Find(PlayerId))
	{
		return Score->CurrentGrade;
	}
	return EMGScoreGrade::F;
}

TArray<FMGScoreEvent> UMGScoringSubsystem::GetRecentEvents(const FString& PlayerId) const
{
	if (const FMGPlayerScore* Score = PlayerScores.Find(PlayerId))
	{
		return Score->RecentEvents;
	}
	return TArray<FMGScoreEvent>();
}

// Session Management
void UMGScoringSubsystem::StartScoringSession(const FString& PlayerId, const FString& RaceId)
{
	ResetPlayerScore(PlayerId);
	ActiveRaceIds.Add(PlayerId, RaceId);

	TSet<int32> EmptySet;
	ReachedMilestones.Add(PlayerId, EmptySet);
}

FMGRaceScoreSummary UMGScoringSubsystem::EndScoringSession(const FString& PlayerId)
{
	FMGRaceScoreSummary Summary;
	Summary.PlayerId = PlayerId;
	Summary.Timestamp = FDateTime::Now();

	const FString* RaceId = ActiveRaceIds.Find(PlayerId);
	if (RaceId)
	{
		Summary.RaceId = *RaceId;
	}

	const FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (Score)
	{
		Summary.TotalScore = Score->TotalScore;
		Summary.FinalGrade = Score->CurrentGrade;
		Summary.LongestChain = Score->LongestChain;
		Summary.HighestMultiplier = Score->HighestMultiplier;
		Summary.HighestSingleEvent = Score->HighestSingleEvent;

		// Category breakdown
		if (const int32* Racing = Score->CategoryScores.Find(EMGScoreCategory::Racing))
		{
			Summary.RacingScore = *Racing;
		}
		if (const int32* Style = Score->CategoryScores.Find(EMGScoreCategory::Style))
		{
			Summary.StyleScore = *Style;
		}
		if (const int32* Combat = Score->CategoryScores.Find(EMGScoreCategory::Combat))
		{
			Summary.CombatScore = *Combat;
		}
		if (const int32* Bonus = Score->CategoryScores.Find(EMGScoreCategory::Bonus))
		{
			Summary.BonusScore = *Bonus;
		}
		if (const int32* Penalty = Score->CategoryScores.Find(EMGScoreCategory::Penalty))
		{
			Summary.PenaltyScore = *Penalty;
		}

		// Event breakdown
		Summary.EventBreakdown = Score->EventTypePoints;

		// Calculate total events
		for (const auto& Pair : Score->EventTypeCounts)
		{
			Summary.TotalEvents += Pair.Value;
		}
	}

	// End any active chain
	EndChain(PlayerId);

	OnScoreSummary.Broadcast(PlayerId, Summary);

	ActiveRaceIds.Remove(PlayerId);
	ReachedMilestones.Remove(PlayerId);

	return Summary;
}

void UMGScoringSubsystem::ResetPlayerScore(const FString& PlayerId)
{
	FMGPlayerScore NewScore;
	NewScore.PlayerId = PlayerId;
	PlayerScores.Add(PlayerId, NewScore);

	ActiveChains.Remove(PlayerId);
	PlayerMultipliers.Remove(PlayerId);
}

// Grade Calculation
EMGScoreGrade UMGScoringSubsystem::CalculateGrade(int32 Score, float AverageMultiplier, int32 LongestChain) const
{
	EMGScoreGrade HighestGrade = EMGScoreGrade::F;

	for (const auto& Pair : GradeThresholds)
	{
		const FMGScoreGradeThreshold& Threshold = Pair.Value;

		if (Score >= Threshold.MinScore &&
			(Threshold.MinMultiplierAverage <= 1.0f || AverageMultiplier >= Threshold.MinMultiplierAverage) &&
			LongestChain >= Threshold.MinChainLength)
		{
			if (Pair.Key > HighestGrade)
			{
				HighestGrade = Pair.Key;
			}
		}
	}

	return HighestGrade;
}

FMGScoreGradeThreshold UMGScoringSubsystem::GetGradeThreshold(EMGScoreGrade Grade) const
{
	if (const FMGScoreGradeThreshold* Threshold = GradeThresholds.Find(Grade))
	{
		return *Threshold;
	}
	return FMGScoreGradeThreshold();
}

int32 UMGScoringSubsystem::GetScoreForGrade(EMGScoreGrade Grade) const
{
	if (const FMGScoreGradeThreshold* Threshold = GradeThresholds.Find(Grade))
	{
		return Threshold->MinScore;
	}
	return 0;
}

// Definitions
FMGScoreEventDefinition UMGScoringSubsystem::GetEventDefinition(EMGScoreEventType Type) const
{
	if (const FMGScoreEventDefinition* Def = EventDefinitions.Find(Type))
	{
		return *Def;
	}
	return FMGScoreEventDefinition();
}

TArray<FMGScoreEventDefinition> UMGScoringSubsystem::GetAllEventDefinitions() const
{
	TArray<FMGScoreEventDefinition> Result;
	EventDefinitions.GenerateValueArray(Result);
	return Result;
}

// Update
void UMGScoringSubsystem::UpdateScoringSystem(float MGDeltaTime)
{
	TickChains(DeltaTime);
	TickMultipliers(DeltaTime);
}

// Protected
void UMGScoringSubsystem::TickChains(float MGDeltaTime)
{
	TArray<FString> ChainsToEnd;

	for (auto& Pair : ActiveChains)
	{
		FMGScoreChain& Chain = Pair.Value;

		if (!Chain.bIsActive)
		{
			continue;
		}

		Chain.ChainTimer -= DeltaTime;

		if (Chain.ChainTimer <= 0.0f)
		{
			ChainsToEnd.Add(Pair.Key);
		}
	}

	for (const FString& PlayerId : ChainsToEnd)
	{
		EndChain(PlayerId);
	}
}

void UMGScoringSubsystem::TickMultipliers(float MGDeltaTime)
{
	for (auto& Pair : PlayerMultipliers)
	{
		TArray<FString> ExpiredSources;

		for (FMGScoreMultiplierSource& Source : Pair.Value)
		{
			if (!Source.bIsPermanent && Source.Duration > 0.0f)
			{
				Source.RemainingTime -= DeltaTime;

				if (Source.RemainingTime <= 0.0f)
				{
					ExpiredSources.Add(Source.SourceId);
				}
			}
		}

		for (const FString& SourceId : ExpiredSources)
		{
			RemoveMultiplierSource(Pair.Key, SourceId);
		}
	}
}

void UMGScoringSubsystem::UpdateGrade(const FString& PlayerId)
{
	FMGPlayerScore* Score = PlayerScores.Find(PlayerId);
	if (!Score)
	{
		return;
	}

	EMGScoreGrade OldGrade = Score->CurrentGrade;
	EMGScoreGrade NewGrade = CalculateGrade(Score->TotalScore, Score->HighestMultiplier, Score->LongestChain);

	if (NewGrade != OldGrade)
	{
		Score->CurrentGrade = NewGrade;
		OnGradeChanged.Broadcast(PlayerId, OldGrade, NewGrade);
	}
}

void UMGScoringSubsystem::CheckMilestones(const FString& PlayerId, int32 NewTotal)
{
	TSet<int32>* Reached = ReachedMilestones.Find(PlayerId);
	if (!Reached)
	{
		return;
	}

	for (int32 Milestone : ScoreMilestones)
	{
		if (NewTotal >= Milestone && !Reached->Contains(Milestone))
		{
			Reached->Add(Milestone);
			OnMilestoneScore.Broadcast(PlayerId, Milestone, NewTotal);
		}
	}
}

int32 UMGScoringSubsystem::CalculateFinalPoints(int32 BasePoints, float Multiplier, const FMGScoreEventDefinition& Definition) const
{
	int32 FinalPoints = FMath::RoundToInt(BasePoints * Multiplier);

	if (Definition.MaxPoints > 0)
	{
		FinalPoints = FMath::Clamp(FinalPoints, Definition.MinPoints, Definition.MaxPoints);
	}

	return FinalPoints;
}

FString UMGScoringSubsystem::GenerateEventId() const
{
	return FString::Printf(TEXT("SCORE_%d_%lld"), ++const_cast<UMGScoringSubsystem*>(this)->EventCounter, FDateTime::Now().GetTicks());
}

FString UMGScoringSubsystem::GenerateChainId() const
{
	return FString::Printf(TEXT("CHAIN_%d_%lld"), ++const_cast<UMGScoringSubsystem*>(this)->ChainCounter, FDateTime::Now().GetTicks());
}
