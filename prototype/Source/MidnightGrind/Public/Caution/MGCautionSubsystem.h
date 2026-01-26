// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCautionSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCautionType : uint8
{
	None,
	LocalYellow,
	FullCourseYellow,
	SafetyCar,
	VirtualSafetyCar,
	RedFlag,
	Code60
};

UENUM(BlueprintType)
enum class EMGCautionReason : uint8
{
	None,
	Accident,
	Debris,
	VehicleStopped,
	MedicalEmergency,
	WeatherConditions,
	TrackInvasion,
	OilOnTrack,
	UnsafeConditions,
	RaceControl,
	Steward
};

UENUM(BlueprintType)
enum class EMGCautionState : uint8
{
	Clear,
	CautionDeployed,
	CatchingUp,
	Bunched,
	RestartPending,
	GreenFlagPending
};

UENUM(BlueprintType)
enum class EMGFlagType : uint8
{
	None,
	Green,
	Yellow,
	DoubleYellow,
	Blue,
	White,
	Red,
	Black,
	BlackOrange,
	BlackWhite,
	Checkered,
	SafetyCarBoard
};

UENUM(BlueprintType)
enum class EMGSafetyCarPhase : uint8
{
	NotDeployed,
	Deploying,
	PickingUpLeader,
	Leading,
	InLap,
	PitEntry
};

USTRUCT(BlueprintType)
struct FMGCautionZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ZoneIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGFlagType ActiveFlag = EMGFlagType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionReason Reason = EMGCautionReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector IncidentLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNoOvertaking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ActivatedTime;
};

USTRUCT(BlueprintType)
struct FMGCautionPeriod
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CautionNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionType Type = EMGCautionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionReason Reason = EMGCautionReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCautionState State = EMGCautionState::Clear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapsUnderCaution = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> InvolvedVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector IncidentLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPitLaneOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLappedCarsCanUnlap = true;
};

USTRUCT(BlueprintType)
struct FMGSafetyCarState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDeployed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSafetyCarPhase Phase = EMGSafetyCarPhase::NotDeployed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetSpeed = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LeaderVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToLeader = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapsLed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLightsOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReadyToWithdraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongTrack = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGVirtualSafetyCarState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetDelta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> VehicleDeltas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, bool> VehicleCompliance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDelta = -0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDelta = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEndingSoon = false;
};

USTRUCT(BlueprintType)
struct FMGRestartProcedure
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDoubleFileRestart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRollingStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartZoneStart = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartZoneEnd = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RestartLeader;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeaderControlsRestart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRestartSpeed = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRestartSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WarningLapsRemaining = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGreenFlagReady = false;
};

USTRUCT(BlueprintType)
struct FMGCautionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableCautions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDeploySafetyCar = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoDeployVSC = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SafetyCarSpeed = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VSCSpeedLimit = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLapsUnderSC = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLapsUnderSC = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowLappedCarsToUnlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClosePitOnRedFlag = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DebrisCleanupTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccidentResponseTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseDoubleFileRestarts = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestartWarningDistance = 500.0f;
};

USTRUCT(BlueprintType)
struct FMGCautionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCautions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SafetyCarPeriods = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VSCPeriods = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RedFlags = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLapsUnderCaution = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeUnderCaution = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCautionPeriod> CautionHistory;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCautionDeployed, EMGCautionType, Type, EMGCautionReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCautionEnded, EMGCautionType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCautionStateChanged, EMGCautionState, OldState, EMGCautionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSafetyCarDeployed, const FMGSafetyCarState&, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSafetyCarIn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFlagDisplayed, FName, VehicleID, EMGFlagType, Flag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGreenFlag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedFlag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRestartWarning, int32, LapsRemaining, bool, bFinalWarning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVSCDeltaViolation, FName, VehicleID, float, Delta);

