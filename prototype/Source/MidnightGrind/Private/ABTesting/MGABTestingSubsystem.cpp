// Copyright Midnight Grind. All Rights Reserved.

#include "ABTesting/MGABTestingSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/SecureHash.h"

void UMGABTestingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Generate consistent user ID (would come from account system)
	UserID = FGuid::NewGuid().ToString();

	InitializeDefaultFlags();
	LoadLocalAssignments();
	LoadConfiguration();
	UpdateUserSegments();
	AssignToExperiments();

	// Set up periodic refresh
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UMGABTestingSubsystem::RefreshFeatureFlags,
			RefreshIntervalSeconds,
			true
		);
	}
}

void UMGABTestingSubsystem::Deinitialize()
{
	SaveLocalAssignments();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	Super::Deinitialize();
}

bool UMGABTestingSubsystem::IsFeatureEnabled(const FString& FlagID) const
{
	// Check overrides first
	if (const bool* Override = FlagOverrides.Find(FlagID))
	{
		return *Override;
	}

	for (const FMGFeatureFlag& Flag : FeatureFlags)
	{
		if (Flag.FlagID == FlagID)
		{
			if (!Flag.bEnabled)
				return false;

			// Check if user is in enabled list
			if (Flag.EnabledUserIDs.Contains(UserID))
				return true;

			// Check if user is in enabled segment
			for (const FString& Segment : Flag.EnabledSegments)
			{
				if (UserSegments.Contains(Segment))
					return true;
			}

			// Check rollout percentage
			if (Flag.RolloutPercent >= 100.0f)
				return true;

			if (Flag.RolloutPercent > 0.0f)
			{
				return IsInRollout(FlagID);
			}

			return Flag.EnabledSegments.Num() == 0 && Flag.EnabledUserIDs.Num() == 0;
		}
	}

	return false;
}

FString UMGABTestingSubsystem::GetFeatureConfig(const FString& FlagID, const FString& Key, const FString& DefaultValue) const
{
	for (const FMGFeatureFlag& Flag : FeatureFlags)
	{
		if (Flag.FlagID == FlagID)
		{
			if (const FString* Value = Flag.Configuration.Find(Key))
			{
				return *Value;
			}
		}
	}
	return DefaultValue;
}

int32 UMGABTestingSubsystem::GetFeatureConfigInt(const FString& FlagID, const FString& Key, int32 DefaultValue) const
{
	FString Value = GetFeatureConfig(FlagID, Key, TEXT(""));
	if (Value.IsEmpty())
		return DefaultValue;
	return FCString::Atoi(*Value);
}

float UMGABTestingSubsystem::GetFeatureConfigFloat(const FString& FlagID, const FString& Key, float DefaultValue) const
{
	FString Value = GetFeatureConfig(FlagID, Key, TEXT(""));
	if (Value.IsEmpty())
		return DefaultValue;
	return FCString::Atof(*Value);
}

void UMGABTestingSubsystem::RefreshFeatureFlags()
{
	LoadConfiguration();

	int32 UpdatedCount = FeatureFlags.Num();
	OnConfigurationRefreshed.Broadcast(UpdatedCount);
}

EMGVariantType UMGABTestingSubsystem::GetExperimentVariant(const FString& ExperimentID) const
{
	// Check overrides first
	if (const EMGVariantType* Override = ExperimentOverrides.Find(ExperimentID))
	{
		return *Override;
	}

	for (const FMGExperimentAssignment& Assignment : MyAssignments)
	{
		if (Assignment.ExperimentID == ExperimentID)
		{
			return Assignment.AssignedVariant;
		}
	}

	return EMGVariantType::Control;
}

bool UMGABTestingSubsystem::IsInExperiment(const FString& ExperimentID) const
{
	for (const FMGExperimentAssignment& Assignment : MyAssignments)
	{
		if (Assignment.ExperimentID == ExperimentID)
		{
			return true;
		}
	}
	return false;
}

FString UMGABTestingSubsystem::GetExperimentParam(const FString& ExperimentID, const FString& ParamKey, const FString& DefaultValue) const
{
	EMGVariantType MyVariant = GetExperimentVariant(ExperimentID);

	for (const FMGExperiment& Experiment : Experiments)
	{
		if (Experiment.ExperimentID == ExperimentID)
		{
			for (const FMGExperimentVariant& Variant : Experiment.Variants)
			{
				if (Variant.Type == MyVariant)
				{
					if (const FString* Value = Variant.Parameters.Find(ParamKey))
					{
						return *Value;
					}
				}
			}
		}
	}

	return DefaultValue;
}

