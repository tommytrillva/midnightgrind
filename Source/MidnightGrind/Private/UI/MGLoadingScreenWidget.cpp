// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGLoadingScreenWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"

UMGLoadingScreenWidget::UMGLoadingScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMGLoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CreateUIElements();

	// Start with fade in
	FadeProgress = 0.0f;
	SetRenderOpacity(0.0f);
}

void UMGLoadingScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateProgressAnimation(InDeltaTime);
	UpdateTipRotation(InDeltaTime);
	UpdateBackgroundAnimation(InDeltaTime);

	// Handle fade in
	if (FadeProgress < 1.0f && !bLoadingComplete)
	{
		FadeProgress = FMath::Min(1.0f, FadeProgress + InDeltaTime * 2.0f);
		SetRenderOpacity(FadeProgress);
	}

	// Handle fade out
	if (bLoadingComplete && FadeProgress > 0.0f)
	{
		FadeProgress = FMath::Max(0.0f, FadeProgress - InDeltaTime * 2.0f);
		SetRenderOpacity(FadeProgress);

		if (FadeProgress <= 0.0f)
		{
			RemoveFromParent();
		}
	}
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGLoadingScreenWidget::SetContext(EMGLoadingContext Context)
{
	LoadingContext = Context;

	// Show/hide race info panel based on context
	if (RaceInfoPanel)
	{
		bool bShowRaceInfo = (Context == EMGLoadingContext::Race);
		RaceInfoPanel->SetVisibility(bShowRaceInfo ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UMGLoadingScreenWidget::SetRaceData(const FMGRaceLoadingData& Data)
{
	RaceData = Data;
	UpdateRaceInfoDisplay();
}

void UMGLoadingScreenWidget::SetLoadingTips(const TArray<FMGLoadingTip>& Tips)
{
	LoadingTips = Tips;
	CurrentTipIndex = 0;
	TipTimer = 0.0f;

	// Show first tip
	if (LoadingTips.Num() > 0)
	{
		ShowTip(0);
	}
}

void UMGLoadingScreenWidget::SetTipInterval(float Seconds)
{
	TipInterval = FMath::Max(1.0f, Seconds);
}

// ==========================================
// PROGRESS
// ==========================================

void UMGLoadingScreenWidget::SetProgress(float Progress)
{
	TargetProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
}

void UMGLoadingScreenWidget::SetProgressText(const FText& Text)
{
	if (ProgressText)
	{
		ProgressText->SetText(Text);
	}
}

void UMGLoadingScreenWidget::SetLoadingComplete()
{
	bLoadingComplete = true;
	TargetProgress = 1.0f;
}

// ==========================================
// TIPS
// ==========================================

void UMGLoadingScreenWidget::ShowNextTip()
{
	if (LoadingTips.Num() == 0)
	{
		return;
	}

	int32 NextIndex = (CurrentTipIndex + 1) % LoadingTips.Num();
	ShowTip(NextIndex);
}

void UMGLoadingScreenWidget::ShowTip(int32 Index)
{
	if (Index < 0 || Index >= LoadingTips.Num())
	{
		return;
	}

	CurrentTipIndex = Index;
	bTipTransitioning = true;
	TipTransitionProgress = 0.0f;

	const FMGLoadingTip& Tip = LoadingTips[Index];

	if (TipText)
	{
		TipText->SetText(Tip.TipText);
	}

	if (TipIcon && Tip.Icon)
	{
		TipIcon->SetBrushFromTexture(Tip.Icon);
		TipIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else if (TipIcon)
	{
		TipIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ==========================================
// UI CREATION
// ==========================================

void UMGLoadingScreenWidget::CreateUIElements()
{
	if (!WidgetTree)
	{
		return;
	}

	// Create root canvas
	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	if (!RootCanvas)
	{
		return;
	}
	WidgetTree->RootWidget = RootCanvas;

	CreateAnimatedBackground();
	CreateRaceInfoSection();
	CreateTipSection();
	CreateProgressSection();
}

void UMGLoadingScreenWidget::CreateProgressSection()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Progress container - bottom of screen
	UCanvasPanel* ProgressPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ProgressPanel"));
	if (ProgressPanel)
	{
		RootCanvas->AddChild(ProgressPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(ProgressPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.1f, 0.85f, 0.9f, 0.92f));
			Slot->SetOffsets(FMargin(0.0f));
		}

		// Progress bar background
		UBorder* ProgressBg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ProgressBg"));
		if (ProgressBg)
		{
			ProgressPanel->AddChild(ProgressBg);
			if (UCanvasPanelSlot* BgSlot = Cast<UCanvasPanelSlot>(ProgressBg->Slot))
			{
				BgSlot->SetAnchors(FAnchors(0.0f, 0.5f, 1.0f, 0.5f));
				BgSlot->SetAlignment(FVector2D(0.0f, 0.5f));
				BgSlot->SetSize(FVector2D(0.0f, 8.0f));
				BgSlot->SetAutoSize(false);
			}
			ProgressBg->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.15f, 1.0f));
		}

		// Progress bar
		ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("ProgressBar"));
		if (ProgressBar)
		{
			ProgressPanel->AddChild(ProgressBar);
			if (UCanvasPanelSlot* BarSlot = Cast<UCanvasPanelSlot>(ProgressBar->Slot))
			{
				BarSlot->SetAnchors(FAnchors(0.0f, 0.5f, 1.0f, 0.5f));
				BarSlot->SetAlignment(FVector2D(0.0f, 0.5f));
				BarSlot->SetSize(FVector2D(0.0f, 8.0f));
				BarSlot->SetAutoSize(false);
			}
			ProgressBar->SetFillColorAndOpacity(CyanNeon);
			ProgressBar->SetPercent(0.0f);
		}

		// Progress text
		ProgressText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressText"));
		if (ProgressText)
		{
			ProgressPanel->AddChild(ProgressText);
			if (UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(ProgressText->Slot))
			{
				TextSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
				TextSlot->SetAlignment(FVector2D(0.0f, 1.0f));
				TextSlot->SetPosition(FVector2D(0.0f, -15.0f));
				TextSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = ProgressText->GetFont();
			Font.Size = 18;
			ProgressText->SetFont(Font);
			ProgressText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.7f));
			ProgressText->SetText(FText::FromString(TEXT("Loading...")));
		}

		// Progress percent
		ProgressPercentText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProgressPercent"));
		if (ProgressPercentText)
		{
			ProgressPanel->AddChild(ProgressPercentText);
			if (UCanvasPanelSlot* PercentSlot = Cast<UCanvasPanelSlot>(ProgressPercentText->Slot))
			{
				PercentSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
				PercentSlot->SetAlignment(FVector2D(1.0f, 1.0f));
				PercentSlot->SetPosition(FVector2D(0.0f, -15.0f));
				PercentSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = ProgressPercentText->GetFont();
			Font.Size = 18;
			ProgressPercentText->SetFont(Font);
			ProgressPercentText->SetColorAndOpacity(FSlateColor(CyanNeon));
			ProgressPercentText->SetText(FText::FromString(TEXT("0%")));
		}
	}
}

void UMGLoadingScreenWidget::CreateTipSection()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Tip panel - bottom center, above progress
	TipPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("TipPanel"));
	if (TipPanel)
	{
		RootCanvas->AddChild(TipPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(TipPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.15f, 0.72f, 0.85f, 0.82f));
			Slot->SetOffsets(FMargin(0.0f));
		}

		// Tip label
		TipLabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TipLabel"));
		if (TipLabelText)
		{
			TipPanel->AddChild(TipLabelText);
			if (UCanvasPanelSlot* LabelSlot = Cast<UCanvasPanelSlot>(TipLabelText->Slot))
			{
				LabelSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				LabelSlot->SetAlignment(FVector2D(0.5f, 0.0f));
				LabelSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = TipLabelText->GetFont();
			Font.Size = 16;
			TipLabelText->SetFont(Font);
			TipLabelText->SetColorAndOpacity(FSlateColor(PinkNeon * 0.8f));
			TipLabelText->SetText(FText::FromString(TEXT("TIP")));
		}

		// Tip text
		TipText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TipText"));
		if (TipText)
		{
			TipPanel->AddChild(TipText);
			if (UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(TipText->Slot))
			{
				TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
				TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				TextSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = TipText->GetFont();
			Font.Size = 22;
			TipText->SetFont(Font);
			TipText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			TipText->SetJustification(ETextJustify::Center);
			TipText->SetAutoWrapText(true);
		}
	}
}

