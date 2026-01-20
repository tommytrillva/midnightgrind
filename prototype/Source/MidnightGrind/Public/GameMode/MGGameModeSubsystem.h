// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGGameModeSubsystem.generated.h"

/**
 * Game Mode System - Race Type Management
 * - Multiple race types with unique rules
 * - Dynamic rule modification
 * - Custom game mode creation
 * - Playlist management for quickplay
 * - Tournament mode integration
 */

UENUM(BlueprintType)
enum class EMGGameModeType : uint8
{
	CircuitRace,         // Traditional lap-based racing
	SprintRace,          // Point-to-point race
	Drift,               // Scoring based on drift performance
	TimeAttack,          // Beat the clock
	Elimination,         // Last place eliminated each lap
	KingOfTheHill,       // Hold position at front
	Tag,                 // Be "it" the longest
	Cops,                // Evade or catch
	Drag,                // 1/4 mile sprint
	Touge,               // Mountain pass battle
	FreeroamRace,        // Impromptu street race
	Tournament,          // Multi-race series
	Custom               // User-defined rules
};

UENUM(BlueprintType)
enum class EMGTrafficMode : uint8
{
	None,                // No traffic
	Light,               // Sparse traffic
	Normal,              // Standard traffic
	Heavy,               // Dense traffic
	OncomingOnly         // Traffic in opposite lanes only
};

UENUM(BlueprintType)
enum class EMGCatchUpMode : uint8
{
	Disabled,            // No rubber banding
	Subtle,              // Slight speed adjustment
	Moderate,            // Noticeable but fair
	Aggressive           // Strong catch-up
};

USTRUCT(BlueprintType)
struct FMGGameModeRules
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGGameModeType ModeType = EMGGameModeType::CircuitRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRacers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinRacers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeLimit = 0.0f; // 0 = no limit

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowNitro = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCollisions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGhostMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTrafficMode Traffic = EMGTrafficMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCatchUpMode CatchUp = EMGCatchUpMode::Subtle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDamageEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSlipstreamEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRestrictedCarClass = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredCarClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerformanceCapPI = 0; // 0 = no cap

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTeamRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamCount = 2;
};

USTRUCT(BlueprintType)
struct FMGGameModeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGameModeRules DefaultRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOfficial = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinReputationTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> ModeIcon;
};

USTRUCT(BlueprintType)
struct FMGPlaylistEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlaylistName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGGameModeType> IncludedModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> IncludedTracks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFeatured = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGGameModeRules RuleOverrides;
};

USTRUCT(BlueprintType)
struct FMGEliminationState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EliminatedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilElimination = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerInLastPlace;
};

USTRUCT(BlueprintType)
struct FMGDriftScoring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 CurrentCombo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DriftSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboTimer = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnGameModeChanged, const FMGGameModeInfo&, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRulesChanged, const FMGGameModeRules&, NewRules);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerEliminated, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnDriftScoreUpdate, const FMGDriftScoring&, Score, const FString&, PlayerID);

UCLASS()
class MIDNIGHTGRIND_API UMGGameModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Mode Selection
	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetGameMode(EMGGameModeType ModeType);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetGameModeByID(FName ModeID);

	UFUNCTION(BlueprintPure, Category = "GameMode")
	FMGGameModeInfo GetCurrentMode() const { return CurrentMode; }

	UFUNCTION(BlueprintPure, Category = "GameMode")
	FMGGameModeRules GetCurrentRules() const { return CurrentRules; }

	UFUNCTION(BlueprintPure, Category = "GameMode")
	TArray<FMGGameModeInfo> GetAvailableModes() const { return AvailableModes; }

	// Rule Modification
	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetRules(const FMGGameModeRules& Rules);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetLapCount(int32 Laps);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetTrafficMode(EMGTrafficMode Traffic);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetCatchUpMode(EMGCatchUpMode CatchUp);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetCollisionsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetNitroEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void SetPerformanceCap(int32 MaxPI);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Rules")
	void ResetToDefaultRules();

	// Playlists
	UFUNCTION(BlueprintPure, Category = "GameMode|Playlist")
	TArray<FMGPlaylistEntry> GetPlaylists() const { return Playlists; }

	UFUNCTION(BlueprintPure, Category = "GameMode|Playlist")
	TArray<FMGPlaylistEntry> GetFeaturedPlaylists() const;

	UFUNCTION(BlueprintCallable, Category = "GameMode|Playlist")
	void SelectPlaylist(FName PlaylistID);

	UFUNCTION(BlueprintPure, Category = "GameMode|Playlist")
	FMGPlaylistEntry GetCurrentPlaylist() const { return CurrentPlaylist; }

	// Elimination Mode
	UFUNCTION(BlueprintPure, Category = "GameMode|Elimination")
	FMGEliminationState GetEliminationState() const { return EliminationState; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Elimination")
	void EliminatePlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Elimination")
	void UpdateEliminationTimer(float DeltaTime);

	UFUNCTION(BlueprintPure, Category = "GameMode|Elimination")
	bool IsPlayerEliminated(const FString& PlayerID) const;

	// Drift Mode
	UFUNCTION(BlueprintPure, Category = "GameMode|Drift")
	FMGDriftScoring GetDriftScore(const FString& PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "GameMode|Drift")
	void UpdateDriftScore(const FString& PlayerID, float DriftAngle, float Speed, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Drift")
	void EndDriftCombo(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "GameMode|Drift")
	TArray<TPair<FString, int64>> GetDriftLeaderboard() const;

	// Custom Modes
	UFUNCTION(BlueprintCallable, Category = "GameMode|Custom")
	FName CreateCustomMode(const FMGGameModeInfo& ModeInfo);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Custom")
	void SaveCustomMode(FName ModeID);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Custom")
	void DeleteCustomMode(FName ModeID);

	UFUNCTION(BlueprintPure, Category = "GameMode|Custom")
	TArray<FMGGameModeInfo> GetCustomModes() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnGameModeChanged OnGameModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnRulesChanged OnRulesChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnPlayerEliminated OnPlayerEliminated;

	UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
	FMGOnDriftScoreUpdate OnDriftScoreUpdate;

protected:
	void InitializeModes();
	void InitializePlaylists();
	FMGGameModeInfo CreateModeInfo(EMGGameModeType Type) const;

private:
	UPROPERTY()
	TArray<FMGGameModeInfo> AvailableModes;

	UPROPERTY()
	TArray<FMGGameModeInfo> CustomModes;

	UPROPERTY()
	TArray<FMGPlaylistEntry> Playlists;

	FMGGameModeInfo CurrentMode;
	FMGGameModeRules CurrentRules;
	FMGPlaylistEntry CurrentPlaylist;
	FMGEliminationState EliminationState;
	TMap<FString, FMGDriftScoring> DriftScores;
};
