// Copyright Midnight Grind. All Rights Reserved.

#include "AntiCheat/MGAntiCheatSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/SecureHash.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMisc.h"

void UMGAntiCheatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LocalPlayerID = FPlatformMisc::GetDeviceId();
	InitializeChecksums();

	// Start periodic integrity checks
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			IntegrityCheckTimerHandle,
			this,
			&UMGAntiCheatSubsystem::PeriodicIntegrityCheck,
			60.0f, // Every minute
			true
		);
	}
}

void UMGAntiCheatSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(IntegrityCheckTimerHandle);
	}
	Super::Deinitialize();
}

// ==========================================
// REAL-TIME VALIDATION
// ==========================================

bool UMGAntiCheatSubsystem::ValidatePosition(const FString& PlayerID, FVector Position, FVector PreviousPosition, float DeltaTime)
{
	if (!bAntiCheatEnabled || DeltaTime <= 0)
	{
		return true;
	}

	float Distance = FVector::Distance(Position, PreviousPosition);
	float Speed = Distance / DeltaTime;

	// Check for teleportation
	if (Distance > ValidationThresholds.MaxTeleportDistance)
	{
		TMap<FString, FString> Evidence;
		Evidence.Add(TEXT("Distance"), FString::Printf(TEXT("%.2f"), Distance));
		Evidence.Add(TEXT("Threshold"), FString::Printf(TEXT("%.2f"), ValidationThresholds.MaxTeleportDistance));
		Evidence.Add(TEXT("From"), PreviousPosition.ToString());
		Evidence.Add(TEXT("To"), Position.ToString());

		RecordViolation(PlayerID, EMGViolationType::TeleportHack, EMGViolationSeverity::Major,
			TEXT("Possible teleportation detected"), Evidence);
		return false;
	}

	// Update last position
	LastPlayerPositions.Add(PlayerID, Position);

	return true;
}

bool UMGAntiCheatSubsystem::ValidateSpeed(const FString& PlayerID, float CurrentSpeed, FName VehicleID)
{
	if (!bAntiCheatEnabled)
	{
		return true;
	}

	// Convert to km/h if needed
	float SpeedKmh = CurrentSpeed * 3.6f; // Assuming CurrentSpeed is in m/s

	if (SpeedKmh > ValidationThresholds.MaxSpeed * 1.2f) // 20% tolerance
	{
		TMap<FString, FString> Evidence;
		Evidence.Add(TEXT("Speed"), FString::Printf(TEXT("%.2f km/h"), SpeedKmh));
		Evidence.Add(TEXT("MaxSpeed"), FString::Printf(TEXT("%.2f km/h"), ValidationThresholds.MaxSpeed));
		Evidence.Add(TEXT("VehicleID"), VehicleID.ToString());

		RecordViolation(PlayerID, EMGViolationType::SpeedHack, EMGViolationSeverity::Major,
			TEXT("Impossible speed detected"), Evidence);
		return false;
	}

	return true;
}

bool UMGAntiCheatSubsystem::ValidateLapTime(const FString& PlayerID, float LapTime, FName TrackID)
{
	if (!bAntiCheatEnabled)
	{
		return true;
	}

	// Check for impossibly fast lap
	if (LapTime < ValidationThresholds.MinLapTime)
	{
		TMap<FString, FString> Evidence;
		Evidence.Add(TEXT("LapTime"), FString::Printf(TEXT("%.3f"), LapTime));
		Evidence.Add(TEXT("MinTime"), FString::Printf(TEXT("%.3f"), ValidationThresholds.MinLapTime));
		Evidence.Add(TEXT("TrackID"), TrackID.ToString());

		RecordViolation(PlayerID, EMGViolationType::TimerManipulation, EMGViolationSeverity::Critical,
			TEXT("Impossibly fast lap time"), Evidence);
		return false;
	}

	return true;
}