void UMGABTestingSubsystem::TrackExperimentExposure(const FString& ExperimentID)
{
	for (FMGExperimentAssignment& Assignment : MyAssignments)
	{
		if (Assignment.ExperimentID == ExperimentID && !Assignment.bExposed)
		{
			Assignment.bExposed = true;
			// Would send to analytics backend
			SaveLocalAssignments();
			break;
		}
	}
}

void UMGABTestingSubsystem::TrackExperimentConversion(const FString& ExperimentID, const FString& MetricName, float Value)
{
	for (FMGExperimentAssignment& Assignment : MyAssignments)
	{
		if (Assignment.ExperimentID == ExperimentID && !Assignment.bConverted)
		{
			Assignment.bConverted = true;
			// Would send conversion event to analytics backend
			SaveLocalAssignments();
			break;
		}
	}
}

TArray<FMGExperiment> UMGABTestingSubsystem::GetActiveExperiments() const
{
	TArray<FMGExperiment> Active;
	for (const FMGExperiment& Experiment : Experiments)
	{
		if (Experiment.Status == EMGExperimentStatus::Running)
		{
			Active.Add(Experiment);
		}
	}
	return Active;
}

void UMGABTestingSubsystem::UpdateUserSegments()
{
	UserSegments.Empty();

	for (const FMGUserSegment& Segment : Segments)
	{
		if (EvaluateSegmentCriteria(Segment))
		{
			UserSegments.Add(Segment.SegmentID);
		}
	}
}

bool UMGABTestingSubsystem::IsInSegment(const FString& SegmentID) const
{
	return UserSegments.Contains(SegmentID);
}

void UMGABTestingSubsystem::OverrideFeatureFlag(const FString& FlagID, bool bEnabled)
{
	bool bOldValue = IsFeatureEnabled(FlagID);
	FlagOverrides.Add(FlagID, bEnabled);

	if (bOldValue != bEnabled)
	{
		OnFeatureFlagChanged.Broadcast(FlagID, bEnabled);
	}
}

void UMGABTestingSubsystem::OverrideExperimentVariant(const FString& ExperimentID, EMGVariantType Variant)
{
	ExperimentOverrides.Add(ExperimentID, Variant);
	OnExperimentAssigned.Broadcast(ExperimentID, Variant);
}

void UMGABTestingSubsystem::ClearAllOverrides()
{
	FlagOverrides.Empty();
	ExperimentOverrides.Empty();
}

bool UMGABTestingSubsystem::HasOverride(const FString& ID) const
{
	return FlagOverrides.Contains(ID) || ExperimentOverrides.Contains(ID);
}

float UMGABTestingSubsystem::GetRolloutPercent(const FString& FlagID) const
{
	for (const FMGFeatureFlag& Flag : FeatureFlags)
	{
		if (Flag.FlagID == FlagID)
		{
			return Flag.RolloutPercent;
		}
	}
	return 0.0f;
}

bool UMGABTestingSubsystem::IsInRollout(const FString& FlagID) const
{
	uint32 Bucket = GenerateUserBucket(FlagID);
	float RolloutPercent = GetRolloutPercent(FlagID);
	return (Bucket % 100) < static_cast<uint32>(RolloutPercent);
}

void UMGABTestingSubsystem::LoadConfiguration()
{
	// Would fetch from remote config service
	// For now, use initialized defaults
}

void UMGABTestingSubsystem::SaveLocalAssignments()
{
	// Would save to local storage
}

void UMGABTestingSubsystem::LoadLocalAssignments()
{
	// Would load from local storage
}

void UMGABTestingSubsystem::AssignToExperiments()
{
	for (const FMGExperiment& Experiment : Experiments)
	{
		if (Experiment.Status != EMGExperimentStatus::Running)
			continue;

		// Check if already assigned
		bool bAlreadyAssigned = false;
		for (const FMGExperimentAssignment& Assignment : MyAssignments)
		{
			if (Assignment.ExperimentID == Experiment.ExperimentID)
			{
				bAlreadyAssigned = true;
				break;
			}
		}

		if (bAlreadyAssigned)
			continue;

		// Check segment targeting
		bool bInTargetSegment = Experiment.TargetSegments.Num() == 0;
		for (const FString& TargetSegment : Experiment.TargetSegments)
		{
			if (UserSegments.Contains(TargetSegment))
			{
				bInTargetSegment = true;
				break;
			}
		}

		if (!bInTargetSegment)
			continue;

		// Assign variant
		EMGVariantType AssignedVariant = DetermineVariantAssignment(Experiment);

		FMGExperimentAssignment NewAssignment;
		NewAssignment.ExperimentID = Experiment.ExperimentID;
		NewAssignment.AssignedVariant = AssignedVariant;
		NewAssignment.AssignedAt = FDateTime::UtcNow();
		MyAssignments.Add(NewAssignment);

		OnExperimentAssigned.Broadcast(Experiment.ExperimentID, AssignedVariant);
	}

	SaveLocalAssignments();
}

