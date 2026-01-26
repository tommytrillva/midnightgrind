// Copyright Epic Games, Inc. All Rights Reserved.

#include "Shortcut/MGShortcutSubsystem.h"
#include "Save/MGSaveManagerSubsystem.h"

void UMGShortcutSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadShortcutData();
}

void UMGShortcutSubsystem::Deinitialize()
{
	SaveShortcutData();
	Super::Deinitialize();
}

bool UMGShortcutSubsystem::RegisterShortcut(const FMGShortcutDefinition& Shortcut)
{
	if (Shortcut.ShortcutId.IsEmpty())
	{
		return false;
	}

	RegisteredShortcuts.Add(Shortcut.ShortcutId, Shortcut);

	// Initialize progress if not exists
	if (!ShortcutProgress.Contains(Shortcut.ShortcutId))
	{
		FMGShortcutProgress Progress;
		Progress.ShortcutId = Shortcut.ShortcutId;
		Progress.State = EMGShortcutState::Unknown;
		ShortcutProgress.Add(Shortcut.ShortcutId, Progress);
	}

	return true;
}

FMGShortcutDefinition UMGShortcutSubsystem::GetShortcut(const FString& ShortcutId) const
{
	if (const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ShortcutId))
	{
		return *Shortcut;
	}
	return FMGShortcutDefinition();
}

TArray<FMGShortcutDefinition> UMGShortcutSubsystem::GetAllShortcuts() const
{
	TArray<FMGShortcutDefinition> Result;
	RegisteredShortcuts.GenerateValueArray(Result);
	return Result;
}

TArray<FMGShortcutDefinition> UMGShortcutSubsystem::GetShortcutsForTrack(const FString& TrackId) const
{
	TArray<FMGShortcutDefinition> Result;

	for (const auto& ShortcutPair : RegisteredShortcuts)
	{
		if (ShortcutPair.Value.TrackId == TrackId)
		{
			Result.Add(ShortcutPair.Value);
		}
	}

	return Result;
}

TArray<FMGShortcutDefinition> UMGShortcutSubsystem::GetDiscoveredShortcuts() const
{
	TArray<FMGShortcutDefinition> Result;

	for (const auto& ProgressPair : ShortcutProgress)
	{
		if (ProgressPair.Value.State >= EMGShortcutState::Discovered)
		{
			if (const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ProgressPair.Key))
			{
				Result.Add(*Shortcut);
			}
		}
	}

	return Result;
}

bool UMGShortcutSubsystem::TryEnterShortcut(const FString& ShortcutId, FVector PlayerLocation, FVector PlayerVelocity)
{
	if (bInShortcut)
	{
		return false;
	}

	const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ShortcutId);
	if (!Shortcut)
	{
		return false;
	}

	// Check distance to entry
	float DistToEntry = FVector::Dist(PlayerLocation, Shortcut->Entry.Location);
	if (DistToEntry > Shortcut->Entry.TriggerRadius)
	{
		return false;
	}

	// Check speed requirements
	float Speed = PlayerVelocity.Size() * 0.036f; // Convert to km/h
	if (Speed < Shortcut->Entry.MinSpeed || Speed > Shortcut->Entry.MaxSpeed)
	{
		return false;
	}

	// Check approach angle
	FVector ApproachDir = PlayerVelocity.GetSafeNormal();
	FVector RequiredDir = Shortcut->Entry.RequiredApproach.Vector();
	float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ApproachDir, RequiredDir)));
	if (AngleDiff > Shortcut->Entry.ApproachTolerance)
	{
		return false;
	}

	// Check breakable requirement
	if (Shortcut->Entry.bRequiresBreakable)
	{
		if (!IsObstacleBroken(Shortcut->Entry.BreakableId))
		{
			return false;
		}
	}

	// Check unlock requirement
	if (Shortcut->bRequiresUnlock)
	{
		// TODO: Check unlock condition
	}

	// Start shortcut
	bInShortcut = true;
	ActiveAttempt = FMGActiveShortcutAttempt();
	ActiveAttempt.ShortcutId = ShortcutId;
	ActiveAttempt.EntrySpeed = Speed;
	ActiveAttempt.CurrentWaypoint = 0;
	ActiveAttempt.bIsValid = true;
	ActiveAttempt.StartTime = FDateTime::Now();

	// Discover if not yet discovered
	if (GetShortcutState(ShortcutId) < EMGShortcutState::Discovered)
	{
		DiscoverShortcut(ShortcutId);
	}

	OnShortcutEntered.Broadcast(ShortcutId, Speed);
	return true;
}

