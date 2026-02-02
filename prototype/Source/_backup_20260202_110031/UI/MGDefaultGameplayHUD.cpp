// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGDefaultGameplayHUD.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/Border.h"
#include "UI/MGMinimapWidget.h"
#include "Animation/WidgetAnimation.h"

void UMGDefaultGameplayHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize drift panel as hidden
	if (DriftScorePanel)
	{
		DriftScorePanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Initialize optional glow effects as hidden
	if (SpeedGlowEffect)
	{
		SpeedGlowEffect->SetOpacity(0.0f);
	}

	if (NitrousActiveGlow)
	{
		NitrousActiveGlow->SetOpacity(0.0f);
	}

	// Set initial colors
	if (TachometerBar)
	{
		TachometerBar->SetFillColorAndOpacity(TachBarColor);
	}

	if (NitrousBar)
	{
		NitrousBar->SetFillColorAndOpacity(NitrousBarColor);
	}
}

void UMGDefaultGameplayHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMGDefaultGameplayHUD::UpdateSpeedDisplay_Implementation(float SpeedKPH, float SpeedMPH, bool bUseMPH)
{
	if (!SpeedText)
	{
		return;
	}

	float DisplaySpeed = bUseMPH ? SpeedMPH : SpeedKPH;
	int32 SpeedInt = FMath::RoundToInt(DisplaySpeed);

	SpeedText->SetText(FText::AsNumber(SpeedInt));

	if (SpeedUnitText)
	{
		SpeedUnitText->SetText(FText::FromString(bUseMPH ? TEXT("MPH") : TEXT("KPH")));
	}

	// Color based on speed threshold
	bool bHighSpeed = SpeedMPH > HighSpeedThreshold;
	FLinearColor TargetColor = bHighSpeed ? HighSpeedTextColor : SpeedTextColor;
	SpeedText->SetColorAndOpacity(FSlateColor(TargetColor));

	// Glow effect at high speed
	if (SpeedGlowEffect)
	{
		float TargetOpacity = bHighSpeed ? 1.0f : 0.0f;
		float CurrentOpacity = SpeedGlowEffect->GetColorAndOpacity().A;
		UWorld* World = GetWorld();
		float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
		float NewOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaSeconds, 5.0f);
		SpeedGlowEffect->SetOpacity(NewOpacity);
	}
}

