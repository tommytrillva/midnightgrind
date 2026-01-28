// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGShortcutSubsystem.h
 * @brief Secret Routes, Alternate Paths, and Discovery System for racing gameplay
 *
 * @section Overview
 * This subsystem manages hidden shortcuts and alternate routes throughout the
 * game world. Think of it like the shortcut system in games like Burnout or
 * Need for Speed - secret alleys, rooftop jumps, breakable fences that reveal
 * faster paths during races.
 *
 * @section WhyShortcuts Why Shortcuts Matter
 * Shortcuts add depth to racing gameplay:
 *   - Reward exploration and track knowledge
 *   - Create risk/reward decisions during races
 *   - Allow skilled players to gain advantages
 *   - Encourage replayability to discover all secrets
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * @subsection ShortcutTypes 1. Shortcut Types (EMGShortcutType)
 *   - Alley: Narrow street passage
 *   - Tunnel: Underground or covered passage
 *   - JumpRamp: Aerial shortcut over obstacles
 *   - Rooftop: Path across building roofs
 *   - Underground: Parking garage or subway route
 *   - Breakable: Requires smashing through obstacle (fence, gate, etc.)
 *   - Hidden: Not visible until discovered
 *   - Risky: High-speed path with danger of crashing
 *   - Technical: Requires precise driving skill
 *   - Secret: Special unmarked shortcuts with bonus rewards
 *
 * @subsection DiscoveryStates 2. Discovery States (EMGShortcutState)
 *   - Unknown: Player hasn't found it yet (no map marker)
 *   - Hinted: Player has received a hint about its existence
 *   - Discovered: Player has found the entry point
 *   - Used: Player has successfully used the shortcut
 *   - Mastered: Player has used it enough times (unlocks bonus)
 *
 * @subsection EntryExit 3. Entry/Exit Points (FMGShortcutEntry/FMGShortcutExit)
 * Entry: Where the shortcut begins
 *   - Has required approach angle (can't enter from wrong direction)
 *   - May have speed requirements (too fast = overshoot, too slow = can't make jump)
 *   - May require breaking through an obstacle first
 *
 * Exit: Where the shortcut ends
 *   - May be a jump that launches you back onto the track
 *   - Has a boost multiplier for dramatic exits
 *
 * @subsection Waypoints 4. Waypoints (FMGShortcutWaypoint)
 * Checkpoints inside the shortcut path. Missing waypoints can:
 *   - Fail the shortcut attempt (went wrong way)
 *   - Reduce the time saved bonus
 *   - Track player progress through complex shortcuts
 *
 * @subsection Breakables 5. Breakable Obstacles (FMGBreakableObstacle)
 * Objects that can be destroyed to access shortcuts:
 *   - Minimum speed required to break through
 *   - Respawn after a timer (so AI can use them too)
 *   - Track how many times player has broken each one
 *
 * @section CodeExamples Code Examples
 *
 * @subsection BasicUsage Basic Usage
 * @code
 * // Get the subsystem
 * UMGShortcutSubsystem* Shortcuts = GetGameInstance()->GetSubsystem<UMGShortcutSubsystem>();
 *
 * // Start tracking shortcuts for a race session
 * Shortcuts->StartSession();
 *
 * // Check if player is near a shortcut entry
 * FString NearbyShortcut;
 * if (Shortcuts->IsNearShortcutEntry(PlayerLocation, NearbyShortcut))
 * {
 *     // Show "Shortcut Ahead" UI indicator
 * }
 *
 * // When player enters a shortcut trigger
 * if (Shortcuts->TryEnterShortcut(ShortcutId, PlayerLocation, PlayerVelocity))
 * {
 *     // Player successfully entered! Start tracking their progress.
 * }
 *
 * // Each frame while in shortcut
 * Shortcuts->UpdateActiveShortcut(PlayerLocation, DeltaTime);
 *
 * // When player reaches exit
 * Shortcuts->ExitShortcut(true); // true = successful completion
 *
 * // End session and get stats
 * FMGShortcutSessionStats Stats = Shortcuts->GetSessionStats();
 * // Stats.TotalTimeSaved, Stats.ShortcutsUsed, etc.
 * @endcode
 *
 * @subsection DiscoveryExample Discovery Flow Example
 * @code
 * // Level designer places hint triggers near shortcuts but not at entry
 * void AShortcutHintTrigger::OnOverlap(AActor* OtherActor)
 * {
 *     if (IsPlayerVehicle(OtherActor))
 *     {
 *         // Hint appears on minimap as "?" marker
 *         Shortcuts->HintShortcut(ShortcutId);
 *     }
 * }
 *
 * // When player finds actual entry
 * void AShortcutEntryTrigger::OnOverlap(AActor* OtherActor)
 * {
 *     // Shortcut now appears with full icon on map
 *     Shortcuts->DiscoverShortcut(ShortcutId);
 * }
 * @endcode
 *
 * @subsection EventsExample Listening for Events
 * @code
 * // Bind to shortcut events
 * Shortcuts->OnShortcutDiscovered.AddDynamic(this, &AMyClass::HandleDiscovery);
 * Shortcuts->OnShortcutCompleted.AddDynamic(this, &AMyClass::HandleCompletion);
 * Shortcuts->OnShortcutMastered.AddDynamic(this, &AMyClass::HandleMastery);
 *
 * void AMyClass::HandleDiscovery(const FString& ShortcutId, int32 DiscoveryPoints)
 * {
 *     ShowDiscoveryPopup(ShortcutId, DiscoveryPoints);
 * }
 *
 * void AMyClass::HandleCompletion(const FString& ShortcutId, float TimeTaken, float TimeSaved)
 * {
 *     ShowTimeSavedUI(TimeSaved);
 * }
 * @endcode
 *
 * @subsection BreakablesExample Breaking Through Obstacles
 * @code
 * void AVehicle::OnCollision(const FHitResult& Hit)
 * {
 *     // Check if we hit a breakable obstacle
 *     FString ObstacleId = GetObstacleId(Hit.GetActor());
 *     if (!ObstacleId.IsEmpty())
 *     {
 *         float ImpactSpeed = GetVelocity().Size();
 *         if (Shortcuts->TryBreakObstacle(ObstacleId, ImpactSpeed))
 *         {
 *             // Obstacle broken! Shortcut path is now open.
 *         }
 *     }
 * }
 * @endcode
 *
 * @section DiscoveryFlow Discovery Flow Summary
 * 1. Place hint triggers in the world (near shortcuts but not at entry)
 * 2. When player drives through hint trigger, call HintShortcut(ShortcutId)
 * 3. Hint appears on minimap as "?" marker
 * 4. When player finds actual entry, call DiscoverShortcut(ShortcutId)
 * 5. Shortcut now appears with full icon on map
 * 6. After successful use, state changes to "Used"
 * 7. After 10 successful uses (configurable), state becomes "Mastered"
 *
 * @section Events Events to Listen For
 *   - OnShortcutDiscovered: First time player finds a shortcut (show discovery popup)
 *   - OnShortcutEntered: Player entered a shortcut (start shortcut UI)
 *   - OnShortcutCompleted: Player successfully exited (show time saved)
 *   - OnShortcutFailed: Player crashed or went wrong way (show failure message)
 *   - OnShortcutMastered: Player mastered a shortcut (bonus XP/cash)
 *   - OnBreakableDestroyed: Player broke through an obstacle
 *   - OnSecretShortcutFound: Player found a secret shortcut (big bonus)
 *
 * @see UMGRaceModeSubsystem Race timing factors in shortcut time savings
 * @see UMGNavigationSubsystem Shortcuts can appear on minimap
 *
 * @author Midnight Grind Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGShortcutSubsystem.generated.h"

/**
 * Type of shortcut
 */
UENUM(BlueprintType)
enum class EMGShortcutType : uint8
{
	Alley			UMETA(DisplayName = "Alley"),
	Tunnel			UMETA(DisplayName = "Tunnel"),
	JumpRamp		UMETA(DisplayName = "Jump Ramp"),
	Rooftop			UMETA(DisplayName = "Rooftop"),
	Underground		UMETA(DisplayName = "Underground"),
	Breakable		UMETA(DisplayName = "Breakable"),
	Hidden			UMETA(DisplayName = "Hidden Path"),
	Risky			UMETA(DisplayName = "Risky Route"),
	Scenic			UMETA(DisplayName = "Scenic Route"),
	Technical		UMETA(DisplayName = "Technical"),
	Secret			UMETA(DisplayName = "Secret")
};

/**
 * Shortcut discovery state
 */
UENUM(BlueprintType)
enum class EMGShortcutState : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),
	Hinted			UMETA(DisplayName = "Hinted"),
	Discovered		UMETA(DisplayName = "Discovered"),
	Used			UMETA(DisplayName = "Used"),
	Mastered		UMETA(DisplayName = "Mastered")
};

