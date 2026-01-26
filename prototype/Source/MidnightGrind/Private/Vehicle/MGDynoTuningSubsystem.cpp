// Copyright Midnight Grind. All Rights Reserved.

#include "Vehicle/MGDynoTuningSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGDynoTuningSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Initializing dyno and tuning system"));
}

void UMGDynoTuningSubsystem::Deinitialize()
{
	// Clear any active dyno sessions
	for (auto& SessionPair : ActiveSessions)
	{
		if (SessionPair.Value.Status == EMGDynoStatus::Running)
		{
			SessionPair.Value.Status = EMGDynoStatus::Failed;
		}
	}

	Super::Deinitialize();
}

// ==========================================
// DYNO OPERATIONS
// ==========================================

FGuid UMGDynoTuningSubsystem::StartDynoSession(FGuid PlayerID, FGuid VehicleID)
{
	// Check for existing session
	for (const auto& SessionPair : ActiveSessions)
	{
		if (SessionPair.Value.VehicleID == VehicleID &&
			SessionPair.Value.Status != EMGDynoStatus::Complete &&
			SessionPair.Value.Status != EMGDynoStatus::Failed)
		{
			UE_LOG(LogTemp, Warning, TEXT("MGDynoTuningSubsystem: Vehicle already has active dyno session"));
			return FGuid();
		}
	}

	FMGDynoSession NewSession;
	NewSession.SessionID = FGuid::NewGuid();
	NewSession.VehicleID = VehicleID;
	NewSession.PlayerID = PlayerID;
	NewSession.Status = EMGDynoStatus::Idle;
	NewSession.Progress = 0.0f;
	NewSession.CurrentRPM = 0;
	NewSession.StartTime = FDateTime::UtcNow();

	ActiveSessions.Add(NewSession.SessionID, NewSession);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Started dyno session %s for vehicle %s"),
		*NewSession.SessionID.ToString(), *VehicleID.ToString());

	return NewSession.SessionID;
}

bool UMGDynoTuningSubsystem::StartDynoRun(FGuid SessionID)
{
	FMGDynoSession* Session = ActiveSessions.Find(SessionID);
	if (!Session)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoTuningSubsystem: Session not found"));
		return false;
	}

	if (Session->Status != EMGDynoStatus::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoTuningSubsystem: Session not in idle state"));
		return false;
	}

	// Start warm-up phase
	Session->Status = EMGDynoStatus::WarmingUp;
	Session->Progress = 0.0f;
	Session->CurrentRPM = DynoStartRPM;
	Session->LiveData.Empty();

	OnDynoStatusChanged.Broadcast(SessionID, EMGDynoStatus::WarmingUp);

	// Simulate warm-up for 2 seconds then start actual run
	if (UWorld* World = GetWorld())
	{
		FTimerDelegate WarmupDelegate;
		WarmupDelegate.BindLambda([this, SessionID]()
		{
			FMGDynoSession* Sess = ActiveSessions.Find(SessionID);
			if (Sess && Sess->Status == EMGDynoStatus::WarmingUp)
			{
				Sess->Status = EMGDynoStatus::Running;
				OnDynoStatusChanged.Broadcast(SessionID, EMGDynoStatus::Running);

				// Start the dyno tick simulation
				if (UWorld* W = GetWorld())
				{
					W->GetTimerManager().SetTimer(
						DynoTickTimerHandle,
						FTimerDelegate::CreateUObject(this, &UMGDynoTuningSubsystem::TickDynoSession, SessionID, DynoTickInterval),
						DynoTickInterval,
						true
					);
				}
			}
		});

		World->GetTimerManager().SetTimer(
			DynoTickTimerHandle,
			WarmupDelegate,
			2.0f,
			false
		);
	}

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Starting dyno run for session %s"), *SessionID.ToString());
	return true;
}

void UMGDynoTuningSubsystem::CancelDynoRun(FGuid SessionID)
{
	FMGDynoSession* Session = ActiveSessions.Find(SessionID);
	if (!Session)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DynoTickTimerHandle);
	}

	Session->Status = EMGDynoStatus::Failed;
	OnDynoStatusChanged.Broadcast(SessionID, EMGDynoStatus::Failed);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Cancelled dyno run for session %s"), *SessionID.ToString());
}

bool UMGDynoTuningSubsystem::GetDynoSession(FGuid SessionID, FMGDynoSession& OutSession) const
{
	const FMGDynoSession* Session = ActiveSessions.Find(SessionID);
	if (Session)
	{
		OutSession = *Session;
		return true;
	}
	return false;
}

