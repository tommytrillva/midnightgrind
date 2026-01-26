// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MGCasterToolsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCasterCameraMode : uint8
{
	FollowLeader,
	FollowPlayer,
	TrackOverview,
	HelicopterCam,
	OrbitCam,
	FreeCam,
	BattleCam,
	OnboardCam,
	ReplayCam,
	PitLaneCam,
	StartFinishCam
};

UENUM(BlueprintType)
enum class EMGOverlayPreset : uint8
{
	None,
	Minimal,
	Standard,
	Detailed,
	Broadcast,
	Analysis,
	Custom
};

UENUM(BlueprintType)
enum class EMGHighlightType : uint8
{
	Overtake,
	NearMiss,
	Crash,
	DriftCombo,
	NitroBoost,
	FastestLap,
	LeadChange,
	PhotoFinish,
	PerfectCorner,
	BigJump,
	PoliceEscape,
	Spinout
};

USTRUCT(BlueprintType)
struct FMGCasterCameraConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCasterCameraMode Mode = EMGCasterCameraMode::FollowLeader;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FollowDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FollowHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothingSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoSwitch = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoSwitchInterval = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrioritizeBattles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowSpeedLines = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShakeIntensity = 0.5f;
};

USTRUCT(BlueprintType)
struct FMGRacerOverlayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PositionChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToLeader = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GapToAhead = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NitroRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TeamColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> DriverPhoto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OvertakesMade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OvertakesLost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInPit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRetired = false;
};

USTRUCT(BlueprintType)
struct FMGBattleZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedPlayerIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CenterLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForPosition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PositionFightingFor = 0;
};

USTRUCT(BlueprintType)
struct FMGHighlightMoment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHighlightType Type = EMGHighlightType::Overtake;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Significance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReplayStartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReplayDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoReplayTriggered = false;
};

USTRUCT(BlueprintType)
struct FMGTrackSector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SectorIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SectorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EndDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FastestPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGRaceStatsSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalOvertakes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LeadChanges = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentLeaderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeaderGapToSecond = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString FastestLapHolderID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FastestLapLapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RaceElapsedTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLaps = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedReached = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TopSpeedPlayerID;
};

USTRUCT(BlueprintType)
struct FMGCasterHotkey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraModeChanged, EMGCasterCameraMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusedPlayerChanged, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighlightDetected, const FMGHighlightMoment&, Highlight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleZoneDetected, const FMGBattleZone&, Battle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeadChanged, const FString&, NewLeaderID, const FString&, OldLeaderID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFastestLapSet, const FMGRacerOverlayData&, RacerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInstantReplayStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInstantReplayEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverlayPresetChanged, EMGOverlayPreset, NewPreset);

