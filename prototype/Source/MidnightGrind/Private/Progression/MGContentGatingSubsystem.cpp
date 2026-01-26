// Copyright Midnight Grind. All Rights Reserved.

#include "Progression/MGContentGatingSubsystem.h"

void UMGContentGatingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SetupDefaultContent();
}

void UMGContentGatingSubsystem::Deinitialize()
{
	PlayerStates.Empty();
	Super::Deinitialize();
}

// ==========================================
// REP MANAGEMENT
// ==========================================

void UMGContentGatingSubsystem::AddREP(FGuid PlayerID, int32 Amount, const FString& Reason)
{
	FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		InitializePlayer(PlayerID);
		State = PlayerStates.Find(PlayerID);
	}

	if (!State)
	{
		return;
	}

	EMGReputationTier OldTier = State->CurrentTier;
	State->CurrentREP += Amount;
	State->CurrentTier = CalculateTierFromREP(State->CurrentREP);

	OnREPChanged.Broadcast(PlayerID, State->CurrentREP);

	// Check for tier up
	if (State->CurrentTier != OldTier)
	{
		OnTierUnlocked.Broadcast(PlayerID, State->CurrentTier);
		UnlockTierContent(PlayerID, State->CurrentTier);
	}

	// Check for other unlocks
	CheckAndUnlockContent(PlayerID);
}

void UMGContentGatingSubsystem::RemoveREP(FGuid PlayerID, int32 Amount, const FString& Reason)
{
	FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return;
	}

	EMGReputationTier OldTier = State->CurrentTier;
	State->CurrentREP = FMath::Max(0, State->CurrentREP - Amount);
	State->CurrentTier = CalculateTierFromREP(State->CurrentREP);

	OnREPChanged.Broadcast(PlayerID, State->CurrentREP);

	// Note: We don't re-lock content on tier down per PRD - once unlocked, stays unlocked
	// Only REP display changes
}

int32 UMGContentGatingSubsystem::GetPlayerREP(FGuid PlayerID) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	return State ? State->CurrentREP : 0;
}

EMGReputationTier UMGContentGatingSubsystem::GetPlayerTier(FGuid PlayerID) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	return State ? State->CurrentTier : EMGReputationTier::Unknown;
}

int32 UMGContentGatingSubsystem::GetREPForNextTier(EMGReputationTier CurrentTier) const
{
	int32 TierIndex = static_cast<int32>(CurrentTier);
	if (TierIndex >= 5)
	{
		return -1; // Already max tier
	}
	return TierThresholds[TierIndex + 1];
}

float UMGContentGatingSubsystem::GetTierProgress(FGuid PlayerID) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return 0.0f;
	}

	int32 TierIndex = static_cast<int32>(State->CurrentTier);
	if (TierIndex >= 5)
	{
		return 1.0f; // Max tier
	}

	int32 CurrentThreshold = TierThresholds[TierIndex];
	int32 NextThreshold = TierThresholds[TierIndex + 1];

	float Progress = static_cast<float>(State->CurrentREP - CurrentThreshold) /
		static_cast<float>(NextThreshold - CurrentThreshold);

	return FMath::Clamp(Progress, 0.0f, 1.0f);
}

// ==========================================
// REP REWARDS
// ==========================================

int32 UMGContentGatingSubsystem::CalculateRaceWinREP(FName RaceType, float WinMargin, bool bCleanRace, bool bComeback, int32 OpponentCount) const
{
	// Base REP per PRD Section 4.2
	int32 BaseREP = 50;

	// Race type modifiers
	if (RaceType == TEXT("Sprint"))
	{
		BaseREP = 50;
	}
	else if (RaceType == TEXT("Circuit"))
	{
		BaseREP = 75;
	}
	else if (RaceType == TEXT("Drag"))
	{
		BaseREP = 40;
	}
	else if (RaceType == TEXT("Drift"))
	{
		BaseREP = 60;
	}
	else if (RaceType == TEXT("TimeTrial"))
	{
		BaseREP = 30;
	}
	else if (RaceType == TEXT("HighwayBattle"))
	{
		BaseREP = 100;
	}
	else if (RaceType == TEXT("Touge"))
	{
		BaseREP = 120;
	}
	else if (RaceType == TEXT("PinkSlip"))
	{
		BaseREP = 200;
	}

	// Opponent count bonus (more opponents = more REP)
	float OpponentMultiplier = 1.0f + (OpponentCount - 1) * 0.1f;

	// Dominant win bonus (+10 seconds ahead = +25%)
	float DominanceBonus = 1.0f;
	if (WinMargin >= 10.0f)
	{
		DominanceBonus = 1.25f;
	}

	// Comeback win bonus (was losing, won = +50%)
	float ComebackBonus = bComeback ? 1.5f : 1.0f;

	// Clean race bonus (no collisions = +10%)
	float CleanBonus = bCleanRace ? 1.1f : 1.0f;

	int32 FinalREP = static_cast<int32>(BaseREP * OpponentMultiplier * DominanceBonus * ComebackBonus * CleanBonus);

	return FinalREP;
}