bool UMGDynoTuningSubsystem::GetDynoRunResult(FGuid RunID, FMGDynoRunResult& OutResult) const
{
	const FMGDynoRunResult* Result = DynoHistory.Find(RunID);
	if (Result)
	{
		OutResult = *Result;
		return true;
	}
	return false;
}

TArray<FMGDynoRunResult> UMGDynoTuningSubsystem::GetDynoHistory(FGuid VehicleID, int32 MaxResults) const
{
	TArray<FMGDynoRunResult> Results;

	for (const auto& HistoryPair : DynoHistory)
	{
		if (HistoryPair.Value.VehicleID == VehicleID)
		{
			Results.Add(HistoryPair.Value);
		}
	}

	// Sort by timestamp descending
	Results.Sort([](const FMGDynoRunResult& A, const FMGDynoRunResult& B)
	{
		return A.Timestamp > B.Timestamp;
	});

	if (Results.Num() > MaxResults)
	{
		Results.SetNum(MaxResults);
	}

	return Results;
}

void UMGDynoTuningSubsystem::CompareDynoRuns(FGuid RunID1, FGuid RunID2, float& OutHPDiff, float& OutTorqueDiff, int32& OutPowerBandDiff) const
{
	const FMGDynoRunResult* Run1 = DynoHistory.Find(RunID1);
	const FMGDynoRunResult* Run2 = DynoHistory.Find(RunID2);

	if (!Run1 || !Run2)
	{
		OutHPDiff = 0.0f;
		OutTorqueDiff = 0.0f;
		OutPowerBandDiff = 0;
		return;
	}

	OutHPDiff = Run2->PeakHP - Run1->PeakHP;
	OutTorqueDiff = Run2->PeakTorque - Run1->PeakTorque;

	int32 PowerBand1 = Run1->PowerBandEndRPM - Run1->PowerBandStartRPM;
	int32 PowerBand2 = Run2->PowerBandEndRPM - Run2->PowerBandStartRPM;
	OutPowerBandDiff = PowerBand2 - PowerBand1;
}

// ==========================================
// TUNING OPERATIONS
// ==========================================

FGuid UMGDynoTuningSubsystem::CreateTuneProfile(FGuid PlayerID, FGuid VehicleID, const FString& TuneName)
{
	FMGVehicleTuneProfile NewTune;
	NewTune.TuneID = FGuid::NewGuid();
	NewTune.VehicleID = VehicleID;
	NewTune.TuneName = TuneName;
	NewTune.StylePreset = EMGDrivingStylePreset::Custom;
	NewTune.CreatedAt = FDateTime::UtcNow();
	NewTune.LastModified = NewTune.CreatedAt;

	// Initialize with balanced preset
	FMGVehicleTuneProfile BalancedPreset = GeneratePresetTune(EMGDrivingStylePreset::Balanced);
	NewTune.ECU = BalancedPreset.ECU;
	NewTune.ForcedInduction = BalancedPreset.ForcedInduction;
	NewTune.Transmission = BalancedPreset.Transmission;
	NewTune.Suspension = BalancedPreset.Suspension;
	NewTune.Alignment = BalancedPreset.Alignment;
	NewTune.Differential = BalancedPreset.Differential;
	NewTune.Brakes = BalancedPreset.Brakes;
	NewTune.Nitrous = BalancedPreset.Nitrous;

	TuneProfiles.Add(NewTune.TuneID, NewTune);

	OnTuneSaved.Broadcast(PlayerID, VehicleID, NewTune);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Created tune profile '%s' for vehicle %s"),
		*TuneName, *VehicleID.ToString());

	return NewTune.TuneID;
}

bool UMGDynoTuningSubsystem::SaveTuneProfile(FGuid TuneID)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->LastModified = FDateTime::UtcNow();

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Saved tune profile %s"), *TuneID.ToString());
	return true;
}

bool UMGDynoTuningSubsystem::DeleteTuneProfile(FGuid TuneID)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	// Remove from public tunes if published
	PublicTuneIDs.Remove(TuneID);

	// Remove from active tunes
	for (auto It = ActiveVehicleTunes.CreateIterator(); It; ++It)
	{
		if (It.Value() == TuneID)
		{
			It.RemoveCurrent();
		}
	}

	TuneProfiles.Remove(TuneID);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Deleted tune profile %s"), *TuneID.ToString());
	return true;
}

bool UMGDynoTuningSubsystem::GetTuneProfile(FGuid TuneID, FMGVehicleTuneProfile& OutProfile) const
{
	const FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (Tune)
	{
		OutProfile = *Tune;
		return true;
	}
	return false;
}