/**
 * Shortcut difficulty rating
 */
UENUM(BlueprintType)
enum class EMGShortcutDifficulty : uint8
{
	Easy			UMETA(DisplayName = "Easy"),
	Medium			UMETA(DisplayName = "Medium"),
	Hard			UMETA(DisplayName = "Hard"),
	Expert			UMETA(DisplayName = "Expert"),
	Insane			UMETA(DisplayName = "Insane")
};

/**
 * Shortcut entry point - Where a shortcut begins
 *
 * Defines the conditions required to enter a shortcut successfully:
 * - Location: Where the trigger volume is
 * - Approach Angle: Must enter from roughly the right direction
 * - Speed Requirements: Some shortcuts require minimum/maximum speed
 * - Breakable Gate: Some shortcuts require smashing through an obstacle
 *
 * Example: A rooftop jump shortcut might require:
 * - MinSpeed = 80 km/h (need enough speed to make the jump)
 * - MaxSpeed = 150 km/h (too fast and you'll overshoot)
 * - ApproachTolerance = 30 degrees (must approach the ramp straight)
 */
USTRUCT(BlueprintType)
struct FMGShortcutEntry
{
	GENERATED_BODY()

	/** World position of the shortcut entry trigger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** Direction player should be facing to enter correctly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RequiredApproach = FRotator::ZeroRotator;

	/** How many degrees off from RequiredApproach is still valid (cone angle) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ApproachTolerance = 45.0f;

	/** Minimum speed required to enter (0 = no minimum). In km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeed = 0.0f;

	/** Maximum speed allowed to enter (999 = no maximum). In km/h. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 999.0f;

	/** Radius of the entry trigger volume in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerRadius = 10.0f;

	/** Does this shortcut require breaking through an obstacle first? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresBreakable = false;

	/** ID of the breakable obstacle that must be destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BreakableId;
};

/**
 * Shortcut exit point
 */
USTRUCT(BlueprintType)
struct FMGShortcutExit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator ExitDirection = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJumpExit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpBoostMultiplier = 1.0f;
};