void UMGShortcutSubsystem::UpdateActiveShortcut(FVector PlayerLocation, float DeltaTime)
{
	if (!bInShortcut)
	{
		return;
	}

	ActiveAttempt.ElapsedTime += DeltaTime;

	// Check waypoint progress
	CheckWaypointProgress(PlayerLocation);

	// Check if reached exit
	const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ActiveAttempt.ShortcutId);
	if (Shortcut)
	{
		float DistToExit = FVector::Dist(PlayerLocation, Shortcut->Exit.Location);
		if (DistToExit <= Shortcut->Exit.TriggerRadius)
		{
			ExitShortcut(true);
		}
	}
}

void UMGShortcutSubsystem::ExitShortcut(bool bSuccessful)
{
	if (!bInShortcut)
	{
		return;
	}

	const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ActiveAttempt.ShortcutId);
	FMGShortcutProgress* Progress = ShortcutProgress.Find(ActiveAttempt.ShortcutId);

	if (bSuccessful && Shortcut && Progress)
	{
		Progress->TimesUsed++;
		Progress->SuccessfulRuns++;
		Progress->LastUsed = FDateTime::Now();

		if (Progress->State < EMGShortcutState::Used)
		{
			Progress->State = EMGShortcutState::Used;
		}

		float TimeSaved = CalculateTimeSaved(*Shortcut, ActiveAttempt.ElapsedTime);
		Progress->TotalTimeSaved += TimeSaved;

		if (Progress->BestTime == 0.0f || ActiveAttempt.ElapsedTime < Progress->BestTime)
		{
			Progress->BestTime = ActiveAttempt.ElapsedTime;
		}

		// Update session stats
		SessionStats.ShortcutsUsed++;
		SessionStats.TotalTimeSaved += TimeSaved;
		SessionStats.TotalPoints += Shortcut->UsePoints;

		int32& TypeCount = SessionStats.UsageByType.FindOrAdd(Shortcut->Type);
		TypeCount++;

		// Check for mastery
		CheckMastery(ActiveAttempt.ShortcutId);

		OnShortcutCompleted.Broadcast(ActiveAttempt.ShortcutId, ActiveAttempt.ElapsedTime, TimeSaved);
	}
	else if (!bSuccessful && Progress)
	{
		Progress->FailedRuns++;
		SessionStats.ShortcutsFailed++;
	}

	bInShortcut = false;
	ActiveAttempt = FMGActiveShortcutAttempt();
}

void UMGShortcutSubsystem::FailShortcut(const FString& Reason)
{
	if (!bInShortcut)
	{
		return;
	}

	ActiveAttempt.bIsValid = false;
	OnShortcutFailed.Broadcast(ActiveAttempt.ShortcutId, Reason);
	ExitShortcut(false);
}

bool UMGShortcutSubsystem::IsInShortcut() const
{
	return bInShortcut;
}

FMGActiveShortcutAttempt UMGShortcutSubsystem::GetActiveAttempt() const
{
	return ActiveAttempt;
}

FString UMGShortcutSubsystem::GetActiveShortcutId() const
{
	return bInShortcut ? ActiveAttempt.ShortcutId : FString();
}