TArray<FMGVehicleTuneProfile> UMGDynoTuningSubsystem::GetVehicleTuneProfiles(FGuid VehicleID) const
{
	TArray<FMGVehicleTuneProfile> Results;

	for (const auto& TunePair : TuneProfiles)
	{
		if (TunePair.Value.VehicleID == VehicleID)
		{
			Results.Add(TunePair.Value);
		}
	}

	// Sort by last modified
	Results.Sort([](const FMGVehicleTuneProfile& A, const FMGVehicleTuneProfile& B)
	{
		return A.LastModified > B.LastModified;
	});

	return Results;
}

bool UMGDynoTuningSubsystem::ApplyTuneProfile(FGuid VehicleID, FGuid TuneID)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoTuningSubsystem: Tune profile not found"));
		return false;
	}

	// Apply the tune to the vehicle systems
	ApplyTuneToVehicle(VehicleID, *Tune);

	// Set as active tune
	ActiveVehicleTunes.Add(VehicleID, TuneID);

	OnTuneApplied.Broadcast(VehicleID, TuneID);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Applied tune %s to vehicle %s"),
		*TuneID.ToString(), *VehicleID.ToString());

	return true;
}

bool UMGDynoTuningSubsystem::LoadPreset(FGuid TuneID, EMGDrivingStylePreset Preset)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	FMGVehicleTuneProfile PresetTune = GeneratePresetTune(Preset);

	Tune->StylePreset = Preset;
	Tune->ECU = PresetTune.ECU;
	Tune->ForcedInduction = PresetTune.ForcedInduction;
	Tune->Transmission = PresetTune.Transmission;
	Tune->Suspension = PresetTune.Suspension;
	Tune->Alignment = PresetTune.Alignment;
	Tune->Differential = PresetTune.Differential;
	Tune->Brakes = PresetTune.Brakes;
	Tune->Nitrous = PresetTune.Nitrous;
	Tune->LastModified = FDateTime::UtcNow();

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Loaded preset %d to tune %s"),
		static_cast<int32>(Preset), *TuneID.ToString());

	return true;
}

// ==========================================
// INDIVIDUAL TUNING PARAMETERS
// ==========================================

bool UMGDynoTuningSubsystem::UpdateECUTune(FGuid TuneID, const FMGECUTuneData& ECUData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->ECU = ECUData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateForcedInductionTune(FGuid TuneID, const FMGForcedInductionTuneData& FIData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->ForcedInduction = FIData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateTransmissionTune(FGuid TuneID, const FMGTransmissionTuneData& TransData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->Transmission = TransData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateSuspensionTune(FGuid TuneID, const FMGSuspensionTuneData& SuspData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->Suspension = SuspData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateAlignmentTune(FGuid TuneID, const FMGAlignmentTuneData& AlignData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->Alignment = AlignData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateDifferentialTune(FGuid TuneID, const FMGDifferentialTuneData& DiffData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->Differential = DiffData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateBrakeTune(FGuid TuneID, const FMGBrakeTuneData& BrakeData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->Brakes = BrakeData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

bool UMGDynoTuningSubsystem::UpdateNitrousTune(FGuid TuneID, const FMGNitrousTuneData& NitroData)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->Nitrous = NitroData;
	Tune->StylePreset = EMGDrivingStylePreset::Custom;
	Tune->LastModified = FDateTime::UtcNow();

	return true;
}

// ==========================================
// TUNE SHARING
// ==========================================

bool UMGDynoTuningSubsystem::PublishTune(FGuid TuneID)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->bIsPublic = true;

	if (!PublicTuneIDs.Contains(TuneID))
	{
		PublicTuneIDs.Add(TuneID);
	}

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Published tune %s"), *TuneID.ToString());
	return true;
}

bool UMGDynoTuningSubsystem::UnpublishTune(FGuid TuneID)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		return false;
	}

	Tune->bIsPublic = false;
	PublicTuneIDs.Remove(TuneID);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Unpublished tune %s"), *TuneID.ToString());
	return true;
}

FGuid UMGDynoTuningSubsystem::DownloadSharedTune(FGuid PlayerID, FGuid SharedTuneID, FGuid TargetVehicleID)
{
	const FMGVehicleTuneProfile* SourceTune = TuneProfiles.Find(SharedTuneID);
	if (!SourceTune || !SourceTune->bIsPublic)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoTuningSubsystem: Shared tune not found or not public"));
		return FGuid();
	}

	// Create a copy for the player
	FMGVehicleTuneProfile NewTune = *SourceTune;
	NewTune.TuneID = FGuid::NewGuid();
	NewTune.VehicleID = TargetVehicleID;
	NewTune.TuneName = FString::Printf(TEXT("%s (Downloaded)"), *SourceTune->TuneName);
	NewTune.CreatedAt = FDateTime::UtcNow();
	NewTune.LastModified = NewTune.CreatedAt;
	NewTune.bIsPublic = false;
	NewTune.DownloadCount = 0;
	NewTune.Rating = 0.0f;

	TuneProfiles.Add(NewTune.TuneID, NewTune);

	// Increment download count on source
	FMGVehicleTuneProfile* SourceMutable = TuneProfiles.Find(SharedTuneID);
	if (SourceMutable)
	{
		SourceMutable->DownloadCount++;
	}

	OnTuneSaved.Broadcast(PlayerID, TargetVehicleID, NewTune);

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Downloaded tune %s as %s"),
		*SharedTuneID.ToString(), *NewTune.TuneID.ToString());

	return NewTune.TuneID;
}

TArray<FMGVehicleTuneProfile> UMGDynoTuningSubsystem::SearchPublicTunes(FName VehicleModelID, EMGDrivingStylePreset StyleFilter, int32 MaxResults)
{
	TArray<FMGVehicleTuneProfile> Results;

	for (const FGuid& TuneID : PublicTuneIDs)
	{
		const FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
		if (!Tune)
		{
			continue;
		}

		// Filter by style if specified
		if (StyleFilter != EMGDrivingStylePreset::Custom && Tune->StylePreset != StyleFilter)
		{
			continue;
		}

		// Note: Vehicle model filtering would require additional data we don't have in this struct
		// In production, you'd match VehicleModelID against vehicle data

		Results.Add(*Tune);

		if (Results.Num() >= MaxResults)
		{
			break;
		}
	}

	// Sort by rating and download count
	Results.Sort([](const FMGVehicleTuneProfile& A, const FMGVehicleTuneProfile& B)
	{
		float ScoreA = A.Rating * 100.0f + A.DownloadCount;
		float ScoreB = B.Rating * 100.0f + B.DownloadCount;
		return ScoreA > ScoreB;
	});

	return Results;
}

bool UMGDynoTuningSubsystem::RateTune(FGuid TuneID, float Rating)
{
	FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune || !Tune->bIsPublic)
	{
		return false;
	}

	// Simple averaging (in production, track individual ratings)
	if (Tune->Rating == 0.0f)
	{
		Tune->Rating = FMath::Clamp(Rating, 1.0f, 5.0f);
	}
	else
	{
		Tune->Rating = (Tune->Rating + FMath::Clamp(Rating, 1.0f, 5.0f)) / 2.0f;
	}

	return true;
}

