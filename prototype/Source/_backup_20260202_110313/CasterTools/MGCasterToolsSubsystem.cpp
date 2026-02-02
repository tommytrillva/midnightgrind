// Copyright Midnight Grind. All Rights Reserved.

#include "CasterTools/MGCasterToolsSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UMGCasterToolsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default camera config
	CurrentCameraConfig.Mode = EMGCasterCameraMode::FollowLeader;
	CurrentCameraConfig.bAutoSwitch = true;
	CurrentCameraConfig.AutoSwitchInterval = 8.0f;

	// Initialize default sectors
	for (int32 i = 0; i < 3; i++)
	{
		FMGTrackSector Sector;
		Sector.SectorIndex = i + 1;
		Sector.SectorName = FString::Printf(TEXT("Sector %d"), i + 1);
		Sector.StartDistance = i * 1000.0f;
		Sector.EndDistance = (i + 1) * 1000.0f;
		SectorData.Add(Sector);
	}

	InitializeHotkeys();

	// Start caster tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CasterTickHandle,
			this,
			&UMGCasterToolsSubsystem::OnCasterTick,
			0.1f,
			true
		);
	}
}

void UMGCasterToolsSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CasterTickHandle);
	}
	Super::Deinitialize();
}

bool UMGCasterToolsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create in game worlds with spectator/caster mode
	return true;
}

// Camera Control
void UMGCasterToolsSubsystem::SetCameraMode(EMGCasterCameraMode Mode)
{
	if (CurrentCameraConfig.Mode != Mode)
	{
		CurrentCameraConfig.Mode = Mode;
		OnCameraModeChanged.Broadcast(Mode);
	}
}

void UMGCasterToolsSubsystem::SetCameraConfig(const FMGCasterCameraConfig& Config)
{
	EMGCasterCameraMode OldMode = CurrentCameraConfig.Mode;
	CurrentCameraConfig = Config;

	if (OldMode != Config.Mode)
	{
		OnCameraModeChanged.Broadcast(Config.Mode);
	}
}

void UMGCasterToolsSubsystem::FocusOnPlayer(const FString& PlayerID)
{
	if (FocusedPlayerID != PlayerID)
	{
		FocusedPlayerID = PlayerID;
		CurrentCameraConfig.TargetPlayerID = PlayerID;
		CurrentCameraConfig.Mode = EMGCasterCameraMode::FollowPlayer;
		AutoSwitchTimer = 0.0f;

		OnFocusedPlayerChanged.Broadcast(PlayerID);
		OnCameraModeChanged.Broadcast(EMGCasterCameraMode::FollowPlayer);
	}
}

void UMGCasterToolsSubsystem::FocusOnBattle(const FMGBattleZone& Battle)
{
	if (Battle.InvolvedPlayerIDs.Num() > 0)
	{
		CurrentCameraConfig.Mode = EMGCasterCameraMode::BattleCam;
		FocusedPlayerID = Battle.InvolvedPlayerIDs[0];
		AutoSwitchTimer = 0.0f;

		OnCameraModeChanged.Broadcast(EMGCasterCameraMode::BattleCam);
	}
}

