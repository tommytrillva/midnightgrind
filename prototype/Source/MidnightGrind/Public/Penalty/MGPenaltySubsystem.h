// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPenaltySubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGPenaltyType : uint8
{
	None,
	Warning,
	TimeAdded,
	DriveThrough,
	StopAndGo,
	PositionPenalty,
	GridPenalty,
	Disqualification,
	Exclusion,
	PointsDeduction,
	FinePenalty,
	LicensePoints
};

UENUM(BlueprintType)
enum class EMGViolationType : uint8
{
	None,
	TrackLimits,
	CuttingCorner,
	GainingAdvantage,
	IllegalOvertake,
	Collision,
	DangerousDriving,
	UnsafeRejoin,
	Blocking,
	IgnoringFlags,
	FalseStart,
	PitSpeeding,
	PitLaneViolation,
	UnservedPenalty,
	TechnicalInfringement,
	UnsportsmanlikeConduct,
	TeamOrders
};

UENUM(BlueprintType)
enum class EMGPenaltyState : uint8
{
	Pending,
	Announced,
	Active,
	Served,
	Cancelled,
	Appealed
};

UENUM(BlueprintType)
enum class EMGTrackLimitsSeverity : uint8
{
	Minor,
	Moderate,
	Severe,
	Deliberate
};

USTRUCT(BlueprintType)
struct FMGPenalty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PenaltyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPenaltyType Type = EMGPenaltyType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGViolationType Violation = EMGViolationType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPenaltyState State = EMGPenaltyState::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PositionValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FineValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapIssued = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapToServe = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapsToServe = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IssuedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ServedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector IncidentLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OtherVehicleInvolved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoApplied = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAppealable = true;
};

USTRUCT(BlueprintType)
struct FMGTrackLimitsViolation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CornerNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrackLimitsSeverity Severity = EMGTrackLimitsSeverity::Minor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeGained = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ViolationPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLapTimeDeleted = false;
};

USTRUCT(BlueprintType)
struct FMGCollisionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Vehicle1ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Vehicle2ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ImpactForce = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RelativeSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlapPercentage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CornerNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AtFaultDriver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRacingIncident = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGPenaltyRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceTrackLimits = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLimitsWarnings = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackLimitsTimeAdded = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDeleteLapTimes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceCornerCutting = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceCollisions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionSpeedThreshold = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceFalseStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FalseStartThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitSpeedLimit = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitSpeedPenalty = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnforceBlueFlags = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BlueFlagIgnoreLimit = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowAppeals = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PenaltyServeLaps = 3;
};

USTRUCT(BlueprintType)
struct FMGDriverIncidents
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLimitsViolations = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLimitsWarnings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollisionsAtFault = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollisionsVictim = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacingIncidents = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Warnings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Penalties = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimePenalties = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LicensePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPenalty> PenaltyHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTrackLimitsViolation> TrackLimitsHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCollisionData> CollisionHistory;
};

USTRUCT(BlueprintType)
struct FMGPenaltySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnablePenalties = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStrictRules = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowPenaltyNotifications = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowWarnings = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoServePenalties = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GhostingOnContact = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlowdownPenaltyMultiplier = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPenaltyIssued, FName, VehicleID, const FMGPenalty&, Penalty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPenaltyServed, FName, VehicleID, const FMGPenalty&, Penalty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPenaltyCancelled, FName, VehicleID, FGuid, PenaltyID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWarningIssued, FName, VehicleID, EMGViolationType, Violation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrackLimitsViolation, FName, VehicleID, const FMGTrackLimitsViolation&, Violation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollisionDetected, const FMGCollisionData&, Collision);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisqualification, FName, VehicleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapTimeDeleted, FName, VehicleID, int32, LapNumber);

