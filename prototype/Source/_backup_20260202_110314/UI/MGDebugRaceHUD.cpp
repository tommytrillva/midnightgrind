// Copyright Midnight Grind. All Rights Reserved.
// Stage 54: Debug Race HUD - MVP Minimal Display

#include "UI/MGDebugRaceHUD.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Kismet/GameplayStatics.h"

void UMGDebugRaceHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Get HUD subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		HUDSubsystem = GI->GetSubsystem<UMGRaceHUDSubsystem>();
	}

	// Initialize displays
	SetSpeed(0.0f);
	SetPosition(1, 1);
	SetLap(0, 3);
	SetLapTime(0.0f);
	HideCountdown();
}

void UMGDebugRaceHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Auto-update from subsystem
	UpdateFromSubsystem();
}

void UMGDebugRaceHUD::SetSpeed(float SpeedMPH)
{
	if (SpeedText)
	{
		SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%d MPH"), FMath::RoundToInt(SpeedMPH))));
	}
}

void UMGDebugRaceHUD::SetPosition(int32 Position, int32 TotalRacers)
{
	if (PositionText)
	{
		FString Suffix = GetPositionSuffix(Position);
		PositionText->SetText(FText::FromString(FString::Printf(TEXT("%d%s / %d"), Position, *Suffix, TotalRacers)));
	}
}

void UMGDebugRaceHUD::SetLap(int32 CurrentLap, int32 TotalLaps)
{
	if (LapText)
	{
		// Lap 0 means pre-race
		if (CurrentLap <= 0)
		{
			LapText->SetText(FText::FromString(FString::Printf(TEXT("Lap -- / %d"), TotalLaps)));
		}
		else
		{
			LapText->SetText(FText::FromString(FString::Printf(TEXT("Lap %d / %d"), CurrentLap, TotalLaps)));
		}
	}
}

void UMGDebugRaceHUD::SetLapTime(float LapTimeSeconds)
{
	if (TimeText)
	{
		TimeText->SetText(FText::FromString(FormatTime(LapTimeSeconds)));
	}
}

void UMGDebugRaceHUD::ShowCountdown(int32 Seconds)
{
	if (CountdownText)
	{
		CountdownText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Seconds)));
		CountdownText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UMGDebugRaceHUD::ShowGO()
{
	if (CountdownText)
	{
		CountdownText->SetText(FText::FromString(TEXT("GO!")));
		CountdownText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UMGDebugRaceHUD::HideCountdown()
{
	if (CountdownText)
	{
		CountdownText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMGDebugRaceHUD::ShowFinished(int32 FinalPosition)
{
	if (CountdownText)
	{
		FString Suffix = GetPositionSuffix(FinalPosition);
		CountdownText->SetText(FText::FromString(FString::Printf(TEXT("FINISHED %d%s!"), FinalPosition, *Suffix)));
		CountdownText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UMGDebugRaceHUD::UpdateFromSubsystem()
{
	if (!HUDSubsystem.IsValid())
	{
		return;
	}

	// Get telemetry
	FMGVehicleTelemetry Telemetry = HUDSubsystem->GetVehicleTelemetry();
	SetSpeed(Telemetry.SpeedMPH);

	// Get race status
	FMGRaceStatus Status = HUDSubsystem->GetRaceStatus();
	SetPosition(Status.CurrentPosition, Status.TotalRacers);
	SetLap(Status.CurrentLap, Status.TotalLaps);
	SetLapTime(Status.CurrentLapTime);
}

FString UMGDebugRaceHUD::FormatTime(float Seconds) const
{
	if (Seconds <= 0.0f)
	{
		return TEXT("0:00.000");
	}

	int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	float RemainingSeconds = FMath::Fmod(Seconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(RemainingSeconds);
	int32 Milliseconds = FMath::FloorToInt((RemainingSeconds - WholeSeconds) * 1000.0f);

	return FString::Printf(TEXT("%d:%02d.%03d"), Minutes, WholeSeconds, Milliseconds);
}

FString UMGDebugRaceHUD::GetPositionSuffix(int32 Position) const
{
	// Handle special cases for 11th, 12th, 13th
	if (Position >= 11 && Position <= 13)
	{
		return TEXT("th");
	}

	switch (Position % 10)
	{
	case 1:
		return TEXT("st");
	case 2:
		return TEXT("nd");
	case 3:
		return TEXT("rd");
	default:
		return TEXT("th");
	}
}
