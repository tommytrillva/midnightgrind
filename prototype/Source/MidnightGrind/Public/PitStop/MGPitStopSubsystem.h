// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPitStopSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGPitStopService : uint8
{
	None,
	Refuel,
	TireChange,
	RepairDamage,
	AdjustSetup,
	DriverChange,
	PenaltyServe,
	QuickService,
	FullService
};

UENUM(BlueprintType)
enum class EMGPitStopState : uint8
{
	Available,
	Approaching,
	InPitLane,
	Stopping,
	Servicing,
	Departing,
	Cooldown,
	Closed
};

UENUM(BlueprintType)
enum class EMGPitCrewRole : uint8
{
	JackOperator,
	FuelMan,
	TireChangerFL,
	TireChangerFR,
	TireChangerRL,
	TireChangerRR,
	BodyRepair,
	SetupEngineer,
	LollipopMan,
	FireExtinguisher
};

UENUM(BlueprintType)
enum class EMGTireCompound : uint8
{
	UltraSoft,
	Soft,
	Medium,
	Hard,
	Intermediate,
	FullWet,
	AllSeason,
	Drift
};

UENUM(BlueprintType)
enum class EMGPitLaneViolation : uint8
{
	None,
	Speeding,
	UnsafeRelease,
	CrossingLine,
	EquipmentContact,
	WrongBox,
	IgnoringRedLight
};

USTRUCT(BlueprintType)
struct FMGPitStopRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> RequestedServices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTireCompound NewTireCompound = EMGTireCompound::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChangeFrontTires = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bChangeRearTires = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairFrontWing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairRearWing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRepairBodywork = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FrontWingAdjustment = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RearWingAdjustment = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriorityLevel = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGPitStopResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StationaryTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PitLaneTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> CompletedServices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelAdded = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TiresChanged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageRepaired = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHadError = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ErrorDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPitLaneViolation Violation = EMGPitLaneViolation::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimePenalty = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

USTRUCT(BlueprintType)
struct FMGPitCrewMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPitCrewRole Role = EMGPitCrewRole::JackOperator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrewMemberID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Fatigue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ErrorChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseServiceTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = true;
};

USTRUCT(BlueprintType)
struct FMGPitBoxConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoxNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator BoxRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StopPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPitCrewMember> Crew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGTireCompound, int32> TireInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelCapacity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFuel = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EquipmentQuality = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AssignedVehicle;
};

USTRUCT(BlueprintType)
struct FMGPitLaneConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedLimit = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaneLength = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EntryPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ExitPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPitBoxConfig> PitBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPitLaneOpen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSpeedLimitEnforcement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedingPenaltyTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasPitLimiter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTrafficLight = true;
};

USTRUCT(BlueprintType)
struct FMGActivePitStop
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AssignedBox = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPitStopState CurrentState = EMGPitStopState::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPitStopRequest Request;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StateStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedTimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> CompletedServices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPitStopService> PendingServices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentServiceProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJackRaised = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGreenLightGiven = false;
};

USTRUCT(BlueprintType)
struct FMGPitStrategy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StrategyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlannedStops = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> PlannedStopLaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGTireCompound> PlannedCompounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> PlannedFuelLoads;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptimizeForPosition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReactToWeather = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUndercut = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOvercut = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinLapsOnTire = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TireWearThreshold = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelReserveTarget = 2.0f;
};

USTRUCT(BlueprintType)
struct FMGPitStopStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPitStops = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestPitStop = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AveragePitStop = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PitStopErrors = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpeedingViolations = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeLostToPenalties = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPitStopResult> PitStopHistory;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStopRequested, FName, VehicleID, const FMGPitStopRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStopStateChanged, FName, VehicleID, EMGPitStopState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStopCompleted, FName, VehicleID, const FMGPitStopResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPitServiceStarted, FName, VehicleID, EMGPitStopService, Service, float, EstimatedTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitServiceCompleted, FName, VehicleID, EMGPitStopService, Service);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitLaneViolation, FName, VehicleID, EMGPitLaneViolation, Violation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPitStrategyUpdated, FName, VehicleID, const FMGPitStrategy&, Strategy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPitLaneStatusChanged, bool, bIsOpen);

