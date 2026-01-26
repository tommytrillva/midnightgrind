// Copyright Midnight Grind. All Rights Reserved.

#include "Dyno/MGDynoSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Vehicle/MGStatCalculator.h"
#include "Vehicle/MGVehicleData.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// ==========================================
// FMGDynoResult Implementation
// ==========================================

float FMGDynoResult::GetHorsepowerAtRPM(int32 RPM, bool bWheelHP) const
{
	if (RPMPoints.Num() == 0)
	{
		return 0.0f;
	}

	const TArray<float>& HPCurve = bWheelHP ? WheelHorsepowerCurve : CrankHorsepowerCurve;
	if (HPCurve.Num() != RPMPoints.Num())
	{
		return 0.0f;
	}

	// Find surrounding points for interpolation
	for (int32 i = 0; i < RPMPoints.Num() - 1; ++i)
	{
		if (RPM >= RPMPoints[i] && RPM <= RPMPoints[i + 1])
		{
			const float Alpha = static_cast<float>(RPM - RPMPoints[i]) /
				static_cast<float>(RPMPoints[i + 1] - RPMPoints[i]);
			return FMath::Lerp(HPCurve[i], HPCurve[i + 1], Alpha);
		}
	}

	// Clamp to endpoints if outside range
	if (RPM < RPMPoints[0])
	{
		return HPCurve[0];
	}
	return HPCurve.Last();
}

float FMGDynoResult::GetTorqueAtRPM(int32 RPM, bool bWheelTorque) const
{
	if (RPMPoints.Num() == 0)
	{
		return 0.0f;
	}

	const TArray<float>& TorqueCurve = bWheelTorque ? WheelTorqueCurve : CrankTorqueCurve;
	if (TorqueCurve.Num() != RPMPoints.Num())
	{
		return 0.0f;
	}

	// Find surrounding points for interpolation
	for (int32 i = 0; i < RPMPoints.Num() - 1; ++i)
	{
		if (RPM >= RPMPoints[i] && RPM <= RPMPoints[i + 1])
		{
			const float Alpha = static_cast<float>(RPM - RPMPoints[i]) /
				static_cast<float>(RPMPoints[i + 1] - RPMPoints[i]);
			return FMath::Lerp(TorqueCurve[i], TorqueCurve[i + 1], Alpha);
		}
	}

	// Clamp to endpoints if outside range
	if (RPM < RPMPoints[0])
	{
		return TorqueCurve[0];
	}
	return TorqueCurve.Last();
}

// ==========================================
// UMGDynoSubsystem Implementation
// ==========================================

void UMGDynoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGDynoSubsystem: Initializing dyno testing system"));
	UE_LOG(LogTemp, Log, TEXT("MGDynoSubsystem: Dyno pull cost: $%lld"), DynoPullCost);
}

void UMGDynoSubsystem::Deinitialize()
{
	// Cancel any active sessions
	for (auto& SessionPair : ActiveSessions)
	{
		if (SessionPair.Value.State != EMGDynoRunState::Idle &&
			SessionPair.Value.State != EMGDynoRunState::Complete &&
			SessionPair.Value.State != EMGDynoRunState::Failed)
		{
			SetDynoState(SessionPair.Key, EMGDynoRunState::Failed);
		}
	}

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DynoTickHandle);
	}

	Super::Deinitialize();
}

// ==========================================
// DYNO OPERATIONS
// ==========================================

