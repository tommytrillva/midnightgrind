// Copyright Midnight Grind. All Rights Reserved.


#pragma once
// Stage 51: Race Starter - Bridge between Garage/UI and Race Flow

/**
 * @file MGRaceStarter.h
 * @brief Race Starter Component - Simplified interface for starting races from UI/Blueprints
 *
 * This component and accompanying Blueprint Function Library provide a simplified,
 * high-level interface for starting races. It bridges the gap between the garage/menu
 * UI and the more complex RaceFlowSubsystem, handling all the setup details automatically.
 *
 * ============================================================================
 * KEY CONCEPTS FOR BEGINNERS
 * ============================================================================
 *
 * WHY DOES THIS EXIST?
 * Starting a race involves many steps:
 * 1. Get the player's selected vehicle from the garage
 * 2. Validate the vehicle is race-ready
 * 3. Choose or validate the track
 * 4. Configure AI opponents
 * 5. Set up timing and scoring
 * 6. Trigger level loading
 *
 * The RaceStarter simplifies this to single function calls like StartQuickRace()
 * while handling all the complexity internally.
 *
 * TWO WAYS TO USE:
 * 1. UMGRaceStarter (Component): Attach to a menu actor for state tracking
 * 2. UMGRaceStarterLibrary (Static): Call from anywhere, no setup needed
 *
 * QUICK RACE VS CUSTOM RACE:
 * - Quick Race: One function call, uses defaults and randomization
 * - Custom Race: Builder pattern for full control over settings
 *
 * ============================================================================
 * COMPONENT USAGE (UMGRaceStarter)
 * ============================================================================
 *
 * The component approach is best when you need:
 * - Persistent settings between race attempts
 * - Custom race builder with step-by-step configuration
 * - Event callbacks for race start results
 *
 * @code
 * // In your garage/menu actor
 * UPROPERTY()
 * UMGRaceStarter* RaceStarter;
 *
 * void AMyGarageActor::BeginPlay()
 * {
 *     RaceStarter = NewObject<UMGRaceStarter>(this);
 *     RaceStarter->OnRaceStartResult.AddDynamic(this, &AMyGarageActor::HandleResult);
 * }
 *
 * // One-click quick race
 * void AMyGarageActor::OnQuickRaceClicked()
 * {
 *     RaceStarter->StartQuickRace();
 * }
 *
 * // Custom race with builder pattern
 * void AMyGarageActor::OnStartCustomRace()
 * {
 *     RaceStarter->BeginCustomRace();
 *     RaceStarter->SetTrack(TEXT("Tokyo_Highway"));
 *     RaceStarter->SetRaceType(TEXT("Sprint"));
 *     RaceStarter->SetAI(5, 0.7f); // 5 opponents, 70% difficulty
 *     RaceStarter->SetTimeOfDay(0.0f); // Midnight
 *     RaceStarter->CommitRace();
 * }
 * @endcode
 *
 * ============================================================================
 * STATIC LIBRARY USAGE (UMGRaceStarterLibrary)
 * ============================================================================
 *
 * The static library is best for:
 * - Simple one-off race starts
 * - Blueprint scripts without permanent state
 * - Quick testing and prototyping
 *
 * @code
 * // Start a quick race on any track
 * UMGRaceStarterLibrary::StartQuickRace(this);
 *
 * // Start a quick race on a specific track
 * UMGRaceStarterLibrary::StartQuickRace(this, FName("Downtown_Circuit"));
 *
 * // Start with full configuration
 * FMGRaceSetupRequest Setup;
 * Setup.TrackID = FName("Mountain_Pass");
 * Setup.PlayerVehicleID = UMGRaceStarterLibrary::GetSelectedVehicleID(this);
 * Setup.LapCount = 5;
 * Setup.AICount = 7;
 * Setup.AIDifficulty = 0.8f;
 * UMGRaceStarterLibrary::StartRace(this, Setup);
 *
 * // Test race for development
 * UMGRaceStarterLibrary::StartTestRace(this);
 * @endcode
 *
 * ============================================================================
 * BLUEPRINT USAGE
 * ============================================================================
 *
 * All functions are exposed to Blueprint. Common patterns:
 *
 * Quick Race Button:
 *   [Button Clicked] -> [Start Quick Race] -> Done!
 *
 * Custom Race Screen:
 *   [Begin Custom Race]
 *        |
 *   [User selects track] -> [Set Track]
 *        |
 *   [User selects options] -> [Set AI], [Set Laps], etc.
 *        |
 *   [Start Button] -> [Commit Race]
 *
 * ============================================================================
 * RACE EVENT TYPES
 * ============================================================================
 *
 * @see EMGRaceEventType for categorizing race purposes:
 * - Standard: Normal race selected from menu
 * - QuickRace: Randomized quick-start race
 * - Career: Story mode progression race
 * - Multiplayer: Online race
 * - PinkSlip: Wager race for vehicle ownership
 * - Custom: Player-configured private race
 *
 * ============================================================================
 * QUICK RACE SETTINGS
 * ============================================================================
 *
 * @see FMGQuickRaceSettings for configuring quick race defaults:
 * - PreferredRaceType: Default race type (Circuit, Sprint, etc.)
 * - DefaultLaps: Number of laps for circuit races
 * - DefaultAICount: Number of AI opponents
 * - DefaultDifficulty: AI skill level (0-1)
 * - bRandomizeTrack: Pick random track vs use favorites
 * - FavoriteTracks: List of preferred tracks for random selection
 * - bMidnightOnly: Always use midnight time of day
 *
 * @see UMGRaceFlowSubsystem for the underlying race orchestration
 * @see UMGGarageSubsystem for vehicle selection
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "MGRaceStarter.generated.h"

class UMGGarageSubsystem;
class UMGRaceFlowSubsystem;

/**
 * Race event type
 */