// ==========================================
// ANALYSIS
// ==========================================

TArray<FString> UMGDynoTuningSubsystem::AnalyzeTune(FGuid TuneID) const
{
	TArray<FString> Analysis;

	const FMGVehicleTuneProfile* Tune = TuneProfiles.Find(TuneID);
	if (!Tune)
	{
		Analysis.Add(TEXT("Error: Tune profile not found"));
		return Analysis;
	}

	// ECU Analysis
	if (Tune->ECU.IgnitionTimingHigh > 5.0f)
	{
		Analysis.Add(TEXT("Warning: High ignition timing at high RPM may cause detonation"));
	}

	if (Tune->ECU.FuelMapHigh < -10.0f)
	{
		Analysis.Add(TEXT("Warning: Lean fuel mixture at high RPM - risk of engine damage"));
	}

	if (Tune->ECU.bAntiLagEnabled && !Tune->ForcedInduction.bHasTurbo)
	{
		Analysis.Add(TEXT("Note: Anti-lag enabled but no turbo installed - feature will have no effect"));
	}

	// Suspension Analysis
	float SpringBias = Tune->Suspension.FrontSpringRate / FMath::Max(Tune->Suspension.RearSpringRate, 1.0f);
	if (SpringBias > 1.3f)
	{
		Analysis.Add(TEXT("Info: Front-biased spring setup - may cause understeer"));
	}
	else if (SpringBias < 0.7f)
	{
		Analysis.Add(TEXT("Info: Rear-biased spring setup - may cause oversteer"));
	}

	// Alignment Analysis
	if (FMath::Abs(Tune->Alignment.FrontCamber) > 5.0f)
	{
		Analysis.Add(TEXT("Warning: Extreme front camber will significantly reduce straight-line grip"));
	}

	if (Tune->Alignment.RearToe < -0.5f)
	{
		Analysis.Add(TEXT("Warning: Rear toe-out can cause instability at high speeds"));
	}

	// Differential Analysis
	if (Tune->Differential.bLimitedSlip && Tune->Differential.AccelerationLock > 80)
	{
		Analysis.Add(TEXT("Info: High LSD lock - good for drag racing, may cause understeer in corners"));
	}

	// Brake Analysis
	if (Tune->Brakes.BrakeBias > 70)
	{
		Analysis.Add(TEXT("Warning: High front brake bias - risk of front lockup"));
	}
	else if (Tune->Brakes.BrakeBias < 50)
	{
		Analysis.Add(TEXT("Warning: Rear-biased brakes - risk of spin under heavy braking"));
	}

	// Nitrous Analysis
	if (Tune->Nitrous.ShotSizeHP > 200)
	{
		Analysis.Add(TEXT("Warning: Large nitrous shot - ensure engine can handle the additional stress"));
	}

	if (Tune->Nitrous.ActivationRPM < 3000)
	{
		Analysis.Add(TEXT("Note: Low nitrous activation RPM - may cause wheel spin"));
	}

	// Transmission Analysis
	if (Tune->Transmission.GearRatios.Num() > 0)
	{
		float FirstGear = Tune->Transmission.GearRatios[0];
		if (FirstGear > 4.0f)
		{
			Analysis.Add(TEXT("Info: Short first gear - good for acceleration but short top speed"));
		}
	}

	if (Analysis.Num() == 0)
	{
		Analysis.Add(TEXT("Tune appears well-balanced with no obvious issues"));
	}

	return Analysis;
}

