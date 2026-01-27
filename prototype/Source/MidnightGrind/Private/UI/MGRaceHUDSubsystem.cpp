// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGRaceHUDSubsystem.h"
#include "Engine/World.h"

void UMGRaceHUDSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default element visibility
	ElementVisibility.Add(FName("Speedometer"), true);
	ElementVisibility.Add(FName("Tachometer"), true);
	ElementVisibility.Add(FName("GearIndicator"), true);
	ElementVisibility.Add(FName("NOSGauge"), true);
	ElementVisibility.Add(FName("Position"), true);
	ElementVisibility.Add(FName("LapCounter"), true);
	ElementVisibility.Add(FName("Timer"), true);
	ElementVisibility.Add(FName("Minimap"), true);
	ElementVisibility.Add(FName("GapDisplay"), true);
	ElementVisibility.Add(FName("DriftScore"), true);

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUDSubsystem initialized"));
}

void UMGRaceHUDSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UMGRaceHUDSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// ==========================================
// HUD CONTROL
// ==========================================

void UMGRaceHUDSubsystem::SetHUDMode(EMGHUDMode Mode)
{
	if (CurrentHUDMode == Mode)
	{
		return;
	}

	PreviousHUDMode = CurrentHUDMode;
	CurrentHUDMode = Mode;

	ApplyHUDMode(Mode);

	OnHUDModeChanged.Broadcast(Mode);

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Mode changed to %d"), static_cast<int32>(Mode));
}

void UMGRaceHUDSubsystem::SetElementVisibility(FName ElementName, bool bVisible)
{
	ElementVisibility.Add(ElementName, bVisible);
	RefreshHUD();
}

void UMGRaceHUDSubsystem::ToggleHUD()
{
	if (CurrentHUDMode == EMGHUDMode::Hidden)
	{
		SetHUDMode(PreviousHUDMode);
	}
	else
	{
		SetHUDMode(EMGHUDMode::Hidden);
	}
}

// ==========================================
// DATA UPDATES
// ==========================================

void UMGRaceHUDSubsystem::UpdateVehicleTelemetry(const FMGVehicleTelemetry& Telemetry)
{
	CurrentTelemetry = Telemetry;

	// Convert speed if needed
	if (bDisplayMPH)
	{
		CurrentTelemetry.SpeedMPH = Telemetry.SpeedKPH * 0.621371f;
	}
	else
	{
		CurrentTelemetry.SpeedMPH = Telemetry.SpeedKPH * 0.621371f;
	}
}

void UMGRaceHUDSubsystem::UpdateRaceStatus(const FMGRaceStatus& Status)
{
	int32 OldPosition = CurrentRaceStatus.CurrentPosition;
	int32 OldLap = CurrentRaceStatus.CurrentLap;

	CurrentRaceStatus = Status;

	// Check for position change
	if (bRaceActive && OldPosition != Status.CurrentPosition && OldPosition > 0)
	{
		ShowPositionChange(OldPosition, Status.CurrentPosition);
		OnPositionChanged.Broadcast(OldPosition, Status.CurrentPosition);
	}

	// Check for lap change
	if (bRaceActive && OldLap != Status.CurrentLap && Status.CurrentLap > OldLap)
	{
		OnLapCompleted.Broadcast(Status.CurrentLap - 1);
	}
}

void UMGRaceHUDSubsystem::UpdateDriftScore(const FMGDriftScoreData& DriftData)
{
	CurrentDriftData = DriftData;
}

// ==========================================
// NOTIFICATIONS
// ==========================================

void UMGRaceHUDSubsystem::ShowPositionChange(int32 OldPosition, int32 NewPosition)
{
	// This would trigger the overlay widget to show position change animation
	// Implementation depends on widget setup

	if (NewPosition < OldPosition)
	{
		// Gained position - positive feedback
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Position gained %d -> %d"), OldPosition, NewPosition);
	}
	else
	{
		// Lost position
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Position lost %d -> %d"), OldPosition, NewPosition);
	}
}

void UMGRaceHUDSubsystem::ShowLapNotification(int32 LapNumber, float LapTime, bool bIsBestLap, bool bIsFinalLap)
{
	// Format lap time
	int32 Minutes = FMath::FloorToInt(LapTime / 60.0f);
	float Seconds = FMath::Fmod(LapTime, 60.0f);

	FString LapTimeStr = FString::Printf(TEXT("%d:%05.2f"), Minutes, Seconds);

	if (bIsFinalLap)
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: FINAL LAP!"));
	}
	else if (bIsBestLap)
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Lap %d complete - %s (BEST LAP!)"), LapNumber, *LapTimeStr);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Lap %d complete - %s"), LapNumber, *LapTimeStr);
	}
}