bool UMGAntiCheatSubsystem::ValidateRaceResult(const FString& PlayerID, int32 Position, float RaceTime, int32 CashEarned, int32 XPEarned)
{
	if (!bAntiCheatEnabled)
	{
		return true;
	}

	bool bValid = true;

	// Check currency rewards
	if (CashEarned > ValidationThresholds.MaxCurrencyPerRace)
	{
		TMap<FString, FString> Evidence;
		Evidence.Add(TEXT("CashEarned"), FString::Printf(TEXT("%d"), CashEarned));
		Evidence.Add(TEXT("MaxAllowed"), FString::Printf(TEXT("%d"), ValidationThresholds.MaxCurrencyPerRace));

		RecordViolation(PlayerID, EMGViolationType::ResourceHack, EMGViolationSeverity::Critical,
			TEXT("Excessive cash reward"), Evidence);
		bValid = false;
	}

	if (XPEarned > ValidationThresholds.MaxXPPerRace)
	{
		TMap<FString, FString> Evidence;
		Evidence.Add(TEXT("XPEarned"), FString::Printf(TEXT("%d"), XPEarned));
		Evidence.Add(TEXT("MaxAllowed"), FString::Printf(TEXT("%d"), ValidationThresholds.MaxXPPerRace));

		RecordViolation(PlayerID, EMGViolationType::ResourceHack, EMGViolationSeverity::Critical,
			TEXT("Excessive XP reward"), Evidence);
		bValid = false;
	}

	return bValid;
}

bool UMGAntiCheatSubsystem::ValidateCurrencyTransaction(const FString& PlayerID, int32 Amount, const FString& Source)
{
	if (!bAntiCheatEnabled)
	{
		return true;
	}

	// Check for suspicious transactions
	if (Amount > 1000000) // 1 million
	{
		TMap<FString, FString> Evidence;
		Evidence.Add(TEXT("Amount"), FString::Printf(TEXT("%d"), Amount));
		Evidence.Add(TEXT("Source"), Source);

		RecordViolation(PlayerID, EMGViolationType::ResourceHack, EMGViolationSeverity::Critical,
			TEXT("Suspicious currency transaction"), Evidence);
		return false;
	}

	return true;
}

bool UMGAntiCheatSubsystem::ValidateVehicleStats(FName VehicleID, float Speed, float Acceleration, float Handling)
{
	if (!bAntiCheatEnabled)
	{
		return true;
	}

	// Check for modified vehicle stats
	// Would compare against server-side vehicle data
	return true;
}

// ==========================================
// INTEGRITY CHECKS
// ==========================================

FMGIntegrityCheckResult UMGAntiCheatSubsystem::RunIntegrityCheck()
{
	FMGIntegrityCheckResult Result;
	Result.CheckTime = FDateTime::Now();
	Result.bPassed = true;

	// File integrity
	if (!CheckFileIntegrity())
	{
		Result.bPassed = false;
		Result.FailedChecks.Add(TEXT("File Integrity"));
	}

	// Memory integrity
	if (!CheckMemoryIntegrity())
	{
		Result.bPassed = false;
		Result.FailedChecks.Add(TEXT("Memory Integrity"));
		Result.bMemoryModified = true;
	}

	// Debugger detection
	if (CheckForDebugger())
	{
		Result.bPassed = false;
		Result.FailedChecks.Add(TEXT("Debugger Detected"));
		Result.bDebuggerDetected = true;
	}

	// VM detection
	if (CheckForVirtualMachine())
	{
		Result.FailedChecks.Add(TEXT("Virtual Machine Detected"));
		Result.bVirtualMachineDetected = true;
		// Don't fail for VM - could be legitimate
	}

	// Time sync
	if (!VerifyTimeSync())
	{
		Result.bPassed = false;
		Result.FailedChecks.Add(TEXT("Time Desync"));
	}

	if (!Result.bPassed)
	{
		TMap<FString, FString> Evidence;
		for (const FString& Check : Result.FailedChecks)
		{
			Evidence.Add(Check, TEXT("Failed"));
		}

		RecordViolation(LocalPlayerID, EMGViolationType::MemoryManipulation, EMGViolationSeverity::Critical,
			TEXT("Integrity check failed"), Evidence);
	}

	return Result;
}