void UMGDynoTuningSubsystem::PredictPerformanceChange(FGuid VehicleID, const FMGVehicleTuneProfile& NewTune, float& OutHPChange, float& OutHandlingChange, float& OutTopSpeedChange) const
{
	// Simplified performance prediction based on tune parameters
	OutHPChange = 0.0f;
	OutHandlingChange = 0.0f;
	OutTopSpeedChange = 0.0f;

	// ECU adjustments affect HP
	OutHPChange += (NewTune.ECU.IgnitionTimingMid + NewTune.ECU.IgnitionTimingHigh) * 0.5f;
	OutHPChange += NewTune.ECU.FuelMapHigh * 0.3f;

	// Forced induction affects HP significantly
	if (NewTune.ForcedInduction.bHasTurbo)
	{
		OutHPChange += NewTune.ForcedInduction.TargetBoostPSI * 5.0f;
	}
	if (NewTune.ForcedInduction.bHasSupercharger)
	{
		OutHPChange += (2.0f - NewTune.ForcedInduction.SuperchargerPulleyRatio) * 50.0f;
	}

	// Nitrous adds temporary HP
	OutHPChange += NewTune.Nitrous.ShotSizeHP * 0.1f; // Factor for "effective" HP

	// Suspension affects handling
	float SuspBalance = 1.0f - FMath::Abs((NewTune.Suspension.FrontSpringRate / FMath::Max(NewTune.Suspension.RearSpringRate, 1.0f)) - 1.0f);
	OutHandlingChange += SuspBalance * 10.0f;

	// Anti-roll bars affect handling
	OutHandlingChange += (NewTune.Suspension.FrontAntiRollBar + NewTune.Suspension.RearAntiRollBar) * 0.1f;

	// Alignment affects handling
	OutHandlingChange -= FMath::Abs(NewTune.Alignment.FrontCamber) * 0.5f; // Too much camber hurts
	OutHandlingChange += NewTune.Alignment.Caster * 0.2f;

	// Final drive affects top speed
	OutTopSpeedChange -= (NewTune.Transmission.FinalDriveRatio - 3.73f) * 5.0f; // Lower ratio = higher top speed
}

