// Copyright Epic Games, Inc. All Rights Reserved.

#include "Speedtrap/MGSpeedtrapSubsystem.h"
#include "TimerManager.h"

void UMGSpeedtrapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	AttemptCounter = 0;

	// Set up default speed zone config
	SpeedZoneConfig.MinSpeedMPH = 50.0f;
	SpeedZoneConfig.SpeedDecayRate = 10.0f;
	SpeedZoneConfig.ComboMultiplierPerZone = 0.1f;
	SpeedZoneConfig.MaxComboMultiplier = 3.0f;
	SpeedZoneConfig.NearMissBonusPercent = 10.0f;
	SpeedZoneConfig.DriftBonusPercent = 15.0f;
	SpeedZoneConfig.OvertakeBonusPercent = 20.0f;

	// Initialize player stats
	PlayerStats = FMGSpeedtrapPlayerStats();

	LoadSpeedtrapData();
}

void UMGSpeedtrapSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpeedtrapTickTimer);
	}

	SaveSpeedtrapData();
	Super::Deinitialize();
}

// ============================================================================
// Registration
// ============================================================================

void UMGSpeedtrapSubsystem::RegisterSpeedtrap(const FMGSpeedtrapDefinition& Definition)
{
	if (Definition.SpeedtrapId.IsEmpty())
	{
		return;
	}

	RegisteredSpeedtraps.Add(Definition.SpeedtrapId, Definition);
}

void UMGSpeedtrapSubsystem::UnregisterSpeedtrap(const FString& SpeedtrapId)
{
	RegisteredSpeedtraps.Remove(SpeedtrapId);
}

FMGSpeedtrapDefinition UMGSpeedtrapSubsystem::GetSpeedtrap(const FString& SpeedtrapId) const
{
	if (const FMGSpeedtrapDefinition* Found = RegisteredSpeedtraps.Find(SpeedtrapId))
	{
		return *Found;
	}
	return FMGSpeedtrapDefinition();
}