void UMGLoadingScreenWidget::CreateRaceInfoSection()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Race info panel - upper area
	RaceInfoPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RaceInfoPanel"));
	if (RaceInfoPanel)
	{
		RootCanvas->AddChild(RaceInfoPanel);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(RaceInfoPanel->Slot))
		{
			Slot->SetAnchors(FAnchors(0.0f, 0.1f, 1.0f, 0.65f));
			Slot->SetOffsets(FMargin(60.0f, 0.0f, 60.0f, 0.0f));
		}
		RaceInfoPanel->SetVisibility(ESlateVisibility::Collapsed);

		// Track name - large, centered
		TrackNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TrackName"));
		if (TrackNameText)
		{
			RaceInfoPanel->AddChild(TrackNameText);
			if (UCanvasPanelSlot* NameSlot = Cast<UCanvasPanelSlot>(TrackNameText->Slot))
			{
				NameSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				NameSlot->SetAlignment(FVector2D(0.5f, 0.0f));
				NameSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = TrackNameText->GetFont();
			Font.Size = 56;
			TrackNameText->SetFont(Font);
			TrackNameText->SetColorAndOpacity(FSlateColor(CyanNeon));
			TrackNameText->SetJustification(ETextJustify::Center);
		}

		// Track location
		TrackLocationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TrackLocation"));
		if (TrackLocationText)
		{
			RaceInfoPanel->AddChild(TrackLocationText);
			if (UCanvasPanelSlot* LocSlot = Cast<UCanvasPanelSlot>(TrackLocationText->Slot))
			{
				LocSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				LocSlot->SetAlignment(FVector2D(0.5f, 0.0f));
				LocSlot->SetPosition(FVector2D(0.0f, 70.0f));
				LocSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = TrackLocationText->GetFont();
			Font.Size = 24;
			TrackLocationText->SetFont(Font);
			TrackLocationText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.6f));
		}

		// Race mode
		RaceModeText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RaceMode"));
		if (RaceModeText)
		{
			RaceInfoPanel->AddChild(RaceModeText);
			if (UCanvasPanelSlot* ModeSlot = Cast<UCanvasPanelSlot>(RaceModeText->Slot))
			{
				ModeSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
				ModeSlot->SetAlignment(FVector2D(0.5f, 0.0f));
				ModeSlot->SetPosition(FVector2D(0.0f, 110.0f));
				ModeSlot->SetAutoSize(true);
			}
			FSlateFontInfo Font = RaceModeText->GetFont();
			Font.Size = 20;
			RaceModeText->SetFont(Font);
			RaceModeText->SetColorAndOpacity(FSlateColor(PinkNeon));
		}

		// Info row (laps, weather)
		UHorizontalBox* InfoRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("InfoRow"));
		if (InfoRow)
		{
			RaceInfoPanel->AddChild(InfoRow);
			if (UCanvasPanelSlot* RowSlot = Cast<UCanvasPanelSlot>(InfoRow->Slot))
			{
				RowSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
				RowSlot->SetAlignment(FVector2D(0.5f, 1.0f));
				RowSlot->SetPosition(FVector2D(0.0f, -20.0f));
				RowSlot->SetAutoSize(true);
			}

			// Lap count
			LapCountText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LapCount"));
			if (LapCountText)
			{
				InfoRow->AddChild(LapCountText);
				if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(LapCountText->Slot))
				{
					HSlot->SetPadding(FMargin(0.0f, 0.0f, 40.0f, 0.0f));
				}
				FSlateFontInfo Font = LapCountText->GetFont();
				Font.Size = 20;
				LapCountText->SetFont(Font);
				LapCountText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.8f));
			}

			// Weather
			WeatherText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Weather"));
			if (WeatherText)
			{
				InfoRow->AddChild(WeatherText);
				if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(WeatherText->Slot))
				{
					HSlot->SetPadding(FMargin(0.0f, 0.0f, 40.0f, 0.0f));
				}
				FSlateFontInfo Font = WeatherText->GetFont();
				Font.Size = 20;
				WeatherText->SetFont(Font);
				WeatherText->SetColorAndOpacity(FSlateColor(FLinearColor::White * 0.8f));
			}

			// Vehicle
			VehicleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Vehicle"));
			if (VehicleText)
			{
				InfoRow->AddChild(VehicleText);
				FSlateFontInfo Font = VehicleText->GetFont();
				Font.Size = 20;
				VehicleText->SetFont(Font);
				VehicleText->SetColorAndOpacity(FSlateColor(CyanNeon * 0.9f));
			}
		}
	}
}