TArray<float> UMGDynoTuningSubsystem::GetRecommendedGearRatios(FGuid VehicleID, FName TrackType) const
{
	TArray<float> Recommendations;

	// Default 6-speed ratios for different track types
	if (TrackType == FName(TEXT("Drag")))
	{
		// Short ratios for maximum acceleration
		Recommendations = { 3.35f, 2.18f, 1.57f, 1.23f, 1.00f, 0.82f };
	}
	else if (TrackType == FName(TEXT("TopSpeed")))
	{
		// Tall ratios for top speed
		Recommendations = { 2.97f, 1.93f, 1.35f, 1.02f, 0.81f, 0.65f };
	}
	else if (TrackType == FName(TEXT("Circuit")))
	{
		// Balanced ratios
		Recommendations = { 3.17f, 2.05f, 1.48f, 1.13f, 0.91f, 0.74f };
	}
	else if (TrackType == FName(TEXT("Drift")))
	{
		// Close-ratio for maintaining revs
		Recommendations = { 3.08f, 2.19f, 1.70f, 1.36f, 1.14f, 0.95f };
	}
	else if (TrackType == FName(TEXT("Touge")))
	{
		// Mid-range focus for canyon runs
		Recommendations = { 3.25f, 2.14f, 1.53f, 1.17f, 0.94f, 0.78f };
	}
	else
	{
		// Default balanced
		Recommendations = { 3.17f, 2.05f, 1.48f, 1.13f, 0.91f, 0.74f };
	}

	return Recommendations;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGDynoTuningSubsystem::TickDynoSession(FGuid SessionID, float DeltaTime)
{
	FMGDynoSession* Session = ActiveSessions.Find(SessionID);
	if (!Session || Session->Status != EMGDynoStatus::Running)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DynoTickTimerHandle);
		}
		return;
	}

	// Increment RPM
	Session->CurrentRPM += DynoRPMStep;

	// Calculate progress
	Session->Progress = static_cast<float>(Session->CurrentRPM - DynoStartRPM) /
		static_cast<float>(DynoEndRPM - DynoStartRPM);

	// Simulate data point
	FMGDynoDataPoint DataPoint = SimulateDynoDataPoint(Session->VehicleID, Session->CurrentRPM);
	Session->LiveData.Add(DataPoint);
	Session->CurrentHP = DataPoint.Horsepower;
	Session->CurrentTorque = DataPoint.Torque;

	OnDynoDataPoint.Broadcast(SessionID, DataPoint);

	// Check if run is complete
	if (Session->CurrentRPM >= DynoEndRPM)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DynoTickTimerHandle);
		}

		// Start cooling phase
		Session->Status = EMGDynoStatus::Cooling;
		OnDynoStatusChanged.Broadcast(SessionID, EMGDynoStatus::Cooling);

		// After 1 second, complete the run
		if (UWorld* World = GetWorld())
		{
			FTimerHandle CooldownHandle;
			FTimerDelegate CooldownDelegate;
			CooldownDelegate.BindLambda([this, SessionID]()
			{
				FMGDynoSession* Sess = ActiveSessions.Find(SessionID);
				if (Sess)
				{
					Sess->Status = EMGDynoStatus::Complete;

					// Calculate and store result
					FMGDynoRunResult Result = CalculateDynoResult(*Sess);
					DynoHistory.Add(Result.RunID, Result);

					OnDynoStatusChanged.Broadcast(SessionID, EMGDynoStatus::Complete);
					OnDynoRunComplete.Broadcast(SessionID, Result);

					UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Dyno run complete - Peak HP: %.1f @ %d RPM, Peak Torque: %.1f @ %d RPM"),
						Result.PeakHP, Result.PeakHPRPM, Result.PeakTorque, Result.PeakTorqueRPM);
				}
			});

			World->GetTimerManager().SetTimer(CooldownHandle, CooldownDelegate, 1.0f, false);
		}
	}
}

FMGDynoDataPoint UMGDynoTuningSubsystem::SimulateDynoDataPoint(FGuid VehicleID, int32 RPM) const
{
	FMGDynoDataPoint Point;
	Point.RPM = RPM;

	// Simulate realistic power curve (simplified model)
	// Peak torque around 4500 RPM, peak HP around 6500 RPM
	float NormalizedRPM = static_cast<float>(RPM) / 7000.0f;

	// Torque curve - peaks mid-range, falls off at high RPM
	float TorqueCurve = FMath::Sin(NormalizedRPM * PI * 0.8f) * (1.0f - NormalizedRPM * 0.2f);
	Point.Torque = 250.0f * FMath::Max(TorqueCurve, 0.1f); // Base 250 lb-ft peak

	// HP = Torque * RPM / 5252
	Point.Horsepower = (Point.Torque * RPM) / 5252.0f;

	// AFR - richer at higher RPM
	Point.AirFuelRatio = 14.7f - (NormalizedRPM * 2.0f); // Goes from ~14.7 to ~12.7

	// Boost simulation (if turbo)
	// Would need to check vehicle's tune - for now simulate generic
	Point.BoostPSI = FMath::Max(0.0f, (NormalizedRPM - 0.3f) * 15.0f);

	// Temperatures increase with RPM
	Point.ExhaustGasTemp = 400.0f + (NormalizedRPM * 400.0f); // 400-800 C
	Point.OilTemp = 80.0f + (NormalizedRPM * 30.0f); // 80-110 C
	Point.CoolantTemp = 85.0f + (NormalizedRPM * 15.0f); // 85-100 C

	return Point;
}

