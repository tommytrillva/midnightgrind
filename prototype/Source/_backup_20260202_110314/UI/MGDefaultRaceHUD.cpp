// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGDefaultRaceHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "TimerManager.h"

void UMGDefaultRaceHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// Create UI elements if not bound from Blueprint
	CreateUIElements();
}

void UMGDefaultRaceHUD::CreateUIElements()
{
	// Get or create root canvas
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		if (RootCanvas)
		{
			WidgetTree->RootWidget = RootCanvas;
		}
	}

	if (!RootCanvas)
	{
		return;
	}

	// ==========================================
	// SPEED CLUSTER (Bottom Right)
	// ==========================================

	if (!SpeedText)
	{
		SpeedText = CreateStyledText(TEXT("0"), 72, PrimaryColor, EHorizontalAlignment::HAlign_Right);
		PositionElement(SpeedText, FVector2D(-200.0f, -150.0f), FVector2D(180.0f, 80.0f), FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
	}

	if (!SpeedUnitText)
	{
		SpeedUnitText = CreateStyledText(TEXT("MPH"), 24, PrimaryColor, EHorizontalAlignment::HAlign_Right);
		PositionElement(SpeedUnitText, FVector2D(-20.0f, -90.0f), FVector2D(60.0f, 30.0f), FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
	}

	if (!GearText)
	{
		GearText = CreateStyledText(TEXT("N"), 48, AccentColor, EHorizontalAlignment::HAlign_Center);
		PositionElement(GearText, FVector2D(-120.0f, -80.0f), FVector2D(60.0f, 60.0f), FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
	}

	if (!RPMBar)
	{
		RPMBar = CreateStyledProgressBar(
			FLinearColor(0.0f, 1.0f, 0.9f, 1.0f), // Cyan fill
			FLinearColor(0.1f, 0.1f, 0.1f, 0.8f)  // Dark background
		);
		PositionElement(RPMBar, FVector2D(-200.0f, -60.0f), FVector2D(180.0f, 20.0f), FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
	}

	if (!NOSBar)
	{
		NOSBar = CreateStyledProgressBar(
			FLinearColor(0.0f, 0.5f, 1.0f, 1.0f), // Blue fill
			FLinearColor(0.1f, 0.1f, 0.1f, 0.8f)  // Dark background
		);
		PositionElement(NOSBar, FVector2D(-200.0f, -35.0f), FVector2D(180.0f, 12.0f), FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
	}

	// ==========================================
	// RACE INFO (Top Left)
	// ==========================================

	if (!PositionText)
	{
		PositionText = CreateStyledText(TEXT("P1"), 48, FSlateColor(FLinearColor(1.0f, 0.843f, 0.0f, 1.0f)), EHorizontalAlignment::HAlign_Left);
		PositionElement(PositionText, FVector2D(20.0f, 20.0f), FVector2D(100.0f, 60.0f), FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	}

	if (!LapText)
	{
		LapText = CreateStyledText(TEXT("LAP 1/3"), 28, PrimaryColor, EHorizontalAlignment::HAlign_Left);
		PositionElement(LapText, FVector2D(20.0f, 80.0f), FVector2D(150.0f, 40.0f), FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
	}

	// ==========================================
	// TIME DISPLAY (Top Center)
	// ==========================================

	if (!CurrentLapTimeText)
	{
		CurrentLapTimeText = CreateStyledText(TEXT("0:00.000"), 36, PrimaryColor, EHorizontalAlignment::HAlign_Center);
		PositionElement(CurrentLapTimeText, FVector2D(-100.0f, 20.0f), FVector2D(200.0f, 45.0f), FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
	}

	if (!BestLapTimeText)
	{
		BestLapTimeText = CreateStyledText(TEXT("BEST: --:--.---"), 20, AccentColor, EHorizontalAlignment::HAlign_Center);
		PositionElement(BestLapTimeText, FVector2D(-100.0f, 65.0f), FVector2D(200.0f, 25.0f), FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
	}

	if (!TotalTimeText)
	{
		TotalTimeText = CreateStyledText(TEXT("TOTAL: 0:00.000"), 18, PrimaryColor, EHorizontalAlignment::HAlign_Center);
		PositionElement(TotalTimeText, FVector2D(-100.0f, 90.0f), FVector2D(200.0f, 22.0f), FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
	}

	// ==========================================
	// GAP DISPLAY (Top Right)
	// ==========================================

	if (!GapText)
	{
		GapText = CreateStyledText(TEXT(""), 24, PrimaryColor, EHorizontalAlignment::HAlign_Right);
		PositionElement(GapText, FVector2D(-170.0f, 30.0f), FVector2D(150.0f, 30.0f), FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
	}

	// ==========================================
	// DRIFT DISPLAY (Center Right)
	// ==========================================

	if (!DriftScoreText)
	{
		DriftScoreText = CreateStyledText(TEXT(""), 36, AccentColor, EHorizontalAlignment::HAlign_Right);
		PositionElement(DriftScoreText, FVector2D(-20.0f, -50.0f), FVector2D(200.0f, 45.0f), FAnchors(1.0f, 0.5f, 1.0f, 0.5f));
	}

	if (!DriftMultiplierText)
	{
		DriftMultiplierText = CreateStyledText(TEXT(""), 24, WarningColor, EHorizontalAlignment::HAlign_Right);
		PositionElement(DriftMultiplierText, FVector2D(-20.0f, -10.0f), FVector2D(150.0f, 30.0f), FAnchors(1.0f, 0.5f, 1.0f, 0.5f));
	}

	// ==========================================
	// STATUS MESSAGE (Center)
	// ==========================================

	if (!StatusMessageText)
	{
		StatusMessageText = CreateStyledText(TEXT(""), 64, WarningColor, EHorizontalAlignment::HAlign_Center);
		PositionElement(StatusMessageText, FVector2D(-200.0f, -50.0f), FVector2D(400.0f, 100.0f), FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		StatusMessageText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UTextBlock* UMGDefaultRaceHUD::CreateStyledText(const FString& InitialText, int32 FontSize, FSlateColor Color, EHorizontalAlignment HAlign)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(InitialText));
		TextBlock->SetColorAndOpacity(Color);
		TextBlock->SetJustification(ETextJustify::Type(HAlign));

		FSlateFontInfo FontInfo = TextBlock->GetFont();
		FontInfo.Size = FontSize;
		FontInfo.OutlineSettings.OutlineSize = 2;
		FontInfo.OutlineSettings.OutlineColor = FLinearColor::Black;
		TextBlock->SetFont(FontInfo);

		// Add shadow for Y2K aesthetic
		TextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
		TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));

		RootCanvas->AddChild(TextBlock);
	}
	return TextBlock;
}

UProgressBar* UMGDefaultRaceHUD::CreateStyledProgressBar(FLinearColor FillColor, FLinearColor BackgroundColor)
{
	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	if (Bar)
	{
		Bar->SetFillColorAndOpacity(FillColor);
		Bar->SetPercent(0.0f);

		RootCanvas->AddChild(Bar);
	}
	return Bar;
}

void UMGDefaultRaceHUD::PositionElement(UWidget* Widget, FVector2D Position, FVector2D Size, FAnchors Anchors)
{
	if (!Widget || !RootCanvas)
	{
		return;
	}

	UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot);
	if (!Slot)
	{
		return;
	}

	Slot->SetAnchors(Anchors);
	Slot->SetPosition(Position);
	Slot->SetSize(Size);
	Slot->SetAutoSize(false);
}

// ==========================================
// UPDATE IMPLEMENTATIONS
// ==========================================

void UMGDefaultRaceHUD::UpdateSpeedDisplay_Implementation(float SpeedKPH, float SpeedMPH, bool bUseMPH)
{
	Super::UpdateSpeedDisplay_Implementation(SpeedKPH, SpeedMPH, bUseMPH);

	if (SpeedText)
	{
		float DisplaySpeed = bUseMPH ? SpeedMPH : SpeedKPH;
		SpeedText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::RoundToInt(DisplayedSpeed))));
	}

	if (SpeedUnitText)
	{
		SpeedUnitText->SetText(FText::FromString(bUseMPH ? TEXT("MPH") : TEXT("KPH")));
	}
}

void UMGDefaultRaceHUD::UpdateTachometer_Implementation(float RPM, float MaxRPM, int32 Gear, int32 TotalGears)
{
	Super::UpdateTachometer_Implementation(RPM, MaxRPM, Gear, TotalGears);

	if (GearText)
	{
		FString GearStr;
		if (Gear == 0)
		{
			GearStr = TEXT("N");
		}
		else if (Gear == -1)
		{
			GearStr = TEXT("R");
		}
		else
		{
			GearStr = FString::Printf(TEXT("%d"), Gear);
		}
		GearText->SetText(FText::FromString(GearStr));

		// Color based on RPM
		if (bRedlineActive)
		{
			GearText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)));
		}
		else if (bShiftIndicatorActive)
		{
			GearText->SetColorAndOpacity(WarningColor);
		}
		else
		{
			GearText->SetColorAndOpacity(AccentColor);
		}
	}

	if (RPMBar)
	{
		float RPMPercent = MaxRPM > 0.0f ? DisplayedRPM / MaxRPM : 0.0f;
		RPMBar->SetPercent(RPMPercent);

		// Color gradient from cyan to yellow to red
		FLinearColor RPMColor;
		if (RPMPercent < 0.7f)
		{
			RPMColor = FLinearColor(0.0f, 1.0f, 0.9f, 1.0f); // Cyan
		}
		else if (RPMPercent < 0.9f)
		{
			float T = (RPMPercent - 0.7f) / 0.2f;
			RPMColor = FLinearColor::LerpUsingHSV(
				FLinearColor(0.0f, 1.0f, 0.9f, 1.0f),
				FLinearColor(1.0f, 1.0f, 0.0f, 1.0f),
				T
			);
		}
		else
		{
			float T = (RPMPercent - 0.9f) / 0.1f;
			RPMColor = FLinearColor::LerpUsingHSV(
				FLinearColor(1.0f, 1.0f, 0.0f, 1.0f),
				FLinearColor(1.0f, 0.0f, 0.0f, 1.0f),
				T
			);
		}
		RPMBar->SetFillColorAndOpacity(RPMColor);
	}
}

void UMGDefaultRaceHUD::UpdateNOSGauge_Implementation(float NOSAmount, bool bNOSActive)
{
	Super::UpdateNOSGauge_Implementation(NOSAmount, bNOSActive);

	if (NOSBar)
	{
		NOSBar->SetPercent(NOSAmount / 100.0f);

		// Pulse when active
		if (bNOSActive)
		{
			float TimeSeconds = 0.0f;
			if (UWorld* World = GetWorld())
			{
				TimeSeconds = World->GetTimeSeconds();
			}
			float Pulse = 0.7f + 0.3f * FMath::Abs(FMath::Sin(TimeSeconds * 10.0f));
			NOSBar->SetFillColorAndOpacity(FLinearColor(0.0f, Pulse, 1.0f, 1.0f));
		}
		else
		{
			NOSBar->SetFillColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f));
		}
	}
}