void UMGRaceHUDSubsystem::ShowNearMissBonus(int32 BonusPoints)
{
	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Near Miss! +%d"), BonusPoints);
}

void UMGRaceHUDSubsystem::ShowDriftScorePopup(int32 Score, float Multiplier)
{
	if (Multiplier > 1.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Drift! %d x%.1f"), Score, Multiplier);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Drift! %d"), Score);
	}
}

void UMGRaceHUDSubsystem::ShowNotification(const FText& Message, float Duration, FLinearColor Color)
{
	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: %s"), *Message.ToString());
}

void UMGRaceHUDSubsystem::ShowCountdown(int32 CountdownValue)
{
	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Countdown %d"), CountdownValue);
}

void UMGRaceHUDSubsystem::ShowRaceStart()
{
	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: GO!"));
}

void UMGRaceHUDSubsystem::ShowWrongWayWarning(bool bShow)
{
	if (bShowingWrongWay == bShow)
	{
		return;
	}

	bShowingWrongWay = bShow;

	if (bShow)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGRaceHUD: WRONG WAY!"));
	}
}

// ==========================================
// DAMAGE FEEDBACK
// ==========================================

void UMGRaceHUDSubsystem::UpdateDamageState(const FMGDamageHUDData& DamageData)
{
	bool bWasLimping = CurrentDamageData.bIsLimping;
	bool bWasOnFire = CurrentDamageData.bEngineOnFire;
	bool bWasSmoking = CurrentDamageData.bEngineSmoking;

	CurrentDamageData = DamageData;

	// Calculate target vignette intensity based on damage
	TargetVignetteIntensity = 0.0f;
	if (DamageData.bEngineOnFire)
	{
		TargetVignetteIntensity = 0.6f;
	}
	else if (DamageData.bIsLimping)
	{
		TargetVignetteIntensity = 0.4f;
	}
	else if (DamageData.bEngineSmoking)
	{
		TargetVignetteIntensity = 0.2f;
	}
	else if (DamageData.OverallDamage > 0.5f)
	{
		TargetVignetteIntensity = DamageData.OverallDamage * 0.3f;
	}

	// Show warnings for state transitions
	if (DamageData.bEngineOnFire && !bWasOnFire)
	{
		ShowDamageWarning(FText::FromString(TEXT("ENGINE FIRE!")), 3.0f);
	}
	else if (DamageData.bIsLimping && !bWasLimping)
	{
		ShowDamageWarning(FText::FromString(TEXT("CRITICAL DAMAGE!")), 2.5f);
	}
	else if (DamageData.bEngineSmoking && !bWasSmoking)
	{
		ShowDamageWarning(FText::FromString(TEXT("ENGINE DAMAGE")), 2.0f);
	}

	// Broadcast state change
	OnDamageStateChanged.Broadcast(DamageData);
}

void UMGRaceHUDSubsystem::TriggerImpactFeedback(const FMGImpactFeedback& Feedback)
{
	// Set impact flash
	ImpactFlashAlpha = FMath::Clamp(Feedback.Intensity, 0.0f, 1.0f);

	// Broadcast for widgets to handle
	OnImpactReceived.Broadcast(Feedback);

	if (Feedback.bShowVignette)
	{
		// Temporarily boost vignette
		DamageVignetteIntensity = FMath::Max(DamageVignetteIntensity, Feedback.Intensity * 0.8f);
	}

	if (Feedback.bTriggerShake && Feedback.Intensity > 0.3f)
	{
		// Camera shake would be triggered here via player camera manager
		// The actual shake is handled by the player controller
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Impact shake triggered (intensity: %.2f)"), Feedback.Intensity);
	}

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Impact feedback (intensity: %.2f, direction: %.2f, %.2f)"),
		Feedback.Intensity, Feedback.Direction.X, Feedback.Direction.Y);
}

void UMGRaceHUDSubsystem::ShowDamageWarning(const FText& Message, float Duration)
{
	// Show warning notification with red/orange color
	FLinearColor WarningColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);
	ShowNotification(Message, Duration, WarningColor);
}

