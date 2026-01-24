// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAerodynamicsSubsystem.generated.h"

/**
 * Aerodynamic profile type
 */
UENUM(BlueprintType)
enum class EMGAeroProfile : uint8
{
	Standard			UMETA(DisplayName = "Standard"),
	LowDrag				UMETA(DisplayName = "Low Drag"),
	HighDownforce		UMETA(DisplayName = "High Downforce"),
	Balanced			UMETA(DisplayName = "Balanced"),
	SpeedFocused		UMETA(DisplayName = "Speed Focused"),
	GripFocused			UMETA(DisplayName = "Grip Focused"),
	DriftOptimized		UMETA(DisplayName = "Drift Optimized"),
	TopSpeed			UMETA(DisplayName = "Top Speed")
};

/**
 * Slipstream state
 */
UENUM(BlueprintType)
enum class EMGSlipstreamState : uint8
{
	None				UMETA(DisplayName = "None"),
	Entering			UMETA(DisplayName = "Entering"),
	Active				UMETA(DisplayName = "Active"),
	Optimal				UMETA(DisplayName = "Optimal"),
	Exiting				UMETA(DisplayName = "Exiting"),
	Slingshot			UMETA(DisplayName = "Slingshot")
};

/**
 * Wind effect type
 */
UENUM(BlueprintType)
enum class EMGWindEffect : uint8
{
	None				UMETA(DisplayName = "None"),
	Headwind			UMETA(DisplayName = "Headwind"),
	Tailwind			UMETA(DisplayName = "Tailwind"),
	Crosswind			UMETA(DisplayName = "Crosswind"),
	Gust				UMETA(DisplayName = "Gust"),
	Turbulence			UMETA(DisplayName = "Turbulence")
};

/**
 * Body kit type for aero
 */
UENUM(BlueprintType)
enum class EMGBodyKitType : uint8
{
	Stock				UMETA(DisplayName = "Stock"),
	StreetRacer			UMETA(DisplayName = "Street Racer"),
	TrackDay			UMETA(DisplayName = "Track Day"),
	TimeAttack			UMETA(DisplayName = "Time Attack"),
	DriftSpec			UMETA(DisplayName = "Drift Spec"),
	WidebodyKit			UMETA(DisplayName = "Widebody Kit"),
	Canards				UMETA(DisplayName = "Canards"),
	GTWing				UMETA(DisplayName = "GT Wing"),
	Custom				UMETA(DisplayName = "Custom")
};

/**
 * Vehicle aerodynamic state
 */
USTRUCT(BlueprintType)
struct FMGVehicleAeroState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LiftForce = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceTotal = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceFront = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceRear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirDensity = 1.225f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectiveDragCoefficient = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectiveDownforceCoefficient = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSlipstreamState SlipstreamState = EMGSlipstreamState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipstreamBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipstreamCharge = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGWindEffect CurrentWindEffect = EMGWindEffect::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindForce = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GripMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedMultiplier = 1.0f;
};

/**
 * Aerodynamic profile definition
 */
USTRUCT(BlueprintType)
struct FMGAeroProfileDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProfileId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAeroProfile Type = EMGAeroProfile::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragCoefficient = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LiftCoefficient = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceCoefficient = 0.50f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrontalArea = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceFrontBias = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedEffect = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CorneringGripEffect = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingStabilityEffect = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlipstreamEffectiveness = 1.0f;
};

/**
 * Slipstream session
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamSession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FollowerVehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LeaderVehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSlipstreamState State = EMGSlipstreamState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragReduction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotReady = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOptimal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;
};

/**
 * Wind zone definition
 */
USTRUCT(BlueprintType)
struct FMGWindZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ZoneId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WindDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GustFrequency = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GustIntensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurbulenceLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAffectsSlipstream = true;
};

/**
 * Spoiler configuration
 */
USTRUCT(BlueprintType)
struct FMGSpoilerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpoilerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AngleDegrees = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragPenalty = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoAdjusting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedForEffect = 60.0f;
};