void UMGDefaultRaceHUD::UpdatePositionDisplay_Implementation(int32 Position, int32 TotalRacers)
{
	Super::UpdatePositionDisplay_Implementation(Position, TotalRacers);

	if (PositionText)
	{
		PositionText->SetText(FText::FromString(FString::Printf(TEXT("P%d"), Position)));
		PositionText->SetColorAndOpacity(FSlateColor(GetPositionColor(Position)));
	}
}

void UMGDefaultRaceHUD::UpdateLapDisplay_Implementation(int32 CurrentLap, int32 TotalLaps, bool bFinalLap)
{
	Super::UpdateLapDisplay_Implementation(CurrentLap, TotalLaps, bFinalLap);

	if (LapText)
	{
		LapText->SetText(FText::FromString(FString::Printf(TEXT("LAP %d/%d"), CurrentLap, TotalLaps)));

		if (bFinalLap)
		{
			LapText->SetColorAndOpacity(WarningColor);
		}
		else
		{
			LapText->SetColorAndOpacity(PrimaryColor);
		}
	}
}

void UMGDefaultRaceHUD::UpdateTimeDisplay_Implementation(float CurrentLapTime, float BestLapTime, float TotalTime)
{
	Super::UpdateTimeDisplay_Implementation(CurrentLapTime, BestLapTime, TotalTime);

	if (CurrentLapTimeText)
	{
		CurrentLapTimeText->SetText(FormatTime(CurrentLapTime));
	}

	if (BestLapTimeText)
	{
		if (BestLapTime > 0.0f)
		{
			BestLapTimeText->SetText(FText::FromString(FString::Printf(TEXT("BEST: %s"), *FormatTime(BestLapTime).ToString())));
		}
		else
		{
			BestLapTimeText->SetText(FText::FromString(TEXT("BEST: --:--.---")));
		}
	}

	if (TotalTimeText)
	{
		TotalTimeText->SetText(FText::FromString(FString::Printf(TEXT("TOTAL: %s"), *FormatTime(TotalTime).ToString())));
	}
}