FMGDynoRunResult UMGDynoTuningSubsystem::CalculateDynoResult(const FMGDynoSession& Session) const
{
	FMGDynoRunResult Result;
	Result.RunID = FGuid::NewGuid();
	Result.VehicleID = Session.VehicleID;
	Result.Timestamp = FDateTime::UtcNow();
	Result.DataPoints = Session.LiveData;

	// Find peak values
	Result.PeakHP = 0.0f;
	Result.PeakTorque = 0.0f;

	for (const FMGDynoDataPoint& Point : Session.LiveData)
	{
		if (Point.Horsepower > Result.PeakHP)
		{
			Result.PeakHP = Point.Horsepower;
			Result.PeakHPRPM = Point.RPM;
		}
		if (Point.Torque > Result.PeakTorque)
		{
			Result.PeakTorque = Point.Torque;
			Result.PeakTorqueRPM = Point.RPM;
		}
	}

	// Calculate power band (90% of peak HP range)
	float PowerBandThreshold = Result.PeakHP * 0.9f;
	Result.PowerBandStartRPM = 0;
	Result.PowerBandEndRPM = 0;

	for (const FMGDynoDataPoint& Point : Session.LiveData)
	{
		if (Point.Horsepower >= PowerBandThreshold)
		{
			if (Result.PowerBandStartRPM == 0)
			{
				Result.PowerBandStartRPM = Point.RPM;
			}
			Result.PowerBandEndRPM = Point.RPM;
		}
	}

	// Redline is end of run
	Result.RedlineRPM = DynoEndRPM;

	// Atmospheric conditions (simulated)
	Result.AmbientTemp = 25.0f;
	Result.BarometricPressure = 1013.25f;
	Result.CorrectionFactor = 1.0f; // SAE J1349 correction

	// Compare to previous run
	TArray<FMGDynoRunResult> History = GetDynoHistory(Session.VehicleID, 1);
	if (History.Num() > 0)
	{
		Result.HPGainFromPrevious = Result.PeakHP - History[0].PeakHP;
		Result.TorqueGainFromPrevious = Result.PeakTorque - History[0].PeakTorque;
	}

	return Result;
}

void UMGDynoTuningSubsystem::ApplyTuneToVehicle(FGuid VehicleID, const FMGVehicleTuneProfile& Tune)
{
	// In production, this would interface with the vehicle physics system
	// to apply the actual tune parameters to the vehicle

	UE_LOG(LogTemp, Log, TEXT("MGDynoTuningSubsystem: Applying tune to vehicle %s"), *VehicleID.ToString());
	UE_LOG(LogTemp, Log, TEXT("  - Rev Limiter: %d RPM"), Tune.ECU.RevLimiter);
	UE_LOG(LogTemp, Log, TEXT("  - Target Boost: %.1f PSI"), Tune.ForcedInduction.TargetBoostPSI);
	UE_LOG(LogTemp, Log, TEXT("  - Final Drive: %.2f"), Tune.Transmission.FinalDriveRatio);
	UE_LOG(LogTemp, Log, TEXT("  - Brake Bias: %d%% front"), Tune.Brakes.BrakeBias);

	// The tune data is now available via the subsystem for other systems to query
	// Vehicle physics components would call GetTuneProfile() to get current settings
}

