// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLicenseSubsystem.generated.h"

/**
 * License tier determining access to vehicle classes and events
 */
UENUM(BlueprintType)
enum class EMGLicenseTier : uint8
{
	None			UMETA(DisplayName = "No License"),
	Learner			UMETA(DisplayName = "Learner Permit"),
	Street			UMETA(DisplayName = "Street License"),
	Club			UMETA(DisplayName = "Club License"),
	Regional		UMETA(DisplayName = "Regional License"),
	National		UMETA(DisplayName = "National License"),
	International	UMETA(DisplayName = "International License"),
	Professional	UMETA(DisplayName = "Professional License"),
	Elite			UMETA(DisplayName = "Elite License"),
	Legend			UMETA(DisplayName = "Legend License")
};

/**
 * License category for different racing disciplines
 */
UENUM(BlueprintType)
enum class EMGLicenseCategory : uint8
{
	General			UMETA(DisplayName = "General Racing"),
	Street			UMETA(DisplayName = "Street Racing"),
	Drift			UMETA(DisplayName = "Drift"),
	Drag			UMETA(DisplayName = "Drag Racing"),
	Circuit			UMETA(DisplayName = "Circuit Racing"),
	Rally			UMETA(DisplayName = "Rally"),
	Touge			UMETA(DisplayName = "Touge"),
	TimeAttack		UMETA(DisplayName = "Time Attack"),
	Endurance		UMETA(DisplayName = "Endurance")
};

/**
 * Test type for license examinations
 */
UENUM(BlueprintType)
enum class EMGLicenseTestType : uint8
{
	Written			UMETA(DisplayName = "Written Test"),
	BasicControl	UMETA(DisplayName = "Basic Control"),
	Cornering		UMETA(DisplayName = "Cornering"),
	Braking			UMETA(DisplayName = "Braking"),
	Overtaking		UMETA(DisplayName = "Overtaking"),
	RaceSimulation	UMETA(DisplayName = "Race Simulation"),
	TimeChallenge	UMETA(DisplayName = "Time Challenge"),
	Consistency		UMETA(DisplayName = "Consistency Test"),
	Advanced		UMETA(DisplayName = "Advanced Techniques")
};

/**
 * Grade for test completion
 */
UENUM(BlueprintType)
enum class EMGTestGrade : uint8
{
	NotAttempted	UMETA(DisplayName = "Not Attempted"),
	Failed			UMETA(DisplayName = "Failed"),
	Bronze			UMETA(DisplayName = "Bronze"),
	Silver			UMETA(DisplayName = "Silver"),
	Gold			UMETA(DisplayName = "Gold"),
	Platinum		UMETA(DisplayName = "Platinum")
};

/**
 * License test state
 */
UENUM(BlueprintType)
enum class EMGTestState : uint8
{
	Locked			UMETA(DisplayName = "Locked"),
	Available		UMETA(DisplayName = "Available"),
	InProgress		UMETA(DisplayName = "In Progress"),
	Completed		UMETA(DisplayName = "Completed"),
	AllGold			UMETA(DisplayName = "All Gold")
};

/**
 * Individual license test definition
 */
USTRUCT(BlueprintType)
struct FMGLicenseTest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTestType TestType = EMGLicenseTestType::BasicControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTier RequiredTier = EMGLicenseTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseCategory Category = EMGLicenseCategory::General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeTime = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverTime = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldTime = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlatinumTime = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeScore = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverScore = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldScore = 3000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumScore = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownBetweenAttempts = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PrerequisiteTestIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> TutorialAsset;
};

/**
 * Player's test result
 */
USTRUCT(BlueprintType)
struct FMGTestResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTestGrade BestGrade = EMGTestGrade::NotAttempted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstCompletedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BestGradeDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastAttemptDate;
};

/**
 * License school containing multiple tests
 */
USTRUCT(BlueprintType)
struct FMGLicenseSchool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SchoolId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SchoolName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTier TargetTier = EMGLicenseTier::Street;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseCategory Category = EMGLicenseCategory::General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLicenseTest> Tests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TestsRequiredToPass = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldTestsForBonus = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CashReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldBonusCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumBonusCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> VehicleRewardIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> SchoolIcon;
};