TArray<FMGSpeedtrapDefinition> UMGSpeedtrapSubsystem::GetAllSpeedtraps() const
{
	TArray<FMGSpeedtrapDefinition> Result;
	for (const auto& Pair : RegisteredSpeedtraps)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

TArray<FMGSpeedtrapDefinition> UMGSpeedtrapSubsystem::GetSpeedtrapsInArea(FVector Center, float Radius) const
{
	TArray<FMGSpeedtrapDefinition> Result;

	for (const auto& Pair : RegisteredSpeedtraps)
	{
		float Distance = FVector::Dist(Center, Pair.Value.StartLocation);
		if (Distance <= Radius)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGSpeedtrapDefinition> UMGSpeedtrapSubsystem::GetSpeedtrapsForTrack(const FString& TrackId) const
{
	TArray<FMGSpeedtrapDefinition> Result;

	for (const auto& Pair : RegisteredSpeedtraps)
	{
		if (Pair.Value.TrackId == TrackId)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

// ============================================================================
// Detection
// ============================================================================

void UMGSpeedtrapSubsystem::UpdateSpeedtrapDetection(const FString& PlayerId, FVector Location, FVector Velocity, float DeltaTime)
{
	float CurrentSpeedCmPerSec = Velocity.Size();
	float CurrentSpeedMPH = ConvertToMPH(CurrentSpeedCmPerSec);

	// Check for entry into any speedtrap
	for (const auto& Pair : RegisteredSpeedtraps)
	{
		const FMGSpeedtrapDefinition& Speedtrap = Pair.Value;

		// Skip if already active
		if (ActiveSpeedtraps.Contains(PlayerId) &&
		    ActiveSpeedtraps[PlayerId].SpeedtrapId == Speedtrap.SpeedtrapId)
		{
			continue;
		}

		if (IsInSpeedtrapTrigger(Location, Speedtrap))
		{
			TryEnterSpeedtrap(PlayerId, Speedtrap.SpeedtrapId, CurrentSpeedMPH, Velocity);
		}
	}

	// Update active speedtrap if any
	if (HasActiveSpeedtrap(PlayerId))
	{
		UpdateActiveZone(PlayerId, CurrentSpeedMPH, DeltaTime);

		// Check for exit
		FMGActiveSpeedtrap& Active = ActiveSpeedtraps[PlayerId];
		const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(Active.SpeedtrapId);

		if (Def)
		{
			// For camera type, recording happens immediately at trigger
			if (Def->Type == EMGSpeedtrapType::Camera)
			{
				// Camera already recorded on entry
			}
			// For zone types, check if exited zone
			else if (Def->ZoneLength > 0.0f)
			{
				if (!IsInSpeedtrapZone(Location, *Def))
				{
					ExitSpeedtrap(PlayerId, Active.SpeedtrapId, CurrentSpeedMPH);
				}
			}
		}
	}
}

bool UMGSpeedtrapSubsystem::TryEnterSpeedtrap(const FString& PlayerId, const FString& SpeedtrapId, float Speed, FVector Velocity)
{
	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
	if (!Def)
	{
		return false;
	}

	// Check direction requirement
	if (Def->bRequiresDirection)
	{
		FVector VelocityDir = Velocity.GetSafeNormal();
		FVector RequiredDir = Def->Rotation.RotateVector(Def->RequiredDirection);

		float DotProduct = FVector::DotProduct(VelocityDir, RequiredDir);
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

		if (AngleDegrees > Def->DirectionTolerance)
		{
			return false;
		}
	}

	// Discover speedtrap if not already discovered
	if (!IsSpeedtrapDiscovered(SpeedtrapId))
	{
		DiscoverSpeedtrap(SpeedtrapId);
	}

	// Create active state
	FMGActiveSpeedtrap Active;
	Active.SpeedtrapId = SpeedtrapId;
	Active.PlayerId = PlayerId;
	Active.State = EMGSpeedtrapState::Active;
	Active.EntrySpeed = Speed;
	Active.CurrentValue = Speed;
	Active.MaxValue = Speed;
	Active.TimeElapsed = 0.0f;

	if (Def->bHasTimeLimit)
	{
		Active.TimeRemaining = Def->TimeLimit;
	}

	if (Def->ZoneLength > 0.0f)
	{
		Active.DistanceRemaining = Def->ZoneLength;
		Active.State = EMGSpeedtrapState::InProgress;
	}

	Active.SampleCount = 1;
	Active.SpeedSum = Speed;
	Active.CurrentRating = CalculateRating(SpeedtrapId, Speed);

	ActiveSpeedtraps.Add(PlayerId, Active);

	OnSpeedtrapEntered.Broadcast(SpeedtrapId, PlayerId, Speed);

	// For camera type, record immediately
	if (Def->Type == EMGSpeedtrapType::Camera)
	{
		RecordCameraSpeed(PlayerId, SpeedtrapId, Speed);
	}

	return true;
}

void UMGSpeedtrapSubsystem::ExitSpeedtrap(const FString& PlayerId, const FString& SpeedtrapId, float ExitSpeed)
{
	FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId);
	if (!Active || Active->SpeedtrapId != SpeedtrapId)
	{
		return;
	}

	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
	if (!Def)
	{
		ActiveSpeedtraps.Remove(PlayerId);
		return;
	}

	bool bCompleted = true;
	FMGSpeedtrapAttempt Attempt;

	// Determine if completed based on type
	switch (Def->Type)
	{
		case EMGSpeedtrapType::Zone:
		case EMGSpeedtrapType::Average:
			// Need to complete the full zone
			if (Active->DistanceRemaining > 0.0f)
			{
				bCompleted = false;
			}
			else
			{
				Attempt = RecordZoneCompletion(PlayerId, SpeedtrapId);
			}
			break;

		case EMGSpeedtrapType::TopSpeed:
			// Record max speed achieved
			Attempt = FinalizeAttempt(PlayerId, SpeedtrapId, Active->MaxValue);
			break;

		default:
			Attempt = FinalizeAttempt(PlayerId, SpeedtrapId, Active->CurrentValue);
			break;
	}

	if (!bCompleted)
	{
		Attempt.PlayerId = PlayerId;
		Attempt.SpeedtrapId = SpeedtrapId;
		Attempt.Rating = EMGSpeedtrapRating::None;
	}

	Attempt.ExitSpeed = ExitSpeed;

	OnSpeedtrapExited.Broadcast(SpeedtrapId, Attempt, bCompleted);

	ActiveSpeedtraps.Remove(PlayerId);
}

bool UMGSpeedtrapSubsystem::IsInSpeedtrapTrigger(FVector Location, const FMGSpeedtrapDefinition& Speedtrap) const
{
	// Transform location to speedtrap space
	FVector LocalLocation = Location - Speedtrap.StartLocation;
	LocalLocation = Speedtrap.Rotation.UnrotateVector(LocalLocation);

	// Check if within trigger plane
	bool bWithinX = FMath::Abs(LocalLocation.X) <= 100.0f; // Thin trigger plane
	bool bWithinY = FMath::Abs(LocalLocation.Y) <= Speedtrap.TriggerWidth;
	bool bWithinZ = FMath::Abs(LocalLocation.Z) <= Speedtrap.TriggerHeight;

	return bWithinX && bWithinY && bWithinZ;
}

bool UMGSpeedtrapSubsystem::IsInSpeedtrapZone(FVector Location, const FMGSpeedtrapDefinition& Speedtrap) const
{
	if (Speedtrap.ZoneLength <= 0.0f)
	{
		return IsInSpeedtrapTrigger(Location, Speedtrap);
	}

	// Transform location to speedtrap space
	FVector LocalLocation = Location - Speedtrap.StartLocation;
	LocalLocation = Speedtrap.Rotation.UnrotateVector(LocalLocation);

	// Check if within zone boundaries
	bool bWithinX = LocalLocation.X >= 0.0f && LocalLocation.X <= Speedtrap.ZoneLength;
	bool bWithinY = FMath::Abs(LocalLocation.Y) <= Speedtrap.TriggerWidth;
	bool bWithinZ = FMath::Abs(LocalLocation.Z) <= Speedtrap.TriggerHeight;

	return bWithinX && bWithinY && bWithinZ;
}

// ============================================================================
// Active State
// ============================================================================

FMGActiveSpeedtrap UMGSpeedtrapSubsystem::GetActiveSpeedtrap(const FString& PlayerId) const
{
	if (const FMGActiveSpeedtrap* Found = ActiveSpeedtraps.Find(PlayerId))
	{
		return *Found;
	}
	return FMGActiveSpeedtrap();
}

bool UMGSpeedtrapSubsystem::HasActiveSpeedtrap(const FString& PlayerId) const
{
	return ActiveSpeedtraps.Contains(PlayerId);
}

float UMGSpeedtrapSubsystem::GetCurrentSpeed(const FString& PlayerId) const
{
	if (const FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId))
	{
		return Active->CurrentValue;
	}
	return 0.0f;
}

EMGSpeedtrapRating UMGSpeedtrapSubsystem::GetCurrentRating(const FString& PlayerId) const
{
	if (const FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId))
	{
		return Active->CurrentRating;
	}
	return EMGSpeedtrapRating::None;
}

void UMGSpeedtrapSubsystem::CancelActiveSpeedtrap(const FString& PlayerId)
{
	if (FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId))
	{
		FMGSpeedtrapAttempt FailedAttempt;
		FailedAttempt.PlayerId = PlayerId;
		FailedAttempt.SpeedtrapId = Active->SpeedtrapId;
		FailedAttempt.Rating = EMGSpeedtrapRating::None;

		OnSpeedtrapExited.Broadcast(Active->SpeedtrapId, FailedAttempt, false);
		ActiveSpeedtraps.Remove(PlayerId);
	}
}

// ============================================================================
// Recording
// ============================================================================

FMGSpeedtrapAttempt UMGSpeedtrapSubsystem::RecordCameraSpeed(const FString& PlayerId, const FString& SpeedtrapId, float Speed)
{
	return FinalizeAttempt(PlayerId, SpeedtrapId, Speed);
}

FMGSpeedtrapAttempt UMGSpeedtrapSubsystem::RecordZoneCompletion(const FString& PlayerId, const FString& SpeedtrapId)
{
	FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId);
	if (!Active || Active->SpeedtrapId != SpeedtrapId)
	{
		return FMGSpeedtrapAttempt();
	}

	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
	if (!Def)
	{
		return FMGSpeedtrapAttempt();
	}

	// Calculate final value based on type
	float RecordedValue = 0.0f;

	switch (Def->Type)
	{
		case EMGSpeedtrapType::Zone:
		case EMGSpeedtrapType::TopSpeed:
			RecordedValue = Active->MaxValue;
			break;

		case EMGSpeedtrapType::Average:
			if (Active->SampleCount > 0)
			{
				RecordedValue = Active->SpeedSum / Active->SampleCount;
			}
			break;

		default:
			RecordedValue = Active->CurrentValue;
			break;
	}

	return FinalizeAttempt(PlayerId, SpeedtrapId, RecordedValue);
}