void UMGLoadingScreenWidget::CreateAnimatedBackground()
{
	if (!RootCanvas || !WidgetTree)
	{
		return;
	}

	// Solid background
	BackgroundImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Background"));
	if (BackgroundImage)
	{
		RootCanvas->AddChild(BackgroundImage);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(BackgroundImage->Slot))
		{
			Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			Slot->SetOffsets(FMargin(0.0f));
		}
		BackgroundImage->SetColorAndOpacity(BackgroundColor);
	}

	// Grid overlay would be added here for Y2K aesthetic
	// Would need custom material or tiled texture
}

// ==========================================
// UPDATE
// ==========================================

void UMGLoadingScreenWidget::UpdateProgressAnimation(float MGDeltaTime)
{
	// Smooth progress bar
	DisplayedProgress = FMath::FInterpTo(DisplayedProgress, TargetProgress, DeltaTime, ProgressBarSmoothSpeed);

	if (ProgressBar)
	{
		ProgressBar->SetPercent(DisplayedProgress);

		// Glow effect on progress bar
		ProgressGlowTime += DeltaTime;
		float GlowPulse = 0.8f + 0.2f * FMath::Sin(ProgressGlowTime * 4.0f);
		ProgressBar->SetFillColorAndOpacity(CyanNeon * GlowPulse);
	}

	if (ProgressPercentText)
	{
		int32 PercentValue = FMath::RoundToInt(DisplayedProgress * 100.0f);
		ProgressPercentText->SetText(FText::Format(FText::FromString(TEXT("{0}%")), FText::AsNumber(PercentValue)));
	}
}

