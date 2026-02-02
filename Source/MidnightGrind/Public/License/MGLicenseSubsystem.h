// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGLicenseSubsystem.h
 * @brief Racing License and Driving School Progression System inspired by Gran Turismo
 *
 * @section Overview
 * This subsystem manages the player's racing licenses and driving school tests.
 * It's inspired by Gran Turismo's license system - players must earn licenses
 * to access higher-tier vehicles, events, and online features.
 *
 * @section WhyLicenses Why Licenses Exist
 *
 * @subsection ProgressionGating 1. Progression Gating
 * Licenses gate content to create a sense of progression.
 * New players start with basic vehicles and work up to supercars.
 *
 * @subsection SkillVerification 2. Skill Verification
 * License tests teach and verify driving skills:
 *   - Braking points
 *   - Racing lines
 *   - Overtaking techniques
 *   - Vehicle control
 *
 * @subsection FairOnline 3. Fair Online Play
 * Higher licenses indicate skill level, enabling:
 *   - Skill-based matchmaking
 *   - Access to competitive events
 *   - Tournament eligibility
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * @subsection LicenseTiers 1. License Tiers (EMGLicenseTier)
 * Progression from beginner to expert:
 *   - None: No license yet
 *   - Learner: Just started, tutorial access only
 *   - Street: Can participate in street races
 *   - Club: Access to club events and basic online
 *   - Regional: Regional championships unlocked
 *   - National: National-level competition
 *   - International: Cross-region competition
 *   - Professional: Pro-level events
 *   - Elite: Elite championships
 *   - Legend: Maximum prestige, all content unlocked
 *
 * @subsection LicenseCategories 2. License Categories (EMGLicenseCategory)
 * Different racing disciplines have separate licenses:
 *   - General: Overall racing proficiency
 *   - Street: Illegal street racing
 *   - Drift: Drifting competitions
 *   - Drag: Quarter-mile acceleration
 *   - Circuit: Closed track racing
 *   - Rally: Off-road and mixed surface
 *   - Touge: Mountain pass racing (Initial D style)
 *   - TimeAttack: Hot lap time trials
 *   - Endurance: Long-distance races
 *
 * @subsection LicenseSchools 3. License Schools (FMGLicenseSchool)
 * Collections of tests for a specific license tier.
 * Example: "Street License School" contains 10 tests teaching
 * street racing fundamentals. Pass enough tests to earn the license.
 *
 * @subsection LicenseTests 4. License Tests (FMGLicenseTest)
 * Individual challenges with time/score requirements:
 *   - Bronze: Minimum passing grade
 *   - Silver: Good performance
 *   - Gold: Excellent performance
 *   - Platinum: Near-perfect execution
 *
 * Getting Gold on all tests in a school often unlocks bonus rewards.
 *
 * @subsection LicensePrivileges 5. License Privileges (FMGLicensePrivileges)
 * What each license tier unlocks:
 *   - Vehicle classes you can purchase
 *   - Event types you can enter
 *   - Tracks you can access
 *   - Maximum upgrade levels
 *   - Online features (crews, tournaments)
 *   - Earnings multipliers
 *
 * @section CodeExamples Code Examples
 *
 * @subsection GettingSubsystem Getting the Subsystem
 * @code
 * UMGLicenseSubsystem* License = GetGameInstance()->GetSubsystem<UMGLicenseSubsystem>();
 * @endcode
 *
 * @subsection CheckingLicense Checking Player's License
 * @code
 * // Check player's current license tier
 * EMGLicenseTier CurrentTier = License->GetCurrentLicenseTier(EMGLicenseCategory::Street);
 *
 * // Check if player can access a vehicle class
 * if (!License->CanAccessVehicleClass("S_Class"))
 * {
 *     ShowMessage("Earn a National License to unlock S-Class vehicles!");
 * }
 *
 * // Check if player can enter an event
 * if (License->CanAccessEvent("Tournament"))
 * {
 *     ShowTournamentLobby();
 * }
 * @endcode
 *
 * @subsection BrowsingSchools Browsing License Schools
 * @code
 * // Get available license schools for a category
 * TArray<FMGLicenseSchool> Schools = License->GetAvailableSchools(EMGLicenseCategory::Street);
 *
 * for (const FMGLicenseSchool& School : Schools)
 * {
 *     float Progress = License->GetSchoolCompletionPercent(School.SchoolId);
 *     int32 GoldCount = License->GetSchoolGoldCount(School.SchoolId);
 *
 *     DisplaySchoolCard(School.SchoolName, Progress, GoldCount);
 * }
 * @endcode
 *
 * @subsection TakingTest Taking a License Test
 * @code
 * // Start a license test
 * License->StartTest("Street_Test_01", "Street_School");
 *
 * // During test: track penalties (hitting cones, going off track)
 * void ATestTrack::OnConeHit()
 * {
 *     License->AddPenalty(2.0f, "Hit cone");  // 2 second penalty
 * }
 *
 * // Record sector times as player passes checkpoints
 * void ACheckpoint::OnPlayerPass(float SectorTime)
 * {
 *     License->RecordSectorTime(SectorTime);
 * }
 *
 * // If player crashes badly
 * void AVehicle::OnCriticalCrash()
 * {
 *     License->InvalidateTest("Vehicle totaled");
 * }
 *
 * // When player crosses finish line
 * void AFinishLine::OnPlayerFinish(float FinalTime)
 * {
 *     License->EndTest(FinalTime, 0, true);  // time, score, completed
 *
 *     // Get the grade achieved
 *     EMGTestGrade Grade = License->GetTestGrade("Street_Test_01");
 * }
 * @endcode
 *
 * @subsection ListeningEvents Listening for Events
 * @code
 * // In your class setup
 * License->OnLicenseUpgraded.AddDynamic(this, &AMyClass::OnGotNewLicense);
 * License->OnTestCompleted.AddDynamic(this, &AMyClass::OnFinishedTest);
 * License->OnNewBestTime.AddDynamic(this, &AMyClass::OnSetNewRecord);
 *
 * void AMyClass::OnGotNewLicense(EMGLicenseCategory Category, EMGLicenseTier NewTier)
 * {
 *     ShowLicenseCeremony(Category, NewTier);
 *     PlayFanfare();
 * }
 *
 * void AMyClass::OnFinishedTest(const FString& TestId, EMGTestGrade Grade, float Time)
 * {
 *     ShowGradeAnimation(Grade);
 *     if (Grade >= EMGTestGrade::Gold)
 *     {
 *         PlayGoldMedalSound();
 *     }
 * }
 *
 * void AMyClass::OnSetNewRecord(const FString& TestId, float OldTime, float NewTime)
 * {
 *     ShowNewRecordPopup(OldTime, NewTime);
 * }
 * @endcode
 *
 * @section TestFlow Test Execution Flow
 * 1. Call StartTest() to begin a license test
 * 2. Track penalties with AddPenalty() (hitting cones, going off track)
 * 3. Record sector times with RecordSectorTime()
 * 4. If player crashes badly, call InvalidateTest()
 * 5. Call EndTest() when finished with final time/score
 * 6. System calculates grade and updates progress
 * 7. If enough tests passed, license upgrades automatically
 *
 * @section Events Events to Listen For
 *   - OnLicenseUpgraded: Player earned a new license tier
 *   - OnTestCompleted: Player finished a test (shows grade)
 *   - OnSchoolCompleted: Player finished all tests in a school
 *   - OnTestStarted: Test began (setup UI)
 *   - OnTestFailed: Player failed/invalidated test
 *   - OnNewBestTime: Player set a new personal best
 *
 * @see UMGProgressionSubsystem Handles overall player progression
 * @see UMGVehicleClassSubsystem License affects accessible vehicle classes
 * @see UMGEventSubsystem License affects accessible events
 *
 * @author Midnight Grind Team
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TestFramework/MGTestFrameworkSubsystem.h"
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
 *
 * A test is a single challenge the player must complete to earn their license.
 * Tests can be time-based (complete in under X seconds) or score-based
 * (achieve at least Y points).
 *
 * GRADING SYSTEM:
 * Each test has four grade thresholds:
 * - Bronze: Minimum passing grade (easiest to achieve)
 * - Silver: Good performance
 * - Gold: Excellent performance
 * - Platinum: Near-perfect execution (bragging rights)
 *
 * Getting Gold/Platinum on all tests in a school often unlocks bonus rewards.
 *
 * EXAMPLE TEST PROGRESSION:
 * Test: "Braking 101"
 * - Objective: Stop within the marked zone from 100 km/h
 * - Bronze: Stop within 5m of target
 * - Silver: Stop within 3m of target
 * - Gold: Stop within 1m of target
 * - Platinum: Stop within 0.5m of target
 */