int32 UMGContentGatingSubsystem::CalculateRaceLossREP(FName RaceType, float LossMargin) const
{
	// REP loss per PRD
	int32 BaseLoss = 10;

	// Race type modifiers
	if (RaceType == TEXT("PinkSlip"))
	{
		BaseLoss = 50; // Pink slip losses hurt more
	}
	else if (RaceType == TEXT("Touge") || RaceType == TEXT("HighwayBattle"))
	{
		BaseLoss = 25;
	}

	// Larger loss margin = more REP loss
	if (LossMargin >= 30.0f)
	{
		BaseLoss = static_cast<int32>(BaseLoss * 1.5f);
	}
	else if (LossMargin >= 10.0f)
	{
		BaseLoss = static_cast<int32>(BaseLoss * 1.25f);
	}

	return BaseLoss;
}

int32 UMGContentGatingSubsystem::CalculateBustREPLoss(int32 HeatLevel) const
{
	// REP loss scales with heat level
	switch (HeatLevel)
	{
	case 1: return 25;
	case 2: return 50;
	case 3: return 100;
	case 4: return 200;
	case 5: return 300; // Manhunt bust is devastating
	default: return 0;
	}
}

// ==========================================
// CONTENT ACCESS
// ==========================================

bool UMGContentGatingSubsystem::CanAccessLocation(FGuid PlayerID, FName LocationID) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return false;
	}

	// Check if explicitly unlocked
	if (State->UnlockedLocations.Contains(LocationID))
	{
		return true;
	}

	// Check tier requirement
	for (const FMGLocationData& Location : LocationDefinitions)
	{
		if (Location.LocationID == LocationID)
		{
			return State->CurrentTier >= Location.RequiredTier;
		}
	}

	return false;
}

bool UMGContentGatingSubsystem::CanAccessRaceType(FGuid PlayerID, FName RaceTypeID) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return false;
	}

	if (State->UnlockedRaceTypes.Contains(RaceTypeID))
	{
		return true;
	}

	for (const FMGRaceTypeUnlockData& RaceType : RaceTypeDefinitions)
	{
		if (RaceType.RaceTypeID == RaceTypeID)
		{
			// Check tier
			if (State->CurrentTier < RaceType.RequiredTier)
			{
				return false;
			}

			// Check win requirements
			int32 CircuitWins = State->RaceTypeWinCounts.FindRef(TEXT("Circuit"));
			int32 SprintWins = State->RaceTypeWinCounts.FindRef(TEXT("Sprint"));

			if (CircuitWins < RaceType.RequiredCircuitWins)
			{
				return false;
			}
			if (SprintWins < RaceType.RequiredSprintWins)
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

bool UMGContentGatingSubsystem::CanAccessPinkSlipClass(FGuid PlayerID, EMGPinkSlipClass VehicleClass) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return false;
	}

	// Per PRD Section 4.3:
	// Tier 1: No pink slips
	// Tier 2: Class D-C
	// Tier 3: Class D-B
	// Tier 4: Class D-A
	// Tier 5: All classes (D-X)

	switch (State->CurrentTier)
	{
	case EMGReputationTier::Unknown:
	case EMGReputationTier::Rookie:
		return false; // No pink slips at Tier 0-1

	case EMGReputationTier::Known: // Tier 2
		return VehicleClass <= EMGPinkSlipClass::C;

	case EMGReputationTier::Respected: // Tier 3
		return VehicleClass <= EMGPinkSlipClass::B;

	case EMGReputationTier::Feared: // Tier 4
		return VehicleClass <= EMGPinkSlipClass::A;

	case EMGReputationTier::Legend: // Tier 5
		return true; // All classes

	default:
		return false;
	}
}