/**
 * Slipstream config
 */
USTRUCT(BlueprintType)
struct FMGSlipstreamConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OptimalDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeAngleDegrees = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDragReduction = 0.40f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeedBonus = 0.10f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeRate = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DischargeRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotBoost = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SlingshotDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeedMPH = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlipstreamPoints = 50;
};

/**
 * Player aerodynamics stats
 */
USTRUCT(BlueprintType)
struct FMGAeroPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalSlipstreamTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestSlipstreamSession = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlingshotsUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OvertakesFromSlipstream = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlipstreamPointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedInSlipstream = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceDraftedMiles = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectSlipstreams = 0;
};

/**
 * Global aerodynamics config
 */
USTRUCT(BlueprintType)
struct FMGGlobalAeroConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirDensityBase = 1.225f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AltitudeEffect = 0.0001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureEffect = 0.004f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HumidityEffect = 0.001f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DownforceGripMultiplierMax = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DragTopSpeedPenaltyMax = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSimulateDetailedAero = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableWindEffects = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSlipstream = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlipstreamEntered, const FString&, FollowerId, const FString&, LeaderId, float, Distance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlipstreamExited, const FString&, FollowerId, float, TotalDuration, int32, PointsEarned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlipstreamStateChanged, const FString&, VehicleId, EMGSlipstreamState, OldState, EMGSlipstreamState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSlingshotReady, const FString&, VehicleId, float, BoostAmount, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlingshotUsed, const FString&, VehicleId, float, SpeedGained);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDownforceChanged, const FString&, VehicleId, float, OldDownforce, float, NewDownforce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWindEffectApplied, const FString&, VehicleId, EMGWindEffect, Effect, FVector, Force);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAeroProfileChanged, const FString&, VehicleId, const FString&, OldProfileId, const FString&, NewProfileId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnOptimalSlipstream, const FString&, VehicleId, float, BonusMultiplier);