bool UMGDynoSubsystem::StartDynoRun(FGuid VehicleID, const FMGVehicleData& VehicleData,
									 const UMGVehicleModelData* BaseModel, const FString& TuneDescription)
{
	// Validate inputs
	if (!VehicleID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoSubsystem: Invalid vehicle ID"));
		return false;
	}

	if (!BaseModel)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoSubsystem: Base model is null"));
		return false;
	}

	// Check for existing active session
	if (ActiveSessions.Contains(VehicleID))
	{
		const FDynoSession& ExistingSession = ActiveSessions[VehicleID];
		if (ExistingSession.State != EMGDynoRunState::Idle &&
			ExistingSession.State != EMGDynoRunState::Complete &&
			ExistingSession.State != EMGDynoRunState::Failed)
		{
			UE_LOG(LogTemp, Warning, TEXT("MGDynoSubsystem: Dyno run already in progress for vehicle %s"),
				*VehicleID.ToString());
			return false;
		}
	}

	// Check if player can afford the dyno pull
	UMGEconomySubsystem* EconomySystem = GetEconomySubsystem();
	if (EconomySystem)
	{
		if (!EconomySystem->CanAfford(DynoPullCost))
		{
			UE_LOG(LogTemp, Warning, TEXT("MGDynoSubsystem: Cannot afford dyno pull (cost: $%lld)"), DynoPullCost);
			OnDynoRunFailed.Broadcast(VehicleID, FText::FromString(TEXT("Insufficient funds for dyno rental")));
			return false;
		}

		// Deduct cost
		EconomySystem->SpendCredits(DynoPullCost, EMGTransactionType::DynoRental,
			FText::FromString(TEXT("Dyno Rental")), FName(TEXT("DynoRental")));
		TotalDynoSpending += DynoPullCost;
	}

	// Create new session
	FDynoSession NewSession;
	NewSession.VehicleID = VehicleID;
	NewSession.VehicleData = VehicleData;
	NewSession.BaseModel = BaseModel;
	NewSession.TuneDescription = TuneDescription;
	NewSession.State = EMGDynoRunState::Preparing;
	NewSession.CurrentRPM = 0;
	NewSession.Progress = 0.0f;
	NewSession.StartTime = FDateTime::UtcNow();
	NewSession.LiveData.Empty();

	// Set drivetrain loss based on drivetrain type
	switch (VehicleData.Drivetrain.DrivetrainType)
	{
	case EMGDrivetrainType::FWD:
		NewSession.DrivetrainLossPercent = 12.0f;
		break;
	case EMGDrivetrainType::RWD:
		NewSession.DrivetrainLossPercent = 15.0f;
		break;
	case EMGDrivetrainType::AWD:
		NewSession.DrivetrainLossPercent = 20.0f;
		break;
	default:
		NewSession.DrivetrainLossPercent = 15.0f;
		break;
	}

	ActiveSessions.Add(VehicleID, NewSession);

	UE_LOG(LogTemp, Log, TEXT("MGDynoSubsystem: Starting dyno run for vehicle %s"), *VehicleID.ToString());

	// Broadcast start event
	OnDynoRunStarted.Broadcast(VehicleID, EMGDynoRunState::Preparing);

	// Start the simulation after a brief preparation phase
	if (UWorld* World = GetWorld())
	{
		FTimerDelegate PrepDelegate;
		PrepDelegate.BindLambda([this, VehicleID]()
		{
			FDynoSession* Session = ActiveSessions.Find(VehicleID);
			if (Session && Session->State == EMGDynoRunState::Preparing)
			{
				SetDynoState(VehicleID, EMGDynoRunState::WarmingUp);

				// After warmup, start the actual run
				if (UWorld* W = GetWorld())
				{
					FTimerDelegate WarmupDelegate;
					WarmupDelegate.BindLambda([this, VehicleID]()
					{
						FDynoSession* Sess = ActiveSessions.Find(VehicleID);
						if (Sess && Sess->State == EMGDynoRunState::WarmingUp)
						{
							Sess->CurrentRPM = DynoStartRPM;
							SetDynoState(VehicleID, EMGDynoRunState::Running);

							// Start the tick timer
							if (UWorld* World = GetWorld())
							{
								World->GetTimerManager().SetTimer(
									DynoTickHandle,
									FTimerDelegate::CreateUObject(this, &UMGDynoSubsystem::TickDynoSimulation, VehicleID),
									DynoTickInterval,
									true
								);
							}
						}
					});

					W->GetTimerManager().SetTimer(DynoTickHandle, WarmupDelegate, WarmupDuration, false);
				}
			}
		});

		World->GetTimerManager().SetTimer(DynoTickHandle, PrepDelegate, 0.5f, false);
	}

	return true;
}

void UMGDynoSubsystem::CancelDynoRun(FGuid VehicleID)
{
	FDynoSession* Session = ActiveSessions.Find(VehicleID);
	if (!Session)
	{
		return;
	}

	if (Session->State == EMGDynoRunState::Complete || Session->State == EMGDynoRunState::Failed)
	{
		return;
	}

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DynoTickHandle);
	}

	SetDynoState(VehicleID, EMGDynoRunState::Failed);
	OnDynoRunFailed.Broadcast(VehicleID, FText::FromString(TEXT("Dyno run cancelled by user")));

	UE_LOG(LogTemp, Log, TEXT("MGDynoSubsystem: Cancelled dyno run for vehicle %s"), *VehicleID.ToString());
}

bool UMGDynoSubsystem::GetDynoResults(FGuid VehicleID, FMGDynoResult& OutResult) const
{
	const FGuid* ResultIDPtr = LatestResultByVehicle.Find(VehicleID);
	if (!ResultIDPtr)
	{
		return false;
	}

	return GetDynoResultByID(*ResultIDPtr, OutResult);
}

bool UMGDynoSubsystem::GetDynoResultByID(FGuid ResultID, FMGDynoResult& OutResult) const
{
	const FMGDynoResult* Result = DynoHistory.Find(ResultID);
	if (Result)
	{
		OutResult = *Result;
		return true;
	}
	return false;
}