void UMGCasterToolsSubsystem::CycleToNextPlayer()
{
	if (RacerData.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = RacerData.IndexOfByPredicate([this](const FMGRacerOverlayData& Data)
	{
		return Data.PlayerID == FocusedPlayerID;
	});

	int32 NextIndex = (CurrentIndex + 1) % RacerData.Num();
	FocusOnPlayer(RacerData[NextIndex].PlayerID);
}

void UMGCasterToolsSubsystem::CycleToPreviousPlayer()
{
	if (RacerData.Num() == 0)
	{
		return;
	}

	int32 CurrentIndex = RacerData.IndexOfByPredicate([this](const FMGRacerOverlayData& Data)
	{
		return Data.PlayerID == FocusedPlayerID;
	});

	int32 PrevIndex = (CurrentIndex - 1 + RacerData.Num()) % RacerData.Num();
	FocusOnPlayer(RacerData[PrevIndex].PlayerID);
}

void UMGCasterToolsSubsystem::ToggleAutoCameraSwitch()
{
	CurrentCameraConfig.bAutoSwitch = !CurrentCameraConfig.bAutoSwitch;
}

// Overlay Management
void UMGCasterToolsSubsystem::SetOverlayPreset(EMGOverlayPreset Preset)
{
	if (CurrentOverlayPreset != Preset)
	{
		CurrentOverlayPreset = Preset;

		// Configure overlays based on preset
		switch (Preset)
		{
		case EMGOverlayPreset::None:
			bShowLeaderboard = false;
			bShowTimingTower = false;
			bShowMinimap = false;
			bShowDriverCards = false;
			bShowBattleIndicators = false;
			break;

		case EMGOverlayPreset::Minimal:
			bShowLeaderboard = false;
			bShowTimingTower = true;
			bShowMinimap = false;
			bShowDriverCards = false;
			bShowBattleIndicators = false;
			break;

		case EMGOverlayPreset::Standard:
			bShowLeaderboard = true;
			bShowTimingTower = true;
			bShowMinimap = true;
			bShowDriverCards = false;
			bShowBattleIndicators = true;
			break;

		case EMGOverlayPreset::Detailed:
			bShowLeaderboard = true;
			bShowTimingTower = true;
			bShowMinimap = true;
			bShowDriverCards = true;
			bShowBattleIndicators = true;
			break;

		case EMGOverlayPreset::Broadcast:
			bShowLeaderboard = true;
			bShowTimingTower = true;
			bShowMinimap = true;
			bShowDriverCards = true;
			bShowBattleIndicators = true;
			break;

		case EMGOverlayPreset::Analysis:
			bShowLeaderboard = true;
			bShowTimingTower = true;
			bShowMinimap = true;
			bShowDriverCards = true;
			bShowBattleIndicators = true;
			break;

		default:
			break;
		}

		OnOverlayPresetChanged.Broadcast(Preset);
	}
}

void UMGCasterToolsSubsystem::SetShowLeaderboard(bool bShow)
{
	bShowLeaderboard = bShow;
	CurrentOverlayPreset = EMGOverlayPreset::Custom;
}

void UMGCasterToolsSubsystem::SetShowTimingTower(bool bShow)
{
	bShowTimingTower = bShow;
	CurrentOverlayPreset = EMGOverlayPreset::Custom;
}

void UMGCasterToolsSubsystem::SetShowMinimap(bool bShow)
{
	bShowMinimap = bShow;
	CurrentOverlayPreset = EMGOverlayPreset::Custom;
}

void UMGCasterToolsSubsystem::SetShowDriverCards(bool bShow)
{
	bShowDriverCards = bShow;
	CurrentOverlayPreset = EMGOverlayPreset::Custom;
}

void UMGCasterToolsSubsystem::SetShowBattleIndicators(bool bShow)
{
	bShowBattleIndicators = bShow;
	CurrentOverlayPreset = EMGOverlayPreset::Custom;
}

void UMGCasterToolsSubsystem::HighlightPlayer(const FString& PlayerID, float Duration)
{
	// Would trigger UI highlight effect
	FocusOnPlayer(PlayerID);
}

void UMGCasterToolsSubsystem::ShowComparisonOverlay(const FString& PlayerA, const FString& PlayerB)
{
	// Would show side-by-side comparison UI
}

void UMGCasterToolsSubsystem::HideComparisonOverlay()
{
	// Would hide comparison UI
}

// Racer Data
FMGRacerOverlayData UMGCasterToolsSubsystem::GetRacerData(const FString& PlayerID) const
{
	const FMGRacerOverlayData* Data = RacerData.FindByPredicate([&PlayerID](const FMGRacerOverlayData& D)
	{
		return D.PlayerID == PlayerID;
	});

	return Data ? *Data : FMGRacerOverlayData();
}

// Battle Detection
FMGBattleZone UMGCasterToolsSubsystem::GetMostIntenseBattle() const
{
	FMGBattleZone MostIntense;
	float HighestIntensity = 0.0f;

	for (const FMGBattleZone& Battle : ActiveBattles)
	{
		if (Battle.Intensity > HighestIntensity)
		{
			HighestIntensity = Battle.Intensity;
			MostIntense = Battle;
		}
	}

	return MostIntense;
}

void UMGCasterToolsSubsystem::SetBattleDetectionThreshold(float GapThreshold)
{
	BattleGapThreshold = FMath::Max(0.1f, GapThreshold);
}

// Highlights and Replay
TArray<FMGHighlightMoment> UMGCasterToolsSubsystem::GetHighlightsByType(EMGHighlightType Type) const
{
	TArray<FMGHighlightMoment> Result;

	for (const FMGHighlightMoment& H : Highlights)
	{
		if (H.Type == Type)
		{
			Result.Add(H);
		}
	}

	return Result;
}

void UMGCasterToolsSubsystem::TriggerInstantReplay(const FMGHighlightMoment& Highlight)
{
	if (bPlayingInstantReplay)
	{
		return;
	}

	bPlayingInstantReplay = true;
	OnInstantReplayStarted.Broadcast();

	// Would trigger replay subsystem with specific highlight
}

void UMGCasterToolsSubsystem::TriggerInstantReplayOfLast(float Seconds)
{
	if (bPlayingInstantReplay)
	{
		return;
	}

	bPlayingInstantReplay = true;
	OnInstantReplayStarted.Broadcast();

	// Would trigger replay of last N seconds
}

void UMGCasterToolsSubsystem::StopInstantReplay()
{
	if (!bPlayingInstantReplay)
	{
		return;
	}

	bPlayingInstantReplay = false;
	OnInstantReplayEnded.Broadcast();
}

void UMGCasterToolsSubsystem::SetAutoReplayEnabled(bool bEnabled)
{
	bAutoReplayEnabled = bEnabled;
}

void UMGCasterToolsSubsystem::SetAutoReplayMinSignificance(float Significance)
{
	AutoReplayMinSignificance = FMath::Clamp(Significance, 0.0f, 1.0f);
}

void UMGCasterToolsSubsystem::BookmarkMoment(const FString& Description)
{
	FMGHighlightMoment Bookmark;
	Bookmark.Type = EMGHighlightType::Overtake; // Generic type
	Bookmark.RaceTime = RaceStats.RaceElapsedTime;
	Bookmark.Significance = 1.0f;
	Bookmark.PlayerID = FocusedPlayerID;

	RegisterHighlight(Bookmark);
}

// Graphics Effects
void UMGCasterToolsSubsystem::SetSlowMotion(float TimeScale, float Duration)
{
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(TimeScale);

		// Would set timer to reset
	}
}