FMGSpeedtrapAttempt UMGSpeedtrapSubsystem::RecordJumpDistance(const FString& PlayerId, const FString& SpeedtrapId, float Distance)
{
	return FinalizeAttempt(PlayerId, SpeedtrapId, Distance);
}

// ============================================================================
// Rating Calculation
// ============================================================================

EMGSpeedtrapRating UMGSpeedtrapSubsystem::CalculateRating(const FString& SpeedtrapId, float Value) const
{
	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
	if (!Def)
	{
		return EMGSpeedtrapRating::None;
	}

	if (Value >= Def->LegendThreshold)
	{
		return EMGSpeedtrapRating::Legend;
	}
	else if (Value >= Def->DiamondThreshold)
	{
		return EMGSpeedtrapRating::Diamond;
	}
	else if (Value >= Def->PlatinumThreshold)
	{
		return EMGSpeedtrapRating::Platinum;
	}
	else if (Value >= Def->GoldThreshold)
	{
		return EMGSpeedtrapRating::Gold;
	}
	else if (Value >= Def->SilverThreshold)
	{
		return EMGSpeedtrapRating::Silver;
	}
	else if (Value >= Def->BronzeThreshold)
	{
		return EMGSpeedtrapRating::Bronze;
	}

	return EMGSpeedtrapRating::None;
}

