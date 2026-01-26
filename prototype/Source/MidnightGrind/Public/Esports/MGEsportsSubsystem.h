// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGEsportsSubsystem.generated.h"

/**
 * Esports System - Professional Tournament Support
 * - Tournament bracket management
 * - Caster tools and commentary support
 * - Live stat overlays for production
 * - Auto-director for camera work
 * - Instant replay management
 * - Match history and vods
 */

UENUM(BlueprintType)
enum class EMGTournamentFormat : uint8
{
	SingleElimination,
	DoubleElimination,
	RoundRobin,
	Swiss,
	Custom
};

UENUM(BlueprintType)
enum class EMGMatchState : uint8
{
	Scheduled,
	Lobby,
	InProgress,
	Completed,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGAutoDirectorMode : uint8
{
	Disabled,
	BattlesFocus,    // Focus on close battles
	LeaderFocus,     // Focus on race leader
	DramaFocus,      // Follow interesting storylines
	Balanced         // Mix of everything
};

USTRUCT(BlueprintType)
struct FMGTournamentInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TournamentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TournamentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTournamentFormat Format = EMGTournamentFormat::SingleElimination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxParticipants = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRounds = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 PrizePool = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOfficial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLive = false;
};

USTRUCT(BlueprintType)
struct FMGTournamentMatch
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MatchID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TournamentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MatchNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ParticipantIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchState State = EMGMatchState::Scheduled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WinnerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ScheduledTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumRaces = 3; // Best of X
};

USTRUCT(BlueprintType)
struct FMGParticipantStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TeamName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Losses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageFinishPosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FastestLaps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimespan BestLapTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Seed = 0;
};

USTRUCT(BlueprintType)
struct FMGCasterInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CasterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPrimaryCaster = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasCameraControl = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasReplayControl = false;
};

USTRUCT(BlueprintType)
struct FMGCasterToolsState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowExtendedStats = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowGapTiming = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSectorTimes = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowTireCondition = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowNitroStatus = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowHistoricalData = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FocusedPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> ComparePlayerIDs;
};

USTRUCT(BlueprintType)
struct FMGInstantReplay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaybackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutoGenerated = false;
};

USTRUCT(BlueprintType)
struct FMGAutoDirectorSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGAutoDirectorMode Mode = EMGAutoDirectorMode::Balanced;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinCameraDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxCameraDuration = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BattleProximityThreshold = 50.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoReplay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReplayDelay = 5.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMatchStateChanged, const FMGTournamentMatch&, Match);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnTournamentAdvanced, const FMGTournamentInfo&, Tournament, int32, NewRound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnReplayAvailable, const FMGInstantReplay&, Replay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAutoDirectorCameraSwitch, const FString&, NewFocusPlayerID);