USTRUCT(BlueprintType)
struct FMGLicenseTest
{
	GENERATED_BODY()

	/** Unique identifier for this test */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TestId;

	/** Display name (e.g., "Braking 101", "Hairpin Mastery") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TestName;

	/** Description of what the player must do */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Type of test (affects objectives and scoring) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTestType TestType = EMGLicenseTestType::BasicControl;

	/** License tier player needs before attempting this test */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTier RequiredTier = EMGLicenseTier::None;

	/** Which license category this test counts towards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseCategory Category = EMGLicenseCategory::General;

	/** Track/course where this test takes place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	/** Vehicle player must use (specific car for fair comparison) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	// --- TIME-BASED GRADING (lower is better) ---

	/** Time threshold for Bronze grade (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BronzeTime = 120.0f;

	/** Time threshold for Silver grade (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SilverTime = 100.0f;

	/** Time threshold for Gold grade (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GoldTime = 80.0f;

	/** Time threshold for Platinum grade (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlatinumTime = 70.0f;

	// --- SCORE-BASED GRADING (higher is better) ---

	/** Score threshold for Bronze grade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeScore = 1000;

	/** Score threshold for Silver grade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverScore = 2000;

	/** Score threshold for Gold grade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldScore = 3000;

	/** Score threshold for Platinum grade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlatinumScore = 5000;

	// --- ATTEMPT RESTRICTIONS ---

	/** Maximum number of attempts allowed (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttempts = 0;

	/** Time player must wait between attempts in seconds (0 = no cooldown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownBetweenAttempts = 0.0f;

	/** Tests that must be passed before this one is available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PrerequisiteTestIds;

	/** Tutorial video/content to show before first attempt */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> TutorialAsset;
};