int32 UMGSpeedtrapSubsystem::GetPointsForRating(const FString& SpeedtrapId, EMGSpeedtrapRating Rating) const
{
	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
	if (!Def)
	{
		return 0;
	}

	switch (Rating)
	{
		case EMGSpeedtrapRating::Bronze:
			return Def->BronzePoints;
		case EMGSpeedtrapRating::Silver:
			return Def->SilverPoints;
		case EMGSpeedtrapRating::Gold:
			return Def->GoldPoints;
		case EMGSpeedtrapRating::Platinum:
			return Def->PlatinumPoints;
		case EMGSpeedtrapRating::Diamond:
			return Def->DiamondPoints;
		case EMGSpeedtrapRating::Legend:
			return Def->LegendPoints;
		default:
			return 0;
	}
}

float UMGSpeedtrapSubsystem::GetThresholdForRating(const FString& SpeedtrapId, EMGSpeedtrapRating Rating) const
{
	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
	if (!Def)
	{
		return 0.0f;
	}

	switch (Rating)
	{
		case EMGSpeedtrapRating::Bronze:
			return Def->BronzeThreshold;
		case EMGSpeedtrapRating::Silver:
			return Def->SilverThreshold;
		case EMGSpeedtrapRating::Gold:
			return Def->GoldThreshold;
		case EMGSpeedtrapRating::Platinum:
			return Def->PlatinumThreshold;
		case EMGSpeedtrapRating::Diamond:
			return Def->DiamondThreshold;
		case EMGSpeedtrapRating::Legend:
			return Def->LegendThreshold;
		default:
			return 0.0f;
	}
}