UENUM(BlueprintType)
enum class EMGRaceEventType : uint8
{
	/** Standard race (any type) */
	Standard,
	/** Quick race (random settings) */
	QuickRace,
	/** Story/Career race */
	Career,
	/** Online multiplayer race */
	Multiplayer,
	/** Pink slip race */
	PinkSlip,
	/** Custom/private race */
	Custom
};

/**
 * Quick race settings for one-click racing
 */
USTRUCT(BlueprintType)
struct FMGQuickRaceSettings
{
	GENERATED_BODY()

	/** Preferred race type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	FName PreferredRaceType = FName("Circuit");

	/** Default lap count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	int32 DefaultLaps = 3;

	/** Default AI count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	int32 DefaultAICount = 5;

	/** Default difficulty (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	float DefaultDifficulty = 0.5f;

	/** Randomize track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	bool bRandomizeTrack = true;

	/** Favorite tracks (for random selection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	TArray<FName> FavoriteTracks;

	/** Always use midnight time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickRace")
	bool bMidnightOnly = true;
};

/**
 * Delegate for race starter events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceStarterResult, bool, bSuccess, const FString&, Message);

/**
 * Race Starter
 *
 * Utility component that bridges the garage/UI to the race flow system.
 * Place in your garage or menu level to easily start races.
 *
 * Features:
 * - Start races with current garage selection
 * - Quick race with one function call
 * - Race customization before starting
 * - Handles all validation and setup
 */
UCLASS(ClassGroup=(MidnightGrind), meta=(BlueprintSpawnableComponent))
class MIDNIGHTGRIND_API UMGRaceStarter : public UActorComponent
{
	GENERATED_BODY()

public:
	UMGRaceStarter();

	virtual void BeginPlay() override;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when race start attempt completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarterResult OnRaceStartResult;

	// ==========================================
	// QUICK START FUNCTIONS
	// ==========================================

	/**
	 * Start a quick race with the currently selected vehicle
	 * Uses random or favorite track
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter")
	bool StartQuickRace();

	/**
	 * Start a race with the selected vehicle on a specific track
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter")
	bool StartRaceOnTrack(FName TrackID);

	/**
	 * Start a test race for MVP verification
	 * Uses default vehicle and easy AI
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter")
	bool StartTestRace();

	/**
	 * Start a career/story race
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter")
	bool StartCareerRace(FName EventID);

	// ==========================================
	// CUSTOM RACE SETUP
	// ==========================================

	/**
	 * Begin setting up a custom race
	 * Call this first, then Set functions, then CommitRace
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void BeginCustomRace();

	/** Set track for custom race */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetTrack(FName TrackID);

	/** Set race type */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetRaceType(FName RaceType);