TArray<FMGDynoResult> UMGDynoSubsystem::GetDynoHistory(FGuid VehicleID, int32 MaxResults) const
{
	TArray<FMGDynoResult> Results;

	for (const auto& HistoryPair : DynoHistory)
	{
		if (HistoryPair.Value.VehicleID == VehicleID)
		{
			Results.Add(HistoryPair.Value);
		}
	}

	// Sort by timestamp (newest first)
	Results.Sort([](const FMGDynoResult& A, const FMGDynoResult& B)
	{
		return A.Timestamp > B.Timestamp;
	});

	// Limit results
	if (MaxResults > 0 && Results.Num() > MaxResults)
	{
		Results.SetNum(MaxResults);
	}

	return Results;
}

FString UMGDynoSubsystem::ExportDynoData(FGuid ResultID, EMGDynoExportFormat Format) const
{
	FMGDynoResult Result;
	if (!GetDynoResultByID(ResultID, Result))
	{
		return TEXT("");
	}

	switch (Format)
	{
	case EMGDynoExportFormat::CSV:
		return GenerateCSVExport(Result);
	case EMGDynoExportFormat::JSON:
		return GenerateJSONExport(Result);
	case EMGDynoExportFormat::Text:
		return GenerateTextExport(Result);
	default:
		return GenerateCSVExport(Result);
	}
}

bool UMGDynoSubsystem::ExportDynoDataToFile(FGuid ResultID, const FString& FilePath, EMGDynoExportFormat Format) const
{
	FString ExportData = ExportDynoData(ResultID, Format);
	if (ExportData.IsEmpty())
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(ExportData, *FilePath);
}

// ==========================================
// COMPARISON
// ==========================================

bool UMGDynoSubsystem::SetComparisonBaseline(FGuid VehicleID, FGuid ResultID)
{
	if (!DynoHistory.Contains(ResultID))
	{
		UE_LOG(LogTemp, Warning, TEXT("MGDynoSubsystem: Cannot set baseline - result not found"));
		return false;
	}

	ComparisonBaselines.Add(VehicleID, ResultID);
	UE_LOG(LogTemp, Log, TEXT("MGDynoSubsystem: Set comparison baseline for vehicle %s"), *VehicleID.ToString());
	return true;
}

void UMGDynoSubsystem::ClearComparisonBaseline(FGuid VehicleID)
{
	ComparisonBaselines.Remove(VehicleID);
}

bool UMGDynoSubsystem::CompareDynoResults(FGuid BaselineResultID, FGuid ComparisonResultID, FMGDynoComparison& OutComparison) const
{
	FMGDynoResult Baseline, Comparison;

	if (!GetDynoResultByID(BaselineResultID, Baseline) || !GetDynoResultByID(ComparisonResultID, Comparison))
	{
		return false;
	}

	OutComparison.BaselineResultID = BaselineResultID;
	OutComparison.ComparisonResultID = ComparisonResultID;

	// Calculate gains
	OutComparison.WheelHPGain = Comparison.PeakWheelHP - Baseline.PeakWheelHP;
	OutComparison.WheelHPGainPercent = Baseline.PeakWheelHP > 0.0f ?
		(OutComparison.WheelHPGain / Baseline.PeakWheelHP) * 100.0f : 0.0f;

	OutComparison.WheelTorqueGain = Comparison.PeakWheelTorque - Baseline.PeakWheelTorque;
	OutComparison.WheelTorqueGainPercent = Baseline.PeakWheelTorque > 0.0f ?
		(OutComparison.WheelTorqueGain / Baseline.PeakWheelTorque) * 100.0f : 0.0f;

	OutComparison.CrankHPGain = Comparison.PeakCrankHP - Baseline.PeakCrankHP;
	OutComparison.CrankTorqueGain = Comparison.PeakCrankTorque - Baseline.PeakCrankTorque;

	OutComparison.PowerBandWidthChange = Comparison.PowerBandWidth - Baseline.PowerBandWidth;
	OutComparison.PeakHPRPMShift = Comparison.PeakWheelHPRPM - Baseline.PeakWheelHPRPM;
	OutComparison.PeakTorqueRPMShift = Comparison.PeakWheelTorqueRPM - Baseline.PeakWheelTorqueRPM;

	// Generate summary
	FString Summary;
	if (OutComparison.WheelHPGain > 0)
	{
		Summary = FString::Printf(TEXT("+%.1f WHP (+%.1f%%), +%.1f lb-ft torque"),
			OutComparison.WheelHPGain, OutComparison.WheelHPGainPercent, OutComparison.WheelTorqueGain);
	}
	else if (OutComparison.WheelHPGain < 0)
	{
		Summary = FString::Printf(TEXT("%.1f WHP (%.1f%%), %.1f lb-ft torque"),
			OutComparison.WheelHPGain, OutComparison.WheelHPGainPercent, OutComparison.WheelTorqueGain);
	}
	else
	{
		Summary = TEXT("No significant change in power output");
	}

	OutComparison.GainSummary = FText::FromString(Summary);

	return true;
}

