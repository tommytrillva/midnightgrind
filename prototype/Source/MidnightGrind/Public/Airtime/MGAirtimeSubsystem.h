// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAirtimeSubsystem.generated.h"

/**
 * Jump type
 */
UENUM(BlueprintType)
enum class EMGJumpType : uint8
{
	None				UMETA(DisplayName = "None"),
	Ramp				UMETA(DisplayName = "Ramp Jump"),
	Terrain				UMETA(DisplayName = "Terrain Jump"),
	Bump				UMETA(DisplayName = "Bump"),
	Kicker				UMETA(DisplayName = "Kicker"),
	MegaRamp			UMETA(DisplayName = "Mega Ramp"),
	HalfPipe			UMETA(DisplayName = "Half Pipe"),
	Billboard			UMETA(DisplayName = "Billboard"),
	Rooftop				UMETA(DisplayName = "Rooftop"),
	Shortcut			UMETA(DisplayName = "Shortcut Jump"),
	SecretJump			UMETA(DisplayName = "Secret Jump")
};

/**
 * Landing quality
 */
UENUM(BlueprintType)
enum class EMGLandingQuality : uint8
{
	Perfect				UMETA(DisplayName = "Perfect"),
	Great				UMETA(DisplayName = "Great"),
	Good				UMETA(DisplayName = "Good"),
	Rough				UMETA(DisplayName = "Rough"),
	Bad					UMETA(DisplayName = "Bad"),
	Crash				UMETA(DisplayName = "Crash")
};

/**
 * Airtime trick type
 */
UENUM(BlueprintType)
enum class EMGAirtimeTrick : uint8
{
	None				UMETA(DisplayName = "None"),
	Barrel				UMETA(DisplayName = "Barrel Roll"),
	Flip				UMETA(DisplayName = "Flip"),
	Spin				UMETA(DisplayName = "Spin"),
	Corkscrew			UMETA(DisplayName = "Corkscrew"),
	FlatSpin			UMETA(DisplayName = "Flat Spin"),
	Invert				UMETA(DisplayName = "Invert"),
	NoseGrab			UMETA(DisplayName = "Nose Grab"),
	TailGrab			UMETA(DisplayName = "Tail Grab")
};

/**
 * Jump rating
 */
UENUM(BlueprintType)
enum class EMGJumpRating : uint8
{
	None				UMETA(DisplayName = "None"),
	Bronze				UMETA(DisplayName = "Bronze"),
	Silver				UMETA(DisplayName = "Silver"),
	Gold				UMETA(DisplayName = "Gold"),
	Platinum			UMETA(DisplayName = "Platinum"),
	Diamond				UMETA(DisplayName = "Diamond"),
	Legend				UMETA(DisplayName = "Legend")
};

/**
 * Active jump session
 */
USTRUCT(BlueprintType)
struct FMGActiveJump
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString JumpId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpType Type = EMGJumpType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirtimeDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LaunchVelocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CurrentRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator TotalRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGAirtimeTrick> ActiveTricks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TricksCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNearMissWhileAirborne = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NearMissCount = 0;
};

/**
 * Jump result
 */
USTRUCT(BlueprintType)
struct FMGJumpResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpType Type = EMGJumpType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirtimeDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HorizontalDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandingSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLandingQuality LandingQuality = EMGLandingQuality::Good;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpRating Rating = EMGJumpRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrickScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LandingBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGAirtimeTrick> TricksPerformed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrickCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPersonalBest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWorldRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Ramp definition
 */
USTRUCT(BlueprintType)
struct FMGRampDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpType Type = EMGJumpType::Ramp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBoostPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerWidth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerLength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinLaunchSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeDistanceMeters = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverDistanceMeters = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldDistanceMeters = 75;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumDistanceMeters = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiamondDistanceMeters = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LegendDistanceMeters = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowTricks = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSecret = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> RampAsset;
};

/**
 * Trick definition
 */
USTRUCT(BlueprintType)
struct FMGTrickDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAirtimeTrick Type = EMGAirtimeTrick::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BasePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinAirtimeRequired = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotationRequired = 360.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotationAxis = FRotator(0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExecutionTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanChain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChainMultiplier = 1.2f;
};

/**
 * Player airtime stats
 */
USTRUCT(BlueprintType)
struct FMGAirtimePlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalJumps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalAirtime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestAirtime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestJump = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalTricks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrashLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestSingleJumpScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGAirtimeTrick, int32> TrickCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGJumpRating, int32> RatingCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> RampBestDistances;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SecretRampsFound = 0;
};

/**
 * Ramp record
 */
