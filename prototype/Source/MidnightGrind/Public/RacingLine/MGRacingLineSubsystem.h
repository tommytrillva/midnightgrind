// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRacingLineSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGRacingLineType : uint8
{
	Optimal,
	Safe,
	Aggressive,
	DriftLine,
	FuelSaving,
	WetWeather,
	Custom
};

UENUM(BlueprintType)
enum class EMGLineSegmentType : uint8
{
	Straight,
	Corner,
	Hairpin,
	Chicane,
	Braking,
	Acceleration,
	DriftZone,
	Slipstream
};

UENUM(BlueprintType)
enum class EMGLineVisualMode : uint8
{
	Off,
	Simple,
	Detailed,
	ThreeD,
	AR,
	Predictive
};

UENUM(BlueprintType)
enum class EMGBrakingIndicator : uint8
{
	None,
	Light,
	Medium,
	Heavy,
	MaxBraking
};

UENUM(BlueprintType)
enum class EMGCornerPhase : uint8
{
	Approach,
	BrakingZone,
	TurnIn,
	Apex,
	Exit,
	Acceleration
};

USTRUCT(BlueprintType)
struct FMGRacingLinePoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceAlongTrack = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackWidth = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Curvature = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Grade = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Camber = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLineSegmentType SegmentType = EMGLineSegmentType::Straight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBrakingIndicator BrakingLevel = EMGBrakingIndicator::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottlePercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakePercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GearSuggestion = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bApexPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBrakingPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTurnInPoint = false;
};

USTRUCT(BlueprintType)
struct FMGCornerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CornerNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CornerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntryDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExitDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ApexDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ApexPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BrakingPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TurnInPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CornerAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalEntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalApexSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalExitSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLeftHander = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHairpin = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecommendedGear = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Difficulty = 1.0f;
};

USTRUCT(BlueprintType)
struct FMGRacingLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRacingLineType LineType = EMGRacingLineType::Optimal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRacingLinePoint> Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCornerData> Corners;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bValidated = false;
};

USTRUCT(BlueprintType)
struct FMGDriverAssistSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRacingLine = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLineVisualMode VisualMode = EMGLineVisualMode::Simple;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowBrakingPoints = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowApexMarkers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedAdvisor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGearSuggestion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCornerNames = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPredictiveLine = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineOpacity = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LineWidth = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor AccelerateColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BrakeColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CoastColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookAheadDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeDistance = 50.0f;
};

USTRUCT(BlueprintType)
struct FMGLineDeviation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LateralDeviation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedDeviation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrottleDeviation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakeDeviation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnLine = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTooFast = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTooSlow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeviationScore = 100.0f;
};

USTRUCT(BlueprintType)
struct FMGLinePerformance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageDeviation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestCornerScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorstCornerScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WorstCornerNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverallLineScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToOptimal = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> CornerScores;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorDeviations;
};

USTRUCT(BlueprintType)
struct FMGBrakingZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExitSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalBrakeForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AssociatedCorner = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDownhill = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDistance = 0.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRacingLineLoaded, FName, TrackID, EMGRacingLineType, LineType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLineDeviationUpdated, FName, VehicleID, const FMGLineDeviation&, Deviation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCornerApproaching, FName, VehicleID, const FMGCornerData&, Corner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBrakingZoneEntered, FName, VehicleID, const FMGBrakingZone&, Zone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeedWarning, FName, VehicleID, float, SpeedDifference);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssistSettingsChanged, const FMGDriverAssistSettings&, NewSettings);