void UMGCasterToolsSubsystem::ResetTimeScale()
{
	if (UWorld* World = GetWorld())
	{
		World->GetWorldSettings()->SetTimeDilation(1.0f);
	}
}

void UMGCasterToolsSubsystem::ApplyDramaticFilter(bool bEnable)
{
	// Would enable/disable post-process effects
}

void UMGCasterToolsSubsystem::SetDepthOfField(bool bEnable, float FocalDistance)
{
	// Would configure camera DoF settings
}

// Telestrator
void UMGCasterToolsSubsystem::StartDrawing()
{
	bDrawingMode = true;
}

void UMGCasterToolsSubsystem::StopDrawing()
{
	bDrawingMode = false;
}

void UMGCasterToolsSubsystem::ClearDrawings()
{
	// Would clear all telestrator drawings
}

void UMGCasterToolsSubsystem::SetDrawingColor(FLinearColor Color)
{
	DrawingColor = Color;
}

void UMGCasterToolsSubsystem::SetDrawingThickness(float Thickness)
{
	DrawingThickness = FMath::Max(1.0f, Thickness);
}

// Recording
void UMGCasterToolsSubsystem::StartBroadcastRecording()
{
	bRecordingBroadcast = true;
}

void UMGCasterToolsSubsystem::StopBroadcastRecording()
{
	bRecordingBroadcast = false;
}

// Hotkeys
void UMGCasterToolsSubsystem::SetHotkeyBinding(const FKey& Key, const FString& ActionName)
{
	FMGCasterHotkey* Existing = HotkeyBindings.FindByPredicate([&ActionName](const FMGCasterHotkey& H)
	{
		return H.ActionName == ActionName;
	});

	if (Existing)
	{
		Existing->Key = Key;
	}
	else
	{
		FMGCasterHotkey NewHotkey;
		NewHotkey.Key = Key;
		NewHotkey.ActionName = ActionName;
		HotkeyBindings.Add(NewHotkey);
	}
}

// Internal
void UMGCasterToolsSubsystem::OnCasterTick()
{
	UpdateRacerData();
	DetectBattles();
	DetectHighlights();
	ProcessAutoCamera();
}

void UMGCasterToolsSubsystem::UpdateRacerData()
{
	// Would pull from race subsystem
	// Simulated data for now
	if (RacerData.Num() == 0)
	{
		for (int32 i = 0; i < 8; i++)
		{
			FMGRacerOverlayData Data;
			Data.PlayerID = FString::Printf(TEXT("Player_%d"), i);
			Data.DisplayName = FString::Printf(TEXT("Racer%d"), FMath::RandRange(100, 999));
			Data.Position = i + 1;
			Data.CurrentSpeed = 150.0f + FMath::RandRange(-30.0f, 30.0f);
			Data.CurrentLap = 1;
			RacerData.Add(Data);
		}

		RaceStats.CurrentLeaderID = RacerData[0].PlayerID;
		PreviousLeaderID = RaceStats.CurrentLeaderID;
	}

	// Update gaps and positions
	for (int32 i = 0; i < RacerData.Num(); i++)
	{
		if (i > 0)
		{
			RacerData[i].GapToLeader = i * 0.5f + FMath::RandRange(0.0f, 0.3f);
			RacerData[i].GapToAhead = 0.5f + FMath::RandRange(0.0f, 0.2f);
		}
	}

	// Check for lead change
	if (RaceStats.CurrentLeaderID != PreviousLeaderID)
	{
		OnLeadChanged.Broadcast(RaceStats.CurrentLeaderID, PreviousLeaderID);
		RaceStats.LeadChanges++;
		PreviousLeaderID = RaceStats.CurrentLeaderID;
	}

	// Update race elapsed time
	RaceStats.RaceElapsedTime += 0.1f;
}