void UMGLoadingScreenWidget::UpdateTipRotation(float MGDeltaTime)
{
	if (LoadingTips.Num() <= 1)
	{
		return;
	}

	TipTimer += DeltaTime;

	if (TipTimer >= TipInterval)
	{
		TipTimer = 0.0f;
		ShowNextTip();
	}

	// Handle tip transition animation
	if (bTipTransitioning)
	{
		TipTransitionProgress += DeltaTime / TipFadeDuration;

		if (TipTransitionProgress >= 1.0f)
		{
			TipTransitionProgress = 1.0f;
			bTipTransitioning = false;
		}

		// Fade in effect
		if (TipText)
		{
			TipText->SetRenderOpacity(TipTransitionProgress);
		}
	}
}

void UMGLoadingScreenWidget::UpdateBackgroundAnimation(float MGDeltaTime)
{
	BackgroundAnimTime += DeltaTime;

	// Could animate grid overlay, scanlines, etc.
	// For now just track time for potential use
}

void UMGLoadingScreenWidget::UpdateRaceInfoDisplay()
{
	if (TrackNameText)
	{
		TrackNameText->SetText(RaceData.TrackName);
	}

	if (TrackLocationText)
	{
		TrackLocationText->SetText(RaceData.TrackLocation);
	}

	if (RaceModeText)
	{
		FText ModeText = RaceData.RaceMode;
		if (RaceData.bIsRanked)
		{
			ModeText = FText::Format(FText::FromString(TEXT("{0} - RANKED")), RaceData.RaceMode);
		}
		RaceModeText->SetText(ModeText);
	}

	if (LapCountText)
	{
		LapCountText->SetText(FText::Format(
			FText::FromString(TEXT("{0} LAPS")),
			FText::AsNumber(RaceData.LapCount)
		));
	}

	if (WeatherText)
	{
		FText WeatherAndTime = FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			RaceData.Weather,
			RaceData.TimeOfDay
		);
		WeatherText->SetText(WeatherAndTime);
	}

	if (VehicleText)
	{
		VehicleText->SetText(RaceData.PlayerVehicle);
	}

	if (TrackPreviewImage && RaceData.TrackPreviewImage)
	{
		TrackPreviewImage->SetBrushFromTexture(RaceData.TrackPreviewImage);
	}

	if (VehiclePreviewImage && RaceData.VehiclePreviewImage)
	{
		VehiclePreviewImage->SetBrushFromTexture(RaceData.VehiclePreviewImage);
	}
}

// ==========================================
// ANIMATION
// ==========================================

void UMGLoadingScreenWidget::AnimateTipTransition()
{
	bTipTransitioning = true;
	TipTransitionProgress = 0.0f;

	if (TipText)
	{
		TipText->SetRenderOpacity(0.0f);
	}
}

void UMGLoadingScreenWidget::PlayFadeIn()
{
	FadeProgress = 0.0f;
}

void UMGLoadingScreenWidget::PlayFadeOut()
{
	bLoadingComplete = true;
}
