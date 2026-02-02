// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGRaceHUDWidget.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Engine/World.h"

void UMGRaceHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Cache HUD subsystem
	if (UWorld* World = GetWorld())
	{
		HUDSubsystem = World->GetSubsystem<UMGRaceHUDSubsystem>();
	}
}

void UMGRaceHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update smooth values
	UpdateSmoothValues(InDeltaTime);

	// Refresh display with latest data
	if (HUDSubsystem.IsValid())
	{
		CurrentTelemetry = HUDSubsystem->GetVehicleTelemetry();
		CurrentRaceStatus = HUDSubsystem->GetRaceStatus();
		CurrentDriftData = HUDSubsystem->GetDriftScoreData();
	}
}

// ==========================================
// UPDATE FUNCTIONS
// ==========================================

void UMGRaceHUDWidget::RefreshDisplay()
{
	if (!HUDSubsystem.IsValid())
	{
		return;
	}

	bool bUseMPH = HUDSubsystem->IsUsingMPH();

	UpdateSpeedDisplay(CurrentTelemetry.SpeedKPH, CurrentTelemetry.SpeedMPH, bUseMPH);
	UpdateTachometer(CurrentTelemetry.RPM, CurrentTelemetry.MaxRPM, CurrentTelemetry.CurrentGear, CurrentTelemetry.TotalGears);
	UpdateNOSGauge(CurrentTelemetry.NOSAmount, CurrentTelemetry.bNOSActive);
	UpdatePositionDisplay(CurrentRaceStatus.CurrentPosition, CurrentRaceStatus.TotalRacers);
	UpdateLapDisplay(CurrentRaceStatus.CurrentLap, CurrentRaceStatus.TotalLaps, CurrentRaceStatus.bFinalLap);
	UpdateTimeDisplay(CurrentRaceStatus.CurrentLapTime, CurrentRaceStatus.BestLapTime, CurrentRaceStatus.TotalRaceTime);
	UpdateGapDisplay(CurrentRaceStatus.GapToLeader, CurrentRaceStatus.GapToNext);
	UpdateDriftDisplay(CurrentDriftData.CurrentDriftScore, CurrentDriftData.DriftMultiplier, CurrentDriftData.DriftChainCount, CurrentDriftData.ChainTimeRemaining);
}

void UMGRaceHUDWidget::UpdateSpeedDisplay_Implementation(float SpeedKPH, float SpeedMPH, bool bUseMPH)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::UpdateTachometer_Implementation(float RPM, float MaxRPM, int32 Gear, int32 TotalGears)
{
	// Check shift indicator
	float RPMPercent = MaxRPM > 0.0f ? RPM / MaxRPM : 0.0f;

	if (RPMPercent >= ShiftIndicatorThreshold && !bShiftIndicatorActive)
	{
		bShiftIndicatorActive = true;
		PlayShiftIndicator();
	}
	else if (RPMPercent < ShiftIndicatorThreshold * 0.9f)
	{
		bShiftIndicatorActive = false;
	}

	// Check redline
	if (RPMPercent >= RedlineThreshold && !bRedlineActive)
	{
		bRedlineActive = true;
		PlayRedlineWarning();
	}
	else if (RPMPercent < RedlineThreshold * 0.95f)
	{
		bRedlineActive = false;
	}
}

void UMGRaceHUDWidget::UpdateNOSGauge_Implementation(float NOSAmount, bool bNOSActive)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::UpdatePositionDisplay_Implementation(int32 Position, int32 TotalRacers)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::UpdateLapDisplay_Implementation(int32 CurrentLap, int32 TotalLaps, bool bFinalLap)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::UpdateTimeDisplay_Implementation(float CurrentLapTime, float BestLapTime, float TotalTime)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::UpdateGapDisplay_Implementation(float GapToLeader, float GapToNext)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::UpdateDriftDisplay_Implementation(int32 CurrentScore, float Multiplier, int32 ChainCount, float ChainTimeRemaining)
{
	// Default implementation - override in Blueprint
}

// ==========================================
// ELEMENT VISIBILITY
// ==========================================