void UMGCasterToolsSubsystem::DetectBattles()
{
	ActiveBattles.Empty();

	// Find close groups of racers
	for (int32 i = 0; i < RacerData.Num() - 1; i++)
	{
		if (RacerData[i + 1].GapToAhead < BattleGapThreshold)
		{
			FMGBattleZone Battle;
			Battle.InvolvedPlayerIDs.Add(RacerData[i].PlayerID);
			Battle.InvolvedPlayerIDs.Add(RacerData[i + 1].PlayerID);
			Battle.bForPosition = true;
			Battle.PositionFightingFor = RacerData[i].Position;
			Battle.Intensity = 1.0f - (RacerData[i + 1].GapToAhead / BattleGapThreshold);

			// Check if more racers in this battle
			for (int32 j = i + 2; j < RacerData.Num(); j++)
			{
				if (RacerData[j].GapToAhead < BattleGapThreshold)
				{
					Battle.InvolvedPlayerIDs.Add(RacerData[j].PlayerID);
					Battle.Intensity = FMath::Min(1.0f, Battle.Intensity + 0.2f);
				}
				else
				{
					break;
				}
			}

			ActiveBattles.Add(Battle);
			OnBattleZoneDetected.Broadcast(Battle);
		}
	}
}

void UMGCasterToolsSubsystem::DetectHighlights()
{
	// Would detect various highlight-worthy moments based on game events
}

void UMGCasterToolsSubsystem::ProcessAutoCamera()
{
	if (!CurrentCameraConfig.bAutoSwitch)
	{
		return;
	}

	AutoSwitchTimer += 0.1f;

	if (AutoSwitchTimer >= CurrentCameraConfig.AutoSwitchInterval)
	{
		AutoSwitchTimer = 0.0f;

		// Priority: battles > leader > random interesting racer
		if (CurrentCameraConfig.bPrioritizeBattles && ActiveBattles.Num() > 0)
		{
			FMGBattleZone MostIntense = GetMostIntenseBattle();
			if (MostIntense.InvolvedPlayerIDs.Num() > 0)
			{
				FocusOnBattle(MostIntense);
				return;
			}
		}

		// Cycle through interesting positions
		CycleToNextPlayer();
	}
}

void UMGCasterToolsSubsystem::RegisterHighlight(const FMGHighlightMoment& Highlight)
{
	Highlights.Add(Highlight);
	OnHighlightDetected.Broadcast(Highlight);

	// Trigger auto replay if significant enough
	if (bAutoReplayEnabled && Highlight.Significance >= AutoReplayMinSignificance)
	{
		TriggerInstantReplay(Highlight);
	}
}

void UMGCasterToolsSubsystem::InitializeHotkeys()
{
	HotkeyBindings.Empty();

	auto AddHotkey = [this](const FKey& Key, const FString& Action, const FString& Desc)
	{
		FMGCasterHotkey H;
		H.Key = Key;
		H.ActionName = Action;
		H.Description = Desc;
		HotkeyBindings.Add(H);
	};

	AddHotkey(EKeys::One, TEXT("FocusP1"), TEXT("Focus on 1st place"));
	AddHotkey(EKeys::Two, TEXT("FocusP2"), TEXT("Focus on 2nd place"));
	AddHotkey(EKeys::Three, TEXT("FocusP3"), TEXT("Focus on 3rd place"));
	AddHotkey(EKeys::Tab, TEXT("CyclePlayer"), TEXT("Cycle to next player"));
	AddHotkey(EKeys::F1, TEXT("OverlayMinimal"), TEXT("Minimal overlay"));
	AddHotkey(EKeys::F2, TEXT("OverlayStandard"), TEXT("Standard overlay"));
	AddHotkey(EKeys::F3, TEXT("OverlayDetailed"), TEXT("Detailed overlay"));
	AddHotkey(EKeys::R, TEXT("InstantReplay"), TEXT("Trigger instant replay"));
	AddHotkey(EKeys::B, TEXT("FocusBattle"), TEXT("Focus on battle"));
	AddHotkey(EKeys::L, TEXT("FocusLeader"), TEXT("Focus on leader"));
	AddHotkey(EKeys::T, TEXT("ToggleTelestrator"), TEXT("Toggle telestrator"));
	AddHotkey(EKeys::SpaceBar, TEXT("ToggleAutoCamera"), TEXT("Toggle auto camera"));
}