bool UMGContentGatingSubsystem::IsContentUnlocked(FGuid PlayerID, FName ContentID) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return false;
	}

	// Check all unlock sets
	if (State->UnlockedLocations.Contains(ContentID))
	{
		return true;
	}
	if (State->UnlockedRaceTypes.Contains(ContentID))
	{
		return true;
	}
	if (State->UnlockedAchievements.Contains(ContentID))
	{
		return true;
	}

	// Check against gated content definitions
	for (const FMGGatedContent& Content : GatedContentDefinitions)
	{
		if (Content.ContentID == ContentID)
		{
			return State->CurrentTier >= Content.Requirement.RequiredTier &&
				State->CurrentREP >= Content.Requirement.RequiredREP;
		}
	}

	return false;
}

FMGUnlockRequirement UMGContentGatingSubsystem::GetUnlockRequirement(FName ContentID) const
{
	for (const FMGGatedContent& Content : GatedContentDefinitions)
	{
		if (Content.ContentID == ContentID)
		{
			return Content.Requirement;
		}
	}

	return FMGUnlockRequirement();
}

// ==========================================
// UNLOCKS
// ==========================================

TArray<FName> UMGContentGatingSubsystem::GetUnlockedLocations(FGuid PlayerID) const
{
	TArray<FName> Result;

	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (State)
	{
		Result = State->UnlockedLocations.Array();
	}

	return Result;
}

TArray<FName> UMGContentGatingSubsystem::GetUnlockedRaceTypes(FGuid PlayerID) const
{
	TArray<FName> Result;

	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (State)
	{
		Result = State->UnlockedRaceTypes.Array();
	}

	return Result;
}

TArray<FMGLocationData> UMGContentGatingSubsystem::GetLocationsByTier(EMGReputationTier Tier) const
{
	TArray<FMGLocationData> Result;

	for (const FMGLocationData& Location : LocationDefinitions)
	{
		if (Location.RequiredTier == Tier)
		{
			Result.Add(Location);
		}
	}

	return Result;
}

TArray<FMGGatedContent> UMGContentGatingSubsystem::GetNextUnlockableContent(FGuid PlayerID) const
{
	TArray<FMGGatedContent> Result;

	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return Result;
	}

	int32 NextTierIndex = static_cast<int32>(State->CurrentTier) + 1;
	if (NextTierIndex > 5)
	{
		return Result;
	}

	EMGReputationTier NextTier = static_cast<EMGReputationTier>(NextTierIndex);

	for (const FMGGatedContent& Content : GatedContentDefinitions)
	{
		if (Content.Requirement.RequiredTier == NextTier)
		{
			Result.Add(Content);
		}
	}

	return Result;
}

// ==========================================
// PLAYER STATE
// ==========================================

void UMGContentGatingSubsystem::InitializePlayer(FGuid PlayerID)
{
	if (PlayerStates.Contains(PlayerID))
	{
		return;
	}

	FMGPlayerUnlockState NewState;
	NewState.PlayerID = PlayerID;
	NewState.CurrentREP = 0;
	NewState.CurrentTier = EMGReputationTier::Unknown;

	// Unlock Tier 0 content
	UnlockTierContent(PlayerID, EMGReputationTier::Unknown);

	PlayerStates.Add(PlayerID, NewState);
}

bool UMGContentGatingSubsystem::GetPlayerUnlockState(FGuid PlayerID, FMGPlayerUnlockState& OutState) const
{
	const FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (State)
	{
		OutState = *State;
		return true;
	}
	return false;
}

void UMGContentGatingSubsystem::RecordRaceWin(FGuid PlayerID, FName RaceType)
{
	FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		InitializePlayer(PlayerID);
		State = PlayerStates.Find(PlayerID);
	}

	if (!State)
	{
		return;
	}

	State->TotalRaceWins++;
	State->RaceTypeWinCounts.FindOrAdd(RaceType)++;

	// Check for race-type-based unlocks
	CheckAndUnlockContent(PlayerID);
}

void UMGContentGatingSubsystem::RecordPinkSlipWin(FGuid PlayerID, EMGPinkSlipClass VehicleClass)
{
	FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return;
	}

	State->TotalPinkSlipWins++;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGContentGatingSubsystem::SetupDefaultContent()
{
	SetupLocations();
	SetupRaceTypes();
	SetupPinkSlipClasses();
}