void UMGRaceHUDWidget::SetElementVisible_Implementation(FName ElementName, bool bVisible)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::SetHUDOpacity_Implementation(float Opacity)
{
	SetRenderOpacity(Opacity);
}

void UMGRaceHUDWidget::SetHUDScale_Implementation(float Scale)
{
	SetRenderScale(FVector2D(Scale, Scale));
}

// ==========================================
// ANIMATIONS
// ==========================================

void UMGRaceHUDWidget::PlayPositionChangeAnimation_Implementation(int32 OldPosition, int32 NewPosition)
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::PlayShiftIndicator_Implementation()
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::PlayRedlineWarning_Implementation()
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::PlayNOSActivationEffect_Implementation()
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::PlayFinalLapEffect_Implementation()
{
	// Default implementation - override in Blueprint
}

void UMGRaceHUDWidget::PlayBestLapEffect_Implementation()
{
	// Default implementation - override in Blueprint
}

// ==========================================
// INTERNAL
// ==========================================

UMGRaceHUDSubsystem* UMGRaceHUDWidget::GetHUDSubsystem() const
{
	return HUDSubsystem.Get();
}

FText UMGRaceHUDWidget::FormatTime(float TimeInSeconds) const
{
	if (TimeInSeconds < 0.0f)
	{
		return FText::FromString(TEXT("--:--.---"));
	}

	int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeInSeconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(Seconds);
	int32 Milliseconds = FMath::FloorToInt((Seconds - WholeSeconds) * 1000.0f);

	return FText::FromString(FString::Printf(TEXT("%d:%02d.%03d"), Minutes, WholeSeconds, Milliseconds));
}

FText UMGRaceHUDWidget::FormatGapTime(float GapInSeconds) const
{
	if (FMath::Abs(GapInSeconds) < 0.001f)
	{
		return FText::FromString(TEXT("0.000"));
	}

	FString Sign = GapInSeconds > 0.0f ? TEXT("+") : TEXT("-");
	float AbsGap = FMath::Abs(GapInSeconds);

	return FText::FromString(FString::Printf(TEXT("%s%.3f"), *Sign, AbsGap));
}

FLinearColor UMGRaceHUDWidget::GetPositionColor(int32 Position) const
{
	switch (Position)
	{
	case 1:
		return FLinearColor(1.0f, 0.843f, 0.0f, 1.0f); // Gold
	case 2:
		return FLinearColor(0.753f, 0.753f, 0.753f, 1.0f); // Silver
	case 3:
		return FLinearColor(0.804f, 0.498f, 0.196f, 1.0f); // Bronze
	default:
		return FLinearColor::White;
	}
}

FLinearColor UMGRaceHUDWidget::GetGapColor(float Gap) const
{
	if (Gap < -0.001f)
	{
		// Ahead - green
		return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (Gap > 0.001f)
	{
		// Behind - red
		return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		// Even - white
		return FLinearColor::White;
	}
}

void UMGRaceHUDWidget::UpdateSmoothValues(float DeltaTime)
{
	// Smooth speed display
	float TargetSpeed = HUDSubsystem.IsValid() && HUDSubsystem->IsUsingMPH()
		? CurrentTelemetry.SpeedMPH
		: CurrentTelemetry.SpeedKPH;

	DisplayedSpeed = FMath::FInterpTo(DisplayedSpeed, TargetSpeed, DeltaTime, SpeedInterpRate);

	// Smooth RPM display
	DisplayedRPM = FMath::FInterpTo(DisplayedRPM, CurrentTelemetry.RPM, DeltaTime, RPMInterpRate);

	// Update element animations
	for (auto& Pair : ElementAnimations)
	{
		FMGHUDAnimationState& State = Pair.Value;
		State.CurrentAlpha = FMath::FInterpTo(State.CurrentAlpha, State.TargetAlpha, DeltaTime, 10.0f);
		State.CurrentScale = FMath::FInterpTo(State.CurrentScale, State.TargetScale, DeltaTime, 10.0f);
		State.CurrentOffset = FMath::Vector2DInterpTo(State.CurrentOffset, State.TargetOffset, DeltaTime, 10.0f);
	}
}