/**
 * Shortcut waypoint for navigation
 */
USTRUCT(BlueprintType)
struct FMGShortcutWaypoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SuggestedSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCritical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HintText;
};

/**
 * Shortcut definition - Complete data for a single shortcut
 *
 * This is the main data structure defining a shortcut in the game.
 * It includes everything needed to:
 * - Locate the shortcut (entry/exit points, waypoints)
 * - Validate player's attempt (speed, direction, waypoint hits)
 * - Reward the player (time saved, discovery points, mastery)
 *
 * REWARD SYSTEM:
 * - DiscoveryPoints: Awarded first time player finds/uses shortcut
 * - UsePoints: Awarded each time shortcut is successfully used
 * - Mastery: After MasteryUses successful completions, extra bonus
 *
 * RISK/REWARD:
 * - Higher RiskLevel = harder to execute but more time saved
 * - Secret shortcuts have bigger rewards but no hints
 */
USTRUCT(BlueprintType)
struct FMGShortcutDefinition
{
	GENERATED_BODY()

	/** Unique identifier for this shortcut */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShortcutId;

	/** Display name (e.g., "Warehouse Rooftop Jump") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortcutName;

	/** Flavor text describing the shortcut */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Category of shortcut (alley, tunnel, jump, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShortcutType Type = EMGShortcutType::Alley;

	/** How hard is this shortcut to execute? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShortcutDifficulty Difficulty = EMGShortcutDifficulty::Medium;

	/** Entry point configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGShortcutEntry Entry;

	/** Exit point configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGShortcutExit Exit;

	/** Waypoints that must be hit (in order) during the shortcut */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGShortcutWaypoint> Waypoints;

	/** How much time this shortcut saves vs the normal route (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedTimeSaved = 2.0f;

	/** Length of the shortcut path in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PathLength = 100.0f;

	/** Points/XP awarded when player first discovers this shortcut */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiscoveryPoints = 100;

	/** Points/XP awarded each time shortcut is used successfully */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UsePoints = 25;

	/** How many successful uses to achieve "Mastered" status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MasteryUses = 10;

	/** Risk rating from 0.0 (safe) to 1.0 (very risky). Affects AI usage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RiskLevel = 0.5f;

	/** Is this a secret shortcut? (No hints, no minimap marker until found) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSecret = false;

	/** Does this shortcut require a special unlock condition? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresUnlock = false;

	/** Description of what's needed to unlock (e.g., "Win the Downtown King event") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnlockRequirement;

	/** IDs of shortcuts that connect to this one (for combo bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ConnectedShortcuts;

	/** Which track/route this shortcut belongs to (for race-specific shortcuts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;
};

/**
 * Player's shortcut progress
 */
USTRUCT(BlueprintType)
struct FMGShortcutProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShortcutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShortcutState State = EMGShortcutState::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeSaved = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SuccessfulRuns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FailedRuns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstDiscovered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastUsed;
};

/**
 * Active shortcut attempt
 */
USTRUCT(BlueprintType)
struct FMGActiveShortcutAttempt
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShortcutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EntrySpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentWaypoint = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaypointsMissed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;
};

