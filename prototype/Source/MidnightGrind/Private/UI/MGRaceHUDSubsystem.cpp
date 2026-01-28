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

	// Start notification processing
	StartNotificationTicker();

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUDSubsystem initialized"));
}

void UMGRaceHUDSubsystem::Deinitialize()
{
	// Stop notification processing
	StopNotificationTicker();
	ActiveNotifications.Empty();

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
	FMGHUDNotification Notification;
	Notification.Category = FName("PositionChange");
	Notification.bStackable = false;
	Notification.Duration = 2.0f;

	if (NewPosition < OldPosition)
	{
		// Gained position - positive feedback
		int32 PositionsGained = OldPosition - NewPosition;
		if (PositionsGained > 1)
		{
			Notification.Message = FText::FromString(FString::Printf(TEXT("+%d POSITIONS!"), PositionsGained));
		}
		else
		{
			Notification.Message = FText::FromString(FString::Printf(TEXT("P%d"), NewPosition));
		}
		Notification.Color = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f); // Green
		Notification.Priority = EMGHUDNotificationPriority::High;
		Notification.IconName = FName("ArrowUp");
	}
	else
	{
		// Lost position
		Notification.Message = FText::FromString(FString::Printf(TEXT("P%d"), NewPosition));
		Notification.Color = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f); // Orange-red
		Notification.Priority = EMGHUDNotificationPriority::Normal;
		Notification.IconName = FName("ArrowDown");
	}

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowLapNotification(int32 LapNumber, float LapTime, bool bIsBestLap, bool bIsFinalLap)
{
	// Format lap time
	int32 Minutes = FMath::FloorToInt(LapTime / 60.0f);
	float Seconds = FMath::Fmod(LapTime, 60.0f);
	FString LapTimeStr = FString::Printf(TEXT("%d:%05.2f"), Minutes, Seconds);

	FMGHUDNotification Notification;
	Notification.Category = FName("LapComplete");
	Notification.bStackable = false;

	if (bIsFinalLap)
	{
		Notification.Message = FText::FromString(TEXT("FINAL LAP!"));
		Notification.Color = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
		Notification.Priority = EMGHUDNotificationPriority::Critical;
		Notification.Duration = 3.0f;
		Notification.IconName = FName("Flag");
	}
	else if (bIsBestLap)
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("LAP %d - %s BEST LAP!"), LapNumber, *LapTimeStr));
		Notification.Color = FLinearColor(0.6f, 0.2f, 1.0f, 1.0f); // Purple
		Notification.Priority = EMGHUDNotificationPriority::High;
		Notification.Duration = 3.5f;
		Notification.IconName = FName("Trophy");
	}
	else
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("LAP %d - %s"), LapNumber, *LapTimeStr));
		Notification.Color = FLinearColor::White;
		Notification.Priority = EMGHUDNotificationPriority::Normal;
		Notification.Duration = 2.5f;
		Notification.IconName = FName("Lap");
	}

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowNearMissBonus(int32 BonusPoints)
{
	FMGHUDNotification Notification;
	Notification.Message = FText::FromString(FString::Printf(TEXT("NEAR MISS! +%d"), BonusPoints));
	Notification.Duration = 1.5f;
	Notification.Color = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f); // Cyan
	Notification.Priority = EMGHUDNotificationPriority::Low;
	Notification.Category = FName("Bonus");
	Notification.IconName = FName("NearMiss");

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowDriftScorePopup(int32 Score, float Multiplier)
{
	FMGHUDNotification Notification;
	Notification.Category = FName("DriftScore");
	Notification.bStackable = false;
	Notification.Duration = 1.5f;
	Notification.Priority = EMGHUDNotificationPriority::Low;
	Notification.IconName = FName("Drift");

	if (Multiplier > 1.0f)
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("DRIFT! %d x%.1f"), Score, Multiplier));
		// Color intensifies with multiplier
		float ColorIntensity = FMath::Min(Multiplier / 5.0f, 1.0f);
		Notification.Color = FLinearColor::LerpUsingHSV(
			FLinearColor(1.0f, 0.6f, 0.0f, 1.0f), // Orange
			FLinearColor(1.0f, 0.0f, 0.5f, 1.0f), // Pink
			ColorIntensity
		);
	}
	else
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("DRIFT! %d"), Score));
		Notification.Color = FLinearColor(1.0f, 0.6f, 0.0f, 1.0f); // Orange
	}

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowAirtimePopup(float AirtimeSeconds, int32 Score)
{
	FMGHUDNotification Notification;
	Notification.Message = FText::FromString(FString::Printf(TEXT("AIRTIME! %.2fs +%d"), AirtimeSeconds, Score));
	Notification.Duration = 2.0f;
	Notification.Color = FLinearColor(0.4f, 0.8f, 1.0f, 1.0f); // Light blue
	Notification.Priority = EMGHUDNotificationPriority::Normal;
	Notification.Category = FName("Bonus");
	Notification.IconName = FName("Airtime");

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowTrickPopup(const FText& TrickName, int32 Score)
{
	FMGHUDNotification Notification;
	Notification.Message = FText::FromString(FString::Printf(TEXT("%s +%d"), *TrickName.ToString(), Score));
	Notification.Duration = 2.0f;
	Notification.Color = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
	Notification.Priority = EMGHUDNotificationPriority::Normal;
	Notification.Category = FName("Trick");
	Notification.IconName = FName("Trick");

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowNotification(const FText& Message, float Duration, FLinearColor Color)
{
	FMGHUDNotification Notification;
	Notification.Message = Message;
	Notification.Duration = Duration;
	Notification.RemainingTime = Duration;
	Notification.Color = Color;
	Notification.Priority = EMGHUDNotificationPriority::Normal;

	ShowNotificationAdvanced(Notification);
}

int32 UMGRaceHUDSubsystem::ShowNotificationAdvanced(const FMGHUDNotification& InNotification)
{
	FMGHUDNotification Notification = InNotification;

	// Assign unique ID
	Notification.NotificationID = NextNotificationID++;
	Notification.RemainingTime = Notification.Duration;
	Notification.AnimationAlpha = 0.0f;

	// Record creation time
	if (UWorld* World = GetWorld())
	{
		Notification.CreationTime = World->GetTimeSeconds();
	}

	// Check for non-stackable duplicates
	if (!Notification.bStackable && !Notification.Category.IsNone())
	{
		// Remove existing notifications with same category
		for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
		{
			if (ActiveNotifications[i].Category == Notification.Category)
			{
				int32 RemovedID = ActiveNotifications[i].NotificationID;
				ActiveNotifications.RemoveAt(i);
				OnNotificationRemoved.Broadcast(RemovedID);
			}
		}
	}

	// Add to active list
	ActiveNotifications.Add(Notification);

	// Sort by priority
	SortNotificationsByPriority();

	// Enforce limit
	EnforceNotificationLimit();

	// Broadcast event
	OnNotificationAdded.Broadcast(Notification);

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: [%d] %s (%.1fs, Priority: %d)"),
		Notification.NotificationID,
		*Notification.Message.ToString(),
		Notification.Duration,
		static_cast<int32>(Notification.Priority));

	return Notification.NotificationID;
}

void UMGRaceHUDSubsystem::UpdateNotificationProgress(int32 NotificationID, float Progress)
{
	if (FMGHUDNotification* Notification = FindNotificationByID(NotificationID))
	{
		Notification->Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
		OnNotificationProgressUpdated.Broadcast(NotificationID, Notification->Progress);
	}
}

void UMGRaceHUDSubsystem::DismissNotification(int32 NotificationID)
{
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		if (ActiveNotifications[i].NotificationID == NotificationID)
		{
			ActiveNotifications.RemoveAt(i);
			OnNotificationRemoved.Broadcast(NotificationID);
			return;
		}
	}
}

