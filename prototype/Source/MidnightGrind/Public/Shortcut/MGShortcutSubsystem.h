// Copyright Epic Games, Inc. All Rights Reserved.

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
 * Shortcut entry point
 */
USTRUCT(BlueprintType)
struct FMGShortcutEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RequiredApproach = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ApproachTolerance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresBreakable = false;

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
 * Shortcut definition
 */
USTRUCT(BlueprintType)
struct FMGShortcutDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShortcutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortcutName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShortcutType Type = EMGShortcutType::Alley;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShortcutDifficulty Difficulty = EMGShortcutDifficulty::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGShortcutEntry Entry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGShortcutExit Exit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGShortcutWaypoint> Waypoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedTimeSaved = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PathLength = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DiscoveryPoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UsePoints = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MasteryUses = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RiskLevel = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSecret = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresUnlock = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnlockRequirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ConnectedShortcuts;

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