void UMGContentGatingSubsystem::SetupLocations()
{
	// Per PRD Section 4.2:
	// Tier 0: Tutorial area only
	// Tier 1: Main city, street races
	// Tier 2: Highway/industrial
	// Tier 3: Canyon/touge
	// Tier 4: All areas
	// Tier 5: Secret locations

	FMGLocationData Tutorial;
	Tutorial.LocationID = TEXT("Tutorial");
	Tutorial.DisplayName = FText::FromString(TEXT("Training Grounds"));
	Tutorial.RequiredTier = EMGReputationTier::Unknown;
	Tutorial.AvailableRaceTypes = { TEXT("Tutorial") };
	LocationDefinitions.Add(Tutorial);

	FMGLocationData Downtown;
	Downtown.LocationID = TEXT("Downtown");
	Downtown.DisplayName = FText::FromString(TEXT("Downtown"));
	Downtown.RequiredTier = EMGReputationTier::Rookie;
	Downtown.AvailableRaceTypes = { TEXT("Sprint"), TEXT("Circuit"), TEXT("Drag") };
	LocationDefinitions.Add(Downtown);

	FMGLocationData Industrial;
	Industrial.LocationID = TEXT("Industrial");
	Industrial.DisplayName = FText::FromString(TEXT("Industrial District"));
	Industrial.RequiredTier = EMGReputationTier::Known;
	Industrial.AvailableRaceTypes = { TEXT("Sprint"), TEXT("Circuit"), TEXT("Drift") };
	LocationDefinitions.Add(Industrial);

	FMGLocationData Highway;
	Highway.LocationID = TEXT("Highway");
	Highway.DisplayName = FText::FromString(TEXT("Midnight Highway"));
	Highway.RequiredTier = EMGReputationTier::Known;
	Highway.AvailableRaceTypes = { TEXT("HighwayBattle"), TEXT("Sprint") };
	LocationDefinitions.Add(Highway);

	FMGLocationData Canyon;
	Canyon.LocationID = TEXT("Canyon");
	Canyon.DisplayName = FText::FromString(TEXT("Mountain Pass"));
	Canyon.RequiredTier = EMGReputationTier::Respected;
	Canyon.AvailableRaceTypes = { TEXT("Touge"), TEXT("Circuit"), TEXT("Drift") };
	LocationDefinitions.Add(Canyon);

	FMGLocationData Docks;
	Docks.LocationID = TEXT("Docks");
	Docks.DisplayName = FText::FromString(TEXT("Waterfront Docks"));
	Docks.RequiredTier = EMGReputationTier::Feared;
	Docks.AvailableRaceTypes = { TEXT("Sprint"), TEXT("Circuit"), TEXT("Drag"), TEXT("Drift") };
	LocationDefinitions.Add(Docks);

	FMGLocationData Airfield;
	Airfield.LocationID = TEXT("Airfield");
	Airfield.DisplayName = FText::FromString(TEXT("Abandoned Airfield"));
	Airfield.RequiredTier = EMGReputationTier::Legend;
	Airfield.AvailableRaceTypes = { TEXT("Drag"), TEXT("TopSpeed"), TEXT("Drift") };
	Airfield.bSecretLocation = true;
	LocationDefinitions.Add(Airfield);

	// Add corresponding gated content entries
	for (const FMGLocationData& Location : LocationDefinitions)
	{
		FMGGatedContent Content;
		Content.ContentID = Location.LocationID;
		Content.DisplayName = Location.DisplayName;
		Content.ContentType = EMGGatedContentType::Location;
		Content.Requirement.RequiredTier = Location.RequiredTier;
		Content.bShowWhenLocked = !Location.bSecretLocation;
		GatedContentDefinitions.Add(Content);
	}
}