void UMGRaceHUDSubsystem::DismissNotificationsByCategory(FName Category)
{
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		if (ActiveNotifications[i].Category == Category)
		{
			int32 RemovedID = ActiveNotifications[i].NotificationID;
			ActiveNotifications.RemoveAt(i);
			OnNotificationRemoved.Broadcast(RemovedID);
		}
	}
}

void UMGRaceHUDSubsystem::ClearAllNotifications()
{
	ActiveNotifications.Empty();
	OnAllNotificationsCleared.Broadcast();
}

void UMGRaceHUDSubsystem::SetMaxNotifications(int32 MaxCount)
{
	MaxActiveNotifications = FMath::Max(1, MaxCount);
	EnforceNotificationLimit();
}

void UMGRaceHUDSubsystem::ShowCountdown(int32 CountdownValue)
{
	FMGHUDNotification Notification;
	Notification.Category = FName("Countdown");
	Notification.bStackable = false;
	Notification.Duration = 0.9f; // Slightly less than 1 second so they don't overlap
	Notification.Priority = EMGHUDNotificationPriority::Critical;
	Notification.IconName = FName("Countdown");

	if (CountdownValue <= 0)
	{
		Notification.Message = FText::FromString(TEXT("GO!"));
		Notification.Color = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // Green
		Notification.Duration = 1.5f;
	}
	else
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("%d"), CountdownValue));
		// Color transitions from red (3) to yellow (1)
		float ColorLerp = FMath::Clamp((3.0f - CountdownValue) / 2.0f, 0.0f, 1.0f);
		Notification.Color = FLinearColor::LerpUsingHSV(
			FLinearColor(1.0f, 0.2f, 0.2f, 1.0f), // Red
			FLinearColor(1.0f, 0.9f, 0.2f, 1.0f), // Yellow
			ColorLerp
		);
	}

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::ShowRaceStart()
{
	FMGHUDNotification Notification;
	Notification.Message = FText::FromString(TEXT("GO!"));
	Notification.Duration = 1.5f;
	Notification.Color = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // Green
	Notification.Priority = EMGHUDNotificationPriority::Critical;
	Notification.Category = FName("Countdown");
	Notification.bStackable = false;
	Notification.IconName = FName("RaceStart");

	ShowNotificationAdvanced(Notification);
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
		FMGHUDNotification Notification;
		Notification.Message = FText::FromString(TEXT("WRONG WAY!"));
		Notification.Duration = 999.0f; // Persistent until dismissed
		Notification.Color = FLinearColor(1.0f, 0.1f, 0.1f, 1.0f); // Bright red
		Notification.Priority = EMGHUDNotificationPriority::Critical;
		Notification.Category = FName("WrongWay");
		Notification.bStackable = false;
		Notification.IconName = FName("Warning");

		ShowNotificationAdvanced(Notification);
	}
	else
	{
		// Dismiss the wrong way warning
		DismissNotificationsByCategory(FName("WrongWay"));
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

	// Clear any stale notifications
	ClearAllNotifications();

	SetHUDMode(EMGHUDMode::Full);

	UE_LOG(LogTemp, Log, TEXT("MGRaceHUD: Race started"));
}