UCLASS()
class MIDNIGHTGRIND_API UMGCautionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Caution Control
	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void DeployCaution(EMGCautionType Type, EMGCautionReason Reason, const FVector& IncidentLocation);

	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void EndCaution();

	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void EscalateCaution(EMGCautionType NewType);

	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void DeployLocalYellow(int32 ZoneIndex, EMGCautionReason Reason, const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void ClearLocalYellow(int32 ZoneIndex);

	UFUNCTION(BlueprintCallable, Category = "Caution|Control")
	void ClearAllLocalYellows();

	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	bool IsCautionActive() const;

	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	EMGCautionType GetActiveCautionType() const;

	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	FMGCautionPeriod GetCurrentCaution() const { return CurrentCaution; }

	UFUNCTION(BlueprintPure, Category = "Caution|Control")
	EMGCautionState GetCautionState() const { return CurrentState; }

	// Safety Car
	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void DeploySafetyCar(EMGCautionReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void BringSafetyCarIn();

	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void UpdateSafetyCarPosition(const FVector& Position, float Speed);

	UFUNCTION(BlueprintPure, Category = "Caution|SafetyCar")
	FMGSafetyCarState GetSafetyCarState() const { return SafetyCarState; }

	UFUNCTION(BlueprintPure, Category = "Caution|SafetyCar")
	bool IsSafetyCarDeployed() const { return SafetyCarState.bDeployed; }

	UFUNCTION(BlueprintPure, Category = "Caution|SafetyCar")
	float GetSafetyCarSpeed() const { return SafetyCarState.CurrentSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Caution|SafetyCar")
	void SetSafetyCarPhase(EMGSafetyCarPhase Phase);

	// Virtual Safety Car
	UFUNCTION(BlueprintCallable, Category = "Caution|VSC")
	void DeployVirtualSafetyCar();

	UFUNCTION(BlueprintCallable, Category = "Caution|VSC")
	void EndVirtualSafetyCar();

	UFUNCTION(BlueprintCallable, Category = "Caution|VSC")
	void UpdateVSCDelta(FName VehicleID, float Delta);

	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	FMGVirtualSafetyCarState GetVSCState() const { return VSCState; }

	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	bool IsVSCActive() const { return VSCState.bActive; }

	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	float GetVSCDelta(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Caution|VSC")
	bool IsVehicleVSCCompliant(FName VehicleID) const;

	// Red Flag
	UFUNCTION(BlueprintCallable, Category = "Caution|RedFlag")
	void DeployRedFlag(EMGCautionReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Caution|RedFlag")
	void RestartFromRedFlag();

	UFUNCTION(BlueprintPure, Category = "Caution|RedFlag")
	bool IsRedFlagActive() const;

	// Flags
	UFUNCTION(BlueprintCallable, Category = "Caution|Flags")
	void ShowFlag(FName VehicleID, EMGFlagType Flag);

	UFUNCTION(BlueprintCallable, Category = "Caution|Flags")
	void ClearFlag(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Caution|Flags")
	EMGFlagType GetVehicleFlag(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "Caution|Flags")
	TArray<FName> GetVehiclesWithFlag(EMGFlagType Flag) const;

	// Zones
	UFUNCTION(BlueprintCallable, Category = "Caution|Zones")
	void ConfigureZones(int32 NumZones, float TrackLength);

	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	TArray<FMGCautionZone> GetCautionZones() const { return CautionZones; }

	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	FMGCautionZone GetZoneAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	bool IsZoneUnderCaution(int32 ZoneIndex) const;

	UFUNCTION(BlueprintPure, Category = "Caution|Zones")
	float GetZoneSpeedLimit(int32 ZoneIndex) const;

	// Restart
	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void PrepareRestart();

	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void InitiateRestart();

	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void SetRestartLeader(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "Caution|Restart")
	FMGRestartProcedure GetRestartProcedure() const { return RestartProcedure; }

	UFUNCTION(BlueprintPure, Category = "Caution|Restart")
	bool IsRestartPending() const;

	UFUNCTION(BlueprintCallable, Category = "Caution|Restart")
	void AbortRestart();

	// Pit Lane Control
	UFUNCTION(BlueprintCallable, Category = "Caution|Pit")
	void SetPitLaneOpen(bool bOpen);

	UFUNCTION(BlueprintPure, Category = "Caution|Pit")
	bool IsPitLaneOpen() const { return bPitLaneOpen; }

	UFUNCTION(BlueprintCallable, Category = "Caution|Pit")
	void AllowLappedCarsToUnlap();

	UFUNCTION(BlueprintPure, Category = "Caution|Pit")
	TArray<FName> GetLappedCars() const { return LappedCars; }

	// Stats
	UFUNCTION(BlueprintPure, Category = "Caution|Stats")
	FMGCautionStats GetCautionStats() const { return CautionStats; }

	UFUNCTION(BlueprintPure, Category = "Caution|Stats")
	TArray<FMGCautionPeriod> GetCautionHistory() const { return CautionStats.CautionHistory; }

	UFUNCTION(BlueprintCallable, Category = "Caution|Stats")
	void ResetStats();

	// Settings
	UFUNCTION(BlueprintCallable, Category = "Caution|Settings")
	void SetCautionSettings(const FMGCautionSettings& NewSettings);

	UFUNCTION(BlueprintPure, Category = "Caution|Settings")
	FMGCautionSettings GetCautionSettings() const { return Settings; }

	// Events
	UFUNCTION(BlueprintCallable, Category = "Caution|Events")
	void ReportIncident(FName VehicleID, const FVector& Location, EMGCautionReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Caution|Events")
	void ReportDebris(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Caution|Events")
	void ReportStoppedVehicle(FName VehicleID, const FVector& Location);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnCautionDeployed OnCautionDeployed;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnCautionEnded OnCautionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnCautionStateChanged OnCautionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnSafetyCarDeployed OnSafetyCarDeployed;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnSafetyCarIn OnSafetyCarIn;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnFlagDisplayed OnFlagDisplayed;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnGreenFlag OnGreenFlag;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnRedFlag OnRedFlag;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnRestartWarning OnRestartWarning;

	UPROPERTY(BlueprintAssignable, Category = "Caution|Events")
	FOnVSCDeltaViolation OnVSCDeltaViolation;

protected:
	void OnCautionTick();
	void UpdateSafetyCar(float DeltaTime);
	void UpdateVSC(float DeltaTime);
	void UpdateRestartProcedure(float DeltaTime);
	void SetCautionState(EMGCautionState NewState);
	int32 GetZoneIndex(float Distance) const;
	void RecordCautionPeriod();

	UPROPERTY()
	FMGCautionPeriod CurrentCaution;

	UPROPERTY()
	EMGCautionState CurrentState = EMGCautionState::Clear;

	UPROPERTY()
	FMGSafetyCarState SafetyCarState;

	UPROPERTY()
	FMGVirtualSafetyCarState VSCState;

	UPROPERTY()
	FMGRestartProcedure RestartProcedure;

	UPROPERTY()
	TArray<FMGCautionZone> CautionZones;

	UPROPERTY()
	TMap<FName, EMGFlagType> VehicleFlags;

	UPROPERTY()
	FMGCautionStats CautionStats;

	UPROPERTY()
	FMGCautionSettings Settings;

	UPROPERTY()
	bool bPitLaneOpen = true;

	UPROPERTY()
	TArray<FName> LappedCars;

	UPROPERTY()
	float TrackLength = 5000.0f;

	UPROPERTY()
	int32 CautionCounter = 0;

	FTimerHandle CautionTickHandle;
};