EMGVariantType UMGABTestingSubsystem::DetermineVariantAssignment(const FMGExperiment& Experiment) const
{
	uint32 Bucket = GenerateUserBucket(Experiment.ExperimentID);
	float BucketPercent = static_cast<float>(Bucket % 100);

	float CumulativePercent = 0.0f;
	for (const FMGExperimentVariant& Variant : Experiment.Variants)
	{
		CumulativePercent += Variant.AllocationPercent;
		if (BucketPercent < CumulativePercent)
		{
			return Variant.Type;
		}
	}

	return EMGVariantType::Control;
}

bool UMGABTestingSubsystem::EvaluateSegmentCriteria(const FMGUserSegment& Segment) const
{
	// Would evaluate criteria against user properties
	// For now, simple implementation
	return true;
}

uint32 UMGABTestingSubsystem::GenerateUserBucket(const FString& ExperimentID) const
{
	FString Combined = UserID + ExperimentID;
	uint32 Hash = GetTypeHash(Combined);
	return Hash;
}

void UMGABTestingSubsystem::InitializeDefaultFlags()
{
	// New UI experiment
	FMGFeatureFlag NewUIFlag;
	NewUIFlag.FlagID = TEXT("new_garage_ui");
	NewUIFlag.FlagName = TEXT("New Garage UI");
	NewUIFlag.bEnabled = true;
	NewUIFlag.RolloutPercent = 50.0f;
	FeatureFlags.Add(NewUIFlag);

	// Enhanced matchmaking
	FMGFeatureFlag MatchmakingFlag;
	MatchmakingFlag.FlagID = TEXT("enhanced_matchmaking");
	MatchmakingFlag.FlagName = TEXT("Enhanced Matchmaking");
	MatchmakingFlag.bEnabled = true;
	MatchmakingFlag.RolloutPercent = 100.0f;
	FeatureFlags.Add(MatchmakingFlag);

	// Experimental physics
	FMGFeatureFlag PhysicsFlag;
	PhysicsFlag.FlagID = TEXT("experimental_physics");
	PhysicsFlag.FlagName = TEXT("Experimental Physics");
	PhysicsFlag.bEnabled = false;
	PhysicsFlag.RolloutPercent = 0.0f;
	FeatureFlags.Add(PhysicsFlag);

	// Nitro balance experiment
	FMGExperiment NitroExperiment;
	NitroExperiment.ExperimentID = TEXT("nitro_balance_v2");
	NitroExperiment.ExperimentName = TEXT("Nitro Balance Test");
	NitroExperiment.Description = TEXT("Testing different nitro refill rates");
	NitroExperiment.Status = EMGExperimentStatus::Running;
	NitroExperiment.TargetMetric = TEXT("race_completion_rate");
	NitroExperiment.MinSampleSize = 5000;

	FMGExperimentVariant ControlVariant;
	ControlVariant.Type = EMGVariantType::Control;
	ControlVariant.VariantName = TEXT("Control");
	ControlVariant.AllocationPercent = 50.0f;
	ControlVariant.Parameters.Add(TEXT("nitro_refill_rate"), TEXT("1.0"));
	NitroExperiment.Variants.Add(ControlVariant);

	FMGExperimentVariant TestVariant;
	TestVariant.Type = EMGVariantType::VariantA;
	TestVariant.VariantName = TEXT("Faster Refill");
	TestVariant.AllocationPercent = 50.0f;
	TestVariant.Parameters.Add(TEXT("nitro_refill_rate"), TEXT("1.5"));
	NitroExperiment.Variants.Add(TestVariant);

	Experiments.Add(NitroExperiment);

	// User segments
	FMGUserSegment NewPlayerSegment;
	NewPlayerSegment.SegmentID = TEXT("new_players");
	NewPlayerSegment.SegmentName = TEXT("New Players");
	NewPlayerSegment.Criteria.Add(TEXT("days_since_install"), TEXT("<7"));
	Segments.Add(NewPlayerSegment);

	FMGUserSegment VeteranSegment;
	VeteranSegment.SegmentID = TEXT("veterans");
	VeteranSegment.SegmentName = TEXT("Veteran Players");
	VeteranSegment.Criteria.Add(TEXT("total_races"), TEXT(">100"));
	Segments.Add(VeteranSegment);

	FMGUserSegment WhaleSegment;
	WhaleSegment.SegmentID = TEXT("high_spenders");
	WhaleSegment.SegmentName = TEXT("High Spenders");
	WhaleSegment.Criteria.Add(TEXT("total_spent"), TEXT(">50"));
	Segments.Add(WhaleSegment);
}