/**
 * Shortcut hint for undiscovered shortcuts
 */
USTRUCT(BlueprintType)
struct FMGShortcutHint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShortcutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText HintText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HintLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HintRadius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowOnMinimap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> HintIcon;
};

/**
 * Breakable obstacle for shortcuts
 */
USTRUCT(BlueprintType)
struct FMGBreakableObstacle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ObstacleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinBreakSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrokenTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesDestroyed = 0;
};

/**
 * Session shortcut statistics
 */
USTRUCT(BlueprintType)
struct FMGShortcutSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShortcutsDiscovered = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShortcutsUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShortcutsFailed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTimeSaved = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BreakablesDestroyed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SecretsFound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGShortcutType, int32> UsageByType;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShortcutDiscovered, const FString&, ShortcutId, int32, DiscoveryPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShortcutEntered, const FString&, ShortcutId, float, EntrySpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShortcutCompleted, const FString&, ShortcutId, float, TimeTaken, float, TimeSaved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShortcutFailed, const FString&, ShortcutId, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShortcutMastered, const FString&, ShortcutId, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaypointReached, int32, WaypointIndex, float, TimeTaken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBreakableDestroyed, const FString&, ObstacleId, int32, Points);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShortcutHintRevealed, const FString&, ShortcutId, const FMGShortcutHint&, Hint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSecretShortcutFound, const FString&, ShortcutId, int32, BonusPoints);