float UMGSpeedtrapSubsystem::GetNextRatingThreshold(const FString& SpeedtrapId, EMGSpeedtrapRating CurrentRating) const
{
	switch (CurrentRating)
	{
		case EMGSpeedtrapRating::None:
			return GetThresholdForRating(SpeedtrapId, EMGSpeedtrapRating::Bronze);
		case EMGSpeedtrapRating::Bronze:
			return GetThresholdForRating(SpeedtrapId, EMGSpeedtrapRating::Silver);
		case EMGSpeedtrapRating::Silver:
			return GetThresholdForRating(SpeedtrapId, EMGSpeedtrapRating::Gold);
		case EMGSpeedtrapRating::Gold:
			return GetThresholdForRating(SpeedtrapId, EMGSpeedtrapRating::Platinum);
		case EMGSpeedtrapRating::Platinum:
			return GetThresholdForRating(SpeedtrapId, EMGSpeedtrapRating::Diamond);
		case EMGSpeedtrapRating::Diamond:
			return GetThresholdForRating(SpeedtrapId, EMGSpeedtrapRating::Legend);
		default:
			return 0.0f;
	}
}

// ============================================================================
// Records
// ============================================================================

FMGSpeedtrapRecord UMGSpeedtrapSubsystem::GetSpeedtrapRecord(const FString& SpeedtrapId) const
{
	if (const FMGSpeedtrapRecord* Found = PlayerStats.Records.Find(SpeedtrapId))
	{
		return *Found;
	}
	return FMGSpeedtrapRecord();
}

float UMGSpeedtrapSubsystem::GetPersonalBest(const FString& SpeedtrapId) const
{
	if (const FMGSpeedtrapRecord* Found = PlayerStats.Records.Find(SpeedtrapId))
	{
		return Found->PersonalBest;
	}
	return 0.0f;
}

float UMGSpeedtrapSubsystem::GetWorldRecord(const FString& SpeedtrapId) const
{
	if (const FMGSpeedtrapRecord* Found = PlayerStats.Records.Find(SpeedtrapId))
	{
		return Found->WorldRecord;
	}
	return 0.0f;
}

void UMGSpeedtrapSubsystem::SetWorldRecord(const FString& SpeedtrapId, float Value, const FString& PlayerName)
{
	FMGSpeedtrapRecord& Record = PlayerStats.Records.FindOrAdd(SpeedtrapId);
	Record.SpeedtrapId = SpeedtrapId;
	Record.WorldRecord = Value;
	Record.WorldRecordHolder = PlayerName;
}

// ============================================================================
// Leaderboards
// ============================================================================

TArray<FMGSpeedtrapLeaderboardEntry> UMGSpeedtrapSubsystem::GetLeaderboard(const FString& SpeedtrapId, int32 MaxEntries) const
{
	if (const TArray<FMGSpeedtrapLeaderboardEntry>* Found = Leaderboards.Find(SpeedtrapId))
	{
		TArray<FMGSpeedtrapLeaderboardEntry> Result;
		int32 Count = FMath::Min(MaxEntries, Found->Num());
		for (int32 i = 0; i < Count; i++)
		{
			Result.Add((*Found)[i]);
		}
		return Result;
	}
	return TArray<FMGSpeedtrapLeaderboardEntry>();
}

