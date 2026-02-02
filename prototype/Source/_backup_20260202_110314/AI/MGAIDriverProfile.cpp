// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGAIDriverProfile.h"

// ==========================================
// UMGAIDriverProfile
// ==========================================

int32 UMGAIDriverProfile::GetOverallRating() const
{
	float Rating = 0.0f;

	// Skill contribution (40%)
	Rating += Skill.SkillLevel * 40.0f;

	// Speed contribution (30%)
	Rating += Speed.BaseSpeedPercent * 30.0f;

	// Consistency contribution (20%)
	Rating += Skill.Consistency * 20.0f;

	// Racecraft contribution (10%)
	float RacecraftAvg = (Racecraft.Awareness + Racecraft.Anticipation + Racecraft.GapFinding) / 3.0f;
	Rating += RacecraftAvg * 10.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

int32 UMGAIDriverProfile::GetSpeedRating() const
{
	float Rating = 0.0f;

	Rating += Speed.BaseSpeedPercent * 50.0f;
	Rating += Speed.CornerSpeedMultiplier * 25.0f;
	Rating += Skill.CornerExitSpeed * 25.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

int32 UMGAIDriverProfile::GetAggressionRating() const
{
	float Rating = 0.0f;

	Rating += Aggression.Aggression * 40.0f;
	Rating += Aggression.OvertakeAggression * 30.0f;
	Rating += Aggression.RiskTaking * 30.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

int32 UMGAIDriverProfile::GetConsistencyRating() const
{
	float Rating = 0.0f;

	Rating += Skill.Consistency * 50.0f;
	Rating += (1.0f - Skill.MistakeFrequency) * 30.0f;
	Rating += Skill.RecoverySkill * 20.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

void UMGAIDriverProfile::ApplyDifficultyModifier(float Modifier)
{
	// Adjust skill parameters
	Skill.SkillLevel = FMath::Clamp(Skill.SkillLevel * Modifier, 0.0f, 1.0f);
	Skill.BrakingAccuracy = FMath::Clamp(Skill.BrakingAccuracy * Modifier, 0.0f, 1.0f);
	Skill.LineAccuracy = FMath::Clamp(Skill.LineAccuracy * Modifier, 0.0f, 1.0f);
	Skill.Consistency = FMath::Clamp(Skill.Consistency * Modifier, 0.0f, 1.0f);

	// Adjust reaction time (inverse - lower modifier = slower reactions)
	Skill.ReactionTime = FMath::Clamp(Skill.ReactionTime / Modifier, 0.1f, 1.0f);

	// Adjust speed
	Speed.BaseSpeedPercent = FMath::Clamp(Speed.BaseSpeedPercent * Modifier, 0.5f, 1.1f);
	Speed.CornerSpeedMultiplier = FMath::Clamp(Speed.CornerSpeedMultiplier * Modifier, 0.7f, 1.1f);

	// Adjust mistake frequency (inverse)
	Skill.MistakeFrequency = FMath::Clamp(Skill.MistakeFrequency / Modifier, 0.0f, 1.0f);
}

// ==========================================
// ADAPTIVE BEHAVIOR
// ==========================================

void UMGAIDriverProfile::RecordRaceResult(bool bWon, FName TrackID, float FinishTimeDelta)
{
	AdaptiveData.RacesAgainstPlayer++;

	if (bWon)
	{
		AdaptiveData.WinsAgainstPlayer++;

		// AI gains confidence, improves slightly
		AdaptiveData.SkillGrowthFactor += 0.005f;
	}
	else
	{
		AdaptiveData.LossesAgainstPlayer++;

		// AI adapts after losses - learns track better
		if (float* TrackMod = AdaptiveData.TrackSkillModifiers.Find(TrackID))
		{
			*TrackMod += 0.02f;
		}
		else
		{
			AdaptiveData.TrackSkillModifiers.Add(TrackID, 0.02f);
		}
	}

	// Cap growth factor
	AdaptiveData.SkillGrowthFactor = FMath::Clamp(AdaptiveData.SkillGrowthFactor, 0.0f, 0.15f);
}

void UMGAIDriverProfile::UpdateMood(float PositionDelta, float DamageReceived, bool bWasOvertaken)
{
	// Positive position change = moving up = confidence
	// Negative = dropping = frustration

	if (PositionDelta > 2)
	{
		CurrentMood = EMGAIMood::Confident;
	}
	else if (PositionDelta < -2)
	{
		CurrentMood = EMGAIMood::Frustrated;
	}
	else if (DamageReceived > 0.3f)
	{
		// Took significant damage
		if (Aggression.Aggression > 0.6f)
		{
			CurrentMood = EMGAIMood::Vengeful;
		}
		else
		{
			CurrentMood = EMGAIMood::Intimidated;
		}
	}
	else if (bWasOvertaken && Aggression.Aggression > 0.5f)
	{
		CurrentMood = EMGAIMood::Frustrated;
	}

	// Check for "in the zone" - consistent high performance
	if (PositionDelta == 0 && DamageReceived < 0.1f && Skill.Consistency > 0.8f)
	{
		if (FMath::FRand() < 0.1f) // 10% chance per update
		{
			CurrentMood = EMGAIMood::InTheZone;
		}
	}

	// Desperate if far behind
	if (PositionDelta < -4)
	{
		CurrentMood = EMGAIMood::Desperate;
	}
}

float UMGAIDriverProfile::GetEffectiveSkill() const
{
	float BaseSkill = Skill.SkillLevel;

	// Apply learning multiplier
	BaseSkill *= AdaptiveData.GetLearningMultiplier();

	// Apply mood modifiers
	switch (CurrentMood)
	{
		case EMGAIMood::InTheZone:
			BaseSkill *= 1.1f; // Peak performance
			break;
		case EMGAIMood::Confident:
			BaseSkill *= 1.03f;
			break;
		case EMGAIMood::Frustrated:
			BaseSkill *= 0.97f; // Slight mistakes
			break;
		case EMGAIMood::Desperate:
			BaseSkill *= 0.9f; // More mistakes
			break;
		case EMGAIMood::Intimidated:
			BaseSkill *= 0.95f;
			break;
		default:
			break;
	}

	return FMath::Clamp(BaseSkill, 0.1f, 1.0f);
}

float UMGAIDriverProfile::GetEffectiveAggression() const
{
	float BaseAggression = Aggression.Aggression;

	// Apply mood modifiers
	switch (CurrentMood)
	{
		case EMGAIMood::Vengeful:
			BaseAggression *= 1.3f; // Very aggressive
			break;
		case EMGAIMood::Frustrated:
			BaseAggression *= 1.15f;
			break;
		case EMGAIMood::Desperate:
			BaseAggression *= 1.4f; // Takes big risks
			break;
		case EMGAIMood::Intimidated:
			BaseAggression *= 0.7f; // Backs off
			break;
		case EMGAIMood::Confident:
			BaseAggression *= 1.05f;
			break;
		default:
			break;
	}

	// Apply rivalry intensity
	if (PlayerRivalry.Intensity > 0.5f)
	{
		BaseAggression *= (1.0f + PlayerRivalry.Intensity * 0.3f);
	}

	return FMath::Clamp(BaseAggression, 0.0f, 1.0f);
}

void UMGAIDriverProfile::LearnPlayerBehavior(float ObservedAggression, float ObservedBraking, float OvertakeSide)
{
	// Exponential moving average to learn player tendencies
	const float LearningRate = 0.2f;

	AdaptiveData.PlayerAggressionEstimate = FMath::Lerp(
		AdaptiveData.PlayerAggressionEstimate,
		ObservedAggression,
		LearningRate
	);

	AdaptiveData.PlayerBrakingPointEstimate = FMath::Lerp(
		AdaptiveData.PlayerBrakingPointEstimate,
		ObservedBraking,
		LearningRate
	);

	AdaptiveData.PlayerOvertakePreference = FMath::Lerp(
		AdaptiveData.PlayerOvertakePreference,
		OvertakeSide,
		LearningRate
	);
}

void UMGAIDriverProfile::GetPredictedPlayerBehavior(float& OutAggression, float& OutBrakingPoint, float& OutOvertakeSide) const
{
	OutAggression = AdaptiveData.PlayerAggressionEstimate;
	OutBrakingPoint = AdaptiveData.PlayerBrakingPointEstimate;
	OutOvertakeSide = AdaptiveData.PlayerOvertakePreference;
}

void UMGAIDriverProfile::UpdateRivalry(bool bPlayerWon, bool bWasPinkSlip, const FString& EventDescription)
{
	// Update rivalry intensity
	if (bPlayerWon)
	{
		// Player won - AI respects them more but rivalry intensifies
		PlayerRivalry.Respect += 0.05f;
		PlayerRivalry.Intensity += 0.1f;

		if (bWasPinkSlip)
		{
			// Lost pink slip - major rivalry event
			PlayerRivalry.PinkSlipsLost++;
			PlayerRivalry.Intensity += 0.3f;
		}
	}
	else
	{
		// AI won - less respect, rivalry might cool
		PlayerRivalry.Respect -= 0.03f;
		PlayerRivalry.Intensity -= 0.05f;

		if (bWasPinkSlip)
		{
			// Won pink slip from player
			PlayerRivalry.PinkSlipsWon++;
			PlayerRivalry.Respect += 0.1f; // They earned a grudge match
		}
	}

	// Record event
	FString EventRecord = FString::Printf(TEXT("[%s] %s - %s"),
		*FDateTime::Now().ToString(),
		bPlayerWon ? TEXT("Player Win") : TEXT("AI Win"),
		*EventDescription
	);
	PlayerRivalry.RivalryHistory.Add(EventRecord);

	// Keep history manageable
	if (PlayerRivalry.RivalryHistory.Num() > 20)
	{
		PlayerRivalry.RivalryHistory.RemoveAt(0);
	}

	// Clamp values
	PlayerRivalry.Intensity = FMath::Clamp(PlayerRivalry.Intensity, -1.0f, 1.0f);
	PlayerRivalry.Respect = FMath::Clamp(PlayerRivalry.Respect, 0.0f, 1.0f);
}

// ==========================================
// AGGRESSION ESCALATION SYSTEM
// ==========================================

void UMGAIDriverProfile::RecordContact(AActor* Offender, float Severity, bool bWasPlayer, bool bSeemedIntentional)
{
	if (!Offender)
	{
		return;
	}

	// Check if we already have a grudge against this offender
	FMGAIContactEvent* ExistingContact = nullptr;
	for (FMGAIContactEvent& Contact : RecentContacts)
	{
		if (Contact.Offender.Get() == Offender)
		{
			ExistingContact = &Contact;
			break;
		}
	}

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	if (ExistingContact)
	{
		// Update existing grudge
		ExistingContact->ContactCount++;
		ExistingContact->Severity = FMath::Max(ExistingContact->Severity, Severity);
		ExistingContact->TimeStamp = CurrentTime;
		ExistingContact->bSeemedIntentional = ExistingContact->bSeemedIntentional || bSeemedIntentional;
	}
	else
	{
		// Create new grudge
		FMGAIContactEvent NewContact;
		NewContact.Offender = Offender;
		NewContact.bWasPlayer = bWasPlayer;
		NewContact.Severity = Severity;
		NewContact.TimeStamp = CurrentTime;
		NewContact.ContactCount = 1;
		NewContact.bSeemedIntentional = bSeemedIntentional;
		RecentContacts.Add(NewContact);
	}

	// Escalate aggression based on contact
	float EscalationAmount = Severity * Aggression.EscalationRate;

	// Intentional contact escalates more
	if (bSeemedIntentional)
	{
		EscalationAmount *= 1.5f;
	}

	// Repeated contact escalates even more
	if (ExistingContact && ExistingContact->ContactCount > 1)
	{
		EscalationAmount *= (1.0f + ExistingContact->ContactCount * 0.2f);
	}

	// Player contact may escalate differently based on personality
	if (bWasPlayer && Aggression.bTargetsPlayer)
	{
		EscalationAmount *= 1.3f;
	}

	AccumulatedAggression = FMath::Clamp(AccumulatedAggression + EscalationAmount, 0.0f, 1.2f);

	// Update grudge target
	if (Severity >= Aggression.MajorContactThreshold)
	{
		CurrentGrudgeTarget = Offender;
	}

	// Check for stage change
	EMGAggressionStage NewStage = CalculateAggressionStage(AccumulatedAggression);
	if (NewStage != CurrentAggressionStage)
	{
		CurrentAggressionStage = NewStage;
		TimeInAggressionStage = 0.0f;
	}
}

void UMGAIDriverProfile::UpdateAggressionState(float DeltaTime, int32 CurrentPosition, bool bUnderPressure, bool bApplyingPressure)
{
	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// Clean up old grudges
	CleanupExpiredGrudges(CurrentTime);

	// Natural de-escalation over time
	if (!bUnderPressure && !bApplyingPressure && RecentContacts.Num() == 0)
	{
		float DeescalationAmount = Aggression.DeescalationRate * DeltaTime * 0.1f;
		AccumulatedAggression = FMath::Max(0.0f, AccumulatedAggression - DeescalationAmount);
	}

	// Pressure-based escalation
	if (bUnderPressure)
	{
		// Being chased escalates aggression for defensive personalities
		if (Personality == EMGDriverPersonality::Defensive || Personality == EMGDriverPersonality::Aggressive)
		{
			AccumulatedAggression += Aggression.EscalationRate * DeltaTime * 0.05f;
		}
	}

	if (bApplyingPressure)
	{
		// Chasing escalates aggression for aggressive personalities
		if (Personality == EMGDriverPersonality::Aggressive || Personality == EMGDriverPersonality::Rival)
		{
			AccumulatedAggression += Aggression.EscalationRate * DeltaTime * 0.03f;
		}
	}

	// Position-based escalation
	if (CurrentPosition > 1 && CurrentMood == EMGAIMood::Frustrated)
	{
		AccumulatedAggression += Aggression.EscalationRate * DeltaTime * 0.02f;
	}

	// Clamp
	AccumulatedAggression = FMath::Clamp(AccumulatedAggression, 0.0f, 1.2f);

	// Update stage
	EMGAggressionStage NewStage = CalculateAggressionStage(AccumulatedAggression);
	if (NewStage != CurrentAggressionStage)
	{
		CurrentAggressionStage = NewStage;
		TimeInAggressionStage = 0.0f;
	}
	else
	{
		TimeInAggressionStage += DeltaTime;
	}

	// Rage mode causes mistakes - check if we should exit it
	if (CurrentAggressionStage == EMGAggressionStage::Rage && TimeInAggressionStage > 5.0f)
	{
		// Cool down from rage after a while
		AccumulatedAggression = Aggression.MaxAggressionThreshold - 0.1f;
		CurrentAggressionStage = EMGAggressionStage::Maximum;
		TimeInAggressionStage = 0.0f;
	}
}

float UMGAIDriverProfile::GetEscalatedAggression() const
{
	float BaseAggression = GetEffectiveAggression();

	// Add escalation bonus
	float EscalationBonus = 0.0f;
	switch (CurrentAggressionStage)
	{
		case EMGAggressionStage::Baseline:
			EscalationBonus = 0.0f;
			break;
		case EMGAggressionStage::Elevated:
			EscalationBonus = 0.1f;
			break;
		case EMGAggressionStage::High:
			EscalationBonus = 0.2f;
			break;
		case EMGAggressionStage::Maximum:
			EscalationBonus = 0.35f;
			break;
		case EMGAggressionStage::Rage:
			EscalationBonus = 0.5f; // Very high but causes mistakes
			break;
	}

	return FMath::Clamp(BaseAggression + EscalationBonus, 0.0f, 1.0f);
}

EMGContactResponse UMGAIDriverProfile::GetContactResponse(float Severity) const
{
	if (Severity >= Aggression.MajorContactThreshold)
	{
		return Aggression.MajorContactResponse;
	}
	return Aggression.MinorContactResponse;
}

bool UMGAIDriverProfile::HasGrudgeAgainst(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	for (const FMGAIContactEvent& Contact : RecentContacts)
	{
		if (Contact.Offender.Get() == Actor)
		{
			return true;
		}
	}
	return false;
}

float UMGAIDriverProfile::GetGrudgeIntensity(AActor* Actor) const
{
	if (!Actor)
	{
		return 0.0f;
	}

	for (const FMGAIContactEvent& Contact : RecentContacts)
	{
		if (Contact.Offender.Get() == Actor)
		{
			// Intensity based on severity, count, and whether it seemed intentional
			float Intensity = Contact.Severity;
			Intensity *= (1.0f + Contact.ContactCount * 0.3f);
			if (Contact.bSeemedIntentional)
			{
				Intensity *= 1.5f;
			}
			return FMath::Clamp(Intensity, 0.0f, 1.0f);
		}
	}
	return 0.0f;
}

bool UMGAIDriverProfile::WillUseDirtyTactics(int32 CurrentPosition, bool bIsDefending) const
{
	// Check position threshold
	if (Aggression.bOnlyDirtyWhenLosing && CurrentPosition <= 1)
	{
		return false;
	}

	if (CurrentPosition < Aggression.DirtyTacticsPositionThreshold)
	{
		return false;
	}

	// Get effective willingness
	float Willingness = bIsDefending ? Aggression.DefenseContactWillingness : Aggression.AttackContactWillingness;

	// Escalation increases willingness
	if (CurrentAggressionStage >= EMGAggressionStage::High)
	{
		Willingness *= 1.5f;
	}
	if (CurrentAggressionStage == EMGAggressionStage::Maximum || CurrentAggressionStage == EMGAggressionStage::Rage)
	{
		Willingness *= 2.0f;
	}

	// Grudge increases willingness
	if (CurrentGrudgeTarget.IsValid())
	{
		Willingness *= 1.3f;
	}

	// Mood affects willingness
	if (CurrentMood == EMGAIMood::Vengeful)
	{
		Willingness *= 1.5f;
	}
	else if (CurrentMood == EMGAIMood::Desperate)
	{
		Willingness *= 1.3f;
	}
	else if (CurrentMood == EMGAIMood::Intimidated)
	{
		Willingness *= 0.5f;
	}

	// Random check
	return FMath::FRand() < Willingness;
}

void UMGAIDriverProfile::EnterBattleMode(AActor* Opponent)
{
	bInBattleMode = true;
	BattleOpponent = Opponent;

	// Entering battle slightly escalates aggression
	AccumulatedAggression = FMath::Min(AccumulatedAggression + 0.1f, 1.0f);
}

void UMGAIDriverProfile::ExitBattleMode()
{
	bInBattleMode = false;
	BattleOpponent = nullptr;
}

void UMGAIDriverProfile::ResetAggressionState()
{
	CurrentAggressionStage = EMGAggressionStage::Baseline;
	AccumulatedAggression = 0.0f;
	RecentContacts.Empty();
	CurrentGrudgeTarget = nullptr;
	TimeInAggressionStage = 0.0f;
	bInBattleMode = false;
	BattleOpponent = nullptr;
	CurrentMood = EMGAIMood::Neutral;
}

FMGPersonalityBehaviors UMGAIDriverProfile::GetEffectivePersonalityBehaviors() const
{
	FMGPersonalityBehaviors Behaviors = PersonalityBehaviors;

	// Apply personality-specific adjustments
	switch (Personality)
	{
		case EMGDriverPersonality::Aggressive:
			Behaviors.BrakePointBias = FMath::Max(Behaviors.BrakePointBias, 0.3f);
			Behaviors.PushWideTendency = FMath::Max(Behaviors.PushWideTendency, 0.4f);
			Behaviors.ChopTendency = FMath::Max(Behaviors.ChopTendency, 0.3f);
			Behaviors.SideBySideWillingness = FMath::Max(Behaviors.SideBySideWillingness, 0.7f);
			break;

		case EMGDriverPersonality::Defensive:
			Behaviors.BrakePointBias = FMath::Min(Behaviors.BrakePointBias, -0.2f);
			Behaviors.StraightLineWeaving = FMath::Max(Behaviors.StraightLineWeaving, 0.4f);
			Behaviors.SideBySideWillingness = FMath::Max(Behaviors.SideBySideWillingness, 0.6f);
			break;

		case EMGDriverPersonality::Calculated:
			Behaviors.FeintChance = FMath::Max(Behaviors.FeintChance, 0.3f);
			Behaviors.AdaptToOpponentSkill = FMath::Max(Behaviors.AdaptToOpponentSkill, 0.7f);
			Behaviors.SpecialMoveProbability = FMath::Max(Behaviors.SpecialMoveProbability, 0.2f);
			break;

		case EMGDriverPersonality::Unpredictable:
			Behaviors.StraightLineWeaving = FMath::Max(Behaviors.StraightLineWeaving, 0.6f);
			Behaviors.FeintChance = FMath::Max(Behaviors.FeintChance, 0.5f);
			Behaviors.SpecialMoveProbability = FMath::Max(Behaviors.SpecialMoveProbability, 0.4f);
			break;

		case EMGDriverPersonality::Rookie:
			Behaviors.BrakePointBias = FMath::Min(Behaviors.BrakePointBias, -0.3f);
			Behaviors.SideBySideWillingness = FMath::Min(Behaviors.SideBySideWillingness, 0.3f);
			Behaviors.AdaptToOpponentSkill = FMath::Min(Behaviors.AdaptToOpponentSkill, 0.2f);
			break;

		case EMGDriverPersonality::Veteran:
			Behaviors.AdaptToOpponentSkill = FMath::Max(Behaviors.AdaptToOpponentSkill, 0.8f);
			Behaviors.FeintChance = FMath::Max(Behaviors.FeintChance, 0.2f);
			Behaviors.SpecialMoveProbability = FMath::Max(Behaviors.SpecialMoveProbability, 0.15f);
			break;

		case EMGDriverPersonality::Rival:
			Behaviors.BrakePointBias = FMath::Max(Behaviors.BrakePointBias, 0.2f);
			Behaviors.PushWideTendency = FMath::Max(Behaviors.PushWideTendency, 0.5f);
			Behaviors.ChopTendency = FMath::Max(Behaviors.ChopTendency, 0.4f);
			Behaviors.SpecialMoveProbability = FMath::Max(Behaviors.SpecialMoveProbability, 0.3f);
			break;
	}

	// Escalation affects behaviors
	if (CurrentAggressionStage >= EMGAggressionStage::High)
	{
		Behaviors.BrakePointBias += 0.1f;
		Behaviors.PushWideTendency += 0.15f;
		Behaviors.ChopTendency += 0.1f;
	}

	if (CurrentAggressionStage == EMGAggressionStage::Rage)
	{
		// Rage makes behaviors more extreme but less controlled
		Behaviors.BrakePointBias = FMath::Clamp(Behaviors.BrakePointBias + 0.3f, -1.0f, 1.0f);
		Behaviors.SpecialMoveProbability += 0.3f;
	}

	return Behaviors;
}

bool UMGAIDriverProfile::ShouldFeint() const
{
	FMGPersonalityBehaviors Behaviors = GetEffectivePersonalityBehaviors();
	return FMath::FRand() < Behaviors.FeintChance;
}

float UMGAIDriverProfile::GetSpecialMoveProbability(bool bIsFinalLap, bool bIsForPosition) const
{
	FMGPersonalityBehaviors Behaviors = GetEffectivePersonalityBehaviors();
	float Probability = Behaviors.SpecialMoveProbability;

	// Final lap increases probability
	if (bIsFinalLap)
	{
		Probability *= 2.0f;
	}

	// Fighting for position increases probability
	if (bIsForPosition)
	{
		Probability *= 1.5f;
	}

	// Mood affects probability
	if (CurrentMood == EMGAIMood::Desperate)
	{
		Probability *= 2.0f;
	}
	else if (CurrentMood == EMGAIMood::InTheZone)
	{
		Probability *= 1.3f;
	}

	return FMath::Clamp(Probability, 0.0f, 1.0f);
}

void UMGAIDriverProfile::CleanupExpiredGrudges(float CurrentTime)
{
	RecentContacts.RemoveAll([this, CurrentTime](const FMGAIContactEvent& Contact)
	{
		return (CurrentTime - Contact.TimeStamp) > Aggression.GrudgeMemoryDuration;
	});

	// Clear grudge target if expired
	if (CurrentGrudgeTarget.IsValid())
	{
		bool bStillHasGrudge = false;
		for (const FMGAIContactEvent& Contact : RecentContacts)
		{
			if (Contact.Offender.Get() == CurrentGrudgeTarget.Get())
			{
				bStillHasGrudge = true;
				break;
			}
		}
		if (!bStillHasGrudge)
		{
			CurrentGrudgeTarget = nullptr;
		}
	}
}

EMGAggressionStage UMGAIDriverProfile::CalculateAggressionStage(float AggressionLevel) const
{
	if (AggressionLevel >= 1.0f && Aggression.bCanEnterRageMode)
	{
		return EMGAggressionStage::Rage;
	}
	else if (AggressionLevel >= Aggression.MaxAggressionThreshold)
	{
		return EMGAggressionStage::Maximum;
	}
	else if (AggressionLevel >= Aggression.HighAggressionThreshold)
	{
		return EMGAggressionStage::High;
	}
	else if (AggressionLevel >= 0.4f)
	{
		return EMGAggressionStage::Elevated;
	}
	return EMGAggressionStage::Baseline;
}

void UMGAIDriverProfile::ApplyPersonalityDefaults()
{
	// Set default behaviors based on personality type
	switch (Personality)
	{
		case EMGDriverPersonality::Aggressive:
			PersonalityBehaviors.BrakePointBias = 0.3f;
			PersonalityBehaviors.PushWideTendency = 0.4f;
			PersonalityBehaviors.ChopTendency = 0.3f;
			PersonalityBehaviors.SideBySideWillingness = 0.8f;
			PersonalityBehaviors.BumpDraftingTendency = 0.5f;
			break;

		case EMGDriverPersonality::Defensive:
			PersonalityBehaviors.BrakePointBias = -0.2f;
			PersonalityBehaviors.StraightLineWeaving = 0.5f;
			PersonalityBehaviors.LineBias = 0.0f;
			PersonalityBehaviors.SideBySideWillingness = 0.4f;
			break;

		case EMGDriverPersonality::Calculated:
			PersonalityBehaviors.AdaptToOpponentSkill = 0.8f;
			PersonalityBehaviors.FeintChance = 0.3f;
			PersonalityBehaviors.SpecialMoveProbability = 0.15f;
			break;

		case EMGDriverPersonality::Unpredictable:
			PersonalityBehaviors.StraightLineWeaving = 0.7f;
			PersonalityBehaviors.FeintChance = 0.6f;
			PersonalityBehaviors.SpecialMoveProbability = 0.4f;
			PersonalityBehaviors.BrakePointBias = FMath::FRandRange(-0.3f, 0.3f);
			break;

		case EMGDriverPersonality::Rookie:
			PersonalityBehaviors.BrakePointBias = -0.4f;
			PersonalityBehaviors.SideBySideWillingness = 0.2f;
			PersonalityBehaviors.AdaptToOpponentSkill = 0.1f;
			PersonalityBehaviors.SpecialMoveProbability = 0.05f;
			break;

		case EMGDriverPersonality::Veteran:
			PersonalityBehaviors.AdaptToOpponentSkill = 0.9f;
			PersonalityBehaviors.FeintChance = 0.25f;
			PersonalityBehaviors.SideBySideWillingness = 0.7f;
			PersonalityBehaviors.SpecialMoveProbability = 0.2f;
			break;

		case EMGDriverPersonality::Rival:
			PersonalityBehaviors.BrakePointBias = 0.25f;
			PersonalityBehaviors.PushWideTendency = 0.6f;
			PersonalityBehaviors.ChopTendency = 0.5f;
			PersonalityBehaviors.SpecialMoveProbability = 0.35f;
			break;
	}
}

// ==========================================
// UMGAIDriverRoster
// ==========================================

TArray<UMGAIDriverProfile*> UMGAIDriverRoster::GetRandomDrivers(int32 Count, float MinSkill, float MaxSkill) const
{
	TArray<UMGAIDriverProfile*> EligibleDrivers;

	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver && Driver->Skill.SkillLevel >= MinSkill && Driver->Skill.SkillLevel <= MaxSkill)
		{
			EligibleDrivers.Add(Driver);
		}
	}

	// Shuffle
	for (int32 i = EligibleDrivers.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		EligibleDrivers.Swap(i, j);
	}

	// Take requested count
	TArray<UMGAIDriverProfile*> Result;
	for (int32 i = 0; i < FMath::Min(Count, EligibleDrivers.Num()); i++)
	{
		Result.Add(EligibleDrivers[i]);
	}

	return Result;
}

TArray<UMGAIDriverProfile*> UMGAIDriverRoster::GetDriversByPersonality(EMGDriverPersonality Personality) const
{
	TArray<UMGAIDriverProfile*> Result;

	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver && Driver->Personality == Personality)
		{
			Result.Add(Driver);
		}
	}

	return Result;
}

UMGAIDriverProfile* UMGAIDriverRoster::GetDriverByName(const FString& Name) const
{
	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver && Driver->ShortName == Name)
		{
			return Driver;
		}
	}
	return nullptr;
}

TArray<UMGAIDriverProfile*> UMGAIDriverRoster::GetDriversForBracket(int32 PlayerRating, int32 Variance) const
{
	TArray<UMGAIDriverProfile*> Result;

	int32 MinRating = PlayerRating - Variance;
	int32 MaxRating = PlayerRating + Variance;

	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver)
		{
			int32 DriverRating = Driver->GetOverallRating();
			if (DriverRating >= MinRating && DriverRating <= MaxRating)
			{
				Result.Add(Driver);
			}
		}
	}

	return Result;
}