UCLASS()
class MIDNIGHTGRIND_API UMGPitStopSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Pit Stop Requests
	UFUNCTION(BlueprintCallable, Category = "PitStop|Request")
	bool RequestPitStop(FName VehicleID, const FMGPitStopRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "PitStop|Request")
	void CancelPitStopRequest(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "PitStop|Request")
	void ModifyPitStopRequest(FName VehicleID, const FMGPitStopRequest& NewRequest);

	UFUNCTION(BlueprintPure, Category = "PitStop|Request")
	bool HasPendingPitStop(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Request")
	FMGPitStopRequest GetPendingRequest(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Request")
	float EstimatePitStopTime(const FMGPitStopRequest& Request) const;

	// Pit Stop State
	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void EnterPitLane(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void ArriveAtPitBox(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void BeginServicing(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void ReleaseFromPitBox(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "PitStop|State")
	void ExitPitLane(FName VehicleID);

	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	EMGPitStopState GetPitStopState(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	FMGActivePitStop GetActivePitStop(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	bool IsVehicleInPitLane(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|State")
	bool IsVehicleBeingServiced(FName VehicleID) const;

	// Pit Lane Management
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void SetPitLaneConfig(const FMGPitLaneConfig& Config);

	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	FMGPitLaneConfig GetPitLaneConfig() const { return PitLaneConfig; }

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void OpenPitLane();

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void ClosePitLane();

	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	bool IsPitLaneOpen() const { return PitLaneConfig.bPitLaneOpen; }

	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	float GetPitLaneSpeedLimit() const { return PitLaneConfig.SpeedLimit; }

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void ReportVehicleSpeed(FName VehicleID, float CurrentSpeed);

	UFUNCTION(BlueprintPure, Category = "PitStop|PitLane")
	int32 GetAvailablePitBox() const;

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitLane")
	void AssignPitBox(FName VehicleID, int32 BoxIndex);

	// Pit Box Management
	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void ConfigurePitBox(int32 BoxIndex, const FMGPitBoxConfig& Config);

	UFUNCTION(BlueprintPure, Category = "PitStop|PitBox")
	FMGPitBoxConfig GetPitBoxConfig(int32 BoxIndex) const;

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void SetCrewMemberSkill(int32 BoxIndex, EMGPitCrewRole Role, float SkillLevel);

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void RefillTireInventory(int32 BoxIndex, EMGTireCompound Compound, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "PitStop|PitBox")
	void RefuelPitBox(int32 BoxIndex, float Amount);

	UFUNCTION(BlueprintPure, Category = "PitStop|PitBox")
	int32 GetTireInventory(int32 BoxIndex, EMGTireCompound Compound) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|PitBox")
	float GetPitBoxFuel(int32 BoxIndex) const;

	// Strategy
	UFUNCTION(BlueprintCallable, Category = "PitStop|Strategy")
	void SetPitStrategy(FName VehicleID, const FMGPitStrategy& Strategy);

	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	FMGPitStrategy GetPitStrategy(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	FMGPitStrategy CalculateOptimalStrategy(FName VehicleID, int32 RemainingLaps, float CurrentFuel, float TireWear) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	int32 GetRecommendedPitLap(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Strategy")
	EMGTireCompound GetRecommendedCompound(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "PitStop|Strategy")
	void UpdateStrategyForWeather(FName VehicleID, bool bRaining);

	UFUNCTION(BlueprintCallable, Category = "PitStop|Strategy")
	void UpdateStrategyForPosition(FName VehicleID, int32 CurrentPosition, int32 GapToAhead, int32 GapToBehind);

	// Quick Actions
	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestQuickFuel(FName VehicleID, float Amount);

	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestQuickTires(FName VehicleID, EMGTireCompound Compound);

	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestFullService(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "PitStop|Quick")
	void RequestMinimalService(FName VehicleID);

	// Stats
	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	FMGPitStopStats GetPitStopStats(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	TArray<FMGPitStopResult> GetPitStopHistory(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	float GetFastestPitStop() const;

	UFUNCTION(BlueprintPure, Category = "PitStop|Stats")
	FMGPitStopResult GetLastPitStop(FName VehicleID) const;

	UFUNCTION(BlueprintCallable, Category = "PitStop|Stats")
	void ResetRaceStats();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStopRequested OnPitStopRequested;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStopStateChanged OnPitStopStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStopCompleted OnPitStopCompleted;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitServiceStarted OnPitServiceStarted;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitServiceCompleted OnPitServiceCompleted;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitLaneViolation OnPitLaneViolation;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitStrategyUpdated OnPitStrategyUpdated;

	UPROPERTY(BlueprintAssignable, Category = "PitStop|Events")
	FOnPitLaneStatusChanged OnPitLaneStatusChanged;

protected:
	void OnPitStopTick();
	void UpdateActivePitStops(float DeltaTime);
	void ProcessService(FMGActivePitStop& PitStop, float DeltaTime);
	float CalculateServiceTime(EMGPitStopService Service, int32 BoxIndex) const;
	bool CheckForCrewError(int32 BoxIndex, EMGPitCrewRole Role) const;
	void ApplyPenalty(FName VehicleID, EMGPitLaneViolation Violation);
	FMGPitStopResult CompletePitStop(const FMGActivePitStop& PitStop);
	void SavePitStopData();
	void LoadPitStopData();

	UPROPERTY()
	FMGPitLaneConfig PitLaneConfig;

	UPROPERTY()
	TMap<FName, FMGPitStopRequest> PendingRequests;

	UPROPERTY()
	TMap<FName, FMGActivePitStop> ActivePitStops;

	UPROPERTY()
	TMap<FName, FMGPitStrategy> VehicleStrategies;

	UPROPERTY()
	TMap<FName, FMGPitStopStats> VehicleStats;

	UPROPERTY()
	TArray<FMGPitStopResult> RacePitStopHistory;

	UPROPERTY()
	float FastestPitStopTime = 0.0f;

	UPROPERTY()
	FName FastestPitStopVehicle;

	FTimerHandle PitStopTickHandle;
};