bool UMGAntiCheatSubsystem::CheckFileIntegrity()
{
	for (const auto& Pair : ExpectedFileChecksums)
	{
		FString CurrentChecksum = CalculateFileChecksum(Pair.Key);
		if (CurrentChecksum != Pair.Value)
		{
			return false;
		}
	}
	return true;
}

bool UMGAntiCheatSubsystem::CheckMemoryIntegrity()
{
	// Would check for memory modifications
	// Using memory checksums, canary values, etc.
	return true;
}

bool UMGAntiCheatSubsystem::CheckForDebugger()
{
#if !UE_BUILD_SHIPPING
	return false; // Allow debugging in non-shipping builds
#endif

	return FPlatformMisc::IsDebuggerPresent();
}

bool UMGAntiCheatSubsystem::CheckForVirtualMachine()
{
	// Check for VM indicators
	// This is informational only, not a block
	return false;
}

bool UMGAntiCheatSubsystem::VerifyTimeSync()
{
	// Would compare local time with server time
	// Check if desync is within threshold
	return FMath::Abs(ServerTimeOffset) < ValidationThresholds.TimeDesyncThreshold;
}

// ==========================================
// ANOMALY DETECTION
// ==========================================

void UMGAntiCheatSubsystem::ReportAnomaly(const FString& PlayerID, EMGViolationType Type, const FString& Description,
	const TMap<FString, FString>& Evidence)
{
	RecordViolation(PlayerID, Type, EMGViolationSeverity::Warning, Description, Evidence);
}

bool UMGAntiCheatSubsystem::CheckStatisticalAnomaly(const FString& PlayerID, const FString& StatType, float Value)
{
	// Would compare against expected statistical distributions
	// Flag if value is multiple standard deviations from mean
	return true;
}

bool UMGAntiCheatSubsystem::DetectInputAnomaly(const TArray<FVector2D>& InputHistory)
{
	if (InputHistory.Num() < 10)
	{
		return false;
	}

	// Check for inhuman input patterns
	// - Perfectly consistent timing
	// - Impossible precision
	// - Repeated exact sequences

	int32 IdenticalCount = 0;
	for (int32 i = 1; i < InputHistory.Num(); i++)
	{
		if (InputHistory[i].Equals(InputHistory[i - 1], 0.001f))
		{
			IdenticalCount++;
		}
	}

	// If more than 50% are identical, suspicious
	return (float)IdenticalCount / (float)InputHistory.Num() > 0.5f;
}

// ==========================================
// PLAYER REPORTS
// ==========================================

bool UMGAntiCheatSubsystem::ReportPlayer(const FString& ReportedPlayerID, const FString& Reason, const FString& Description)
{
	if (!CanReportPlayer(ReportedPlayerID))
	{
		return false;
	}

	FMGPlayerReport Report;
	Report.ReportID = FGuid::NewGuid().ToString();
	Report.ReporterID = LocalPlayerID;
	Report.ReportedPlayerID = ReportedPlayerID;
	Report.Reason = Reason;
	Report.Description = Description;
	Report.Timestamp = FDateTime::Now();

	PlayerReports.Add(Report);
	LastReportTimes.Add(ReportedPlayerID, FDateTime::Now());

	// Would send to server

	return true;
}

TArray<FMGPlayerReport> UMGAntiCheatSubsystem::GetPlayerReports(const FString& PlayerID) const
{
	TArray<FMGPlayerReport> Result;
	for (const FMGPlayerReport& Report : PlayerReports)
	{
		if (Report.ReportedPlayerID == PlayerID)
		{
			Result.Add(Report);
		}
	}
	return Result;
}

int32 UMGAntiCheatSubsystem::GetReportCount(const FString& PlayerID) const
{
	int32 Count = 0;
	for (const FMGPlayerReport& Report : PlayerReports)
	{
		if (Report.ReportedPlayerID == PlayerID)
		{
			Count++;
		}
	}
	return Count;
}