void UMGShortcutSubsystem::DiscoverShortcut(const FString& ShortcutId)
{
	FMGShortcutProgress* Progress = ShortcutProgress.Find(ShortcutId);
	const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ShortcutId);

	if (!Progress || !Shortcut)
	{
		return;
	}

	if (Progress->State < EMGShortcutState::Discovered)
	{
		Progress->State = EMGShortcutState::Discovered;
		Progress->FirstDiscovered = FDateTime::Now();

		SessionStats.ShortcutsDiscovered++;
		SessionStats.TotalPoints += Shortcut->DiscoveryPoints;

		if (Shortcut->bIsSecret)
		{
			SessionStats.SecretsFound++;
			OnSecretShortcutFound.Broadcast(ShortcutId, Shortcut->DiscoveryPoints * 2);
		}

		OnShortcutDiscovered.Broadcast(ShortcutId, Shortcut->DiscoveryPoints);
	}
}

void UMGShortcutSubsystem::HintShortcut(const FString& ShortcutId)
{
	FMGShortcutProgress* Progress = ShortcutProgress.Find(ShortcutId);

	if (Progress && Progress->State == EMGShortcutState::Unknown)
	{
		Progress->State = EMGShortcutState::Hinted;
	}
}

bool UMGShortcutSubsystem::IsShortcutDiscovered(const FString& ShortcutId) const
{
	return GetShortcutState(ShortcutId) >= EMGShortcutState::Discovered;
}

EMGShortcutState UMGShortcutSubsystem::GetShortcutState(const FString& ShortcutId) const
{
	if (const FMGShortcutProgress* Progress = ShortcutProgress.Find(ShortcutId))
	{
		return Progress->State;
	}
	return EMGShortcutState::Unknown;
}

FMGShortcutProgress UMGShortcutSubsystem::GetShortcutProgress(const FString& ShortcutId) const
{
	if (const FMGShortcutProgress* Progress = ShortcutProgress.Find(ShortcutId))
	{
		return *Progress;
	}
	return FMGShortcutProgress();
}

float UMGShortcutSubsystem::GetDiscoveryPercent(const FString& TrackId) const
{
	int32 Total = 0;
	int32 Discovered = 0;

	for (const auto& ShortcutPair : RegisteredShortcuts)
	{
		if (ShortcutPair.Value.TrackId == TrackId)
		{
			Total++;

			if (const FMGShortcutProgress* Progress = ShortcutProgress.Find(ShortcutPair.Key))
			{
				if (Progress->State >= EMGShortcutState::Discovered)
				{
					Discovered++;
				}
			}
		}
	}

	return Total > 0 ? (static_cast<float>(Discovered) / static_cast<float>(Total)) * 100.0f : 0.0f;
}

void UMGShortcutSubsystem::RegisterHint(const FMGShortcutHint& Hint)
{
	if (!Hint.ShortcutId.IsEmpty())
	{
		ShortcutHints.Add(Hint.ShortcutId, Hint);
	}
}

TArray<FMGShortcutHint> UMGShortcutSubsystem::GetActiveHints() const
{
	TArray<FMGShortcutHint> Result;

	for (const auto& HintPair : ShortcutHints)
	{
		if (GetShortcutState(HintPair.Key) == EMGShortcutState::Hinted)
		{
			Result.Add(HintPair.Value);
		}
	}

	return Result;
}

FMGShortcutHint UMGShortcutSubsystem::GetNearestHint(FVector Location) const
{
	FMGShortcutHint Nearest;
	float NearestDist = FLT_MAX;

	for (const auto& HintPair : ShortcutHints)
	{
		if (GetShortcutState(HintPair.Key) == EMGShortcutState::Hinted)
		{
			float Dist = FVector::Dist(Location, HintPair.Value.HintLocation);
			if (Dist < NearestDist)
			{
				NearestDist = Dist;
				Nearest = HintPair.Value;
			}
		}
	}

	return Nearest;
}