UCLASS()
class MIDNIGHTGRIND_API UMGEsportsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tournament Management
	UFUNCTION(BlueprintCallable, Category = "Esports|Tournament")
	void CreateTournament(const FMGTournamentInfo& Info);

	UFUNCTION(BlueprintCallable, Category = "Esports|Tournament")
	void StartTournament(const FString& TournamentID);

	UFUNCTION(BlueprintCallable, Category = "Esports|Tournament")
	void AdvanceToNextRound(const FString& TournamentID);

	UFUNCTION(BlueprintCallable, Category = "Esports|Tournament")
	void RegisterParticipant(const FString& TournamentID, const FMGParticipantStats& Participant);

	UFUNCTION(BlueprintPure, Category = "Esports|Tournament")
	FMGTournamentInfo GetTournamentInfo(const FString& TournamentID) const;

	UFUNCTION(BlueprintPure, Category = "Esports|Tournament")
	TArray<FMGTournamentMatch> GetBracket(const FString& TournamentID) const;

	UFUNCTION(BlueprintPure, Category = "Esports|Tournament")
	TArray<FMGParticipantStats> GetStandings(const FString& TournamentID) const;

	// Match Management
	UFUNCTION(BlueprintCallable, Category = "Esports|Match")
	void StartMatch(const FString& MatchID);

	UFUNCTION(BlueprintCallable, Category = "Esports|Match")
	void EndMatch(const FString& MatchID, const FString& WinnerID);

	UFUNCTION(BlueprintCallable, Category = "Esports|Match")
	void RecordRaceResult(const FString& MatchID, const TArray<FString>& FinishOrder);

	UFUNCTION(BlueprintPure, Category = "Esports|Match")
	FMGTournamentMatch GetCurrentMatch() const { return CurrentMatch; }

	UFUNCTION(BlueprintPure, Category = "Esports|Match")
	bool IsMatchInProgress() const { return CurrentMatch.State == EMGMatchState::InProgress; }

	// Caster Tools
	UFUNCTION(BlueprintCallable, Category = "Esports|Caster")
	void JoinAsCaster(const FMGCasterInfo& CasterInfo);

	UFUNCTION(BlueprintCallable, Category = "Esports|Caster")
	void LeaveCaster();

	UFUNCTION(BlueprintCallable, Category = "Esports|Caster")
	void SetCasterToolsState(const FMGCasterToolsState& State);

	UFUNCTION(BlueprintPure, Category = "Esports|Caster")
	FMGCasterToolsState GetCasterToolsState() const { return CasterTools; }

	UFUNCTION(BlueprintPure, Category = "Esports|Caster")
	bool IsCasting() const { return bIsCasting; }

	UFUNCTION(BlueprintCallable, Category = "Esports|Caster")
	void FocusOnPlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Esports|Caster")
	void SetComparisonPlayers(const TArray<FString>& PlayerIDs);

	UFUNCTION(BlueprintPure, Category = "Esports|Caster")
	FMGParticipantStats GetLivePlayerStats(const FString& PlayerID) const;

	// Auto-Director
	UFUNCTION(BlueprintCallable, Category = "Esports|Director")
	void EnableAutoDirector(const FMGAutoDirectorSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Esports|Director")
	void DisableAutoDirector();

	UFUNCTION(BlueprintPure, Category = "Esports|Director")
	bool IsAutoDirectorEnabled() const { return bAutoDirectorEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Esports|Director")
	void OverrideAutoDirector(const FString& FocusPlayerID, float Duration);

	// Instant Replay
	UFUNCTION(BlueprintCallable, Category = "Esports|Replay")
	void MarkReplayMoment(const FText& Label);

	UFUNCTION(BlueprintCallable, Category = "Esports|Replay")
	void PlayInstantReplay(const FMGInstantReplay& Replay);

	UFUNCTION(BlueprintCallable, Category = "Esports|Replay")
	void StopReplay();

	UFUNCTION(BlueprintPure, Category = "Esports|Replay")
	TArray<FMGInstantReplay> GetAvailableReplays() const { return AvailableReplays; }

	UFUNCTION(BlueprintPure, Category = "Esports|Replay")
	bool IsReplayPlaying() const { return bReplayPlaying; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Esports|Events")
	FMGOnMatchStateChanged OnMatchStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Esports|Events")
	FMGOnTournamentAdvanced OnTournamentAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Esports|Events")
	FMGOnReplayAvailable OnReplayAvailable;

	UPROPERTY(BlueprintAssignable, Category = "Esports|Events")
	FMGOnAutoDirectorCameraSwitch OnAutoDirectorCameraSwitch;

protected:
	void UpdateAutoDirector(float DeltaTime);
	void GenerateAutoReplay(const TArray<FString>& InvolvedPlayers, float Duration);
	void GenerateBracket(const FString& TournamentID);
	FString DetermineNextFocus();

private:
	UPROPERTY()
	TMap<FString, FMGTournamentInfo> Tournaments;

	UPROPERTY()
	TMap<FString, TArray<FMGTournamentMatch>> TournamentBrackets;

	UPROPERTY()
	TMap<FString, TArray<FMGParticipantStats>> TournamentParticipants;

	FMGTournamentMatch CurrentMatch;
	FMGCasterInfo LocalCaster;
	FMGCasterToolsState CasterTools;
	FMGAutoDirectorSettings AutoDirectorSettings;
	TArray<FMGInstantReplay> AvailableReplays;
	FTimerHandle AutoDirectorTimerHandle;
	float TimeSinceLastCameraSwitch = 0.0f;
	FString CurrentAutoDirectorFocus;
	bool bIsCasting = false;
	bool bAutoDirectorEnabled = false;
	bool bReplayPlaying = false;
	int32 MaxReplaysStored = 20;
};