void UMGDefaultGameplayHUD::UpdateTachometer_Implementation(float RPM, float MaxRPM, int32 Gear, int32 TotalGears)
{
	if (TachometerBar)
	{
		float Percent = FMath::Clamp(RPM / MaxRPM, 0.0f, 1.0f);
		TachometerBar->SetPercent(Percent);

		// Color shift toward redline
		float RedlineThreshold = 0.85f;
		if (Percent > RedlineThreshold)
		{
			float RedlineFactor = (Percent - RedlineThreshold) / (1.0f - RedlineThreshold);
			FLinearColor BarColor = FMath::Lerp(TachBarColor, TachRedlineColor, RedlineFactor);
			TachometerBar->SetFillColorAndOpacity(BarColor);
		}
		else
		{
			TachometerBar->SetFillColorAndOpacity(TachBarColor);
		}
	}

	if (TachometerRedline)
	{
		bool bInRedline = RPM / MaxRPM > 0.9f;
		float TargetOpacity = bInRedline ? 1.0f : 0.3f;
		TachometerRedline->SetOpacity(TargetOpacity);
	}

	if (GearText)
	{
		FString GearString;
		if (Gear == 0)
		{
			GearString = TEXT("N");
		}
		else if (Gear == -1)
		{
			GearString = TEXT("R");
		}
		else
		{
			GearString = FString::FromInt(Gear);
		}
		GearText->SetText(FText::FromString(GearString));
	}

	// Shift light when near redline and not at max gear
	if (ShiftLightImage)
	{
		bool bShowShiftLight = (RPM / MaxRPM > 0.92f) && (Gear < TotalGears) && (Gear > 0);
		ShiftLightImage->SetVisibility(bShowShiftLight ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// Redline pulse effect
	if (RedlinePulse && RPM / MaxRPM > 0.95f)
	{
		PlayRedlineWarning();
	}
}

void UMGDefaultGameplayHUD::UpdateNOSGauge_Implementation(float NOSAmount, bool bNOSActive)
{
	if (NitrousBar)
	{
		NitrousBar->SetPercent(FMath::Clamp(NOSAmount, 0.0f, 1.0f));

		FLinearColor BarColor = bNOSActive ? NitrousActiveColor : NitrousBarColor;
		NitrousBar->SetFillColorAndOpacity(BarColor);
	}

	if (NitrousActiveGlow)
	{
		float TargetOpacity = bNOSActive ? 1.0f : 0.0f;
		float CurrentOpacity = NitrousActiveGlow->GetColorAndOpacity().A;
		UWorld* World = GetWorld();
		float DeltaSeconds = World ? World->GetDeltaSeconds() : 0.016f;
		float NewOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaSeconds, 8.0f);
		NitrousActiveGlow->SetOpacity(NewOpacity);
	}

	if (NitrousLabel)
	{
		FLinearColor LabelColor = bNOSActive ? NitrousActiveColor : FLinearColor::White;
		NitrousLabel->SetColorAndOpacity(FSlateColor(LabelColor));
	}

	// Trigger activation effect on state change
	if (bNOSActive && !bWasNOSActive)
	{
		PlayNOSActivationEffect();
	}
	bWasNOSActive = bNOSActive;
}

void UMGDefaultGameplayHUD::UpdatePositionDisplay_Implementation(int32 Position, int32 TotalRacers)
{
	if (PositionText)
	{
		PositionText->SetText(FText::AsNumber(Position));
	}

	if (PositionSuffixText)
	{
		PositionSuffixText->SetText(FText::FromString(GetPositionSuffix(Position)));
	}

	if (TotalRacersText)
	{
		TotalRacersText->SetText(FText::FromString(FString::Printf(TEXT("/%d"), TotalRacers)));
	}

	// Trigger animation on position change
	if (LastPosition != 0 && LastPosition != Position)
	{
		PlayPositionChangeAnimation(LastPosition, Position);
	}
	LastPosition = Position;
}

void UMGDefaultGameplayHUD::UpdateLapDisplay_Implementation(int32 CurrentLap, int32 TotalLaps, bool bFinalLap)
{
	if (LapText)
	{
		LapText->SetText(FText::FromString(FString::Printf(TEXT("LAP %d/%d"), CurrentLap, TotalLaps)));
	}

	if (FinalLapIndicator)
	{
		FinalLapIndicator->SetVisibility(bFinalLap ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

		if (bFinalLap)
		{
			PlayFinalLapEffect();
		}
	}
}

void UMGDefaultGameplayHUD::UpdateTimeDisplay_Implementation(float CurrentLapTime, float BestLapTime, float TotalTime)
{
	auto FormatTime = [](float TimeSeconds) -> FString
	{
		int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
		float Seconds = FMath::Fmod(TimeSeconds, 60.0f);
		return FString::Printf(TEXT("%d:%05.2f"), Minutes, Seconds);
	};

	if (CurrentLapTimeText)
	{
		CurrentLapTimeText->SetText(FText::FromString(FormatTime(CurrentLapTime)));
	}

	if (BestLapTimeText)
	{
		if (BestLapTime > 0.0f)
		{
			BestLapTimeText->SetText(FText::FromString(FString::Printf(TEXT("BEST: %s"), *FormatTime(BestLapTime))));
		}
		else
		{
			BestLapTimeText->SetText(FText::FromString(TEXT("BEST: --:--.--")));
		}
	}

	if (TotalTimeText)
	{
		TotalTimeText->SetText(FText::FromString(FString::Printf(TEXT("TOTAL: %s"), *FormatTime(TotalTime))));
	}
}

void UMGDefaultGameplayHUD::UpdateGapDisplay_Implementation(float GapToLeader, float GapToNext)
{
	if (!GapText)
	{
		return;
	}

	auto FormatGap = [](float Gap) -> FString
	{
		if (FMath::Abs(Gap) < 0.01f)
		{
			return TEXT("--");
		}
		FString Sign = Gap > 0.0f ? TEXT("+") : TEXT("");
		return FString::Printf(TEXT("%s%.2f"), *Sign, Gap);
	};

	// Show gap to leader if not in first, otherwise gap to next
	if (GapToLeader > 0.0f)
	{
		GapText->SetText(FText::FromString(FString::Printf(TEXT("GAP: %s"), *FormatGap(GapToLeader))));
		GapText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.3f, 0.3f, 1.0f))); // Red = behind
	}
	else if (GapToNext < 0.0f)
	{
		GapText->SetText(FText::FromString(FString::Printf(TEXT("LEAD: %s"), *FormatGap(-GapToNext))));
		GapText->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 1.0f, 0.3f, 1.0f))); // Green = ahead
	}
	else
	{
		GapText->SetText(FText::FromString(TEXT("GAP: --")));
		GapText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
}