bool UMGDynoSubsystem::CompareLatestToBaseline(FGuid VehicleID, FMGDynoComparison& OutComparison) const
{
	const FGuid* BaselineID = ComparisonBaselines.Find(VehicleID);
	const FGuid* LatestID = LatestResultByVehicle.Find(VehicleID);

	if (!BaselineID || !LatestID)
	{
		return false;
	}

	return CompareDynoResults(*BaselineID, *LatestID, OutComparison);
}

bool UMGDynoSubsystem::HasComparisonBaseline(FGuid VehicleID) const
{
	return ComparisonBaselines.Contains(VehicleID);
}

// ==========================================
// ECONOMY
// ==========================================

bool UMGDynoSubsystem::CanAffordDynoPull() const
{
	UMGEconomySubsystem* EconomySystem = GetEconomySubsystem();
	if (EconomySystem)
	{
		return EconomySystem->CanAfford(DynoPullCost);
	}
	return true; // If no economy system, allow the pull
}

// ==========================================
// STATE QUERIES
// ==========================================

EMGDynoRunState UMGDynoSubsystem::GetDynoRunState(FGuid VehicleID) const
{
	const FDynoSession* Session = ActiveSessions.Find(VehicleID);
	return Session ? Session->State : EMGDynoRunState::Idle;
}

bool UMGDynoSubsystem::IsDynoRunInProgress(FGuid VehicleID) const
{
	EMGDynoRunState State = GetDynoRunState(VehicleID);
	return State != EMGDynoRunState::Idle &&
		   State != EMGDynoRunState::Complete &&
		   State != EMGDynoRunState::Failed;
}

int32 UMGDynoSubsystem::GetCurrentDynoRPM(FGuid VehicleID) const
{
	const FDynoSession* Session = ActiveSessions.Find(VehicleID);
	return Session ? Session->CurrentRPM : 0;
}

float UMGDynoSubsystem::GetDynoRunProgress(FGuid VehicleID) const
{
	const FDynoSession* Session = ActiveSessions.Find(VehicleID);
	return Session ? Session->Progress : 0.0f;
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGDynoSubsystem::SetDrivetrainLossPercent(FGuid VehicleID, float LossPercent)
{
	FDynoSession* Session = ActiveSessions.Find(VehicleID);
	if (Session)
	{
		Session->DrivetrainLossPercent = FMath::Clamp(LossPercent, 0.0f, 50.0f);
	}
}

void UMGDynoSubsystem::SetCorrectionStandard(EMGDynoCorrectionStandard Standard)
{
	CurrentCorrectionStandard = Standard;
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void UMGDynoSubsystem::TickDynoSimulation(FGuid VehicleID)
{
	FDynoSession* Session = ActiveSessions.Find(VehicleID);
	if (!Session || Session->State != EMGDynoRunState::Running)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DynoTickHandle);
		}
		return;
	}

	// Calculate redline from vehicle config
	const int32 Redline = UMGStatCalculator::CalculateRedline(Session->VehicleData.Engine, Session->BaseModel);

	// Simulate data point at current RPM
	FMGDynoDataPoint DataPoint = SimulateDataPoint(*Session, Session->CurrentRPM);
	Session->LiveData.Add(DataPoint);

	// Update progress
	Session->Progress = static_cast<float>(Session->CurrentRPM - DynoStartRPM) /
		static_cast<float>(Redline - DynoStartRPM);
	Session->Progress = FMath::Clamp(Session->Progress, 0.0f, 1.0f);

	// Broadcast live data
	OnDynoLiveData.Broadcast(VehicleID, Session->CurrentRPM, DataPoint.WheelHP);

	// Increment RPM
	Session->CurrentRPM += DynoRPMStep;

	// Check if we've reached redline
	if (Session->CurrentRPM >= Redline)
	{
		// Stop the tick timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DynoTickHandle);
		}

		// Start cooldown phase
		SetDynoState(VehicleID, EMGDynoRunState::CoolingDown);

		// After cooldown, complete the run
		if (UWorld* World = GetWorld())
		{
			FTimerDelegate CooldownDelegate;
			CooldownDelegate.BindLambda([this, VehicleID]()
			{
				FDynoSession* Sess = ActiveSessions.Find(VehicleID);
				if (Sess && Sess->State == EMGDynoRunState::CoolingDown)
				{
					// Calculate and store results
					FMGDynoResult Result = CalculateResults(*Sess);
					DynoHistory.Add(Result.ResultID, Result);
					LatestResultByVehicle.Add(VehicleID, Result.ResultID);

					SetDynoState(VehicleID, EMGDynoRunState::Complete);

					// Broadcast completion
					OnDynoRunComplete.Broadcast(VehicleID, Result);

					UE_LOG(LogTemp, Log, TEXT("MGDynoSubsystem: Dyno complete - Peak WHP: %.1f @ %d RPM, Peak Torque: %.1f @ %d RPM"),
						Result.PeakWheelHP, Result.PeakWheelHPRPM,
						Result.PeakWheelTorque, Result.PeakWheelTorqueRPM);
				}
			});

			World->GetTimerManager().SetTimer(DynoTickHandle, CooldownDelegate, CooldownDuration, false);
		}
	}
}