void UMGRaceHUDSubsystem::OnRaceEnd(bool bPlayerWon)
{
	bRaceActive = false;

	FMGHUDNotification Notification;
	Notification.Category = FName("RaceResult");
	Notification.bStackable = false;
	Notification.Priority = EMGHUDNotificationPriority::Critical;
	Notification.Duration = 5.0f;

	if (bPlayerWon)
	{
		Notification.Message = FText::FromString(TEXT("VICTORY!"));
		Notification.Color = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
		Notification.IconName = FName("Trophy");
	}
	else
	{
		Notification.Message = FText::FromString(TEXT("RACE COMPLETE"));
		Notification.Color = FLinearColor::White;
		Notification.IconName = FName("Flag");
	}

	ShowNotificationAdvanced(Notification);
}

void UMGRaceHUDSubsystem::OnPlayerFinished(int32 FinalPosition, float FinalTime)
{
	// Format final time
	int32 Minutes = FMath::FloorToInt(FinalTime / 60.0f);
	float Seconds = FMath::Fmod(FinalTime, 60.0f);
	FString TimeStr = FString::Printf(TEXT("%d:%05.2f"), Minutes, Seconds);

	FMGHUDNotification Notification;
	Notification.Category = FName("RaceFinish");
	Notification.bStackable = false;
	Notification.Priority = EMGHUDNotificationPriority::Critical;
	Notification.Duration = 4.0f;

	// Position-based message and color
	if (FinalPosition == 1)
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("1ST PLACE - %s"), *TimeStr));
		Notification.Color = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); // Gold
		Notification.IconName = FName("Trophy");
	}
	else if (FinalPosition == 2)
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("2ND PLACE - %s"), *TimeStr));
		Notification.Color = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f); // Silver
		Notification.IconName = FName("Medal");
	}
	else if (FinalPosition == 3)
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("3RD PLACE - %s"), *TimeStr));
		Notification.Color = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f); // Bronze
		Notification.IconName = FName("Medal");
	}
	else
	{
		Notification.Message = FText::FromString(FString::Printf(TEXT("P%d - %s"), FinalPosition, *TimeStr));
		Notification.Color = FLinearColor::White;
		Notification.IconName = FName("Flag");
	}

	ShowNotificationAdvanced(Notification);
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