UCLASS()
class MIDNIGHTGRIND_API UMGPenaltySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Penalty Management
	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	FMGPenalty IssuePenalty(FName VehicleID, EMGPenaltyType Type, EMGViolationType Violation, float TimeValue = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueWarning(FName VehicleID, EMGViolationType Violation);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueDriveThrough(FName VehicleID, EMGViolationType Violation);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueStopAndGo(FName VehicleID, EMGViolationType Violation, float Duration = 10.0f);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueTimePenalty(FName VehicleID, EMGViolationType Violation, float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssuePositionPenalty(FName VehicleID, EMGViolationType Violation, int32 Positions);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Issue")
	void IssueDisqualification(FName VehicleID, EMGViolationType Violation);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Serve")
	void ServePenalty(FName VehicleID, FGuid PenaltyID);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Serve")
	void CancelPenalty(FName VehicleID, FGuid PenaltyID);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Serve")
	void AppealPenalty(FName VehicleID, FGuid PenaltyID);

	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	TArray<FMGPenalty> GetPendingPenalties(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	TArray<FMGPenalty> GetAllPenalties(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	bool HasPendingPenalty(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Penalty|Query")
	FMGPenalty GetMostSeverePenalty(FName VehicleID) const;

	// Track Limits
	UFUNCTION(BlueprintCallable, Category = "Penalty|TrackLimits")
	void ReportTrackLimitsViolation(FName VehicleID, int32 CornerNumber, int32 LapNumber, const FVector& Position);

	UFUNCTION(BlueprintCallable, Category = "Penalty|TrackLimits")
	void SetTrackLimitsSeverity(FName VehicleID, EMGTrackLimitsSeverity Severity, float TimeGained);

	UFUNCTION(BlueprintPure, Category = "Penalty|TrackLimits")
	int32 GetTrackLimitsCount(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Penalty|TrackLimits")
	int32 GetTrackLimitsWarningsRemaining(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Penalty|TrackLimits")
	void DeleteLapTime(FName VehicleID, int32 LapNumber);

	// Collisions
	UFUNCTION(BlueprintCallable, Category = "Penalty|Collision")
	void ReportCollision(const FMGCollisionData& Collision);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Collision")
	void DetermineFault(FMGCollisionData& Collision);

	UFUNCTION(BlueprintPure, Category = "Penalty|Collision")
	TArray<FMGCollisionData> GetCollisionHistory() const;

	UFUNCTION(BlueprintPure, Category = "Penalty|Collision")
	int32 GetCollisionsAtFault(FName VehicleID) const;

	// Race Events
	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void CheckFalseStart(FName VehicleID, float ReactionTime);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void CheckPitSpeeding(FName VehicleID, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void CheckBlueFlagIgnore(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Events")
	void ClearBlueFlagCount(FName VehicleID);

	// Driver Incidents
	UFUNCTION(BlueprintPure, Category = "Penalty|Incidents")
	FMGDriverIncidents GetDriverIncidents(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "Penalty|Incidents")
	void ResetDriverIncidents(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Penalty|Incidents")
	void ResetAllIncidents();

	UFUNCTION(BlueprintPure, Category = "Penalty|Incidents")
	float GetTotalTimePenalties(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Penalty|Incidents")
	int32 GetTotalWarnings(FName VehicleID) const;

	// Rules
	UFUNCTION(BlueprintCallable, Category = "Penalty|Rules")
	void SetPenaltyRules(const FMGPenaltyRules& NewRules);

	UFUNCTION(BlueprintPure, Category = "Penalty|Rules")
	FMGPenaltyRules GetPenaltyRules() const { return Rules; }

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Penalty|Settings")
	void SetPenaltySettings(const FMGPenaltySettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Penalty|Settings")
	FMGPenaltySettings GetPenaltySettings() const { return Settings; }

	// Lap Events
	UFUNCTION(BlueprintCallable, Category = "Penalty|Lap")
	void OnLapCompleted(FName VehicleID, int32 LapNumber);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnPenaltyIssued OnPenaltyIssued;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnPenaltyServed OnPenaltyServed;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnPenaltyCancelled OnPenaltyCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnWarningIssued OnWarningIssued;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnTrackLimitsViolation OnTrackLimitsViolation;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnCollisionDetected OnCollisionDetected;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnDisqualification OnDisqualification;

	UPROPERTY(BlueprintAssignable, Category = "Penalty|Events")
	FOnLapTimeDeleted OnLapTimeDeleted;

protected:
	void OnPenaltyTick();
	void CheckUnservedPenalties();
	void ProcessTrackLimits(FName VehicleID);
	FMGDriverIncidents& GetOrCreateIncidents(FName VehicleID);

	UPROPERTY()
	TMap<FName, FMGDriverIncidents> DriverIncidents;

	UPROPERTY()
	TMap<FName, int32> BlueFlagCounts;

	UPROPERTY()
	TArray<FMGCollisionData> AllCollisions;

	UPROPERTY()
	FMGPenaltyRules Rules;

	UPROPERTY()
	FMGPenaltySettings Settings;

	UPROPERTY()
	int32 CurrentLap = 0;

	FTimerHandle PenaltyTickHandle;
};