void UMGDefaultRaceHUD::UpdateGapDisplay_Implementation(float GapToLeader, float GapToNext)
{
	Super::UpdateGapDisplay_Implementation(GapToLeader, GapToNext);

	if (GapText)
	{
		if (GapToLeader > 0.001f)
		{
			GapText->SetText(FText::FromString(FString::Printf(TEXT("LEADER: %s"), *FormatGapTime(GapToLeader).ToString())));
			GapText->SetColorAndOpacity(FSlateColor(GetGapColor(GapToLeader)));
		}
		else if (GapToNext > 0.001f)
		{
			GapText->SetText(FText::FromString(FString::Printf(TEXT("GAP: %s"), *FormatGapTime(-GapToNext).ToString())));
			GapText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)));
		}
		else
		{
			GapText->SetText(FText::GetEmpty());
		}
	}
}

void UMGDefaultRaceHUD::UpdateDriftDisplay_Implementation(int32 CurrentScore, float Multiplier, int32 ChainCount, float ChainTimeRemaining)
{
	Super::UpdateDriftDisplay_Implementation(CurrentScore, Multiplier, ChainCount, ChainTimeRemaining);

	if (DriftScoreText)
	{
		if (CurrentScore > 0)
		{
			DriftScoreText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentScore)));
			DriftScoreText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			DriftScoreText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (DriftMultiplierText)
	{
		if (Multiplier > 1.0f)
		{
			DriftMultiplierText->SetText(FText::FromString(FString::Printf(TEXT("x%.1f"), Multiplier)));
			DriftMultiplierText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			DriftMultiplierText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

// ==========================================
// ANIMATION IMPLEMENTATIONS
// ==========================================

void UMGDefaultRaceHUD::PlayPositionChangeAnimation_Implementation(int32 OldPosition, int32 NewPosition)
{
	Super::PlayPositionChangeAnimation_Implementation(OldPosition, NewPosition);

	if (StatusMessageText)
	{
		FString Message;
		if (NewPosition < OldPosition)
		{
			Message = FString::Printf(TEXT("UP TO P%d!"), NewPosition);
			StatusMessageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)));
		}
		else
		{
			Message = FString::Printf(TEXT("DOWN TO P%d"), NewPosition);
			StatusMessageText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)));
		}

		StatusMessageText->SetText(FText::FromString(Message));
		StatusMessageText->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StatusMessageTimerHandle);
			World->GetTimerManager().SetTimer(StatusMessageTimerHandle, this, &UMGDefaultRaceHUD::ClearStatusMessage, 2.0f, false);
		}
	}
}