// ==========================================
// NOTIFICATION MANAGEMENT
// ==========================================

void UMGRaceHUDSubsystem::TickNotifications()
{
	if (ActiveNotifications.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float DeltaTime = World->GetDeltaSeconds();
	float CurrentTime = World->GetTimeSeconds();

	// Update each notification
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		FMGHUDNotification& Notification = ActiveNotifications[i];

		// Update remaining time
		Notification.RemainingTime -= DeltaTime;

		// Calculate animation alpha
		float TimeSinceCreation = CurrentTime - Notification.CreationTime;
		float TimeUntilEnd = Notification.RemainingTime;

		if (TimeSinceCreation < NotificationFadeInDuration)
		{
			// Fading in
			Notification.AnimationAlpha = TimeSinceCreation / NotificationFadeInDuration;
		}
		else if (TimeUntilEnd < NotificationFadeOutDuration)
		{
			// Fading out
			Notification.AnimationAlpha = FMath::Max(0.0f, TimeUntilEnd / NotificationFadeOutDuration);
		}
		else
		{
			// Fully visible
			Notification.AnimationAlpha = 1.0f;
		}
	}

	// Remove expired notifications
	RemoveExpiredNotifications();
}

void UMGRaceHUDSubsystem::StartNotificationTicker()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Set up timer to tick notifications at 30Hz
	World->GetTimerManager().SetTimer(
		NotificationTickHandle,
		this,
		&UMGRaceHUDSubsystem::TickNotifications,
		1.0f / 30.0f, // 30 updates per second
		true // Loop
	);
}

void UMGRaceHUDSubsystem::StopNotificationTicker()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(NotificationTickHandle);
	}
}

FMGHUDNotification* UMGRaceHUDSubsystem::FindNotificationByID(int32 NotificationID)
{
	for (FMGHUDNotification& Notification : ActiveNotifications)
	{
		if (Notification.NotificationID == NotificationID)
		{
			return &Notification;
		}
	}
	return nullptr;
}

void UMGRaceHUDSubsystem::RemoveExpiredNotifications()
{
	for (int32 i = ActiveNotifications.Num() - 1; i >= 0; --i)
	{
		if (ActiveNotifications[i].RemainingTime <= 0.0f)
		{
			int32 RemovedID = ActiveNotifications[i].NotificationID;
			ActiveNotifications.RemoveAt(i);
			OnNotificationRemoved.Broadcast(RemovedID);
		}
	}
}

void UMGRaceHUDSubsystem::EnforceNotificationLimit()
{
	// Remove oldest low-priority notifications if over limit
	while (ActiveNotifications.Num() > MaxActiveNotifications)
	{
		// Find lowest priority notification (from the back after sorting)
		int32 RemoveIndex = ActiveNotifications.Num() - 1;

		// Find the oldest among lowest priority
		EMGHUDNotificationPriority LowestPriority = EMGHUDNotificationPriority::Critical;
		float OldestCreationTime = FLT_MAX;

		for (int32 i = 0; i < ActiveNotifications.Num(); ++i)
		{
			const FMGHUDNotification& Notification = ActiveNotifications[i];

			if (Notification.Priority < LowestPriority ||
				(Notification.Priority == LowestPriority && Notification.CreationTime < OldestCreationTime))
			{
				LowestPriority = Notification.Priority;
				OldestCreationTime = Notification.CreationTime;
				RemoveIndex = i;
			}
		}

		int32 RemovedID = ActiveNotifications[RemoveIndex].NotificationID;
		ActiveNotifications.RemoveAt(RemoveIndex);
		OnNotificationRemoved.Broadcast(RemovedID);
	}
}

void UMGRaceHUDSubsystem::SortNotificationsByPriority()
{
	// Sort by priority (Critical first), then by creation time (newest first within same priority)
	ActiveNotifications.Sort([](const FMGHUDNotification& A, const FMGHUDNotification& B)
	{
		if (A.Priority != B.Priority)
		{
			return static_cast<uint8>(A.Priority) > static_cast<uint8>(B.Priority);
		}
		return A.CreationTime > B.CreationTime;
	});
}