TArray<FMGSpeedtrapLeaderboardEntry> UMGSpeedtrapSubsystem::GetFriendsLeaderboard(const FString& SpeedtrapId, const TArray<FString>& FriendIds) const
{
	TArray<FMGSpeedtrapLeaderboardEntry> Result;

	if (const TArray<FMGSpeedtrapLeaderboardEntry>* Found = Leaderboards.Find(SpeedtrapId))
	{
		for (const FMGSpeedtrapLeaderboardEntry& Entry : *Found)
		{
			if (FriendIds.Contains(Entry.PlayerId))
			{
				Result.Add(Entry);
			}
		}
	}

	return Result;
}

int32 UMGSpeedtrapSubsystem::GetPlayerRank(const FString& SpeedtrapId) const
{
	if (const TArray<FMGSpeedtrapLeaderboardEntry>* Found = Leaderboards.Find(SpeedtrapId))
	{
		for (const FMGSpeedtrapLeaderboardEntry& Entry : *Found)
		{
			if (Entry.PlayerId == PlayerStats.PlayerId)
			{
				return Entry.Rank;
			}
		}
	}
	return 0;
}

// ============================================================================
// Stats
// ============================================================================

FMGSpeedtrapPlayerStats UMGSpeedtrapSubsystem::GetPlayerStats() const
{
	return PlayerStats;
}

int32 UMGSpeedtrapSubsystem::GetTotalSpeedtrapsDiscovered() const
{
	return DiscoveredSpeedtrapIds.Num();
}

int32 UMGSpeedtrapSubsystem::GetTotalRatingsAtLevel(EMGSpeedtrapRating Rating) const
{
	switch (Rating)
	{
		case EMGSpeedtrapRating::Gold:
			return PlayerStats.TotalGoldRatings;
		case EMGSpeedtrapRating::Platinum:
			return PlayerStats.TotalPlatinumRatings;
		case EMGSpeedtrapRating::Diamond:
			return PlayerStats.TotalDiamondRatings;
		case EMGSpeedtrapRating::Legend:
			return PlayerStats.TotalLegendRatings;
		default:
			return 0;
	}
}

float UMGSpeedtrapSubsystem::GetCompletionPercentage() const
{
	if (RegisteredSpeedtraps.Num() == 0)
	{
		return 0.0f;
	}

	int32 Completed = 0;
	for (const auto& Pair : PlayerStats.Records)
	{
		if (Pair.Value.BestRating >= EMGSpeedtrapRating::Bronze)
		{
			Completed++;
		}
	}

	return static_cast<float>(Completed) / RegisteredSpeedtraps.Num() * 100.0f;
}

// ============================================================================
// Discovery
// ============================================================================

void UMGSpeedtrapSubsystem::DiscoverSpeedtrap(const FString& SpeedtrapId)
{
	if (DiscoveredSpeedtrapIds.Contains(SpeedtrapId))
	{
		return;
	}

	DiscoveredSpeedtrapIds.Add(SpeedtrapId);
	PlayerStats.TotalSpeedtrapsFound = DiscoveredSpeedtrapIds.Num();

	OnSpeedtrapDiscovered.Broadcast(SpeedtrapId, DiscoveredSpeedtrapIds.Num());
}

bool UMGSpeedtrapSubsystem::IsSpeedtrapDiscovered(const FString& SpeedtrapId) const
{
	return DiscoveredSpeedtrapIds.Contains(SpeedtrapId);
}

TArray<FString> UMGSpeedtrapSubsystem::GetDiscoveredSpeedtraps() const
{
	return DiscoveredSpeedtrapIds;
}