void UMGRaceHUDSubsystem::SetDamageVignetteIntensity(float Intensity)
{
	DamageVignetteIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

// ==========================================
// RACE EVENTS
// ==========================================

void UMGRaceHUDSubsystem::OnRaceStart()
{
	bRaceActive = true;

	// Reset race status
	CurrentRaceStatus = FMGRaceStatus();
	CurrentDriftData = FMGDriftScoreData();

	SetHUDMode(EMGHUDMode::Full);

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Race started"));
}

void UMGRaceHUDSubsystem::OnRaceEnd(bool bPlayerWon)
{
	bRaceActive = false;

	if (bPlayerWon)
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Race ended - VICTORY!"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Race ended"));
	}
}

void UMGRaceHUDSubsystem::OnPlayerFinished(int32 FinalPosition, float FinalTime)
{
	// Format final time
	int32 Minutes = FMath::FloorToInt(FinalTime / 60.0f);
	float Seconds = FMath::Fmod(FinalTime, 60.0f);

	FString TimeStr = FString::Printf(TEXT("%d:%05.2f"), Minutes, Seconds);

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Finished P%d - %s"), FinalPosition, *TimeStr);
}

void UMGRaceHUDSubsystem::OnEnterPhotoMode()
{
	SetHUDMode(EMGHUDMode::PhotoMode);
}

void UMGRaceHUDSubsystem::OnExitPhotoMode()
{
	SetHUDMode(PreviousHUDMode);
}

// ==========================================
// MINIMAP
// ==========================================

void UMGRaceHUDSubsystem::UpdateMinimapPlayerPosition(FVector2D Position, float Rotation)
{
	// Forward to minimap widget
}

void UMGRaceHUDSubsystem::UpdateMinimapOpponentPosition(int32 OpponentIndex, FVector2D Position, float Rotation)
{
	// Forward to minimap widget
}

void UMGRaceHUDSubsystem::SetMinimapTrackData(UTexture2D* TrackTexture, FVector2D TrackBoundsMin, FVector2D TrackBoundsMax)
{
	// Forward to minimap widget
}

void UMGRaceHUDSubsystem::SetMinimapZoom(float ZoomLevel)
{
	// Forward to minimap widget
}

// ==========================================
// SETTINGS
// ==========================================

void UMGRaceHUDSubsystem::SetSpeedUnitMPH(bool bUseMPH)
{
	bDisplayMPH = bUseMPH;
}

void UMGRaceHUDSubsystem::SetTachometerStyle(int32 StyleIndex)
{
	TachometerStyle = StyleIndex;
	RefreshHUD();
}

void UMGRaceHUDSubsystem::SetHUDScale(float Scale)
{
	HUDScale = FMath::Clamp(Scale, 0.5f, 2.0f);
	RefreshHUD();
}

void UMGRaceHUDSubsystem::SetHUDOpacity(float Opacity)
{
	HUDOpacity = FMath::Clamp(Opacity, 0.0f, 1.0f);
	RefreshHUD();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGRaceHUDSubsystem::RefreshHUD()
{
	// Would refresh all widget bindings
}

void UMGRaceHUDSubsystem::ApplyHUDMode(EMGHUDMode Mode)
{
	switch (Mode)
	{
	case EMGHUDMode::Full:
		// Show all elements
		for (auto& Pair : ElementVisibility)
		{
			Pair.Value = true;
		}
		break;

	case EMGHUDMode::Minimal:
		// Show only essential elements
		for (auto& Pair : ElementVisibility)
		{
			Pair.Value = false;
		}
		ElementVisibility[FName("Speedometer")] = true;
		ElementVisibility[FName("Position")] = true;
		ElementVisibility[FName("LapCounter")] = true;
		break;

	case EMGHUDMode::Hidden:
		// Hide everything
		for (auto& Pair : ElementVisibility)
		{
			Pair.Value = false;
		}
		break;

	case EMGHUDMode::PhotoMode:
		// Hide racing elements, show photo controls
		for (auto& Pair : ElementVisibility)
		{
			Pair.Value = false;
		}
		break;

	case EMGHUDMode::Replay:
		// Show replay controls, minimal racing info
		for (auto& Pair : ElementVisibility)
		{
			Pair.Value = false;
		}
		ElementVisibility[FName("Timer")] = true;
		break;
	}

	RefreshHUD();
}