void UMGShortcutSubsystem::RevealHint(const FString& ShortcutId)
{
	if (GetShortcutState(ShortcutId) == EMGShortcutState::Unknown)
	{
		HintShortcut(ShortcutId);

		if (const FMGShortcutHint* Hint = ShortcutHints.Find(ShortcutId))
		{
			OnShortcutHintRevealed.Broadcast(ShortcutId, *Hint);
		}
	}
}

void UMGShortcutSubsystem::RegisterBreakable(const FMGBreakableObstacle& Obstacle)
{
	if (!Obstacle.ObstacleId.IsEmpty())
	{
		Breakables.Add(Obstacle.ObstacleId, Obstacle);
	}
}

bool UMGShortcutSubsystem::TryBreakObstacle(const FString& ObstacleId, float ImpactSpeed)
{
	FMGBreakableObstacle* Obstacle = Breakables.Find(ObstacleId);
	if (!Obstacle || Obstacle->bIsBroken)
	{
		return false;
	}

	if (ImpactSpeed >= Obstacle->MinBreakSpeed)
	{
		Obstacle->bIsBroken = true;
		Obstacle->BrokenTimer = Obstacle->RespawnTime;
		Obstacle->TimesDestroyed++;

		SessionStats.BreakablesDestroyed++;

		int32 Points = 50; // Base points for breaking
		OnBreakableDestroyed.Broadcast(ObstacleId, Points);

		return true;
	}

	return false;
}

void UMGShortcutSubsystem::UpdateBreakables(float DeltaTime)
{
	for (auto& BreakablePair : Breakables)
	{
		if (BreakablePair.Value.bIsBroken)
		{
			BreakablePair.Value.BrokenTimer -= DeltaTime;
			if (BreakablePair.Value.BrokenTimer <= 0.0f)
			{
				BreakablePair.Value.bIsBroken = false;
				BreakablePair.Value.BrokenTimer = 0.0f;
			}
		}
	}
}

bool UMGShortcutSubsystem::IsObstacleBroken(const FString& ObstacleId) const
{
	if (const FMGBreakableObstacle* Obstacle = Breakables.Find(ObstacleId))
	{
		return Obstacle->bIsBroken;
	}
	return true; // If not registered, assume it's passable
}

FMGBreakableObstacle UMGShortcutSubsystem::GetBreakable(const FString& ObstacleId) const
{
	if (const FMGBreakableObstacle* Obstacle = Breakables.Find(ObstacleId))
	{
		return *Obstacle;
	}
	return FMGBreakableObstacle();
}

FString UMGShortcutSubsystem::GetNearestShortcutEntry(FVector Location, float MaxDistance) const
{
	FString NearestId;
	float NearestDist = MaxDistance;

	for (const auto& ShortcutPair : RegisteredShortcuts)
	{
		float Dist = FVector::Dist(Location, ShortcutPair.Value.Entry.Location);
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			NearestId = ShortcutPair.Key;
		}
	}

	return NearestId;
}

bool UMGShortcutSubsystem::IsNearShortcutEntry(FVector Location, FString& OutShortcutId) const
{
	for (const auto& ShortcutPair : RegisteredShortcuts)
	{
		float Dist = FVector::Dist(Location, ShortcutPair.Value.Entry.Location);
		if (Dist <= ShortcutPair.Value.Entry.TriggerRadius * 2.0f)
		{
			OutShortcutId = ShortcutPair.Key;
			return true;
		}
	}

	return false;
}

TArray<FString> UMGShortcutSubsystem::GetShortcutsInRange(FVector Location, float Range) const
{
	TArray<FString> Result;

	for (const auto& ShortcutPair : RegisteredShortcuts)
	{
		float Dist = FVector::Dist(Location, ShortcutPair.Value.Entry.Location);
		if (Dist <= Range)
		{
			Result.Add(ShortcutPair.Key);
		}
	}

	return Result;
}

FMGShortcutSessionStats UMGShortcutSubsystem::GetSessionStats() const
{
	return SessionStats;
}