/**
 * Player license data for a category
 */
USTRUCT(BlueprintType)
struct FMGPlayerLicense
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseCategory Category = EMGLicenseCategory::General;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTier CurrentTier = EMGLicenseTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LicensePoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalGoldMedals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPlatinumMedals = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FMGTestResult> TestResults;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CompletedSchools;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LicenseObtainedDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUpgradeDate;
};

/**
 * Active test session
 */
USTRUCT(BlueprintType)
struct FMGActiveTestSession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TestId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SchoolId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PenaltyCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PenaltyTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;
};

/**
 * License privileges - what a license tier unlocks
 */
USTRUCT(BlueprintType)
struct FMGLicensePrivileges
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTier Tier = EMGLicenseTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedVehicleClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedEventTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedTracks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPurchasePrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxUpgradeLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanEnterOnlineRaces = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanEnterTournaments = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCreateCrew = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CashEarningsMultiplier = 1.0f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLicenseUpgraded, EMGLicenseCategory, Category, EMGLicenseTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTestCompleted, const FString&, TestId, EMGTestGrade, Grade, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSchoolCompleted, const FString&, SchoolId, int32, GoldMedals);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestStarted, const FString&, TestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTestFailed, const FString&, TestId, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewBestTime, const FString&, TestId, float, OldTime, float, NewTime);