void UMGContentGatingSubsystem::SetupRaceTypes()
{
	// Sprint - available from start
	FMGRaceTypeUnlockData Sprint;
	Sprint.RaceTypeID = TEXT("Sprint");
	Sprint.DisplayName = FText::FromString(TEXT("Sprint Race"));
	Sprint.RequiredTier = EMGReputationTier::Rookie;
	RaceTypeDefinitions.Add(Sprint);

	// Circuit - available from start
	FMGRaceTypeUnlockData Circuit;
	Circuit.RaceTypeID = TEXT("Circuit");
	Circuit.DisplayName = FText::FromString(TEXT("Circuit Race"));
	Circuit.RequiredTier = EMGReputationTier::Rookie;
	RaceTypeDefinitions.Add(Circuit);

	// Drag - requires some wins
	FMGRaceTypeUnlockData Drag;
	Drag.RaceTypeID = TEXT("Drag");
	Drag.DisplayName = FText::FromString(TEXT("Drag Race"));
	Drag.RequiredTier = EMGReputationTier::Rookie;
	Drag.RequiredSprintWins = 3;
	RaceTypeDefinitions.Add(Drag);

	// Time Trial
	FMGRaceTypeUnlockData TimeTrial;
	TimeTrial.RaceTypeID = TEXT("TimeTrial");
	TimeTrial.DisplayName = FText::FromString(TEXT("Time Trial"));
	TimeTrial.RequiredTier = EMGReputationTier::Rookie;
	TimeTrial.RequiredCircuitWins = 2;
	RaceTypeDefinitions.Add(TimeTrial);

	// Drift
	FMGRaceTypeUnlockData Drift;
	Drift.RaceTypeID = TEXT("Drift");
	Drift.DisplayName = FText::FromString(TEXT("Drift Session"));
	Drift.RequiredTier = EMGReputationTier::Known;
	RaceTypeDefinitions.Add(Drift);

	// Highway Battle (Wangan style)
	FMGRaceTypeUnlockData HighwayBattle;
	HighwayBattle.RaceTypeID = TEXT("HighwayBattle");
	HighwayBattle.DisplayName = FText::FromString(TEXT("Highway Battle"));
	HighwayBattle.RequiredTier = EMGReputationTier::Known;
	HighwayBattle.RequiredSprintWins = 10;
	RaceTypeDefinitions.Add(HighwayBattle);

	// Touge (Canyon Duel)
	FMGRaceTypeUnlockData Touge;
	Touge.RaceTypeID = TEXT("Touge");
	Touge.DisplayName = FText::FromString(TEXT("Touge Duel"));
	Touge.RequiredTier = EMGReputationTier::Respected;
	Touge.RequiredCircuitWins = 10;
	RaceTypeDefinitions.Add(Touge);

	// Pink Slip
	FMGRaceTypeUnlockData PinkSlip;
	PinkSlip.RaceTypeID = TEXT("PinkSlip");
	PinkSlip.DisplayName = FText::FromString(TEXT("Pink Slip"));
	PinkSlip.RequiredTier = EMGReputationTier::Known;
	RaceTypeDefinitions.Add(PinkSlip);
}

void UMGContentGatingSubsystem::SetupPinkSlipClasses()
{
	// Pink slip class access is handled dynamically in CanAccessPinkSlipClass
	// based on current tier
}

EMGReputationTier UMGContentGatingSubsystem::CalculateTierFromREP(int32 REP) const
{
	if (REP >= TierThresholds[5]) return EMGReputationTier::Legend;
	if (REP >= TierThresholds[4]) return EMGReputationTier::Feared;
	if (REP >= TierThresholds[3]) return EMGReputationTier::Respected;
	if (REP >= TierThresholds[2]) return EMGReputationTier::Known;
	if (REP >= TierThresholds[1]) return EMGReputationTier::Rookie;
	return EMGReputationTier::Unknown;
}

void UMGContentGatingSubsystem::CheckAndUnlockContent(FGuid PlayerID)
{
	FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return;
	}

	// Check all race types for win-based unlocks
	for (const FMGRaceTypeUnlockData& RaceType : RaceTypeDefinitions)
	{
		if (!State->UnlockedRaceTypes.Contains(RaceType.RaceTypeID))
		{
			if (CanAccessRaceType(PlayerID, RaceType.RaceTypeID))
			{
				State->UnlockedRaceTypes.Add(RaceType.RaceTypeID);
				OnContentUnlocked.Broadcast(PlayerID, RaceType.RaceTypeID, EMGGatedContentType::RaceType);
			}
		}
	}
}

void UMGContentGatingSubsystem::UnlockTierContent(FGuid PlayerID, EMGReputationTier Tier)
{
	FMGPlayerUnlockState* State = PlayerStates.Find(PlayerID);
	if (!State)
	{
		return;
	}

	// Unlock all locations at or below this tier
	for (const FMGLocationData& Location : LocationDefinitions)
	{
		if (Location.RequiredTier <= Tier && !State->UnlockedLocations.Contains(Location.LocationID))
		{
			State->UnlockedLocations.Add(Location.LocationID);
			OnContentUnlocked.Broadcast(PlayerID, Location.LocationID, EMGGatedContentType::Location);
		}
	}

	// Unlock race types at or below this tier (that don't have other requirements)
	for (const FMGRaceTypeUnlockData& RaceType : RaceTypeDefinitions)
	{
		if (RaceType.RequiredTier <= Tier &&
			RaceType.RequiredCircuitWins == 0 &&
			RaceType.RequiredSprintWins == 0 &&
			!State->UnlockedRaceTypes.Contains(RaceType.RaceTypeID))
		{
			State->UnlockedRaceTypes.Add(RaceType.RaceTypeID);
			OnContentUnlocked.Broadcast(PlayerID, RaceType.RaceTypeID, EMGGatedContentType::RaceType);
		}
	}
}