FMGDynoDataPoint UMGDynoSubsystem::SimulateDataPoint(const FDynoSession& Session, int32 RPM) const
{
	FMGDynoDataPoint Point;
	Point.RPM = RPM;

	// Calculate power curve from vehicle config
	FMGPowerCurve PowerCurve = UMGStatCalculator::CalculatePowerCurve(Session.VehicleData.Engine, Session.BaseModel);

	// Find the closest point in the power curve or interpolate
	float HP = 0.0f;
	float Torque = 0.0f;

	for (int32 i = 0; i < PowerCurve.CurvePoints.Num() - 1; ++i)
	{
		const FMGPowerCurvePoint& P1 = PowerCurve.CurvePoints[i];
		const FMGPowerCurvePoint& P2 = PowerCurve.CurvePoints[i + 1];

		if (RPM >= P1.RPM && RPM <= P2.RPM)
		{
			const float Alpha = static_cast<float>(RPM - P1.RPM) / static_cast<float>(P2.RPM - P1.RPM);
			HP = FMath::Lerp(P1.Horsepower, P2.Horsepower, Alpha);
			Torque = FMath::Lerp(P1.Torque, P2.Torque, Alpha);
			break;
		}
	}

	// If no match found, use the peak values with some falloff
	if (HP == 0.0f && PowerCurve.CurvePoints.Num() > 0)
	{
		if (RPM < PowerCurve.CurvePoints[0].RPM)
		{
			HP = PowerCurve.CurvePoints[0].Horsepower * 0.6f;
			Torque = PowerCurve.CurvePoints[0].Torque * 0.7f;
		}
		else
		{
			HP = PowerCurve.CurvePoints.Last().Horsepower * 0.95f;
			Torque = PowerCurve.CurvePoints.Last().Torque * 0.9f;
		}
	}

	// These are crank values from the stat calculator
	Point.CrankHP = HP;
	Point.CrankTorque = Torque;

	// Calculate wheel values with drivetrain loss
	const float LossMultiplier = 1.0f - (Session.DrivetrainLossPercent / 100.0f);
	Point.WheelHP = HP * LossMultiplier;
	Point.WheelTorque = Torque * LossMultiplier;

	// Simulate boost if applicable
	if (Session.VehicleData.Engine.ForcedInduction.Type != EMGForcedInductionType::None)
	{
		const float MaxBoost = Session.VehicleData.Engine.ForcedInduction.MaxBoostPSI;
		const float NormalizedRPM = static_cast<float>(RPM) / static_cast<float>(PowerCurve.Redline);

		// Boost builds with RPM, peaks around 80% of redline
		Point.BoostPSI = MaxBoost * FMath::Sin(FMath::Clamp(NormalizedRPM * 1.25f, 0.0f, 1.0f) * PI * 0.5f);
	}

	// Simulate AFR (richer at higher RPM for power)
	const float NormalizedRPM = static_cast<float>(RPM) / static_cast<float>(PowerCurve.Redline);
	Point.AirFuelRatio = 14.7f - (NormalizedRPM * 2.0f); // Goes from ~14.7 to ~12.7

	// Add some realistic variation
	Point.WheelHP *= FMath::RandRange(0.98f, 1.02f);
	Point.WheelTorque *= FMath::RandRange(0.98f, 1.02f);

	return Point;
}