TArray<FString> UMGSpeedtrapSubsystem::GetUndiscoveredSpeedtraps() const
{
	TArray<FString> Result;
	for (const auto& Pair : RegisteredSpeedtraps)
	{
		if (!DiscoveredSpeedtrapIds.Contains(Pair.Key))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

// ============================================================================
// Configuration
// ============================================================================

void UMGSpeedtrapSubsystem::SetSpeedZoneConfig(const FMGSpeedZoneConfig& Config)
{
	SpeedZoneConfig = Config;
}

FMGSpeedZoneConfig UMGSpeedtrapSubsystem::GetSpeedZoneConfig() const
{
	return SpeedZoneConfig;
}

// ============================================================================
// Unit Conversion
// ============================================================================

float UMGSpeedtrapSubsystem::ConvertToMPH(float CMPerSecond) const
{
	// cm/s to mph: divide by 44.704
	return CMPerSecond / 44.704f;
}

float UMGSpeedtrapSubsystem::ConvertToKPH(float CMPerSecond) const
{
	// cm/s to kph: divide by 27.778
	return CMPerSecond / 27.778f;
}

FText UMGSpeedtrapSubsystem::FormatSpeed(float SpeedMPH, bool bUseMetric) const
{
	if (bUseMetric)
	{
		float SpeedKPH = SpeedMPH * 1.60934f;
		return FText::FromString(FString::Printf(TEXT("%.0f KPH"), SpeedKPH));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("%.0f MPH"), SpeedMPH));
	}
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGSpeedtrapSubsystem::SaveSpeedtrapData()
{
	// Placeholder - would serialize to save game
}

void UMGSpeedtrapSubsystem::LoadSpeedtrapData()
{
	// Placeholder - would deserialize from save game
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGSpeedtrapSubsystem::UpdateActiveZone(const FString& PlayerId, float Speed, float DeltaTime)
{
	FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId);
	if (!Active)
	{
		return;
	}

	const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(Active->SpeedtrapId);
	if (!Def)
	{
		return;
	}

	// Update values
	Active->CurrentValue = Speed;
	Active->MaxValue = FMath::Max(Active->MaxValue, Speed);
	Active->TimeElapsed += DeltaTime;

	// Sample for average calculation
	Active->SampleCount++;
	Active->SpeedSum += Speed;

	// Update time remaining
	if (Def->bHasTimeLimit)
	{
		Active->TimeRemaining -= DeltaTime;
		if (Active->TimeRemaining <= 0.0f)
		{
			Active->State = EMGSpeedtrapState::Failed;
			CancelActiveSpeedtrap(PlayerId);
			return;
		}
	}

	// Update distance remaining (estimate based on speed)
	if (Active->DistanceRemaining > 0.0f)
	{
		// Convert MPH to cm/s and multiply by delta time
		float DistanceCovered = (Speed * 44.704f) * DeltaTime;
		Active->DistanceRemaining -= DistanceCovered;
	}

	// Update current rating
	float AverageSpeed = Active->SampleCount > 0 ? Active->SpeedSum / Active->SampleCount : 0.0f;
	float ValueForRating = Def->Type == EMGSpeedtrapType::Average ? AverageSpeed : Active->MaxValue;
	Active->CurrentRating = CalculateRating(Active->SpeedtrapId, ValueForRating);

	// Broadcast progress
	float Progress = 0.0f;
	if (Def->ZoneLength > 0.0f)
	{
		Progress = 1.0f - (Active->DistanceRemaining / Def->ZoneLength);
	}
	OnSpeedtrapProgress.Broadcast(Active->SpeedtrapId, Progress);
}

FMGSpeedtrapAttempt UMGSpeedtrapSubsystem::FinalizeAttempt(const FString& PlayerId, const FString& SpeedtrapId, float RecordedValue)
{
	FMGSpeedtrapAttempt Attempt;
	Attempt.AttemptId = GenerateAttemptId();
	Attempt.SpeedtrapId = SpeedtrapId;
	Attempt.PlayerId = PlayerId;
	Attempt.RecordedValue = RecordedValue;
	Attempt.Timestamp = FDateTime::Now();

	// Get active state data if available
	if (const FMGActiveSpeedtrap* Active = ActiveSpeedtraps.Find(PlayerId))
	{
		Attempt.MaxSpeed = Active->MaxValue;
		Attempt.EntrySpeed = Active->EntrySpeed;
		Attempt.TimeTaken = Active->TimeElapsed;

		if (Active->SampleCount > 0)
		{
			Attempt.AverageSpeed = Active->SpeedSum / Active->SampleCount;
		}
	}

	// Calculate rating
	Attempt.Rating = CalculateRating(SpeedtrapId, RecordedValue);
	Attempt.PointsEarned = GetPointsForRating(SpeedtrapId, Attempt.Rating);

	// Check for personal best
	FMGSpeedtrapRecord& Record = PlayerStats.Records.FindOrAdd(SpeedtrapId);
	Record.SpeedtrapId = SpeedtrapId;
	Record.TotalAttempts++;

	if (Attempt.Rating != EMGSpeedtrapRating::None)
	{
		Record.TotalCompletions++;
	}

	if (RecordedValue > Record.PersonalBest)
	{
		float OldBest = Record.PersonalBest;
		Record.PersonalBest = RecordedValue;
		Record.PersonalBestDate = Attempt.Timestamp;
		Attempt.bIsPersonalBest = true;

		OnSpeedtrapNewPersonalBest.Broadcast(SpeedtrapId, OldBest, RecordedValue);
	}

	Attempt.DeltaFromBest = RecordedValue - Record.PersonalBest;

	// Update best rating
	if (Attempt.Rating > Record.BestRating)
	{
		Record.BestRating = Attempt.Rating;
		CheckForRatingAchievement(Attempt.Rating);
	}

	// Check for world record
	if (RecordedValue > Record.WorldRecord)
	{
		Record.WorldRecord = RecordedValue;
		Attempt.bIsWorldRecord = true;
		OnSpeedtrapNewWorldRecord.Broadcast(SpeedtrapId, RecordedValue);
	}

	// Update player stats
	PlayerStats.TotalPoints += Attempt.PointsEarned;

	if (RecordedValue > PlayerStats.HighestSpeedRecorded)
	{
		const FMGSpeedtrapDefinition* Def = RegisteredSpeedtraps.Find(SpeedtrapId);
		if (Def && Def->Type != EMGSpeedtrapType::Jump)
		{
			PlayerStats.HighestSpeedRecorded = RecordedValue;
		}
	}

	OnSpeedtrapRecorded.Broadcast(SpeedtrapId, RecordedValue, Attempt.Rating);

	return Attempt;
}

void UMGSpeedtrapSubsystem::UpdateRecords(const FMGSpeedtrapAttempt& Attempt)
{
	// Already handled in FinalizeAttempt
}

void UMGSpeedtrapSubsystem::CheckForRatingAchievement(EMGSpeedtrapRating Rating)
{
	int32 TotalAtRating = 0;

	switch (Rating)
	{
		case EMGSpeedtrapRating::Gold:
			PlayerStats.TotalGoldRatings++;
			TotalAtRating = PlayerStats.TotalGoldRatings;
			break;
		case EMGSpeedtrapRating::Platinum:
			PlayerStats.TotalPlatinumRatings++;
			TotalAtRating = PlayerStats.TotalPlatinumRatings;
			break;
		case EMGSpeedtrapRating::Diamond:
			PlayerStats.TotalDiamondRatings++;
			TotalAtRating = PlayerStats.TotalDiamondRatings;
			break;
		case EMGSpeedtrapRating::Legend:
			PlayerStats.TotalLegendRatings++;
			TotalAtRating = PlayerStats.TotalLegendRatings;
			break;
		default:
			return;
	}

	OnSpeedtrapRatingAchieved.Broadcast(Rating, TotalAtRating);
}

FString UMGSpeedtrapSubsystem::GenerateAttemptId() const
{
	return FString::Printf(TEXT("SPDTRP_%d_%lld"), ++const_cast<UMGSpeedtrapSubsystem*>(this)->AttemptCounter,
	                       FDateTime::Now().GetTicks());
}