bool UMGAntiCheatSubsystem::CanReportPlayer(const FString& PlayerID) const
{
	if (const FDateTime* LastReport = LastReportTimes.Find(PlayerID))
	{
		return (FDateTime::Now() - *LastReport).GetTotalSeconds() > ReportCooldownSeconds;
	}
	return true;
}

// ==========================================
// TRUST SYSTEM
// ==========================================

EMGTrustLevel UMGAntiCheatSubsystem::GetPlayerTrustLevel(const FString& PlayerID) const
{
	int32 Score = GetTrustScore(PlayerID);
	return GetTrustLevelFromScore(Score);
}

void UMGAntiCheatSubsystem::IncreaseTrustScore(const FString& PlayerID, int32 Amount)
{
	int32& Score = TrustScores.FindOrAdd(PlayerID);
	Score = FMath::Min(1000, Score + Amount); // Cap at 1000

	UpdateTrustLevel(PlayerID);
}

void UMGAntiCheatSubsystem::DecreaseTrustScore(const FString& PlayerID, int32 Amount)
{
	int32& Score = TrustScores.FindOrAdd(PlayerID);
	Score = FMath::Max(-1000, Score - Amount); // Floor at -1000

	UpdateTrustLevel(PlayerID);
}

int32 UMGAntiCheatSubsystem::GetTrustScore(const FString& PlayerID) const
{
	if (const int32* Score = TrustScores.Find(PlayerID))
	{
		return *Score;
	}
	return 500; // Default neutral score
}

// ==========================================
// VIOLATION MANAGEMENT
// ==========================================

TArray<FMGViolationRecord> UMGAntiCheatSubsystem::GetViolationHistory(const FString& PlayerID) const
{
	TArray<FMGViolationRecord> Result;
	for (const FMGViolationRecord& Record : ViolationRecords)
	{
		if (Record.PlayerID == PlayerID)
		{
			Result.Add(Record);
		}
	}
	return Result;
}

int32 UMGAntiCheatSubsystem::GetViolationCount(const FString& PlayerID, EMGViolationType Type) const
{
	int32 Count = 0;
	for (const FMGViolationRecord& Record : ViolationRecords)
	{
		if (Record.PlayerID == PlayerID)
		{
			if (Type == EMGViolationType::Unknown || Record.Type == Type)
			{
				Count++;
			}
		}
	}
	return Count;
}

void UMGAntiCheatSubsystem::ClearOldViolations(int32 DaysOld)
{
	FDateTime Cutoff = FDateTime::Now() - FTimespan::FromDays(DaysOld);

	ViolationRecords.RemoveAll([Cutoff](const FMGViolationRecord& Record) {
		return Record.Timestamp < Cutoff;
	});
}

// ==========================================
// BAN MANAGEMENT
// ==========================================

bool UMGAntiCheatSubsystem::IsPlayerBanned(const FString& PlayerID) const
{
	if (const FDateTime* BanExpiry = BanList.Find(PlayerID))
	{
		return *BanExpiry > FDateTime::Now();
	}
	return false;
}

FString UMGAntiCheatSubsystem::GetBanReason(const FString& PlayerID) const
{
	if (const FString* Reason = BanReasons.Find(PlayerID))
	{
		return *Reason;
	}
	return FString();
}