FMGVehicleTuneProfile UMGDynoTuningSubsystem::GeneratePresetTune(EMGDrivingStylePreset Preset) const
{
	FMGVehicleTuneProfile Tune;
	Tune.StylePreset = Preset;

	// Initialize transmission gear ratios
	Tune.Transmission.GearRatios = { 3.17f, 2.05f, 1.48f, 1.13f, 0.91f, 0.74f };
	Tune.ForcedInduction.BoostByGear = { 0.7f, 0.8f, 0.9f, 1.0f, 1.0f, 1.0f };

	switch (Preset)
	{
	case EMGDrivingStylePreset::Balanced:
		// Default balanced setup
		Tune.ECU.RevLimiter = 7000;
		Tune.ECU.LaunchControlRPM = 4000;
		Tune.Suspension.FrontSpringRate = 80.0f;
		Tune.Suspension.RearSpringRate = 70.0f;
		Tune.Alignment.FrontCamber = -1.0f;
		Tune.Alignment.RearCamber = -0.5f;
		Tune.Differential.AccelerationLock = 60;
		Tune.Differential.DecelerationLock = 40;
		Tune.Brakes.BrakeBias = 60;
		break;

	case EMGDrivingStylePreset::Grip:
		// Grip-focused setup
		Tune.ECU.RevLimiter = 7200;
		Tune.ECU.LaunchControlRPM = 4500;
		Tune.Suspension.FrontSpringRate = 100.0f;
		Tune.Suspension.RearSpringRate = 90.0f;
		Tune.Suspension.FrontAntiRollBar = 25.0f;
		Tune.Suspension.RearAntiRollBar = 20.0f;
		Tune.Alignment.FrontCamber = -2.5f;
		Tune.Alignment.RearCamber = -1.5f;
		Tune.Alignment.FrontToe = 0.0f;
		Tune.Alignment.RearToe = 0.1f;
		Tune.Differential.AccelerationLock = 70;
		Tune.Differential.DecelerationLock = 50;
		Tune.Brakes.BrakeBias = 58;
		Tune.Brakes.bABSEnabled = true;
		break;

	case EMGDrivingStylePreset::Drift:
		// Drift-focused setup
		Tune.ECU.RevLimiter = 7500;
		Tune.ECU.LaunchControlRPM = 5000;
		Tune.ECU.bFlatFootShifting = true;
		Tune.Suspension.FrontSpringRate = 70.0f;
		Tune.Suspension.RearSpringRate = 60.0f;
		Tune.Suspension.FrontAntiRollBar = 15.0f;
		Tune.Suspension.RearAntiRollBar = 10.0f;
		Tune.Alignment.FrontCamber = -4.0f;
		Tune.Alignment.RearCamber = -2.0f;
		Tune.Alignment.MaxSteeringAngle = 55.0f;
		Tune.Differential.AccelerationLock = 85;
		Tune.Differential.DecelerationLock = 30;
		Tune.Brakes.BrakeBias = 65;
		Tune.Brakes.bHandbrakeDriftMode = true;
		Tune.Brakes.bABSEnabled = false;
		Tune.Transmission.GearRatios = { 3.08f, 2.19f, 1.70f, 1.36f, 1.14f, 0.95f }; // Close ratio
		break;

	case EMGDrivingStylePreset::Drag:
		// Drag racing setup
		Tune.ECU.RevLimiter = 8000;
		Tune.ECU.LaunchControlRPM = 5500;
		Tune.ECU.bAntiLagEnabled = true;
		Tune.ForcedInduction.bHasTurbo = true;
		Tune.ForcedInduction.TargetBoostPSI = 25.0f;
		Tune.Suspension.FrontSpringRate = 60.0f;
		Tune.Suspension.RearSpringRate = 50.0f;
		Tune.Suspension.RearRideHeight = -20.0f; // Squat for traction
		Tune.Alignment.FrontCamber = 0.0f;
		Tune.Alignment.RearCamber = 0.0f;
		Tune.Differential.AccelerationLock = 100; // Locked
		Tune.Differential.DecelerationLock = 0;
		Tune.Brakes.BrakeBias = 55;
		Tune.Transmission.GearRatios = { 3.35f, 2.18f, 1.57f, 1.23f, 1.00f, 0.82f }; // Short ratios
		Tune.Transmission.FinalDriveRatio = 4.10f;
		Tune.Nitrous.ShotSizeHP = 150;
		Tune.Nitrous.bProgressiveController = true;
		break;

	case EMGDrivingStylePreset::TopSpeed:
		// Top speed setup
		Tune.ECU.RevLimiter = 7000;
		Tune.ECU.SpeedLimiterMPH = 0; // No limiter
		Tune.Suspension.FrontSpringRate = 90.0f;
		Tune.Suspension.RearSpringRate = 85.0f;
		Tune.Suspension.FrontRideHeight = -40.0f;
		Tune.Suspension.RearRideHeight = -30.0f;
		Tune.Alignment.FrontCamber = -0.5f;
		Tune.Alignment.RearCamber = -0.3f;
		Tune.Differential.AccelerationLock = 50;
		Tune.Brakes.BrakeBias = 60;
		Tune.Transmission.GearRatios = { 2.97f, 1.93f, 1.35f, 1.02f, 0.81f, 0.65f }; // Tall ratios
		Tune.Transmission.FinalDriveRatio = 3.23f;
		break;

	case EMGDrivingStylePreset::Touge:
		// Canyon/mountain road setup
		Tune.ECU.RevLimiter = 7200;
		Tune.ECU.LaunchControlRPM = 4200;
		Tune.Suspension.FrontSpringRate = 85.0f;
		Tune.Suspension.RearSpringRate = 75.0f;
		Tune.Suspension.FrontCompressionDamping = 18;
		Tune.Suspension.RearCompressionDamping = 16;
		Tune.Alignment.FrontCamber = -2.0f;
		Tune.Alignment.RearCamber = -1.0f;
		Tune.Alignment.Caster = 6.0f;
		Tune.Differential.AccelerationLock = 65;
		Tune.Differential.DecelerationLock = 45;
		Tune.Brakes.BrakeBias = 58;
		Tune.Brakes.bABSEnabled = true;
		Tune.Brakes.ABSSetting = 3; // Aggressive ABS
		Tune.Transmission.GearRatios = { 3.25f, 2.14f, 1.53f, 1.17f, 0.94f, 0.78f };
		break;

	default:
		// Custom preset starts as balanced
		break;
	}

	return Tune;
}