void UMGDefaultGameplayHUD::UpdateDriftDisplay_Implementation(int32 CurrentScore, float Multiplier, int32 ChainCount, float ChainTimeRemaining)
{
	bool bCurrentlyDrifting = CurrentScore > 0 && ChainTimeRemaining > 0.0f;
	UpdateDriftVisibility(bCurrentlyDrifting);

	if (!bCurrentlyDrifting)
	{
		return;
	}

	if (DriftScoreText)
	{
		DriftScoreText->SetText(FText::AsNumber(CurrentScore));
		DriftScoreText->SetColorAndOpacity(FSlateColor(DriftScoreColor));
	}

	if (DriftMultiplierText)
	{
		DriftMultiplierText->SetText(FText::FromString(FString::Printf(TEXT("x%.1f"), Multiplier)));
	}

	if (DriftChainBar)
	{
		// Chain bar depletes as time runs out (assuming max chain time of ~3 seconds)
		float MaxChainTime = 3.0f;
		float ChainPercent = FMath::Clamp(ChainTimeRemaining / MaxChainTime, 0.0f, 1.0f);
		DriftChainBar->SetPercent(ChainPercent);

		// Color shifts from green to red as chain depletes
		FLinearColor ChainColor = FMath::Lerp(
			FLinearColor(1.0f, 0.2f, 0.0f, 1.0f),  // Red (low time)
			FLinearColor(0.0f, 1.0f, 0.3f, 1.0f),  // Green (full time)
			ChainPercent
		);
		DriftChainBar->SetFillColorAndOpacity(ChainColor);
	}
}

void UMGDefaultGameplayHUD::PlayPositionChangeAnimation_Implementation(int32 OldPosition, int32 NewPosition)
{
	if (!PositionText)
	{
		return;
	}

	// Color feedback - green for gaining, red for losing
	if (NewPosition < OldPosition)
	{
		// Gained position
		PositionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)));
	}
	else
	{
		// Lost position
		PositionText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)));
	}

	AnimatePulse(PositionText, 0.5f);
}

void UMGDefaultGameplayHUD::PlayShiftIndicator_Implementation()
{
	if (ShiftLightImage)
	{
		AnimatePulse(ShiftLightImage, 0.2f);
	}
}

void UMGDefaultGameplayHUD::PlayRedlineWarning_Implementation()
{
	if (RedlinePulse)
	{
		// Flash the redline border
		UWorld* World = GetWorld();
		float TimeSeconds = World ? World->GetTimeSeconds() : 0.0f;
		float PulseAlpha = FMath::Sin(TimeSeconds * 15.0f) * 0.5f + 0.5f;
		RedlinePulse->SetBrushColor(FLinearColor(TachRedlineColor.R, TachRedlineColor.G, TachRedlineColor.B, PulseAlpha));
	}
}

void UMGDefaultGameplayHUD::PlayNOSActivationEffect_Implementation()
{
	if (NitrousBar)
	{
		AnimatePulse(NitrousBar, 0.3f);
	}

	if (NitrousLabel)
	{
		AnimatePulse(NitrousLabel, 0.3f);
	}
}

void UMGDefaultGameplayHUD::PlayFinalLapEffect_Implementation()
{
	if (LapText)
	{
		LapText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f)));
		AnimatePulse(LapText, 1.0f);
	}

	if (FinalLapIndicator)
	{
		AnimatePulse(FinalLapIndicator, 1.0f);
	}
}

void UMGDefaultGameplayHUD::PlayBestLapEffect_Implementation()
{
	if (BestLapTimeText)
	{
		BestLapTimeText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)));
		AnimatePulse(BestLapTimeText, 0.8f);
	}
}

FString UMGDefaultGameplayHUD::GetPositionSuffix(int32 Position) const
{
	if (Position <= 0)
	{
		return TEXT("");
	}

	int32 LastTwoDigits = Position % 100;
	if (LastTwoDigits >= 11 && LastTwoDigits <= 13)
	{
		return TEXT("th");
	}

	switch (Position % 10)
	{
		case 1: return TEXT("st");
		case 2: return TEXT("nd");
		case 3: return TEXT("rd");
		default: return TEXT("th");
	}
}

void UMGDefaultGameplayHUD::AnimatePulse(UWidget* Widget, float Duration)
{
	if (!Widget)
	{
		return;
	}

	// Simple scale pulse using render transform
	FWidgetTransform Transform = Widget->GetRenderTransform();
	Transform.Scale = FVector2D(1.2f, 1.2f);
	Widget->SetRenderTransform(Transform);

	// Timer to reset - in production would use proper widget animation
	if (UWorld* World = GetWorld())
	{
		FTimerHandle ResetHandle;
		TWeakObjectPtr<UWidget> WeakWidget(Widget);
		World->GetTimerManager().SetTimer(ResetHandle, [WeakWidget]()
		{
			if (WeakWidget.IsValid())
			{
				FWidgetTransform ResetTransform = WeakWidget->GetRenderTransform();
				ResetTransform.Scale = FVector2D(1.0f, 1.0f);
				WeakWidget->SetRenderTransform(ResetTransform);
			}
		}, Duration, false);
	}
}

void UMGDefaultGameplayHUD::UpdateDriftVisibility(bool bDrifting)
{
	if (!DriftScorePanel)
	{
		return;
	}

	if (bDrifting && !bIsDrifting)
	{
		// Started drifting - show panel
		DriftScorePanel->SetVisibility(ESlateVisibility::Visible);
	}
	else if (!bDrifting && bIsDrifting)
	{
		// Stopped drifting - hide panel
		DriftScorePanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	bIsDrifting = bDrifting;
}