/**
 * Aerodynamics Subsystem
 * Manages drag, lift, downforce, slipstream, and wind effects
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAerodynamicsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlipstreamEntered OnSlipstreamEntered;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlipstreamExited OnSlipstreamExited;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlipstreamStateChanged OnSlipstreamStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlingshotReady OnSlingshotReady;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnSlingshotUsed OnSlingshotUsed;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnDownforceChanged OnDownforceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnWindEffectApplied OnWindEffectApplied;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnAeroProfileChanged OnAeroProfileChanged;

	UPROPERTY(BlueprintAssignable, Category = "Aerodynamics|Events")
	FOnOptimalSlipstream OnOptimalSlipstream;

	// Vehicle Registration
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Vehicle")
	void RegisterVehicle(const FString& VehicleId, const FString& ProfileId);

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Vehicle")
	void UnregisterVehicle(const FString& VehicleId);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Vehicle")
	FMGVehicleAeroState GetVehicleAeroState(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Vehicle")
	void SetVehicleProfile(const FString& VehicleId, const FString& ProfileId);

	// Aerodynamic Profiles
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Profile")
	void RegisterAeroProfile(const FMGAeroProfileDefinition& Profile);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Profile")
	FMGAeroProfileDefinition GetAeroProfile(const FString& ProfileId) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Profile")
	TArray<FMGAeroProfileDefinition> GetAllProfiles() const;

	// Force Calculations
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	float CalculateDragForce(const FString& VehicleId, float SpeedMS) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	float CalculateLiftForce(const FString& VehicleId, float SpeedMS) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	float CalculateDownforce(const FString& VehicleId, float SpeedMS) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Forces")
	FVector CalculateTotalAeroForce(const FString& VehicleId, FVector Velocity) const;

	// Update Vehicle State
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Update")
	void UpdateVehicleAero(const FString& VehicleId, FVector Position, FVector Velocity, float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Update")
	float GetEffectiveTopSpeed(const FString& VehicleId, float BaseTopSpeed) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Update")
	float GetEffectiveGrip(const FString& VehicleId, float BaseGrip) const;

	// Slipstream
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Slipstream")
	void CheckSlipstream(const FString& FollowerId, const FString& LeaderId, FVector FollowerPos, FVector LeaderPos, FVector FollowerVelocity, FVector LeaderVelocity);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	bool IsInSlipstream(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	FMGSlipstreamSession GetSlipstreamSession(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	float GetSlipstreamCharge(const FString& VehicleId) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Slipstream")
	bool IsSlingshotReady(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Slipstream")
	float ActivateSlingshot(const FString& VehicleId);

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Slipstream")
	void ExitSlipstream(const FString& VehicleId);

	// Wind Effects
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Wind")
	void RegisterWindZone(const FMGWindZone& Zone);

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Wind")
	void UnregisterWindZone(const FString& ZoneId);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	FMGWindZone GetWindZone(const FString& ZoneId) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	TArray<FMGWindZone> GetAllWindZones() const;

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Wind")
	void SetGlobalWind(FVector Direction, float Speed);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	FVector GetWindAtLocation(FVector Location) const;

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Wind")
	FVector CalculateWindForce(const FString& VehicleId, FVector VehicleVelocity) const;

	// Spoilers
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Spoiler")
	void SetVehicleSpoiler(const FString& VehicleId, const FMGSpoilerConfig& Spoiler);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Spoiler")
	FMGSpoilerConfig GetVehicleSpoiler(const FString& VehicleId) const;

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Spoiler")
	void SetSpoilerAngle(const FString& VehicleId, float AngleDegrees);

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Spoiler")
	void SetSpoilerActive(const FString& VehicleId, bool bActive);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Stats")
	FMGAeroPlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Config")
	void SetSlipstreamConfig(const FMGSlipstreamConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Config")
	FMGSlipstreamConfig GetSlipstreamConfig() const;

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Config")
	void SetGlobalAeroConfig(const FMGGlobalAeroConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Aerodynamics|Config")
	FMGGlobalAeroConfig GetGlobalAeroConfig() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Update")
	void UpdateAerodynamics(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Persistence")
	void SaveAeroData();

	UFUNCTION(BlueprintCallable, Category = "Aerodynamics|Persistence")
	void LoadAeroData();

protected:
	void TickAerodynamics(float DeltaTime);
	void UpdateSlipstreams(float DeltaTime);
	void UpdateWindEffects(float DeltaTime);
	void CalculateAeroForces(const FString& VehicleId, float DeltaTime);
	float GetAirDensityAtLocation(FVector Location) const;
	bool IsInSlipstreamCone(FVector FollowerPos, FVector LeaderPos, FVector LeaderForward, float ConeAngle, float MaxDist) const;
	void UpdateSlipstreamState(FMGSlipstreamSession& Session, float Distance, float DeltaTime);
	void AwardSlipstreamPoints(const FString& VehicleId, float Duration);
	void UpdatePlayerStats(const FString& PlayerId, const FMGSlipstreamSession& Session);
	FString GenerateSessionId() const;

private:
	UPROPERTY()
	TMap<FString, FMGVehicleAeroState> VehicleStates;

	UPROPERTY()
	TMap<FString, FString> VehicleProfiles;

	UPROPERTY()
	TMap<FString, FMGAeroProfileDefinition> AeroProfiles;

	UPROPERTY()
	TMap<FString, FMGSlipstreamSession> ActiveSlipstreams;

	UPROPERTY()
	TMap<FString, FMGWindZone> WindZones;

	UPROPERTY()
	TMap<FString, FMGSpoilerConfig> VehicleSpoilers;

	UPROPERTY()
	TMap<FString, FMGAeroPlayerStats> PlayerStats;

	UPROPERTY()
	FMGSlipstreamConfig SlipstreamConfig;

	UPROPERTY()
	FMGGlobalAeroConfig GlobalConfig;

	UPROPERTY()
	FVector GlobalWindDirection = FVector::ZeroVector;

	UPROPERTY()
	float GlobalWindSpeed = 0.0f;

	UPROPERTY()
	int32 SessionCounter = 0;

	FTimerHandle AeroTickTimer;
};