USTRUCT(BlueprintType)
struct FMGRampRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RampId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PersonalBestDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorldRecordDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WorldRecordHolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PersonalBestScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGJumpRating BestRating = EMGJumpRating::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SuccessfulLandings = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime PersonalBestDate;
};

/**
 * Airtime scoring config
 */
USTRUCT(BlueprintType)
struct FMGAirtimeScoringConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsPerSecondAirtime = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsPerMeterHeight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointsPerMeterDistance = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectLandingMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatLandingMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodLandingMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoughLandingMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BadLandingMultiplier = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrashLandingMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrickChainMultiplierPerTrick = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxTrickChainMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NearMissWhileAirborneBonus = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusMultiplier = 1.25f;
};

/**
 * Landing detection config
 */
USTRUCT(BlueprintType)
struct FMGLandingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectAngleTolerance = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GreatAngleTolerance = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoodAngleTolerance = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RoughAngleTolerance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrashAngleThreshold = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinGroundCheckDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandingImpactThreshold = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerfectSpeedRetention = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrashSpeedLoss = 0.5f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnJumpStarted, const FString&, PlayerId, EMGJumpType, Type, float, LaunchSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJumpEnded, const FString&, PlayerId, const FMGJumpResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAirtimeUpdate, const FString&, PlayerId, float, CurrentAirtime, float, CurrentHeight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrickCompleted, const FString&, PlayerId, EMGAirtimeTrick, Trick, int32, PointsEarned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrickChain, const FString&, PlayerId, int32, ChainCount, float, ChainMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLanding, const FString&, PlayerId, EMGLandingQuality, Quality, int32, LandingBonus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnJumpRating, const FString&, PlayerId, EMGJumpRating, Rating, const FString&, RampId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewPersonalBest, const FString&, PlayerId, const FString&, RampId, float, NewDistance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSecretRampFound, const FString&, PlayerId, const FString&, RampId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHeightReached, const FString&, PlayerId, float, Height);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNearMissWhileAirborne, const FString&, PlayerId, float, BonusMultiplier);