FMGDynoResult UMGDynoSubsystem::CalculateResults(const FDynoSession& Session) const
{
	FMGDynoResult Result;
	Result.ResultID = FGuid::NewGuid();
	Result.VehicleID = Session.VehicleID;
	Result.Timestamp = FDateTime::UtcNow();
	Result.TuneDescription = Session.TuneDescription;
	Result.DrivetrainLossPercent = Session.DrivetrainLossPercent;
	Result.CorrectionStandard = CurrentCorrectionStandard;

	// Atmospheric conditions (simulated)
	Result.AmbientTempC = FMath::RandRange(20.0f, 30.0f);
	Result.BarometricPressure = FMath::RandRange(1000.0f, 1025.0f);
	Result.RelativeHumidity = FMath::RandRange(40.0f, 60.0f);

	// Calculate correction factor
	Result.CorrectionFactor = CalculateSAECorrectionFactor(
		Result.AmbientTempC, Result.BarometricPressure, Result.RelativeHumidity);

	// Transfer data points to arrays
	for (const FMGDynoDataPoint& Point : Session.LiveData)
	{
		Result.RPMPoints.Add(Point.RPM);
		Result.WheelHorsepowerCurve.Add(Point.WheelHP * Result.CorrectionFactor);
		Result.WheelTorqueCurve.Add(Point.WheelTorque * Result.CorrectionFactor);
		Result.CrankHorsepowerCurve.Add(Point.CrankHP * Result.CorrectionFactor);
		Result.CrankTorqueCurve.Add(Point.CrankTorque * Result.CorrectionFactor);
		Result.BoostCurve.Add(Point.BoostPSI);
		Result.AFRCurve.Add(Point.AirFuelRatio);
	}

	// Find peak values
	Result.PeakWheelHP = 0.0f;
	Result.PeakWheelTorque = 0.0f;
	Result.PeakCrankHP = 0.0f;
	Result.PeakCrankTorque = 0.0f;
	Result.PeakBoostPSI = 0.0f;

	for (int32 i = 0; i < Result.RPMPoints.Num(); ++i)
	{
		if (Result.WheelHorsepowerCurve[i] > Result.PeakWheelHP)
		{
			Result.PeakWheelHP = Result.WheelHorsepowerCurve[i];
			Result.PeakWheelHPRPM = Result.RPMPoints[i];
		}
		if (Result.WheelTorqueCurve[i] > Result.PeakWheelTorque)
		{
			Result.PeakWheelTorque = Result.WheelTorqueCurve[i];
			Result.PeakWheelTorqueRPM = Result.RPMPoints[i];
		}
		if (Result.CrankHorsepowerCurve[i] > Result.PeakCrankHP)
		{
			Result.PeakCrankHP = Result.CrankHorsepowerCurve[i];
			Result.PeakCrankHPRPM = Result.RPMPoints[i];
		}
		if (Result.CrankTorqueCurve[i] > Result.PeakCrankTorque)
		{
			Result.PeakCrankTorque = Result.CrankTorqueCurve[i];
			Result.PeakCrankTorqueRPM = Result.RPMPoints[i];
		}
		if (Result.BoostCurve[i] > Result.PeakBoostPSI)
		{
			Result.PeakBoostPSI = Result.BoostCurve[i];
		}
	}

	// Calculate drivetrain loss
	Result.DrivetrainLossHP = Result.PeakCrankHP - Result.PeakWheelHP;

	// Calculate power band (90% of peak HP)
	const float PowerBandThreshold = Result.PeakWheelHP * 0.9f;
	Result.PowerBandStartRPM = 0;
	Result.PowerBandEndRPM = 0;

	for (int32 i = 0; i < Result.RPMPoints.Num(); ++i)
	{
		if (Result.WheelHorsepowerCurve[i] >= PowerBandThreshold)
		{
			if (Result.PowerBandStartRPM == 0)
			{
				Result.PowerBandStartRPM = Result.RPMPoints[i];
			}
			Result.PowerBandEndRPM = Result.RPMPoints[i];
		}
	}

	Result.PowerBandWidth = Result.PowerBandEndRPM - Result.PowerBandStartRPM;

	// Set redline
	Result.RedlineRPM = Result.RPMPoints.Num() > 0 ? Result.RPMPoints.Last() : 7000;

	return Result;
}

float UMGDynoSubsystem::CalculateSAECorrectionFactor(float TempC, float PressureMbar, float HumidityPercent) const
{
	// SAE J1349 correction formula (simplified)
	// CF = (1013.25 / P) * ((T + 273) / 298)^0.5
	// Where P = pressure in mbar, T = temp in Celsius

	const float PressureRatio = 1013.25f / PressureMbar;
	const float TempRatio = FMath::Sqrt((TempC + 273.0f) / 298.0f);

	// Humidity correction (approximate)
	const float HumidityFactor = 1.0f - (HumidityPercent * 0.0001f);

	float CorrectionFactor = PressureRatio * TempRatio * HumidityFactor;

	// Clamp to reasonable range
	return FMath::Clamp(CorrectionFactor, 0.9f, 1.1f);
}