/**
 * License Subsystem
 * Manages racing licenses, driving schools, and test progression
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLicenseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "License|Events")
	FOnLicenseUpgraded OnLicenseUpgraded;

	UPROPERTY(BlueprintAssignable, Category = "License|Events")
	FOnTestCompleted OnTestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "License|Events")
	FOnSchoolCompleted OnSchoolCompleted;

	UPROPERTY(BlueprintAssignable, Category = "License|Events")
	FOnTestStarted OnTestStarted;

	UPROPERTY(BlueprintAssignable, Category = "License|Events")
	FOnTestFailed OnTestFailed;

	UPROPERTY(BlueprintAssignable, Category = "License|Events")
	FOnNewBestTime OnNewBestTime;

	// License Management
	UFUNCTION(BlueprintCallable, Category = "License")
	EMGLicenseTier GetCurrentLicenseTier(EMGLicenseCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "License")
	FMGPlayerLicense GetPlayerLicense(EMGLicenseCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "License")
	bool HasLicenseTier(EMGLicenseCategory Category, EMGLicenseTier RequiredTier) const;

	UFUNCTION(BlueprintCallable, Category = "License")
	bool CanUpgradeLicense(EMGLicenseCategory Category, EMGLicenseTier TargetTier) const;

	UFUNCTION(BlueprintCallable, Category = "License")
	bool UpgradeLicense(EMGLicenseCategory Category, EMGLicenseTier NewTier);

	UFUNCTION(BlueprintPure, Category = "License")
	FMGLicensePrivileges GetLicensePrivileges(EMGLicenseTier Tier) const;

	UFUNCTION(BlueprintPure, Category = "License")
	EMGLicenseTier GetHighestLicenseTier() const;

	UFUNCTION(BlueprintPure, Category = "License")
	int32 GetTotalGoldMedals() const;

	UFUNCTION(BlueprintPure, Category = "License")
	int32 GetTotalPlatinumMedals() const;

	// School Management
	UFUNCTION(BlueprintCallable, Category = "License|School")
	bool RegisterSchool(const FMGLicenseSchool& School);

	UFUNCTION(BlueprintCallable, Category = "License|School")
	FMGLicenseSchool GetSchool(const FString& SchoolId) const;

	UFUNCTION(BlueprintCallable, Category = "License|School")
	TArray<FMGLicenseSchool> GetAvailableSchools(EMGLicenseCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "License|School")
	TArray<FMGLicenseSchool> GetAllSchools() const;

	UFUNCTION(BlueprintPure, Category = "License|School")
	bool IsSchoolCompleted(const FString& SchoolId) const;

	UFUNCTION(BlueprintPure, Category = "License|School")
	bool IsSchoolAllGold(const FString& SchoolId) const;

	UFUNCTION(BlueprintPure, Category = "License|School")
	float GetSchoolCompletionPercent(const FString& SchoolId) const;

	UFUNCTION(BlueprintPure, Category = "License|School")
	int32 GetSchoolGoldCount(const FString& SchoolId) const;

	// Test Management
	UFUNCTION(BlueprintCallable, Category = "License|Test")
	bool StartTest(const FString& TestId, const FString& SchoolId);

	UFUNCTION(BlueprintCallable, Category = "License|Test")
	bool EndTest(float FinalTime, int32 FinalScore, bool bCompleted);

	UFUNCTION(BlueprintCallable, Category = "License|Test")
	void CancelTest();

	UFUNCTION(BlueprintCallable, Category = "License|Test")
	void AddPenalty(float PenaltySeconds, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "License|Test")
	void RecordSectorTime(float SectorTime);

	UFUNCTION(BlueprintCallable, Category = "License|Test")
	void InvalidateTest(const FString& Reason);

	UFUNCTION(BlueprintPure, Category = "License|Test")
	bool IsTestActive() const;

	UFUNCTION(BlueprintPure, Category = "License|Test")
	FMGActiveTestSession GetActiveTestSession() const;

	UFUNCTION(BlueprintPure, Category = "License|Test")
	bool IsTestAvailable(const FString& TestId) const;

	UFUNCTION(BlueprintPure, Category = "License|Test")
	FMGTestResult GetTestResult(const FString& TestId) const;

	UFUNCTION(BlueprintPure, Category = "License|Test")
	EMGTestGrade GetTestGrade(const FString& TestId) const;

	UFUNCTION(BlueprintPure, Category = "License|Test")
	EMGTestGrade CalculateGradeFromTime(const FMGLicenseTest& Test, float Time) const;

	UFUNCTION(BlueprintPure, Category = "License|Test")
	EMGTestGrade CalculateGradeFromScore(const FMGLicenseTest& Test, int32 Score) const;

	// Privileges & Access
	UFUNCTION(BlueprintPure, Category = "License|Access")
	bool CanAccessVehicleClass(const FString& VehicleClassId) const;

	UFUNCTION(BlueprintPure, Category = "License|Access")
	bool CanAccessEvent(const FString& EventType) const;

	UFUNCTION(BlueprintPure, Category = "License|Access")
	bool CanAccessTrack(const FString& TrackId) const;

	UFUNCTION(BlueprintPure, Category = "License|Access")
	bool CanPurchaseVehicle(int32 VehiclePrice) const;

	UFUNCTION(BlueprintPure, Category = "License|Access")
	int32 GetMaxUpgradeLevel() const;

	UFUNCTION(BlueprintPure, Category = "License|Access")
	float GetReputationMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "License|Access")
	float GetCashMultiplier() const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "License|Stats")
	int32 GetTotalTestsCompleted() const;

	UFUNCTION(BlueprintPure, Category = "License|Stats")
	int32 GetTotalTestAttempts() const;

	UFUNCTION(BlueprintPure, Category = "License|Stats")
	float GetAverageTestGrade() const;

	UFUNCTION(BlueprintPure, Category = "License|Stats")
	float GetOverallLicenseProgress() const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "License|Persistence")
	void SaveLicenseData();

	UFUNCTION(BlueprintCallable, Category = "License|Persistence")
	void LoadLicenseData();

protected:
	void UpdateLicenseFromSchoolCompletion(const FString& SchoolId);
	void GrantSchoolRewards(const FMGLicenseSchool& School, int32 GoldCount, int32 PlatinumCount);
	void CheckLicenseUpgrade(EMGLicenseCategory Category);
	FMGLicenseTest* FindTest(const FString& TestId, FMGLicenseSchool** OutSchool = nullptr);

private:
	UPROPERTY()
	TMap<EMGLicenseCategory, FMGPlayerLicense> PlayerLicenses;

	UPROPERTY()
	TMap<FString, FMGLicenseSchool> RegisteredSchools;

	UPROPERTY()
	TMap<EMGLicenseTier, FMGLicensePrivileges> TierPrivileges;

	UPROPERTY()
	FMGActiveTestSession ActiveTestSession;

	UPROPERTY()
	bool bTestActive = false;

	FTimerHandle TestUpdateTimer;
};