/**
 * Shortcut Subsystem
 * Manages secret shortcuts, alternate routes, and discovery mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGShortcutSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnShortcutDiscovered OnShortcutDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnShortcutEntered OnShortcutEntered;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnShortcutCompleted OnShortcutCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnShortcutFailed OnShortcutFailed;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnShortcutMastered OnShortcutMastered;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnWaypointReached OnWaypointReached;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnBreakableDestroyed OnBreakableDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnShortcutHintRevealed OnShortcutHintRevealed;

	UPROPERTY(BlueprintAssignable, Category = "Shortcut|Events")
	FOnSecretShortcutFound OnSecretShortcutFound;

	// Shortcut Registration
	UFUNCTION(BlueprintCallable, Category = "Shortcut")
	bool RegisterShortcut(const FMGShortcutDefinition& Shortcut);

	UFUNCTION(BlueprintCallable, Category = "Shortcut")
	FMGShortcutDefinition GetShortcut(const FString& ShortcutId) const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut")
	TArray<FMGShortcutDefinition> GetAllShortcuts() const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut")
	TArray<FMGShortcutDefinition> GetShortcutsForTrack(const FString& TrackId) const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut")
	TArray<FMGShortcutDefinition> GetDiscoveredShortcuts() const;

	// Active Shortcut
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Active")
	bool TryEnterShortcut(const FString& ShortcutId, FVector PlayerLocation, FVector PlayerVelocity);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Active")
	void UpdateActiveShortcut(FVector PlayerLocation, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Active")
	void ExitShortcut(bool bSuccessful);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Active")
	void FailShortcut(const FString& Reason);

	UFUNCTION(BlueprintPure, Category = "Shortcut|Active")
	bool IsInShortcut() const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Active")
	FMGActiveShortcutAttempt GetActiveAttempt() const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Active")
	FString GetActiveShortcutId() const;

	// Discovery
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Discovery")
	void DiscoverShortcut(const FString& ShortcutId);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Discovery")
	void HintShortcut(const FString& ShortcutId);

	UFUNCTION(BlueprintPure, Category = "Shortcut|Discovery")
	bool IsShortcutDiscovered(const FString& ShortcutId) const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Discovery")
	EMGShortcutState GetShortcutState(const FString& ShortcutId) const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Discovery")
	FMGShortcutProgress GetShortcutProgress(const FString& ShortcutId) const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Discovery")
	float GetDiscoveryPercent(const FString& TrackId) const;

	// Hints
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Hints")
	void RegisterHint(const FMGShortcutHint& Hint);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Hints")
	TArray<FMGShortcutHint> GetActiveHints() const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Hints")
	FMGShortcutHint GetNearestHint(FVector Location) const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Hints")
	void RevealHint(const FString& ShortcutId);

	// Breakables
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Breakables")
	void RegisterBreakable(const FMGBreakableObstacle& Obstacle);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Breakables")
	bool TryBreakObstacle(const FString& ObstacleId, float ImpactSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Breakables")
	void UpdateBreakables(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "Shortcut|Breakables")
	bool IsObstacleBroken(const FString& ObstacleId) const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Breakables")
	FMGBreakableObstacle GetBreakable(const FString& ObstacleId) const;

	// Proximity Detection
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Detection")
	FString GetNearestShortcutEntry(FVector Location, float MaxDistance) const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Detection")
	bool IsNearShortcutEntry(FVector Location, FString& OutShortcutId) const;

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Detection")
	TArray<FString> GetShortcutsInRange(FVector Location, float Range) const;

	// Statistics
	UFUNCTION(BlueprintPure, Category = "Shortcut|Stats")
	FMGShortcutSessionStats GetSessionStats() const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Stats")
	int32 GetTotalDiscoveredCount() const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Stats")
	int32 GetTotalMasteredCount() const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Stats")
	float GetTotalTimeSaved() const;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Session")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Session")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "Shortcut|Session")
	bool IsSessionActive() const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "Shortcut|Utility")
	FText GetShortcutTypeDisplayName(EMGShortcutType Type) const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Utility")
	FText GetDifficultyDisplayName(EMGShortcutDifficulty Difficulty) const;

	UFUNCTION(BlueprintPure, Category = "Shortcut|Utility")
	FLinearColor GetDifficultyColor(EMGShortcutDifficulty Difficulty) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Shortcut|Persistence")
	void SaveShortcutData();

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Persistence")
	void LoadShortcutData();

protected:
	void CheckWaypointProgress(FVector PlayerLocation);
	void CheckMastery(const FString& ShortcutId);
	float CalculateTimeSaved(const FMGShortcutDefinition& Shortcut, float ActualTime) const;

private:
	UPROPERTY()
	TMap<FString, FMGShortcutDefinition> RegisteredShortcuts;

	UPROPERTY()
	TMap<FString, FMGShortcutProgress> ShortcutProgress;

	UPROPERTY()
	TMap<FString, FMGShortcutHint> ShortcutHints;

	UPROPERTY()
	TMap<FString, FMGBreakableObstacle> Breakables;

	UPROPERTY()
	FMGActiveShortcutAttempt ActiveAttempt;

	UPROPERTY()
	FMGShortcutSessionStats SessionStats;

	UPROPERTY()
	bool bSessionActive = false;

	UPROPERTY()
	bool bInShortcut = false;
};