int32 UMGShortcutSubsystem::GetTotalDiscoveredCount() const
{
	int32 Count = 0;
	for (const auto& ProgressPair : ShortcutProgress)
	{
		if (ProgressPair.Value.State >= EMGShortcutState::Discovered)
		{
			Count++;
		}
	}
	return Count;
}

int32 UMGShortcutSubsystem::GetTotalMasteredCount() const
{
	int32 Count = 0;
	for (const auto& ProgressPair : ShortcutProgress)
	{
		if (ProgressPair.Value.State >= EMGShortcutState::Mastered)
		{
			Count++;
		}
	}
	return Count;
}

float UMGShortcutSubsystem::GetTotalTimeSaved() const
{
	float Total = 0.0f;
	for (const auto& ProgressPair : ShortcutProgress)
	{
		Total += ProgressPair.Value.TotalTimeSaved;
	}
	return Total;
}

void UMGShortcutSubsystem::StartSession()
{
	bSessionActive = true;
	SessionStats = FMGShortcutSessionStats();

	// Reset breakables
	for (auto& BreakablePair : Breakables)
	{
		BreakablePair.Value.bIsBroken = false;
		BreakablePair.Value.BrokenTimer = 0.0f;
	}
}

void UMGShortcutSubsystem::EndSession()
{
	if (bInShortcut)
	{
		ExitShortcut(false);
	}

	bSessionActive = false;
	SaveShortcutData();
}

bool UMGShortcutSubsystem::IsSessionActive() const
{
	return bSessionActive;
}

FText UMGShortcutSubsystem::GetShortcutTypeDisplayName(EMGShortcutType Type) const
{
	switch (Type)
	{
		case EMGShortcutType::Alley: return FText::FromString(TEXT("Alley"));
		case EMGShortcutType::Tunnel: return FText::FromString(TEXT("Tunnel"));
		case EMGShortcutType::JumpRamp: return FText::FromString(TEXT("Jump Ramp"));
		case EMGShortcutType::Rooftop: return FText::FromString(TEXT("Rooftop"));
		case EMGShortcutType::Underground: return FText::FromString(TEXT("Underground"));
		case EMGShortcutType::Breakable: return FText::FromString(TEXT("Breakable"));
		case EMGShortcutType::Hidden: return FText::FromString(TEXT("Hidden Path"));
		case EMGShortcutType::Risky: return FText::FromString(TEXT("Risky Route"));
		case EMGShortcutType::Scenic: return FText::FromString(TEXT("Scenic Route"));
		case EMGShortcutType::Technical: return FText::FromString(TEXT("Technical"));
		case EMGShortcutType::Secret: return FText::FromString(TEXT("Secret"));
		default: return FText::FromString(TEXT("Shortcut"));
	}
}

