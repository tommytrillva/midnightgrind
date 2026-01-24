// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGTelemetrySubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTelemetryChannel : uint8
{
	Speed,
	RPM,
	Gear,
	Throttle,
	Brake,
	Steering,
	Nitro,
	LapTime,
	SectorTime,
	Position,
	DeltaTime,
	TireTemp,
	TireWear,
	FuelLevel,
	EngineTemp,
	Suspension,
	GForce,
	Altitude,
	TrackPosition
};

UENUM(BlueprintType)
enum class EMGTelemetryOverlayStyle : uint8
{
	Minimal,
	Standard,
	Detailed,
	Professional,
	Streamer,
	Custom
};

USTRUCT(BlueprintType)
struct FMGTelemetryFrame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMPH = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RPM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Gear = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakeInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SteeringInput = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNitroActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector GForce = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralG = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongitudinalG = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> TireTemperatures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> TireWear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SuspensionTravel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> WheelSlip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngineTemperature = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OilTemperature = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackPercentage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSector = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacePosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDrifting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftAngle = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGLapTelemetry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopGear = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearShifts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroUsed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDriftAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DriftCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLateralG = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLongitudinalG = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGTelemetryFrame> Frames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;
};

USTRUCT(BlueprintType)
struct FMGTelemetrySession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLapTelemetry> Laps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapTelemetry BestLap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLaps = 0;
};

USTRUCT(BlueprintType)
struct FMGTelemetryComparison
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapTelemetry ReferenceLap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapTelemetry CurrentLap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> DeltaAtDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAhead = false;
};

USTRUCT(BlueprintType)
struct FMGTelemetryOverlayConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTelemetryOverlayStyle Style = EMGTelemetryOverlayStyle::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRPM = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowInputs = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGForce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTireInfo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDelta = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowMinimap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowLapInfo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlayOpacity = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D OverlayPosition = FVector2D(0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverlayScale = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTelemetryFrameRecorded, const FMGTelemetryFrame&, Frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompleted, const FMGLapTelemetry&, LapData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectorCompleted, int32, Sector, float, SectorTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPersonalBest, const FMGLapTelemetry&, BestLap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeltaUpdated, float, DeltaTime);

UCLASS()
class MIDNIGHTGRIND_API UMGTelemetrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Recording Control
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void StopRecording();

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void PauseRecording();

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void ResumeRecording();

	UFUNCTION(BlueprintPure, Category = "Telemetry|Recording")
	bool IsRecording() const { return bIsRecording; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Recording")
	void SetRecordingRate(float FramesPerSecond);

	// Current Frame Data
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Data")
	void RecordFrame(const FMGTelemetryFrame& Frame);

	UFUNCTION(BlueprintPure, Category = "Telemetry|Data")
	FMGTelemetryFrame GetCurrentFrame() const { return CurrentFrame; }

	UFUNCTION(BlueprintPure, Category = "Telemetry|Data")
	FMGTelemetryFrame GetFrameAtTime(float Timestamp) const;

	// Lap Management
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Lap")
	void StartLap();

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Lap")
	void CompleteLap(float LapTime);

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Lap")
	void CompleteSector(int32 Sector, float SectorTime);

	UFUNCTION(BlueprintPure, Category = "Telemetry|Lap")
	FMGLapTelemetry GetCurrentLapTelemetry() const { return CurrentLap; }

	UFUNCTION(BlueprintPure, Category = "Telemetry|Lap")
	FMGLapTelemetry GetBestLapTelemetry() const { return BestLap; }

	UFUNCTION(BlueprintPure, Category = "Telemetry|Lap")
	TArray<FMGLapTelemetry> GetAllLapsTelemetry() const;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	void StartSession(FName TrackID, FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "Telemetry|Session")
	FMGTelemetrySession GetCurrentSession() const { return CurrentSession; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	void SaveSession(const FString& Filename);

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Session")
	bool LoadSession(const FString& Filename);

	// Comparison
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Comparison")
	void SetReferenceLap(const FMGLapTelemetry& Lap);

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Comparison")
	void SetReferenceLapFromBest();

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Comparison")
	void SetReferenceLapFromGhost(FName GhostID);

	UFUNCTION(BlueprintPure, Category = "Telemetry|Comparison")
	float GetCurrentDelta() const;

	UFUNCTION(BlueprintPure, Category = "Telemetry|Comparison")
	float GetDeltaAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "Telemetry|Comparison")
	FMGTelemetryComparison GetComparison() const { return Comparison; }

	// Overlay Configuration
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Overlay")
	void SetOverlayConfig(const FMGTelemetryOverlayConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Telemetry|Overlay")
	FMGTelemetryOverlayConfig GetOverlayConfig() const { return OverlayConfig; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Overlay")
	void SetOverlayVisible(bool bVisible);

	UFUNCTION(BlueprintPure, Category = "Telemetry|Overlay")
	bool IsOverlayVisible() const { return bOverlayVisible; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Overlay")
	void SetOverlayStyle(EMGTelemetryOverlayStyle Style);

	// Data Export
	UFUNCTION(BlueprintCallable, Category = "Telemetry|Export")
	void ExportToCSV(const FString& Filename);

	UFUNCTION(BlueprintCallable, Category = "Telemetry|Export")
	void ExportToJSON(const FString& Filename);

	UFUNCTION(BlueprintPure, Category = "Telemetry|Export")
	FString GetTelemetryAsString() const;

	// Analysis
	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetAverageSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetMaxSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetMaxGForce() const;

	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	float GetBrakingEfficiency() const;

	UFUNCTION(BlueprintPure, Category = "Telemetry|Analysis")
	TArray<FVector> GetDrivingLine() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnTelemetryFrameRecorded OnTelemetryFrameRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnLapCompleted OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnSectorCompleted OnSectorCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnPersonalBest OnPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Telemetry|Events")
	FOnDeltaUpdated OnDeltaUpdated;

protected:
	void OnTelemetryTick();
	void ProcessCurrentFrame();
	void UpdateComparison();
	void UpdateLapStatistics();
	FMGTelemetryFrame InterpolateFrames(const FMGTelemetryFrame& A, const FMGTelemetryFrame& B, float Alpha) const;

	UPROPERTY()
	FMGTelemetryFrame CurrentFrame;

	UPROPERTY()
	FMGLapTelemetry CurrentLap;

	UPROPERTY()
	FMGLapTelemetry BestLap;

	UPROPERTY()
	FMGLapTelemetry ReferenceLap;

	UPROPERTY()
	FMGTelemetrySession CurrentSession;

	UPROPERTY()
	FMGTelemetryComparison Comparison;

	UPROPERTY()
	FMGTelemetryOverlayConfig OverlayConfig;

	UPROPERTY()
	bool bIsRecording = false;

	UPROPERTY()
	bool bIsPaused = false;

	UPROPERTY()
	bool bOverlayVisible = true;

	UPROPERTY()
	float RecordingInterval = 0.05f;

	UPROPERTY()
	float TotalDistance = 0.0f;

	FTimerHandle TelemetryTickHandle;
};