	/** Set lap count */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetLapCount(int32 Laps);

	/** Set AI count and difficulty */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetAI(int32 Count, float Difficulty);

	/** Set time of day (0 = midnight, 0.5 = noon) */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetTimeOfDay(float Time);

	/** Set weather (0 = clear, 1 = storm) */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetWeather(float Weather);

	/** Set as pink slip race */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetPinkSlip(bool bEnabled, FName OpponentVehicleID);

	/** Override vehicle (instead of using garage selection) */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void SetVehicleOverride(FName VehicleID);

	/** Get current custom race setup */
	UFUNCTION(BlueprintPure, Category = "RaceStarter|Custom")
	FMGRaceSetupRequest GetCurrentSetup() const { return PendingSetup; }

	/**
	 * Start the custom race
	 * Returns false if setup is invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	bool CommitRace();

	/** Cancel custom race setup */
	UFUNCTION(BlueprintCallable, Category = "RaceStarter|Custom")
	void CancelCustomRace();

	// ==========================================
	// SETTINGS
	// ==========================================

	/** Quick race settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FMGQuickRaceSettings QuickRaceSettings;

	// ==========================================
	// STATUS
	// ==========================================

	/** Is custom race being configured? */
	UFUNCTION(BlueprintPure, Category = "RaceStarter")
	bool IsConfiguringRace() const { return bConfiguringRace; }

	/** Can start a race? (not already in one) */
	UFUNCTION(BlueprintPure, Category = "RaceStarter")
	bool CanStartRace() const;

	/** Get selected vehicle from garage */
	UFUNCTION(BlueprintPure, Category = "RaceStarter")
	FName GetSelectedVehicleID() const;

	/** Get available tracks */
	UFUNCTION(BlueprintPure, Category = "RaceStarter")
	TArray<FName> GetAvailableTracks() const;

protected:
	/** Pending race setup */
	UPROPERTY()
	FMGRaceSetupRequest PendingSetup;

	/** Is configuring a custom race */
	bool bConfiguringRace = false;

	/** Cached garage subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGGarageSubsystem> GarageSubsystem;

	/** Cached race flow subsystem */
	UPROPERTY()
	TWeakObjectPtr<UMGRaceFlowSubsystem> RaceFlowSubsystem;

	/** Cache subsystem references */
	void CacheSubsystems();

	/** Get a random track from favorites or all */
	FName SelectRandomTrack() const;

	/** Apply quick race settings to setup */
	void ApplyQuickRaceSettings(FMGRaceSetupRequest& Setup);

	/** Fill in missing setup values with defaults */
	void FillDefaultValues(FMGRaceSetupRequest& Setup);

	/** Report result */
	void ReportResult(bool bSuccess, const FString& Message);

	/** Load career event configuration by ID */
	FMGRaceSetupRequest LoadCareerEventConfig(FName EventID);
};

/**
 * Blueprint Function Library for race starting
 * Provides static functions for starting races from anywhere
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceStarterLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Start a quick race with the player's selected vehicle
	 * Easiest way to start racing
	 */
	UFUNCTION(BlueprintCallable, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static bool StartQuickRace(UObject* WorldContextObject, FName TrackID = NAME_None);

	/**
	 * Start a race with full configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static bool StartRace(UObject* WorldContextObject, const FMGRaceSetupRequest& Setup);

	/**
	 * Start a test race for development/MVP
	 */
	UFUNCTION(BlueprintCallable, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static bool StartTestRace(UObject* WorldContextObject);

	/**
	 * Get the race flow subsystem
	 */
	UFUNCTION(BlueprintPure, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static UMGRaceFlowSubsystem* GetRaceFlowSubsystem(UObject* WorldContextObject);

	/**
	 * Can a race be started right now?
	 */
	UFUNCTION(BlueprintPure, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static bool CanStartRace(UObject* WorldContextObject);

	/**
	 * Get the last race result
	 */
	UFUNCTION(BlueprintPure, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static FMGRaceFlowResult GetLastRaceResult(UObject* WorldContextObject);

	/**
	 * Get selected vehicle ID from garage
	 */
	UFUNCTION(BlueprintPure, Category = "MidnightGrind|Race", meta = (WorldContext = "WorldContextObject"))
	static FName GetSelectedVehicleID(UObject* WorldContextObject);
};