/**
 * Airtime Subsystem
 * Manages jumps, airtime, tricks, and landing mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAirtimeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnJumpStarted OnJumpStarted;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnJumpEnded OnJumpEnded;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnAirtimeUpdate OnAirtimeUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnTrickCompleted OnTrickCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnTrickChain OnTrickChain;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnLanding OnLanding;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnJumpRating OnJumpRating;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnSecretRampFound OnSecretRampFound;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnMaxHeightReached OnMaxHeightReached;

	UPROPERTY(BlueprintAssignable, Category = "Airtime|Events")
	FOnNearMissWhileAirborne OnNearMissWhileAirborne;

	// Ramp Registration
	UFUNCTION(BlueprintCallable, Category = "Airtime|Ramp")
	void RegisterRamp(const FMGRampDefinition& Ramp);

	UFUNCTION(BlueprintCallable, Category = "Airtime|Ramp")
	void UnregisterRamp(const FString& RampId);

	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	FMGRampDefinition GetRamp(const FString& RampId) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	TArray<FMGRampDefinition> GetAllRamps() const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	TArray<FMGRampDefinition> GetRampsInArea(FVector Center, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Ramp")
	TArray<FMGRampDefinition> GetRampsForTrack(const FString& TrackId) const;

	// Jump Detection
	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	bool CheckRampLaunch(const FString& PlayerId, FVector Location, FVector Velocity);

	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	void StartJump(const FString& PlayerId, const FString& RampId, FVector LaunchPosition, FVector LaunchVelocity);

	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	void UpdateJump(const FString& PlayerId, FVector Position, FVector Velocity, FRotator Rotation, bool bIsGrounded, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Airtime|Detection")
	FMGJumpResult EndJump(const FString& PlayerId, FVector LandingPosition, FVector LandingVelocity, float LandingAngle);

	UFUNCTION(BlueprintPure, Category = "Airtime|Detection")
	bool IsAirborne(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Detection")
	FMGActiveJump GetActiveJump(const FString& PlayerId) const;

	// Tricks
	UFUNCTION(BlueprintCallable, Category = "Airtime|Tricks")
	void RegisterTrick(const FMGTrickDefinition& Trick);

	UFUNCTION(BlueprintPure, Category = "Airtime|Tricks")
	FMGTrickDefinition GetTrickDefinition(EMGAirtimeTrick Type) const;

	UFUNCTION(BlueprintCallable, Category = "Airtime|Tricks")
	bool PerformTrick(const FString& PlayerId, EMGAirtimeTrick Trick);

	UFUNCTION(BlueprintCallable, Category = "Airtime|Tricks")
	void DetectTricks(const FString& PlayerId, FRotator DeltaRotation, float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Airtime|Tricks")
	bool CanPerformTrick(const FString& PlayerId, EMGAirtimeTrick Trick) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Tricks")
	TArray<EMGAirtimeTrick> GetAvailableTricks(const FString& PlayerId) const;

	// Landing
	UFUNCTION(BlueprintPure, Category = "Airtime|Landing")
	EMGLandingQuality CalculateLandingQuality(FVector Velocity, FVector SurfaceNormal, FRotator VehicleRotation) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Landing")
	int32 GetLandingBonus(EMGLandingQuality Quality) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Landing")
	float GetLandingSpeedRetention(EMGLandingQuality Quality) const;

	// Scoring
	UFUNCTION(BlueprintPure, Category = "Airtime|Scoring")
	int32 CalculateJumpScore(const FMGActiveJump& Jump, EMGLandingQuality Landing) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Scoring")
	EMGJumpRating CalculateRating(const FString& RampId, float Distance) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Scoring")
	int32 CalculateTrickScore(EMGAirtimeTrick Trick, int32 ChainCount) const;

	// Records
	UFUNCTION(BlueprintPure, Category = "Airtime|Records")
	FMGRampRecord GetRampRecord(const FString& RampId) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Records")
	float GetPersonalBestDistance(const FString& RampId) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Records")
	float GetWorldRecord(const FString& RampId) const;

	UFUNCTION(BlueprintCallable, Category = "Airtime|Records")
	void SetWorldRecord(const FString& RampId, float Distance, const FString& PlayerName);

	// Bonuses
	UFUNCTION(BlueprintCallable, Category = "Airtime|Bonus")
	void RegisterNearMissWhileAirborne(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Airtime|Bonus")
	void ApplySpeedBonus(const FString& PlayerId, float SpeedMPH);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Airtime|Stats")
	FMGAirtimePlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Airtime|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Discovery
	UFUNCTION(BlueprintCallable, Category = "Airtime|Discovery")
	void DiscoverSecretRamp(const FString& PlayerId, const FString& RampId);

	UFUNCTION(BlueprintPure, Category = "Airtime|Discovery")
	bool IsRampDiscovered(const FString& RampId) const;

	UFUNCTION(BlueprintPure, Category = "Airtime|Discovery")
	TArray<FString> GetDiscoveredRamps() const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Airtime|Config")
	void SetScoringConfig(const FMGAirtimeScoringConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Airtime|Config")
	FMGAirtimeScoringConfig GetScoringConfig() const;

	UFUNCTION(BlueprintCallable, Category = "Airtime|Config")
	void SetLandingConfig(const FMGLandingConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Airtime|Config")
	FMGLandingConfig GetLandingConfig() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Airtime|Update")
	void UpdateAirtimeSystem(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Airtime|Persistence")
	void SaveAirtimeData();

	UFUNCTION(BlueprintCallable, Category = "Airtime|Persistence")
	void LoadAirtimeData();

protected:
	void TickAirtime(float DeltaTime);
	void UpdateActiveJumps(float DeltaTime);
	void UpdateJumpMetrics(FMGActiveJump& Jump, FVector Position, float DeltaTime);
	FMGJumpResult FinalizeJump(const FString& PlayerId, EMGLandingQuality Landing);
	void UpdateRecords(const FString& PlayerId, const FMGJumpResult& Result);
	void UpdatePlayerStats(const FString& PlayerId, const FMGJumpResult& Result);
	bool IsInRampTrigger(FVector Position, const FMGRampDefinition& Ramp) const;
	FString GenerateJumpId() const;
	FString GenerateResultId() const;

private:
	UPROPERTY()
	TMap<FString, FMGRampDefinition> Ramps;

	UPROPERTY()
	TMap<FString, FMGActiveJump> ActiveJumps;

	UPROPERTY()
	TMap<EMGAirtimeTrick, FMGTrickDefinition> TrickDefinitions;

	UPROPERTY()
	TMap<FString, FMGRampRecord> RampRecords;

	UPROPERTY()
	TMap<FString, FMGAirtimePlayerStats> PlayerStats;

	UPROPERTY()
	TArray<FString> DiscoveredRamps;

	UPROPERTY()
	FMGAirtimeScoringConfig ScoringConfig;

	UPROPERTY()
	FMGLandingConfig LandingConfig;

	UPROPERTY()
	int32 JumpCounter = 0;

	UPROPERTY()
	int32 ResultCounter = 0;

	FTimerHandle AirtimeTickTimer;
};