// FMGTestResult - Canonical definition in: TestFramework/MGTestFrameworkSubsystem.h (included above)

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
 *
 * Each license tier grants specific privileges. Higher tiers unlock:
 * - Access to faster/more expensive vehicles
 * - Access to more challenging events
 * - Online multiplayer features
 * - Better earning multipliers
 *
 * EXAMPLE PROGRESSION:
 * - Learner: Only D-class vehicles, basic events, no online
 * - Street: C/D-class vehicles, street races, basic online
 * - Club: B/C/D-class, club events, create crews
 * - National: A/B/C/D-class, national events, tournaments
 * - Legend: All vehicles, all events, maximum multipliers
 *
 * This creates meaningful progression where licenses feel rewarding.
 */
USTRUCT(BlueprintType)
struct FMGLicensePrivileges
{
	GENERATED_BODY()

	/** Which license tier these privileges belong to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLicenseTier Tier = EMGLicenseTier::None;

	/** Vehicle class tiers the player can access (e.g., "D_Class", "C_Class") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedVehicleClasses;

	/** Event types the player can enter (e.g., "StreetRace", "Tournament") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedEventTypes;

	/** Tracks/venues the player can access */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> AllowedTracks;

	/** Maximum vehicle purchase price allowed (in game currency) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPurchasePrice = 0;

	/** Maximum upgrade level player can install (1 = Stage 1, 2 = Stage 2, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxUpgradeLevel = 1;

	/** Can the player participate in online races? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanEnterOnlineRaces = false;

	/** Can the player enter official tournaments? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanEnterTournaments = false;

	/** Can the player create their own racing crew? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCreateCrew = false;

	/** Multiplier applied to reputation earnings (1.0 = normal, 1.5 = 50% bonus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReputationMultiplier = 1.0f;

	/** Multiplier applied to cash earnings (1.0 = normal, 1.5 = 50% bonus) */
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