void UMGDynoSubsystem::SetDynoState(FGuid VehicleID, EMGDynoRunState NewState)
{
	FDynoSession* Session = ActiveSessions.Find(VehicleID);
	if (!Session)
	{
		return;
	}

	EMGDynoRunState OldState = Session->State;
	Session->State = NewState;

	OnDynoStateChanged.Broadcast(VehicleID, OldState, NewState);
}

UMGEconomySubsystem* UMGDynoSubsystem::GetEconomySubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMGEconomySubsystem>();
	}
	return nullptr;
}

FString UMGDynoSubsystem::GenerateCSVExport(const FMGDynoResult& Result) const
{
	FString Output;

	// Header
	Output += TEXT("# Midnight Grind Dyno Results\n");
	Output += FString::Printf(TEXT("# Date: %s\n"), *Result.Timestamp.ToString());
	Output += FString::Printf(TEXT("# Vehicle ID: %s\n"), *Result.VehicleID.ToString());
	Output += FString::Printf(TEXT("# Tune: %s\n"), *Result.TuneDescription);
	Output += FString::Printf(TEXT("# Peak Wheel HP: %.1f @ %d RPM\n"), Result.PeakWheelHP, Result.PeakWheelHPRPM);
	Output += FString::Printf(TEXT("# Peak Wheel Torque: %.1f lb-ft @ %d RPM\n"), Result.PeakWheelTorque, Result.PeakWheelTorqueRPM);
	Output += FString::Printf(TEXT("# Drivetrain Loss: %.1f%%\n"), Result.DrivetrainLossPercent);
	Output += TEXT("# \n");

	// Column headers
	Output += TEXT("RPM,WheelHP,WheelTorque,CrankHP,CrankTorque,BoostPSI,AFR\n");

	// Data rows
	for (int32 i = 0; i < Result.RPMPoints.Num(); ++i)
	{
		Output += FString::Printf(TEXT("%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n"),
			Result.RPMPoints[i],
			Result.WheelHorsepowerCurve[i],
			Result.WheelTorqueCurve[i],
			Result.CrankHorsepowerCurve[i],
			Result.CrankTorqueCurve[i],
			Result.BoostCurve[i],
			Result.AFRCurve[i]);
	}

	return Output;
}

FString UMGDynoSubsystem::GenerateJSONExport(const FMGDynoResult& Result) const
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	// Metadata
	RootObject->SetStringField(TEXT("resultId"), Result.ResultID.ToString());
	RootObject->SetStringField(TEXT("vehicleId"), Result.VehicleID.ToString());
	RootObject->SetStringField(TEXT("timestamp"), Result.Timestamp.ToString());
	RootObject->SetStringField(TEXT("tuneDescription"), Result.TuneDescription);

	// Peak values
	TSharedPtr<FJsonObject> PeaksObject = MakeShareable(new FJsonObject);
	PeaksObject->SetNumberField(TEXT("peakWheelHP"), Result.PeakWheelHP);
	PeaksObject->SetNumberField(TEXT("peakWheelHPRPM"), Result.PeakWheelHPRPM);
	PeaksObject->SetNumberField(TEXT("peakWheelTorque"), Result.PeakWheelTorque);
	PeaksObject->SetNumberField(TEXT("peakWheelTorqueRPM"), Result.PeakWheelTorqueRPM);
	PeaksObject->SetNumberField(TEXT("peakCrankHP"), Result.PeakCrankHP);
	PeaksObject->SetNumberField(TEXT("peakCrankTorque"), Result.PeakCrankTorque);
	PeaksObject->SetNumberField(TEXT("drivetrainLossPercent"), Result.DrivetrainLossPercent);
	RootObject->SetObjectField(TEXT("peaks"), PeaksObject);

	// Curve data
	TArray<TSharedPtr<FJsonValue>> DataPoints;
	for (int32 i = 0; i < Result.RPMPoints.Num(); ++i)
	{
		TSharedPtr<FJsonObject> PointObject = MakeShareable(new FJsonObject);
		PointObject->SetNumberField(TEXT("rpm"), Result.RPMPoints[i]);
		PointObject->SetNumberField(TEXT("wheelHP"), Result.WheelHorsepowerCurve[i]);
		PointObject->SetNumberField(TEXT("wheelTorque"), Result.WheelTorqueCurve[i]);
		PointObject->SetNumberField(TEXT("crankHP"), Result.CrankHorsepowerCurve[i]);
		PointObject->SetNumberField(TEXT("crankTorque"), Result.CrankTorqueCurve[i]);
		PointObject->SetNumberField(TEXT("boostPSI"), Result.BoostCurve[i]);
		PointObject->SetNumberField(TEXT("afr"), Result.AFRCurve[i]);
		DataPoints.Add(MakeShareable(new FJsonValueObject(PointObject)));
	}
	RootObject->SetArrayField(TEXT("curveData"), DataPoints);

	// Conditions
	TSharedPtr<FJsonObject> ConditionsObject = MakeShareable(new FJsonObject);
	ConditionsObject->SetNumberField(TEXT("ambientTempC"), Result.AmbientTempC);
	ConditionsObject->SetNumberField(TEXT("barometricPressure"), Result.BarometricPressure);
	ConditionsObject->SetNumberField(TEXT("relativeHumidity"), Result.RelativeHumidity);
	ConditionsObject->SetNumberField(TEXT("correctionFactor"), Result.CorrectionFactor);
	RootObject->SetObjectField(TEXT("conditions"), ConditionsObject);

	// Serialize to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return OutputString;
}