FDateTime UMGAntiCheatSubsystem::GetBanExpiry(const FString& PlayerID) const
{
	if (const FDateTime* Expiry = BanList.Find(PlayerID))
	{
		return *Expiry;
	}
	return FDateTime::MinValue();
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGAntiCheatSubsystem::SetValidationThresholds(const FMGValidationThresholds& Thresholds)
{
	ValidationThresholds = Thresholds;
}

void UMGAntiCheatSubsystem::SetAntiCheatEnabled(bool bEnabled)
{
	bAntiCheatEnabled = bEnabled;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGAntiCheatSubsystem::InitializeChecksums()
{
	// Would load expected file checksums from secure storage
	// These would be verified against game files
}

void UMGAntiCheatSubsystem::RecordViolation(const FString& PlayerID, EMGViolationType Type, EMGViolationSeverity Severity,
	const FString& Description, const TMap<FString, FString>& Evidence)
{
	FMGViolationRecord Record;
	Record.ViolationID = FGuid::NewGuid().ToString();
	Record.PlayerID = PlayerID;
	Record.Type = Type;
	Record.Severity = Severity;
	Record.Description = Description;
	Record.Evidence = Evidence;
	Record.Timestamp = FDateTime::Now();

	ViolationRecords.Add(Record);

	// Decrease trust score based on severity
	int32 TrustPenalty = 0;
	switch (Severity)
	{
		case EMGViolationSeverity::Info: TrustPenalty = 0; break;
		case EMGViolationSeverity::Warning: TrustPenalty = 10; break;
		case EMGViolationSeverity::Minor: TrustPenalty = 25; break;
		case EMGViolationSeverity::Major: TrustPenalty = 50; break;
		case EMGViolationSeverity::Critical: TrustPenalty = 100; break;
	}

	if (TrustPenalty > 0)
	{
		DecreaseTrustScore(PlayerID, TrustPenalty);
	}

	// Report to server
	ReportToServer(Record);

	// Apply automatic penalty
	ApplyAutomaticPenalty(PlayerID, Record);

	OnViolationDetected.Broadcast(Record);
}

void UMGAntiCheatSubsystem::UpdateTrustLevel(const FString& PlayerID)
{
	EMGTrustLevel NewLevel = GetPlayerTrustLevel(PlayerID);

	if (PlayerID == LocalPlayerID)
	{
		if (NewLevel != LocalTrustLevel)
		{
			LocalTrustLevel = NewLevel;
			OnTrustLevelChanged.Broadcast(NewLevel);
		}
	}
}

EMGTrustLevel UMGAntiCheatSubsystem::GetTrustLevelFromScore(int32 Score) const
{
	if (Score <= -500) return EMGTrustLevel::Banned;
	if (Score <= -100) return EMGTrustLevel::Flagged;
	if (Score <= 200) return EMGTrustLevel::Suspicious;
	if (Score >= 800) return EMGTrustLevel::Trusted;
	return EMGTrustLevel::Normal;
}

void UMGAntiCheatSubsystem::ApplyAutomaticPenalty(const FString& PlayerID, const FMGViolationRecord& Violation)
{
	// Check for repeat offenders
	int32 ViolationCount = GetViolationCount(PlayerID, Violation.Type);

	if (Violation.Severity == EMGViolationSeverity::Critical || ViolationCount >= 5)
	{
		// Automatic ban
		FDateTime BanExpiry;

		if (ViolationCount >= 10)
		{
			BanExpiry = FDateTime::MaxValue(); // Permanent
		}
		else if (ViolationCount >= 5)
		{
			BanExpiry = FDateTime::Now() + FTimespan::FromDays(30);
		}
		else
		{
			BanExpiry = FDateTime::Now() + FTimespan::FromDays(7);
		}

		BanList.Add(PlayerID, BanExpiry);
		BanReasons.Add(PlayerID, Violation.Description);

		if (PlayerID == LocalPlayerID)
		{
			bIsLocalBanned = true;
			OnPlayerBanned.Broadcast(Violation.Description);
		}
	}
}

void UMGAntiCheatSubsystem::ReportToServer(const FMGViolationRecord& Violation)
{
	// Would send violation to server for review and action
}

void UMGAntiCheatSubsystem::PeriodicIntegrityCheck()
{
	RunIntegrityCheck();
}

FString UMGAntiCheatSubsystem::CalculateFileChecksum(const FString& FilePath) const
{
	TArray<uint8> FileData;
	if (FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return FMD5::HashBytes(FileData.GetData(), FileData.Num());
	}
	return FString();
}