UCLASS()
class MIDNIGHTGRIND_API UMGCasterToolsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Camera Control
	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void SetCameraMode(EMGCasterCameraMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void SetCameraConfig(const FMGCasterCameraConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Caster|Camera")
	EMGCasterCameraMode GetCurrentCameraMode() const { return CurrentCameraConfig.Mode; }

	UFUNCTION(BlueprintPure, Category = "Caster|Camera")
	FMGCasterCameraConfig GetCameraConfig() const { return CurrentCameraConfig; }

	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void FocusOnPlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void FocusOnBattle(const FMGBattleZone& Battle);

	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void CycleToNextPlayer();

	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void CycleToPreviousPlayer();

	UFUNCTION(BlueprintCallable, Category = "Caster|Camera")
	void ToggleAutoCameraSwitch();

	UFUNCTION(BlueprintPure, Category = "Caster|Camera")
	FString GetFocusedPlayerID() const { return FocusedPlayerID; }

	// Overlay Management
	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetOverlayPreset(EMGOverlayPreset Preset);

	UFUNCTION(BlueprintPure, Category = "Caster|Overlay")
	EMGOverlayPreset GetCurrentOverlayPreset() const { return CurrentOverlayPreset; }

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowLeaderboard(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowTimingTower(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowMinimap(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowDriverCards(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void SetShowBattleIndicators(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void HighlightPlayer(const FString& PlayerID, float Duration = 3.0f);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void ShowComparisonOverlay(const FString& PlayerA, const FString& PlayerB);

	UFUNCTION(BlueprintCallable, Category = "Caster|Overlay")
	void HideComparisonOverlay();

	// Racer Data
	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	TArray<FMGRacerOverlayData> GetAllRacerData() const { return RacerData; }

	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	FMGRacerOverlayData GetRacerData(const FString& PlayerID) const;

	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	FMGRaceStatsSummary GetRaceStats() const { return RaceStats; }

	UFUNCTION(BlueprintPure, Category = "Caster|Data")
	TArray<FMGTrackSector> GetSectorData() const { return SectorData; }

	// Battle Detection
	UFUNCTION(BlueprintPure, Category = "Caster|Battle")
	TArray<FMGBattleZone> GetActiveBattles() const { return ActiveBattles; }

	UFUNCTION(BlueprintPure, Category = "Caster|Battle")
	FMGBattleZone GetMostIntenseBattle() const;

	UFUNCTION(BlueprintCallable, Category = "Caster|Battle")
	void SetBattleDetectionThreshold(float GapThreshold);

	// Highlights and Replay
	UFUNCTION(BlueprintPure, Category = "Caster|Highlights")
	TArray<FMGHighlightMoment> GetHighlights() const { return Highlights; }

	UFUNCTION(BlueprintPure, Category = "Caster|Highlights")
	TArray<FMGHighlightMoment> GetHighlightsByType(EMGHighlightType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void TriggerInstantReplay(const FMGHighlightMoment& Highlight);

	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void TriggerInstantReplayOfLast(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void StopInstantReplay();

	UFUNCTION(BlueprintPure, Category = "Caster|Highlights")
	bool IsPlayingInstantReplay() const { return bPlayingInstantReplay; }

	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void SetAutoReplayEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void SetAutoReplayMinSignificance(float Significance);

	UFUNCTION(BlueprintCallable, Category = "Caster|Highlights")
	void BookmarkMoment(const FString& Description);

	// Graphics Effects
	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void SetSlowMotion(float TimeScale, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void ResetTimeScale();

	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void ApplyDramaticFilter(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Caster|Graphics")
	void SetDepthOfField(bool bEnable, float FocalDistance = 0.0f);

	// Telestrator / Drawing
	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void StartDrawing();

	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void StopDrawing();

	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void ClearDrawings();

	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void SetDrawingColor(FLinearColor Color);

	UFUNCTION(BlueprintCallable, Category = "Caster|Telestrator")
	void SetDrawingThickness(float Thickness);

	UFUNCTION(BlueprintPure, Category = "Caster|Telestrator")
	bool IsDrawingMode() const { return bDrawingMode; }

	// Hotkeys
	UFUNCTION(BlueprintPure, Category = "Caster|Hotkeys")
	TArray<FMGCasterHotkey> GetHotkeyBindings() const { return HotkeyBindings; }

	UFUNCTION(BlueprintCallable, Category = "Caster|Hotkeys")
	void SetHotkeyBinding(const FKey& Key, const FString& ActionName);

	// Recording
	UFUNCTION(BlueprintCallable, Category = "Caster|Recording")
	void StartBroadcastRecording();

	UFUNCTION(BlueprintCallable, Category = "Caster|Recording")
	void StopBroadcastRecording();

	UFUNCTION(BlueprintPure, Category = "Caster|Recording")
	bool IsRecordingBroadcast() const { return bRecordingBroadcast; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnCameraModeChanged OnCameraModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnFocusedPlayerChanged OnFocusedPlayerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnHighlightDetected OnHighlightDetected;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnBattleZoneDetected OnBattleZoneDetected;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnLeadChanged OnLeadChanged;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnFastestLapSet OnFastestLapSet;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnInstantReplayStarted OnInstantReplayStarted;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnInstantReplayEnded OnInstantReplayEnded;

	UPROPERTY(BlueprintAssignable, Category = "Caster|Events")
	FOnOverlayPresetChanged OnOverlayPresetChanged;

protected:
	void OnCasterTick();
	void UpdateRacerData();
	void DetectBattles();
	void DetectHighlights();
	void ProcessAutoCamera();
	void RegisterHighlight(const FMGHighlightMoment& Highlight);
	void InitializeHotkeys();

	UPROPERTY()
	FMGCasterCameraConfig CurrentCameraConfig;

	UPROPERTY()
	FString FocusedPlayerID;

	UPROPERTY()
	EMGOverlayPreset CurrentOverlayPreset = EMGOverlayPreset::Standard;

	UPROPERTY()
	TArray<FMGRacerOverlayData> RacerData;

	UPROPERTY()
	FMGRaceStatsSummary RaceStats;

	UPROPERTY()
	TArray<FMGTrackSector> SectorData;

	UPROPERTY()
	TArray<FMGBattleZone> ActiveBattles;

	UPROPERTY()
	TArray<FMGHighlightMoment> Highlights;

	UPROPERTY()
	TArray<FMGCasterHotkey> HotkeyBindings;

	UPROPERTY()
	bool bShowLeaderboard = true;

	UPROPERTY()
	bool bShowTimingTower = true;

	UPROPERTY()
	bool bShowMinimap = true;

	UPROPERTY()
	bool bShowDriverCards = false;

	UPROPERTY()
	bool bShowBattleIndicators = true;

	UPROPERTY()
	bool bPlayingInstantReplay = false;

	UPROPERTY()
	bool bAutoReplayEnabled = true;

	UPROPERTY()
	float AutoReplayMinSignificance = 0.7f;

	UPROPERTY()
	bool bDrawingMode = false;

	UPROPERTY()
	FLinearColor DrawingColor = FLinearColor::Yellow;

	UPROPERTY()
	float DrawingThickness = 3.0f;

	UPROPERTY()
	bool bRecordingBroadcast = false;

	UPROPERTY()
	float BattleGapThreshold = 0.5f;

	UPROPERTY()
	float AutoSwitchTimer = 0.0f;

	UPROPERTY()
	FString PreviousLeaderID;

	FTimerHandle CasterTickHandle;
};