void UMGDefaultRaceHUD::PlayShiftIndicator_Implementation()
{
	Super::PlayShiftIndicator_Implementation();

	// Flash handled in UpdateTachometer via bShiftIndicatorActive
}

void UMGDefaultRaceHUD::PlayRedlineWarning_Implementation()
{
	Super::PlayRedlineWarning_Implementation();

	// Flash handled in UpdateTachometer via bRedlineActive
}

void UMGDefaultRaceHUD::PlayNOSActivationEffect_Implementation()
{
	Super::PlayNOSActivationEffect_Implementation();

	if (StatusMessageText)
	{
		StatusMessageText->SetText(FText::FromString(TEXT("NOS!")));
		StatusMessageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f)));
		StatusMessageText->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StatusMessageTimerHandle);
			World->GetTimerManager().SetTimer(StatusMessageTimerHandle, this, &UMGDefaultRaceHUD::ClearStatusMessage, 1.0f, false);
		}
	}
}

void UMGDefaultRaceHUD::PlayFinalLapEffect_Implementation()
{
	Super::PlayFinalLapEffect_Implementation();

	if (StatusMessageText)
	{
		StatusMessageText->SetText(FText::FromString(TEXT("FINAL LAP!")));
		StatusMessageText->SetColorAndOpacity(WarningColor);
		StatusMessageText->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StatusMessageTimerHandle);
			World->GetTimerManager().SetTimer(StatusMessageTimerHandle, this, &UMGDefaultRaceHUD::ClearStatusMessage, 3.0f, false);
		}
	}
}

void UMGDefaultRaceHUD::PlayBestLapEffect_Implementation()
{
	Super::PlayBestLapEffect_Implementation();

	if (StatusMessageText)
	{
		StatusMessageText->SetText(FText::FromString(TEXT("NEW BEST LAP!")));
		StatusMessageText->SetColorAndOpacity(AccentColor);
		StatusMessageText->SetVisibility(ESlateVisibility::HitTestInvisible);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(StatusMessageTimerHandle);
			World->GetTimerManager().SetTimer(StatusMessageTimerHandle, this, &UMGDefaultRaceHUD::ClearStatusMessage, 3.0f, false);
		}
	}
}

void UMGDefaultRaceHUD::ClearStatusMessage()
{
	if (StatusMessageText)
	{
		StatusMessageText->SetVisibility(ESlateVisibility::Collapsed);
	}
}