FString UMGDynoSubsystem::GenerateTextExport(const FMGDynoResult& Result) const
{
	FString Output;

	Output += TEXT("================================================================================\n");
	Output += TEXT("                    MIDNIGHT GRIND DYNO RESULTS                                 \n");
	Output += TEXT("================================================================================\n\n");

	Output += FString::Printf(TEXT("Date:          %s\n"), *Result.Timestamp.ToString());
	Output += FString::Printf(TEXT("Vehicle ID:    %s\n"), *Result.VehicleID.ToString());
	Output += FString::Printf(TEXT("Configuration: %s\n\n"), *Result.TuneDescription);

	Output += TEXT("--- PEAK POWER ---\n");
	Output += FString::Printf(TEXT("Peak Wheel HP:      %.1f HP @ %d RPM\n"), Result.PeakWheelHP, Result.PeakWheelHPRPM);
	Output += FString::Printf(TEXT("Peak Wheel Torque:  %.1f lb-ft @ %d RPM\n"), Result.PeakWheelTorque, Result.PeakWheelTorqueRPM);
	Output += FString::Printf(TEXT("Est. Crank HP:      %.1f HP @ %d RPM\n"), Result.PeakCrankHP, Result.PeakCrankHPRPM);
	Output += FString::Printf(TEXT("Est. Crank Torque:  %.1f lb-ft @ %d RPM\n\n"), Result.PeakCrankTorque, Result.PeakCrankTorqueRPM);

	Output += TEXT("--- DRIVETRAIN ---\n");
	Output += FString::Printf(TEXT("Drivetrain Loss:    %.1f%% (%.1f HP)\n\n"), Result.DrivetrainLossPercent, Result.DrivetrainLossHP);

	Output += TEXT("--- POWER BAND ---\n");
	Output += FString::Printf(TEXT("Usable Range:       %d - %d RPM (%d RPM width)\n"),
		Result.PowerBandStartRPM, Result.PowerBandEndRPM, Result.PowerBandWidth);
	Output += FString::Printf(TEXT("Redline:            %d RPM\n\n"), Result.RedlineRPM);

	if (Result.PeakBoostPSI > 0.0f)
	{
		Output += TEXT("--- BOOST ---\n");
		Output += FString::Printf(TEXT("Peak Boost:         %.1f PSI\n\n"), Result.PeakBoostPSI);
	}

	Output += TEXT("--- CONDITIONS ---\n");
	Output += FString::Printf(TEXT("Temperature:        %.1f C\n"), Result.AmbientTempC);
	Output += FString::Printf(TEXT("Pressure:           %.1f mbar\n"), Result.BarometricPressure);
	Output += FString::Printf(TEXT("Humidity:           %.1f%%\n"), Result.RelativeHumidity);
	Output += FString::Printf(TEXT("Correction Factor:  %.3f\n\n"), Result.CorrectionFactor);

	Output += TEXT("================================================================================\n");
	Output += TEXT("                           POWER CURVE DATA                                     \n");
	Output += TEXT("================================================================================\n\n");
	Output += TEXT("   RPM    |   WHP   |  W-TQ   |   CHP   |  C-TQ   | Boost | AFR\n");
	Output += TEXT("----------|---------|---------|---------|---------|-------|------\n");

	for (int32 i = 0; i < Result.RPMPoints.Num(); ++i)
	{
		Output += FString::Printf(TEXT("  %5d   | %6.1f  | %6.1f  | %6.1f  | %6.1f  | %4.1f  | %4.1f\n"),
			Result.RPMPoints[i],
			Result.WheelHorsepowerCurve[i],
			Result.WheelTorqueCurve[i],
			Result.CrankHorsepowerCurve[i],
			Result.CrankTorqueCurve[i],
			Result.BoostCurve[i],
			Result.AFRCurve[i]);
	}

	Output += TEXT("\n================================================================================\n");

	return Output;
}