UCLASS()
class MIDNIGHTGRIND_API UMGRacingLineSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Racing Line Loading
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Load")
	bool LoadRacingLine(FName TrackID, EMGRacingLineType LineType = EMGRacingLineType::Optimal);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Load")
	bool LoadCustomLine(FName TrackID, const FMGRacingLine& CustomLine);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Load")
	void UnloadRacingLine();

	UFUNCTION(BlueprintPure, Category = "RacingLine|Load")
	bool IsLineLoaded() const { return bLineLoaded; }

	UFUNCTION(BlueprintPure, Category = "RacingLine|Load")
	FMGRacingLine GetCurrentLine() const { return CurrentLine; }

	UFUNCTION(BlueprintPure, Category = "RacingLine|Load")
	TArray<EMGRacingLineType> GetAvailableLineTypes(FName TrackID) const;

	// Line Query
	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FMGRacingLinePoint GetPointAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FMGRacingLinePoint GetNearestPoint(const FVector& WorldPosition) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	float GetDistanceAlongLine(const FVector& WorldPosition) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FVector GetLinePositionAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	FVector GetLineDirectionAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	float GetOptimalSpeedAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Query")
	TArray<FMGRacingLinePoint> GetPointsInRange(float StartDistance, float EndDistance) const;

	// Corner Information
	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	TArray<FMGCornerData> GetAllCorners() const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	FMGCornerData GetCorner(int32 CornerNumber) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	FMGCornerData GetNextCorner(float CurrentDistance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	float GetDistanceToNextCorner(float CurrentDistance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	EMGCornerPhase GetCornerPhase(float CurrentDistance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Corner")
	bool IsInCorner(float CurrentDistance) const;

	// Braking Zones
	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	TArray<FMGBrakingZone> GetAllBrakingZones() const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	FMGBrakingZone GetNextBrakingZone(float CurrentDistance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	float GetDistanceToNextBrakingZone(float CurrentDistance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	bool IsInBrakingZone(float CurrentDistance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Braking")
	EMGBrakingIndicator GetBrakingIndicator(float CurrentDistance) const;

	// Vehicle Tracking
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Track")
	void RegisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Track")
	void UnregisterVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Track")
	void UpdateVehiclePosition(FName VehicleID, const FVector& Position, float CurrentSpeed, float Throttle, float Brake);

	UFUNCTION(BlueprintPure, Category = "RacingLine|Track")
	FMGLineDeviation GetVehicleDeviation(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Track")
	FMGLinePerformance GetVehiclePerformance(FName VehicleID) const;

	// Speed Advisory
	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetRecommendedSpeed(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetSpeedDifference(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	int32 GetRecommendedGear(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetRecommendedThrottle(FName VehicleID) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Speed")
	float GetRecommendedBrake(FName VehicleID) const;

	// Assist Settings
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetAssistSettings(const FMGDriverAssistSettings& Settings);

	UFUNCTION(BlueprintPure, Category = "RacingLine|Settings")
	FMGDriverAssistSettings GetAssistSettings() const { return AssistSettings; }

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetLineVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetVisualMode(EMGLineVisualMode Mode);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Settings")
	void SetLookAheadDistance(float Distance);

	// Line Generation
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	FMGRacingLine GenerateLineFromSpline(const TArray<FVector>& SplinePoints, FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	FMGRacingLine GenerateOptimalLine(FName TrackID, FName VehicleClass);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	void RecordPlayerLine(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	void StopRecordingLine();

	UFUNCTION(BlueprintPure, Category = "RacingLine|Generate")
	bool IsRecording() const { return bRecording; }

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	FMGRacingLine GetRecordedLine() const;

	UFUNCTION(BlueprintCallable, Category = "RacingLine|Generate")
	void SaveLine(const FMGRacingLine& Line, FName TrackID, FName LineName);

	// Visualization Data
	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	TArray<FVector> GetVisibleLinePoints(const FVector& ViewerPosition, float LookAhead) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	FLinearColor GetLineColorAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	TArray<FVector> GetBrakingMarkerPositions() const;

	UFUNCTION(BlueprintPure, Category = "RacingLine|Visual")
	TArray<FVector> GetApexMarkerPositions() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnRacingLineLoaded OnRacingLineLoaded;

	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnLineDeviationUpdated OnLineDeviationUpdated;

	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnCornerApproaching OnCornerApproaching;

	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnBrakingZoneEntered OnBrakingZoneEntered;

	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnSpeedWarning OnSpeedWarning;

	UPROPERTY(BlueprintAssignable, Category = "RacingLine|Events")
	FOnAssistSettingsChanged OnAssistSettingsChanged;

protected:
	void OnLineTick();
	void UpdateVehicleDeviations();
	void CheckCornerApproach(FName VehicleID, float Distance);
	void CheckBrakingZone(FName VehicleID, float Distance);
	void CalculateBrakingZones();
	void IdentifyCorners();
	FMGRacingLinePoint InterpolatePoint(float Distance) const;
	int32 FindNearestPointIndex(float Distance) const;
	void SaveLineData();
	void LoadLineData();

	UPROPERTY()
	FMGRacingLine CurrentLine;

	UPROPERTY()
	bool bLineLoaded = false;

	UPROPERTY()
	TMap<FName, TArray<FMGRacingLine>> TrackLines;

	UPROPERTY()
	TArray<FMGBrakingZone> BrakingZones;

	UPROPERTY()
	FMGDriverAssistSettings AssistSettings;

	UPROPERTY()
	TMap<FName, FVector> VehiclePositions;

	UPROPERTY()
	TMap<FName, float> VehicleSpeeds;

	UPROPERTY()
	TMap<FName, float> VehicleDistances;

	UPROPERTY()
	TMap<FName, FMGLineDeviation> VehicleDeviations;

	UPROPERTY()
	TMap<FName, FMGLinePerformance> VehiclePerformances;

	UPROPERTY()
	bool bRecording = false;

	UPROPERTY()
	FName RecordingVehicle;

	UPROPERTY()
	TArray<FMGRacingLinePoint> RecordedPoints;

	UPROPERTY()
	TMap<FName, int32> VehicleLastCornerWarning;

	UPROPERTY()
	TMap<FName, int32> VehicleLastBrakingWarning;

	FTimerHandle LineTickHandle;
};