FText UMGShortcutSubsystem::GetDifficultyDisplayName(EMGShortcutDifficulty Difficulty) const
{
	switch (Difficulty)
	{
		case EMGShortcutDifficulty::Easy: return FText::FromString(TEXT("Easy"));
		case EMGShortcutDifficulty::Medium: return FText::FromString(TEXT("Medium"));
		case EMGShortcutDifficulty::Hard: return FText::FromString(TEXT("Hard"));
		case EMGShortcutDifficulty::Expert: return FText::FromString(TEXT("Expert"));
		case EMGShortcutDifficulty::Insane: return FText::FromString(TEXT("Insane"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

FLinearColor UMGShortcutSubsystem::GetDifficultyColor(EMGShortcutDifficulty Difficulty) const
{
	switch (Difficulty)
	{
		case EMGShortcutDifficulty::Easy: return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
		case EMGShortcutDifficulty::Medium: return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
		case EMGShortcutDifficulty::Hard: return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
		case EMGShortcutDifficulty::Expert: return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
		case EMGShortcutDifficulty::Insane: return FLinearColor(0.5f, 0.0f, 0.5f, 1.0f);
		default: return FLinearColor::White;
	}
}

void UMGShortcutSubsystem::SaveShortcutData()
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

void UMGShortcutSubsystem::LoadShortcutData()
{
	// Load shortcut data from central save manager
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			if (const UMGSaveGame* SaveData = SaveManager->GetCurrentSaveData())
			{
				// Restore discovered shortcuts
				for (const FName& ShortcutName : SaveData->ShortcutData.DiscoveredShortcuts)
				{
					FMGShortcutProgress Progress;
					Progress.bDiscovered = true;
					ShortcutProgress.Add(ShortcutName.ToString(), Progress);
				}
				// Restore session stats
				SessionStats.TotalShortcutsUsed = SaveData->ShortcutData.TotalShortcutsUsed;
				SessionStats.TotalTimeSaved = SaveData->ShortcutData.TotalTimeSaved;
				SessionStats.SecretShortcutsFound = SaveData->ShortcutData.SecretShortcutsFound;
				UE_LOG(LogTemp, Log, TEXT("ShortcutSubsystem: Loaded shortcut data - Discovered: %d, Used: %d"),
					SaveData->ShortcutData.DiscoveredShortcuts.Num(), SaveData->ShortcutData.TotalShortcutsUsed);
			}
		}
	}
}

void UMGShortcutSubsystem::CheckWaypointProgress(FVector PlayerLocation)
{
	const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ActiveAttempt.ShortcutId);
	if (!Shortcut || ActiveAttempt.CurrentWaypoint >= Shortcut->Waypoints.Num())
	{
		return;
	}

	const FMGShortcutWaypoint& CurrentWP = Shortcut->Waypoints[ActiveAttempt.CurrentWaypoint];
	float DistToWaypoint = FVector::Dist(PlayerLocation, CurrentWP.Location);

	if (DistToWaypoint <= CurrentWP.Radius)
	{
		OnWaypointReached.Broadcast(ActiveAttempt.CurrentWaypoint, ActiveAttempt.ElapsedTime);
		ActiveAttempt.CurrentWaypoint++;
	}
	else if (CurrentWP.bIsCritical)
	{
		// Check if we've passed the waypoint without hitting it
		// Simple check: if we're getting farther away and past the waypoint
		if (ActiveAttempt.CurrentWaypoint > 0)
		{
			const FMGShortcutWaypoint& PrevWP = Shortcut->Waypoints[ActiveAttempt.CurrentWaypoint - 1];
			float DistToPrev = FVector::Dist(PlayerLocation, PrevWP.Location);
			float WPToPrev = FVector::Dist(CurrentWP.Location, PrevWP.Location);

			if (DistToPrev > WPToPrev && DistToWaypoint > CurrentWP.Radius * 3.0f)
			{
				ActiveAttempt.WaypointsMissed++;

				if (CurrentWP.bIsCritical)
				{
					FailShortcut(TEXT("Missed critical waypoint"));
				}
			}
		}
	}
}

void UMGShortcutSubsystem::CheckMastery(const FString& ShortcutId)
{
	const FMGShortcutDefinition* Shortcut = RegisteredShortcuts.Find(ShortcutId);
	FMGShortcutProgress* Progress = ShortcutProgress.Find(ShortcutId);

	if (!Shortcut || !Progress)
	{
		return;
	}

	if (Progress->State < EMGShortcutState::Mastered && Progress->SuccessfulRuns >= Shortcut->MasteryUses)
	{
		Progress->State = EMGShortcutState::Mastered;

		int32 BonusPoints = Shortcut->DiscoveryPoints * 3;
		SessionStats.TotalPoints += BonusPoints;

		OnShortcutMastered.Broadcast(ShortcutId, BonusPoints);
	}
}

float UMGShortcutSubsystem::CalculateTimeSaved(const FMGShortcutDefinition& Shortcut, float ActualTime) const
{
	// Estimate regular path time based on shortcut's estimated time saved
	float EstimatedRegularTime = ActualTime + Shortcut.EstimatedTimeSaved;

	// Calculate actual time saved
	float TimeSaved = EstimatedRegularTime - ActualTime;

	return FMath::Max(0.0f, TimeSaved);
}
